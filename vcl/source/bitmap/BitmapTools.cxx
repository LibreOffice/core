/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sal/config.h>

#include <array>
#include <utility>

#include <tools/helpers.hxx>
#include <tools/mapunit.hxx>
#include <vcl/BitmapTools.hxx>

#include <sal/log.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/seqstream.hxx>
#include <vcl/canvastools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

#include <com/sun/star/graphic/SvgTools.hpp>
#include <com/sun/star/graphic/Primitive2DTools.hpp>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

#include <com/sun/star/rendering/XIntegerReadOnlyBitmap.hpp>

#include <vcl/alpha.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#if ENABLE_CAIRO_CANVAS
#include <cairo.h>
#endif
#include <comphelper/diagnose_ex.hxx>
#include <tools/fract.hxx>
#include <tools/stream.hxx>
#include <vcl/BitmapWriteAccess.hxx>
#include <vcl/RawBitmap.hxx>

using namespace css;

using drawinglayer::primitive2d::Primitive2DSequence;
using drawinglayer::primitive2d::Primitive2DReference;

namespace vcl::bitmap
{

Bitmap loadFromName(const OUString& rFileName, const ImageLoadFlags eFlags)
{
    bool bSuccess = true;
    OUString aIconTheme;
    Bitmap aBitmap;
    try
    {
        aIconTheme = Application::GetSettings().GetStyleSettings().DetermineIconTheme();
        ImageTree::get().loadImage(rFileName, aIconTheme, aBitmap, true, eFlags);
    }
    catch (...)
    {
        bSuccess = false;
    }

    SAL_WARN_IF(!bSuccess, "vcl", "vcl::bitmap::loadFromName : could not load image " << rFileName << " via icon theme " << aIconTheme);

    return aBitmap;
}

void loadFromSvg(SvStream& rStream, const OUString& sPath, Bitmap& rBitmap, double fScalingFactor)
{
    const uno::Reference<uno::XComponentContext>& xContext(comphelper::getProcessComponentContext());
    const uno::Reference<graphic::XSvgParser> xSvgParser = graphic::SvgTools::create(xContext);

    std::size_t nSize = rStream.remainingSize();
    std::vector<sal_Int8> aBuffer(nSize + 1);
    rStream.ReadBytes(aBuffer.data(), nSize);
    aBuffer[nSize] = 0;

    uno::Sequence<sal_Int8> aData(aBuffer.data(), nSize + 1);
    uno::Reference<io::XInputStream> aInputStream(new comphelper::SequenceInputStream(aData));

    const Primitive2DSequence aPrimitiveSequence = xSvgParser->getDecomposition(aInputStream, sPath);

    if (!aPrimitiveSequence.hasElements())
        return;

    uno::Sequence<beans::PropertyValue> aViewParameters;

    geometry::RealRectangle2D aRealRect;
    basegfx::B2DRange aRange;
    for (css::uno::Reference<css::graphic::XPrimitive2D> const & xReference : aPrimitiveSequence)
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
        rBitmap = vcl::unotools::bitmapFromXBitmap(xIntBmp);
    }

}

/** Copy block of image data into the bitmap.
    Assumes that the Bitmap has been constructed with the desired size.

    @param pData
    The block of data to copy
    @param nStride
    The number of bytes in a scanline, must >= (width * nBitCount / 8)
    @param bReversColors
    In case the endianness of pData is wrong, you could reverse colors
*/
Bitmap CreateFromData(sal_uInt8 const *pData, sal_Int32 nWidth, sal_Int32 nHeight,
                        sal_Int32 nStride, sal_Int8 nBitCount,
                        bool bReversColors, bool bReverseAlpha)
{
    assert(nStride >= (nWidth * nBitCount / 8));
    assert(nBitCount == 1 || nBitCount == 8 || nBitCount == 24 || nBitCount == 32);

    PixelFormat ePixelFormat;
    if (nBitCount == 1)
        ePixelFormat = PixelFormat::N8_BPP; // we convert 1-bit input data to 8-bit format
    else if (nBitCount == 8)
        ePixelFormat = PixelFormat::N8_BPP;
    else if (nBitCount == 24)
        ePixelFormat = PixelFormat::N24_BPP;
    else if (nBitCount == 32)
        ePixelFormat = PixelFormat::N32_BPP;
    else
        std::abort();
    Bitmap aBmp;
    if (nBitCount == 1)
    {
        BitmapPalette aBiLevelPalette { COL_BLACK, COL_WHITE };
        aBmp = Bitmap(Size(nWidth, nHeight), PixelFormat::N8_BPP, &aBiLevelPalette);
    }
    else
        aBmp = Bitmap(Size(nWidth, nHeight), ePixelFormat);

    BitmapScopedWriteAccess pWrite(aBmp);
    assert(pWrite.get());
    if( !pWrite )
        return Bitmap();
    if (nBitCount == 32)
    {
        for( tools::Long y = 0; y < nHeight; ++y )
        {
            sal_uInt8 const *p = pData + (y * nStride);
            Scanline pScanline = pWrite->GetScanline(y);
            for (tools::Long x = 0; x < nWidth; ++x)
            {
                // FIXME this parameter is badly named
                const sal_uInt8 nAlphaValue = bReverseAlpha ? p[3] : 0xff - p[3];
                BitmapColor col;
                if ( bReversColors )
                    col = BitmapColor( ColorAlpha, p[2], p[1], p[0], nAlphaValue );
                else
                    col = BitmapColor( ColorAlpha, p[0], p[1], p[2], nAlphaValue );
                pWrite->SetPixelOnData(pScanline, x, col);
                p += 4;
            }
        }
    }
    else if (nBitCount == 1)
    {
        for( tools::Long y = 0; y < nHeight; ++y )
        {
            sal_uInt8 const *p = pData + y * nStride / 8;
            Scanline pScanline = pWrite->GetScanline(y);
            for (tools::Long x = 0; x < nWidth; ++x)
            {
                int bitIndex = (y * nStride + x) % 8;

                pWrite->SetPixelOnData(pScanline, x, BitmapColor((*p >> bitIndex) & 1));
            }
        }
    }
    else
    {
        for( tools::Long y = 0; y < nHeight; ++y )
        {
            sal_uInt8 const *p = pData + (y * nStride);
            Scanline pScanline = pWrite->GetScanline(y);
            for (tools::Long x = 0; x < nWidth; ++x)
            {
                BitmapColor col;
                if (nBitCount == 8)
                    col = BitmapColor( *p );
                else if ( bReversColors )
                    col = BitmapColor( p[2], p[1], p[0] );
                else
                    col = BitmapColor( p[0], p[1], p[2] );
                pWrite->SetPixelOnData(pScanline, x, col);
                p += nBitCount/8;
            }
        }
    }
    // Avoid further bitmap use with unfinished write access
    pWrite.reset();
    return aBmp;
}

