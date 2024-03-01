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

#ifndef INCLUDED_VCL_BITMAP_HXX
#define INCLUDED_VCL_BITMAP_HXX

#include <tools/degree.hxx>
#include <tools/helpers.hxx>
#include <vcl/checksum.hxx>
#include <vcl/dllapi.h>
#include <vcl/mapmod.hxx>
#include <vcl/region.hxx>
#include <vcl/bitmap/BitmapTypes.hxx>

#include <o3tl/typed_flags_set.hxx>

#include <algorithm>
#include <memory>

inline sal_uInt8 GAMMA(double _def_cVal, double _def_InvGamma)
{
    return FRound(std::clamp(pow(_def_cVal / 255.0, _def_InvGamma) * 255.0, 0.0, 255.0));
}

class Color;

template <typename Arg, typename Ret> class Link;

enum class BmpMirrorFlags
{
    NONE             = 0x00,
    Horizontal       = 0x01,
    Vertical         = 0x02,
};

namespace o3tl
{
    template<> struct typed_flags<BmpMirrorFlags> : is_typed_flags<BmpMirrorFlags, 0x03> {};
}

enum class BmpScaleFlag
{
// Try to preferably use these.
    Default,
    Fast,
    BestQuality,
// Specific algorithms, use only if you really need to (mainly used for tests)
    NearestNeighbor,
    Interpolate, // fast, integer bilinear
    Lanczos,
    BiCubic,
    BiLinear
};

#define BMP_COL_TRANS               Color( 252, 3, 251 )

enum class BmpConversion
{
    NNONE,
    N1BitThreshold,
    N8BitGreys,
    N8BitColors,
    N24Bit,
    N32Bit,
    N8BitTrans,
    N8BitNoConversion // make 8bit without color conversion (e.g. take the red channel)
};

class   BitmapInfoAccess;
class   BitmapReadAccess;
class   BitmapWriteAccess;
class   BitmapPalette;
class   GDIMetaFile;
class   AlphaMask;
class   OutputDevice;
class   SalBitmap;

namespace basegfx { class SystemDependentDataHolder; }

struct BitmapSystemData
{
    #if defined(_WIN32)
    void* pDIB; // device independent byte buffer
    #elif defined( MACOSX ) || defined( IOS )
    // Nothing needed, apparently
    #else
    void* aPixmap;
    #endif
    int mnWidth;
    int mnHeight;
};

class SAL_WARN_UNUSED VCL_DLLPUBLIC Bitmap final
{
public:

                            Bitmap();
                            Bitmap( const Bitmap& rBitmap );
                            Bitmap( const Size& rSizePixel, vcl::PixelFormat ePixelFormat, const BitmapPalette* pPal = nullptr );
    SAL_DLLPRIVATE explicit Bitmap( std::shared_ptr<SalBitmap> xSalBitmap );
                            ~Bitmap();

    Bitmap&                 operator=( const Bitmap& rBitmap );
    Bitmap&                 operator=( Bitmap&& rBitmap ) noexcept;
    bool                    operator==( const Bitmap& rBitmap ) const;
    bool                    operator!=( const Bitmap& rBitmap ) const { return !operator==(rBitmap); }

    inline bool             IsEmpty() const;
    void                    SetEmpty();

    inline const MapMode&   GetPrefMapMode() const;
    inline void             SetPrefMapMode( const MapMode& rMapMode );

    inline const Size&      GetPrefSize() const;
    inline void             SetPrefSize( const Size& rSize );

    Size                    GetSizePixel() const;

    vcl::PixelFormat getPixelFormat() const;
    inline sal_Int64        GetSizeBytes() const;
    bool                    HasGreyPalette8Bit() const;
    bool                    HasGreyPaletteAny() const;
    /** get system dependent bitmap data

        @param rData
        The system dependent BitmapSystemData structure to be filled

        @return true if the bitmap has a valid system object (e.g. not empty)
    */
    bool                    GetSystemData( BitmapSystemData& rData ) const;

    BitmapChecksum          GetChecksum() const;

    SAL_DLLPRIVATE Bitmap   CreateDisplayBitmap( OutputDevice* pDisplay ) const;

    static const BitmapPalette&
                            GetGreyPalette( int nEntries );

public:

    /** Convert bitmap format

        @param eConversion
        The format this bitmap should be converted to.

        @return true the conversion was completed successfully.
     */
    bool                    Convert( BmpConversion eConversion );

