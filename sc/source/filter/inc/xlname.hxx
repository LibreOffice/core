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

#ifndef SC_XLNAME_HXX
#define SC_XLNAME_HXX

#include <sal/types.h>

// Constants and Enumerations =================================================

// (0x0018, 0x0218) NAME ------------------------------------------------------

const sal_uInt16 EXC_ID_NAME                = 0x0018;
const sal_uInt16 EXC_ID34_NAME              = 0x0218;

// flags
const sal_uInt16 EXC_NAME_DEFAULT           = 0x0000;
const sal_uInt16 EXC_NAME_HIDDEN            = 0x0001;
const sal_uInt16 EXC_NAME_FUNC              = 0x0002;
const sal_uInt16 EXC_NAME_VB                = 0x0004;
const sal_uInt16 EXC_NAME_PROC              = 0x0008;
const sal_uInt16 EXC_NAME_CALCEXP           = 0x0010;
const sal_uInt16 EXC_NAME_BUILTIN           = 0x0020;
const sal_uInt16 EXC_NAME_FGROUPMASK        = 0x0FC0;
const sal_uInt16 EXC_NAME_BIG               = 0x1000;

const sal_uInt8 EXC_NAME2_FUNC              = 0x02;     /// BIFF2 function/command flag.

const sal_uInt16 EXC_NAME_GLOBAL            = 0;        /// 0 = Globally defined name.

// codes for built-in names
const sal_Unicode EXC_BUILTIN_CONSOLIDATEAREA   = '\x00';
const sal_Unicode EXC_BUILTIN_AUTOOPEN          = '\x01';
const sal_Unicode EXC_BUILTIN_AUTOCLOSE         = '\x02';
const sal_Unicode EXC_BUILTIN_EXTRACT           = '\x03';
const sal_Unicode EXC_BUILTIN_DATABASE          = '\x04';
const sal_Unicode EXC_BUILTIN_CRITERIA          = '\x05';
const sal_Unicode EXC_BUILTIN_PRINTAREA         = '\x06';
const sal_Unicode EXC_BUILTIN_PRINTTITLES       = '\x07';
const sal_Unicode EXC_BUILTIN_RECORDER          = '\x08';
const sal_Unicode EXC_BUILTIN_DATAFORM          = '\x09';
const sal_Unicode EXC_BUILTIN_AUTOACTIVATE      = '\x0A';
const sal_Unicode EXC_BUILTIN_AUTODEACTIVATE    = '\x0B';
const sal_Unicode EXC_BUILTIN_SHEETTITLE        = '\x0C';
const sal_Unicode EXC_BUILTIN_FILTERDATABASE    = '\x0D';
const sal_Unicode EXC_BUILTIN_UNKNOWN           = '\x0E';

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
