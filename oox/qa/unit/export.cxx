/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <test/xmltesttools.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>

#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>

#include <unotools/mediadescriptor.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>

using namespace ::com::sun::star;

namespace
{
/// Covers ooox/source/export/ fixes.
class Test : public test::BootstrapFixture, public unotest::MacrosTest, public XmlTestTools
{
private:
    uno::Reference<lang::XComponent> mxComponent;
    utl::TempFile maTempFile;

public:
    void setUp() override;
    void tearDown() override;
    void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx) override;
    utl::TempFile& getTempFile() { return maTempFile; }
    void loadAndSave(const OUString& rURL, const OUString& rFilterName);
    void testPolylineConnectorShapes(const OUString& rURL);
};

void Test::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void Test::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

void Test::registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx)
{
    XmlTestTools::registerOOXMLNamespaces(pXmlXpathCtx);
}

void Test::testPolylineConnectorShapes(const OUString& rURL)
{
    mxComponent = loadFromDesktop(rURL);

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xGroup(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xGroupShape(xGroup, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3537), xGroupShape->getSize().Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5659), xGroupShape->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5001), xGroupShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), xGroupShape->getPosition().Y);

    // Height and width sometimes differ 1 or 2 pixel, possibly because of
    // integer calculations and rounding. In such cases CPPUNIT_ASSERT_LESSEQUAL
    // macro is used

    // Shape 0
    uno::Reference<drawing::XShape> xShape0(xGroup->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_LESSEQUAL(sal_Int32(2), abs(sal_Int32(2122) - xShape0->getSize().Height));
    CPPUNIT_ASSERT_LESSEQUAL(sal_Int32(2), abs(sal_Int32(4242) - xShape0->getSize().Width));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5001), xShape0->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3415), xShape0->getPosition().Y);

    // Shape 1
    uno::Reference<drawing::XShape> xShape1(xGroup->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xShape1->getSize().Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4244), xShape1->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6416), xShape1->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), xShape1->getPosition().Y);

    // Shape 2
    uno::Reference<drawing::XShape> xShape2(xGroup->getByIndex(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_LESSEQUAL(sal_Int32(2), abs(sal_Int32(3537) - xShape2->getSize().Height));
    CPPUNIT_ASSERT_LESSEQUAL(sal_Int32(2), abs(sal_Int32(1416) - xShape2->getSize().Width));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7121), xShape2->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), xShape2->getPosition().Y);

    mxComponent->dispose();
    mxComponent.clear();
}

void Test::loadAndSave(const OUString& rURL, const OUString& rFilterName)
{
    mxComponent = loadFromDesktop(rURL);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= rFilterName;
    maTempFile.EnableKillingFile();
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    mxComponent->dispose();
    mxComponent.clear();
    // too many DOCX validation errors right now
    if (rFilterName != "Office Open XML Text")
    {
        validate(maTempFile.GetFileName(), test::OOXML);
    }
}

constexpr OUStringLiteral DATA_DIRECTORY = u"/oox/qa/unit/data/";

CPPUNIT_TEST_FIXTURE(Test, testPolylineConnectorPosition)
{
    // Given a document with a group shape and therein a polyline and a connector.
    OUString aURL
        = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf141786_PolylineConnectorInGroup.odt";
    // When saving that to DOCX:
    loadAndSave(aURL, "Office Open XML Text");

    // Then make sure polyline and connector have the correct position.
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(mxComponentContext, getTempFile().GetURL());
    uno::Reference<io::XInputStream> xInputStream(xNameAccess->getByName("word/document.xml"),
                                                  uno::UNO_QUERY);
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());

    // For child elements of groups in Writer the position has to be adapted to be relative
    // to group instead of being relative to anchor. That was missing for polyline and
    // connector.
    // Polyline: Without fix it would have failed with expected: 0, actual: 1800360
    assertXPath(pXmlDoc, "//wpg:wgp/wps:wsp[1]/wps:spPr/a:xfrm/a:off", "x", "0");
    // ... failed with expected: 509400, actual: 1229400
    assertXPath(pXmlDoc, "//wpg:wgp/wps:wsp[1]/wps:spPr/a:xfrm/a:off", "y", "509400");

    // Connector: Without fix it would have failed with expected: 763200, actual: 2563560
    assertXPath(pXmlDoc, "//wpg:wgp/wps:wsp[3]/wps:spPr/a:xfrm/a:off", "x", "763200");
    // ... failed with expected: 0, actual: 720000
    assertXPath(pXmlDoc, "//wpg:wgp/wps:wsp[3]/wps:spPr/a:xfrm/a:off", "y", "0");
    // Polyline and connector were shifted 1800360EMU right, 720000EMU down.

    // Testing to make sure the actual position and size
    // of the group and shapes inside it are unchanged
    // comparing initial load and after save/reload

    // initial load
    testPolylineConnectorShapes(aURL);

    // after save and reload
    testPolylineConnectorShapes(getTempFile().GetURL());
}

CPPUNIT_TEST_FIXTURE(Test, testRotatedShapePosition)
{
    // Given a document with a group shape and therein a rotated custom shape.
    OUString aURL
        = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf141786_RotatedShapeInGroup.odt";
    // When saving that to DOCX:
    loadAndSave(aURL, "Office Open XML Text");

    // Then make sure the rotated child shape has the correct position.
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(mxComponentContext, getTempFile().GetURL());
    uno::Reference<io::XInputStream> xInputStream(xNameAccess->getByName("word/document.xml"),
                                                  uno::UNO_QUERY);
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());

    // For a group itself and for shapes outside of groups, the position calculation is done in
    // DocxSdrExport. For child elements of groups it has to be done in
    // DrawingML::WriteShapeTransformation(), but was missing.
    // Without fix it would have failed with expected: 469440, actual: 92160
    // The shape was about 1cm shifted up and partly outside its group.
    assertXPath(pXmlDoc, "//wpg:wgp/wps:wsp[1]/wps:spPr/a:xfrm/a:off", "y", "469440");
}

CPPUNIT_TEST_FIXTURE(Test, testDmlGroupshapePolygon)
{
    // Given a document with a group shape, containing a single polygon child shape:
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "dml-groupshape-polygon.docx";

    // When saving that to DOCX:
    loadAndSave(aURL, "Office Open XML Text");

    // Then make sure that the group shape, the group shape's child size and the child shape's size
    // match:
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(mxComponentContext, getTempFile().GetURL());
    uno::Reference<io::XInputStream> xInputStream(xNameAccess->getByName("word/document.xml"),
                                                  uno::UNO_QUERY);
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    assertXPath(pXmlDoc, "//wpg:grpSpPr/a:xfrm/a:ext", "cx", "5328360");
    // Without the accompanying fix in place, this test would have failed, the <a:chExt> element was
    // not written.
    assertXPath(pXmlDoc, "//wpg:grpSpPr/a:xfrm/a:chExt", "cx", "5328360");
    assertXPath(pXmlDoc, "//wps:spPr/a:xfrm/a:ext", "cx", "5328360");
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
