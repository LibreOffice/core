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

#include <vcl/bitmapaccess.hxx>
#include <vcl/bitmapscalesuper.hxx>

#include <algorithm>
#include <memory>
#include <comphelper/threadpool.hxx>

namespace {

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
        pMapFX[nX] = (long) ((fTemp - pMapIX[nX]) * 128.0);
    }
}

struct ScaleContext {
    BitmapReadAccess  *mpSrc;
    BitmapWriteAccess *mpDest;
    long mnSrcW, mnDestW;
    long mnSrcH, mnDestH;
    bool mbHMirr, mbVMirr;
    std::unique_ptr<long[]> mpMapIX;
    std::unique_ptr<long[]> mpMapIY;
    std::unique_ptr<long[]> mpMapFX;
    std::unique_ptr<long[]> mpMapFY;
    ScaleContext( BitmapReadAccess *pSrc,
                  BitmapWriteAccess *pDest,
                  long nSrcW, long nDestW,
                  long nSrcH, long nDestH,
                  bool bHMirr, bool bVMirr)
        : mpSrc( pSrc ), mpDest( pDest )
        , mnSrcW( nSrcW ), mnDestW( nDestW )
        , mnSrcH( nSrcH ), mnDestH( nDestH )
        , mbHMirr( bHMirr ), mbVMirr( bVMirr )
        , mpMapIX( new long[ nDestW ] )
        , mpMapIY( new long[ nDestH ] )
        , mpMapFX( new long[ nDestW ] )
        , mpMapFY( new long[ nDestH ] )
    {
        generateMap(nSrcW, nDestW, bHMirr, mpMapIX.get(), mpMapFX.get());
        generateMap(nSrcH, nDestH, bVMirr, mpMapIY.get(), mpMapFY.get());
    }
};

#define SCALE_THREAD_STRIP 32
struct ScaleRangeContext {
    ScaleContext *mrCtx;
    long mnStartY, mnEndY;
    ScaleRangeContext( ScaleContext *rCtx, long nStartY )
        : mrCtx( rCtx ), mnStartY( nStartY ),
          mnEndY( nStartY + SCALE_THREAD_STRIP ) {}
};

typedef void (*ScaleRangeFn)(ScaleContext &rCtx, long nStartY, long nEndY);

class ScaleTask : public comphelper::ThreadTask
{
    ScaleRangeFn mpFn;
    std::vector< ScaleRangeContext > maStrips;
public:
    explicit ScaleTask( ScaleRangeFn pFn ) : mpFn( pFn ) {}
    void push( ScaleRangeContext &aRC ) { maStrips.push_back( aRC ); }
    virtual void doWork() override
    {
        std::vector< ScaleRangeContext >::iterator it;
        for (it = maStrips.begin(); it != maStrips.end(); ++it)
            mpFn( *(it->mrCtx), it->mnStartY, it->mnEndY );
    }
};

void scalePallete8bit(ScaleContext &rCtx, long nStartY, long nEndY)
{
    const long nStartX = 0, nEndX = rCtx.mnDestW - 1L;

    for( long nY = nStartY; nY <= nEndY; nY++ )
    {
        long nTempY = rCtx.mpMapIY[ nY ];
        long nTempFY = rCtx.mpMapFY[ nY ];
        Scanline pLine0 = rCtx.mpSrc->GetScanline( nTempY );
        Scanline pLine1 = rCtx.mpSrc->GetScanline( ++nTempY );

        for(long nX = nStartX, nXDst = 0L; nX <= nEndX; nX++ )
        {
            long nTempX = rCtx.mpMapIX[ nX ];
            long nTempFX = rCtx.mpMapFX[ nX ];

            const BitmapColor& rCol0 = rCtx.mpSrc->GetPaletteColor( pLine0[ nTempX ] );
            const BitmapColor& rCol2 = rCtx.mpSrc->GetPaletteColor( pLine1[ nTempX ] );
            const BitmapColor& rCol1 = rCtx.mpSrc->GetPaletteColor( pLine0[ ++nTempX ] );
            const BitmapColor& rCol3 = rCtx.mpSrc->GetPaletteColor( pLine1[ nTempX ] );

            sal_uInt8 cR0 = MAP( rCol0.GetRed(), rCol1.GetRed(), nTempFX );
            sal_uInt8 cG0 = MAP( rCol0.GetGreen(), rCol1.GetGreen(), nTempFX );
            sal_uInt8 cB0 = MAP( rCol0.GetBlue(), rCol1.GetBlue(), nTempFX );

            sal_uInt8 cR1 = MAP( rCol2.GetRed(), rCol3.GetRed(), nTempFX );
            sal_uInt8 cG1 = MAP( rCol2.GetGreen(), rCol3.GetGreen(), nTempFX );
            sal_uInt8 cB1 = MAP( rCol2.GetBlue(), rCol3.GetBlue(), nTempFX );

            BitmapColor aColRes( MAP( cR0, cR1, nTempFY ),
                    MAP( cG0, cG1, nTempFY ),
                    MAP( cB0, cB1, nTempFY ) );
            rCtx.mpDest->SetPixel( nY, nXDst++, aColRes );
        }
    }
}

