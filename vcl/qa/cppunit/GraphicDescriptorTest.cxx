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
#include <test/cppunitasserthelper.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <vcl/graphicfilter.hxx>
#include <tools/stream.hxx>

using namespace css;

namespace
{
class GraphicDescriptorTest : public CppUnit::TestFixture
{
    void testDetectPNG();
    void testDetectJPG();
    void testDetectGIF();

    CPPUNIT_TEST_SUITE(GraphicDescriptorTest);
    CPPUNIT_TEST(testDetectPNG);
    CPPUNIT_TEST(testDetectJPG);
    CPPUNIT_TEST(testDetectGIF);
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

void GraphicDescriptorTest::testDetectPNG()
{
    SvMemoryStream aStream;
    createBitmapAndExportForType(aStream, "png");

    GraphicDescriptor aDescriptor(aStream, nullptr);
    aDescriptor.Detect(true);

    CPPUNIT_ASSERT_EQUAL(GraphicFileFormat::PNG, aDescriptor.GetFileFormat());

    CPPUNIT_ASSERT_EQUAL(100L, aDescriptor.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(100L, aDescriptor.GetSizePixel().Height());
}

void GraphicDescriptorTest::testDetectJPG()
{
    SvMemoryStream aStream;
    createBitmapAndExportForType(aStream, "jpg");

    GraphicDescriptor aDescriptor(aStream, nullptr);
    aDescriptor.Detect(true);

    CPPUNIT_ASSERT_EQUAL(GraphicFileFormat::JPG, aDescriptor.GetFileFormat());

    CPPUNIT_ASSERT_EQUAL(100L, aDescriptor.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(100L, aDescriptor.GetSizePixel().Height());
}

void GraphicDescriptorTest::testDetectGIF()
{
    SvMemoryStream aStream;
    createBitmapAndExportForType(aStream, "gif");

    GraphicDescriptor aDescriptor(aStream, nullptr);
    aDescriptor.Detect(true);

    CPPUNIT_ASSERT_EQUAL(GraphicFileFormat::GIF, aDescriptor.GetFileFormat());

    CPPUNIT_ASSERT_EQUAL(100L, aDescriptor.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(100L, aDescriptor.GetSizePixel().Height());
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(GraphicDescriptorTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
