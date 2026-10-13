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

// Built with AVX2. Every inline function this file defines is internal to it, so each inline
// function in vcl has one definition and the rest of vcl runs on any x86-64 CPU.

#include <bitmap/BitmapScaleConvolutionRows.hxx>
#include <tools/simdsupport.hxx>

#ifdef LO_AVX2_AVAILABLE

namespace vcl::bitmap::convolution
{
namespace
{
constexpr int32_t constRounding = 1 << (constWeightShift - 1);
}

LO_FORCE_INLINE uint8_t toByte(int32_t nValue)
{
    nValue >>= constWeightShift;
    return uint8_t(nValue < 0 ? 0 : (nValue > 255 ? 255 : nValue));
}

void convolveRowN32AVX2(const uint8_t* pSource, uint8_t* pDestination, int32_t nDestinationWidth,
                        const int16_t* pWeights, const int32_t* pStarts, int32_t nWindow)
{
    // Interleave the channels of neighboring pixels, so one multiply-add weights both.
    const __m128i aPairShuffle
        = _mm_setr_epi8(0, 4, 1, 5, 2, 6, 3, 7, 8, 12, 9, 13, 10, 14, 11, 15);
    const __m256i aRounding = _mm256_set1_epi32(constRounding);
    const int32_t nGroups = nWindow / 4;
    const int32_t nWeightCount = nGroups * 16;

    // Channel sums of pixels 0 and 1 go to the low half, of pixels 2 and 3 to the high half.
    auto multiplyGroup = [&](const uint8_t* pPixels, const int16_t* pGroupWeights)
    {
        const __m128i aPixels = _mm_shuffle_epi8(
            _mm_loadu_si128(reinterpret_cast<const __m128i*>(pPixels)), aPairShuffle);
        return _mm256_madd_epi16(
            _mm256_cvtepu8_epi16(aPixels),
            _mm256_loadu_si256(reinterpret_cast<const __m256i*>(pGroupWeights)));
    };

    // Two destination pixels per step share the rounding and the store.
    int32_t x = 0;
    for (; x + 2 <= nDestinationWidth; x += 2)
    {
        const uint8_t* pPixelsA = pSource + pStarts[x] * 4;
        const uint8_t* pPixelsB = pSource + pStarts[x + 1] * 4;
        const int16_t* pWeightsA = pWeights + x * nWeightCount;
        const int16_t* pWeightsB = pWeightsA + nWeightCount;
        __m256i aSumA = _mm256_setzero_si256();
        __m256i aSumB = _mm256_setzero_si256();
        for (int32_t nGroup = 0; nGroup < nGroups; nGroup++)
        {
            aSumA = _mm256_add_epi32(
                aSumA, multiplyGroup(pPixelsA + nGroup * 16, pWeightsA + nGroup * 16));
            aSumB = _mm256_add_epi32(
                aSumB, multiplyGroup(pPixelsB + nGroup * 16, pWeightsB + nGroup * 16));
        }

        // Pixel A to the low half, pixel B to the high half.
        __m256i aBoth = _mm256_add_epi32(_mm256_permute2x128_si256(aSumA, aSumB, 0x20),
                                         _mm256_permute2x128_si256(aSumA, aSumB, 0x31));
        aBoth = _mm256_srai_epi32(_mm256_add_epi32(aBoth, aRounding), constWeightShift);
        // The saturating packs clamp each channel to 0..255.
        const __m128i aWords
            = _mm_packs_epi32(_mm256_castsi256_si128(aBoth), _mm256_extracti128_si256(aBoth, 1));
        _mm_storel_epi64(reinterpret_cast<__m128i*>(pDestination + x * 4),
                         _mm_packus_epi16(aWords, aWords));
    }

    if (x < nDestinationWidth)
    {
        const uint8_t* pPixels = pSource + pStarts[x] * 4;
        const int16_t* pPixelWeights = pWeights + x * nWeightCount;
        __m256i aSum = _mm256_setzero_si256();
        for (int32_t nGroup = 0; nGroup < nGroups; nGroup++)
        {
            aSum = _mm256_add_epi32(
                aSum, multiplyGroup(pPixels + nGroup * 16, pPixelWeights + nGroup * 16));
        }
        __m128i aSum4
            = _mm_add_epi32(_mm256_castsi256_si128(aSum), _mm256_extracti128_si256(aSum, 1));
        aSum4 = _mm_srai_epi32(_mm_add_epi32(aSum4, _mm256_castsi256_si128(aRounding)),
                               constWeightShift);
        const __m128i aWords = _mm_packs_epi32(aSum4, aSum4);
        _mm_storeu_si32(pDestination + x * 4, _mm_packus_epi16(aWords, aWords));
    }
}

void blendRowsN32AVX2(const uint8_t* const* pSourceRows, const int16_t* pWeights, int32_t nRowCount,
                      uint8_t* pDestination, int32_t nWidth)
{
    const __m256i aRounding = _mm256_set1_epi32(constRounding);
    const __m256i aZero = _mm256_setzero_si256();
    const int32_t nBytes = nWidth * 4;

    int32_t nOffset = 0;
    for (; nOffset + 32 <= nBytes; nOffset += 32)
    {
        // aSum0 holds bytes 0-3 and 16-19, aSum1 bytes 4-7 and 20-23, and so on.
        __m256i aSum0 = aRounding;
        __m256i aSum1 = aRounding;
        __m256i aSum2 = aRounding;
        __m256i aSum3 = aRounding;

        // Interleave two rows, so one multiply-add weights both. An odd last row pairs with zeros.
        for (int32_t j = 0; j < nRowCount; j += 2)
        {
            const bool bPair = j + 1 < nRowCount;
            const __m256i aRowA
                = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(pSourceRows[j] + nOffset));
            const __m256i aRowB = bPair ? _mm256_loadu_si256(reinterpret_cast<const __m256i*>(
                                              pSourceRows[j + 1] + nOffset))
                                        : aZero;
            const uint32_t nWeightA = uint16_t(pWeights[j]);
            const uint32_t nWeightB = bPair ? uint16_t(pWeights[j + 1]) : 0;
            const __m256i aWeightPair = _mm256_set1_epi32(int32_t(nWeightA | (nWeightB << 16)));

            const __m256i aLow = _mm256_unpacklo_epi8(aRowA, aRowB);
            const __m256i aHigh = _mm256_unpackhi_epi8(aRowA, aRowB);
            aSum0 = _mm256_add_epi32(
                aSum0, _mm256_madd_epi16(_mm256_unpacklo_epi8(aLow, aZero), aWeightPair));
            aSum1 = _mm256_add_epi32(
                aSum1, _mm256_madd_epi16(_mm256_unpackhi_epi8(aLow, aZero), aWeightPair));
            aSum2 = _mm256_add_epi32(
                aSum2, _mm256_madd_epi16(_mm256_unpacklo_epi8(aHigh, aZero), aWeightPair));
            aSum3 = _mm256_add_epi32(
                aSum3, _mm256_madd_epi16(_mm256_unpackhi_epi8(aHigh, aZero), aWeightPair));
        }

        aSum0 = _mm256_srai_epi32(aSum0, constWeightShift);
        aSum1 = _mm256_srai_epi32(aSum1, constWeightShift);
        aSum2 = _mm256_srai_epi32(aSum2, constWeightShift);
        aSum3 = _mm256_srai_epi32(aSum3, constWeightShift);
        // The packs work per 16-byte half, so the bytes stay in order. Saturation clamps to 0..255.
        const __m256i aBytes = _mm256_packus_epi16(_mm256_packs_epi32(aSum0, aSum1),
                                                   _mm256_packs_epi32(aSum2, aSum3));
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(pDestination + nOffset), aBytes);
    }

    for (; nOffset < nBytes; nOffset++)
    {
        int32_t nSum = constRounding;
        for (int32_t j = 0; j < nRowCount; j++)
            nSum += pWeights[j] * pSourceRows[j][nOffset];
        pDestination[nOffset] = toByte(nSum);
    }
}

