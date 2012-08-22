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

#ifndef SC_XLCONTENT_HXX
#define SC_XLCONTENT_HXX

#include <sal/types.h>

// Constants ==================================================================

// (0x005B) FILESHARING -------------------------------------------------------

const sal_uInt16 EXC_ID_FILESHARING         = 0x005B;

// (0x00E5) MERGEDCELLS -------------------------------------------------------

const sal_uInt16 EXC_ID_MERGEDCELLS         = 0x00E5;
const sal_uInt16 EXC_MERGEDCELLS_MAXCOUNT   = 1027;

// (0x002F) FILEPASS ----------------------------------------------------------

const sal_uInt16 EXC_ID_FILEPASS            = 0x002F;

const sal_uInt16 EXC_FILEPASS_BIFF5         = 0x0000;
const sal_uInt16 EXC_FILEPASS_BIFF8         = 0x0001;
const sal_uInt16 EXC_FILEPASS_BIFF8_STD     = 0x0001;
const sal_uInt16 EXC_FILEPASS_BIFF8_STRONG  = 0x0002;

// (0x00FC, 0x00FF) SST, EXTSST -----------------------------------------------

const sal_uInt16 EXC_ID_SST                 = 0x00FC;
const sal_uInt16 EXC_ID_EXTSST              = 0x00FF;

// (0x015F) LABELRANGES -------------------------------------------------------

const sal_uInt16 EXC_ID_LABELRANGES         = 0x015F;

// (0x01B0) CONDFMT, (0x01B1) CF ----------------------------------------------

const sal_uInt16 EXC_ID_CONDFMT             = 0x01B0;
const sal_uInt16 EXC_ID_CF                  = 0x01B1;

const sal_uInt8 EXC_CF_TYPE_NONE            = 0x00;
const sal_uInt8 EXC_CF_TYPE_CELL            = 0x01;
const sal_uInt8 EXC_CF_TYPE_FMLA            = 0x02;

const sal_uInt8 EXC_CF_CMP_NONE             = 0x00;
const sal_uInt8 EXC_CF_CMP_BETWEEN          = 0x01;
const sal_uInt8 EXC_CF_CMP_NOT_BETWEEN      = 0x02;
const sal_uInt8 EXC_CF_CMP_EQUAL            = 0x03;
const sal_uInt8 EXC_CF_CMP_NOT_EQUAL        = 0x04;
const sal_uInt8 EXC_CF_CMP_GREATER          = 0x05;
const sal_uInt8 EXC_CF_CMP_LESS             = 0x06;
const sal_uInt8 EXC_CF_CMP_GREATER_EQUAL    = 0x07;
const sal_uInt8 EXC_CF_CMP_LESS_EQUAL       = 0x08;

const sal_uInt32 EXC_CF_BORDER_LEFT         = 0x00000400;   /// Left border line modified?
const sal_uInt32 EXC_CF_BORDER_RIGHT        = 0x00000800;   /// Right border line modified?
const sal_uInt32 EXC_CF_BORDER_TOP          = 0x00001000;   /// Top border line modified?
const sal_uInt32 EXC_CF_BORDER_BOTTOM       = 0x00002000;   /// Bottom border line modified?
const sal_uInt32 EXC_CF_BORDER_ALL          = 0x00003C00;   /// Any border line modified?
const sal_uInt32 EXC_CF_AREA_PATTERN        = 0x00010000;   /// Pattern modified?
const sal_uInt32 EXC_CF_AREA_FGCOLOR        = 0x00020000;   /// Foreground color modified?
const sal_uInt32 EXC_CF_AREA_BGCOLOR        = 0x00040000;   /// Background color modified?
const sal_uInt32 EXC_CF_AREA_ALL            = 0x00070000;   /// Any area attribute modified?
const sal_uInt32 EXC_CF_ALLDEFAULT          = 0x003FFFFF;   /// Default flags.
const sal_uInt32 EXC_CF_BLOCK_FONT          = 0x04000000;   /// Font block present?
const sal_uInt32 EXC_CF_BLOCK_BORDER        = 0x10000000;   /// Border block present?
const sal_uInt32 EXC_CF_BLOCK_AREA          = 0x20000000;   /// Pattern block present?

const sal_uInt32 EXC_CF_FONT_STYLE          = 0x00000002;   /// Font posture or weight modified?
const sal_uInt32 EXC_CF_FONT_STRIKEOUT      = 0x00000080;   /// Font cancellation modified?
const sal_uInt32 EXC_CF_FONT_ALLDEFAULT     = 0x0000009A;   /// Default flags.

const sal_uInt32 EXC_CF_FONT_UNDERL         = 0x00000001;   /// Font underline type modified?
const sal_uInt32 EXC_CF_FONT_ESCAPEM        = 0x00000001;   /// Font escapement type modified?

// (0x01B2) DVAL --------------------------------------------------------------

const sal_uInt16 EXC_ID_DVAL                = 0x01B2;
const sal_uInt32 EXC_DVAL_NOOBJ             = 0xFFFFFFFF;

// (0x01BE) DV ----------------------------------------------------------------

const sal_uInt16 EXC_ID_DV                  = 0x01BE;

