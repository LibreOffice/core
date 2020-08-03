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

#pragma once

#include <ostream>

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
enum class XclSupbookType
{
    Unknown,     /// unknown SUPBOOK record type.
    Self,        /// SUPBOOK is used for internal references.
    Extern,      /// SUPBOOK is used for external references.
    Addin,       /// SUPBOOK contains add-in functions.
    Special,     /// SUPBOOK is used for DDE or OLE links.
    Eurotool     /// SUPBOOK is used for EUROCONVERT.
};

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const XclSupbookType& type )
{
    switch (type)
    {
    case XclSupbookType::Unknown: return stream << "unknown";
    case XclSupbookType::Self: return stream << "self";
    case XclSupbookType::Extern: return stream << "extern";
    case XclSupbookType::Addin: return stream << "addin";
    case XclSupbookType::Special: return stream << "special";
    case XclSupbookType::Eurotool: return stream << "eurotool";
    default: return stream << static_cast<int>(type);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
