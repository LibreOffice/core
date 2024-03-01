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

#include <config_features.h>

#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <tools/helpers.hxx>

#include <utility>
#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/outdev.hxx>

#include <svdata.hxx>
#include <salinst.hxx>
#include <salbmp.hxx>
#if HAVE_FEATURE_SKIA
#include <vcl/skia/SkiaHelper.hxx>
#endif
#include <vcl/BitmapMonochromeFilter.hxx>

#include <bitmap/BitmapScaleSuperFilter.hxx>
#include <bitmap/BitmapScaleConvolutionFilter.hxx>
#include <bitmap/BitmapFastScaleFilter.hxx>
#include <bitmap/BitmapInterpolateScaleFilter.hxx>
#include <vcl/BitmapWriteAccess.hxx>
#include <bitmap/impoctree.hxx>
#include <bitmap/Octree.hxx>

#include "floyd.hxx"

#include <math.h>
#include <algorithm>
#include <memory>

#ifdef DBG_UTIL
#include <cstdlib>
#include <tools/stream.hxx>
#include <vcl/graphicfilter.hxx>
#endif

Bitmap::Bitmap()
{
}

Bitmap::Bitmap(const Bitmap& rBitmap)
    : mxSalBmp(rBitmap.mxSalBmp)
    , maPrefMapMode(rBitmap.maPrefMapMode)
    , maPrefSize(rBitmap.maPrefSize)
{
}

Bitmap::Bitmap(std::shared_ptr<SalBitmap> pSalBitmap)
    : mxSalBmp(std::move(pSalBitmap))
    , maPrefMapMode(MapMode(MapUnit::MapPixel))
    , maPrefSize(mxSalBmp->GetSize())
{
}

Bitmap::Bitmap( const Size& rSizePixel, vcl::PixelFormat ePixelFormat, const BitmapPalette* pPal )
{
    if (!(rSizePixel.Width() && rSizePixel.Height()))
        return;

    switch (ePixelFormat)
    {
        case vcl::PixelFormat::N8_BPP:
        {
            static const BitmapPalette aPalN8_BPP = [] {
                BitmapPalette aPal(1 << sal_uInt16(vcl::PixelFormat::N8_BPP));
                aPal[ 0 ] = COL_BLACK;
                aPal[ 1 ] = COL_BLUE;
                aPal[ 2 ] = COL_GREEN;
                aPal[ 3 ] = COL_CYAN;
                aPal[ 4 ] = COL_RED;
                aPal[ 5 ] = COL_MAGENTA;
                aPal[ 6 ] = COL_BROWN;
                aPal[ 7 ] = COL_GRAY;
                aPal[ 8 ] = COL_LIGHTGRAY;
                aPal[ 9 ] = COL_LIGHTBLUE;
                aPal[ 10 ] = COL_LIGHTGREEN;
                aPal[ 11 ] = COL_LIGHTCYAN;
                aPal[ 12 ] = COL_LIGHTRED;
                aPal[ 13 ] = COL_LIGHTMAGENTA;
                aPal[ 14 ] = COL_YELLOW;
                aPal[ 15 ] = COL_WHITE;

                // Create dither palette
                sal_uInt16 nActCol = 16;

                for( sal_uInt16 nB = 0; nB < 256; nB += 51 )
                    for( sal_uInt16 nG = 0; nG < 256; nG += 51 )
                        for( sal_uInt16 nR = 0; nR < 256; nR += 51 )
                            aPal[ nActCol++ ] = BitmapColor( static_cast<sal_uInt8>(nR), static_cast<sal_uInt8>(nG), static_cast<sal_uInt8>(nB) );

                // Set standard Office colors
                aPal[ nActCol++ ] = BitmapColor( 0, 184, 255 );
                return aPal;
            }();
            if (!pPal)
                pPal = &aPalN8_BPP;
            break;
        }
        default:
        {
            static const BitmapPalette aPalEmpty;
            if (!pPal || !vcl::isPalettePixelFormat(ePixelFormat))
                pPal = &aPalEmpty;
            break;
        }
    }

    mxSalBmp = ImplGetSVData()->mpDefInst->CreateSalBitmap();
    mxSalBmp->Create(rSizePixel, ePixelFormat, *pPal);
}

#ifdef DBG_UTIL

namespace
{
void savePNG(const OUString& sWhere, const Bitmap& rBmp)
{
    SvFileStream aStream(sWhere, StreamMode::WRITE | StreamMode::TRUNC);
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
    rFilter.compressAsPNG(BitmapEx(rBmp), aStream);
}
}

#endif

Bitmap::~Bitmap()
{
#ifdef DBG_UTIL
    // VCL_DUMP_BMP_PATH should be like C:/path/ or ~/path/
    static const OUString sDumpPath(OUString::createFromAscii(std::getenv("VCL_DUMP_BMP_PATH")));
    // Stepping into the dtor of a bitmap you need, and setting the volatile variable to true in
    // debugger, would dump the bitmap in question
    static volatile bool save(false);
    if (!sDumpPath.isEmpty() && save)
    {
        save = false;
        savePNG(sDumpPath + "BitmapDump.png", *this);
    }
#endif
}

namespace
{
template <size_t N>
constexpr std::enable_if_t<255 % (N - 1) == 0, std::array<BitmapColor, N>> getGreyscalePalette()
{
    const int step = 255 / (N - 1);
    std::array<BitmapColor, N> a;
    for (size_t i = 0; i < N; ++i)
        a[i] = BitmapColor(i * step, i * step, i * step);
    return a;
}
}

const BitmapPalette& Bitmap::GetGreyPalette( int nEntries )
{
    // Create greyscale palette with 2, 4, 16 or 256 entries
    switch (nEntries)
    {
        case 2:
        {
            static const BitmapPalette aGreyPalette2 = getGreyscalePalette<2>();
            return aGreyPalette2;
        }
        case 4:
        {
            static const BitmapPalette aGreyPalette4 = getGreyscalePalette<4>();
            return aGreyPalette4;
        }
        case 16:
        {
            static const BitmapPalette aGreyPalette16 = getGreyscalePalette<16>();
            return aGreyPalette16;
        }
        case 256:
        {
            static const BitmapPalette aGreyPalette256 = getGreyscalePalette<256>();
            return aGreyPalette256;
        }
    }
    OSL_FAIL("Bitmap::GetGreyPalette: invalid entry count (2/4/16/256 allowed)");
    return GetGreyPalette(2);
}

Bitmap& Bitmap::operator=( const Bitmap& rBitmap )
{
    if (this == &rBitmap)
        return *this;

    maPrefSize = rBitmap.maPrefSize;
    maPrefMapMode = rBitmap.maPrefMapMode;
    mxSalBmp = rBitmap.mxSalBmp;

    return *this;
}

Bitmap& Bitmap::operator=( Bitmap&& rBitmap ) noexcept
{
    maPrefSize = std::move(rBitmap.maPrefSize);
    maPrefMapMode = std::move(rBitmap.maPrefMapMode);
    mxSalBmp = std::move(rBitmap.mxSalBmp);

    return *this;
}

bool Bitmap::operator==( const Bitmap& rBmp ) const
{
    if (rBmp.mxSalBmp == mxSalBmp) // Includes both are nullptr
        return true;
    if (!rBmp.mxSalBmp || !mxSalBmp)
        return false;
    if (rBmp.mxSalBmp->GetSize() != mxSalBmp->GetSize() ||
        rBmp.mxSalBmp->GetBitCount() != mxSalBmp->GetBitCount())
        return false;
    BitmapChecksum aChecksum1 = rBmp.mxSalBmp->GetChecksum();
    BitmapChecksum aChecksum2 = mxSalBmp->GetChecksum();
    // If the bitmaps can't calculate a checksum, best to regard them as different.
    if (aChecksum1 == 0 || aChecksum2 == 0)
        return false;
    return aChecksum1 == aChecksum2;
}

void Bitmap::SetEmpty()
{
    maPrefMapMode = MapMode();
    maPrefSize = Size();
    mxSalBmp.reset();
}

Size Bitmap::GetSizePixel() const
{
    return( mxSalBmp ? mxSalBmp->GetSize() : Size() );
}

vcl::PixelFormat Bitmap::getPixelFormat() const
{
    if (!mxSalBmp)
        return vcl::PixelFormat::INVALID;

    sal_uInt16 nBitCount = mxSalBmp->GetBitCount();
    if (nBitCount <= 8)
        return vcl::PixelFormat::N8_BPP;
    if (nBitCount <= 24)
        return vcl::PixelFormat::N24_BPP;
    if (nBitCount <= 32)
        return vcl::PixelFormat::N32_BPP;

    return vcl::PixelFormat::INVALID;
}

