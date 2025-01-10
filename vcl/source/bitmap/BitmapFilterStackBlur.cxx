/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sal/log.hxx>
#include <comphelper/threadpool.hxx>

#include <vcl/bitmap/BitmapFilterStackBlur.hxx>
#include <vcl/BitmapWriteAccess.hxx>

namespace
{
const sal_Int16 constMultiplyTable[255]
    = { 512, 512, 456, 512, 328, 456, 335, 512, 405, 328, 271, 456, 388, 335, 292, 512, 454,
        405, 364, 328, 298, 271, 496, 456, 420, 388, 360, 335, 312, 292, 273, 512, 482, 454,
        428, 405, 383, 364, 345, 328, 312, 298, 284, 271, 259, 496, 475, 456, 437, 420, 404,
        388, 374, 360, 347, 335, 323, 312, 302, 292, 282, 273, 265, 512, 497, 482, 468, 454,
        441, 428, 417, 405, 394, 383, 373, 364, 354, 345, 337, 328, 320, 312, 305, 298, 291,
        284, 278, 271, 265, 259, 507, 496, 485, 475, 465, 456, 446, 437, 428, 420, 412, 404,
        396, 388, 381, 374, 367, 360, 354, 347, 341, 335, 329, 323, 318, 312, 307, 302, 297,
        292, 287, 282, 278, 273, 269, 265, 261, 512, 505, 497, 489, 482, 475, 468, 461, 454,
        447, 441, 435, 428, 422, 417, 411, 405, 399, 394, 389, 383, 378, 373, 368, 364, 359,
        354, 350, 345, 341, 337, 332, 328, 324, 320, 316, 312, 309, 305, 301, 298, 294, 291,
        287, 284, 281, 278, 274, 271, 268, 265, 262, 259, 257, 507, 501, 496, 491, 485, 480,
        475, 470, 465, 460, 456, 451, 446, 442, 437, 433, 428, 424, 420, 416, 412, 408, 404,
        400, 396, 392, 388, 385, 381, 377, 374, 370, 367, 363, 360, 357, 354, 350, 347, 344,
        341, 338, 335, 332, 329, 326, 323, 320, 318, 315, 312, 310, 307, 304, 302, 299, 297,
        294, 292, 289, 287, 285, 282, 280, 278, 275, 273, 271, 269, 267, 265, 263, 261, 259 };

const sal_Int16 constShiftTable[255]
    = { 9,  11, 12, 13, 13, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17, 17, 17, 17, 17, 17, 17,
        18, 18, 18, 18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
        19, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21, 21, 21,
        21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
        21, 21, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
        22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23,
        23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
        23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
        23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
        24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
        24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
        24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24 };

struct BlurSharedData
{
    BitmapReadAccess* mpReadAccess;
    BitmapWriteAccess* mpWriteAccess;
    sal_Int32 mnRadius;
    sal_Int32 mnComponentWidth;
    sal_Int32 mnDiv;
    sal_Int32 mnColorChannels;

    BlurSharedData(BitmapReadAccess* pReadAccess, BitmapWriteAccess* pWriteAccess,
                   sal_Int32 aRadius, sal_Int32 nComponentWidth, sal_Int32 nColorChannels)
        : mpReadAccess(pReadAccess)
        , mpWriteAccess(pWriteAccess)
        , mnRadius(aRadius)
        , mnComponentWidth(nComponentWidth)
        , mnDiv(aRadius + aRadius + 1)
        , mnColorChannels(nColorChannels)
    {
    }
};

struct BlurArrays
{
    BlurSharedData maShared;

    std::vector<sal_uInt8> maStackBuffer;
    std::vector<sal_Int32> maPositionTable;
    std::vector<sal_Int32> maWeightTable;

    std::vector<sal_Int32> mnSumVector;
    std::vector<sal_Int32> mnInSumVector;
    std::vector<sal_Int32> mnOutSumVector;

    BlurArrays(BlurSharedData const& rShared)
        : maShared(rShared)
        , maStackBuffer(maShared.mnDiv * maShared.mnComponentWidth)
        , maPositionTable(maShared.mnDiv)
        , maWeightTable(maShared.mnDiv)
        , mnSumVector(maShared.mnColorChannels)
        , mnInSumVector(maShared.mnColorChannels)
        , mnOutSumVector(maShared.mnColorChannels)
    {
    }

