/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <unotest/bootstrapfixturebase.hxx>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <vcl/graphicfilter.hxx>
#include <tools/stream.hxx>

using namespace css;

namespace
{
class GraphicDescriptorTest : public test::BootstrapFixtureBase
{
    OUString getFullUrl(std::u16string_view sFileName)
    {
        return m_directories.getURLFromSrc(u"/vcl/qa/cppunit/data/") + sFileName;
    }
    void testDetectPNG();
    void testDetectJPG();
    void testDetectGIF();
    void testDetectTIF();
    void testDetectBMP();

    CPPUNIT_TEST_SUITE(GraphicDescriptorTest);
    CPPUNIT_TEST(testDetectPNG);
    CPPUNIT_TEST(testDetectJPG);
    CPPUNIT_TEST(testDetectGIF);
    CPPUNIT_TEST(testDetectTIF);
    CPPUNIT_TEST(testDetectBMP);
    CPPUNIT_TEST_SUITE_END();
};

BitmapEx createBitmap()
{
    Bitmap aBitmap(Size(100, 100), vcl::PixelFormat::N24_BPP);
    aBitmap.Erase(COL_LIGHTRED);

    return BitmapEx(aBitmap);
}

void createBitmapAndExportForType(SvStream& rStream, std::u16string_view sType)
{
    BitmapEx aBitmapEx = createBitmap();

    uno::Sequence<beans::PropertyValue> aFilterData;
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    sal_uInt16 nFilterFormat = rGraphicFilter.GetExportFormatNumberForShortName(sType);
    rGraphicFilter.ExportGraphic(aBitmapEx, "none", rStream, nFilterFormat, &aFilterData);

    rStream.Seek(STREAM_SEEK_TO_BEGIN);
}

void GraphicDescriptorTest::testDetectPNG()
{
    SvMemoryStream aStream;
    createBitmapAndExportForType(aStream, u"png");

    GraphicDescriptor aDescriptor(aStream, nullptr);
    aDescriptor.Detect(true);

    CPPUNIT_ASSERT_EQUAL(GraphicFileFormat::PNG, aDescriptor.GetFileFormat());

    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aDescriptor.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aDescriptor.GetSizePixel().Height());
}

void GraphicDescriptorTest::testDetectJPG()
{
    SvMemoryStream aStream;
    createBitmapAndExportForType(aStream, u"jpg");

    GraphicDescriptor aDescriptor(aStream, nullptr);
    aDescriptor.Detect(true);

    CPPUNIT_ASSERT_EQUAL(GraphicFileFormat::JPG, aDescriptor.GetFileFormat());

    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aDescriptor.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aDescriptor.GetSizePixel().Height());
}

void GraphicDescriptorTest::testDetectGIF()
{
    SvMemoryStream aStream;
    createBitmapAndExportForType(aStream, u"gif");

    GraphicDescriptor aDescriptor(aStream, nullptr);
    aDescriptor.Detect(true);

    CPPUNIT_ASSERT_EQUAL(GraphicFileFormat::GIF, aDescriptor.GetFileFormat());

    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aDescriptor.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aDescriptor.GetSizePixel().Height());
}

void GraphicDescriptorTest::testDetectTIF()
{
    SvMemoryStream aStream;
    createBitmapAndExportForType(aStream, u"tif");

    GraphicDescriptor aDescriptor(aStream, nullptr);
    aDescriptor.Detect(true);

    CPPUNIT_ASSERT_EQUAL(GraphicFileFormat::TIF, aDescriptor.GetFileFormat());

    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aDescriptor.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aDescriptor.GetSizePixel().Height());
}

void GraphicDescriptorTest::testDetectBMP()
{
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    SvFileStream aFileStream(getFullUrl(u"graphic-descriptor-mapmode.bmp"), StreamMode::READ);

    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aFileStream);

    CPPUNIT_ASSERT(!aGraphic.isAvailable());
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2 (MapUnit::MapMM)
    // - Actual  : 0 (MapUnit::Map100thMM)
    // i.e. lazy load and load created different map modes, breaking the contour polygon code in
    // Writer.
    CPPUNIT_ASSERT_EQUAL(MapUnit::MapMM, aGraphic.GetPrefMapMode().GetMapUnit());
    aGraphic.makeAvailable();
    CPPUNIT_ASSERT_EQUAL(MapUnit::MapMM, aGraphic.GetPrefMapMode().GetMapUnit());
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(GraphicDescriptorTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
