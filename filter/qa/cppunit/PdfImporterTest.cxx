/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <sal/types.h>
#include <test/bootstrapfixture.hxx>
#include <test/xmltesttools.hxx>

#include <filter/pdfimporter/PDFImporter.hxx>
#include <vcl/BinaryDataContainer.hxx>
#include <tools/stream.hxx>
#include <drawinglayer/converters.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/tools/primitive2dxmldump.hxx>

#include <tools/stream.hxx>

class PDFImporterTest : public test::BootstrapFixture, public XmlTestTools
{
    OUString getFullUrl(std::u16string_view sFileName)
    {
        return m_directories.getURLFromSrc(u"/filter/qa/cppunit/data/") + sFileName;
    }

    void testPath();
    void testImage();
    void testText();

    CPPUNIT_TEST_SUITE(PDFImporterTest);
    //CPPUNIT_TEST(testPath);
    //CPPUNIT_TEST(testImage);
    //CPPUNIT_TEST(testText);
    CPPUNIT_TEST_SUITE_END();
};

void PDFImporterTest::testPath()
{
    SvFileStream aFileStream(getFullUrl(u"/PdfTest-Rect.pdf"), StreamMode::READ);
    const sal_uInt64 nStreamLength = aFileStream.TellEnd();
    auto rData = std::make_unique<std::vector<sal_uInt8>>(nStreamLength);
    aFileStream.ReadBytes(rData->data(), rData->size());
    BinaryDataContainer aDataContainer(std::move(rData));
    PDFImporter aImporter(aDataContainer);

    drawinglayer::primitive2d::Primitive2DContainer aContainer;
    aImporter.importPage(0, aContainer);

    drawinglayer::geometry::ViewInformation2D rViewInformation2D;

    auto aRange = aContainer.getB2DRange(rViewInformation2D);

    drawinglayer::primitive2d::Primitive2DContainer aCopy(aContainer);

    BitmapEx aBitmapEx = drawinglayer::convertToBitmapEx(std::move(aCopy), rViewInformation2D, 2000,
                                                         2000, 10000 * 1000);

    //SvFileStream aNew("~/xxxxxxx.png", StreamMode::WRITE | StreamMode::TRUNC);
    //vcl::PNGWriter aPNGWriter(aBitmapEx);
    //aPNGWriter.Write(aNew);

    drawinglayer::Primitive2dXmlDump aDumper;
    aDumper.dump(aContainer, OUString());
    xmlDocUniquePtr pDocument = aDumper.dumpAndParse(aContainer);
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/transform/unifiedtransparence",
                "transparence", "0.498039215686275");

    CPPUNIT_ASSERT_EQUAL(-0.5, aRange.getMinX());
    CPPUNIT_ASSERT_EQUAL(-0.5, aRange.getMinY());
    CPPUNIT_ASSERT_EQUAL(612.5, aRange.getMaxX());
    CPPUNIT_ASSERT_EQUAL(792.5, aRange.getMaxY());

    CPPUNIT_ASSERT_EQUAL(tools::Long(1000), aBitmapEx.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(1000), aBitmapEx.GetSizePixel().Height());

    CPPUNIT_ASSERT(false);
}

void PDFImporterTest::testImage()
{
    SvFileStream aFileStream(getFullUrl(u"/PdfTest-Image.pdf"), StreamMode::READ);
    const sal_uInt64 nStreamLength = aFileStream.TellEnd();
    auto rData = std::make_unique<std::vector<sal_uInt8>>(nStreamLength);
    aFileStream.ReadBytes(rData->data(), rData->size());
    BinaryDataContainer aDataContainer(std::move(rData));
    PDFImporter aImporter(aDataContainer);

    drawinglayer::primitive2d::Primitive2DContainer aContainer;
    aImporter.importPage(0, aContainer);

    drawinglayer::geometry::ViewInformation2D rViewInformation2D;

    auto aRange = aContainer.getB2DRange(rViewInformation2D);

    drawinglayer::primitive2d::Primitive2DContainer aCopy(aContainer);

    BitmapEx aBitmapEx = drawinglayer::convertToBitmapEx(std::move(aCopy), rViewInformation2D, 2000,
                                                         2000, 10000 * 1000);

    //SvFileStream aNew("~/xxxxxxx.png", StreamMode::WRITE | StreamMode::TRUNC);
    //vcl::PNGWriter aPNGWriter(aBitmapEx);
    //aPNGWriter.Write(aNew);

    drawinglayer::Primitive2dXmlDump aDumper;
    aDumper.dump(aContainer, OUString());
    xmlDocUniquePtr pDocument = aDumper.dumpAndParse(aContainer);
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/transform/unifiedtransparence",
                "transparence", "0.498039215686275");

    CPPUNIT_ASSERT_EQUAL(-0.5, aRange.getMinX());
    CPPUNIT_ASSERT_EQUAL(-0.5, aRange.getMinY());
    CPPUNIT_ASSERT_EQUAL(612.5, aRange.getMaxX());
    CPPUNIT_ASSERT_EQUAL(792.5, aRange.getMaxY());

    CPPUNIT_ASSERT_EQUAL(tools::Long(1000), aBitmapEx.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(1000), aBitmapEx.GetSizePixel().Height());

    CPPUNIT_ASSERT(false);
}

void PDFImporterTest::testText()
{
    SvFileStream aFileStream(getFullUrl(u"/PdfTest-Text.pdf"), StreamMode::READ);
    const sal_uInt64 nStreamLength = aFileStream.TellEnd();
    auto rData = std::make_unique<std::vector<sal_uInt8>>(nStreamLength);
    aFileStream.ReadBytes(rData->data(), rData->size());
    BinaryDataContainer aDataContainer(std::move(rData));
    PDFImporter aImporter(aDataContainer);

    drawinglayer::primitive2d::Primitive2DContainer aContainer;
    aImporter.importPage(0, aContainer);

    drawinglayer::geometry::ViewInformation2D rViewInformation2D;

    auto aRange = aContainer.getB2DRange(rViewInformation2D);

    drawinglayer::primitive2d::Primitive2DContainer aCopy(aContainer);

    BitmapEx aBitmapEx = drawinglayer::convertToBitmapEx(std::move(aCopy), rViewInformation2D, 2000,
                                                         2000, 10000 * 1000);

    //SvFileStream aNew("~/xxxxxxx.png", StreamMode::WRITE | StreamMode::TRUNC);
    //vcl::PNGWriter aPNGWriter(aBitmapEx);
    //aPNGWriter.Write(aNew);

    drawinglayer::Primitive2dXmlDump aDumper;
    aDumper.dump(aContainer, OUString());
    xmlDocUniquePtr pDocument = aDumper.dumpAndParse(aContainer);
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, "/primitive2D/metafile/transform/mask/transform/unifiedtransparence",
                "transparence", "0.498039215686275");

    CPPUNIT_ASSERT_EQUAL(-0.5, aRange.getMinX());
    CPPUNIT_ASSERT_EQUAL(-0.5, aRange.getMinY());
    CPPUNIT_ASSERT_EQUAL(612.5, aRange.getMaxX());
    CPPUNIT_ASSERT_EQUAL(792.5, aRange.getMaxY());

    CPPUNIT_ASSERT_EQUAL(tools::Long(1000), aBitmapEx.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(1000), aBitmapEx.GetSizePixel().Height());

    CPPUNIT_ASSERT(false);
}

CPPUNIT_TEST_SUITE_REGISTRATION(PDFImporterTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
