/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SVX_SVDOIMP_HXX
#define _SVX_SVDOIMP_HXX

#include <vcl/mapmod.hxx>

//#include <svl/lstner.hxx>
//#include <vcl/timer.hxx>
//#include <svx/svdsob.hxx>
//#include <svx/svdtypes.hxx> // fuer SdrLayerID
//#include <svx/svdglue.hxx> // Klebepunkte
//#include <svx/xdash.hxx>
//#include <svx/xpoly.hxx>
//#include <svx/xenum.hxx>
//#include <basegfx/vector/b2dvector.hxx>
#include <svx/rectenum.hxx>
//#include <basegfx/polygon/b2dpolypolygon.hxx>

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