bool Bitmap::HasGreyPaletteAny() const
{
    bool bRet = false;

    BitmapScopedInfoAccess pIAcc(*this);

    if( pIAcc )
    {
        bRet = pIAcc->HasPalette() && pIAcc->GetPalette().IsGreyPaletteAny();
    }

    return bRet;
}

bool Bitmap::HasGreyPalette8Bit() const
{
    bool            bRet = false;
    BitmapScopedInfoAccess pIAcc(*this);

    if( pIAcc )
    {
        bRet = pIAcc->HasPalette() && pIAcc->GetPalette().IsGreyPalette8Bit();
    }

    return bRet;
}

BitmapChecksum Bitmap::GetChecksum() const
{
    if( !mxSalBmp )
        return 0;

    BitmapChecksum nRet = mxSalBmp->GetChecksum();
    if (!nRet)
    {
        // nRet == 0 => probably, we were not able to acquire
        // the buffer in SalBitmap::updateChecksum;
        // so, we need to update the imp bitmap for this bitmap instance
        // as we do in BitmapInfoAccess::ImplCreate
        std::shared_ptr<SalBitmap> xNewImpBmp(ImplGetSVData()->mpDefInst->CreateSalBitmap());
        if (xNewImpBmp->Create(*mxSalBmp, getPixelFormat()))
        {
            Bitmap* pThis = const_cast<Bitmap*>(this);
            pThis->mxSalBmp = xNewImpBmp;
            nRet = mxSalBmp->GetChecksum();
        }
    }

    return nRet;
}

void Bitmap::ImplMakeUnique()
{
    if (mxSalBmp && mxSalBmp.use_count() > 1)
    {
        std::shared_ptr<SalBitmap> xOldImpBmp = mxSalBmp;
        mxSalBmp = ImplGetSVData()->mpDefInst->CreateSalBitmap();
        (void)mxSalBmp->Create(*xOldImpBmp);
    }
}

void Bitmap::ReassignWithSize(const Bitmap& rBitmap)
{
    const Size aOldSizePix(GetSizePixel());
    const Size aNewSizePix(rBitmap.GetSizePixel());
    const MapMode aOldMapMode(maPrefMapMode);
    Size aNewPrefSize;

    if ((aOldSizePix != aNewSizePix) && aOldSizePix.Width() && aOldSizePix.Height())
    {
        aNewPrefSize.setWidth(FRound(maPrefSize.Width() * aNewSizePix.Width() / aOldSizePix.Width()));
        aNewPrefSize.setHeight(FRound(maPrefSize.Height() * aNewSizePix.Height() / aOldSizePix.Height()));
    }
    else
    {
        aNewPrefSize = maPrefSize;
    }

    *this = rBitmap;

    maPrefSize = aNewPrefSize;
    maPrefMapMode = aOldMapMode;
}

void Bitmap::ImplSetSalBitmap(const std::shared_ptr<SalBitmap>& xImpBmp)
{
    mxSalBmp = xImpBmp;
}

bool Bitmap::Crop( const tools::Rectangle& rRectPixel )
{
    const Size          aSizePix( GetSizePixel() );
    tools::Rectangle           aRect( rRectPixel );

    aRect.Intersection( tools::Rectangle( Point(), aSizePix ) );

    if( aRect.IsEmpty() || aSizePix == aRect.GetSize())
        return false;

    BitmapScopedReadAccess pReadAcc(*this);
    if( !pReadAcc )
        return false;

    const tools::Rectangle     aNewRect( Point(), aRect.GetSize() );
    Bitmap aNewBmp(aNewRect.GetSize(), getPixelFormat(), &pReadAcc->GetPalette());
    BitmapScopedWriteAccess pWriteAcc(aNewBmp);
    if( !pWriteAcc )
        return false;

    const tools::Long nOldX = aRect.Left();
    const tools::Long nOldY = aRect.Top();
    const tools::Long nNewWidth = aNewRect.GetWidth();
    const tools::Long nNewHeight = aNewRect.GetHeight();

    for( tools::Long nY = 0, nY2 = nOldY; nY < nNewHeight; nY++, nY2++ )
    {
        Scanline pScanline = pWriteAcc->GetScanline(nY);
        Scanline pScanlineRead = pReadAcc->GetScanline(nY2);
        for( tools::Long nX = 0, nX2 = nOldX; nX < nNewWidth; nX++, nX2++ )
            pWriteAcc->SetPixelOnData( pScanline, nX, pReadAcc->GetPixelFromData( pScanlineRead, nX2 ) );
    }

    pWriteAcc.reset();
    pReadAcc.reset();

    ReassignWithSize( aNewBmp );

    return true;
};

bool Bitmap::CopyPixel( const tools::Rectangle& rRectDst,
                        const tools::Rectangle& rRectSrc )
{
    const Size  aSizePix( GetSizePixel() );
    tools::Rectangle   aRectDst( rRectDst );

    aRectDst.Intersection( tools::Rectangle( Point(), aSizePix ) );

    if( aRectDst.IsEmpty() )
        return false;

    tools::Rectangle aRectSrc( rRectSrc );

    aRectSrc.Intersection( tools::Rectangle( Point(), aSizePix ) );

    if( aRectSrc.IsEmpty() || ( aRectSrc == aRectDst ) )
        return false;

    BitmapScopedWriteAccess   pWriteAcc(*this);
    if( !pWriteAcc )
        return false;

    const tools::Long  nWidth = std::min( aRectSrc.GetWidth(), aRectDst.GetWidth() );
    const tools::Long  nHeight = std::min( aRectSrc.GetHeight(), aRectDst.GetHeight() );
    const tools::Long  nSrcX = aRectSrc.Left();
    const tools::Long  nSrcY = aRectSrc.Top();
    const tools::Long  nSrcEndX1 = nSrcX + nWidth - 1;
    const tools::Long  nSrcEndY1 = nSrcY + nHeight - 1;
    const tools::Long  nDstX = aRectDst.Left();
    const tools::Long  nDstY = aRectDst.Top();
    const tools::Long  nDstEndX1 = nDstX + nWidth - 1;
    const tools::Long  nDstEndY1 = nDstY + nHeight - 1;

    if( ( nDstX <= nSrcX ) && ( nDstY <= nSrcY ) )
    {
        for( tools::Long nY = nSrcY, nYN = nDstY; nY <= nSrcEndY1; nY++, nYN++ )
        {
            Scanline pScanline = pWriteAcc->GetScanline(nYN);
            Scanline pScanlineSrc = pWriteAcc->GetScanline(nY);
            for( tools::Long nX = nSrcX, nXN = nDstX; nX <= nSrcEndX1; nX++, nXN++ )
                pWriteAcc->SetPixelOnData( pScanline, nXN, pWriteAcc->GetPixelFromData( pScanlineSrc, nX ) );
        }
    }
    else if( ( nDstX <= nSrcX ) && ( nDstY >= nSrcY ) )
    {
        for( tools::Long nY = nSrcEndY1, nYN = nDstEndY1; nY >= nSrcY; nY--, nYN-- )
        {
            Scanline pScanline = pWriteAcc->GetScanline(nYN);
            Scanline pScanlineSrc = pWriteAcc->GetScanline(nY);
            for( tools::Long nX = nSrcX, nXN = nDstX; nX <= nSrcEndX1; nX++, nXN++ )
                pWriteAcc->SetPixelOnData( pScanline, nXN, pWriteAcc->GetPixelFromData( pScanlineSrc, nX ) );
        }
    }
    else if( ( nDstX >= nSrcX ) && ( nDstY <= nSrcY ) )
    {
        for( tools::Long nY = nSrcY, nYN = nDstY; nY <= nSrcEndY1; nY++, nYN++ )
        {
            Scanline pScanline = pWriteAcc->GetScanline(nYN);
            Scanline pScanlineSrc = pWriteAcc->GetScanline(nY);
            for( tools::Long nX = nSrcEndX1, nXN = nDstEndX1; nX >= nSrcX; nX--, nXN-- )
                pWriteAcc->SetPixelOnData( pScanline, nXN, pWriteAcc->GetPixelFromData( pScanlineSrc, nX ) );
        }
    }
    else
    {
        for( tools::Long nY = nSrcEndY1, nYN = nDstEndY1; nY >= nSrcY; nY--, nYN-- )
        {
            Scanline pScanline = pWriteAcc->GetScanline(nYN);
            Scanline pScanlineSrc = pWriteAcc->GetScanline(nY);
            for( tools::Long nX = nSrcEndX1, nXN = nDstEndX1; nX >= nSrcX; nX--, nXN-- )
                pWriteAcc->SetPixelOnData( pScanline, nXN, pWriteAcc->GetPixelFromData( pScanlineSrc, nX ) );
        }
    }

    return true;
}

