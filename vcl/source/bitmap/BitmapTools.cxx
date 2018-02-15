/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/BitmapTools.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/seqstream.hxx>
#include <vcl/canvastools.hxx>

#include <com/sun/star/graphic/SvgTools.hpp>
#include <com/sun/star/graphic/Primitive2DTools.hpp>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

#include <com/sun/star/rendering/XIntegerReadOnlyBitmap.hpp>

#include <unotools/resmgr.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/bitmapaccess.hxx>
#if ENABLE_CAIRO_CANVAS
#include <cairo.h>
#endif

using namespace css;

using drawinglayer::primitive2d::Primitive2DSequence;
using drawinglayer::primitive2d::Primitive2DReference;

namespace vcl
{

namespace bitmap
{

BitmapEx loadFromName(const OUString& rFileName, const ImageLoadFlags eFlags)
{
    BitmapEx aBitmapEx;

    OUString aIconTheme = Application::GetSettings().GetStyleSettings().DetermineIconTheme();

    ImageTree::get().loadImage(rFileName, aIconTheme, aBitmapEx, true, eFlags);

    return aBitmapEx;
}

void loadFromSvg(SvStream& rStream, const OUString& sPath, BitmapEx& rBitmapEx, double fScalingFactor)
{
    uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());
    const uno::Reference<graphic::XSvgParser> xSvgParser = graphic::SvgTools::create(xContext);

    std::size_t nSize = rStream.remainingSize();
    std::vector<sal_Int8> aBuffer(nSize + 1);
    rStream.ReadBytes(aBuffer.data(), nSize);
    aBuffer[nSize] = 0;

    uno::Sequence<sal_Int8> aData(aBuffer.data(), nSize + 1);
    uno::Reference<io::XInputStream> aInputStream(new comphelper::SequenceInputStream(aData));

    Primitive2DSequence aPrimitiveSequence = xSvgParser->getDecomposition(aInputStream, sPath);

    if (!aPrimitiveSequence.hasElements())
        return;

    uno::Sequence<beans::PropertyValue> aViewParameters;

    geometry::RealRectangle2D aRealRect;
    basegfx::B2DRange aRange;
    for (Primitive2DReference const & xReference : aPrimitiveSequence)
    {
        if (xReference.is())
        {
            aRealRect = xReference->getRange(aViewParameters);
            aRange.expand(basegfx::B2DRange(aRealRect.X1, aRealRect.Y1, aRealRect.X2, aRealRect.Y2));
        }
    }

    aRealRect.X1 = aRange.getMinX();
    aRealRect.Y1 = aRange.getMinY();
    aRealRect.X2 = aRange.getMaxX();
    aRealRect.Y2 = aRange.getMaxY();

    double nDPI = 96 * fScalingFactor;

    const css::uno::Reference<css::graphic::XPrimitive2DRenderer> xPrimitive2DRenderer = css::graphic::Primitive2DTools::create(xContext);
    const css::uno::Reference<css::rendering::XBitmap> xBitmap(
        xPrimitive2DRenderer->rasterize(aPrimitiveSequence, aViewParameters, nDPI, nDPI, aRealRect, 256*256));

