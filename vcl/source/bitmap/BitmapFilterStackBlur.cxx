/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/BitmapFilterStackBlur.hxx>
#include <vcl/bitmapaccess.hxx>

namespace
{

static const sal_Int16 constMultiplyTable[255] =
{
    512, 512, 456, 512, 328, 456, 335, 512, 405, 328, 271, 456, 388, 335, 292, 512,
    454, 405, 364, 328, 298, 271, 496, 456, 420, 388, 360, 335, 312, 292, 273, 512,
    482, 454, 428, 405, 383, 364, 345, 328, 312, 298, 284, 271, 259, 496, 475, 456,
    437, 420, 404, 388, 374, 360, 347, 335, 323, 312, 302, 292, 282, 273, 265, 512,
    497, 482, 468, 454, 441, 428, 417, 405, 394, 383, 373, 364, 354, 345, 337, 328,
    320, 312, 305, 298, 291, 284, 278, 271, 265, 259, 507, 496, 485, 475, 465, 456,
    446, 437, 428, 420, 412, 404, 396, 388, 381, 374, 367, 360, 354, 347, 341, 335,
    329, 323, 318, 312, 307, 302, 297, 292, 287, 282, 278, 273, 269, 265, 261, 512,
    505, 497, 489, 482, 475, 468, 461, 454, 447, 441, 435, 428, 422, 417, 411, 405,
    399, 394, 389, 383, 378, 373, 368, 364, 359, 354, 350, 345, 341, 337, 332, 328,
    324, 320, 316, 312, 309, 305, 301, 298, 294, 291, 287, 284, 281, 278, 274, 271,
    268, 265, 262, 259, 257, 507, 501, 496, 491, 485, 480, 475, 470, 465, 460, 456,
    451, 446, 442, 437, 433, 428, 424, 420, 416, 412, 408, 404, 400, 396, 392, 388,
    385, 381, 377, 374, 370, 367, 363, 360, 357, 354, 350, 347, 344, 341, 338, 335,
    332, 329, 326, 323, 320, 318, 315, 312, 310, 307, 304, 302, 299, 297, 294, 292,
    289, 287, 285, 282, 280, 278, 275, 273, 271, 269, 267, 265, 263, 261, 259
};

static const sal_Int16 constShiftTable[255] =
{
     9, 11, 12, 13, 13, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17,
    17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19,
    19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21,
    21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
    21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22,
    22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
    22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24
};

class BlurSharedData
{
public:
    long mnRadius;
    long mnComponentWidth;
    long mnColorChannels;
    long mnDiv;
    std::vector<sal_uInt8> maStackBuffer;
    std::vector<long> maPositionTable;
    std::vector<long> maWeightTable;

    std::vector<long> mnSumVector;
    std::vector<long> mnInSumVector;
    std::vector<long> mnOutSumVector;

    BlurSharedData(long aRadius, long nComponentWidth, long nColorChannels)
        : mnRadius(aRadius)
        , mnComponentWidth(nComponentWidth)
        , mnColorChannels(nColorChannels)
        , mnDiv(aRadius + aRadius + 1)
        , maStackBuffer(mnDiv * mnComponentWidth)
        , maPositionTable(mnDiv)
        , maWeightTable(mnDiv)
        , mnSumVector(mnColorChannels)
        , mnInSumVector(mnColorChannels)
        , mnOutSumVector(mnColorChannels)
    {
    }

    void calculateWeightAndPositions(long nLastIndex)
    {
        for (long i = 0; i < mnDiv; i++)
        {
            maPositionTable[i] = std::min(nLastIndex, std::max(0L, i - mnRadius));
            maWeightTable[i] = mnRadius + 1 - std::abs(i - mnRadius);
        }
    }

    long getMultiplyValue()
    {
        return static_cast<long>(constMultiplyTable[mnRadius]);
    }

    long getShiftValue()
    {
        return static_cast<long>(constShiftTable[mnRadius]);
    }
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
    static inline void add(long*& pValue1, long nConstant)
    {
        pValue1[0] += nConstant;
    }

    static inline void set(long*& pValue1, long nConstant)
    {
        pValue1[0] = nConstant;
    }

    static inline void add(long*& pValue1, sal_uInt8*& pValue2)
    {
        pValue1[0] += pValue2[0];
    }

