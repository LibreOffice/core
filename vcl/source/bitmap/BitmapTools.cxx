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
#include <tools/diagnose_ex.h>

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
            sal_uInt8 const *p = pData + (y * nStride);
            Scanline pScanline = pWrite->GetScanline(y);
            for (long x = 0; x < nWidth; ++x)
            {
                BitmapColor col(p[0], p[1], p[2]);
                pWrite->SetPixelOnData(pScanline, x, col);
                p += nBitCount/8;
            }
            if (nBitCount == 32)
            {
                p = pData + (y * nStride) + 3;
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
    auto nBitCount = rawBitmap.GetBitCount();
    assert( nBitCount == 24 || nBitCount == 32);
    Bitmap aBmp( rawBitmap.maSize, nBitCount );

    Bitmap::ScopedWriteAccess pWrite(aBmp);
    assert(pWrite.get());
    if( !pWrite )
        return BitmapEx();
    std::unique_ptr<AlphaMask> pAlphaMask;
    AlphaMask::ScopedWriteAccess xMaskAcc;
    if (nBitCount == 32)
    {
        pAlphaMask.reset( new AlphaMask( rawBitmap.maSize ) );
        xMaskAcc = AlphaMask::ScopedWriteAccess(*pAlphaMask);
    }

    auto nHeight = rawBitmap.maSize.getHeight();
    auto nWidth = rawBitmap.maSize.getWidth();
    auto nStride = nWidth * nBitCount / 8;
    for( long y = 0; y < nHeight; ++y )
    {
        sal_uInt8 const *p = rawBitmap.mpData.get() + (y * nStride);
        Scanline pScanline = pWrite->GetScanline(y);
        for (long x = 0; x < nWidth; ++x)
        {
            BitmapColor col(p[0], p[1], p[2]);
            pWrite->SetPixelOnData(pScanline, x, col);
            p += nBitCount/8;
        }
        if (nBitCount == 32)
        {
            p = rawBitmap.mpData.get() + (y * nStride) + 3;
            Scanline pMaskScanLine = xMaskAcc->GetScanline(y);
            for (long x = 0; x < nWidth; ++x)
            {
                xMaskAcc->SetPixelOnData(pMaskScanLine, x, BitmapColor(*p));
                p += 4;
            }
        }
    }
    if (nBitCount == 32)
        return BitmapEx(aBmp, *pAlphaMask);
    else
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

BitmapEx CanvasTransformBitmap( const BitmapEx&                 rBitmap,
                                const ::basegfx::B2DHomMatrix&  rTransform,
                                ::basegfx::B2DRectangle const & rDestRect,
                                ::basegfx::B2DHomMatrix const & rLocalTransform )
{
    bool bCopyBack( false );
    const Size aBmpSize( rBitmap.GetSizePixel() );
    Bitmap aSrcBitmap( rBitmap.GetBitmap() );
    Bitmap aSrcAlpha;

    // differentiate mask and alpha channel (on-off
    // vs. multi-level transparency)
    if( rBitmap.IsTransparent() )
    {
        if( rBitmap.IsAlpha() )
            aSrcAlpha = rBitmap.GetAlpha().GetBitmap();
        else
            aSrcAlpha = rBitmap.GetMask();
    }

    Bitmap::ScopedReadAccess pReadAccess( aSrcBitmap );
    Bitmap::ScopedReadAccess pAlphaReadAccess( rBitmap.IsTransparent() ?
                                             aSrcAlpha.AcquireReadAccess() :
                                             nullptr,
                                             aSrcAlpha );

    if( pReadAccess.get() == nullptr ||
        (pAlphaReadAccess.get() == nullptr && rBitmap.IsTransparent()) )
    {
        // TODO(E2): Error handling!
        ENSURE_OR_THROW( false,
                          "transformBitmap(): could not access source bitmap" );
    }

    // mapping table, to translate pAlphaReadAccess' pixel
    // values into destination alpha values (needed e.g. for
    // paletted 1-bit masks).
    sal_uInt8 aAlphaMap[256];

    if( rBitmap.IsTransparent() )
    {
        if( rBitmap.IsAlpha() )
        {
            // source already has alpha channel - 1:1 mapping,
            // i.e. aAlphaMap[0]=0,...,aAlphaMap[255]=255.
            sal_uInt8  val=0;
            sal_uInt8* pCur=aAlphaMap;
            sal_uInt8* const pEnd=&aAlphaMap[256];
            while(pCur != pEnd)
                *pCur++ = val++;
        }
        else
        {
            // mask transparency - determine used palette colors
            const BitmapColor& rCol0( pAlphaReadAccess->GetPaletteColor( 0 ) );
            const BitmapColor& rCol1( pAlphaReadAccess->GetPaletteColor( 1 ) );

            // shortcut for true luminance calculation
            // (assumes that palette is grey-level)
            aAlphaMap[0] = rCol0.GetRed();
            aAlphaMap[1] = rCol1.GetRed();
        }
    }
    // else: mapping table is not used

    const Size aDestBmpSize( ::basegfx::fround( rDestRect.getWidth() ),
                             ::basegfx::fround( rDestRect.getHeight() ) );

    if( aDestBmpSize.Width() == 0 || aDestBmpSize.Height() == 0 )
        return BitmapEx();

    Bitmap aDstBitmap( aDestBmpSize, aSrcBitmap.GetBitCount(), &pReadAccess->GetPalette() );
    Bitmap aDstAlpha( AlphaMask( aDestBmpSize ).GetBitmap() );

    {
        // just to be on the safe side: let the
        // ScopedAccessors get destructed before
        // copy-constructing the resulting bitmap. This will
        // rule out the possibility that cached accessor data
        // is not yet written back.
        Bitmap::ScopedWriteAccess pWriteAccess( aDstBitmap );
        Bitmap::ScopedWriteAccess pAlphaWriteAccess( aDstAlpha );


        if( pWriteAccess.get() != nullptr &&
            pAlphaWriteAccess.get() != nullptr &&
            rTransform.isInvertible() )
        {
            // we're doing inverse mapping here, i.e. mapping
            // points from the destination bitmap back to the
            // source
            ::basegfx::B2DHomMatrix aTransform( rLocalTransform );
            aTransform.invert();

            // for the time being, always read as ARGB
            for( long y=0; y<aDestBmpSize.Height(); ++y )
            {
                // differentiate mask and alpha channel (on-off
                // vs. multi-level transparency)
                if( rBitmap.IsTransparent() )
                {
                    Scanline pScan = pWriteAccess->GetScanline( y );
                    Scanline pScanAlpha = pAlphaWriteAccess->GetScanline( y );
                    // Handling alpha and mask just the same...
                    for( long x=0; x<aDestBmpSize.Width(); ++x )
                    {
                        ::basegfx::B2DPoint aPoint(x,y);
                        aPoint *= aTransform;

                        const int nSrcX( ::basegfx::fround( aPoint.getX() ) );
                        const int nSrcY( ::basegfx::fround( aPoint.getY() ) );
                        if( nSrcX < 0 || nSrcX >= aBmpSize.Width() ||
                            nSrcY < 0 || nSrcY >= aBmpSize.Height() )
                        {
                            pAlphaWriteAccess->SetPixelOnData( pScanAlpha, x, BitmapColor(255) );
                        }
                        else
                        {
                            const sal_uInt8 cAlphaIdx = pAlphaReadAccess->GetPixelIndex( nSrcY, nSrcX );
                            pAlphaWriteAccess->SetPixelOnData( pScanAlpha, x, BitmapColor(aAlphaMap[ cAlphaIdx ]) );
                            pWriteAccess->SetPixelOnData( pScan, x, pReadAccess->GetPixel( nSrcY, nSrcX ) );
                        }
                    }
                }
                else
                {
                    Scanline pScan = pWriteAccess->GetScanline( y );
                    Scanline pScanAlpha = pAlphaWriteAccess->GetScanline( y );
                    for( long x=0; x<aDestBmpSize.Width(); ++x )
                    {
                        ::basegfx::B2DPoint aPoint(x,y);
                        aPoint *= aTransform;

                        const int nSrcX( ::basegfx::fround( aPoint.getX() ) );
                        const int nSrcY( ::basegfx::fround( aPoint.getY() ) );
                        if( nSrcX < 0 || nSrcX >= aBmpSize.Width() ||
                            nSrcY < 0 || nSrcY >= aBmpSize.Height() )
                        {
                            pAlphaWriteAccess->SetPixelOnData( pScanAlpha, x, BitmapColor(255) );
                        }
                        else
                        {
                            pAlphaWriteAccess->SetPixelOnData( pScanAlpha, x, BitmapColor(0) );
                            pWriteAccess->SetPixelOnData( pScan, x, pReadAccess->GetPixel( nSrcY,
                                                                                 nSrcX ) );
                        }
                    }
                }
            }

            bCopyBack = true;
        }
        else
        {
            // TODO(E2): Error handling!
            ENSURE_OR_THROW( false,
                              "transformBitmap(): could not access bitmap" );
        }
    }

    if( bCopyBack )
        return BitmapEx( aDstBitmap, AlphaMask( aDstAlpha ) );
    else
        return BitmapEx();
}


void DrawAlphaBitmapAndAlphaGradient(BitmapEx & rBitmapEx, bool bFixedTransparence, float fTransparence, AlphaMask & rNewMask)
{
    // mix existing and new alpha mask
    AlphaMask aOldMask;

    if(rBitmapEx.IsAlpha())
    {
        aOldMask = rBitmapEx.GetAlpha();
    }
    else if(TransparentType::Bitmap == rBitmapEx.GetTransparentType())
    {
        aOldMask = rBitmapEx.GetMask();
    }
    else if(TransparentType::Color == rBitmapEx.GetTransparentType())
    {
        aOldMask = rBitmapEx.GetBitmap().CreateMask(rBitmapEx.GetTransparentColor());
    }

    {
        AlphaMask::ScopedWriteAccess pOld(aOldMask);

        assert(pOld && "Got no access to old alpha mask (!)");

        const double fFactor(1.0 / 255.0);

        if(bFixedTransparence)
        {
            const double fOpNew(1.0 - fTransparence);

            for(long y(0); y < pOld->Height(); y++)
            {
                Scanline pScanline = pOld->GetScanline( y );
                for(long x(0); x < pOld->Width(); x++)
                {
                    const double fOpOld(1.0 - (pOld->GetIndexFromData(pScanline, x) * fFactor));
                    const sal_uInt8 aCol(basegfx::fround((1.0 - (fOpOld * fOpNew)) * 255.0));

                    pOld->SetPixelOnData(pScanline, x, BitmapColor(aCol));
                }
            }
        }
        else
        {
            AlphaMask::ScopedReadAccess pNew(rNewMask);

            assert(pNew && "Got no access to new alpha mask (!)");

            assert(pOld->Width() == pNew->Width() && pOld->Height() == pNew->Height() &&
                    "Alpha masks have different sizes (!)");

            for(long y(0); y < pOld->Height(); y++)
            {
                Scanline pScanline = pOld->GetScanline( y );
                for(long x(0); x < pOld->Width(); x++)
                {
                    const double fOpOld(1.0 - (pOld->GetIndexFromData(pScanline, x) * fFactor));
                    const double fOpNew(1.0 - (pNew->GetIndexFromData(pScanline, x) * fFactor));
                    const sal_uInt8 aCol(basegfx::fround((1.0 - (fOpOld * fOpNew)) * 255.0));

                    pOld->SetPixelOnData(pScanline, x, BitmapColor(aCol));
                }
            }
        }

    }

    // apply combined bitmap as mask
    rBitmapEx = BitmapEx(rBitmapEx.GetBitmap(), aOldMask);
}

}} // end vcl::bitmap

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
