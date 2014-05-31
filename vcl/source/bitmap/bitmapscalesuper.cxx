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

#include <vcl/bmpacc.hxx>
#include <vcl/bitmapscalesuper.hxx>

#include <boost/scoped_array.hpp>

#define MAP( cVal0, cVal1, nFrac )  ((sal_uInt8)((((long)(cVal0)<<7L)+nFrac*((long)(cVal1)-(cVal0)))>>7L))

void generateMap(long nW, long nDstW, bool bHMirr, long* pMapIX, long* pMapFX)
{
    const double fRevScaleX = (nDstW > 1L) ? (double) (nW - 1) / (nDstW - 1) : 0.0;

    long nTemp = nW - 2L;
    long nTempX = nW - 1L;
    for (long nX = 0L; nX < nDstW; nX++)
    {
        double fTemp = nX * fRevScaleX;
        if (bHMirr)
            fTemp = nTempX - fTemp;
        pMapIX[nX] = MinMax((long) fTemp, 0, nTemp);
        pMapFX[nX] = (long) (fTemp - pMapIX[nX]) * 128.0;
    }
}

void scalePallete8bit(BitmapReadAccess* pAcc, BitmapWriteAccess* pWAcc,
                      long nStartX, long nEndX, long nStartY, long nEndY,
                      bool bVMirr, bool bHMirr)
{
    boost::scoped_array<long> pMapIX(new long[pWAcc->Width()]);
    boost::scoped_array<long> pMapIY(new long[pWAcc->Height()]);
    boost::scoped_array<long> pMapFX(new long[pWAcc->Width()]);
    boost::scoped_array<long> pMapFY(new long[pWAcc->Height()]);

    generateMap(pAcc->Width(),  pWAcc->Width(),  bHMirr, pMapIX.get(), pMapFX.get());
    generateMap(pAcc->Height(), pWAcc->Height(), bVMirr, pMapIY.get(), pMapFY.get());

    for( long nY = nStartY, nYDst = 0L; nY <= nEndY; nY++, nYDst++ )
    {
        long nTempY = pMapIY[ nY ];
        long nTempFY = pMapFY[ nY ];
        Scanline pLine0 = pAcc->GetScanline( nTempY );
        Scanline pLine1 = pAcc->GetScanline( ++nTempY );

        for(long nX = nStartX, nXDst = 0L; nX <= nEndX; nX++ )
        {
            long nTempX = pMapIX[ nX ];
            long nTempFX = pMapFX[ nX ];

            const BitmapColor& rCol0 = pAcc->GetPaletteColor( pLine0[ nTempX ] );
            const BitmapColor& rCol2 = pAcc->GetPaletteColor( pLine1[ nTempX ] );
            const BitmapColor& rCol1 = pAcc->GetPaletteColor( pLine0[ ++nTempX ] );
            const BitmapColor& rCol3 = pAcc->GetPaletteColor( pLine1[ nTempX ] );

            sal_uInt8 cR0 = MAP( rCol0.GetRed(), rCol1.GetRed(), nTempFX );
            sal_uInt8 cG0 = MAP( rCol0.GetGreen(), rCol1.GetGreen(), nTempFX );
            sal_uInt8 cB0 = MAP( rCol0.GetBlue(), rCol1.GetBlue(), nTempFX );

            sal_uInt8 cR1 = MAP( rCol2.GetRed(), rCol3.GetRed(), nTempFX );
            sal_uInt8 cG1 = MAP( rCol2.GetGreen(), rCol3.GetGreen(), nTempFX );
            sal_uInt8 cB1 = MAP( rCol2.GetBlue(), rCol3.GetBlue(), nTempFX );

            BitmapColor aColRes( MAP( cR0, cR1, nTempFY ),
                    MAP( cG0, cG1, nTempFY ),
                    MAP( cB0, cB1, nTempFY ) );
            pWAcc->SetPixel( nYDst, nXDst++, aColRes );
        }
    }
}

void scalePalleteGeneral(BitmapReadAccess* pAcc, BitmapWriteAccess* pWAcc,
                         long nStartX, long nEndX, long nStartY, long nEndY,
                         bool bVMirr, bool bHMirr)

{
    boost::scoped_array<long> pMapIX(new long[pWAcc->Width()]);
    boost::scoped_array<long> pMapIY(new long[pWAcc->Height()]);
    boost::scoped_array<long> pMapFX(new long[pWAcc->Width()]);
    boost::scoped_array<long> pMapFY(new long[pWAcc->Height()]);

    generateMap(pAcc->Width(),  pWAcc->Width(),  bHMirr, pMapIX.get(), pMapFX.get());
    generateMap(pAcc->Height(), pWAcc->Height(), bVMirr, pMapIY.get(), pMapFY.get());

    for( long nY = nStartY, nYDst = 0L; nY <= nEndY; nY++, nYDst++ )
    {
        long nTempY = pMapIY[ nY ];
        long nTempFY = pMapFY[ nY ];

        for( long nX = nStartX, nXDst = 0L; nX <= nEndX; nX++ )
        {
            long nTempX = pMapIX[ nX ];
            long nTempFX = pMapFX[ nX ];

            BitmapColor aCol0 = pAcc->GetPaletteColor( pAcc->GetPixelIndex( nTempY, nTempX ) );
            BitmapColor aCol1 = pAcc->GetPaletteColor( pAcc->GetPixelIndex( nTempY, ++nTempX ) );
            sal_uInt8 cR0 = MAP( aCol0.GetRed(), aCol1.GetRed(), nTempFX );
            sal_uInt8 cG0 = MAP( aCol0.GetGreen(), aCol1.GetGreen(), nTempFX );
            sal_uInt8 cB0 = MAP( aCol0.GetBlue(), aCol1.GetBlue(), nTempFX );

            aCol1 = pAcc->GetPaletteColor( pAcc->GetPixelIndex( ++nTempY, nTempX ) );
            aCol0 = pAcc->GetPaletteColor( pAcc->GetPixelIndex( nTempY--, --nTempX ) );
            sal_uInt8 cR1 = MAP( aCol0.GetRed(), aCol1.GetRed(), nTempFX );
            sal_uInt8 cG1 = MAP( aCol0.GetGreen(), aCol1.GetGreen(), nTempFX );
            sal_uInt8 cB1 = MAP( aCol0.GetBlue(), aCol1.GetBlue(), nTempFX );

            BitmapColor aColRes( MAP( cR0, cR1, nTempFY ),
                    MAP( cG0, cG1, nTempFY ),
                    MAP( cB0, cB1, nTempFY ) );
            pWAcc->SetPixel( nYDst, nXDst++, aColRes );
        }
    }
}

