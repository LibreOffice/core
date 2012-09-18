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

#ifndef SC_XLCONST_HXX
#define SC_XLCONST_HXX

#include "address.hxx"

// Common =====================================================================

// BIFF versions --------------------------------------------------------------

/** An enumeration for all Excel file format types (BIFF types). */
enum XclBiff
{
    EXC_BIFF2 = 0,              /// MS Excel 2.1
    EXC_BIFF3,                  /// MS Excel 3.0
    EXC_BIFF4,                  /// MS Excel 4.0
    EXC_BIFF5,                  /// MS Excel 5.0, MS Excel 7.0 (95)
    EXC_BIFF8,                  /// MS Excel 8.0 (97), 9.0 (2000), 10.0 (XP), 11.0 (2003)
    EXC_BIFF_UNKNOWN            /// Unknown BIFF version.
};

/** An enumeration for all Excel output format types. */
enum XclOutput
{
    EXC_OUTPUT_BINARY,          /// MS Excel binary .xls
    EXC_OUTPUT_XML_2007,        /// MS Excel 2007 .xlsx
};

// Excel sheet dimensions -----------------------------------------------------

const SCCOL EXC_MAXCOL2                     = 255;
const SCROW EXC_MAXROW2                     = 16383;
const SCTAB EXC_MAXTAB2                     = 0;

const SCCOL EXC_MAXCOL3                     = EXC_MAXCOL2;
const SCROW EXC_MAXROW3                     = EXC_MAXROW2;
const SCTAB EXC_MAXTAB3                     = EXC_MAXTAB2;

const SCCOL EXC_MAXCOL4                     = EXC_MAXCOL3;
const SCROW EXC_MAXROW4                     = EXC_MAXROW3;
const SCTAB EXC_MAXTAB4                     = 32767;

const SCCOL EXC_MAXCOL5                     = EXC_MAXCOL4;
const SCROW EXC_MAXROW5                     = EXC_MAXROW4;
const SCTAB EXC_MAXTAB5                     = EXC_MAXTAB4;

const SCCOL EXC_MAXCOL8                     = EXC_MAXCOL5;
const SCROW EXC_MAXROW8                     = 65535;
const SCTAB EXC_MAXTAB8                     = EXC_MAXTAB5;

const SCCOL EXC_MAXCOL_XML_2007             = 16383;
const SCROW EXC_MAXROW_XML_2007             = 1048575;
const SCTAB EXC_MAXTAB_XML_2007             = 1023;

const sal_uInt16 EXC_NOTAB                  = SAL_MAX_UINT16;   /// An invalid Excel sheet index, for common use.
const SCTAB SCTAB_INVALID                   = SCTAB_MAX;        /// An invalid Calc sheet index, for common use.
const SCTAB SCTAB_GLOBAL                    = SCTAB_MAX;        /// A Calc sheet index for the workbook globals.

// Storage/stream names -------------------------------------------------------

#define EXC_STORAGE_OLE_LINKED              CREATE_STRING( "LNK" )
#define EXC_STORAGE_OLE_EMBEDDED            CREATE_STRING( "MBD" )
#define EXC_STORAGE_VBA_PROJECT             CREATE_STRING( "_VBA_PROJECT_CUR" )

#define EXC_STREAM_BOOK                     CREATE_STRING( "Book" )
#define EXC_STREAM_WORKBOOK                 CREATE_STRING( "Workbook" )
#define EXC_STREAM_CTLS                     CREATE_STRING( "Ctls" )

// Encoded URLs ---------------------------------------------------------------

const sal_Unicode EXC_URLSTART_ENCODED      = '\x01';   /// Encoded URL.
const sal_Unicode EXC_URLSTART_SELF         = '\x02';   /// Reference to own workbook.
const sal_Unicode EXC_URLSTART_SELFENCODED  = '\x03';   /// Encoded self reference.
const sal_Unicode EXC_URLSTART_OWNDOC       = '\x04';   /// Reference to own workbook (BIFF5/BIFF7).

const sal_Unicode EXC_URL_DOSDRIVE          = '\x01';   /// DOS drive letter or UNC server name.
const sal_Unicode EXC_URL_DRIVEROOT         = '\x02';   /// Root directory of current drive.
const sal_Unicode EXC_URL_SUBDIR            = '\x03';   /// Directory name delimiter.
const sal_Unicode EXC_URL_PARENTDIR         = '\x04';   /// Parent directory.
const sal_Unicode EXC_URL_RAW               = '\x05';   /// Unencoded URL.
const sal_Unicode EXC_URL_SHEETNAME         = '\x09';   /// Sheet name starts here (BIFF4).

