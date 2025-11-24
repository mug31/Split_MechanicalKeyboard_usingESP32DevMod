/*
 * Kode Bluetooth Keyboard Matrix V4
 * Matrix Fisik: 4 Baris (Rows) x 5 Kolom (Cols)
 * Logika Layer:
 * 1. Base (Layer 0)
 * 2. Shifted (Layer 1) - Diaktifkan oleh tombol R4/C2
 * 3. Fn/S-Key (Layer 2) - Diaktifkan oleh tombol R4/C3
 */

// 1. IMPORT LIBRARY-NYA
#include <BleKeyboard.h>

// Nama Bluetooth saat pairing
BleKeyboard bleKeyboard("Keyboard 4x5 Final");

// ==========================================
// 2. DEFINISI PETA KEYMAP & DIMENSI
// ==========================================

const int numRows = 4;
const int numCols = 5;

// --- LAYER 0 (Base Keys - Huruf Kecil) ---
// Note: Posisi Modifiers (R3/C1, R3/C2, R3/C3) akan di-handle secara terpisah.
char layer0_keymap[numRows][numCols] = {
  {'q', 'w', 'e', 'r', 't'}, // R1
  {'a', 's', 'd', 'f', 'g'}, // R2
  {'z', 'x', 'c', 'v', 'b'}, // R3
  {0, 0, 0, 0, 0}            // R4: Modifiers (Ctrl, Shift, S-Key), sisa kolom 0
};

// --- LAYER 2 (Fn/S-Key Layer - Symbols FIX!) ---
// Kita gunakan key code langsung. Layer ini aktif JIKA S-Key (Fn) ditekan.
// Mapping Sesuai permintaan Mukti: !@#()$%^-_&*C+=
const uint8_t layer2_keymap[numRows][numCols] = {
  {'!', '@', '#', '(', ')'}, // R1: ! @ # ( )
  {'$', '%', '^', '-', '_'}, // R2: $ % ^ - _ 
  {'&', '*', 'C', '+', '='}, // R3: & * C + =
  {0, 0, 0, 0, 0}            // R4: Modifiers (Diabaikan)
};

// ==========================================
// 3. DEFINISI POSISI MODIFIER & PIN
// ==========================================

// Posisi Tombol Modifiers di Matriks (Indeks dimulai dari 0)
// Modifiers ada di Baris 4 (Indeks 3)
const int MOD_ROW = 3;

// Posisi Tombol Modifier untuk Ctrl, Shift, dan S-Key
const int CTRL_COL = 0; // R4/C1 -> Ctrl
const int SHIFT_COL = 1; // R4/C2 -> Shift
const int FN_COL = 2; // R4/C3 -> S-Key/Fn

// --- PIN DEFINITION ---
const int rowPins[numRows] = {12, 13, 14, 15};    // GPIO 12, 13, 14, 15 (4 Baris)
const int colPins[numCols] = {16, 17, 18, 19, 21}; // GPIO 16, 17, 18, 19, 21 (5 Kolom)

bool lastKeyState[numRows][numCols];
bool isFnPressed = false; // State untuk S-Key (Fn)

// ==========================================
// SETUP
// ==========================================
void setup() {
  Serial.begin(115200);
  Serial.println("Mulai Uji Coba Keyboard BLE 4x5 (3 Layer Logics)...");

  // --- Setup Pin Baris (Row) sebagai INPUT_PULLUP ---
  for (int r = 0; r < numRows; r++) {
    pinMode(rowPins[r], INPUT_PULLUP);
  }
  
  // --- Setup Pin Kolom (Col) sebagai OUTPUT dan set HIGH ---
  for (int c = 0; c < numCols; c++) {
    pinMode(colPins[c], OUTPUT);
    digitalWrite(colPins[c], HIGH);
  }
  
  // Set semua status awal
  for (int r = 0; r < numRows; r++) {
    for (int c = 0; c < numCols; c++) {
      lastKeyState[r][c] = false;
    }
  }

  // Nyalain Keyboard Bluetooth-nya!
  bleKeyboard.begin();
  Serial.println("Keyboard Bluetooth siap di-pairing!");
}


