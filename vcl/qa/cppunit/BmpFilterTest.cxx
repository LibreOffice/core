/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>

#include <vcl/bitmapex.hxx>
#include <tools/stream.hxx>
#include <filter/BmpReader.hxx>
#include <unotools/tempfile.hxx>

class BmpFilterTest : public test::BootstrapFixture
{
public:
    OUString maDataUrl;
    OUString getFullUrl(std::u16string_view sFileName)
    {
        return m_directories.getURLFromSrc(maDataUrl) + sFileName;
    }
    BmpFilterTest()
        : maDataUrl(u"/vcl/qa/cppunit/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(BmpFilterTest, testBMP_RGB_888)
{
    SvFileStream aFileStream(getFullUrl(u"BMP_R8G8B8.bmp"), StreamMode::READ);
    Graphic aGraphic;
    CPPUNIT_ASSERT(BmpReader(aFileStream, aGraphic));
    auto aBitmap = aGraphic.GetBitmapEx();
    CPPUNIT_ASSERT_EQUAL(10L, aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(10L, aBitmap.GetSizePixel().Height());
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, aBitmap.GetPixelColor(0, 0));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, aBitmap.GetPixelColor(9, 9));
    CPPUNIT_ASSERT_EQUAL(Color(0x72, 0xd1, 0xc8), aBitmap.GetPixelColor(1, 1));
    CPPUNIT_ASSERT_EQUAL(Color(0x72, 0xd1, 0xc8), aBitmap.GetPixelColor(8, 8));
}

CPPUNIT_TEST_FIXTURE(BmpFilterTest, testBMP_RGB_565)
{
    SvFileStream aFileStream(getFullUrl(u"BMP_R5G6B5.bmp"), StreamMode::READ);
    Graphic aGraphic;
    CPPUNIT_ASSERT(BmpReader(aFileStream, aGraphic));
    auto aBitmap = aGraphic.GetBitmapEx();
    CPPUNIT_ASSERT_EQUAL(10L, aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(10L, aBitmap.GetSizePixel().Height());

    // White is not completely white
    //CPPUNIT_ASSERT_EQUAL(Color(0xf8, 0xfc, 0xf8), aBitmap.GetPixelColor(0, 0));
    //CPPUNIT_ASSERT_EQUAL(Color(0xf8, 0xfc, 0xf8), aBitmap.GetPixelColor(9, 9));

    //CPPUNIT_ASSERT_EQUAL(Color(0x70, 0xd0, 0xc0), aBitmap.GetPixelColor(1, 1));
    //CPPUNIT_ASSERT_EQUAL(Color(0x70, 0xd0, 0xc0), aBitmap.GetPixelColor(8, 8));
}

CPPUNIT_TEST_FIXTURE(BmpFilterTest, testBMP_32_ARGB_8888)
{
    SvFileStream aFileStream(getFullUrl(u"BMP_A8R8G8B8.bmp"), StreamMode::READ);
    Graphic aGraphic;
    CPPUNIT_ASSERT(BmpReader(aFileStream, aGraphic));
    auto aBitmap = aGraphic.GetBitmapEx();
    CPPUNIT_ASSERT_EQUAL(4L, aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(4L, aBitmap.GetSizePixel().Height());

    //CPPUNIT_ASSERT_EQUAL(COL_LIGHTBLUE, aBitmap.GetPixelColor(0, 0));
    //CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, aBitmap.GetPixelColor(2, 0));
    //CPPUNIT_ASSERT_EQUAL(COL_YELLOW, aBitmap.GetPixelColor(0, 2));
    //CPPUNIT_ASSERT_EQUAL(COL_LIGHTGREEN, aBitmap.GetPixelColor(2, 2));
}

CPPUNIT_TEST_FIXTURE(BmpFilterTest, testBMP_Paint_24_RGB_888)
{
    SvFileStream aFileStream(getFullUrl(u"BMP_Paint_24bit.bmp"), StreamMode::READ);
    Graphic aGraphic;
    CPPUNIT_ASSERT(BmpReader(aFileStream, aGraphic));
    auto aBitmap = aGraphic.GetBitmapEx();
    CPPUNIT_ASSERT_EQUAL(4L, aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(4L, aBitmap.GetSizePixel().Height());

    CPPUNIT_ASSERT_EQUAL(COL_LIGHTBLUE, aBitmap.GetPixelColor(0, 0));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, aBitmap.GetPixelColor(2, 0));
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, aBitmap.GetPixelColor(0, 2));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTGREEN, aBitmap.GetPixelColor(2, 2));
}

CPPUNIT_TEST_FIXTURE(BmpFilterTest, testBMP_Index_1BPP)
{
    SvFileStream aFileStream(getFullUrl(u"BMP_Paint_1bit.bmp"), StreamMode::READ);
    Graphic aGraphic;
    CPPUNIT_ASSERT(BmpReader(aFileStream, aGraphic));
    auto aBitmap = aGraphic.GetBitmapEx();
    CPPUNIT_ASSERT_EQUAL(4L, aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(4L, aBitmap.GetSizePixel().Height());

    CPPUNIT_ASSERT_EQUAL(COL_BLACK, aBitmap.GetPixelColor(0, 0));
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, aBitmap.GetPixelColor(0, 2));
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, aBitmap.GetPixelColor(2, 0));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, aBitmap.GetPixelColor(2, 2));
}

