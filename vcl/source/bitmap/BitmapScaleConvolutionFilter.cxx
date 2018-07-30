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

#include <bitmapwriteaccess.hxx>
#include <BitmapScaleConvolutionFilter.hxx>

#include <algorithm>
#include <memory>

namespace vcl
{

namespace
{

void ImplCalculateContributions(
    const long aSourceSize,
    const long aDestinationSize,
    long& aNumberOfContributions,
    std::vector<double>& rWeights,
    std::vector<long>& rPixels,
    std::vector<long>& rCounts,
    const Kernel& aKernel)
{
    const double fSamplingRadius(aKernel.GetWidth());
    const double fScale(aDestinationSize / static_cast< double >(aSourceSize));
    const double fScaledRadius((fScale < 1.0) ? fSamplingRadius / fScale : fSamplingRadius);
    const double fFilterFactor(std::min(fScale, 1.0));

    aNumberOfContributions = (long(fabs(ceil(fScaledRadius))) * 2) + 1;
    const long nAllocSize(aDestinationSize * aNumberOfContributions);
    rWeights.resize(nAllocSize);
    rPixels.resize(nAllocSize);
    rCounts.resize(aDestinationSize);

    for(long i(0); i < aDestinationSize; i++)
    {
        const long aIndex(i * aNumberOfContributions);
        const double aCenter(i / fScale);
        const sal_Int32 aLeft(static_cast< sal_Int32 >(floor(aCenter - fScaledRadius)));
        const sal_Int32 aRight(static_cast< sal_Int32 >(ceil(aCenter + fScaledRadius)));
        long aCurrentCount(0);

        for(sal_Int32 j(aLeft); j <= aRight; j++)
        {
            const double aWeight(aKernel.Calculate(fFilterFactor * (aCenter - static_cast< double>(j))));

            // Reduce calculations with ignoring weights of 0.0
            if(fabs(aWeight) < 0.0001)
            {
                continue;
            }

            // Handling on edges
            const long aPixelIndex(MinMax(j, 0, aSourceSize - 1));
            const long nIndex(aIndex + aCurrentCount);

            rWeights[nIndex] = aWeight;
            rPixels[nIndex] = aPixelIndex;

            aCurrentCount++;
        }

        rCounts[i] = aCurrentCount;
    }
}

bool ImplScaleConvolutionHor(Bitmap& rSource, Bitmap& rTarget, const double& rScaleX, const Kernel& aKernel)
{
    // Do horizontal filtering
    OSL_ENSURE(rScaleX > 0.0, "Error in scaling: Mirror given in non-mirror-capable method (!)");
    const long nWidth(rSource.GetSizePixel().Width());
    const long nNewWidth(FRound(nWidth * rScaleX));

    if(nWidth == nNewWidth)
    {
        return true;
    }

    Bitmap::ScopedReadAccess pReadAcc(rSource);

    if(pReadAcc)
    {
        std::vector<double> aWeights;
        std::vector<long> aPixels;
        std::vector<long> aCounts;
        long aNumberOfContributions(0);

        const long nHeight(rSource.GetSizePixel().Height());
        ImplCalculateContributions(nWidth, nNewWidth, aNumberOfContributions, aWeights, aPixels, aCounts, aKernel);
        rTarget = Bitmap(Size(nNewWidth, nHeight), 24);
        BitmapScopedWriteAccess pWriteAcc(rTarget);
        bool bResult(pWriteAcc);

        if(bResult)
        {
            for(long y(0); y < nHeight; y++)
            {
                Scanline pScanline = pWriteAcc->GetScanline( y );
                Scanline pScanlineRead = pReadAcc->GetScanline( y );
                for(long x(0); x < nNewWidth; x++)
                {
                    const long aBaseIndex(x * aNumberOfContributions);
                    double aSum(0.0);
                    double aValueRed(0.0);
                    double aValueGreen(0.0);
                    double aValueBlue(0.0);

                    for(long j(0); j < aCounts[x]; j++)
                    {
                        const long aIndex(aBaseIndex + j);
                        const double aWeight(aWeights[aIndex]);
                        BitmapColor aColor;

                        aSum += aWeight;

                        if(pReadAcc->HasPalette())
                        {
                            aColor = pReadAcc->GetPaletteColor(pReadAcc->GetIndexFromData(pScanlineRead, aPixels[aIndex]));
                        }
                        else
                        {
                            aColor = pReadAcc->GetPixelFromData(pScanlineRead, aPixels[aIndex]);
                        }

                        aValueRed += aWeight * aColor.GetRed();
                        aValueGreen += aWeight * aColor.GetGreen();
                        aValueBlue += aWeight * aColor.GetBlue();
                    }

                    const BitmapColor aResultColor(
                        static_cast< sal_uInt8 >(MinMax(static_cast< sal_Int32 >(aValueRed / aSum), 0, 255)),
                        static_cast< sal_uInt8 >(MinMax(static_cast< sal_Int32 >(aValueGreen / aSum), 0, 255)),
                        static_cast< sal_uInt8 >(MinMax(static_cast< sal_Int32 >(aValueBlue / aSum), 0, 255)));

                    pWriteAcc->SetPixelOnData(pScanline, x, aResultColor);
                }
            }

            pWriteAcc.reset();
        }

        aWeights.clear();
        aCounts.clear();
        aPixels.clear();

        if(bResult)
        {
            return true;
        }
    }

    return false;
}

bool ImplScaleConvolutionVer(Bitmap& rSource, Bitmap& rTarget, const double& rScaleY, const Kernel& aKernel)
{
    // Do vertical filtering
    OSL_ENSURE(rScaleY > 0.0, "Error in scaling: Mirror given in non-mirror-capable method (!)");
    const long nHeight(rSource.GetSizePixel().Height());
    const long nNewHeight(FRound(nHeight * rScaleY));

    if(nHeight == nNewHeight)
    {
        return true;
    }

    Bitmap::ScopedReadAccess pReadAcc(rSource);

    if(pReadAcc)
    {
        std::vector<double> aWeights;
        std::vector<long> aPixels;
        std::vector<long> aCounts;
        long aNumberOfContributions(0);

        const long nWidth(rSource.GetSizePixel().Width());
        ImplCalculateContributions(nHeight, nNewHeight, aNumberOfContributions, aWeights, aPixels, aCounts, aKernel);
        rTarget = Bitmap(Size(nWidth, nNewHeight), 24);
        BitmapScopedWriteAccess pWriteAcc(rTarget);
        bool bResult(pWriteAcc);

        if(pWriteAcc)
        {
            for(long x(0); x < nWidth; x++)
            {
                for(long y(0); y < nNewHeight; y++)
                {
                    const long aBaseIndex(y * aNumberOfContributions);
                    double aSum(0.0);
                    double aValueRed(0.0);
                    double aValueGreen(0.0);
                    double aValueBlue(0.0);

                    for(long j(0); j < aCounts[y]; j++)
                    {
                        const long aIndex(aBaseIndex + j);
                        const double aWeight(aWeights[aIndex]);
                        BitmapColor aColor;

                        aSum += aWeight;

                        if(pReadAcc->HasPalette())
                        {
                            aColor = pReadAcc->GetPaletteColor(pReadAcc->GetPixelIndex(aPixels[aIndex], x));
                        }
                        else
                        {
                            aColor = pReadAcc->GetPixel(aPixels[aIndex], x);
                        }

                        aValueRed += aWeight * aColor.GetRed();
                        aValueGreen += aWeight * aColor.GetGreen();
                        aValueBlue += aWeight * aColor.GetBlue();
                    }

                    const BitmapColor aResultColor(
                        static_cast< sal_uInt8 >(MinMax(static_cast< sal_Int32 >(aValueRed / aSum), 0, 255)),
                        static_cast< sal_uInt8 >(MinMax(static_cast< sal_Int32 >(aValueGreen / aSum), 0, 255)),
                        static_cast< sal_uInt8 >(MinMax(static_cast< sal_Int32 >(aValueBlue / aSum), 0, 255)));

                    if(pWriteAcc->HasPalette())
                    {
                        pWriteAcc->SetPixelIndex(y, x, static_cast< sal_uInt8 >(pWriteAcc->GetBestPaletteIndex(aResultColor)));
                    }
                    else
                    {
                        pWriteAcc->SetPixel(y, x, aResultColor);
                    }
                }
            }
        }

        aWeights.clear();
        aCounts.clear();
        aPixels.clear();

        if(bResult)
        {
            return true;
        }
    }

    return false;
}

bool ImplScaleConvolution(Bitmap& rBitmap, const double& rScaleX, const double& rScaleY, const Kernel& aKernel)
{
    const bool bMirrorHor(rScaleX < 0.0);
    const bool bMirrorVer(rScaleY < 0.0);
    const double fScaleX(bMirrorHor ? -rScaleX : rScaleX);
    const double fScaleY(bMirrorVer ? -rScaleY : rScaleY);
    const long nWidth(rBitmap.GetSizePixel().Width());
    const long nHeight(rBitmap.GetSizePixel().Height());
    const long nNewWidth(FRound(nWidth * fScaleX));
    const long nNewHeight(FRound(nHeight * fScaleY));
    const bool bScaleHor(nWidth != nNewWidth);
    const bool bScaleVer(nHeight != nNewHeight);
    const bool bMirror(bMirrorHor || bMirrorVer);

    if (!bMirror && !bScaleHor && !bScaleVer)
    {
        return true;
    }

    bool bResult(true);
    BmpMirrorFlags nMirrorFlags(BmpMirrorFlags::NONE);
    bool bMirrorAfter(false);

    if (bMirror)
    {
        if(bMirrorHor)
        {
            nMirrorFlags |= BmpMirrorFlags::Horizontal;
        }

        if(bMirrorVer)
        {
            nMirrorFlags |= BmpMirrorFlags::Vertical;
        }

        const long nStartSize(nWidth * nHeight);
        const long nEndSize(nNewWidth * nNewHeight);

        bMirrorAfter = nStartSize > nEndSize;

        if(!bMirrorAfter)
        {
            bResult = rBitmap.Mirror(nMirrorFlags);
        }
    }

    Bitmap aResult;

    if (bResult)
    {
        const long nInBetweenSizeHorFirst(nHeight * nNewWidth);
        const long nInBetweenSizeVerFirst(nNewHeight * nWidth);
        Bitmap aSource(rBitmap);

        if(nInBetweenSizeHorFirst < nInBetweenSizeVerFirst)
        {
            if(bScaleHor)
            {
                bResult = ImplScaleConvolutionHor(aSource, aResult, fScaleX, aKernel);
            }

            if(bResult && bScaleVer)
            {
                if(bScaleHor)
                {
                    // copy partial result, independent of color depth
                    aSource = aResult;
                }

                bResult = ImplScaleConvolutionVer(aSource, aResult, fScaleY, aKernel);
            }
        }
        else
        {
            if(bScaleVer)
            {
                bResult = ImplScaleConvolutionVer(aSource, aResult, fScaleY, aKernel);
            }

            if(bResult && bScaleHor)
            {
                if(bScaleVer)
                {
                    // copy partial result, independent of color depth
                    aSource = aResult;
                }

                bResult = ImplScaleConvolutionHor(aSource, aResult, fScaleX, aKernel);
            }
        }
    }

    if(bResult && bMirrorAfter)
    {
        bResult = aResult.Mirror(nMirrorFlags);
    }

    if(bResult)
    {
        rBitmap.AdaptBitCount(aResult);
        rBitmap = aResult;
    }

    return bResult;
}

} // end anonymous namespace

BitmapEx BitmapScaleConvolutionFilter::execute(BitmapEx const& rBitmapEx)
{
    bool bRetval = false;
    Bitmap aBitmap(rBitmapEx.GetBitmap());

    bRetval = ImplScaleConvolution(aBitmap, mrScaleX, mrScaleY, *mxKernel);

    if (bRetval)
        return BitmapEx(aBitmap);

    return BitmapEx();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