void scale24bitBGR(BitmapReadAccess* pAcc, BitmapWriteAccess* pWAcc,
                         long nStartX, long nEndX, long nStartY, long nEndY,
                         bool bVMirr, bool bHMirr)
{
    boost::scoped_array<long> pMapIX(new long[pWAcc->Width()]);
    boost::scoped_array<long> pMapIY(new long[pWAcc->Height()]);
    boost::scoped_array<long> pMapFX(new long[pWAcc->Width()]);
    boost::scoped_array<long> pMapFY(new long[pWAcc->Height()]);

    generateMap(pAcc->Width(),  pWAcc->Width(),  bHMirr, pMapIX.get(), pMapFX.get());
    generateMap(pAcc->Height(), pWAcc->Height(), bVMirr, pMapIY.get(), pMapFY.get());

    for( long nY = nStartY, nYDst = 0L; nY <= nEndY; nY++, nYDst++ )
    {
        long nTempY = pMapIY[ nY ];
        long nTempFY = pMapFY[ nY ];
        Scanline pLine0 = pAcc->GetScanline( nTempY );
        Scanline pLine1 = pAcc->GetScanline( ++nTempY );

        for( long nX = nStartX, nXDst = 0L; nX <= nEndX; nX++ )
        {
            long nOff = 3L * pMapIX[ nX ];
            long nTempFX = pMapFX[ nX ];

            Scanline pTmp0 = pLine0 + nOff ;
            Scanline pTmp1 = pTmp0 + 3L;
            sal_uInt8 cB0 = MAP( *pTmp0, *pTmp1, nTempFX );
            pTmp0++; pTmp1++;
            sal_uInt8 cG0 = MAP( *pTmp0, *pTmp1, nTempFX );
            pTmp0++; pTmp1++;
            sal_uInt8 cR0 = MAP( *pTmp0, *pTmp1, nTempFX );

            pTmp1 = ( pTmp0 = pLine1 + nOff ) + 3L;
            sal_uInt8 cB1 = MAP( *pTmp0, *pTmp1, nTempFX );
            pTmp0++; pTmp1++;
            sal_uInt8 cG1 = MAP( *pTmp0, *pTmp1, nTempFX );
            pTmp0++; pTmp1++;
            sal_uInt8 cR1 = MAP( *pTmp0, *pTmp1, nTempFX );

            BitmapColor aColRes( MAP( cR0, cR1, nTempFY ),
                    MAP( cG0, cG1, nTempFY ),
                    MAP( cB0, cB1, nTempFY ) );
            pWAcc->SetPixel( nYDst, nXDst++, aColRes );
        }
    }
}

void scale24bitRGB(BitmapReadAccess* pAcc, BitmapWriteAccess* pWAcc,
                         long nStartX, long nEndX, long nStartY, long nEndY,
                         bool bVMirr, bool bHMirr)
{
    boost::scoped_array<long> pMapIX(new long[pWAcc->Width()]);
    boost::scoped_array<long> pMapIY(new long[pWAcc->Height()]);
    boost::scoped_array<long> pMapFX(new long[pWAcc->Width()]);
    boost::scoped_array<long> pMapFY(new long[pWAcc->Height()]);

    generateMap(pAcc->Width(),  pWAcc->Width(),  bHMirr, pMapIX.get(), pMapFX.get());
    generateMap(pAcc->Height(), pWAcc->Height(), bVMirr, pMapIY.get(), pMapFY.get());

    for( long nY = nStartY, nYDst = 0L; nY <= nEndY; nY++, nYDst++ )
    {
        long nTempY = pMapIY[ nY ];
        long nTempFY = pMapFY[ nY ];
        Scanline pLine0 = pAcc->GetScanline( nTempY );
        Scanline pLine1 = pAcc->GetScanline( ++nTempY );

        for( long nX = nStartX, nXDst = 0L; nX <= nEndX; nX++ )
        {
            long nOff = 3L * pMapIX[ nX ];
            long nTempFX = pMapFX[ nX ];

            Scanline pTmp0 = pLine0 + nOff;
            Scanline pTmp1 = pTmp0 + 3L;
            sal_uInt8 cR0 = MAP( *pTmp0, *pTmp1, nTempFX );
            pTmp0++; pTmp1++;
            sal_uInt8 cG0 = MAP( *pTmp0, *pTmp1, nTempFX );
            pTmp0++; pTmp1++;
            sal_uInt8 cB0 = MAP( *pTmp0, *pTmp1, nTempFX );

            pTmp1 = ( pTmp0 = pLine1 + nOff ) + 3L;
            sal_uInt8 cR1 = MAP( *pTmp0, *pTmp1, nTempFX );
            pTmp0++; pTmp1++;
            sal_uInt8 cG1 = MAP( *pTmp0, *pTmp1, nTempFX );
            pTmp0++; pTmp1++;
            sal_uInt8 cB1 = MAP( *pTmp0, *pTmp1, nTempFX );

            BitmapColor aColRes( MAP( cR0, cR1, nTempFY ),
                    MAP( cG0, cG1, nTempFY ),
                    MAP( cB0, cB1, nTempFY ) );
            pWAcc->SetPixel( nYDst, nXDst++, aColRes );
        }
    }
}

