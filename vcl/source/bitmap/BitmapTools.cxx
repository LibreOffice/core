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

#include <sal/log.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/seqstream.hxx>
#include <vcl/canvastools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

#include <com/sun/star/graphic/SvgTools.hpp>
#include <com/sun/star/graphic/Primitive2DTools.hpp>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

#include <com/sun/star/rendering/XIntegerReadOnlyBitmap.hpp>

#include <vcl/dibtools.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#if ENABLE_CAIRO_CANVAS
#include <cairo.h>
#endif
#include <tools/diagnose_ex.h>
#include <tools/fract.hxx>
#include <tools/stream.hxx>
#include <bitmapwriteaccess.hxx>

using namespace css;

using drawinglayer::primitive2d::Primitive2DSequence;
using drawinglayer::primitive2d::Primitive2DReference;

namespace vcl::bitmap
{

BitmapEx loadFromName(const OUString& rFileName, const ImageLoadFlags eFlags)
{
    bool bSuccess = true;
    OUString aIconTheme;
    BitmapEx aBitmapEx;
    try
    {
        aIconTheme = Application::GetSettings().GetStyleSettings().DetermineIconTheme();
        ImageTree::get().loadImage(rFileName, aIconTheme, aBitmapEx, true, eFlags);
    }
    catch (...)
    {
        bSuccess = false;
    }

    SAL_WARN_IF(!bSuccess, "vcl", "vcl::bitmap::loadFromName : could not load image " << rFileName << " via icon theme " << aIconTheme);

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

    const Primitive2DSequence aPrimitiveSequence = xSvgParser->getDecomposition(aInputStream, sPath);

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
        rBitmapEx = vcl::unotools::bitmapExFromXBitmap(xIntBmp);
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

    BitmapScopedWriteAccess pWrite(aBmp);
    assert(pWrite.get());
    if( !pWrite )
        return BitmapEx();
    std::unique_ptr<AlphaMask> pAlphaMask;
    AlphaScopedWriteAccess xMaskAcc;
    if (nBitCount == 32)
    {
        pAlphaMask.reset( new AlphaMask( Size(nWidth, nHeight) ) );
        xMaskAcc = AlphaScopedWriteAccess(*pAlphaMask);
    }
    if (nBitCount == 1)
    {
        for( tools::Long y = 0; y < nHeight; ++y )
        {
            Scanline pScanline = pWrite->GetScanline(y);
            for (tools::Long x = 0; x < nWidth; ++x)
            {
                sal_uInt8 const *p = pData + y * nStride / 8;
                int bitIndex = (y * nStride) % 8;
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
                BitmapColor col(p[0], p[1], p[2]);
                pWrite->SetPixelOnData(pScanline, x, col);
                p += nBitCount/8;
            }
            if (nBitCount == 32)
            {
                p = pData + (y * nStride) + 3;
                Scanline pMaskScanLine = xMaskAcc->GetScanline(y);
                for (tools::Long x = 0; x < nWidth; ++x)
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
        return BitmapEx(aBmp);
}

/** Copy block of image data into the bitmap.
    Assumes that the Bitmap has been constructed with the desired size.
*/
BitmapEx CreateFromData( RawBitmap&& rawBitmap )
{
    auto nBitCount = rawBitmap.GetBitCount();
    assert( nBitCount == 24 || nBitCount == 32);
    Bitmap aBmp( rawBitmap.maSize, nBitCount );

    BitmapScopedWriteAccess pWrite(aBmp);
    assert(pWrite.get());
    if( !pWrite )
        return BitmapEx();
    std::unique_ptr<AlphaMask> pAlphaMask;
    AlphaScopedWriteAccess xMaskAcc;
    if (nBitCount == 32)
    {
        pAlphaMask.reset( new AlphaMask( rawBitmap.maSize ) );
        xMaskAcc = AlphaScopedWriteAccess(*pAlphaMask);
    }

    auto nHeight = rawBitmap.maSize.getHeight();
    auto nWidth = rawBitmap.maSize.getWidth();
    auto nStride = nWidth * nBitCount / 8;
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
        if (nBitCount == 32)
        {
            p = rawBitmap.mpData.get() + (y * nStride) + 3;
            Scanline pMaskScanLine = xMaskAcc->GetScanline(y);
            for (tools::Long x = 0; x < nWidth; ++x)
            {
                xMaskAcc->SetPixelOnData(pMaskScanLine, x, BitmapColor(*p));
                p += 4;
            }
        }
    }
    if (nBitCount == 32)
        return BitmapEx(aBmp, *pAlphaMask);
    else
        return BitmapEx(aBmp);
}

#if ENABLE_CAIRO_CANVAS
BitmapEx* CreateFromCairoSurface(Size aSize, cairo_surface_t * pSurface)
{
    // FIXME: if we could teach VCL/ about cairo handles, life could
    // be significantly better here perhaps.

#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 12, 0)
    cairo_surface_t *pPixels = cairo_surface_create_similar_image(pSurface,
#else
    cairo_surface_t *pPixels = cairo_image_surface_create(
#endif
            CAIRO_FORMAT_ARGB32, aSize.Width(), aSize.Height());
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

    BitmapScopedWriteAccess pRGBWrite(aRGB);
    assert(pRGBWrite);
    if (!pRGBWrite)
        return nullptr;

    AlphaScopedWriteAccess pMaskWrite(aMask);
    assert(pMaskWrite);
    if (!pMaskWrite)
        return nullptr;

    cairo_surface_flush(pPixels);
    unsigned char *pSrc = cairo_image_surface_get_data( pPixels );
    unsigned int nStride = cairo_image_surface_get_stride( pPixels );
    vcl::bitmap::lookup_table unpremultiply_table = vcl::bitmap::get_unpremultiply_table();
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
                nR = unpremultiply_table[nAlpha][nR];
                nG = unpremultiply_table[nAlpha][nG];
                nB = unpremultiply_table[nAlpha][nB];
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

    if( aDestBmpSize.IsEmpty() )
        return BitmapEx();

    Bitmap aDstBitmap( aDestBmpSize, aSrcBitmap.GetBitCount(), &pReadAccess->GetPalette() );
    Bitmap aDstAlpha( AlphaMask( aDestBmpSize ).GetBitmap() );

    {
        // just to be on the safe side: let the
        // ScopedAccessors get destructed before
        // copy-constructing the resulting bitmap. This will
        // rule out the possibility that cached accessor data
        // is not yet written back.
        BitmapScopedWriteAccess pWriteAccess( aDstBitmap );
        BitmapScopedWriteAccess pAlphaWriteAccess( aDstAlpha );


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
            for( tools::Long y=0; y<aDestBmpSize.Height(); ++y )
            {
                // differentiate mask and alpha channel (on-off
                // vs. multi-level transparency)
                if( rBitmap.IsTransparent() )
                {
                    Scanline pScan = pWriteAccess->GetScanline( y );
                    Scanline pScanAlpha = pAlphaWriteAccess->GetScanline( y );
                    // Handling alpha and mask just the same...
                    for( tools::Long x=0; x<aDestBmpSize.Width(); ++x )
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
                    for( tools::Long x=0; x<aDestBmpSize.Width(); ++x )
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
        }
        else
        {
            // TODO(E2): Error handling!
            ENSURE_OR_THROW( false,
                              "transformBitmap(): could not access bitmap" );
        }
    }

    return BitmapEx(aDstBitmap, AlphaMask(aDstAlpha));
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
        AlphaScopedWriteAccess pOld(aOldMask);

        assert(pOld && "Got no access to old alpha mask (!)");

        const double fFactor(1.0 / 255.0);

        if(bFixedTransparence)
        {
            const double fOpNew(1.0 - fTransparence);

            for(tools::Long y(0); y < pOld->Height(); y++)
            {
                Scanline pScanline = pOld->GetScanline( y );
                for(tools::Long x(0); x < pOld->Width(); x++)
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

            for(tools::Long y(0); y < pOld->Height(); y++)
            {
                Scanline pScanline = pOld->GetScanline( y );
                for(tools::Long x(0); x < pOld->Width(); x++)
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


void DrawAndClipBitmap(const Point& rPos, const Size& rSize, const BitmapEx& rBitmap, BitmapEx & aBmpEx, basegfx::B2DPolyPolygon const & rClipPath)
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

    if(aBmpEx.IsTransparent())
    {
        // bitmap already uses a Mask or Alpha, we need to blend that with
        // the new masking in pVDev
        if(aBmpEx.IsAlpha())
        {
            // need to blend in AlphaMask quality (8Bit)
            AlphaMask fromVDev(aVDevMask);
            AlphaMask fromBmpEx(aBmpEx.GetAlpha());
            AlphaMask::ScopedReadAccess pR(fromVDev);
            AlphaScopedWriteAccess pW(fromBmpEx);

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
                        const sal_uInt8 nIndW(pW->GetIndexFromData(pScanlineW, nX));

                        // these values represent transparency (0 == no, 255 == fully transparent),
                        // so to blend these we have to multiply the inverse (opacity)
                        // and re-invert the result to transparence
                        const sal_uInt8 nCombined(0x00ff - (((0x00ff - nIndR) * (0x00ff - nIndW)) >> 8));

                        pW->SetPixelOnData(pScanlineW, nX, BitmapColor(nCombined));
                    }
                }
            }

            pR.reset();
            pW.reset();
            aBmpEx = BitmapEx(aBmpEx.GetBitmap(), fromBmpEx);
        }
        else
        {
            // need to blend in Mask quality (1Bit)
            Bitmap aMask(aVDevMask.CreateMask(COL_WHITE));

            if ( rBitmap.GetTransparentColor() == COL_WHITE )
            {
                aMask.CombineSimple( rBitmap.GetMask(), BmpCombine::Or );
            }
            else
            {
                aMask.CombineSimple( rBitmap.GetMask(), BmpCombine::And );
            }

            aBmpEx = BitmapEx( rBitmap.GetBitmap(), aMask );
        }
    }
    else
    {
        // no mask yet, create and add new mask. For better quality, use Alpha,
        // this allows the drawn mask being processed with AntiAliasing (AAed)
        aBmpEx = BitmapEx(rBitmap.GetBitmap(), aVDevMask);
    }
}


css::uno::Sequence< sal_Int8 > GetMaskDIB(BitmapEx const & aBmpEx)
{
    if ( aBmpEx.IsAlpha() )
    {
        SvMemoryStream aMem;
        WriteDIB(aBmpEx.GetAlpha().GetBitmap(), aMem, false, true);
        return css::uno::Sequence< sal_Int8 >( static_cast<sal_Int8 const *>(aMem.GetData()), aMem.Tell() );
    }
    else if ( aBmpEx.IsTransparent() )
    {
        SvMemoryStream aMem;
        WriteDIB(aBmpEx.GetMask(), aMem, false, true);
        return css::uno::Sequence< sal_Int8 >( static_cast<sal_Int8 const *>(aMem.GetData()), aMem.Tell() );
    }

    return css::uno::Sequence< sal_Int8 >();
}

static bool readAlpha( BitmapReadAccess const * pAlphaReadAcc, tools::Long nY, const tools::Long nWidth, unsigned char* data, tools::Long nOff )
{
    bool bIsAlpha = false;
    tools::Long nX;
    int nAlpha;
    Scanline pReadScan;

    nOff += 3;

    switch( pAlphaReadAcc->GetScanlineFormat() )
    {
        case ScanlineFormat::N8BitPal:
            pReadScan = pAlphaReadAcc->GetScanline( nY );
            for( nX = 0; nX < nWidth; nX++ )
            {
                BitmapColor const& rColor(
                    pAlphaReadAcc->GetPaletteColor(*pReadScan));
                pReadScan++;
                nAlpha = data[ nOff ] = 255 - rColor.GetIndex();
                if( nAlpha != 255 )
                    bIsAlpha = true;
                nOff += 4;
            }
            break;
        default:
            SAL_INFO( "canvas.cairo", "fallback to GetColor for alpha - slow, format: " << static_cast<int>(pAlphaReadAcc->GetScanlineFormat()) );
            for( nX = 0; nX < nWidth; nX++ )
            {
                nAlpha = data[ nOff ] = 255 - pAlphaReadAcc->GetColor( nY, nX ).GetIndex();
                if( nAlpha != 255 )
                    bIsAlpha = true;
                nOff += 4;
            }
    }

    return bIsAlpha;
}



/**
 * @param data will be filled with alpha data, if xBitmap is alpha/transparent image
 * @param bHasAlpha will be set to true if resulting surface has alpha
 **/
void CanvasCairoExtractBitmapData( BitmapEx const & aBmpEx, Bitmap & aBitmap, unsigned char*& data, bool& bHasAlpha, tools::Long& rnWidth, tools::Long& rnHeight )
{
    AlphaMask aAlpha = aBmpEx.GetAlpha();

    ::BitmapReadAccess* pBitmapReadAcc = aBitmap.AcquireReadAccess();
    ::BitmapReadAccess* pAlphaReadAcc = nullptr;
    const tools::Long      nWidth = rnWidth = pBitmapReadAcc->Width();
    const tools::Long      nHeight = rnHeight = pBitmapReadAcc->Height();
    tools::Long nX, nY;
    bool bIsAlpha = false;

    if( aBmpEx.IsTransparent() || aBmpEx.IsAlpha() )
        pAlphaReadAcc = aAlpha.AcquireReadAccess();

    data = static_cast<unsigned char*>(malloc( nWidth*nHeight*4 ));

    tools::Long nOff = 0;
    ::Color aColor;
    unsigned int nAlpha = 255;

    vcl::bitmap::lookup_table premultiply_table = vcl::bitmap::get_premultiply_table();
    for( nY = 0; nY < nHeight; nY++ )
    {
        ::Scanline pReadScan;

        switch( pBitmapReadAcc->GetScanlineFormat() )
        {
        case ScanlineFormat::N8BitPal:
            pReadScan = pBitmapReadAcc->GetScanline( nY );
            if( pAlphaReadAcc )
                if( readAlpha( pAlphaReadAcc, nY, nWidth, data, nOff ) )
                    bIsAlpha = true;

            for( nX = 0; nX < nWidth; nX++ )
            {
#ifdef OSL_BIGENDIAN
                if( pAlphaReadAcc )
                    nAlpha = data[ nOff++ ];
                else
                    nAlpha = data[ nOff++ ] = 255;
#else
                if( pAlphaReadAcc )
                    nAlpha = data[ nOff + 3 ];
                else
                    nAlpha = data[ nOff + 3 ] = 255;
#endif
                aColor = pBitmapReadAcc->GetPaletteColor(*pReadScan++);

#ifdef OSL_BIGENDIAN
                data[ nOff++ ] = premultiply_table[nAlpha][aColor.GetRed()];
                data[ nOff++ ] = premultiply_table[nAlpha][aColor.GetGreen()];
                data[ nOff++ ] = premultiply_table[nAlpha][aColor.GetBlue()];
#else
                data[ nOff++ ] = premultiply_table[nAlpha][aColor.GetBlue()];
                data[ nOff++ ] = premultiply_table[nAlpha][aColor.GetGreen()];
                data[ nOff++ ] = premultiply_table[nAlpha][aColor.GetRed()];
                nOff++;
#endif
            }
            break;
        case ScanlineFormat::N24BitTcBgr:
            pReadScan = pBitmapReadAcc->GetScanline( nY );
            if( pAlphaReadAcc )
                if( readAlpha( pAlphaReadAcc, nY, nWidth, data, nOff ) )
                    bIsAlpha = true;

            for( nX = 0; nX < nWidth; nX++ )
            {
#ifdef OSL_BIGENDIAN
                if( pAlphaReadAcc )
                    nAlpha = data[ nOff ];
                else
                    nAlpha = data[ nOff ] = 255;
                data[ nOff + 3 ] = premultiply_table[nAlpha][*pReadScan++];
                data[ nOff + 2 ] = premultiply_table[nAlpha][*pReadScan++];
                data[ nOff + 1 ] = premultiply_table[nAlpha][*pReadScan++];
                nOff += 4;
#else
                if( pAlphaReadAcc )
                    nAlpha = data[ nOff + 3 ];
                else
                    nAlpha = data[ nOff + 3 ] = 255;
                data[ nOff++ ] = premultiply_table[nAlpha][*pReadScan++];
                data[ nOff++ ] = premultiply_table[nAlpha][*pReadScan++];
                data[ nOff++ ] = premultiply_table[nAlpha][*pReadScan++];
                nOff++;
#endif
            }
            break;
        case ScanlineFormat::N24BitTcRgb:
            pReadScan = pBitmapReadAcc->GetScanline( nY );
            if( pAlphaReadAcc )
                if( readAlpha( pAlphaReadAcc, nY, nWidth, data, nOff ) )
                    bIsAlpha = true;

            for( nX = 0; nX < nWidth; nX++ )
            {
#ifdef OSL_BIGENDIAN
                if( pAlphaReadAcc )
                    nAlpha = data[ nOff++ ];
                else
                    nAlpha = data[ nOff++ ] = 255;
                data[ nOff++ ] = premultiply_table[nAlpha][*pReadScan++];
                data[ nOff++ ] = premultiply_table[nAlpha][*pReadScan++];
                data[ nOff++ ] = premultiply_table[nAlpha][*pReadScan++];
#else
                if( pAlphaReadAcc )
                    nAlpha = data[ nOff + 3 ];
                else
                    nAlpha = data[ nOff + 3 ] = 255;
                data[ nOff++ ] = premultiply_table[nAlpha][pReadScan[ 2 ]];
                data[ nOff++ ] = premultiply_table[nAlpha][pReadScan[ 1 ]];
                data[ nOff++ ] = premultiply_table[nAlpha][pReadScan[ 0 ]];
                pReadScan += 3;
                nOff++;
#endif
            }
            break;
        case ScanlineFormat::N32BitTcBgra:
            pReadScan = pBitmapReadAcc->GetScanline( nY );
            if( pAlphaReadAcc )
                if( readAlpha( pAlphaReadAcc, nY, nWidth, data, nOff ) )
                    bIsAlpha = true;

            for( nX = 0; nX < nWidth; nX++ )
            {
#ifdef OSL_BIGENDIAN
                if( pAlphaReadAcc )
                    nAlpha = data[ nOff++ ];
                else
                    nAlpha = data[ nOff++ ] = 255;
                data[ nOff++ ] = premultiply_table[nAlpha][pReadScan[ 2 ]];
                data[ nOff++ ] = premultiply_table[nAlpha][pReadScan[ 1 ]];
                data[ nOff++ ] = premultiply_table[nAlpha][pReadScan[ 0 ]];
                pReadScan += 4;
#else
                if( pAlphaReadAcc )
                    nAlpha = data[ nOff + 3 ];
                else
                    nAlpha = data[ nOff + 3 ] = 255;
                data[ nOff++ ] = premultiply_table[nAlpha][*pReadScan++];
                data[ nOff++ ] = premultiply_table[nAlpha][*pReadScan++];
                data[ nOff++ ] = premultiply_table[nAlpha][*pReadScan++];
                pReadScan++;
                nOff++;
#endif
            }
            break;
        case ScanlineFormat::N32BitTcRgba:
            pReadScan = pBitmapReadAcc->GetScanline( nY );
            if( pAlphaReadAcc )
                if( readAlpha( pAlphaReadAcc, nY, nWidth, data, nOff ) )
                    bIsAlpha = true;

            for( nX = 0; nX < nWidth; nX++ )
            {
#ifdef OSL_BIGENDIAN
                if( pAlphaReadAcc )
                    nAlpha = data[ nOff ++ ];
                else
                    nAlpha = data[ nOff ++ ] = 255;
                data[ nOff++ ] = premultiply_table[nAlpha][*pReadScan++];
                data[ nOff++ ] = premultiply_table[nAlpha][*pReadScan++];
                data[ nOff++ ] = premultiply_table[nAlpha][*pReadScan++];
                pReadScan++;
#else
                if( pAlphaReadAcc )
                    nAlpha = data[ nOff + 3 ];
                else
                    nAlpha = data[ nOff + 3 ] = 255;
                data[ nOff++ ] = premultiply_table[nAlpha][pReadScan[ 2 ]];
                data[ nOff++ ] = premultiply_table[nAlpha][pReadScan[ 1 ]];
                data[ nOff++ ] = premultiply_table[nAlpha][pReadScan[ 0 ]];
                pReadScan += 4;
                nOff++;
#endif
            }
            break;
        default:
            SAL_INFO( "canvas.cairo", "fallback to GetColor - slow, format: " << static_cast<int>(pBitmapReadAcc->GetScanlineFormat()) );

            if( pAlphaReadAcc )
                if( readAlpha( pAlphaReadAcc, nY, nWidth, data, nOff ) )
                    bIsAlpha = true;

            for( nX = 0; nX < nWidth; nX++ )
            {
                aColor = pBitmapReadAcc->GetColor( nY, nX );

                // cairo need premultiplied color values
                // TODO(rodo) handle endianness
#ifdef OSL_BIGENDIAN
                if( pAlphaReadAcc )
                    nAlpha = data[ nOff++ ];
                else
                    nAlpha = data[ nOff++ ] = 255;
                data[ nOff++ ] = premultiply_table[nAlpha][aColor.GetRed()];
                data[ nOff++ ] = premultiply_table[nAlpha][aColor.GetGreen()];
                data[ nOff++ ] = premultiply_table[nAlpha][aColor.GetBlue()];
#else
                if( pAlphaReadAcc )
                    nAlpha = data[ nOff + 3 ];
                else
                    nAlpha = data[ nOff + 3 ] = 255;
                data[ nOff++ ] = premultiply_table[nAlpha][aColor.GetBlue()];
                data[ nOff++ ] = premultiply_table[nAlpha][aColor.GetGreen()];
                data[ nOff++ ] = premultiply_table[nAlpha][aColor.GetRed()];
                nOff ++;
#endif
            }
        }
    }

    ::Bitmap::ReleaseAccess( pBitmapReadAcc );
    if( pAlphaReadAcc )
        aAlpha.ReleaseAccess( pAlphaReadAcc );

    bHasAlpha = bIsAlpha;

}

    uno::Sequence< sal_Int8 > CanvasExtractBitmapData(BitmapEx const & rBitmapEx, const geometry::IntegerRectangle2D& rect)
    {
        Bitmap aBitmap( rBitmapEx.GetBitmap() );
        Bitmap aAlpha( rBitmapEx.GetAlpha().GetBitmap() );

        Bitmap::ScopedReadAccess pReadAccess( aBitmap );
        Bitmap::ScopedReadAccess pAlphaReadAccess( aAlpha.IsEmpty() ?
                                                 nullptr : aAlpha.AcquireReadAccess(),
                                                 aAlpha );

        assert( pReadAccess );

        // TODO(F1): Support more formats.
        const Size aBmpSize( aBitmap.GetSizePixel() );

        // for the time being, always return as BGRA
        uno::Sequence< sal_Int8 > aRes( 4*aBmpSize.Width()*aBmpSize.Height() );
        sal_Int8* pRes = aRes.getArray();

        int nCurrPos(0);
        for( tools::Long y=rect.Y1;
             y<aBmpSize.Height() && y<rect.Y2;
             ++y )
        {
            if( pAlphaReadAccess.get() != nullptr )
            {
                Scanline pScanlineReadAlpha = pAlphaReadAccess->GetScanline( y );
                for( tools::Long x=rect.X1;
                     x<aBmpSize.Width() && x<rect.X2;
                     ++x )
                {
                    pRes[ nCurrPos++ ] = pReadAccess->GetColor( y, x ).GetRed();
                    pRes[ nCurrPos++ ] = pReadAccess->GetColor( y, x ).GetGreen();
                    pRes[ nCurrPos++ ] = pReadAccess->GetColor( y, x ).GetBlue();
                    pRes[ nCurrPos++ ] = pAlphaReadAccess->GetIndexFromData( pScanlineReadAlpha, x );
                }
            }
            else
            {
                for( tools::Long x=rect.X1;
                     x<aBmpSize.Width() && x<rect.X2;
                     ++x )
                {
                    pRes[ nCurrPos++ ] = pReadAccess->GetColor( y, x ).GetRed();
                    pRes[ nCurrPos++ ] = pReadAccess->GetColor( y, x ).GetGreen();
                    pRes[ nCurrPos++ ] = pReadAccess->GetColor( y, x ).GetBlue();
                    pRes[ nCurrPos++ ] = sal_uInt8(255);
                }
            }
        }
        return aRes;
    }

    BitmapEx createHistorical8x8FromArray(std::array<sal_uInt8,64> const & pArray, Color aColorPix, Color aColorBack)
    {
        BitmapPalette aPalette(2);

        aPalette[0] = BitmapColor(aColorBack);
        aPalette[1] = BitmapColor(aColorPix);

        Bitmap aBitmap(Size(8, 8), 1, &aPalette);
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

        return BitmapEx(aBitmap);
    }

    bool isHistorical8x8(const BitmapEx& rBitmapEx, Color& o_rBack, Color& o_rFront)
    {
        bool bRet(false);

        if(!rBitmapEx.IsTransparent())
        {
            Bitmap aBitmap(rBitmapEx.GetBitmap());

            if(8 == aBitmap.GetSizePixel().Width() && 8 == aBitmap.GetSizePixel().Height())
            {
                if(2 == aBitmap.GetColorCount())
                {
                    BitmapReadAccess* pRead = aBitmap.AcquireReadAccess();

                    if(pRead)
                    {
                        if(pRead->HasPalette() && 2 == pRead->GetPaletteEntryCount())
                        {
                            const BitmapPalette& rPalette = pRead->GetPalette();

                            // #i123564# background and foreground were exchanged; of course
                            // rPalette[0] is the background color
                            o_rFront = rPalette[1];
                            o_rBack = rPalette[0];

                            bRet = true;
                        }

                        Bitmap::ReleaseAccess(pRead);
                    }
                }
            }
        }

        return bRet;
    }

    sal_uInt8 unpremultiply(sal_uInt8 c, sal_uInt8 a)
    {
        return (a == 0) ? 0 : (c * 255 + a / 2) / a;
    }

    sal_uInt8 premultiply(sal_uInt8 c, sal_uInt8 a)
    {
        return (c * a + 127) / 255;
    }

    lookup_table get_unpremultiply_table()
    {
        static bool inited;
        static sal_uInt8 unpremultiply_table[256][256];

        if (!inited)
        {
            for (int a = 0; a < 256; ++a)
                for (int c = 0; c < 256; ++c)
                    unpremultiply_table[a][c] = unpremultiply(c, a);
            inited = true;
        }

        return unpremultiply_table;
    }

    lookup_table get_premultiply_table()
    {
        static bool inited;
        static sal_uInt8 premultiply_table[256][256];

        if (!inited)
        {
            for (int a = 0; a < 256; ++a)
                for (int c = 0; c < 256; ++c)
                    premultiply_table[a][c] = premultiply(c, a);
            inited = true;
        }

        return premultiply_table;
    }

bool convertBitmap32To24Plus8(BitmapEx const & rInput, BitmapEx & rResult)
{
    Bitmap aBitmap(rInput.GetBitmap());
    if (aBitmap.GetBitCount() != 32)
        return false;

    Size aSize = aBitmap.GetSizePixel();
    Bitmap aResultBitmap(aSize, 24);
    AlphaMask aResultAlpha(aSize);
    {
        BitmapScopedWriteAccess pResultBitmapAccess(aResultBitmap);
        AlphaScopedWriteAccess pResultAlphaAccess(aResultAlpha);

        Bitmap::ScopedReadAccess pReadAccess(aBitmap);

        for (tools::Long nY = 0; nY < aSize.Height(); ++nY)
        {
            Scanline aResultScan = pResultBitmapAccess->GetScanline(nY);
            Scanline aResultScanAlpha = pResultAlphaAccess->GetScanline(nY);

            Scanline aReadScan = pReadAccess->GetScanline(nY);

            for (tools::Long nX = 0; nX < aSize.Width(); ++nX)
            {
                const BitmapColor aColor = pReadAccess->GetPixelFromData(aReadScan, nX);
                BitmapColor aResultColor(aColor.GetRed(), aColor.GetGreen(), aColor.GetBlue());
                BitmapColor aResultColorAlpha(aColor.GetAlpha(), aColor.GetAlpha(), aColor.GetAlpha());

                pResultBitmapAccess->SetPixelOnData(aResultScan, nX, aResultColor);
                pResultAlphaAccess->SetPixelOnData(aResultScanAlpha, nX, aResultColorAlpha);
            }
        }
    }
    if (rInput.IsTransparent())
        rResult = BitmapEx(aResultBitmap, rInput.GetAlpha());
    else
        rResult = BitmapEx(aResultBitmap, aResultAlpha);
    return true;
}

} // end vcl::bitmap

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
