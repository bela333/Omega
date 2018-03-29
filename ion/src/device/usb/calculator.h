#ifndef ION_DEVICE_USB_CALCULATOR_H
#define ION_DEVICE_USB_CALCULATOR_H

#include "device.h"
#include "dfu_interface.h"
#include "stack/bos_descriptor.h"
#include "stack/configuration_descriptor.h"
#include "stack/descriptor.h"
#include "stack/device_descriptor.h"
#include "stack/dfu_functional_descriptor.h"
#include "stack/interface_descriptor.h"
#include "stack/language_id_string_descriptor.h"
#include "stack/microsoft_os_string_descriptor.h"
#include "stack/string_descriptor.h"
#include "stack/url_descriptor.h"
#include "stack/webusb_platform_descriptor.h"
#include <stddef.h>
#include <assert.h>

namespace Ion {
namespace USB {
namespace Device {

class Calculator : public Device {
public:
  static void Poll();
  Calculator() :
    Device(&m_dfuInterface),
    m_deviceDescriptor(
        0x0210, /* bcdUSB: USB Specification Number which the device complies
                 * to. Must be greater than 0x0200 to use the BOS. */
        0,      // bDeviceClass: The class is defined by the interface.
        0,      // bDeviceSUBClass: The subclass is defined by the interface.
        0,      // bDeviceProtocol: The protocol is defined by the interface.
        64,     // bMaxPacketSize0: Maximum packet size for endpoint 0
        0x0483, // idVendor
        0xA291, // idProduct
        0x0001, // bcdDevice: Device Release Number //TODO
        1,      // iManufacturer: Index of the manufacturer name string, see m_descriptor
        2,      // iProduct: Index of the product name string, see m_descriptor
        3,      // iSerialNumber: Index of the SerialNumber string, see m_descriptor
        1),     // bNumConfigurations
    m_dfuFunctionalDescriptor(
        0b1111, /* bmAttributes:
                 * - bitWillDetach: If true, the device will perform a bus
                 *   detach-attach sequence when it receives a DFU_DETACH
                 *   request. The host must not issue a USB Reset.
                 * TODO Will we attach ?
                 * - bitManifestationTolerant: device is able to communicate via
                 * USB after Manifestation phase
                 * - bitCanUpload
                 * - bitCanDnload */
        0,      /* wDetachTimeOut: Time, in milliseconds, that the device will
                 * wait after receipt of the DFU_DETACH request.
                 * We do not use it as bitWillDetach = 1.*/
        2048,   // wTransferSize: Maximum number of bytes that the device can accept per control-write transaction
        0x0100),// bcdDFUVersion
    m_interfaceDescriptor(
        0,      // bInterfaceNumber
        k_dfuInterfaceAlternateSetting,      // bAlternateSetting
        0,      // bNumEndpoints: Other than endpoint 0
        0xFE,   // bInterfaceClass: DFU (http://www.usb.org/developers/defined_class)
        1,      // bInterfaceSubClass: DFU
        2,      // bInterfaceProtocol: DFU Mode (not DFU Runtime, which would be 1)
        4,      // iInterface: Index of the Interface string, see m_descriptor
        &m_dfuFunctionalDescriptor),
    m_configurationDescriptor(
        9 + 9 + 9, // wTotalLength: configuration descriptor + interface descriptor + dfu functional descriptor lengths
        1,      // bNumInterfaces
        k_bConfigurationValue, // bConfigurationValue
        0,      // iConfiguration: No string descriptor for the configuration
        0x80,   /* bmAttributes:
                 * Bit 7: Reserved, set to 1
                 * Bit 6: Self Powered
                 * Bit 5: Remote Wakeup (allows the device to wake up the host when the host is in suspend)
                 * Bit 4..0: Reserved, set to 0 */
        0x32,   // bMaxPower: half of the Maximum Power Consumption
        &m_interfaceDescriptor),
    m_webUSBPlatformDescriptor(
        k_webUSBVendorCode,
        k_webUSBLandingPageIndex),
    m_bosDescriptor(
        5 + 24, // wTotalLength: BOS descriptor + webusb platform descriptor lengths
        1,      // bNumDeviceCapabilities
        &m_webUSBPlatformDescriptor),
    m_languageStringDescriptor(),
    m_manufacturerStringDescriptor("NumWorks"),
    m_productStringDescriptor("Calculator"),
    m_serialNumberStringDescriptor("12345"),
    m_interfaceStringDescriptor("@Flash/0x08000000/04*016Kg,01*064Kg,07*128Kg"),
    //m_interfaceStringDescriptor("@SRAM/0x20000000/01*256Ke"), //TODO: Add this descriptor to use dfu-util to write in the SRAM
    m_microsoftOSStringDescriptor(k_microsoftOSVendorCode),
    m_workshopURLDescriptor(URLDescriptor::Scheme::HTTPS, "workshop.numworks.com"),
    m_descriptors{
      &m_deviceDescriptor,             // Type = Device, Index = 0
      &m_configurationDescriptor,      // Type = Configuration, Index = 0
      &m_languageStringDescriptor,     // Type = String, Index = 0
      &m_manufacturerStringDescriptor, // Type = String, Index = 1
      &m_productStringDescriptor,      // Type = String, Index = 2
      &m_serialNumberStringDescriptor, // Type = String, Index = 3
      &m_interfaceStringDescriptor,    // Type = String, Index = 4
      &m_bosDescriptor                 // Type = BOS, Index = 0
    },
    m_dfuInterface(&m_ep0, k_dfuInterfaceAlternateSetting)
  {
  }
protected:
  virtual Descriptor * descriptor(uint8_t type, uint8_t index) override;
  virtual void setActiveConfiguration(uint8_t configurationIndex) override {
    assert(configurationIndex == k_bConfigurationValue);
  }
  virtual uint8_t getActiveConfiguration() override {
    return k_bConfigurationValue;
  }
  bool processSetupInRequest(SetupPacket * request, uint8_t * transferBuffer, uint16_t * transferBufferLength, uint16_t transferBufferMaxLength) override;

private:
  static constexpr uint8_t k_bConfigurationValue = 1;
  static constexpr uint8_t k_dfuInterfaceAlternateSetting = 0;
  static constexpr uint8_t k_webUSBVendorCode = 1;
  static constexpr uint8_t k_webUSBLandingPageIndex = 1;
  static constexpr uint8_t k_microsoftOSVendorCode = 2;
  bool getURLCommand(uint8_t * transferBuffer, uint16_t * transferBufferLength, uint16_t transferBufferMaxLength);
  DeviceDescriptor m_deviceDescriptor;
  DFUFunctionalDescriptor m_dfuFunctionalDescriptor;
  InterfaceDescriptor m_interfaceDescriptor;
  ConfigurationDescriptor m_configurationDescriptor;
  WebUSBPlatformDescriptor m_webUSBPlatformDescriptor;
  BOSDescriptor m_bosDescriptor;
  LanguageIDStringDescriptor m_languageStringDescriptor;
  StringDescriptor m_manufacturerStringDescriptor;
  StringDescriptor m_productStringDescriptor;
  StringDescriptor m_serialNumberStringDescriptor;
  StringDescriptor m_interfaceStringDescriptor;
  MicrosoftOSStringDescriptor m_microsoftOSStringDescriptor;
  URLDescriptor m_workshopURLDescriptor;

  Descriptor * m_descriptors[8];
  /* m_descriptors contains only descriptors that sould be returned via the
   * method descriptor(uint8_t type, uint8_t index), so do not count descriptors
   * included in other descriptors or returned by other functions :
   *   - m_interfaceDescriptor,
   *   - m_dfuFunctionalDescriptor,
   *   - m_webUSBPlatformDescriptor
   *   - m_workshopURLDescriptor */

  DFUInterface m_dfuInterface;
};

}
}
}

#endif
