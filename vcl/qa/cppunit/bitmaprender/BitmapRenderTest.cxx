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
#include <vcl/salbtype.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/wrkwin.hxx>

#include <tools/stream.hxx>
#include <vcl/pngwrite.hxx>

#include <vcl/graphicfilter.hxx>

class BitmapRenderTest : public test::BootstrapFixture
{
    OUString maDataUrl;

    OUString getFullUrl(const OUString& sFileName)
    {
        return m_directories.getURLFromSrc(maDataUrl) + sFileName;
    }

public:
    BitmapRenderTest()
        : BootstrapFixture(true, false)
        , maDataUrl("/vcl/qa/cppunit/bitmaprender/data/")
    {
    }

    void testTdf104141();
    void testTdf113918();

    CPPUNIT_TEST_SUITE(BitmapRenderTest);
    CPPUNIT_TEST(testTdf104141);
    CPPUNIT_TEST(testTdf113918);
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

CPPUNIT_TEST_SUITE_REGISTRATION(BitmapRenderTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
