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

#ifndef _SV_BITMAP_HXX
#define _SV_BITMAP_HXX

#include <tools/link.hxx>
#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/mapmod.hxx>
#include <vcl/region.hxx>
#include <vcl/scopedbitmapaccess.hxx>

// -----------
// - Defines -
// -----------

#define BMP_MIRROR_NONE             0x00000000UL
#define BMP_MIRROR_HORZ             0x00000001UL
#define BMP_MIRROR_VERT             0x00000002UL

// -----------------------------------------------------------------------------

#define BMP_SCALE_NONE              0x00000000UL
#define BMP_SCALE_FAST              0x00000001UL
#define BMP_SCALE_INTERPOLATE       0x00000002UL
#define BMP_SCALE_LANCZOS           0x00000003UL
#define BMP_SCALE_BICUBIC           0x00000004UL
#define BMP_SCALE_BILINEAR          0x00000005UL
#define BMP_SCALE_BOX               0x00000006UL

// Aliases, try to use these two (or BMP_SCALE_FAST/BMP_SCALE_NONE),
// use a specific algorithm only if you really need to.
#define BMP_SCALE_BEST              BMP_SCALE_LANCZOS
#define BMP_SCALE_DEFAULT           BMP_SCALE_BOX

// -----------------------------------------------------------------------------

#define BMP_DITHER_NONE             0x00000000UL
#define BMP_DITHER_MATRIX           0x00000001UL
#define BMP_DITHER_FLOYD            0x00000002UL
#define BMP_DITHER_FLOYD_16         0x00000004UL

// -----------------------------------------------------------------------------

#define BMP_VECTORIZE_NONE          BMP_VECTORIZE_OUTER
#define BMP_VECTORIZE_INNER         0x00000001UL
#define BMP_VECTORIZE_OUTER         0x00000002UL
#define BMP_VECTORIZE_BOUND_ONLY    0x00000004UL
#define BMP_VECTORIZE_REDUCE_EDGES  0x00000008UL

// -----------------------------------------------------------------------------

#define BMP_COL_TRANS                   Color( 252, 3, 251 )

// ---------
// - Enums -
// ---------

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

// ------------------------------------------------------------------------

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

// ------------------------------------------------------------------------

enum BmpReduce
{
    BMP_REDUCE_SIMPLE = 0,
    BMP_REDUCE_POPULAR = 1,
    BMP_REDUCE_MEDIAN = 2
};

// ------------------------------------------------------------------------

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

// ------------------------------------------------------------------------

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

    BMP_FILTER_UNKNOWN = 65535
};

// ------------------------------------------------------------------------

// --------------------
// - FilterParameters -
// --------------------

class VCL_DLLPUBLIC BmpFilterParam
{
    friend class Bitmap;
    friend class BitmapEx;
    friend class Animation;

private:
    BmpFilter       meFilter;
    sal_uLong       mnProgressStart;
    sal_uLong       mnProgressEnd;

public:
    struct MosaicTileSize
    {
        sal_uLong mnTileWidth;
        sal_uLong mnTileHeight;
    };

    struct EmbossAngles
    {
        sal_uInt16 mnAzimuthAngle100;
        sal_uInt16 mnElevationAngle100;
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
        mnSepiaPercent( nSepiaPercent ) {}

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
};

// --------------------
// Resample Kernels
// --------------------

class Kernel
{

public:
    Kernel () {}
    virtual ~Kernel() {}

    virtual double GetWidth() = 0;
    virtual double Calculate( double x ) = 0;
};

class Lanczos3Kernel : public Kernel
{

public:
    virtual double GetWidth() { return 3.0; }
    virtual double Calculate (double x)
    {
        return (-3.0 <= x && x < 3.0) ? SincFilter(x) * SincFilter( x / 3.0 ) : 0.0;
    }