CPPUNIT_TEST_FIXTURE(BmpFilterTest, testBMP_Index_4BPP)
{
    SvFileStream aFileStream(getFullUrl(u"BMP_Paint_4bit.bmp"), StreamMode::READ);
    Graphic aGraphic;
    CPPUNIT_ASSERT(BmpReader(aFileStream, aGraphic));
    auto aBitmap = aGraphic.GetBitmapEx();
    CPPUNIT_ASSERT_EQUAL(4L, aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(4L, aBitmap.GetSizePixel().Height());

    CPPUNIT_ASSERT_EQUAL(COL_LIGHTBLUE, aBitmap.GetPixelColor(0, 0));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, aBitmap.GetPixelColor(2, 0));
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, aBitmap.GetPixelColor(0, 2));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTGREEN, aBitmap.GetPixelColor(2, 2));
}

CPPUNIT_TEST_FIXTURE(BmpFilterTest, testBMP_Index_8BPP)
{
    SvFileStream aFileStream(getFullUrl(u"BMP_Paint_8bit.bmp"), StreamMode::READ);
    Graphic aGraphic;
    CPPUNIT_ASSERT(BmpReader(aFileStream, aGraphic));
    auto aBitmap = aGraphic.GetBitmapEx();
    CPPUNIT_ASSERT_EQUAL(4L, aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(4L, aBitmap.GetSizePixel().Height());

    CPPUNIT_ASSERT_EQUAL(COL_LIGHTBLUE, aBitmap.GetPixelColor(0, 0));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, aBitmap.GetPixelColor(2, 0));
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, aBitmap.GetPixelColor(0, 2));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTGREEN, aBitmap.GetPixelColor(2, 2));
}

CPPUNIT_TEST_FIXTURE(BmpFilterTest, testBMP_Index_8BPP_RLE)
{
    SvFileStream aFileStream(getFullUrl(u"BMP_8bit_RLE.bmp"), StreamMode::READ);
    Graphic aGraphic;
    CPPUNIT_ASSERT(BmpReader(aFileStream, aGraphic));
    auto aBitmap = aGraphic.GetBitmapEx();
    CPPUNIT_ASSERT_EQUAL(4L, aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(4L, aBitmap.GetSizePixel().Height());

    CPPUNIT_ASSERT_EQUAL(COL_LIGHTBLUE, aBitmap.GetPixelColor(0, 0));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, aBitmap.GetPixelColor(2, 0));
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, aBitmap.GetPixelColor(0, 2));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTGREEN, aBitmap.GetPixelColor(2, 2));
}

CPPUNIT_TEST_FIXTURE(BmpFilterTest, testBMP_V4)
{
    SvFileStream aFileStream(getFullUrl(u"BMP_RLE.bmp"), StreamMode::READ);
    Graphic aGraphic;
    CPPUNIT_ASSERT(BmpReader(aFileStream, aGraphic));
    auto aBitmap = aGraphic.GetBitmapEx();
    CPPUNIT_ASSERT_EQUAL(4L, aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(4L, aBitmap.GetSizePixel().Height());

    CPPUNIT_ASSERT_EQUAL(COL_LIGHTBLUE, aBitmap.GetPixelColor(0, 0));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, aBitmap.GetPixelColor(2, 0));
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, aBitmap.GetPixelColor(0, 2));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTGREEN, aBitmap.GetPixelColor(2, 2));
}

CPPUNIT_TEST_FIXTURE(BmpFilterTest, testBMP_V3)
{
    SvFileStream aFileStream(getFullUrl(u"BMP_RLE_V3.bmp"), StreamMode::READ);
    Graphic aGraphic;
    CPPUNIT_ASSERT(BmpReader(aFileStream, aGraphic));
    auto aBitmap = aGraphic.GetBitmapEx();
    CPPUNIT_ASSERT_EQUAL(4L, aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(4L, aBitmap.GetSizePixel().Height());

    CPPUNIT_ASSERT_EQUAL(COL_LIGHTBLUE, aBitmap.GetPixelColor(0, 0));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, aBitmap.GetPixelColor(2, 0));
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, aBitmap.GetPixelColor(0, 2));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTGREEN, aBitmap.GetPixelColor(2, 2));
}

CPPUNIT_TEST_FIXTURE(BmpFilterTest, testBMP_V2)
{
    SvFileStream aFileStream(getFullUrl(u"BMP_RLE_V2.bmp"), StreamMode::READ);
    Graphic aGraphic;
    CPPUNIT_ASSERT(BmpReader(aFileStream, aGraphic));
    auto aBitmap = aGraphic.GetBitmapEx();
    CPPUNIT_ASSERT_EQUAL(4L, aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(4L, aBitmap.GetSizePixel().Height());

    CPPUNIT_ASSERT_EQUAL(COL_LIGHTBLUE, aBitmap.GetPixelColor(0, 0));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, aBitmap.GetPixelColor(2, 0));
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, aBitmap.GetPixelColor(0, 2));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTGREEN, aBitmap.GetPixelColor(2, 2));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
