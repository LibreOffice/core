/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "vendorlist.hxx"
#include "gnujre.hxx"
#include "sunjre.hxx"
#include "otherjre.hxx"
#include "osl/thread.h"
#include <stdio.h>


namespace jfw_plugin
{

/* Note: The vendor strings must be UTF-8. For example, if
   the string contains an a umlaut then it must be expressed
   by "\xXX\xXX"
 */
BEGIN_VENDOR_MAP()
#ifdef MACOSX
    VENDOR_MAP_ENTRY("Apple Inc.", OtherInfo)
    VENDOR_MAP_ENTRY("Apple Computer, Inc.", OtherInfo)
#endif
    VENDOR_MAP_ENTRY("Sun Microsystems Inc.", SunInfo)
    VENDOR_MAP_ENTRY("Oracle Corporation", SunInfo)
#ifndef MACOSX // For OS X, don't bother with implementations that aren't relevant (or have never existed)
    VENDOR_MAP_ENTRY("IBM Corporation", OtherInfo)
    VENDOR_MAP_ENTRY("Blackdown Java-Linux Team", OtherInfo)
    VENDOR_MAP_ENTRY("BEA Systems, Inc.", OtherInfo)
    VENDOR_MAP_ENTRY("Free Software Foundation, Inc.", GnuInfo)
    VENDOR_MAP_ENTRY("The FreeBSD Foundation", OtherInfo)
#endif
    VENDOR_MAP_ENTRY("Azul Systems, Inc.", OtherInfo)
END_VENDOR_MAP()


bool isVendorSupported(const OUString& sVendor)
{
    const size_t count = sizeof(gVendorMap) / sizeof (VendorSupportMapEntry) - 1;
    for ( size_t pos = 0; pos < count; ++pos )
    {
        if (sVendor.equalsAscii(gVendorMap[pos].sVendorName))
            return true;
    }
    SAL_INFO(
        "jfw.level2", "sunjavaplugin does not support vendor: " << sVendor);
    return false;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
