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

String html = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <title>ESP32 LED Controller</title>
    <meta charset="UTF-8">
    <style>
      body { font-family: Arial; text-align: center; margin-top: 40px; }
      input, select, button { margin: 10px; padding: 8px; font-size: 16px; }
    </style>
  </head>
  <body>
    <h2>ESP32 LED Control</h2>
    <form action="/led" method="get">
      <label for="color">Color:</label>
      <select name="color" id="color">
        <option value="red">Red</option>
        <option value="yellow">Yellow</option>
        <option value="green">Green</option>
        <option value="blue">Blue</option>
        <option value="white">White</option>
        <option value="black">Off</option>
      </select>
      <br>
      <label for="percentage">Percentage (0–100):</label>
      <input type="number" id="percentage" name="percentage" min="0" max="100" value="50">
      <br>
      <button type="submit">Set LEDs</button>
    </form>
  </body>
  </html>
  )rawliteral";


void handleIndex() {
  addCORSHeaders();
  server.send(200, "text/html", html);
}

void handleLED() {
  addCORSHeaders();

  if (!server.hasArg("color") || !server.hasArg("percentage")) {
    server.send(400, "text/plain", "Missing color or percentage argument");
    return;
  }

  String color = server.arg("color");
  int percentage = server.arg("percentage").toInt();
  percentage = constrain(percentage, 0, 100);

  Serial.printf("Received color: %s, percentage: %d\n", color.c_str(), percentage);

  // Determine LED color
  CRGB selectedColor;
  if (color == "red") selectedColor = CRGB::Red;
  else if (color == "yellow") selectedColor = CRGB::Yellow;
  else if (color == "green") selectedColor = CRGB::Green;
  else if (color == "blue") selectedColor = CRGB::Blue;
  else if (color == "white") selectedColor = CRGB::White;
  else if (color == "black") selectedColor = CRGB::Black;
  else {
    server.send(400, "text/plain", "Unknown color");
    return;
  }

  // Compute number of LEDs to light based on percentage
  int numToLight = map(percentage, 0, 100, 0, NUM_LEDS);
  Serial.printf("Lighting %d out of %d LEDs\n", numToLight, NUM_LEDS);

  // Light LEDs
  for (int i = 0; i < NUM_LEDS; i++) {
    if (i < numToLight) leds[i] = selectedColor;
    else leds[i] = CRGB::Black;
  }
  FastLED.show();

  server.send(200, "text/html", html);
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
  server.on("/led", HTTP_GET, handleLED);
  server.on("/led", HTTP_OPTIONS, handleOptions);

  server.begin();
}

void loop() {
  server.handleClient();
}