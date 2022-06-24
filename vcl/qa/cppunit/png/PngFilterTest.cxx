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

#include <sal/config.h>

#include <string_view>
#include <functional>

#include <test/bootstrapfixture.hxx>
#include <tools/stream.hxx>
#include <vcl/filter/PngImageReader.hxx>
#include <vcl/filter/PngImageWriter.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <bitmap/BitmapWriteAccess.hxx>
#include <vcl/alpha.hxx>
#include <vcl/graphicfilter.hxx>
#include <unotools/tempfile.hxx>

using namespace css;

namespace
{
using AssertFunction = std::function<bool(const BitmapEx&)>;
// Checks that a pngs BitmapEx is the same after reading and
// after writing. Takes a vector of function pointers if there's need to test
// special cases
void checkImportExportPng(const OUString& sFilePath,
                          const std::vector<AssertFunction>& fAssertFunctions)
{
    SvFileStream aFileStream(sFilePath, StreamMode::READ);
    SvMemoryStream aExportStream;
    BitmapEx aImportedBitmapEx;
    BitmapEx aExportedImportedBitmapEx;

    bool bOpenOk = !aFileStream.GetError() && aFileStream.GetBufferSize() > 0;
    CPPUNIT_ASSERT_MESSAGE(OString("Failed to open file: " + sFilePath.toUtf8()).getStr(), bOpenOk);

    // Read the png from the file
    {
        vcl::PngImageReader aPngReader(aFileStream);
        bool bReadOk = aPngReader.read(aImportedBitmapEx);
        CPPUNIT_ASSERT_MESSAGE(OString("Failed to read png from: " + sFilePath.toUtf8()).getStr(),
                               bReadOk);
        if (!fAssertFunctions.empty())
        {
            for (const auto& fCurrent : fAssertFunctions)
            {
                bool bAssertPassed = fCurrent(aImportedBitmapEx);
                CPPUNIT_ASSERT_MESSAGE(
                    OString("Failed custom assert function during first read for file: "
                            + sFilePath.toUtf8())
                        .getStr(),
                    bAssertPassed);
            }
        }
    }

    // Write the imported png to a stream
    {
        vcl::PngImageWriter aPngWriter(aExportStream);
        bool bWriteOk = aPngWriter.write(aImportedBitmapEx);
        CPPUNIT_ASSERT_MESSAGE(OString("Failed to write png: " + sFilePath.toUtf8()).getStr(),
                               bWriteOk);
        aExportStream.Seek(0);
    }

    // Read the png again from the exported stream
    {
        vcl::PngImageReader aPngReader(aExportStream);
        bool bReadOk = aPngReader.read(aExportedImportedBitmapEx);
        CPPUNIT_ASSERT_MESSAGE(
            OString("Failed to read exported png: " + sFilePath.toUtf8()).getStr(), bReadOk);
        if (!fAssertFunctions.empty())
        {
            for (const auto& fCurrent : fAssertFunctions)
            {
                bool bAssertPassed = fCurrent(aExportedImportedBitmapEx);
                CPPUNIT_ASSERT_MESSAGE(
                    OString("Failed custom assert function during second read for file: "
                            + sFilePath.toUtf8())
                        .getStr(),
                    bAssertPassed);
            }
        }
    }

    // Compare imported and exported BitmapEx
    // This compares size, inner bitmap and alpha mask
    bool bIsSame = (aExportedImportedBitmapEx == aImportedBitmapEx);
    CPPUNIT_ASSERT_MESSAGE(
        OString("Import->Export png test failed for png: " + sFilePath.toUtf8()).getStr(), bIsSame);
}

// Checks that aPngReader.read returns false on corrupted files
void checkImportCorruptedPng(const OUString& sFilePath)
{
    SvFileStream aFileStream(sFilePath, StreamMode::READ);
    BitmapEx aImportedBitmapEx;

    bool bOpenOk = !aFileStream.GetError() && aFileStream.GetBufferSize() > 0;
    CPPUNIT_ASSERT_MESSAGE(OString("Failed to open file: " + sFilePath.toUtf8()).getStr(), bOpenOk);
    vcl::PngImageReader aPngReader(aFileStream);
    bool bReadOk = aPngReader.read(aImportedBitmapEx);
    // Make sure this file was not read successfully
    CPPUNIT_ASSERT_MESSAGE(
        OString("Corrupted png should not be opened: " + sFilePath.toUtf8()).getStr(), !bReadOk);
}
}

class PngFilterTest : public test::BootstrapFixture
{
    // Should keep the temp files (should be false)
    static constexpr bool bKeepTemp = true;

    OUString maDataUrl;

