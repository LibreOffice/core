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

#include <comphelper/threadpool.hxx>

#include <tools/helpers.hxx>
#include <vcl/bitmapaccess.hxx>

#include <bitmapwriteaccess.hxx>
#include <BitmapScaleSuperFilter.hxx>

#include <algorithm>
#include <memory>
#include <svdata.hxx>
#include <sal/log.hxx>

namespace {

#define MAP_PRECISION 7

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
    long mnDestW;
    bool mbHMirr;
    bool mbVMirr;
    std::vector<long> maMapIX;
    std::vector<long> maMapIY;
    std::vector<BilinearWeightType> maMapFX;
    std::vector<BilinearWeightType> maMapFY;

    ScaleContext( BitmapReadAccess *pSrc,
                  BitmapWriteAccess *pDest,
                  long nSrcW, long nDestW,
                  long nSrcH, long nDestH,
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

    static void generateMap(long nSourceLength, long nDestinationLength, bool bMirrored,
        std::vector<long> & rMapIX, std::vector<BilinearWeightType> & rMapFX)
    {
        const double fRevScale = (nDestinationLength > 1) ? double(nSourceLength - 1) / (nDestinationLength - 1) : 0.0;

        long nTemp = nSourceLength - 2;
        long nTempX = nSourceLength - 1;

        for (long i = 0; i < nDestinationLength; i++)
        {
            double fTemp = i * fRevScale;
            if (bMirrored)
                fTemp = nTempX - fTemp;
            rMapIX[i] = MinMax(long(fTemp), 0, nTemp);
            rMapFX[i] = BilinearWeightType((fTemp - rMapIX[i]) * (BilinearWeightType(1) << MAP_PRECISION));
        }
    }
};

constexpr long constScaleThreadStrip = 32;

typedef void (*ScaleRangeFn)(ScaleContext &rContext, long nStartY, long nEndY);

struct ScaleDown24bit {
    static int getColorComponent(){return 3;}

    static std::vector<int> sumRowsVector() {
      std::vector<int> sumRows(3);
      return sumRows;
    }

    static std::vector<int> sumNumbersVector() {
      std::vector<int> sumNumbers(3);
      return sumNumbers;
    }

    static inline void generateSumRows(Scanline pTmpX, std::vector<int> &sumRows) {
       sumRows[0] += (*pTmpX) << MAP_PRECISION; pTmpX++;
       sumRows[1] += (*pTmpX) << MAP_PRECISION; pTmpX++;
       sumRows[2] += (*pTmpX) << MAP_PRECISION; pTmpX++;
    }

    static inline void generateSumRows(BilinearWeightType nWeightX, Scanline pTmpX, std::vector<int> &sumRows) {
       sumRows[0] += (nWeightX * (*pTmpX)); pTmpX++;
       sumRows[1] += (nWeightX * (*pTmpX)); pTmpX++;
       sumRows[2] += (nWeightX * (*pTmpX)); pTmpX++;
    }

    static inline void generateSumRows(BilinearWeightType nTotalWeightX, std::vector<int> &sumRows) {
       sumRows[0] /= nTotalWeightX;
       sumRows[1] /= nTotalWeightX;
       sumRows[2] /= nTotalWeightX;
    }

    static inline void generateSumNumbers(BilinearWeightType nWeightY, std::vector<int> &sumRows,
                                         std::vector<int> &sumNumbers)
    {
       sumNumbers[0] += nWeightY * sumRows[0];
       sumNumbers[1] += nWeightY * sumRows[1];
       sumNumbers[2] += nWeightY * sumRows[2];
    }

    static inline void generateSumNumbers(BilinearWeightType nTotalWeightY, std::vector<int> &sumNumbers) {
       sumNumbers[0] /= nTotalWeightY;
       sumNumbers[1] /= nTotalWeightY;
       sumNumbers[2] /= nTotalWeightY;
    }

   static inline void calculateDestination(Scanline pScanDest, std::vector<int> &sumNumbers) {

      *pScanDest = sumNumbers[0]; pScanDest++;
      *pScanDest = sumNumbers[1]; pScanDest++;
      *pScanDest = sumNumbers[2]; pScanDest++;
    }

};

struct ScaleDown32bit {
    static int getColorComponent(){return 4;}

    static std::vector<int> sumRowsVector() {
      std::vector<int> sumRows(4);
      return sumRows;
    }