bool Bitmap::CopyPixel( const tools::Rectangle& rRectDst,
                        const tools::Rectangle& rRectSrc, const Bitmap& rBmpSrc )
{
    const Size  aSizePix( GetSizePixel() );
    tools::Rectangle   aRectDst( rRectDst );

    aRectDst.Intersection( tools::Rectangle( Point(), aSizePix ) );

    if( aRectDst.IsEmpty() )
        return false;

    if( rBmpSrc.mxSalBmp == mxSalBmp ) // if self-copy
        return CopyPixel(rRectDst, rRectSrc);

    Bitmap*         pSrc = &const_cast<Bitmap&>(rBmpSrc);
    const Size      aCopySizePix( pSrc->GetSizePixel() );
    tools::Rectangle       aRectSrc( rRectSrc );
    const sal_uInt16 nSrcBitCount = vcl::pixelFormatBitCount(rBmpSrc.getPixelFormat());
    const sal_uInt16 nDstBitCount = vcl::pixelFormatBitCount(getPixelFormat());

    if( nSrcBitCount > nDstBitCount )
    {
        int nNextIndex = 0;

        if (nSrcBitCount == 24)
            Convert( BmpConversion::N24Bit );
        else if (nSrcBitCount == 8)
        {
            Convert( BmpConversion::N8BitColors );
            nNextIndex = 16;
        }
        else if (nSrcBitCount == 4)
        {
            assert(false);
        }

        if( nNextIndex )
        {
            BitmapScopedReadAccess  pSrcAcc(*pSrc);
            BitmapScopedWriteAccess pDstAcc(*this);

            if( pSrcAcc && pDstAcc )
            {
                const int nSrcCount = pSrcAcc->GetPaletteEntryCount();
                const int nDstCount = 1 << nDstBitCount;

                for (int i = 0; ( i < nSrcCount ) && ( nNextIndex < nDstCount ); ++i)
                {
                    const BitmapColor& rSrcCol = pSrcAcc->GetPaletteColor( static_cast<sal_uInt16>(i) );

                    bool bFound = false;

                    for (int j = 0; j < nDstCount; ++j)
                    {
                        if( rSrcCol == pDstAcc->GetPaletteColor( static_cast<sal_uInt16>(j) ) )
                        {
                            bFound = true;
                            break;
                        }
                    }

                    if( !bFound )
                        pDstAcc->SetPaletteColor( static_cast<sal_uInt16>(nNextIndex++), rSrcCol );
                }
            }
        }
    }

    aRectSrc.Intersection( tools::Rectangle( Point(), aCopySizePix ) );

    if( aRectSrc.IsEmpty() )
        return false;

    BitmapScopedReadAccess pReadAcc(*pSrc);
    if( !pReadAcc )
        return false;

    BitmapScopedWriteAccess pWriteAcc(*this);
    if( !pWriteAcc )
        return false;

    const tools::Long  nWidth = std::min( aRectSrc.GetWidth(), aRectDst.GetWidth() );
    const tools::Long  nHeight = std::min( aRectSrc.GetHeight(), aRectDst.GetHeight() );
    const tools::Long  nSrcEndX = aRectSrc.Left() + nWidth;
    const tools::Long  nSrcEndY = aRectSrc.Top() + nHeight;
    tools::Long        nDstY = aRectDst.Top();

    if( pReadAcc->HasPalette() && pWriteAcc->HasPalette() )
    {
        const sal_uInt16    nCount = pReadAcc->GetPaletteEntryCount();
        std::unique_ptr<sal_uInt8[]> pMap(new sal_uInt8[ nCount ]);

        // Create index map for the color table, as the bitmap should be copied
        // retaining it's color information relatively well
        for( sal_uInt16 i = 0; i < nCount; i++ )
            pMap[ i ] = static_cast<sal_uInt8>(pWriteAcc->GetBestPaletteIndex( pReadAcc->GetPaletteColor( i ) ));

        for( tools::Long nSrcY = aRectSrc.Top(); nSrcY < nSrcEndY; nSrcY++, nDstY++ )
        {
            Scanline pScanline = pWriteAcc->GetScanline(nDstY);
            Scanline pScanlineRead = pReadAcc->GetScanline(nSrcY);
            for( tools::Long nSrcX = aRectSrc.Left(), nDstX = aRectDst.Left(); nSrcX < nSrcEndX; nSrcX++, nDstX++ )
                pWriteAcc->SetPixelOnData( pScanline, nDstX, BitmapColor( pMap[ pReadAcc->GetIndexFromData( pScanlineRead, nSrcX ) ] ));
        }
    }
    else if( pReadAcc->HasPalette() )
    {
        for( tools::Long nSrcY = aRectSrc.Top(); nSrcY < nSrcEndY; nSrcY++, nDstY++ )
        {
            Scanline pScanline = pWriteAcc->GetScanline(nDstY);
            Scanline pScanlineRead = pReadAcc->GetScanline(nSrcY);
            for( tools::Long nSrcX = aRectSrc.Left(), nDstX = aRectDst.Left(); nSrcX < nSrcEndX; nSrcX++, nDstX++ )
                pWriteAcc->SetPixelOnData( pScanline, nDstX, pReadAcc->GetPaletteColor( pReadAcc->GetIndexFromData( pScanlineRead, nSrcX ) ) );
        }
    }
    else
        for( tools::Long nSrcY = aRectSrc.Top(); nSrcY < nSrcEndY; nSrcY++, nDstY++ )
        {
            Scanline pScanline = pWriteAcc->GetScanline(nDstY);
            Scanline pScanlineRead = pReadAcc->GetScanline(nSrcY);
            for( tools::Long nSrcX = aRectSrc.Left(), nDstX = aRectDst.Left(); nSrcX < nSrcEndX; nSrcX++, nDstX++ )
                pWriteAcc->SetPixelOnData( pScanline, nDstX, pReadAcc->GetPixelFromData( pScanlineRead, nSrcX ) );
        }

    bool bRet = ( nWidth > 0 ) && ( nHeight > 0 );

    return bRet;
}