    OUString getFullUrl(std::u16string_view sFileName)
    {
        return m_directories.getURLFromSrc(maDataUrl) + sFileName;
    }

public:
    PngFilterTest()
        : BootstrapFixture(true, false)
        , maDataUrl("/vcl/qa/cppunit/png/data/")
    {
    }

    void testPng();
    void testPngSuite();
    void testMsGifInPng();
    void testPngRoundtrip8BitGrey();
    void testPngRoundtrip24();
    void testPngRoundtrip24_8();
    void testPngRoundtrip32();

    CPPUNIT_TEST_SUITE(PngFilterTest);
    CPPUNIT_TEST(testPng);
    CPPUNIT_TEST(testPngSuite);
    CPPUNIT_TEST(testMsGifInPng);
    CPPUNIT_TEST(testPngRoundtrip8BitGrey);
    CPPUNIT_TEST(testPngRoundtrip24);
    CPPUNIT_TEST(testPngRoundtrip24_8);
    CPPUNIT_TEST(testPngRoundtrip32);
    CPPUNIT_TEST_SUITE_END();
};

void PngFilterTest::testPng()
{
    for (const OUString& aFileName : { OUString("rect-1bit-pal.png") })
    {
        SvFileStream aFileStream(getFullUrl(aFileName), StreamMode::READ);

        vcl::PngImageReader aPngReader(aFileStream);
        BitmapEx aBitmapEx;
        aPngReader.read(aBitmapEx);

        Bitmap aBitmap = aBitmapEx.GetBitmap();
        {
            Bitmap::ScopedReadAccess pAccess(aBitmap);
            CPPUNIT_ASSERT_EQUAL(tools::Long(4), pAccess->Width());
            CPPUNIT_ASSERT_EQUAL(tools::Long(4), pAccess->Height());

            if (pAccess->GetBitCount() == 24 || pAccess->GetBitCount() == 32)
            {
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x00),
                                     pAccess->GetPixel(0, 0));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x00),
                                     pAccess->GetPixel(3, 3));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x00),
                                     pAccess->GetPixel(3, 0));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x00),
                                     pAccess->GetPixel(0, 3));

                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0x00, 0x00, 0x00),
                                     pAccess->GetPixel(1, 1));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0x00, 0x00, 0x00),
                                     pAccess->GetPixel(1, 2));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0x00, 0x00, 0x00),
                                     pAccess->GetPixel(2, 1));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0x00, 0x00, 0x00),
                                     pAccess->GetPixel(2, 2));
            }
            else
            {
                CPPUNIT_ASSERT_MESSAGE("Bitmap is not 24 or 32 bit.", false);
            }
        }
    }

    OUString aFilenames[] = {
        OUString("color-rect-8bit-RGB.png"),
        OUString("color-rect-8bit-RGB-interlaced.png"),
        OUString("color-rect-4bit-pal.png"),
    };

    for (const OUString& aFileName : aFilenames)
    {
        SvFileStream aFileStream(getFullUrl(aFileName), StreamMode::READ);

        vcl::PngImageReader aPngReader(aFileStream);
        BitmapEx aBitmapEx;
        aPngReader.read(aBitmapEx);

        Bitmap aBitmap = aBitmapEx.GetBitmap();
        {
            Bitmap::ScopedReadAccess pAccess(aBitmap);
            CPPUNIT_ASSERT_EQUAL(tools::Long(4), pAccess->Width());
            CPPUNIT_ASSERT_EQUAL(tools::Long(4), pAccess->Height());
            if (pAccess->GetBitCount() == 24 || pAccess->GetBitCount() == 32)
            {
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x00),
                                     pAccess->GetPixel(0, 0));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x00),
                                     pAccess->GetPixel(3, 3));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x00),
                                     pAccess->GetPixel(3, 0));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x00),
                                     pAccess->GetPixel(0, 3));

                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0x00, 0x00, 0x00),
                                     pAccess->GetPixel(1, 1));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0xFF, 0x00, 0x00),
                                     pAccess->GetPixel(1, 2));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0x00, 0xFF, 0x00),
                                     pAccess->GetPixel(2, 1));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0x00, 0x00),
                                     pAccess->GetPixel(2, 2));
            }
            else
            {
                CPPUNIT_ASSERT_MESSAGE("Bitmap is not 24 or 32 bit.", false);
            }
        }
    }
    for (const OUString& aFileName : { OUString("alpha-rect-8bit-RGBA.png") })
    {
        SvFileStream aFileStream(getFullUrl(aFileName), StreamMode::READ);

        vcl::PngImageReader aPngReader(aFileStream);
        BitmapEx aBitmapEx;
        aPngReader.read(aBitmapEx);

        Bitmap aBitmap = aBitmapEx.GetBitmap();
        {
            Bitmap::ScopedReadAccess pAccess(aBitmap);
            CPPUNIT_ASSERT_EQUAL(tools::Long(4), pAccess->Width());
            CPPUNIT_ASSERT_EQUAL(tools::Long(4), pAccess->Height());

            if (pAccess->GetBitCount() == 24)
            {
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x00),
                                     pAccess->GetPixel(0, 0));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x00),
                                     pAccess->GetPixel(3, 3));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x00),
                                     pAccess->GetPixel(3, 0));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x00),
                                     pAccess->GetPixel(0, 3));

                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0x00, 0x00, 0x00),
                                     pAccess->GetPixel(1, 1));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0xFF, 0x00, 0x00),
                                     pAccess->GetPixel(1, 2));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0x00, 0xFF, 0x00),
                                     pAccess->GetPixel(2, 1));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0x00, 0x00),
                                     pAccess->GetPixel(2, 2));

                AlphaMask aAlpha = aBitmapEx.GetAlpha();
                {
                    AlphaMask::ScopedReadAccess pAlphaAccess(aAlpha);
                    CPPUNIT_ASSERT_EQUAL(sal_uInt16(8), pAlphaAccess->GetBitCount());
                    CPPUNIT_ASSERT_EQUAL(tools::Long(4), pAlphaAccess->Width());
                    CPPUNIT_ASSERT_EQUAL(tools::Long(4), pAlphaAccess->Height());

                    CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0x00, 0x80, 0x00),
                                         pAlphaAccess->GetPixel(0, 0));
                    CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0x00, 0x80, 0x00),
                                         pAlphaAccess->GetPixel(3, 3));
                    CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0x00, 0x80, 0x00),
                                         pAlphaAccess->GetPixel(3, 0));
                    CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0x00, 0x80, 0x00),
                                         pAlphaAccess->GetPixel(0, 3));

                    CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0x00, 0x40, 0x00),
                                         pAlphaAccess->GetPixel(1, 1));
                    CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0x00, 0xC0, 0x00),
                                         pAlphaAccess->GetPixel(1, 2));
                    CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0x00, 0xC0, 0x00),
                                         pAlphaAccess->GetPixel(2, 1));
                    CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0x00, 0x40, 0x00),
                                         pAlphaAccess->GetPixel(2, 2));
                }
            }
            else if (pAccess->GetBitCount() == 32)
            {
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x80),
                                     pAccess->GetPixel(0, 0));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x80),
                                     pAccess->GetPixel(3, 3));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x80),
                                     pAccess->GetPixel(3, 0));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x80),
                                     pAccess->GetPixel(0, 3));

                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0x00, 0x00, 0x40),
                                     pAccess->GetPixel(1, 1));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0xFF, 0x00, 0xC0),
                                     pAccess->GetPixel(1, 2));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0x00, 0xFF, 0xC0),
                                     pAccess->GetPixel(2, 1));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0x00, 0x40),
                                     pAccess->GetPixel(2, 2));
            }
            else
            {
                CPPUNIT_ASSERT_MESSAGE("Bitmap is not 24 or 32 bit.", false);
            }
        }
    }
}

