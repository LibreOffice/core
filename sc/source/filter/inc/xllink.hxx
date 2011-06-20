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

#ifndef SC_XLLINK_HXX
#define SC_XLLINK_HXX

#include <sal/types.h>

// Constants and Enumerations =================================================

const sal_uInt16 EXC_TAB_EXTERNAL           = 0xFFFE;     /// Special sheet index for external links.
const sal_uInt16 EXC_TAB_DELETED            = 0xFFFF;     /// Deleted sheet in a 3D reference.

// (0x0016) EXTERNCOUNT -------------------------------------------------------

const sal_uInt16 EXC_ID_EXTERNCOUNT         = 0x0016;

// (0x0017) EXTERNSHEET -------------------------------------------------------

const sal_uInt16 EXC_ID_EXTERNSHEET         = 0x0017;

const sal_Unicode EXC_EXTSH_URL             = '\x01';
const sal_Unicode EXC_EXTSH_OWNTAB          = '\x02';
const sal_Unicode EXC_EXTSH_TABNAME         = '\x03';
const sal_Unicode EXC_EXTSH_OWNDOC          = '\x04';
const sal_Unicode EXC_EXTSH_ADDIN           = '\x3A';

// (0x0023) EXTERNNAME --------------------------------------------------------

const sal_uInt16 EXC_ID_EXTERNNAME          = 0x0023;

const sal_uInt16 EXC_EXTN_BUILTIN           = 0x0001;
const sal_uInt16 EXC_EXTN_OLE               = 0x0010;
const sal_uInt16 EXC_EXTN_OLE_OR_DDE        = 0xFFFE;

const sal_uInt16 EXC_EXTN_EXPDDE_STDDOC     = 0x7FEA;  /// for export
const sal_uInt16 EXC_EXTN_EXPDDE            = 0x7FE2;  /// for export

// (0x0059, 0x005A) XCT, CRN --------------------------------------------------

const sal_uInt16 EXC_ID_XCT                 = 0x0059;
const sal_uInt16 EXC_ID_CRN                 = 0x005A;

// (0x013D) TABID -------------------------------------------------------------

const sal_uInt16 EXC_ID_TABID               = 0x013D;

// (0x01AE) SUPBOOK -----------------------------------------------------------

const sal_uInt16 EXC_ID_SUPBOOK             = 0x01AE;

const sal_uInt16 EXC_SUPB_SELF              = 0x0401;
const sal_uInt16 EXC_SUPB_ADDIN             = 0x3A01;

/** This enumeration specifies the type of a SUPBOOK record. */
enum XclSupbookType
{
    EXC_SBTYPE_UNKNOWN,     /// unknown SUPBOOK record type.
    EXC_SBTYPE_SELF,        /// SUPBOOK is used for internal references.
    EXC_SBTYPE_EXTERN,      /// SUPBOOK is used for external references.
    EXC_SBTYPE_ADDIN,       /// SUPBOOK contains add-in functions.
    EXC_SBTYPE_SPECIAL,     /// SUPBOOK is used for DDE or OLE links.
    EXC_SBTYPE_EUROTOOL     /// SUPBOOK is uesd for EUROCONVERT.
};

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