    /** Apply a Floyd dither algorithm to the bitmap

     This method dithers the bitmap inplace, i.e. a true color
     bitmap is converted to a paletted bitmap, reducing the color
     deviation by error diffusion.

     */
    bool                    Dither();

    /** Crop the bitmap

        @param rRectPixel
        A rectangle specifying the crop amounts on all four sides of
        the bitmap. If the upper left corner of the bitmap is assigned
        (0,0), then this method cuts out the given rectangle from the
        bitmap. Note that the rectangle is clipped to the bitmap's
        dimension, i.e. negative left,top rectangle coordinates or
        exceeding width or height is ignored.

        @return true cropping was performed successfully. If
        nothing had to be cropped, because e.g. the crop rectangle
        included the bitmap, false is returned, too!
     */
    bool                    Crop( const tools::Rectangle& rRectPixel );

    /** Expand the bitmap by pixel padding

        @param nDX
        Number of pixel to pad at the right border of the bitmap

        @param nDY
        Number of scanlines to pad at the bottom border of the bitmap

        @param pInitColor
        Color to use for padded pixel

        @return true, if padding was performed successfully. false is
        not only returned when the operation failed, but also if
        nothing had to be done, e.g. because nDX and nDY were zero.
     */
    SAL_DLLPRIVATE bool Expand(
                                sal_Int32 nDX, sal_Int32 nDY,
                                const Color* pInitColor = nullptr );

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

        @return true, if the operation completed successfully. false
        is not only returned when the operation failed, but also if
        nothing had to be done, e.g. because one of the rectangles are
        empty.
     */
    SAL_DLLPRIVATE bool CopyPixel(
                                const tools::Rectangle& rRectDst,
                                const tools::Rectangle& rRectSrc,
                                const Bitmap& rBmpSrc );

    /** Copy a rectangular area inside this bitmap.

        @param rRectDst
        Destination rectangle in this bitmap. This is clipped to the
        bitmap dimensions.

        @param rRectSrc
        Source rectangle. This is clipped to the
        bitmap dimensions. Note further that no scaling takes place
        during this copy operation, i.e. only the minimum of source
        and destination rectangle's width and height are used.

        @return true, if the operation completed successfully. false
        is not only returned when the operation failed, but also if
        nothing had to be done, e.g. because one of the rectangles are
        empty.
     */
    SAL_DLLPRIVATE bool CopyPixel(
                                const tools::Rectangle& rRectDst,
                                const tools::Rectangle& rRectSrc );

    SAL_DLLPRIVATE bool CopyPixel_AlphaOptimized(
                                const tools::Rectangle& rRectDst,
                                const tools::Rectangle& rRectSrc,
                                const AlphaMask& rBmpSrc );

    SAL_DLLPRIVATE bool CopyPixel_AlphaOptimized(
                                const tools::Rectangle& rRectDst,
                                const tools::Rectangle& rRectSrc );

    /** Alpha-blend the given bitmap against a specified uniform
          background color.

        @attention This method might convert paletted bitmaps to
        truecolor, to be able to represent every necessary color. Note
        that during alpha blending, lots of colors not originally
        included in the bitmap can be generated.

        @param rAlpha
        Alpha mask to blend with

        @param rBackgroundColor
        Background color to use for every pixel during alpha blending

        @return true, if blending was successful, false otherwise
     */
    SAL_DLLPRIVATE bool Blend(
                                const AlphaMask& rAlpha,
                                const Color& rBackgroundColor );

    /** Fill the entire bitmap with the given color

        @param rFillColor
        Color value to use for filling

        @return true, if the operation was completed successfully.
     */
    bool                    Erase( const Color& rFillColor );

    /** Perform the Invert operation on every pixel

        @return true, if the operation was completed successfully.
     */
    bool                    Invert();

    /** Mirror the bitmap

        @param nMirrorFlags
        About which axis (horizontal, vertical, or both) to mirror

        @return true, if the operation was completed successfully.
     */
    bool                    Mirror( BmpMirrorFlags nMirrorFlags );

    /** Scale the bitmap

        @param rNewSize
        The resulting size of the scaled bitmap

        @param nScaleFlag
        The algorithm to be used for scaling

        @return true, if the operation was completed successfully.
     */
    bool                    Scale( const Size& rNewSize, BmpScaleFlag nScaleFlag = BmpScaleFlag::Default );

