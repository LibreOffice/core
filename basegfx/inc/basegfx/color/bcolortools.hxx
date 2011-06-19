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

#ifndef _BGFX_COLOR_BCOLORTOOLS_HXX
#define _BGFX_COLOR_BCOLORTOOLS_HXX

#include <sal/types.h>
#include <basegfx/basegfxdllapi.h>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    class BColor;

    namespace tools
    {
        /// Transform from RGB to HSL
        BASEGFX_DLLPUBLIC BColor rgb2hsl(const BColor& rRGBColor);
        /// Transform from HSL to RGB
        BASEGFX_DLLPUBLIC BColor hsl2rgb(const BColor& rHSLColor);

        /// Transform from RGB to HSV
        BASEGFX_DLLPUBLIC BColor rgb2hsv(const BColor& rRGBColor);
        /// Transform from HSV to RGB
        BASEGFX_DLLPUBLIC BColor hsv2rgb(const BColor& rHSVColor);

        /// Transform from R'G'B' to YIQ (NTSC color model - Y is used in monochrome mode)
        BASEGFX_DLLPUBLIC BColor rgb2yiq(const BColor& rRGBColor);
        /// Transform from YIQ to R'G'B' (NTSC color model - Y is used in monochrome mode)
        BASEGFX_DLLPUBLIC BColor yiq2rgb(const BColor& rYIQColor);

        /// Transform from R'G'B' to Y'PbPr (the [0,1]x[-.5,.5]x[-.5,.5] equivalent of Y'CbCr (which is scaled into 8bit))
        BASEGFX_DLLPUBLIC BColor rgb2ypbpr(const BColor& rRGBColor);
        /// Transform from Y'PbPr (the [0,1]x[-.5,.5]x[-.5,.5] equivalent of Y'CbCr (which is scaled into 8bit)) into R'G'B'
        BASEGFX_DLLPUBLIC BColor ypbpr2rgb(const BColor& rYPbPrColor);

        /// Transform from CIE XYZ into Rec. 709 RGB (D65 white point)
        BASEGFX_DLLPUBLIC BColor ciexyz2rgb( const BColor& rXYZColor );
        /// Transform from Rec. 709 RGB (D65 white point) into CIE XYZ
        BASEGFX_DLLPUBLIC BColor rgb2ciexyz( const BColor& rRGBColor );

    }
} // end of namespace basegfx

#endif /* _BGFX_COLOR_BCOLORTOOLS_HXX */

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