void scalePalleteGeneral(ScaleContext &rCtx, long nStartY, long nEndY)
{
    const long nStartX = 0, nEndX = rCtx.mnDestW - 1L;

    for( long nY = nStartY; nY <= nEndY; nY++ )
    {
        long nTempY = rCtx.mpMapIY[ nY ];
        long nTempFY = rCtx.mpMapFY[ nY ];

        for( long nX = nStartX, nXDst = 0L; nX <= nEndX; nX++ )
        {
            long nTempX = rCtx.mpMapIX[ nX ];
            long nTempFX = rCtx.mpMapFX[ nX ];

            BitmapColor aCol0 = rCtx.mpSrc->GetPaletteColor( rCtx.mpSrc->GetPixelIndex( nTempY, nTempX ) );
            BitmapColor aCol1 = rCtx.mpSrc->GetPaletteColor( rCtx.mpSrc->GetPixelIndex( nTempY, ++nTempX ) );
            sal_uInt8 cR0 = MAP( aCol0.GetRed(), aCol1.GetRed(), nTempFX );
            sal_uInt8 cG0 = MAP( aCol0.GetGreen(), aCol1.GetGreen(), nTempFX );
            sal_uInt8 cB0 = MAP( aCol0.GetBlue(), aCol1.GetBlue(), nTempFX );

            aCol1 = rCtx.mpSrc->GetPaletteColor( rCtx.mpSrc->GetPixelIndex( ++nTempY, nTempX ) );
            aCol0 = rCtx.mpSrc->GetPaletteColor( rCtx.mpSrc->GetPixelIndex( nTempY--, --nTempX ) );
            sal_uInt8 cR1 = MAP( aCol0.GetRed(), aCol1.GetRed(), nTempFX );
            sal_uInt8 cG1 = MAP( aCol0.GetGreen(), aCol1.GetGreen(), nTempFX );
            sal_uInt8 cB1 = MAP( aCol0.GetBlue(), aCol1.GetBlue(), nTempFX );

            BitmapColor aColRes( MAP( cR0, cR1, nTempFY ),
                    MAP( cG0, cG1, nTempFY ),
                    MAP( cB0, cB1, nTempFY ) );
            rCtx.mpDest->SetPixel( nY, nXDst++, aColRes );
        }
    }
}

void scale24bitBGR(ScaleContext &rCtx, long nStartY, long nEndY)
{
    const long nStartX = 0, nEndX = rCtx.mnDestW - 1L;

    for( long nY = nStartY; nY <= nEndY; nY++ )
    {
        long nTempY = rCtx.mpMapIY[ nY ];
        long nTempFY = rCtx.mpMapFY[ nY ];
        Scanline pLine0 = rCtx.mpSrc->GetScanline( nTempY );
        Scanline pLine1 = rCtx.mpSrc->GetScanline( ++nTempY );

        for( long nX = nStartX, nXDst = 0L; nX <= nEndX; nX++ )
        {
            long nOff = 3L * rCtx.mpMapIX[ nX ];
            long nTempFX = rCtx.mpMapFX[ nX ];

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
            rCtx.mpDest->SetPixel( nY, nXDst++, aColRes );
        }
    }
}

void scale24bitRGB(ScaleContext &rCtx, long nStartY, long nEndY)
{
    const long nStartX = 0, nEndX = rCtx.mnDestW - 1L;

    for( long nY = nStartY; nY <= nEndY; nY++ )
    {
        long nTempY = rCtx.mpMapIY[ nY ];
        long nTempFY = rCtx.mpMapFY[ nY ];
        Scanline pLine0 = rCtx.mpSrc->GetScanline( nTempY );
        Scanline pLine1 = rCtx.mpSrc->GetScanline( ++nTempY );

        for( long nX = nStartX, nXDst = 0L; nX <= nEndX; nX++ )
        {
            long nOff = 3L * rCtx.mpMapIX[ nX ];
            long nTempFX = rCtx.mpMapFX[ nX ];

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
            rCtx.mpDest->SetPixel( nY, nXDst++, aColRes );
        }
    }
}