    static std::vector<int> sumNumbersVector() {
      std::vector<int> sumNumbers(4);
      return sumNumbers;
    }

    static inline void generateSumRows(Scanline pTmpX, std::vector<int> &sumRows) {
       sumRows[0] += (*pTmpX) << MAP_PRECISION; pTmpX++;
       sumRows[1] += (*pTmpX) << MAP_PRECISION; pTmpX++;
       sumRows[2] += (*pTmpX) << MAP_PRECISION; pTmpX++;
       sumRows[3] += (*pTmpX) << MAP_PRECISION; pTmpX++;
    }

    static inline void generateSumRows(BilinearWeightType nWeightX, Scanline pTmpX, std::vector<int> &sumRows) {
       sumRows[0] += (nWeightX * (*pTmpX)); pTmpX++;
       sumRows[1] += (nWeightX * (*pTmpX)); pTmpX++;
       sumRows[2] += (nWeightX * (*pTmpX)); pTmpX++;
       sumRows[3] += (nWeightX * (*pTmpX)); pTmpX++;
    }

    static inline void generateSumRows(BilinearWeightType nTotalWeightX, std::vector<int> &sumRows) {
       sumRows[0] /= nTotalWeightX;
       sumRows[1] /= nTotalWeightX;
       sumRows[2] /= nTotalWeightX;
       sumRows[3] /= nTotalWeightX;
    }

    static inline void generateSumNumbers(BilinearWeightType nWeightY,
                                std::vector<int> &sumRows, std::vector<int> &sumNumbers)
    {
       sumNumbers[0] += nWeightY * sumRows[0];
       sumNumbers[1] += nWeightY * sumRows[1];
       sumNumbers[2] += nWeightY * sumRows[2];
       sumNumbers[3] += nWeightY * sumRows[3];
    }

    static inline void generateSumNumbers(BilinearWeightType nTotalWeightY, std::vector<int> &sumNumbers) {
       sumNumbers[0] /= nTotalWeightY;
       sumNumbers[1] /= nTotalWeightY;
       sumNumbers[2] /= nTotalWeightY;
       sumNumbers[3] /= nTotalWeightY;
    }

    static inline void calculateDestination(Scanline pScanDest, std::vector<int> &sumNumbers) {
      *pScanDest = sumNumbers[0]; pScanDest++;
      *pScanDest = sumNumbers[1]; pScanDest++;
      *pScanDest = sumNumbers[2]; pScanDest++;
      *pScanDest = sumNumbers[3]; pScanDest++;
    }

};

struct ScaleUp24bit {
    static int getColorComponent() {return 4;};
    static std::vector<sal_uInt8> colorComponents1() {
        std::vector<sal_uInt8> nComponent(4);
        return nComponent;
    }

    static std::vector<sal_uInt8> colorComponents2() {
        std::vector<sal_uInt8> nComponent(4);
        return nComponent;
    }

    static inline void generateComponent(Scanline pColorPtr0, Scanline pColorPtr1,
                                         BilinearWeightType nTempFX, std::vector<sal_uInt8>&nComponents)
    {
        nComponents[0] = MAP(*pColorPtr0, *pColorPtr1, nTempFX);
        pColorPtr0++; pColorPtr1++;
        nComponents[1] = MAP(*pColorPtr0, *pColorPtr1, nTempFX);
        pColorPtr0++; pColorPtr1++;
        nComponents[2] = MAP(*pColorPtr0, *pColorPtr1, nTempFX);
    }

    static inline void calculteScanDestination(Scanline pScanDest, BilinearWeightType nTempFY,
                                      std::vector<sal_uInt8>&nComponents1, std::vector<sal_uInt8>&nComponents2)
    {
        *pScanDest = MAP(nComponents1[0], nComponents2[0], nTempFY);
        pScanDest++;
        *pScanDest = MAP(nComponents1[1], nComponents2[1], nTempFY);
        pScanDest++;
        *pScanDest = MAP(nComponents1[2], nComponents2[2], nTempFY);
        pScanDest++;
    }

};

struct ScaleUp32bit {
    static int getColorComponent() {return 4;};
    static std::vector<sal_uInt8> colorComponents1() {
        std::vector<sal_uInt8> nComponent(4);
        return nComponent;
    }

    static std::vector<sal_uInt8> colorComponents2() {
        std::vector<sal_uInt8> nComponent(4);
        return nComponent;
    }

