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

#include <tools/solar.h>
#include <tools/degree.hxx>
#include <vcl/checksum.hxx>
#include <vcl/dllapi.h>
#include <vcl/mapmod.hxx>
#include <vcl/region.hxx>
#include <vcl/scopedbitmapaccess.hxx>

#include <o3tl/typed_flags_set.hxx>

#include <memory>

#define GAMMA( _def_cVal, _def_InvGamma )   (static_cast<sal_uInt8>(MinMax(FRound(pow( _def_cVal/255.0,_def_InvGamma)*255.0),0,255)))

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
    BiLinear,
    Super // bilinear interpolation when supersampling and averaging when subsampling under certain scale
};

#define BMP_COL_TRANS               Color( 252, 3, 251 )

enum class BmpConversion
{
    NNONE,
    N1BitThreshold,
    N4BitGreys,
    N4BitColors,
    N8BitGreys,
    N8BitColors,
    N24Bit,
    N32Bit,
    N8BitTrans,
    N8BitNoConversion // make 8bit without color conversion (e.g. take the red channel)
};

enum class BmpCombine
{
    Or, And
};

class   BitmapInfoAccess;
class   BitmapReadAccess;
class   BitmapWriteAccess;
class   BitmapPalette;
class   GDIMetaFile;
class   AlphaMask;
class   OutputDevice;
class   SalBitmap;

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

class SAL_WARN_UNUSED VCL_DLLPUBLIC Bitmap
{
public:

                            Bitmap();
                            Bitmap( const Bitmap& rBitmap );
                            Bitmap( const Size& rSizePixel, sal_uInt16 nBitCount, const BitmapPalette* pPal = nullptr );
    explicit                Bitmap( std::shared_ptr<SalBitmap> const & xSalBitmap );
    virtual                 ~Bitmap();

    Bitmap&                 operator=( const Bitmap& rBitmap );
    Bitmap&                 operator=( Bitmap&& rBitmap ) noexcept;
    inline bool             operator!() const;
    bool                    operator==( const Bitmap& rBitmap ) const;
    bool                    operator!=( const Bitmap& rBitmap ) const { return !operator==(rBitmap); }

    inline bool             IsEmpty() const;
    void                    SetEmpty();

    inline const MapMode&   GetPrefMapMode() const;
    inline void             SetPrefMapMode( const MapMode& rMapMode );

    inline const Size&      GetPrefSize() const;
    inline void             SetPrefSize( const Size& rSize );

    Size                    GetSizePixel() const;

    sal_uInt16              GetBitCount() const;
    inline sal_Int64        GetColorCount() const;
    inline sal_uLong        GetSizeBytes() const;
    bool                    HasGreyPalette8Bit() const;
    bool                    HasGreyPaletteAny() const;
    /** get system dependent bitmap data

        @param rData
        The system dependent BitmapSystemData structure to be filled

        @return true if the bitmap has a valid system object (e.g. not empty)
    */
    bool                    GetSystemData( BitmapSystemData& rData ) const;

    BitmapChecksum          GetChecksum() const;