    void initializeWeightAndPositions(sal_Int32 nLastIndex)
    {
        for (sal_Int32 i = 0; i < maShared.mnDiv; i++)
        {
            maPositionTable[i] = std::clamp(i - maShared.mnRadius, sal_Int32(0), nLastIndex);
            maWeightTable[i] = maShared.mnRadius + 1 - std::abs(i - maShared.mnRadius);
        }
    }

    sal_Int32 getMultiplyValue() const
    {
        return static_cast<sal_Int32>(constMultiplyTable[maShared.mnRadius]);
    }

    sal_Int32 getShiftValue() const
    {
        return static_cast<sal_Int32>(constShiftTable[maShared.mnRadius]);
    }
};

typedef void (*BlurRangeFn)(BlurSharedData const& rShared, sal_Int32 nStartY, sal_Int32 nEndY);

class BlurTask : public comphelper::ThreadTask
{
    BlurRangeFn mpBlurFunction;
    BlurSharedData& mrShared;
    sal_Int32 mnStartY;
    sal_Int32 mnEndY;

public:
    explicit BlurTask(const std::shared_ptr<comphelper::ThreadTaskTag>& pTag,
                      BlurRangeFn pBlurFunction, BlurSharedData& rShared, sal_Int32 nStartY,
                      sal_Int32 nEndY)
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
    static inline void add(sal_Int32*& pValue1, sal_Int32 nConstant)
    {
        pValue1[0] += nConstant;
        pValue1[1] += nConstant;
        pValue1[2] += nConstant;
    }

    static inline void set(sal_Int32*& pValue1, sal_Int32 nConstant)
    {
        pValue1[0] = nConstant;
        pValue1[1] = nConstant;
        pValue1[2] = nConstant;
    }

    static inline void add(sal_Int32*& pValue1, const sal_uInt8* pValue2)
    {
        pValue1[0] += pValue2[0];
        pValue1[1] += pValue2[1];
        pValue1[2] += pValue2[2];
    }

    static inline void add(sal_Int32*& pValue1, const sal_Int32* pValue2)
    {
        pValue1[0] += pValue2[0];
        pValue1[1] += pValue2[1];
        pValue1[2] += pValue2[2];
    }

    static inline void sub(sal_Int32*& pValue1, const sal_uInt8* pValue2)
    {
        pValue1[0] -= pValue2[0];
        pValue1[1] -= pValue2[1];
        pValue1[2] -= pValue2[2];
    }

    static inline void sub(sal_Int32*& pValue1, const sal_Int32* pValue2)
    {
        pValue1[0] -= pValue2[0];
        pValue1[1] -= pValue2[1];
        pValue1[2] -= pValue2[2];
    }

    static inline void assignPtr(sal_uInt8*& pValue1, const sal_uInt8* pValue2)
    {
        pValue1[0] = pValue2[0];
        pValue1[1] = pValue2[1];
        pValue1[2] = pValue2[2];
    }

    static inline void assignMulAndShr(sal_uInt8*& result, const sal_Int32* sum, sal_Int32 multiply,
                                       sal_Int32 shift)
    {
        result[0] = (multiply * sum[0]) >> shift;
        result[1] = (multiply * sum[1]) >> shift;
        result[2] = (multiply * sum[2]) >> shift;
    }
};

struct SumFunction8
{
    static inline void add(sal_Int32*& pValue1, sal_Int32 nConstant) { pValue1[0] += nConstant; }

    static inline void set(sal_Int32*& pValue1, sal_Int32 nConstant) { pValue1[0] = nConstant; }

    static inline void add(sal_Int32*& pValue1, const sal_uInt8* pValue2)
    {
        pValue1[0] += pValue2[0];
    }

    static inline void add(sal_Int32*& pValue1, const sal_Int32* pValue2)
    {
        pValue1[0] += pValue2[0];
    }

    static inline void sub(sal_Int32*& pValue1, const sal_uInt8* pValue2)
    {
        pValue1[0] -= pValue2[0];
    }

    static inline void sub(sal_Int32*& pValue1, const sal_Int32* pValue2)
    {
        pValue1[0] -= pValue2[0];
    }

