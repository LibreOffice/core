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
#include <vcl/bitmapaccess.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/BitmapSmoothenFilter.hxx>
#include <vcl/BitmapSharpenFilter.hxx>
#include <vcl/BitmapMedianFilter.hxx>
#include <vcl/BitmapSobelGreyFilter.hxx>
#include <vcl/BitmapSolarizeFilter.hxx>
#include <vcl/BitmapPopArtFilter.hxx>

#include <bitmapwriteaccess.hxx>

#include <memory>
#include <stdlib.h>

static inline sal_uInt8 lcl_getDuotoneColorComponent( sal_uInt8 base, sal_uInt16 color1, sal_uInt16 color2 )
{
    color2 = color2*base/0xFF;
    color1 = color1*(0xFF-base)/0xFF;

    return static_cast<sal_uInt8>(color1+color2);
}

bool Bitmap::Filter( BmpFilter eFilter, const BmpFilterParam* pFilterParam )
{
    bool bRet = false;

    switch( eFilter )
    {
        case BmpFilter::Smooth:
        {
            BitmapEx aBmpEx(*this);
            bRet = BitmapFilter::Filter(aBmpEx, BitmapSmoothenFilter(pFilterParam->mnRadius));
            *this = aBmpEx.GetBitmap();
        }
        break;

        case BmpFilter::Sharpen:
        {
            BitmapEx aBmpEx(*this);
            bRet = BitmapFilter::Filter(aBmpEx, BitmapSharpenFilter());
            *this = aBmpEx.GetBitmap();
        }
        break;

        case BmpFilter::RemoveNoise:
        {
            BitmapEx aBmpEx(*this);
            bRet = BitmapFilter::Filter(aBmpEx, BitmapMedianFilter());
            *this = aBmpEx.GetBitmap();
        }
        break;

        case BmpFilter::SobelGrey:
        {
            BitmapEx aBmpEx(*this);
            bRet = BitmapFilter::Filter(aBmpEx, BitmapSobelGreyFilter());
            *this = aBmpEx.GetBitmap();
        }
        break;

        case BmpFilter::Solarize:
        {
            BitmapEx aBmpEx(*this);
            bRet = BitmapFilter::Filter(aBmpEx, BitmapSolarizeFilter(pFilterParam->mcSolarGreyThreshold));
            *this = aBmpEx.GetBitmap();
        }
        break;

        case BmpFilter::Sepia:
            bRet = ImplSepia( pFilterParam );
        break;

        case BmpFilter::Mosaic:
            bRet = ImplMosaic( pFilterParam );
        break;

        case BmpFilter::EmbossGrey:
            bRet = ImplEmbossGrey( pFilterParam );
        break;

        case BmpFilter::PopArt:
        {
            BitmapEx aBmpEx(*this);
            bRet = BitmapFilter::Filter(aBmpEx, BitmapPopArtFilter());
            *this = aBmpEx.GetBitmap();
        }
        break;

        case BmpFilter::DuoTone:
            bRet = ImplDuotoneFilter( pFilterParam->mnProgressStart, pFilterParam->mnProgressEnd );
        break;

        default:
            OSL_FAIL( "Bitmap::Convert(): Unsupported filter" );
        break;
    }

    return bRet;
}