void scaleNonPalleteGeneral(BitmapReadAccess* pAcc, BitmapWriteAccess* pWAcc,
                            long nStartX, long nEndX, long nStartY, long nEndY,
                            bool bVMirr, bool bHMirr)
{
    boost::scoped_array<long> pMapIX(new long[pWAcc->Width()]);
    boost::scoped_array<long> pMapIY(new long[pWAcc->Height()]);
    boost::scoped_array<long> pMapFX(new long[pWAcc->Width()]);
    boost::scoped_array<long> pMapFY(new long[pWAcc->Height()]);

    generateMap(pAcc->Width(),  pWAcc->Width(),  bHMirr, pMapIX.get(), pMapFX.get());
    generateMap(pAcc->Height(), pWAcc->Height(), bVMirr, pMapIY.get(), pMapFY.get());

    for( long nY = nStartY, nYDst = 0L; nY <= nEndY; nY++, nYDst++ )
    {
        long nTempY = pMapIY[ nY ];
        long nTempFY = pMapFY[ nY ];

        for( long nX = nStartX, nXDst = 0L; nX <= nEndX; nX++ )
        {
            long nTempX = pMapIX[ nX ];
            long nTempFX = pMapFX[ nX ];

            BitmapColor aCol0 = pAcc->GetPixel( nTempY, nTempX );
            BitmapColor aCol1 = pAcc->GetPixel( nTempY, ++nTempX );
            sal_uInt8 cR0 = MAP( aCol0.GetRed(), aCol1.GetRed(), nTempFX );
            sal_uInt8 cG0 = MAP( aCol0.GetGreen(), aCol1.GetGreen(), nTempFX );
            sal_uInt8 cB0 = MAP( aCol0.GetBlue(), aCol1.GetBlue(), nTempFX );

            aCol1 = pAcc->GetPixel( ++nTempY, nTempX );
            aCol0 = pAcc->GetPixel( nTempY--, --nTempX );
            sal_uInt8 cR1 = MAP( aCol0.GetRed(), aCol1.GetRed(), nTempFX );
            sal_uInt8 cG1 = MAP( aCol0.GetGreen(), aCol1.GetGreen(), nTempFX );
            sal_uInt8 cB1 = MAP( aCol0.GetBlue(), aCol1.GetBlue(), nTempFX );

            BitmapColor aColRes( MAP( cR0, cR1, nTempFY ),
                    MAP( cG0, cG1, nTempFY ),
                    MAP( cB0, cB1, nTempFY ) );
            pWAcc->SetPixel( nYDst, nXDst++, aColRes );
        }
    }
}

