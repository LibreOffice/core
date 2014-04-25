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

#ifndef INCLUDED_VCL_INC_OUTDEVSTATE_HXX
#define INCLUDED_VCL_INC_OUTDEVSTATE_HXX

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
    sal_uLong       mnTextLayoutMode;
    LanguageType    meTextLanguage;
    sal_uInt16      mnFlags;
};

#endif // INCLUDED_VCL_INC_OUTDEVSTATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