// data validation flags
const sal_uInt32 EXC_DV_STRINGLIST          = 0x00000080;
const sal_uInt32 EXC_DV_IGNOREBLANK         = 0x00000100;
const sal_uInt32 EXC_DV_SUPPRESSDROPDOWN    = 0x00000200;
const sal_uInt32 EXC_DV_SHOWPROMPT          = 0x00040000;
const sal_uInt32 EXC_DV_SHOWERROR           = 0x00080000;

// data validation data mode
const sal_uInt32 EXC_DV_MODE_MASK           = 0x0000000F;
const sal_uInt32 EXC_DV_MODE_ANY            = 0x00000000;
const sal_uInt32 EXC_DV_MODE_WHOLE          = 0x00000001;
const sal_uInt32 EXC_DV_MODE_DECIMAL        = 0x00000002;
const sal_uInt32 EXC_DV_MODE_LIST           = 0x00000003;
const sal_uInt32 EXC_DV_MODE_DATE           = 0x00000004;
const sal_uInt32 EXC_DV_MODE_TIME           = 0x00000005;
const sal_uInt32 EXC_DV_MODE_TEXTLEN        = 0x00000006;
const sal_uInt32 EXC_DV_MODE_CUSTOM         = 0x00000007;

// data validation conditions
const sal_uInt32 EXC_DV_COND_MASK           = 0x00F00000;
const sal_uInt32 EXC_DV_COND_BETWEEN        = 0x00000000;
const sal_uInt32 EXC_DV_COND_NOTBETWEEN     = 0x00100000;
const sal_uInt32 EXC_DV_COND_EQUAL          = 0x00200000;
const sal_uInt32 EXC_DV_COND_NOTEQUAL       = 0x00300000;
const sal_uInt32 EXC_DV_COND_GREATER        = 0x00400000;
const sal_uInt32 EXC_DV_COND_LESS           = 0x00500000;
const sal_uInt32 EXC_DV_COND_EQGREATER      = 0x00600000;
const sal_uInt32 EXC_DV_COND_EQLESS         = 0x00700000;

// data validation error style
const sal_uInt32 EXC_DV_ERROR_MASK          = 0x00000070;
const sal_uInt32 EXC_DV_ERROR_STOP          = 0x00000000;
const sal_uInt32 EXC_DV_ERROR_WARNING       = 0x00000010;
const sal_uInt32 EXC_DV_ERROR_INFO          = 0x00000020;

// (0x01B8) HLINK -------------------------------------------------------------

const sal_uInt16 EXC_ID_HLINK               = 0x01B8;

const sal_uInt32 EXC_HLINK_BODY             = 0x00000001;   /// Contains file link or URL.
const sal_uInt32 EXC_HLINK_ABS              = 0x00000002;   /// Absolute path.
const sal_uInt32 EXC_HLINK_DESCR            = 0x00000014;   /// Description.
const sal_uInt32 EXC_HLINK_MARK             = 0x00000008;   /// Text mark.
const sal_uInt32 EXC_HLINK_FRAME            = 0x00000080;   /// Target frame.
const sal_uInt32 EXC_HLINK_UNC              = 0x00000100;   /// UNC path.

// web queries ================================================================

#define EXC_WEBQRY_FILTER                   "calc_HTML_WebQuery"

// (0x00CD) WQSTRING
const sal_uInt16 EXC_ID_WQSTRING            = 0x00CD;

// (0x00DC) PARAMQRY
const sal_uInt16 EXC_ID_PQRY                = 0x00DC;
const sal_uInt16 EXC_PQRYTYPE_ODBC          = 1;            /// Source type: ODBC.
const sal_uInt16 EXC_PQRYTYPE_WEBQUERY      = 4;            /// Source type: webquery.
const sal_uInt16 EXC_PQRY_ODBC              = 0x0008;       /// ODBC connection.
const sal_uInt16 EXC_PQRY_WEBQUERY          = 0x0040;       /// Web query.
const sal_uInt16 EXC_PQRY_TABLES            = 0x0100;       /// All tables.

// (0x01AD) QSI
const sal_uInt16 EXC_ID_QSI                 = 0x01AD;
const sal_uInt16 EXC_QSI_DEFAULTFLAGS       = 0x0349;       /// Flags for export.

// (0x0802) unknown record
const sal_uInt16 EXC_ID_0802                = 0x0802;

// (0x0803) WEBQRYSETTINGS
const sal_uInt16 EXC_ID_WQSETT              = 0x0803;
const sal_uInt16 EXC_WQSETT_ALL             = 0x0000;       /// All tables or entire document.
const sal_uInt16 EXC_WQSETT_SPECTABLES      = 0x0002;       /// Specific tables.
const sal_uInt16 EXC_WQSETT_DEFAULTFLAGS    = 0x0023;       /// Flags for export.
const sal_uInt16 EXC_WQSETT_NOFORMAT        = 0x0001;
const sal_uInt16 EXC_WQSETT_FORMATRTF       = 0x0002;
const sal_uInt16 EXC_WQSETT_FORMATFULL      = 0x0003;

// (0x0804) WEBQRYTABLES
const sal_uInt16 EXC_ID_WQTABLES            = 0x0804;

const sal_uInt16 EXC_ID_EXTLST              = 0x9988;       /// it is just a random number
// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