    static inline void add(long*& pValue1, long*& pValue2)
    {
        pValue1[0] += pValue2[0];
    }

    static inline void sub(long*& pValue1, sal_uInt8*& pValue2)
    {
        pValue1[0] -= pValue2[0];
    }

    static inline void sub(long*& pValue1, long*& pValue2)
    {
        pValue1[0] -= pValue2[0];
    }

    static inline void assignPtr(sal_uInt8*& pValue1, sal_uInt8*& pValue2)
    {
        pValue1[0] = pValue2[0];
    }

    static inline void assignMulAndShr(sal_uInt8*& result, long*& sum, long multiply, long shift)
    {
        result[0] = (multiply * sum[0]) >> shift;
    }
};

template<typename SumFunction>
void stackBlurHorizontal(
        BitmapReadAccess* pReadAccess,
        BitmapWriteAccess* pWriteAccess,
        BlurSharedData& rShared)
{
    long nWidth = pReadAccess->Width();
    long nHeight = pReadAccess->Height();

    sal_uInt8* pStack = rShared.maStackBuffer.data();
    sal_uInt8* pStackPtr;

    long nLastIndexX = nWidth - 1;

    long nMultiplyValue = rShared.getMultiplyValue();
    long nShiftValue = rShared.getShiftValue();

    long nRadius = rShared.mnRadius;
    long nComponentWidth = rShared.mnComponentWidth;
    long nDiv = rShared.mnDiv;

    Scanline pSourcePointer;
    Scanline pDestinationPointer;

    long nXPosition;
    long nStackIndex;
    long nStackIndexStart;
    long nWeight;

    long* nSum = rShared.mnSumVector.data();
    long* nInSum = rShared.mnInSumVector.data();
    long* nOutSum = rShared.mnOutSumVector.data();

    rShared.calculateWeightAndPositions(nLastIndexX);
    long* pPositionPointer = rShared.maPositionTable.data();
    long* pWeightPointer = rShared.maWeightTable.data();

    for (long y = 0; y < nHeight; y++)
    {
        SumFunction::set(nSum, 0L);
        SumFunction::set(nInSum, 0L);
        SumFunction::set(nOutSum, 0L);

        for (long i = 0; i < nDiv; i++)
        {
            pSourcePointer = pReadAccess->GetScanline(pPositionPointer[i]);

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

template<typename SumFunction>
void stackBlurVertical(
        BitmapReadAccess* pReadAccess,
        BitmapWriteAccess* pWriteAccess,
        BlurSharedData& rShared)
{
    long nWidth = pReadAccess->Width();
    long nHeight = pReadAccess->Height();

    sal_uInt8* pStack = rShared.maStackBuffer.data();
    sal_uInt8* pStackPtr;

    long nLastIndexY = nHeight - 1;

    long nMultiplyValue = rShared.getMultiplyValue();
    long nShiftValue = rShared.getShiftValue();

    long nRadius = rShared.mnRadius;
    long nComponentWidth = rShared.mnComponentWidth;
    long nDiv = rShared.mnDiv;

    Scanline pSourcePointer;
    Scanline pDestinationPointer;

    long nYPosition;
    long nStackIndex;
    long nStackIndexStart;
    long nWeight;

    long* nSum = rShared.mnSumVector.data();
    long* nInSum = rShared.mnInSumVector.data();
    long* nOutSum = rShared.mnOutSumVector.data();

    rShared.calculateWeightAndPositions(nLastIndexY);
    long* pPositionPointer = rShared.maPositionTable.data();
    long* pWeightPointer = rShared.maWeightTable.data();

    for (long x = 0; x < nWidth; x++)
    {
        SumFunction::set(nSum, 0L);
        SumFunction::set(nInSum, 0L);
        SumFunction::set(nOutSum, 0L);

        for (long i = 0; i < nDiv; i++)
        {
            pSourcePointer = pReadAccess->GetScanline(pPositionPointer[i]);

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

void stackBlur24(Bitmap& rBitmap, sal_Int32 nRadius, sal_Int32 nComponentWidth)
{
    // Limit radius
    nRadius = std::min<sal_Int32>(254, std::max<sal_Int32>(2, nRadius));
    const long nColorChannels = 3; // 3 color channel
    BlurSharedData aData(nRadius, nComponentWidth, nColorChannels);

    {
        Bitmap::ScopedReadAccess pReadAccess(rBitmap);
        Bitmap::ScopedWriteAccess pWriteAccess(rBitmap);

        stackBlurHorizontal<SumFunction24>(pReadAccess.get(), pWriteAccess.get(), aData);
    }

    {
        Bitmap::ScopedReadAccess pReadAccess(rBitmap);
        Bitmap::ScopedWriteAccess pWriteAccess(rBitmap);

        stackBlurVertical<SumFunction24>(pReadAccess.get(), pWriteAccess.get(), aData);
    }
}

void stackBlur8(Bitmap& rBitmap, sal_Int32 nRadius, sal_Int32 nComponentWidth)
{
    // Limit radius
    nRadius = std::min<sal_Int32>(254, std::max<sal_Int32>(2, nRadius));
    const long nColorChannels = 1; // 1 color channel
    BlurSharedData aData(nRadius, nComponentWidth, nColorChannels);

    {
        Bitmap::ScopedReadAccess pReadAccess(rBitmap);
        Bitmap::ScopedWriteAccess pWriteAccess(rBitmap);

        stackBlurHorizontal<SumFunction8>(pReadAccess.get(), pWriteAccess.get(), aData);
    }

    {
        Bitmap::ScopedReadAccess pReadAccess(rBitmap);
        Bitmap::ScopedWriteAccess pWriteAccess(rBitmap);

        stackBlurVertical<SumFunction8>(pReadAccess.get(), pWriteAccess.get(), aData);
    }
}

void centerExtendBitmap(Bitmap& rBitmap, sal_Int32 nExtendSize, Color aColor)
{
    const Size& rSize = rBitmap.GetSizePixel();
    const Size aNewSize(rSize.Width()  + nExtendSize * 2,
                        rSize.Height() + nExtendSize * 2);

    Bitmap aNewBitmap(aNewSize, rBitmap.GetBitCount());

    {
        Bitmap::ScopedReadAccess pReadAccess(rBitmap);
        Bitmap::ScopedWriteAccess pWriteAccess(aNewBitmap);

        long nWidthBorder  = nExtendSize + rSize.Width();
        long nHeightBorder = nExtendSize + rSize.Height();

        for (long y = 0; y < aNewSize.Height(); y++)
        {
            for (long x = 0; x < aNewSize.Width(); x++)
            {
                if (y < nExtendSize || y >= nHeightBorder
                 || x < nExtendSize || x >= nWidthBorder)
                {
                    pWriteAccess->SetPixel(y, x, aColor);
                }
                else
                {
                    pWriteAccess->SetPixel(y, x, pReadAccess->GetPixel(y - nExtendSize, x - nExtendSize));
                }
            }
        }
    }
    rBitmap = aNewBitmap;
}

} // end anonymous namespace

/**
 * Implementation of stack blur - a fast Gaussian blur approximation.
 * nRadius - blur radious, valid values are between 2 and 254
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
BitmapFilterStackBlur::BitmapFilterStackBlur(sal_Int32 nRadius, bool bExtend)
    : mnRadius(nRadius)
    , mbExtend(bExtend)
{}

BitmapFilterStackBlur::~BitmapFilterStackBlur()
{}

bool BitmapFilterStackBlur::filter(Bitmap& rBitmap)
{
    sal_uLong nScanlineFormat;
    {
        Bitmap::ScopedReadAccess pReadAccess(rBitmap);
        nScanlineFormat = pReadAccess->GetScanlineFormat();
    }

    if (nScanlineFormat == BMP_FORMAT_24BIT_TC_RGB ||
        nScanlineFormat == BMP_FORMAT_24BIT_TC_BGR ||
        nScanlineFormat == BMP_FORMAT_32BIT_TC_MASK)
    {
        int nComponentWidth = (nScanlineFormat == BMP_FORMAT_32BIT_TC_MASK) ? 4 : 3;

        if (mbExtend)
        {
            centerExtendBitmap(rBitmap, mnRadius, COL_WHITE);
        }

        stackBlur24(rBitmap, mnRadius, nComponentWidth);
    }
    else if (nScanlineFormat == BMP_FORMAT_8BIT_PAL)
    {
        int nComponentWidth = 1;

        if (mbExtend)
        {
            centerExtendBitmap(rBitmap, mnRadius, COL_WHITE);
        }

        stackBlur8(rBitmap, mnRadius, nComponentWidth);
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
