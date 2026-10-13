/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <bitmap/BitmapScaleConvolutionRows.hxx>

#include <algorithm>

namespace vcl::bitmap::convolution
{
namespace
{
constexpr int32_t constRounding = 1 << (constWeightShift - 1);

uint8_t toByte(int32_t nValue)
{
    return uint8_t(std::clamp<int32_t>(nValue >> constWeightShift, 0, 255));
}

// The index of the first copy of tap nTap.
int32_t firstWeightIndex(int32_t nTap)
{
    const int32_t nInGroup = nTap % 4;
    return nTap / 4 * 16 + (nInGroup < 2 ? nInGroup : 8 + nInGroup - 2);
}
}

int32_t windowWeightCount(int32_t nWindow) { return (nWindow + 3) / 4 * 16; }

void spreadWindowWeights(const int16_t* pTaps, int32_t nWindow, int16_t* pWeights)
{
    std::fill_n(pWeights, windowWeightCount(nWindow), 0);
    for (int32_t i = 0; i < nWindow; i++)
    {
        const int32_t nFirst = firstWeightIndex(i);
        for (int32_t nRepeat = 0; nRepeat < 4; nRepeat++)
            pWeights[nFirst + nRepeat * 2] = pTaps[i];
    }
}

void convolveRowN32(const uint8_t* pSource, uint8_t* pDestination, int32_t nDestinationWidth,
                    const int16_t* pWeights, const int32_t* pStarts, int32_t nWindow)
{
    const int32_t nWeightCount = windowWeightCount(nWindow);
    for (int32_t x = 0; x < nDestinationWidth; x++)
    {
        const uint8_t* pPixel = pSource + pStarts[x] * 4;
        const int16_t* pWeight = pWeights + x * nWeightCount;
        int32_t nValue0 = constRounding;
        int32_t nValue1 = constRounding;
        int32_t nValue2 = constRounding;
        int32_t nValue3 = constRounding;

        auto addTap = [&](int32_t nWeight)
        {
            nValue0 += nWeight * pPixel[0];
            nValue1 += nWeight * pPixel[1];
            nValue2 += nWeight * pPixel[2];
            nValue3 += nWeight * pPixel[3];
            pPixel += 4;
        };

        // The 4 taps of a group start at 0, 1, 8 and 9.
        int32_t nTap = 0;
        for (; nTap + 4 <= nWindow; nTap += 4)
        {
            const int16_t* pGroup = pWeight + nTap * 4;
            addTap(pGroup[0]);
            addTap(pGroup[1]);
            addTap(pGroup[8]);
            addTap(pGroup[9]);
        }
        for (; nTap < nWindow; nTap++)
            addTap(pWeight[firstWeightIndex(nTap)]);

        uint8_t* pOut = pDestination + x * 4;
        pOut[0] = toByte(nValue0);
        pOut[1] = toByte(nValue1);
        pOut[2] = toByte(nValue2);
        pOut[3] = toByte(nValue3);
    }
}

void blendRowsN32(const uint8_t* const* pSourceRows, const int16_t* pWeights, int32_t nRowCount,
                  uint8_t* pDestination, int32_t nWidth)
{
    // Summing in chunks reads each source row front to back.
    constexpr int32_t constChunkSize = 256;
    int32_t aSums[constChunkSize];
    const int32_t nBytes = nWidth * 4;

    for (int32_t nChunkStart = 0; nChunkStart < nBytes; nChunkStart += constChunkSize)
    {
        const int32_t nChunkSize = std::min(constChunkSize, nBytes - nChunkStart);
        std::fill_n(aSums, nChunkSize, constRounding);

        for (int32_t j = 0; j < nRowCount; j++)
        {
            const int32_t nWeight = pWeights[j];
            const uint8_t* pRow = pSourceRows[j] + nChunkStart;
            for (int32_t i = 0; i < nChunkSize; i++)
                aSums[i] += nWeight * pRow[i];
        }

        for (int32_t i = 0; i < nChunkSize; i++)
            pDestination[nChunkStart + i] = toByte(aSums[i]);
    }
}

void boxHalveRowN32(const uint8_t* const* pSourceRows, uint8_t* pDestination,
                    int32_t nDestinationWidth)
{
    const uint8_t* pRow0 = pSourceRows[0];
    const uint8_t* pRow1 = pSourceRows[1];

    for (int32_t i = 0; i < nDestinationWidth * 4; i += 4)
    {
        for (int32_t c = 0; c < 4; c++)
        {
            const int32_t nSum
                = pRow0[2 * i + c] + pRow0[2 * i + 4 + c] + pRow1[2 * i + c] + pRow1[2 * i + 4 + c];
            pDestination[i + c] = uint8_t((nSum + 2) >> 2);
        }
    }
}

void boxQuarterRowN32(const uint8_t* const* pSourceRows, uint8_t* pDestination,
                      int32_t nDestinationWidth)
{
    for (int32_t i = 0; i < nDestinationWidth * 4; i += 4)
    {
        for (int32_t c = 0; c < 4; c++)
        {
            int32_t nSum = 0;
            for (int32_t nRow = 0; nRow < 4; nRow++)
            {
                const uint8_t* pBlock = pSourceRows[nRow] + 4 * i + c;
                nSum += pBlock[0] + pBlock[4] + pBlock[8] + pBlock[12];
            }
            pDestination[i + c] = uint8_t((nSum + 8) >> 4);
        }
    }
}

void expandRowN24ToN32(const uint8_t* pSource, uint8_t* pDestination, int32_t nWidth,
                       bool bSwapRedBlue)
{
    const int32_t nFirst = bSwapRedBlue ? 2 : 0;
    for (int32_t x = 0; x < nWidth; x++)
    {
        pDestination[x * 4] = pSource[x * 3 + nFirst];
        pDestination[x * 4 + 1] = pSource[x * 3 + 1];
        pDestination[x * 4 + 2] = pSource[x * 3 + 2 - nFirst];
        pDestination[x * 4 + 3] = 255;
    }
}

void packRowN32ToN24(const uint8_t* pSource, uint8_t* pDestination, int32_t nWidth)
{
    for (int32_t x = 0; x < nWidth; x++)
    {
        pDestination[x * 3] = pSource[x * 4];
        pDestination[x * 3 + 1] = pSource[x * 4 + 1];
        pDestination[x * 3 + 2] = pSource[x * 4 + 2];
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
