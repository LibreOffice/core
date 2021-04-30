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
#include <config_wasm_strip.h>

#include <vcl/bitmapex.hxx>
#include <vcl/ImageTree.hxx>
#if ENABLE_CAIRO_CANVAS
#include <vcl/cairo.hxx>
#endif
#include <basegfx/range/b2drectangle.hxx>
#include <array>
#include <vcl/RawBitmap.hxx>

class SvStream;
namespace basegfx { class B2DHomMatrix; }
namespace com::sun::star::geometry { struct IntegerRectangle2D; }

namespace vcl::bitmap {

#ifndef ENABLE_WASM_STRIP_PREMULTIPLY
typedef std::array<std::array<sal_uInt8, 256>, 256> lookup_table;

VCL_DLLPUBLIC lookup_table const & get_premultiply_table();
VCL_DLLPUBLIC lookup_table const & get_unpremultiply_table();
#endif

sal_uInt8 unpremultiply(sal_uInt8 c, sal_uInt8 a);
sal_uInt8 premultiply(sal_uInt8 c, sal_uInt8 a);

BitmapEx VCL_DLLPUBLIC loadFromName(const OUString& rFileName, const ImageLoadFlags eFlags = ImageLoadFlags::NONE);

void loadFromSvg(SvStream& rStream, const OUString& sPath, BitmapEx& rBitmapEx, double fScaleFactor);

/** Copy block of image data into the bitmap.
    Assumes that the Bitmap has been constructed with the desired size.

    @param pData
    The block of data to copy
    @param nStride
    The number of bytes in a scanline, must be >= (width * bitcount / 8)
*/
BitmapEx VCL_DLLPUBLIC CreateFromData(sal_uInt8 const *pData,
                                      sal_Int32 nWidth, sal_Int32 nHeight, sal_Int32 nStride,
                                      vcl::PixelFormat ePixelFormat,
                                      bool bReversColors = false, bool bReverseAlpha = false);

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
VCL_DLLPUBLIC void CanvasCairoExtractBitmapData( BitmapEx const & rBmpEx, Bitmap & rBitmap, unsigned char*& data, bool& bHasAlpha, tools::Long& rnWidth, tools::Long& rnHeight );

VCL_DLLPUBLIC css::uno::Sequence< sal_Int8 > CanvasExtractBitmapData(BitmapEx const & rBitmapEx, const css::geometry::IntegerRectangle2D& rect);

// helper to construct historical 8x8 bitmaps with two colors

BitmapEx VCL_DLLPUBLIC createHistorical8x8FromArray(std::array<sal_uInt8,64> const & pArray, Color aColorPix, Color aColorBack);
bool VCL_DLLPUBLIC isHistorical8x8(const BitmapEx& rBitmapEx, Color& o_rBack, Color& o_rFront);

VCL_DLLPUBLIC bool convertBitmap32To24Plus8(BitmapEx const & rInput, BitmapEx & rResult);

/** Retrieve downsampled and cropped bitmap

    Takes destination size in twips units.

    @attention This method ignores negative rDstSz values, thus
    mirroring must happen outside this method (e.g. in DrawBitmap)
 */
VCL_DLLPUBLIC Bitmap GetDownsampledBitmap(Size const& rDstSizeTwip, Point const& rSrcPt, Size const& rSrcSz,
                            Bitmap const& rBmp, tools::Long nMaxBmpDPIX, tools::Long nMaxBmpDPIY);

} // end vcl::bitmap

#endif // INCLUDED_VCL_BITMAP_TOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
