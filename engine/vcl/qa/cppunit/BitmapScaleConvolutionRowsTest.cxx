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

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <tools/cpuid.hxx>

#include <bitmap/BitmapScaleConvolutionRows.hxx>

#include <algorithm>
#include <random>
#include <string>
#include <vector>

namespace
{
/// Repeatable pseudo-random test data.
class TestData
{
    std::mt19937 maRandom{ 1234 };

public:
    sal_Int32 next(sal_Int32 nMin, sal_Int32 nMax)
    {
        return nMin + sal_Int32(maRandom() % (nMax - nMin + 1));
    }

    std::vector<sal_uInt8> bytes(sal_Int32 nCount)
    {
        std::vector<sal_uInt8> aBytes(nCount);
        for (sal_uInt8& rByte : aBytes)
            rByte = sal_uInt8(next(0, 255));
        return aBytes;
    }

    // The range of the scaling filter weights, negative lobes included.
    sal_Int16 weight() { return sal_Int16(next(-6000, 18000)); }
};

/// One row for the horizontal pass. The widths vary to cover every tail of the vector loops.
struct ConvolveCase
{
    std::vector<sal_uInt8> maSource;
    // mnWindow taps per destination pixel.
    std::vector<sal_Int16> maTaps;
    // The same taps in the kernel layout.
    std::vector<sal_Int16> maWeights;
    std::vector<int32_t> maStarts;
    sal_Int32 mnWindow;
    sal_Int32 mnDestinationWidth;

    // bWholeGroups makes the window a multiple of 4 taps.
    ConvolveCase(TestData& rData, bool bWholeGroups)
    {
        mnWindow = bWholeGroups ? 4 * rData.next(1, 10) : rData.next(1, 40);
        const sal_Int32 nSourceWidth = mnWindow + rData.next(0, 300);
        const sal_Int32 nWeightCount = vcl::bitmap::convolution::windowWeightCount(mnWindow);
        mnDestinationWidth = rData.next(1, 300);
        maSource = rData.bytes(nSourceWidth * 4);
        maWeights.resize(mnDestinationWidth * nWeightCount);
        for (sal_Int32 x = 0; x < mnDestinationWidth; x++)
        {
            maStarts.push_back(rData.next(0, nSourceWidth - mnWindow));
            for (sal_Int32 i = 0; i < mnWindow; i++)
                maTaps.push_back(rData.weight());
            vcl::bitmap::convolution::spreadWindowWeights(maTaps.data() + x * mnWindow, mnWindow,
                                                          maWeights.data() + x * nWeightCount);
        }
    }

    template <typename Function> std::vector<sal_uInt8> run(Function pFunction) const
    {
        std::vector<sal_uInt8> aResult(mnDestinationWidth * 4);
        pFunction(maSource.data(), aResult.data(), mnDestinationWidth, maWeights.data(),
                  maStarts.data(), mnWindow);
        return aResult;
    }
};

/// Source rows for the vertical pass and for the block averages.
struct RowsCase
{
    std::vector<std::vector<sal_uInt8>> maRows;
    std::vector<const sal_uInt8*> maRowPointers;
    std::vector<sal_Int16> maWeights;
    sal_Int32 mnWidth;

    RowsCase(TestData& rData, sal_Int32 nRowCount, sal_Int32 nWidth)
        : mnWidth(nWidth)
    {
        for (sal_Int32 j = 0; j < nRowCount; j++)
        {
            maRows.push_back(rData.bytes(nWidth * 4));
            maWeights.push_back(rData.weight());
        }
        for (const std::vector<sal_uInt8>& rRow : maRows)
            maRowPointers.push_back(rRow.data());
    }