void PngFilterTest::testPngSuite()
{
    using namespace std::placeholders; // for std::bind placeholders like _1
    struct Case
    {
        OUString msFilename;
        std::vector<AssertFunction> mfAssertFunctions;
    };
    // This is also used for 16 bits per channel and for 24 bpp + separate 8 bpp alpha mask
    auto fAssert24bpp = [](const BitmapEx& aBitmapEx) {
        Bitmap aBitmap = aBitmapEx.GetBitmap();
        Bitmap::ScopedReadAccess pAccess(aBitmap);
        return pAccess->GetBitCount() == 24u;
    };
    auto fAssertSize = [](const BitmapEx& aBitmapEx, tools::Long nWidth, tools::Long nHeight) {
        return aBitmapEx.GetSizePixel().Width() == nWidth
               && aBitmapEx.GetSizePixel().Height() == nHeight;
    };
    // Test the PngSuite test files by Willem van Schaik
    // filename:                               g04i2c08.png
    //                                         || ||||
    // test feature (in this case gamma) ------+| ||||
    // parameter of test (here gamma-value) ----+ ||||
    // interlaced or non-interlaced --------------+|||
    // color-type (numerical) ---------------------+||
    // color-type (descriptive) --------------------+|
    // bit-depth ------------------------------------+
    Case aCases[] = {
        // Basic formats, not interlaced
        {
            OUString("basn0g01.png"),
        }, // b&w
        {
            OUString("basn0g02.png"),
        }, // 2 bit grayscale
        {
            OUString("basn0g04.png"),
        }, // 4 bit grayscale
        {
            OUString("basn0g08.png"),
        }, // 8 bit grayscale
        {
            OUString("basn0g16.png"),
        }, // 16 bit grayscale
        {
            OUString("basn2c08.png"),
        }, // 8 bit rgb
        {
            OUString("basn2c16.png"),
        }, // 16 bit rgb
        {
            OUString("basn3p01.png"),
        }, // 1 bit palette
        {
            OUString("basn3p02.png"),
        }, // 2 bit palette
        {
            OUString("basn3p04.png"),
        }, // 4 bit palette
        {
            OUString("basn3p08.png"),
        }, // 8 bit palette
        {
            OUString("basn4a08.png"),
        }, // 8 bit grayscale + 8 bit alpha
        {
            OUString("basn4a16.png"),
        }, // 16 bit grayscale + 16 bit alpha
        {
            OUString("basn6a08.png"),
        }, // 8 bit rgba
        {
            OUString("basn6a16.png"),
        }, // 16 bit rgba
        // Basic formats, interlaced
        {
            OUString("basi0g01.png"),
        }, // b&w
        {
            OUString("basi0g02.png"),
        }, // 2 bit grayscale
        {
            OUString("basi0g04.png"),
        }, // 4 bit grayscale
        {
            OUString("basi0g08.png"),
        }, // 8 bit grayscale
        {
            OUString("basi0g16.png"),
        }, // 16 bit grayscale
        {
            OUString("basi2c08.png"),
        }, // 8 bit rgb
        { OUString("basi2c16.png"), { fAssert24bpp } }, // 16 bit rgb
        {
            OUString("basi3p01.png"),
        }, // 1 bit palette
        {
            OUString("basi3p02.png"),
        }, // 2 bit palette
        {
            OUString("basi3p04.png"),
        }, // 4 bit palette
        {
            OUString("basi3p08.png"),
        }, // 8 bit palette
        {
            OUString("basi4a08.png"),
        }, // 8 bit grayscale + 8 bit alpha
        { OUString("basi4a16.png"), { fAssert24bpp } }, // 16 bit grayscale + 16 bit alpha
        { OUString("basi6a08.png"), { fAssert24bpp } }, // 8 bit rgba
        {
            OUString("basi6a16.png"),
        }, // 16 bit rgba
        // Odd sizes, not interlaced
        { OUString("s01n3p01.png"), { std::bind(fAssertSize, _1, 1, 1) } }, // 1x1
        { OUString("s02n3p01.png"), { std::bind(fAssertSize, _1, 2, 2) } }, // 2x2
        {
            OUString("s03n3p01.png"),
        }, // 3x3
        {
            OUString("s04n3p01.png"),
        }, // 4x4
        {
            OUString("s05n3p02.png"),
        }, // 5x5
        {
            OUString("s06n3p02.png"),
        }, // 6x6
        {
            OUString("s07n3p02.png"),
        }, // 7x7
        {
            OUString("s08n3p02.png"),
        }, // 8x8
        {
            OUString("s09n3p02.png"),
        }, // 9x9
        {
            OUString("s32n3p04.png"),
        }, // 32x32
        {
            OUString("s33n3p04.png"),
        }, // 33x33
        {
            OUString("s34n3p04.png"),
        }, // 34x34
        {
            OUString("s35n3p04.png"),
        }, // 35x35
        {
            OUString("s36n3p04.png"),
        }, // 36x36
        {
            OUString("s37n3p04.png"),
        }, // 37x37
        {
            OUString("s38n3p04.png"),
        }, // 38x38
        {
            OUString("s39n3p04.png"),
        }, // 39x39
        {
            OUString("s40n3p04.png"),
        }, // 40x40
        // Odd sizes, interlaced
        {
            OUString("s01i3p01.png"),
        }, // 1x1
        {
            OUString("s02i3p01.png"),
        }, // 2x2
        {
            OUString("s03i3p01.png"),
        }, // 3x3
        {
            OUString("s04i3p01.png"),
        }, // 4x4
        {
            OUString("s05i3p02.png"),
        }, // 5x5
        {
            OUString("s06i3p02.png"),
        }, // 6x6
        {
            OUString("s07i3p02.png"),
        }, // 7x7
        {
            OUString("s08i3p02.png"),
        }, // 8x8
        {
            OUString("s09i3p02.png"),
        }, // 9x9
        {
            OUString("s32i3p04.png"),
        }, // 32x32
        {
            OUString("s33i3p04.png"),
        }, // 33x33
        {
            OUString("s34i3p04.png"),
        }, // 34x34
        {
            OUString("s35i3p04.png"),
        }, // 35x35
        {
            OUString("s36i3p04.png"),
        }, // 36x36
        {
            OUString("s37i3p04.png"),
        }, // 37x37
        {
            OUString("s38i3p04.png"),
        }, // 38x38
        {
            OUString("s39i3p04.png"),
        }, // 39x39
        {
            OUString("s40i3p04.png"),
        }, // 40x40
        // Background colors
        {
            OUString("bgai4a08.png"),
        }, // 8 bit grayscale, }, alpha, }, no background chunk, interlaced
        {
            OUString("bgai4a16.png"),
        }, // 16 bit grayscale, }, alpha, }, no background chunk, interlaced
        {
            OUString("bgan6a08.png"),
        }, // 3x8 bits rgb color, }, alpha, no background chunk
        {
            OUString("bgan6a16.png"),
        }, // 3x16 bits rgb color, }, alpha, no background chunk
        {
            OUString("bgbn4a08.png"),
        }, // 8 bit grayscale, }, alpha, black background chunk
        {
            OUString("bggn4a16.png"),
        }, // 16 bit grayscale, }, alpha, gray background chunk
        {
            OUString("bgwn6a08.png"),
        }, // 3x8 bits rgb color, }, alpha, white background chunk
        {
            OUString("bgyn6a16.png"),
        }, // 3x16 bits rgb color, }, alpha, yellow background chunk
        // Transparency
        {
            OUString("tbbn0g04.png"),
        }, // transparent, black background chunk
        {
            OUString("tbbn2c16.png"),
        }, // transparent, blue background chunk
        {
            OUString("tbbn3p08.png"),
        }, // transparent, black background chunk
        {
            OUString("tbgn2c16.png"),
        }, // transparent, green background chunk
        {
            OUString("tbgn3p08.png"),
        }, // transparent, light-gray background chunk
        {
            OUString("tbrn2c08.png"),
        }, // transparent, red background chunk
        {
            OUString("tbwn0g16.png"),
        }, // transparent, white background chunk
        {
            OUString("tbwn3p08.png"),
        }, // transparent, white background chunk
        {
            OUString("tbyn3p08.png"),
        }, // transparent, yellow background chunk
        {
            OUString("tp1n3p08.png"),
        }, // transparent, but no background chunk
        {
            OUString("tm3n3p02.png"),
        }, // multiple levels of transparency, 3 entries
        // Gamma
        {
            OUString("g03n0g16.png"),
        }, // grayscale, file-gamma = 0.35
        {
            OUString("g03n2c08.png"),
        }, // color, file-gamma = 0.35
        {
            OUString("g03n3p04.png"),
        }, // paletted, file-gamma = 0.35
        {
            OUString("g04n0g16.png"),
        }, // grayscale, file-gamma = 0.45
        {
            OUString("g04n2c08.png"),
        }, // color, file-gamma = 0.45
        {
            OUString("g04n3p04.png"),
        }, // paletted, file-gamma = 0.45
        {
            OUString("g05n0g16.png"),
        }, // grayscale, file-gamma = 0.55
        {
            OUString("g05n2c08.png"),
        }, // color, file-gamma = 0.55
        {
            OUString("g05n3p04.png"),
        }, // paletted, file-gamma = 0.55
        {
            OUString("g07n0g16.png"),
        }, // grayscale, file-gamma = 0.70
        {
            OUString("g07n2c08.png"),
        }, // color, file-gamma = 0.70
        {
            OUString("g07n3p04.png"),
        }, // paletted, file-gamma = 0.70
        {
            OUString("g10n0g16.png"),
        }, // grayscale, file-gamma = 1.00
        {
            OUString("g10n2c08.png"),
        }, // color, file-gamma = 1.00
        {
            OUString("g10n3p04.png"),
        }, // paletted, file-gamma = 1.00
        {
            OUString("g25n0g16.png"),
        }, // grayscale, file-gamma = 2.50
        {
            OUString("g25n2c08.png"),
        }, // color, file-gamma = 2.50
        {
            OUString("g25n3p04.png"),
        }, // paletted, file-gamma = 2.50
        // Image filtering
        {
            OUString("f00n0g08.png"),
        }, // grayscale, }, no interlacing, filter-type 0
        {
            OUString("f00n2c08.png"),
        }, // color, }, no interlacing, filter-type 0
        {
            OUString("f01n0g08.png"),
        }, // grayscale, }, no interlacing, filter-type 1
        {
            OUString("f01n2c08.png"),
        }, // color, }, no interlacing, filter-type 1
        {
            OUString("f02n0g08.png"),
        }, // grayscale, }, no interlacing, filter-type 2
        {
            OUString("f02n2c08.png"),
        }, // color, }, no interlacing, filter-type 2
        {
            OUString("f03n0g08.png"),
        }, // grayscale, }, no interlacing, filter-type 3
        {
            OUString("f03n2c08.png"),
        }, // color, }, no interlacing, filter-type 3
        {
            OUString("f04n0g08.png"),
        }, // grayscale, }, no interlacing, filter-type 4
        {
            OUString("f04n2c08.png"),
        }, // color, }, no interlacing, filter-type 4
        {
            OUString("f99n0g04.png"),
        }, // bit-depth 4, filter changing per scanline
        // Additional palettes
        {
            OUString("pp0n2c16.png"),
        }, // six-cube palette-chunk in true-color image
        {
            OUString("pp0n6a08.png"),
        }, // six-cube palette-chunk in true-color+alpha image
        {
            OUString("ps1n0g08.png"),
        }, // six-cube suggested palette (1 byte) in grayscale image
        {
            OUString("ps1n2c16.png"),
        }, // six-cube suggested palette (1 byte) in true-color image
        {
            OUString("ps2n0g08.png"),
        }, // six-cube suggested palette (2 bytes) in grayscale image
        {
            OUString("ps2n2c16.png"),
        }, // six-cube suggested palette (2 bytes) in true-color image
        // Ancillary chunks
        {
            OUString("ccwn2c08.png"),
        }, // chroma chunk w:0.3127, },0.329 r:0.64, },0.33 g:0.30, },0.60 b:0.15,0.06
        {
            OUString("ccwn3p08.png"),
        }, // chroma chunk w:0.3127, },0.329 r:0.64, },0.33 g:0.30, },0.60 b:0.15,0.06
        {
            OUString("cdfn2c08.png"),
        }, // physical pixel dimensions, 8x32 flat pixels
        {
            OUString("cdhn2c08.png"),
        }, // physical pixel dimensions, 32x8 high pixels
        {
            OUString("cdsn2c08.png"),
        }, // physical pixel dimensions, 8x8 square pixels
        {
            OUString("cdun2c08.png"),
        }, // physical pixel dimensions, 1000 pixels per 1 meter
        {
            OUString("ch1n3p04.png"),
        }, // histogram 15 colors
        {
            OUString("ch2n3p08.png"),
        }, // histogram 256 colors
        {
            OUString("cm0n0g04.png"),
        }, // modification time, 01-jan-2000 12:34:56
        {
            OUString("cm7n0g04.png"),
        }, // modification time, 01-jan-1970 00:00:00
        {
            OUString("cm9n0g04.png"),
        }, // modification time, 31-dec-1999 23:59:59
        {
            OUString("cs3n2c16.png"),
        }, // color, 13 significant bits
        {
            OUString("cs3n3p08.png"),
        }, // paletted, 3 significant bits
        {
            OUString("cs5n2c08.png"),
        }, // color, 5 significant bits
        {
            OUString("cs5n3p08.png"),
        }, // paletted, 5 significant bits
        {
            OUString("cs8n2c08.png"),
        }, // color, 8 significant bits (reference)
        {
            OUString("cs8n3p08.png"),
        }, // paletted, 8 significant bits (reference)
        {
            OUString("ct0n0g04.png"),
        }, // no textual data
        {
            OUString("ct1n0g04.png"),
        }, // with textual data
        {
            OUString("ctzn0g04.png"),
        }, // with compressed textual data
        {
            OUString("cten0g04.png"),
        }, // international UTF-8, english
        {
            OUString("ctfn0g04.png"),
        }, // international UTF-8, finnish
        {
            OUString("ctgn0g04.png"),
        }, // international UTF-8, greek
        {
            OUString("cthn0g04.png"),
        }, // international UTF-8, hindi
        {
            OUString("ctjn0g04.png"),
        }, // international UTF-8, japanese
        {
            OUString("exif2c08.png"),
        }, // chunk with jpeg exif data
        // Chunk ordering
        {
            OUString("oi1n0g16.png"),
        }, // grayscale mother image with 1 idat-chunk
        {
            OUString("oi1n2c16.png"),
        }, // color mother image with 1 idat-chunk
        {
            OUString("oi2n0g16.png"),
        }, // grayscale image with 2 idat-chunks
        {
            OUString("oi2n2c16.png"),
        }, // color image with 2 idat-chunks
        {
            OUString("oi4n0g16.png"),
        }, // grayscale image with 4 unequal sized idat-chunks
        {
            OUString("oi4n2c16.png"),
        }, // color image with 4 unequal sized idat-chunks
        {
            OUString("oi9n0g16.png"),
        }, // grayscale image with all idat-chunks length one
        {
            OUString("oi9n2c16.png"),
        }, // color image with all idat-chunks length one
        // Zlib compression
        {
            OUString("z00n2c08.png"),
        }, // color, }, no interlacing, compression level 0 (none)
        {
            OUString("z03n2c08.png"),
        }, // color, }, no interlacing, compression level 3
        {
            OUString("z06n2c08.png"),
        }, // color, }, no interlacing, compression level 6 (default)
        {
            OUString("z09n2c08.png"),
        }, // color, }, no interlacing, compression level 9 (maximum)
    };

    for (const auto& aCase : aCases)
    {
        checkImportExportPng(getFullUrl(aCase.msFilename), aCase.mfAssertFunctions);
    }

    OUString aCorruptedFilenames[] = {
        OUString("xs1n0g01.png"), // signature byte 1 MSBit reset to zero
        OUString("xs2n0g01.png"), // signature byte 2 is a 'Q'
        OUString("xs4n0g01.png"), // signature byte 4 lowercase
        OUString("xs7n0g01.png"), // 7th byte a space instead of control-Z
        OUString("xcrn0g04.png"), // added cr bytes
        OUString("xlfn0g04.png"), // added lf bytes
        OUString("xhdn0g08.png"), // incorrect IHDR checksum
        OUString("xc1n0g08.png"), // color type 1
        OUString("xc9n2c08.png"), // color type 9
        OUString("xd0n2c08.png"), // bit-depth 0
        OUString("xd3n2c08.png"), // bit-depth 3
        OUString("xd9n2c08.png"), // bit-depth 99
        OUString("xdtn0g01.png"), // missing IDAT chunk
        OUString("xcsn0g01.png"), // incorrect IDAT checksum
    };

    for (const auto& aFilename : aCorruptedFilenames)
    {
        checkImportCorruptedPng(getFullUrl(aFilename));
    }
}

