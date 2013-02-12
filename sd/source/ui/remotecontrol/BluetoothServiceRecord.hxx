/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef BLUETOOTH_SERVICE_RECORD_HXX
#define BLUETOOTH_SERVICE_RECORD_HXX

// FIXME: look into sharing definitions across OS's (i.e. UUID and port ).
// Look into dynamically determining which ports are available.

// SDP is a Service Description Protocol cf.
//   http://developer.bluetooth.org/TechnologyOverview/Pages/DI.aspx
// This is an XML representation, an alternative would be a
// binary SDP record.

static const char *bluetooth_service_record =
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    "<record>"
      "<attribute id=\"0x0001\">"       // Service class ID list
        "<sequence>"
          "<uuid value=\"0x1101\"/>"
        "</sequence>"
      "</attribute>"
      "<attribute id=\"0x0004\">"       // Protocol Descriptor list
        "<sequence>"
          "<sequence>"
            "<uuid value=\"0x0100\"/>"
          "</sequence>"
          "<sequence>"
            "<uuid value=\"0x0003\"/>"  // enumeration value of RFCOMM protocol
            "<uint8 value=\"0x05\"/>"   // RFCOMM port number
          "</sequence>"
        "</sequence>"
      "</attribute>"
      "<attribute id=\"0x0005\">"       // Browse Group List
        "<sequence>"
          "<uuid value=\"0x1002\"/>"
        "</sequence>"
      "</attribute>"
      "<attribute id=\"0x0006\">"       // Language Base Attribute ID List
        "<sequence>"
          "<uint16 value=\"0x656e\"/>"
          "<uint16 value=\"0x006a\"/>"
          "<uint16 value=\"0x0100\"/>"
        "</sequence>"
      "</attribute>"
      "<attribute id=\"0x0009\">"       // Bluetooth Profile Descriptor List
        "<sequence>"
          "<sequence>"
            "<uuid value=\"0x1101\"/>"
            "<uint16 value=\"0x0100\"/>"
          "</sequence>"
        "</sequence>"
      "</attribute>"
      "<attribute id=\"0x0100\">"
        "<text value=\"Serial Port\"/>"
      "</attribute>"
      "<attribute id=\"0x0101\">"
        "<text value=\"COM Port\"/>"
      "</attribute>"
    "</record>"
  ;

#endif // BLUETOOTH_SERVICE_RECORD_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
