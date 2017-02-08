/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <test/setupvcl.hxx>
#include <unotest/bootstrapfixturebase.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/bitmapaccess.hxx>

class JpegReaderTest : public test::BootstrapFixtureBase
{
    OUString maDataUrl;

    OUString getFullUrl(const OUString& sFileName)
    {
        return m_directories.getURLFromSrc(maDataUrl) + sFileName;
    }

    Bitmap loadJPG(const OUString& aURL);

public:
    JpegReaderTest() :
        maDataUrl("/vcl/qa/cppunit/jpeg/data/")
    {}

    void testReadRGB();
    void testReadGray();
    void testReadCMYK();

    CPPUNIT_TEST_SUITE(JpegReaderTest);
    CPPUNIT_TEST(testReadRGB);
    CPPUNIT_TEST(testReadGray);
    CPPUNIT_TEST(testReadCMYK);
    CPPUNIT_TEST_SUITE_END();
};

int deltaColor(BitmapColor aColor1, BitmapColor aColor2)
{
    int deltaR = std::abs(aColor1.GetRed()   - aColor2.GetRed());
    int deltaG = std::abs(aColor1.GetGreen() - aColor2.GetGreen());
    int deltaB = std::abs(aColor1.GetBlue()  - aColor2.GetBlue());

    return std::max(std::max(deltaR, deltaG), deltaB);
}

bool checkRect(Bitmap& rBitmap, int aLayerNumber, long nAreaHeight, long nAreaWidth, Color aExpectedColor, int nMaxDelta)
{
    Bitmap::ScopedWriteAccess pAccess(rBitmap);

    long nWidth  = std::min(nAreaWidth,  pAccess->Width());
    long nHeight = std::min(nAreaHeight, pAccess->Height());

    long firstX = 0 + aLayerNumber;
    long firstY = 0 + aLayerNumber;

    long lastX = nWidth  - 1 - aLayerNumber;
    long lastY = nHeight - 1 - aLayerNumber;

    int delta;

    for (long y = firstY; y <= lastY; y++)
    {
        Color aColorFirst = pAccess->GetPixel(y, firstX);
        delta = deltaColor(aColorFirst, aExpectedColor);
        if (delta > nMaxDelta)
            return false;

        Color aColorLast  = pAccess->GetPixel(y, lastX);
        delta = deltaColor(aColorLast, aExpectedColor);
        if (delta > nMaxDelta)
            return false;
    }
    for (long x = firstX; x <= lastX; x++)
    {
        Color aColorFirst = pAccess->GetPixel(firstY, x);
        delta = deltaColor(aColorFirst, aExpectedColor);
        if (delta > nMaxDelta)
            return false;

        Color aColorLast  = pAccess->GetPixel(lastY, x);
        delta = deltaColor(aColorLast, aExpectedColor);
        if (delta > nMaxDelta)
            return false;
    }
    return true;
}

Bitmap JpegReaderTest::loadJPG(const OUString& aURL)
{
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic;
    SvFileStream aFileStream(aURL, StreamMode::READ);
    bool bResult = rFilter.ImportGraphic(aGraphic, aURL, aFileStream) == 0;
    CPPUNIT_ASSERT(bResult);
    return aGraphic.GetBitmapEx().GetBitmap();
}

void JpegReaderTest::testReadRGB()
{
    Bitmap aBitmap = loadJPG(getFullUrl("JPEGTestRGB.jpeg"));
    Size aSize = aBitmap.GetSizePixel();
    CPPUNIT_ASSERT_EQUAL(12L, aSize.Width());
    CPPUNIT_ASSERT_EQUAL(12L, aSize.Height());

    int nMaxDelta = 1; // still acceptable color error
    CPPUNIT_ASSERT(checkRect(aBitmap, 0, 8, 8, Color(0xff, 0xff, 0xff), nMaxDelta));
    CPPUNIT_ASSERT(checkRect(aBitmap, 1, 8, 8, Color(0xff, 0x00, 0x00), nMaxDelta));
    CPPUNIT_ASSERT(checkRect(aBitmap, 2, 8, 8, Color(0x00, 0xff, 0x00), nMaxDelta));
    CPPUNIT_ASSERT(checkRect(aBitmap, 3, 8, 8, Color(0x00, 0x00, 0xff), nMaxDelta));
}

void JpegReaderTest::testReadGray()
{
    Bitmap aBitmap = loadJPG(getFullUrl("JPEGTestGray.jpeg"));
    Size aSize = aBitmap.GetSizePixel();
    CPPUNIT_ASSERT_EQUAL(12L, aSize.Width());
    CPPUNIT_ASSERT_EQUAL(12L, aSize.Height());

    aBitmap.Convert(BmpConversion::N24Bit); // convert to 24bit so we don't need to deal with palette

    int nMaxDelta = 1;
    CPPUNIT_ASSERT(checkRect(aBitmap, 0, 8, 8, Color(0xff, 0xff, 0xff), nMaxDelta));
    CPPUNIT_ASSERT(checkRect(aBitmap, 1, 8, 8, Color(0x36, 0x36, 0x36), nMaxDelta));
    CPPUNIT_ASSERT(checkRect(aBitmap, 2, 8, 8, Color(0xb6, 0xb6, 0xb6), nMaxDelta));
    CPPUNIT_ASSERT(checkRect(aBitmap, 3, 8, 8, Color(0x12, 0x12, 0x12), nMaxDelta));
}

void JpegReaderTest::testReadCMYK()
{
    Bitmap aBitmap = loadJPG(getFullUrl("JPEGTestCMYK.jpeg"));
    Size aSize = aBitmap.GetSizePixel();
    CPPUNIT_ASSERT_EQUAL(12L, aSize.Width());
    CPPUNIT_ASSERT_EQUAL(12L, aSize.Height());

    int maxDelta = 1;
    CPPUNIT_ASSERT(checkRect(aBitmap, 0, 8, 8, Color(0xff, 0xff, 0xff), maxDelta));
    CPPUNIT_ASSERT(checkRect(aBitmap, 1, 8, 8, Color(0xff, 0x00, 0x00), maxDelta));
    CPPUNIT_ASSERT(checkRect(aBitmap, 2, 8, 8, Color(0x00, 0xff, 0x00), maxDelta));
    CPPUNIT_ASSERT(checkRect(aBitmap, 3, 8, 8, Color(0x00, 0x00, 0xff), maxDelta));
}

CPPUNIT_TEST_SUITE_REGISTRATION(JpegReaderTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