void PngFilterTest::testMsGifInPng()
{
    Graphic aGraphic;
    const OUString aURL(getFullUrl(u"ms-gif.png"));
    SvFileStream aFileStream(aURL, StreamMode::READ);
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
    ErrCode aResult = rFilter.ImportGraphic(aGraphic, aURL, aFileStream);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, aResult);
    CPPUNIT_ASSERT(aGraphic.IsGfxLink());
    // The image is technically a PNG, but it has an animated Gif as a chunk (Microsoft extension).
    CPPUNIT_ASSERT_EQUAL(GfxLinkType::NativeGif, aGraphic.GetSharedGfxLink()->GetType());
    CPPUNIT_ASSERT(aGraphic.IsAnimated());
}

void PngFilterTest::testPngRoundtrip8BitGrey()
{
    utl::TempFile aTempFile(u"testPngRoundtrip8BitGrey");
    if (!bKeepTemp)
        aTempFile.EnableKillingFile();
    {
        SvStream& rStream = *aTempFile.GetStream(StreamMode::WRITE);
        Bitmap aBitmap(Size(16, 16), vcl::PixelFormat::N8_BPP, &Bitmap::GetGreyPalette(256));
        {
            BitmapScopedWriteAccess pWriteAccess(aBitmap);
            pWriteAccess->Erase(COL_BLACK);
            for (int i = 0; i < 8; ++i)
            {
                for (int j = 0; j < 8; ++j)
                {
                    pWriteAccess->SetPixel(i, j, COL_GRAY);
                }
            }
            for (int i = 8; i < 16; ++i)
            {
                for (int j = 8; j < 16; ++j)
                {
                    pWriteAccess->SetPixel(i, j, COL_LIGHTGRAY);
                }
            }
        }
        BitmapEx aBitmapEx(aBitmap);

        vcl::PngImageWriter aPngWriter(rStream);
        CPPUNIT_ASSERT_EQUAL(true, aPngWriter.write(aBitmapEx));
        aTempFile.CloseStream();
    }
    {
        SvStream& rStream = *aTempFile.GetStream(StreamMode::READ);

        vcl::PngImageReader aPngReader(rStream);
        BitmapEx aBitmapEx;
        CPPUNIT_ASSERT_EQUAL(true, aPngReader.read(aBitmapEx));

        CPPUNIT_ASSERT_EQUAL(16L, aBitmapEx.GetSizePixel().Width());
        CPPUNIT_ASSERT_EQUAL(16L, aBitmapEx.GetSizePixel().Height());

        CPPUNIT_ASSERT_EQUAL(COL_GRAY, aBitmapEx.GetPixelColor(0, 0));
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTGRAY, aBitmapEx.GetPixelColor(15, 15));
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, aBitmapEx.GetPixelColor(15, 0));
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, aBitmapEx.GetPixelColor(0, 15));
    }
}

