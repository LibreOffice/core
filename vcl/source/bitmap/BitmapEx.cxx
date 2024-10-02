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

#include <sal/log.hxx>
#include <rtl/crc.h>
#include <rtl/math.hxx>
#include <o3tl/underlyingenumvalue.hxx>
#include <osl/diagnose.h>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/color/bcolormodifier.hxx>

#include <vcl/ImageTree.hxx>
#include <vcl/bitmap/BitmapMonochromeFilter.hxx>
#include <vcl/outdev.hxx>
#include <vcl/alpha.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <vcl/settings.hxx>

// BitmapEx::Create
#include <salbmp.hxx>
#include <salinst.hxx>
#include <svdata.hxx>
#include <vcl/BitmapWriteAccess.hxx>
#include <bitmap/BitmapMaskToAlphaFilter.hxx>

#include <o3tl/any.hxx>
#include <tools/stream.hxx>
#include <vcl/filter/PngImageWriter.hxx>

#include <com/sun/star/beans/XFastPropertySet.hpp>

#include <memory>

using namespace ::com::sun::star;

BitmapEx::BitmapEx()
{
}

BitmapEx::BitmapEx( const BitmapEx& ) = default;

BitmapEx::BitmapEx( const BitmapEx& rBitmapEx, Point aSrc, Size aSize )
{
    if( rBitmapEx.IsEmpty() || aSize.IsEmpty() )
        return;

    maBitmap = Bitmap(aSize, rBitmapEx.maBitmap.getPixelFormat());
    maBitmapSize = aSize;
    if( rBitmapEx.IsAlpha() )
        maAlphaMask = AlphaMask( aSize );

    tools::Rectangle aDestRect( Point( 0, 0 ), aSize );
    tools::Rectangle aSrcRect( aSrc, aSize );
    CopyPixel( aDestRect, aSrcRect, rBitmapEx );
}

BitmapEx::BitmapEx(Size aSize, vcl::PixelFormat ePixelFormat)
{
    maBitmap = Bitmap(aSize, ePixelFormat);
    maBitmapSize = aSize;
}

BitmapEx::BitmapEx( const OUString& rIconName )
{
    loadFromIconTheme( rIconName );
}

void BitmapEx::loadFromIconTheme( const OUString& rIconName )
{
    bool bSuccess;
    OUString aIconTheme;

    try
    {
        aIconTheme = Application::GetSettings().GetStyleSettings().DetermineIconTheme();
        bSuccess = ImageTree::get().loadImage(rIconName, aIconTheme, *this, true);
    }
    catch (...)
    {
        bSuccess = false;
    }

    SAL_WARN_IF( !bSuccess, "vcl", "BitmapEx::BitmapEx(): could not load image " << rIconName << " via icon theme " << aIconTheme);
}

BitmapEx::BitmapEx( const Bitmap& rBmp ) :
        maBitmap     ( rBmp ),
        maBitmapSize ( maBitmap.GetSizePixel() )
{
}

BitmapEx::BitmapEx( const Bitmap& rBmp, const Bitmap& rMask ) :
        maBitmap         ( rBmp ),
        maBitmapSize     ( maBitmap.GetSizePixel() )
{
    if (rMask.IsEmpty())
        return;

    assert(typeid(rMask) != typeid(AlphaMask)
        && "If this mask is actually an AlphaMask, then it will be inverted unnecessarily "
           "and the alpha channel will be wrong");

    if( rMask.getPixelFormat() == vcl::PixelFormat::N8_BPP && rMask.HasGreyPalette8Bit() )
    {
        maAlphaMask = rMask;
        maAlphaMask.Invert();
    }
    else if( rMask.getPixelFormat() == vcl::PixelFormat::N8_BPP )
    {
        BitmapEx aMaskEx(rMask);
        BitmapFilter::Filter(aMaskEx, BitmapMonochromeFilter(255));
        aMaskEx.Invert();
        maAlphaMask = aMaskEx.GetBitmap();
    }
    else
    {
        // convert to alpha bitmap
        SAL_WARN("vcl", "BitmapEx: forced mask to monochrome");
        BitmapEx aMaskEx(rMask);
        BitmapFilter::Filter(aMaskEx, BitmapMonochromeFilter(255));
        aMaskEx.Invert();
        maAlphaMask = aMaskEx.GetBitmap();
    }

    if (!maBitmap.IsEmpty() && maBitmap.GetSizePixel() != maAlphaMask.GetSizePixel())
    {
        SAL_WARN("vcl", "Mask size differs from Bitmap size, corrected Mask (!)");
        maAlphaMask.Scale(maBitmap.GetSizePixel(), BmpScaleFlag::Fast);
    }
}

BitmapEx::BitmapEx( const Bitmap& rBmp, const AlphaMask& rAlphaMask ) :
        maBitmap         ( rBmp ),
        maAlphaMask      ( rAlphaMask ),
        maBitmapSize     ( maBitmap.GetSizePixel() )
{
    if (!maBitmap.IsEmpty() && !maAlphaMask.IsEmpty() && maBitmap.GetSizePixel() != maAlphaMask.GetSizePixel())
    {
        SAL_WARN("vcl", "Alpha size differs from Bitmap size, corrected Mask (!)");
        maAlphaMask.Scale(rBmp.GetSizePixel(), BmpScaleFlag::Fast);
    }
}


BitmapEx::BitmapEx( const Bitmap& rBmp, const Color& rTransparentColor ) :
        maBitmap             ( rBmp ),
        maBitmapSize         ( maBitmap.GetSizePixel() )
{
    maAlphaMask = maBitmap.CreateAlphaMask( rTransparentColor );

    SAL_WARN_IF(rBmp.GetSizePixel() != maAlphaMask.GetSizePixel(), "vcl",
                "BitmapEx::BitmapEx(): size mismatch for bitmap and alpha mask.");
}


BitmapEx& BitmapEx::operator=( const BitmapEx& ) = default;

bool BitmapEx::operator==( const BitmapEx& rBitmapEx ) const
{
    if (GetSizePixel() != rBitmapEx.GetSizePixel())
        return false;

    if (maBitmap != rBitmapEx.maBitmap)
        return false;

    return maAlphaMask == rBitmapEx.maAlphaMask;
}

bool BitmapEx::IsEmpty() const
{
    return( maBitmap.IsEmpty() && maAlphaMask.IsEmpty() );
}

void BitmapEx::SetEmpty()
{
    maBitmap.SetEmpty();
    maAlphaMask.SetEmpty();
}

void BitmapEx::Clear()
{
    SetEmpty();
}

void BitmapEx::ClearAlpha()
{
    maAlphaMask.SetEmpty();
}

bool BitmapEx::IsAlpha() const
{
    return !maAlphaMask.IsEmpty();
}

const Bitmap& BitmapEx::GetBitmap() const
{
    return maBitmap;
}

Bitmap BitmapEx::GetBitmap( Color aTransparentReplaceColor ) const
{
    Bitmap aRetBmp( maBitmap );

    if( !maAlphaMask.IsEmpty() )
    {
        aRetBmp.Replace( maAlphaMask, aTransparentReplaceColor );
    }

    return aRetBmp;
}

sal_Int64 BitmapEx::GetSizeBytes() const
{
    sal_Int64 nSizeBytes = maBitmap.GetSizeBytes();

    if( !maAlphaMask.IsEmpty() )
        nSizeBytes += maAlphaMask.GetSizeBytes();

    return nSizeBytes;
}

