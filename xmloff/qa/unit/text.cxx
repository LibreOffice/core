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
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/BibliographyDataType.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>

#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>

using namespace ::com::sun::star;

constexpr OUStringLiteral DATA_DIRECTORY = u"/xmloff/qa/unit/data/";

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
    // <text:database-display> in editeng text aborted the whole import process.
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testCommentResolved)
{
    getComponent() = loadFromDesktop("private:factory/swriter");
    uno::Sequence<beans::PropertyValue> aCommentProps = comphelper::InitPropertySequence({
        { "Text", uno::makeAny(OUString("comment")) },
    });
    dispatchCommand(getComponent(), ".uno:InsertAnnotation", aCommentProps);
    uno::Reference<text::XTextDocument> xTextDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPortionEnum = xPara->createEnumeration();
    uno::Reference<beans::XPropertySet> xPortion(xPortionEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xField(xPortion->getPropertyValue("TextField"),
                                               uno::UNO_QUERY);
    xField->setPropertyValue("Resolved", uno::makeAny(true));

    uno::Reference<frame::XStorable> xStorable(getComponent(), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProps = comphelper::InitPropertySequence({
        { "FilterName", uno::makeAny(OUString("writer8")) },
    });
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    xStorable->storeToURL(aTempFile.GetURL(), aStoreProps);
    getComponent()->dispose();

    getComponent() = loadFromDesktop(aTempFile.GetURL());
    xTextDocument.set(getComponent(), uno::UNO_QUERY);
    xParaEnumAccess.set(xTextDocument->getText(), uno::UNO_QUERY);
    xParaEnum = xParaEnumAccess->createEnumeration();
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPortionEnum = xPara->createEnumeration();
    xPortion.set(xPortionEnum->nextElement(), uno::UNO_QUERY);
    xField.set(xPortion->getPropertyValue("TextField"), uno::UNO_QUERY);
    bool bResolved = false;
    xField->getPropertyValue("Resolved") >>= bResolved;
    // Without the accompanying fix in place, this test would have failed, as the resolved state was
    // not saved for non-range comments.
    CPPUNIT_ASSERT(bResolved);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testBibliographyLocalUrl)
{
    // Given a document with a biblio field, with non-empty LocalURL:
    getComponent() = loadFromDesktop("private:factory/swriter");
    uno::Reference<lang::XMultiServiceFactory> xFactory(getComponent(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xField(
        xFactory->createInstance("com.sun.star.text.TextField.Bibliography"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aFields = {
        comphelper::makePropertyValue("BibiliographicType", text::BibliographyDataType::WWW),
        comphelper::makePropertyValue("Identifier", OUString("AT")),
        comphelper::makePropertyValue("Author", OUString("Author")),
        comphelper::makePropertyValue("Title", OUString("Title")),
        comphelper::makePropertyValue("URL", OUString("http://www.example.com/test.pdf#page=1")),
        comphelper::makePropertyValue("LocalURL", OUString("file:///home/me/test.pdf")),
    };
    xField->setPropertyValue("Fields", uno::makeAny(aFields));
    uno::Reference<text::XTextDocument> xTextDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextContent> xContent(xField, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContent, /*bAbsorb=*/false);

    // When invoking ODT export + import on it:
    uno::Reference<frame::XStorable> xStorable(getComponent(), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProps = {
        comphelper::makePropertyValue("FilterName", OUString("writer8")),
    };
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    // Without the accompanying fix in place, this test would have resulted in an assertion failure,
    // as LocalURL was mapped to XML_TOKEN_INVALID.
    xStorable->storeToURL(aTempFile.GetURL(), aStoreProps);
    getComponent()->dispose();
    validate(aTempFile.GetFileName(), test::ODF);
    getComponent() = loadFromDesktop(aTempFile.GetURL());

    // Then make sure that LocalURL is preserved:
    xTextDocument.set(getComponent(), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPortionEnum = xPara->createEnumeration();
    uno::Reference<beans::XPropertySet> xPortion(xPortionEnum->nextElement(), uno::UNO_QUERY);
    xField.set(xPortion->getPropertyValue("TextField"), uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aMap(xField->getPropertyValue("Fields"));
    CPPUNIT_ASSERT(aMap.find("LocalURL") != aMap.end());
    auto aActual = aMap["LocalURL"].get<OUString>();
    CPPUNIT_ASSERT_EQUAL(OUString("file:///home/me/test.pdf"), aActual);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testCommentTableBorder)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "comment-table-border.fodt";
    // Without the accompanying fix in place, this failed to load, as a comment that started in a
    // table and ended outside a table aborted the whole importer.
    getComponent() = loadFromDesktop(aURL);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testParaStyleListLevel)
{
    // Given a document with style:list-level="...":
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "para-style-list-level.fodt";

    // When loading that document:
    getComponent() = loadFromDesktop(aURL);

    // Then make sure we map that to the paragraph style's numbering level:
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(getComponent(),
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xStyleFamily(
        xStyleFamilies->getByName("ParagraphStyles"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStyle(xStyleFamily->getByName("mystyle"), uno::UNO_QUERY);
    sal_Int16 nNumberingLevel{};
    CPPUNIT_ASSERT(xStyle->getPropertyValue("NumberingLevel") >>= nNumberingLevel);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1), nNumberingLevel);

    // Test the export as well:

    // Given a doc model that has a para style with NumberingLevel=2:
    uno::Reference<frame::XStorable> xStorable(getComponent(), uno::UNO_QUERY);

    // When exporting that to ODT:
    uno::Sequence<beans::PropertyValue> aStoreProps = comphelper::InitPropertySequence({
        { "FilterName", uno::makeAny(OUString("writer8")) },
    });
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    xStorable->storeToURL(aTempFile.GetURL(), aStoreProps);

    // Then make sure we save the style's numbering level:
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(mxComponentContext, aTempFile.GetURL());
    uno::Reference<io::XInputStream> xInputStream(xNameAccess->getByName("styles.xml"),
                                                  uno::UNO_QUERY);
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    // Without the accompanying fix in place, this failed with:
    // - XPath '/office:document-styles/office:styles/style:style[@style:name='mystyle']' no attribute 'list-level' exist
    // i.e. a custom NumberingLevel was lost on save.
    assertXPath(pXmlDoc, "/office:document-styles/office:styles/style:style[@style:name='mystyle']",
                "list-level", "2");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
