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

#ifndef _XENUM_HXX
#define _XENUM_HXX
namespace binfilter {

enum XLineStyle			{ XLINE_NONE, XLINE_SOLID, XLINE_DASH };
enum XLineJoint			
{
    XLINEJOINT_NONE,		// no rounding
    XLINEJOINT_MIDDLE,		// calc middle value between joints
    XLINEJOINT_BEVEL,		// join edges with line
    XLINEJOINT_MITER,		// extend till cut
    XLINEJOINT_ROUND		// create arc
};
enum XDashStyle			{ XDASH_RECT, XDASH_ROUND, XDASH_RECTRELATIVE,
                          XDASH_ROUNDRELATIVE };
enum XFillStyle			{ XFILL_NONE, XFILL_SOLID, XFILL_GRADIENT, XFILL_HATCH,
                          XFILL_BITMAP };
enum XGradientStyle		{ XGRAD_LINEAR, XGRAD_AXIAL, XGRAD_RADIAL,
                          XGRAD_ELLIPTICAL, XGRAD_SQUARE, XGRAD_RECT };
enum XHatchStyle		{ XHATCH_SINGLE, XHATCH_DOUBLE, XHATCH_TRIPLE };
enum XFormTextStyle		{ XFT_ROTATE, XFT_UPRIGHT, XFT_SLANTX, XFT_SLANTY,
                          XFT_NONE };
enum XFormTextAdjust	{ XFT_LEFT, XFT_RIGHT, XFT_AUTOSIZE, XFT_CENTER };
enum XFormTextShadow	{ XFTSHADOW_NONE, XFTSHADOW_NORMAL, XFTSHADOW_SLANT};
enum XFormTextStdForm	{ XFTFORM_NONE = 0, XFTFORM_TOPCIRC, XFTFORM_BOTCIRC,
                          XFTFORM_LFTCIRC, XFTFORM_RGTCIRC, XFTFORM_TOPARC,
                          XFTFORM_BOTARC, XFTFORM_LFTARC, XFTFORM_RGTARC,
                          XFTFORM_BUTTON1, XFTFORM_BUTTON2,
                          XFTFORM_BUTTON3, XFTFORM_BUTTON4};
enum XBitmapStyle		{ XBITMAP_TILE, XBITMAP_STRETCH };
enum XBitmapType 		{ XBITMAP_IMPORT, XBITMAP_8X8, XBITMAP_NONE,
                          XBITMAP_16X16 };

}//end of namespace binfilter
#endif		// _XENUM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