bool Bitmap::ImplEmbossGrey( const BmpFilterParam* pFilterParam )
{
    bool bRet = ImplMakeGreyscales( 256 );

    if( bRet )
    {
        bRet = false;

        ScopedReadAccess pReadAcc(*this);

        if( pReadAcc )
        {
            Bitmap              aNewBmp( GetSizePixel(), 8, &pReadAcc->GetPalette() );
            BitmapScopedWriteAccess pWriteAcc(aNewBmp);

            if( pWriteAcc )
            {
                BitmapColor aGrey( sal_uInt8(0) );
                const long  nWidth = pWriteAcc->Width();
                const long  nHeight = pWriteAcc->Height();
                long        nGrey11, nGrey12, nGrey13;
                long        nGrey21, nGrey22, nGrey23;
                long        nGrey31, nGrey32, nGrey33;
                double      fAzim = ( ( pFilterParam && pFilterParam->meFilter == BmpFilter::EmbossGrey ) ?
                                      ( pFilterParam->maEmbossAngles.mnAzimuthAngle100 * 0.01 ) : 0.0 ) * F_PI180;
                double      fElev = ( ( pFilterParam && pFilterParam->meFilter == BmpFilter::EmbossGrey ) ?
                                      ( pFilterParam->maEmbossAngles.mnElevationAngle100 * 0.01 ) : 90.0 ) * F_PI180;
                long*       pHMap = new long[ nWidth + 2 ];
                long*       pVMap = new long[ nHeight + 2 ];
                long        nX, nY, nNx, nNy, nDotL;
                const long  nLx = FRound( cos( fAzim ) * cos( fElev ) * 255.0 );
                const long  nLy = FRound( sin( fAzim ) * cos( fElev ) * 255.0 );
                const long  nLz = FRound( sin( fElev ) * 255.0 );
                const auto  nZ2 = ( ( 6 * 255 ) / 4 ) * ( ( 6 * 255 ) / 4 );
                const long  nNzLz = ( ( 6 * 255 ) / 4 ) * nLz;
                const sal_uInt8 cLz = static_cast<sal_uInt8>(SAL_BOUND( nLz, 0, 255 ));

                // fill mapping tables
                pHMap[ 0 ] = 0;
                for( nX = 1; nX <= nWidth; nX++ )
                    pHMap[ nX ] = nX - 1;
                pHMap[ nWidth + 1 ] = nWidth - 1;

                pVMap[ 0 ] = 0;
                for( nY = 1; nY <= nHeight; nY++ )
                    pVMap[ nY ] = nY - 1;
                pVMap[ nHeight + 1 ] = nHeight - 1;

                for( nY = 0; nY < nHeight ; nY++ )
                {
                    nGrey11 = pReadAcc->GetPixel( pVMap[ nY ], pHMap[ 0 ] ).GetIndex();
                    nGrey12 = pReadAcc->GetPixel( pVMap[ nY ], pHMap[ 1 ] ).GetIndex();
                    nGrey13 = pReadAcc->GetPixel( pVMap[ nY ], pHMap[ 2 ] ).GetIndex();
                    nGrey21 = pReadAcc->GetPixel( pVMap[ nY + 1 ], pHMap[ 0 ] ).GetIndex();
                    nGrey22 = pReadAcc->GetPixel( pVMap[ nY + 1 ], pHMap[ 1 ] ).GetIndex();
                    nGrey23 = pReadAcc->GetPixel( pVMap[ nY + 1 ], pHMap[ 2 ] ).GetIndex();
                    nGrey31 = pReadAcc->GetPixel( pVMap[ nY + 2 ], pHMap[ 0 ] ).GetIndex();
                    nGrey32 = pReadAcc->GetPixel( pVMap[ nY + 2 ], pHMap[ 1 ] ).GetIndex();
                    nGrey33 = pReadAcc->GetPixel( pVMap[ nY + 2 ], pHMap[ 2 ] ).GetIndex();

                    Scanline pScanline = pWriteAcc->GetScanline(nY);
                    for( nX = 0; nX < nWidth; nX++ )
                    {
                        nNx = nGrey11 + nGrey21 + nGrey31 - nGrey13 - nGrey23 - nGrey33;
                        nNy = nGrey31 + nGrey32 + nGrey33 - nGrey11 - nGrey12 - nGrey13;

                        if( !nNx && !nNy )
                            aGrey.SetIndex( cLz );
                        else if( ( nDotL = nNx * nLx + nNy * nLy +nNzLz ) < 0 )
                            aGrey.SetIndex( 0 );
                        else
                        {
                            const double fGrey = nDotL / sqrt( static_cast<double>(nNx * nNx + nNy * nNy + nZ2) );
                            aGrey.SetIndex( static_cast<sal_uInt8>(SAL_BOUND( fGrey, 0, 255 )) );
                        }

                        pWriteAcc->SetPixelOnData( pScanline, nX, aGrey );

                        if( nX < ( nWidth - 1 ) )
                        {
                            const long nNextX = pHMap[ nX + 3 ];

                            nGrey11 = nGrey12; nGrey12 = nGrey13; nGrey13 = pReadAcc->GetPixel( pVMap[ nY ], nNextX ).GetIndex();
                            nGrey21 = nGrey22; nGrey22 = nGrey23; nGrey23 = pReadAcc->GetPixel( pVMap[ nY + 1 ], nNextX ).GetIndex();
                            nGrey31 = nGrey32; nGrey32 = nGrey33; nGrey33 = pReadAcc->GetPixel( pVMap[ nY + 2 ], nNextX ).GetIndex();
                        }
                    }
                }

                delete[] pHMap;
                delete[] pVMap;
                pWriteAcc.reset();
                bRet = true;
            }

            pReadAcc.reset();

            if( bRet )
            {
                const MapMode   aMap( maPrefMapMode );
                const Size      aSize( maPrefSize );

                *this = aNewBmp;

                maPrefMapMode = aMap;
                maPrefSize = aSize;
            }
        }
    }

    return bRet;
}