bool Bitmap::CopyPixel_AlphaOptimized( const tools::Rectangle& rRectDst, const tools::Rectangle& rRectSrc )
{
    assert(HasGreyPalette8Bit());
    // Note: this code is copied from Bitmap::CopyPixel but avoids any palette lookups
    // This optimization is possible because the palettes of AlphaMasks are always identical (8bit GreyPalette, see ctor)
    const Size  aSizePix( GetSizePixel() );
    tools::Rectangle   aRectDst( rRectDst );

    aRectDst.Intersection( tools::Rectangle( Point(), aSizePix ) );

    if( aRectDst.IsEmpty() )
        return false;

    tools::Rectangle aRectSrc( rRectSrc );
    aRectSrc.Intersection( tools::Rectangle( Point(), aSizePix ) );
    if( aRectSrc.IsEmpty() || ( aRectSrc == aRectDst ) )
        return false;

    BitmapScopedWriteAccess   pWriteAcc(*this);
    if( !pWriteAcc )
        return false;

    const tools::Long  nWidth = std::min( aRectSrc.GetWidth(), aRectDst.GetWidth() );
    const tools::Long  nHeight = std::min( aRectSrc.GetHeight(), aRectDst.GetHeight() );
    const tools::Long  nSrcX = aRectSrc.Left();
    const tools::Long  nSrcY = aRectSrc.Top();
    const tools::Long  nSrcEndX1 = nSrcX + nWidth - 1;
    const tools::Long  nSrcEndY1 = nSrcY + nHeight - 1;
    const tools::Long  nDstX = aRectDst.Left();
    const tools::Long  nDstY = aRectDst.Top();
    const tools::Long  nDstEndX1 = nDstX + nWidth - 1;
    const tools::Long  nDstEndY1 = nDstY + nHeight - 1;

    if( ( nDstX <= nSrcX ) && ( nDstY <= nSrcY ) )
    {
        for( tools::Long nY = nSrcY, nYN = nDstY; nY <= nSrcEndY1; nY++, nYN++ )
        {
            Scanline pScanline = pWriteAcc->GetScanline(nYN);
            Scanline pScanlineSrc = pWriteAcc->GetScanline(nY);
            for( tools::Long nX = nSrcX, nXN = nDstX; nX <= nSrcEndX1; nX++, nXN++ )
                pWriteAcc->SetPixelOnData( pScanline, nXN, pWriteAcc->GetPixelFromData( pScanlineSrc, nX ) );
        }
    }
    else if( ( nDstX <= nSrcX ) && ( nDstY >= nSrcY ) )
    {
        for( tools::Long nY = nSrcEndY1, nYN = nDstEndY1; nY >= nSrcY; nY--, nYN-- )
        {
            Scanline pScanline = pWriteAcc->GetScanline(nYN);
            Scanline pScanlineSrc = pWriteAcc->GetScanline(nY);
            for( tools::Long nX = nSrcX, nXN = nDstX; nX <= nSrcEndX1; nX++, nXN++ )
                pWriteAcc->SetPixelOnData( pScanline, nXN, pWriteAcc->GetPixelFromData( pScanlineSrc, nX ) );
        }
    }
    else if( ( nDstX >= nSrcX ) && ( nDstY <= nSrcY ) )
    {
        for( tools::Long nY = nSrcY, nYN = nDstY; nY <= nSrcEndY1; nY++, nYN++ )
        {
            Scanline pScanline = pWriteAcc->GetScanline(nYN);
            Scanline pScanlineSrc = pWriteAcc->GetScanline(nY);
            for( tools::Long nX = nSrcEndX1, nXN = nDstEndX1; nX >= nSrcX; nX--, nXN-- )
                pWriteAcc->SetPixelOnData( pScanline, nXN, pWriteAcc->GetPixelFromData( pScanlineSrc, nX ) );
        }
    }
    else
    {
        for( tools::Long nY = nSrcEndY1, nYN = nDstEndY1; nY >= nSrcY; nY--, nYN-- )
        {
            Scanline pScanline = pWriteAcc->GetScanline(nYN);
            Scanline pScanlineSrc = pWriteAcc->GetScanline(nY);
            for( tools::Long nX = nSrcEndX1, nXN = nDstEndX1; nX >= nSrcX; nX--, nXN-- )
                pWriteAcc->SetPixelOnData( pScanline, nXN, pWriteAcc->GetPixelFromData( pScanlineSrc, nX ) );
        }
    }

    return true;
}

bool Bitmap::CopyPixel_AlphaOptimized( const tools::Rectangle& rRectDst, const tools::Rectangle& rRectSrc,
                           const AlphaMask& rBmpSrc )
{
    assert(HasGreyPalette8Bit());
    assert(rBmpSrc.GetBitmap().HasGreyPalette8Bit());
    // Note: this code is copied from Bitmap::CopyPixel but avoids any palette lookups
    // This optimization is possible because the palettes of AlphaMasks are always identical (8bit GreyPalette, see ctor)
    const Size  aSizePix( GetSizePixel() );
    tools::Rectangle   aRectDst( rRectDst );

    aRectDst.Intersection( tools::Rectangle( Point(), aSizePix ) );

    if( aRectDst.IsEmpty() )
        return false;

    if( rBmpSrc.GetBitmap().mxSalBmp == mxSalBmp ) // self-copy
        return CopyPixel_AlphaOptimized(rRectDst, rRectSrc);

    Bitmap*         pSrc = &const_cast<Bitmap&>(rBmpSrc.GetBitmap());
    const Size      aCopySizePix( pSrc->GetSizePixel() );
    tools::Rectangle       aRectSrc( rRectSrc );

    aRectSrc.Intersection( tools::Rectangle( Point(), aCopySizePix ) );
    if( aRectSrc.IsEmpty() )
        return false;

    BitmapScopedReadAccess pReadAcc(*pSrc);
    if( !pReadAcc )
        return false;

    BitmapScopedWriteAccess pWriteAcc(*this);
    if( !pWriteAcc )
        return false;

    const tools::Long  nWidth = std::min( aRectSrc.GetWidth(), aRectDst.GetWidth() );
    const tools::Long  nHeight = std::min( aRectSrc.GetHeight(), aRectDst.GetHeight() );
    const tools::Long  nSrcEndX = aRectSrc.Left() + nWidth;
    const tools::Long  nSrcEndY = aRectSrc.Top() + nHeight;
    tools::Long        nDstY = aRectDst.Top();

    for( tools::Long nSrcY = aRectSrc.Top(); nSrcY < nSrcEndY; nSrcY++, nDstY++)
    {
        Scanline pScanline = pWriteAcc->GetScanline(nDstY);
        Scanline pScanlineRead = pReadAcc->GetScanline(nSrcY);
        for( tools::Long nSrcX = aRectSrc.Left(), nDstX = aRectDst.Left(); nSrcX < nSrcEndX; nSrcX++, nDstX++ )
            pWriteAcc->SetPixelOnData( pScanline, nDstX, pReadAcc->GetPixelFromData( pScanlineRead, nSrcX ) );
    }

    bool bRet = ( nWidth > 0 ) && ( nHeight > 0 );

    return bRet;
}

bool Bitmap::Expand( sal_Int32 nDX, sal_Int32 nDY, const Color* pInitColor )
{
    if( !nDX && !nDY )
        return false;

    const Size          aSizePixel( GetSizePixel() );
    const tools::Long          nWidth = aSizePixel.Width();
    const tools::Long          nHeight = aSizePixel.Height();
    const Size          aNewSize( nWidth + nDX, nHeight + nDY );
    BitmapScopedReadAccess pReadAcc(*this);
    if( !pReadAcc )
        return false;

    BitmapPalette       aBmpPal( pReadAcc->GetPalette() );
    Bitmap aNewBmp(aNewSize, getPixelFormat(), &aBmpPal);
    BitmapScopedWriteAccess pWriteAcc(aNewBmp);
    if( !pWriteAcc )
        return false;

    BitmapColor aColor;
    const tools::Long  nNewX = nWidth;
    const tools::Long  nNewY = nHeight;
    const tools::Long  nNewWidth = pWriteAcc->Width();
    const tools::Long  nNewHeight = pWriteAcc->Height();
    tools::Long        nX;
    tools::Long        nY;

    if( pInitColor )
        aColor = pWriteAcc->GetBestMatchingColor( *pInitColor );

    for( nY = 0; nY < nHeight; nY++ )
    {
        pWriteAcc->CopyScanline( nY, *pReadAcc );

        if( pInitColor && nDX )
        {
            Scanline pScanline = pWriteAcc->GetScanline(nY);
            for( nX = nNewX; nX < nNewWidth; nX++ )
                pWriteAcc->SetPixelOnData( pScanline, nX, aColor );
        }
    }

    if( pInitColor && nDY )
        for( nY = nNewY; nY < nNewHeight; nY++ )
        {
            Scanline pScanline = pWriteAcc->GetScanline(nY);
            for( nX = 0; nX < nNewWidth; nX++ )
                pWriteAcc->SetPixelOnData( pScanline, nX, aColor );
        }

    pWriteAcc.reset();
    pReadAcc.reset();

    ReassignWithSize(aNewBmp);

    return true;
}

Bitmap Bitmap::CreateDisplayBitmap( OutputDevice* pDisplay ) const
{
    Bitmap aDispBmp( *this );

    SalGraphics* pDispGraphics = pDisplay->GetGraphics();

    if( mxSalBmp && pDispGraphics )
    {
        std::shared_ptr<SalBitmap> xImpDispBmp(ImplGetSVData()->mpDefInst->CreateSalBitmap());
        if (xImpDispBmp->Create(*mxSalBmp, pDispGraphics))
            aDispBmp.ImplSetSalBitmap(xImpDispBmp);
    }

    return aDispBmp;
}

bool Bitmap::GetSystemData( BitmapSystemData& rData ) const
{
    return mxSalBmp && mxSalBmp->GetSystemData(rData);
}