void boxHalveRowN32AVX2(const uint8_t* const* pSourceRows, uint8_t* pDestination,
                        int32_t nDestinationWidth)
{
    const uint8_t* pRow0 = pSourceRows[0];
    const uint8_t* pRow1 = pSourceRows[1];
    const __m256i aZero = _mm256_setzero_si256();
    const __m256i aTwo = _mm256_set1_epi16(2);

    // Averages 8 pixels of both rows to 4 pixels of 16-bit channels: 0 and 1 in the low half, 2
    // and 3 in the high half.
    auto sumBlocks = [&](const uint8_t* pFirst, const uint8_t* pSecond)
    {
        const __m256i aFirst = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(pFirst));
        const __m256i aSecond = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(pSecond));
        // Source pixels 0 and 1 (low half) and 4 and 5 (high half).
        const __m256i aLow = _mm256_add_epi16(_mm256_unpacklo_epi8(aFirst, aZero),
                                              _mm256_unpacklo_epi8(aSecond, aZero));
        // Source pixels 2 and 3 (low half) and 6 and 7 (high half).
        const __m256i aHigh = _mm256_add_epi16(_mm256_unpackhi_epi8(aFirst, aZero),
                                               _mm256_unpackhi_epi8(aSecond, aZero));
        const __m256i aLowPair = _mm256_add_epi16(aLow, _mm256_srli_si256(aLow, 8));
        const __m256i aHighPair = _mm256_add_epi16(aHigh, _mm256_srli_si256(aHigh, 8));
        return _mm256_srli_epi16(_mm256_add_epi16(_mm256_unpacklo_epi64(aLowPair, aHighPair), aTwo),
                                 2);
    };

    int32_t x = 0;
    for (; x + 8 <= nDestinationWidth; x += 8)
    {
        const __m256i aFirst = sumBlocks(pRow0 + x * 8, pRow1 + x * 8);
        const __m256i aSecond = sumBlocks(pRow0 + x * 8 + 32, pRow1 + x * 8 + 32);
        // The pack gives the order 0 1 4 5 2 3 6 7, the permute restores it.
        const __m256i aBytes = _mm256_permute4x64_epi64(_mm256_packus_epi16(aFirst, aSecond),
                                                        _MM_SHUFFLE(3, 1, 2, 0));
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(pDestination + x * 4), aBytes);
    }

    for (int32_t i = x * 4; i < nDestinationWidth * 4; i += 4)
    {
        for (int32_t c = 0; c < 4; c++)
        {
            const int32_t nSum
                = pRow0[2 * i + c] + pRow0[2 * i + 4 + c] + pRow1[2 * i + c] + pRow1[2 * i + 4 + c];
            pDestination[i + c] = uint8_t((nSum + 2) >> 2);
        }
    }
}

