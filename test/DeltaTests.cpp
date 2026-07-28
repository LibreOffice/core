/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Unit tests for tile delta generation and compression.
 * Classes: DeltaTests
 */

#include <config.h>

#include <test/lokassert.hpp>

#include <random>

#include <Delta.hpp>
#include <common/HexUtil.hpp>
#include <common/Util.hpp>
#include <Png.hpp>

#include <cppunit/extensions/HelperMacros.h>

#define DEBUG_DELTA_TESTS 0

/// Delta unit-tests.
class DeltaTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(DeltaTests);

    CPPUNIT_TEST(testRle);
    CPPUNIT_TEST(testRleComplex);
    CPPUNIT_TEST(testRleRandom);
    CPPUNIT_TEST(testRleIdentical);
    CPPUNIT_TEST(testDeltaSequence);
    CPPUNIT_TEST(testRandomDeltas);
    CPPUNIT_TEST(testDeltaCopyOutOfBounds);
    CPPUNIT_TEST(testSimdCopyRowSwapRB);

    CPPUNIT_TEST_SUITE_END();

    void testRle();
    void testRleComplex();
    void testRleRandom();
    void testRleIdentical();
    void testDeltaSequence();
    void testRandomDeltas();
    void testDeltaCopyOutOfBounds();
    void testSimdCopyRowSwapRB();

    std::vector<char> applyDelta(const std::vector<char>& pixmap, uint32_t width, uint32_t height,
                                 const std::vector<char>& delta, const std::string_view testname);

    void assertEqual(const std::vector<char>& a, const std::vector<char>& b, int width, int height,
                     const std::string_view testname);
};

namespace {
void checkzDelta(const std::vector<char> &zDelta, const char *legend)
{
    constexpr std::string_view testname = __func__;

#if DEBUG_DELTA_TESTS
    std::cout << "zdelta: " << legend << "\n";
    HexUtil::dumpHex(std::cout, zDelta, "");
#else
    (void)legend;
#endif

    LOK_ASSERT(zDelta.size() >= 4);
    LOK_ASSERT_EQUAL('D', zDelta[0]);

    std::vector<char> delta;
    delta.resize(1024*1024*4); // lots of extra space.

    size_t compSize = ZSTD_decompress( delta.data(), delta.size(),
                                       zDelta.data() + 1, zDelta.size() - 1);
    LOK_ASSERT_EQUAL(ZSTD_isError(compSize), (unsigned)false);

    delta.resize(compSize);

#if DEBUG_DELTA_TESTS
    std::cout << "delta - uncompressed: " << legend << " of size " << delta.size() << ":\n";
#endif

    size_t i;
    for (i = 0; i < delta.size();)
    {
        switch (delta[i])
        {
        case 'c': // copy row.
        {
#if DEBUG_DELTA_TESTS
            size_t count = static_cast<uint8_t>(delta[i+1]);
            size_t srcRow = static_cast<uint8_t>(delta[i+2]);
            size_t destRow = static_cast<uint8_t>(delta[i+3]);
            std::cout << "c(opy row) from " << srcRow << " to " << destRow << " number of rows: " << count << "\n";
#endif
            i += 4;
            break;
        }
        case 'd': // new run
        {
#if DEBUG_DELTA_TESTS
            size_t destRow = static_cast<uint8_t>(delta[i+1]);
            size_t destCol = static_cast<uint8_t>(delta[i+2]);
#endif
            size_t length = static_cast<uint8_t>(delta[i+3]);
            i += 4;

#if DEBUG_DELTA_TESTS
            std::cout << "d(new pixels) to row " << destRow << " col " << destCol << " number of pixels: " << length << "\n";
            std::vector<char> data(delta.data() + 1, delta.data() + i + length * 4);
            std::cout << HexUtil::stringifyHexLine(data, 0) << "\n";
#endif
            i += 4 * length;
            break;
        }
        case 't': // termination
            LOK_ASSERT_EQUAL(delta.size() - 1, i);
#if DEBUG_DELTA_TESTS
            std::cout << "t(ermination) - delta ended\n";
#endif
            i++;
            break;
        default:
            std::cout << "Unknown delta code " << delta[i] << '\n';
            LOK_ASSERT(false);
            break;
        }
    }
    LOK_ASSERT_EQUAL(delta.size(), i);
}
}