    Bitmap                  CreateDisplayBitmap( OutputDevice* pDisplay ) const;

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
    bool                    Expand(
                                sal_uLong nDX, sal_uLong nDY,
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
    bool                    CopyPixel(
                                const tools::Rectangle& rRectDst,
                                const tools::Rectangle& rRectSrc,
                                const Bitmap* pBmpSrc = nullptr );

    bool                    CopyPixel_AlphaOptimized(
                                const tools::Rectangle& rRectDst,
                                const tools::Rectangle& rRectSrc,
                                const Bitmap* pBmpSrc );

    /** Perform boolean operations with another bitmap

        @param rMask
        The mask bitmap in the selected combine operation

        @param eCombine
        The combine operation to perform on the bitmap

        @return true, if the operation was completed successfully.
     */
    bool                    CombineSimple(
                                const Bitmap& rMask,
                                BmpCombine eCombine );

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
    bool                    Blend(
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
    void AdaptBitCount(Bitmap& rNew) const;

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
    bool                    Rotate( Degree10 nAngle10, const Color& rFillColor );

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
    Bitmap                  CreateMask( const Color& rTransColor, sal_uInt8 nTol = 0 ) const;

    /** Create region of similar colors in a given rectangle

        @param rColor
        All pixel which have this color are included in the calculated region

        @param rRect
        The rectangle within which matching pixel are looked for. This
        rectangle is always clipped to the bitmap dimensions.

        @return the generated region.
     */
    vcl::Region                  CreateRegion( const Color& rColor, const tools::Rectangle& rRect ) const;

    /** Replace all pixel where the given mask is on with the specified color

        @param rMask
        Mask specifying which pixel should be replaced

        @param rReplaceColor
        Color to be placed in all changed pixel

        @return true, if the operation was completed successfully.
     */
    bool                    Replace( const Bitmap& rMask, const Color& rReplaceColor );

    /** Merge bitmap with given background color according to specified alpha mask

        @param rAlpha
        Alpha mask specifying the amount of background color to merge in

        @param rMergeColor
        Background color to be used for merging

        @return true, if the operation was completed successfully.
     */
    bool                    Replace( const AlphaMask& rAlpha, const Color& rMergeColor );

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
    bool                    Replace( const Color& rSearchColor, const Color& rReplaceColor, sal_uInt8 nTol = 0 );

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
    bool                    Replace(
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
    bool                    Adjust(
                                short nLuminancePercent,
                                short nContrastPercent = 0,
                                short nChannelRPercent = 0,
                                short nChannelGPercent = 0,
                                short nChannelBPercent = 0,
                                double fGamma = 1.0,
                                bool bInvert = false,
                                bool msoBrightness = false );

public:
    /** ReassignWithSize and recalculate bitmap.

      ReassignWithSizes the bitmap, and recalculates the bitmap size based on the new bitmap.

      @param rBitmap Bitmap to reassign and use for size calculation
     */
    SAL_DLLPRIVATE void     ReassignWithSize(const Bitmap& rBitmap);

    SAL_DLLPRIVATE void     ImplMakeUnique();
    const std::shared_ptr<SalBitmap>& ImplGetSalBitmap() const { return mxSalBmp; }
    SAL_DLLPRIVATE void     ImplSetSalBitmap( const std::shared_ptr<SalBitmap>& xImpBmp );

    SAL_DLLPRIVATE bool     ImplMakeGreyscales( sal_uInt16 nGreyscales );

public:

    BitmapInfoAccess*       AcquireInfoAccess();
    BitmapReadAccess*       AcquireReadAccess();
    BitmapWriteAccess*      AcquireWriteAccess();
    static void             ReleaseAccess( BitmapInfoAccess* pAccess );

    typedef vcl::ScopedBitmapAccess<BitmapReadAccess, Bitmap, &Bitmap::AcquireReadAccess> ScopedReadAccess;
    typedef vcl::ScopedBitmapAccess<BitmapInfoAccess, Bitmap, &Bitmap::AcquireInfoAccess> ScopedInfoAccess;

private:
    SAL_DLLPRIVATE bool ImplConvertUp(sal_uInt16 nBitCount, Color const* pExtColor = nullptr);
    SAL_DLLPRIVATE bool ImplConvertDown(sal_uInt16 nBitCount, Color const* pExtColor = nullptr);

private:
    std::shared_ptr<SalBitmap> mxSalBmp;
    MapMode maPrefMapMode;
    Size maPrefSize;

};

inline bool Bitmap::operator!() const
{
    return( mxSalBmp == nullptr );
}

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

inline sal_Int64 Bitmap::GetColorCount() const
{
    return sal_Int64(1) << sal_Int64(GetBitCount());
}

inline sal_uLong Bitmap::GetSizeBytes() const
{
    const Size aSizePix( GetSizePixel() );
    return( ( static_cast<sal_uLong>(aSizePix.Width()) * aSizePix.Height() * GetBitCount() ) >> 3 );
}

#endif // INCLUDED_VCL_BITMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
