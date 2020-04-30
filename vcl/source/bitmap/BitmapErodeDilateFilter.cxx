/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/BitmapErodeDilateFilter.hxx>
#include <vcl/bitmapaccess.hxx>
#include <bitmapwriteaccess.hxx>
#include <sal/log.hxx>

#include <comphelper/threadpool.hxx>

namespace
{
class BlurTask : public comphelper::ThreadTask
{
    BlurRangeFn mpBlurFunction;
    BlurSharedData& mrShared;
    long mnStartY;
    long mnEndY;

public:
    explicit BlurTask(const std::shared_ptr<comphelper::ThreadTaskTag>& pTag,
                      BlurRangeFn pBlurFunction, BlurSharedData& rShared, long nStartY, long nEndY)
        : comphelper::ThreadTask(pTag)
        , mpBlurFunction(pBlurFunction)
        , mrShared(rShared)
        , mnStartY(nStartY)
        , mnEndY(nEndY)
    {
    }

    virtual void doWork() override { mpBlurFunction(mrShared, mnStartY, mnEndY); }
};

struct SumFunction24
{
    static inline void add(long*& pValue1, long nConstant)
    {
        pValue1[0] += nConstant;
        pValue1[1] += nConstant;
        pValue1[2] += nConstant;
    }

    static inline void set(long*& pValue1, long nConstant)
    {
        pValue1[0] = nConstant;
        pValue1[1] = nConstant;
        pValue1[2] = nConstant;
    }

    static inline void add(long*& pValue1, sal_uInt8*& pValue2)
    {
        pValue1[0] += pValue2[0];
        pValue1[1] += pValue2[1];
        pValue1[2] += pValue2[2];
    }

    static inline void add(long*& pValue1, long*& pValue2)
    {
        pValue1[0] += pValue2[0];
        pValue1[1] += pValue2[1];
        pValue1[2] += pValue2[2];
    }

    static inline void sub(long*& pValue1, sal_uInt8*& pValue2)
    {
        pValue1[0] -= pValue2[0];
        pValue1[1] -= pValue2[1];
        pValue1[2] -= pValue2[2];
    }

    static inline void sub(long*& pValue1, long*& pValue2)
    {
        pValue1[0] -= pValue2[0];
        pValue1[1] -= pValue2[1];
        pValue1[2] -= pValue2[2];
    }

    static inline void assignPtr(sal_uInt8*& pValue1, sal_uInt8*& pValue2)
    {
        pValue1[0] = pValue2[0];
        pValue1[1] = pValue2[1];
        pValue1[2] = pValue2[2];
    }

    static inline void assignMulAndShr(sal_uInt8*& result, long*& sum, long multiply, long shift)
    {
        result[0] = (multiply * sum[0]) >> shift;
        result[1] = (multiply * sum[1]) >> shift;
        result[2] = (multiply * sum[2]) >> shift;
    }
};

struct SumFunction8
{
    static inline void add(long*& pValue1, long nConstant) { pValue1[0] += nConstant; }

    static inline void set(long*& pValue1, long nConstant) { pValue1[0] = nConstant; }

    static inline void add(long*& pValue1, sal_uInt8*& pValue2) { pValue1[0] += pValue2[0]; }

    static inline void add(long*& pValue1, long*& pValue2) { pValue1[0] += pValue2[0]; }

    static inline void sub(long*& pValue1, sal_uInt8*& pValue2) { pValue1[0] -= pValue2[0]; }

    static inline void sub(long*& pValue1, long*& pValue2) { pValue1[0] -= pValue2[0]; }

    static inline void assignPtr(sal_uInt8*& pValue1, sal_uInt8*& pValue2)
    {
        pValue1[0] = pValue2[0];
    }

