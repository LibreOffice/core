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


#include "sbxres.hxx"

#include <rtl/ustring.hxx>

constexpr OUString pSbxRes[] = {
    u"Empty"_ustr,
    u"Null"_ustr,
    u"Integer"_ustr,
    u"Long"_ustr,
    u"Single"_ustr,
    u"Double"_ustr,
    u"Currency"_ustr,
    u"Date"_ustr,
    u"String"_ustr,
    u"Object"_ustr,
    u"Error"_ustr,
    u"Boolean"_ustr,
    u"Variant"_ustr,
    u"Any"_ustr,
    u"Type14"_ustr,
    u"Type15"_ustr,
    u"Char"_ustr,
    u"Byte"_ustr,
    u"UShort"_ustr,
    u"ULong"_ustr,
    u"Long64"_ustr,
    u"ULong64"_ustr,
    u"Int"_ustr,
    u"UInt"_ustr,
    u"Void"_ustr,
    u"HResult"_ustr,
    u"Pointer"_ustr,
    u"DimArray"_ustr,
    u"CArray"_ustr,
    u"Any"_ustr,
    u"LpStr"_ustr,
    u"LpWStr"_ustr,
    u" As "_ustr,
    u"Optional "_ustr,
    u"Byref "_ustr,

    u"Name"_ustr,
    u"Parent"_ustr,
    u"Application"_ustr,
    u"Count"_ustr,
    u"Add"_ustr,
    u"Item"_ustr,
    u"Remove"_ustr,

    u"Error "_ustr,   // with blank!
    u"False"_ustr,
    u"True"_ustr
};

OUString GetSbxRes( StringId nId )
{
    return  ( nId > StringId::LastValue ) ? u"???"_ustr : pSbxRes[ static_cast<int>( nId ) ];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