BitmapChecksum BitmapEx::GetChecksum() const
{
    BitmapChecksum  nCrc = maBitmap.GetChecksum();

    if( !maAlphaMask.IsEmpty() )
    {
        BitmapChecksumOctetArray aBCOA;
        BCToBCOA( maAlphaMask.GetChecksum(), aBCOA );
        nCrc = rtl_crc32( nCrc, aBCOA, BITMAP_CHECKSUM_SIZE );
    }

    return nCrc;
}

// AS: Because JPEGs require the alpha channel provided separately (JPEG does not
//  natively support alpha channel, but SWF lets you provide it separately), we
//  extract the alpha channel into a separate array here.
void BitmapEx::GetSplitData( std::vector<sal_uInt8>& rvColorData, std::vector<sal_uInt8>& rvAlphaData ) const
{
    if( IsEmpty() )
        return;

    BitmapScopedReadAccess pRAcc(maBitmap);

    assert( pRAcc );

    AlphaMask   aAlpha;
    sal_uInt32 nWidth = pRAcc->Width();
    sal_uInt32 nHeight = pRAcc->Height();
    rvColorData.resize(nWidth*nHeight*4);
    rvAlphaData.resize(nWidth*nHeight);
    sal_uInt8* p = rvColorData.data(), *pAlpha = rvAlphaData.data();


    if (IsAlpha())
    {
        aAlpha = GetAlphaMask();
    }
    else
    {
        sal_uInt8 cAlphaVal = 0;
        aAlpha = AlphaMask(maBitmap.GetSizePixel(), &cAlphaVal);
    }

    BitmapScopedReadAccess pAAcc(aAlpha);

    assert( pAAcc );

    for( sal_uInt32 nY = 0; nY < nHeight; nY++ )
    {
        Scanline pScanlineAA = pAAcc->GetScanline( nY );
        for( sal_uInt32 nX = 0; nX < nWidth; nX++ )
        {
            const sal_uInt8     nAlpha = pAAcc->GetIndexFromData( pScanlineAA, nX );
            const BitmapColor   aPixelColor( pRAcc->GetColor( nY, nX ) );

            if( nAlpha == 0xff )
            {
                *p++ = 0;
                *p++ = 0;
                *p++ = 0;
                *p++ = 0;
            }
            else
            {
                *p++ = 0xff-nAlpha;
                *p++ = aPixelColor.GetRed();
                *p++ = aPixelColor.GetGreen();
                *p++ = aPixelColor.GetBlue();
            }
            *pAlpha++ = 0xff - nAlpha;
        }
    }
}

bool BitmapEx::Invert()
{
    if (!maBitmap.IsEmpty())
        return maBitmap.Invert();

    return false;
}

bool BitmapEx::Mirror(BmpMirrorFlags nMirrorFlags)
{
    if (maBitmap.IsEmpty())
        return false;

    bool bRet = maBitmap.Mirror( nMirrorFlags );

    if (bRet && !maAlphaMask.IsEmpty())
        maAlphaMask.Mirror(nMirrorFlags);

    return bRet;
}

bool BitmapEx::Scale( const double& rScaleX, const double& rScaleY, BmpScaleFlag nScaleFlag )
{

    if (maBitmap.IsEmpty())
        return false;

    bool bRet = maBitmap.Scale( rScaleX, rScaleY, nScaleFlag );

    if (bRet && !maAlphaMask.IsEmpty())
        maAlphaMask.Scale(rScaleX, rScaleY, nScaleFlag);

    maBitmapSize = maBitmap.GetSizePixel();

    SAL_WARN_IF(!maAlphaMask.IsEmpty() && maBitmap.GetSizePixel() != maAlphaMask.GetSizePixel(), "vcl",
                "BitmapEx::Scale(): size mismatch for bitmap and alpha mask.");

    return bRet;
}

static bool lcl_ShouldScale(Size const& rOrigSize, Size const& rNewSize)
{
    return rOrigSize.Width() && rOrigSize.Height()
            && (rNewSize.Width() != rOrigSize.Width() || rNewSize.Height() != rOrigSize.Height());
}

bool BitmapEx::Scale( const Size& rNewSize, BmpScaleFlag nScaleFlag )
{
    if (lcl_ShouldScale(GetSizePixel(), rNewSize))
    {
        return Scale(static_cast<double>(rNewSize.Width()) / GetSizePixel().Width(),
                 static_cast<double>(rNewSize.Height()) / GetSizePixel().Height(),
                 nScaleFlag);
    }

    return true;
}

bool BitmapEx::Rotate( Degree10 nAngle10, const Color& rFillColor )
{
    if (maBitmap.IsEmpty())
        return false;

    const bool bTransRotate = ( COL_TRANSPARENT == rFillColor );

    bool bRet = false;

    if (bTransRotate)
    {
        bRet = maBitmap.Rotate(nAngle10, COL_BLACK);

        if (maAlphaMask.IsEmpty())
        {
            maAlphaMask = Bitmap(GetSizePixel(), vcl::PixelFormat::N8_BPP, &Bitmap::GetGreyPalette(256));
            maAlphaMask.Erase( 0 );
        }

        if (bRet && !maAlphaMask.IsEmpty())
            maAlphaMask.Rotate(nAngle10, COL_ALPHA_TRANSPARENT);
    }
    else
    {
        bRet = maBitmap.Rotate( nAngle10, rFillColor );

        if (bRet && !maAlphaMask.IsEmpty())
            maAlphaMask.Rotate(nAngle10, COL_ALPHA_TRANSPARENT);
    }

    maBitmapSize = maBitmap.GetSizePixel();

    SAL_WARN_IF(!maAlphaMask.IsEmpty() && maBitmap.GetSizePixel() != maAlphaMask.GetSizePixel(), "vcl",
                "BitmapEx::Rotate(): size mismatch for bitmap and alpha mask.");

    return bRet;
}

bool BitmapEx::Crop( const tools::Rectangle& rRectPixel )
{
    if (maBitmap.IsEmpty())
        return false;

    bool bRet = maBitmap.Crop(rRectPixel);

    if (bRet && !maAlphaMask.IsEmpty())
        maAlphaMask.Crop(rRectPixel);

    maBitmapSize = maBitmap.GetSizePixel();

    SAL_WARN_IF(!maAlphaMask.IsEmpty() && maBitmap.GetSizePixel() != maAlphaMask.GetSizePixel(), "vcl",
                "BitmapEx::Crop(): size mismatch for bitmap and alpha mask.");

    return bRet;
}

bool BitmapEx::Convert( BmpConversion eConversion )
{
    return !maBitmap.IsEmpty() && maBitmap.Convert( eConversion );
}

void BitmapEx::Expand(sal_Int32 nDX, sal_Int32 nDY, bool bExpandTransparent)
{
    if (maBitmap.IsEmpty())
        return;

    bool bRet = maBitmap.Expand( nDX, nDY );

    if ( bRet && !maAlphaMask.IsEmpty() )
    {
        Color aColor( bExpandTransparent ? COL_ALPHA_TRANSPARENT : COL_ALPHA_OPAQUE );
        maAlphaMask.Expand( nDX, nDY, &aColor );
    }

    maBitmapSize = maBitmap.GetSizePixel();

    SAL_WARN_IF(!maAlphaMask.IsEmpty() && maBitmap.GetSizePixel() != maAlphaMask.GetSizePixel(), "vcl",
                "BitmapEx::Expand(): size mismatch for bitmap and alpha mask.");
}

