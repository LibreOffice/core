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

#ifndef INCLUDED_VCL_BITMAPEX_HXX
#define INCLUDED_VCL_BITMAPEX_HXX

#include <vcl/dllapi.h>
#include <vcl/bitmap.hxx>
#include <vcl/alpha.hxx>
#include <tools/color.hxx>

#include <sal/types.h>

namespace com { namespace sun { namespace star { namespace rendering {
    class XBitmapCanvas;
} } } }
namespace com { namespace sun { namespace star { namespace uno { template <class interface_type> class Reference; } } } }
namespace basegfx { class BColorModifierStack; }

enum class TransparentType
{
    NONE,
    Color,
    Bitmap
};

class SAL_WARN_UNUSED VCL_DLLPUBLIC BitmapEx
{
public:

                        BitmapEx();
    explicit            BitmapEx( const OUString& rIconName );
                        BitmapEx( const BitmapEx& rBitmapEx );
                        BitmapEx( const BitmapEx& rBitmapEx, Point aSrc, Size aSize );
    explicit            BitmapEx( const Bitmap& rBmp );
                        BitmapEx( const Bitmap& rBmp, const Bitmap& rMask );
                        BitmapEx( const Bitmap& rBmp, const AlphaMask& rAlphaMask );
                        BitmapEx( const Bitmap& rBmp, const Color& rTransparentColor );

    BitmapEx&           operator=( const BitmapEx& rBitmapEx );
    BitmapEx&           operator=( const Bitmap& rBitmap ) { return operator=(BitmapEx(rBitmap)); }
    bool                operator==( const BitmapEx& rBitmapEx ) const;
    bool                operator!=( const BitmapEx& rBitmapEx ) const { return !(*this==rBitmapEx); }
    bool                operator!() const { return !maBitmap; }

    bool                IsEmpty() const;
    void                SetEmpty();
    void                Clear();

    void                Draw( OutputDevice* pOutDev,
                              const Point& rDestPt ) const;
    void                Draw( OutputDevice* pOutDev,
                              const Point& rDestPt, const Size& rDestSize ) const;

    bool                IsTransparent() const;
    TransparentType     GetTransparentType() const { return meTransparent; }

    Bitmap              GetBitmap( const Color* pTransReplaceColor = nullptr ) const;
    /// Gives direct access to the contained bitmap.
    const Bitmap&       GetBitmapRef() const;
    Bitmap              GetMask() const;

    bool                IsAlpha() const;
    AlphaMask           GetAlpha() const;

    const Size&         GetSizePixel() const { return maBitmapSize; }
    void                SetSizePixel(const Size& rNewSize);

    const Size&         GetPrefSize() const { return maBitmap.GetPrefSize(); }
    void                SetPrefSize( const Size& rPrefSize ) { maBitmap.SetPrefSize( rPrefSize ); }

    const MapMode&      GetPrefMapMode() const { return maBitmap.GetPrefMapMode(); }
    void                SetPrefMapMode( const MapMode& rPrefMapMode ) { maBitmap.SetPrefMapMode( rPrefMapMode ); }

    const Color&        GetTransparentColor() const { return maTransparentColor; }

    sal_uInt16          GetBitCount() const { return maBitmap.GetBitCount(); }
    sal_uLong           GetSizeBytes() const;
    BitmapChecksum      GetChecksum() const;

public:

    /**
     * @brief extract the bitmap and alpha data separately. Used by the SWF filter.
     */
    void GetSplitData( std::vector<sal_uInt8>& rvColorData, std::vector<sal_uInt8>& rvAlphaData ) const;

    /** Convert bitmap format

        @param eConversion
        The format this bitmap should be converted to.

        @return true, if the conversion was completed successfully.
     */
    bool                Convert( BmpConversion eConversion );

    /** Crop the bitmap

        @param rRectPixel
        A rectangle specifying the crop amounts on all four sides of
        the bitmap. If the upper left corner of the bitmap is assigned
        (0,0), then this method cuts out the given rectangle from the
        bitmap. Note that the rectangle is clipped to the bitmap's
        dimension, i.e. negative left,top rectangle coordinates or
        exceeding width or height is ignored.

        @return true, if cropping was performed successfully. If
        nothing had to be cropped, because e.g. the crop rectangle
        included the bitmap, false is returned, too!
     */
    bool                Crop( const tools::Rectangle& rRectPixel );

    /** Expand the bitmap by pixel padding

        @param nDX
        Number of pixel to pad at the right border of the bitmap

        @param nDY
        Number of scanlines to pad at the bottom border of the bitmap

        @param bExpandTransparent
        Whether to expand the transparency color or not.
     */
    void                Expand(
                            sal_uLong nDX, sal_uLong nDY,
                            bool bExpandTransparent = false );

