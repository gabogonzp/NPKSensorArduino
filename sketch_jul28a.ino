#include <SoftwareSerial.h>
#include <ModbusMaster.h> // Include the ModbusMaster library

// Define the RS485 control pins.
// Connect MAX485 DE pin to Arduino Digital Pin 7
// Connect MAX485 RE pin to Arduino Digital Pin 8
#define MAX485_DE_PIN 7 
#define MAX485_RE_PIN 8

// Create a SoftwareSerial object for Modbus communication
// Connect MAX485 RO to Arduino Digital Pin 2 (RX)
// Connect MAX485 DI to Arduino Digital Pin 3 (TX)
SoftwareSerial modbusSerial(2, 3); 

// Create a ModbusMaster object
// The 'node' object represents the Modbus slave device (your NPK sensor)
ModbusMaster node;

// --- Function Prototypes ---
// These functions will handle the RS485 transmit/receive enabling
void preTransmission();
void postTransmission();

// Functions to read sensor values
float readNitrogen();
float readPhosphorous();
float readPotassium();
float readElectroconductivity();
float readPH();
float readTemperature();

void setup() {
  // Initialize the standard Serial port for debugging output
  Serial.begin(9600);
  while (!Serial); // Wait for Serial Monitor to open (optional, for some boards)
  Serial.println("NPK Sensor Reader Initializing...");

  // Initialize the SoftwareSerial port for Modbus communication
  modbusSerial.begin(9600); // Most NPK sensors default to 9600 baud rate

  // Configure the RS485 DE/RE control pins
  pinMode(MAX485_DE_PIN, OUTPUT);
  pinMode(MAX485_RE_PIN, OUTPUT); // Set RE pin as output

  // Set the ModbusMaster object to use our SoftwareSerial port
  node.begin(1, modbusSerial); // Slave ID 1 (common default for NPK sensors)

  // Assign the preTransmission and postTransmission functions
  // These functions are called by the ModbusMaster library to control the RS485 transceiver
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  // Initial state: set MAX485 to receive mode (DE LOW, RE LOW)
  digitalWrite(MAX485_DE_PIN, LOW);  // Disable driver
  digitalWrite(MAX485_RE_PIN, LOW);   // Enable receiver

  Serial.println("NPK Sensor Reader Ready.");
  delay(1000); // Give some time for everything to stabilize
}

void loop() {
  Serial.println("\n--- Reading Sensor Data ---");

  float nitrogen = readNitrogen();
  if (nitrogen != -1.0) {
    Serial.print("Nitrogen: ");
    Serial.print(nitrogen);
    Serial.println(" mg/kg");
  } else {
    Serial.println("Nitrogen: Error reading data");
  }
  delay(1000); // Delay between readings for stability

  float phosphorous = readPhosphorous();
  if (phosphorous != -1.0) {
    Serial.print("Phosphorous: ");
    Serial.print(phosphorous);
    Serial.println(" mg/kg");
  } else {
    Serial.println("Phosphorous: Error reading data");
  }
  delay(1000);

  float potassium = readPotassium();
  if (potassium != -1.0) {
    Serial.print("Potassium: ");
    Serial.print(potassium);
    Serial.println(" mg/kg");
  } else {
    Serial.println("Potassium: Error reading data");
  }
  delay(1000);

  float ec = readElectroconductivity();
  if (ec != -1.0) {
    Serial.print("Electroconductivity: ");
    Serial.print(ec);
    Serial.println(" uS/cm");
  } else {
    Serial.println("Electroconductivity: Error reading data");
  }
  delay(1000);

  float ph = readPH();
  if (ph != -1.0) {
    Serial.print("Soil pH: ");
    Serial.print(ph);
    Serial.println(" pH");
  } else {
    Serial.println("Soil pH: Error reading data");
  }
  delay(1000);

  float temperature = readTemperature();
  if (temperature != -1.0) {
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" C");
  } else {
    Serial.println("Temperature: Error reading data");
  }
  delay(2000); // Longer delay before the next full cycle
}