bool BitmapEx::CopyPixel( const tools::Rectangle& rRectDst, const tools::Rectangle& rRectSrc,
                          const BitmapEx& rBmpExSrc )
{
    if( maBitmap.IsEmpty() )
        return false;

    if (!maBitmap.CopyPixel( rRectDst, rRectSrc, rBmpExSrc.maBitmap ))
        return false;

    if( rBmpExSrc.IsAlpha() )
    {
        if( IsAlpha() )
            // cast to use the optimized AlphaMask::CopyPixel
            maAlphaMask.CopyPixel_AlphaOptimized( rRectDst, rRectSrc, rBmpExSrc.maAlphaMask );
        else
        {
            sal_uInt8 nTransparencyOpaque = 0;
            maAlphaMask = AlphaMask(GetSizePixel(), &nTransparencyOpaque);
            maAlphaMask.CopyPixel( rRectDst, rRectSrc, rBmpExSrc.maAlphaMask );
        }
    }
    else if (IsAlpha())
    {
        sal_uInt8 nTransparencyOpaque = 0;
        const AlphaMask aAlphaSrc(rBmpExSrc.GetSizePixel(), &nTransparencyOpaque);

        maAlphaMask.CopyPixel( rRectDst, rRectSrc, aAlphaSrc );
    }

    return true;
}

bool BitmapEx::Erase( const Color& rFillColor )
{
    if (maBitmap.IsEmpty())
        return false;

    if (!maBitmap.Erase(rFillColor))
        return false;

    if (!maAlphaMask.IsEmpty())
    {
        // Respect transparency on fill color
        if (rFillColor.IsTransparent())
            maAlphaMask.Erase(255 - rFillColor.GetAlpha());
        else
            maAlphaMask.Erase(0);
    }

    return true;
}

void BitmapEx::Replace( const Color& rSearchColor, const Color& rReplaceColor )
{
    if (!maBitmap.IsEmpty())
        maBitmap.Replace( rSearchColor, rReplaceColor );
}

void BitmapEx::Replace( const Color* pSearchColors, const Color* pReplaceColors, size_t nColorCount )
{
    if (!maBitmap.IsEmpty())
        maBitmap.Replace( pSearchColors, pReplaceColors, nColorCount, /*pTols*/nullptr );
}

bool BitmapEx::Adjust( short nLuminancePercent, short nContrastPercent,
                       short nChannelRPercent, short nChannelGPercent, short nChannelBPercent,
                       double fGamma, bool bInvert, bool msoBrightness )
{
    return !maBitmap.IsEmpty() && maBitmap.Adjust( nLuminancePercent, nContrastPercent,
                                        nChannelRPercent, nChannelGPercent, nChannelBPercent,
                                        fGamma, bInvert, msoBrightness );
}

void BitmapEx::Draw( OutputDevice* pOutDev, const Point& rDestPt ) const
{
    pOutDev->DrawBitmapEx( rDestPt, *this );
}

void BitmapEx::Draw( OutputDevice* pOutDev,
                     const Point& rDestPt, const Size& rDestSize ) const
{
    pOutDev->DrawBitmapEx( rDestPt, rDestSize, *this );
}

BitmapEx BitmapEx::AutoScaleBitmap(BitmapEx const & aBitmap, const tools::Long aStandardSize)
{
    Point aEmptyPoint(0,0);
    double imgposX = 0;
    double imgposY = 0;
    BitmapEx  aRet = aBitmap;
    double imgOldWidth = aRet.GetSizePixel().Width();
    double imgOldHeight = aRet.GetSizePixel().Height();

    if (imgOldWidth >= aStandardSize || imgOldHeight >= aStandardSize)
    {
        sal_Int32 imgNewWidth = 0;
        sal_Int32 imgNewHeight = 0;
        if (imgOldWidth >= imgOldHeight)
        {
            imgNewWidth = aStandardSize;
            imgNewHeight = sal_Int32(imgOldHeight / (imgOldWidth / aStandardSize) + 0.5);
            imgposX = 0;
            imgposY = (aStandardSize - (imgOldHeight / (imgOldWidth / aStandardSize) + 0.5)) / 2 + 0.5;
        }
        else
        {
            imgNewHeight = aStandardSize;
            imgNewWidth = sal_Int32(imgOldWidth / (imgOldHeight / aStandardSize) + 0.5);
            imgposY = 0;
            imgposX = (aStandardSize - (imgOldWidth / (imgOldHeight / aStandardSize) + 0.5)) / 2 + 0.5;
        }

        Size aScaledSize( imgNewWidth, imgNewHeight );
        aRet.Scale( aScaledSize, BmpScaleFlag::BestQuality );
    }
    else
    {
        imgposX = (aStandardSize - imgOldWidth) / 2 + 0.5;
        imgposY = (aStandardSize - imgOldHeight) / 2 + 0.5;
    }

    Size aStdSize( aStandardSize, aStandardSize );
    tools::Rectangle aRect(aEmptyPoint, aStdSize );

    ScopedVclPtrInstance< VirtualDevice > aVirDevice(*Application::GetDefaultDevice());
    aVirDevice->SetOutputSizePixel( aStdSize );
    aVirDevice->SetFillColor( COL_TRANSPARENT );
    aVirDevice->SetLineColor( COL_TRANSPARENT );

    // Draw a rect into virDevice
    aVirDevice->DrawRect( aRect );
    Point aPointPixel( static_cast<tools::Long>(imgposX), static_cast<tools::Long>(imgposY) );
    aVirDevice->DrawBitmapEx( aPointPixel, aRet );
    aRet = aVirDevice->GetBitmapEx( aEmptyPoint, aStdSize );

    return aRet;
}

sal_uInt8 BitmapEx::GetAlpha(sal_Int32 nX, sal_Int32 nY) const
{
    if(maBitmap.IsEmpty())
        return 0;

    if (nX < 0 || nX >= GetSizePixel().Width() || nY < 0 || nY >= GetSizePixel().Height())
        return 0;

    if (maBitmap.getPixelFormat() == vcl::PixelFormat::N32_BPP)
        return GetPixelColor(nX, nY).GetAlpha();

    sal_uInt8 nAlpha(0);
    if (maAlphaMask.IsEmpty())
    {
        // Not transparent, ergo all covered
        nAlpha = 255;
    }
    else
    {
        BitmapScopedReadAccess pRead(maAlphaMask);
        if(pRead)
        {
            const BitmapColor aBitmapColor(pRead->GetPixel(nY, nX));
            nAlpha = aBitmapColor.GetIndex();
        }
    }
    return nAlpha;
}


Color BitmapEx::GetPixelColor(sal_Int32 nX, sal_Int32 nY) const
{
    BitmapScopedReadAccess pReadAccess( maBitmap );
    assert(pReadAccess);

    BitmapColor aColor = pReadAccess->GetColor(nY, nX);

    if (IsAlpha())
    {
        AlphaMask aAlpha = GetAlphaMask();
        BitmapScopedReadAccess pAlphaReadAccess(aAlpha);
        aColor.SetAlpha(pAlphaReadAccess->GetPixel(nY, nX).GetIndex());
    }
    else if (maBitmap.getPixelFormat() != vcl::PixelFormat::N32_BPP)
    {
        aColor.SetAlpha(255);
    }
    return aColor;
}

