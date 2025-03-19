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

#include <vcl/BitmapWriteAccess.hxx>
#include <bitmap/BitmapScaleConvolutionFilter.hxx>

#include <algorithm>
#include <memory>

namespace vcl
{

namespace
{

void ImplCalculateContributions(const sal_Int32 nSoureSize, const sal_Int32 nDestinationSize,
                            sal_Int32& rNumberOfContributions, std::vector<sal_Int16>& rWeights,
                            std::vector<sal_Int32>& rPixels, std::vector<sal_Int32>& rCounts,
                            const Kernel& rKernel)
{
    const double fSamplingRadius(rKernel.GetWidth());
    const double fScale(nDestinationSize / static_cast< double >(nSoureSize));
    const double fScaledRadius((fScale < 1.0) ? fSamplingRadius / fScale : fSamplingRadius);
    const double fFilterFactor(std::min(fScale, 1.0));

    rNumberOfContributions = (sal_Int32(fabs(ceil(fScaledRadius))) * 2) + 1;
    const sal_Int32 nAllocSize(nDestinationSize * rNumberOfContributions);
    rWeights.resize(nAllocSize);
    rPixels.resize(nAllocSize);
    rCounts.resize(nDestinationSize);

    for(sal_Int32 i(0); i < nDestinationSize; i++)
    {
        const sal_Int32 nIndexes(i * rNumberOfContributions);
        const double aCenter(i / fScale);
        const sal_Int32 nLeft(static_cast< sal_Int32 >(floor(aCenter - fScaledRadius)));
        const sal_Int32 nRight(static_cast< sal_Int32 >(ceil(aCenter + fScaledRadius)));
        sal_Int32 nCurrentCount(0);

        for(sal_Int32 j(nLeft); j <= nRight; j++)
        {
            const double fWeight(rKernel.Calculate(fFilterFactor * (aCenter - static_cast< double>(j))));

            // Reduce calculations with ignoring weights of 0.0
            if(fabs(fWeight) < 0.0001)
                continue;

            // Handling on edges
            const sal_Int32 nPixelIndex(std::clamp(j, sal_Int32(0), nSoureSize - 1));
            const sal_Int32 nIndex(nIndexes + nCurrentCount);

            // scale the weight by 255 since we're converting from float to int
            rWeights[nIndex] = fWeight * 255;
            rPixels[nIndex] = nPixelIndex;

            nCurrentCount++;
        }

        rCounts[i] = nCurrentCount;
    }
}

bool ImplScaleConvolutionHor(const Bitmap& rSource, Bitmap& rTarget, const double& rScaleX, const Kernel& rKernel)
{
    // Do horizontal filtering
    OSL_ENSURE(rScaleX > 0.0, "Error in scaling: Mirror given in non-mirror-capable method (!)");
    const sal_Int32 nWidth(rSource.GetSizePixel().Width());
    const sal_Int32 nNewWidth(basegfx::fround(nWidth * rScaleX));

    if (nWidth == nNewWidth)
        return true;

    BitmapScopedReadAccess pReadAcc(rSource);

    if (!pReadAcc)
        return false;

    std::vector<sal_Int16> aWeights;
    std::vector<sal_Int32> aPixels;
    std::vector<sal_Int32> aCounts;
    sal_Int32 nNumberOfContributions(0);

    const sal_Int32 nHeight(rSource.GetSizePixel().Height());
    ImplCalculateContributions(nWidth, nNewWidth, nNumberOfContributions, aWeights, aPixels, aCounts, rKernel);
    rTarget = Bitmap(Size(nNewWidth, nHeight), vcl::PixelFormat::N24_BPP);
    BitmapScopedWriteAccess pWriteAcc(rTarget);

    if (!pWriteAcc)
        return false;

    for (sal_Int32 y(0); y < nHeight; y++)
    {
        Scanline pScanline = pWriteAcc->GetScanline( y );
        Scanline pScanlineRead = pReadAcc->GetScanline( y );

        for (sal_Int32 x(0); x < nNewWidth; x++)
        {
            const sal_Int32 nBaseIndex(x * nNumberOfContributions);
            sal_Int32 nSum(0);
            sal_Int32 nValueRed(0);
            sal_Int32 nValueGreen(0);
            sal_Int32 nValueBlue(0);

            for (sal_Int32 j(0); j < aCounts[x]; j++)
            {
                const sal_Int32 nIndex(nBaseIndex + j);
                const sal_Int16 nWeight(aWeights[nIndex]);
                BitmapColor aColor;

                nSum += nWeight;

                if (pReadAcc->HasPalette())
                    aColor = pReadAcc->GetPaletteColor(pReadAcc->GetIndexFromData(pScanlineRead, aPixels[nIndex]));
                else
                    aColor = pReadAcc->GetPixelFromData(pScanlineRead, aPixels[nIndex]);

                nValueRed += nWeight * aColor.GetRed();
                nValueGreen += nWeight * aColor.GetGreen();
                nValueBlue += nWeight * aColor.GetBlue();
            }

            assert(nSum != 0);

            const BitmapColor aResultColor(
                static_cast< sal_uInt8 >(std::clamp< sal_Int32 >(nValueRed / nSum, 0, 255)),
                static_cast< sal_uInt8 >(std::clamp< sal_Int32 >(nValueGreen / nSum, 0, 255)),
                static_cast< sal_uInt8 >(std::clamp< sal_Int32 >(nValueBlue / nSum, 0, 255)));

            pWriteAcc->SetPixelOnData(pScanline, x, aResultColor);
        }
    }

    pWriteAcc.reset();

    return true;
}

bool ImplScaleConvolutionVer(const Bitmap& rSource, Bitmap& rTarget, const double& rScaleY, const Kernel& rKernel)
{
    // Do vertical filtering
    OSL_ENSURE(rScaleY > 0.0, "Error in scaling: Mirror given in non-mirror-capable method (!)");
    const sal_Int32 nHeight(rSource.GetSizePixel().Height());
    const sal_Int32 nNewHeight(basegfx::fround(nHeight * rScaleY));

    if(nHeight == nNewHeight)
    {
        return true;
    }

    BitmapScopedReadAccess pReadAcc(rSource);
    if(!pReadAcc)
        return false;

    std::vector<sal_Int16> aWeights;
    std::vector<sal_Int32> aPixels;
    std::vector<sal_Int32> aCounts;
    sal_Int32 nNumberOfContributions(0);

    const sal_Int32 nWidth(rSource.GetSizePixel().Width());
    ImplCalculateContributions(nHeight, nNewHeight, nNumberOfContributions, aWeights, aPixels, aCounts, rKernel);
    rTarget = Bitmap(Size(nWidth, nNewHeight), vcl::PixelFormat::N24_BPP);
    BitmapScopedWriteAccess pWriteAcc(rTarget);
    if(!pWriteAcc)
        return false;

    std::vector<BitmapColor> aScanline(nHeight);
    for(sal_Int32 x(0); x < nWidth; x++)
    {
        for(sal_Int32 y(0); y < nHeight; y++)
                if(pReadAcc->HasPalette())
                    aScanline[y] = pReadAcc->GetPaletteColor(pReadAcc->GetPixelIndex(y, x));
                else
                    aScanline[y] = pReadAcc->GetPixel(y, x);
        for(sal_Int32 y(0); y < nNewHeight; y++)
        {
            const sal_Int32 nBaseIndex = y * nNumberOfContributions;
            sal_Int32 nSum(0);
            sal_Int32 nValueRed(0);
            sal_Int32 nValueGreen(0);
            sal_Int32 nValueBlue(0);

            for(sal_Int32 j(0); j < aCounts[y]; j++)
            {
                const sal_Int32 nIndex(nBaseIndex + j);
                const sal_Int16 nWeight(aWeights[nIndex]);
                nSum += nWeight;
                const BitmapColor & aColor = aScanline[aPixels[nIndex]];
                nValueRed += nWeight * aColor.GetRed();
                nValueGreen += nWeight * aColor.GetGreen();
                nValueBlue += nWeight * aColor.GetBlue();
            }

            assert(nSum != 0);

            const BitmapColor aResultColor(
                static_cast< sal_uInt8 >(std::clamp< sal_Int32 >(nValueRed / nSum, 0, 255)),
                static_cast< sal_uInt8 >(std::clamp< sal_Int32 >(nValueGreen / nSum, 0, 255)),
                static_cast< sal_uInt8 >(std::clamp< sal_Int32 >(nValueBlue / nSum, 0, 255)));

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

    return true;
}

bool ImplScaleConvolution(Bitmap& rBitmap, const double& rScaleX, const double& rScaleY, const Kernel& rKernel)
{
    const bool bMirrorHor(rScaleX < 0.0);
    const bool bMirrorVer(rScaleY < 0.0);
    const double fScaleX(bMirrorHor ? -rScaleX : rScaleX);
    const double fScaleY(bMirrorVer ? -rScaleY : rScaleY);
    const sal_Int32 nWidth(rBitmap.GetSizePixel().Width());
    const sal_Int32 nHeight(rBitmap.GetSizePixel().Height());
    const sal_Int32 nNewWidth(basegfx::fround(nWidth * fScaleX));
    const sal_Int32 nNewHeight(basegfx::fround(nHeight * fScaleY));
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

        const sal_Int32 nStartSize(nWidth * nHeight);
        const sal_Int32 nEndSize(nNewWidth * nNewHeight);

        bMirrorAfter = nStartSize > nEndSize;

        if(!bMirrorAfter)
        {
            bResult = rBitmap.Mirror(nMirrorFlags);
        }
    }

    Bitmap aResult;

    if (bResult)
    {
        const sal_Int32 nInBetweenSizeHorFirst(nHeight * nNewWidth);
        const sal_Int32 nInBetweenSizeVerFirst(nNewHeight * nWidth);
        Bitmap aSource(rBitmap);

        if(nInBetweenSizeHorFirst < nInBetweenSizeVerFirst)
        {
            if(bScaleHor)
            {
                bResult = ImplScaleConvolutionHor(aSource, aResult, fScaleX, rKernel);
            }

            if(bResult && bScaleVer)
            {
                if(bScaleHor)
                {
                    // copy partial result, independent of color depth
                    aSource = aResult;
                }

                bResult = ImplScaleConvolutionVer(aSource, aResult, fScaleY, rKernel);
            }
        }
        else
        {
            if(bScaleVer)
            {
                bResult = ImplScaleConvolutionVer(aSource, aResult, fScaleY, rKernel);
            }

            if(bResult && bScaleHor)
            {
                if(bScaleVer)
                {
                    // copy partial result, independent of color depth
                    aSource = aResult;
                }

                bResult = ImplScaleConvolutionHor(aSource, aResult, fScaleX, rKernel);
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
        rBitmap = std::move(aResult);
    }

    return bResult;
}

} // end anonymous namespace

BitmapEx BitmapScaleConvolutionFilter::execute(BitmapEx const& rBitmapEx) const
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
