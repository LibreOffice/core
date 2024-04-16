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

#include <comphelper/profilezone.hxx>
#include <comphelper/threadpool.hxx>
#include <tools/helpers.hxx>

#include <vcl/BitmapWriteAccess.hxx>
#include <bitmap/BitmapScaleSuperFilter.hxx>

#include <algorithm>
#include <memory>
#include <svdata.hxx>
#include <sal/log.hxx>

/*
A scaling algorithm that uses bilinear if not downscaling too much,
and averaging otherwise (bilinear would produce poor results for big downscaling).

By default the combination of two filters is used: bilinear and averaging algorithm. 
Bilinear filtering is used for bitmap enlarging and shrinking till factor 0.6. Below
this bilinear gives bad results because of limited sampling. For such cases averaging 
is used which is a simple algorithm for shrinking. In averaging the algorithm 
calculates the average of samples which result is the new pixel.
*/

namespace {

constexpr int MAP_PRECISION = 7;

typedef sal_Int32 BilinearWeightType;

constexpr BilinearWeightType lclMaxWeight()
{
    return BilinearWeightType(1) << MAP_PRECISION;
}

constexpr sal_uInt8 MAP(sal_uInt8 cVal0, sal_uInt8 cVal1, BilinearWeightType nFrac)
{
    return sal_uInt8(((BilinearWeightType(cVal0) << MAP_PRECISION) + nFrac * (BilinearWeightType(cVal1) - BilinearWeightType(cVal0))) >> MAP_PRECISION);
}

struct ScaleContext
{
    BitmapReadAccess*  mpSrc;
    BitmapWriteAccess* mpDest;
    sal_Int32 mnDestW;
    bool mbHMirr;
    bool mbVMirr;
    std::vector<sal_Int32> maMapIX;
    std::vector<sal_Int32> maMapIY;
    std::vector<BilinearWeightType> maMapFX;
    std::vector<BilinearWeightType> maMapFY;

    ScaleContext( BitmapReadAccess *pSrc,
                  BitmapWriteAccess *pDest,
                  sal_Int32 nSrcW, sal_Int32 nDestW,
                  sal_Int32 nSrcH, sal_Int32 nDestH,
                  bool bHMirr, bool bVMirr)
        : mpSrc(pSrc)
        , mpDest(pDest)
        , mnDestW(nDestW)
        , mbHMirr(bHMirr)
        , mbVMirr(bVMirr)
        , maMapIX(nDestW)
        , maMapIY(nDestH)
        , maMapFX(nDestW)
        , maMapFY(nDestH)
    {
        generateMap(nSrcW, nDestW, bHMirr, maMapIX, maMapFX);
        generateMap(nSrcH, nDestH, bVMirr, maMapIY, maMapFY);
    }

