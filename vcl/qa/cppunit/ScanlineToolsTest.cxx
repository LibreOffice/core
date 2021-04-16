/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <bitmap/ScanlineTools.hxx>

namespace
{
class ScanlineToolsTest : public CppUnit::TestFixture
{
    void ScanlineTransformer_32_ARGB();
    void ScanlineTransformer_24_BGR();
    void ScanlineTransformer_8bit_Palette();
    void ScanlineTransformer_4bit_Palette();
    void ScanlineTransformer_1bit_Palette();

    CPPUNIT_TEST_SUITE(ScanlineToolsTest);
    CPPUNIT_TEST(ScanlineTransformer_32_ARGB);
    CPPUNIT_TEST(ScanlineTransformer_24_BGR);
    CPPUNIT_TEST(ScanlineTransformer_8bit_Palette);
    CPPUNIT_TEST(ScanlineTransformer_4bit_Palette);
    CPPUNIT_TEST(ScanlineTransformer_1bit_Palette);
    CPPUNIT_TEST_SUITE_END();
};

void ScanlineToolsTest::ScanlineTransformer_32_ARGB()
{
    BitmapPalette aPalette;
    std::unique_ptr<vcl::bitmap::ScanlineTransformer> pScanlineTransformer
        = vcl::bitmap::getScanlineTransformer(32, aPalette);

    std::vector<sal_uInt8> aScanLine(5 * 4, 0); // 5 * 4 BytesPerPixel
    pScanlineTransformer->startLine(aScanLine.data());

    std::vector<Color> aColors{
        Color(ColorAlpha, 255, 10, 250, 120), Color(ColorAlpha, 205, 30, 230, 110),
        Color(ColorAlpha, 155, 50, 210, 100), Color(ColorAlpha, 105, 70, 190, 90),
        Color(ColorAlpha, 55, 90, 170, 80),
    };

    for (Color const& aColor : aColors)
    {
        pScanlineTransformer->writePixel(aColor);
    }

    std::vector<sal_uInt8> aExpectedBytes{ 255, 10,  250, 120, 205, 30, 230, 110, 155, 50,
                                           210, 100, 105, 70,  190, 90, 55,  90,  170, 80 };

    for (size_t i = 0; i < aScanLine.size(); ++i)
    {
        CPPUNIT_ASSERT_EQUAL(int(aExpectedBytes[i]), int(aScanLine[i]));
    }
}

void ScanlineToolsTest::ScanlineTransformer_24_BGR()
{
    BitmapPalette aPalette;
    std::unique_ptr<vcl::bitmap::ScanlineTransformer> pScanlineTransformer
        = vcl::bitmap::getScanlineTransformer(24, aPalette);

    std::vector<sal_uInt8> aScanLine(5 * 3, 0); // 5 * 3 BytesPerPixel
    pScanlineTransformer->startLine(aScanLine.data());

    std::vector<Color> aColors{
        Color(ColorTransparency, 0, 10, 250, 120),   Color(ColorTransparency, 50, 30, 230, 110),
        Color(ColorTransparency, 100, 50, 210, 100), Color(ColorTransparency, 150, 70, 190, 90),
        Color(ColorTransparency, 200, 90, 170, 80),
    };

    for (Color const& aColor : aColors)
    {
        pScanlineTransformer->writePixel(aColor);
    }

    std::vector<sal_uInt8> aExpectedBytes{ 120, 250, 10,  110, 230, 30,  100, 210,
                                           50,  90,  190, 70,  80,  170, 90 };

    for (size_t i = 0; i < aScanLine.size(); ++i)
    {
        CPPUNIT_ASSERT_EQUAL(int(aExpectedBytes[i]), int(aScanLine[i]));
    }
}

void ScanlineToolsTest::ScanlineTransformer_8bit_Palette()
{
    std::vector<Color> aColors{
        Color(ColorTransparency, 0, 10, 250, 120),   Color(ColorTransparency, 50, 30, 230, 110),
        Color(ColorTransparency, 100, 50, 210, 100), Color(ColorTransparency, 150, 70, 190, 90),
        Color(ColorTransparency, 200, 90, 170, 80),
    };

    BitmapPalette aPalette(256);
    for (size_t i = 0; i < aColors.size(); ++i)
        aPalette[i] = aColors[i];

    std::unique_ptr<vcl::bitmap::ScanlineTransformer> pScanlineTransformer
        = vcl::bitmap::getScanlineTransformer(8, aPalette);

    std::vector<sal_uInt8> aScanLine(5, 0); // 5 * 1 BytesPerPixel
    pScanlineTransformer->startLine(aScanLine.data());

    for (Color const& aColor : aColors)
    {
        pScanlineTransformer->writePixel(aColor);
    }

    std::vector<sal_uInt8> aExpectedBytes{ 0, 1, 2, 3, 4 };

    for (size_t i = 0; i < aScanLine.size(); ++i)
    {
        CPPUNIT_ASSERT_EQUAL(int(aExpectedBytes[i]), int(aScanLine[i]));
    }

    pScanlineTransformer->startLine(aScanLine.data());

    for (size_t i = 0; i < aColors.size(); ++i)
    {
        Color aColor = pScanlineTransformer->readPixel();
        CPPUNIT_ASSERT_EQUAL(aColors[i], aColor);
    }
}

void ScanlineToolsTest::ScanlineTransformer_4bit_Palette()
{
    std::vector<Color> aColors{
        Color(10, 250, 120), Color(30, 230, 110), Color(50, 210, 100),
        Color(70, 190, 90),  Color(90, 170, 80),  Color(110, 150, 70),
    };

    BitmapPalette aPalette(16);
    for (size_t i = 0; i < aColors.size(); ++i)
    {
        aPalette[i] = aColors[i];
    }

    std::unique_ptr<vcl::bitmap::ScanlineTransformer> pScanlineTransformer
        = vcl::bitmap::getScanlineTransformer(4, aPalette);

    std::vector<sal_uInt8> aScanLine(3, 0); // 6 * 0.5 BytesPerPixel
    pScanlineTransformer->startLine(aScanLine.data());

    for (Color const& aColor : aColors)
    {
        pScanlineTransformer->writePixel(aColor);
    }

    std::vector<sal_uInt8> aExpectedBytes{ 0x01, 0x23, 0x45 };

    for (size_t i = 0; i < aScanLine.size(); ++i)
    {
        CPPUNIT_ASSERT_EQUAL(int(aExpectedBytes[i]), int(aScanLine[i]));
    }

    pScanlineTransformer->startLine(aScanLine.data());

    for (size_t i = 0; i < aColors.size(); ++i)
    {
        Color aColor = pScanlineTransformer->readPixel();
        CPPUNIT_ASSERT_EQUAL(aColors[i], aColor);
    }
}

void ScanlineToolsTest::ScanlineTransformer_1bit_Palette()
{
    std::vector<Color> aColors{
        Color(10, 250, 120), Color(30, 230, 110), Color(50, 210, 100), Color(70, 190, 90),
        Color(90, 170, 80),  Color(110, 150, 70), Color(130, 130, 60), Color(150, 110, 50),
        Color(170, 90, 40),  Color(190, 70, 30),  Color(210, 50, 20),  Color(230, 30, 10),
        Color(250, 10, 0),
    };

    BitmapPalette aPalette(2);
    aPalette[0] = Color(10, 250, 120);
    aPalette[1] = Color(110, 150, 70);

    std::unique_ptr<vcl::bitmap::ScanlineTransformer> pScanlineTransformer
        = vcl::bitmap::getScanlineTransformer(1, aPalette);

    std::vector<sal_uInt8> aScanLine(2, 0); // 13 * 1/8 BytesPerPixel
    pScanlineTransformer->startLine(aScanLine.data());

    for (Color const& aColor : aColors)
    {
        pScanlineTransformer->writePixel(aColor);
    }

    std::vector<sal_uInt8> aExpectedBytes{
        // We expect 3x index 0 and 10x index 1 => 000 111111111
        0x1f, // 0001 1111
        0xf8 // 1111 1000
    };

    for (size_t i = 0; i < aScanLine.size(); ++i)
    {
        CPPUNIT_ASSERT_EQUAL(int(aExpectedBytes[i]), int(aScanLine[i]));
    }

    pScanlineTransformer->startLine(aScanLine.data());

    std::vector<Color> aColorsExpected{
        Color(10, 250, 120), Color(10, 250, 120), Color(10, 250, 120), Color(110, 150, 70),
        Color(110, 150, 70), Color(110, 150, 70), Color(110, 150, 70), Color(110, 150, 70),
        Color(110, 150, 70), Color(110, 150, 70), Color(110, 150, 70), Color(110, 150, 70),
        Color(110, 150, 70),
    };

    for (size_t i = 0; i < aColors.size(); ++i)
    {
        Color aColor = pScanlineTransformer->readPixel();
        CPPUNIT_ASSERT_EQUAL(aColorsExpected[i], aColor);
    }
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(ScanlineToolsTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