// Shift alpha transparent pixels between cppcanvas/ implementations
// and vcl in a generally grotesque and under-performing fashion
bool BitmapEx::Create( const css::uno::Reference< css::rendering::XBitmapCanvas > &xBitmapCanvas,
                       const Size &rSize )
{
    uno::Reference< beans::XFastPropertySet > xFastPropertySet( xBitmapCanvas, uno::UNO_QUERY );
    if( xFastPropertySet )
    {
        // 0 means get BitmapEx
        uno::Any aAny = xFastPropertySet->getFastPropertyValue( 0 );
        std::unique_ptr<BitmapEx> xBitmapEx(reinterpret_cast<BitmapEx*>(*o3tl::doAccess<sal_Int64>(aAny)));
        if( xBitmapEx )
        {
            *this = *xBitmapEx;
            return true;
        }
    }

    std::shared_ptr<SalBitmap> pSalBmp;
    std::shared_ptr<SalBitmap> pSalMask;

    pSalBmp = ImplGetSVData()->mpDefInst->CreateSalBitmap();

    Size aLocalSize(rSize);
    if( pSalBmp->Create( xBitmapCanvas, aLocalSize ) )
    {
        pSalMask = ImplGetSVData()->mpDefInst->CreateSalBitmap();
        if ( pSalMask->Create( xBitmapCanvas, aLocalSize, true ) )
        {
            *this = BitmapEx(Bitmap(std::move(pSalBmp)), Bitmap(std::move(pSalMask)) );
            return true;
        }
        else
        {
            *this = BitmapEx(Bitmap(std::move(pSalBmp)));
            return true;
        }
    }

    return false;
}

namespace
{
    Bitmap impTransformBitmap(
        const Bitmap& rSource,
        const Size& rDestinationSize,
        const basegfx::B2DHomMatrix& rTransform,
        bool bSmooth, bool bAlphaMask)
    {
        Bitmap aDestination(rDestinationSize, vcl::PixelFormat::N24_BPP);
        BitmapScopedWriteAccess xWrite(aDestination);

        if(xWrite)
        {
            BitmapScopedReadAccess xRead(rSource);

            if (xRead)
            {
                const Size aDestinationSizePixel(aDestination.GetSizePixel());

                // tdf#157795 set color to black outside of bitmap bounds
                // Due to commit 81994cb2b8b32453a92bcb011830fcb884f22ff3,
                // transparent areas are now black instead of white.
                // tdf#160831 only set outside color to black for alpha masks
                // The outside color still needs to be white for the content
                // so only apply the fix for tdf#157795 to the alpha mask.
                const BitmapColor aOutside = bAlphaMask ? BitmapColor(0x0, 0x0, 0x0) : BitmapColor(0xff, 0xff, 0xff);

                for(tools::Long y(0); y < aDestinationSizePixel.getHeight(); y++)
                {
                    Scanline pScanline = xWrite->GetScanline( y );
                    for(tools::Long x(0); x < aDestinationSizePixel.getWidth(); x++)
                    {
                        const basegfx::B2DPoint aSourceCoor(rTransform * basegfx::B2DPoint(x, y));

                        if(bSmooth)
                        {
                            xWrite->SetPixelOnData(
                                pScanline,
                                x,
                                xRead->GetInterpolatedColorWithFallback(
                                    aSourceCoor.getY(),
                                    aSourceCoor.getX(),
                                    aOutside));
                        }
                        else
                        {
                            // this version does the correct <= 0.0 checks, so no need
                            // to do the static_cast< sal_Int32 > self and make an error
                            xWrite->SetPixelOnData(
                                pScanline,
                                x,
                                xRead->GetColorWithFallback(
                                    aSourceCoor.getY(),
                                    aSourceCoor.getX(),
                                    aOutside));
                        }
                    }
                }
            }
        }
        xWrite.reset();

        rSource.AdaptBitCount(aDestination);

        return aDestination;
    }

    /// Decides if rTransformation needs smoothing or not (e.g. 180 deg rotation doesn't need it).
    bool implTransformNeedsSmooth(const basegfx::B2DHomMatrix& rTransformation)
    {
        basegfx::B2DVector aScale, aTranslate;
        double fRotate, fShearX;
        rTransformation.decompose(aScale, aTranslate, fRotate, fShearX);
        if (aScale != basegfx::B2DVector(1, 1))
        {
            return true;
        }

        fRotate = fmod( fRotate, 2 * M_PI );
        if (fRotate < 0)
        {
            fRotate += 2 * M_PI;
        }
        if (!rtl::math::approxEqual(fRotate, 0)
            && !rtl::math::approxEqual(fRotate, M_PI_2)
            && !rtl::math::approxEqual(fRotate, M_PI)
            && !rtl::math::approxEqual(fRotate, 3 * M_PI_2))
        {
            return true;
        }

        if (!rtl::math::approxEqual(fShearX, 0))
        {
            return true;
        }

        return false;
    }
} // end of anonymous namespace

BitmapEx BitmapEx::TransformBitmapEx(
    double fWidth,
    double fHeight,
    const basegfx::B2DHomMatrix& rTransformation) const
{
    if(fWidth <= 1 || fHeight <= 1)
        return BitmapEx();

    // force destination to 24 bit, we want to smooth output
    const Size aDestinationSize(basegfx::fround<tools::Long>(fWidth), basegfx::fround<tools::Long>(fHeight));
    bool bSmooth = implTransformNeedsSmooth(rTransformation);
    const Bitmap aDestination(impTransformBitmap(GetBitmap(), aDestinationSize, rTransformation, bSmooth, false));

    // create mask
    if(IsAlpha())
    {
        const Bitmap aAlpha(impTransformBitmap(GetAlphaMask().GetBitmap(), aDestinationSize, rTransformation, bSmooth, true));
        return BitmapEx(aDestination, AlphaMask(aAlpha));
    }

    return BitmapEx(aDestination);
}

BitmapEx BitmapEx::getTransformed(
    const basegfx::B2DHomMatrix& rTransformation,
    const basegfx::B2DRange& rVisibleRange,
    double fMaximumArea) const
{
    if (IsEmpty())
        return BitmapEx();

    const sal_uInt32 nSourceWidth(GetSizePixel().Width());
    const sal_uInt32 nSourceHeight(GetSizePixel().Height());

    if (!nSourceWidth || !nSourceHeight)
        return BitmapEx();

    // Get aOutlineRange
    basegfx::B2DRange aOutlineRange(0.0, 0.0, 1.0, 1.0);

    aOutlineRange.transform(rTransformation);

    // create visible range from it by moving from relative to absolute
    basegfx::B2DRange aVisibleRange(rVisibleRange);

    aVisibleRange.transform(
        basegfx::utils::createScaleTranslateB2DHomMatrix(
            aOutlineRange.getRange(),
            aOutlineRange.getMinimum()));

    // get target size (which is visible range's size)
    double fWidth(aVisibleRange.getWidth());
    double fHeight(aVisibleRange.getHeight());

    if (fWidth < 1.0 || fHeight < 1.0)
        return BitmapEx();

    // test if discrete size (pixel) maybe too big and limit it
    const double fArea(fWidth * fHeight);
    const bool bNeedToReduce(basegfx::fTools::more(fArea, fMaximumArea));
    double fReduceFactor(1.0);

    if(bNeedToReduce)
    {
        fReduceFactor = sqrt(fMaximumArea / fArea);
        fWidth *= fReduceFactor;
        fHeight *= fReduceFactor;
    }

    // Build complete transform from source pixels to target pixels.
    // Start by scaling from source pixel size to unit coordinates
    basegfx::B2DHomMatrix aTransform(
        basegfx::utils::createScaleB2DHomMatrix(
            1.0 / nSourceWidth,
            1.0 / nSourceHeight));

    // multiply with given transform which leads from unit coordinates inside
    // aOutlineRange
    aTransform = rTransformation * aTransform;

    // subtract top-left of absolute VisibleRange
    aTransform.translate(
        -aVisibleRange.getMinX(),
        -aVisibleRange.getMinY());

    // scale to target pixels (if needed)
    if(bNeedToReduce)
    {
        aTransform.scale(fReduceFactor, fReduceFactor);
    }

    // invert to get transformation from target pixel coordinates to source pixels
    aTransform.invert();

    // create bitmap using source, destination and linear back-transformation
    return TransformBitmapEx(fWidth, fHeight, aTransform);
}

