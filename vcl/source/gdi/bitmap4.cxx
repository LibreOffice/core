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


#include <stdlib.h>
#include <vcl/bmpacc.hxx>
#include <vcl/bitmap.hxx>

#define S2(a,b)             { long t; if( ( t = b - a ) < 0 ) { a += t; b -= t; } }
#define MN3(a,b,c)          S2(a,b); S2(a,c);
#define MX3(a,b,c)          S2(b,c); S2(a,c);
#define MNMX3(a,b,c)        MX3(a,b,c); S2(a,b);
#define MNMX4(a,b,c,d)      S2(a,b); S2(c,d); S2(a,c); S2(b,d);
#define MNMX5(a,b,c,d,e)    S2(a,b); S2(c,d); MN3(a,c,e); MX3(b,d,e);
#define MNMX6(a,b,c,d,e,f)  S2(a,d); S2(b,e); S2(c,f); MN3(a,b,c); MX3(d,e,f);

static inline sal_uInt8 lcl_getDuotoneColorComponent( sal_uInt8 base, sal_uInt16 color1, sal_uInt16 color2 )
{
    color2 = color2*base/0xFF;
    color1 = color1*(0xFF-base)/0xFF;

    return (sal_uInt8) (color1+color2);
}

sal_Bool Bitmap::Filter( BmpFilter eFilter, const BmpFilterParam* pFilterParam, const Link* pProgress )
{
    sal_Bool bRet = sal_False;

    switch( eFilter )
    {
        case( BMP_FILTER_SMOOTH ):
        {
            // Blur for positive values of mnRadius
            if (pFilterParam->mnRadius > 0.0)
            {
                bRet = ImplSeparableBlurFilter(pFilterParam->mnRadius);
            }
            // Unsharpen Mask for negative values of mnRadius
            else if (pFilterParam->mnRadius < 0.0)
            {
                bRet = ImplSeparableUnsharpenFilter(pFilterParam->mnRadius);
            }
            else
            {
                bRet = sal_False;
            }
        }
        break;

        case( BMP_FILTER_SHARPEN ):
        {
            const long pSharpenMatrix[] = { -1, -1,  -1, -1, 16, -1, -1, -1,  -1 };
            bRet = ImplConvolute3( &pSharpenMatrix[ 0 ], 8, pFilterParam, pProgress );
        }
        break;

        case( BMP_FILTER_REMOVENOISE ):
            bRet = ImplMedianFilter( pFilterParam, pProgress );
        break;

        case( BMP_FILTER_SOBEL_GREY ):
            bRet = ImplSobelGrey( pFilterParam, pProgress );
        break;

        case( BMP_FILTER_SOLARIZE ):
            bRet = ImplSolarize( pFilterParam, pProgress );
        break;

        case( BMP_FILTER_SEPIA ):
            bRet = ImplSepia( pFilterParam, pProgress );
        break;

        case( BMP_FILTER_MOSAIC ):
            bRet = ImplMosaic( pFilterParam, pProgress );
        break;

        case( BMP_FILTER_EMBOSS_GREY ):
            bRet = ImplEmbossGrey( pFilterParam, pProgress );
        break;

        case( BMP_FILTER_POPART ):
            bRet = ImplPopArt( pFilterParam, pProgress );
        break;

        case( BMP_FILTER_DUOTONE ):
            bRet = ImplDuotoneFilter( pFilterParam->mnProgressStart, pFilterParam->mnProgressEnd );
        break;

        default:
            OSL_FAIL( "Bitmap::Convert(): Unsupported filter" );
        break;
    }

    return bRet;
}

