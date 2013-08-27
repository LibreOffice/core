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

#ifndef _SV_BITMAPEX_HXX
#define _SV_BITMAPEX_HXX

#include <vcl/dllapi.h>
#include <vcl/bitmap.hxx>
#include <vcl/alpha.hxx>
#include <tools/color.hxx>
#include <basegfx/color/bcolormodifier.hxx>

#include <com/sun/star/uno/Reference.hxx>

namespace com { namespace sun { namespace star { namespace rendering {
    class XBitmapCanvas;
} } } }

// -------------------
// - TransparentType -
// -------------------

enum TransparentType
{
    TRANSPARENT_NONE,
    TRANSPARENT_COLOR,
    TRANSPARENT_BITMAP
};

// ------------
// - BitmapEx -
// ------------

class VCL_DLLPUBLIC BitmapEx
{
private:
    friend class ImpGraphic;
    friend bool VCL_DLLPUBLIC WriteDIBBitmapEx(const BitmapEx& rSource, SvStream& rOStm);

    Bitmap              aBitmap;
    Bitmap              aMask;
    Size                aBitmapSize;
    Color               aTransparentColor;
    TransparentType     eTransparent;
    sal_Bool            bAlpha;

public:


    SAL_DLLPRIVATE  ImpBitmap*  ImplGetBitmapImpBitmap() const { return aBitmap.ImplGetImpBitmap(); }
    SAL_DLLPRIVATE  ImpBitmap*  ImplGetMaskImpBitmap() const { return aMask.ImplGetImpBitmap(); }

                        BitmapEx();
                        BitmapEx( const ResId& rResId );
                        BitmapEx( const BitmapEx& rBitmapEx );
                        BitmapEx( const BitmapEx& rBitmapEx, Point aSrc, Size aSize );
                        BitmapEx( const Bitmap& rBmp );
                        BitmapEx( const Bitmap& rBmp, const Bitmap& rMask );
                        BitmapEx( const Bitmap& rBmp, const AlphaMask& rAlphaMask );
                        BitmapEx( const Bitmap& rBmp, const Color& rTransparentColor );
                        ~BitmapEx();

    BitmapEx&           operator=( const BitmapEx& rBitmapEx );
    sal_Bool                operator==( const BitmapEx& rBitmapEx ) const;
    sal_Bool                operator!=( const BitmapEx& rBitmapEx ) const { return !(*this==rBitmapEx); }
    sal_Bool                operator!() const { return !aBitmap; }

    sal_Bool                IsEqual( const BitmapEx& rBmpEx ) const;

    sal_Bool                IsEmpty() const;
    void                SetEmpty();
    void                Clear();

    void                Draw( OutputDevice* pOutDev,
                              const Point& rDestPt ) const;
    void                Draw( OutputDevice* pOutDev,
                              const Point& rDestPt, const Size& rDestSize ) const;

    sal_Bool                IsTransparent() const;
    TransparentType     GetTransparentType() const { return eTransparent; }

    Bitmap              GetBitmap( const Color* pTransReplaceColor = NULL ) const;
    Bitmap              GetMask() const;

    BitmapEx            GetColorTransformedBitmapEx() const;

    sal_Bool                IsAlpha() const;
    AlphaMask           GetAlpha() const;

    const Size&         GetSizePixel() const { return aBitmapSize; }
    void                SetSizePixel( const Size& rNewSize, sal_uInt32 nScaleFlag = BMP_SCALE_DEFAULT );

    const Size&         GetPrefSize() const { return aBitmap.GetPrefSize(); }
    void                SetPrefSize( const Size& rPrefSize ) { aBitmap.SetPrefSize( rPrefSize ); }

    const MapMode&      GetPrefMapMode() const { return aBitmap.GetPrefMapMode(); }
    void                SetPrefMapMode( const MapMode& rPrefMapMode ) { aBitmap.SetPrefMapMode( rPrefMapMode ); }

