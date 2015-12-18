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

#include <tools/color.hxx>
#include <tools/link.hxx>
#include <tools/solar.h>
#include <vcl/checksum.hxx>
#include <vcl/dllapi.h>
#include <vcl/mapmod.hxx>
#include <vcl/region.hxx>
#include <vcl/scopedbitmapaccess.hxx>
#include <o3tl/typed_flags_set.hxx>

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
    NONE              = 0,
// Try to preferably use these.
    Default           = 1,
    Fast              = 2,
    BestQuality       = 3,
// Specific algorithms,  use only if you really need to.
    Interpolate       = 4,
    Super             = 5,
    Lanczos           = 6,
    BiCubic           = 7,
    BiLinear          = 8,
    Box               = 9,
};


enum class BmpDitherFlags
{
    NONE             = 0x0000,
    Matrix           = 0x0001,
    Floyd            = 0x0002,
    Floyd16          = 0x0004,
};
namespace o3tl
{
    template<> struct typed_flags<BmpDitherFlags> : is_typed_flags<BmpDitherFlags, 0x07> {};
}

enum class BmpVectorizeFlags
{
    Inner         = 0x0001,
    Outer         = 0x0002,
    BoundOnly     = 0x0004,
    ReduceEdges   = 0x0008,
};
namespace o3tl
{
    template<> struct typed_flags<BmpVectorizeFlags> : is_typed_flags<BmpVectorizeFlags, 0x0f> {};
}

#define BMP_COL_TRANS               Color( 252, 3, 251 )

enum BmpConversion
{
    BMP_CONVERSION_NONE = 0,
    BMP_CONVERSION_1BIT_THRESHOLD = 1,
    BMP_CONVERSION_1BIT_MATRIX = 2,
    BMP_CONVERSION_4BIT_GREYS = 3,
    BMP_CONVERSION_4BIT_COLORS = 4,
    BMP_CONVERSION_8BIT_GREYS = 5,
    BMP_CONVERSION_8BIT_COLORS = 6,
    BMP_CONVERSION_24BIT = 7,
    BMP_CONVERSION_4BIT_TRANS = 8,
    BMP_CONVERSION_8BIT_TRANS = 9,
    BMP_CONVERSION_GHOSTED = 10
};

enum BmpCombine
{
    BMP_COMBINE_COPY = 0,
    BMP_COMBINE_INVERT = 1,
    BMP_COMBINE_AND = 2,
    BMP_COMBINE_NAND = 3,
    BMP_COMBINE_OR = 4,
    BMP_COMBINE_NOR = 5,
    BMP_COMBINE_XOR = 6,
    BMP_COMBINE_NXOR = 7
};

enum BmpReduce
{
    BMP_REDUCE_SIMPLE = 0,
    BMP_REDUCE_POPULAR = 1,
    BMP_REDUCE_MEDIAN = 2
};

enum BmpEmboss
{
    BMP_EMBOSS_TOPLEFT = 0,
    BMP_EMBOSS_TOP = 1,
    BMP_EMBOSS_TOPRIGHT = 2,
    BMP_EMBOSS_LEFT = 3,
    BMP_EMBOSS_MIDDLE = 4,
    BMP_EMBOSS_RIGHT = 5,
    BMP_EMBOSS_BOTTOMLEFT = 6,
    BMP_EMBOSS_BOTTOM = 7,
    BMP_EMBOSS_BOTTOMRIGHT = 8
};

enum BmpFilter
{
    BMP_FILTER_SMOOTH = 0,
    BMP_FILTER_SHARPEN = 1,
    BMP_FILTER_REMOVENOISE = 2,
    BMP_FILTER_SOBEL_GREY = 3,
    BMP_FILTER_EMBOSS_GREY = 4,
    BMP_FILTER_SOLARIZE = 5,
    BMP_FILTER_SEPIA = 6,
    BMP_FILTER_MOSAIC = 7,
    BMP_FILTER_POPART = 8,
    BMP_FILTER_DUOTONE = 9,

    BMP_FILTER_UNKNOWN = 65535
};

class VCL_DLLPUBLIC BmpFilterParam
{
public:

    BmpFilterParam( sal_uLong nProgressStart = 0, sal_uLong nProgressEnd = 0 ) :
        meFilter( BMP_FILTER_UNKNOWN ), mnProgressStart( nProgressStart ), mnProgressEnd( nProgressEnd ) {}

    BmpFilterParam( sal_uInt8 cSolarGreyThreshold, sal_uLong nProgressStart = 0, sal_uLong nProgressEnd = 0 ) :
        meFilter( BMP_FILTER_SOLARIZE ), mnProgressStart( nProgressStart ), mnProgressEnd( nProgressEnd ),
        mcSolarGreyThreshold( cSolarGreyThreshold ) {}

    BmpFilterParam( double nRadius, sal_uLong nProgressStart = 0, sal_uLong nProgressEnd = 0 ) :
        meFilter( BMP_FILTER_SMOOTH ), mnProgressStart( nProgressStart ), mnProgressEnd( nProgressEnd ),
        mnRadius( nRadius ) {}

    BmpFilterParam( sal_uInt16 nSepiaPercent, sal_uLong nProgressStart = 0, sal_uLong nProgressEnd = 0 ) :
        meFilter( BMP_FILTER_SEPIA ), mnProgressStart( nProgressStart ), mnProgressEnd( nProgressEnd ),
        mnSepiaPercent( nSepiaPercent )
        {
            assert(nSepiaPercent<=100);
        }

    BmpFilterParam( const Size& rMosaicTileSize, sal_uLong nProgressStart = 0, sal_uLong nProgressEnd = 0 ) :
        meFilter( BMP_FILTER_MOSAIC ), mnProgressStart( nProgressStart ), mnProgressEnd( nProgressEnd )
        {
            maMosaicTileSize.mnTileWidth = rMosaicTileSize.Width();
            maMosaicTileSize.mnTileHeight= rMosaicTileSize.Height();
        }
    BmpFilterParam( sal_uInt16 nEmbossAzimuthAngle100, sal_uInt16 nEmbossElevationAngle100,
                    sal_uLong nProgressStart = 0, sal_uLong nProgressEnd = 0 ) :
        meFilter( BMP_FILTER_EMBOSS_GREY ), mnProgressStart( nProgressStart ), mnProgressEnd( nProgressEnd )
        {
            maEmbossAngles.mnAzimuthAngle100 = nEmbossAzimuthAngle100;
            maEmbossAngles.mnElevationAngle100 = nEmbossElevationAngle100;
        }

private:
    friend class ::Bitmap;
    friend class BitmapEx;
    friend class Animation;

private:
    BmpFilter       meFilter;
    sal_uLong       mnProgressStart;
    sal_uLong       mnProgressEnd;

public:
    struct MosaicTileSize
    {
        sal_uLong   mnTileWidth;
        sal_uLong   mnTileHeight;
    };

    struct EmbossAngles
    {
        sal_uInt16  mnAzimuthAngle100;
        sal_uInt16  mnElevationAngle100;
    };

private:
    union
    {
        sal_uInt16  mnSepiaPercent;
        sal_uInt8   mcSolarGreyThreshold;
        double      mnRadius;

        MosaicTileSize maMosaicTileSize;
        EmbossAngles maEmbossAngles;
    };

};

class   BitmapInfoAccess;
class   BitmapReadAccess;
class   BitmapWriteAccess;
class   BitmapPalette;
class   ImpBitmap;
class   Color;
class   ResId;
class   GDIMetaFile;
class   AlphaMask;
class   OutputDevice;
class   SalBitmap;
namespace vcl
{
    class Kernel;
}

struct BitmapSystemData
{
    #if defined( WNT )
    void* pDIB; // device independent byte buffer
    void* pDDB; // if not NULL then this is actually an HBITMAP
    #elif defined( MACOSX ) || defined( IOS )
    void* rImageContext;     //Image context (CGContextRef)
    #else
    void* aPixmap;
    #endif
    int mnWidth;
    int mnHeight;
};

class VCL_DLLPUBLIC Bitmap
{
public:

                            Bitmap();
                            Bitmap( const Bitmap& rBitmap );
                            Bitmap( const Size& rSizePixel, sal_uInt16 nBitCount, const BitmapPalette* pPal = nullptr );
                            Bitmap( const ResId& rResId );
                            Bitmap( SalBitmap* pSalBitmap );
    virtual                 ~Bitmap();

    Bitmap&                 operator=( const Bitmap& rBitmap );
    inline bool             operator!() const;
    inline bool             operator==( const Bitmap& rBitmap ) const;
    inline bool             operator!=( const Bitmap& rBitmap ) const;

    inline bool             IsSameInstance( const Bitmap& rBmp ) const;
    bool                    IsEqual( const Bitmap& rBmp ) const;

    inline bool             IsEmpty() const;
    void                    SetEmpty();

    inline const MapMode&   GetPrefMapMode() const;
    inline void             SetPrefMapMode( const MapMode& rMapMode );

    inline const Size&      GetPrefSize() const;
    inline void             SetPrefSize( const Size& rSize );

    Size                    GetSizePixel() const;

    sal_uInt16              GetBitCount() const;
    inline sal_uLong        GetColorCount() const;
    inline sal_uLong        GetSizeBytes() const;
    bool                    HasGreyPalette() const;
    /** get system dependent bitmap data

        @param rData
        The system dependent BitmapSystemData structure to be filled

        @return true if the bitmap has a valid system object (e.g. not empty)
    */
    bool                    GetSystemData( BitmapSystemData& rData ) const;

    BitmapChecksum          GetChecksum() const;

    Bitmap                  CreateDisplayBitmap( OutputDevice* pDisplay );

    static const BitmapPalette&
                            GetGreyPalette( int nEntries );

public:

    bool                    MakeMono( sal_uInt8 cThreshold );


    /** Convert bitmap format

        @param eConversion
        The format this bitmap should be converted to.

        @return true, if the conversion was completed successfully.
     */
    bool                    Convert( BmpConversion eConversion );

    /** Reduce number of colors for the bitmap

        @param nNewColorCount
        Maximal number of bitmap colors after the reduce operation

        @param eReduce
        Algorithm to use for color reduction

        @return true, if the color reduction operation was completed successfully.
     */
    bool                    ReduceColors(
                                sal_uInt16 nNewColorCount,
                                BmpReduce eReduce = BMP_REDUCE_SIMPLE );