void scalePallete8bit2(BitmapReadAccess* pAcc, BitmapWriteAccess* pWAcc,
               long nStartX, long nEndX, long nStartY, long nEndY,
               bool bVMirr, bool bHMirr)
{
    boost::scoped_array<long> pMapIX(new long[pWAcc->Width()]);
    boost::scoped_array<long> pMapIY(new long[pWAcc->Height()]);
    boost::scoped_array<long> pMapFX(new long[pWAcc->Width()]);
    boost::scoped_array<long> pMapFY(new long[pWAcc->Height()]);

    generateMap(pAcc->Width(),  pWAcc->Width(),  bHMirr, pMapIX.get(), pMapFX.get());
    generateMap(pAcc->Height(), pWAcc->Height(), bVMirr, pMapIY.get(), pMapFY.get());

    const long nMax = 1 << 7L;

    for( long nY = nStartY , nYDst = 0L; nY <= nEndY; nY++, nYDst++ )
    {
        long nTop = bVMirr ? ( nY + 1 ) : nY;
        long nBottom = bVMirr ? nY : ( nY + 1 ) ;

        long nLineStart, nLineRange;
        if( nY == nEndY )
        {
            nLineStart = pMapIY[ nY ];
            nLineRange = 0;
        }
        else
        {
            nLineStart = pMapIY[ nTop ] ;
            nLineRange = ( pMapIY[ nBottom ] == pMapIY[ nTop ] ) ? 1 :( pMapIY[ nBottom ] - pMapIY[ nTop ] );
        }

        for( long nX = nStartX , nXDst = 0L; nX <= nEndX; nX++ )
        {
            long nLeft = bHMirr ? ( nX + 1 ) : nX;
            long nRight = bHMirr ? nX : ( nX + 1 ) ;

            long nRowStart;
            long nRowRange;
            if( nX == nEndX )
            {
                nRowStart = pMapIX[ nX ];
                nRowRange = 0;
            }
            else
            {
                nRowStart = pMapIX[ nLeft ];
                nRowRange = ( pMapIX[ nRight ] == pMapIX[ nLeft ] )? 1 : ( pMapIX[ nRight ] - pMapIX[ nLeft ] );
            }

            long nSumR = 0;
            long nSumG = 0;
            long nSumB = 0;
            long nTotalWeightY = 0;

            for(int i = 0; i<= nLineRange; i++)
            {
                Scanline pTmpY = pAcc->GetScanline( nLineStart + i );
                long nSumRowR = 0;
                long nSumRowG = 0;
                long nSumRowB = 0;
                long nTotalWeightX = 0;

                for(int j = 0; j <= nRowRange; j++)
                {
                    const BitmapColor& rCol = pAcc->GetPaletteColor( pTmpY[ nRowStart + j ] );

                    if(nX == nEndX )
                    {
                        nSumRowB += rCol.GetBlue() << 7L;
                        nSumRowG += rCol.GetGreen() << 7L;
                        nSumRowR += rCol.GetRed() << 7L;
                        nTotalWeightX += 1 << 7L;
                    }
                    else if( j == 0 )
                    {
                        long nWeightX = (nMax- pMapFX[ nLeft ]) ;
                        nSumRowB += ( nWeightX *rCol.GetBlue()) ;
                        nSumRowG += ( nWeightX *rCol.GetGreen()) ;
                        nSumRowR += ( nWeightX *rCol.GetRed()) ;
                        nTotalWeightX += nWeightX;
                    }
                    else if ( nRowRange == j )
                    {
                        long nWeightX = pMapFX[ nRight ] ;
                        nSumRowB += ( nWeightX *rCol.GetBlue() );
                        nSumRowG += ( nWeightX *rCol.GetGreen() );
                        nSumRowR += ( nWeightX *rCol.GetRed() );
                        nTotalWeightX += nWeightX;
                    }
                    else
                    {
                        nSumRowB += rCol.GetBlue() << 7L;
                        nSumRowG += rCol.GetGreen() << 7L;
                        nSumRowR += rCol.GetRed() << 7L;
                        nTotalWeightX += 1 << 7L;
                    }
                }

                long nWeightY = nMax;
                if( nY == nEndY )
                    nWeightY = nMax;
                else if( i == 0 )
                    nWeightY = nMax - pMapFY[ nTop ];
                else if( nLineRange == 1 )
                    nWeightY = pMapFY[ nTop ];
                else if ( nLineRange == i )
                    nWeightY = pMapFY[ nBottom ];

                nSumB += nWeightY * ( nSumRowB / nTotalWeightX );
                nSumG += nWeightY * ( nSumRowG / nTotalWeightX );
                nSumR += nWeightY * ( nSumRowR / nTotalWeightX );
                nTotalWeightY += nWeightY;
            }

            BitmapColor aColRes ( ( sal_uInt8 ) (( nSumR / nTotalWeightY ) ),
                    ( sal_uInt8 ) (( nSumG / nTotalWeightY) ),
                    ( sal_uInt8 ) (( nSumB / nTotalWeightY) ) );
            pWAcc->SetPixel( nYDst, nXDst++, aColRes );
        }
    }
}

void scalePalleteGeneral2(BitmapReadAccess* pAcc, BitmapWriteAccess* pWAcc,
                          long nStartX, long nEndX, long nStartY, long nEndY,
                          bool bVMirr, bool bHMirr)
{
    boost::scoped_array<long> pMapIX(new long[pWAcc->Width()]);
    boost::scoped_array<long> pMapIY(new long[pWAcc->Height()]);
    boost::scoped_array<long> pMapFX(new long[pWAcc->Width()]);
    boost::scoped_array<long> pMapFY(new long[pWAcc->Height()]);

    generateMap(pAcc->Width(),  pWAcc->Width(),  bHMirr, pMapIX.get(), pMapFX.get());
    generateMap(pAcc->Height(), pWAcc->Height(), bVMirr, pMapIY.get(), pMapFY.get());

    const long nMax = 1 << 7L;

    for( long nY = nStartY , nYDst = 0L; nY <= nEndY; nY++, nYDst++ )
    {
        long nTop = bVMirr ? ( nY + 1 ) : nY;
        long nBottom = bVMirr ? nY : ( nY + 1 ) ;

        long nLineStart, nLineRange;
        if( nY ==nEndY )
        {
            nLineStart = pMapIY[ nY ];
            nLineRange = 0;
        }
        else
        {
            nLineStart = pMapIY[ nTop ] ;
            nLineRange = ( pMapIY[ nBottom ] == pMapIY[ nTop ] ) ? 1 :( pMapIY[ nBottom ] - pMapIY[ nTop ] );
        }

        for( long nX = nStartX , nXDst = 0L; nX <= nEndX; nX++ )
        {
            long nLeft = bHMirr ? ( nX + 1 ) : nX;
            long nRight = bHMirr ? nX : ( nX + 1 ) ;

            long nRowStart, nRowRange;
            if( nX == nEndX )
            {
                nRowStart = pMapIX[ nX ];
                nRowRange = 0;
            }
            else
            {
                nRowStart = pMapIX[ nLeft ];
                nRowRange = ( pMapIX[ nRight ] == pMapIX[ nLeft ] )? 1 : ( pMapIX[ nRight ] - pMapIX[ nLeft ] );
            }

            long nSumR = 0;
            long nSumG = 0;
            long nSumB = 0;
            long nTotalWeightY = 0;

            for(int i = 0; i<= nLineRange; i++)
            {
                long nSumRowR = 0;
                long nSumRowG = 0;
                long nSumRowB = 0;
                long nTotalWeightX = 0;

                for(int j = 0; j <= nRowRange; j++)
                {
                    BitmapColor aCol0 = pAcc->GetPaletteColor ( pAcc->GetPixelIndex( nLineStart + i, nRowStart + j ) );

                    if(nX == nEndX )
                    {

                        nSumRowB += aCol0.GetBlue() << 7L;
                        nSumRowG += aCol0.GetGreen() << 7L;
                        nSumRowR += aCol0.GetRed() << 7L;
                        nTotalWeightX += 1 << 7L;
                    }
                    else if( j == 0 )
                    {

                        long nWeightX = (nMax- pMapFX[ nLeft ]) ;
                        nSumRowB += ( nWeightX *aCol0.GetBlue()) ;
                        nSumRowG += ( nWeightX *aCol0.GetGreen()) ;
                        nSumRowR += ( nWeightX *aCol0.GetRed()) ;
                        nTotalWeightX += nWeightX;
                    }
                    else if ( nRowRange == j )
                    {

                        long nWeightX = pMapFX[ nRight ] ;
                        nSumRowB += ( nWeightX *aCol0.GetBlue() );
                        nSumRowG += ( nWeightX *aCol0.GetGreen() );
                        nSumRowR += ( nWeightX *aCol0.GetRed() );
                        nTotalWeightX += nWeightX;
                    }
                    else
                    {

                        nSumRowB += aCol0.GetBlue() << 7L;
                        nSumRowG += aCol0.GetGreen() << 7L;
                        nSumRowR += aCol0.GetRed() << 7L;
                        nTotalWeightX += 1 << 7L;
                    }
                }

                long nWeightY = nMax;
                if( nY == nEndY )
                    nWeightY = nMax;
                else if( i == 0 )
                    nWeightY = nMax - pMapFY[ nTop ];
                else if( nLineRange == 1 )
                    nWeightY = pMapFY[ nTop ];
                else if ( nLineRange == i )
                    nWeightY = pMapFY[ nBottom ];

                nSumB += nWeightY * ( nSumRowB / nTotalWeightX );
                nSumG += nWeightY * ( nSumRowG / nTotalWeightX );
                nSumR += nWeightY * ( nSumRowR / nTotalWeightX );
                nTotalWeightY += nWeightY;
            }

            BitmapColor aColRes( ( sal_uInt8 ) (( nSumR / nTotalWeightY ) ),
                    ( sal_uInt8 ) (( nSumG / nTotalWeightY) ),
                    ( sal_uInt8 ) (( nSumB / nTotalWeightY) ));
            pWAcc->SetPixel( nYDst, nXDst++, aColRes );
        }
    }
}

