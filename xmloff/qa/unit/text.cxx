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
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

#include <comphelper/propertysequence.hxx>
#include <unotools/tempfile.hxx>

using namespace ::com::sun::star;

char const DATA_DIRECTORY[] = "/xmloff/qa/unit/data/";

/// Covers xmloff/source/text/ fixes.
class XmloffStyleTest : public test::BootstrapFixture,
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

void XmloffStyleTest::registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx)
{
    XmlTestTools::registerODFNamespaces(pXmlXpathCtx);
}

void XmloffStyleTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void XmloffStyleTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testMailMergeInEditeng)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "mail-merge-editeng.odt";
    getComponent() = loadFromDesktop(aURL);
    // Without the accompanying fix in place, this test would have failed, as unexpected
    // <text:database-display> in editeng text abored the whole import process.
    CPPUNIT_ASSERT(getComponent().is());
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testListId)
{
    // Given a document with a simple list (no continue-list="..." attribute):
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "list-id.fodt";
    getComponent() = loadFromDesktop(aURL);

    // When storing that document as ODF:
    uno::Reference<frame::XStorable> xStorable(getComponent(), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProps = comphelper::InitPropertySequence({
        { "FilterName", uno::makeAny(OUString("writer8")) },
    });
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    xStorable->storeToURL(aTempFile.GetURL(), aStoreProps);

    // Then make sure that unreferenced xml:id="..." attributes are not written:
    std::unique_ptr<SvStream> pStream = parseExportStream(aTempFile, "content.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    // Without the accompanying fix in place, this failed with:
    // - XPath '//text:list' unexpected 'id' attribute
    // i.e. xml:id="..." was written unconditionally, even when no other list needed it.
    assertXPathNoAttribute(pXmlDoc, "//text:list", "id");
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testRelativeWidth)
{
    // Given a document with an 50% wide text frame:
    getComponent() = loadFromDesktop("private:factory/swriter");
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(getComponent(),
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName("PageStyles"),
                                                        uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStyle(xStyleFamily->getByName("Standard"), uno::UNO_QUERY);
    // Body frame width is 6cm (2+2cm margin).
    xStyle->setPropertyValue("Width", uno::makeAny(static_cast<sal_Int32>(10000)));
    uno::Reference<lang::XMultiServiceFactory> xMSF(getComponent(), uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xTextFrame(
        xMSF->createInstance("com.sun.star.text.TextFrame"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextFrameProps(xTextFrame, uno::UNO_QUERY);
    xTextFrameProps->setPropertyValue("RelativeWidth", uno::makeAny(static_cast<sal_Int16>(50)));
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertTextContent(xCursor, xTextFrame, /*bAbsorb=*/false);
    // Body frame width is 16cm.
    xStyle->setPropertyValue("Width", uno::makeAny(static_cast<sal_Int32>(20000)));

    uno::Reference<frame::XStorable> xStorable(getComponent(), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProps = comphelper::InitPropertySequence({
        { "FilterName", uno::makeAny(OUString("writer8")) },
    });
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    xStorable->storeToURL(aTempFile.GetURL(), aStoreProps);

    std::unique_ptr<SvStream> pStream = parseExportStream(aTempFile, "content.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    // Without the accompanying fix in place, this failed with:
    // - Expected: 3.1492in (8cm)
    // - Actual  : 0.0161in (0.04 cm)
    // i.e. the fallback width value wasn't the expected half of the body frame width, but a smaller
    // value.
    assertXPath(pXmlDoc, "//draw:frame", "width", "3.1492in");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