// Quick hack for debugging
std::vector<char> DeltaTests::applyDelta(const std::vector<char>& pixmap, uint32_t width,
                                         uint32_t height, const std::vector<char>& zDelta,
                                         const std::string_view testname)
{
    LOK_ASSERT(zDelta.size() >= 4);
    LOK_ASSERT_EQUAL('D', zDelta[0]);

    std::vector<char> delta;
    delta.resize(1024*1024*4); // lots of extra space.

    size_t compSize = ZSTD_decompress( delta.data(), delta.size(),
                                       zDelta.data() + 1, zDelta.size() - 1);
    LOK_ASSERT_EQUAL(ZSTD_isError(compSize), (unsigned)false);

    delta.resize(compSize);

    // start with the same state.
    std::vector<char> output = pixmap;
    LOK_ASSERT_EQUAL(output.size(), size_t(pixmap.size()));
    LOK_ASSERT_EQUAL(output.size(), size_t(width * height * 4));

    size_t offset = 0, i;
    for (i = 0; i < delta.size() && offset < output.size();)
    {
        switch (delta[i])
        {
        case 'c': // copy row.
        {
            size_t count = static_cast<uint8_t>(delta[i+1]);
            size_t srcRow = static_cast<uint8_t>(delta[i+2]);
            size_t destRow = static_cast<uint8_t>(delta[i+3]);

            LOK_ASSERT(srcRow + count <= height);

//            std::cout << "copy " << count <<" row(s) " << srcRow << " to " << destRow << '\n';
            for (size_t cnt = 0; cnt < count; ++cnt)
            {
                const char *src = &pixmap[width * (srcRow + cnt) * 4];
                char *dest = &output[width * (destRow + cnt) * 4];
                for (size_t j = 0; j < width * 4; ++j)
                    *dest++ = *src++;
            }
            i += 4;
            break;
        }
        case 'd': // new run
        {
            size_t destRow = static_cast<uint8_t>(delta[i+1]);
            size_t destCol = static_cast<uint8_t>(delta[i+2]);
            size_t length = static_cast<uint8_t>(delta[i+3]);
            i += 4;

//            std::cout << "new " << length << " at " << destCol << ", " << destRow << '\n';
            LOK_ASSERT(length <= width - destCol);

            char *dest = &output[width * destRow * 4 + destCol * 4];
            for (size_t j = 0; j < length * 4 && i < delta.size(); j += 4)
            {
                *dest++ = delta[i];
                *dest++ = delta[i + 1];
                *dest++ = delta[i + 2];
                *dest++ = delta[i + 3];
                i += 4;
            }
            break;
        }
        case 't': // termination
            LOK_ASSERT_EQUAL(delta.size() - 1, i);
            i++;
            break;
        default:
            std::cout << "Unknown delta code " << delta[i] << '\n';
            LOK_ASSERT(false);
            break;
        }
    }
    LOK_ASSERT_EQUAL(delta.size(), i);
    return output;
}

void DeltaTests::assertEqual(const std::vector<char>& a, const std::vector<char>& b, int width,
                             int /* height */, const std::string_view testname)
{
    LOK_ASSERT_EQUAL(a.size(), b.size());
    for (size_t i = 0; i < a.size(); ++i)
    {
        if (a[i] != b[i])
        {
            std::cout << "Differences starting at byte " << i << ' '
                      << (i/4 % width) << ", " << (i / (width * 4)) << ":\n";
            size_t len;
            for (len = 0; (a[i+len] != b[i+len] || len < 8) && i + len < a.size(); ++len)
            {
                std::cout << std::hex << (int)((unsigned char)a[i+len]) << " != ";
                std::cout << std::hex << (int)((unsigned char)b[i+len]) << "  ";
                if (len > 0 && (len % 16 == 0))
                    std::cout<< '\n';
            }
            std::cout << " size " << len << '\n';
            HexUtil::dumpHex(std::cout, a, "a");
            HexUtil::dumpHex(std::cout, b, "b");
            LOK_ASSERT(false);
        }
    }
}

