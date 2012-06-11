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

#ifndef _VCL_VCLENUM_HXX
#define _VCL_VCLENUM_HXX

#include <sal/types.h>
#include <tools/solar.h>
#include <tools/fontenum.hxx>

#ifndef ENUM_TIMEFIELDFORMAT_DECLARED
#define ENUM_TIMEFIELDFORMAT_DECLARED

// By changes you must also change: rsc/vclrsc.hxx
enum TimeFieldFormat {TIMEF_NONE, TIMEF_SEC, TIMEF_100TH_SEC, TIMEF_SEC_CS, TimeFieldFormat_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

#endif

// ------------------------------------------------------------

#ifndef ENUM_EXTTIMEFIELDFORMAT_DECLARED
#define ENUM_EXTTIMEFIELDFORMAT_DECLARED

enum ExtTimeFieldFormat { EXTTIMEF_24H_SHORT, EXTTIMEF_24H_LONG,
                          EXTTIMEF_12H_SHORT, EXTTIMEF_12H_LONG,
                          EXTTIMEF_DURATION_SHORT, EXTTIMEF_DURATION_LONG };

#endif

// ------------------------------------------------------------

#ifndef ENUM_EXTDATEFIELDFORMAT_DECLARED
#define ENUM_EXTDATEFIELDFORMAT_DECLARED

enum ExtDateFieldFormat { XTDATEF_SYSTEM_SHORT, XTDATEF_SYSTEM_SHORT_YY, XTDATEF_SYSTEM_SHORT_YYYY,
                          XTDATEF_SYSTEM_LONG,
                          XTDATEF_SHORT_DDMMYY, XTDATEF_SHORT_MMDDYY, XTDATEF_SHORT_YYMMDD,
                          XTDATEF_SHORT_DDMMYYYY, XTDATEF_SHORT_MMDDYYYY, XTDATEF_SHORT_YYYYMMDD,
                          XTDATEF_SHORT_YYMMDD_DIN5008, XTDATEF_SHORT_YYYYMMDD_DIN5008, ExtDateFieldFormat_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

#endif

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

#ifndef ENUM_HATCHSTYLE_DECLARED
#define ENUM_HATCHSTYLE_DECLARED

enum HatchStyle
{
    HATCH_SINGLE = 0,
    HATCH_DOUBLE = 1,
    HATCH_TRIPLE = 2,
    HatchStyle_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
};

#endif

// ------------------------------------------------------------

// to avoid conflicts with enum's declared otherwise
#define LINE_NONE               LineStyle_NONE
#define LINE_SOLID              LineStyle_SOLID
#define LINE_DASH               LineStyle_DASH
#define LINE_FORCE_EQUAL_SIZE   LineStyle_FORCE_EQUAL_SIZE

#ifndef ENUM_LINESTYLE_DECLARED
#define ENUM_LINESTYLE_DECLARED

enum LineStyle
{
    LINE_NONE = 0,
    LINE_SOLID = 1,
    LINE_DASH = 2,
    LineStyle_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
};

#endif

// ------------------------------------------------------------

#ifndef ENUM_RASTEROP_DECLARED
#define ENUM_RASTEROP_DECLARED

enum RasterOp { ROP_OVERPAINT, ROP_XOR, ROP_0, ROP_1, ROP_INVERT };

#endif

// ------------------------------------------------------------


#ifndef ENUM_FONTAUTOHINT_DECLARED
#define ENUM_FONTAUTOHINT_DECLARED

enum FontAutoHint { AUTOHINT_DONTKNOW, AUTOHINT_FALSE, AUTOHINT_TRUE };

#endif

#ifndef ENUM_FONTHINTING_DECLARED
#define ENUM_FONTHINTING_DECLARED

enum FontHinting { HINTING_DONTKNOW, HINTING_FALSE, HINTING_TRUE };

#endif

#ifndef ENUM_FONTHINTSTYLE_DECLARED
#define ENUM_FONTHINTSTYLE_DECLARED

enum FontHintStyle { HINT_NONE, HINT_SLIGHT, HINT_MEDIUM, HINT_FULL };

#endif

// ------------------------------------------------------------

#ifndef ENUM_KEYFUNCTYPE_DECLARED
#define ENUM_KEYFUNCTYPE_DECLARED

enum KeyFuncType { KEYFUNC_DONTKNOW, KEYFUNC_NEW, KEYFUNC_OPEN, KEYFUNC_SAVE,
                   KEYFUNC_SAVEAS, KEYFUNC_PRINT, KEYFUNC_CLOSE, KEYFUNC_QUIT,
                   KEYFUNC_CUT, KEYFUNC_COPY, KEYFUNC_PASTE, KEYFUNC_UNDO,
                   KEYFUNC_REDO, KEYFUNC_DELETE, KEYFUNC_REPEAT, KEYFUNC_FIND,
                   KEYFUNC_FINDBACKWARD, KEYFUNC_PROPERTIES, KEYFUNC_FRONT,
                   KeyFuncType_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

#endif

typedef sal_uInt32 sal_UCS4;    // TODO: this should be moved to rtl

#ifndef ENUM_OUTDEVSUPPORT_DECLARED
#define ENUM_OUTDEVSUPPORT_DECLARED

enum OutDevSupportType { OutDevSupport_TransparentRect, OutDevSupport_B2DClip, OutDevSupport_B2DDraw };

#endif

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

#endif  // _VCL_VCLENUM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
