/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sal/config.h>

#include <string_view>

#include <unotest/bootstrapfixturebase.hxx>
#include <vcl/graphicfilter.hxx>
#include <bitmapwriteaccess.hxx>
#include <tools/stream.hxx>

constexpr OUStringLiteral gaDataUrl(u"/vcl/qa/cppunit/jpeg/data/");

class JpegReaderTest : public test::BootstrapFixtureBase
{
    OUString getFullUrl(std::u16string_view sFileName)
    {
        return m_directories.getURLFromSrc(gaDataUrl) + sFileName;
    }

    Graphic loadJPG(const OUString& aURL);

public:
    void testReadRGB();
    void testReadGray();
    void testReadCMYK();
    void testTdf138950();

    CPPUNIT_TEST_SUITE(JpegReaderTest);
    CPPUNIT_TEST(testReadRGB);
    CPPUNIT_TEST(testReadGray);
    CPPUNIT_TEST(testReadCMYK);
    CPPUNIT_TEST(testTdf138950);
    CPPUNIT_TEST_SUITE_END();
};

static int deltaColor(BitmapColor aColor1, BitmapColor aColor2)
{
    int deltaR = std::abs(aColor1.GetRed() - aColor2.GetRed());
    int deltaG = std::abs(aColor1.GetGreen() - aColor2.GetGreen());
    int deltaB = std::abs(aColor1.GetBlue() - aColor2.GetBlue());

    return std::max(std::max(deltaR, deltaG), deltaB);
}

static bool checkRect(Bitmap& rBitmap, int aLayerNumber, tools::Long nAreaHeight,
                      tools::Long nAreaWidth, Color aExpectedColor, int nMaxDelta)
{
    BitmapScopedWriteAccess pAccess(rBitmap);

    tools::Long nWidth = std::min(nAreaWidth, pAccess->Width());
    tools::Long nHeight = std::min(nAreaHeight, pAccess->Height());

    tools::Long firstX = 0 + aLayerNumber;
    tools::Long firstY = 0 + aLayerNumber;

    tools::Long lastX = nWidth - 1 - aLayerNumber;
    tools::Long lastY = nHeight - 1 - aLayerNumber;

    int delta;

    for (tools::Long y = firstY; y <= lastY; y++)
    {
        Color aColorFirst = pAccess->GetPixel(y, firstX);
        delta = deltaColor(aColorFirst, aExpectedColor);
        if (delta > nMaxDelta)
            return false;

        Color aColorLast = pAccess->GetPixel(y, lastX);
        delta = deltaColor(aColorLast, aExpectedColor);
        if (delta > nMaxDelta)
            return false;
    }
    for (tools::Long x = firstX; x <= lastX; x++)
    {
        Color aColorFirst = pAccess->GetPixel(firstY, x);
        delta = deltaColor(aColorFirst, aExpectedColor);
        if (delta > nMaxDelta)
            return false;

        Color aColorLast = pAccess->GetPixel(lastY, x);
        delta = deltaColor(aColorLast, aExpectedColor);
        if (delta > nMaxDelta)
            return false;
    }
    return true;
}

static int getNumberOfImageComponents(const Graphic& rGraphic)
{
    GfxLink aLink = rGraphic.GetGfxLink();
    SvMemoryStream aMemoryStream(const_cast<sal_uInt8*>(aLink.GetData()), aLink.GetDataSize(),
                                 StreamMode::READ | StreamMode::WRITE);
    GraphicDescriptor aDescriptor(aMemoryStream, nullptr);
    CPPUNIT_ASSERT(aDescriptor.Detect(true));
    return aDescriptor.GetNumberOfImageComponents();
}

Graphic JpegReaderTest::loadJPG(const OUString& aURL)
{
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic;
    SvFileStream aFileStream(aURL, StreamMode::READ);
    ErrCode bResult = rFilter.ImportGraphic(aGraphic, aURL, aFileStream);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, bResult);
    return aGraphic;
}

