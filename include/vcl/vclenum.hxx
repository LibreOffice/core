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

#ifndef INCLUDED_VCL_VCLENUM_HXX
#define INCLUDED_VCL_VCLENUM_HXX

#include <rsc/rsc-vcl-shared-types.hxx>
#include <sal/types.h>
#include <tools/fontenum.hxx>

enum class ExtTimeFieldFormat
{
    Short24H, Long24H
};

enum class ExtDateFieldFormat
{
    SystemShort, SystemShortYY, SystemShortYYYY,
    SystemLong,
    ShortDDMMYY, ShortMMDDYY, ShortYYMMDD,
    ShortDDMMYYYY, ShortMMDDYYYY, ShortYYYYMMDD,
    ShortYYMMDD_DIN5008, ShortYYYYMMDD_DIN5008,
    FORCE_EQUAL_SIZE=SAL_MAX_ENUM
};

// this appears to be a direct copy of css::awt::GradientStyle
enum class GradientStyle
{
    Linear = 0,
    Axial = 1,
    Radial = 2,
    Elliptical = 3,
    Square = 4,
    Rect = 5,
    FORCE_EQUAL_SIZE = SAL_MAX_ENUM
};

enum class HatchStyle
{
    Single = 0,
    Double = 1,
    Triple = 2,
    FORCE_EQUAL_SIZE = SAL_MAX_ENUM
};

enum class LineStyle
{
    NONE = 0,
    Solid = 1,
    Dash = 2,
    FORCE_EQUAL_SIZE = SAL_MAX_ENUM
};

enum class RasterOp { OverPaint, Xor, N0, N1, Invert };

enum class FontAutoHint { DontKnow, No, Yes };

enum class FontHinting { DontKnow, No, Yes };

enum class FontHintStyle { NONE, Slight, Medium, Full };

typedef sal_uInt32 sal_UCS4;    // TODO: this should be moved to rtl

enum class OutDevSupportType { TransparentRect, B2DDraw };

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
    return !(a == b);
}

enum class VclAlign
{
    Fill,
    Start,
    End,
    Center
};

enum class VclPackType
{
    Start = 0,
    End = 1,
    LAST = End
};

// Return Values from Dialog::Execute
//!!! bei Aenderungen /basic/source/runtime/methods.cxx msgbox anpassen

enum VclResponseType
{
    RET_CANCEL  = 0,
    RET_OK      = 1,
    RET_YES     = 2,
    RET_NO      = 3,
    RET_RETRY   = 4,
    RET_IGNORE  = 5,
    RET_CLOSE   = 7,
    RET_HELP    = 10
};

#endif // INCLUDED_VCL_VCLENUM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