bool Bitmap::Convert( BmpConversion eConversion )
{
    // try to convert in backend
    if (mxSalBmp)
    {
        // avoid large chunk of obsolete and hopefully rarely used conversions.
        if (eConversion == BmpConversion::N8BitNoConversion)
        {
            if (mxSalBmp->GetBitCount() == 8 && HasGreyPalette8Bit())
                return true;
            std::shared_ptr<SalBitmap> xImpBmp(ImplGetSVData()->mpDefInst->CreateSalBitmap());
            // frequently used conversion for creating alpha masks
            if (xImpBmp->Create(*mxSalBmp) && xImpBmp->InterpretAs8Bit())
            {
                ImplSetSalBitmap(xImpBmp);
                SAL_INFO( "vcl.opengl", "Ref count: " << mxSalBmp.use_count() );
                return true;
            }
        }
        if (eConversion == BmpConversion::N8BitGreys)
        {
            std::shared_ptr<SalBitmap> xImpBmp(ImplGetSVData()->mpDefInst->CreateSalBitmap());
            if (xImpBmp->Create(*mxSalBmp) && xImpBmp->ConvertToGreyscale())
            {
                ImplSetSalBitmap(xImpBmp);
                SAL_INFO( "vcl.opengl", "Ref count: " << mxSalBmp.use_count() );
                return true;
            }
        }
    }

    const sal_uInt16 nBitCount = vcl::pixelFormatBitCount(getPixelFormat());
    bool bRet = false;

    switch( eConversion )
    {
        case BmpConversion::N1BitThreshold:
        {
            BitmapEx aBmpEx(*this);
            bRet = BitmapFilter::Filter(aBmpEx, BitmapMonochromeFilter(128));
            *this = aBmpEx.GetBitmap();
        }
        break;

        case BmpConversion::N8BitGreys:
        case BmpConversion::N8BitNoConversion:
            bRet = ImplMakeGreyscales();
        break;

        case BmpConversion::N8BitColors:
        {
            if( nBitCount < 8 )
                bRet = ImplConvertUp(vcl::PixelFormat::N8_BPP);
            else if( nBitCount > 8 )
                bRet = ImplConvertDown8BPP();
            else
                bRet = true;
        }
        break;

        case BmpConversion::N8BitTrans:
        {
            Color aTrans( BMP_COL_TRANS );

            if( nBitCount < 8 )
                bRet = ImplConvertUp(vcl::PixelFormat::N8_BPP, &aTrans );
            else
                bRet = ImplConvertDown8BPP(&aTrans );
        }
        break;

        case BmpConversion::N24Bit:
        {
            if( nBitCount < 24 )
                bRet = ImplConvertUp(vcl::PixelFormat::N24_BPP);
            else
                bRet = true;
        }
        break;

        case BmpConversion::N32Bit:
        {
            if( nBitCount < 32 )
                bRet = ImplConvertUp(vcl::PixelFormat::N32_BPP);
            else
                bRet = true;
        }
        break;

        default:
            OSL_FAIL( "Bitmap::Convert(): Unsupported conversion" );
        break;
    }

    return bRet;
}

bool Bitmap::ImplMakeGreyscales()
{
    BitmapScopedReadAccess pReadAcc(*this);
    if( !pReadAcc )
        return false;

    const BitmapPalette& rPal = GetGreyPalette(256);
    sal_uLong nShift = 0;
    bool bPalDiffers = !pReadAcc->HasPalette() || ( rPal.GetEntryCount() != pReadAcc->GetPaletteEntryCount() );

    if( !bPalDiffers )
        bPalDiffers = ( rPal != pReadAcc->GetPalette() );
    if( !bPalDiffers )
        return true;

    const auto ePixelFormat = vcl::PixelFormat::N8_BPP;
    Bitmap aNewBmp(GetSizePixel(), ePixelFormat, &rPal );
    BitmapScopedWriteAccess pWriteAcc(aNewBmp);
    if( !pWriteAcc )
        return false;

    const tools::Long nWidth = pWriteAcc->Width();
    const tools::Long nHeight = pWriteAcc->Height();

    if( pReadAcc->HasPalette() )
    {
        for( tools::Long nY = 0; nY < nHeight; nY++ )
        {
            Scanline pScanline = pWriteAcc->GetScanline(nY);
            Scanline pScanlineRead = pReadAcc->GetScanline(nY);
            for( tools::Long nX = 0; nX < nWidth; nX++ )
            {
                const sal_uInt8 cIndex = pReadAcc->GetIndexFromData( pScanlineRead, nX );
                pWriteAcc->SetPixelOnData( pScanline, nX,
                    BitmapColor(pReadAcc->GetPaletteColor( cIndex ).GetLuminance() >> nShift) );
            }
        }
    }
    else if( pReadAcc->GetScanlineFormat() == ScanlineFormat::N24BitTcBgr &&
             pWriteAcc->GetScanlineFormat() == ScanlineFormat::N8BitPal )
    {
        nShift += 8;

        for( tools::Long nY = 0; nY < nHeight; nY++ )
        {
            Scanline pReadScan = pReadAcc->GetScanline( nY );
            Scanline pWriteScan = pWriteAcc->GetScanline( nY );

            for( tools::Long nX = 0; nX < nWidth; nX++ )
            {
                const sal_uLong nB = *pReadScan++;
                const sal_uLong nG = *pReadScan++;
                const sal_uLong nR = *pReadScan++;

                *pWriteScan++ = static_cast<sal_uInt8>( ( nB * 28UL + nG * 151UL + nR * 77UL ) >> nShift );
            }
        }
    }
    else if( pReadAcc->GetScanlineFormat() == ScanlineFormat::N24BitTcRgb &&
             pWriteAcc->GetScanlineFormat() == ScanlineFormat::N8BitPal )
    {
        nShift += 8;

        for( tools::Long nY = 0; nY < nHeight; nY++ )
        {
            Scanline pReadScan = pReadAcc->GetScanline( nY );
            Scanline pWriteScan = pWriteAcc->GetScanline( nY );

            for( tools::Long nX = 0; nX < nWidth; nX++ )
            {
                const sal_uLong nR = *pReadScan++;
                const sal_uLong nG = *pReadScan++;
                const sal_uLong nB = *pReadScan++;

                *pWriteScan++ = static_cast<sal_uInt8>( ( nB * 28UL + nG * 151UL + nR * 77UL ) >> nShift );
            }
        }
    }
    else
    {
        for( tools::Long nY = 0; nY < nHeight; nY++ )
        {
            Scanline pScanline = pWriteAcc->GetScanline(nY);
            Scanline pScanlineRead = pReadAcc->GetScanline(nY);
            for( tools::Long nX = 0; nX < nWidth; nX++ )
                pWriteAcc->SetPixelOnData( pScanline, nX, BitmapColor(pReadAcc->GetPixelFromData( pScanlineRead, nX ).GetLuminance() >> nShift) );
        }
    }

    pWriteAcc.reset();
    pReadAcc.reset();

    const MapMode aMap( maPrefMapMode );
    const Size aSize( maPrefSize );

    *this = aNewBmp;

    maPrefMapMode = aMap;
    maPrefSize = aSize;

    return true;
}

bool Bitmap::ImplConvertUp(vcl::PixelFormat ePixelFormat, Color const * pExtColor)
{
    SAL_WARN_IF(ePixelFormat <= getPixelFormat(), "vcl", "New pixel format must be greater!" );

    BitmapScopedReadAccess pReadAcc(*this);
    if (!pReadAcc)
        return false;

    BitmapPalette aPalette;
    Bitmap aNewBmp(GetSizePixel(), ePixelFormat, pReadAcc->HasPalette() ? &pReadAcc->GetPalette() : &aPalette);
    BitmapScopedWriteAccess pWriteAcc(aNewBmp);
    if (!pWriteAcc)
        return false;

    const tools::Long nWidth = pWriteAcc->Width();
    const tools::Long nHeight = pWriteAcc->Height();

    if (pWriteAcc->HasPalette())
    {
        const BitmapPalette& rOldPalette = pReadAcc->GetPalette();
        const sal_uInt16 nOldCount = rOldPalette.GetEntryCount();
        assert(nOldCount <= (1 << vcl::pixelFormatBitCount(getPixelFormat())));

        aPalette.SetEntryCount(1 << vcl::pixelFormatBitCount(ePixelFormat));

        for (sal_uInt16 i = 0; i < nOldCount; i++)
            aPalette[i] = rOldPalette[i];

        if (pExtColor)
            aPalette[aPalette.GetEntryCount() - 1] = *pExtColor;

        pWriteAcc->SetPalette(aPalette);

        for (tools::Long nY = 0; nY < nHeight; nY++)
        {
            Scanline pScanline = pWriteAcc->GetScanline(nY);
            Scanline pScanlineRead = pReadAcc->GetScanline(nY);
            for (tools::Long nX = 0; nX < nWidth; nX++)
            {
                pWriteAcc->SetPixelOnData(pScanline, nX, pReadAcc->GetPixelFromData(pScanlineRead, nX));
            }
        }
    }
    else
    {
        if (pReadAcc->HasPalette())
        {
            for (tools::Long nY = 0; nY < nHeight; nY++)
            {
                Scanline pScanline = pWriteAcc->GetScanline(nY);
                Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                for (tools::Long nX = 0; nX < nWidth; nX++)
                {
                    pWriteAcc->SetPixelOnData(pScanline, nX, pReadAcc->GetPaletteColor(pReadAcc->GetIndexFromData(pScanlineRead, nX)));
                }
            }
        }
        else
        {
            for (tools::Long nY = 0; nY < nHeight; nY++)
            {
                Scanline pScanline = pWriteAcc->GetScanline(nY);
                Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                for (tools::Long nX = 0; nX < nWidth; nX++)
                {
                    pWriteAcc->SetPixelOnData(pScanline, nX, pReadAcc->GetPixelFromData(pScanlineRead, nX));
                }
            }
        }
    }

    const MapMode aMap(maPrefMapMode);
    const Size aSize(maPrefSize);

    *this = aNewBmp;

    maPrefMapMode = aMap;
    maPrefSize = aSize;

    return true;
}