namespace
{
class BufferedData_ModifiedBitmapEx : public basegfx::SystemDependentData
{
    Bitmap maChangedBitmap;
    basegfx::BColorModifierStack maBColorModifierStack;

public:
    BufferedData_ModifiedBitmapEx(
        const Bitmap& rChangedBitmap,
        const basegfx::BColorModifierStack& rBColorModifierStack)
    : basegfx::SystemDependentData(
        Application::GetSystemDependentDataManager(),
        basegfx::SDD_Type::SDDType_ModifiedBitmapEx)
    , maChangedBitmap(rChangedBitmap)
    , maBColorModifierStack(rBColorModifierStack)
    {
    }

    const Bitmap& getChangedBitmap() const { return maChangedBitmap; }
    const basegfx::BColorModifierStack& getBColorModifierStack() const { return maBColorModifierStack; }

    virtual sal_Int64 estimateUsageInBytes() const override;
};

sal_Int64 BufferedData_ModifiedBitmapEx::estimateUsageInBytes() const
{
    return maChangedBitmap.GetSizeBytes();
}
}

BitmapEx BitmapEx::ModifyBitmapEx(const basegfx::BColorModifierStack& rBColorModifierStack) const
{
    if (0 == rBColorModifierStack.count())
    {
        // no modifiers, done
        return *this;
    }

    // check for BColorModifier_replace at the top of the stack
    const basegfx::BColorModifierSharedPtr& rLastModifier(rBColorModifierStack.getBColorModifier(rBColorModifierStack.count() - 1));
    const basegfx::BColorModifier_replace* pLastModifierReplace(dynamic_cast<const basegfx::BColorModifier_replace*>(rLastModifier.get()));

    if (nullptr != pLastModifierReplace && !IsAlpha())
    {
        // at the top of the stack we have a BColorModifier_replace -> no Bitmap needed,
        // representation can be replaced by filled colored polygon. signal the caller
        // about that by returning empty BitmapEx
        return BitmapEx();
    }

    const basegfx::SystemDependentDataHolder* pHolder(GetBitmap().accessSystemDependentDataHolder());
    std::shared_ptr<BufferedData_ModifiedBitmapEx> pBufferedData_ModifiedBitmapEx;

    if (nullptr != pHolder)
    {
        // try to access SystemDependentDataHolder and buffered data
        pBufferedData_ModifiedBitmapEx = std::static_pointer_cast<BufferedData_ModifiedBitmapEx>(
            pHolder->getSystemDependentData(basegfx::SDD_Type::SDDType_ModifiedBitmapEx));

        if (nullptr != pBufferedData_ModifiedBitmapEx
            && !(pBufferedData_ModifiedBitmapEx->getBColorModifierStack() == rBColorModifierStack))
        {
            // BColorModifierStack is different -> data invalid
            pBufferedData_ModifiedBitmapEx = nullptr;
        }

        if (nullptr != pBufferedData_ModifiedBitmapEx)
        {
            // found existing instance of modified Bitmap, return reused/buffered result
            if(IsAlpha())
                return BitmapEx(pBufferedData_ModifiedBitmapEx->getChangedBitmap(), GetAlphaMask());
            return BitmapEx(pBufferedData_ModifiedBitmapEx->getChangedBitmap());
        }
    }

    // have to create modified Bitmap
    Bitmap aChangedBitmap(GetBitmap());

    if (nullptr != pLastModifierReplace)
    {
        // special case -> we have BColorModifier_replace but Alpha channel
        if (vcl::isPalettePixelFormat(aChangedBitmap.getPixelFormat()))
        {
            // For e.g. 8bit Bitmaps, the nearest color to the given erase color is
            // determined and used -> this may be different from what is wanted here.
            // Better create a new bitmap with the needed color explicitly.
            BitmapScopedReadAccess xReadAccess(aChangedBitmap);
            SAL_WARN_IF(!xReadAccess, "vcl", "Got no Bitmap ReadAccess ?!?");

            if(xReadAccess)
            {
                BitmapPalette aNewPalette(xReadAccess->GetPalette());
                aNewPalette[0] = BitmapColor(Color(pLastModifierReplace->getBColor()));
                aChangedBitmap = Bitmap(
                    aChangedBitmap.GetSizePixel(),
                    aChangedBitmap.getPixelFormat(),
                    &aNewPalette);
            }
        }
        else
        {
            // clear bitmap with dest color
            aChangedBitmap.Erase(Color(pLastModifierReplace->getBColor()));
        }
    }
    else
    {
        BitmapScopedWriteAccess xContent(aChangedBitmap);

        if(xContent)
        {
            const double fConvertColor(1.0 / 255.0);

            if(xContent->HasPalette())
            {
                const sal_uInt16 nCount(xContent->GetPaletteEntryCount());

                for(sal_uInt16 b(0); b < nCount; b++)
                {
                    const BitmapColor& rCol = xContent->GetPaletteColor(b);
                    const basegfx::BColor aBSource(
                        rCol.GetRed() * fConvertColor,
                        rCol.GetGreen() * fConvertColor,
                        rCol.GetBlue() * fConvertColor);
                    const basegfx::BColor aBDest(rBColorModifierStack.getModifiedColor(aBSource));
                    xContent->SetPaletteColor(b, BitmapColor(Color(aBDest)));
                }
            }
            else if(ScanlineFormat::N24BitTcBgr == xContent->GetScanlineFormat())
            {
                for(tools::Long y(0); y < xContent->Height(); y++)
                {
                    Scanline pScan = xContent->GetScanline(y);

                    for(tools::Long x(0); x < xContent->Width(); x++)
                    {
                        const basegfx::BColor aBSource(
                            *(pScan + 2)* fConvertColor,
                            *(pScan + 1) * fConvertColor,
                            *pScan * fConvertColor);
                        const basegfx::BColor aBDest(rBColorModifierStack.getModifiedColor(aBSource));
                        *pScan++ = static_cast< sal_uInt8 >(aBDest.getBlue() * 255.0);
                        *pScan++ = static_cast< sal_uInt8 >(aBDest.getGreen() * 255.0);
                        *pScan++ = static_cast< sal_uInt8 >(aBDest.getRed() * 255.0);
                    }
                }
            }
            else if(ScanlineFormat::N24BitTcRgb == xContent->GetScanlineFormat())
            {
                for(tools::Long y(0); y < xContent->Height(); y++)
                {
                    Scanline pScan = xContent->GetScanline(y);

                    for(tools::Long x(0); x < xContent->Width(); x++)
                    {
                        const basegfx::BColor aBSource(
                            *pScan * fConvertColor,
                            *(pScan + 1) * fConvertColor,
                            *(pScan + 2) * fConvertColor);
                        const basegfx::BColor aBDest(rBColorModifierStack.getModifiedColor(aBSource));
                        *pScan++ = static_cast< sal_uInt8 >(aBDest.getRed() * 255.0);
                        *pScan++ = static_cast< sal_uInt8 >(aBDest.getGreen() * 255.0);
                        *pScan++ = static_cast< sal_uInt8 >(aBDest.getBlue() * 255.0);
                    }
                }
            }
            else
            {
                for(tools::Long y(0); y < xContent->Height(); y++)
                {
                    Scanline pScanline = xContent->GetScanline( y );
                    for(tools::Long x(0); x < xContent->Width(); x++)
                    {
                        const BitmapColor aBMCol(xContent->GetColor(y, x));
                        const basegfx::BColor aBSource(
                            static_cast<double>(aBMCol.GetRed()) * fConvertColor,
                            static_cast<double>(aBMCol.GetGreen()) * fConvertColor,
                            static_cast<double>(aBMCol.GetBlue()) * fConvertColor);
                        const basegfx::BColor aBDest(rBColorModifierStack.getModifiedColor(aBSource));

                        xContent->SetPixelOnData(pScanline, x, BitmapColor(Color(aBDest)));
                    }
                }
            }
        }
    }

    if (nullptr != pHolder)
    {
        // create new BufferedData_ModifiedBitmapEx (should be nullptr here)
        if (nullptr == pBufferedData_ModifiedBitmapEx)
        {
            pBufferedData_ModifiedBitmapEx = std::make_shared<BufferedData_ModifiedBitmapEx>(aChangedBitmap, rBColorModifierStack);
        }

        // register it, evtl. it's a new one
        basegfx::SystemDependentData_SharedPtr r2(pBufferedData_ModifiedBitmapEx);
        const_cast<basegfx::SystemDependentDataHolder*>(pHolder)->addOrReplaceSystemDependentData(r2);
    }

    // return result
    if(IsAlpha())
        return BitmapEx(aChangedBitmap, GetAlphaMask());
    return BitmapEx(aChangedBitmap);
}