    inline double SincFilter(double x)
    {
        if (x == 0.0)
        {
            return 1.0;
        }
        x = x * M_PI;
        return sin(x) / x;
    }
};

class BicubicKernel : public Kernel {
    virtual double GetWidth() { return 2.0; }
    virtual double Calculate (double x)
    {
        if (x < 0.0)
        {
            x = -x;
        }

        if (x <= 1.0)
        {
            return (1.5 * x - 2.5) * x * x + 1.0;
        }
        else if (x < 2.0)
        {
            return ((-0.5 * x + 2.5) * x - 4) * x + 2;
        }
        return 0.0;
    }
};

class BilinearKernel : public Kernel {
    virtual double GetWidth() { return 1.0; }
    virtual double Calculate (double x)
    {
        if (x < 0.0)
        {
            x = -x;
        }
        if (x < 1.0)
        {
            return 1.0-x;
        }
        return 0.0;
    }
};

class BoxKernel : public Kernel {
    virtual double GetWidth() { return 0.5; }
    virtual double Calculate (double x)
    {
        if (-0.5 <= x && x < 0.5)
            return 1.0;
        return 0.0;
    }
};

// ----------
// - Bitmap -
// ----------

class   BitmapReadAccess;
class   BitmapWriteAccess;
class   BitmapPalette;
class   ImpBitmap;
class   Color;
class   SvStream;
struct  DIBInfoHeader;
class   ResId;
class   GDIMetaFile;
class   AlphaMask;
class   OutputDevice;
class   SalBitmap;

struct BitmapSystemData
{
    #if defined( WNT )
    void* pDIB; // device independent byte buffer
    void* pDDB; // if not NULL then this is actually an HBITMAP
    #elif defined( QUARTZ ) || defined( IOS )
    void* rImageContext;     //Image context (CGContextRef)
    #else
    void* aPixmap;
    #endif
    int mnWidth;
    int mnHeight;
};

class VCL_DLLPUBLIC Bitmap
{
private:

    ImpBitmap*          mpImpBmp;
    MapMode             maPrefMapMode;
    Size                maPrefSize;

public:

    SAL_DLLPRIVATE void                 ImplReleaseRef();
    SAL_DLLPRIVATE void                 ImplMakeUnique();
                   ImpBitmap*           ImplGetImpBitmap() const;
    SAL_DLLPRIVATE void                 ImplSetImpBitmap( ImpBitmap* pImpBmp );
    SAL_DLLPRIVATE void                 ImplAssignWithSize( const Bitmap& rBitmap );

    SAL_DLLPRIVATE static sal_Bool      ImplReadDIB( SvStream& rIStm, Bitmap& rBmp, sal_uLong nOffset, sal_Bool bMSOFormat = sal_False );
    SAL_DLLPRIVATE static sal_Bool      ImplReadDIBFileHeader( SvStream& rIStm, sal_uLong& rOffset );
    SAL_DLLPRIVATE static sal_Bool      ImplReadDIBInfoHeader( SvStream& rIStm, DIBInfoHeader& rHeader, sal_Bool& bTopDown, sal_Bool bMSOFormat = sal_False );
    SAL_DLLPRIVATE static sal_Bool      ImplReadDIBPalette( SvStream& rIStm, BitmapWriteAccess& rAcc, sal_Bool bQuad );
    SAL_DLLPRIVATE static sal_Bool      ImplReadDIBBits( SvStream& rIStm, DIBInfoHeader& rHeader, BitmapWriteAccess& rAcc, sal_Bool bTopDown );
    SAL_DLLPRIVATE sal_Bool             ImplWriteDIB( SvStream& rOStm, BitmapReadAccess& rAcc, sal_Bool bCompressed ) const;
    SAL_DLLPRIVATE static sal_Bool      ImplWriteDIBFileHeader( SvStream& rOStm, BitmapReadAccess& rAcc );
    SAL_DLLPRIVATE static sal_Bool      ImplWriteDIBPalette( SvStream& rOStm, BitmapReadAccess& rAcc );
    SAL_DLLPRIVATE static sal_Bool      ImplWriteDIBBits( SvStream& rOStm, BitmapReadAccess& rAcc,
                                                            sal_uLong nCompression, sal_uInt32& rImageSize );
    SAL_DLLPRIVATE static void          ImplDecodeRLE( sal_uInt8* pBuffer, DIBInfoHeader& rHeader,
                                                            BitmapWriteAccess& rAcc, sal_Bool bRLE4 );
    SAL_DLLPRIVATE static sal_Bool      ImplWriteRLE( SvStream& rOStm, BitmapReadAccess& rAcc, sal_Bool bRLE4 );