    /** Scale the bitmap

        @param rScaleX
        The scale factor in x direction.

        @param rScaleY
        The scale factor in y direction.

        @param nScaleFlag
        Method of scaling - it is recommended that either BmpScaleFlag::Default or BmpScaleFlag::BestQuality be used.

        @return true, if the operation was completed successfully.
     */
    bool                    Scale( const double& rScaleX, const double& rScaleY, BmpScaleFlag nScaleFlag = BmpScaleFlag::Default );

    /**
      Returns true if bitmap scaling is considered to be fast.

      Currently this returns true if OpenGL is used for scaling, otherwise false (CPU scaling is slower).

      @since 4.5
    */
    static bool HasFastScale();

    // Adapt the BitCount of rNew to BitCount of total, including grey or color palette
    // Can be used to create alpha/mask bitmaps after their processing in 24bit
    SAL_DLLPRIVATE void AdaptBitCount(Bitmap& rNew) const;

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
    SAL_DLLPRIVATE bool Rotate( Degree10 nAngle10, const Color& rFillColor );

    /** Create on-off mask from bitmap

        This method creates a bitmask from the bitmap, where every
        pixel that equals rTransColor is set transparent, the rest
        opaque.

        @param rTransColor
        Color value where the bitmask should be transparent

        @return the resulting bitmask.
     */
    Bitmap                  CreateMask( const Color& rTransColor ) const;

    /** Create on-off mask from bitmap

        This method creates a bitmask from the bitmap, where every
        pixel that equals rTransColor is set transparent, the rest
        opaque.

        @param rTransColor
        Color value where the bitmask should be transparent

        @param nTol
        Tolerance value. Specifies the maximal difference between
        rTransColor and the individual pixel values, such that the
        corresponding pixel is still regarded as transparent.

        @return the resulting bitmask.
     */
    Bitmap                  CreateMask( const Color& rTransColor, sal_uInt8 nTol ) const;

    /** Create on-off alpha mask from bitmap

        This method creates a bitmask from the bitmap, where every
        pixel that equals rTransColor is set transparent, the rest
        opaque.

        @param rTransColor
        Color value where the bitmask should be transparent

        @return the resulting bitmask.
     */
    AlphaMask              CreateAlphaMask( const Color& rTransColor ) const;

    /** Create on-off alpha mask from bitmap

        This method creates a bitmask from the bitmap, where every
        pixel that equals rTransColor is set transparent, the rest
        opaque.

        @param rTransColor
        Color value where the bitmask should be transparent

        @param nTol
        Tolerance value. Specifies the maximal difference between
        rTransColor and the individual pixel values, such that the
        corresponding pixel is still regarded as transparent.

        @return the resulting bitmask.
     */
    AlphaMask             CreateAlphaMask( const Color& rTransColor, sal_uInt8 nTol ) const;

    /** Create region of similar colors in a given rectangle

        @param rColor
        All pixel which have this color are included in the calculated region

        @param rRect
        The rectangle within which matching pixel are looked for. This
        rectangle is always clipped to the bitmap dimensions.

        @return the generated region.
     */
    SAL_DLLPRIVATE vcl::Region CreateRegion( const Color& rColor, const tools::Rectangle& rRect ) const;

    /** Merge bitmap with given background color according to specified alpha mask

        @param rAlpha
        Alpha mask specifying the amount of background color to merge in

        @param rMergeColor
        Background color to be used for merging

        @return true, if the operation was completed successfully.
     */
    SAL_DLLPRIVATE bool Replace( const AlphaMask& rAlpha, const Color& rMergeColor );

    /** Replace all pixel where the given mask/alpha layer is on with the specified color

        @param rMask
        Mask specifying which pixel should be replaced

        @param rReplaceColor
        Color to be placed in all changed pixel

        @return true, if the operation was completed successfully.
     */
    SAL_DLLPRIVATE bool ReplaceMask( const AlphaMask& rMask, const Color& rReplaceColor );

    /** Replace all pixel having the search color with the specified color

        @param rSearchColor
        Color specifying which pixel should be replaced

        @param rReplaceColor
        Color to be placed in all changed pixel

        @param nTol
        Tolerance value. Specifies the maximal difference between
        rSearchColor and the individual pixel values, such that the
        corresponding pixel is still regarded a match.

        @return true, if the operation was completed successfully.
     */
    SAL_DLLPRIVATE bool Replace( const Color& rSearchColor, const Color& rReplaceColor, sal_uInt8 nTol = 0 );