bool Bitmap::ImplSepia( const BmpFilterParam* pFilterParam )
{
    ScopedReadAccess    pReadAcc(*this);
    bool                bRet = false;

    if( pReadAcc )
    {
        long            nSepiaPercent = ( pFilterParam && pFilterParam->meFilter == BmpFilter::Sepia ) ?
                                        pFilterParam->mnSepiaPercent : 10;
        const long      nSepia = 10000 - 100 * SAL_BOUND( nSepiaPercent, 0, 100 );
        BitmapPalette   aSepiaPal( 256 );

        for( sal_uInt16 i = 0; i < 256; i++ )
        {
            BitmapColor&    rCol = aSepiaPal[ i ];
            const sal_uInt8 cSepiaValue = static_cast<sal_uInt8>( nSepia * i / 10000 );

            rCol.SetRed( static_cast<sal_uInt8>(i) );
            rCol.SetGreen( cSepiaValue );
            rCol.SetBlue( cSepiaValue );
        }

        Bitmap              aNewBmp( GetSizePixel(), 8, &aSepiaPal );
        BitmapScopedWriteAccess pWriteAcc(aNewBmp);

        if( pWriteAcc )
        {
            BitmapColor aCol( sal_uInt8(0) );
            const long  nWidth = pWriteAcc->Width();
            const long  nHeight = pWriteAcc->Height();

            if( pReadAcc->HasPalette() )
            {
                const sal_uInt16             nPalCount = pReadAcc->GetPaletteEntryCount();
                std::unique_ptr<sal_uInt8[]> pIndexMap( new sal_uInt8[ nPalCount ] );
                for( sal_uInt16 i = 0; i < nPalCount; i++ )
                    pIndexMap[ i ] = pReadAcc->GetPaletteColor( i ).GetLuminance();

                for( long nY = 0; nY < nHeight ; nY++ )
                {
                    Scanline pScanline = pWriteAcc->GetScanline(nY);
                    Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                    for( long nX = 0; nX < nWidth; nX++ )
                    {
                        aCol.SetIndex( pIndexMap[ pReadAcc->GetIndexFromData( pScanlineRead, nX ) ] );
                        pWriteAcc->SetPixelOnData( pScanline, nX, aCol );
                    }
                }
            }
            else
            {
                for( long nY = 0; nY < nHeight ; nY++ )
                {
                    Scanline pScanline = pWriteAcc->GetScanline(nY);
                    Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                    for( long nX = 0; nX < nWidth; nX++ )
                    {
                        aCol.SetIndex( pReadAcc->GetPixelFromData( pScanlineRead, nX ).GetLuminance() );
                        pWriteAcc->SetPixelOnData( pScanline, nX, aCol );
                    }
                }
            }

            pWriteAcc.reset();
            bRet = true;
        }

        pReadAcc.reset();

        if( bRet )
        {
            const MapMode   aMap( maPrefMapMode );
            const Size      aSize( maPrefSize );

            *this = aNewBmp;

            maPrefMapMode = aMap;
            maPrefSize = aSize;
        }
    }

    return bRet;
}

