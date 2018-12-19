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

#include <osl/diagnose.h>
#include <tools/helpers.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/outdev.hxx>

#include <svdata.hxx>
#include <salinst.hxx>
#include <salbmp.hxx>
#include <bitmapwriteaccess.hxx>

#include <algorithm>
#include <memory>

Bitmap::Bitmap()
{
}

Bitmap::Bitmap(const Bitmap& rBitmap)
    : mxSalBmp(rBitmap.mxSalBmp)
    , maPrefMapMode(rBitmap.maPrefMapMode)
    , maPrefSize(rBitmap.maPrefSize)
{
}

Bitmap::Bitmap(std::shared_ptr<SalBitmap> const & pSalBitmap)
    : mxSalBmp(pSalBitmap)
    , maPrefMapMode(MapMode(MapUnit::MapPixel))
    , maPrefSize(mxSalBmp->GetSize())
{
}

Bitmap::Bitmap( const Size& rSizePixel, sal_uInt16 nBitCount, const BitmapPalette* pPal )
{
    if (rSizePixel.Width() && rSizePixel.Height())
    {
        BitmapPalette   aPal;
        BitmapPalette*  pRealPal = nullptr;

        if( nBitCount <= 8 )
        {
            if( !pPal )
            {
                if( 1 == nBitCount )
                {
                    aPal.SetEntryCount( 2 );
                    aPal[ 0 ] = COL_BLACK;
                    aPal[ 1 ] = COL_WHITE;
                }
                else if( ( 4 == nBitCount ) || ( 8 == nBitCount ) )
                {
                    aPal.SetEntryCount( 1 << nBitCount );
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
                    if( 8 == nBitCount )
                    {
                        sal_uInt16 nActCol = 16;

                        for( sal_uInt16 nB = 0; nB < 256; nB += 51 )
                            for( sal_uInt16 nG = 0; nG < 256; nG += 51 )
                                for( sal_uInt16 nR = 0; nR < 256; nR += 51 )
                                    aPal[ nActCol++ ] = BitmapColor( static_cast<sal_uInt8>(nR), static_cast<sal_uInt8>(nG), static_cast<sal_uInt8>(nB) );

                        // Set standard Office colors
                        aPal[ nActCol++ ] = BitmapColor( 0, 184, 255 );
                    }
                }
            }
            else
                pRealPal = const_cast<BitmapPalette*>(pPal);
        }

        mxSalBmp = ImplGetSVData()->mpDefInst->CreateSalBitmap();
        mxSalBmp->Create( rSizePixel, nBitCount, pRealPal ? *pRealPal : aPal );
    }
}

Bitmap::~Bitmap()
{
}

const BitmapPalette& Bitmap::GetGreyPalette( int nEntries )
{
    static BitmapPalette aGreyPalette2;
    static BitmapPalette aGreyPalette4;
    static BitmapPalette aGreyPalette16;
    static BitmapPalette aGreyPalette256;

    // Create greyscale palette with 2, 4, 16 or 256 entries
    if( 2 == nEntries || 4 == nEntries || 16 == nEntries || 256 == nEntries )
    {
        if( 2 == nEntries )
        {
            if( !aGreyPalette2.GetEntryCount() )
            {
                aGreyPalette2.SetEntryCount( 2 );
                aGreyPalette2[ 0 ] = BitmapColor( 0, 0, 0 );
                aGreyPalette2[ 1 ] = BitmapColor( 255, 255, 255 );
            }

            return aGreyPalette2;
        }
        else if( 4 == nEntries )
        {
            if( !aGreyPalette4.GetEntryCount() )
            {
                aGreyPalette4.SetEntryCount( 4 );
                aGreyPalette4[ 0 ] = BitmapColor( 0, 0, 0 );
                aGreyPalette4[ 1 ] = BitmapColor( 85, 85, 85 );
                aGreyPalette4[ 2 ] = BitmapColor( 170, 170, 170 );
                aGreyPalette4[ 3 ] = BitmapColor( 255, 255, 255 );
            }

            return aGreyPalette4;
        }
        else if( 16 == nEntries )
        {
            if( !aGreyPalette16.GetEntryCount() )
            {
                sal_uInt8 cGrey = 0;
                sal_uInt8 const cGreyInc = 17;

                aGreyPalette16.SetEntryCount( 16 );

                for( sal_uInt16 i = 0; i < 16; i++, cGrey = sal::static_int_cast<sal_uInt8>(cGrey + cGreyInc) )
                    aGreyPalette16[ i ] = BitmapColor( cGrey, cGrey, cGrey );
            }

            return aGreyPalette16;
        }
        else
        {
            if( !aGreyPalette256.GetEntryCount() )
            {
                aGreyPalette256.SetEntryCount( 256 );

                for( sal_uInt16 i = 0; i < 256; i++ )
                    aGreyPalette256[ i ] = BitmapColor( static_cast<sal_uInt8>(i), static_cast<sal_uInt8>(i), static_cast<sal_uInt8>(i) );
            }

            return aGreyPalette256;
        }
    }
    else
    {
        OSL_FAIL( "Bitmap::GetGreyPalette: invalid entry count (2/4/16/256 allowed)" );
        return aGreyPalette2;
    }
}