const sal_Unicode EXC_DDE_DELIM             = '\x03';   /// DDE application-topic delimiter

// Error codes ----------------------------------------------------------------

const sal_uInt8 EXC_ERR_NULL                = 0x00;
const sal_uInt8 EXC_ERR_DIV0                = 0x07;
const sal_uInt8 EXC_ERR_VALUE               = 0x0F;
const sal_uInt8 EXC_ERR_REF                 = 0x17;
const sal_uInt8 EXC_ERR_NAME                = 0x1D;
const sal_uInt8 EXC_ERR_NUM                 = 0x24;
const sal_uInt8 EXC_ERR_NA                  = 0x2A;

// Cached values list (EXTERNNAME, ptgArray, ...) -----------------------------

const sal_uInt8 EXC_CACHEDVAL_EMPTY         = 0x00;
const sal_uInt8 EXC_CACHEDVAL_DOUBLE        = 0x01;
const sal_uInt8 EXC_CACHEDVAL_STRING        = 0x02;
const sal_uInt8 EXC_CACHEDVAL_BOOL          = 0x04;
const sal_uInt8 EXC_CACHEDVAL_ERROR         = 0x10;

// RK values ------------------------------------------------------------------

const sal_Int32 EXC_RK_100FLAG              = 0x00000001;
const sal_Int32 EXC_RK_INTFLAG              = 0x00000002;
const sal_Int32 EXC_RK_VALUEMASK            = 0xFFFFFFFC;

const sal_Int32 EXC_RK_DBL                  = 0x00000000;
const sal_Int32 EXC_RK_DBL100               = EXC_RK_100FLAG;
const sal_Int32 EXC_RK_INT                  = EXC_RK_INTFLAG;
const sal_Int32 EXC_RK_INT100               = EXC_RK_100FLAG | EXC_RK_INTFLAG;

// Measures -------------------------------------------------------------------

const sal_Int32 EXC_POINTS_PER_INCH         = 72;
const sal_Int32 EXC_TWIPS_PER_INCH          = EXC_POINTS_PER_INCH * 20;

const double EXC_POINTS_PER_HMM             = static_cast< double >( EXC_POINTS_PER_INCH ) / 2540.0;

const sal_uInt8 EXC_ORIENT_NONE             = 0;        /// Text orientation: not rotated.
const sal_uInt8 EXC_ORIENT_STACKED          = 1;        /// Text orientation: vertically stacked.
const sal_uInt8 EXC_ORIENT_90CCW            = 2;        /// Text orientation: 90 deg counterclockwise.
const sal_uInt8 EXC_ORIENT_90CW             = 3;        /// Text orientation: 90 deg clockwise.

const sal_uInt8 EXC_ROT_NONE                = 0;        /// Text rotation: not rotated.
const sal_uInt8 EXC_ROT_90CCW               = 90;       /// Text rotation: 90 deg counterclockwise.
const sal_uInt8 EXC_ROT_90CW                = 180;      /// Text rotation: 90 deg clockwise.
const sal_uInt8 EXC_ROT_STACKED             = 255;      /// Text rotation: vertically stacked.

// Records (ordered by lowest record ID) ======================================

// (0x0009, 0x0209, 0x0409, 0x0809) BOF ---------------------------------------

const sal_uInt16 EXC_ID2_BOF                = 0x0009;
const sal_uInt16 EXC_ID3_BOF                = 0x0209;
const sal_uInt16 EXC_ID4_BOF                = 0x0409;
const sal_uInt16 EXC_ID5_BOF                = 0x0809;

const sal_uInt16 EXC_BOF_BIFF2              = 0x0200;
const sal_uInt16 EXC_BOF_BIFF3              = 0x0300;
const sal_uInt16 EXC_BOF_BIFF4              = 0x0400;
const sal_uInt16 EXC_BOF_BIFF5              = 0x0500;
const sal_uInt16 EXC_BOF_BIFF8              = 0x0600;

const sal_uInt16 EXC_BOF_GLOBALS            = 0x0005;   /// BIFF5-BIFF8 workbook globals.
const sal_uInt16 EXC_BOF_VBMODULE           = 0x0006;   /// BIFF5-BIFF8 Visual BASIC module.
const sal_uInt16 EXC_BOF_SHEET              = 0x0010;   /// Regular worksheet.
const sal_uInt16 EXC_BOF_CHART              = 0x0020;   /// Chart sheet.
const sal_uInt16 EXC_BOF_MACROSHEET         = 0x0040;   /// Macro sheet.
const sal_uInt16 EXC_BOF_WORKSPACE          = 0x0100;   /// Workspace.
const sal_uInt16 EXC_BOF_UNKNOWN            = 0xFFFF;   /// Internal use only.