void scaleNonPalleteGeneral(ScaleContext &rCtx, long nStartY, long nEndY)
{
    const long nStartX = 0, nEndX = rCtx.mnDestW - 1L;

    for( long nY = nStartY; nY <= nEndY; nY++ )
    {
        long nTempY = rCtx.mpMapIY[ nY ];
        long nTempFY = rCtx.mpMapFY[ nY ];

        for( long nX = nStartX, nXDst = 0L; nX <= nEndX; nX++ )
        {
            long nTempX = rCtx.mpMapIX[ nX ];
            long nTempFX = rCtx.mpMapFX[ nX ];

            BitmapColor aCol0 = rCtx.mpSrc->GetPixel( nTempY, nTempX );
            BitmapColor aCol1 = rCtx.mpSrc->GetPixel( nTempY, ++nTempX );
            sal_uInt8 cR0 = MAP( aCol0.GetRed(), aCol1.GetRed(), nTempFX );
            sal_uInt8 cG0 = MAP( aCol0.GetGreen(), aCol1.GetGreen(), nTempFX );
            sal_uInt8 cB0 = MAP( aCol0.GetBlue(), aCol1.GetBlue(), nTempFX );

            aCol1 = rCtx.mpSrc->GetPixel( ++nTempY, nTempX );
            aCol0 = rCtx.mpSrc->GetPixel( nTempY--, --nTempX );
            sal_uInt8 cR1 = MAP( aCol0.GetRed(), aCol1.GetRed(), nTempFX );
            sal_uInt8 cG1 = MAP( aCol0.GetGreen(), aCol1.GetGreen(), nTempFX );
            sal_uInt8 cB1 = MAP( aCol0.GetBlue(), aCol1.GetBlue(), nTempFX );

            BitmapColor aColRes( MAP( cR0, cR1, nTempFY ),
                    MAP( cG0, cG1, nTempFY ),
                    MAP( cB0, cB1, nTempFY ) );
            rCtx.mpDest->SetPixel( nY, nXDst++, aColRes );
        }
    }
}

