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

#ifndef INCLUDED_VCL_OUTDEVSTATE_HXX
#define INCLUDED_VCL_OUTDEVSTATE_HXX

#include <sal/types.h>

#include <vcl/mapmod.hxx>
#include <vcl/region.hxx>
#include <vcl/font.hxx>
#include <vcl/vclenum.hxx>

#include <tools/gen.hxx>
#include <tools/color.hxx>
#include <tools/fontenum.hxx>

// Flags for OutputDevice::Push() and OutDevState
#define PUSH_LINECOLOR                  ((sal_uInt16)0x0001)
#define PUSH_FILLCOLOR                  ((sal_uInt16)0x0002)
#define PUSH_FONT                       ((sal_uInt16)0x0004)
#define PUSH_TEXTCOLOR                  ((sal_uInt16)0x0008)
#define PUSH_MAPMODE                    ((sal_uInt16)0x0010)
#define PUSH_CLIPREGION                 ((sal_uInt16)0x0020)
#define PUSH_RASTEROP                   ((sal_uInt16)0x0040)
#define PUSH_TEXTFILLCOLOR              ((sal_uInt16)0x0080)
#define PUSH_TEXTALIGN                  ((sal_uInt16)0x0100)
#define PUSH_REFPOINT                   ((sal_uInt16)0x0200)
#define PUSH_TEXTLINECOLOR              ((sal_uInt16)0x0400)
#define PUSH_TEXTLAYOUTMODE             ((sal_uInt16)0x0800)
#define PUSH_TEXTLANGUAGE               ((sal_uInt16)0x1000)
#define PUSH_OVERLINECOLOR              ((sal_uInt16)0x2000)
#define PUSH_ALLTEXT                    (PUSH_TEXTCOLOR | PUSH_TEXTFILLCOLOR | PUSH_TEXTLINECOLOR | PUSH_OVERLINECOLOR | PUSH_TEXTALIGN | PUSH_TEXTLAYOUTMODE | PUSH_TEXTLANGUAGE)
#define PUSH_ALLFONT                    (PUSH_ALLTEXT | PUSH_FONT)
#define PUSH_ALL                        ((sal_uInt16)0xFFFF)

// LayoutModes for Complex Text Layout
// These are flag values, i.e they can be combined
enum ComplexTextLayoutMode
{
  TEXT_LAYOUT_DEFAULT =             ((sal_uLong)0x00000000),
  TEXT_LAYOUT_BIDI_RTL =            ((sal_uLong)0x00000001),
  TEXT_LAYOUT_BIDI_STRONG =         ((sal_uLong)0x00000002),
  TEXT_LAYOUT_TEXTORIGIN_LEFT =     ((sal_uLong)0x00000004),
  TEXT_LAYOUT_TEXTORIGIN_RIGHT =    ((sal_uLong)0x00000008),
  TEXT_LAYOUT_COMPLEX_DISABLED =    ((sal_uLong)0x00000100),
  TEXT_LAYOUT_ENABLE_LIGATURES =    ((sal_uLong)0x00000200),
  TEXT_LAYOUT_SUBSTITUTE_DIGITS =   ((sal_uLong)0x00000400)
};
// make combining these type-safe
inline ComplexTextLayoutMode operator| (ComplexTextLayoutMode lhs, ComplexTextLayoutMode rhs)
{
    return (ComplexTextLayoutMode)((sal_uLong)lhs | (sal_uLong)rhs);
}
inline ComplexTextLayoutMode operator& (ComplexTextLayoutMode lhs, ComplexTextLayoutMode rhs)
{
    return (ComplexTextLayoutMode)((sal_uLong)lhs & (sal_uLong)rhs);
}
inline ComplexTextLayoutMode operator~ (ComplexTextLayoutMode rhs)
{
    return (ComplexTextLayoutMode)(~(sal_uLong)rhs);
}
inline ComplexTextLayoutMode& operator|= (ComplexTextLayoutMode& lhs, ComplexTextLayoutMode rhs)
{
    lhs = (ComplexTextLayoutMode)((sal_uLong)lhs | (sal_uLong)rhs);
    return lhs;
}
inline ComplexTextLayoutMode& operator&= (ComplexTextLayoutMode& lhs, ComplexTextLayoutMode rhs)
{
    lhs = (ComplexTextLayoutMode)((sal_uLong)lhs & (sal_uLong)rhs);
    return lhs;
}


class OutDevState
{
public:
    ~OutDevState();

    MapMode*        mpMapMode;
    bool            mbMapActive;
    Region*         mpClipRegion;
    Color*          mpLineColor;
    Color*          mpFillColor;
    Font*           mpFont;
    Color*          mpTextColor;
    Color*          mpTextFillColor;
    Color*          mpTextLineColor;
    Color*          mpOverlineColor;
    Point*          mpRefPoint;
    TextAlign       meTextAlign;
    RasterOp        meRasterOp;
    ComplexTextLayoutMode  mnTextLayoutMode;
    LanguageType    meTextLanguage;
    sal_uInt16      mnFlags;
};

#endif // INCLUDED_VCL_OUTDEVSTATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