void scale24bitBGR2(BitmapReadAccess* pAcc, BitmapWriteAccess* pWAcc,
                    long nStartX, long nEndX, long nStartY, long nEndY,
                    bool bVMirr, bool bHMirr)
{
    boost::scoped_array<long> pMapIX(new long[pWAcc->Width()]);
    boost::scoped_array<long> pMapIY(new long[pWAcc->Height()]);
    boost::scoped_array<long> pMapFX(new long[pWAcc->Width()]);
    boost::scoped_array<long> pMapFY(new long[pWAcc->Height()]);

    generateMap(pAcc->Width(),  pWAcc->Width(),  bHMirr, pMapIX.get(), pMapFX.get());
    generateMap(pAcc->Height(), pWAcc->Height(), bVMirr, pMapIY.get(), pMapFY.get());

    const long nMax = 1 << 7L;

    for( long nY = nStartY , nYDst = 0L; nY <= nEndY; nY++, nYDst++ )
    {
        long nTop = bVMirr ? ( nY + 1 ) : nY;
        long nBottom = bVMirr ? nY : ( nY + 1 ) ;

        long nLineStart;
        long nLineRange;
        if( nY ==nEndY )
        {
            nLineStart = pMapIY[ nY ];
            nLineRange = 0;
        }
        else
        {
            nLineStart = pMapIY[ nTop ] ;
            nLineRange = ( pMapIY[ nBottom ] == pMapIY[ nTop ] ) ? 1 :( pMapIY[ nBottom ] - pMapIY[ nTop ] );
        }

        for( long nX = nStartX , nXDst = 0L; nX <= nEndX; nX++ )
        {
            long nLeft = bHMirr ? ( nX + 1 ) : nX;
            long nRight = bHMirr ? nX : ( nX + 1 ) ;

            long nRowStart;
            long nRowRange;
            if( nX == nEndX )
            {
                nRowStart = pMapIX[ nX ];
                nRowRange = 0;
            }
            else
            {
                nRowStart = pMapIX[ nLeft ];
                nRowRange = ( pMapIX[ nRight ] == pMapIX[ nLeft ] )? 1 : ( pMapIX[ nRight ] - pMapIX[ nLeft ] );
            }

            long nSumR = 0;
            long nSumG = 0;
            long nSumB = 0;
            long nTotalWeightY = 0;

            for(int i = 0; i<= nLineRange; i++)
            {
                Scanline pTmpY = pAcc->GetScanline( nLineStart + i );
                Scanline pTmpX = pTmpY + 3L * nRowStart;
                long nSumRowR = 0;
                long nSumRowG = 0;
                long nSumRowB = 0;
                long nTotalWeightX = 0;

                for(int j = 0; j <= nRowRange; j++)
                {
                    if(nX == nEndX )
                    {
                        nSumRowB += ( *pTmpX ) << 7L;pTmpX++;
                        nSumRowG += ( *pTmpX ) << 7L;pTmpX++;
                        nSumRowR += ( *pTmpX ) << 7L;pTmpX++;
                        nTotalWeightX += 1 << 7L;
                    }
                    else if( j == 0 )
                    {
                        long nWeightX = (nMax- pMapFX[ nLeft ]) ;
                        nSumRowB += ( nWeightX *( *pTmpX )) ;pTmpX++;
                        nSumRowG += ( nWeightX *( *pTmpX )) ;pTmpX++;
                        nSumRowR += ( nWeightX *( *pTmpX )) ;pTmpX++;
                        nTotalWeightX += nWeightX;
                    }
                    else if ( nRowRange == j )
                    {
                        long nWeightX = pMapFX[ nRight ] ;
                        nSumRowB += ( nWeightX *( *pTmpX ) );pTmpX++;
                        nSumRowG += ( nWeightX *( *pTmpX ) );pTmpX++;
                        nSumRowR += ( nWeightX *( *pTmpX ) );pTmpX++;
                        nTotalWeightX += nWeightX;
                    }
                    else
                    {
                        nSumRowB += ( *pTmpX ) << 7L;pTmpX++;
                        nSumRowG += ( *pTmpX ) << 7L;pTmpX++;
                        nSumRowR += ( *pTmpX ) << 7L;pTmpX++;
                        nTotalWeightX += 1 << 7L;
                    }
                }

                long nWeightY = nMax;
                if( nY == nEndY )
                    nWeightY = nMax;
                else if( i == 0 )
                    nWeightY = nMax - pMapFY[ nTop ];
                else if( nLineRange == 1 )
                    nWeightY = pMapFY[ nTop ];
                else if ( nLineRange == i )
                    nWeightY = pMapFY[ nBottom ];

                nSumB += nWeightY * ( nSumRowB / nTotalWeightX );
                nSumG += nWeightY * ( nSumRowG / nTotalWeightX );
                nSumR += nWeightY * ( nSumRowR / nTotalWeightX );
                nTotalWeightY += nWeightY;
            }

            BitmapColor aColRes( ( sal_uInt8 ) (( nSumR / nTotalWeightY ) ),
                    ( sal_uInt8 ) (( nSumG / nTotalWeightY) ),
                    ( sal_uInt8 ) (( nSumB / nTotalWeightY) ));
            pWAcc->SetPixel( nYDst, nXDst++, aColRes );
        }
    }
}