void JpegReaderTest::testReadRGB()
{
    Graphic aGraphic = loadJPG(getFullUrl(u"JPEGTestRGB.jpeg"));
    Bitmap aBitmap = aGraphic.GetBitmapEx().GetBitmap();
    Size aSize = aBitmap.GetSizePixel();
    CPPUNIT_ASSERT_EQUAL(tools::Long(12), aSize.Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(12), aSize.Height());

    int nMaxDelta = 1; // still acceptable color error
    CPPUNIT_ASSERT(checkRect(aBitmap, 0, 8, 8, Color(0xff, 0xff, 0xff), nMaxDelta));
    CPPUNIT_ASSERT(checkRect(aBitmap, 1, 8, 8, Color(0xff, 0x00, 0x00), nMaxDelta));
    CPPUNIT_ASSERT(checkRect(aBitmap, 2, 8, 8, Color(0x00, 0xff, 0x00), nMaxDelta));
    CPPUNIT_ASSERT(checkRect(aBitmap, 3, 8, 8, Color(0x00, 0x00, 0xff), nMaxDelta));

    CPPUNIT_ASSERT_EQUAL(3, getNumberOfImageComponents(aGraphic));
}

void JpegReaderTest::testReadGray()
{
    Graphic aGraphic = loadJPG(getFullUrl(u"JPEGTestGray.jpeg"));
    Bitmap aBitmap = aGraphic.GetBitmapEx().GetBitmap();
    Size aSize = aBitmap.GetSizePixel();
    CPPUNIT_ASSERT_EQUAL(tools::Long(12), aSize.Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(12), aSize.Height());

    aBitmap.Convert(
        BmpConversion::N24Bit); // convert to 24bit so we don't need to deal with palette

    int nMaxDelta = 1;
    CPPUNIT_ASSERT(checkRect(aBitmap, 0, 8, 8, Color(0xff, 0xff, 0xff), nMaxDelta));
    CPPUNIT_ASSERT(checkRect(aBitmap, 1, 8, 8, Color(0x36, 0x36, 0x36), nMaxDelta));
    CPPUNIT_ASSERT(checkRect(aBitmap, 2, 8, 8, Color(0xb6, 0xb6, 0xb6), nMaxDelta));
    CPPUNIT_ASSERT(checkRect(aBitmap, 3, 8, 8, Color(0x12, 0x12, 0x12), nMaxDelta));

    CPPUNIT_ASSERT_EQUAL(1, getNumberOfImageComponents(aGraphic));
}

void JpegReaderTest::testReadCMYK()
{
    Graphic aGraphic = loadJPG(getFullUrl(u"JPEGTestCMYK.jpeg"));
    Bitmap aBitmap = aGraphic.GetBitmapEx().GetBitmap();
    Size aSize = aBitmap.GetSizePixel();
    CPPUNIT_ASSERT_EQUAL(tools::Long(12), aSize.Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(12), aSize.Height());

    int maxDelta = 1;
    CPPUNIT_ASSERT(checkRect(aBitmap, 0, 8, 8, Color(0xff, 0xff, 0xff), maxDelta));
    CPPUNIT_ASSERT(checkRect(aBitmap, 1, 8, 8, Color(0xff, 0x00, 0x00), maxDelta));
    CPPUNIT_ASSERT(checkRect(aBitmap, 2, 8, 8, Color(0x00, 0xff, 0x00), maxDelta));
    CPPUNIT_ASSERT(checkRect(aBitmap, 3, 8, 8, Color(0x00, 0x00, 0xff), maxDelta));

    CPPUNIT_ASSERT_EQUAL(4, getNumberOfImageComponents(aGraphic));
}

void JpegReaderTest::testTdf138950()
{
    Graphic aGraphic = loadJPG(getFullUrl(u"tdf138950.jpeg"));
    Bitmap aBitmap = aGraphic.GetBitmapEx().GetBitmap();
    Size aSize = aBitmap.GetSizePixel();
    CPPUNIT_ASSERT_EQUAL(tools::Long(720), aSize.Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(1280), aSize.Height());

    Bitmap::ScopedReadAccess pReadAccess(aBitmap);
    int nBlackCount = 0;
    for (tools::Long nY = 0; nY < aSize.Height(); ++nY)
    {
        for (tools::Long nX = 0; nX < aSize.Width(); ++nX)
        {
            const Color aColor = pReadAccess->GetColor(nY, nX);
            if ((aColor.GetRed() == 0x00) && (aColor.GetGreen() == 0x00)
                && (aColor.GetBlue() == 0x00))
                ++nBlackCount;
        }
    }

    // Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 921600
    CPPUNIT_ASSERT_EQUAL(0, nBlackCount);
}

CPPUNIT_TEST_SUITE_REGISTRATION(JpegReaderTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