    /** Copy a rectangular area from another bitmap

        @param rRectDst
        Destination rectangle in this bitmap. This is clipped to the
        bitmap dimensions.

        @param rRectSrc
        Source rectangle in pBmpSrc. This is clipped to the source
        bitmap dimensions. Note further that no scaling takes place
        during this copy operation, i.e. only the minimum of source
        and destination rectangle's width and height are used.

        @param pBmpExSrc
        The source bitmap to copy from. If this argument is NULL, or
        equal to the object this method is called on, copying takes
        place within the same bitmap.

        @return true, if the operation completed successfully. false
        is not only returned when the operation failed, but also if
        nothing had to be done, e.g. because one of the rectangles are
        empty.
     */
    bool                CopyPixel(
                            const tools::Rectangle& rRectDst,
                            const tools::Rectangle& rRectSrc,
                            const BitmapEx* pBmpExSrc );

    /** Fill the entire bitmap with the given color

        @param rFillColor
        Color value to use for filling. Set the transparency part of
        the color to fill the mask.

        @return true, if the operation was completed successfully.
     */
    bool                Erase( const Color& rFillColor );

    /** Perform the Invert operation on every pixel

        @return true, if the operation was completed successfully.
     */
    bool                Invert();

    /** Mirror the bitmap

        @param nMirrorFlags
        About which axis (horizontal, vertical, or both) to mirror

        @return true, if the operation was completed successfully.
     */
    bool                Mirror( BmpMirrorFlags nMirrorFlags );

    /** Scale the bitmap

        @param rNewSize
        The resulting size of the scaled bitmap

        @param nScaleFlag
        The algorithm to be used for scaling

        @return true, if the operation was completed successfully.
     */
    bool                Scale(
                            const Size& rNewSize,
                            BmpScaleFlag nScaleFlag = BmpScaleFlag::Default );

    /** Scale the bitmap

        @param rScaleX
        The scale factor in x direction.

        @param rScaleY
        The scale factor in y direction.

        @param nScaleFlag
        The algorithm to be used for scaling

        @return true, if the operation was completed successfully.
     */
    bool                Scale(
                            const double& rScaleX,
                            const double& rScaleY,
                            BmpScaleFlag nScaleFlag = BmpScaleFlag::Default );

    /** Rotate bitmap by the specified angle

        @param nAngle10
        The rotation angle in tenth of a degree. The bitmap is always rotated around its center.

        @param rFillColor
        The color to use for filling blank areas. During rotation, the
        bitmap is enlarged such that the whole rotation result fits
        in. The empty spaces around that rotated original bitmap are
        then filled with this color.

        @return true, if the operation was completed successfully.
     */
    bool                Rotate(
                            long nAngle10,
                            const Color& rFillColor );

    /** Replace all pixel having the search color with the specified color

        @param rSearchColor
        Color specifying which pixel should be replaced

        @param rReplaceColor
        Color to be placed in all changed pixel
     */
    void                Replace(
                            const Color& rSearchColor,
                            const Color& rReplaceColor );

    /** Replace all pixel having the search color with the specified color

        @param rSearchColor
        Color specifying which pixel should be replaced

        @param rReplaceColor
        Color to be placed in all changed pixel

        @param nTolerance
        Tolerance value. Specifies the maximal difference between
        rSearchColor and the individual pixel values, such that the
        corresponding pixel is still regarded a match.
     */
    void                Replace(
                            const Color& rSearchColor,
                            const Color& rReplaceColor,
                            sal_uInt8 nTolerance );

    /** Replace all pixel having one the search colors with the corresponding replace color

        @param pSearchColors
        Array of colors specifying which pixel should be replaced

        @param pReplaceColors
        Array of colors to be placed in all changed pixel

        @param nColorCount
        Size of the aforementioned color arrays

        @param pTols
        Tolerance value. Specifies the maximal difference between
        pSearchColor colors and the individual pixel values, such that
        the corresponding pixel is still regarded a match.
     */
    void                Replace(
                            const Color* pSearchColors,
                            const Color* pReplaceColors,
                            sal_uLong nColorCount );

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
        If true, invert the channel values with the logical 'not' operator

        @param msoBrightness
        Use the same formula for brightness as used by MSOffice.

        @return true, if the operation was completed successfully.
     */
    bool                Adjust(
                            short nLuminancePercent,
                            short nContrastPercent,
                            short nChannelRPercent,
                            short nChannelGPercent,
                            short nChannelBPercent,
                            double fGamma = 1.0,
                            bool bInvert = false,
                            bool msoBrightness = false );

    /** Get transparency at given position

        @param nX
        integer X-Position in Bitmap

        @param nY
        integer Y-Position in Bitmap

        @return transparency value in the range of [0 .. 255] where
                0 is not transparent, 255 is fully transparent
     */
    sal_uInt8           GetTransparency(
                            sal_Int32 nX,
                            sal_Int32 nY) const;

    /** Get pixel color (including alpha) at given position

        @param nX
        integer X-Position in Bitmap

        @param nY
        integer Y-Position in Bitmap
     */
    ::Color             GetPixelColor(
                            sal_Int32 nX,
                            sal_Int32 nY) const;

