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
struct Case
{
    tools::Long mnWidth;
    tools::Long mnHeight;
    sal_uInt16 mnBpp;
    bool mbHasPalette;
    bool mbIsAlpha;
};
// Checks that a pngs BitmapEx is the same after reading and
// after writing. Takes a vector of function pointers if there's need to test
// special cases
void checkImportExportPng(const OUString& sFilePath, const Case& aCase)
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
        Bitmap aImportedBitmap = aImportedBitmapEx.GetBitmap();
        Bitmap::ScopedInfoAccess pAccess(aImportedBitmap);
        auto nActualWidth = aImportedBitmapEx.GetSizePixel().Width();
        auto nActualHeight = aImportedBitmapEx.GetSizePixel().Height();
        auto nActualBpp = vcl::pixelFormatBitCount(aImportedBitmapEx.GetBitmap().getPixelFormat());
        auto bActualHasPalette = pAccess->HasPalette();
        auto bActualIsAlpha = aImportedBitmapEx.IsAlpha();
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            OString("Width comparison failed for exported png:" + sFilePath.toUtf8()).getStr(),
            aCase.mnWidth, nActualWidth);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            OString("Height comparison failed for exported png:" + sFilePath.toUtf8()).getStr(),
            aCase.mnHeight, nActualHeight);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            OString("Bpp comparison failed for exported png:" + sFilePath.toUtf8()).getStr(),
            aCase.mnBpp, nActualBpp);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            OString("HasPalette comparison failed for exported png:" + sFilePath.toUtf8()).getStr(),
            aCase.mbHasPalette, bActualHasPalette);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            OString("IsAlpha comparison failed for exported png:" + sFilePath.toUtf8()).getStr(),
            aCase.mbIsAlpha, bActualIsAlpha);
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
        Bitmap aExportedImportedBitmap = aExportedImportedBitmapEx.GetBitmap();
        Bitmap::ScopedInfoAccess pAccess(aExportedImportedBitmap);
        auto nActualWidth = aExportedImportedBitmapEx.GetSizePixel().Width();
        auto nActualHeight = aExportedImportedBitmapEx.GetSizePixel().Height();
        auto nActualBpp
            = vcl::pixelFormatBitCount(aExportedImportedBitmapEx.GetBitmap().getPixelFormat());
        auto bActualHasPalette = pAccess->HasPalette();
        auto bActualIsAlpha = aExportedImportedBitmapEx.IsAlpha();
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            OString("Width comparison failed for exported png:" + sFilePath.toUtf8()).getStr(),
            aCase.mnWidth, nActualWidth);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            OString("Height comparison failed for exported png:" + sFilePath.toUtf8()).getStr(),
            aCase.mnHeight, nActualHeight);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            OString("Bpp comparison failed for exported png:" + sFilePath.toUtf8()).getStr(),
            aCase.mnBpp, nActualBpp);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            OString("HasPalette comparison failed for exported png:" + sFilePath.toUtf8()).getStr(),
            aCase.mbHasPalette, bActualHasPalette);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            OString("IsAlpha comparison failed for exported png:" + sFilePath.toUtf8()).getStr(),
            aCase.mbIsAlpha, bActualIsAlpha);
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
    void testPngWrite1BitRGBPalette();

    CPPUNIT_TEST_SUITE(PngFilterTest);
    CPPUNIT_TEST(testPng);
    CPPUNIT_TEST(testPngSuite);
    CPPUNIT_TEST(testMsGifInPng);
    CPPUNIT_TEST(testPngRoundtrip8BitGrey);
    CPPUNIT_TEST(testPngRoundtrip24);
    CPPUNIT_TEST(testPngRoundtrip24_8);
    CPPUNIT_TEST(testPngRoundtrip32);
    CPPUNIT_TEST(testPngWrite1BitRGBPalette);
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
    // Test the PngSuite test files by Willem van Schaik
    // filename:                               g04i2c08.png
    //                                         || ||||
    // test feature (in this case gamma) ------+| ||||
    // parameter of test (here gamma-value) ----+ ||||
    // interlaced or non-interlaced --------------+|||
    // color-type (numerical) ---------------------+||
    // color-type (descriptive) --------------------+|
    // bit-depth ------------------------------------+

    // Some notes about the cases:
    // - RGB palette PNGs get converted to a bitmap with png_set_palette_to_rgb
    // - Grayscale PNGs with alpha also do with png_set_gray_to_rgb
    // - Grayscale PNGs without alpha use BitmapEx palette utilities
    // - 1, 2, 4 bit grayscale w/o alpha gets converted to 8 bit with png_set_expand_gray_1_2_4_to_8
    // - 16 bit per channel gets converted to 8 bit per channel with png_set_scale_16
    // - PNGs that are not size related have size 32x32
    // - Internally BitmapEx is never 32 bpp, instead it's 24 bpp (rgb) and uses an 8 bpp alpha mask
    std::pair<std::u16string_view, Case> aCases[] = {
        // Basic formats, not interlaced
        { u"basn0g01.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // b&w
        { u"basn0g02.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // 2 bit grayscale
        { u"basn0g04.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // 4 bit grayscale
        { u"basn0g08.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // 8 bit grayscale
        { u"basn0g16.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // 16 bit grayscale
        { u"basn2c08.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // 8 bit rgb
        { u"basn2c16.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // 16 bit rgb
        { u"basn3p01.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // 1 bit palette
        { u"basn3p02.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // 2 bit palette
        { u"basn3p04.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // 4 bit palette
        { u"basn3p08.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // 8 bit palette
        { u"basn4a08.png",
          {
              32,
              32,
              24,
              false,
              true,
          } }, // 8 bit grayscale + 8 bit alpha
        { u"basn4a16.png",
          {
              32,
              32,
              24,
              false,
              true,
          } }, // 16 bit grayscale + 16 bit alpha
        { u"basn6a08.png",
          {
              32,
              32,
              24,
              false,
              true,
          } }, // 8 bit rgba
        { u"basn6a16.png",
          {
              32,
              32,
              24,
              false,
              true,
          } }, // 16 bit rgba
        // Basic formats, interlaced
        { u"basi0g01.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // b&w
        { u"basi0g02.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // 2 bit grayscale
        { u"basi0g04.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // 4 bit grayscale
        { u"basi0g08.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // 8 bit grayscale
        { u"basi0g16.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // 16 bit grayscale
        { u"basi2c08.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // 8 bit rgb
        { u"basi2c16.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // 16 bit rgb
        { u"basi3p01.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // 1 bit palette
        { u"basi3p02.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // 2 bit palette
        { u"basi3p04.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // 4 bit palette
        { u"basi3p08.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // 8 bit palette
        { u"basi4a08.png",
          {
              32,
              32,
              24,
              false,
              true,
          } }, // 8 bit grayscale + 8 bit alpha
        { u"basi4a16.png",
          {
              32,
              32,
              24,
              false,
              true,
          } }, // 16 bit grayscale + 16 bit alpha
        { u"basi6a08.png",
          {
              32,
              32,
              24,
              false,
              true,
          } }, // 8 bit rgba
        { u"basi6a16.png",
          {
              32,
              32,
              24,
              false,
              true,
          } }, // 16 bit rgba
        // // Odd sizes, not interlaced
        { u"s01n3p01.png",
          {
              1,
              1,
              24,
              false,
              false,
          } }, // 1x1
        { u"s02n3p01.png",
          {
              2,
              2,
              24,
              false,
              false,
          } }, // 2x2
        { u"s03n3p01.png",
          {
              3,
              3,
              24,
              false,
              false,
          } }, // 3x3
        { u"s04n3p01.png",
          {
              4,
              4,
              24,
              false,
              false,
          } }, // 4x4
        { u"s05n3p02.png",
          {
              5,
              5,
              24,
              false,
              false,
          } }, // 5x5
        { u"s06n3p02.png",
          {
              6,
              6,
              24,
              false,
              false,
          } }, // 6x6
        { u"s07n3p02.png",
          {
              7,
              7,
              24,
              false,
              false,
          } }, // 7x7
        { u"s08n3p02.png",
          {
              8,
              8,
              24,
              false,
              false,
          } }, // 8x8
        { u"s09n3p02.png",
          {
              9,
              9,
              24,
              false,
              false,
          } }, // 9x9
        { u"s32n3p04.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // 32x32
        { u"s33n3p04.png",
          {
              33,
              33,
              24,
              false,
              false,
          } }, // 33x33
        { u"s34n3p04.png",
          {
              34,
              34,
              24,
              false,
              false,
          } }, // 34x34
        { u"s35n3p04.png",
          {
              35,
              35,
              24,
              false,
              false,
          } }, // 35x35
        { u"s36n3p04.png",
          {
              36,
              36,
              24,
              false,
              false,
          } }, // 36x36
        { u"s37n3p04.png",
          {
              37,
              37,
              24,
              false,
              false,
          } }, // 37x37
        { u"s38n3p04.png",
          {
              38,
              38,
              24,
              false,
              false,
          } }, // 38x38
        { u"s39n3p04.png",
          {
              39,
              39,
              24,
              false,
              false,
          } }, // 39x39
        { u"s40n3p04.png",
          {
              40,
              40,
              24,
              false,
              false,
          } }, // 40x40
        // // Odd sizes, interlaced
        { u"s01i3p01.png",
          {
              1,
              1,
              24,
              false,
              false,
          } }, // 1x1
        { u"s02i3p01.png",
          {
              2,
              2,
              24,
              false,
              false,
          } }, // 2x2
        { u"s03i3p01.png",
          {
              3,
              3,
              24,
              false,
              false,
          } }, // 3x3
        { u"s04i3p01.png",
          {
              4,
              4,
              24,
              false,
              false,
          } }, // 4x4
        { u"s05i3p02.png",
          {
              5,
              5,
              24,
              false,
              false,
          } }, // 5x5
        { u"s06i3p02.png",
          {
              6,
              6,
              24,
              false,
              false,
          } }, // 6x6
        { u"s07i3p02.png",
          {
              7,
              7,
              24,
              false,
              false,
          } }, // 7x7
        { u"s08i3p02.png",
          {
              8,
              8,
              24,
              false,
              false,
          } }, // 8x8
        { u"s09i3p02.png",
          {
              9,
              9,
              24,
              false,
              false,
          } }, // 9x9
        { u"s32i3p04.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // 32x32
        { u"s33i3p04.png",
          {
              33,
              33,
              24,
              false,
              false,
          } }, // 33x33
        { u"s34i3p04.png",
          {
              34,
              34,
              24,
              false,
              false,
          } }, // 34x34
        { u"s35i3p04.png",
          {
              35,
              35,
              24,
              false,
              false,
          } }, // 35x35
        { u"s36i3p04.png",
          {
              36,
              36,
              24,
              false,
              false,
          } }, // 36x36
        { u"s37i3p04.png",
          {
              37,
              37,
              24,
              false,
              false,
          } }, // 37x37
        { u"s38i3p04.png",
          {
              38,
              38,
              24,
              false,
              false,
          } }, // 38x38
        { u"s39i3p04.png",
          {
              39,
              39,
              24,
              false,
              false,
          } }, // 39x39
        { u"s40i3p04.png",
          {
              40,
              40,
              24,
              false,
              false,
          } }, // 40x40
        // Background colors
        { u"bgai4a08.png",
          {
              32,
              32,
              24,
              false,
              true,
          } }, // 8 bit grayscale alpha no background chunk, interlaced
        { u"bgai4a16.png",
          {
              32,
              32,
              24,
              false,
              true,
          } }, // 16 bit grayscale alpha no background chunk, interlaced
        { u"bgan6a08.png",
          {
              32,
              32,
              24,
              false,
              true,
          } }, // 3 * 8 bits rgb color alpha, no background chunk
        { u"bgan6a16.png",
          {
              32,
              32,
              24,
              false,
              true,
          } }, // 3 * 16 bits rgb color alpha, no background chunk
        { u"bgbn4a08.png",
          {
              32,
              32,
              24,
              false,
              true,
          } }, // 8 bit grayscale alpha, black background chunk
        { u"bggn4a16.png",
          {
              32,
              32,
              24,
              false,
              true,
          } }, // 16 bit grayscale alpha, gray background chunk
        { u"bgwn6a08.png",
          {
              32,
              32,
              24,
              false,
              true,
          } }, // 3 * 8 bits rgb color alpha, white background chunk
        { u"bgyn6a16.png",
          {
              32,
              32,
              24,
              false,
              true,
          } }, // 3 * 16 bits rgb color alpha, yellow background chunk
        // Transparency
        { u"tbbn0g04.png",
          {
              32,
              32,
              24,
              false,
              true,
          } }, // transparent, black background chunk
        { u"tbbn2c16.png",
          {
              32,
              32,
              24,
              false,
              true,
          } }, // transparent, blue background chunk
        { u"tbbn3p08.png",
          {
              32,
              32,
              24,
              false,
              true,
          } }, // transparent, black background chunk
        { u"tbgn2c16.png",
          {
              32,
              32,
              24,
              false,
              true,
          } }, // transparent, green background chunk
        { u"tbgn3p08.png",
          {
              32,
              32,
              24,
              false,
              true,
          } }, // transparent, light-gray background chunk
        { u"tbrn2c08.png",
          {
              32,
              32,
              24,
              false,
              true,
          } }, // transparent, red background chunk
        { u"tbwn0g16.png",
          {
              32,
              32,
              24,
              false,
              true,
          } }, // transparent, white background chunk
        { u"tbwn3p08.png",
          {
              32,
              32,
              24,
              false,
              true,
          } }, // transparent, white background chunk
        { u"tbyn3p08.png",
          {
              32,
              32,
              24,
              false,
              true,
          } }, // transparent, yellow background chunk
        { u"tp1n3p08.png",
          {
              32,
              32,
              24,
              false,
              true,
          } }, // transparent, but no background chunk
        { u"tm3n3p02.png",
          {
              32,
              32,
              24,
              false,
              true,
          } }, // multiple levels of transparency, 3 entries
        // Gamma
        { u"g03n0g16.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // grayscale, file-gamma = 0.35
        { u"g03n2c08.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // color, file-gamma = 0.35
        { u"g03n3p04.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // paletted, file-gamma = 0.35
        { u"g04n0g16.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // grayscale, file-gamma = 0.45
        { u"g04n2c08.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // color, file-gamma = 0.45
        { u"g04n3p04.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // paletted, file-gamma = 0.45
        { u"g05n0g16.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // grayscale, file-gamma = 0.55
        { u"g05n2c08.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // color, file-gamma = 0.55
        { u"g05n3p04.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // paletted, file-gamma = 0.55
        { u"g07n0g16.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // grayscale, file-gamma = 0.70
        { u"g07n2c08.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // color, file-gamma = 0.70
        { u"g07n3p04.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // paletted, file-gamma = 0.70
        { u"g10n0g16.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // grayscale, file-gamma = 1.00
        { u"g10n2c08.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // color, file-gamma = 1.00
        { u"g10n3p04.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // paletted, file-gamma = 1.00
        { u"g25n0g16.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // grayscale, file-gamma = 2.50
        { u"g25n2c08.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // color, file-gamma = 2.50
        { u"g25n3p04.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // paletted, file-gamma = 2.50
        // Image filtering
        { u"f00n0g08.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // grayscale no interlacing, filter-type 0
        { u"f00n2c08.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // color no interlacing, filter-type 0
        { u"f01n0g08.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // grayscale no interlacing, filter-type 1
        { u"f01n2c08.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // color no interlacing, filter-type 1
        { u"f02n0g08.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // grayscale no interlacing, filter-type 2
        { u"f02n2c08.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // color no interlacing, filter-type 2
        { u"f03n0g08.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // grayscale no interlacing, filter-type 3
        { u"f03n2c08.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // color no interlacing, filter-type 3
        { u"f04n0g08.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // grayscale no interlacing, filter-type 4
        { u"f04n2c08.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // color no interlacing, filter-type 4
        { u"f99n0g04.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // grayscale bit-depth 4, filter changing per scanline
        // Additional palettes
        { u"pp0n2c16.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // six-cube palette-chunk in true-color image
        { u"pp0n6a08.png",
          {
              32,
              32,
              24,
              false,
              true,
          } }, // six-cube palette-chunk in true-color+alpha image
        { u"ps1n0g08.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // six-cube suggested palette (1 byte) in grayscale image
        { u"ps1n2c16.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // six-cube suggested palette (1 byte) in true-color image
        { u"ps2n0g08.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // six-cube suggested palette (2 bytes) in grayscale image
        { u"ps2n2c16.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // six-cube suggested palette (2 bytes) in true-color image
        // Ancillary chunks
        { u"ccwn2c08.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // chroma chunk w:0.31270.329 r:0.640.33 g:0.300.60 b:0.15,0.06
        { u"ccwn3p08.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // chroma chunk w:0.31270.329 r:0.640.33 g:0.300.60 b:0.15,0.06
        { u"cdfn2c08.png",
          {
              8,
              32,
              24,
              false,
              false,
          } }, // physical pixel dimensions, 8x32 flat pixels
        { u"cdhn2c08.png",
          {
              32,
              8,
              24,
              false,
              false,
          } }, // physical pixel dimensions, 32x8 high pixels
        { u"cdsn2c08.png",
          {
              8,
              8,
              24,
              false,
              false,
          } }, // physical pixel dimensions, 8x8 square pixels
        { u"cdun2c08.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // physical pixel dimensions, 1000 pixels per 1 meter
        { u"ch1n3p04.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // histogram 15 colors
        { u"ch2n3p08.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // histogram 256 colors
        { u"cm0n0g04.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // grayscale modification time, 01-jan-2000 12:34:56
        { u"cm7n0g04.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // grayscale modification time, 01-jan-1970 00:00:00
        { u"cm9n0g04.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // grayscale modification time, 31-dec-1999 23:59:59
        { u"cs3n2c16.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // color, 13 significant bits
        { u"cs3n3p08.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // paletted, 3 significant bits
        { u"cs5n2c08.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // color, 5 significant bits
        { u"cs5n3p08.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // paletted, 5 significant bits
        { u"cs8n2c08.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // color, 8 significant bits (reference)
        { u"cs8n3p08.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // paletted, 8 significant bits (reference)
        { u"ct0n0g04.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // grayscale no textual data
        { u"ct1n0g04.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // grayscale with textual data
        { u"ctzn0g04.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // grayscale with compressed textual data
        { u"cten0g04.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // grayscale international UTF-8, english
        { u"ctfn0g04.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // grayscale international UTF-8, finnish
        { u"ctgn0g04.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // grayscale international UTF-8, greek
        { u"cthn0g04.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // grayscale international UTF-8, hindi
        { u"ctjn0g04.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // grayscale international UTF-8, japanese
        { u"exif2c08.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // chunk with jpeg exif data
        // Chunk ordering
        { u"oi1n0g16.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // grayscale mother image with 1 idat-chunk
        { u"oi1n2c16.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // color mother image with 1 idat-chunk
        { u"oi2n0g16.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // grayscale image with 2 idat-chunks
        { u"oi2n2c16.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // color image with 2 idat-chunks
        { u"oi4n0g16.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // grayscale image with 4 unequal sized idat-chunks
        { u"oi4n2c16.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // color image with 4 unequal sized idat-chunks
        { u"oi9n0g16.png",
          {
              32,
              32,
              8,
              true,
              false,
          } }, // grayscale image with all idat-chunks length one
        { u"oi9n2c16.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // color image with all idat-chunks length one
        // Zlib compression
        { u"z00n2c08.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // color no interlacing, compression level 0 (none)
        { u"z03n2c08.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // color no interlacing, compression level 3
        { u"z06n2c08.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // color no interlacing, compression level 6 (default)
        { u"z09n2c08.png",
          {
              32,
              32,
              24,
              false,
              false,
          } }, // color no interlacing, compression level 9 (maximum)
    };

    for (const auto & [ aCaseName, aCase ] : aCases)
    {
        checkImportExportPng(getFullUrl(aCaseName), aCase);
    }

    OUString aCorruptedFilenames[] = {
        "xs1n0g01.png", // signature byte 1 MSBit reset to zero
        "xs2n0g01.png", // signature byte 2 is a 'Q'
        "xs4n0g01.png", // signature byte 4 lowercase
        "xs7n0g01.png", // 7th byte a space instead of control-Z
        "xcrn0g04.png", // added cr bytes
        "xlfn0g04.png", // added lf bytes
        "xhdn0g08.png", // incorrect IHDR checksum
        "xc1n0g08.png", // color type 1
        "xc9n2c08.png", // color type 9
        "xd0n2c08.png", // bit-depth 0
        "xd3n2c08.png", // bit-depth 3
        "xd9n2c08.png", // bit-depth 99
        "xdtn0g01.png", // missing IDAT chunk
        "xcsn0g01.png", // incorrect IDAT checksum
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

void PngFilterTest::testPngWrite1BitRGBPalette()
{
    SvMemoryStream aExportStream;
    {
        BitmapPalette aPal;
        aPal.SetEntryCount(2);
        aPal[0] = COL_RED;
        aPal[1] = COL_GREEN;
        Bitmap aBitmap(Size(16, 16), vcl::PixelFormat::N1_BPP, &aPal);
        {
            BitmapScopedWriteAccess pWriteAccessBitmap(aBitmap);
            // Top left
            for (int i = 0; i < 8; i++)
            {
                for (int j = 0; j < 8; j++)
                {
                    pWriteAccessBitmap->SetPixelIndex(i, j, 0);
                }
            }
            // Top right
            for (int i = 0; i < 8; i++)
            {
                for (int j = 8; j < 16; j++)
                {
                    pWriteAccessBitmap->SetPixelIndex(i, j, 1);
                }
            }
            // Bottom left
            for (int i = 8; i < 16; i++)
            {
                for (int j = 0; j < 8; j++)
                {
                    pWriteAccessBitmap->SetPixelIndex(i, j, 1);
                }
            }
            // Bottom right
            for (int i = 8; i < 16; i++)
            {
                for (int j = 8; j < 16; j++)
                {
                    pWriteAccessBitmap->SetPixelIndex(i, j, 0);
                }
            }
        }
        BitmapEx aBitmapEx(aBitmap);
        vcl::PngImageWriter aPngWriter(aExportStream);
        CPPUNIT_ASSERT_EQUAL(true, aPngWriter.write(aBitmapEx));
    }
    aExportStream.Seek(0);
    {
        vcl::PngImageReader aPngReader(aExportStream);
        BitmapEx aBitmapEx;
        CPPUNIT_ASSERT_EQUAL(true, aPngReader.read(aBitmapEx));

        CPPUNIT_ASSERT_EQUAL(16L, aBitmapEx.GetSizePixel().Width());
        CPPUNIT_ASSERT_EQUAL(16L, aBitmapEx.GetSizePixel().Height());

        CPPUNIT_ASSERT_EQUAL(COL_RED, aBitmapEx.GetPixelColor(0, 0));
        CPPUNIT_ASSERT_EQUAL(COL_RED, aBitmapEx.GetPixelColor(15, 15));
        CPPUNIT_ASSERT_EQUAL(COL_GREEN, aBitmapEx.GetPixelColor(15, 0));
        CPPUNIT_ASSERT_EQUAL(COL_GREEN, aBitmapEx.GetPixelColor(0, 15));
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(PngFilterTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