void DeltaTests::testRle()
{
    constexpr std::string_view testname = __func__;

    DeltaGenerator::DeltaBitmapRow rowa;
    DeltaGenerator::DeltaBitmapRow rowb;
    const uint32_t data[] = { 42, 42, 42, 42, 0, 0, 0, 1, 2, 3, 4,
        7, 7, 7, 1, 2, 3, 3, 2, 1, 0, 9, 9, 9, 9, 9, 0, 9, 0, 9, 0, 9 };
    const uint32_t elems = N_ELEMENTS(data);
    rowa.initRow(data, elems);
    rowb.initRow(data, elems);
    LOK_ASSERT(rowa.identical(rowb));

    DeltaGenerator::DeltaBitmapRow::PixIterator it(rowa);
    for (uint32_t i = 0; i < elems; ++i)
    {
        LOK_ASSERT_EQUAL(data[i], it.getPixel());
        it.next();
    }

    const uint32_t empty[256] = { 0, };
    DeltaGenerator::DeltaBitmapRow rowc;
    rowc.initRow(empty, 256);
    LOK_ASSERT(!rowa.identical(rowc));
    LOK_ASSERT(!rowb.identical(rowc));
    DeltaGenerator::DeltaBitmapRow::PixIterator it2(rowc);
    for (uint32_t i = 0; i < 256; ++i)
    {
        LOK_ASSERT_EQUAL(empty[i], it2.getPixel());
        it2.next();
    }
}

void DeltaTests::testRleComplex()
{
    constexpr std::string_view testname = __func__;

    DeltaGenerator gen;

    uint32_t height, width, rowBytes;
    const TileWireId textWid = 1;
    std::vector<char> text =
        Png::loadPng(TDOC "/delta-graphic.png", height, width, rowBytes);

    DeltaGenerator::DeltaData data(
        textWid, reinterpret_cast<unsigned char*>(text.data()),
        0, 0, 256, 256, TileLocation(9, 9, 9, 0, CanonicalViewId(1), 0), 256, 256);

    size_t off = 0;
    for (int y = 0; y < 256; ++y)
    {
        DeltaGenerator::DeltaBitmapRow::PixIterator it(data.getRow(y));
        for (uint32_t x = 0; x < 256; ++x)
        {
            uint32_t pix = reinterpret_cast<uint32_t *>(text.data())[off];
            uint32_t rpix = it.getPixel();
            if (pix != rpix)
            {
                // breakpoint point
                LOK_ASSERT_EQUAL(pix, rpix);
            }
            it.next();
            off++;
        }
    }
}

void DeltaTests::testRleRandom()
{
    constexpr std::string_view testname = __func__;

    DeltaGenerator gen;

    std::vector<unsigned char> randomImg(256*256*4);
    std::mt19937 random;
    random.seed(42);
    std::uniform_int_distribution<unsigned char> dist(0,255);

    for (size_t i = 0; i < randomImg.size(); ++i)
        randomImg[i] = dist(random);

    DeltaGenerator::DeltaData data(
        1, randomImg.data(), 0, 0, 256, 256,
        TileLocation(9, 9, 9, 0, CanonicalViewId(1), 0), 256, 256);

    // Compress
    std::vector<char> output;
    size_t size = gen.compressOrDelta(
        randomImg.data(), 0, 0, 256, 256, 256, 256,
        TileLocation(42, 2, 3, 0, CanonicalViewId(1), 0),
        output, 1, true, false, COKitTileMode::RGBA);
    LOK_ASSERT(size > 1);
    LOK_ASSERT_EQUAL('Z', output[0]);

    // Decompress
    std::vector<char> frame;
    frame.resize(1024*1024*4); // lots of extra space.
    size_t compSize = ZSTD_decompress( frame.data(), frame.size(),
                                       output.data() + 1, output.size() - 1);
    LOK_ASSERT_EQUAL(ZSTD_isError(compSize), (unsigned)false);
}