void scale24bitRGB2(BitmapReadAccess* pAcc, BitmapWriteAccess* pWAcc,
                    long nStartX, long nEndX, long nStartY, long nEndY,
                    bool bVMirr, bool bHMirr)
{
    boost::scoped_array<long> pMapIX(new long[pWAcc->Width()]);
    boost::scoped_array<long> pMapIY(new long[pWAcc->Height()]);
    boost::scoped_array<long> pMapFX(new long[pWAcc->Width()]);
    boost::scoped_array<long> pMapFY(new long[pWAcc->Height()]);

    generateMap(pAcc->Width(),  pWAcc->Width(),  bHMirr, pMapIX.get(), pMapFX.get());
    generateMap(pAcc->Height(), pWAcc->Height(), bVMirr, pMapIY.get(), pMapFY.get());

    const long nMax = 1 << 7L;

    for( long nY = nStartY , nYDst = 0L; nY <= nEndY; nY++, nYDst++ )
    {
        long nTop = bVMirr ? ( nY + 1 ) : nY;
        long nBottom = bVMirr ? nY : ( nY + 1 ) ;

        long nLineStart, nLineRange;
        if( nY ==nEndY )
        {
            nLineStart = pMapIY[ nY ];
            nLineRange = 0;
        }
        else
        {
            nLineStart = pMapIY[ nTop ] ;
            nLineRange = ( pMapIY[ nBottom ] == pMapIY[ nTop ] ) ? 1 :( pMapIY[ nBottom ] - pMapIY[ nTop ] );
        }

        for( long nX = nStartX , nXDst = 0L; nX <= nEndX; nX++ )
        {
            long nLeft = bHMirr ? ( nX + 1 ) : nX;
            long nRight = bHMirr ? nX : ( nX + 1 ) ;

            long nRowStart, nRowRange;
            if( nX == nEndX )
            {
                nRowStart = pMapIX[ nX ];
                nRowRange = 0;
            }
            else
            {
                nRowStart = pMapIX[ nLeft ];
                nRowRange = ( pMapIX[ nRight ] == pMapIX[ nLeft ] )? 1 : ( pMapIX[ nRight ] - pMapIX[ nLeft ] );
            }

            long nSumR = 0;
            long nSumG = 0;
            long nSumB = 0;
            long nTotalWeightY = 0;

            for(int i = 0; i<= nLineRange; i++)
            {
                Scanline pTmpY = pAcc->GetScanline( nLineStart + i );
                Scanline pTmpX = pTmpY + 3L * nRowStart;
                long nSumRowR = 0;
                long nSumRowG = 0;
                long nSumRowB = 0;
                long nTotalWeightX = 0;

                for(int j = 0; j <= nRowRange; j++)
                {
                    if(nX == nEndX )
                    {
                        nSumRowR += ( *pTmpX ) << 7L;pTmpX++;
                        nSumRowG += ( *pTmpX ) << 7L;pTmpX++;
                        nSumRowB += ( *pTmpX ) << 7L;pTmpX++;
                        nTotalWeightX += 1 << 7L;
                    }
                    else if( j == 0 )
                    {
                        long nWeightX = (nMax- pMapFX[ nLeft ]) ;
                        nSumRowR += ( nWeightX *( *pTmpX )) ;pTmpX++;
                        nSumRowG += ( nWeightX *( *pTmpX )) ;pTmpX++;
                        nSumRowB += ( nWeightX *( *pTmpX )) ;pTmpX++;
                        nTotalWeightX += nWeightX;
                    }
                    else if ( nRowRange == j )
                    {
                        long nWeightX = pMapFX[ nRight ] ;
                        nSumRowR += ( nWeightX *( *pTmpX ) );pTmpX++;
                        nSumRowG += ( nWeightX *( *pTmpX ) );pTmpX++;
                        nSumRowB += ( nWeightX *( *pTmpX ) );pTmpX++;
                        nTotalWeightX += nWeightX;
                    }
                    else
                    {
                        nSumRowR += ( *pTmpX ) << 7L;pTmpX++;
                        nSumRowG += ( *pTmpX ) << 7L;pTmpX++;
                        nSumRowB += ( *pTmpX ) << 7L;pTmpX++;
                        nTotalWeightX += 1 << 7L;
                    }
                }

                long nWeightY = nMax;
                if( nY == nEndY )
                    nWeightY = nMax;
                else if( i == 0 )
                    nWeightY = nMax - pMapFY[ nTop ];
                else if( nLineRange == 1 )
                    nWeightY = pMapFY[ nTop ];
                else if ( nLineRange == i )
                    nWeightY = pMapFY[ nBottom ];

                nSumB += nWeightY * ( nSumRowB / nTotalWeightX );
                nSumG += nWeightY * ( nSumRowG / nTotalWeightX );
                nSumR += nWeightY * ( nSumRowR / nTotalWeightX );
                nTotalWeightY += nWeightY;
            }

            BitmapColor aColRes( ( sal_uInt8 ) (( nSumR / nTotalWeightY ) ),
                    ( sal_uInt8 ) (( nSumG / nTotalWeightY) ),
                    ( sal_uInt8 ) (( nSumB / nTotalWeightY) ));
            pWAcc->SetPixel( nYDst, nXDst++, aColRes );
        }
    }
}

