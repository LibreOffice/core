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

using namespace com::sun::star::uno;


namespace jfw_plugin
{

/* Note: The vendor strings must be UTF-8. For example, if
   the string contains an a umlaut then it must be expressed
   by "\xXX\xXX"
 */
BEGIN_VENDOR_MAP()
    VENDOR_MAP_ENTRY("Sun Microsystems Inc.", SunInfo)
    VENDOR_MAP_ENTRY("Oracle Corporation", SunInfo)
    VENDOR_MAP_ENTRY("IBM Corporation", OtherInfo)
    VENDOR_MAP_ENTRY("Blackdown Java-Linux Team", OtherInfo)
    VENDOR_MAP_ENTRY("Apple Inc.", OtherInfo)
    VENDOR_MAP_ENTRY("Apple Computer, Inc.", OtherInfo)
    VENDOR_MAP_ENTRY("BEA Systems, Inc.", OtherInfo)
    VENDOR_MAP_ENTRY("Free Software Foundation, Inc.", GnuInfo)
    VENDOR_MAP_ENTRY("The FreeBSD Foundation", OtherInfo)
END_VENDOR_MAP()


Sequence<OUString> getVendorNames()
{
    const size_t count = sizeof(gVendorMap) / sizeof (VendorSupportMapEntry) - 1;
    OUString arNames[count];
    for ( size_t pos = 0; pos < count; ++pos )
    {
        OString sVendor(gVendorMap[pos].sVendorName);
        arNames[pos] = OStringToOUString(sVendor, RTL_TEXTENCODING_UTF8);
    }
    return Sequence<OUString>(arNames, count);
}

bool isVendorSupported(const OUString& sVendor)
{
    Sequence<OUString> seqNames = getVendorNames();
    const OUString * arNames = seqNames.getConstArray();
    sal_Int32 count = seqNames.getLength();

    for (int i = 0; i < count; i++)
    {
        if (sVendor.equals(arNames[i]))
            return true;
    }
#if OSL_DEBUG_LEVEL >= 2
    OString sVendorName = OUStringToOString(sVendor, osl_getThreadTextEncoding());
    fprintf(stderr, "[Java frameworksunjavaplugin.so]sunjavaplugin does not support vendor: %s.\n",
            sVendorName.getStr());
#endif
    return false;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