    static void generateMap(sal_Int32 nSourceLength, sal_Int32 nDestinationLength, bool bMirrored,
        std::vector<sal_Int32> & rMapIX, std::vector<BilinearWeightType> & rMapFX)
    {
        const double fRevScale = (nDestinationLength > 1) ? double(nSourceLength - 1) / (nDestinationLength - 1) : 0.0;

        sal_Int32 nTemp = nSourceLength - 2;
        sal_Int32 nTempX = nSourceLength - 1;

        for (sal_Int32 i = 0; i < nDestinationLength; i++)
        {
            double fTemp = i * fRevScale;
            if (bMirrored)
                fTemp = nTempX - fTemp;
            rMapIX[i] = std::clamp(sal_Int32(fTemp), sal_Int32(0), nTemp);
            rMapFX[i] = BilinearWeightType((fTemp - rMapIX[i]) * (BilinearWeightType(1) << MAP_PRECISION));
        }
    }
};

constexpr sal_Int32 constScaleThreadStrip = 32;

typedef void (*ScaleRangeFn)(const ScaleContext & rContext, sal_Int32 nStartY, sal_Int32 nEndY);

template <size_t nSize> struct ScaleFunc
{
    // for scale down

    static inline void generateSumRows(Scanline& pTmpX, std::array<int, nSize>& sumRows)
    {
        for (int& n : sumRows)
            n += (*pTmpX++) << MAP_PRECISION;
    }

    static inline void generateSumRows(BilinearWeightType const nWeightX, Scanline& pTmpX,
                                       std::array<int, nSize>& sumRows)
    {
        for (int& n : sumRows)
            n += (nWeightX * (*pTmpX++));
    }

    static inline void generateSumRows(BilinearWeightType const nTotalWeightX,
                                       std::array<int, nSize>& sumRows)
    {
        for (int& n : sumRows)
            n /= nTotalWeightX;
    }

    static inline void generateSumNumbers(BilinearWeightType const nWeightY,
                                          std::array<int, nSize>& sumRows,
                                          std::array<int, nSize>& sumNumbers)
    {
        std::transform(sumRows.begin(), sumRows.end(), sumNumbers.begin(), sumNumbers.begin(),
                       [nWeightY](int n1, int n2) { return nWeightY * n1 + n2; });
    }

    static inline void generateSumNumbers(BilinearWeightType const nTotalWeightY,
                                          std::array<int, nSize>& sumNumbers)
    {
        for (int& n : sumNumbers)
            n /= nTotalWeightY;
    }

    static inline void calculateDestination(Scanline& pScanDest, std::array<int, nSize>& sumNumbers)
    {
        pScanDest = std::copy(sumNumbers.begin(), sumNumbers.end(), pScanDest);
    }

    // for scale up

    static inline void generateComponent(Scanline pColorPtr0, Scanline pColorPtr1,
                                         BilinearWeightType const nTempFX,
                                         std::array<sal_uInt8, nSize>& nComponents)
    {
        for (sal_uInt8& rComponent : nComponents)
            rComponent = MAP(*pColorPtr0++, *pColorPtr1++, nTempFX);
    }

    static inline void calculateDestination(Scanline& pScanDest, BilinearWeightType const nTempFY,
                                            const std::array<sal_uInt8, nSize>& nComponents1,
                                            const std::array<sal_uInt8, nSize>& nComponents2)
    {
        pScanDest = std::transform(
            nComponents1.begin(), nComponents1.end(), nComponents2.begin(), pScanDest,
            [nTempFY](sal_uInt8 c1, sal_uInt8 c2) { return MAP(c1, c2, nTempFY); });
    }
};

template <int nColorBits>
void scaleDown (const ScaleContext &rCtx, sal_Int32 nStartY, sal_Int32 nEndY)
{
    comphelper::ProfileZone pz("BitmapScaleSuperFilter::scaleDown");
    constexpr int nColorComponents = nColorBits / 8;
    static_assert(nColorComponents * 8 == nColorBits, "nColorBits must be divisible by 8");
    using ScaleFunction = ScaleFunc<nColorComponents>;
    const sal_Int32 nStartX = 0;
    const sal_Int32 nEndX = rCtx.mnDestW - 1;

    for (sal_Int32 nY = nStartY; nY <= nEndY; nY++)
    {
        sal_Int32 nTop = rCtx.mbVMirr ? (nY + 1) : nY;
        sal_Int32 nBottom = rCtx.mbVMirr ? nY : (nY + 1);

        sal_Int32 nLineStart;
        sal_Int32 nLineRange;
        if (nY == nEndY)
        {
            nLineStart = rCtx.maMapIY[nY];
            nLineRange = 0;
        }
        else
        {
            nLineStart = rCtx.maMapIY[nTop];
            nLineRange = (rCtx.maMapIY[nBottom] == rCtx.maMapIY[nTop]) ?
                            1 : (rCtx.maMapIY[nBottom] - rCtx.maMapIY[nTop]);
        }

        Scanline pScanDest = rCtx.mpDest->GetScanline(nY);
        for (sal_Int32 nX = nStartX; nX <= nEndX; nX++)
        {
            sal_Int32 nLeft = rCtx.mbHMirr ? (nX + 1) : nX;
            sal_Int32 nRight = rCtx.mbHMirr ? nX : (nX + 1);

            sal_Int32 nRowStart;
            sal_Int32 nRowRange;
            if (nX == nEndX)
            {
                nRowStart = rCtx.maMapIX[nX];
                nRowRange = 0;
            }
            else
            {
                nRowStart = rCtx.maMapIX[nLeft];
                nRowRange = (rCtx.maMapIX[nRight] == rCtx.maMapIX[nLeft]) ?
                                1 : (rCtx.maMapIX[nRight] - rCtx.maMapIX[nLeft]);
            }

            std::array<int, nColorComponents> sumNumbers{}; // zero-initialize
            BilinearWeightType nTotalWeightY = 0;

            for (sal_Int32 i = 0; i<= nLineRange; i++)
            {
                Scanline pTmpY = rCtx.mpSrc->GetScanline(nLineStart + i);
                Scanline pTmpX = pTmpY + nColorComponents * nRowStart;

                std::array<int, nColorComponents> sumRows{}; // zero-initialize
                BilinearWeightType nTotalWeightX = 0;

                for (sal_Int32 j = 0; j <= nRowRange; j++)
                {
                    if (nX == nEndX)
                    {
                        ScaleFunction::generateSumRows(pTmpX, sumRows);
                        nTotalWeightX += lclMaxWeight();
                    }
                    else if(j == 0)
                    {
                        BilinearWeightType nWeightX = lclMaxWeight() - rCtx.maMapFX[nLeft];
                        ScaleFunction::generateSumRows(nWeightX, pTmpX, sumRows);
                        nTotalWeightX += nWeightX;
                    }
                    else if ( nRowRange == j )
                    {
                        BilinearWeightType nWeightX = rCtx.maMapFX[ nRight ] ;
                        ScaleFunction::generateSumRows(nWeightX, pTmpX, sumRows);
                        nTotalWeightX += nWeightX;
                    }
                    else
                    {
                       ScaleFunction::generateSumRows(pTmpX, sumRows);
                       nTotalWeightX += lclMaxWeight();
                    }
                }

                BilinearWeightType nWeightY = lclMaxWeight();
                if (nY == nEndY)
                    nWeightY = lclMaxWeight();
                else if (i == 0)
                    nWeightY = lclMaxWeight() - rCtx.maMapFY[nTop];
                else if (nLineRange == 1)
                    nWeightY = rCtx.maMapFY[nTop];
                else if (nLineRange == i)
                    nWeightY = rCtx.maMapFY[nBottom];

                if (nTotalWeightX)
                {
                  ScaleFunction::generateSumRows(nTotalWeightX, sumRows);
                }
                ScaleFunction::generateSumNumbers(nWeightY, sumRows, sumNumbers);
                nTotalWeightY += nWeightY;

            }

            if (nTotalWeightY)
            {
                ScaleFunction::generateSumNumbers(nTotalWeightY, sumNumbers);
            }

            // Write the calculated color components to the destination
            ScaleFunction::calculateDestination(pScanDest, sumNumbers);
        }
    }
}

template <int nColorBits>
void scaleUp(const ScaleContext &rCtx, sal_Int32 nStartY, sal_Int32 nEndY)
{
    comphelper::ProfileZone pz("BitmapScaleSuperFilter::scaleUp");
    constexpr int nColorComponents = nColorBits / 8;
    static_assert(nColorComponents * 8 == nColorBits, "nColorBits must be divisible by 8");
    using ScaleFunction = ScaleFunc<nColorComponents>;
    const sal_Int32 nStartX = 0;
    const sal_Int32 nEndX = rCtx.mnDestW - 1;

    for (sal_Int32 nY = nStartY; nY <= nEndY; nY++)
    {
        sal_Int32 nTempY = rCtx.maMapIY[nY];
        BilinearWeightType nTempFY = rCtx.maMapFY[nY];

        Scanline pLine0 = rCtx.mpSrc->GetScanline(nTempY+0);
        Scanline pLine1 = rCtx.mpSrc->GetScanline(nTempY+1);
        Scanline pScanDest = rCtx.mpDest->GetScanline(nY);

        std::array<sal_uInt8, nColorComponents> nComponents1; // no need to initialize since it's
        std::array<sal_uInt8, nColorComponents> nComponents2; // initialized in generateComponent

        Scanline pColorPtr0;
        Scanline pColorPtr1;

        for (sal_Int32 nX = nStartX; nX <= nEndX; nX++)
        {
            sal_Int32 nTempX = rCtx.maMapIX[nX];
            BilinearWeightType nTempFX = rCtx.maMapFX[nX];

            pColorPtr0 = pLine0 + nTempX * nColorComponents;
            pColorPtr1 = pColorPtr0 + nColorComponents;

            ScaleFunction::generateComponent(pColorPtr0, pColorPtr1, nTempFX, nComponents1);

            pColorPtr0 = pLine1 + nTempX * nColorComponents;
            pColorPtr1 = pColorPtr0 + nColorComponents;

            ScaleFunction::generateComponent(pColorPtr0, pColorPtr1, nTempFX, nComponents2);
            ScaleFunction::calculateDestination(pScanDest, nTempFY, nComponents1, nComponents2);
        }
    }
}

class ScaleTask : public comphelper::ThreadTask
{
    ScaleRangeFn mpScaleRangeFunction;
    const ScaleContext& mrContext;
    sal_Int32 mnStartY;
    sal_Int32 mnEndY;

public:
    explicit ScaleTask(const std::shared_ptr<comphelper::ThreadTaskTag>& pTag,
                       ScaleRangeFn pScaleRangeFunction,
                       const ScaleContext& rContext,
                       sal_Int32 nStartY, sal_Int32 nEndY)
        : comphelper::ThreadTask(pTag)
        , mpScaleRangeFunction(pScaleRangeFunction)
        , mrContext(rContext)
        , mnStartY(nStartY)
        , mnEndY(nEndY)
    {}