/** Copy block of image data into the bitmap.
    Assumes that the Bitmap has been constructed with the desired size.
*/
Bitmap CreateFromData( RawBitmap&& rawBitmap )
{
    auto nBitCount = rawBitmap.GetBitCount();
    assert( nBitCount == 24 || nBitCount == 32);

    auto ePixelFormat = vcl::PixelFormat::INVALID;

    if (nBitCount == 24)
        ePixelFormat = vcl::PixelFormat::N24_BPP;
    else if (nBitCount == 32)
        ePixelFormat = vcl::PixelFormat::N32_BPP;

    assert(ePixelFormat != vcl::PixelFormat::INVALID);

    Bitmap aBmp(rawBitmap.maSize, ePixelFormat);

    BitmapScopedWriteAccess pWrite(aBmp);
    assert(pWrite.get());
    if( !pWrite )
        return Bitmap();

    auto nHeight = rawBitmap.maSize.getHeight();
    auto nWidth = rawBitmap.maSize.getWidth();
    auto nStride = nWidth * nBitCount / 8;
    if (nBitCount == 32)
    {
        for( tools::Long y = 0; y < nHeight; ++y )
        {
            sal_uInt8 const *p = rawBitmap.mpData.get() + (y * nStride);
            Scanline pScanline = pWrite->GetScanline(y);
            for (tools::Long x = 0; x < nWidth; ++x)
            {
                BitmapColor col(ColorAlpha, p[0], p[1], p[2], p[3]);
                pWrite->SetPixelOnData(pScanline, x, col);
                p += 4;
            }
        }
    }
    else
    {
        for( tools::Long y = 0; y < nHeight; ++y )
        {
            sal_uInt8 const *p = rawBitmap.mpData.get() + (y * nStride);
            Scanline pScanline = pWrite->GetScanline(y);
            for (tools::Long x = 0; x < nWidth; ++x)
            {
                BitmapColor col(p[0], p[1], p[2]);
                pWrite->SetPixelOnData(pScanline, x, col);
                p += nBitCount/8;
            }
        }
    }
    pWrite.reset();

    return aBmp;
}

void fillWithData(sal_uInt8* pData, Bitmap const& rBitmap)
{
    BitmapScopedReadAccess aReadAccess(rBitmap);
    assert(aReadAccess);

    sal_uInt8* p = pData;

    for (tools::Long y = 0, nHeight = aReadAccess->Height(); y < nHeight; ++y)
    {
        Scanline pScanline = aReadAccess->GetScanline(y);

        for (tools::Long x = 0, nWidth = aReadAccess->Width(); x < nWidth; ++x)
        {
            BitmapColor aColor = aReadAccess->GetPixelFromData(pScanline, x);
            *p++ = aColor.GetBlue();
            *p++ = aColor.GetGreen();
            *p++ = aColor.GetRed();
            *p++ = aColor.GetAlpha();
        }
    }
}