    const Color&        GetTransparentColor() const { return aTransparentColor; }
    void                SetTransparentColor( const Color& rColor ) { aTransparentColor = rColor; }

    sal_uInt16              GetBitCount() const { return aBitmap.GetBitCount(); }
    sal_uLong               GetSizeBytes() const;
    sal_uLong               GetChecksum() const;

public:

    /** Convert bitmap format

        @param eConversion
        The format this bitmap should be converted to.

        @return sal_True, if the conversion was completed successfully.
     */
    sal_Bool                Convert( BmpConversion eConversion );

    /** Reduce number of colors for the bitmap

        @param nNewColorCount
        Maximal number of bitmap colors after the reduce operation

        @param eReduce
        Algorithm to use for color reduction

        @return sal_True, if the color reduction operation was completed successfully.
     */
    sal_Bool                ReduceColors( sal_uInt16 nNewColorCount,
                                      BmpReduce eReduce = BMP_REDUCE_SIMPLE );

    /** Apply a dither algorithm to the bitmap

        This method dithers the bitmap inplace, i.e. a true color
        bitmap is converted to a paletted bitmap, reducing the color
        deviation by error diffusion.

        @param nDitherFlags
        The algorithm to be used for dithering
     */
    sal_Bool                Dither( sal_uLong nDitherFlags = BMP_DITHER_MATRIX );

    /** Crop the bitmap

        @param rRectPixel
        A rectangle specifying the crop amounts on all four sides of
        the bitmap. If the upper left corner of the bitmap is assigned
        (0,0), then this method cuts out the given rectangle from the
        bitmap. Note that the rectangle is clipped to the bitmap's
        dimension, i.e. negative left,top rectangle coordinates or
        exceeding width or height is ignored.

        @return sal_True, if cropping was performed successfully. If
        nothing had to be cropped, because e.g. the crop rectangle
        included the bitmap, sal_False is returned, too!
     */
    sal_Bool                Crop( const Rectangle& rRectPixel );

    /** Expand the bitmap by pixel padding

        @param nDX
        Number of pixel to pad at the right border of the bitmap

        @param nDY
        Number of scanlines to pad at the bottom border of the bitmap

        @param pInitColor
        Color to use for padded pixel

        @return sal_True, if padding was performed successfully. sal_False is
        not only returned when the operation failed, but also if
        nothing had to be done, e.g. because nDX and nDY were zero.
     */
    sal_Bool                Expand( sal_uLong nDX, sal_uLong nDY,
                                const Color* pInitColor = NULL,
                                sal_Bool bExpandTransparent = sal_False );

    /** Copy a rectangular area from another bitmap

        @param rRectDst
        Destination rectangle in this bitmap. This is clipped to the
        bitmap dimensions.

        @param rRectSrc
        Source rectangle in pBmpSrc. This is clipped to the source
        bitmap dimensions. Note further that no scaling takes place
        during this copy operation, i.e. only the minimum of source
        and destination rectangle's width and height are used.

        @param pBmpSrc
        The source bitmap to copy from. If this argument is NULL, or
        equal to the object this method is called on, copying takes
        place within the same bitmap.

        @return sal_True, if the operation completed successfully. sal_False
        is not only returned when the operation failed, but also if
        nothing had to be done, e.g. because one of the rectangles are
        empty.
     */
    sal_Bool                CopyPixel( const Rectangle& rRectDst,
                                   const Rectangle& rRectSrc,
                                   const BitmapEx* pBmpExSrc = NULL );

    /** Fill the entire bitmap with the given color

        @param rFillColor
        Color value to use for filling. Set the transparency part of
        the color to fill the mask.

        @return sal_True, if the operation was completed successfully.
     */
    sal_Bool                Erase( const Color& rFillColor );

    /** Perform the Invert operation on every pixel

        @return sal_True, if the operation was completed successfully.
     */
    sal_Bool                Invert();