BitmapEx createBlendFrame(
    const Size& rSize,
    sal_uInt8 nAlpha,
    Color aColorTopLeft,
    Color aColorBottomRight)
{
    const sal_uInt32 nW(rSize.Width());
    const sal_uInt32 nH(rSize.Height());

    if(nW || nH)
    {
        Color aColTopRight(aColorTopLeft);
        Color aColBottomLeft(aColorTopLeft);
        const sal_uInt32 nDE(nW + nH);

        aColTopRight.Merge(aColorBottomRight, 255 - sal_uInt8((nW * 255) / nDE));
        aColBottomLeft.Merge(aColorBottomRight, 255 - sal_uInt8((nH * 255) / nDE));

        return createBlendFrame(rSize, nAlpha, aColorTopLeft, aColTopRight, aColorBottomRight, aColBottomLeft);
    }

    return BitmapEx();
}

BitmapEx createBlendFrame(
    const Size& rSize,
    sal_uInt8 nAlpha,
    Color aColorTopLeft,
    Color aColorTopRight,
    Color aColorBottomRight,
    Color aColorBottomLeft)
{
    // FIXME the call sites are actually passing in transparency
    nAlpha = 255 - nAlpha;
    BlendFrameCache* pBlendFrameCache = ImplGetBlendFrameCache();

    if(pBlendFrameCache->m_aLastSize == rSize
        && pBlendFrameCache->m_nLastAlpha == nAlpha
        && pBlendFrameCache->m_aLastColorTopLeft == aColorTopLeft
        && pBlendFrameCache->m_aLastColorTopRight == aColorTopRight
        && pBlendFrameCache->m_aLastColorBottomRight == aColorBottomRight
        && pBlendFrameCache->m_aLastColorBottomLeft == aColorBottomLeft)
    {
        return pBlendFrameCache->m_aLastResult;
    }

    pBlendFrameCache->m_aLastSize = rSize;
    pBlendFrameCache->m_nLastAlpha = nAlpha;
    pBlendFrameCache->m_aLastColorTopLeft = aColorTopLeft;
    pBlendFrameCache->m_aLastColorTopRight = aColorTopRight;
    pBlendFrameCache->m_aLastColorBottomRight = aColorBottomRight;
    pBlendFrameCache->m_aLastColorBottomLeft = aColorBottomLeft;
    pBlendFrameCache->m_aLastResult.Clear();

    const tools::Long nW(rSize.Width());
    const tools::Long nH(rSize.Height());

    if(nW > 1 && nH > 1)
    {
        sal_uInt8 aEraseTrans(0xff);
        Bitmap aContent(rSize, vcl::PixelFormat::N24_BPP);
        AlphaMask aAlpha(rSize, &aEraseTrans);

        aContent.Erase(COL_BLACK);

        BitmapScopedWriteAccess pContent(aContent);
        BitmapScopedWriteAccess pAlpha(aAlpha);

        if(pContent && pAlpha)
        {
            tools::Long x(0);
            tools::Long y(0);
            Scanline pScanContent = pContent->GetScanline( 0 );
            Scanline pScanAlpha = pContent->GetScanline( 0 );

            // x == 0, y == 0, top-left corner
            pContent->SetPixelOnData(pScanContent, 0, aColorTopLeft);
            pAlpha->SetPixelOnData(pScanAlpha, 0, BitmapColor(nAlpha));

            // y == 0, top line left to right
            for(x = 1; x < nW - 1; x++)
            {
                Color aMix(aColorTopLeft);

                aMix.Merge(aColorTopRight, 255 - sal_uInt8((x * 255) / nW));
                pContent->SetPixelOnData(pScanContent, x, aMix);
                pAlpha->SetPixelOnData(pScanAlpha, x, BitmapColor(nAlpha));
            }

            // x == nW - 1, y == 0, top-right corner
            // #i123690# Caution! When nW is 1, x == nW is possible (!)
            if(x < nW)
            {
                pContent->SetPixelOnData(pScanContent, x, aColorTopRight);
                pAlpha->SetPixelOnData(pScanAlpha, x, BitmapColor(nAlpha));
            }

            // x == 0 and nW - 1, left and right line top-down
            for(y = 1; y < nH - 1; y++)
            {
                pScanContent = pContent->GetScanline( y );
                pScanAlpha = pContent->GetScanline( y );
                Color aMixA(aColorTopLeft);

                aMixA.Merge(aColorBottomLeft, 255 - sal_uInt8((y * 255) / nH));
                pContent->SetPixelOnData(pScanContent, 0, aMixA);
                pAlpha->SetPixelOnData(pScanAlpha, 0, BitmapColor(nAlpha));

                // #i123690# Caution! When nW is 1, x == nW is possible (!)
                if(x < nW)
                {
                    Color aMixB(aColorTopRight);

                    aMixB.Merge(aColorBottomRight, 255 - sal_uInt8((y * 255) / nH));
                    pContent->SetPixelOnData(pScanContent, x, aMixB);
                    pAlpha->SetPixelOnData(pScanAlpha, x, BitmapColor(nAlpha));
                }
            }

            // #i123690# Caution! When nH is 1, y == nH is possible (!)
            if(y < nH)
            {
                // x == 0, y == nH - 1, bottom-left corner
                pContent->SetPixelOnData(pScanContent, 0, aColorBottomLeft);
                pAlpha->SetPixelOnData(pScanAlpha, 0, BitmapColor(nAlpha));

                // y == nH - 1, bottom line left to right
                for(x = 1; x < nW - 1; x++)
                {
                    Color aMix(aColorBottomLeft);

                    aMix.Merge(aColorBottomRight, 255 - sal_uInt8(((x - 0)* 255) / nW));
                    pContent->SetPixelOnData(pScanContent, x, aMix);
                    pAlpha->SetPixelOnData(pScanAlpha, x, BitmapColor(nAlpha));
                }

                // x == nW - 1, y == nH - 1, bottom-right corner
                // #i123690# Caution! When nW is 1, x == nW is possible (!)
                if(x < nW)
                {
                    pContent->SetPixelOnData(pScanContent, x, aColorBottomRight);
                    pAlpha->SetPixelOnData(pScanAlpha, x, BitmapColor(nAlpha));
                }
            }

            pContent.reset();
            pAlpha.reset();

            pBlendFrameCache->m_aLastResult = BitmapEx(aContent, aAlpha);
        }
    }

    return pBlendFrameCache->m_aLastResult;
}