// ==========================================
// LOOP (Logika Pemindaian Matriks)
// ==========================================
void loop() {
  if (bleKeyboard.isConnected()) {
    
    // Lakukan pemindaian matriks kolom demi kolom
    for (int c = 0; c < numCols; c++) {
      // Set kolom saat ini LOW (aktif)
      digitalWrite(colPins[c], LOW); 

      // Baca semua baris di kolom ini
      for (int r = 0; r < numRows; r++) {
        
        // Baca kondisi pin saat ini
        bool keyIsPressed = (digitalRead(rowPins[r]) == LOW);

        // Hanya proses jika ada perubahan state
        if (keyIsPressed != lastKeyState[r][c]) {
          
          // Cek apakah tombol yang sedang diproses ada di baris Modifiers (R4)
          bool isModifierRow = (r == MOD_ROW);

          if (keyIsPressed) {
            // === LOGIKA PRESS (DITEKAN) ===
            
            if (isModifierRow) {
              // Jika ini adalah baris Modifiers (R4)
              if (c == CTRL_COL) {
                // R4/C1: CTRL
                bleKeyboard.press(KEY_LEFT_CTRL);
                Serial.println("MODIFIER: CTRL Ditekan.");
              } else if (c == SHIFT_COL) {
                // R4/C2: SHIFT (Aktifkan Layer 1)
                bleKeyboard.press(KEY_LEFT_SHIFT);
                Serial.println("MODIFIER: SHIFT Ditekan (Aktivasi Layer Kapital).");
              } else if (c == FN_COL) {
                // R4/C3: S-KEY/FN (Aktifkan Layer 2)
                isFnPressed = true;
                Serial.println("MODIFIER: FN Ditekan (Aktivasi Layer 2/Symbols).");
              }
            } else {
              // Ini adalah tombol Huruf/Base (R1-R3)
              if (isFnPressed) {
                // Jika S-Key (Fn) sedang ditekan, kirim Layer 2 Key (Symbols)
                uint8_t key = layer2_keymap[r][c];
                bleKeyboard.press(key);
                Serial.print("Ditekan Layer 2 (FN/Symbol): ");
                Serial.println((char)key); // Cast to char untuk Serial.println
              } else {
                // Jika S-Key (Fn) TIDAK ditekan, kirim Layer 0 Key (Huruf)
                // Jika tombol Shift (R4/C2) ditekan, otomatis menjadi kapital.
                char key = layer0_keymap[r][c];
                bleKeyboard.press(key);
                Serial.print("Ditekan Layer 0/1: ");
                Serial.println(key);
              }
            }
          } 
          else {
            // === LOGIKA RELEASE (DILEPAS) ===
            
            if (isModifierRow) {
              // Jika ini adalah baris Modifiers (R4)
              if (c == CTRL_COL) {
                // R4/C1: CTRL
                bleKeyboard.release(KEY_LEFT_CTRL);
                Serial.println("MODIFIER: CTRL Dilepas.");
              } else if (c == SHIFT_COL) {
                // R4/C2: SHIFT
                bleKeyboard.release(KEY_LEFT_SHIFT);
                Serial.println("MODIFIER: SHIFT Dilepas.");
              } else if (c == FN_COL) {
                // R4/C3: S-KEY/FN
                isFnPressed = false;
                Serial.println("MODIFIER: FN Dilepas (Kembali ke Layer 0/1).");
              }
            } else {
              // Ini adalah tombol Huruf/Base (R1-R3)
              if (isFnPressed) {
                // Lepas Layer 2 Key (Symbols)
                uint8_t key = layer2_keymap[r][c];
                bleKeyboard.release(key);
              } else {
                // Lepas Layer 0/1 Key
                char key = layer0_keymap[r][c];
                bleKeyboard.release(key);
              }
              Serial.println("...Tombol Dilepas.");
            }
          }
          
          // Simpan state terakhir
          lastKeyState[r][c] = keyIsPressed; 
        }
      }
      
      // Setelah membaca semua baris, set kolom kembali HIGH (nonaktif)
      digitalWrite(colPins[c], HIGH); 
    }
  }
  delay(10); // Debounce delay
}