    /** Create transformed Bitmap

        @param fWidth
        The target width in pixels

        @param fHeight
        The target height in pixels

        @param rTransformation
        The back transformation for each pixel in (0 .. fWidth),(0 .. fHeight) to
        local pixel coordinates

        @param bSmooth
        Defines if pixel interpolation is to be used to create the result
    */
    [[nodiscard]]
    BitmapEx            TransformBitmapEx(
                            double fWidth,
                            double fHeight,
                            const basegfx::B2DHomMatrix& rTransformation,
                            bool bSmooth) const;

    /** Create transformed Bitmap

        @param rTransformation
        The transformation from unit coordinates to the unit range

        @param rVisibleRange
        The relative visible range in unit coordinates, relative to (0,0,1,1) which
        defines the whole target area

        @param fMaximumArea
        A limitation for the maximum size of pixels to use for the result

        The traget size of the result bitmap is defined by transforming the given
        rTargetRange with the given rTransformation; the area of the result is
        linearly scaled to not exceed the given fMaximumArea

        @return The transformed bitmap
    */
    [[nodiscard]]
    BitmapEx            getTransformed(
                            const basegfx::B2DHomMatrix& rTransformation,
                            const basegfx::B2DRange& rVisibleRange,
                            double fMaximumArea) const;

    /** Create ColorStack-modified version of this BitmapEx

        @param rBColorModifierStack
        A ColrModifierStack which defines how each pixel has to be modified
    */
    [[nodiscard]]
    BitmapEx            ModifyBitmapEx( const basegfx::BColorModifierStack& rBColorModifierStack) const;

    [[nodiscard]]
    static BitmapEx     AutoScaleBitmap( BitmapEx const & aBitmap, const long aStandardSize );

    /// populate from a canvas implementation
    bool                Create(
                            const css::uno::Reference< css::rendering::XBitmapCanvas > &xBitmapCanvas,
                            const Size &rSize );

    void                setAlphaFrom( sal_uInt8 cIndexFrom, sal_Int8 nAlphaTo );

    void                AdjustTransparency( sal_uInt8 cTrans );

    void                CombineMaskOr(Color maskColor, sal_uInt8 nTol);

    /**
     * Retrieves the color model data we need for the XImageConsumer stuff.
     */
    void                GetColorModel(css::uno::Sequence< sal_Int32 >& rRGBPalette,
                            sal_uInt32& rnRedMask, sal_uInt32& rnGreenMask, sal_uInt32& rnBlueMask, sal_uInt32& rnAlphaMask, sal_uInt32& rnTransparencyIndex,
                            sal_uInt32& rnWidth, sal_uInt32& rnHeight, sal_uInt8& rnBitCount);
public:

    SAL_DLLPRIVATE std::shared_ptr<SalBitmap> const & ImplGetBitmapSalBitmap() const { return maBitmap.ImplGetSalBitmap(); }
    SAL_DLLPRIVATE std::shared_ptr<SalBitmap> const & ImplGetMaskSalBitmap() const { return maMask.ImplGetSalBitmap(); }


private:
    friend class ImpGraphic;
    friend bool VCL_DLLPUBLIC WriteDIBBitmapEx(const BitmapEx& rSource, SvStream& rOStm);
    void  loadFromIconTheme( const OUString& rIconName );

    Bitmap              maBitmap;
    Bitmap              maMask;
    Size                maBitmapSize;
    Color               maTransparentColor;
    TransparentType     meTransparent;
    bool                mbAlpha;

};


/** Create a blend frame as BitmapEx

    @param nAlpha
    The blend value defines how strong the frame will be blended with the
    existing content, 255 == full coverage, 0 == no frame will be drawn

    @param aColorTopLeft, aColorBottomRight
    The colors defining the frame. These colors are linearly interpolated from
    aColorTopLeft and aColorBottomRight using the width and height of the area

    @param rSize
    The size of the frame in pixels
    */
BitmapEx VCL_DLLPUBLIC createBlendFrame(
    const Size& rSize,
    sal_uInt8 nAlpha,
    Color aColorTopLeft,
    Color aColorBottomRight);


/** Create a blend frame as BitmapEx

    @param nAlpha
    The blend value defines how strong the frame will be blended with the
    existing content, 255 == full coverage, 0 == no frame will be drawn

    @param aColorTopLeft, aColorBottomRight, aColorTopRight, aColorBottomLeft
    The colors defining the frame.

    @param rSize
    The size of the frame in pixels
    */
BitmapEx VCL_DLLPUBLIC createBlendFrame(
    const Size& rSize,
    sal_uInt8 nAlpha,
    Color aColorTopLeft,
    Color aColorTopRight,
    Color aColorBottomRight,
    Color aColorBottomLeft);

#endif // INCLUDED_VCL_BITMAPEX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