void DeltaTests::testRleIdentical()
{
    constexpr std::string_view testname = __func__;

    DeltaGenerator gen;

    uint32_t height, width, rowBytes;
    const TileWireId textWid = 1;
    std::vector<char> text =
        Png::loadPng(TDOC "/delta-graphic.png", height, width, rowBytes);
    LOK_ASSERT_EQUAL(uint32_t(256), height);
    LOK_ASSERT_EQUAL(uint32_t(256), width);
    LOK_ASSERT_EQUAL(uint32_t(256 * 4), rowBytes);
    LOK_ASSERT_EQUAL(size_t(256 * 256 * 4), text.size());

    DeltaGenerator::DeltaData data(
        textWid, reinterpret_cast<unsigned char*>(text.data()),
        0, 0, 256, 256, TileLocation(9, 9, 9, 0, CanonicalViewId(1), 0), 256, 256);

    std::vector<char> text2 =
        Png::loadPng(TDOC "/delta-graphic2.png", height, width, rowBytes);
    LOK_ASSERT_EQUAL(uint32_t(256), height);
    LOK_ASSERT_EQUAL(uint32_t(256), width);
    LOK_ASSERT_EQUAL(uint32_t(256 * 4), rowBytes);
    LOK_ASSERT_EQUAL(size_t(256 * 256 * 4), text2.size());

    DeltaGenerator::DeltaData data2(
        textWid, reinterpret_cast<unsigned char*>(text.data()),
        0, 0, 256, 256, TileLocation(9, 9, 9, 0, CanonicalViewId(1), 0), 256, 256);

    // find identical rows
    for (int y = 0; y < 256; ++y)
    {
        for (int y2 = 0; y2 < 256; y2++)
        {
            const auto& row = data.getRow(y);
            const auto& row2 = data2.getRow(y2);
            if (row.identical(row2))
            {
                DeltaGenerator::DeltaBitmapRow::PixIterator it(row);
                DeltaGenerator::DeltaBitmapRow::PixIterator it2(row2);
                for (uint32_t i = 0; i < 256; ++i)
                {
                    if (it.getPixel() != it2.getPixel())
                    {
                        // breakpoint point.
                        LOK_ASSERT_EQUAL(it.getPixel(), it2.getPixel());
                    }
                    it.next(); it2.next();
                }
                break;
            }
        }
    }
}

void DeltaTests::testDeltaSequence()
{
    constexpr std::string_view testname = __func__;

    DeltaGenerator gen;

    uint32_t height, width, rowBytes;
    const TileWireId textWid = 1;
    std::vector<char> text =
        Png::loadPng(TDOC "/delta-text.png", height, width, rowBytes);
    LOK_ASSERT_EQUAL(uint32_t(256), height);
    LOK_ASSERT_EQUAL(uint32_t(256), width);
    LOK_ASSERT_EQUAL(uint32_t(256 * 4), rowBytes);
    LOK_ASSERT_EQUAL(size_t(256 * 256 * 4), text.size());

    const TileWireId text2Wid = 2;
    std::vector<char> text2 =
        Png::loadPng(TDOC "/delta-text2.png", height, width, rowBytes);
    LOK_ASSERT_EQUAL(uint32_t(256), height);
    LOK_ASSERT_EQUAL(uint32_t(256), width);
    LOK_ASSERT_EQUAL(uint32_t(256 * 4), rowBytes);
    LOK_ASSERT_EQUAL(size_t(256 * 256 * 4), text2.size());

    std::vector<char> delta;
    std::shared_ptr<DeltaGenerator::DeltaData> rleData;

    // Stash it in the cache
    LOK_ASSERT(gen.createDelta(
                       reinterpret_cast<unsigned char *>(text.data()),
                       0, 0, width, height, width, height,
                       TileLocation(1, 2, 3, 0, CanonicalViewId(1), 0), delta,
                       textWid, false, COKitTileMode::RGBA, rleData) == false);
    LOK_ASSERT(delta.empty());

    // Build a delta between text2 & textWid
    LOK_ASSERT(gen.createDelta(
                       reinterpret_cast<unsigned char *>(text2.data()),
                       0, 0, width, height, width, height,
                       TileLocation(1, 2, 3, 0, CanonicalViewId(1), 0), delta,
                       text2Wid, false, COKitTileMode::RGBA, rleData) == true);
    LOK_ASSERT(delta.size() > 0);
    checkzDelta(delta, "text2 to textWid");

    // Apply it to move to the second frame
    std::vector<char> reText2 = applyDelta(text, width, height, delta, testname);
    assertEqual(reText2, text2, width, height, testname);

    // Build a delta between text & text2Wid
    std::vector<char> two2one;
    LOK_ASSERT(gen.createDelta(
                       reinterpret_cast<unsigned char *>(text.data()),
                       0, 0, width, height, width, height,
                       TileLocation(1, 2, 3, 0, CanonicalViewId(1), 0), two2one,
                       textWid, false, COKitTileMode::RGBA, rleData) == true);
    LOK_ASSERT(two2one.size() > 0);
    checkzDelta(two2one, "text to text2Wid");

    // Apply it to get back to where we started
    std::vector<char> reText = applyDelta(text2, width, height, two2one, testname);
    assertEqual(reText, text, width, height, testname);
}

void DeltaTests::testRandomDeltas()
{
}