    /** Mirror the bitmap

        @param nMirrorFlags
        About which axis (horizontal, vertical, or both) to mirror

        @return sal_True, if the operation was completed successfully.
     */
    sal_Bool                Mirror( sal_uLong nMirrorFlags );

    /** Scale the bitmap

        @param rNewSize
        The resulting size of the scaled bitmap

        @param nScaleFlag
        The algorithm to be used for scaling

        @return sal_True, if the operation was completed successfully.
     */
    sal_Bool                Scale( const Size& rNewSize, sal_uInt32 nScaleFlag = BMP_SCALE_DEFAULT );

    /** Scale the bitmap

        @param rScaleX
        The scale factor in x direction.

        @param rScaleY
        The scale factor in y direction.

        @return sal_True, if the operation was completed successfully.
     */
    sal_Bool                Scale( const double& rScaleX, const double& rScaleY, sal_uInt32 nScaleFlag = BMP_SCALE_DEFAULT );

    /** Rotate bitmap by the specified angle

        @param nAngle10
        The rotation angle in tenth of a degree. The bitmap is always rotated around its center.

        @param rFillColor
        The color to use for filling blank areas. During rotation, the
        bitmap is enlarged such that the whole rotation result fits
        in. The empty spaces around that rotated original bitmap are
        then filled with this color.

        @return sal_True, if the operation was completed successfully.
     */
    sal_Bool                Rotate( long nAngle10, const Color& rFillColor );

    /** Replace all pixel having the search color with the specified color

        @param rSearchColor
        Color specifying which pixel should be replaced

        @param rReplaceColor
        Color to be placed in all changed pixel

        @param nTol
        Tolerance value. Specifies the maximal difference between
        rSearchColor and the individual pixel values, such that the
        corresponding pixel is still regarded a match.

        @return sal_True, if the operation was completed successfully.
     */
    sal_Bool                Replace( const Color& rSearchColor, const Color& rReplaceColor, sal_uLong nTol = 0 );

    /** Replace all pixel having one the search colors with the corresponding replace color

        @param pSearchColor
        Array of colors specifying which pixel should be replaced

        @param pReplaceColor
        Array of colors to be placed in all changed pixel

        @param nColorCount
        Size of the aforementioned color arrays

        @param nTol
        Tolerance value. Specifies the maximal difference between
        pSearchColor colors and the individual pixel values, such that
        the corresponding pixel is still regarded a match.

        @return sal_True, if the operation was completed successfully.
     */
    sal_Bool                Replace( const Color* pSearchColors, const Color* pReplaceColors,
                                 sal_uLong nColorCount, const sal_uLong* pTols = NULL );

    /** Change various global color characteristics

        @param nLuminancePercent
        Percent of luminance change, valid range [-100,100]. Values outside this range are clipped to the valid range.

        @param nContrastPercent
        Percent of contrast change, valid range [-100,100]. Values outside this range are clipped to the valid range.

        @param nChannelRPercent
        Percent of red channel change, valid range [-100,100]. Values outside this range are clipped to the valid range.

        @param nChannelGPercent
        Percent of green channel change, valid range [-100,100]. Values outside this range are clipped to the valid range.

        @param nChannelBPercent
        Percent of blue channel change, valid range [-100,100]. Values outside this range are clipped to the valid range.

        @param fGamma
        Exponent of the gamma function applied to the bitmap. The
        value 1.0 results in no change, the valid range is
        (0.0,10.0]. Values outside this range are regarded as 1.0.

        @param bInvert
        If sal_True, invert the channel values with the logical 'not' operator

        @return sal_True, if the operation was completed successfully.
     */
    sal_Bool                Adjust( short nLuminancePercent = 0,
                                short nContrastPercent = 0,
                                short nChannelRPercent = 0,
                                short nChannelGPercent = 0,
                                short nChannelBPercent = 0,
                                double fGamma = 1.0,
                                sal_Bool bInvert = sal_False );