void BitmapEx::Replace(const Color& rSearchColor,
                           const Color& rReplaceColor,
                           sal_uInt8 nTolerance)
{
    maBitmap.Replace(rSearchColor, rReplaceColor, nTolerance);
}

void BitmapEx::Replace( const Color* pSearchColors,
                        const Color* pReplaceColors,
                        size_t nColorCount,
                        sal_uInt8 const * pTols )
{
    maBitmap.Replace( pSearchColors, pReplaceColors, nColorCount, pTols );
}

void BitmapEx::ReplaceTransparency(const Color& rColor)
{
    if( IsAlpha() )
    {
        maBitmap.Replace( GetAlphaMask(), rColor );
        maAlphaMask = Bitmap();
        maBitmapSize = maBitmap.GetSizePixel();
    }
}

static Bitmap DetectEdges( const Bitmap& rBmp )
{
    constexpr sal_uInt8 cEdgeDetectThreshold = 128;
    const Size  aSize( rBmp.GetSizePixel() );

    if( ( aSize.Width() <= 2 ) || ( aSize.Height() <= 2 ) )
        return rBmp;

    Bitmap aWorkBmp( rBmp );

    if( !aWorkBmp.Convert( BmpConversion::N8BitGreys ) )
        return rBmp;

    ScopedVclPtr<VirtualDevice> pVirDev(VclPtr<VirtualDevice>::Create());
    pVirDev->SetOutputSizePixel(aSize);
    BitmapScopedReadAccess pReadAcc(aWorkBmp);
    if( !pReadAcc )
        return rBmp;

    const tools::Long          nWidth = aSize.Width();
    const tools::Long          nWidth2 = nWidth - 2;
    const tools::Long          nHeight = aSize.Height();
    const tools::Long          nHeight2 = nHeight - 2;
    const tools::Long          lThres2 = static_cast<tools::Long>(cEdgeDetectThreshold) * cEdgeDetectThreshold;
    tools::Long                nSum1;
    tools::Long                nSum2;
    tools::Long                lGray;

    // initialize border with white pixels
    pVirDev->SetLineColor( COL_WHITE );
    pVirDev->DrawLine( Point(), Point( nWidth - 1, 0L ) );
    pVirDev->DrawLine( Point( nWidth - 1, 0L ), Point( nWidth - 1, nHeight - 1 ) );
    pVirDev->DrawLine( Point( nWidth - 1, nHeight - 1 ), Point( 0L, nHeight - 1 ) );
    pVirDev->DrawLine( Point( 0, nHeight - 1 ), Point() );

    for( tools::Long nY = 0, nY1 = 1, nY2 = 2; nY < nHeight2; nY++, nY1++, nY2++ )
    {
        Scanline pScanlineRead = pReadAcc->GetScanline( nY );
        Scanline pScanlineRead1 = pReadAcc->GetScanline( nY1 );
        Scanline pScanlineRead2 = pReadAcc->GetScanline( nY2 );
        for( tools::Long nX = 0, nXDst = 1, nXTmp; nX < nWidth2; nX++, nXDst++ )
        {
            nXTmp = nX;

            nSum2 = pReadAcc->GetIndexFromData( pScanlineRead, nXTmp++ );
            nSum1 = -nSum2;
            nSum2 += static_cast<tools::Long>(pReadAcc->GetIndexFromData( pScanlineRead, nXTmp++ )) << 1;
            lGray = pReadAcc->GetIndexFromData( pScanlineRead, nXTmp );
            nSum1 += lGray;
            nSum2 += lGray;

            nSum1 += static_cast<tools::Long>(pReadAcc->GetIndexFromData( pScanlineRead1, nXTmp )) << 1;
            nXTmp -= 2;
            nSum1 -= static_cast<tools::Long>(pReadAcc->GetIndexFromData( pScanlineRead1, nXTmp )) << 1;

            lGray = -static_cast<tools::Long>(pReadAcc->GetIndexFromData( pScanlineRead2, nXTmp++ ));
            nSum1 += lGray;
            nSum2 += lGray;
            nSum2 -= static_cast<tools::Long>(pReadAcc->GetIndexFromData( pScanlineRead2, nXTmp++ )) << 1;
            lGray = static_cast<tools::Long>(pReadAcc->GetIndexFromData( pScanlineRead2, nXTmp ));
            nSum1 += lGray;
            nSum2 -= lGray;

            if( ( nSum1 * nSum1 + nSum2 * nSum2 ) < lThres2 )
                pVirDev->DrawPixel( Point(nXDst, nY), COL_WHITE );
            else
                pVirDev->DrawPixel( Point(nXDst, nY), COL_BLACK );
        }
    }

    pReadAcc.reset();

    Bitmap aRetBmp = pVirDev->GetBitmap(Point(0,0), aSize);

    if( aRetBmp.IsEmpty() )
        aRetBmp = rBmp;
    else
    {
        aRetBmp.SetPrefMapMode( rBmp.GetPrefMapMode() );
        aRetBmp.SetPrefSize( rBmp.GetPrefSize() );
    }

    return aRetBmp;
}

/** Get contours in image */
tools::Polygon  BitmapEx::GetContour( bool bContourEdgeDetect,
                                    const tools::Rectangle* pWorkRectPixel )
{
    Bitmap aWorkBmp;
    tools::Rectangle   aWorkRect( Point(), maBitmap.GetSizePixel() );

    if( pWorkRectPixel )
        aWorkRect.Intersection( *pWorkRectPixel );

    aWorkRect.Normalize();

    if ((aWorkRect.GetWidth() <= 4) || (aWorkRect.GetHeight() <= 4))
        return tools::Polygon();

    // if the flag is set, we need to detect edges
    if( bContourEdgeDetect )
        aWorkBmp = DetectEdges( maBitmap );
    else
        aWorkBmp = maBitmap;

    BitmapScopedReadAccess pAcc(aWorkBmp);

    const tools::Long nWidth = pAcc ? pAcc->Width() : 0;
    const tools::Long nHeight = pAcc ? pAcc->Height() : 0;

    if (!pAcc || !nWidth || !nHeight)
        return tools::Polygon();

    const tools::Long          nStartX1 = aWorkRect.Left() + 1;
    const tools::Long          nEndX1 = aWorkRect.Right();
    const tools::Long          nStartX2 = nEndX1 - 1;
    const tools::Long          nStartY1 = aWorkRect.Top() + 1;
    const tools::Long          nEndY1 = aWorkRect.Bottom();

    sal_uInt16              nPolyPos = 0;

    // tdf#161833 treat semi-transparent pixels as opaque
    // Limiting the contour wrapping polygon to only opaque pixels
    // causes clipping of any shadows or other semi-transparent
    // areas in the image. So, instead of testing for fully opaque
    // pixels, treat pixels that are not fully transparent as opaque.
    // tdf#162062 only apply fix for tdf#161833 if there is a palette
    const BitmapColor   aTransparent = pAcc->GetBestMatchingColor( pAcc->HasPalette() ? COL_ALPHA_TRANSPARENT : COL_ALPHA_OPAQUE );

    std::unique_ptr<Point[]> pPoints1;
    std::unique_ptr<Point[]> pPoints2;

    pPoints1.reset(new Point[ nHeight ]);
    pPoints2.reset(new Point[ nHeight ]);

    for (tools::Long nY = nStartY1; nY < nEndY1; nY++ )
    {
        tools::Long nX = nStartX1;
        Scanline pScanline = pAcc->GetScanline( nY );

        // scan row from left to right
        while( nX < nEndX1 )
        {
            if( aTransparent != pAcc->GetPixelFromData( pScanline, nX ) )
            {
                pPoints1[ nPolyPos ] = Point( nX, nY );
                nX = nStartX2;

                // this loop always breaks eventually as there is at least one pixel
                while( true )
                {
                    if( aTransparent != pAcc->GetPixelFromData( pScanline, nX ) )
                    {
                        pPoints2[ nPolyPos ] = Point( nX, nY );
                        break;
                    }

                    nX--;
                }

                nPolyPos++;
                break;
            }

            nX++;
        }
    }

    const sal_uInt16 nNewSize1 = nPolyPos << 1;

    tools::Polygon aRetPoly(nPolyPos, pPoints1.get());
    aRetPoly.SetSize( nNewSize1 + 1 );
    aRetPoly[ nNewSize1 ] = aRetPoly[ 0 ];

    for( sal_uInt16 j = nPolyPos; nPolyPos < nNewSize1; )
    {
        aRetPoly[ nPolyPos++ ] = pPoints2[ --j ];
    }

    Size const& rPrefSize = aWorkBmp.GetPrefSize();
    const double fFactorX = static_cast<double>(rPrefSize.Width()) / nWidth;
    const double fFactorY = static_cast<double>(rPrefSize.Height()) / nHeight;

    if( ( fFactorX != 0. ) && ( fFactorY != 0. ) )
        aRetPoly.Scale( fFactorX, fFactorY );

    return aRetPoly;
}