// (0x000A) EOF ---------------------------------------------------------------
const sal_uInt16 EXC_ID_EOF                 = 0x000A;

// (0x0012) PROTECT -----------------------------------------------------------
const sal_uInt16 EXC_ID_PROTECT             = 0x0012;

// (0x0013) PASSWORD ----------------------------------------------------------
const sal_uInt16 EXC_ID_PASSWORD            = 0x0013;

// (0x0019) WINDOWPROTECT -----------------------------------------------------
const sal_uInt16 EXC_ID_WINDOWPROTECT       = 0x0019;

// (0x0042) CODEPAGE ----------------------------------------------------------
const sal_uInt16 EXC_ID_CODEPAGE            = 0x0042;

// (0x0081) WSBOOL ------------------------------------------------------------
const sal_uInt16 EXC_ID_WSBOOL              = 0x0081;

const sal_uInt16 EXC_WSBOOL_ROWBELOW        = 0x0040;
const sal_uInt16 EXC_WSBOOL_COLBELOW        = 0x0080;
const sal_uInt16 EXC_WSBOOL_FITTOPAGE       = 0x0100;

const sal_uInt16 EXC_WSBOOL_DEFAULTFLAGS    = 0x04C1;

// (0x0086) WRITEPROT ---------------------------------------------------------
const sal_uInt16 EXC_ID_WRITEPROT           = 0x0086;

// (0x008C) COUNTRY -----------------------------------------------------------
const sal_uInt16 EXC_ID_COUNTRY             = 0x008C;

// (0x009B) FILTERMODE --------------------------------------------------------
const sal_uInt16 EXC_ID_FILTERMODE          = 0x009B;

// (0x009C) FNGROUPCOUNT ------------------------------------------------------
const sal_uInt16 EXC_ID_FNGROUPCOUNT        = 0x009C;

// (0x009D) AUTOFILTERINFO ----------------------------------------------------
const sal_uInt16 EXC_ID_AUTOFILTERINFO      = 0x009D;

// (0x009E) AUTOFILTER --------------------------------------------------------
const sal_uInt16 EXC_ID_AUTOFILTER          = 0x009E;

// (0x00BF, 0x00C0, 0x00C1) TOOLBARHDR, TOOLBAREND, MMS -----------------------
const sal_uInt16 EXC_ID_TOOLBARHDR          = 0x00BF;
const sal_uInt16 EXC_ID_TOOLBAREND          = 0x00C0;
const sal_uInt16 EXC_ID_MMS                 = 0x00C1;

// (0x00E1, 0x00E2) INTERFACEHDR, INTERFACEEND --------------------------------
const sal_uInt16 EXC_ID_INTERFACEHDR        = 0x00E1;
const sal_uInt16 EXC_ID_INTERFACEEND        = 0x00E2;

// (0x0160) USESELFS ----------------------------------------------------------
const sal_uInt16 EXC_ID_USESELFS            = 0x0160;

// (0x0161) DSF ---------------------------------------------------------------
const sal_uInt16 EXC_ID_DSF                 = 0x0161;

// (0x01AA,0x01AB) USERSVIEWBEGIN, USERSVIEWEND -------------------------------
const sal_uInt16 EXC_ID_USERSVIEWBEGIN      = 0x01AA;
const sal_uInt16 EXC_ID_USERSVIEWEND        = 0x01AB;

// (0x01BA) CODENAME ----------------------------------------------------------
const sal_uInt16 EXC_ID_CODENAME            = 0x01BA;

// (0x01C0) XL9FILE -----------------------------------------------------------
const sal_uInt16 EXC_ID_XL9FILE             = 0x01C0;

// (0x8xx) Future records -----------------------------------------------------

/** Enumerates different header types of future records. */
enum XclFutureRecType
{
    EXC_FUTUREREC_SIMPLE,           /// Record identifier and empty flags field.
    EXC_FUTUREREC_UNUSEDREF         /// Record identifier, empty flags field, unused range address.
};

const sal_uInt16 EXC_FUTUREREC_EMPTYFLAGS   = 0x0000;
const sal_uInt16 EXC_FUTUREREC_HASREF       = 0x0001;
const sal_uInt16 EXC_FUTUREREC_ALERT        = 0x0002;

// Border import/export

// TODO: These values are approximate; we should probably tweak these values
// further to better match Excel's border thickness.

const sal_uInt16 EXC_BORDER_THICK = 16;
const sal_uInt16 EXC_BORDER_MEDIUM = 12;
const sal_uInt16 EXC_BORDER_THIN = 4;
const sal_uInt16 EXC_BORDER_HAIR = 1;

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
