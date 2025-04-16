#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL3HQnEVDXA"
#define BLYNK_TEMPLATE_NAME "WEATHER STATION"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "DHT.h"

// Blynk Auth Token
char auth[] = "q6iDpq9A-X0KHXbUNg21rAjcXD_YQS7R";

// WiFi Credentials
char ssid[] = "Srambiyan";
char pass[] = "qwerty@123";

// DHT Sensor Configuration
#define DHTPIN 32
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Hardware Pin Definitions
#define LED_PIN 2
#define RELAY1_PIN 26
#define RELAY2_PIN 27
#define CONTROL_PIN 5

BlynkTimer timer;

// Ensure WiFi connectivity
void connectWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, pass);
    int attempt = 0;
    while (WiFi.status() != WL_CONNECTED && attempt < 20) {
      delay(500);
      Serial.print(".");
      attempt++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected to WiFi!");
    } else {
      Serial.println("\nWiFi connection failed!");
    }
  }
}

// Read DHT and send data to Blynk
void sendSensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("DHT Sensor Read Failed. Retrying...");
    delay(500);
    h = dht.readHumidity();
    t = dht.readTemperature();
    if (isnan(h) || isnan(t)) {
      Serial.println("DHT Sensor Failed after retry.");
      return;
    }
  }

  Serial.print("Humidity: "); Serial.print(h);
  Serial.print(" % | Temperature: "); Serial.print(t);
  Serial.println(" °C");

  Blynk.virtualWrite(V5, h);
  Blynk.virtualWrite(V6, t);

  digitalWrite(LED_PIN, (h >= 80.0) ? HIGH : LOW);

  if (t >= 32.0) {
    digitalWrite(CONTROL_PIN, HIGH);
    Serial.println("Pin 5 ON (Temperature ≥ 40°C)");
  } else {
    digitalWrite(CONTROL_PIN, LOW);
    Serial.println("Pin 5 OFF (Temperature < 40°C)");
  }
}

// Relay 1 control via Blynk (V7)
BLYNK_WRITE(V7) {
  int relay1State = param.asInt();
  digitalWrite(RELAY1_PIN, relay1State);
  Serial.print("Relay 1 (Pin 26) is now ");
  Serial.println(relay1State ? "ON" : "OFF");
}

// Relay 2 control via Blynk (V8)
BLYNK_WRITE(V8) {
  int relay2State = param.asInt();
  digitalWrite(RELAY2_PIN, relay2State);
  Serial.print("Relay 2 (Pin 27) is now ");
  Serial.println(relay2State ? "ON" : "OFF");
}

void setup() {
  Serial.begin(115200);

  // Pin Modes
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(CONTROL_PIN, OUTPUT);

  // Default States
  digitalWrite(LED_PIN, LOW);
  digitalWrite(RELAY1_PIN, LOW);
  digitalWrite(RELAY2_PIN, LOW);
  digitalWrite(CONTROL_PIN, LOW);

  // Start Blynk and DHT
  Blynk.begin(auth, ssid, pass);
  dht.begin();

  // Send sensor data every 2 seconds
  timer.setInterval(2000L, sendSensor);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!Blynk.connected()) {
    Blynk.connect();
  }

  Blynk.run();
  timer.run();
}