    static inline void assignPtr(sal_uInt8*& pValue1, const sal_uInt8* pValue2)
    {
        pValue1[0] = pValue2[0];
    }

    static inline void assignMulAndShr(sal_uInt8*& result, const sal_Int32* sum, sal_Int32 multiply,
                                       sal_Int32 shift)
    {
        result[0] = (multiply * sum[0]) >> shift;
    }
};

template <typename SumFunction>
void stackBlurHorizontal(BlurSharedData const& rShared, sal_Int32 nStart, sal_Int32 nEnd)
{
    BitmapReadAccess* pReadAccess = rShared.mpReadAccess;
    BitmapWriteAccess* pWriteAccess = rShared.mpWriteAccess;

    BlurArrays aArrays(rShared);

    sal_uInt8* pStack = aArrays.maStackBuffer.data();
    sal_uInt8* pStackPtr;

    const sal_Int32 nWidth = pReadAccess->Width();
    const sal_Int32 nLastIndexX = nWidth - 1;

    const sal_Int32 nMultiplyValue = aArrays.getMultiplyValue();
    const sal_Int32 nShiftValue = aArrays.getShiftValue();

    const sal_Int32 nRadius = rShared.mnRadius;
    const sal_Int32 nComponentWidth = rShared.mnComponentWidth;
    const sal_Int32 nDiv = rShared.mnDiv;

    Scanline pSourcePointer;
    Scanline pDestinationPointer;

    aArrays.initializeWeightAndPositions(nLastIndexX);

    sal_Int32* nSum = aArrays.mnSumVector.data();
    sal_Int32* nInSum = aArrays.mnInSumVector.data();
    sal_Int32* nOutSum = aArrays.mnOutSumVector.data();

    sal_Int32* pPositionPointer = aArrays.maPositionTable.data();
    sal_Int32* pWeightPointer = aArrays.maWeightTable.data();

    for (sal_Int32 y = nStart; y <= nEnd; y++)
    {
        SumFunction::set(nSum, 0);
        SumFunction::set(nInSum, 0);
        SumFunction::set(nOutSum, 0);

        // Pre-initialize blur data for first pixel.
        // aArrays.maPositionTable contains values like (for radius of 5): [0,0,0,0,0,0,1,2,3,4,5],
        // which are used as pixels indices in the current row that we use to prepare information
        // for the first pixel; aArrays.maWeightTable has [1,2,3,4,5,6,5,4,3,2,1]. Before looking at
        // the first row pixel, we pretend to have processed fake previous pixels, as if the row was
        // extended to the left with the same color as that of the first pixel.
        for (sal_Int32 i = 0; i < nDiv; i++)
        {
            pSourcePointer = pReadAccess->GetScanline(y) + nComponentWidth * pPositionPointer[i];

            pStackPtr = &pStack[nComponentWidth * i];

            SumFunction::assignPtr(pStackPtr, pSourcePointer);

            SumFunction::add(nSum, pSourcePointer[0] * pWeightPointer[i]);

            if (i - nRadius > 0)
            {
                SumFunction::add(nInSum, pSourcePointer);
            }
            else
            {
                SumFunction::add(nOutSum, pSourcePointer);
            }
        }

        sal_Int32 nStackIndex = nRadius;
        sal_Int32 nXPosition = std::min(nRadius, nLastIndexX);

        pSourcePointer = pReadAccess->GetScanline(y) + nComponentWidth * nXPosition;

        for (sal_Int32 x = 0; x < nWidth; x++)
        {
            pDestinationPointer = pWriteAccess->GetScanline(y) + nComponentWidth * x;

            SumFunction::assignMulAndShr(pDestinationPointer, nSum, nMultiplyValue, nShiftValue);

            SumFunction::sub(nSum, nOutSum);

            sal_Int32 nStackIndexStart = nStackIndex + nDiv - nRadius;
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
void stackBlurVertical(BlurSharedData const& rShared, sal_Int32 nStart, sal_Int32 nEnd)
{
    BitmapReadAccess* pReadAccess = rShared.mpReadAccess;
    BitmapWriteAccess* pWriteAccess = rShared.mpWriteAccess;

    BlurArrays aArrays(rShared);

    sal_uInt8* pStack = aArrays.maStackBuffer.data();
    sal_uInt8* pStackPtr;

    sal_Int32 nHeight = pReadAccess->Height();
    sal_Int32 nLastIndexY = nHeight - 1;

    sal_Int32 nMultiplyValue = aArrays.getMultiplyValue();
    sal_Int32 nShiftValue = aArrays.getShiftValue();

    sal_Int32 nRadius = rShared.mnRadius;
    sal_Int32 nComponentWidth = rShared.mnComponentWidth;
    sal_Int32 nDiv = rShared.mnDiv;

    Scanline pSourcePointer;
    Scanline pDestinationPointer;

    aArrays.initializeWeightAndPositions(nLastIndexY);

    sal_Int32* nSum = aArrays.mnSumVector.data();
    sal_Int32* nInSum = aArrays.mnInSumVector.data();
    sal_Int32* nOutSum = aArrays.mnOutSumVector.data();
    sal_Int32* pPositionPointer = aArrays.maPositionTable.data();
    sal_Int32* pWeightPointer = aArrays.maWeightTable.data();

    for (sal_Int32 x = nStart; x <= nEnd; x++)
    {
        SumFunction::set(nSum, 0);
        SumFunction::set(nInSum, 0);
        SumFunction::set(nOutSum, 0);

        // Pre-initialize blur data for first pixel.
        // aArrays.maPositionTable contains values like (for radius of 5): [0,0,0,0,0,0,1,2,3,4,5],
        // which are used as pixels indices in the current column that we use to prepare information
        // for the first pixel; aArrays.maWeightTable has [1,2,3,4,5,6,5,4,3,2,1]. Before looking at
        // the first column pixels, we pretend to have processed fake previous pixels, as if the
        // column was extended to the top with the same color as that of the first pixel.
        for (sal_Int32 i = 0; i < nDiv; i++)
        {
            pSourcePointer = pReadAccess->GetScanline(pPositionPointer[i]) + nComponentWidth * x;

            pStackPtr = &pStack[nComponentWidth * i];

            SumFunction::assignPtr(pStackPtr, pSourcePointer);

            SumFunction::add(nSum, pSourcePointer[0] * pWeightPointer[i]);

            if (i - nRadius > 0)
            {
                SumFunction::add(nInSum, pSourcePointer);
            }
            else
            {
                SumFunction::add(nOutSum, pSourcePointer);
            }
        }

        sal_Int32 nStackIndex = nRadius;
        sal_Int32 nYPosition = std::min(nRadius, nLastIndexY);

        pSourcePointer = pReadAccess->GetScanline(nYPosition) + nComponentWidth * x;

        for (sal_Int32 y = 0; y < nHeight; y++)
        {
            pDestinationPointer = pWriteAccess->GetScanline(y) + nComponentWidth * x;

            SumFunction::assignMulAndShr(pDestinationPointer, nSum, nMultiplyValue, nShiftValue);

            SumFunction::sub(nSum, nOutSum);

            sal_Int32 nStackIndexStart = nStackIndex + nDiv - nRadius;

            if (nStackIndexStart >= nDiv)
                nStackIndexStart -= nDiv;

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
                nStackIndex = 0;

            pStackPtr = &pStack[nStackIndex * nComponentWidth];

            SumFunction::add(nOutSum, pStackPtr);
            SumFunction::sub(nInSum, pStackPtr);
        }
    }
}

constexpr sal_Int32 nThreadStrip = 16;

void runStackBlur(Bitmap& rBitmap, const sal_Int32 nRadius, const sal_Int32 nComponentWidth,
                  const sal_Int32 nColorChannels, BlurRangeFn pBlurHorizontalFn,
                  BlurRangeFn pBlurVerticalFn, const bool bParallel)
{
    if (bParallel)
    {
        try
        {
            comphelper::ThreadPool& rShared = comphelper::ThreadPool::getSharedOptimalPool();
            auto pTag = comphelper::ThreadPool::createThreadTaskTag();

            {
                BitmapScopedReadAccess pReadAccess(rBitmap);
                BitmapScopedWriteAccess pWriteAccess(rBitmap);
                BlurSharedData aSharedData(pReadAccess.get(), pWriteAccess.get(), nRadius,
                                           nComponentWidth, nColorChannels);

                const sal_Int32 nFirstIndex = 0;
                const sal_Int32 nLastIndex = pReadAccess->Height() - 1;

                vcl::bitmap::generateStripRanges<nThreadStrip>(
                    nFirstIndex, nLastIndex,
                    [&](sal_Int32 const nStart, sal_Int32 const nEnd, bool const bLast) {
                        if (!bLast)
                        {
                            auto pTask(std::make_unique<BlurTask>(pTag, pBlurHorizontalFn,
                                                                  aSharedData, nStart, nEnd));
                            rShared.pushTask(std::move(pTask));
                        }
                        else
                            pBlurHorizontalFn(aSharedData, nStart, nEnd);
                    });
                rShared.waitUntilDone(pTag);
            }
            {
                BitmapScopedReadAccess pReadAccess(rBitmap);
                BitmapScopedWriteAccess pWriteAccess(rBitmap);
                BlurSharedData aSharedData(pReadAccess.get(), pWriteAccess.get(), nRadius,
                                           nComponentWidth, nColorChannels);

                const sal_Int32 nFirstIndex = 0;
                const sal_Int32 nLastIndex = pReadAccess->Width() - 1;

                vcl::bitmap::generateStripRanges<nThreadStrip>(
                    nFirstIndex, nLastIndex,
                    [&](sal_Int32 const nStart, sal_Int32 const nEnd, bool const bLast) {
                        if (!bLast)
                        {
                            auto pTask(std::make_unique<BlurTask>(pTag, pBlurVerticalFn,
                                                                  aSharedData, nStart, nEnd));
                            rShared.pushTask(std::move(pTask));
                        }
                        else
                            pBlurVerticalFn(aSharedData, nStart, nEnd);
                    });

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
            BitmapScopedReadAccess pReadAccess(rBitmap);
            BitmapScopedWriteAccess pWriteAccess(rBitmap);
            BlurSharedData aSharedData(pReadAccess.get(), pWriteAccess.get(), nRadius,
                                       nComponentWidth, nColorChannels);
            sal_Int32 nFirstIndex = 0;
            sal_Int32 nLastIndex = pReadAccess->Height() - 1;
            pBlurHorizontalFn(aSharedData, nFirstIndex, nLastIndex);
        }
        {
            BitmapScopedReadAccess pReadAccess(rBitmap);
            BitmapScopedWriteAccess pWriteAccess(rBitmap);
            BlurSharedData aSharedData(pReadAccess.get(), pWriteAccess.get(), nRadius,
                                       nComponentWidth, nColorChannels);
            sal_Int32 nFirstIndex = 0;
            sal_Int32 nLastIndex = pReadAccess->Width() - 1;
            pBlurVerticalFn(aSharedData, nFirstIndex, nLastIndex);
        }
    }
}

void stackBlur24(Bitmap& rBitmap, sal_Int32 nRadius, sal_Int32 nComponentWidth)
{
    const bool bParallel = true;
    // Limit radius
    nRadius = std::clamp<sal_Int32>(nRadius, 2, 254);
    const sal_Int32 nColorChannels = 3; // 3 color channel

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
    const sal_Int32 nColorChannels = 1; // 1 color channel

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
    const Bitmap& aBitmap = rBitmapEx.GetBitmap();
    Bitmap result = filter(aBitmap);
    return BitmapEx(result, rBitmapEx.GetAlphaMask());
}

Bitmap BitmapFilterStackBlur::filter(Bitmap const& rBitmap) const
{
    Bitmap bitmapCopy(rBitmap);
    ScanlineFormat nScanlineFormat;
    {
        BitmapScopedReadAccess pReadAccess(bitmapCopy);
        nScanlineFormat = pReadAccess ? pReadAccess->GetScanlineFormat() : ScanlineFormat::NONE;
    }

    if (nScanlineFormat == ScanlineFormat::N24BitTcRgb
        || nScanlineFormat == ScanlineFormat::N24BitTcBgr
        || nScanlineFormat == ScanlineFormat::N32BitTcMask
        || nScanlineFormat == ScanlineFormat::N32BitTcBgra
        || nScanlineFormat == ScanlineFormat::N32BitTcBgrx)
    {
        int nComponentWidth = (nScanlineFormat == ScanlineFormat::N32BitTcMask
                               || nScanlineFormat == ScanlineFormat::N32BitTcBgra
                               || nScanlineFormat == ScanlineFormat::N32BitTcBgrx)
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