    sal_Int32 byte(sal_Int32 nRow, sal_Int32 nPixel, sal_Int32 nChannel) const
    {
        return maRows[nRow][nPixel * 4 + nChannel];
    }
};

sal_uInt8 roundedWeightedSum(sal_Int64 nSum)
{
    const sal_Int64 nValue = (nSum + (1 << (vcl::bitmap::convolution::constWeightShift - 1)))
                             >> vcl::bitmap::convolution::constWeightShift;
    return sal_uInt8(std::clamp<sal_Int64>(nValue, 0, 255));
}

std::vector<sal_uInt8> expectedConvolve(const ConvolveCase& rCase)
{
    std::vector<sal_uInt8> aResult;
    for (sal_Int32 x = 0; x < rCase.mnDestinationWidth; x++)
    {
        for (sal_Int32 c = 0; c < 4; c++)
        {
            sal_Int64 nSum = 0;
            for (sal_Int32 i = 0; i < rCase.mnWindow; i++)
            {
                nSum += rCase.maTaps[x * rCase.mnWindow + i]
                        * rCase.maSource[(rCase.maStarts[x] + i) * 4 + c];
            }
            aResult.push_back(roundedWeightedSum(nSum));
        }
    }
    return aResult;
}

std::vector<sal_uInt8> expectedBlend(const RowsCase& rCase)
{
    std::vector<sal_uInt8> aResult;
    for (sal_Int32 nByte = 0; nByte < rCase.mnWidth * 4; nByte++)
    {
        sal_Int64 nSum = 0;
        for (size_t j = 0; j < rCase.maRows.size(); j++)
            nSum += rCase.maWeights[j] * rCase.maRows[j][nByte];
        aResult.push_back(roundedWeightedSum(nSum));
    }
    return aResult;
}

std::vector<sal_uInt8> expectedBlockAverage(const RowsCase& rCase, sal_Int32 nFactor,
                                            sal_Int32 nDestinationWidth)
{
    std::vector<sal_uInt8> aResult;
    for (sal_Int32 x = 0; x < nDestinationWidth; x++)
    {
        for (sal_Int32 c = 0; c < 4; c++)
        {
            sal_Int32 nSum = 0;
            for (sal_Int32 nRow = 0; nRow < nFactor; nRow++)
                for (sal_Int32 i = 0; i < nFactor; i++)
                    nSum += rCase.byte(nRow, x * nFactor + i, c);
            aResult.push_back((nSum + nFactor * nFactor / 2) / (nFactor * nFactor));
        }
    }
    return aResult;
}

std::vector<sal_uInt8> expectedExpand(const std::vector<sal_uInt8>& rSource, bool bSwapRedBlue)
{
    std::vector<sal_uInt8> aResult;
    for (size_t i = 0; i < rSource.size(); i += 3)
    {
        aResult.push_back(rSource[bSwapRedBlue ? i + 2 : i]);
        aResult.push_back(rSource[i + 1]);
        aResult.push_back(rSource[bSwapRedBlue ? i : i + 2]);
        aResult.push_back(255);
    }
    return aResult;
}

std::vector<sal_uInt8> expectedPack(const std::vector<sal_uInt8>& rSource)
{
    std::vector<sal_uInt8> aResult;
    for (size_t i = 0; i < rSource.size(); i += 4)
        aResult.insert(aResult.end(), rSource.begin() + i, rSource.begin() + i + 3);
    return aResult;
}

std::vector<sal_uInt8> runExpand(const std::vector<sal_uInt8>& rSource, bool bSwapRedBlue,
                                 void (*pFunction)(const uint8_t*, uint8_t*, int32_t, bool))
{
    const sal_Int32 nWidth = sal_Int32(rSource.size() / 3);
    std::vector<sal_uInt8> aResult(nWidth * 4);
    pFunction(rSource.data(), aResult.data(), nWidth, bSwapRedBlue);
    return aResult;
}

std::vector<sal_uInt8> runPack(const std::vector<sal_uInt8>& rSource,
                               void (*pFunction)(const uint8_t*, uint8_t*, int32_t))
{
    const sal_Int32 nWidth = sal_Int32(rSource.size() / 4);
    std::vector<sal_uInt8> aResult(nWidth * 3);
    pFunction(rSource.data(), aResult.data(), nWidth);
    return aResult;
}

std::vector<sal_uInt8> runBlend(const RowsCase& rCase,
                                void (*pFunction)(const uint8_t* const*, const int16_t*, int32_t,
                                                  uint8_t*, int32_t))
{
    std::vector<sal_uInt8> aResult(rCase.mnWidth * 4);
    pFunction(rCase.maRowPointers.data(), rCase.maWeights.data(), sal_Int32(rCase.maRows.size()),
              aResult.data(), rCase.mnWidth);
    return aResult;
}

std::vector<sal_uInt8> runBlockAverage(const RowsCase& rCase, sal_Int32 nDestinationWidth,
                                       void (*pFunction)(const uint8_t* const*, uint8_t*,
                                                         int32_t))
{
    std::vector<sal_uInt8> aResult(nDestinationWidth * 4);
    pFunction(rCase.maRowPointers.data(), aResult.data(), nDestinationWidth);
    return aResult;
}

void assertSameBytes(const std::vector<sal_uInt8>& rExpected, const std::vector<sal_uInt8>& rActual,
                     const std::string& rKernel, sal_Int32 nCase)
{
    CPPUNIT_ASSERT_EQUAL(rExpected.size(), rActual.size());
    const auto aMismatch = std::mismatch(rExpected.begin(), rExpected.end(), rActual.begin());
    if (aMismatch.first == rExpected.end())
        return;

    const std::string aMessage = rKernel + ", case " + std::to_string(nCase) + ", byte "
                                 + std::to_string(aMismatch.first - rExpected.begin());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(aMessage, int(*aMismatch.first), int(*aMismatch.second));
}

constexpr sal_Int32 constCases = 300;

class BitmapScaleConvolutionRowsTest : public CppUnit::TestFixture
{
    void testRowsGiveRoundedWeightedSums();
    void testAVX2RowsGiveSameBytesAsScalarRows();