void boxQuarterRowN32AVX2(const uint8_t* const* pSourceRows, uint8_t* pDestination,
                          int32_t nDestinationWidth)
{
    const __m256i aZero = _mm256_setzero_si256();
    const __m256i aEight = _mm256_set1_epi16(8);

    // Sums 8 pixels of all 4 rows to 2 pixels of 16-bit channels, in the low 8 bytes of each half.
    auto sumBlocks = [&](int32_t nOffset)
    {
        __m256i aLow = aZero;
        __m256i aHigh = aZero;
        for (int32_t nRow = 0; nRow < 4; nRow++)
        {
            const __m256i aPixels
                = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(pSourceRows[nRow] + nOffset));
            aLow = _mm256_add_epi16(aLow, _mm256_unpacklo_epi8(aPixels, aZero));
            aHigh = _mm256_add_epi16(aHigh, _mm256_unpackhi_epi8(aPixels, aZero));
        }
        const __m256i aSum = _mm256_add_epi16(aLow, aHigh);
        return _mm256_add_epi16(aSum, _mm256_srli_si256(aSum, 8));
    };

    int32_t x = 0;
    for (; x + 4 <= nDestinationWidth; x += 4)
    {
        // The unpack gives the order 0 2 1 3, the permute restores it.
        __m256i aSums = _mm256_unpacklo_epi64(sumBlocks(x * 16), sumBlocks(x * 16 + 32));
        aSums = _mm256_srli_epi16(_mm256_add_epi16(aSums, aEight), 4);
        aSums = _mm256_permute4x64_epi64(aSums, _MM_SHUFFLE(3, 1, 2, 0));
        const __m128i aBytes
            = _mm_packus_epi16(_mm256_castsi256_si128(aSums), _mm256_extracti128_si256(aSums, 1));
        _mm_storeu_si128(reinterpret_cast<__m128i*>(pDestination + x * 4), aBytes);
    }

    for (int32_t i = x * 4; i < nDestinationWidth * 4; i += 4)
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

void expandRowN24ToN32AVX2(const uint8_t* pSource, uint8_t* pDestination, int32_t nWidth,
                           bool bSwapRedBlue)
{
    // Spreads 4 pixels from 3 to 4 bytes. aOpaque sets the alpha.
    const __m128i aSpread
        = bSwapRedBlue ? _mm_setr_epi8(2, 1, 0, -1, 5, 4, 3, -1, 8, 7, 6, -1, 11, 10, 9, -1)
                       : _mm_setr_epi8(0, 1, 2, -1, 3, 4, 5, -1, 6, 7, 8, -1, 9, 10, 11, -1);
    const __m128i aOpaque = _mm_set1_epi32(int32_t(0xff000000));

    // Each step reads 16 bytes and uses 12, so stop 2 pixels before the end.
    int32_t x = 0;
    for (; x + 6 <= nWidth; x += 4)
    {
        const __m128i aPixels = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pSource + x * 3));
        _mm_storeu_si128(reinterpret_cast<__m128i*>(pDestination + x * 4),
                         _mm_or_si128(_mm_shuffle_epi8(aPixels, aSpread), aOpaque));
    }
    expandRowN24ToN32(pSource + x * 3, pDestination + x * 4, nWidth - x, bSwapRedBlue);
}

void packRowN32ToN24AVX2(const uint8_t* pSource, uint8_t* pDestination, int32_t nWidth)
{
    const __m128i aPack = _mm_setr_epi8(0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14, -1, -1, -1, -1);

    // Each step writes 16 bytes and the next one overwrites the last 4, so stop 2 pixels before
    // the end.
    int32_t x = 0;
    for (; x + 6 <= nWidth; x += 4)
    {
        const __m128i aPixels = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pSource + x * 4));
        _mm_storeu_si128(reinterpret_cast<__m128i*>(pDestination + x * 3),
                         _mm_shuffle_epi8(aPixels, aPack));
    }
    packRowN32ToN24(pSource + x * 4, pDestination + x * 3, nWidth - x);
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
