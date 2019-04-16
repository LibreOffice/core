/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <unotest/filters-test.hxx>
#include <test/bootstrapfixture.hxx>

#include <vcl/virdev.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/wrkwin.hxx>

#include <tools/stream.hxx>
#include <vcl/pngwrite.hxx>

#include <vcl/graphicfilter.hxx>
#include <vcl/filter/PngImageReader.hxx>

static OUString const gaDataUrl = "/vcl/qa/cppunit/bitmaprender/data/";

class BitmapRenderTest : public test::BootstrapFixture
{
    OUString getFullUrl(const OUString& sFileName)
    {
        return m_directories.getURLFromSrc(gaDataUrl) + sFileName;
    }

public:
    BitmapRenderTest()
        : BootstrapFixture(true, false)
    {
    }

    void testTdf104141();
    void testTdf113918();
    void testDrawBitmap32();
    void testTdf116888();

    CPPUNIT_TEST_SUITE(BitmapRenderTest);
    CPPUNIT_TEST(testTdf104141);
    CPPUNIT_TEST(testTdf113918);
    CPPUNIT_TEST(testDrawBitmap32);
    CPPUNIT_TEST(testTdf116888);

    CPPUNIT_TEST_SUITE_END();
};

void BitmapRenderTest::testTdf104141()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    pVDev->SetOutputSizePixel(Size(400, 400));
    pVDev->SetBackground(Wallpaper(COL_GREEN));
    pVDev->Erase();

    // Load animated GIF and draw it on green background
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic;
    const OUString aURL(getFullUrl("tdf104141.gif"));
    SvFileStream aFileStream(aURL, StreamMode::READ);
    ErrCode bResult = rFilter.ImportGraphic(aGraphic, aURL, aFileStream);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, bResult);
    BitmapEx aBitmap = aGraphic.GetBitmapEx();
    pVDev->DrawBitmapEx(Point(20, 20), aBitmap);

    // Check drawing results: ensure that it contains transparent
    // (greenish) pixels
#if !defined MACOSX //TODO: on Mac colors are drifted, so exact compare fails
    const Color aColor = pVDev->GetPixel(Point(21, 21));
    CPPUNIT_ASSERT(aColor.GetGreen() > 10 * aColor.GetRed()
                   && aColor.GetGreen() > 10 * aColor.GetBlue());
#endif
}

void BitmapRenderTest::testTdf113918()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    pVDev->SetOutputSizePixel(Size(2480, 3508));
    pVDev->SetBackground(Wallpaper(COL_GREEN));
    pVDev->Erase();

    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic;
    const OUString aURL(getFullUrl("tdf113918.png"));
    SvFileStream aFileStream(aURL, StreamMode::READ);
    ErrCode bResult = rFilter.ImportGraphic(aGraphic, aURL, aFileStream);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, bResult);
    BitmapEx aBitmap = aGraphic.GetBitmapEx();
    pVDev->DrawBitmapEx(Point(0, 0), aBitmap);

    // Ensure that image is drawn with white background color from palette
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, pVDev->GetPixel(Point(21, 21)));

    // Ensure that image is drawn with gray text color from palette
    const Color aColor = pVDev->GetPixel(Point(1298, 1368));
    CPPUNIT_ASSERT(aColor.GetGreen() == aColor.GetRed() && aColor.GetGreen() == aColor.GetBlue());
    CPPUNIT_ASSERT(aColor.GetGreen() > 100);
}

void BitmapRenderTest::testDrawBitmap32()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    pVDev->SetOutputSizePixel(Size(8, 8));
    pVDev->SetBackground(Wallpaper(COL_WHITE));
    pVDev->Erase();

    CPPUNIT_ASSERT_EQUAL(COL_WHITE, pVDev->GetPixel(Point(0, 0)));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, pVDev->GetPixel(Point(1, 1)));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, pVDev->GetPixel(Point(2, 2)));

    SvFileStream aFileStream(getFullUrl("ImageRGBA.png"), StreamMode::READ);

    vcl::PngImageReader aPngReader(aFileStream);
    BitmapEx aBitmapEx;
    aPngReader.read(aBitmapEx);
    pVDev->DrawBitmapEx(Point(), aBitmapEx);

    CPPUNIT_ASSERT_EQUAL(COL_WHITE, pVDev->GetPixel(Point(0, 0)));
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, pVDev->GetPixel(Point(1, 1)));

// sometimes on Windows we get rounding error in blending so let's ignore this on Windows for now.
#if !defined(_WIN32)
    CPPUNIT_ASSERT_EQUAL(Color(0x00, 0x7F, 0xFF, 0x7F), pVDev->GetPixel(Point(2, 2)));
#endif
}

void BitmapRenderTest::testTdf116888()
{
    // The image is a 8bit image with a non-grayscale palette. In OpenGL mode
    // pdf export of the image was broken, because OpenGLSalBitmap::ReadTexture()
    // didn't handle 8bit non-grayscale and moreover OpenGLSalBitmap::AcquireBuffer()
    // didn't properly release mpUserBuffer after ReadTexture() failure.
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic;
    const OUString aURL(getFullUrl("tdf116888.gif"));
    SvFileStream aFileStream(aURL, StreamMode::READ);
    ErrCode bResult = rFilter.ImportGraphic(aGraphic, aURL, aFileStream);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, bResult);
    Bitmap aBitmap = aGraphic.GetBitmapEx().GetBitmap();
    CPPUNIT_ASSERT(!aBitmap.IsEmpty());
    aBitmap.Scale(0.8, 0.8); // This scaling discards mpUserData,
    Bitmap::ScopedReadAccess pAccess(aBitmap); // forcing ReadTexture() here.
    // Check that there is mpUserBuffer content.
    CPPUNIT_ASSERT(pAccess);
    const ScanlineFormat eFormat = pAccess->GetScanlineFormat();
    CPPUNIT_ASSERT_EQUAL(ScanlineFormat::N8BitPal, eFormat);
    CPPUNIT_ASSERT(!aBitmap.HasGreyPalette());
    // HACK: Some rendering backends change white to #FEFEFE while scaling for some reason.
    // That is pretty much white too in practice, so adjust for that.
    BitmapColor white(COL_WHITE);
    if (pAccess->GetColor(0, 0) == Color(0xfe, 0xfe, 0xfe))
        white = Color(0xfe, 0xfe, 0xfe);
    // Check that the image contents are also valid.
    CPPUNIT_ASSERT_EQUAL(white, pAccess->GetColor(0, 0));
    CPPUNIT_ASSERT_EQUAL(white, pAccess->GetColor(0, pAccess->Width() - 1));
    CPPUNIT_ASSERT_EQUAL(white, pAccess->GetColor(pAccess->Height() - 1, 0));
    CPPUNIT_ASSERT_EQUAL(BitmapColor(COL_BLACK),
                         pAccess->GetColor(pAccess->Height() - 1, pAccess->Width() - 1));
}

CPPUNIT_TEST_SUITE_REGISTRATION(BitmapRenderTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
