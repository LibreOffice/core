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

static const char* pSbxRes[] = {
    "Empty",
    "Null",
    "Integer",
    "Long",
    "Single",
    "Double",
    "Currency",
    "Date",
    "String",
    "Object",
    "Error",
    "Boolean",
    "Variant",
    "Any",
    "Type14",
    "Type15",
    "Char",
    "Byte",
    "UShort",
    "ULong",
    "Long64",
    "ULong64",
    "Int",
    "UInt",
    "Void",
    "HResult",
    "Pointer",
    "DimArray",
    "CArray",
    "Any",
    "LpStr",
    "LpWStr",
    " As ",
    "Optional ",
    "Byref ",

    "Name",
    "Parent",
    "Application",
    "Count",
    "Add",
    "Item",
    "Remove",

    "Error ",   // with blank!
    "False",
    "True"
};

OUString GetSbxRes( StringId nId )
{
    return OUString::createFromAscii( ( nId > StringId::LastValue ) ? "???" : pSbxRes[ static_cast<int>( nId ) ] );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
