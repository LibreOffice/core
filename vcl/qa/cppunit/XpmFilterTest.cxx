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
#include <filter/XpmReader.hxx>
#include <unotools/tempfile.hxx>

class XpmFilterTest : public test::BootstrapFixture
{
public:
    OUString maDataUrl;
    OUString getFullUrl(std::u16string_view sFileName)
    {
        return m_directories.getURLFromSrc(maDataUrl) + sFileName;
    }
    XpmFilterTest()
        : maDataUrl(u"/vcl/qa/cppunit/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(XpmFilterTest, testXPM_8bit)
{
    SvFileStream aFileStream(getFullUrl(u"XPM_8.xpm"), StreamMode::READ);
    Graphic aGraphic;
    CPPUNIT_ASSERT(ImportXPM(aFileStream, aGraphic));
    auto aBitmap = aGraphic.GetBitmapEx();
    CPPUNIT_ASSERT_EQUAL(tools::Long(4), aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(4), aBitmap.GetSizePixel().Height());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(8), aBitmap.GetBitCount());
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTBLUE, aBitmap.GetPixelColor(0, 0));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, aBitmap.GetPixelColor(2, 0));
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, aBitmap.GetPixelColor(0, 2));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTGREEN, aBitmap.GetPixelColor(2, 2));
}

CPPUNIT_TEST_FIXTURE(XpmFilterTest, testXPM_4bit)
{
    SvFileStream aFileStream(getFullUrl(u"XPM_4.xpm"), StreamMode::READ);
    Graphic aGraphic;
    CPPUNIT_ASSERT(ImportXPM(aFileStream, aGraphic));
    auto aBitmap = aGraphic.GetBitmapEx();
    CPPUNIT_ASSERT_EQUAL(tools::Long(4), aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(4), aBitmap.GetSizePixel().Height());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(4), aBitmap.GetBitCount());
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTBLUE, aBitmap.GetPixelColor(0, 0));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, aBitmap.GetPixelColor(2, 0));
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, aBitmap.GetPixelColor(0, 2));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTGREEN, aBitmap.GetPixelColor(2, 2));
}

CPPUNIT_TEST_FIXTURE(XpmFilterTest, testXPM_1bit)
{
    SvFileStream aFileStream(getFullUrl(u"XPM_1.xpm"), StreamMode::READ);
    Graphic aGraphic;
    CPPUNIT_ASSERT(ImportXPM(aFileStream, aGraphic));
    auto aBitmap = aGraphic.GetBitmapEx();
    CPPUNIT_ASSERT_EQUAL(tools::Long(10), aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(10), aBitmap.GetSizePixel().Height());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), aBitmap.GetBitCount());
    CPPUNIT_ASSERT_EQUAL(Color(0xffffff), aBitmap.GetPixelColor(0, 0));
    CPPUNIT_ASSERT_EQUAL(Color(0x72d1c8), aBitmap.GetPixelColor(1, 1));
    CPPUNIT_ASSERT_EQUAL(Color(0x72d1c8), aBitmap.GetPixelColor(8, 8));
    CPPUNIT_ASSERT_EQUAL(Color(0xffffff), aBitmap.GetPixelColor(9, 9));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
