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

namespace jfw_plugin
{

/* Note: The vendor strings must be UTF-8. For example, if
   the string contains an umlaut then it must be expressed
   by "\xXX\xXX"
 */
VendorSupportMapEntry const gVendorMap[] ={
// For macOS, don't bother with implementations that aren't relevant (or have never existed)
#ifdef MACOSX
    VENDOR_MAP_ENTRY<OtherInfo>("Apple Inc."),
    VENDOR_MAP_ENTRY<OtherInfo>("Apple Computer, Inc."),
#endif
    VENDOR_MAP_ENTRY<SunInfo>("Sun Microsystems Inc."),
    VENDOR_MAP_ENTRY<SunInfo>("Oracle Corporation"),
    VENDOR_MAP_ENTRY<SunInfo>("AdoptOpenJdk"),
    VENDOR_MAP_ENTRY<SunInfo>("Amazon.com Inc."),
    VENDOR_MAP_ENTRY<SunInfo>("Azul Systems, Inc."),
#ifndef MACOSX
    VENDOR_MAP_ENTRY<OtherInfo>("IBM Corporation"),
    VENDOR_MAP_ENTRY<OtherInfo>("Blackdown Java-Linux Team"),
    VENDOR_MAP_ENTRY<OtherInfo>("BEA Systems, Inc."),
    VENDOR_MAP_ENTRY<GnuInfo>("Free Software Foundation, Inc."),
    VENDOR_MAP_ENTRY<OtherInfo>("The FreeBSD Foundation"),
#endif
    {nullptr, nullptr, nullptr} };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
