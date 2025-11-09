#include <WiFi.h>
#include <WebServer.h>
#include <FastLED.h>

#define DATA_PIN 25
#define NUM_LEDS 41

CRGB leds[NUM_LEDS];

const char* ssid = "Redmi Note 14 Pro";//RUC-IOT
const char* password = "jeanne123";//GiHa9289La

WebServer server(80);

// Function to add CORS headers
void addCORSHeaders() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

// Handle OPTIONS requests (CORS preflight)
void handleOptions() {
  addCORSHeaders();
  server.send(204);  // No Content response
}

// Handle colour change requests
void handlecolour() {
  addCORSHeaders(); // Add CORS headers to response

  if (server.hasArg("colour")) { // http://127.0.0.1:80/colour?colour=
    String colour = server.arg("colour");
    Serial.println("Received colour: " + colour);

    if (colour == "red") {
      fill_solid(leds, NUM_LEDS, CRGB::Red);
    } else if (colour == "yellow") {
      fill_solid(leds, NUM_LEDS, CRGB::Yellow);
    } else if (colour == "green") {
      fill_solid(leds, NUM_LEDS, CRGB::Green);
    } else if (colour == "black") {
      fill_solid(leds, NUM_LEDS, CRGB::Black);
    } else {
      Serial.println("Unknown colour received.");
      server.send(400, "text/plain", "Unknown colour");
      return;
    }

    FastLED.show();
    server.send(200, "text/plain", "Colour set to " + colour);
  } else {
    server.send(400, "text/plain", "Missing colour argument");
  }
}

void handleIndex() {
  addCORSHeaders(); // Add CORS headers to response
  server.send(200, "text/plain", "Hello from ESP32 Web Server");
}

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, handleIndex);
  
  // Handle colour requests using GET request
  // To use POST request, change HTTP_GET to HTTP_POST
  server.on("/colour", HTTP_GET, handlecolour); // http://127.0.01:80/colour?colour=red

  // Handle CORS preflight using OPTIONS request
  server.on("/colour", HTTP_OPTIONS, handleOptions);

  server.begin();
}

void loop() {
  server.handleClient();
}