    /** Apply a dither algorithm to the bitmap

        This method dithers the bitmap inplace, i.e. a true color
        bitmap is converted to a paletted bitmap, reducing the color
        deviation by error diffusion.

        @param nDitherFlags
        The algorithm to be used for dithering
     */
    bool                    Dither( BmpDitherFlags nDitherFlags = BmpDitherFlags::Matrix );

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
    bool                    Crop( const Rectangle& rRectPixel );

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
                                const Rectangle& rRectDst,
                                const Rectangle& rRectSrc,
                                const Bitmap* pBmpSrc = nullptr );

    bool                    CopyPixel_AlphaOptimized(
                                const Rectangle& rRectDst,
                                const Rectangle& rRectSrc,
                                const Bitmap* pBmpSrc = nullptr );

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
    void                    AdaptBitCount(Bitmap& rNew) const;

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
    bool                    Rotate( long nAngle10, const Color& rFillColor );

    /** Create on-off mask from bitmap

        This method creates a bitmask from the bitmap, where every
        pixel that equals rTransColor is set transparent, the rest
        opaque.

        @param rTransColor
        Color value where the bitmask should be transparent

        @param nTol
        Tolerance value. Specifies the maximal difference between
        rTransColor and the individual pixel values, such that the
        corresponding pixel is still regarded transparent.

        @return the resulting bitmask.
     */
    Bitmap                  CreateMask( const Color& rTransColor, sal_uLong nTol = 0UL ) const;

    /** Create region of similar colors in a given rectangle

        @param rColor
        All pixel which have this color are included in the calculated region

        @param rRect
        The rectangle within which matching pixel are looked for. This
        rectangle is always clipped to the bitmap dimensions.

        @return the generated region.
     */
    vcl::Region                  CreateRegion( const Color& rColor, const Rectangle& rRect ) const;

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
    bool                    Replace( const Color& rSearchColor, const Color& rReplaceColor, sal_uLong nTol = 0 );

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
                                sal_uLong nColorCount,
                                sal_uLong* pTols = nullptr );

    /** Convert the bitmap to a PolyPolygon

        This works by putting continuous areas of the same color into
        a polygon, by tracing its bounding line.

        @param rPolyPoly
        The resulting PolyPolygon

        @param nFlags
        Whether the inline or the outline of the color areas should be
        represented by the polygon

        @return true, if the operation was completed successfully.
     */
    bool                    Vectorize(
                                tools::PolyPolygon& rPolyPoly,
                                BmpVectorizeFlags nFlags = BmpVectorizeFlags::Outer );

    /** Convert the bitmap to a meta file

        This works by putting continuous areas of the same color into
        polygons painted in this color, by tracing the area's bounding
        line.

        @param rMtf
        The resulting meta file

        @param cReduce
        If non-null, minimal size of bound rects for individual polygons. Smaller ones are ignored.

        @param nFlags
        Whether the inline or the outline of the color areas should be
        represented by the polygon

        @param pProgress
        A callback for showing the progress of the vectorization

        @return true, if the operation was completed successfully.
     */
    bool                    Vectorize(
                                GDIMetaFile& rMtf,
                                sal_uInt8 cReduce = 0,
                                BmpVectorizeFlags nFlags = BmpVectorizeFlags::Inner,
                                const Link<long,void>* pProgress = nullptr );

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
                                short nLuminancePercent = 0,
                                short nContrastPercent = 0,
                                short nChannelRPercent = 0,
                                short nChannelGPercent = 0,
                                short nChannelBPercent = 0,
                                double fGamma = 1.0,
                                bool bInvert = false,
                                bool msoBrightness = false );

    /** Apply specified filter to the bitmap

        @param eFilter
        The filter algorithm to apply

        @param pFilterParam
        Various parameter for the different bitmap filter algorithms

        @param pProgress
        A callback for showing the progress of the vectorization

        @return true, if the operation was completed successfully.
     */
    bool                    Filter(
                                BmpFilter eFilter,
                                const BmpFilterParam* pFilterParam = nullptr );

