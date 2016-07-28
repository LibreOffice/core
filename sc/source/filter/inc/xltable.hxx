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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_XLTABLE_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_XLTABLE_HXX

#include <sal/types.h>

// Constants and Enumerations =================================================

// Specials for outlines ------------------------------------------------------

const sal_uInt8 EXC_OUTLINE_MAX             = 7;
const sal_uInt8 EXC_OUTLINE_COUNT           = EXC_OUTLINE_MAX + 1;

// (0x0000, 0x0200) DIMENSIONS ------------------------------------------------
const sal_uInt16 EXC_ID2_DIMENSIONS         = 0x0000;
const sal_uInt16 EXC_ID3_DIMENSIONS         = 0x0200;

// (0x0001, 0x0201) BLANK -----------------------------------------------------
const sal_uInt16 EXC_ID2_BLANK              = 0x0001;
const sal_uInt16 EXC_ID3_BLANK              = 0x0201;

// (0x0002) INTEGER -----------------------------------------------------------
const sal_uInt16 EXC_ID2_INTEGER            = 0x0002;

// (0x0003, 0x0203) NUMBER ----------------------------------------------------
const sal_uInt16 EXC_ID2_NUMBER             = 0x0003;
const sal_uInt16 EXC_ID3_NUMBER             = 0x0203;

// (0x0004, 0x0204) LABEL -----------------------------------------------------
const sal_uInt16 EXC_ID2_LABEL              = 0x0004;
const sal_uInt16 EXC_ID3_LABEL              = 0x0204;

const sal_uInt8 EXC_LABEL_MAXLEN            = 0xFF;

// (0x0005, 0x0205) BOOLERR ---------------------------------------------------
const sal_uInt16 EXC_ID2_BOOLERR            = 0x0005;
const sal_uInt16 EXC_ID3_BOOLERR            = 0x0205;

const sal_uInt8 EXC_BOOLERR_BOOL            = 0x00;
const sal_uInt8 EXC_BOOLERR_ERROR           = 0x01;

// (0x0006, 0x0206, 0x0406) FORMULA -------------------------------------------
const sal_uInt16 EXC_ID2_FORMULA            = 0x0006;
const sal_uInt16 EXC_ID3_FORMULA            = 0x0206;
const sal_uInt16 EXC_ID4_FORMULA            = 0x0406;

const sal_uInt16 EXC_FORMULA_RECALC_ALWAYS  = 0x0001;
const sal_uInt16 EXC_FORMULA_RECALC_ONLOAD  = 0x0002;
const sal_uInt16 EXC_FORMULA_SHARED         = 0x0008;
const sal_uInt16 EXC_FORMULA_DEFAULTFLAGS   = EXC_FORMULA_RECALC_ONLOAD;

const sal_uInt8 EXC_FORMULA_RES_STRING      = 0x00;     /// Result is a string.
const sal_uInt8 EXC_FORMULA_RES_BOOL        = 0x01;     /// Result is Boolean value.
const sal_uInt8 EXC_FORMULA_RES_ERROR       = 0x02;     /// Result is error code.
const sal_uInt8 EXC_FORMULA_RES_EMPTY       = 0x03;     /// Result is empty cell (BIFF8 only).

// (0x0007, 0x0207) STRING ----------------------------------------------------
const sal_uInt16 EXC_ID2_STRING             = 0x0007;
const sal_uInt16 EXC_ID3_STRING             = 0x0207;

// (0x0008, 0x0208) ROW -------------------------------------------------------
const sal_uInt16 EXC_ID2_ROW                = 0x0008;
const sal_uInt16 EXC_ID3_ROW                = 0x0208;

const sal_uInt16 EXC_ROW_COLLAPSED          = 0x0010;
const sal_uInt16 EXC_ROW_HIDDEN             = 0x0020;
const sal_uInt16 EXC_ROW_UNSYNCED           = 0x0040;
const sal_uInt16 EXC_ROW_USEDEFXF           = 0x0080;
const sal_uInt16 EXC_ROW_DEFAULTFLAGS       = 0x0100;

const sal_uInt16 EXC_ROW_XFMASK             = 0x0FFF;

const sal_uInt16 EXC_ROW_DEFAULTHEIGHT      = 255;
const sal_uInt16 EXC_ROW_FLAGDEFHEIGHT      = 0x8000;
const sal_uInt16 EXC_ROW_HEIGHTMASK         = 0x7FFF;