void BitmapEx::ChangeColorAlpha( sal_uInt8 cIndexFrom, sal_Int8 nAlphaTo )
{
    AlphaMask aAlphaMask(GetAlphaMask());
    BitmapScopedWriteAccess pAlphaWriteAccess(aAlphaMask);
    BitmapScopedReadAccess pReadAccess(maBitmap);
    assert( pReadAccess.get() && pAlphaWriteAccess.get() );
    if ( !(pReadAccess.get() && pAlphaWriteAccess.get()) )
        return;

    for ( tools::Long nY = 0; nY < pReadAccess->Height(); nY++ )
    {
        Scanline pScanline = pAlphaWriteAccess->GetScanline( nY );
        Scanline pScanlineRead = pReadAccess->GetScanline( nY );
        for ( tools::Long nX = 0; nX < pReadAccess->Width(); nX++ )
        {
            const sal_uInt8 cIndex = pReadAccess->GetPixelFromData( pScanlineRead, nX ).GetIndex();
            if ( cIndex == cIndexFrom )
                pAlphaWriteAccess->SetPixelOnData( pScanline, nX, BitmapColor(nAlphaTo) );
        }
    }
    *this = BitmapEx( GetBitmap(), aAlphaMask );
}

void BitmapEx::AdjustTransparency(sal_uInt8 cTrans)
{
    AlphaMask   aAlpha;

    if (!IsAlpha())
    {
        aAlpha = AlphaMask(GetSizePixel(), &cTrans);
    }
    else
    {
        aAlpha = GetAlphaMask();
        BitmapScopedWriteAccess pA(aAlpha);
        assert(pA);

        if( !pA )
            return;

        sal_uLong nTrans = cTrans;
        const tools::Long  nWidth = pA->Width(), nHeight = pA->Height();

        if( pA->GetScanlineFormat() == ScanlineFormat::N8BitPal )
        {
            for( tools::Long nY = 0; nY < nHeight; nY++ )
            {
                Scanline pAScan = pA->GetScanline( nY );

                for( tools::Long nX = 0; nX < nWidth; nX++ )
                {
                    sal_uLong nNewTrans = nTrans + (255 - *pAScan);
                    // clamp to 255
                    nNewTrans = ( nNewTrans & 0xffffff00 ) ? 255 : nNewTrans;
                    *pAScan++ = static_cast<sal_uInt8>( 255 - nNewTrans );
                }
            }
        }
        else
        {
            BitmapColor aAlphaValue( 0 );

            for( tools::Long nY = 0; nY < nHeight; nY++ )
            {
                Scanline pScanline = pA->GetScanline( nY );
                for( tools::Long nX = 0; nX < nWidth; nX++ )
                {
                    sal_uLong nNewTrans = nTrans + (255 - pA->GetIndexFromData( pScanline, nX ));
                    // clamp to 255
                    nNewTrans = ( nNewTrans & 0xffffff00 ) ? 255 : nNewTrans;
                    // convert back to alpha
                    aAlphaValue.SetIndex( static_cast<sal_uInt8>(255 - nNewTrans) );
                    pA->SetPixelOnData( pScanline, nX, aAlphaValue );
                }
            }
        }
    }
    *this = BitmapEx( GetBitmap(), aAlpha );
}

void BitmapEx::CombineMaskOr(Color maskColor, sal_uInt8 nTol)
{
    AlphaMask aNewMask = maBitmap.CreateAlphaMask( maskColor, nTol );
    if ( IsAlpha() )
         aNewMask.AlphaCombineOr( maAlphaMask );
    maAlphaMask = std::move(aNewMask);
}

/**
 * Retrieves the color model data we need for the XImageConsumer stuff.
 */
void  BitmapEx::GetColorModel(css::uno::Sequence< sal_Int32 >& rRGBPalette,
        sal_uInt32& rnRedMask, sal_uInt32& rnGreenMask, sal_uInt32& rnBlueMask, sal_uInt32& rnAlphaMask, sal_uInt32& rnTransparencyIndex,
        sal_uInt32& rnWidth, sal_uInt32& rnHeight, sal_uInt8& rnBitCount)
{
    BitmapScopedReadAccess pReadAccess( maBitmap );
    assert( pReadAccess );

    if( pReadAccess->HasPalette() )
    {
        sal_uInt16 nPalCount = pReadAccess->GetPaletteEntryCount();

        if( nPalCount )
        {
            rRGBPalette = css::uno::Sequence< sal_Int32 >( nPalCount + 1 );

            sal_Int32* pTmp = rRGBPalette.getArray();

            for( sal_uInt32 i = 0; i < nPalCount; i++, pTmp++ )
            {
                const BitmapColor& rCol = pReadAccess->GetPaletteColor( static_cast<sal_uInt16>(i) );

                *pTmp = static_cast<sal_Int32>(rCol.GetRed()) << sal_Int32(24);
                *pTmp |= static_cast<sal_Int32>(rCol.GetGreen()) << sal_Int32(16);
                *pTmp |= static_cast<sal_Int32>(rCol.GetBlue()) << sal_Int32(8);
                *pTmp |= sal_Int32(0x000000ffL);
            }

            if( IsAlpha() )
            {
                // append transparent entry
                *pTmp = sal_Int32(0xffffff00L);
                rnTransparencyIndex = nPalCount;
                nPalCount++;
            }
            else
                rnTransparencyIndex = 0;
        }
    }
    else
    {
        rnRedMask = 0xff000000UL;
        rnGreenMask = 0x00ff0000UL;
        rnBlueMask = 0x0000ff00UL;
        rnAlphaMask = 0x000000ffUL;
        rnTransparencyIndex = 0;
    }

    rnWidth = pReadAccess->Width();
    rnHeight = pReadAccess->Height();
    rnBitCount = pReadAccess->GetBitCount();
}

void BitmapEx::DumpAsPng(const char* pFileName) const
{
    OUString sPath;
    if (pFileName)
    {
        sPath = OUString::fromUtf8(pFileName);
    }
    else if (const char* pEnv = std::getenv("VCL_DUMP_BMP_PATH"))
    {
        sPath = OUString::fromUtf8(pEnv);
    }
    else
    {
        sPath = "file:///tmp/bitmap.png";
    }
    SvFileStream aStream(sPath, StreamMode::STD_READWRITE | StreamMode::TRUNC);
    assert(aStream.good());
    vcl::PngImageWriter aWriter(aStream);
    aWriter.write(*this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
