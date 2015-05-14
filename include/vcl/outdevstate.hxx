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

#include <vcl/mapmod.hxx>
#include <vcl/region.hxx>
#include <vcl/font.hxx>
#include <vcl/vclenum.hxx>

#include <tools/solar.h>
#include <tools/gen.hxx>
#include <tools/color.hxx>
#include <tools/fontenum.hxx>
#include <o3tl/typed_flags_set.hxx>

// Flags for OutputDevice::Push() and OutDevState
enum class PushFlags {
    NONE            = 0x0000,
    LINECOLOR       = 0x0001,
    FILLCOLOR       = 0x0002,
    FONT            = 0x0004,
    TEXTCOLOR       = 0x0008,
    MAPMODE         = 0x0010,
    CLIPREGION      = 0x0020,
    RASTEROP        = 0x0040,
    TEXTFILLCOLOR   = 0x0080,
    TEXTALIGN       = 0x0100,
    REFPOINT        = 0x0200,
    TEXTLINECOLOR   = 0x0400,
    TEXTLAYOUTMODE  = 0x0800,
    TEXTLANGUAGE    = 0x1000,
    OVERLINECOLOR   = 0x2000,
    ALL             = 0xFFFF
};

namespace o3tl
{
    template<> struct typed_flags<PushFlags> : is_typed_flags<PushFlags, 0xFFFF> {};
}
#define PUSH_ALLTEXT  (PushFlags::TEXTCOLOR | PushFlags::TEXTFILLCOLOR | PushFlags::TEXTLINECOLOR | PushFlags::OVERLINECOLOR | PushFlags::TEXTALIGN | PushFlags::TEXTLAYOUTMODE | PushFlags::TEXTLANGUAGE)
#define PUSH_ALLFONT  (PUSH_ALLTEXT | PushFlags::FONT)

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
    return static_cast<ComplexTextLayoutMode>(static_cast<sal_uLong>(lhs) | static_cast<sal_uLong>(rhs));
}
inline ComplexTextLayoutMode operator& (ComplexTextLayoutMode lhs, ComplexTextLayoutMode rhs)
{
    return static_cast<ComplexTextLayoutMode>(static_cast<sal_uLong>(lhs) & static_cast<sal_uLong>(rhs));
}
inline ComplexTextLayoutMode operator~ (ComplexTextLayoutMode rhs)
{
    return static_cast<ComplexTextLayoutMode>(0x7ff & ~(static_cast<sal_uLong>(rhs)));
}
inline ComplexTextLayoutMode& operator|= (ComplexTextLayoutMode& lhs, ComplexTextLayoutMode rhs)
{
    lhs = static_cast<ComplexTextLayoutMode>(static_cast<sal_uLong>(lhs) | static_cast<sal_uLong>(rhs));
    return lhs;
}
inline ComplexTextLayoutMode& operator&= (ComplexTextLayoutMode& lhs, ComplexTextLayoutMode rhs)
{
    lhs = static_cast<ComplexTextLayoutMode>(static_cast<sal_uLong>(lhs) & static_cast<sal_uLong>(rhs));
    return lhs;
}

class OutDevState
{
public:
    ~OutDevState();

    MapMode*        mpMapMode;
    bool            mbMapActive;
    vcl::Region*    mpClipRegion;
    Color*          mpLineColor;
    Color*          mpFillColor;
    vcl::Font*      mpFont;
    Color*          mpTextColor;
    Color*          mpTextFillColor;
    Color*          mpTextLineColor;
    Color*          mpOverlineColor;
    Point*          mpRefPoint;
    TextAlign       meTextAlign;
    RasterOp        meRasterOp;
    ComplexTextLayoutMode  mnTextLayoutMode;
    LanguageType    meTextLanguage;
    PushFlags       mnFlags;
};

#endif // INCLUDED_VCL_OUTDEVSTATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