#if ENABLE_CAIRO_CANVAS
Bitmap CreateFromCairoSurface(Size aSize, cairo_surface_t * pSurface)
{
    // FIXME: if we could teach VCL/ about cairo handles, life could
    // be significantly better here perhaps.

    cairo_surface_t *pPixels = cairo_surface_create_similar_image(pSurface,
            CAIRO_FORMAT_ARGB32, aSize.Width(), aSize.Height());
    cairo_t *pCairo = cairo_create( pPixels );
    if( !pPixels || !pCairo || cairo_status(pCairo) != CAIRO_STATUS_SUCCESS )
        return Bitmap();

    // suck ourselves from the X server to this buffer so then we can fiddle with
    // Alpha to turn it into the ultra-lame vcl required format and then push it
    // all back again later at vast expense [ urgh ]
    cairo_set_source_surface( pCairo, pSurface, 0, 0 );
    cairo_set_operator( pCairo, CAIRO_OPERATOR_SOURCE );
    cairo_paint( pCairo );

    Bitmap aRGBA(aSize, vcl::PixelFormat::N32_BPP);

    BitmapScopedWriteAccess pRGBAWrite(aRGBA);
    assert(pRGBAWrite);
    if (!pRGBAWrite)
        return Bitmap();

    cairo_surface_flush(pPixels);
    unsigned char *pSrc = cairo_image_surface_get_data( pPixels );
    unsigned int nStride = cairo_image_surface_get_stride( pPixels );
#if !ENABLE_WASM_STRIP_PREMULTIPLY
    vcl::bitmap::lookup_table const & unpremultiply_table = vcl::bitmap::get_unpremultiply_table();
#endif
    for( tools::Long y = 0; y < aSize.Height(); y++ )
    {
        sal_uInt32 *pPix = reinterpret_cast<sal_uInt32 *>(pSrc + nStride * y);
        for( tools::Long x = 0; x < aSize.Width(); x++ )
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
#if ENABLE_WASM_STRIP_PREMULTIPLY
                nR = vcl::bitmap::unpremultiply(nR, nAlpha);
                nG = vcl::bitmap::unpremultiply(nG, nAlpha);
                nB = vcl::bitmap::unpremultiply(nB, nAlpha);
#else
                nR = unpremultiply_table[nAlpha][nR];
                nG = unpremultiply_table[nAlpha][nG];
                nB = unpremultiply_table[nAlpha][nB];
#endif
            }
            pRGBAWrite->SetPixel( y, x, BitmapColor( ColorAlpha, nR, nG, nB, nAlpha ) );
            pPix++;
        }
    }

    pRGBAWrite.reset();

    // ignore potential errors above. will get caller a
    // uniformly white bitmap, but not that there would
    // be error handling in calling code ...

    cairo_destroy( pCairo );
    cairo_surface_destroy( pPixels );
    return aRGBA;
}
#endif

