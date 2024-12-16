#include <LiquidCrystal.h>
#include "DHT.h"

#define DHTPIN 13      
#define DHTTYPE DHT11     
#define TEMP_ANALOG A5   
#define PHOTO_DO 12       
#define PHOTO_AO A1 

LiquidCrystal lcd(7, 6, 5, 4, 3, 2);


DHT dht(DHTPIN, DHTTYPE);

const int BRIGHT_THRESHOLD = 500;
const int VERY_BRIGHT_THRESHOLD = 800;
const float TEMP_OFFSET = -2.5;     
const float TEMP_SCALE = 1.1;      


float tempDHT = 0;
float tempAnalog = 0;
float avgTemperature = 0;
float humidity = 0;
int lightValue = 0;
int lightHigh = 0;
char lightStatus[5] = "";  
char lightPower[12] = "";


unsigned long previousMillis = 0;
const long interval = 3000;  
byte displayMode = 0;      

void setup() {
  Serial.begin(9600);
  delay(100);  
  
  lcd.begin(16, 2);
  lcd.clear();
  
  dht.begin();
  pinMode(PHOTO_DO, INPUT);
  
  lcd.print("Weather Station");
  delay(2000);

  lcd.clear();
}

void loop() {
  readSensors();
  calculateAverageTemp();
  
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    displayMode = (displayMode + 1) % 3;
    lcd.clear();
    delay(100);  
  }
  
  switch(displayMode) {
    case 0:
      showScreen1();
      break;
    case 1:
      showScreen2();
      break;
    case 2:
      showScreen3();
      break;
  }
  
  printDebugInfo();
  delay(200);
}

void calibrateSensor() {
  const int numReadings = 10;
  float analogSum = 0;
  float dhtSum = 0;
  int validReadings = 0;
  
  lcd.clear();
  lcd.print("Calibrating...");
  
  for(int i = 0; i < numReadings; i++) {
    float dhtReading = dht.readTemperature();
    int sensorValue = analogRead(TEMP_ANALOG);
    float voltage = sensorValue * (5.0 / 1023.0);
    float rawTemp = voltage * 10;
    
    if (!isnan(dhtReading)) {
      analogSum += rawTemp;
      dhtSum += dhtReading;
      validReadings++;
    }
    
    delay(1000);
  }
  
  if (validReadings > 0) {
    float analogAvg = analogSum / validReadings;
    float dhtAvg = dhtSum / validReadings;
    
    Serial.println("Calibration Data:");
    Serial.print("Analog Average: ");
    Serial.println(analogAvg);
    Serial.print("DHT Average: ");
    Serial.println(dhtAvg);
    Serial.print("Suggested Offset: ");
    Serial.println(dhtAvg - analogAvg);
  }
}

void readSensors() {
  humidity = dht.readHumidity();
  tempDHT = dht.readTemperature();
  
  int sensorValue = analogRead(TEMP_ANALOG);
  float voltage = sensorValue * (5.0 / 1023.0);
  float rawTemp = voltage * 10;
  
  tempAnalog = (rawTemp * TEMP_SCALE) + TEMP_OFFSET;
  
  lightValue = analogRead(PHOTO_DO);
  lightHigh = digitalRead(PHOTO_AO);
  
  if(lightHigh==0) strcpy(lightPower,"Power Light");
    else strcpy(lightPower,"Normal Light");
  
  if(lightValue < BRIGHT_THRESHOLD) {
    strcpy(lightStatus, " LOW");
  } else if(lightValue < VERY_BRIGHT_THRESHOLD) {
    strcpy(lightStatus, " MED");
  } else {
    strcpy(lightStatus, " HIGH");
  }
  
  if (isnan(humidity) || isnan(tempDHT)) {
    tempDHT = 0;
    humidity = 0;
  }
}

void calculateAverageTemp() {
  if (tempDHT != 0) {
    avgTemperature = (tempDHT + tempAnalog) / 2.0;
  } else {
    avgTemperature = tempAnalog;
  }
}

void showScreen1() {
  lcd.setCursor(0, 0);
  lcd.print("Temp Media:");
  lcd.setCursor(0, 1);
  lcd.print(avgTemperature, 1);
  lcd.print("C");
}

void showScreen2() {
  lcd.setCursor(0, 0);
  lcd.print("T1:");
  lcd.print(tempAnalog, 1);
  lcd.print(" T2:");
  lcd.print(tempDHT, 1);
  lcd.setCursor(0, 1);
  lcd.print("Temp: A & DHT11");
}

void showScreen3() {
  lcd.setCursor(0, 0);
  lcd.print("Umid:");
  lcd.print(humidity, 1);
  lcd.print("%");
  lcd.setCursor(0, 1);
  lcd.print("Lumina:");
  lcd.print(lightStatus);
  lcd.setCursor(0, 1);
  lcd.print(lightPower);
}

void printDebugInfo() {
  Serial.print("Analog Temp: ");
  Serial.print(String(tempAnalog));
  Serial.print("°C, DHT Temp: ");
  Serial.print(String(tempDHT));
  Serial.print("°C, Avg Temp: ");
  Serial.print(String(avgTemperature));
  Serial.print("°C, Humidity: ");
  Serial.print(String(humidity));
  Serial.print("%, Light: ");
  Serial.print(String(lightValue));
  Serial.print(" (");
  Serial.print(String(lightStatus));
  Serial.print("), ");
  Serial.print(lightPower);
  Serial.print(" [");
  Serial.print(lightHigh);  
  Serial.println("]");
  delay(500);
}