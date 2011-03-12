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

#ifndef _SVX_SVDOIMP_HXX
#define _SVX_SVDOIMP_HXX

#include <vcl/mapmod.hxx>

#include <svx/rectenum.hxx>


class Bitmap;

///////////////////////////////////////////////////////////////////////////////

// #104609# Extracted from old XOutDev's ImpCalcBmpFillStartValues

/** Calc offset and size for bitmap fill

    This method calculates the size and the offset from the left, top
    position of a shape in logical coordinates

    @param rStartOffset
    The offset from the left, top position of the output rectangle is returned

    @param rBmpOutputSize
    The output size of the bitmap is returned herein

    @param rOutputRect
    Specifies the output rectangle into which the bitmap should be tiled into

    @param rOutputMapMode
    Specifies the logical coordinate system the output rectangle is in

    @param rFillBitmap
    Specifies the bitmap to fill with

    @param rBmpSize
    The desired destination bitmap size. If null, size is taken from the bitmap

    @param rBmpPerCent
    Percentage of bitmap size, relative to the output rectangle

    @param rBmpOffPerCent
    Offset for bitmap tiling, in percentage relative to bitmap output size

    @param bBmpLogSize
    True when using the preferred bitmap size, False when using the percentage value

    @param bBmpTile
    True for tiling. False only paints one instance of the bitmap

    @param bBmpStretch
    True if bitmap should be stretched to output rect dimension

    @param eBmpRectPoint
    Position of the start point relative to the bitmap

 */
void ImpCalcBmpFillSizes( Size&            rStartOffset,
                          Size&            rBmpOutputSize,
                          const Rectangle& rOutputRect,
                          const MapMode&   rOutputMapMode,
                          const Bitmap&    rFillBitmap,
                          const Size&      rBmpSize,
                          const Size&      rBmpPerCent,
                          const Size&      rBmpOffPerCent,
                          sal_Bool             bBmpLogSize,
                          sal_Bool             bBmpTile,
                          sal_Bool             bBmpStretch,
                          RECT_POINT       eBmpRectPoint );


////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // _SVX_SVDOIMP_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