    static inline void generateComponent(Scanline pColorPtr0, Scanline pColorPtr1,
                                         BilinearWeightType nTempFX, std::vector<sal_uInt8>&nComponents)
    {
        nComponents[0] = MAP(*pColorPtr0, *pColorPtr1, nTempFX);
        pColorPtr0++; pColorPtr1++;
        nComponents[1] = MAP(*pColorPtr0, *pColorPtr1, nTempFX);
        pColorPtr0++; pColorPtr1++;
        nComponents[2] = MAP(*pColorPtr0, *pColorPtr1, nTempFX);
        pColorPtr0++; pColorPtr1++;
        nComponents[3] = MAP(*pColorPtr0, *pColorPtr1, nTempFX);
    }

    static inline void calculteScanDestination(Scanline pScanDest, BilinearWeightType nTempFY,
                                      std::vector<sal_uInt8>&nComponents1, std::vector<sal_uInt8>&nComponents2)
    {
        *pScanDest = MAP(nComponents1[0], nComponents2[0], nTempFY);
        pScanDest++;
        *pScanDest = MAP(nComponents1[1], nComponents2[1], nTempFY);
        pScanDest++;
        *pScanDest = MAP(nComponents1[2], nComponents2[2], nTempFY);
        pScanDest++;
        *pScanDest = MAP(nComponents1[3], nComponents2[3], nTempFY);
        pScanDest++;
    }

};

template<typename ScaleFunction>
void scaleDown (ScaleContext &rCtx, long nStartY, long nEndY) {

    const int constColorComponents = ScaleFunction::getColorComponent();
    const long nStartX = 0;
    const long nEndX = rCtx.mnDestW - 1;

    for (long nY = nStartY; nY <= nEndY; nY++)
    {
        long nTop = rCtx.mbVMirr ? (nY + 1) : nY;
        long nBottom = rCtx.mbVMirr ? nY : (nY + 1);

        long nLineStart;
        long nLineRange;
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
        for (long nX = nStartX; nX <= nEndX; nX++)
        {
            long nLeft = rCtx.mbHMirr ? (nX + 1) : nX;
            long nRight = rCtx.mbHMirr ? nX : (nX + 1);

            long nRowStart;
            long nRowRange;
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

            std::vector<int> sumNumbers = ScaleFunction::sumNumbersVector();
            BilinearWeightType nTotalWeightY = 0;

            for (long i = 0; i<= nLineRange; i++)
            {
                Scanline pTmpY = rCtx.mpSrc->GetScanline(nLineStart + i);
                Scanline pTmpX = pTmpY + constColorComponents * nRowStart;

                std::vector<int> sumRows = ScaleFunction::sumRowsVector();
                BilinearWeightType nTotalWeightX = 0;

                for (long j = 0; j <= nRowRange; j++)
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

template <typename ScaleFunction>
void scaleUp(ScaleContext &rCtx, long nStartY, long nEndY) {
    const int nColorComponents = ScaleFunction::getColorComponent();
    const long nStartX = 0;
    const long nEndX = rCtx.mnDestW - 1;

    for (long nY = nStartY; nY <= nEndY; nY++)
    {
        long nTempY = rCtx.maMapIY[nY];
        BilinearWeightType nTempFY = rCtx.maMapFY[nY];

        Scanline pLine0 = rCtx.mpSrc->GetScanline(nTempY+0);
        Scanline pLine1 = rCtx.mpSrc->GetScanline(nTempY+1);
        Scanline pScanDest = rCtx.mpDest->GetScanline(nY);

        std::vector<sal_uInt8> nComponents1 = ScaleFunction::colorComponents1();
        std::vector<sal_uInt8> nComponents2 = ScaleFunction::colorComponents1();

        Scanline pColorPtr0;
        Scanline pColorPtr1;

        for (long nX = nStartX; nX <= nEndX; nX++)
        {
            long nTempX = rCtx.maMapIX[nX];
            BilinearWeightType nTempFX = rCtx.maMapFX[nX];

            pColorPtr0 = pLine0 + nTempX * nColorComponents;
            pColorPtr1 = pColorPtr0 + nColorComponents;

            ScaleFunction::generateComponent(pColorPtr0, pColorPtr1, nTempFX, nComponents1);

            pColorPtr0 = pLine1 + nTempX * nColorComponents;
            pColorPtr1 = pColorPtr0 + nColorComponents;

            ScaleFunction::generateComponent(pColorPtr0, pColorPtr1, nTempFX, nComponents1);
            ScaleFunction::calculteScanDestination(pScanDest, nTempFY, nComponents1, nComponents2);

         }
    }
}

class ScaleTask : public comphelper::ThreadTask
{
    ScaleRangeFn mpScaleRangeFunction;
    ScaleContext& mrContext;
    long mnStartY;
    long mnEndY;

public:
    explicit ScaleTask(const std::shared_ptr<comphelper::ThreadTaskTag>& pTag,
                       ScaleRangeFn pScaleRangeFunction,
                       ScaleContext& rContext,
                       long nStartY, long nEndY)
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

void scaleUpPalette8bit(ScaleContext &rCtx, long nStartY, long nEndY)
{
    const long nStartX = 0, nEndX = rCtx.mnDestW - 1;

    for( long nY = nStartY; nY <= nEndY; nY++ )
    {
        long nTempY = rCtx.maMapIY[ nY ];
        BilinearWeightType nTempFY = rCtx.maMapFY[ nY ];
        Scanline pLine0 = rCtx.mpSrc->GetScanline( nTempY );
        Scanline pLine1 = rCtx.mpSrc->GetScanline( ++nTempY );
        Scanline pScanDest = rCtx.mpDest->GetScanline( nY );

        for(long nX = nStartX, nXDst = 0; nX <= nEndX; nX++ )
        {
            long nTempX = rCtx.maMapIX[ nX ];
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

void scaleUpPaletteGeneral(ScaleContext &rCtx, long nStartY, long nEndY)
{
    const long nStartX = 0, nEndX = rCtx.mnDestW - 1;

    for( long nY = nStartY; nY <= nEndY; nY++ )
    {
        long nTempY = rCtx.maMapIY[ nY ];
        BilinearWeightType nTempFY = rCtx.maMapFY[ nY ];
        Scanline pScanline = rCtx.mpDest->GetScanline( nY );

        for( long nX = nStartX, nXDst = 0; nX <= nEndX; nX++ )
        {
            long nTempX = rCtx.maMapIX[ nX ];
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

void scaleUpNonPaletteGeneral(ScaleContext &rCtx, long nStartY, long nEndY)
{
    const long nStartX = 0, nEndX = rCtx.mnDestW - 1;

    for( long nY = nStartY; nY <= nEndY; nY++ )
    {
        long nTempY = rCtx.maMapIY[ nY ];
        BilinearWeightType nTempFY = rCtx.maMapFY[ nY ];
        Scanline pScanDest = rCtx.mpDest->GetScanline( nY );

        for( long nX = nStartX, nXDst = 0; nX <= nEndX; nX++ )
        {
            long nTempX = rCtx.maMapIX[ nX ];
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

void scaleDownPalette8bit(ScaleContext &rCtx, long nStartY, long nEndY)
{
    const long nStartX = 0, nEndX = rCtx.mnDestW - 1;

    for( long nY = nStartY; nY <= nEndY; nY++ )
    {
        long nTop = rCtx.mbVMirr ? ( nY + 1 ) : nY;
        long nBottom = rCtx.mbVMirr ? nY : ( nY + 1 ) ;

        long nLineStart, nLineRange;
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
        for( long nX = nStartX , nXDst = 0; nX <= nEndX; nX++ )
        {
            long nLeft = rCtx.mbHMirr ? ( nX + 1 ) : nX;
            long nRight = rCtx.mbHMirr ? nX : ( nX + 1 ) ;

            long nRowStart;
            long nRowRange;
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

            for(long i = 0; i<= nLineRange; i++)
            {
                Scanline pTmpY = rCtx.mpSrc->GetScanline( nLineStart + i );
                int nSumRowR = 0;
                int nSumRowG = 0;
                int nSumRowB = 0;
                BilinearWeightType nTotalWeightX = 0;

                for(long j = 0; j <= nRowRange; j++)
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

void scaleDownPaletteGeneral(ScaleContext &rCtx, long nStartY, long nEndY)
{
    const long nStartX = 0, nEndX = rCtx.mnDestW - 1;

    for( long nY = nStartY; nY <= nEndY; nY++ )
    {
        long nTop = rCtx.mbVMirr ? ( nY + 1 ) : nY;
        long nBottom = rCtx.mbVMirr ? nY : ( nY + 1 ) ;

        long nLineStart, nLineRange;
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
        for( long nX = nStartX , nXDst = 0; nX <= nEndX; nX++ )
        {
            long nLeft = rCtx.mbHMirr ? ( nX + 1 ) : nX;
            long nRight = rCtx.mbHMirr ? nX : ( nX + 1 ) ;

            long nRowStart, nRowRange;
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

            for(long i = 0; i<= nLineRange; i++)
            {
                int nSumRowR = 0;
                int nSumRowG = 0;
                int nSumRowB = 0;
                BilinearWeightType nTotalWeightX = 0;

                Scanline pScanlineSrc = rCtx.mpSrc->GetScanline( nLineStart + i );
                for(long j = 0; j <= nRowRange; j++)
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

                long nWeightY = lclMaxWeight();
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

void scaleDownNonPaletteGeneral(ScaleContext &rCtx, long nStartY, long nEndY)
{
    const long nStartX = 0, nEndX = rCtx.mnDestW - 1;

    for( long nY = nStartY; nY <= nEndY; nY++ )
    {
        long nTop = rCtx.mbVMirr ? ( nY + 1 ) : nY;
        long nBottom = rCtx.mbVMirr ? nY : ( nY + 1 ) ;

        long nLineStart, nLineRange;
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
        for( long nX = nStartX , nXDst = 0; nX <= nEndX; nX++ )
        {
            long nLeft = rCtx.mbHMirr ? ( nX + 1 ) : nX;
            long nRight = rCtx.mbHMirr ? nX : ( nX + 1 ) ;

            long nRowStart, nRowRange;
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

            for(long i = 0; i<= nLineRange; i++)
            {
                int nSumRowR = 0;
                int nSumRowG = 0;
                int nSumRowB = 0;
                BilinearWeightType nTotalWeightX = 0;

                Scanline pScanlineSrc = rCtx.mpSrc->GetScanline( nLineStart + i );
                for(long j = 0; j <= nRowRange; j++)
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

    const long nDstW = FRound(aSizePix.Width()  * fScaleX);
    const long nDstH = FRound(aSizePix.Height() * fScaleY);

    const double fScaleThresh = 0.6;

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
        Bitmap::ScopedReadAccess pReadAccess(aBitmap);

        sal_uInt16 nSourceBitcount = aBitmap.GetBitCount();

        Bitmap aOutBmp(Size(nDstW, nDstH), std::max(nSourceBitcount, sal_uInt16(24)));
        Size aOutSize = aOutBmp.GetSizePixel();
        sal_uInt16 nTargetBitcount = aOutBmp.GetBitCount();

        if (!aOutSize.Width() || !aOutSize.Height())
        {
            SAL_WARN("vcl.gdi", "bmp creation failed");
            return BitmapEx();
        }

        BitmapScopedWriteAccess pWriteAccess(aOutBmp);

        const long nStartY = 0;
        const long nEndY   = nDstH - 1;

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
            else if (nSourceBitcount != nTargetBitcount)
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
                    pScaleRangeFn = bScaleUp ? scaleUp<ScaleUp24bit> : scaleDown<ScaleDown24bit>;
                    break;
                case ScanlineFormat::N32BitTcRgba:
                case ScanlineFormat::N32BitTcBgra:
                case ScanlineFormat::N32BitTcArgb:
                case ScanlineFormat::N32BitTcAbgr:
                    pScaleRangeFn = bScaleUp ? scaleUp<ScaleUp32bit> : scaleDown<ScaleDown32bit>;
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

                    long nStripYStart = nStartY;
                    long nStripYEnd = nStripYStart + constScaleThreadStrip - 1;

                    while (nStripYEnd < nEndY)
                    {
                        std::unique_ptr<ScaleTask> pTask(new ScaleTask(pTag, pScaleRangeFn, aContext, nStripYStart, nStripYEnd));
                        rShared.pushTask(std::move(pTask));
                        nStripYStart += constScaleThreadStrip;
                        nStripYEnd += constScaleThreadStrip;
                    }
                    if (nStripYStart <= nEndY)
                    {
                        std::unique_ptr<ScaleTask> pTask(new ScaleTask(pTag, pScaleRangeFn, aContext, nStripYStart, nEndY));
                        rShared.pushTask(std::move(pTask));
                    }
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