    /** Apply specified filter to the bitmap

        @param eFilter
        The filter algorithm to apply

        @param pFilterParam
        Various parameter for the different bitmap filter algorithms

        @param pProgress
        A callback for showing the progress of the vectorization

        @return sal_True, if the operation was completed successfully.
     */
    sal_Bool                Filter( BmpFilter eFilter,
                                const BmpFilterParam* pFilterParam = NULL,
                                const Link* pProgress = NULL );

    /** Get transparency at given position

        @param nX
        integer X-Position in Bitmap

        @param nY
        integer Y-Position in Bitmap

        @return transparency value in the range of [0 .. 255] where
                0 is not transparent, 255 is fully transparent
     */
    sal_uInt8 GetTransparency(sal_Int32 nX, sal_Int32 nY) const;

    /** Create transformed Bitmap

        @param fWidth
        The target width in pixels

        @param fHeight
        The target height in pixels

        @param rTransformation
        The back transformation for each pixel in (0 .. fWidth),(0 .. fHeight) to
        local pixel coordiantes

        @param bSmooth
        Defines if pixel interpolation is to be used to create the result
    */
    BitmapEx TransformBitmapEx(
        double fWidth,
        double fHeight,
        const basegfx::B2DHomMatrix& rTransformation,
        bool bSmooth = true) const;

    /** Create transformed Bitmap

        @param rTransformation
        The transformation from unit coordinates to the unit range

        @param rVisibleRange
        The relative visible range in unit coordinates, relative to (0,0,1,1) which
        defines the whole target area

        @param fMaximumArea
        A limitation for the maximum size of pixels to use for the result

        @param bSmooth
        Defines if pixel interpolation is to be used to create the result

        The traget size of the result bitmap is defined by transforming the given
        rTargetRange with the given rTransformation; the area of the result is
        linearly scaled to not exceed the given fMaximumArea

        @return The transformed bitmap
    */
    BitmapEx getTransformed(
        const basegfx::B2DHomMatrix& rTransformation,
        const basegfx::B2DRange& rVisibleRange,
        double fMaximumArea = 500000.0,
        bool bSmooth = true) const;

    /** Create ColorStack-modified version of this BitmapEx

        @param rBColorModifierStack
        A ColrModifierStack which defines how each pixel has to be modified
    */
    BitmapEx ModifyBitmapEx(const basegfx::BColorModifierStack& rBColorModifierStack) const;

public:

    static BitmapEx AutoScaleBitmap(BitmapEx & aBitmap, const long aStandardSize);

    /// populate from a canvas implementation
    bool Create( const ::com::sun::star::uno::Reference<
                 ::com::sun::star::rendering::XBitmapCanvas > &xBitmapCanvas,
                 const Size &rSize );
};

// ------------------------------------------------------------------
/** Create a blend frame as BitmapEx

    @param nAlpha
    The blend value defines how strong the frame will be blended with the
    existing content, 255 == full coverage, 0 == no frame will be drawn

    @param aColorTopLeft, aColorBottomRight, aColorTopRight, aColorBottomLeft
    The colors defining the frame. If the version without aColorTopRight and
    aColorBottomLeft is used, these colors are linearly interpolated from
    aColorTopLeft and aColorBottomRight using the width and height of the area

    @param rSize
    The size of the frame in pixels
    */

BitmapEx VCL_DLLPUBLIC createBlendFrame(
    const Size& rSize,
    sal_uInt8 nAlpha,
    Color aColorTopLeft,
    Color aColorBottomRight);

BitmapEx VCL_DLLPUBLIC createBlendFrame(
    const Size& rSize,
    sal_uInt8 nAlpha,
    Color aColorTopLeft,
    Color aColorTopRight,
    Color aColorBottomRight,
    Color aColorBottomLeft);

// ------------------------------------------------------------------

#endif // _SV_BITMAPEX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
