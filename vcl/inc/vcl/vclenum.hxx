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

#ifndef _VCL_VCLENUM_HXX
#define _VCL_VCLENUM_HXX

#include <sal/types.h>
#include <tools/solar.h>
#include <tools/fontenum.hxx>
#include <rsc/rsc-vcl-shared-types.hxx>

// ------------------------------------------------------------

enum ExtTimeFieldFormat { EXTTIMEF_24H_SHORT, EXTTIMEF_24H_LONG,
                          EXTTIMEF_12H_SHORT, EXTTIMEF_12H_LONG,
                          EXTTIMEF_DURATION_SHORT, EXTTIMEF_DURATION_LONG };

// ------------------------------------------------------------

enum ExtDateFieldFormat { XTDATEF_SYSTEM_SHORT, XTDATEF_SYSTEM_SHORT_YY, XTDATEF_SYSTEM_SHORT_YYYY,
                          XTDATEF_SYSTEM_LONG,
                          XTDATEF_SHORT_DDMMYY, XTDATEF_SHORT_MMDDYY, XTDATEF_SHORT_YYMMDD,
                          XTDATEF_SHORT_DDMMYYYY, XTDATEF_SHORT_MMDDYYYY, XTDATEF_SHORT_YYYYMMDD,
                          XTDATEF_SHORT_YYMMDD_DIN5008, XTDATEF_SHORT_YYYYMMDD_DIN5008, ExtDateFieldFormat_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

// ------------------------------------------------------------

enum GradientStyle
{
    GradientStyle_LINEAR = 0,
    GradientStyle_AXIAL = 1,
    GradientStyle_RADIAL = 2,
    GradientStyle_ELLIPTICAL = 3,
    GradientStyle_SQUARE = 4,
    GradientStyle_RECT = 5,
    GradientStyle_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
};

// ------------------------------------------------------------

// to avoid conflicts with enum's declared otherwise
#define HATCH_SINGLE            HatchStyle_SINGLE
#define HATCH_DOUBLE            HatchStyle_DOUBLE
#define HATCH_TRIPLE            HatchStyle_TRIPLE
#define HATCH_FORCE_EQUAL_SIZE  HatchStyle_FORCE_EQUAL_SIZE

enum HatchStyle
{
    HATCH_SINGLE = 0,
    HATCH_DOUBLE = 1,
    HATCH_TRIPLE = 2,
    HatchStyle_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
};

// ------------------------------------------------------------

// to avoid conflicts with enum's declared otherwise
#define LINE_NONE               LineStyle_NONE
#define LINE_SOLID              LineStyle_SOLID
#define LINE_DASH               LineStyle_DASH
#define LINE_FORCE_EQUAL_SIZE   LineStyle_FORCE_EQUAL_SIZE

enum LineStyle
{
    LINE_NONE = 0,
    LINE_SOLID = 1,
    LINE_DASH = 2,
    LineStyle_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
};

// ------------------------------------------------------------

enum RasterOp { ROP_OVERPAINT, ROP_XOR, ROP_0, ROP_1, ROP_INVERT };

// ------------------------------------------------------------

enum FontAutoHint { AUTOHINT_DONTKNOW, AUTOHINT_FALSE, AUTOHINT_TRUE };

enum FontHinting { HINTING_DONTKNOW, HINTING_FALSE, HINTING_TRUE };

enum FontHintStyle { HINT_NONE, HINT_SLIGHT, HINT_MEDIUM, HINT_FULL };

// ------------------------------------------------------------

typedef sal_uInt32 sal_UCS4;    // TODO: this should be moved to rtl

enum OutDevSupportType { OutDevSupport_TransparentRect, OutDevSupport_B2DClip, OutDevSupport_B2DDraw };

struct ItalicMatrix
{
    double xx, xy, yx, yy;
    ItalicMatrix() : xx(1), xy(0), yx(0), yy(1) {}
};

inline bool operator ==(const ItalicMatrix& a, const ItalicMatrix& b)
{
    return a.xx == b.xx && a.xy == b.xy && a.yx == b.yx && a.yy == b.yy;
}

inline bool operator !=(const ItalicMatrix& a, const ItalicMatrix& b)
{
    return a.xx != b.xx || a.xy != b.xy || a.yx != b.yx || a.yy != b.yy;
}

enum VclAlign
{
    VCL_ALIGN_FILL,
    VCL_ALIGN_START,
    VCL_ALIGN_END,
    VCL_ALIGN_CENTER
};

enum VclPackType
{
    VCL_PACK_START = 0,
    VCL_PACK_END = 1
};

#endif  // _VCL_VCLENUM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
