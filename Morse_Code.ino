#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Initialize the LCD with I2C address 0x27, 16 columns and 2 rows
LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long signal_len, t1, t2; // Time for which button is pressed
int inputPin = 2;                 // Input pin for push button
int ledPin = 13;                  // Output pin for LED
String code = "";                 // String in which one alphabet is stored
String decodedMessage = "";       // Accumulated decoded message
const unsigned long resetDuration = 3000;  // Duration to press the button to reset (3 seconds)
const unsigned long spaceDuration = 3000;  // Minimum duration to press the button to add a space (3 seconds)
const unsigned long clearDuration = 10000; // Minimum duration to press the button to clear (10 seconds)

void setup() {
  Serial.begin(9600);
  pinMode(inputPin, INPUT_PULLUP); // Internal pullup resistor is used to simplify the circuit
  pinMode(ledPin, OUTPUT);

  // Initialize the LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Morse Decoder");
  delay(2000); // Display the title for 2 seconds
  lcd.clear(); // Clear the screen after displaying the title
}

void loop() {
NextDotDash:
  while (digitalRead(inputPin) == HIGH) {}
  t1 = millis();                            // Time at button press
  digitalWrite(ledPin, HIGH);               // LED on while button pressed
  while (digitalRead(inputPin) == LOW) {}
  t2 = millis();                            // Time at button release
  digitalWrite(ledPin, LOW);                // LED off on button release
  signal_len = t2 - t1;                     // Time for which button is pressed
  Serial.print("Button press duration: ");
  Serial.println(signal_len);               // Debug: Print button press duration
  if (signal_len > 50)                      // To account for switch debouncing
  {
    char morseChar = readio();              // Function to read dot, dash, space, or clear the screen
    if (morseChar == '\0') {
      return; // Screen cleared, exit the loop
    }
    if (morseChar == ' ') {
      decodedMessage += ' '; // Add space to the decoded message
    } else {
      code += morseChar;
    }
  }
  while ((millis() - t2) < 500)             // If time between button press greater than 0.5sec, skip loop and go to next alphabet
  {     
    if (digitalRead(inputPin) == LOW)
    {
      goto NextDotDash;
    }
  }
  convertor();                          // Function to decipher code into alphabet
}

char readio() {
  if (signal_len < 600 && signal_len > 50) {
    return '.';                        // If button press less than 0.6sec, it is a dot
  } else if (signal_len >= 600 && signal_len < spaceDuration) {
    return '-';                        // If button press more than 0.6sec but less than 3 seconds, it is a dash
  } else if (signal_len >= spaceDuration && signal_len < clearDuration) {
    Serial.println("Space detected");  // Debug: Print space detection
    return ' ';                        // If button press between 3 and 10 seconds, it is a space
  } else if (signal_len >= clearDuration) {
    clearScreen();
    return '\0';                       // Special character to indicate screen clear
  }
  return '\0';                         // Return null character if none of the conditions are met
}

void convertor() {
  static String letters[] = {".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-",
                             ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..", "E"};
  int i = 0;
  if (code == ".-.-.-") {
    decodedMessage += ". "; // For break, add space
  } else {
    while (letters[i] != "E")  // Loop for comparing input code with letters array
    {
      if (letters[i] == code) {
        decodedMessage += char('A' + i); // Append decoded character to the message
        break;
      }
      i++;
    }
    if (letters[i] == "E") {
      decodedMessage += "";  // If input code doesn't match any letter, error
    }
  }
  code = "";                            // Reset code to blank string
  updateLCD();                          // Update LCD with the full message
}

void updateLCD() {
  lcd.clear();                           // Clear previous content on the LCD
  lcd.setCursor(0, 0);                   // Set cursor to the beginning of the first line
  
  int maxCharsPerLine = 16;              // Maximum characters per line for a 16x2 LCD
  int msgLength = decodedMessage.length();
  
  if (msgLength > maxCharsPerLine * 2) { // If the message exceeds the display capacity, truncate it
    decodedMessage = decodedMessage.substring(msgLength - maxCharsPerLine * 2);
    msgLength = decodedMessage.length();
  }

  // Print the first line
  if (msgLength > maxCharsPerLine) {
    lcd.print(decodedMessage.substring(0, maxCharsPerLine));
    // Print the second line
    lcd.setCursor(0, 1);
    lcd.print(decodedMessage.substring(maxCharsPerLine));
  } else {
    lcd.print(decodedMessage);           // Print the full message if it fits within the first line
  }
}

void resetDecoder() {
  decodedMessage = "";  // Clear the accumulated message
  lcd.clear();          // Clear the LCD screen
  lcd.setCursor(0, 0);
  lcd.print("Morse Decoder"); // Optionally, display the title again
}

void clearScreen() {
  decodedMessage = "";  // Clear the accumulated message
  code = "";            // Clear the current code
  lcd.clear();          // Clear the LCD screen
  lcd.setCursor(0, 0);
  lcd.print("Morse Decoder"); // Optionally, display the title again
}