bool Bitmap::ImplConvertDown8BPP(Color const * pExtColor)
{
    SAL_WARN_IF(vcl::PixelFormat::N8_BPP > getPixelFormat(), "vcl", "New pixelformat must be lower ( or equal when pExtColor is set )!");

    BitmapScopedReadAccess pReadAcc(*this);
    if (!pReadAcc)
        return false;

    BitmapPalette aPalette;
    Bitmap aNewBmp(GetSizePixel(), vcl::PixelFormat::N8_BPP, &aPalette);
    BitmapScopedWriteAccess pWriteAcc(aNewBmp);
    if (!pWriteAcc)
        return false;

    sal_Int16 nNewBitCount = sal_Int16(vcl::PixelFormat::N8_BPP);
    const sal_uInt16 nCount = 1 << nNewBitCount;
    const tools::Long nWidth = pWriteAcc->Width();
    const tools::Long nWidth1 = nWidth - 1;
    const tools::Long nHeight = pWriteAcc->Height();
    Octree aOctree(*pReadAcc, pExtColor ? (nCount - 1) : nCount);
    aPalette = aOctree.GetPalette();
    InverseColorMap aColorMap(aPalette);
    BitmapColor aColor;
    ImpErrorQuad aErrQuad;
    std::vector<ImpErrorQuad> aErrQuad1(nWidth);
    std::vector<ImpErrorQuad> aErrQuad2(nWidth);
    ImpErrorQuad* pQLine1 = aErrQuad1.data();
    ImpErrorQuad* pQLine2 = nullptr;
    tools::Long nYTmp = 0;
    sal_uInt8 cIndex;
    bool bQ1 = true;

    if (pExtColor)
    {
        aPalette.SetEntryCount(aPalette.GetEntryCount() + 1);
        aPalette[aPalette.GetEntryCount() - 1] = *pExtColor;
    }

    // set Black/White always, if we have enough space
    if (aPalette.GetEntryCount() < (nCount - 1))
    {
        aPalette.SetEntryCount(aPalette.GetEntryCount() + 2);
        aPalette[aPalette.GetEntryCount() - 2] = COL_BLACK;
        aPalette[aPalette.GetEntryCount() - 1] = COL_WHITE;
    }

    pWriteAcc->SetPalette(aPalette);

    for (tools::Long nY = 0; nY < std::min(nHeight, tools::Long(2)); nY++, nYTmp++)
    {
        pQLine2 = !nY ? aErrQuad1.data() : aErrQuad2.data();
        Scanline pScanlineRead = pReadAcc->GetScanline(nYTmp);
        for (tools::Long nX = 0; nX < nWidth; nX++)
        {
            if (pReadAcc->HasPalette())
                pQLine2[nX] = pReadAcc->GetPaletteColor(pReadAcc->GetIndexFromData(pScanlineRead, nX));
            else
                pQLine2[nX] = pReadAcc->GetPixelFromData(pScanlineRead, nX);
        }
    }

    assert(pQLine2 || nHeight == 0);

    for (tools::Long nY = 0; nY < nHeight; nY++, nYTmp++)
    {
        // first pixel in the line
        cIndex = static_cast<sal_uInt8>(aColorMap.GetBestPaletteIndex(pQLine1[0].ImplGetColor()));
        Scanline pScanline = pWriteAcc->GetScanline(nY);
        pWriteAcc->SetPixelOnData(pScanline, 0, BitmapColor(cIndex));

        tools::Long nX;
        for (nX = 1; nX < nWidth1; nX++)
        {
            aColor = pQLine1[nX].ImplGetColor();
            cIndex = static_cast<sal_uInt8>(aColorMap.GetBestPaletteIndex(aColor));
            aErrQuad = (ImpErrorQuad(aColor) -= pWriteAcc->GetPaletteColor(cIndex));
            pQLine1[++nX].ImplAddColorError7(aErrQuad);
            pQLine2[nX--].ImplAddColorError1(aErrQuad);
            pQLine2[nX--].ImplAddColorError5(aErrQuad);
            pQLine2[nX++].ImplAddColorError3(aErrQuad);
            pWriteAcc->SetPixelOnData(pScanline, nX, BitmapColor(cIndex));
        }

        // Last RowPixel
        if (nX < nWidth)
        {
            cIndex = static_cast<sal_uInt8>(aColorMap.GetBestPaletteIndex(pQLine1[nWidth1].ImplGetColor()));
            pWriteAcc->SetPixelOnData(pScanline, nX, BitmapColor(cIndex));
        }

        // Refill/copy row buffer
        pQLine1 = pQLine2;
        bQ1 = !bQ1;
        pQLine2 = bQ1 ? aErrQuad2.data() : aErrQuad1.data();

        if (nYTmp < nHeight)
        {
            Scanline pScanlineRead = pReadAcc->GetScanline(nYTmp);
            for (nX = 0; nX < nWidth; nX++)
            {
                if (pReadAcc->HasPalette())
                    pQLine2[nX] = pReadAcc->GetPaletteColor(pReadAcc->GetIndexFromData(pScanlineRead, nX));
                else
                    pQLine2[nX] = pReadAcc->GetPixelFromData(pScanlineRead, nX);
            }
        }
    }

    pWriteAcc.reset();

    const MapMode aMap(maPrefMapMode);
    const Size aSize(maPrefSize);

    *this = aNewBmp;

    maPrefMapMode = aMap;
    maPrefSize = aSize;

    return true;
}

bool Bitmap::Scale( const double& rScaleX, const double& rScaleY, BmpScaleFlag nScaleFlag )
{
    if(basegfx::fTools::equalZero(rScaleX) || basegfx::fTools::equalZero(rScaleY))
    {
        // no scale
        return true;
    }

    if(basegfx::fTools::equal(rScaleX, 1.0) && basegfx::fTools::equal(rScaleY, 1.0))
    {
        // no scale
        return true;
    }

    const auto eStartPixelFormat = getPixelFormat();

    if (mxSalBmp && mxSalBmp->ScalingSupported())
    {
        // implementation specific scaling
        std::shared_ptr<SalBitmap> xImpBmp(ImplGetSVData()->mpDefInst->CreateSalBitmap());
        if (xImpBmp->Create(*mxSalBmp) && xImpBmp->Scale(rScaleX, rScaleY, nScaleFlag))
        {
            ImplSetSalBitmap(xImpBmp);
            SAL_INFO( "vcl.opengl", "Ref count: " << mxSalBmp.use_count() );
            maPrefMapMode = MapMode( MapUnit::MapPixel );
            maPrefSize = xImpBmp->GetSize();
            return true;
        }
    }

    BitmapEx aBmpEx(*this);
    bool bRetval(false);

    switch(nScaleFlag)
    {
        case BmpScaleFlag::Default:
            if (GetSizePixel().Width() < 2 || GetSizePixel().Height() < 2)
                bRetval = BitmapFilter::Filter(aBmpEx, BitmapFastScaleFilter(rScaleX, rScaleY));
            else
                bRetval = BitmapFilter::Filter(aBmpEx, BitmapScaleSuperFilter(rScaleX, rScaleY));
            break;

        case BmpScaleFlag::Fast:
        case BmpScaleFlag::NearestNeighbor:
            bRetval = BitmapFilter::Filter(aBmpEx, BitmapFastScaleFilter(rScaleX, rScaleY));
            break;

        case BmpScaleFlag::Interpolate:
            bRetval = BitmapFilter::Filter(aBmpEx, BitmapInterpolateScaleFilter(rScaleX, rScaleY));
            break;

        case BmpScaleFlag::BestQuality:
        case BmpScaleFlag::Lanczos:
            bRetval = BitmapFilter::Filter(aBmpEx, vcl::BitmapScaleLanczos3Filter(rScaleX, rScaleY));
            break;

        case BmpScaleFlag::BiCubic:
            bRetval = BitmapFilter::Filter(aBmpEx, vcl::BitmapScaleBicubicFilter(rScaleX, rScaleY));
            break;

        case BmpScaleFlag::BiLinear:
            bRetval = BitmapFilter::Filter(aBmpEx, vcl::BitmapScaleBilinearFilter(rScaleX, rScaleY));
            break;
    }

    if (bRetval)
        *this = aBmpEx.GetBitmap();

    OSL_ENSURE(!bRetval || eStartPixelFormat == getPixelFormat(), "Bitmap::Scale has changed the ColorDepth, this should *not* happen (!)");
    return bRetval;
}

