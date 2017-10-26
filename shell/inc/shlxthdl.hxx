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

#ifndef INCLUDED_SHELL_INC_INTERNAL_SHLXTHDL_HXX
#define INCLUDED_SHELL_INC_INTERNAL_SHLXTHDL_HXX

#include <objbase.h>

// {087B3AE3-E237-4467-B8DB-5A38AB959AC9}
const CLSID CLSID_INFOTIP_HANDLER =
{0x87b3ae3, 0xe237, 0x4467, {0xb8, 0xdb, 0x5a, 0x38, 0xab, 0x95, 0x9a, 0xc9}};

// {C52AF81D-F7A0-4aab-8E87-F80A60CCD396}
const CLSID CLSID_COLUMN_HANDLER =
{ 0xc52af81d, 0xf7a0, 0x4aab, { 0x8e, 0x87, 0xf8, 0xa, 0x60, 0xcc, 0xd3, 0x96 } };

// {63542C48-9552-494a-84F7-73AA6A7C99C1}
const CLSID CLSID_PROPERTYSHEET_HANDLER =
{ 0x63542c48, 0x9552, 0x494a, { 0x84, 0xf7, 0x73, 0xaa, 0x6a, 0x7c, 0x99, 0xc1 } };

// {3B092F0C-7696-40e3-A80F-68D74DA84210}
const CLSID CLSID_THUMBVIEWER_HANDLER =
{ 0x3b092f0c, 0x7696, 0x40e3, { 0xa8, 0xf, 0x68, 0xd7, 0x4d, 0xa8, 0x42, 0x10 } };

extern HINSTANCE g_hModule;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