Bitmap CanvasTransformBitmap( const Bitmap&                 rSrcBitmap,
                                const ::basegfx::B2DHomMatrix&  rTransform,
                                ::basegfx::B2DRectangle const & rDestRect,
                                ::basegfx::B2DHomMatrix const & rLocalTransform )
{
    const Size aDestBmpSize( ::basegfx::fround<tools::Long>( rDestRect.getWidth() ),
                             ::basegfx::fround<tools::Long>( rDestRect.getHeight() ) );

    if( aDestBmpSize.IsEmpty() )
        return Bitmap();

    const Size aBmpSize( rSrcBitmap.GetSizePixel() );

    // differentiate mask and alpha channel (on-off
    // vs. multi-level transparency)

    BitmapScopedReadAccess pReadAccess( rSrcBitmap );

    if( !pReadAccess )
    {
        // TODO(E2): Error handling!
        ENSURE_OR_THROW( false,
                          "transformBitmap(): could not access source bitmap" );
    }

    // mapping table, to translate pAlphaReadAccess' pixel
    // values into destination alpha values (needed e.g. for
    // paletted 1-bit masks).
    sal_uInt8 aAlphaMap[256];

    if( rSrcBitmap.HasAlpha() )
    {
        // source already has alpha channel - 1:1 mapping,
        // i.e. aAlphaMap[0]=0,...,aAlphaMap[255]=255.
        sal_uInt8  val=0;
        sal_uInt8* pCur=aAlphaMap;
        sal_uInt8* const pEnd=&aAlphaMap[256];
        while(pCur != pEnd)
            *pCur++ = val++;
    }
    // else: mapping table is not used

    Bitmap aDstBitmap(aDestBmpSize, rSrcBitmap.getPixelFormat(), &pReadAccess->GetPalette());

    {
        // just to be on the safe side: let the
        // ScopedAccessors get destructed before
        // copy-constructing the resulting bitmap. This will
        // rule out the possibility that cached accessor data
        // is not yet written back.
        BitmapScopedWriteAccess pWriteAccess( aDstBitmap );

        if( pWriteAccess.get() != nullptr &&
            rTransform.isInvertible() )
        {
            // we're doing inverse mapping here, i.e. mapping
            // points from the destination bitmap back to the
            // source
            ::basegfx::B2DHomMatrix aTransform( rLocalTransform );
            aTransform.invert();

            // for the time being, always read as ARGB
            for( tools::Long y=0; y<aDestBmpSize.Height(); ++y )
            {
                // differentiate mask and alpha channel (on-off
                // vs. multi-level transparency)
                if( rSrcBitmap.HasAlpha() )
                {
                    Scanline pScan = pWriteAccess->GetScanline( y );
                    // Handling alpha and mask just the same...
                    for( tools::Long x=0; x<aDestBmpSize.Width(); ++x )
                    {
                        ::basegfx::B2DPoint aPoint(x,y);
                        aPoint *= aTransform;

                        const tools::Long nSrcX( ::basegfx::fround<tools::Long>( aPoint.getX() ) );
                        const tools::Long nSrcY( ::basegfx::fround<tools::Long>( aPoint.getY() ) );
                        if( nSrcX < 0 || nSrcX >= aBmpSize.Width() ||
                            nSrcY < 0 || nSrcY >= aBmpSize.Height() )
                        {
                            pWriteAccess->SetPixelOnData( pScan, x, BitmapColor(ColorAlpha, 0, 0, 0, 0) );
                        }
                        else
                        {
                            BitmapColor aCol = pReadAccess->GetPixel( nSrcY, nSrcX );
                            const sal_uInt8 cAlphaIdx = aCol.GetAlpha();
                            aCol.SetAlpha(aAlphaMap[ cAlphaIdx ]);
                            pWriteAccess->SetPixelOnData( pScan, x, aCol );
                        }
                    }
                }
                else
                {
                    Scanline pScan = pWriteAccess->GetScanline( y );
                    for( tools::Long x=0; x<aDestBmpSize.Width(); ++x )
                    {
                        ::basegfx::B2DPoint aPoint(x,y);
                        aPoint *= aTransform;

                        const tools::Long nSrcX( ::basegfx::fround<tools::Long>( aPoint.getX() ) );
                        const tools::Long nSrcY( ::basegfx::fround<tools::Long>( aPoint.getY() ) );
                        if( nSrcX < 0 || nSrcX >= aBmpSize.Width() ||
                            nSrcY < 0 || nSrcY >= aBmpSize.Height() )
                        {
                            pWriteAccess->SetPixelOnData( pScan, x, BitmapColor(ColorAlpha, 0, 0, 0, 0) );
                        }
                        else
                        {
                            BitmapColor aCol = pReadAccess->GetPixel( nSrcY, nSrcX );
                            aCol.SetAlpha(255);
                            pWriteAccess->SetPixelOnData( pScan, x, aCol );
                        }
                    }
                }
            }
        }
        else
        {
            // TODO(E2): Error handling!
            ENSURE_OR_THROW( false,
                              "transformBitmap(): could not access bitmap" );
        }
    }

    return aDstBitmap;
}

// mix existing and new alpha mask
void DrawAlphaBitmapAndAlphaGradient(Bitmap & rBitmap, bool bFixedTransparence, float fTransparence, const AlphaMask & rNewMask)
{
    const double fFactor(1.0 / 255.0);
    BitmapScopedWriteAccess pOld(rBitmap);
    assert(pOld && "Got no access to old alpha mask (!)");

    if(bFixedTransparence)
    {
        const double fOpNew(1.0 - fTransparence);

        for(tools::Long y(0),nHeight(pOld->Height()); y < nHeight; y++)
        {
            Scanline pScanline = pOld->GetScanline( y );
            for(tools::Long x(0),nWidth(pOld->Width()); x < nWidth; x++)
            {
                BitmapColor aCol = pOld->GetPixelFromData(pScanline, x);
                const double fOpOld(aCol.GetAlpha() * fFactor);
                aCol.SetAlpha(basegfx::fround((fOpOld * fOpNew) * 255.0));

                pOld->SetPixelOnData(pScanline, x, aCol);
            }
        }
    }
    else
    {
        BitmapScopedReadAccess pNew(rNewMask);
        assert(pNew && "Got no access to new alpha mask (!)");

        assert(pOld->Width() == pNew->Width() && pOld->Height() == pNew->Height() &&
                "Alpha masks have different sizes (!)");

        for(tools::Long y(0),nHeight(pOld->Height()); y < nHeight; y++)
        {
            Scanline pScanline = pOld->GetScanline( y );
            for(tools::Long x(0),nWidth(pOld->Width()); x < nWidth; x++)
            {
                BitmapColor aCol = pOld->GetPixelFromData(pScanline, x);
                const double fOpOld(aCol.GetAlpha() * fFactor);
                const double fOpNew(pNew->GetIndexFromData(pScanline, x) * fFactor);
                aCol.SetAlpha(basegfx::fround((fOpOld * fOpNew) * 255.0));

                pOld->SetPixelOnData(pScanline, x, aCol);
            }
        }
    }
}