const sal_uInt16 EXC_ROW_ROWBLOCKSIZE       = 32;       /// Number of rows in a row block.

// (0x0020) COLUMNDEFAULT -----------------------------------------------------
const sal_uInt16 EXC_ID_COLUMNDEFAULT       = 0x0020;

// (0x0021, 0x0221) ARRAY -----------------------------------------------------
const sal_uInt16 EXC_ID2_ARRAY              = 0x0021;
const sal_uInt16 EXC_ID3_ARRAY              = 0x0221;

const sal_uInt16 EXC_ARRAY_RECALC_ALWAYS    = 0x0001;
const sal_uInt16 EXC_ARRAY_RECALC_ONLOAD    = 0x0002;
const sal_uInt16 EXC_ARRAY_DEFAULTFLAGS     = EXC_ARRAY_RECALC_ONLOAD;

// (0x0024) COLWIDTH ----------------------------------------------------------
const sal_uInt16 EXC_ID_COLWIDTH            = 0x0024;

// (0x0025, 0x0225) DEFAULTROWHEIGHT ------------------------------------------
const sal_uInt16 EXC_ID2_DEFROWHEIGHT       = 0x0025;
const sal_uInt16 EXC_ID3_DEFROWHEIGHT       = 0x0225;

const sal_uInt16 EXC_DEFROW_UNSYNCED        = 0x0001;
const sal_uInt16 EXC_DEFROW_HIDDEN          = 0x0002;
const sal_uInt16 EXC_DEFROW_SPACEABOVE      = 0x0004;
const sal_uInt16 EXC_DEFROW_SPACEBELOW      = 0x0008;
const sal_uInt16 EXC_DEFROW_DEFAULTFLAGS    = 0x0000;

const sal_uInt16 EXC_DEFROW_DEFAULTHEIGHT   = 255;

// (0x0036, 0x0236) TABLEOP ---------------------------------------------------
const sal_uInt16 EXC_ID2_TABLEOP            = 0x0036;
const sal_uInt16 EXC_ID3_TABLEOP            = 0x0236;

const sal_uInt16 EXC_TABLEOP_RECALC_ALWAYS  = 0x0001;
const sal_uInt16 EXC_TABLEOP_RECALC_ONLOAD  = 0x0002;
const sal_uInt16 EXC_TABLEOP_ROW            = 0x0004;
const sal_uInt16 EXC_TABLEOP_BOTH           = 0x0008;
const sal_uInt16 EXC_TABLEOP_DEFAULTFLAGS   = EXC_TABLEOP_RECALC_ONLOAD;

// (0x0037) TABLEOP2 ----------------------------------------------------------
const sal_uInt16 EXC_ID2_TABLEOP2           = 0x0037;

// (0x0055) DEFCOLWIDTH -------------------------------------------------------
const sal_uInt16 EXC_ID_DEFCOLWIDTH         = 0x0055;
const sal_uInt16 EXC_DEFCOLWIDTH_DEF        = 10;

// (0x007D) COLINFO -----------------------------------------------------------
const sal_uInt16 EXC_ID_COLINFO             = 0x007D;

const sal_uInt16 EXC_COLINFO_HIDDEN         = 0x0001;
const sal_uInt16 EXC_COLINFO_CUSTOMWIDTH    = 0x0002;
const sal_uInt16 EXC_COLINFO_COLLAPSED      = 0x1000;

// (0x0080) GUTS --------------------------------------------------------------
const sal_uInt16 EXC_ID_GUTS                = 0x0080;

// (0x00BD) MULRK -------------------------------------------------------------
const sal_uInt16 EXC_ID_MULRK               = 0x00BD;

// (0x00BE) MULBLANK ----------------------------------------------------------
const sal_uInt16 EXC_ID_MULBLANK            = 0x00BE;

// (0x00D6) RSTRING -----------------------------------------------------------
const sal_uInt16 EXC_ID_RSTRING             = 0x00D6;

// (0x00FD) LABELSST ----------------------------------------------------------
const sal_uInt16 EXC_ID_LABELSST            = 0x00FD;

// (0x027E) RK ----------------------------------------------------------------
const sal_uInt16 EXC_ID_RK                  = 0x027E;

// (0x04BC) SHRFMLA -----------------------------------------------------------
const sal_uInt16 EXC_ID_SHRFMLA             = 0x04BC;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