    SAL_DLLPRIVATE sal_Bool             ImplScaleFast( const double& rScaleX, const double& rScaleY );
    SAL_DLLPRIVATE sal_Bool             ImplScaleInterpolate( const double& rScaleX, const double& rScaleY );
    SAL_DLLPRIVATE bool                 ImplScaleConvolution( const double& rScaleX, const double& rScaleY, Kernel& aKernel);
    SAL_DLLPRIVATE bool                 ImplTransformAveraging( const double& rScaleX, const double& rScaleY,
                                                const Rectangle& rRotatedRectangle, const long nAngle10, const Color& rFillColor );
    SAL_DLLPRIVATE bool                 ImplTransformBilinearFiltering( const double& rScaleX, const double& rScaleY,
                                                const Rectangle& rRotatedRectangle, const long nAngle10, const Color& rFillColor );

    SAL_DLLPRIVATE static void          ImplCalculateContributions( const int aSourceSize, const int aDestinationSize,
                                                int& aNumberOfContributions, double*& pWeights, int*& pPixels, int*& pCount,
                                                Kernel& aKernel );

    SAL_DLLPRIVATE bool                 ImplConvolutionPass( Bitmap& aNewBitmap, const int nNewSize, BitmapReadAccess* pReadAcc,
                                                int aNumberOfContributions, double* pWeights, int* pPixels, int* pCount );

    SAL_DLLPRIVATE sal_Bool             ImplMakeMono( sal_uInt8 cThreshold );
    SAL_DLLPRIVATE sal_Bool             ImplMakeMonoDither();
    SAL_DLLPRIVATE sal_Bool             ImplMakeGreyscales( sal_uInt16 nGreyscales );
    SAL_DLLPRIVATE sal_Bool             ImplConvertUp( sal_uInt16 nBitCount, Color* pExtColor = NULL );
    SAL_DLLPRIVATE sal_Bool             ImplConvertDown( sal_uInt16 nBitCount, Color* pExtColor = NULL );
    SAL_DLLPRIVATE sal_Bool             ImplConvertGhosted();
    SAL_DLLPRIVATE sal_Bool             ImplDitherMatrix();
    SAL_DLLPRIVATE sal_Bool             ImplDitherFloyd();
    SAL_DLLPRIVATE sal_Bool             ImplDitherFloyd16();
    SAL_DLLPRIVATE sal_Bool             ImplReduceSimple( sal_uInt16 nColorCount );
    SAL_DLLPRIVATE sal_Bool             ImplReducePopular( sal_uInt16 nColorCount );
    SAL_DLLPRIVATE sal_Bool             ImplReduceMedian( sal_uInt16 nColorCount );
    SAL_DLLPRIVATE void                 ImplMedianCut( sal_uLong* pColBuf, BitmapPalette& rPal,
                                                long nR1, long nR2, long nG1, long nG2, long nB1, long nB2,
                                                long nColors, long nPixels, long& rIndex );
    SAL_DLLPRIVATE sal_Bool             ImplConvolute3( const long* pMatrix, long nDivisor,
                                                            const BmpFilterParam* pFilterParam, const Link* pProgress );
    SAL_DLLPRIVATE sal_Bool             ImplMedianFilter( const BmpFilterParam* pFilterParam, const Link* pProgress );
    SAL_DLLPRIVATE sal_Bool             ImplSobelGrey( const BmpFilterParam* pFilterParam, const Link* pProgress );
    SAL_DLLPRIVATE sal_Bool             ImplEmbossGrey( const BmpFilterParam* pFilterParam, const Link* pProgress );
    SAL_DLLPRIVATE sal_Bool             ImplSolarize( const BmpFilterParam* pFilterParam, const Link* pProgress );
    SAL_DLLPRIVATE sal_Bool             ImplSepia( const BmpFilterParam* pFilterParam, const Link* pProgress );
    SAL_DLLPRIVATE sal_Bool             ImplMosaic( const BmpFilterParam* pFilterParam, const Link* pProgress );
    SAL_DLLPRIVATE sal_Bool             ImplPopArt( const BmpFilterParam* pFilterParam, const Link* pProgress );

