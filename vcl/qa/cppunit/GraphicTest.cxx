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
#include <cppunit/plugin/TestPlugIn.h>

#include <com/sun/star/beans/PropertyValue.hpp>

#include <vcl/bitmapaccess.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <tools/stream.hxx>
#include <unotest/directories.hxx>

using namespace css;

namespace
{
class GraphicTest : public CppUnit::TestFixture
{
    void testUnloadedGraphic();
    void testUnloadedGraphicLoading();
    void testUnloadedGraphicWmf();
    void testUnloadedGraphicAlpha();
    void testUnloadedGraphicSizeUnit();

    CPPUNIT_TEST_SUITE(GraphicTest);
    CPPUNIT_TEST(testUnloadedGraphic);
    CPPUNIT_TEST(testUnloadedGraphicLoading);
    CPPUNIT_TEST(testUnloadedGraphicWmf);
    CPPUNIT_TEST(testUnloadedGraphicAlpha);
    CPPUNIT_TEST(testUnloadedGraphicSizeUnit);
    CPPUNIT_TEST_SUITE_END();
};

BitmapEx createBitmap(bool alpha = false)
{
    Bitmap aBitmap(Size(120, 100), 24);
    aBitmap.Erase(COL_LIGHTRED);

    aBitmap.SetPrefSize(Size(6000, 5000));
    aBitmap.SetPrefMapMode(MapMode(MapUnit::Map100thMM));

    if (alpha)
    {
        sal_uInt8 uAlphaValue = 0x80;
        AlphaMask aAlphaMask(Size(120, 100), &uAlphaValue);

        return BitmapEx(aBitmap, aAlphaMask);
    }
    else
    {
        return BitmapEx(aBitmap);
    }
}

void createBitmapAndExportForType(SvStream& rStream, OUString const& sType, bool alpha)
{
    BitmapEx aBitmapEx = createBitmap(alpha);

    uno::Sequence<beans::PropertyValue> aFilterData;
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    sal_uInt16 nFilterFormat = rGraphicFilter.GetExportFormatNumberForShortName(sType);
    rGraphicFilter.ExportGraphic(aBitmapEx, "none", rStream, nFilterFormat, &aFilterData);

    rStream.Seek(STREAM_SEEK_TO_BEGIN);
}

Graphic makeUnloadedGraphic(OUString const& sType, bool alpha = false)
{
    SvMemoryStream aStream;
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    createBitmapAndExportForType(aStream, sType, alpha);
    return rGraphicFilter.ImportUnloadedGraphic(aStream);
}

char const DATA_DIRECTORY[] = "/vcl/qa/cppunit/data/";

void GraphicTest::testUnloadedGraphic()
{
    // make unloaded test graphic
    Graphic aGraphic = makeUnloadedGraphic("png");
    Graphic aGraphic2 = aGraphic;

    // check available
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic2.isAvailable());

    CPPUNIT_ASSERT_EQUAL(true, aGraphic2.makeAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic2.isAvailable());

    // check GetSizePixel doesn't load graphic
    aGraphic = makeUnloadedGraphic("png");
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(120L, aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(100L, aGraphic.GetSizePixel().Height());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    // check GetPrefSize doesn't load graphic
    CPPUNIT_ASSERT_EQUAL(6000L, aGraphic.GetPrefSize().Width());
    CPPUNIT_ASSERT_EQUAL(5000L, aGraphic.GetPrefSize().Height());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    // check GetSizeBytes loads graphic
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT(aGraphic.GetSizeBytes() > 0);
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
}

void GraphicTest::testUnloadedGraphicLoading()
{
    const OUString aFormats[] = { "png", "gif", "jpg" };

    for (OUString const& sFormat : aFormats)
    {
        Graphic aGraphic = makeUnloadedGraphic(sFormat);

        // check available
        CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
        CPPUNIT_ASSERT_EQUAL(120L, aGraphic.GetSizePixel().Width());
        CPPUNIT_ASSERT_EQUAL(100L, aGraphic.GetSizePixel().Height());
        CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
        CPPUNIT_ASSERT(aGraphic.GetSizeBytes() > 0);
        CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
        Bitmap aBitmap(aGraphic.GetBitmapEx().GetBitmap());

        {
            Bitmap::ScopedReadAccess pReadAccess(aBitmap);
            for (long y = 0; y < aGraphic.GetSizePixel().Height(); y++)
            {
                for (long x = 0; x < aGraphic.GetSizePixel().Width(); x++)
                {
                    if (pReadAccess->HasPalette())
                    {
                        Color aColor
                            = pReadAccess->GetPaletteColor(pReadAccess->GetPixelIndex(y, x));
                        CPPUNIT_ASSERT_EQUAL(OUString("ff0000"), aColor.AsRGBHexString());
                    }
                    else
                    {
                        Color aColor = pReadAccess->GetPixel(y, x);
                        if (sFormat != "jpg")
                            CPPUNIT_ASSERT_EQUAL(OUString("ff0000"), aColor.AsRGBHexString());
                    }
                }
            }
        }
    }
}

void GraphicTest::testUnloadedGraphicWmf()
{
    // Create some in-memory WMF data, set its own preferred size to 99x99.
    BitmapEx aBitmapEx = createBitmap();
    SvMemoryStream aStream;
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    sal_uInt16 nFilterFormat = rGraphicFilter.GetExportFormatNumberForShortName("wmf");
    Graphic aGraphic(aBitmapEx);
    aGraphic.SetPrefSize(Size(99, 99));
    aGraphic.SetPrefMapMode(MapMode(MapUnit::Map100thMM));
    rGraphicFilter.ExportGraphic(aGraphic, "none", aStream, nFilterFormat);
    aStream.Seek(STREAM_SEEK_TO_BEGIN);

    // Now lazy-load this WMF data, with a custom preferred size of 42x42.
    Size aMtfSize100(42, 42);
    aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream, 0, &aMtfSize100);
    aGraphic.makeAvailable();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 42x42
    // - Actual  : 99x99
    // i.e. the custom preferred size was lost after lazy-load.
    CPPUNIT_ASSERT_EQUAL(Size(42, 42), aGraphic.GetPrefSize());
}

void GraphicTest::testUnloadedGraphicAlpha()
{
    // make unloaded test graphic with alpha
    Graphic aGraphic = makeUnloadedGraphic("png", true);

    CPPUNIT_ASSERT_EQUAL(true, aGraphic.IsAlpha());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.IsTransparent());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    // make unloaded test graphic without alpha
    aGraphic = makeUnloadedGraphic("png", false);

    CPPUNIT_ASSERT_EQUAL(false, aGraphic.IsAlpha());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.IsTransparent());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
}

void GraphicTest::testUnloadedGraphicSizeUnit()
{
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    test::Directories aDirectories;
    OUString aURL = aDirectories.getURLFromSrc(DATA_DIRECTORY) + "inch-size.emf";
    Size aMtfSize100(42, 42);
    SvFileStream aStream(aURL, StreamMode::READ);
    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream, 0, &aMtfSize100);
    aGraphic.makeAvailable();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 400x363
    // - Actual  : 42x42
    // i.e. a mm100 size was used as a hint and the inch size was set for a non-matching unit.
    CPPUNIT_ASSERT_EQUAL(Size(400, 363), aGraphic.GetPrefSize());
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(GraphicTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