void scalePallete8bit2(ScaleContext &rCtx, long nStartY, long nEndY)
{
    const long nStartX = 0, nEndX = rCtx.mnDestW - 1L;
    const long nMax = 1 << 7L;

    for( long nY = nStartY; nY <= nEndY; nY++ )
    {
        long nTop = rCtx.mbVMirr ? ( nY + 1 ) : nY;
        long nBottom = rCtx.mbVMirr ? nY : ( nY + 1 ) ;

        long nLineStart, nLineRange;
        if( nY == nEndY )
        {
            nLineStart = rCtx.mpMapIY[ nY ];
            nLineRange = 0;
        }
        else
        {
            nLineStart = rCtx.mpMapIY[ nTop ] ;
            nLineRange = ( rCtx.mpMapIY[ nBottom ] == rCtx.mpMapIY[ nTop ] ) ? 1 :( rCtx.mpMapIY[ nBottom ] - rCtx.mpMapIY[ nTop ] );
        }

        for( long nX = nStartX , nXDst = 0L; nX <= nEndX; nX++ )
        {
            long nLeft = rCtx.mbHMirr ? ( nX + 1 ) : nX;
            long nRight = rCtx.mbHMirr ? nX : ( nX + 1 ) ;

            long nRowStart;
            long nRowRange;
            if( nX == nEndX )
            {
                nRowStart = rCtx.mpMapIX[ nX ];
                nRowRange = 0;
            }
            else
            {
                nRowStart = rCtx.mpMapIX[ nLeft ];
                nRowRange = ( rCtx.mpMapIX[ nRight ] == rCtx.mpMapIX[ nLeft ] )? 1 : ( rCtx.mpMapIX[ nRight ] - rCtx.mpMapIX[ nLeft ] );
            }

            long nSumR = 0;
            long nSumG = 0;
            long nSumB = 0;
            long nTotalWeightY = 0;

            for(long i = 0; i<= nLineRange; i++)
            {
                Scanline pTmpY = rCtx.mpSrc->GetScanline( nLineStart + i );
                long nSumRowR = 0;
                long nSumRowG = 0;
                long nSumRowB = 0;
                long nTotalWeightX = 0;

                for(long j = 0; j <= nRowRange; j++)
                {
                    const BitmapColor& rCol = rCtx.mpSrc->GetPaletteColor( pTmpY[ nRowStart + j ] );

                    if(nX == nEndX )
                    {
                        nSumRowB += rCol.GetBlue() << 7L;
                        nSumRowG += rCol.GetGreen() << 7L;
                        nSumRowR += rCol.GetRed() << 7L;
                        nTotalWeightX += 1 << 7L;
                    }
                    else if( j == 0 )
                    {
                        long nWeightX = (nMax- rCtx.mpMapFX[ nLeft ]) ;
                        nSumRowB += ( nWeightX *rCol.GetBlue()) ;
                        nSumRowG += ( nWeightX *rCol.GetGreen()) ;
                        nSumRowR += ( nWeightX *rCol.GetRed()) ;
                        nTotalWeightX += nWeightX;
                    }
                    else if ( nRowRange == j )
                    {
                        long nWeightX = rCtx.mpMapFX[ nRight ] ;
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
                    nWeightY = nMax - rCtx.mpMapFY[ nTop ];
                else if( nLineRange == 1 )
                    nWeightY = rCtx.mpMapFY[ nTop ];
                else if ( nLineRange == i )
                    nWeightY = rCtx.mpMapFY[ nBottom ];

                if (nTotalWeightX)
                {
                    nSumRowB /= nTotalWeightX;
                    nSumRowG /= nTotalWeightX;
                    nSumRowR /= nTotalWeightX;
                }

                nSumB += nWeightY * nSumRowB;
                nSumG += nWeightY * nSumRowG;
                nSumR += nWeightY * nSumRowR;
                nTotalWeightY += nWeightY;
            }

            if (nTotalWeightY)
            {
                nSumR /= nTotalWeightY;
                nSumG /= nTotalWeightY;
                nSumB /= nTotalWeightY;
            }

            BitmapColor aColRes((sal_uInt8)nSumR, (sal_uInt8)nSumG, (sal_uInt8)nSumB);
            rCtx.mpDest->SetPixel( nY, nXDst++, aColRes );
        }
    }
}

void scalePalleteGeneral2(ScaleContext &rCtx, long nStartY, long nEndY)
{
    const long nStartX = 0, nEndX = rCtx.mnDestW - 1L;
    const long nMax = 1 << 7L;

    for( long nY = nStartY; nY <= nEndY; nY++ )
    {
        long nTop = rCtx.mbVMirr ? ( nY + 1 ) : nY;
        long nBottom = rCtx.mbVMirr ? nY : ( nY + 1 ) ;

        long nLineStart, nLineRange;
        if( nY ==nEndY )
        {
            nLineStart = rCtx.mpMapIY[ nY ];
            nLineRange = 0;
        }
        else
        {
            nLineStart = rCtx.mpMapIY[ nTop ] ;
            nLineRange = ( rCtx.mpMapIY[ nBottom ] == rCtx.mpMapIY[ nTop ] ) ? 1 :( rCtx.mpMapIY[ nBottom ] - rCtx.mpMapIY[ nTop ] );
        }

        for( long nX = nStartX , nXDst = 0L; nX <= nEndX; nX++ )
        {
            long nLeft = rCtx.mbHMirr ? ( nX + 1 ) : nX;
            long nRight = rCtx.mbHMirr ? nX : ( nX + 1 ) ;

            long nRowStart, nRowRange;
            if( nX == nEndX )
            {
                nRowStart = rCtx.mpMapIX[ nX ];
                nRowRange = 0;
            }
            else
            {
                nRowStart = rCtx.mpMapIX[ nLeft ];
                nRowRange = ( rCtx.mpMapIX[ nRight ] == rCtx.mpMapIX[ nLeft ] )? 1 : ( rCtx.mpMapIX[ nRight ] - rCtx.mpMapIX[ nLeft ] );
            }

            long nSumR = 0;
            long nSumG = 0;
            long nSumB = 0;
            long nTotalWeightY = 0;

            for(long i = 0; i<= nLineRange; i++)
            {
                long nSumRowR = 0;
                long nSumRowG = 0;
                long nSumRowB = 0;
                long nTotalWeightX = 0;

                for(long j = 0; j <= nRowRange; j++)
                {
                    BitmapColor aCol0 = rCtx.mpSrc->GetPaletteColor ( rCtx.mpSrc->GetPixelIndex( nLineStart + i, nRowStart + j ) );

                    if(nX == nEndX )
                    {

                        nSumRowB += aCol0.GetBlue() << 7L;
                        nSumRowG += aCol0.GetGreen() << 7L;
                        nSumRowR += aCol0.GetRed() << 7L;
                        nTotalWeightX += 1 << 7L;
                    }
                    else if( j == 0 )
                    {

                        long nWeightX = (nMax- rCtx.mpMapFX[ nLeft ]) ;
                        nSumRowB += ( nWeightX *aCol0.GetBlue()) ;
                        nSumRowG += ( nWeightX *aCol0.GetGreen()) ;
                        nSumRowR += ( nWeightX *aCol0.GetRed()) ;
                        nTotalWeightX += nWeightX;
                    }
                    else if ( nRowRange == j )
                    {

                        long nWeightX = rCtx.mpMapFX[ nRight ] ;
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
                    nWeightY = nMax - rCtx.mpMapFY[ nTop ];
                else if( nLineRange == 1 )
                    nWeightY = rCtx.mpMapFY[ nTop ];
                else if ( nLineRange == i )
                    nWeightY = rCtx.mpMapFY[ nBottom ];

                if (nTotalWeightX)
                {
                    nSumRowB /= nTotalWeightX;
                    nSumRowG /= nTotalWeightX;
                    nSumRowR /= nTotalWeightX;
                }

                nSumB += nWeightY * nSumRowB;
                nSumG += nWeightY * nSumRowG;
                nSumR += nWeightY * nSumRowR;
                nTotalWeightY += nWeightY;
            }

            if (nTotalWeightY)
            {
                nSumR /= nTotalWeightY;
                nSumG /= nTotalWeightY;
                nSumB /= nTotalWeightY;
            }

            BitmapColor aColRes((sal_uInt8)nSumR, (sal_uInt8)nSumG, (sal_uInt8)nSumB);
            rCtx.mpDest->SetPixel( nY, nXDst++, aColRes );
        }
    }
}

void scale24bitBGR2(ScaleContext &rCtx, long nStartY, long nEndY)
{
    const long nStartX = 0, nEndX = rCtx.mnDestW - 1L;
    const long nMax = 1 << 7L;

    for( long nY = nStartY; nY <= nEndY; nY++ )
    {
        long nTop = rCtx.mbVMirr ? ( nY + 1 ) : nY;
        long nBottom = rCtx.mbVMirr ? nY : ( nY + 1 ) ;

        long nLineStart;
        long nLineRange;
        if( nY ==nEndY )
        {
            nLineStart = rCtx.mpMapIY[ nY ];
            nLineRange = 0;
        }
        else
        {
            nLineStart = rCtx.mpMapIY[ nTop ] ;
            nLineRange = ( rCtx.mpMapIY[ nBottom ] == rCtx.mpMapIY[ nTop ] ) ? 1 :( rCtx.mpMapIY[ nBottom ] - rCtx.mpMapIY[ nTop ] );
        }

        for( long nX = nStartX , nXDst = 0L; nX <= nEndX; nX++ )
        {
            long nLeft = rCtx.mbHMirr ? ( nX + 1 ) : nX;
            long nRight = rCtx.mbHMirr ? nX : ( nX + 1 ) ;

            long nRowStart;
            long nRowRange;
            if( nX == nEndX )
            {
                nRowStart = rCtx.mpMapIX[ nX ];
                nRowRange = 0;
            }
            else
            {
                nRowStart = rCtx.mpMapIX[ nLeft ];
                nRowRange = ( rCtx.mpMapIX[ nRight ] == rCtx.mpMapIX[ nLeft ] )? 1 : ( rCtx.mpMapIX[ nRight ] - rCtx.mpMapIX[ nLeft ] );
            }

            long nSumR = 0;
            long nSumG = 0;
            long nSumB = 0;
            long nTotalWeightY = 0;

            for(long i = 0; i<= nLineRange; i++)
            {
                Scanline pTmpY = rCtx.mpSrc->GetScanline( nLineStart + i );
                Scanline pTmpX = pTmpY + 3L * nRowStart;
                long nSumRowR = 0;
                long nSumRowG = 0;
                long nSumRowB = 0;
                long nTotalWeightX = 0;

                for(long j = 0; j <= nRowRange; j++)
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
                        long nWeightX = (nMax- rCtx.mpMapFX[ nLeft ]) ;
                        nSumRowB += ( nWeightX *( *pTmpX )) ;pTmpX++;
                        nSumRowG += ( nWeightX *( *pTmpX )) ;pTmpX++;
                        nSumRowR += ( nWeightX *( *pTmpX )) ;pTmpX++;
                        nTotalWeightX += nWeightX;
                    }
                    else if ( nRowRange == j )
                    {
                        long nWeightX = rCtx.mpMapFX[ nRight ] ;
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
                    nWeightY = nMax - rCtx.mpMapFY[ nTop ];
                else if( nLineRange == 1 )
                    nWeightY = rCtx.mpMapFY[ nTop ];
                else if ( nLineRange == i )
                    nWeightY = rCtx.mpMapFY[ nBottom ];

                if (nTotalWeightX)
                {
                    nSumRowB /= nTotalWeightX;
                    nSumRowG /= nTotalWeightX;
                    nSumRowR /= nTotalWeightX;
                }
                nSumB += nWeightY * nSumRowB;
                nSumG += nWeightY * nSumRowG;
                nSumR += nWeightY * nSumRowR;
                nTotalWeightY += nWeightY;
            }

            if (nTotalWeightY)
            {
                nSumR /= nTotalWeightY;
                nSumG /= nTotalWeightY;
                nSumB /= nTotalWeightY;
            }
            BitmapColor aColRes((sal_uInt8)nSumR, (sal_uInt8)nSumG, (sal_uInt8)nSumB);
            rCtx.mpDest->SetPixel( nY, nXDst++, aColRes );
        }
    }
}

void scale24bitRGB2(ScaleContext &rCtx, long nStartY, long nEndY)
{
    const long nStartX = 0, nEndX = rCtx.mnDestW - 1L;
    const long nMax = 1 << 7L;

    for( long nY = nStartY; nY <= nEndY; nY++ )
    {
        long nTop = rCtx.mbVMirr ? ( nY + 1 ) : nY;
        long nBottom = rCtx.mbVMirr ? nY : ( nY + 1 ) ;

        long nLineStart, nLineRange;
        if( nY ==nEndY )
        {
            nLineStart = rCtx.mpMapIY[ nY ];
            nLineRange = 0;
        }
        else
        {
            nLineStart = rCtx.mpMapIY[ nTop ] ;
            nLineRange = ( rCtx.mpMapIY[ nBottom ] == rCtx.mpMapIY[ nTop ] ) ? 1 :( rCtx.mpMapIY[ nBottom ] - rCtx.mpMapIY[ nTop ] );
        }

        for( long nX = nStartX , nXDst = 0L; nX <= nEndX; nX++ )
        {
            long nLeft = rCtx.mbHMirr ? ( nX + 1 ) : nX;
            long nRight = rCtx.mbHMirr ? nX : ( nX + 1 ) ;

            long nRowStart, nRowRange;
            if( nX == nEndX )
            {
                nRowStart = rCtx.mpMapIX[ nX ];
                nRowRange = 0;
            }
            else
            {
                nRowStart = rCtx.mpMapIX[ nLeft ];
                nRowRange = ( rCtx.mpMapIX[ nRight ] == rCtx.mpMapIX[ nLeft ] )? 1 : ( rCtx.mpMapIX[ nRight ] - rCtx.mpMapIX[ nLeft ] );
            }

            long nSumR = 0;
            long nSumG = 0;
            long nSumB = 0;
            long nTotalWeightY = 0;

            for(long i = 0; i<= nLineRange; i++)
            {
                Scanline pTmpY = rCtx.mpSrc->GetScanline( nLineStart + i );
                Scanline pTmpX = pTmpY + 3L * nRowStart;
                long nSumRowR = 0;
                long nSumRowG = 0;
                long nSumRowB = 0;
                long nTotalWeightX = 0;

                for(long j = 0; j <= nRowRange; j++)
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
                        long nWeightX = (nMax- rCtx.mpMapFX[ nLeft ]) ;
                        nSumRowR += ( nWeightX *( *pTmpX )) ;pTmpX++;
                        nSumRowG += ( nWeightX *( *pTmpX )) ;pTmpX++;
                        nSumRowB += ( nWeightX *( *pTmpX )) ;pTmpX++;
                        nTotalWeightX += nWeightX;
                    }
                    else if ( nRowRange == j )
                    {
                        long nWeightX = rCtx.mpMapFX[ nRight ] ;
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
                    nWeightY = nMax - rCtx.mpMapFY[ nTop ];
                else if( nLineRange == 1 )
                    nWeightY = rCtx.mpMapFY[ nTop ];
                else if ( nLineRange == i )
                    nWeightY = rCtx.mpMapFY[ nBottom ];

                if (nTotalWeightX)
                {
                    nSumRowB /= nTotalWeightX;
                    nSumRowG /= nTotalWeightX;
                    nSumRowR /= nTotalWeightX;
                }
                nSumB += nWeightY * nSumRowB;
                nSumG += nWeightY * nSumRowG;
                nSumR += nWeightY * nSumRowR;
                nTotalWeightY += nWeightY;
            }

            if (nTotalWeightY)
            {
                nSumR /= nTotalWeightY;
                nSumG /= nTotalWeightY;
                nSumB /= nTotalWeightY;
            }
            BitmapColor aColRes((sal_uInt8)nSumR, (sal_uInt8)nSumG, (sal_uInt8)nSumB);
            rCtx.mpDest->SetPixel( nY, nXDst++, aColRes );
        }
    }
}

void scaleNonPalleteGeneral2(ScaleContext &rCtx, long nStartY, long nEndY)
{
    const long nStartX = 0, nEndX = rCtx.mnDestW - 1L;
    const long nMax = 1 << 7L;

    for( long nY = nStartY; nY <= nEndY; nY++ )
    {
        long nTop = rCtx.mbVMirr ? ( nY + 1 ) : nY;
        long nBottom = rCtx.mbVMirr ? nY : ( nY + 1 ) ;

        long nLineStart, nLineRange;
        if( nY ==nEndY )
        {
            nLineStart = rCtx.mpMapIY[ nY ];
            nLineRange = 0;
        }
        else
        {
            nLineStart = rCtx.mpMapIY[ nTop ] ;
            nLineRange = ( rCtx.mpMapIY[ nBottom ] == rCtx.mpMapIY[ nTop ] ) ? 1 :( rCtx.mpMapIY[ nBottom ] - rCtx.mpMapIY[ nTop ] );
        }

        for( long nX = nStartX , nXDst = 0L; nX <= nEndX; nX++ )
        {
            long nLeft = rCtx.mbHMirr ? ( nX + 1 ) : nX;
            long nRight = rCtx.mbHMirr ? nX : ( nX + 1 ) ;

            long nRowStart, nRowRange;
            if( nX == nEndX )
            {
                nRowStart = rCtx.mpMapIX[ nX ];
                nRowRange = 0;
            }
            else
            {
                nRowStart = rCtx.mpMapIX[ nLeft ];
                nRowRange = ( rCtx.mpMapIX[ nRight ] == rCtx.mpMapIX[ nLeft ] )? 1 : ( rCtx.mpMapIX[ nRight ] - rCtx.mpMapIX[ nLeft ] );
            }

            long nSumR = 0;
            long nSumG = 0;
            long nSumB = 0;
            long nTotalWeightY = 0;

            for(long i = 0; i<= nLineRange; i++)
            {
                long nSumRowR = 0;
                long nSumRowG = 0;
                long nSumRowB = 0;
                long nTotalWeightX = 0;

                for(long j = 0; j <= nRowRange; j++)
                {
                    BitmapColor aCol0 = rCtx.mpSrc->GetPixel( nLineStart + i, nRowStart + j );

                    if(nX == nEndX )
                    {

                        nSumRowB += aCol0.GetBlue() << 7L;
                        nSumRowG += aCol0.GetGreen() << 7L;
                        nSumRowR += aCol0.GetRed() << 7L;
                        nTotalWeightX += 1 << 7L;
                    }
                    else if( j == 0 )
                    {

                        long nWeightX = (nMax- rCtx.mpMapFX[ nLeft ]) ;
                        nSumRowB += ( nWeightX *aCol0.GetBlue()) ;
                        nSumRowG += ( nWeightX *aCol0.GetGreen()) ;
                        nSumRowR += ( nWeightX *aCol0.GetRed()) ;
                        nTotalWeightX += nWeightX;
                    }
                    else if ( nRowRange == j )
                    {

                        long nWeightX = rCtx.mpMapFX[ nRight ] ;
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
                    nWeightY = nMax - rCtx.mpMapFY[ nTop ];
                else if( nLineRange == 1 )
                    nWeightY = rCtx.mpMapFY[ nTop ];
                else if ( nLineRange == i )
                    nWeightY = rCtx.mpMapFY[ nBottom ];

                if (nTotalWeightX)
                {
                    nSumRowB /= nTotalWeightX;
                    nSumRowG /= nTotalWeightX;
                    nSumRowR /= nTotalWeightX;
                }

                nSumB += nWeightY * nSumRowB;
                nSumG += nWeightY * nSumRowG;
                nSumR += nWeightY * nSumRowR;
                nTotalWeightY += nWeightY;
            }

            if (nTotalWeightY)
            {
                nSumR /= nTotalWeightY;
                nSumG /= nTotalWeightY;
                nSumB /= nTotalWeightY;
            }

            BitmapColor aColRes((sal_uInt8)nSumR, (sal_uInt8)nSumG, (sal_uInt8)nSumB);
            rCtx.mpDest->SetPixel( nY, nXDst++, aColRes );
        }
    }
}

} // end anonymous namespace

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

    const long nStartY = 0;
    const long nEndY   = nDstH - 1L;

    if (pReadAccess && pWriteAccess)
    {
        ScaleRangeFn pScaleRangeFn;
        ScaleContext aContext( pReadAccess.get(),
                               pWriteAccess.get(),
                               pReadAccess->Width(),
                               pWriteAccess->Width(),
                               pReadAccess->Height(),
                               pWriteAccess->Height(),
                               bVMirr, bHMirr );

        bool bScaleUp = fScaleX >= fScaleThresh && fScaleY >= fScaleThresh;
        if( pReadAccess->HasPalette() )
        {
            switch( pReadAccess->GetScanlineFormat() )
            {
            case BMP_FORMAT_8BIT_PAL:
                pScaleRangeFn = bScaleUp ? scalePallete8bit : scalePallete8bit2;
                break;
            default:
                pScaleRangeFn = bScaleUp ? scalePalleteGeneral
                                        : scalePalleteGeneral2;
                break;
            }
        }
        else
        {
            switch( pReadAccess->GetScanlineFormat() )
            {
            case BMP_FORMAT_24BIT_TC_BGR:
                pScaleRangeFn = bScaleUp ? scale24bitBGR : scale24bitBGR2;
                break;
            case BMP_FORMAT_24BIT_TC_RGB:
                pScaleRangeFn = bScaleUp ? scale24bitRGB : scale24bitRGB2;
                break;
            default:
                pScaleRangeFn = bScaleUp ? scaleNonPalleteGeneral
                                        : scaleNonPalleteGeneral2;
                break;
            }
        }

        // We want to thread - only if there is a lot of work to do:
        // We work hard when there is a large destination image, or
        // A large source image.
        bool bHorizontalWork = pReadAccess->Width() > 512 || pWriteAccess->Width() > 512;

        static bool bDisableThreadedScaling = getenv ("VCL_NO_THREAD_SCALE");
        if ( bDisableThreadedScaling || !bHorizontalWork ||
             nEndY - nStartY < SCALE_THREAD_STRIP )
        {
            SAL_INFO("vcl.gdi", "Scale in main thread");
            pScaleRangeFn( aContext, nStartY, nEndY );
        }
        else
        {
            // partition and queue work
            comphelper::ThreadPool &rShared = comphelper::ThreadPool::getSharedOptimalPool();
            sal_uInt32 nThreads = rShared.getWorkerCount();
            assert( nThreads > 0 );
            sal_uInt32 nStrips = ((nEndY - nStartY) + SCALE_THREAD_STRIP - 1) / SCALE_THREAD_STRIP;
            sal_uInt32 nStripsPerThread = nStrips / nThreads;
            SAL_INFO("vcl.gdi", "Scale in " << nStrips << " strips " << nStripsPerThread << " per thread we have " << nThreads << " CPU threads ");
            long nStripY = nStartY;
            for ( sal_uInt32 t = 0; t < nThreads - 1; t++ )
            {
                ScaleTask *pTask = new ScaleTask( pScaleRangeFn );
                for ( sal_uInt32 j = 0; j < nStripsPerThread; j++ )
                {
                    ScaleRangeContext aRC( &aContext, nStripY );
                    pTask->push( aRC );
                    nStripY += SCALE_THREAD_STRIP;
                }
                rShared.pushTask( pTask );
            }
            // finish any remaining bits here
            pScaleRangeFn( aContext, nStripY, nEndY );

            rShared.waitUntilEmpty();
            SAL_INFO("vcl.gdi", "All threaded scaling tasks complete");
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
