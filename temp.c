// Sensor Transfer Function (from MCP9701A Datasheet):
// VOUT = TC × TA + V0C
// => TA = (VOUT - V0C) / TC
// where:
// VOUT = Sensor Output Voltage (Volts)
// TC = Temperature Coefficient (19.5 mV/°C)
// V0C = Output Voltage at 0°C (400 mV)
// TA = Ambient Temperature (°C)

void UART3_Init_Config(unsigned long baudrate)
{
    PPS_Mapping(69, _INPUT, _U3RX);
    PPS_Mapping(68, _OUTPUT, _U3TX);
    UART3_Init(baudrate);
    Delay_ms(100);
}

void ADC_Init() {
    AD1CON1 = 0x0000;          // Manual sampling
    AD1CON1bits.SSRC = 7;      // Auto-convert
    AD1CON2 = 0x0000;          // AVDD and AVSS as reference, single channel
    AD1CON3 = 0x0002;          // ADC clock settings
    AD1CHS0bits.CH0SA = 27;    // Select AN26 as input channel for temp A OR 27 FOR B 
    AD1CON1bits.ADON = 1;      // Turn on ADC
}

// Read ADC value
unsigned int ADC_Read() {
    AD1CON1bits.SAMP = 1;      // Start sampling
    Delay_us(10);              // Sample time
    AD1CON1bits.SAMP = 0;      // Stop sampling, start conversion
    while (!AD1CON1bits.DONE); // Wait for conversion to complete
    return ADC1BUF0;           // Return ADC result
}

void main() {
    PLLFBD = 70;
    CLKDIV = 0x0000;

    UART3_Init_Config(9600);
    UART3_Write_Text("TEST STARTED!\r\n");

    ADC_Init();

    while (1) {
        unsigned int adc_value = ADC_Read();  // Read ADC

        // Convert ADC value to voltage (VOUT = ADC × Vref / 1023)
        float VOUT = ((float)adc_value / 1023.0) * 3.21;

        // Constants based on MCP9701A datasheet
        float V0C = 0.4;       // Voltage at 0°C in Volts
        float TC = 0.0195;     // Temperature coefficient in V/°C

        // Calculate ambient temperature (TA)
        float TA = (VOUT - V0C) / TC;

        // Transmit values over UART
        char buffer[60];
        sprintf(buffer, "ADC: %d, VOUT: %.3fV, TEMP: %.2f°C\r\n", adc_value, VOUT, TA);
        UART3_Write_Text(buffer);

        Delay_ms(500);
    }
}
