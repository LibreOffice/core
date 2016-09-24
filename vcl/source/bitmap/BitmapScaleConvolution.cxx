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

#include "BitmapScaleConvolution.hxx"
#include "ResampleKernel.hxx"

#include <vcl/bitmapaccess.hxx>
#include <osl/diagnose.h>

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
    double*& pWeights,
    long*& pPixels,
    long*& pCount,
    const Kernel& aKernel)
{
    const double fSamplingRadius(aKernel.GetWidth());
    const double fScale(aDestinationSize / static_cast< double >(aSourceSize));
    const double fScaledRadius((fScale < 1.0) ? fSamplingRadius / fScale : fSamplingRadius);
    const double fFilterFactor((fScale < 1.0) ? fScale : 1.0);

    aNumberOfContributions = (long(fabs(ceil(fScaledRadius))) * 2) + 1;
    const long nAllocSize(aDestinationSize * aNumberOfContributions);
    pWeights = new double[nAllocSize];
    pPixels = new long[nAllocSize];
    pCount = new long[aDestinationSize];

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

            pWeights[nIndex] = aWeight;
            pPixels[nIndex] = aPixelIndex;

            aCurrentCount++;
        }

        pCount[i] = aCurrentCount;
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

    BitmapReadAccess* pReadAcc = rSource.AcquireReadAccess();

    if(pReadAcc)
    {
        double* pWeights = nullptr;
        long* pPixels = nullptr;
        long* pCount = nullptr;
        long aNumberOfContributions(0);

        const long nHeight(rSource.GetSizePixel().Height());
        ImplCalculateContributions(nWidth, nNewWidth, aNumberOfContributions, pWeights, pPixels, pCount, aKernel);
        rTarget = Bitmap(Size(nNewWidth, nHeight), 24);
        BitmapWriteAccess* pWriteAcc = rTarget.AcquireWriteAccess();
        bool bResult(nullptr != pWriteAcc);

        if(bResult)
        {
            for(long y(0); y < nHeight; y++)
            {
                for(long x(0); x < nNewWidth; x++)
                {
                    const long aBaseIndex(x * aNumberOfContributions);
                    double aSum(0.0);
                    double aValueRed(0.0);
                    double aValueGreen(0.0);
                    double aValueBlue(0.0);

                    for(long j(0); j < pCount[x]; j++)
                    {
                        const long aIndex(aBaseIndex + j);
                        const double aWeight(pWeights[aIndex]);
                        BitmapColor aColor;

                        aSum += aWeight;

                        if(pReadAcc->HasPalette())
                        {
                            aColor = pReadAcc->GetPaletteColor(pReadAcc->GetPixelIndex(y, pPixels[aIndex]));
                        }
                        else
                        {
                            aColor = pReadAcc->GetPixel(y, pPixels[aIndex]);
                        }

                        aValueRed += aWeight * aColor.GetRed();
                        aValueGreen += aWeight * aColor.GetGreen();
                        aValueBlue += aWeight * aColor.GetBlue();
                    }

                    const BitmapColor aResultColor(
                        static_cast< sal_uInt8 >(MinMax(static_cast< sal_Int32 >(aValueRed / aSum), 0, 255)),
                        static_cast< sal_uInt8 >(MinMax(static_cast< sal_Int32 >(aValueGreen / aSum), 0, 255)),
                        static_cast< sal_uInt8 >(MinMax(static_cast< sal_Int32 >(aValueBlue / aSum), 0, 255)));

                    pWriteAcc->SetPixel(y, x, aResultColor);
                }
            }

            Bitmap::ReleaseAccess(pWriteAcc);
        }

        Bitmap::ReleaseAccess(pReadAcc);
        delete[] pWeights;
        delete[] pCount;
        delete[] pPixels;

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

    BitmapReadAccess* pReadAcc = rSource.AcquireReadAccess();

    if(pReadAcc)
    {
        double* pWeights = nullptr;
        long* pPixels = nullptr;
        long* pCount = nullptr;
        long aNumberOfContributions(0);

        const long nWidth(rSource.GetSizePixel().Width());
        ImplCalculateContributions(nHeight, nNewHeight, aNumberOfContributions, pWeights, pPixels, pCount, aKernel);
        rTarget = Bitmap(Size(nWidth, nNewHeight), 24);
        BitmapWriteAccess* pWriteAcc = rTarget.AcquireWriteAccess();
        bool bResult(nullptr != pWriteAcc);

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

                    for(long j(0); j < pCount[y]; j++)
                    {
                        const long aIndex(aBaseIndex + j);
                        const double aWeight(pWeights[aIndex]);
                        BitmapColor aColor;

                        aSum += aWeight;

                        if(pReadAcc->HasPalette())
                        {
                            aColor = pReadAcc->GetPaletteColor(pReadAcc->GetPixelIndex(pPixels[aIndex], x));
                        }
                        else
                        {
                            aColor = pReadAcc->GetPixel(pPixels[aIndex], x);
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

        Bitmap::ReleaseAccess(pWriteAcc);
        Bitmap::ReleaseAccess(pReadAcc);

        delete[] pWeights;
        delete[] pCount;
        delete[] pPixels;

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
        rBitmap.ImplAdaptBitCount(aResult);
        rBitmap = aResult;
    }

    return bResult;
}

} // end anonymous namespace

bool BitmapScaleConvolution::filter(Bitmap& rBitmap)
{

    switch(meKernelType)
    {
        case ConvolutionKernelType::Box:
            return ImplScaleConvolution(rBitmap, mrScaleX, mrScaleY, BoxKernel());
        case ConvolutionKernelType::BiLinear:
            return ImplScaleConvolution(rBitmap, mrScaleX, mrScaleY, BilinearKernel());
        case ConvolutionKernelType::BiCubic:
            return ImplScaleConvolution(rBitmap, mrScaleX, mrScaleY, BicubicKernel());
        case ConvolutionKernelType::Lanczos3:
            return ImplScaleConvolution(rBitmap, mrScaleX, mrScaleY, Lanczos3Kernel());
        default:
            break;
    }
    return false;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
