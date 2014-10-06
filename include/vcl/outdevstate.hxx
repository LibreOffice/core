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
#include <o3tl/typed_flags.hxx>

// Flags for OutputDevice::Push() and OutDevState
typedef o3tl::typed_flags<sal_uInt16, 0xffff> PushFlags;
const PushFlags PUSH_NONE            (0x0000);
const PushFlags PUSH_LINECOLOR       (0x0001);
const PushFlags PUSH_FILLCOLOR       (0x0002);
const PushFlags PUSH_FONT            (0x0004);
const PushFlags PUSH_TEXTCOLOR       (0x0008);
const PushFlags PUSH_MAPMODE         (0x0010);
const PushFlags PUSH_CLIPREGION      (0x0020);
const PushFlags PUSH_RASTEROP        (0x0040);
const PushFlags PUSH_TEXTFILLCOLOR   (0x0080);
const PushFlags PUSH_TEXTALIGN       (0x0100);
const PushFlags PUSH_REFPOINT        (0x0200);
const PushFlags PUSH_TEXTLINECOLOR   (0x0400);
const PushFlags PUSH_TEXTLAYOUTMODE  (0x0800);
const PushFlags PUSH_TEXTLANGUAGE    (0x1000);
const PushFlags PUSH_OVERLINECOLOR   (0x2000);
const PushFlags PUSH_ALL             (0xFFFF);
const PushFlags PUSH_ALLTEXT         (PUSH_TEXTCOLOR | PUSH_TEXTFILLCOLOR | PUSH_TEXTLINECOLOR | PUSH_OVERLINECOLOR | PUSH_TEXTALIGN | PUSH_TEXTLAYOUTMODE | PUSH_TEXTLANGUAGE);
const PushFlags PUSH_ALLFONT         (PUSH_ALLTEXT | PUSH_FONT);



// LayoutModes for Complex Text Layout
// These are flag values, i.e they can be combined
typedef o3tl::typed_flags<sal_uLong, 0x7ff> ComplexTextLayoutMode;
const ComplexTextLayoutMode TEXT_LAYOUT_DEFAULT           (0x00000000);
const ComplexTextLayoutMode TEXT_LAYOUT_BIDI_RTL          (0x00000001);
const ComplexTextLayoutMode TEXT_LAYOUT_BIDI_STRONG       (0x00000002);
const ComplexTextLayoutMode TEXT_LAYOUT_TEXTORIGIN_LEFT   (0x00000004);
const ComplexTextLayoutMode TEXT_LAYOUT_TEXTORIGIN_RIGHT  (0x00000008);
const ComplexTextLayoutMode TEXT_LAYOUT_COMPLEX_DISABLED  (0x00000100);
const ComplexTextLayoutMode TEXT_LAYOUT_ENABLE_LIGATURES  (0x00000200);
const ComplexTextLayoutMode TEXT_LAYOUT_SUBSTITUTE_DIGITS (0x00000400);



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
