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

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextTable.hpp>

#include <unotools/mediadescriptor.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>

using namespace ::com::sun::star;

constexpr OUStringLiteral DATA_DIRECTORY = u"/xmloff/qa/unit/data/";

/// Covers xmloff/source/draw/ fixes.
class XmloffDrawTest : public test::BootstrapFixture,
                       public unotest::MacrosTest,
                       public XmlTestTools
{
private:
    uno::Reference<lang::XComponent> mxComponent;

public:
    void setUp() override;
    void tearDown() override;
    void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx) override;
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
};

void XmloffDrawTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void XmloffDrawTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

void XmloffDrawTest::registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx)
{
    XmlTestTools::registerODFNamespaces(pXmlXpathCtx);
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testTextBoxLoss)
{
    // Load a document that has a shape with a textbox in it. Save it to ODF and reload.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "textbox-loss.docx";
    getComponent() = loadFromDesktop(aURL);
    uno::Reference<frame::XStorable> xStorable(getComponent(), uno::UNO_QUERY);
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer8");
    xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    getComponent()->dispose();
    getComponent() = loadFromDesktop(aTempFile.GetURL());

    // Make sure that the shape is still a textbox.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    bool bTextBox = false;
    xShape->getPropertyValue("TextBox") >>= bTextBox;

    // Without the accompanying fix in place, this test would have failed, as the shape only had
    // editeng text, losing the image part of the shape text.
    CPPUNIT_ASSERT(bTextBox);
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testTdf141301_Extrusion_Angle)
{
    // Load a document that has a custom shape with extrusion direction as set by LO as its default.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf141301_Extrusion_Skew.odg";
    getComponent() = loadFromDesktop(aURL, "com.sun.star.comp.drawing.DrawingDocument");

    // Prepare use of XPath
    uno::Reference<frame::XStorable> xStorable(getComponent(), uno::UNO_QUERY);
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("draw8");
    xStorable->storeAsURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(mxComponentContext, aTempFile.GetURL());
    uno::Reference<io::XInputStream> xInputStream(xNameAccess->getByName("content.xml"),
                                                  uno::UNO_QUERY);
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());

    // Without fix draw:extrusion-skew="50 -135" was not written to file although "50 -135" is not
    // default in ODF, but only default inside LO.
    assertXPath(pXmlDoc, "//draw:enhanced-geometry", "extrusion-skew", "50 -135");
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testTableInShape)
{
    // Given a document with a shape with a "FrameX" parent style (starts with Frame, but is not
    // Frame):
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "table-in-shape.fodt";

    // When loading that document:
    getComponent() = loadFromDesktop(aURL);

    // Then make sure the table inside the shape is not lost:
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<text::XTextRange> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xText(xShape->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xEnum = xText->createEnumeration();
    uno::Reference<text::XTextTable> xTable(xEnum->nextElement(), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have crashed, as xTable was an empty
    // reference, i.e. the table inside the shape was lost.
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("A1"), xCell->getString());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