void DrawAndClipBitmap(const Point& rPos, const Size& rSize, const Bitmap& rBitmap, Bitmap & aBmp, basegfx::B2DPolyPolygon const & rClipPath)
{
    ScopedVclPtrInstance< VirtualDevice > pVDev;
    MapMode aMapMode( MapUnit::Map100thMM );
    aMapMode.SetOrigin( Point( -rPos.X(), -rPos.Y() ) );
    const Size aOutputSizePixel( pVDev->LogicToPixel( rSize, aMapMode ) );
    const Size aSizePixel( rBitmap.GetSizePixel() );
    if ( aOutputSizePixel.Width() && aOutputSizePixel.Height() )
    {
        aMapMode.SetScaleX( Fraction( aSizePixel.Width(), aOutputSizePixel.Width() ) );
        aMapMode.SetScaleY( Fraction( aSizePixel.Height(), aOutputSizePixel.Height() ) );
    }
    pVDev->SetMapMode( aMapMode );
    pVDev->SetOutputSizePixel( aSizePixel );
    pVDev->SetFillColor( COL_BLACK );
    const tools::PolyPolygon aClip( rClipPath );
    pVDev->DrawPolyPolygon( aClip );

    // #i50672# Extract whole VDev content (to match size of rBitmap)
    pVDev->EnableMapMode( false );
    const Bitmap aVDevMask(pVDev->GetBitmap(Point(), aSizePixel));

    if(aBmp.HasAlpha())
    {
        // bitmap already uses a Mask or Alpha, we need to blend that with
        // the new masking in pVDev.
        // need to blend in AlphaMask quality (8Bit)
        AlphaMask fromVDev(aVDevMask);
        BitmapScopedReadAccess pR(fromVDev);
        BitmapScopedWriteAccess pW(aBmp);
        assert(pR && pW);

        if(pR && pW)
        {
            const tools::Long nWidth(std::min(pR->Width(), pW->Width()));
            const tools::Long nHeight(std::min(pR->Height(), pW->Height()));

            for(tools::Long nY(0); nY < nHeight; nY++)
            {
                Scanline pScanlineR = pR->GetScanline( nY );
                Scanline pScanlineW = pW->GetScanline( nY );
                for(tools::Long nX(0); nX < nWidth; nX++)
                {
                    const sal_uInt8 nIndR(pR->GetIndexFromData(pScanlineR, nX));
                    Color aCol = pW->GetColorFromData(pScanlineW, nX);
                    const sal_uInt8 nIndW(aCol.GetAlpha());

                    // these values represent alpha (255 == no, 0 == fully transparent),
                    // so to blend these we have to multiply
                    const sal_uInt8 nCombined((nIndR * nIndW) >> 8);

                    aCol.SetAlpha(nCombined);
                    pW->SetPixelOnData(pScanlineW, nX, aCol);
                }
            }
        }

        pR.reset();
        pW.reset();
    }
    else
    {
        // no mask yet, create and add new mask. For better quality, use Alpha,
        // this allows the drawn mask being processed with AntiAliasing (AAed)
        aBmp = Bitmap(rBitmap, aVDevMask);
    }
}

css::uno::Sequence< sal_Int8 > GetMaskDIB(Bitmap const & aBmp)
{
    if ( aBmp.HasAlpha() )
    {
        SvMemoryStream aMem;
        AlphaMask aMask = aBmp.CreateAlphaMask();
        // for backwards compatibility for extensions, we need to convert from alpha to transparency
        aMask.Invert();
        WriteDIB(aMask.GetBitmap(), aMem, false, true);
        return css::uno::Sequence< sal_Int8 >( static_cast<sal_Int8 const *>(aMem.GetData()), aMem.Tell() );
    }

    return css::uno::Sequence< sal_Int8 >();
}

/**
 * This returns data formatted the way Cairo wants it, i.e. either CAIRO_FORMAT_ARGB32 or CAIRO_FORMAT_RGB24
 *
 * @param data will be filled with alpha data, if xBitmap is alpha/transparent image
 * @param bHasAlpha will be set to true if resulting surface has alpha
 **/