    static inline void assignMulAndShr(sal_uInt8*& result, long*& sum, long multiply, long shift)
    {
        result[0] = (multiply * sum[0]) >> shift;
    }
};

template <typename SumFunction>
void stackBlurHorizontal(BlurSharedData const& rShared, long nStart, long nEnd)
{
    BitmapReadAccess* pReadAccess = rShared.mpReadAccess;
    BitmapWriteAccess* pWriteAccess = rShared.mpWriteAccess;

    BlurArrays aArrays(rShared);

    sal_uInt8* pStack = aArrays.maStackBuffer.data();
    sal_uInt8* pStackPtr;

    long nWidth = pReadAccess->Width();
    long nLastIndexX = nWidth - 1;

    long nMultiplyValue = aArrays.getMultiplyValue();
    long nShiftValue = aArrays.getShiftValue();

    long nRadius = rShared.mnRadius;
    long nComponentWidth = rShared.mnComponentWidth;
    long nDiv = rShared.mnDiv;

    Scanline pSourcePointer;
    Scanline pDestinationPointer;

    long nXPosition;
    long nStackIndex;
    long nStackIndexStart;
    long nWeight;

    aArrays.initializeWeightAndPositions(nLastIndexX);

    long* nSum = aArrays.mnSumVector.data();
    long* nInSum = aArrays.mnInSumVector.data();
    long* nOutSum = aArrays.mnOutSumVector.data();

    long* pPositionPointer = aArrays.maPositionTable.data();
    long* pWeightPointer = aArrays.maWeightTable.data();

    for (long y = nStart; y <= nEnd; y++)
    {
        SumFunction::set(nSum, 0L);
        SumFunction::set(nInSum, 0L);
        SumFunction::set(nOutSum, 0L);

        // Pre-initialize blur data for first pixel.
        // aArrays.maPositionTable contains values like (for radius of 5): [0,0,0,0,0,0,1,2,3,4,5],
        // which are used as pixels indices in the current row that we use to prepare information
        // for the first pixel; aArrays.maWeightTable has [1,2,3,4,5,6,5,4,3,2,1]. Before looking at
        // the first row pixel, we pretend to have processed fake previous pixels, as if the row was
        // extended to the left with the same color as that of the first pixel.
        for (long i = 0; i < nDiv; i++)
        {
            pSourcePointer = pReadAccess->GetScanline(y) + nComponentWidth * pPositionPointer[i];

            pStackPtr = &pStack[nComponentWidth * i];

            SumFunction::assignPtr(pStackPtr, pSourcePointer);

            nWeight = pWeightPointer[i];

            SumFunction::add(nSum, pSourcePointer[0] * nWeight);

            if (i - nRadius > 0)
            {
                SumFunction::add(nInSum, pSourcePointer);
            }
            else
            {
                SumFunction::add(nOutSum, pSourcePointer);
            }
        }

        nStackIndex = nRadius;
        nXPosition = std::min(nRadius, nLastIndexX);

        pSourcePointer = pReadAccess->GetScanline(y) + nComponentWidth * nXPosition;

        for (long x = 0; x < nWidth; x++)
        {
            pDestinationPointer = pWriteAccess->GetScanline(y) + nComponentWidth * x;

            SumFunction::assignMulAndShr(pDestinationPointer, nSum, nMultiplyValue, nShiftValue);

            SumFunction::sub(nSum, nOutSum);

            nStackIndexStart = nStackIndex + nDiv - nRadius;
            if (nStackIndexStart >= nDiv)
            {
                nStackIndexStart -= nDiv;
            }
            pStackPtr = &pStack[nComponentWidth * nStackIndexStart];

            SumFunction::sub(nOutSum, pStackPtr);

            if (nXPosition < nLastIndexX)
            {
                nXPosition++;
                pSourcePointer = pReadAccess->GetScanline(y) + nComponentWidth * nXPosition;
            }

            SumFunction::assignPtr(pStackPtr, pSourcePointer);

            SumFunction::add(nInSum, pSourcePointer);

            SumFunction::add(nSum, nInSum);

            nStackIndex++;
            if (nStackIndex >= nDiv)
            {
                nStackIndex = 0;
            }

            pStackPtr = &pStack[nStackIndex * nComponentWidth];

            SumFunction::add(nOutSum, pStackPtr);
            SumFunction::sub(nInSum, pStackPtr);
        }
    }
}

template <typename SumFunction>
void stackBlurVertical(BlurSharedData const& rShared, long nStart, long nEnd)
{
    BitmapReadAccess* pReadAccess = rShared.mpReadAccess;
    BitmapWriteAccess* pWriteAccess = rShared.mpWriteAccess;

    BlurArrays aArrays(rShared);

    sal_uInt8* pStack = aArrays.maStackBuffer.data();
    sal_uInt8* pStackPtr;

    long nHeight = pReadAccess->Height();
    long nLastIndexY = nHeight - 1;

    long nMultiplyValue = aArrays.getMultiplyValue();
    long nShiftValue = aArrays.getShiftValue();

    long nRadius = rShared.mnRadius;
    long nComponentWidth = rShared.mnComponentWidth;
    long nDiv = rShared.mnDiv;

    Scanline pSourcePointer;
    Scanline pDestinationPointer;

    long nYPosition;
    long nStackIndex;
    long nStackIndexStart;
    long nWeight;

    aArrays.initializeWeightAndPositions(nLastIndexY);

    long* nSum = aArrays.mnSumVector.data();
    long* nInSum = aArrays.mnInSumVector.data();
    long* nOutSum = aArrays.mnOutSumVector.data();
    long* pPositionPointer = aArrays.maPositionTable.data();
    long* pWeightPointer = aArrays.maWeightTable.data();

    for (long x = nStart; x <= nEnd; x++)
    {
        SumFunction::set(nSum, 0L);
        SumFunction::set(nInSum, 0L);
        SumFunction::set(nOutSum, 0L);

        // Pre-initialize blur data for first pixel.
        // aArrays.maPositionTable contains values like (for radius of 5): [0,0,0,0,0,0,1,2,3,4,5],
        // which are used as pixels indices in the current column that we use to prepare information
        // for the first pixel; aArrays.maWeightTable has [1,2,3,4,5,6,5,4,3,2,1]. Before looking at
        // the first column pixels, we pretend to have processed fake previous pixels, as if the
        // column was extended to the top with the same color as that of the first pixel.
        for (long i = 0; i < nDiv; i++)
        {
            pSourcePointer = pReadAccess->GetScanline(pPositionPointer[i]) + nComponentWidth * x;

            pStackPtr = &pStack[nComponentWidth * i];

            SumFunction::assignPtr(pStackPtr, pSourcePointer);

            nWeight = pWeightPointer[i];

            SumFunction::add(nSum, pSourcePointer[0] * nWeight);

            if (i - nRadius > 0)
            {
                SumFunction::add(nInSum, pSourcePointer);
            }
            else
            {
                SumFunction::add(nOutSum, pSourcePointer);
            }
        }

        nStackIndex = nRadius;
        nYPosition = std::min(nRadius, nLastIndexY);

        pSourcePointer = pReadAccess->GetScanline(nYPosition) + nComponentWidth * x;

        for (long y = 0; y < nHeight; y++)
        {
            pDestinationPointer = pWriteAccess->GetScanline(y) + nComponentWidth * x;

            SumFunction::assignMulAndShr(pDestinationPointer, nSum, nMultiplyValue, nShiftValue);

            SumFunction::sub(nSum, nOutSum);

            nStackIndexStart = nStackIndex + nDiv - nRadius;
            if (nStackIndexStart >= nDiv)
            {
                nStackIndexStart -= nDiv;
            }
            pStackPtr = &pStack[nComponentWidth * nStackIndexStart];

            SumFunction::sub(nOutSum, pStackPtr);

            if (nYPosition < nLastIndexY)
            {
                nYPosition++;
                pSourcePointer = pReadAccess->GetScanline(nYPosition) + nComponentWidth * x;
            }

            SumFunction::assignPtr(pStackPtr, pSourcePointer);
            SumFunction::add(nInSum, pSourcePointer);
            SumFunction::add(nSum, nInSum);

            nStackIndex++;
            if (nStackIndex >= nDiv)
            {
                nStackIndex = 0;
            }

            pStackPtr = &pStack[nStackIndex * nComponentWidth];

            SumFunction::add(nOutSum, pStackPtr);
            SumFunction::sub(nInSum, pStackPtr);
        }
    }
}

void runStackBlur(Bitmap& rBitmap, const long nRadius, const long nComponentWidth,
                  const long nColorChannels, BlurRangeFn pBlurHorizontalFn,
                  BlurRangeFn pBlurVerticalFn, const bool bParallel)
{
    if (bParallel)
    {
        try
        {
            comphelper::ThreadPool& rShared = comphelper::ThreadPool::getSharedOptimalPool();
            std::shared_ptr<comphelper::ThreadTaskTag> pTag
                = comphelper::ThreadPool::createThreadTaskTag();

            {
                Bitmap::ScopedReadAccess pReadAccess(rBitmap);
                BitmapScopedWriteAccess pWriteAccess(rBitmap);

                BlurSharedData aSharedData(pReadAccess.get(), pWriteAccess.get(), nRadius,
                                           nComponentWidth, nColorChannels);

                const Size aSize = rBitmap.GetSizePixel();
                long nEnd = aSize.Height() - 1;

                long nStripStart = 0;
                long nStripEnd = nStripStart + constBlurThreadStrip - 1;

                while (nStripEnd < nEnd)
                {
                    std::unique_ptr<BlurTask> pTask(
                        new BlurTask(pTag, pBlurHorizontalFn, aSharedData, nStripStart, nStripEnd));
                    rShared.pushTask(std::move(pTask));
                    nStripStart += constBlurThreadStrip;
                    nStripEnd += constBlurThreadStrip;
                }
                if (nStripStart <= nEnd)
                {
                    std::unique_ptr<BlurTask> pTask(
                        new BlurTask(pTag, pBlurHorizontalFn, aSharedData, nStripStart, nEnd));
                    rShared.pushTask(std::move(pTask));
                }
                rShared.waitUntilDone(pTag);
            }
            {
                Bitmap::ScopedReadAccess pReadAccess(rBitmap);
                BitmapScopedWriteAccess pWriteAccess(rBitmap);

                BlurSharedData aSharedData(pReadAccess.get(), pWriteAccess.get(), nRadius,
                                           nComponentWidth, nColorChannels);

                const Size aSize = rBitmap.GetSizePixel();
                long nEnd = aSize.Width() - 1;

                long nStripStart = 0;
                long nStripEnd = nStripStart + constBlurThreadStrip - 1;

                while (nStripEnd < nEnd)
                {
                    std::unique_ptr<BlurTask> pTask(
                        new BlurTask(pTag, pBlurVerticalFn, aSharedData, nStripStart, nStripEnd));
                    rShared.pushTask(std::move(pTask));
                    nStripStart += constBlurThreadStrip;
                    nStripEnd += constBlurThreadStrip;
                }
                if (nStripStart <= nEnd)
                {
                    std::unique_ptr<BlurTask> pTask(
                        new BlurTask(pTag, pBlurVerticalFn, aSharedData, nStripStart, nEnd));
                    rShared.pushTask(std::move(pTask));
                }
                rShared.waitUntilDone(pTag);
            }
        }
        catch (...)
        {
            SAL_WARN("vcl.gdi", "threaded bitmap blurring failed");
        }
    }
    else
    {
        {
            Bitmap::ScopedReadAccess pReadAccess(rBitmap);
            BitmapScopedWriteAccess pWriteAccess(rBitmap);
            BlurSharedData aSharedData(pReadAccess.get(), pWriteAccess.get(), nRadius,
                                       nComponentWidth, nColorChannels);
            long nFirstIndex = 0;
            long nLastIndex = pReadAccess->Height() - 1;
            pBlurHorizontalFn(aSharedData, nFirstIndex, nLastIndex);
        }
        {
            Bitmap::ScopedReadAccess pReadAccess(rBitmap);
            BitmapScopedWriteAccess pWriteAccess(rBitmap);
            BlurSharedData aSharedData(pReadAccess.get(), pWriteAccess.get(), nRadius,
                                       nComponentWidth, nColorChannels);
            long nFirstIndex = 0;
            long nLastIndex = pReadAccess->Width() - 1;
            pBlurVerticalFn(aSharedData, nFirstIndex, nLastIndex);
        }
    }
}

void stackBlur24(Bitmap& rBitmap, sal_Int32 nRadius, sal_Int32 nComponentWidth)
{
    const bool bParallel = true;
    // Limit radius
    nRadius = std::clamp<sal_Int32>(nRadius, 2, 254);
    const long nColorChannels = 3; // 3 color channel

    BlurRangeFn pBlurHorizontalFn = stackBlurHorizontal<SumFunction24>;
    BlurRangeFn pBlurVerticalFn = stackBlurVertical<SumFunction24>;

    runStackBlur(rBitmap, nRadius, nComponentWidth, nColorChannels, pBlurHorizontalFn,
                 pBlurVerticalFn, bParallel);
}

void stackBlur8(Bitmap& rBitmap, sal_Int32 nRadius, sal_Int32 nComponentWidth)
{
    const bool bParallel = true;
    // Limit radius
    nRadius = std::clamp<sal_Int32>(nRadius, 2, 254);
    const long nColorChannels = 1; // 1 color channel

    BlurRangeFn pBlurHorizontalFn = stackBlurHorizontal<SumFunction8>;
    BlurRangeFn pBlurVerticalFn = stackBlurVertical<SumFunction8>;

    runStackBlur(rBitmap, nRadius, nComponentWidth, nColorChannels, pBlurHorizontalFn,
                 pBlurVerticalFn, bParallel);
}

} // end anonymous namespace