    CPPUNIT_TEST_SUITE(BitmapScaleConvolutionRowsTest);
    CPPUNIT_TEST(testRowsGiveRoundedWeightedSums);
    CPPUNIT_TEST(testAVX2RowsGiveSameBytesAsScalarRows);
    CPPUNIT_TEST_SUITE_END();
};

void BitmapScaleConvolutionRowsTest::testRowsGiveRoundedWeightedSums()
{
    TestData aData;
    for (sal_Int32 nCase = 0; nCase < constCases; nCase++)
    {
        const ConvolveCase aConvolve(aData, false);
        assertSameBytes(expectedConvolve(aConvolve),
                        aConvolve.run(vcl::bitmap::convolution::convolveRowN32), "convolveRowN32",
                        nCase);

        // The row widths vary to cover every tail of the vector loops.
        const std::vector<sal_uInt8> aRow24 = aData.bytes(3 * aData.next(1, 100));
        for (bool bSwapRedBlue : { false, true })
        {
            assertSameBytes(
                expectedExpand(aRow24, bSwapRedBlue),
                runExpand(aRow24, bSwapRedBlue, vcl::bitmap::convolution::expandRowN24ToN32),
                "expandRowN24ToN32", nCase);
        }
        const std::vector<sal_uInt8> aRow32 = aData.bytes(4 * aData.next(1, 100));
        assertSameBytes(expectedPack(aRow32),
                        runPack(aRow32, vcl::bitmap::convolution::packRowN32ToN24),
                        "packRowN32ToN24", nCase);

        const RowsCase aBlend(aData, aData.next(1, 30), aData.next(1, 200));
        assertSameBytes(expectedBlend(aBlend),
                        runBlend(aBlend, vcl::bitmap::convolution::blendRowsN32), "blendRowsN32",
                        nCase);

        for (sal_Int32 nFactor : { 2, 4 })
        {
            const sal_Int32 nDestinationWidth = aData.next(1, 100);
            // A partial block at the end of the row is dropped.
            const RowsCase aBlocks(aData, nFactor,
                                   nDestinationWidth * nFactor + aData.next(0, nFactor - 1));
            assertSameBytes(expectedBlockAverage(aBlocks, nFactor, nDestinationWidth),
                            runBlockAverage(aBlocks, nDestinationWidth,
                                            nFactor == 2
                                                ? vcl::bitmap::convolution::boxHalveRowN32
                                                : vcl::bitmap::convolution::boxQuarterRowN32),
                            "block average " + std::to_string(nFactor), nCase);
        }
    }
}

void BitmapScaleConvolutionRowsTest::testAVX2RowsGiveSameBytesAsScalarRows()
{
#ifdef LO_X86_SIMD_AVAILABLE
    if (!cpuid::hasAVX2())
        return;

    TestData aData;
    for (sal_Int32 nCase = 0; nCase < constCases; nCase++)
    {
        const ConvolveCase aConvolve(aData, true);
        assertSameBytes(aConvolve.run(vcl::bitmap::convolution::convolveRowN32),
                        aConvolve.run(vcl::bitmap::convolution::convolveRowN32AVX2),
                        "convolveRowN32AVX2", nCase);

        const std::vector<sal_uInt8> aRow24 = aData.bytes(3 * aData.next(1, 100));
        for (bool bSwapRedBlue : { false, true })
        {
            assertSameBytes(
                runExpand(aRow24, bSwapRedBlue, vcl::bitmap::convolution::expandRowN24ToN32),
                runExpand(aRow24, bSwapRedBlue, vcl::bitmap::convolution::expandRowN24ToN32AVX2),
                "expandRowN24ToN32AVX2", nCase);
        }
        const std::vector<sal_uInt8> aRow32 = aData.bytes(4 * aData.next(1, 100));
        assertSameBytes(runPack(aRow32, vcl::bitmap::convolution::packRowN32ToN24),
                        runPack(aRow32, vcl::bitmap::convolution::packRowN32ToN24AVX2),
                        "packRowN32ToN24AVX2", nCase);

        const RowsCase aBlend(aData, aData.next(1, 30), aData.next(1, 200));
        assertSameBytes(runBlend(aBlend, vcl::bitmap::convolution::blendRowsN32),
                        runBlend(aBlend, vcl::bitmap::convolution::blendRowsN32AVX2),
                        "blendRowsN32AVX2", nCase);

        const sal_Int32 nHalfWidth = aData.next(1, 100);
        const RowsCase aHalve(aData, 2, nHalfWidth * 2 + aData.next(0, 1));
        assertSameBytes(
            runBlockAverage(aHalve, nHalfWidth, vcl::bitmap::convolution::boxHalveRowN32),
            runBlockAverage(aHalve, nHalfWidth, vcl::bitmap::convolution::boxHalveRowN32AVX2),
            "boxHalveRowN32AVX2", nCase);

        const sal_Int32 nQuarterWidth = aData.next(1, 100);
        const RowsCase aQuarter(aData, 4, nQuarterWidth * 4 + aData.next(0, 3));
        assertSameBytes(
            runBlockAverage(aQuarter, nQuarterWidth, vcl::bitmap::convolution::boxQuarterRowN32),
            runBlockAverage(aQuarter, nQuarterWidth,
                            vcl::bitmap::convolution::boxQuarterRowN32AVX2),
            "boxQuarterRowN32AVX2", nCase);
    }
#endif
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(BitmapScaleConvolutionRowsTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