bool BitmapPalette::IsGreyPalette() const
{
    const int nEntryCount = GetEntryCount();
    if( !nEntryCount ) // NOTE: an empty palette means 1:1 mapping
        return true;
    // See above: only certain entry values will result in a valid call to GetGreyPalette
    if( nEntryCount == 2 || nEntryCount == 4 || nEntryCount == 16 || nEntryCount == 256 )
    {
        const BitmapPalette& rGreyPalette = Bitmap::GetGreyPalette( nEntryCount );
        if( rGreyPalette == *this )
            return true;
    }

    bool bRet = false;
    // TODO: is it worth to compare the entries for the general case?
    if (nEntryCount == 2)
    {
       const BitmapColor& rCol0(maBitmapColor[0]);
       const BitmapColor& rCol1(maBitmapColor[1]);
       bRet = rCol0.GetRed() == rCol0.GetGreen() && rCol0.GetRed() == rCol0.GetBlue() &&
              rCol1.GetRed() == rCol1.GetGreen() && rCol1.GetRed() == rCol1.GetBlue();
    }
    return bRet;
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

Bitmap& Bitmap::operator=( Bitmap&& rBitmap )
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
    BitmapChecksum aChecksum1, aChecksum2;
    rBmp.mxSalBmp->GetChecksum(aChecksum1);
    mxSalBmp->GetChecksum(aChecksum2);
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

sal_uInt16 Bitmap::GetBitCount() const
{
    if (!mxSalBmp)
        return 0;
    sal_uInt16 nBitCount = mxSalBmp->GetBitCount();
    return ( nBitCount <= 4 ) ? ( ( nBitCount <= 1 ) ? 1 : 4 ):
                                ( ( nBitCount <= 8 ) ? 8 : 24);
}

bool Bitmap::HasGreyPalette() const
{
    const sal_uInt16    nBitCount = GetBitCount();
    bool            bRet = nBitCount == 1;

    ScopedInfoAccess pIAcc(const_cast<Bitmap&>(*this));

    if( pIAcc )
    {
        bRet = pIAcc->HasPalette() && pIAcc->GetPalette().IsGreyPalette();
    }

    return bRet;
}

BitmapChecksum Bitmap::GetChecksum() const
{
    BitmapChecksum nRet = 0;

    if( mxSalBmp )
    {
        mxSalBmp->GetChecksum(nRet);

        if (!nRet)
        {
            // nRet == 0 => probably, we were not able to acquire
            // the buffer in SalBitmap::updateChecksum;
            // so, we need to update the imp bitmap for this bitmap instance
            // as we do in BitmapInfoAccess::ImplCreate
            std::shared_ptr<SalBitmap> xNewImpBmp(ImplGetSVData()->mpDefInst->CreateSalBitmap());
            if (xNewImpBmp->Create(*mxSalBmp, GetBitCount()))
            {
                Bitmap* pThis = const_cast<Bitmap*>(this);
                pThis->mxSalBmp = xNewImpBmp;
                mxSalBmp->GetChecksum(nRet);
            }
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
        mxSalBmp->Create(*xOldImpBmp);
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

BitmapInfoAccess* Bitmap::AcquireInfoAccess()
{
    std::unique_ptr<BitmapInfoAccess> pInfoAccess(new BitmapInfoAccess( *this ));

    if( !*pInfoAccess )
    {
        return nullptr;
    }

    return pInfoAccess.release();
}

BitmapReadAccess* Bitmap::AcquireReadAccess()
{
    std::unique_ptr<BitmapReadAccess> pReadAccess(new BitmapReadAccess( *this ));

    if( !*pReadAccess )
    {
        return nullptr;
    }

    return pReadAccess.release();
}

BitmapWriteAccess* Bitmap::AcquireWriteAccess()
{
    std::unique_ptr<BitmapWriteAccess> pWriteAccess(new BitmapWriteAccess( *this ));

    if( !*pWriteAccess )
    {
        return nullptr;
    }

    return pWriteAccess.release();
}

void Bitmap::ReleaseAccess( BitmapInfoAccess* pBitmapAccess )
{
    delete pBitmapAccess;
}

bool Bitmap::Crop( const tools::Rectangle& rRectPixel )
{
    const Size          aSizePix( GetSizePixel() );
    tools::Rectangle           aRect( rRectPixel );
    bool                bRet = false;

    aRect.Intersection( tools::Rectangle( Point(), aSizePix ) );

    if( !aRect.IsEmpty() && aSizePix != aRect.GetSize())
    {
        ScopedReadAccess pReadAcc(*this);

        if( pReadAcc )
        {
            const tools::Rectangle     aNewRect( Point(), aRect.GetSize() );
            Bitmap              aNewBmp( aNewRect.GetSize(), GetBitCount(), &pReadAcc->GetPalette() );
            BitmapScopedWriteAccess pWriteAcc(aNewBmp);

            if( pWriteAcc )
            {
                const long nOldX = aRect.Left();
                const long nOldY = aRect.Top();
                const long nNewWidth = aNewRect.GetWidth();
                const long nNewHeight = aNewRect.GetHeight();

                for( long nY = 0, nY2 = nOldY; nY < nNewHeight; nY++, nY2++ )
                {
                    Scanline pScanline = pWriteAcc->GetScanline(nY);
                    Scanline pScanlineRead = pReadAcc->GetScanline(nY2);
                    for( long nX = 0, nX2 = nOldX; nX < nNewWidth; nX++, nX2++ )
                        pWriteAcc->SetPixelOnData( pScanline, nX, pReadAcc->GetPixelFromData( pScanlineRead, nX2 ) );
                }

                pWriteAcc.reset();
                bRet = true;
            }

            pReadAcc.reset();

            if( bRet )
                ReassignWithSize( aNewBmp );
        }
    }

    return bRet;
};

bool Bitmap::CopyPixel( const tools::Rectangle& rRectDst,
                        const tools::Rectangle& rRectSrc, const Bitmap* pBmpSrc )
{
    const Size  aSizePix( GetSizePixel() );
    tools::Rectangle   aRectDst( rRectDst );
    bool        bRet = false;

    aRectDst.Intersection( tools::Rectangle( Point(), aSizePix ) );

    if( !aRectDst.IsEmpty() )
    {
        if( pBmpSrc && ( pBmpSrc->mxSalBmp != mxSalBmp ) )
        {
            Bitmap*         pSrc = const_cast<Bitmap*>(pBmpSrc);
            const Size      aCopySizePix( pSrc->GetSizePixel() );
            tools::Rectangle       aRectSrc( rRectSrc );
            const sal_uInt16    nSrcBitCount = pBmpSrc->GetBitCount();
            const sal_uInt16    nDstBitCount = GetBitCount();

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
                    Convert( BmpConversion::N4BitColors );
                    nNextIndex = 2;
                }

                if( nNextIndex )
                {
                    ScopedReadAccess    pSrcAcc(*pSrc);
                    BitmapScopedWriteAccess pDstAcc(*this);

                    if( pSrcAcc && pDstAcc )
                    {
                        const int nSrcCount = pDstAcc->GetPaletteEntryCount();
                        const int nDstCount = 1 << nDstBitCount;

                        for (int i = 0; ( i < nSrcCount ) && ( nNextIndex < nSrcCount ); ++i)
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

            if( !aRectSrc.IsEmpty() )
            {
                ScopedReadAccess pReadAcc(*pSrc);

                if( pReadAcc )
                {
                    BitmapScopedWriteAccess pWriteAcc(*this);

                    if( pWriteAcc )
                    {
                        const long  nWidth = std::min( aRectSrc.GetWidth(), aRectDst.GetWidth() );
                        const long  nHeight = std::min( aRectSrc.GetHeight(), aRectDst.GetHeight() );
                        const long  nSrcEndX = aRectSrc.Left() + nWidth;
                        const long  nSrcEndY = aRectSrc.Top() + nHeight;
                        long        nDstY = aRectDst.Top();

                        if( pReadAcc->HasPalette() && pWriteAcc->HasPalette() )
                        {
                            const sal_uInt16    nCount = pReadAcc->GetPaletteEntryCount();
                            std::unique_ptr<sal_uInt8[]> pMap(new sal_uInt8[ nCount ]);

                            // Create index map for the color table, as the bitmap should be copied
                            // retaining it's color information relatively well
                            for( sal_uInt16 i = 0; i < nCount; i++ )
                                pMap[ i ] = static_cast<sal_uInt8>(pWriteAcc->GetBestPaletteIndex( pReadAcc->GetPaletteColor( i ) ));

                            for( long nSrcY = aRectSrc.Top(); nSrcY < nSrcEndY; nSrcY++, nDstY++ )
                            {
                                Scanline pScanline = pWriteAcc->GetScanline(nDstY);
                                Scanline pScanlineRead = pReadAcc->GetScanline(nSrcY);
                                for( long nSrcX = aRectSrc.Left(), nDstX = aRectDst.Left(); nSrcX < nSrcEndX; nSrcX++, nDstX++ )
                                    pWriteAcc->SetPixelOnData( pScanline, nDstX, BitmapColor( pMap[ pReadAcc->GetIndexFromData( pScanlineRead, nSrcX ) ] ));
                            }
                        }
                        else if( pReadAcc->HasPalette() )
                        {
                            for( long nSrcY = aRectSrc.Top(); nSrcY < nSrcEndY; nSrcY++, nDstY++ )
                            {
                                Scanline pScanline = pWriteAcc->GetScanline(nDstY);
                                Scanline pScanlineRead = pReadAcc->GetScanline(nSrcY);
                                for( long nSrcX = aRectSrc.Left(), nDstX = aRectDst.Left(); nSrcX < nSrcEndX; nSrcX++, nDstX++ )
                                    pWriteAcc->SetPixelOnData( pScanline, nDstX, pReadAcc->GetPaletteColor( pReadAcc->GetIndexFromData( pScanlineRead, nSrcX ) ) );
                            }
                        }
                        else
                            for( long nSrcY = aRectSrc.Top(); nSrcY < nSrcEndY; nSrcY++, nDstY++ )
                            {
                                Scanline pScanline = pWriteAcc->GetScanline(nDstY);
                                Scanline pScanlineRead = pReadAcc->GetScanline(nSrcY);
                                for( long nSrcX = aRectSrc.Left(), nDstX = aRectDst.Left(); nSrcX < nSrcEndX; nSrcX++, nDstX++ )
                                    pWriteAcc->SetPixelOnData( pScanline, nDstX, pReadAcc->GetPixelFromData( pScanlineRead, nSrcX ) );
                            }

                        pWriteAcc.reset();
                        bRet = ( nWidth > 0 ) && ( nHeight > 0 );
                    }

                    pReadAcc.reset();
                }
            }
        }
        else
        {
            tools::Rectangle aRectSrc( rRectSrc );

            aRectSrc.Intersection( tools::Rectangle( Point(), aSizePix ) );

            if( !aRectSrc.IsEmpty() && ( aRectSrc != aRectDst ) )
            {
                BitmapScopedWriteAccess   pWriteAcc(*this);

                if( pWriteAcc )
                {
                    const long  nWidth = std::min( aRectSrc.GetWidth(), aRectDst.GetWidth() );
                    const long  nHeight = std::min( aRectSrc.GetHeight(), aRectDst.GetHeight() );
                    const long  nSrcX = aRectSrc.Left();
                    const long  nSrcY = aRectSrc.Top();
                    const long  nSrcEndX1 = nSrcX + nWidth - 1;
                    const long  nSrcEndY1 = nSrcY + nHeight - 1;
                    const long  nDstX = aRectDst.Left();
                    const long  nDstY = aRectDst.Top();
                    const long  nDstEndX1 = nDstX + nWidth - 1;
                    const long  nDstEndY1 = nDstY + nHeight - 1;

                    if( ( nDstX <= nSrcX ) && ( nDstY <= nSrcY ) )
                    {
                        for( long nY = nSrcY, nYN = nDstY; nY <= nSrcEndY1; nY++, nYN++ )
                        {
                            Scanline pScanline = pWriteAcc->GetScanline(nYN);
                            Scanline pScanlineSrc = pWriteAcc->GetScanline(nY);
                            for( long nX = nSrcX, nXN = nDstX; nX <= nSrcEndX1; nX++, nXN++ )
                                pWriteAcc->SetPixelOnData( pScanline, nXN, pWriteAcc->GetPixelFromData( pScanlineSrc, nX ) );
                        }
                    }
                    else if( ( nDstX <= nSrcX ) && ( nDstY >= nSrcY ) )
                    {
                        for( long nY = nSrcEndY1, nYN = nDstEndY1; nY >= nSrcY; nY--, nYN-- )
                        {
                            Scanline pScanline = pWriteAcc->GetScanline(nYN);
                            Scanline pScanlineSrc = pWriteAcc->GetScanline(nY);
                            for( long nX = nSrcX, nXN = nDstX; nX <= nSrcEndX1; nX++, nXN++ )
                                pWriteAcc->SetPixelOnData( pScanline, nXN, pWriteAcc->GetPixelFromData( pScanlineSrc, nX ) );
                        }
                    }
                    else if( ( nDstX >= nSrcX ) && ( nDstY <= nSrcY ) )
                    {
                        for( long nY = nSrcY, nYN = nDstY; nY <= nSrcEndY1; nY++, nYN++ )
                        {
                            Scanline pScanline = pWriteAcc->GetScanline(nYN);
                            Scanline pScanlineSrc = pWriteAcc->GetScanline(nY);
                            for( long nX = nSrcEndX1, nXN = nDstEndX1; nX >= nSrcX; nX--, nXN-- )
                                pWriteAcc->SetPixelOnData( pScanline, nXN, pWriteAcc->GetPixelFromData( pScanlineSrc, nX ) );
                        }
                    }
                    else
                    {
                        for( long nY = nSrcEndY1, nYN = nDstEndY1; nY >= nSrcY; nY--, nYN-- )
                        {
                            Scanline pScanline = pWriteAcc->GetScanline(nYN);
                            Scanline pScanlineSrc = pWriteAcc->GetScanline(nY);
                            for( long nX = nSrcEndX1, nXN = nDstEndX1; nX >= nSrcX; nX--, nXN-- )
                                pWriteAcc->SetPixelOnData( pScanline, nXN, pWriteAcc->GetPixelFromData( pScanlineSrc, nX ) );
                        }
                    }

                    pWriteAcc.reset();
                    bRet = true;
                }
            }
        }
    }

    return bRet;
}

bool Bitmap::CopyPixel_AlphaOptimized( const tools::Rectangle& rRectDst, const tools::Rectangle& rRectSrc,
                           const Bitmap* pBmpSrc )
{
    // Note: this code is copied from Bitmap::CopyPixel but avoids any palette lookups
    // This optimization is possible because the palettes of AlphaMasks are always identical (8bit GreyPalette, see ctor)
    const Size  aSizePix( GetSizePixel() );
    tools::Rectangle   aRectDst( rRectDst );
    bool        bRet = false;

    aRectDst.Intersection( tools::Rectangle( Point(), aSizePix ) );

    if( !aRectDst.IsEmpty() )
    {
        if( pBmpSrc && ( pBmpSrc->mxSalBmp != mxSalBmp ) )
        {
            Bitmap*         pSrc = const_cast<Bitmap*>(pBmpSrc);
            const Size      aCopySizePix( pSrc->GetSizePixel() );
            tools::Rectangle       aRectSrc( rRectSrc );

            aRectSrc.Intersection( tools::Rectangle( Point(), aCopySizePix ) );

            if( !aRectSrc.IsEmpty() )
            {
                ScopedReadAccess pReadAcc(*pSrc);

                if( pReadAcc )
                {
                    BitmapScopedWriteAccess pWriteAcc(*this);

                    if( pWriteAcc )
                    {
                        const long  nWidth = std::min( aRectSrc.GetWidth(), aRectDst.GetWidth() );
                        const long  nHeight = std::min( aRectSrc.GetHeight(), aRectDst.GetHeight() );
                        const long  nSrcEndX = aRectSrc.Left() + nWidth;
                        const long  nSrcEndY = aRectSrc.Top() + nHeight;
                        long        nDstY = aRectDst.Top();

                        for( long nSrcY = aRectSrc.Top(); nSrcY < nSrcEndY; nSrcY++, nDstY++)
                        {
                            Scanline pScanline = pWriteAcc->GetScanline(nDstY);
                            Scanline pScanlineRead = pReadAcc->GetScanline(nSrcY);
                            for( long nSrcX = aRectSrc.Left(), nDstX = aRectDst.Left(); nSrcX < nSrcEndX; nSrcX++, nDstX++ )
                                pWriteAcc->SetPixelOnData( pScanline, nDstX, pReadAcc->GetPixelFromData( pScanlineRead, nSrcX ) );
                        }

                        pWriteAcc.reset();
                        bRet = ( nWidth > 0 ) && ( nHeight > 0 );
                    }

                    pReadAcc.reset();
                }
            }
        }
        else
        {
            tools::Rectangle aRectSrc( rRectSrc );

            aRectSrc.Intersection( tools::Rectangle( Point(), aSizePix ) );

            if( !aRectSrc.IsEmpty() && ( aRectSrc != aRectDst ) )
            {
                BitmapScopedWriteAccess   pWriteAcc(*this);

                if( pWriteAcc )
                {
                    const long  nWidth = std::min( aRectSrc.GetWidth(), aRectDst.GetWidth() );
                    const long  nHeight = std::min( aRectSrc.GetHeight(), aRectDst.GetHeight() );
                    const long  nSrcX = aRectSrc.Left();
                    const long  nSrcY = aRectSrc.Top();
                    const long  nSrcEndX1 = nSrcX + nWidth - 1;
                    const long  nSrcEndY1 = nSrcY + nHeight - 1;
                    const long  nDstX = aRectDst.Left();
                    const long  nDstY = aRectDst.Top();
                    const long  nDstEndX1 = nDstX + nWidth - 1;
                    const long  nDstEndY1 = nDstY + nHeight - 1;

                    if( ( nDstX <= nSrcX ) && ( nDstY <= nSrcY ) )
                    {
                        for( long nY = nSrcY, nYN = nDstY; nY <= nSrcEndY1; nY++, nYN++ )
                        {
                            Scanline pScanline = pWriteAcc->GetScanline(nYN);
                            Scanline pScanlineSrc = pWriteAcc->GetScanline(nY);
                            for( long nX = nSrcX, nXN = nDstX; nX <= nSrcEndX1; nX++, nXN++ )
                                pWriteAcc->SetPixelOnData( pScanline, nXN, pWriteAcc->GetPixelFromData( pScanlineSrc, nX ) );
                        }
                    }
                    else if( ( nDstX <= nSrcX ) && ( nDstY >= nSrcY ) )
                    {
                        for( long nY = nSrcEndY1, nYN = nDstEndY1; nY >= nSrcY; nY--, nYN-- )
                        {
                            Scanline pScanline = pWriteAcc->GetScanline(nYN);
                            Scanline pScanlineSrc = pWriteAcc->GetScanline(nY);
                            for( long nX = nSrcX, nXN = nDstX; nX <= nSrcEndX1; nX++, nXN++ )
                                pWriteAcc->SetPixelOnData( pScanline, nXN, pWriteAcc->GetPixelFromData( pScanlineSrc, nX ) );
                        }
                    }
                    else if( ( nDstX >= nSrcX ) && ( nDstY <= nSrcY ) )
                    {
                        for( long nY = nSrcY, nYN = nDstY; nY <= nSrcEndY1; nY++, nYN++ )
                        {
                            Scanline pScanline = pWriteAcc->GetScanline(nYN);
                            Scanline pScanlineSrc = pWriteAcc->GetScanline(nY);
                            for( long nX = nSrcEndX1, nXN = nDstEndX1; nX >= nSrcX; nX--, nXN-- )
                                pWriteAcc->SetPixelOnData( pScanline, nXN, pWriteAcc->GetPixelFromData( pScanlineSrc, nX ) );
                        }
                    }
                    else
                    {
                        for( long nY = nSrcEndY1, nYN = nDstEndY1; nY >= nSrcY; nY--, nYN-- )
                        {
                            Scanline pScanline = pWriteAcc->GetScanline(nYN);
                            Scanline pScanlineSrc = pWriteAcc->GetScanline(nY);
                            for( long nX = nSrcEndX1, nXN = nDstEndX1; nX >= nSrcX; nX--, nXN-- )
                                pWriteAcc->SetPixelOnData( pScanline, nXN, pWriteAcc->GetPixelFromData( pScanlineSrc, nX ) );
                        }
                    }

                    pWriteAcc.reset();
                    bRet = true;
                }
            }
        }
    }

    return bRet;

}

bool Bitmap::Expand( sal_uLong nDX, sal_uLong nDY, const Color* pInitColor )
{
    bool bRet = false;

    if( nDX || nDY )
    {
        const Size          aSizePixel( GetSizePixel() );
        const long          nWidth = aSizePixel.Width();
        const long          nHeight = aSizePixel.Height();
        const Size          aNewSize( nWidth + nDX, nHeight + nDY );
        ScopedReadAccess    pReadAcc(*this);

        if( pReadAcc )
        {
            BitmapPalette       aBmpPal( pReadAcc->GetPalette() );
            Bitmap              aNewBmp( aNewSize, GetBitCount(), &aBmpPal );
            BitmapScopedWriteAccess pWriteAcc(aNewBmp);

            if( pWriteAcc )
            {
                BitmapColor aColor;
                const long  nNewX = nWidth;
                const long  nNewY = nHeight;
                const long  nNewWidth = pWriteAcc->Width();
                const long  nNewHeight = pWriteAcc->Height();
                long        nX;
                long        nY;

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
                bRet = true;
            }

            pReadAcc.reset();

            if (bRet)
                ReassignWithSize(aNewBmp);
        }
    }

    return bRet;
}

Bitmap Bitmap::CreateDisplayBitmap( OutputDevice* pDisplay )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