void PngFilterTest::testPngRoundtrip24()
{
    utl::TempFile aTempFile(u"testPngRoundtrip24");
    if (!bKeepTemp)
        aTempFile.EnableKillingFile();
    {
        SvStream& rStream = *aTempFile.GetStream(StreamMode::WRITE);
        Bitmap aBitmap(Size(16, 16), vcl::PixelFormat::N24_BPP);
        {
            BitmapScopedWriteAccess pWriteAccess(aBitmap);
            pWriteAccess->Erase(COL_BLACK);
            for (int i = 0; i < 8; ++i)
            {
                for (int j = 0; j < 8; ++j)
                {
                    pWriteAccess->SetPixel(i, j, COL_LIGHTRED);
                }
            }
            for (int i = 8; i < 16; ++i)
            {
                for (int j = 8; j < 16; ++j)
                {
                    pWriteAccess->SetPixel(i, j, COL_LIGHTBLUE);
                }
            }
        }
        BitmapEx aBitmapEx(aBitmap);

        vcl::PngImageWriter aPngWriter(rStream);
        CPPUNIT_ASSERT_EQUAL(true, aPngWriter.write(aBitmapEx));
    }
    {
        SvStream& rStream = *aTempFile.GetStream(StreamMode::READ);
        rStream.Seek(0);

        vcl::PngImageReader aPngReader(rStream);
        BitmapEx aBitmapEx;
        CPPUNIT_ASSERT_EQUAL(true, aPngReader.read(aBitmapEx));

        CPPUNIT_ASSERT_EQUAL(16L, aBitmapEx.GetSizePixel().Width());
        CPPUNIT_ASSERT_EQUAL(16L, aBitmapEx.GetSizePixel().Height());

        CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, aBitmapEx.GetPixelColor(0, 0));
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTBLUE, aBitmapEx.GetPixelColor(15, 15));
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, aBitmapEx.GetPixelColor(15, 0));
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, aBitmapEx.GetPixelColor(0, 15));
    }
}