void CanvasCairoExtractBitmapData( const Bitmap & aBitmap, unsigned char*& data, bool& bHasAlpha, tools::Long& rnWidth, tools::Long& rnHeight )
{
    BitmapScopedReadAccess pBitmapReadAcc( aBitmap );
    const tools::Long      nWidth = rnWidth = pBitmapReadAcc->Width();
    const tools::Long      nHeight = rnHeight = pBitmapReadAcc->Height();
    tools::Long nX, nY;
    bHasAlpha = aBitmap.HasAlpha();

    /// BOTH of the cairo formats use a 32-bit pixel.
    auto nScanlineSize = nWidth*4;
    data = static_cast<unsigned char*>(malloc(nHeight * nScanlineSize));
    if (!data)
        std::abort();

    ::Color aColor;

    for( nY = 0; nY < nHeight; nY++ )
    {
        ::Scanline pReadScan;
        tools::Long nOff = nY * nScanlineSize;

        switch( pBitmapReadAcc->GetScanlineFormat() )
        {
        case ScanlineFormat::N8BitPal:
            assert(!bHasAlpha);
            pReadScan = pBitmapReadAcc->GetScanline( nY );

            for( nX = 0; nX < nWidth; nX++ )
            {
                aColor = pBitmapReadAcc->GetPaletteColor(*pReadScan++);
#ifdef OSL_BIGENDIAN
                data[ nOff++ ] = 0xff;
                data[ nOff++ ] = aColor.GetRed();
                data[ nOff++ ] = aColor.GetGreen();
                data[ nOff++ ] = aColor.GetBlue();
#else
                data[ nOff++ ] = aColor.GetBlue();
                data[ nOff++ ] = aColor.GetGreen();
                data[ nOff++ ] = aColor.GetRed();
                data[ nOff++ ] = 0xff;
#endif
            }
            break;
        case ScanlineFormat::N24BitTcBgr:
            assert(!bHasAlpha);
            pReadScan = pBitmapReadAcc->GetScanline( nY );

            for( nX = 0; nX < nWidth; nX++ )
            {
#ifdef OSL_BIGENDIAN
                data[ nOff + 3 ] = 0xff;
                data[ nOff + 2 ] = *pReadScan++;
                data[ nOff + 1 ] = *pReadScan++;
                data[ nOff + 0 ] = *pReadScan++;
#else
                data[ nOff + 0 ] = *pReadScan++;
                data[ nOff + 1 ] = *pReadScan++;
                data[ nOff + 2 ] = *pReadScan++;
                data[ nOff + 3 ] = 0xff;
#endif
                nOff += 4;
            }
            break;
        case ScanlineFormat::N24BitTcRgb:
            assert(!bHasAlpha);
            pReadScan = pBitmapReadAcc->GetScanline( nY );

            for( nX = 0; nX < nWidth; nX++ )
            {
#ifdef OSL_BIGENDIAN
                data[ nOff + 3 ] = 0xff;
                data[ nOff + 2 ] = *pReadScan++;
                data[ nOff + 1 ] = *pReadScan++;
                data[ nOff + 0 ] = *pReadScan++;
#else
                data[ nOff + 0 ] = *pReadScan++;
                data[ nOff + 1 ] = *pReadScan++;
                data[ nOff + 2 ] = *pReadScan++;
                data[ nOff + 3 ] = 0xff;
#endif
                nOff += 4;
            }
            break;
        case ScanlineFormat::N32BitTcBgra:
            assert(bHasAlpha);
            pReadScan = pBitmapReadAcc->GetScanline( nY );

            // this data is already premultiplied
            for( nX = 0; nX < nWidth; nX++ )
            {
#ifdef OSL_BIGENDIAN
                data[ nOff++ ] = pReadScan[ 3 ];
                data[ nOff++ ] = pReadScan[ 2 ];
                data[ nOff++ ] = pReadScan[ 1 ];
                data[ nOff++ ] = pReadScan[ 0 ];
                pReadScan += 4;
#else
                data[ nOff++ ] = *pReadScan++;
                data[ nOff++ ] = *pReadScan++;
                data[ nOff++ ] = *pReadScan++;
                data[ nOff++ ] = *pReadScan++;
#endif
            }
            break;

        case ScanlineFormat::N32BitTcBgrx:
            assert(!bHasAlpha);
            pReadScan = pBitmapReadAcc->GetScanline( nY );

            for( nX = 0; nX < nWidth; nX++ )
            {
#ifdef OSL_BIGENDIAN
                data[ nOff++ ] = 0xff;
                data[ nOff++ ] = pReadScan[ 2 ];
                data[ nOff++ ] = pReadScan[ 1 ];
                data[ nOff++ ] = pReadScan[ 0 ];
                pReadScan += 4;
#else
                data[ nOff++ ] = *pReadScan++;
                data[ nOff++ ] = *pReadScan++;
                data[ nOff++ ] = *pReadScan++;
                data[ nOff++ ] = 0xff;
                pReadScan++;
#endif
            }
            break;
        case ScanlineFormat::N32BitTcRgba:
            assert(bHasAlpha);
            pReadScan = pBitmapReadAcc->GetScanline( nY );

            // this data is already premultiplied
            for( nX = 0; nX < nWidth; nX++ )
            {
#ifdef OSL_BIGENDIAN
                data[ nOff++ ] = *pReadScan++;
                data[ nOff++ ] = *pReadScan++;
                data[ nOff++ ] = *pReadScan++;
                data[ nOff++ ] = *pReadScan++;
#else
                data[ nOff++ ] = pReadScan[ 3 ];
                data[ nOff++ ] = pReadScan[ 2 ];
                data[ nOff++ ] = pReadScan[ 1 ];
                data[ nOff++ ] = pReadScan[ 0 ];
                pReadScan += 4;
#endif
            }
            break;

        case ScanlineFormat::N32BitTcRgbx:
            assert(!bHasAlpha);
            pReadScan = pBitmapReadAcc->GetScanline( nY );

            for( nX = 0; nX < nWidth; nX++ )
            {
#ifdef OSL_BIGENDIAN
                data[ nOff++ ] = 0xff;
                data[ nOff++ ] = *pReadScan++;
                data[ nOff++ ] = *pReadScan++;
                data[ nOff++ ] = *pReadScan++;
                pReadScan++;
#else
                data[ nOff++ ] = pReadScan[ 2 ];
                data[ nOff++ ] = pReadScan[ 1 ];
                data[ nOff++ ] = pReadScan[ 0 ];
                data[ nOff++ ] = 0xff;
                pReadScan += 4;
#endif
            }
            break;
        default:
            assert(false && "unknown format");
        }
    }
}

    uno::Sequence< sal_Int8 > CanvasExtractBitmapData(Bitmap const & rBitmap, const geometry::IntegerRectangle2D& rect)
    {
        BitmapScopedReadAccess pReadAccess( rBitmap );
        assert( pReadAccess );

        // TODO(F1): Support more formats.
        const Size aBmpSize( rBitmap.GetSizePixel() );

        // for the time being, always return as BGRA
        uno::Sequence< sal_Int8 > aRes( 4*aBmpSize.Width()*aBmpSize.Height() );
        sal_Int8* pRes = aRes.getArray();

        int nCurrPos(0);
        for( tools::Long y=rect.Y1;
             y<aBmpSize.Height() && y<rect.Y2;
             ++y )
        {
            for( tools::Long x=rect.X1;
                 x<aBmpSize.Width() && x<rect.X2;
                 ++x )
            {
                BitmapColor aCol = pReadAccess->GetColor( y, x );
                pRes[ nCurrPos++ ] = aCol.GetRed();
                pRes[ nCurrPos++ ] = aCol.GetGreen();
                pRes[ nCurrPos++ ] = aCol.GetBlue();
                pRes[ nCurrPos++ ] = 255 - aCol.GetAlpha();
            }
        }
        return aRes;
    }

    Bitmap createHistorical8x8FromArray(std::array<sal_uInt8,64> const & pArray, Color aColorPix, Color aColorBack)
    {
        BitmapPalette aPalette(2);

        aPalette[0] = BitmapColor(aColorBack);
        aPalette[1] = BitmapColor(aColorPix);

        Bitmap aBitmap(Size(8, 8), vcl::PixelFormat::N8_BPP, &aPalette);
        BitmapScopedWriteAccess pContent(aBitmap);

        for(sal_uInt16 a(0); a < 8; a++)
        {
            for(sal_uInt16 b(0); b < 8; b++)
            {
                if(pArray[(a * 8) + b])
                {
                    pContent->SetPixelIndex(a, b, 1);
                }
                else
                {
                    pContent->SetPixelIndex(a, b, 0);
                }
            }
        }

        return aBitmap;
    }

    bool isHistorical8x8(const Bitmap& rBitmap, Color& o_rBack, Color& o_rFront)
    {
        bool bRet(false);

        if(!rBitmap.HasAlpha())
        {
            if(8 == rBitmap.GetSizePixel().Width() && 8 == rBitmap.GetSizePixel().Height())
            {
                // Historical 1bpp images are getting really historical,
                // even to the point that e.g. the png loader actually loads
                // them as RGB. But the pattern code in svx relies on this
                // assumption that any 2-color 1bpp bitmap is a pattern, and so it would
                // get confused by RGB. Try to detect if this image is really
                // just two colors and say it's a pattern bitmap if so.
                BitmapScopedReadAccess access(rBitmap);
                o_rBack = access->GetColor(0,0);
                bool foundSecondColor = false;;
                for(tools::Long y = 0, nHeight = access->Height(); y < nHeight; ++y)
                    for(tools::Long x = 0, nWidth = access->Width(); x < nWidth; ++x)
                    {
                        if(!foundSecondColor)
                        {
                            if( access->GetColor(y,x) != o_rBack )
                            {
                                o_rFront = access->GetColor(y,x);
                                foundSecondColor = true;
                                // Hard to know which of the two colors is the background,
                                // select the lighter one.
                                if( o_rFront.GetLuminance() > o_rBack.GetLuminance())
                                    std::swap( o_rFront, o_rBack );
                            }
                        }
                        else
                        {
                            if( access->GetColor(y,x) != o_rBack && access->GetColor(y,x) != o_rFront)
                                return false;
                        }
                    }
                return true;
            }
        }

        return bRet;
    }

