// This ESPHome component wraps around the modbus-esp8266 by @emelianov:
// https://github.com/emelianov/modbus-esp8266
//
// by @jpeletier - Epic Labs, 2022

#include "modbus_server.h"

#define TAG "ModbusServer"

namespace esphome {
namespace modbus_server {

ModbusServer::ModbusServer() {}

uint32_t ModbusServer::baudRate() { return this->parent_->get_baud_rate(); }

void ModbusServer::setup() { mb.begin(this); }

void ModbusServer::set_re_pin(GPIOPin *re_pin) {
    if (re_pin != nullptr) {
      re_pin_ = re_pin;
      re_pin_->setup();
      re_pin_->digital_write(LOW);
//      ESP_LOGD(TAG, "set_re_pin(): re_pin_ -> LOW");
    }
}

void ModbusServer::set_de_pin(GPIOPin *de_pin) {
  if (de_pin != nullptr) {
    de_pin_ = de_pin;
    de_pin_->setup();
    de_pin_->digital_write(LOW);
//    ESP_LOGD(TAG, "set_de_pin(): de_pin_ -> LOW");
  }
}

void ModbusServer::set_address(uint8_t address) { mb.slave(address); }

bool ModbusServer::add_holding_register(uint16_t start_address, uint16_t value, uint16_t numregs) {
  return mb.addHreg(start_address, value, numregs);
}

bool ModbusServer::add_input_register(uint16_t start_address, uint16_t value, uint16_t numregs) {
  return mb.addIreg(start_address, value, numregs);
}    

bool ModbusServer::write_holding_register(uint16_t address, uint16_t value) { return mb.Hreg(address, value); }

bool ModbusServer::write_input_register(uint16_t address, uint16_t value) { return mb.Ireg(address, value); }    

uint16_t ModbusServer::read_holding_register(uint16_t address) { return mb.Hreg(address); }
    
uint16_t ModbusServer::read_input_register(uint16_t address) { return mb.Ireg(address); }
    
void ModbusServer::on_read_holding_register(uint16_t address, cbOnReadWrite cb, uint16_t numregs) {
  mb.onGet(
      HREG(address), [cb](TRegister *reg, uint16_t val) -> uint16_t { return cb(reg->address.address, val); }, numregs);
}

void ModbusServer::on_read_input_register(uint16_t address, cbOnReadWrite cb, uint16_t numregs) {
  mb.onGet(
      IREG(address), [cb](TRegister *reg, uint16_t val) -> uint16_t { return cb(reg->address.address, val); }, numregs);
}    
    
void ModbusServer::on_write_holding_register(uint16_t address, cbOnReadWrite cb, uint16_t numregs) {
  mb.onSet(
      HREG(address), [cb](TRegister *reg, uint16_t val) -> uint16_t { return cb(reg->address.address, val); }, numregs);
}

void ModbusServer::on_write_input_register(uint16_t address, cbOnReadWrite cb, uint16_t numregs) {
  mb.onSet(
      IREG(address), [cb](TRegister *reg, uint16_t val) -> uint16_t { return cb(reg->address.address, val); }, numregs);
}

    
// Stream class implementation:
size_t ModbusServer::write(uint8_t data) {
    if (( (re_pin_ != nullptr) || (de_pin_ != nullptr) ) && !sending) {
        if (re_pin_ != nullptr)
          re_pin_->digital_write(HIGH);
//          ESP_LOGV(TAG, "write(): re_pin_ -> HIGH");
        if (de_pin_ != nullptr)
          de_pin_->digital_write(HIGH);
//          ESP_LOGV(TAG, "write(): de_pin_ -> HIGH");
    sending = true;
  }
  return uart::UARTDevice::write(data);
}
int ModbusServer::available() { return uart::UARTDevice::available(); }
int ModbusServer::read() { return uart::UARTDevice::read(); }
int ModbusServer::peek() { return uart::UARTDevice::peek(); }
void ModbusServer::flush() {
    uart::UARTDevice::flush();
    if (( (re_pin_ != nullptr) || (de_pin_ != nullptr) ) && sending) {
        if (re_pin_ != nullptr)
          re_pin_->digital_write(LOW);
//          ESP_LOGV(TAG, "flush(): re_pin_ -> LOW");
        if (de_pin_ != nullptr)
          de_pin_->digital_write(LOW);
//          ESP_LOGV(TAG, "flush(): de_pin_ -> LOW");
    sending = false;
  }
}

void ModbusServer::loop() { mb.task(); };
}  // namespace modbus_server

}  // namespace esphome
