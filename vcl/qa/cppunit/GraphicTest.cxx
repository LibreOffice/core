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

using namespace css;

namespace
{
class GraphicTest : public CppUnit::TestFixture
{
    void testUnloadedGraphic();
    void testUnloadedGraphicLoading();

    CPPUNIT_TEST_SUITE(GraphicTest);
    CPPUNIT_TEST(testUnloadedGraphic);
    CPPUNIT_TEST(testUnloadedGraphicLoading);
    CPPUNIT_TEST_SUITE_END();
};

BitmapEx createBitmap()
{
    Bitmap aBitmap(Size(100, 100), 24);
    aBitmap.Erase(COL_LIGHTRED);

    return BitmapEx(aBitmap);
}

void createBitmapAndExportForType(SvStream& rStream, OUString const& sType)
{
    BitmapEx aBitmapEx = createBitmap();

    uno::Sequence<beans::PropertyValue> aFilterData;
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    sal_uInt16 nFilterFormat = rGraphicFilter.GetExportFormatNumberForShortName(sType);
    rGraphicFilter.ExportGraphic(aBitmapEx, "none", rStream, nFilterFormat, &aFilterData);

    rStream.Seek(STREAM_SEEK_TO_BEGIN);
}

Graphic makeUnloadedGraphic(OUString const& sType)
{
    SvMemoryStream aStream;
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    createBitmapAndExportForType(aStream, sType);
    return rGraphicFilter.ImportUnloadedGraphic(aStream);
}

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
    CPPUNIT_ASSERT_EQUAL(100L, aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(100L, aGraphic.GetSizePixel().Height());
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
        CPPUNIT_ASSERT_EQUAL(100L, aGraphic.GetSizePixel().Width());
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

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(GraphicTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