    SAL_DLLPRIVATE bool                 ImplSeparableBlurFilter( const double aRadius = 0.7 );
    SAL_DLLPRIVATE bool                 ImplSeparableUnsharpenFilter( const double aRadius = 0.7 );
    SAL_DLLPRIVATE void                 ImplBlurContributions( const int aSize, const int aNumberOfContributions,
                                                double* pBlurVector, double*& pWeights, int*& pPixels, int*& pCount );
public:

    Bitmap();
    Bitmap( const Bitmap& rBitmap );
    Bitmap( const Size& rSizePixel, sal_uInt16 nBitCount, const BitmapPalette* pPal = NULL );
    Bitmap( const ResId& rResId );
    Bitmap( SalBitmap* pSalBitmap );
    ~Bitmap();

    Bitmap&                 operator=( const Bitmap& rBitmap );
    inline sal_Bool             operator!() const;
    inline sal_Bool             operator==( const Bitmap& rBitmap ) const;
    inline sal_Bool             operator!=( const Bitmap& rBitmap ) const;

    inline sal_Bool             IsSameInstance( const Bitmap& rBmp ) const;
    sal_Bool                    IsEqual( const Bitmap& rBmp ) const;

    inline sal_Bool             IsEmpty() const;
    void                    SetEmpty();

    inline const MapMode&   GetPrefMapMode() const;
    inline void             SetPrefMapMode( const MapMode& rMapMode );

    inline const Size&      GetPrefSize() const;
    inline void             SetPrefSize( const Size& rSize );

    Size                    GetSizePixel() const;

    /**
     * The pixel size of a bitmap's source (e.g. an image file)
     * and the pixel size of its resulting bitmap can differ,
     * e.g. when the image reader has its preview mode enabled.
     */
    void                    SetSourceSizePixel( const Size& );

    sal_uInt16                  GetBitCount() const;
    inline sal_uLong            GetColorCount() const;
    inline sal_uLong            GetSizeBytes() const;
    sal_Bool                    HasGreyPalette() const;
    /** get system dependent bitmap data

        @param rData
        The system dependent BitmapSystemData structure to be filled

        @return sal_True if the bitmap has a valid system object (e.g. not empty)
    */
    bool                    GetSystemData( BitmapSystemData& rData ) const;

    sal_uLong                   GetChecksum() const;

    Bitmap                  CreateDisplayBitmap( OutputDevice* pDisplay );
    Bitmap                  GetColorTransformedBitmap() const;

    static const BitmapPalette& GetGreyPalette( int nEntries );

public:

    sal_Bool MakeMono( sal_uInt8 cThreshold );


    /** Convert bitmap format

        @param eConversion
        The format this bitmap should be converted to.

        @return sal_True, if the conversion was completed successfully.
     */
    sal_Bool                    Convert( BmpConversion eConversion );

    /** Reduce number of colors for the bitmap

        @param nNewColorCount
        Maximal number of bitmap colors after the reduce operation

        @param eReduce
        Algorithm to use for color reduction

        @return sal_True, if the color reduction operation was completed successfully.
     */
    sal_Bool                    ReduceColors( sal_uInt16 nNewColorCount,
                                          BmpReduce eReduce = BMP_REDUCE_SIMPLE );

