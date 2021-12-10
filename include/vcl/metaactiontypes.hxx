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
 *   Licensed to the Apache Software Foundation (ASF), under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_VCL_METAACTIONTYPES_HXX
#define INCLUDED_VCL_METAACTIONTYPES_HXX

enum class MetaActionType
{
    NONE                    = 0,
    PIXEL                   = 100,
    POINT                   = 101,
    LINE                    = 102,
    RECT                    = 103,
    ROUNDRECT               = 104,
    ELLIPSE                 = 105,
    ARC                     = 106,
    PIE                     = 107,
    CHORD                   = 108,
    POLYLINE                = 109,
    POLYGON                 = 110,
    POLYPOLYGON             = 111,
    TEXT                    = 112,
    TEXTARRAY               = 113,
    STRETCHTEXT             = 114,
    TEXTRECT                = 115,
    BMP                     = 116,
    BMPSCALE                = 117,
    BMPSCALEPART            = 118,
    BMPEX                   = 119,
    BMPEXSCALE              = 120,
    BMPEXSCALEPART          = 121,
    MASK                    = 122,
    MASKSCALE               = 123,
    MASKSCALEPART           = 124,
    GRADIENT                = 125,
    HATCH                   = 126,
    WALLPAPER               = 127,
    CLIPREGION              = 128,
    ISECTRECTCLIPREGION     = 129,
    ISECTREGIONCLIPREGION   = 130,
    MOVECLIPREGION          = 131,
    LINECOLOR               = 132,
    FILLCOLOR               = 133,
    TEXTCOLOR               = 134,
    TEXTFILLCOLOR           = 135,
    TEXTALIGN               = 136,
    MAPMODE                 = 137,
    FONT                    = 138,
    PUSH                    = 139,
    POP                     = 140,
    RASTEROP                = 141,
    Transparent             = 142,
    EPS                     = 143,
    REFPOINT                = 144,
    TEXTLINECOLOR           = 145,
    TEXTLINE                = 146,
    FLOATTRANSPARENT        = 147,
    GRADIENTEX              = 148,
    LAYOUTMODE              = 149,
    TEXTLANGUAGE            = 150,
    OVERLINECOLOR           = 151,
    LINEARGRADIENT          = 152,

    COMMENT                 = 512,
    LAST                    = COMMENT
};

#endif // INCLUDED_VCL_METAACTIONTYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
