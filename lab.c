void ADC_MicInit(void) {
    if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK) {
        Error_Handler();
    }
    HAL_ADC_Start_IT(&hadc1);
    HAL_TIM_Base_Start(&htim6);
}


#define LM75_I2C_ADDR    0x48
#define LM75_TEMP_REG    0x00
#define LM75_RESOLUTION  0.5f


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
    if (hadc == &hadc1) {
        int16_t sample = (int16_t)HAL_ADC_GetValue(hadc);
        sample -= (INT16_MAX >> 4);
        HAL_UART_Transmit(&hlpuart1, (uint8_t *)&sample, 2, 1);
    }
}


uint16_t LM75_ReadReg16(uint8_t RegAddr) {
    uint8_t data[2];
    HAL_I2C_Mem_Read(&hi2c1, LM75_I2C_ADDR << 1, RegAddr, I2C_MEMADD_SIZE_8BIT, data, 2, HAL_MAX_DELAY);
    return (uint16_t)((data[0] << 8) | data[1]);
}

void LM75_Test(void) {
    uint16_t tempRaw = (LM75_ReadReg16(LM75_TEMP_REG) >> 7);
    float temp = (float)tempRaw * LM75_RESOLUTION;
    printf("Temperature: %d.%d *C\n", (int)temp, (int)(temp * 10) % 10);
    HAL_Delay(1000);
}


setvbuf(stdin, NULL, _IONBF, 0);
LED_RGB_Init();
ADC_Init();
while (1) {
    LM75_Test();
    // ADC_Test();
}



typedef struct {
    int16_t x, y, z;
} ACC_RawData_t;



#define LSM303C_I2C_ADDR     0x1D
#define LSM303C_WHO_AM_I     0x0F
#define LSM303C_CTRL_REG1_A  0x20
#define LSM303C_CTRL_REG4_A  0x23
#define LSM303C_STATUS_REG_A 0x27
#define LSM303C_OUT_X_L_A    0x28
#define LSM303C_ACC_ID       0x41



void LSM303C_Init(void) {
    uint8_t regValue = 0x00;
    HAL_I2C_Mem_Read(&hi2c3, LSM303C_I2C_ADDR << 1, LSM303C_WHO_AM_I,
                     I2C_MEMADD_SIZE_8BIT, &regValue, 1, HAL_MAX_DELAY);
    if (regValue != LSM303C_ACC_ID) {
        printf("LSM303C not found - Error Handler\n");
        Error_Handler();
    } else {
        regValue = 0x37;
        HAL_I2C_Mem_Write(&hi2c3, LSM303C_I2C_ADDR << 1, LSM303C_CTRL_REG1_A,
                          I2C_MEMADD_SIZE_8BIT, &regValue, 1, HAL_MAX_DELAY);

        regValue = 0x04;
        HAL_I2C_Mem_Write(&hi2c3, LSM303C_I2C_ADDR << 1, LSM303C_CTRL_REG4_A,
                          I2C_MEMADD_SIZE_8BIT, &regValue, 1, HAL_MAX_DELAY);
    }
}




void LSM303C_Test(void) {
    uint8_t statusReg;
    ACC_RawData_t acc;
    uint16_t size = sizeof(acc);
    uint8_t *pAcc = (uint8_t *)&acc;

    HAL_I2C_Mem_Read(&hi2c3, LSM303C_I2C_ADDR << 1, LSM303C_STATUS_REG_A,
                     I2C_MEMADD_SIZE_8BIT, &statusReg, 1, HAL_MAX_DELAY);
    if (statusReg & 0x08) {
        HAL_I2C_Mem_Read(&hi2c3, LSM303C_I2C_ADDR << 1, LSM303C_OUT_X_L_A | 0x80,
                         I2C_MEMADD_SIZE_8BIT, pAcc, size, HAL_MAX_DELAY);
        HAL_UART_Transmit_DMA(&hlpuart1, pAcc, size);
    }
}



setvbuf(stdin, NULL, _IONBF, 0);
LED_RGB_Init();
ADC_Init();
LSM303C_Init();
while (1) {
    LSM303C_Test();
    // LM75_Test();
}