#if ENABLE_WASM_STRIP_PREMULTIPLY
    sal_uInt8 unpremultiply(sal_uInt8 c, sal_uInt8 a)
    {
        return (a == 0) ? 0 : (c * 255 + a / 2) / a;
    }

    sal_uInt8 premultiply(sal_uInt8 c, sal_uInt8 a)
    {
        return (c * a + 127) / 255;
    }
#else
    sal_uInt8 unpremultiply(sal_uInt8 c, sal_uInt8 a)
    {
        return get_unpremultiply_table()[a][c];
    }

    static constexpr sal_uInt8 unpremultiplyImpl(sal_uInt8 c, sal_uInt8 a)
    {
        return (a == 0) ? 0 : (c * 255 + a / 2) / a;
    }

    sal_uInt8 premultiply(sal_uInt8 c, sal_uInt8 a)
    {
        return get_premultiply_table()[a][c];
    }

    static constexpr sal_uInt8 premultiplyImpl(sal_uInt8 c, sal_uInt8 a)
    {
        return (c * a + 127) / 255;
    }

    template<int... Is> static constexpr std::array<sal_uInt8, 256> make_unpremultiply_table_row_(
        int a, std::integer_sequence<int, Is...>)
    {
        return {unpremultiplyImpl(Is, a)...};
    }

    template<int... Is> static constexpr lookup_table make_unpremultiply_table_(
        std::integer_sequence<int, Is...>)
    {
        return {make_unpremultiply_table_row_(Is, std::make_integer_sequence<int, 256>{})...};
    }

    lookup_table const & get_unpremultiply_table()
    {
        static constexpr auto unpremultiply_table = make_unpremultiply_table_(
            std::make_integer_sequence<int, 256>{});
        return unpremultiply_table;
    }

    template<int... Is> static constexpr std::array<sal_uInt8, 256> make_premultiply_table_row_(
        int a, std::integer_sequence<int, Is...>)
    {
        return {premultiplyImpl(Is, a)...};
    }

    template<int... Is> static constexpr lookup_table make_premultiply_table_(
        std::integer_sequence<int, Is...>)
    {
        return {make_premultiply_table_row_(Is, std::make_integer_sequence<int, 256>{})...};
    }

    lookup_table const & get_premultiply_table()
    {
        static constexpr auto premultiply_table = make_premultiply_table_(
            std::make_integer_sequence<int, 256>{});
        return premultiply_table;
    }