sal_Bool Bitmap::ImplConvolute3( const long* pMatrix, long nDivisor,
                             const BmpFilterParam* /*pFilterParam*/, const Link* /*pProgress*/ )
{
    BitmapReadAccess*   pReadAcc = AcquireReadAccess();
    sal_Bool                bRet = sal_False;

    if( pReadAcc )
    {
        Bitmap              aNewBmp( GetSizePixel(), 24 );
        BitmapWriteAccess*  pWriteAcc = aNewBmp.AcquireWriteAccess();

        if( pWriteAcc )
        {
            const long      nWidth = pWriteAcc->Width(), nWidth2 = nWidth + 2;
            const long      nHeight = pWriteAcc->Height(), nHeight2 = nHeight + 2;
            long*           pColm = new long[ nWidth2 ];
            long*           pRows = new long[ nHeight2 ];
            BitmapColor*    pColRow1 = (BitmapColor*) new sal_uInt8[ sizeof( BitmapColor ) * nWidth2 ];
            BitmapColor*    pColRow2 = (BitmapColor*) new sal_uInt8[ sizeof( BitmapColor ) * nWidth2 ];
            BitmapColor*    pColRow3 = (BitmapColor*) new sal_uInt8[ sizeof( BitmapColor ) * nWidth2 ];
            BitmapColor*    pRowTmp1 = pColRow1;
            BitmapColor*    pRowTmp2 = pColRow2;
            BitmapColor*    pRowTmp3 = pColRow3;
            BitmapColor*    pColor;
            long            nY, nX, i, nSumR, nSumG, nSumB, nMatrixVal, nTmp;
            long            (*pKoeff)[ 256 ] = new long[ 9 ][ 256 ];
            long*           pTmp;

            // create LUT of products of matrix value and possible color component values
            for( nY = 0; nY < 9; nY++ )
                for( nX = nTmp = 0, nMatrixVal = pMatrix[ nY ]; nX < 256; nX++, nTmp += nMatrixVal )
                    pKoeff[ nY ][ nX ] = nTmp;

            // create column LUT
            for( i = 0; i < nWidth2; i++ )
                pColm[ i ] = ( i > 0 ) ? ( i - 1 ) : 0;

            pColm[ nWidth + 1 ] = pColm[ nWidth ];

            // create row LUT
            for( i = 0; i < nHeight2; i++ )
                pRows[ i ] = ( i > 0 ) ? ( i - 1 ) : 0;

            pRows[ nHeight + 1 ] = pRows[ nHeight ];

            // read first three rows of bitmap color
            for( i = 0; i < nWidth2; i++ )
            {
                pColRow1[ i ] = pReadAcc->GetColor( pRows[ 0 ], pColm[ i ] );
                pColRow2[ i ] = pReadAcc->GetColor( pRows[ 1 ], pColm[ i ] );
                pColRow3[ i ] = pReadAcc->GetColor( pRows[ 2 ], pColm[ i ] );
            }

            // do convolution
            for( nY = 0; nY < nHeight; )
            {
                for( nX = 0; nX < nWidth; nX++ )
                {
                    // first row
                    nSumR = ( pTmp = pKoeff[ 0 ] )[ ( pColor = pRowTmp1 + nX )->GetRed() ];
                    nSumG = pTmp[ pColor->GetGreen() ];
                    nSumB = pTmp[ pColor->GetBlue() ];

                    nSumR += ( pTmp = pKoeff[ 1 ] )[ ( ++pColor )->GetRed() ];
                    nSumG += pTmp[ pColor->GetGreen() ];
                    nSumB += pTmp[ pColor->GetBlue() ];

                    nSumR += ( pTmp = pKoeff[ 2 ] )[ ( ++pColor )->GetRed() ];
                    nSumG += pTmp[ pColor->GetGreen() ];
                    nSumB += pTmp[ pColor->GetBlue() ];

                    // second row
                    nSumR += ( pTmp = pKoeff[ 3 ] )[ ( pColor = pRowTmp2 + nX )->GetRed() ];
                    nSumG += pTmp[ pColor->GetGreen() ];
                    nSumB += pTmp[ pColor->GetBlue() ];

                    nSumR += ( pTmp = pKoeff[ 4 ] )[ ( ++pColor )->GetRed() ];
                    nSumG += pTmp[ pColor->GetGreen() ];
                    nSumB += pTmp[ pColor->GetBlue() ];

                    nSumR += ( pTmp = pKoeff[ 5 ] )[ ( ++pColor )->GetRed() ];
                    nSumG += pTmp[ pColor->GetGreen() ];
                    nSumB += pTmp[ pColor->GetBlue() ];

                    // third row
                    nSumR += ( pTmp = pKoeff[ 6 ] )[ ( pColor = pRowTmp3 + nX )->GetRed() ];
                    nSumG += pTmp[ pColor->GetGreen() ];
                    nSumB += pTmp[ pColor->GetBlue() ];

                    nSumR += ( pTmp = pKoeff[ 7 ] )[ ( ++pColor )->GetRed() ];
                    nSumG += pTmp[ pColor->GetGreen() ];
                    nSumB += pTmp[ pColor->GetBlue() ];

                    nSumR += ( pTmp = pKoeff[ 8 ] )[ ( ++pColor )->GetRed() ];
                    nSumG += pTmp[ pColor->GetGreen() ];
                    nSumB += pTmp[ pColor->GetBlue() ];

                    // calculate destination color
                    pWriteAcc->SetPixel( nY, nX, BitmapColor( (sal_uInt8) MinMax( nSumR / nDivisor, 0, 255 ),
                                                              (sal_uInt8) MinMax( nSumG / nDivisor, 0, 255 ),
                                                              (sal_uInt8) MinMax( nSumB / nDivisor, 0, 255 ) ) );
                }

                if( ++nY < nHeight )
                {
                    if( pRowTmp1 == pColRow1 )
                        pRowTmp1 = pColRow2, pRowTmp2 = pColRow3, pRowTmp3 = pColRow1;
                    else if( pRowTmp1 == pColRow2 )
                        pRowTmp1 = pColRow3, pRowTmp2 = pColRow1, pRowTmp3 = pColRow2;
                    else
                        pRowTmp1 = pColRow1, pRowTmp2 = pColRow2, pRowTmp3 = pColRow3;

                    for( i = 0; i < nWidth2; i++ )
                        pRowTmp3[ i ] = pReadAcc->GetColor( pRows[ nY + 2 ], pColm[ i ] );
                }
            }

            delete[] pKoeff;
            delete[] (sal_uInt8*) pColRow1;
            delete[] (sal_uInt8*) pColRow2;
            delete[] (sal_uInt8*) pColRow3;
            delete[] pColm;
            delete[] pRows;

            aNewBmp.ReleaseAccess( pWriteAcc );

            bRet = sal_True;
        }

        ReleaseAccess( pReadAcc );

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

sal_Bool Bitmap::ImplMedianFilter( const BmpFilterParam* /*pFilterParam*/, const Link* /*pProgress*/ )
{
    BitmapReadAccess*   pReadAcc = AcquireReadAccess();
    sal_Bool                bRet = sal_False;

    if( pReadAcc )
    {
        Bitmap              aNewBmp( GetSizePixel(), 24 );
        BitmapWriteAccess*  pWriteAcc = aNewBmp.AcquireWriteAccess();

        if( pWriteAcc )
        {
            const long      nWidth = pWriteAcc->Width(), nWidth2 = nWidth + 2;
            const long      nHeight = pWriteAcc->Height(), nHeight2 = nHeight + 2;
            long*           pColm = new long[ nWidth2 ];
            long*           pRows = new long[ nHeight2 ];
            BitmapColor*    pColRow1 = (BitmapColor*) new sal_uInt8[ sizeof( BitmapColor ) * nWidth2 ];
            BitmapColor*    pColRow2 = (BitmapColor*) new sal_uInt8[ sizeof( BitmapColor ) * nWidth2 ];
            BitmapColor*    pColRow3 = (BitmapColor*) new sal_uInt8[ sizeof( BitmapColor ) * nWidth2 ];
            BitmapColor*    pRowTmp1 = pColRow1;
            BitmapColor*    pRowTmp2 = pColRow2;
            BitmapColor*    pRowTmp3 = pColRow3;
            BitmapColor*    pColor;
            long            nY, nX, i;
            long            nR1, nR2, nR3, nR4, nR5, nR6, nR7, nR8, nR9;
            long            nG1, nG2, nG3, nG4, nG5, nG6, nG7, nG8, nG9;
            long            nB1, nB2, nB3, nB4, nB5, nB6, nB7, nB8, nB9;

            // create column LUT
            for( i = 0; i < nWidth2; i++ )
                pColm[ i ] = ( i > 0 ) ? ( i - 1 ) : 0;

            pColm[ nWidth + 1 ] = pColm[ nWidth ];

            // create row LUT
            for( i = 0; i < nHeight2; i++ )
                pRows[ i ] = ( i > 0 ) ? ( i - 1 ) : 0;

            pRows[ nHeight + 1 ] = pRows[ nHeight ];

            // read first three rows of bitmap color
            if (nHeight2 > 2)
            {
                for( i = 0; i < nWidth2; i++ )
                {
                    pColRow1[ i ] = pReadAcc->GetColor( pRows[ 0 ], pColm[ i ] );
                    pColRow2[ i ] = pReadAcc->GetColor( pRows[ 1 ], pColm[ i ] );
                    pColRow3[ i ] = pReadAcc->GetColor( pRows[ 2 ], pColm[ i ] );
                }
            }

            // do median filtering
            for( nY = 0; nY < nHeight; )
            {
                for( nX = 0; nX < nWidth; nX++ )
                {
                    nR1 = ( pColor = pRowTmp1 + nX )->GetRed(), nG1 = pColor->GetGreen(), nB1 = pColor->GetBlue();
                    nR2 = ( ++pColor )->GetRed(), nG2 = pColor->GetGreen(), nB2 = pColor->GetBlue();
                    nR3 = ( ++pColor )->GetRed(), nG3 = pColor->GetGreen(), nB3 = pColor->GetBlue();

                    nR4 = ( pColor = pRowTmp2 + nX )->GetRed(), nG4 = pColor->GetGreen(), nB4 = pColor->GetBlue();
                    nR5 = ( ++pColor )->GetRed(), nG5 = pColor->GetGreen(), nB5 = pColor->GetBlue();
                    nR6 = ( ++pColor )->GetRed(), nG6 = pColor->GetGreen(), nB6 = pColor->GetBlue();

                    nR7 = ( pColor = pRowTmp3 + nX )->GetRed(), nG7 = pColor->GetGreen(), nB7 = pColor->GetBlue();
                    nR8 = ( ++pColor )->GetRed(), nG8 = pColor->GetGreen(), nB8 = pColor->GetBlue();
                    nR9 = ( ++pColor )->GetRed(), nG9 = pColor->GetGreen(), nB9 = pColor->GetBlue();

                    MNMX6( nR1, nR2, nR3, nR4, nR5, nR6 );
                    MNMX5( nR7, nR2, nR3, nR4, nR5 );
                    MNMX4( nR8, nR2, nR3, nR4 );
                    MNMX3( nR9, nR2, nR3 );

                    MNMX6( nG1, nG2, nG3, nG4, nG5, nG6 );
                    MNMX5( nG7, nG2, nG3, nG4, nG5 );
                    MNMX4( nG8, nG2, nG3, nG4 );
                    MNMX3( nG9, nG2, nG3 );

                    MNMX6( nB1, nB2, nB3, nB4, nB5, nB6 );
                    MNMX5( nB7, nB2, nB3, nB4, nB5 );
                    MNMX4( nB8, nB2, nB3, nB4 );
                    MNMX3( nB9, nB2, nB3 );

                    // set destination color
                    pWriteAcc->SetPixel( nY, nX, BitmapColor( (sal_uInt8) nR2, (sal_uInt8) nG2, (sal_uInt8) nB2 ) );
                }

                if( ++nY < nHeight )
                {
                    if( pRowTmp1 == pColRow1 )
                        pRowTmp1 = pColRow2, pRowTmp2 = pColRow3, pRowTmp3 = pColRow1;
                    else if( pRowTmp1 == pColRow2 )
                        pRowTmp1 = pColRow3, pRowTmp2 = pColRow1, pRowTmp3 = pColRow2;
                    else
                        pRowTmp1 = pColRow1, pRowTmp2 = pColRow2, pRowTmp3 = pColRow3;

                    for( i = 0; i < nWidth2; i++ )
                        pRowTmp3[ i ] = pReadAcc->GetColor( pRows[ nY + 2 ], pColm[ i ] );
                }
            }

            delete[] (sal_uInt8*) pColRow1;
            delete[] (sal_uInt8*) pColRow2;
            delete[] (sal_uInt8*) pColRow3;
            delete[] pColm;
            delete[] pRows;

            aNewBmp.ReleaseAccess( pWriteAcc );

            bRet = sal_True;
        }

        ReleaseAccess( pReadAcc );

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

sal_Bool Bitmap::ImplSobelGrey( const BmpFilterParam* /*pFilterParam*/, const Link* /*pProgress*/ )
{
    sal_Bool bRet = ImplMakeGreyscales( 256 );

    if( bRet )
    {
        bRet = sal_False;

        BitmapReadAccess* pReadAcc = AcquireReadAccess();

        if( pReadAcc )
        {
            Bitmap              aNewBmp( GetSizePixel(), 8, &pReadAcc->GetPalette() );
            BitmapWriteAccess*  pWriteAcc = aNewBmp.AcquireWriteAccess();

            if( pWriteAcc )
            {
                BitmapColor aGrey( (sal_uInt8) 0 );
                const long  nWidth = pWriteAcc->Width();
                const long  nHeight = pWriteAcc->Height();
                const long  nMask111 = -1, nMask121 =  0, nMask131 =  1;
                const long  nMask211 = -2, nMask221 =  0, nMask231 =  2;
                const long  nMask311 = -1, nMask321 =  0, nMask331 =  1;
                const long  nMask112 =  1, nMask122 =  2, nMask132 =  1;
                const long  nMask212 =  0, nMask222 =  0, nMask232 =  0;
                const long  nMask312 = -1, nMask322 = -2, nMask332 = -1;
                long        nGrey11, nGrey12, nGrey13;
                long        nGrey21, nGrey22, nGrey23;
                long        nGrey31, nGrey32, nGrey33;
                long*       pHMap = new long[ nWidth + 2 ];
                long*       pVMap = new long[ nHeight + 2 ];
                long        nX, nY, nSum1, nSum2;

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

                    for( nX = 0; nX < nWidth; nX++ )
                    {
                        nSum1 = nSum2 = 0;

                        nSum1 += nMask111 * nGrey11;
                        nSum2 += nMask112 * nGrey11;

                        nSum1 += nMask121 * nGrey12;
                        nSum2 += nMask122 * nGrey12;

                        nSum1 += nMask131 * nGrey13;
                        nSum2 += nMask132 * nGrey13;

                        nSum1 += nMask211 * nGrey21;
                        nSum2 += nMask212 * nGrey21;

                        nSum1 += nMask221 * nGrey22;
                        nSum2 += nMask222 * nGrey22;

                        nSum1 += nMask231 * nGrey23;
                        nSum2 += nMask232 * nGrey23;

                        nSum1 += nMask311 * nGrey31;
                        nSum2 += nMask312 * nGrey31;

                        nSum1 += nMask321 * nGrey32;
                        nSum2 += nMask322 * nGrey32;

                        nSum1 += nMask331 * nGrey33;
                        nSum2 += nMask332 * nGrey33;

                        nSum1 = (long) sqrt( (double)( nSum1 * nSum1 + nSum2 * nSum2 ) );
                        aGrey.SetIndex( ~(sal_uInt8) SAL_BOUND( nSum1, 0, 255 ) );
                        pWriteAcc->SetPixel( nY, nX, aGrey );

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
                aNewBmp.ReleaseAccess( pWriteAcc );
                bRet = sal_True;
            }

            ReleaseAccess( pReadAcc );

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

sal_Bool Bitmap::ImplEmbossGrey( const BmpFilterParam* pFilterParam, const Link* /*pProgress*/ )
{
    sal_Bool bRet = ImplMakeGreyscales( 256 );

    if( bRet )
    {
        bRet = sal_False;

        BitmapReadAccess* pReadAcc = AcquireReadAccess();

        if( pReadAcc )
        {
            Bitmap              aNewBmp( GetSizePixel(), 8, &pReadAcc->GetPalette() );
            BitmapWriteAccess*  pWriteAcc = aNewBmp.AcquireWriteAccess();

            if( pWriteAcc )
            {
                BitmapColor aGrey( (sal_uInt8) 0 );
                const long  nWidth = pWriteAcc->Width();
                const long  nHeight = pWriteAcc->Height();
                long        nGrey11, nGrey12, nGrey13;
                long        nGrey21, nGrey22, nGrey23;
                long        nGrey31, nGrey32, nGrey33;
                double      fAzim = ( ( pFilterParam && pFilterParam->meFilter == BMP_FILTER_EMBOSS_GREY ) ?
                                      ( pFilterParam->maEmbossAngles.mnAzimuthAngle100 * 0.01 ) : 0.0 ) * F_PI180;
                double      fElev = ( ( pFilterParam && pFilterParam->meFilter == BMP_FILTER_EMBOSS_GREY ) ?
                                      ( pFilterParam->maEmbossAngles.mnElevationAngle100 * 0.01 ) : 90.0 ) * F_PI180;
                long*       pHMap = new long[ nWidth + 2 ];
                long*       pVMap = new long[ nHeight + 2 ];
                long        nX, nY, nNx, nNy, nDotL;
                const long  nLx = FRound( cos( fAzim ) * cos( fElev ) * 255.0 );
                const long  nLy = FRound( sin( fAzim ) * cos( fElev ) * 255.0 );
                const long  nLz = FRound( sin( fElev ) * 255.0 );
                const long  nZ2 = ( ( 6 * 255 ) / 4 ) * ( ( 6 * 255 ) / 4 );
                const long  nNzLz = ( ( 6 * 255 ) / 4 ) * nLz;
                const sal_uInt8 cLz = (sal_uInt8) SAL_BOUND( nLz, 0, 255 );

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
                            const double fGrey = nDotL / sqrt( (double)(nNx * nNx + nNy * nNy + nZ2) );
                            aGrey.SetIndex( (sal_uInt8) SAL_BOUND( fGrey, 0, 255 ) );
                        }

                        pWriteAcc->SetPixel( nY, nX, aGrey );

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
                aNewBmp.ReleaseAccess( pWriteAcc );
                bRet = sal_True;
            }

            ReleaseAccess( pReadAcc );

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

sal_Bool Bitmap::ImplSolarize( const BmpFilterParam* pFilterParam, const Link* /*pProgress*/ )
{
    sal_Bool                bRet = sal_False;
    BitmapWriteAccess*  pWriteAcc = AcquireWriteAccess();

    if( pWriteAcc )
    {
        const sal_uInt8 cThreshold = ( pFilterParam && pFilterParam->meFilter == BMP_FILTER_SOLARIZE ) ?
                                pFilterParam->mcSolarGreyThreshold : 128;

        if( pWriteAcc->HasPalette() )
        {
            const BitmapPalette& rPal = pWriteAcc->GetPalette();

            for( sal_uInt16 i = 0, nCount = rPal.GetEntryCount(); i < nCount; i++ )
            {
                if( rPal[ i ].GetLuminance() >= cThreshold )
                {
                    BitmapColor aCol( rPal[ i ] );
                    pWriteAcc->SetPaletteColor( i, aCol.Invert() );
                }
            }
        }
        else
        {
            BitmapColor aCol;
            const long  nWidth = pWriteAcc->Width();
            const long  nHeight = pWriteAcc->Height();

            for( long nY = 0; nY < nHeight ; nY++ )
            {
                for( long nX = 0; nX < nWidth; nX++ )
                {
                    aCol = pWriteAcc->GetPixel( nY, nX );

                    if( aCol.GetLuminance() >= cThreshold )
                        pWriteAcc->SetPixel( nY, nX, aCol.Invert() );
                }
            }
        }

        ReleaseAccess( pWriteAcc );
        bRet = sal_True;
    }

    return bRet;
}

sal_Bool Bitmap::ImplSepia( const BmpFilterParam* pFilterParam, const Link* /*pProgress*/ )
{
    BitmapReadAccess*   pReadAcc = AcquireReadAccess();
    sal_Bool                bRet = sal_False;

    if( pReadAcc )
    {
        long            nSepiaPercent = ( pFilterParam && pFilterParam->meFilter == BMP_FILTER_SEPIA ) ?
                                        pFilterParam->mcSolarGreyThreshold : 10;
        const long      nSepia = 10000 - 100 * SAL_BOUND( nSepiaPercent, 0, 100 );
        BitmapPalette   aSepiaPal( 256 );

        DBG_ASSERT( nSepiaPercent <= 100, "Bitmap::ImplSepia(): sepia value out of range; defaulting to 100%" );

        for( sal_uInt16 i = 0; i < 256; i++ )
        {
            BitmapColor&    rCol = aSepiaPal[ i ];
            const sal_uInt8     cSepiaValue = (sal_uInt8) ( ( nSepia * i ) / 10000 );

            rCol.SetRed( (sal_uInt8) i );
            rCol.SetGreen( cSepiaValue );
            rCol.SetBlue( cSepiaValue );
        }

        Bitmap              aNewBmp( GetSizePixel(), 8, &aSepiaPal );
        BitmapWriteAccess*  pWriteAcc = aNewBmp.AcquireWriteAccess();

        if( pWriteAcc )
        {
            BitmapColor aCol( (sal_uInt8) 0 );
            const long  nWidth = pWriteAcc->Width();
            const long  nHeight = pWriteAcc->Height();

            if( pReadAcc->HasPalette() )
            {
                for( long nY = 0; nY < nHeight ; nY++ )
                {
                    const sal_uInt16    nPalCount = pReadAcc->GetPaletteEntryCount();
                    sal_uInt8*          pIndexMap = new sal_uInt8[ nPalCount ];

                    for( sal_uInt16 i = 0; i < nPalCount; i++ )
                        pIndexMap[ i ] = pReadAcc->GetPaletteColor( i ).GetLuminance();

                    for( long nX = 0; nX < nWidth; nX++ )
                    {
                        aCol.SetIndex( pIndexMap[ pReadAcc->GetPixel( nY, nX ).GetIndex() ] );
                        pWriteAcc->SetPixel( nY, nX, aCol );
                    }

                    delete[] pIndexMap;
                }
            }
            else
            {
                for( long nY = 0; nY < nHeight ; nY++ )
                {
                    for( long nX = 0; nX < nWidth; nX++ )
                    {
                        aCol.SetIndex( pReadAcc->GetPixel( nY, nX ).GetLuminance() );
                        pWriteAcc->SetPixel( nY, nX, aCol );
                    }
                }
            }

            aNewBmp.ReleaseAccess( pWriteAcc );
            bRet = sal_True;
        }

        ReleaseAccess( pReadAcc );

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

sal_Bool Bitmap::ImplMosaic( const BmpFilterParam* pFilterParam, const Link* /*pProgress*/ )
{
    sal_uLong               nTileWidth = ( pFilterParam && pFilterParam->meFilter == BMP_FILTER_MOSAIC ) ?
                                     pFilterParam->maMosaicTileSize.mnTileWidth : 4;
    sal_uLong               nTileHeight = ( pFilterParam && pFilterParam->meFilter == BMP_FILTER_MOSAIC ) ?
                                      pFilterParam->maMosaicTileSize.mnTileHeight : 4;
    sal_Bool                bRet = sal_False;

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
            pNewBmp = NULL;
            pReadAcc = pWriteAcc = AcquireWriteAccess();
        }
        else
        {
            pNewBmp = new Bitmap( GetSizePixel(), 24 );
            pReadAcc = AcquireReadAccess();
            pWriteAcc = pNewBmp->AcquireWriteAccess();
        }

        if( pReadAcc && pWriteAcc )
        {
            BitmapColor aCol;
            long        nWidth = pReadAcc->Width();
            long        nHeight = pReadAcc->Height();
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
                            for( nX = nX1; nX <= nX2; nX++ )
                            {
                                aCol = pReadAcc->GetPixel( nY, nX );
                                nSumR += aCol.GetRed();
                                nSumG += aCol.GetGreen();
                                nSumB += aCol.GetBlue();
                            }
                        }

                        aCol.SetRed( (sal_uInt8) ( nSumR * fArea_1 ) );
                        aCol.SetGreen( (sal_uInt8) ( nSumG * fArea_1 ) );
                        aCol.SetBlue( (sal_uInt8) ( nSumB * fArea_1 ) );

                        for( nY = nY1; nY <= nY2; nY++ )
                            for( nX = nX1; nX <= nX2; nX++ )
                                pWriteAcc->SetPixel( nY, nX, aCol );

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
                            for( nX = nX1; nX <= nX2; nX++ )
                            {
                                const BitmapColor& rCol = pReadAcc->GetPaletteColor( pReadAcc->GetPixelIndex( nY, nX ) );
                                nSumR += rCol.GetRed();
                                nSumG += rCol.GetGreen();
                                nSumB += rCol.GetBlue();
                            }
                        }

                        aCol.SetRed( (sal_uInt8) ( nSumR * fArea_1 ) );
                        aCol.SetGreen( (sal_uInt8) ( nSumG * fArea_1 ) );
                        aCol.SetBlue( (sal_uInt8) ( nSumB * fArea_1 ) );

                        for( nY = nY1; nY <= nY2; nY++ )
                            for( nX = nX1; nX <= nX2; nX++ )
                                pWriteAcc->SetPixel( nY, nX, aCol );

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

            bRet = sal_True;
        }

        ReleaseAccess( pReadAcc );

        if( pNewBmp )
        {
            pNewBmp->ReleaseAccess( pWriteAcc );

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
        bRet = sal_True;

    return bRet;
}

struct PopArtEntry
{
    sal_uInt32  mnIndex;
    sal_uInt32  mnCount;
};

extern "C" int SAL_CALL ImplPopArtCmpFnc( const void* p1, const void* p2 )
{
    int nRet;

    if( ( (PopArtEntry*) p1 )->mnCount < ( (PopArtEntry*) p2 )->mnCount )
        nRet = 1;
    else if( ( (PopArtEntry*) p1 )->mnCount == ( (PopArtEntry*) p2 )->mnCount )
        nRet = 0;
    else
        nRet = -1;

    return nRet;
}

sal_Bool Bitmap::ImplPopArt( const BmpFilterParam* /*pFilterParam*/, const Link* /*pProgress*/ )
{
    sal_Bool bRet = ( GetBitCount() > 8 ) ? Convert( BMP_CONVERSION_8BIT_COLORS ) : sal_True;

    if( bRet )
    {
        bRet = sal_False;

        BitmapWriteAccess* pWriteAcc = AcquireWriteAccess();

        if( pWriteAcc )
        {
            const long      nWidth = pWriteAcc->Width();
            const long      nHeight = pWriteAcc->Height();
            const sal_uLong     nEntryCount = 1 << pWriteAcc->GetBitCount();
            sal_uLong           n;
            PopArtEntry*    pPopArtTable = new PopArtEntry[ nEntryCount ];

            for( n = 0; n < nEntryCount; n++ )
            {
                PopArtEntry& rEntry = pPopArtTable[ n ];
                rEntry.mnIndex = (sal_uInt16) n;
                rEntry.mnCount = 0;
            }

            // get pixel count for each palette entry
            for( long nY = 0; nY < nHeight ; nY++ )
                for( long nX = 0; nX < nWidth; nX++ )
                    pPopArtTable[ pWriteAcc->GetPixel( nY, nX ).GetIndex() ].mnCount++;

            // sort table
            qsort( pPopArtTable, nEntryCount, sizeof( PopArtEntry ), ImplPopArtCmpFnc );

            // get last used entry
            sal_uLong nFirstEntry;
            sal_uLong nLastEntry = 0;

            for( n = 0; n < nEntryCount; n++ )
                if( pPopArtTable[ n ].mnCount )
                    nLastEntry = n;

            // rotate palette (one entry)
            const BitmapColor aFirstCol( pWriteAcc->GetPaletteColor( sal::static_int_cast<sal_uInt16>(pPopArtTable[ 0 ].mnIndex) ) );
            for( nFirstEntry = 0; nFirstEntry < nLastEntry; nFirstEntry++ )
            {
                pWriteAcc->SetPaletteColor( sal::static_int_cast<sal_uInt16>(pPopArtTable[ nFirstEntry ].mnIndex),
                                            pWriteAcc->GetPaletteColor( sal::static_int_cast<sal_uInt16>(pPopArtTable[ nFirstEntry + 1 ].mnIndex) ) );
            }
            pWriteAcc->SetPaletteColor( sal::static_int_cast<sal_uInt16>(pPopArtTable[ nLastEntry ].mnIndex), aFirstCol );

            // cleanup
            delete[] pPopArtTable;
            ReleaseAccess( pWriteAcc );
            bRet = sal_True;
        }
    }

    return bRet;
}

double* MakeBlurKernel(const double radius, int& rows) {
    int intRadius = (int) radius + 1.0;
    rows = intRadius * 2 + 1;
    double* matrix = new double[rows];

    double sigma = radius / 3;
    double radius2 = radius * radius;
    int index = 0;
    for (int row = -intRadius; row <= intRadius; row++)
    {
        double distance = row*row;
        if (distance > radius2) {
            matrix[index] = 0.0;
        }else {
            matrix[index] = exp( -distance / (2.0 * sigma * sigma) ) / sqrt( 2.0 * M_PI * sigma );
        }
        index++;
    }
    return matrix;
}

void Bitmap::ImplBlurContributions( const int aSize, const int aNumberOfContributions,
                                    double* pBlurVector, double*& pWeights, int*& pPixels, int*& pCount )
{
    pWeights = new double[ aSize*aNumberOfContributions ];
    pPixels = new int[ aSize*aNumberOfContributions ];
    pCount = new int[ aSize ];

    int aLeft, aRight, aCurrentCount, aPixelIndex;
    double aWeight;

    for ( int i = 0; i < aSize; i++ )
    {
        aLeft = (int)  i - aNumberOfContributions / 2;
        aRight = (int) i + aNumberOfContributions / 2;
        aCurrentCount = 0;
        for ( int j = aLeft; j <= aRight; j++ )
        {
            aWeight = pBlurVector[aCurrentCount];

            // Mirror edges
            if (j < 0)
            {
                aPixelIndex = -j;
            }
            else if ( j >= aSize )
            {
                aPixelIndex = (aSize - j) + aSize - 1;
            }
            else
            {
                aPixelIndex = j;
            }

            // Edge case for small bitmaps
            if ( aPixelIndex < 0 || aPixelIndex >= aSize )
            {
                aWeight = 0.0;
            }

            pWeights[ i*aNumberOfContributions + aCurrentCount ] = aWeight;
            pPixels[ i*aNumberOfContributions + aCurrentCount ] = aPixelIndex;

            aCurrentCount++;
        }
        pCount[ i ] = aCurrentCount;
    }
}

// Separable Gaussian Blur
//
// Separable Gaussian Blur filter and accepts a blur radius
// as a parameter so the user can change the strength of the blur.
// Radius of 1.0 is 3 * standard deviation of gauss function.
//
// Separable Blur implementation uses 2x separable 1D convolution
// to process the image.
bool Bitmap::ImplSeparableBlurFilter(const double radius)
{
    const long  nWidth = GetSizePixel().Width();
    const long  nHeight = GetSizePixel().Height();

    // Prepare Blur Vector
    int aNumberOfContributions;
    double* pBlurVector = MakeBlurKernel(radius, aNumberOfContributions);

    double* pWeights;
    int* pPixels;
    int* pCount;

    // Do horizontal filtering
    ImplBlurContributions( nWidth, aNumberOfContributions, pBlurVector, pWeights, pPixels, pCount);

    BitmapReadAccess* pReadAcc = AcquireReadAccess();

    // switch coordinates as convolution pass transposes result
    Bitmap aNewBitmap( Size( nHeight, nWidth ), 24 );

    bool bResult = ImplConvolutionPass( aNewBitmap, nWidth, pReadAcc, aNumberOfContributions, pWeights, pPixels, pCount );

    // Cleanup
    ReleaseAccess( pReadAcc );
    delete[] pWeights;
    delete[] pPixels;
    delete[] pCount;

    if ( !bResult )
    {
        delete[] pBlurVector;
        return bResult;
    }

    // Swap current bitmap with new bitmap
    ImplAssignWithSize( aNewBitmap );

    // Do vertical filtering
    ImplBlurContributions(nHeight, aNumberOfContributions, pBlurVector, pWeights, pPixels, pCount );

    pReadAcc = AcquireReadAccess();
    aNewBitmap = Bitmap( Size( nWidth, nHeight ), 24 );
    bResult = ImplConvolutionPass( aNewBitmap, nHeight, pReadAcc, aNumberOfContributions, pWeights, pPixels, pCount );

    // Cleanup
    ReleaseAccess( pReadAcc );
    delete[] pWeights;
    delete[] pCount;
    delete[] pPixels;
    delete[] pBlurVector;

    if ( !bResult )
        return bResult;

    // Swap current bitmap with new bitmap
    ImplAssignWithSize( aNewBitmap );

    return true;
}

// Separable Unsharepn Mask filter is actually a substracted blured
// image from the original image.
bool Bitmap::ImplSeparableUnsharpenFilter(const double radius) {
    const long  nWidth = GetSizePixel().Width();
    const long  nHeight = GetSizePixel().Height();

    Bitmap aBlur( *this );
    aBlur.ImplSeparableBlurFilter(-radius);

    // Amount of unsharpening effect on image - currently set to a fixed value
    double aAmount = 2.0;

    Bitmap aResultBitmap( Size( nWidth, nHeight ), 24);

    BitmapReadAccess* pReadAccBlur = aBlur.AcquireReadAccess();
    BitmapReadAccess* pReadAcc = AcquireReadAccess();
    BitmapWriteAccess* pWriteAcc = aResultBitmap.AcquireWriteAccess();

    BitmapColor aColor, aColorBlur;

    // For all pixels in original image substract pixels values from blured image.
    for( int x = 0; x < nWidth; x++ )
    {
        for( int y = 0; y < nHeight; y++ )
        {
            aColorBlur = pReadAccBlur->GetColor( y , x );
            aColor = pReadAcc->GetColor( y , x );

            BitmapColor aResultColor(
                (sal_uInt8) MinMax( aColor.GetRed()   + (aColor.GetRed()   - aColorBlur.GetRed())   * aAmount, 0, 255 ),
                (sal_uInt8) MinMax( aColor.GetGreen() + (aColor.GetGreen() - aColorBlur.GetGreen()) * aAmount, 0, 255 ),
                (sal_uInt8) MinMax( aColor.GetBlue()  + (aColor.GetBlue()  - aColorBlur.GetBlue())  * aAmount, 0, 255 ) );

            pWriteAcc->SetPixel( y, x, aResultColor );
        }
    }

    ReleaseAccess( pWriteAcc );
    ReleaseAccess( pReadAcc );
    ReleaseAccess( pReadAccBlur );
    ImplAssignWithSize ( aResultBitmap );
    return true;
}


bool Bitmap::ImplDuotoneFilter( const sal_uLong nColorOne, const sal_uLong nColorTwo )
{
    const long  nWidth = GetSizePixel().Width();
    const long  nHeight = GetSizePixel().Height();

    Bitmap aResultBitmap( GetSizePixel(), 24);
    BitmapReadAccess* pReadAcc = AcquireReadAccess();
    BitmapWriteAccess* pWriteAcc = aResultBitmap.AcquireWriteAccess();
    const BitmapColor aColorOne( static_cast< sal_uInt8 >( nColorOne >> 16 ), static_cast< sal_uInt8 >( nColorOne >> 8 ), static_cast< sal_uInt8 >( nColorOne ) );
    const BitmapColor aColorTwo( static_cast< sal_uInt8 >( nColorTwo >> 16 ), static_cast< sal_uInt8 >( nColorTwo >> 8 ), static_cast< sal_uInt8 >( nColorTwo ) );

    for( int x = 0; x < nWidth; x++ )
    {
        for( int y = 0; y < nHeight; y++ )
        {
            BitmapColor aColor = pReadAcc->GetColor( y, x );
            BitmapColor aResultColor(
                    lcl_getDuotoneColorComponent( aColor.GetRed(), aColorOne.GetRed(), aColorTwo.GetRed() ) ,
                    lcl_getDuotoneColorComponent( aColor.GetGreen(), aColorOne.GetGreen(), aColorTwo.GetGreen() ) ,
                    lcl_getDuotoneColorComponent( aColor.GetBlue(), aColorOne.GetBlue(), aColorTwo.GetBlue() ) );
            pWriteAcc->SetPixel( y, x, aResultColor );
        }
    }

    ReleaseAccess( pWriteAcc );
    ReleaseAccess( pReadAcc );
    ImplAssignWithSize ( aResultBitmap );
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
