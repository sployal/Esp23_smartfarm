#include <WiFi.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

const char *ssid = "Griffon";
const char *password = "10481reban";
const char *firebaseHost = "https://fir-demoapp-6f388-default-rtdb.firebaseio.com";

// Fixed constructor for New-LiquidCrystal library
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
DHT dht(15, DHT11);

unsigned long lastUpload = 0, lastDisplay = 0;
float temp = 0, hum = 0;
int soil = 0;
bool err = false;

void uploadToFirebase() {
  if (err) return;
  
  digitalWrite(12, HIGH);
  
  HTTPClient http;
  
  // Upload current temperature
  http.begin(String(firebaseHost) + "/sensorData/temperature.json");
  http.PUT(String(temp));
  http.end();
  
  // Upload current humidity
  http.begin(String(firebaseHost) + "/sensorData/humidity.json");
  http.PUT(String(hum));
  http.end();
  
  // Upload current soil moisture
  http.begin(String(firebaseHost) + "/sensorData/soilMoisture.json");
  http.PUT(String(soil));
  http.end();
  
  // Upload to history
  String json = "{\"temperature\":" + String(temp) + 
                ",\"humidity\":" + String(hum) + 
                ",\"soilMoisture\":" + String(soil) + 
                ",\"timestamp\":" + String(millis()) + "}";
  
  http.begin(String(firebaseHost) + "/sensorData/history/" + String(millis()) + ".json");
  http.PUT(json);
  http.end();
  
  Serial.println("Data uploaded");
  
  delay(200);
  digitalWrite(12, LOW);
}

void setup() {
  Serial.begin(115200);
  
  // Fixed: use begin() instead of init()
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.print("Davie Electronics");
  delay(2000);
  
  dht.begin();
  pinMode(34, INPUT);
  pinMode(12, OUTPUT);
  digitalWrite(12, LOW);
  
  lcd.clear();
  lcd.print("Connecting WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  
  lcd.clear();
  lcd.print("WiFi Connected");
  delay(1000);
  
  lcd.clear();
  lcd.print("Ready!");
  delay(1000);
  lcd.clear();
}

void loop() {
  unsigned long t = millis();
  
  if (t - lastDisplay >= 1000) {
    temp = dht.readTemperature();
    hum = dht.readHumidity();
    soil = analogRead(34);
    err = (isnan(temp) || isnan(hum));
    
    lcd.setCursor(0, 0);
    if (err) {
      lcd.print("Sensor Error    ");
      lcd.setCursor(0, 1);
      lcd.print("                ");
    } else {
      lcd.print("T:");
      lcd.print(temp, 1);
      lcd.print("C H:");
      lcd.print(hum, 0);
      lcd.print("%  ");
      lcd.setCursor(0, 1);
      lcd.print("Soil: ");
      lcd.print(soil);
      lcd.print("       ");
    }
    lastDisplay = t;
  }
  
  if (t - lastUpload >= 5000) {
    uploadToFirebase();
    lastUpload = t;
  }
}