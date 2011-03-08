/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SHLXTHDL_HXX_INCLUDED
#define SHLXTHDL_HXX_INCLUDED

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <objbase.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
