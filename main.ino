#include <Adafruit_LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

//set the height of the building and the standard temperature
const float buildingHeight = 2.5;
const float standardTemperature = 70.0;

int seconds = 0;

//for Ultrasonic sensor
const int pingPin = 7;

//To get the remained battery percentage
const int batteryPin = A0;

//temperature sensor
const int temperatureSensorPin = A1;

//for Piezo alarm
const int buzzerPin = 9;

//for GSM sensor pins
const int GSM_TX_PIN = 2;
const int GSM_RX_PIN = 3;
SoftwareSerial gsmSerial(GSM_TX_PIN, GSM_RX_PIN);

//for GPS sensor pins
const int GPS_TX_PIN = 4;
const int GPS_RX_PIN = 5;
SoftwareSerial gpsSerial(GPS_TX_PIN, GPS_RX_PIN);
TinyGPSPlus gps;

// LCD to show the status of the Arduino
Adafruit_LiquidCrystal lcd_1(0);


void setup()
{
  //initialize LCD
  lcd_1.begin(16, 2);
  //Initialize serial communication
  Serial.begin(9600);
  //Baud rate for GSM communication
  gsmSerial.begin(9600);
  delay(2000);
  gsmSerial.println("AT");
  delay(1000);
  gsmSerial.println("AT+CMGF=1");
  delay(1000);
  //Baud rate for GPS communication
  gpsSerial.begin(9600);

  //introduce myself
  lcd_1.setCursor(0, 0);
  lcd_1.print("Sepehr LATIFI AZAD");
  lcd_1.setCursor(5, 1);
  lcd_1.print("190254082");
  delay(500);
  for(int position = 0; position < 4; position++){
  	lcd_1.scrollDisplayLeft();
    delay(500);
  }
  delay(1000);
  lcd_1.clear();

  //show the sensor status on the LCD
  
  //Distance measurement
  lcd_1.setCursor(0, 0);
  lcd_1.print("D: ");
  
  //Battery percentage
  lcd_1.setCursor(9, 0);
  lcd_1.print("% ");
  
  //Temperature measurement
  lcd_1.setCursor(0, 1);
  Serial.print("T: ");
  
  //Set the buzzer pin as output
  pinMode(buzzerPin, OUTPUT);

  
}

void loop()
{
  //for ultrasonic sensor
  float duration;
  float meter;
  
  //for battery percentage
  int batteryRaw = analogRead(batteryPin);  
  float batteryVoltage = map(batteryRaw, 0, 1023, 0, 9.0);
  int batteryPercentage = map(batteryVoltage, 6.0, 9.0, 0, 100);
  
  //for temperature sensor
  int temperatureSensorPinValue = analogRead(temperatureSensorPin);
  float temperatureSensorPinVoltage = (temperatureSensorPinValue * 5.0) / 1023.0;
  float temperatureC = (temperatureSensorPinVoltage - 0.5) * 100.0;
  
  //initialize the ultrasonic sensor and get the current height of the building
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);
  pinMode(pingPin, INPUT);
  duration = pulseIn(pingPin, HIGH);
  meter = microsecondsToMeter(duration);

  //show the distance on the LCD
  lcd_1.setCursor(3, 0);
  lcd_1.print(meter);

  //show the battery percentage on the LCD
  lcd_1.setCursor(10, 0);
  lcd_1.print(batteryPercentage);
  
  //show the temperature on the LCD
  lcd_1.setCursor(0, 1);
  lcd_1.print(temperatureC);
  lcd_1.print(" C");

  //check if the house is collapsed
  if(meter < buildingHeight){
    while (gpsSerial.available() > 0) {
      if (gps.encode(gpsSerial.read())) {
        if (gps.location.isValid()) {
          // Retrieve latitude and longitude
          float latitude = gps.location.lat();
          float longitude = gps.location.lng();
          String location = "Latitude: " + String(latitude, 6) + "\nLongitude: " + String(longitude, 6);
          //check the temperature in case of fire in the building
          if(temperatureC > standardTemperature){
            //reporting the fire
            //send SMS to the emergency department
            String message = "Emergency! A building at the below coordination has collapsed. The temperature in the building is " + String(temperatureC) + "°C. An emergency team is required immediately. \n" + location;
            sendSMS("911", message.c_str());
          }else{
            //send SMS to the emergency department
            String message = "Emergency! A building at the below coordination has been collapsed, an Emergency team is required immediately. \n" + location;
            sendSMS("911", message.c_str());
          }
          while(true){
              tone(buzzerPin, 1000); // Send 1KHz sound signal
              delay(800);
  	          noTone(buzzerPin); // Stop the tone
  	          delay(200); 
          }
        }
      }
    }
  }
  
}
//convert microseconds to meter for the ultrasonic sensor
float microsecondsToMeter(float microseconds) {
  return (microseconds / 29 / 2)/100;
}

//SMS function to send SMS to the emergency department
void sendSMS(const char* phoneNumber, const char* message) {
  gsmSerial.println("AT+CMGS=\"" + String(phoneNumber) + "\"");
  delay(1000);
  gsmSerial.println(message);
  delay(100);
  gsmSerial.println((char)26);
  delay(1000);
  Serial.println("SMS sent to " + String(phoneNumber));
}