bool Bitmap::ImplMosaic( const BmpFilterParam* pFilterParam )
{
    sal_uLong               nTileWidth = ( pFilterParam && pFilterParam->meFilter == BmpFilter::Mosaic ) ?
                                     pFilterParam->maMosaicTileSize.mnTileWidth : 4;
    sal_uLong               nTileHeight = ( pFilterParam && pFilterParam->meFilter == BmpFilter::Mosaic ) ?
                                      pFilterParam->maMosaicTileSize.mnTileHeight : 4;
    bool                bRet = false;

    if( !nTileWidth )
        nTileWidth = 1;

    if( !nTileHeight )
        nTileHeight = 1;

    if( nTileWidth > 1 || nTileHeight > 1 )
    {
        Bitmap*             pNewBmp;
        BitmapReadAccess*   pReadAcc;
        BitmapWriteAccess*  pWriteAcc;

        if( GetBitCount() > 8 )
        {
            pNewBmp = nullptr;
            pReadAcc = pWriteAcc = AcquireWriteAccess();
        }
        else
        {
            pNewBmp = new Bitmap( GetSizePixel(), 24 );
            pReadAcc = AcquireReadAccess();
            pWriteAcc = pNewBmp->AcquireWriteAccess();
        }

        bool bConditionsMet = false;
        long nWidth(0);
        long nHeight(0);
        if (pReadAcc && pWriteAcc)
        {
            nWidth = pReadAcc->Width();
            nHeight = pReadAcc->Height();
            bConditionsMet = (nWidth > 0 && nHeight > 0);
        }

        if (bConditionsMet)
        {
            BitmapColor aCol;
            long        nX, nY, nX1, nX2, nY1, nY2, nSumR, nSumG, nSumB;
            double      fArea_1;

            nY1 = 0; nY2 = nTileHeight - 1;

            if( nY2 >= nHeight )
                nY2 = nHeight - 1;

            do
            {
                nX1 = 0; nX2 = nTileWidth - 1;

                if( nX2 >= nWidth )
                    nX2 = nWidth - 1;

                fArea_1 = 1.0 / ( ( nX2 - nX1 + 1 ) * ( nY2 - nY1 + 1 ) );

                if( !pNewBmp )
                {
                    do
                    {
                        for( nY = nY1, nSumR = nSumG = nSumB = 0; nY <= nY2; nY++ )
                        {
                            Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                            for( nX = nX1; nX <= nX2; nX++ )
                            {
                                aCol = pReadAcc->GetPixelFromData( pScanlineRead, nX );
                                nSumR += aCol.GetRed();
                                nSumG += aCol.GetGreen();
                                nSumB += aCol.GetBlue();
                            }
                        }

                        aCol.SetRed( static_cast<sal_uInt8>( nSumR * fArea_1 ) );
                        aCol.SetGreen( static_cast<sal_uInt8>( nSumG * fArea_1 ) );
                        aCol.SetBlue( static_cast<sal_uInt8>( nSumB * fArea_1 ) );

                        for( nY = nY1; nY <= nY2; nY++ )
                        {
                            Scanline pScanline = pWriteAcc->GetScanline(nY);
                            for( nX = nX1; nX <= nX2; nX++ )
                                pWriteAcc->SetPixelOnData( pScanline, nX, aCol );
                        }

                        nX1 += nTileWidth; nX2 += nTileWidth;

                        if( nX2 >= nWidth )
                        {
                            nX2 = nWidth - 1;
                            fArea_1 = 1.0 / ( ( nX2 - nX1 + 1 ) * ( nY2 - nY1 + 1 ) );
                        }
                    }
                    while( nX1 < nWidth );
                }
                else
                {
                    do
                    {
                        for( nY = nY1, nSumR = nSumG = nSumB = 0; nY <= nY2; nY++ )
                        {
                            Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                            for( nX = nX1; nX <= nX2; nX++ )
                            {
                                const BitmapColor& rCol = pReadAcc->GetPaletteColor( pReadAcc->GetIndexFromData( pScanlineRead, nX ) );
                                nSumR += rCol.GetRed();
                                nSumG += rCol.GetGreen();
                                nSumB += rCol.GetBlue();
                            }
                        }

                        aCol.SetRed( static_cast<sal_uInt8>( nSumR * fArea_1 ) );
                        aCol.SetGreen( static_cast<sal_uInt8>( nSumG * fArea_1 ) );
                        aCol.SetBlue( static_cast<sal_uInt8>( nSumB * fArea_1 ) );

                        for( nY = nY1; nY <= nY2; nY++ )
                        {
                            Scanline pScanline = pWriteAcc->GetScanline(nY);
                            for( nX = nX1; nX <= nX2; nX++ )
                                pWriteAcc->SetPixelOnData( pScanline, nX, aCol );
                        }

                        nX1 += nTileWidth; nX2 += nTileWidth;

                        if( nX2 >= nWidth )
                        {
                            nX2 = nWidth - 1;
                            fArea_1 = 1.0 / ( ( nX2 - nX1 + 1 ) * ( nY2 - nY1 + 1 ) );
                        }
                    }
                    while( nX1 < nWidth );
                }

                nY1 += nTileHeight; nY2 += nTileHeight;

                if( nY2 >= nHeight )
                    nY2 = nHeight - 1;
            }
            while( nY1 < nHeight );

            bRet = true;
        }

        ReleaseAccess( pReadAcc );

        if( pNewBmp )
        {
            Bitmap::ReleaseAccess( pWriteAcc );

            if( bRet )
            {
                const MapMode   aMap( maPrefMapMode );
                const Size      aSize( maPrefSize );

                *this = *pNewBmp;

                maPrefMapMode = aMap;
                maPrefSize = aSize;
            }

            delete pNewBmp;
        }
    }
    else
        bRet = true;

    return bRet;
}