#endif

Bitmap GetDownsampledBitmap(Size const& rDstSizeTwip, Point const& rSrcPt, Size const& rSrcSz,
                            Bitmap const& rBmp, tools::Long nMaxBmpDPIX, tools::Long nMaxBmpDPIY)
{
    Bitmap aBmp(rBmp);

    if (!aBmp.IsEmpty())
    {
        const tools::Rectangle aBmpRect( Point(), aBmp.GetSizePixel() );
        tools::Rectangle       aSrcRect( rSrcPt, rSrcSz );

        // do cropping if necessary
        if( aSrcRect.Intersection( aBmpRect ) != aBmpRect )
        {
            if( !aSrcRect.IsEmpty() )
                aBmp.Crop( aSrcRect );
            else
                aBmp.SetEmpty();
        }

        if( !aBmp.IsEmpty() )
        {
            // do downsampling if necessary
            // #103209# Normalize size (mirroring has to happen outside of this method)
            Size aDstSizeTwip(std::abs(rDstSizeTwip.Width()), std::abs(rDstSizeTwip.Height()));

            const Size aBmpSize( aBmp.GetSizePixel() );
            const double fBmpPixelX = aBmpSize.Width();
            const double fBmpPixelY = aBmpSize.Height();
            const double fMaxPixelX
                = o3tl::convert<double>(aDstSizeTwip.Width(), o3tl::Length::twip, o3tl::Length::in)
                  * nMaxBmpDPIX;
            const double fMaxPixelY
                = o3tl::convert<double>(aDstSizeTwip.Height(), o3tl::Length::twip, o3tl::Length::in)
                  * nMaxBmpDPIY;

            // check, if the bitmap DPI exceeds the maximum DPI (allow 4 pixel rounding tolerance)
            if (((fBmpPixelX > (fMaxPixelX + 4)) ||
                  (fBmpPixelY > (fMaxPixelY + 4))) &&
                (fBmpPixelY > 0.0) && (fMaxPixelY > 0.0))
            {
                // do scaling
                Size aNewBmpSize;
                const double fBmpWH = fBmpPixelX / fBmpPixelY;
                const double fMaxWH = fMaxPixelX / fMaxPixelY;

                if (fBmpWH < fMaxWH)
                {
                    aNewBmpSize.setWidth(basegfx::fround<tools::Long>(fMaxPixelY * fBmpWH));
                    aNewBmpSize.setHeight(basegfx::fround<tools::Long>(fMaxPixelY));
                }
                else if (fBmpWH > 0.0)
                {
                    aNewBmpSize.setWidth(basegfx::fround<tools::Long>(fMaxPixelX));
                    aNewBmpSize.setHeight(basegfx::fround<tools::Long>(fMaxPixelX / fBmpWH));
                }

                if( aNewBmpSize.Width() && aNewBmpSize.Height() )
                    aBmp.Scale(aNewBmpSize);
                else
                    aBmp.SetEmpty();
            }
        }
    }

    return aBmp;
}

BitmapColor premultiply(const BitmapColor c)
{
    return BitmapColor(ColorAlpha, premultiply(c.GetRed(), c.GetAlpha()),
                       premultiply(c.GetGreen(), c.GetAlpha()),
                       premultiply(c.GetBlue(), c.GetAlpha()), c.GetAlpha());
}

BitmapColor unpremultiply(const BitmapColor c)
{
    return BitmapColor(ColorAlpha, unpremultiply(c.GetRed(), c.GetAlpha()),
                       unpremultiply(c.GetGreen(), c.GetAlpha()),
                       unpremultiply(c.GetBlue(), c.GetAlpha()), c.GetAlpha());
}

} // end vcl::bitmap

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