    if (xBitmap.is())
    {
        const css::uno::Reference<css::rendering::XIntegerReadOnlyBitmap> xIntBmp(xBitmap, uno::UNO_QUERY_THROW);

        if (xIntBmp.is())
        {
            rBitmapEx = vcl::unotools::bitmapExFromXBitmap(xIntBmp);
        }
    }

}

/** Copy block of image data into the bitmap.
    Assumes that the Bitmap has been constructed with the desired size.

    @param pData
    The block of data to copy
    @param nStride
    The number of bytes in a scanline, must >= (width * nBitCount / 8)
*/
BitmapEx CreateFromData( sal_uInt8 const *pData, sal_Int32 nWidth, sal_Int32 nHeight, sal_Int32 nStride, sal_uInt16 nBitCount )
{
    assert(nStride >= (nWidth * nBitCount / 8));
    assert( nBitCount == 1 || nBitCount == 24 || nBitCount == 32);
    Bitmap aBmp( Size( nWidth, nHeight ), nBitCount );

    Bitmap::ScopedWriteAccess pWrite(aBmp);
    assert(pWrite.get());
    if( !pWrite )
        return BitmapEx();
    std::unique_ptr<AlphaMask> pAlphaMask;
    AlphaMask::ScopedWriteAccess xMaskAcc;
    if (nBitCount == 32)
    {
        pAlphaMask.reset( new AlphaMask( Size(nWidth, nHeight) ) );
        xMaskAcc = AlphaMask::ScopedWriteAccess(*pAlphaMask);
    }
    if (nBitCount == 1)
    {
        for( long y = 0; y < nHeight; ++y )
        {
            Scanline pScanline = pWrite->GetScanline(y);
            for (long x = 0; x < nWidth; ++x)
            {
                sal_uInt8 const *p = pData + y * nStride / 8;
                int bitIndex = (y * nStride) % 8;
                pWrite->SetPixelOnData(pScanline, x, BitmapColor((*p >> bitIndex) & 1));
            }
        }
    }
    else
    {
        for( long y = 0; y < nHeight; ++y )
        {
            sal_uInt8 const *p = pData + y * nStride;
            Scanline pScanline = pWrite->GetScanline(y);
            for (long x = 0; x < nWidth; ++x)
            {
                BitmapColor col(p[0], p[1], p[2]);
                pWrite->SetPixelOnData(pScanline, x, col);
                p += nBitCount/8;
            }
            if (nBitCount == 32)
            {
                p = pData + y * nStride + 3;
                Scanline pMaskScanLine = xMaskAcc->GetScanline(y);
                for (long x = 0; x < nWidth; ++x)
                {
                    xMaskAcc->SetPixelOnData(pMaskScanLine, x, BitmapColor(*p));
                    p += 4;
                }
             }
        }
    }
    if (nBitCount == 32)
        return BitmapEx(aBmp, *pAlphaMask);
    else
        return aBmp;
}

/** Copy block of image data into the bitmap.
    Assumes that the Bitmap has been constructed with the desired size.
*/
BitmapEx CreateFromData( RawBitmap&& rawBitmap )
{
    Bitmap aBmp( rawBitmap.maSize, /*nBitCount*/24 );

    Bitmap::ScopedWriteAccess pWrite(aBmp);
    assert(pWrite.get());
    if( !pWrite )
        return BitmapEx();

    auto nHeight = rawBitmap.maSize.getHeight();
    auto nWidth = rawBitmap.maSize.getWidth();
    for( long y = 0; y < nHeight; ++y )
    {
        sal_uInt8 const *p = rawBitmap.mpData.get() + (y * nWidth * 3);
        Scanline pScanline = pWrite->GetScanline(y);
        for (long x = 0; x < nWidth; ++x)
        {
            BitmapColor col(p[0], p[1], p[2]);
            pWrite->SetPixelOnData(pScanline, x, col);
            p += 3;
        }
    }
    return aBmp;
}

#if ENABLE_CAIRO_CANVAS
BitmapEx* CreateFromCairoSurface(Size aSize, cairo_surface_t * pSurface)
{
    // FIXME: if we could teach VCL/ about cairo handles, life could
    // be significantly better here perhaps.
    cairo_surface_t *pPixels = cairo_image_surface_create( CAIRO_FORMAT_ARGB32,
                                              aSize.Width(), aSize.Height() );
    cairo_t *pCairo = cairo_create( pPixels );
    if( !pPixels || !pCairo || cairo_status(pCairo) != CAIRO_STATUS_SUCCESS )
        return nullptr;

    // suck ourselves from the X server to this buffer so then we can fiddle with
    // Alpha to turn it into the ultra-lame vcl required format and then push it
    // all back again later at vast expense [ urgh ]
    cairo_set_source_surface( pCairo, pSurface, 0, 0 );
    cairo_set_operator( pCairo, CAIRO_OPERATOR_SOURCE );
    cairo_paint( pCairo );

    ::Bitmap aRGB( aSize, 24 );
    ::AlphaMask aMask( aSize );

    Bitmap::ScopedWriteAccess pRGBWrite(aRGB);
    assert(pRGBWrite);
    if (!pRGBWrite)
        return nullptr;

    AlphaMask::ScopedWriteAccess pMaskWrite(aMask);
    assert(pMaskWrite);
    if (!pMaskWrite)
        return nullptr;

    cairo_surface_flush(pPixels);
    unsigned char *pSrc = cairo_image_surface_get_data( pPixels );
    unsigned int nStride = cairo_image_surface_get_stride( pPixels );
    for( unsigned long y = 0; y < static_cast<unsigned long>(aSize.Height()); y++ )
    {
        sal_uInt32 *pPix = reinterpret_cast<sal_uInt32 *>(pSrc + nStride * y);
        for( unsigned long x = 0; x < static_cast<unsigned long>(aSize.Width()); x++ )
        {
#if defined OSL_BIGENDIAN
            sal_uInt8 nB = (*pPix >> 24);
            sal_uInt8 nG = (*pPix >> 16) & 0xff;
            sal_uInt8 nR = (*pPix >> 8) & 0xff;
            sal_uInt8 nAlpha = *pPix & 0xff;
#else
            sal_uInt8 nAlpha = (*pPix >> 24);
            sal_uInt8 nR = (*pPix >> 16) & 0xff;
            sal_uInt8 nG = (*pPix >> 8) & 0xff;
            sal_uInt8 nB = *pPix & 0xff;
#endif
            if( nAlpha != 0 && nAlpha != 255 )
            {
                // Cairo uses pre-multiplied alpha - we do not => re-multiply
                nR = static_cast<sal_uInt8>(MinMax( (static_cast<sal_uInt32>(nR) * 255) / nAlpha, 0, 255 ));
                nG = static_cast<sal_uInt8>(MinMax( (static_cast<sal_uInt32>(nG) * 255) / nAlpha, 0, 255 ));
                nB = static_cast<sal_uInt8>(MinMax( (static_cast<sal_uInt32>(nB) * 255) / nAlpha, 0, 255 ));
            }
            pRGBWrite->SetPixel( y, x, BitmapColor( nR, nG, nB ) );
            pMaskWrite->SetPixelIndex( y, x, 255 - nAlpha );
            pPix++;
        }
    }

    // ignore potential errors above. will get caller a
    // uniformly white bitmap, but not that there would
    // be error handling in calling code ...
    ::BitmapEx *pBitmapEx = new ::BitmapEx( aRGB, aMask );

    cairo_destroy( pCairo );
    cairo_surface_destroy( pPixels );
    return pBitmapEx;
}
#endif

}} // end vcl::bitmap

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
