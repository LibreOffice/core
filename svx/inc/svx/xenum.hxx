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

#ifndef _XENUM_HXX
#define _XENUM_HXX

enum XLineStyle         { XLINE_NONE, XLINE_SOLID, XLINE_DASH };
enum XLineJoint
{
    XLINEJOINT_NONE,        // no rounding
    XLINEJOINT_MIDDLE,      // calc middle value between joints
    XLINEJOINT_BEVEL,       // join edges with line
    XLINEJOINT_MITER,       // extend till cut
    XLINEJOINT_ROUND        // create arc
};
enum XDashStyle         { XDASH_RECT, XDASH_ROUND, XDASH_RECTRELATIVE,
                          XDASH_ROUNDRELATIVE };
enum XFillStyle         { XFILL_NONE, XFILL_SOLID, XFILL_GRADIENT, XFILL_HATCH,
                          XFILL_BITMAP };
enum XGradientStyle     { XGRAD_LINEAR, XGRAD_AXIAL, XGRAD_RADIAL,
                          XGRAD_ELLIPTICAL, XGRAD_SQUARE, XGRAD_RECT };
enum XHatchStyle        { XHATCH_SINGLE, XHATCH_DOUBLE, XHATCH_TRIPLE };
enum XFormTextStyle     { XFT_ROTATE, XFT_UPRIGHT, XFT_SLANTX, XFT_SLANTY,
                          XFT_NONE };
enum XFormTextAdjust    { XFT_LEFT, XFT_RIGHT, XFT_AUTOSIZE, XFT_CENTER };
enum XFormTextShadow    { XFTSHADOW_NONE, XFTSHADOW_NORMAL, XFTSHADOW_SLANT};
enum XFormTextStdForm   { XFTFORM_NONE = 0, XFTFORM_TOPCIRC, XFTFORM_BOTCIRC,
                          XFTFORM_LFTCIRC, XFTFORM_RGTCIRC, XFTFORM_TOPARC,
                          XFTFORM_BOTARC, XFTFORM_LFTARC, XFTFORM_RGTARC,
                          XFTFORM_BUTTON1, XFTFORM_BUTTON2,
                          XFTFORM_BUTTON3, XFTFORM_BUTTON4};
enum XBitmapType        { XBITMAP_IMPORT, XBITMAP_8X8 };


#endif      // _XENUM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