void PngFilterTest::testPngRoundtrip24_8()
{
    utl::TempFile aTempFile(u"testPngRoundtrip24_8");
    if (!bKeepTemp)
        aTempFile.EnableKillingFile();
    {
        SvStream& rStream = *aTempFile.GetStream(StreamMode::WRITE);
        Bitmap aBitmap(Size(16, 16), vcl::PixelFormat::N24_BPP);
        AlphaMask aAlpha(Size(16, 16));
        {
            BitmapScopedWriteAccess pWriteAccessBitmap(aBitmap);
            AlphaScopedWriteAccess pWriteAccessAlpha(aAlpha);
            pWriteAccessAlpha->Erase(Color(ColorTransparency, 0x00, 0xAA, 0xAA, 0xAA));
            pWriteAccessBitmap->Erase(COL_BLACK);
            for (int i = 0; i < 8; ++i)
            {
                for (int j = 0; j < 8; ++j)
                {
                    pWriteAccessBitmap->SetPixel(i, j, COL_LIGHTRED);
                    pWriteAccessAlpha->SetPixel(i, j,
                                                Color(ColorTransparency, 0x00, 0xBB, 0xBB, 0xBB));
                }
            }
            for (int i = 8; i < 16; ++i)
            {
                for (int j = 8; j < 16; ++j)
                {
                    pWriteAccessBitmap->SetPixel(i, j, COL_LIGHTBLUE);
                    pWriteAccessAlpha->SetPixel(i, j,
                                                Color(ColorTransparency, 0x00, 0xCC, 0xCC, 0xCC));
                }
            }
        }
        BitmapEx aBitmapEx(aBitmap, aAlpha);
        vcl::PngImageWriter aPngWriter(rStream);
        CPPUNIT_ASSERT_EQUAL(true, aPngWriter.write(aBitmapEx));
    }
    {
        SvStream& rStream = *aTempFile.GetStream(StreamMode::READ);
        rStream.Seek(0);

        vcl::PngImageReader aPngReader(rStream);
        BitmapEx aBitmapEx;
        CPPUNIT_ASSERT_EQUAL(true, aPngReader.read(aBitmapEx));

        CPPUNIT_ASSERT_EQUAL(16L, aBitmapEx.GetSizePixel().Width());
        CPPUNIT_ASSERT_EQUAL(16L, aBitmapEx.GetSizePixel().Height());

        CPPUNIT_ASSERT_EQUAL(Color(ColorTransparency, 0xBB, 0xFF, 0x00, 0x00),
                             aBitmapEx.GetPixelColor(0, 0));
        CPPUNIT_ASSERT_EQUAL(Color(ColorTransparency, 0xCC, 0x00, 0x00, 0xFF),
                             aBitmapEx.GetPixelColor(15, 15));
        CPPUNIT_ASSERT_EQUAL(Color(ColorTransparency, 0xAA, 0x00, 0x00, 0x00),
                             aBitmapEx.GetPixelColor(15, 0));
        CPPUNIT_ASSERT_EQUAL(Color(ColorTransparency, 0xAA, 0x00, 0x00, 0x00),
                             aBitmapEx.GetPixelColor(0, 15));
    }
}

void PngFilterTest::testPngRoundtrip32() {}

CPPUNIT_TEST_SUITE_REGISTRATION(PngFilterTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