    /** Apply a dither algorithm to the bitmap

        This method dithers the bitmap inplace, i.e. a true color
        bitmap is converted to a paletted bitmap, reducing the color
        deviation by error diffusion.

        @param nDitherFlags
        The algorithm to be used for dithering

        @param pDitherPal
        A custom palette to be used when dithering (not yet implemented, leave NULL)
     */
    sal_Bool                    Dither( sal_uLong nDitherFlags = BMP_DITHER_MATRIX );

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
    sal_Bool                    Crop( const Rectangle& rRectPixel );

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
    sal_Bool                    Expand( sal_uLong nDX, sal_uLong nDY,
                                    const Color* pInitColor = NULL );

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
    sal_Bool                    CopyPixel( const Rectangle& rRectDst,
                                       const Rectangle& rRectSrc,
                                       const Bitmap* pBmpSrc = NULL );

    /** Perform boolean operations with another bitmap

        @param rMask
        The mask bitmap in the selected combine operation

        @param eCombine
        The combine operation to perform on the bitmap

        @return sal_True, if the operation was completed successfully.
     */
    sal_Bool                    CombineSimple( const Bitmap& rMask,
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

        @return sal_True, if blending was successful, sal_False otherwise
     */
    sal_Bool                    Blend( const AlphaMask& rAlpha,
                                   const Color&     rBackgroundColor );

    /** Fill the entire bitmap with the given color

        @param rFillColor
        Color value to use for filling

        @return sal_True, if the operation was completed successfully.
     */
    sal_Bool                    Erase( const Color& rFillColor );

    /** Perform the Invert operation on every pixel

        @return sal_True, if the operation was completed successfully.
     */
    sal_Bool                    Invert();

    /** Mirror the bitmap

        @param nMirrorFlags
        About which axis (horizontal, vertical, or both) to mirror

        @return sal_True, if the operation was completed successfully.
     */
    sal_Bool                    Mirror( sal_uLong nMirrorFlags );

    /** Scale the bitmap

        @param rNewSize
        The resulting size of the scaled bitmap

        @param nScaleFlag
        The algorithm to be used for scaling

        @return sal_True, if the operation was completed successfully.
     */
    sal_Bool                    Scale( const Size& rNewSize, sal_uLong nScaleFlag = BMP_SCALE_DEFAULT );

    /** Scale the bitmap

        @param rScaleX
        The scale factor in x direction.

        @param rScaleY
        The scale factor in y direction.

        @return sal_True, if the operation was completed successfully.
     */
    sal_Bool                    Scale( const double& rScaleX, const double& rScaleY, sal_uLong nScaleFlag = BMP_SCALE_DEFAULT );

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
    sal_Bool                    Rotate( long nAngle10, const Color& rFillColor );

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
    Region                  CreateRegion( const Color& rColor, const Rectangle& rRect ) const;

    /** Replace all pixel where the given mask is on with the specified color

        @param rMask
        Mask specifying which pixel should be replaced

        @param rReplaceColor
        Color to be placed in all changed pixel

        @return sal_True, if the operation was completed successfully.
     */
    sal_Bool                    Replace( const Bitmap& rMask, const Color& rReplaceColor );

    /** Merge bitmap with given background color according to specified alpha mask

        @param rAlpha
        Alpha mask specifying the amount of background color to merge in

        @param rMergeColor
        Background color to be used for merging

        @return sal_True, if the operation was completed successfully.
     */
    sal_Bool                    Replace( const AlphaMask& rAlpha, const Color& rMergeColor );

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
    sal_Bool                    Replace( const Color& rSearchColor, const Color& rReplaceColor, sal_uLong nTol = 0 );

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
    sal_Bool                    Replace( const Color* pSearchColors, const Color* rReplaceColors,
                                     sal_uLong nColorCount, sal_uLong* pTols = NULL );

    /** Convert the bitmap to a PolyPolygon

        This works by putting continuous areas of the same color into
        a polygon, by tracing its bounding line.

        @param rPolyPoly
        The resulting PolyPolygon

        @param nFlags
        Whether the inline or the outline of the color areas should be
        represented by the polygon

        @param pProgress
        A callback for showing the progress of the vectorization

        @return sal_True, if the operation was completed successfully.
     */
    sal_Bool                    Vectorize( PolyPolygon& rPolyPoly,
                                       sal_uLong nFlags = BMP_VECTORIZE_OUTER,
                                       const Link* pProgress = NULL );

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

        @return sal_True, if the operation was completed successfully.
     */
    sal_Bool                    Vectorize( GDIMetaFile& rMtf, sal_uInt8 cReduce = 0,
                                       sal_uLong nFlags = BMP_VECTORIZE_INNER,
                                       const Link* pProgress = NULL );

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
    sal_Bool                    Adjust( short nLuminancePercent = 0,
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
    sal_Bool                    Filter( BmpFilter eFilter,
                                    const BmpFilterParam* pFilterParam = NULL,
                                    const Link* pProgress = NULL );

public:
    BitmapReadAccess*       AcquireReadAccess();
    BitmapWriteAccess*      AcquireWriteAccess();
    void                    ReleaseAccess( BitmapReadAccess* pAccess );

    typedef vcl::ScopedBitmapAccess< BitmapReadAccess, Bitmap, &Bitmap::AcquireReadAccess >
        ScopedReadAccess;
    typedef vcl::ScopedBitmapAccess< BitmapWriteAccess, Bitmap, &Bitmap::AcquireWriteAccess >
        ScopedWriteAccess;

public:

    sal_Bool                    Read( SvStream& rIStm, sal_Bool bFileHeader = sal_True, sal_Bool bMSOFormat = sal_False );
    sal_Bool                    Write( SvStream& rOStm, sal_Bool bCompressed = sal_True, sal_Bool bFileHeader = sal_True ) const;

    friend VCL_DLLPUBLIC SvStream&        operator>>( SvStream& rIStm, Bitmap& rBitmap );
    friend VCL_DLLPUBLIC SvStream&        operator<<( SvStream& rOStm, const Bitmap& rBitmap );
};

// -----------
// - Inlines -
// -----------

inline sal_Bool Bitmap::operator!() const
{
    return( mpImpBmp == NULL );
}

// ------------------------------------------------------------------

inline sal_Bool Bitmap::operator==( const Bitmap& rBitmap ) const
{
    return( rBitmap.mpImpBmp == mpImpBmp );
}

// ------------------------------------------------------------------

inline sal_Bool Bitmap::operator!=( const Bitmap& rBitmap ) const
{
    return( rBitmap.mpImpBmp != mpImpBmp );
}

// ------------------------------------------------------------------

inline sal_Bool Bitmap::IsSameInstance( const Bitmap& rBitmap ) const
{
    return( rBitmap.mpImpBmp == mpImpBmp );
}

// ------------------------------------------------------------------

inline sal_Bool Bitmap::IsEmpty() const
{
    return( mpImpBmp == NULL );
}

// ------------------------------------------------------------------

inline const MapMode& Bitmap::GetPrefMapMode() const
{
    return maPrefMapMode;
}

// ------------------------------------------------------------------

inline void Bitmap::SetPrefMapMode( const MapMode& rMapMode )
{
    maPrefMapMode = rMapMode;
}

// ------------------------------------------------------------------

inline const Size& Bitmap::GetPrefSize() const
{
    return maPrefSize;
}

// ------------------------------------------------------------------

inline void Bitmap::SetPrefSize( const Size& rSize )
{
    maPrefSize = rSize;
}

// ------------------------------------------------------------------

inline sal_uLong Bitmap::GetColorCount() const
{
    return( 1UL << (sal_uLong) GetBitCount() );
}

// ------------------------------------------------------------------

inline sal_uLong Bitmap::GetSizeBytes() const
{
    const Size aSizePix( GetSizePixel() );
    return( ( (sal_uLong) aSizePix.Width() * aSizePix.Height() * GetBitCount() ) >> 3UL );
}

#endif // _SV_BITMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