bool Bitmap::Scale( const Size& rNewSize, BmpScaleFlag nScaleFlag )
{
    const Size aSize( GetSizePixel() );
    bool bRet;

    if( aSize.Width() && aSize.Height() )
    {
        bRet = Scale( static_cast<double>(rNewSize.Width()) / aSize.Width(),
                      static_cast<double>(rNewSize.Height()) / aSize.Height(),
                      nScaleFlag );
    }
    else
        bRet = true;

    return bRet;
}

bool Bitmap::HasFastScale()
{
#if HAVE_FEATURE_SKIA
    if( SkiaHelper::isVCLSkiaEnabled() && SkiaHelper::renderMethodToUse() != SkiaHelper::RenderRaster)
        return true;
#endif
    return false;
}

void Bitmap::AdaptBitCount(Bitmap& rNew) const
{
    // aNew is the result of some operation; adapt it's BitCount to the original (this)
    if (getPixelFormat() == rNew.getPixelFormat())
        return;

    switch (getPixelFormat())
    {
        case vcl::PixelFormat::N8_BPP:
        {
            if(HasGreyPaletteAny())
            {
                rNew.Convert(BmpConversion::N8BitGreys);
            }
            else
            {
                rNew.Convert(BmpConversion::N8BitColors);
            }
            break;
        }
        case vcl::PixelFormat::N24_BPP:
        {
            rNew.Convert(BmpConversion::N24Bit);
            break;
        }
        case vcl::PixelFormat::N32_BPP:
        {
            rNew.Convert(BmpConversion::N32Bit);
            break;
        }
        case vcl::PixelFormat::INVALID:
        {
            SAL_WARN("vcl", "Can't adapt the pixelformat as it is invalid.");
            break;
        }
    }
}

static void shiftColors(sal_Int32* pColorArray, const BitmapScopedReadAccess& pReadAcc)
{
    Scanline pScanlineRead = pReadAcc->GetScanline(0); // Why always 0?
    for (tools::Long n = 0; n < pReadAcc->Width(); ++n)
    {
        const BitmapColor& rColor = pReadAcc->GetColorFromData(pScanlineRead, n);
        *pColorArray++ = static_cast<sal_Int32>(rColor.GetBlue()) << 12;
        *pColorArray++ = static_cast<sal_Int32>(rColor.GetGreen()) << 12;
        *pColorArray++ = static_cast<sal_Int32>(rColor.GetRed()) << 12;
    }
}

bool Bitmap::Dither()
{
    const Size aSize( GetSizePixel() );
    if( aSize.Width() == 1 || aSize.Height() == 1 )
        return true;
    if( ( aSize.Width() <= 3 ) || ( aSize.Height() <= 2 ) )
        return false;

    BitmapScopedReadAccess pReadAcc(*this);
    Bitmap aNewBmp(GetSizePixel(), vcl::PixelFormat::N8_BPP);
    BitmapScopedWriteAccess pWriteAcc(aNewBmp);
    if( !pReadAcc || !pWriteAcc )
        return false;

    tools::Long nWidth = pReadAcc->Width();
    tools::Long nWidth1 = nWidth - 1;
    tools::Long nHeight = pReadAcc->Height();
    tools::Long nW = nWidth * 3;
    tools::Long nW2 = nW - 3;
    std::unique_ptr<sal_Int32[]> p1(new sal_Int32[ nW ]);
    std::unique_ptr<sal_Int32[]> p2(new sal_Int32[ nW ]);
    sal_Int32* p1T = p1.get();
    sal_Int32* p2T = p2.get();
    shiftColors(p2T, pReadAcc);
    for( tools::Long nYAcc = 0; nYAcc < nHeight; nYAcc++ )
    {
        std::swap(p1T, p2T);
        if (nYAcc < nHeight - 1)
            shiftColors(p2T, pReadAcc);

        auto CalcError = [](tools::Long n)
        {
            n = std::clamp<tools::Long>(n >> 12, 0, 255);
            return std::pair(FloydErrMap[n], FloydMap[n]);
        };

        auto CalcErrors = [&](tools::Long n)
        { return std::tuple_cat(CalcError(p1T[n]), CalcError(p1T[n + 1]), CalcError(p1T[n + 2])); };

        auto CalcT = [](sal_Int32* dst, const int* src, int b, int g, int r)
        {
            dst[0] += src[b];
            dst[1] += src[g];
            dst[2] += src[r];
        };

        auto Calc1 = [&](int x, int b, int g, int r) { CalcT(p2T + x + 3, FloydError1, b, g, r); };
        auto Calc3 = [&](int x, int b, int g, int r) { CalcT(p2T + x - 3, FloydError3, b, g, r); };
        auto Calc5 = [&](int x, int b, int g, int r) { CalcT(p2T + x, FloydError5, b, g, r); };
        auto Calc7 = [&](int x, int b, int g, int r) { CalcT(p1T + x + 3, FloydError7, b, g, r); };

        Scanline pScanline = pWriteAcc->GetScanline(nYAcc);
        // Examine first Pixel separately
        {
            auto [nBErr, nBC, nGErr, nGC, nRErr, nRC] = CalcErrors(0);
            Calc1(0, nBErr, nGErr, nRErr);
            Calc5(0, nBErr, nGErr, nRErr);
            Calc7(0, nBErr, nGErr, nRErr);
            pWriteAcc->SetPixelOnData( pScanline, 0, BitmapColor(static_cast<sal_uInt8>(nVCLBLut[ nBC ] + nVCLGLut[nGC ] + nVCLRLut[nRC ])) );
        }
        // Get middle Pixels using a loop
        for ( tools::Long nX = 3, nXAcc = 1; nX < nW2; nX += 3, nXAcc++ )
        {
            auto [nBErr, nBC, nGErr, nGC, nRErr, nRC] = CalcErrors(nX);
            Calc1(nX, nBErr, nGErr, nRErr);
            Calc3(nX, nBErr, nGErr, nRErr);
            Calc5(nX, nBErr, nGErr, nRErr);
            Calc7(nX, nBErr, nGErr, nRErr);
            pWriteAcc->SetPixelOnData( pScanline, nXAcc, BitmapColor(static_cast<sal_uInt8>(nVCLBLut[ nBC ] + nVCLGLut[nGC ] + nVCLRLut[nRC ])) );
        }
        // Treat last Pixel separately
        {
            auto [nBErr, nBC, nGErr, nGC, nRErr, nRC] = CalcErrors(nW2);
            Calc3(nW2, nBErr, nGErr, nRErr);
            Calc5(nW2, nBErr, nGErr, nRErr);
            pWriteAcc->SetPixelOnData( pScanline, nWidth1, BitmapColor(static_cast<sal_uInt8>(nVCLBLut[ nBC ] + nVCLGLut[nGC ] + nVCLRLut[nRC ])) );
        }
    }
    pReadAcc.reset();
    pWriteAcc.reset();
    const MapMode aMap( maPrefMapMode );
    const Size aPrefSize( maPrefSize );
    *this = aNewBmp;
    maPrefMapMode = aMap;
    maPrefSize = aPrefSize;
    return true;
}

