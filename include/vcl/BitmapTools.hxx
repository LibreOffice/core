/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_BITMAP_TOOLS_HXX
#define INCLUDED_VCL_BITMAP_TOOLS_HXX

#include <config_cairo_canvas.h>
#include <vcl/bitmapex.hxx>
#include <vcl/ImageTree.hxx>
#include <vcl/BitmapColor.hxx>
#if ENABLE_CAIRO_CANVAS
#include <vcl/cairo.hxx>
#endif
#include <basegfx/range/b2drectangle.hxx>
#include <o3tl/safeint.hxx>
#include <array>

class SvStream;
namespace basegfx { class B2DHomMatrix; }
namespace com { namespace sun { namespace star { namespace geometry { struct IntegerRectangle2D; } } } }

namespace vcl {
namespace bitmap {

typedef sal_uInt8 (*lookup_table)[256];

lookup_table VCL_DLLPUBLIC get_premultiply_table();
lookup_table VCL_DLLPUBLIC get_unpremultiply_table();

VCL_DLLPUBLIC sal_uInt8 unpremultiply(sal_uInt8 c, sal_uInt8 a);
VCL_DLLPUBLIC sal_uInt8 premultiply(sal_uInt8 c, sal_uInt8 a);

/**
 * Intended to be used to feed into CreateFromData to create a BitmapEx. RGB data format.
 */
class VCL_DLLPUBLIC RawBitmap
{
friend BitmapEx VCL_DLLPUBLIC CreateFromData( RawBitmap&& rawBitmap );
    std::unique_ptr<sal_uInt8[]> mpData;
    Size const maSize;
    sal_uInt8 const mnBitCount;
public:
    RawBitmap(Size const & rSize, sal_uInt8 nBitCount)
        : maSize(rSize),
          mnBitCount(nBitCount)
    {
        assert(nBitCount == 24 || nBitCount == 32);
        sal_Int32 nRowSize, nDataSize;
        if (o3tl::checked_multiply<sal_Int32>(rSize.getWidth(), nBitCount/8, nRowSize) ||
            o3tl::checked_multiply<sal_Int32>(nRowSize, rSize.getHeight(), nDataSize) ||
            nDataSize < 0)
        {
            throw std::bad_alloc();
        }
        mpData.reset(new sal_uInt8[nDataSize]);
    }
    void SetPixel(long nY, long nX, Color nColor)
    {
        long p = (nY * maSize.getWidth() + nX) * (mnBitCount/8);
        mpData[ p++ ] = nColor.GetRed();
        mpData[ p++ ] = nColor.GetGreen();
        mpData[ p++ ] = nColor.GetBlue();
        if (mnBitCount == 32)
            mpData[ p ] = nColor.GetTransparency();
    }
    Color GetPixel(long nY, long nX) const
    {
        long p = (nY * maSize.getWidth() + nX) * mnBitCount/8;
        if (mnBitCount == 24)
            return Color( mpData[p], mpData[p+1], mpData[p+2]);
        else
            return Color( mpData[p+3], mpData[p], mpData[p+1], mpData[p+2]);
    }
    // so we don't accidentally leave any code in that uses palette color indexes
    void SetPixel(long nY, long nX, BitmapColor nColor) = delete;
    long Height() { return maSize.Height(); }
    long Width() { return maSize.Width(); }
    sal_uInt8 GetBitCount() const { return mnBitCount; }
};

BitmapEx VCL_DLLPUBLIC loadFromName(const OUString& rFileName, const ImageLoadFlags eFlags = ImageLoadFlags::NONE);

void loadFromSvg(SvStream& rStream, const OUString& sPath, BitmapEx& rBitmapEx, double fScaleFactor);

/** Copy block of image data into the bitmap.
    Assumes that the Bitmap has been constructed with the desired size.

    @param pData
    The block of data to copy
    @param nStride
    The number of bytes in a scanline, must be >= (width * nBitCount / 8)
*/
BitmapEx VCL_DLLPUBLIC CreateFromData( sal_uInt8 const *pData, sal_Int32 nWidth, sal_Int32 nHeight, sal_Int32 nStride, sal_uInt16 nBitCount );

BitmapEx VCL_DLLPUBLIC CreateFromData( RawBitmap && data );

#if ENABLE_CAIRO_CANVAS
VCL_DLLPUBLIC BitmapEx* CreateFromCairoSurface(Size size, cairo_surface_t* pSurface);
#endif

VCL_DLLPUBLIC BitmapEx CanvasTransformBitmap( const BitmapEx& rBitmap,
                                  const ::basegfx::B2DHomMatrix&  rTransform,
                                  ::basegfx::B2DRectangle const & rDestRect,
                                  ::basegfx::B2DHomMatrix const & rLocalTransform );

VCL_DLLPUBLIC void DrawAlphaBitmapAndAlphaGradient(BitmapEx & rBitmapEx, bool bFixedTransparence, float fTransparence, AlphaMask & rNewMask);

VCL_DLLPUBLIC void DrawAndClipBitmap(const Point& rPos, const Size& rSize, const BitmapEx& rBitmap, BitmapEx & aBmpEx, basegfx::B2DPolyPolygon const & rClipPath);

VCL_DLLPUBLIC css::uno::Sequence< sal_Int8 > GetMaskDIB(BitmapEx const & aBmpEx);

/**
 * @param data will be filled with alpha data, if xBitmap is alpha/transparent image
 * @param bHasAlpha will be set to true if resulting surface has alpha
 **/
VCL_DLLPUBLIC void CanvasCairoExtractBitmapData( BitmapEx const & rBmpEx, Bitmap & rBitmap, unsigned char*& data, bool& bHasAlpha, long& rnWidth, long& rnHeight );

VCL_DLLPUBLIC css::uno::Sequence< sal_Int8 > CanvasExtractBitmapData(BitmapEx const & rBitmapEx, const css::geometry::IntegerRectangle2D& rect);

// helper to construct historical 8x8 bitmaps with two colors

BitmapEx VCL_DLLPUBLIC createHistorical8x8FromArray(std::array<sal_uInt8,64> const & pArray, Color aColorPix, Color aColorBack);
bool VCL_DLLPUBLIC isHistorical8x8(const BitmapEx& rBitmapEx, Color& o_rBack, Color& o_rFront);

VCL_DLLPUBLIC bool convertBitmap32To24Plus8(BitmapEx const & rInput, BitmapEx & rResult);

}} // end vcl::bitmap

#endif // INCLUDED_VCL_BITMAP_TOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
