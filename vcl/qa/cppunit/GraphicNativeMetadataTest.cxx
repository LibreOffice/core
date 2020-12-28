/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <unotest/bootstrapfixturebase.hxx>

#include <vcl/GraphicNativeMetadata.hxx>
#include <vcl/graphicfilter.hxx>
#include <tools/stream.hxx>

using namespace css;

namespace
{
class GraphicNativeMetadataTest : public test::BootstrapFixtureBase
{
    OUString getFullUrl(std::u16string_view sFileName)
    {
        return m_directories.getURLFromSrc(u"/vcl/qa/cppunit/data/") + sFileName;
    }

    void testReadFromGraphic();
    void testExifRotationJpeg();

    CPPUNIT_TEST_SUITE(GraphicNativeMetadataTest);
    CPPUNIT_TEST(testReadFromGraphic);
    CPPUNIT_TEST(testExifRotationJpeg);
    CPPUNIT_TEST_SUITE_END();
};

void GraphicNativeMetadataTest::testReadFromGraphic()
{
    SvFileStream aFileStream(getFullUrl(u"Exif1_180.jpg"), StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();

    // don't load the graphic, but try to get the metadata
    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aFileStream);

    {
        GraphicNativeMetadata aMetadata;
        aMetadata.read(aFileStream);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(1800), aMetadata.getRotation().get());
        // just the metadata shouldn't make the graphic available
        CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    }

    // now load, and it should still work the same
    {
        aGraphic.makeAvailable();
        CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());

        GraphicNativeMetadata aMetadata;
        aMetadata.read(aFileStream);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(1800), aMetadata.getRotation().get());
    }
}

void GraphicNativeMetadataTest::testExifRotationJpeg()
{
    {
        // No rotation in metadata
        SvFileStream aFileStream(getFullUrl(u"Exif1.jpg"), StreamMode::READ);
        GraphicNativeMetadata aMetadata;
        aMetadata.read(aFileStream);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(0), aMetadata.getRotation().get());
    }
    {
        // Rotation 90 degree clock-wise = 270 degree counter-clock-wise
        SvFileStream aFileStream(getFullUrl(u"Exif1_090CW.jpg"), StreamMode::READ);
        GraphicNativeMetadata aMetadata;
        aMetadata.read(aFileStream);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(2700), aMetadata.getRotation().get());
    }
    {
        // Rotation 180 degree
        SvFileStream aFileStream(getFullUrl(u"Exif1_180.jpg"), StreamMode::READ);
        GraphicNativeMetadata aMetadata;
        aMetadata.read(aFileStream);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(1800), aMetadata.getRotation().get());
    }
    {
        // Rotation 270 degree clock-wise = 90 degree counter-clock-wise
        SvFileStream aFileStream(getFullUrl(u"Exif1_270CW.jpg"), StreamMode::READ);
        GraphicNativeMetadata aMetadata;
        aMetadata.read(aFileStream);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(900), aMetadata.getRotation().get());
    }
}

} // anonymous namespace

CPPUNIT_TEST_SUITE_REGISTRATION(GraphicNativeMetadataTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