bool Bitmap::ImplDuotoneFilter( const sal_uLong nColorOne, const sal_uLong nColorTwo )
{
    const long  nWidth = GetSizePixel().Width();
    const long  nHeight = GetSizePixel().Height();

    Bitmap aResultBitmap( GetSizePixel(), 24);
    ScopedReadAccess pReadAcc(*this);
    BitmapScopedWriteAccess pWriteAcc(aResultBitmap);
    const BitmapColor aColorOne( static_cast< sal_uInt8 >( nColorOne >> 16 ), static_cast< sal_uInt8 >( nColorOne >> 8 ), static_cast< sal_uInt8 >( nColorOne ) );
    const BitmapColor aColorTwo( static_cast< sal_uInt8 >( nColorTwo >> 16 ), static_cast< sal_uInt8 >( nColorTwo >> 8 ), static_cast< sal_uInt8 >( nColorTwo ) );

    for( long x = 0; x < nWidth; x++ )
    {
        for( long y = 0; y < nHeight; y++ )
        {
            BitmapColor aColor = pReadAcc->GetColor( y, x );
            sal_uInt8 luminance = aColor.GetLuminance();
            BitmapColor aResultColor(
                    lcl_getDuotoneColorComponent( luminance, aColorOne.GetRed(), aColorTwo.GetRed() ) ,
                    lcl_getDuotoneColorComponent( luminance, aColorOne.GetGreen(), aColorTwo.GetGreen() ) ,
                    lcl_getDuotoneColorComponent( luminance, aColorOne.GetBlue(), aColorTwo.GetBlue() ) );
            pWriteAcc->SetPixel( y, x, aResultColor );
        }
    }

    pWriteAcc.reset();
    pReadAcc.reset();
    ReassignWithSize(aResultBitmap);
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