void DeltaTests::testSimdCopyRowSwapRB()
{
    constexpr std::string_view testname = __func__;

    // Simple reference
    auto simpleSwapRB = [](unsigned char *dest, const unsigned char *src, unsigned int count) {
        for (size_t i = 0; i < count * 4; i += 4)
        {
            dest[i + 0] = src[i + 2];  // R <- B
            dest[i + 1] = src[i + 1];  // G <- G
            dest[i + 2] = src[i + 0];  // B <- R
            dest[i + 3] = src[i + 3];  // A <- A
        }
    };

    // Test various sizes: aligned (multiple of 8), unaligned, and edge cases
    const std::vector<unsigned int> testSizes = { 1, 2, 3, 7, 8, 9, 15, 16, 17, 31, 32, 64, 128, 255, 256 };

    std::mt19937 random;
    random.seed(12345);
    std::uniform_int_distribution<unsigned char> dist(0, 255);

    for (unsigned int count : testSizes)
    {
        std::vector<unsigned char> input(count * 4);
        std::vector<unsigned char> simdOutput(count * 4);
        std::vector<unsigned char> goodOutput(count * 4);

        // Fill with random data
        for (size_t i = 0; i < input.size(); ++i)
            input[i] = dist(random);

        // Run simple version for reference
        simpleSwapRB(goodOutput.data(), input.data(), count);

        // Run SIMD version
        int simdUsed = simd_copyRowSwapRB(simdOutput.data(), input.data(), count);

        if (simdUsed)
        {
            // Compare results
            for (size_t i = 0; i < count * 4; ++i)
            {
                if (simdOutput[i] != goodOutput[i])
                {
                    std::cerr << "Mismatch at byte " << i << " (pixel " << (i / 4)
                              << ", component " << (i % 4) << ") for count=" << count
                              << ": SIMD=" << (int)simdOutput[i]
                              << " scalar=" << (int)goodOutput[i] << '\n';
                }
                LOK_ASSERT_EQUAL(goodOutput[i], simdOutput[i]);
            }
        }

        if (count == 256)
            TST_LOG("simd_copyRowSwapRB for " << count << " pixels: SIMD "
                    << (simdUsed ? "used" : "not used (fallback)"));
    }
}

void DeltaTests::testDeltaCopyOutOfBounds()
{
    constexpr std::string_view testname = __func__;

    DeltaGenerator gen;

    uint32_t height, width, rowBytes;
    const TileWireId textWid = 1;
    std::vector<char> text =
        Png::loadPng(TDOC "/delta-graphic.png", height, width, rowBytes);
    LOK_ASSERT_EQUAL(uint32_t(256), height);
    LOK_ASSERT_EQUAL(uint32_t(256), width);
    LOK_ASSERT_EQUAL(uint32_t(256 * 4), rowBytes);
    LOK_ASSERT_EQUAL(size_t(256 * 256 * 4), text.size());

    const TileWireId text2Wid = 2;
    std::vector<char> text2 =
        Png::loadPng(TDOC "/delta-graphic2.png", height, width, rowBytes);
    LOK_ASSERT_EQUAL(uint32_t(256), height);
    LOK_ASSERT_EQUAL(uint32_t(256), width);
    LOK_ASSERT_EQUAL(uint32_t(256 * 4), rowBytes);
    LOK_ASSERT_EQUAL(size_t(256 * 256 * 4), text2.size());

    std::vector<char> delta;
    std::shared_ptr<DeltaGenerator::DeltaData> rleData;

    // Stash it in the cache
    LOK_ASSERT(gen.createDelta(
                       reinterpret_cast<unsigned char *>(text.data()),
                       0, 0, width, height, width, height,
                       TileLocation(1, 2, 3, 0, CanonicalViewId(1), 0), delta,
                       textWid, false, COKitTileMode::RGBA, rleData) == false);
    LOK_ASSERT(delta.empty());

    // Build a delta between the two frames
    LOK_ASSERT(gen.createDelta(
                       reinterpret_cast<unsigned char *>(text2.data()),
                       0, 0, width, height, width, height,
                       TileLocation(1, 2, 3, 0, CanonicalViewId(1), 0), delta,
                       text2Wid, false, COKitTileMode::RGBA, rleData) == true);
    LOK_ASSERT(delta.size() > 0);
    checkzDelta(delta, "copy out of bounds");

    // Apply it to move to the second frame
    std::vector<char> reText2 = applyDelta(text, width, height, delta, testname);
    assertEqual(reText2, text2, width, height, testname);
}

CPPUNIT_TEST_SUITE_REGISTRATION(DeltaTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
