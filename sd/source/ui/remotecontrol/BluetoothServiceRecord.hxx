/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

// FIXME: look into sharing definitions across OS's (i.e. UUID and port ).
// Look into dynamically determining which ports are available.

// SDP is a Service Description Protocol cf.
//   http://developer.bluetooth.org/TechnologyOverview/Pages/DI.aspx
// This is an XML representation, an alternative would be a
// binary SDP record.

// for numbers see:
// https://www.bluetooth.org/Technical/AssignedNumbers/service_discovery.htm

const char * const bluetooth_service_record =
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    "<record>"
      "<attribute id=\"0x0001\">"       // Service class ID list
        "<sequence>"
          "<uuid value=\"0x1101\"/>"    // an assigned service class meaning: 'serial port'
                                        // we could add our own 'LibreOffice remote' service
                                        // class here too in future ...
        "</sequence>"
      "</attribute>"
      "<attribute id=\"0x0004\">"       // Protocol Descriptor list
        "<sequence>"
          "<sequence>"
            "<uuid value=\"0x0100\"/>"  // L2CAP Protocol descriptor
          "</sequence>"
          "<sequence>"
            "<uuid value=\"0x0003\"/>"  // enumeration value of RFCOMM protocol
            "<uint8 value=\"0x05\"/>"   // RFCOMM port number
          "</sequence>"
        "</sequence>"
      "</attribute>"
      "<attribute id=\"0x0005\">"       // Browse Group List
        "<sequence>"
          "<uuid value=\"0x1002\"/>"    // public browse class
        "</sequence>"
      "</attribute>"
      "<attribute id=\"0x0006\">"       // Language Base Attribute ID List
        "<sequence>"
          "<uint16 value=\"0x656e\"/>"  // code_ISO639
          "<uint16 value=\"0x006a\"/>"  // encoding 0x6a
          "<uint16 value=\"0x0100\"/>"  // base_offset ie. points to below =>
        "</sequence>"
      "</attribute>"
      "<attribute id=\"0x0009\">"       // Bluetooth Profile Descriptor List
        "<sequence>"
          "<sequence>"
            "<uuid value=\"0x1101\"/>"  // 'serial port' UUID as above
            "<uint16 value=\"0x0100\"/>"// version number 1.0 ?
          "</sequence>"
        "</sequence>"
      "</attribute>"
      // Attribute identifiers are pointed to by the Language Base Attribute ID List
      //  id+0 = ServiceName, id+1 = ServiceDescription, id+2=ProviderName
      "<attribute id=\"0x0100\">"
        "<text value=\"LibreOffice Impress Remote Control\"/>"
      "</attribute>"
      "<attribute id=\"0x0102\">"
        "<text value=\"The Document Foundation\"/>"
      "</attribute>"
    "</record>"
  ;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