/**
 * Implementation of stack blur - a fast Gaussian blur approximation.
 * nRadius - blur radius, valid values are between 2 and 254
 * bExtend - extend the bitmap in all directions by the radius
 *
 * Stack Blur Algorithm by Mario Klingemann <mario@quasimondo.com>
 * (http://www.quasimondo.com/StackBlurForCanvas/StackBlurDemo.html)
 *
 * Additionally references and implementations:
 * - Blur.js by Jacob Kelley
 *   (http://www.blurjs.com)
 * - BlurEffectForAndroidDesign by Nicolas Pomepuy
 *   (https://github.com/PomepuyN/BlurEffectForAndroidDesign)
 * - StackBluriOS by Thomas Landspurg
 *   (https://github.com/tomsoft1/StackBluriOS)
 * - stackblur.cpp by Benjamin Yates
 *   (https://gist.github.com/benjamin9999/3809142)
 * - stack blur in fog 2D graphic library by Petr Kobalicek
 *   (https://code.google.com/p/fog/)
 *
 */
BitmapFilterStackBlur::BitmapFilterStackBlur(sal_Int32 nRadius)
    : mnRadius(nRadius)
{
}

BitmapFilterStackBlur::~BitmapFilterStackBlur() {}

BitmapEx BitmapFilterStackBlur::execute(BitmapEx const& rBitmapEx) const
{
    Bitmap aBitmap = rBitmapEx.GetBitmap();
    Bitmap result = filter(aBitmap);
    return BitmapEx(result, rBitmapEx.GetMask());
}

Bitmap BitmapFilterStackBlur::filter(Bitmap const& rBitmap) const
{
    Bitmap bitmapCopy(rBitmap);
    ScanlineFormat nScanlineFormat;
    {
        Bitmap::ScopedReadAccess pReadAccess(bitmapCopy);
        nScanlineFormat = pReadAccess->GetScanlineFormat();
    }

    if (nScanlineFormat == ScanlineFormat::N24BitTcRgb
        || nScanlineFormat == ScanlineFormat::N24BitTcBgr
        || nScanlineFormat == ScanlineFormat::N32BitTcMask
        || nScanlineFormat == ScanlineFormat::N32BitTcBgra)
    {
        int nComponentWidth = (nScanlineFormat == ScanlineFormat::N32BitTcMask
                               || nScanlineFormat == ScanlineFormat::N32BitTcBgra)
                                  ? 4
                                  : 3;

        stackBlur24(bitmapCopy, mnRadius, nComponentWidth);
    }
    else if (nScanlineFormat == ScanlineFormat::N8BitPal)
    {
        int nComponentWidth = 1;

        stackBlur8(bitmapCopy, mnRadius, nComponentWidth);
    }

    return bitmapCopy;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