// This function is called by the ModbusMaster library before a transmission
void preTransmission() {
  digitalWrite(MAX485_RE_PIN, HIGH); // Disable receiver
  digitalWrite(MAX485_DE_PIN, HIGH); // Enable driver (transmit mode)
}

// This function is called by the ModbusMaster library after a transmission
void postTransmission() {
  digitalWrite(MAX485_DE_PIN, LOW);  // Disable driver
  digitalWrite(MAX485_RE_PIN, LOW);  // Enable receiver (receive mode)
}

// --- Sensor Reading Functions ---

// Reads Nitrogen (N) value
float readNitrogen() {
  // Read 1 holding register starting at address 0x1E (30 decimal)
  // The ModbusMaster library handles the CRC and response parsing
  uint8_t result = node.readHoldingRegisters(0x1E, 1); 
  if (result == node.ku8MBSuccess) {
    // The value is stored in the response buffer. 
    // For a single register read, it's at index 0.
    uint16_t rawValue = node.getResponseBuffer(0); 
    return (float)rawValue; // NPK values are typically direct readings
  } else {
    Serial.print("Modbus Error (N): ");
    Serial.println(result, HEX); // Print Modbus error code in hex
    return -1.0; // Indicate an error
  }
}

// Reads Phosphorous (P) value
float readPhosphorous() {
  // Read 1 holding register starting at address 0x1F (31 decimal)
  uint8_t result = node.readHoldingRegisters(0x1F, 1);
  if (result == node.ku8MBSuccess) {
    uint16_t rawValue = node.getResponseBuffer(0);
    return (float)rawValue;
  } else {
    Serial.print("Modbus Error (P): ");
    Serial.println(result, HEX);
    return -1.0;
  }
}

// Reads Potassium (K) value
float readPotassium() {
  // Read 1 holding register starting at address 0x20 (32 decimal)
  uint8_t result = node.readHoldingRegisters(0x20, 1);
  if (result == node.ku8MBSuccess) {
    uint16_t rawValue = node.getResponseBuffer(0);
    return (float)rawValue;
  } else {
    Serial.print("Modbus Error (K): ");
    Serial.println(result, HEX);
    return -1.0;
  }
}

// Reads Electroconductivity (EC) value
float readElectroconductivity() {
  // Read 1 holding register starting at address 0x15 (21 decimal)
  uint8_t result = node.readHoldingRegisters(0x15, 1);
  if (result == node.ku8MBSuccess) {
    uint16_t rawValue = node.getResponseBuffer(0);
    // EC values might need scaling (e.g., divide by 10 or 100)
    // Check your sensor's datasheet for the correct scaling factor for EC.
    // Assuming 1 uS/cm resolution for now, adjust if needed.
    return (float)rawValue; 
  } else {
    Serial.print("Modbus Error (EC): ");
    Serial.println(result, HEX);
    return -1.0;
  }
}

// Reads pH value
float readPH() {
  // Read 1 holding register starting at address 0x06 (6 decimal)
  uint8_t result = node.readHoldingRegisters(0x06, 1);
  if (result == node.ku8MBSuccess) {
    uint16_t rawValue = node.getResponseBuffer(0);
    // pH values often need to be divided by 100 (e.g., 725 for pH 7.25)
    // Check your sensor's datasheet for the correct scaling factor for pH.
    return (float)rawValue / 100.0; // Common scaling for pH
  } else {
    Serial.print("Modbus Error (pH): ");
    Serial.println(result, HEX);
    return -1.0;
  }
}

// Reads Temperature value
float readTemperature() {
  // Read 1 holding register starting at address 0x12 (18 decimal)
  uint8_t result = node.readHoldingRegisters(0x12, 1);
  if (result == node.ku8MBSuccess) {
    uint16_t rawValue = node.getResponseBuffer(0);
    // Temperature values often need to be divided by 10 (e.g., 250 for 25.0 C)
    // Check your sensor's datasheet for the correct scaling factor for temperature.
    return (float)rawValue / 10.0; // Common scaling for temperature
  } else {
    Serial.print("Modbus Error (Temp): ");
    Serial.println(result, HEX);
    return -1.0;
  }
}
