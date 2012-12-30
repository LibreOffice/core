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

#ifndef _SBXRES_HXX
#define _SBXRES_HXX

#include <rtl/ustring.hxx>

// Currently there are no resources provided in the SVTOOLS-Project.
// Because it is non-critical resources (BASIC-Keywords),
// we can work with dummies.

#define STRING_TYPES        0
#define STRING_EMPTY        0
#define STRING_NULL         1
#define STRING_INTEGER      2
#define STRING_LONG         3
#define STRING_SINGLE       4
#define STRING_DOUBLE       5
#define STRING_CURRENCY     6
#define STRING_DATE         7
#define STRING_STRING       8
#define STRING_OBJECT       9
#define STRING_ERROR        10
#define STRING_BOOL         11
#define STRING_VARIANT      12
#define STRING_ANY          13
#define STRING_CHAR         16
#define STRING_BYTE         17
#define STRING_USHORT       18
#define STRING_ULONG        19
#define STRING_INT          22
#define STRING_UINT         23
#define STRING_LPSTR        30
#define STRING_LPWSTR       31
#define STRING_AS           32
#define STRING_OPTIONAL     33
#define STRING_BYREF        34

#define STRING_NAMEPROP     35
#define STRING_PARENTPROP   36
#define STRING_APPLPROP     37
#define STRING_COUNTPROP    38
#define STRING_ADDMETH      39
#define STRING_ITEMMETH     40
#define STRING_REMOVEMETH   41

#define STRING_ERRORMSG     42
#define STRING_FALSE        43
#define STRING_TRUE         44

#define SBXRES_MAX          44

class SbxRes : public OUString
{
public:
    SbxRes( sal_uInt16 );
};

const char* GetSbxRes( sal_uInt16 );


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
