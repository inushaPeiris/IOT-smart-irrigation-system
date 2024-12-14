#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <WiFi.h>
#include "DHT.h"

// #define WIFI_SSID "Wokwi-GUEST"
// #define WIFI_PASSWORD ""
#define WIFI_SSID "S20@WIFI"
#define WIFI_PASSWORD ""

// Adafruit IO details
#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883
#define AIO_USERNAME "inusha"
#define AIO_KEY ""

// Pins
#define DHTPIN 14
#define LEDPIN 5
#define DHTTYPE DHT11

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// WiFi and MQTT clients
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// MQTT feeds
Adafruit_MQTT_Publish humidityFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");
Adafruit_MQTT_Publish temperatureFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");
Adafruit_MQTT_Publish actuatorFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/actuator");
Adafruit_MQTT_Subscribe actuatorControl = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/actuator");

// Track current LED state
int currentLEDState = LOW;

void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi connected!");
}

void connectMQTT() {
  while (mqtt.connect() != 0) {
    Serial.println("MQTT connection failed. Retrying...");
    delay(5000);
  }
  Serial.println("MQTT connected!");
}

void setup() {
  Serial.begin(115200);
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, LOW);
  dht.begin();
  
  Serial.println("Setup started");

  connectWiFi();
  mqtt.subscribe(&actuatorControl);

  Serial.println("Setup completed");
}

 void loop() {
  // Reconnect MQTT if needed
  if (!mqtt.connected()) {
    connectMQTT();
  }
  
  delay(500);  // Add a delay here to give the system a chance to stabilize

  mqtt.processPackets(1000);  // process MQTT packets

  delay(500);  // Add another delay here

  // Read sensor data and publish (humidity, temperature)
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Check if the sensor readings are valid
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    humidityFeed.publish(humidity);
    temperatureFeed.publish(temperature);

    Serial.print("Humidity: ");
    Serial.println(humidity);
    Serial.print("Temperature: ");
    Serial.println(temperature);
  }


  // Control actuator based on feed
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    Serial.println("Inside subscription");
    if (subscription == &actuatorControl) {
      // Print received state for debugging
      Serial.println("Actuator feed received: ");
      Serial.println((char *)actuatorControl.lastread);

      int receivedState = atoi((char *)actuatorControl.lastread);  // Convert to int
      Serial.println(receivedState);
      
      // Only update if state has changed
      if (receivedState != currentLEDState) {
        currentLEDState = receivedState;
        digitalWrite(LEDPIN, currentLEDState);  // Update the LED state
        
        // Optionally publish back the state to the actuator feed
        actuatorFeed.publish(String(currentLEDState).c_str());
        
        Serial.println(currentLEDState ? "LED ON" : "LED OFF");
      }
    }
  }

  delay(2000);  // Delay for stability
}