bool Bitmap::Adjust( short nLuminancePercent, short nContrastPercent,
                     short nChannelRPercent, short nChannelGPercent, short nChannelBPercent,
                     double fGamma, bool bInvert, bool msoBrightness )
{
    // nothing to do => return quickly
    if( !nLuminancePercent && !nContrastPercent &&
        !nChannelRPercent && !nChannelGPercent && !nChannelBPercent &&
        ( fGamma == 1.0 ) && !bInvert )
    {
        return true;
    }

    BitmapScopedWriteAccess pAcc(*this);
    if( !pAcc )
        return false;

    BitmapColor aCol;
    const tools::Long nW = pAcc->Width();
    const tools::Long nH = pAcc->Height();
    std::unique_ptr<sal_uInt8[]> cMapR(new sal_uInt8[ 256 ]);
    std::unique_ptr<sal_uInt8[]> cMapG(new sal_uInt8[ 256 ]);
    std::unique_ptr<sal_uInt8[]> cMapB(new sal_uInt8[ 256 ]);
    double fM, fROff, fGOff, fBOff, fOff;

    // calculate slope
    if( nContrastPercent >= 0 )
        fM = 128.0 / ( 128.0 - 1.27 * std::clamp( nContrastPercent, short(0), short(100) ) );
    else
        fM = ( 128.0 + 1.27 * std::clamp( nContrastPercent, short(-100), short(0) ) ) / 128.0;

    if(!msoBrightness)
        // total offset = luminance offset + contrast offset
        fOff = std::clamp( nLuminancePercent, short(-100), short(100) ) * 2.55 + 128.0 - fM * 128.0;
    else
        fOff = std::clamp( nLuminancePercent, short(-100), short(100) ) * 2.55;

    // channel offset = channel offset + total offset
    fROff = nChannelRPercent * 2.55 + fOff;
    fGOff = nChannelGPercent * 2.55 + fOff;
    fBOff = nChannelBPercent * 2.55 + fOff;

    // calculate gamma value
    fGamma = ( fGamma <= 0.0 || fGamma > 10.0 ) ? 1.0 : ( 1.0 / fGamma );
    const bool bGamma = ( fGamma != 1.0 );

    // create mapping table
    for( tools::Long nX = 0; nX < 256; nX++ )
    {
        if(!msoBrightness)
        {
            cMapR[ nX ] = FRound( std::clamp( nX * fM + fROff, 0.0, 255.0 ) );
            cMapG[ nX ] = FRound( std::clamp( nX * fM + fGOff, 0.0, 255.0 ) );
            cMapB[ nX ] = FRound( std::clamp( nX * fM + fBOff, 0.0, 255.0 ) );
        }
        else
        {
            // LO simply uses (in a somewhat optimized form) "newcolor = (oldcolor-128)*contrast+brightness+128"
            // as the formula, i.e. contrast first, brightness afterwards. MSOffice, for whatever weird reason,
            // use neither first, but apparently it applies half of brightness before contrast and half afterwards.
            cMapR[ nX ] = FRound( std::clamp( (nX+fROff/2-128) * fM + 128 + fROff/2, 0.0, 255.0 ) );
            cMapG[ nX ] = FRound( std::clamp( (nX+fGOff/2-128) * fM + 128 + fGOff/2, 0.0, 255.0 ) );
            cMapB[ nX ] = FRound( std::clamp( (nX+fBOff/2-128) * fM + 128 + fBOff/2, 0.0, 255.0 ) );
        }
        if( bGamma )
        {
            cMapR[ nX ] = GAMMA( cMapR[ nX ], fGamma );
            cMapG[ nX ] = GAMMA( cMapG[ nX ], fGamma );
            cMapB[ nX ] = GAMMA( cMapB[ nX ], fGamma );
        }

        if( bInvert )
        {
            cMapR[ nX ] = ~cMapR[ nX ];
            cMapG[ nX ] = ~cMapG[ nX ];
            cMapB[ nX ] = ~cMapB[ nX ];
        }
    }

    // do modifying
    if( pAcc->HasPalette() )
    {
        BitmapColor aNewCol;

        for( sal_uInt16 i = 0, nCount = pAcc->GetPaletteEntryCount(); i < nCount; i++ )
        {
            const BitmapColor& rCol = pAcc->GetPaletteColor( i );
            aNewCol.SetRed( cMapR[ rCol.GetRed() ] );
            aNewCol.SetGreen( cMapG[ rCol.GetGreen() ] );
            aNewCol.SetBlue( cMapB[ rCol.GetBlue() ] );
            pAcc->SetPaletteColor( i, aNewCol );
        }
    }
    else if( pAcc->GetScanlineFormat() == ScanlineFormat::N24BitTcBgr )
    {
        for( tools::Long nY = 0; nY < nH; nY++ )
        {
            Scanline pScan = pAcc->GetScanline( nY );

            for( tools::Long nX = 0; nX < nW; nX++ )
            {
                *pScan = cMapB[ *pScan ]; pScan++;
                *pScan = cMapG[ *pScan ]; pScan++;
                *pScan = cMapR[ *pScan ]; pScan++;
            }
        }
    }
    else if( pAcc->GetScanlineFormat() == ScanlineFormat::N24BitTcRgb )
    {
        for( tools::Long nY = 0; nY < nH; nY++ )
        {
            Scanline pScan = pAcc->GetScanline( nY );

            for( tools::Long nX = 0; nX < nW; nX++ )
            {
                *pScan = cMapR[ *pScan ]; pScan++;
                *pScan = cMapG[ *pScan ]; pScan++;
                *pScan = cMapB[ *pScan ]; pScan++;
            }
        }
    }
    else
    {
        for( tools::Long nY = 0; nY < nH; nY++ )
        {
            Scanline pScanline = pAcc->GetScanline(nY);
            for( tools::Long nX = 0; nX < nW; nX++ )
            {
                aCol = pAcc->GetPixelFromData( pScanline, nX );
                aCol.SetRed( cMapR[ aCol.GetRed() ] );
                aCol.SetGreen( cMapG[ aCol.GetGreen() ] );
                aCol.SetBlue( cMapB[ aCol.GetBlue() ] );
                pAcc->SetPixelOnData( pScanline, nX, aCol );
            }
        }
    }

    pAcc.reset();

    return true;
}

namespace
{
inline sal_uInt8 backBlendAlpha(sal_uInt16 alpha, sal_uInt16 srcCol, sal_uInt16 startCol)
{
    const sal_uInt16 nAlpha((alpha * startCol) / 255);
    if(srcCol > nAlpha)
    {
        return static_cast<sal_uInt8>(((srcCol - nAlpha) * 255) / (255 - nAlpha));
    }

    return 0;
}
}

void Bitmap::RemoveBlendedStartColor(
    const Color& rStartColor,
    const AlphaMask& rAlphaMask)
{
    // no content, done
    if(IsEmpty())
        return;

    BitmapScopedWriteAccess pAcc(*this);
    const tools::Long nHeight(pAcc->Height());
    const tools::Long nWidth(pAcc->Width());

    // no content, done
    if(0 == nHeight || 0 == nWidth)
        return;

    BitmapScopedReadAccess pAlphaAcc(rAlphaMask);

    // inequal sizes of content and alpha, avoid change (maybe assert?)
    if(pAlphaAcc->Height() != nHeight || pAlphaAcc->Width() != nWidth)
        return;

    // prepare local values as sal_uInt16 to avoid multiple conversions
    const sal_uInt16 nStartColRed(rStartColor.GetRed());
    const sal_uInt16 nStartColGreen(rStartColor.GetGreen());
    const sal_uInt16 nStartColBlue(rStartColor.GetBlue());

    for (tools::Long y = 0; y < nHeight; ++y)
    {
        for (tools::Long x = 0; x < nWidth; ++x)
        {
            // get alpha value
            const sal_uInt8 nAlpha8(pAlphaAcc->GetColor(y, x).GetRed());

            // not or completely transparent, no adaptation needed
            if(0 == nAlpha8 || 255 == nAlpha8)
                continue;

            // prepare local value as sal_uInt16 to avoid multiple conversions
            const sal_uInt16 nAlpha16(static_cast<sal_uInt16>(nAlpha8));

            // get source color
            BitmapColor aColor(pAcc->GetColor(y, x));

            // modify/blend back source color
            aColor.SetRed(backBlendAlpha(nAlpha16, static_cast<sal_uInt16>(aColor.GetRed()), nStartColRed));
            aColor.SetGreen(backBlendAlpha(nAlpha16, static_cast<sal_uInt16>(aColor.GetGreen()), nStartColGreen));
            aColor.SetBlue(backBlendAlpha(nAlpha16, static_cast<sal_uInt16>(aColor.GetBlue()), nStartColBlue));

            // write result back
            pAcc->SetPixel(y, x, aColor);
        }
    }
}

const basegfx::SystemDependentDataHolder* Bitmap::accessSystemDependentDataHolder() const
{
    if(!mxSalBmp)
        return nullptr;
    return mxSalBmp->accessSystemDependentDataHolder();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