    /** Replace all pixel having one the search colors with the corresponding replace color

        @param pSearchColors
        Array of colors specifying which pixel should be replaced

        @param rReplaceColors
        Array of colors to be placed in all changed pixel

        @param nColorCount
        Size of the aforementioned color arrays

        @param pTols
        Tolerance value. Specifies the maximal difference between
        pSearchColor colors and the individual pixel values, such that
        the corresponding pixel is still regarded a match.

        @return true, if the operation was completed successfully.
     */
    SAL_DLLPRIVATE bool Replace(
                                const Color* pSearchColors,
                                const Color* rReplaceColors,
                                size_t nColorCount,
                                sal_uInt8 const * pTols );

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
    SAL_DLLPRIVATE bool Adjust(
                                short nLuminancePercent,
                                short nContrastPercent = 0,
                                short nChannelRPercent = 0,
                                short nChannelGPercent = 0,
                                short nChannelBPercent = 0,
                                double fGamma = 1.0,
                                bool bInvert = false,
                                bool msoBrightness = false );

    /** Remove existing blending against COL_WHITE based on given AlphaMask

        Inside convertToBitmapEx the content gets rendered to RGB target (no 'A'),
        so it gets blended against the start condition of the target device which
        is blank (usually white background, but others may be used).
        Usually rendering to RGB is sufficient (e.g. EditViews), but for conversion
        to BitmapEx the alpha channel is needed to e.g. allow export/conversion to
        pixel target formats which support Alpha, e.g. PNG.
        It is possible though to create the fully valid and correct AlphaChannel.
        If the content, the start condition and the alpha values are known it is
        possible to calculate back ("remove") the white blending from the result,
        and this is what this method does.

        @param rColor
        The Color we know this Bitmap is blended against (usually COL_WHITE)

        @param rAlphaMask
        The AlphaMask which was used to blend white against this
     */
    void                    RemoveBlendedStartColor(
                                const Color& rColor,
                                const AlphaMask& rAlphaMask);

    // access to SystemDependentDataHolder, to support overload in derived class(es)
    SAL_DLLPRIVATE const basegfx::SystemDependentDataHolder* accessSystemDependentDataHolder() const;

public:
    /** ReassignWithSize and recalculate bitmap.

      ReassignWithSizes the bitmap, and recalculates the bitmap size based on the new bitmap.

      @param rBitmap Bitmap to reassign and use for size calculation
     */
    SAL_DLLPRIVATE void     ReassignWithSize(const Bitmap& rBitmap);

    SAL_DLLPRIVATE void     ImplMakeUnique();
    const std::shared_ptr<SalBitmap>& ImplGetSalBitmap() const { return mxSalBmp; }
    SAL_DLLPRIVATE void     ImplSetSalBitmap( const std::shared_ptr<SalBitmap>& xImpBmp );

    SAL_DLLPRIVATE bool     ImplMakeGreyscales();

private:
    SAL_DLLPRIVATE bool ImplConvertUp(vcl::PixelFormat ePixelFormat, Color const* pExtColor = nullptr);
    SAL_DLLPRIVATE bool ImplConvertDown8BPP(Color const* pExtColor = nullptr);

private:
    std::shared_ptr<SalBitmap> mxSalBmp;
    MapMode maPrefMapMode;
    Size maPrefSize;
};

inline bool Bitmap::IsEmpty() const
{
    return( mxSalBmp == nullptr );
}

inline const MapMode& Bitmap::GetPrefMapMode() const
{
    return maPrefMapMode;
}

inline void Bitmap::SetPrefMapMode( const MapMode& rMapMode )
{
    maPrefMapMode = rMapMode;
}

inline const Size& Bitmap::GetPrefSize() const
{
    return maPrefSize;
}

inline void Bitmap::SetPrefSize( const Size& rSize )
{
    maPrefSize = rSize;
}

inline sal_Int64 Bitmap::GetSizeBytes() const
{
    const auto aSizePixel = GetSizePixel();
    const sal_Int64 aBitCount = vcl::pixelFormatBitCount(getPixelFormat());
    sal_Int64 aSizeInBytes = (aSizePixel.Width() * aSizePixel.Height() * aBitCount) / 8;
    return aSizeInBytes;
}

#endif // INCLUDED_VCL_BITMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