    virtual void doWork() override
    {
        mpScaleRangeFunction(mrContext, mnStartY, mnEndY);
    }
};

void scaleUpPalette8bit(const ScaleContext &rCtx, sal_Int32 nStartY, sal_Int32 nEndY)
{
    const sal_Int32 nStartX = 0, nEndX = rCtx.mnDestW - 1;

    for( sal_Int32 nY = nStartY; nY <= nEndY; nY++ )
    {
        sal_Int32 nTempY = rCtx.maMapIY[ nY ];
        BilinearWeightType nTempFY = rCtx.maMapFY[ nY ];
        Scanline pLine0 = rCtx.mpSrc->GetScanline( nTempY );
        Scanline pLine1 = rCtx.mpSrc->GetScanline( ++nTempY );
        Scanline pScanDest = rCtx.mpDest->GetScanline( nY );

        for(sal_Int32 nX = nStartX, nXDst = 0; nX <= nEndX; nX++ )
        {
            sal_Int32 nTempX = rCtx.maMapIX[ nX ];
            BilinearWeightType nTempFX = rCtx.maMapFX[ nX ];

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
            rCtx.mpDest->SetPixelOnData( pScanDest, nXDst++, aColRes );
        }
    }
}

void scaleUpPaletteGeneral(const ScaleContext &rCtx, sal_Int32 nStartY, sal_Int32 nEndY)
{
    const sal_Int32 nStartX = 0, nEndX = rCtx.mnDestW - 1;

    for( sal_Int32 nY = nStartY; nY <= nEndY; nY++ )
    {
        sal_Int32 nTempY = rCtx.maMapIY[ nY ];
        BilinearWeightType nTempFY = rCtx.maMapFY[ nY ];
        Scanline pScanline = rCtx.mpDest->GetScanline( nY );

        for( sal_Int32 nX = nStartX, nXDst = 0; nX <= nEndX; nX++ )
        {
            sal_Int32 nTempX = rCtx.maMapIX[ nX ];
            BilinearWeightType nTempFX = rCtx.maMapFX[ nX ];

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
            rCtx.mpDest->SetPixelOnData( pScanline, nXDst++, aColRes );
        }
    }
}

void scaleUpNonPaletteGeneral(const ScaleContext &rCtx, sal_Int32 nStartY, sal_Int32 nEndY)
{
    const sal_Int32 nStartX = 0, nEndX = rCtx.mnDestW - 1;

    for( sal_Int32 nY = nStartY; nY <= nEndY; nY++ )
    {
        sal_Int32 nTempY = rCtx.maMapIY[ nY ];
        BilinearWeightType nTempFY = rCtx.maMapFY[ nY ];
        Scanline pScanDest = rCtx.mpDest->GetScanline( nY );

        for( sal_Int32 nX = nStartX, nXDst = 0; nX <= nEndX; nX++ )
        {
            sal_Int32 nTempX = rCtx.maMapIX[ nX ];
            BilinearWeightType nTempFX = rCtx.maMapFX[ nX ];

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
            rCtx.mpDest->SetPixelOnData( pScanDest, nXDst++, aColRes );
        }
    }
}

void scaleDownPalette8bit(const ScaleContext &rCtx, sal_Int32 nStartY, sal_Int32 nEndY)
{
    const sal_Int32 nStartX = 0, nEndX = rCtx.mnDestW - 1;

    for( sal_Int32 nY = nStartY; nY <= nEndY; nY++ )
    {
        sal_Int32 nTop = rCtx.mbVMirr ? ( nY + 1 ) : nY;
        sal_Int32 nBottom = rCtx.mbVMirr ? nY : ( nY + 1 ) ;

        sal_Int32 nLineStart, nLineRange;
        if( nY == nEndY )
        {
            nLineStart = rCtx.maMapIY[ nY ];
            nLineRange = 0;
        }
        else
        {
            nLineStart = rCtx.maMapIY[ nTop ] ;
            nLineRange = ( rCtx.maMapIY[ nBottom ] == rCtx.maMapIY[ nTop ] ) ? 1 :( rCtx.maMapIY[ nBottom ] - rCtx.maMapIY[ nTop ] );
        }

        Scanline pScanDest = rCtx.mpDest->GetScanline( nY );
        for( sal_Int32 nX = nStartX , nXDst = 0; nX <= nEndX; nX++ )
        {
            sal_Int32 nLeft = rCtx.mbHMirr ? ( nX + 1 ) : nX;
            sal_Int32 nRight = rCtx.mbHMirr ? nX : ( nX + 1 ) ;

            sal_Int32 nRowStart;
            sal_Int32 nRowRange;
            if( nX == nEndX )
            {
                nRowStart = rCtx.maMapIX[ nX ];
                nRowRange = 0;
            }
            else
            {
                nRowStart = rCtx.maMapIX[ nLeft ];
                nRowRange = ( rCtx.maMapIX[ nRight ] == rCtx.maMapIX[ nLeft ] )? 1 : ( rCtx.maMapIX[ nRight ] - rCtx.maMapIX[ nLeft ] );
            }

            int nSumR = 0;
            int nSumG = 0;
            int nSumB = 0;
            BilinearWeightType nTotalWeightY = 0;

            for(sal_Int32 i = 0; i<= nLineRange; i++)
            {
                Scanline pTmpY = rCtx.mpSrc->GetScanline( nLineStart + i );
                int nSumRowR = 0;
                int nSumRowG = 0;
                int nSumRowB = 0;
                BilinearWeightType nTotalWeightX = 0;

                for(sal_Int32 j = 0; j <= nRowRange; j++)
                {
                    const BitmapColor& rCol = rCtx.mpSrc->GetPaletteColor( pTmpY[ nRowStart + j ] );

                    if(nX == nEndX )
                    {
                        nSumRowB += rCol.GetBlue() << MAP_PRECISION;
                        nSumRowG += rCol.GetGreen() << MAP_PRECISION;
                        nSumRowR += rCol.GetRed() << MAP_PRECISION;
                        nTotalWeightX += lclMaxWeight();
                    }
                    else if( j == 0 )
                    {
                        BilinearWeightType nWeightX = lclMaxWeight() - rCtx.maMapFX[ nLeft ];
                        nSumRowB += ( nWeightX *rCol.GetBlue()) ;
                        nSumRowG += ( nWeightX *rCol.GetGreen()) ;
                        nSumRowR += ( nWeightX *rCol.GetRed()) ;
                        nTotalWeightX += nWeightX;
                    }
                    else if ( nRowRange == j )
                    {
                        BilinearWeightType nWeightX = rCtx.maMapFX[ nRight ] ;
                        nSumRowB += ( nWeightX *rCol.GetBlue() );
                        nSumRowG += ( nWeightX *rCol.GetGreen() );
                        nSumRowR += ( nWeightX *rCol.GetRed() );
                        nTotalWeightX += nWeightX;
                    }
                    else
                    {
                        nSumRowB += rCol.GetBlue() << MAP_PRECISION;
                        nSumRowG += rCol.GetGreen() << MAP_PRECISION;
                        nSumRowR += rCol.GetRed() << MAP_PRECISION;
                        nTotalWeightX += lclMaxWeight();
                    }
                }

                BilinearWeightType nWeightY = lclMaxWeight();
                if( nY == nEndY )
                    nWeightY = lclMaxWeight();
                else if( i == 0 )
                    nWeightY = lclMaxWeight() - rCtx.maMapFY[ nTop ];
                else if( nLineRange == 1 )
                    nWeightY = rCtx.maMapFY[ nTop ];
                else if ( nLineRange == i )
                    nWeightY = rCtx.maMapFY[ nBottom ];

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

            BitmapColor aColRes(static_cast<sal_uInt8>(nSumR), static_cast<sal_uInt8>(nSumG), static_cast<sal_uInt8>(nSumB));
            rCtx.mpDest->SetPixelOnData( pScanDest, nXDst++, aColRes );
        }
    }
}

void scaleDownPaletteGeneral(const ScaleContext &rCtx, sal_Int32 nStartY, sal_Int32 nEndY)
{
    const sal_Int32 nStartX = 0, nEndX = rCtx.mnDestW - 1;

    for( sal_Int32 nY = nStartY; nY <= nEndY; nY++ )
    {
        sal_Int32 nTop = rCtx.mbVMirr ? ( nY + 1 ) : nY;
        sal_Int32 nBottom = rCtx.mbVMirr ? nY : ( nY + 1 ) ;

        sal_Int32 nLineStart, nLineRange;
        if( nY ==nEndY )
        {
            nLineStart = rCtx.maMapIY[ nY ];
            nLineRange = 0;
        }
        else
        {
            nLineStart = rCtx.maMapIY[ nTop ] ;
            nLineRange = ( rCtx.maMapIY[ nBottom ] == rCtx.maMapIY[ nTop ] ) ? 1 :( rCtx.maMapIY[ nBottom ] - rCtx.maMapIY[ nTop ] );
        }

        Scanline pScanDest = rCtx.mpDest->GetScanline( nY );
        for( sal_Int32 nX = nStartX , nXDst = 0; nX <= nEndX; nX++ )
        {
            sal_Int32 nLeft = rCtx.mbHMirr ? ( nX + 1 ) : nX;
            sal_Int32 nRight = rCtx.mbHMirr ? nX : ( nX + 1 ) ;

            sal_Int32 nRowStart, nRowRange;
            if( nX == nEndX )
            {
                nRowStart = rCtx.maMapIX[ nX ];
                nRowRange = 0;
            }
            else
            {
                nRowStart = rCtx.maMapIX[ nLeft ];
                nRowRange = ( rCtx.maMapIX[ nRight ] == rCtx.maMapIX[ nLeft ] )? 1 : ( rCtx.maMapIX[ nRight ] - rCtx.maMapIX[ nLeft ] );
            }

            int nSumR = 0;
            int nSumG = 0;
            int nSumB = 0;
            BilinearWeightType nTotalWeightY = 0;

            for(sal_Int32 i = 0; i<= nLineRange; i++)
            {
                int nSumRowR = 0;
                int nSumRowG = 0;
                int nSumRowB = 0;
                BilinearWeightType nTotalWeightX = 0;

                Scanline pScanlineSrc = rCtx.mpSrc->GetScanline( nLineStart + i );
                for(sal_Int32 j = 0; j <= nRowRange; j++)
                {
                    BitmapColor aCol0 = rCtx.mpSrc->GetPaletteColor ( rCtx.mpSrc->GetIndexFromData( pScanlineSrc, nRowStart + j ) );

                    if(nX == nEndX )
                    {

                        nSumRowB += aCol0.GetBlue() << MAP_PRECISION;
                        nSumRowG += aCol0.GetGreen() << MAP_PRECISION;
                        nSumRowR += aCol0.GetRed() << MAP_PRECISION;
                        nTotalWeightX += lclMaxWeight();
                    }
                    else if( j == 0 )
                    {

                        BilinearWeightType nWeightX = lclMaxWeight() - rCtx.maMapFX[ nLeft ];
                        nSumRowB += ( nWeightX *aCol0.GetBlue()) ;
                        nSumRowG += ( nWeightX *aCol0.GetGreen()) ;
                        nSumRowR += ( nWeightX *aCol0.GetRed()) ;
                        nTotalWeightX += nWeightX;
                    }
                    else if ( nRowRange == j )
                    {

                        BilinearWeightType nWeightX = rCtx.maMapFX[ nRight ] ;
                        nSumRowB += ( nWeightX *aCol0.GetBlue() );
                        nSumRowG += ( nWeightX *aCol0.GetGreen() );
                        nSumRowR += ( nWeightX *aCol0.GetRed() );
                        nTotalWeightX += nWeightX;
                    }
                    else
                    {

                        nSumRowB += aCol0.GetBlue() << MAP_PRECISION;
                        nSumRowG += aCol0.GetGreen() << MAP_PRECISION;
                        nSumRowR += aCol0.GetRed() << MAP_PRECISION;
                        nTotalWeightX += lclMaxWeight();
                    }
                }

                sal_Int32 nWeightY = lclMaxWeight();
                if( nY == nEndY )
                    nWeightY = lclMaxWeight();
                else if( i == 0 )
                    nWeightY = lclMaxWeight() - rCtx.maMapFY[ nTop ];
                else if( nLineRange == 1 )
                    nWeightY = rCtx.maMapFY[ nTop ];
                else if ( nLineRange == i )
                    nWeightY = rCtx.maMapFY[ nBottom ];

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

            BitmapColor aColRes(static_cast<sal_uInt8>(nSumR), static_cast<sal_uInt8>(nSumG), static_cast<sal_uInt8>(nSumB));
            rCtx.mpDest->SetPixelOnData( pScanDest, nXDst++, aColRes );
        }
    }
}

void scaleDownNonPaletteGeneral(const ScaleContext &rCtx, sal_Int32 nStartY, sal_Int32 nEndY)
{
    const sal_Int32 nStartX = 0, nEndX = rCtx.mnDestW - 1;

    for( sal_Int32 nY = nStartY; nY <= nEndY; nY++ )
    {
        sal_Int32 nTop = rCtx.mbVMirr ? ( nY + 1 ) : nY;
        sal_Int32 nBottom = rCtx.mbVMirr ? nY : ( nY + 1 ) ;

        sal_Int32 nLineStart, nLineRange;
        if( nY ==nEndY )
        {
            nLineStart = rCtx.maMapIY[ nY ];
            nLineRange = 0;
        }
        else
        {
            nLineStart = rCtx.maMapIY[ nTop ] ;
            nLineRange = ( rCtx.maMapIY[ nBottom ] == rCtx.maMapIY[ nTop ] ) ? 1 :( rCtx.maMapIY[ nBottom ] - rCtx.maMapIY[ nTop ] );
        }

        Scanline pScanDest = rCtx.mpDest->GetScanline( nY );
        for( sal_Int32 nX = nStartX , nXDst = 0; nX <= nEndX; nX++ )
        {
            sal_Int32 nLeft = rCtx.mbHMirr ? ( nX + 1 ) : nX;
            sal_Int32 nRight = rCtx.mbHMirr ? nX : ( nX + 1 ) ;

            sal_Int32 nRowStart, nRowRange;
            if( nX == nEndX )
            {
                nRowStart = rCtx.maMapIX[ nX ];
                nRowRange = 0;
            }
            else
            {
                nRowStart = rCtx.maMapIX[ nLeft ];
                nRowRange = ( rCtx.maMapIX[ nRight ] == rCtx.maMapIX[ nLeft ] )? 1 : ( rCtx.maMapIX[ nRight ] - rCtx.maMapIX[ nLeft ] );
            }

            int nSumR = 0;
            int nSumG = 0;
            int nSumB = 0;
            BilinearWeightType nTotalWeightY = 0;

            for(sal_Int32 i = 0; i<= nLineRange; i++)
            {
                int nSumRowR = 0;
                int nSumRowG = 0;
                int nSumRowB = 0;
                BilinearWeightType nTotalWeightX = 0;

                Scanline pScanlineSrc = rCtx.mpSrc->GetScanline( nLineStart + i );
                for(sal_Int32 j = 0; j <= nRowRange; j++)
                {
                    BitmapColor aCol0 = rCtx.mpSrc->GetPixelFromData( pScanlineSrc, nRowStart + j );

                    if(nX == nEndX )
                    {

                        nSumRowB += aCol0.GetBlue() << MAP_PRECISION;
                        nSumRowG += aCol0.GetGreen() << MAP_PRECISION;
                        nSumRowR += aCol0.GetRed() << MAP_PRECISION;
                        nTotalWeightX += lclMaxWeight();
                    }
                    else if( j == 0 )
                    {

                        BilinearWeightType nWeightX = lclMaxWeight() - rCtx.maMapFX[ nLeft ];
                        nSumRowB += ( nWeightX *aCol0.GetBlue()) ;
                        nSumRowG += ( nWeightX *aCol0.GetGreen()) ;
                        nSumRowR += ( nWeightX *aCol0.GetRed()) ;
                        nTotalWeightX += nWeightX;
                    }
                    else if ( nRowRange == j )
                    {

                        BilinearWeightType nWeightX = rCtx.maMapFX[ nRight ] ;
                        nSumRowB += ( nWeightX *aCol0.GetBlue() );
                        nSumRowG += ( nWeightX *aCol0.GetGreen() );
                        nSumRowR += ( nWeightX *aCol0.GetRed() );
                        nTotalWeightX += nWeightX;
                    }
                    else
                    {
                        nSumRowB += aCol0.GetBlue() << MAP_PRECISION;
                        nSumRowG += aCol0.GetGreen() << MAP_PRECISION;
                        nSumRowR += aCol0.GetRed() << MAP_PRECISION;
                        nTotalWeightX += lclMaxWeight();
                    }
                }

                BilinearWeightType nWeightY = lclMaxWeight();
                if( nY == nEndY )
                    nWeightY = lclMaxWeight();
                else if( i == 0 )
                    nWeightY = lclMaxWeight() - rCtx.maMapFY[ nTop ];
                else if( nLineRange == 1 )
                    nWeightY = rCtx.maMapFY[ nTop ];
                else if ( nLineRange == i )
                    nWeightY = rCtx.maMapFY[ nBottom ];

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

            BitmapColor aColRes(static_cast<sal_uInt8>(nSumR), static_cast<sal_uInt8>(nSumG), static_cast<sal_uInt8>(nSumB));
            rCtx.mpDest->SetPixelOnData( pScanDest, nXDst++, aColRes );
        }
    }
}

} // end anonymous namespace

BitmapScaleSuperFilter::BitmapScaleSuperFilter(const double& rScaleX, const double& rScaleY) :
    mrScaleX(rScaleX),
    mrScaleY(rScaleY)
{}

BitmapScaleSuperFilter::~BitmapScaleSuperFilter()
{}

BitmapEx BitmapScaleSuperFilter::execute(BitmapEx const& rBitmap) const
{
    Bitmap aBitmap(rBitmap.GetBitmap());
    bool bRet = false;

    const Size aSizePix(rBitmap.GetSizePixel());

    bool bHMirr = mrScaleX < 0;
    bool bVMirr = mrScaleY < 0;

    double fScaleX = std::fabs(mrScaleX);
    double fScaleY = std::fabs(mrScaleY);

    const sal_Int32 nDstW = basegfx::fround(aSizePix.Width() * fScaleX);
    const sal_Int32 nDstH = basegfx::fround(aSizePix.Height() * fScaleY);

    constexpr double fScaleThresh = 0.6;

    if (nDstW <= 1 || nDstH <= 1)
        return BitmapEx();

    // check cache for a previously scaled version of this
    ScaleCacheKey aKey(aBitmap.ImplGetSalBitmap().get(),
                       Size(nDstW, nDstH));

    ImplSVData* pSVData = ImplGetSVData();
    auto& rCache = pSVData->maGDIData.maScaleCache;
    auto aFind = rCache.find(aKey);
    if (aFind != rCache.end())
    {
        if (aFind->second.GetSizePixel().Width() == nDstW && aFind->second.GetSizePixel().Height() == nDstH)
            return aFind->second;
        else
            SAL_WARN("vcl.gdi", "Error: size mismatch in scale cache");
    }

    {
        BitmapScopedReadAccess pReadAccess(aBitmap);

        // If source format is less than 24BPP, use 24BPP
        auto eSourcePixelFormat = aBitmap.getPixelFormat();
        auto ePixelFormat = eSourcePixelFormat;
        if (sal_uInt16(eSourcePixelFormat) < 24)
            ePixelFormat = vcl::PixelFormat::N24_BPP;

        Bitmap aOutBmp(Size(nDstW, nDstH), ePixelFormat);
        Size aOutSize = aOutBmp.GetSizePixel();
        auto eTargetPixelFormat = aOutBmp.getPixelFormat();

        if (!aOutSize.Width() || !aOutSize.Height())
        {
            SAL_WARN("vcl.gdi", "bmp creation failed");
            return BitmapEx();
        }

        BitmapScopedWriteAccess pWriteAccess(aOutBmp);

        const sal_Int32 nEndY   = nDstH - 1;

        if (pReadAccess && pWriteAccess)
        {
            ScaleRangeFn pScaleRangeFn;
            const ScaleContext aContext( pReadAccess.get(),
                                   pWriteAccess.get(),
                                   pReadAccess->Width(),
                                   pWriteAccess->Width(),
                                   pReadAccess->Height(),
                                   pWriteAccess->Height(),
                                   bVMirr, bHMirr );

            bool bScaleUp = fScaleX >= fScaleThresh && fScaleY >= fScaleThresh;
            // If we have a source bitmap with a palette the scaling converts
            // from up to 8 bit image -> 24 bit non-palette, which is then
            // adapted back to the same type as original.
            if (pReadAccess->HasPalette())
            {
                switch( pReadAccess->GetScanlineFormat() )
                {
                case ScanlineFormat::N8BitPal:
                    pScaleRangeFn = bScaleUp ? scaleUpPalette8bit
                                             : scaleDownPalette8bit;
                    break;
                default:
                    pScaleRangeFn = bScaleUp ? scaleUpPaletteGeneral
                                             : scaleDownPaletteGeneral;
                    break;
                }
            }
            // Here we know that we are dealing with a non-palette source bitmap.
            // The target is either 24 or 32 bit, depending on the image and
            // the capabilities of the backend. If for some reason the destination
            // is not the same bit-depth as the source, then we can't use
            // a fast path, so we always need to process with a general scaler.
            else if (eSourcePixelFormat != eTargetPixelFormat)
            {
                pScaleRangeFn = bScaleUp ? scaleUpNonPaletteGeneral : scaleDownNonPaletteGeneral;
            }
            // If we get here then we can only use a fast path, but let's
            // still keep the fallback to the general scaler alive.
            else
            {
                switch( pReadAccess->GetScanlineFormat() )
                {
                case ScanlineFormat::N24BitTcBgr:
                case ScanlineFormat::N24BitTcRgb:
                    pScaleRangeFn = bScaleUp ? scaleUp<24> : scaleDown<24>;
                    break;
                case ScanlineFormat::N32BitTcRgba:
                case ScanlineFormat::N32BitTcBgra:
                case ScanlineFormat::N32BitTcArgb:
                case ScanlineFormat::N32BitTcAbgr:
                    pScaleRangeFn = bScaleUp ? scaleUp<32> : scaleDown<32>;
                    break;
                default:
                    pScaleRangeFn = bScaleUp ? scaleUpNonPaletteGeneral
                                             : scaleDownNonPaletteGeneral;
                    break;
                }
            }

            // We want to thread - only if there is a lot of work to do:
            // We work hard when there is a large destination image, or
            // A large source image.
            bool bHorizontalWork = pReadAccess->Height() >= 512 && pReadAccess->Width() >= 512;
            bool bUseThreads = true;
            const sal_Int32 nStartY = 0;

            static bool bDisableThreadedScaling = getenv ("VCL_NO_THREAD_SCALE");
            if (bDisableThreadedScaling || !bHorizontalWork)
            {
                SAL_INFO("vcl.gdi", "Scale in main thread");
                bUseThreads = false;
            }

            if (bUseThreads)
            {
                try
                {
                    // partition and queue work
                    comphelper::ThreadPool &rShared = comphelper::ThreadPool::getSharedOptimalPool();
                    std::shared_ptr<comphelper::ThreadTaskTag> pTag = comphelper::ThreadPool::createThreadTaskTag();

                    vcl::bitmap::generateStripRanges<constScaleThreadStrip>(nStartY, nEndY,
                    [&] (sal_Int32 const nStart, sal_Int32 const nEnd, bool const bLast)
                    {
                        if (!bLast)
                        {
                            auto pTask(std::make_unique<ScaleTask>(pTag, pScaleRangeFn, aContext, nStart, nEnd));
                            rShared.pushTask(std::move(pTask));
                        }
                        else
                            pScaleRangeFn(aContext, nStart, nEnd);
                    });
                    rShared.waitUntilDone(pTag);
                    SAL_INFO("vcl.gdi", "All threaded scaling tasks complete");
                }
                catch (...)
                {
                    SAL_WARN("vcl.gdi", "threaded bitmap scaling failed");
                    bUseThreads = false;
                }
            }

            if (!bUseThreads)
                pScaleRangeFn( aContext, nStartY, nEndY );

            pWriteAccess.reset();
            bRet = true;
            aBitmap.AdaptBitCount(aOutBmp);
            aBitmap = aOutBmp;
        }
    }

    if (bRet)
    {
        tools::Rectangle aRect(Point(0, 0), Point(nDstW, nDstH));
        aBitmap.Crop(aRect);
        BitmapEx aRet(aBitmap);
        rCache.insert(std::make_pair(aKey, aRet));
        return aRet;
    }

    return BitmapEx();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