public:

    SAL_DLLPRIVATE void     ImplReleaseRef();
    SAL_DLLPRIVATE void     ImplMakeUnique();
    ImpBitmap*              ImplGetImpBitmap() const { return mpImpBmp;}
    SAL_DLLPRIVATE void     ImplSetImpBitmap( ImpBitmap* pImpBmp );
    SAL_DLLPRIVATE void     ImplAssignWithSize( const Bitmap& rBitmap );

    SAL_DLLPRIVATE void     ImplAdaptBitCount(Bitmap& rNew) const;
    SAL_DLLPRIVATE bool     ImplScaleFast( const double& rScaleX, const double& rScaleY );
    SAL_DLLPRIVATE bool     ImplScaleInterpolate( const double& rScaleX, const double& rScaleY );

    SAL_DLLPRIVATE bool     ImplConvolutionPass(
                                Bitmap& aNewBitmap,
                                BitmapReadAccess* pReadAcc,
                                int aNumberOfContributions,
                                double* pWeights,
                                int* pPixels,
                                int* pCount );

    SAL_DLLPRIVATE bool     ImplMakeMono( sal_uInt8 cThreshold );
    SAL_DLLPRIVATE bool     ImplMakeMonoDither();
    SAL_DLLPRIVATE bool     ImplMakeGreyscales( sal_uInt16 nGreyscales );
    SAL_DLLPRIVATE bool     ImplConvertUp( sal_uInt16 nBitCount, Color* pExtColor = nullptr );
    SAL_DLLPRIVATE bool     ImplConvertDown( sal_uInt16 nBitCount, Color* pExtColor = nullptr );
    SAL_DLLPRIVATE bool     ImplConvertGhosted();
    SAL_DLLPRIVATE bool     ImplDitherMatrix();
    SAL_DLLPRIVATE bool     ImplDitherFloyd();
    SAL_DLLPRIVATE bool     ImplDitherFloyd16();
    SAL_DLLPRIVATE bool     ImplReduceSimple( sal_uInt16 nColorCount );
    SAL_DLLPRIVATE bool     ImplReducePopular( sal_uInt16 nColorCount );
    SAL_DLLPRIVATE bool     ImplReduceMedian( sal_uInt16 nColorCount );
    SAL_DLLPRIVATE void     ImplMedianCut(
                                sal_uLong* pColBuf,
                                BitmapPalette& rPal,
                                long nR1, long nR2, long nG1, long nG2, long nB1, long nB2,
                                long nColors, long nPixels, long& rIndex );

    SAL_DLLPRIVATE bool     ImplConvolute3(
                                const long* pMatrix, long nDivisor );

    SAL_DLLPRIVATE bool     ImplMedianFilter();
    SAL_DLLPRIVATE bool     ImplSobelGrey();
    SAL_DLLPRIVATE bool     ImplEmbossGrey( const BmpFilterParam* pFilterParam );
    SAL_DLLPRIVATE bool     ImplSolarize( const BmpFilterParam* pFilterParam );
    SAL_DLLPRIVATE bool     ImplSepia( const BmpFilterParam* pFilterParam );
    SAL_DLLPRIVATE bool     ImplMosaic( const BmpFilterParam* pFilterParam );
    SAL_DLLPRIVATE bool     ImplPopArt();

    SAL_DLLPRIVATE bool     ImplSeparableBlurFilter( const double aRadius = 0.7 );
    SAL_DLLPRIVATE bool     ImplSeparableUnsharpenFilter( const double aRadius = 0.7 );
    SAL_DLLPRIVATE bool     ImplDuotoneFilter( const sal_uLong nColorOne,  sal_uLong nColorTwo );
    SAL_DLLPRIVATE void     ImplBlurContributions(
                                const int aSize,
                                const int aNumberOfContributions,
                                double* pBlurVector,
                                double*& pWeights,
                                int*& pPixels,
                                int*& pCount );

public:

    BitmapInfoAccess*       AcquireInfoAccess();
    BitmapReadAccess*       AcquireReadAccess();
    BitmapWriteAccess*      AcquireWriteAccess();
    static void             ReleaseAccess( BitmapInfoAccess* pAccess );

    typedef vcl::ScopedBitmapAccess<BitmapReadAccess, Bitmap, &Bitmap::AcquireReadAccess> ScopedReadAccess;
    typedef vcl::ScopedBitmapAccess<BitmapWriteAccess, Bitmap, &Bitmap::AcquireWriteAccess> ScopedWriteAccess;
    typedef vcl::ScopedBitmapAccess<BitmapInfoAccess, Bitmap, &Bitmap::AcquireInfoAccess> ScopedInfoAccess;

private:

    ImpBitmap*              mpImpBmp;
    MapMode                 maPrefMapMode;
    Size                    maPrefSize;

};

inline bool Bitmap::operator!() const
{
    return( mpImpBmp == nullptr );
}

inline bool Bitmap::operator==( const Bitmap& rBitmap ) const
{
    return( rBitmap.mpImpBmp == mpImpBmp );
}

inline bool Bitmap::operator!=( const Bitmap& rBitmap ) const
{
    return( rBitmap.mpImpBmp != mpImpBmp );
}

inline bool Bitmap::IsSameInstance( const Bitmap& rBitmap ) const
{
    return( rBitmap.mpImpBmp == mpImpBmp );
}

inline bool Bitmap::IsEmpty() const
{
    return( mpImpBmp == nullptr );
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

inline sal_uLong Bitmap::GetColorCount() const
{
    return( (sal_uLong)1 << (sal_uLong) GetBitCount() );
}

inline sal_uLong Bitmap::GetSizeBytes() const
{
    const Size aSizePix( GetSizePixel() );
    return( ( (sal_uLong) aSizePix.Width() * aSizePix.Height() * GetBitCount() ) >> 3UL );
}

#endif // INCLUDED_VCL_BITMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