void scaleNonPalleteGeneral2(BitmapReadAccess* pAcc, BitmapWriteAccess* pWAcc,
                    long nStartX, long nEndX, long nStartY, long nEndY,
                    bool bVMirr, bool bHMirr)
{
    boost::scoped_array<long> pMapIX(new long[pWAcc->Width()]);
    boost::scoped_array<long> pMapIY(new long[pWAcc->Height()]);
    boost::scoped_array<long> pMapFX(new long[pWAcc->Width()]);
    boost::scoped_array<long> pMapFY(new long[pWAcc->Height()]);

    generateMap(pAcc->Width(),  pWAcc->Width(),  bHMirr, pMapIX.get(), pMapFX.get());
    generateMap(pAcc->Height(), pWAcc->Height(), bVMirr, pMapIY.get(), pMapFY.get());

    const long nMax = 1 << 7L;

    for( long nY = nStartY , nYDst = 0L; nY <= nEndY; nY++, nYDst++ )
    {
        long nTop = bVMirr ? ( nY + 1 ) : nY;
        long nBottom = bVMirr ? nY : ( nY + 1 ) ;

        long nLineStart, nLineRange;
        if( nY ==nEndY )
        {
            nLineStart = pMapIY[ nY ];
            nLineRange = 0;
        }
        else
        {
            nLineStart = pMapIY[ nTop ] ;
            nLineRange = ( pMapIY[ nBottom ] == pMapIY[ nTop ] ) ? 1 :( pMapIY[ nBottom ] - pMapIY[ nTop ] );
        }

        for( long nX = nStartX , nXDst = 0L; nX <= nEndX; nX++ )
        {
            long nLeft = bHMirr ? ( nX + 1 ) : nX;
            long nRight = bHMirr ? nX : ( nX + 1 ) ;

            long nRowStart, nRowRange;
            if( nX == nEndX )
            {
                nRowStart = pMapIX[ nX ];
                nRowRange = 0;
            }
            else
            {
                nRowStart = pMapIX[ nLeft ];
                nRowRange = ( pMapIX[ nRight ] == pMapIX[ nLeft ] )? 1 : ( pMapIX[ nRight ] - pMapIX[ nLeft ] );
            }

            long nSumR = 0;
            long nSumG = 0;
            long nSumB = 0;
            long nTotalWeightY = 0;

            for(int i = 0; i<= nLineRange; i++)
            {
                long nSumRowR = 0;
                long nSumRowG = 0;
                long nSumRowB = 0;
                long nTotalWeightX = 0;

                for(int j = 0; j <= nRowRange; j++)
                {
                    BitmapColor aCol0 = pAcc->GetPixel( nLineStart + i, nRowStart + j );

                    if(nX == nEndX )
                    {

                        nSumRowB += aCol0.GetBlue() << 7L;
                        nSumRowG += aCol0.GetGreen() << 7L;
                        nSumRowR += aCol0.GetRed() << 7L;
                        nTotalWeightX += 1 << 7L;
                    }
                    else if( j == 0 )
                    {

                        long nWeightX = (nMax- pMapFX[ nLeft ]) ;
                        nSumRowB += ( nWeightX *aCol0.GetBlue()) ;
                        nSumRowG += ( nWeightX *aCol0.GetGreen()) ;
                        nSumRowR += ( nWeightX *aCol0.GetRed()) ;
                        nTotalWeightX += nWeightX;
                    }
                    else if ( nRowRange == j )
                    {

                        long nWeightX = pMapFX[ nRight ] ;
                        nSumRowB += ( nWeightX *aCol0.GetBlue() );
                        nSumRowG += ( nWeightX *aCol0.GetGreen() );
                        nSumRowR += ( nWeightX *aCol0.GetRed() );
                        nTotalWeightX += nWeightX;
                    }
                    else
                    {
                        nSumRowB += aCol0.GetBlue() << 7L;
                        nSumRowG += aCol0.GetGreen() << 7L;
                        nSumRowR += aCol0.GetRed() << 7L;
                        nTotalWeightX += 1 << 7L;
                    }
                }

                long nWeightY = nMax;
                if( nY == nEndY )
                    nWeightY = nMax;
                else if( i == 0 )
                    nWeightY = nMax - pMapFY[ nTop ];
                else if( nLineRange == 1 )
                    nWeightY = pMapFY[ nTop ];
                else if ( nLineRange == i )
                    nWeightY = pMapFY[ nBottom ];

                nSumB += nWeightY * ( nSumRowB / nTotalWeightX );
                nSumG += nWeightY * ( nSumRowG / nTotalWeightX );
                nSumR += nWeightY * ( nSumRowR / nTotalWeightX );
                nTotalWeightY += nWeightY;
            }

            BitmapColor aColRes( ( sal_uInt8 ) (( nSumR / nTotalWeightY) ),
                    ( sal_uInt8 ) (( nSumG / nTotalWeightY) ),
                    ( sal_uInt8 ) (( nSumB / nTotalWeightY) ));
            pWAcc->SetPixel( nYDst, nXDst++, aColRes );

        }
    }
}

BitmapScaleSuper::BitmapScaleSuper(const double& rScaleX, const double& rScaleY) :
    mrScaleX(rScaleX),
    mrScaleY(rScaleY)
{}

BitmapScaleSuper::~BitmapScaleSuper()
{}

bool BitmapScaleSuper::filter(Bitmap& rBitmap)
{
    bool bRet = false;

    const Size aSizePix(rBitmap.GetSizePixel());

    bool bHMirr = mrScaleX < 0;
    bool bVMirr = mrScaleY < 0;

    double fScaleX = std::fabs(mrScaleX);
    double fScaleY = std::fabs(mrScaleY);

    const long nDstW = FRound(aSizePix.Width()  * fScaleX);
    const long nDstH = FRound(aSizePix.Height() * fScaleY);

    const double fScaleThresh = 0.6;

    if (nDstW <= 1L || nDstH <= 1L)
        return false;

    Bitmap::ScopedReadAccess pReadAccess(rBitmap);

    Bitmap aOutBmp(Size(nDstW, nDstH), 24);
    Bitmap::ScopedWriteAccess pWriteAccess(aOutBmp);

    const long nStartX = 0;
    const long nStartY = 0;
    const long nEndX = nDstW - 1L;
    const long nEndY = nDstH - 1L;

    if (pReadAccess && pWriteAccess)
    {
        if( pReadAccess->HasPalette() )
        {
            if( pReadAccess->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL )
            {
                if( fScaleX >= fScaleThresh && fScaleY >= fScaleThresh )
                {
                    scalePallete8bit(pReadAccess.get(), pWriteAccess.get(),
                               nStartX, nEndX, nStartY, nEndY,
                               bVMirr, bHMirr);
                }
                else
                {
                    scalePallete8bit2(pReadAccess.get(), pWriteAccess.get(),
                               nStartX, nEndX, nStartY, nEndY,
                               bVMirr, bHMirr);
                }
            }
            else
            {
                if( fScaleX >= fScaleThresh && fScaleY >= fScaleThresh )
                {
                    scalePalleteGeneral(pReadAccess.get(), pWriteAccess.get(),
                               nStartX, nEndX, nStartY, nEndY,
                               bVMirr, bHMirr);
                }
                else
                {
                    scalePalleteGeneral2(pReadAccess.get(), pWriteAccess.get(),
                               nStartX, nEndX, nStartY, nEndY,
                               bVMirr, bHMirr);
                }
            }
        }
        else
        {
            if( pReadAccess->GetScanlineFormat() == BMP_FORMAT_24BIT_TC_BGR )
            {
                if( fScaleX >= fScaleThresh && fScaleY >= fScaleThresh )
                {
                    scale24bitBGR(pReadAccess.get(), pWriteAccess.get(),
                               nStartX, nEndX, nStartY, nEndY,
                               bVMirr, bHMirr);
                }
                else
                {
                    scale24bitBGR2(pReadAccess.get(), pWriteAccess.get(),
                               nStartX, nEndX, nStartY, nEndY,
                               bVMirr, bHMirr);
                }
            }
            else if( pReadAccess->GetScanlineFormat() == BMP_FORMAT_24BIT_TC_RGB )
            {
                if( fScaleX >= fScaleThresh && fScaleY >= fScaleThresh )
                {
                    scale24bitRGB(pReadAccess.get(), pWriteAccess.get(),
                               nStartX, nEndX, nStartY, nEndY,
                               bVMirr, bHMirr);
                }
                else
                {
                    scale24bitRGB2(pReadAccess.get(), pWriteAccess.get(),
                               nStartX, nEndX, nStartY, nEndY,
                               bVMirr, bHMirr);
                }
            }
            else
            {
                if( fScaleX >= fScaleThresh && fScaleY >= fScaleThresh )
                {
                    scaleNonPalleteGeneral(pReadAccess.get(), pWriteAccess.get(),
                               nStartX, nEndX, nStartY, nEndY,
                               bVMirr, bHMirr);
                }
                else
                {
                    scaleNonPalleteGeneral2(pReadAccess.get(), pWriteAccess.get(),
                               nStartX, nEndX, nStartY, nEndY,
                               bVMirr, bHMirr);
                }
            }
        }

        bRet = true;
    }

    if( bRet )
    {
        rBitmap.AdaptBitCount(aOutBmp);
        rBitmap = aOutBmp;
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
