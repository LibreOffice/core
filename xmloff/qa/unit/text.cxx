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
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/BibliographyDataType.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
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
        { "Text", uno::Any(OUString("comment")) },
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
    xField->setPropertyValue("Resolved", uno::Any(true));

    uno::Reference<frame::XStorable> xStorable(getComponent(), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProps = comphelper::InitPropertySequence({
        { "FilterName", uno::Any(OUString("writer8")) },
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
    xField->setPropertyValue("Fields", uno::Any(aFields));
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
        { "FilterName", uno::Any(OUString("writer8")) },
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

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testContinueNumberingWord)
{
    // Given a document, which is produced by Word and contains text:continue-numbering="true":
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "continue-numbering-word.odt";

    // When loading that document:
    getComponent() = loadFromDesktop(aURL);

    // Then make sure that the numbering from the 1st para is continued on the 3rd para:
    uno::Reference<text::XTextDocument> xTextDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    xParaEnum->nextElement();
    xParaEnum->nextElement();
    uno::Reference<beans::XPropertySet> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    auto aActual = xPara->getPropertyValue("ListLabelString").get<OUString>();
    // Without the accompanying fix in place, this failed with:
    // - Expected: 2.
    // - Actual  : 1.
    // i.e. the numbering was not continued, like in Word.
    CPPUNIT_ASSERT_EQUAL(OUString("2."), aActual);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testListId)
{
    // Given a document with a simple list (no continue-list="..." attribute):
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "list-id.fodt";
    getComponent() = loadFromDesktop(aURL);

    // When storing that document as ODF:
    uno::Reference<frame::XStorable> xStorable(getComponent(), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProps = comphelper::InitPropertySequence({
        { "FilterName", uno::Any(OUString("writer8")) },
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

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testClearingBreakExport)
{
    // Given a document with a clearing break:
    getComponent() = loadFromDesktop("private:factory/swriter");
    uno::Reference<lang::XMultiServiceFactory> xMSF(getComponent(), uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xLineBreak(
        xMSF->createInstance("com.sun.star.text.LineBreak"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xLineBreakProps(xLineBreak, uno::UNO_QUERY);
    // SwLineBreakClear::ALL;
    sal_Int16 eClear = 3;
    xLineBreakProps->setPropertyValue("Clear", uno::Any(eClear));
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertTextContent(xCursor, xLineBreak, /*bAbsorb=*/false);

    // When exporting to ODT:
    uno::Reference<frame::XStorable> xStorable(getComponent(), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProps = comphelper::InitPropertySequence({
        { "FilterName", uno::Any(OUString("writer8")) },
    });
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    xStorable->storeToURL(aTempFile.GetURL(), aStoreProps);
    validate(aTempFile.GetFileName(), test::ODF);

    // Then make sure the expected markup is used:
    std::unique_ptr<SvStream> pStream = parseExportStream(aTempFile, "content.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    // Without the accompanying fix in place, this failed with:
    // - XPath '//text:line-break' number of nodes is incorrect
    // i.e. the clearing break was lost on export.
    assertXPath(pXmlDoc, "//text:line-break", "clear", "all");
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testClearingBreakImport)
{
    // Given an ODF document with a clearing break:
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "clearing-break.fodt";

    // When loading that document:
    getComponent() = loadFromDesktop(aURL);

    // Then make sure that the "clear" attribute is not lost on import:
    uno::Reference<text::XTextDocument> xTextDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParagraphsAccess(xTextDocument->getText(),
                                                                    uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphs = xParagraphsAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xParagraph(xParagraphs->nextElement(),
                                                             uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPortions = xParagraph->createEnumeration();
    // First portion is the image.
    xPortions->nextElement();
    // Second portion is "foo".
    xPortions->nextElement();
    // Without the accompanying fix in place, this failed with:
    // An uncaught exception of type com.sun.star.container.NoSuchElementException
    // i.e. the line break was a non-clearing one, so we only had 2 portions, not 4 (image, text,
    // linebreak, text).
    uno::Reference<beans::XPropertySet> xPortion(xPortions->nextElement(), uno::UNO_QUERY);
    OUString aTextPortionType;
    xPortion->getPropertyValue("TextPortionType") >>= aTextPortionType;
    CPPUNIT_ASSERT_EQUAL(OUString("LineBreak"), aTextPortionType);
    uno::Reference<text::XTextContent> xLineBreak;
    xPortion->getPropertyValue("LineBreak") >>= xLineBreak;
    uno::Reference<beans::XPropertySet> xLineBreakProps(xLineBreak, uno::UNO_QUERY);
    sal_Int16 eClear{};
    xLineBreakProps->getPropertyValue("Clear") >>= eClear;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(3), eClear);
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
    xStyle->setPropertyValue("Width", uno::Any(static_cast<sal_Int32>(10000)));
    uno::Reference<lang::XMultiServiceFactory> xMSF(getComponent(), uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xTextFrame(
        xMSF->createInstance("com.sun.star.text.TextFrame"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextFrameProps(xTextFrame, uno::UNO_QUERY);
    xTextFrameProps->setPropertyValue("RelativeWidth", uno::Any(static_cast<sal_Int16>(50)));
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertTextContent(xCursor, xTextFrame, /*bAbsorb=*/false);
    // Body frame width is 16cm.
    xStyle->setPropertyValue("Width", uno::Any(static_cast<sal_Int32>(20000)));

    uno::Reference<frame::XStorable> xStorable(getComponent(), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProps = comphelper::InitPropertySequence({
        { "FilterName", uno::Any(OUString("writer8")) },
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

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testContentControlExport)
{
    // Given a document with a content control around one or more text portions:
    getComponent() = loadFromDesktop("private:factory/swriter");
    uno::Reference<lang::XMultiServiceFactory> xMSF(getComponent(), uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, "test", /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance("com.sun.star.text.ContentControl"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    xContentControlProps->setPropertyValue("ShowingPlaceHolder", uno::Any(true));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When exporting to ODT:
    uno::Reference<frame::XStorable> xStorable(getComponent(), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProps = comphelper::InitPropertySequence({
        { "FilterName", uno::Any(OUString("writer8")) },
    });
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    xStorable->storeToURL(aTempFile.GetURL(), aStoreProps);
    validate(aTempFile.GetFileName(), test::ODF);

    // Then make sure the expected markup is used:
    std::unique_ptr<SvStream> pStream = parseExportStream(aTempFile, "content.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    // Without the accompanying fix in place, this failed with:
    // - XPath '//loext:content-control' number of nodes is incorrect
    // i.e. the content control was lost on export.
    assertXPath(pXmlDoc, "//loext:content-control", "showing-place-holder", "true");
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testContentControlImport)
{
    // Given an ODF document with a content control:
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "content-control.fodt";

    // When loading that document:
    getComponent() = loadFromDesktop(aURL);

    // Then make sure that the content control is not lost on import:
    uno::Reference<text::XTextDocument> xTextDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParagraphsAccess(xTextDocument->getText(),
                                                                    uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphs = xParagraphsAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xParagraph(xParagraphs->nextElement(),
                                                             uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPortions = xParagraph->createEnumeration();
    uno::Reference<beans::XPropertySet> xTextPortion(xPortions->nextElement(), uno::UNO_QUERY);
    OUString aPortionType;
    xTextPortion->getPropertyValue("TextPortionType") >>= aPortionType;
    // Without the accompanying fix in place, this failed with:
    // - Expected: ContentControl
    // - Actual  : Text
    // i.e. the content control was lost on import.
    CPPUNIT_ASSERT_EQUAL(OUString("ContentControl"), aPortionType);
    uno::Reference<text::XTextContent> xContentControl;
    xTextPortion->getPropertyValue("ContentControl") >>= xContentControl;
    uno::Reference<text::XTextRange> xContentControlRange(xContentControl, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xContentControlRange->getText();
    uno::Reference<container::XEnumerationAccess> xContentEnumAccess(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xContentEnum = xContentEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xContent(xContentEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("test"), xContent->getString());
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testCheckboxContentControlExport)
{
    // Given a document with a checkbox content control around a text portion:
    getComponent() = loadFromDesktop("private:factory/swriter");
    uno::Reference<lang::XMultiServiceFactory> xMSF(getComponent(), uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, OUString(u"☐"), /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance("com.sun.star.text.ContentControl"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    xContentControlProps->setPropertyValue("Checkbox", uno::Any(true));
    xContentControlProps->setPropertyValue("Checked", uno::Any(true));
    xContentControlProps->setPropertyValue("CheckedState", uno::Any(OUString(u"☒")));
    xContentControlProps->setPropertyValue("UncheckedState", uno::Any(OUString(u"☐")));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When exporting to ODT:
    uno::Reference<frame::XStorable> xStorable(getComponent(), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProps = comphelper::InitPropertySequence({
        { "FilterName", uno::Any(OUString("writer8")) },
    });
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    xStorable->storeToURL(aTempFile.GetURL(), aStoreProps);
    validate(aTempFile.GetFileName(), test::ODF);

    // Then make sure the expected markup is used:
    std::unique_ptr<SvStream> pStream = parseExportStream(aTempFile, "content.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    assertXPath(pXmlDoc, "//loext:content-control", "checkbox", "true");
    assertXPath(pXmlDoc, "//loext:content-control", "checked", "true");
    assertXPath(pXmlDoc, "//loext:content-control", "checked-state", u"☒");
    assertXPath(pXmlDoc, "//loext:content-control", "unchecked-state", u"☐");
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testCheckboxContentControlImport)
{
    // Given an ODF document with a checkbox content control:
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "content-control-checkbox.fodt";

    // When loading that document:
    getComponent() = loadFromDesktop(aURL);

    // Then make sure that the content control is not lost on import:
    uno::Reference<text::XTextDocument> xTextDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParagraphsAccess(xTextDocument->getText(),
                                                                    uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphs = xParagraphsAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xParagraph(xParagraphs->nextElement(),
                                                             uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPortions = xParagraph->createEnumeration();
    uno::Reference<beans::XPropertySet> xTextPortion(xPortions->nextElement(), uno::UNO_QUERY);
    OUString aPortionType;
    xTextPortion->getPropertyValue("TextPortionType") >>= aPortionType;
    CPPUNIT_ASSERT_EQUAL(OUString("ContentControl"), aPortionType);
    uno::Reference<text::XTextContent> xContentControl;
    xTextPortion->getPropertyValue("ContentControl") >>= xContentControl;
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    bool bCheckbox{};
    xContentControlProps->getPropertyValue("Checkbox") >>= bCheckbox;
    // Without the accompanying fix in place, this failed, as the checkbox-related attributes were
    // ignored on import.
    CPPUNIT_ASSERT(bCheckbox);
    bool bChecked{};
    xContentControlProps->getPropertyValue("Checked") >>= bChecked;
    CPPUNIT_ASSERT(bChecked);
    OUString aCheckedState;
    xContentControlProps->getPropertyValue("CheckedState") >>= aCheckedState;
    CPPUNIT_ASSERT_EQUAL(OUString(u"☒"), aCheckedState);
    OUString aUncheckedState;
    xContentControlProps->getPropertyValue("UncheckedState") >>= aUncheckedState;
    CPPUNIT_ASSERT_EQUAL(OUString(u"☐"), aUncheckedState);
    uno::Reference<text::XTextRange> xContentControlRange(xContentControl, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xContentControlRange->getText();
    uno::Reference<container::XEnumerationAccess> xContentEnumAccess(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xContentEnum = xContentEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xContent(xContentEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(u"☒"), xContent->getString());
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testDropdownContentControlExport)
{
    // Given a document with a dropdown content control around a text portion:
    getComponent() = loadFromDesktop("private:factory/swriter");
    uno::Reference<lang::XMultiServiceFactory> xMSF(getComponent(), uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, "choose an item", /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance("com.sun.star.text.ContentControl"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    {
        uno::Sequence<beans::PropertyValues> aListItems = {
            {
                comphelper::makePropertyValue("DisplayText", uno::Any(OUString("red"))),
                comphelper::makePropertyValue("Value", uno::Any(OUString("R"))),
            },
            {
                comphelper::makePropertyValue("DisplayText", uno::Any(OUString("green"))),
                comphelper::makePropertyValue("Value", uno::Any(OUString("G"))),
            },
            {
                comphelper::makePropertyValue("DisplayText", uno::Any(OUString("blue"))),
                comphelper::makePropertyValue("Value", uno::Any(OUString("B"))),
            },
        };
        xContentControlProps->setPropertyValue("ListItems", uno::Any(aListItems));
    }
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When exporting to ODT:
    uno::Reference<frame::XStorable> xStorable(getComponent(), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProps = comphelper::InitPropertySequence({
        { "FilterName", uno::Any(OUString("writer8")) },
    });
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    xStorable->storeToURL(aTempFile.GetURL(), aStoreProps);
    validate(aTempFile.GetFileName(), test::ODF);

    // Then make sure the expected markup is used:
    std::unique_ptr<SvStream> pStream = parseExportStream(aTempFile, "content.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    // Without the accompanying fix in place, this failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//loext:content-control/loext:list-item[1]' number of nodes is incorrect
    // i.e. the list items were lost on export.
    assertXPath(pXmlDoc, "//loext:content-control/loext:list-item[1]", "display-text", "red");
    assertXPath(pXmlDoc, "//loext:content-control/loext:list-item[1]", "value", "R");
    assertXPath(pXmlDoc, "//loext:content-control/loext:list-item[2]", "display-text", "green");
    assertXPath(pXmlDoc, "//loext:content-control/loext:list-item[2]", "value", "G");
    assertXPath(pXmlDoc, "//loext:content-control/loext:list-item[3]", "display-text", "blue");
    assertXPath(pXmlDoc, "//loext:content-control/loext:list-item[3]", "value", "B");
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testDropdownContentControlImport)
{
    // Given an ODF document with a dropdown content control:
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "content-control-dropdown.fodt";

    // When loading that document:
    getComponent() = loadFromDesktop(aURL);

    // Then make sure that the content control is not lost on import:
    uno::Reference<text::XTextDocument> xTextDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParagraphsAccess(xTextDocument->getText(),
                                                                    uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphs = xParagraphsAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xParagraph(xParagraphs->nextElement(),
                                                             uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPortions = xParagraph->createEnumeration();
    uno::Reference<beans::XPropertySet> xTextPortion(xPortions->nextElement(), uno::UNO_QUERY);
    OUString aPortionType;
    xTextPortion->getPropertyValue("TextPortionType") >>= aPortionType;
    CPPUNIT_ASSERT_EQUAL(OUString("ContentControl"), aPortionType);
    uno::Reference<text::XTextContent> xContentControl;
    xTextPortion->getPropertyValue("ContentControl") >>= xContentControl;
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValues> aListItems;
    xContentControlProps->getPropertyValue("ListItems") >>= aListItems;
    // Without the accompanying fix in place, this failed with:
    // - Expected: 3
    // - Actual  : 0
    // i.e. the list items were lost on import.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), aListItems.getLength());
    comphelper::SequenceAsHashMap aMap0(aListItems[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("red"), aMap0["DisplayText"].get<OUString>());
    CPPUNIT_ASSERT_EQUAL(OUString("R"), aMap0["Value"].get<OUString>());
    comphelper::SequenceAsHashMap aMap1(aListItems[1]);
    CPPUNIT_ASSERT_EQUAL(OUString("green"), aMap1["DisplayText"].get<OUString>());
    CPPUNIT_ASSERT_EQUAL(OUString("G"), aMap1["Value"].get<OUString>());
    comphelper::SequenceAsHashMap aMap2(aListItems[2]);
    CPPUNIT_ASSERT_EQUAL(OUString("blue"), aMap2["DisplayText"].get<OUString>());
    CPPUNIT_ASSERT_EQUAL(OUString("B"), aMap2["Value"].get<OUString>());
    uno::Reference<text::XTextRange> xContentControlRange(xContentControl, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xContentControlRange->getText();
    uno::Reference<container::XEnumerationAccess> xContentEnumAccess(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xContentEnum = xContentEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xContent(xContentEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("choose a color"), xContent->getString());
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testPictureContentControlExport)
{
    // Given a document with a picture content control around an as-char image:
    getComponent() = loadFromDesktop("private:factory/swriter");
    uno::Reference<lang::XMultiServiceFactory> xMSF(getComponent(), uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<beans::XPropertySet> xTextGraphic(
        xMSF->createInstance("com.sun.star.text.TextGraphicObject"), uno::UNO_QUERY);
    xTextGraphic->setPropertyValue("AnchorType",
                                   uno::Any(text::TextContentAnchorType_AS_CHARACTER));
    uno::Reference<text::XTextContent> xTextContent(xTextGraphic, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xTextContent, false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance("com.sun.star.text.ContentControl"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    xContentControlProps->setPropertyValue("Picture", uno::Any(true));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When exporting to ODT:
    uno::Reference<frame::XStorable> xStorable(getComponent(), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProps = comphelper::InitPropertySequence({
        { "FilterName", uno::Any(OUString("writer8")) },
    });
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    xStorable->storeToURL(aTempFile.GetURL(), aStoreProps);
    validate(aTempFile.GetFileName(), test::ODF);

    // Then make sure the expected markup is used:
    std::unique_ptr<SvStream> pStream = parseExportStream(aTempFile, "content.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '//loext:content-control' no attribute 'picture' exist
    assertXPath(pXmlDoc, "//loext:content-control", "picture", "true");
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testPictureContentControlImport)
{
    // Given an ODF document with a picture content control:
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "content-control-picture.fodt";

    // When loading that document:
    getComponent() = loadFromDesktop(aURL);

    // Then make sure that the content control is not lost on import:
    uno::Reference<text::XTextDocument> xTextDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParagraphsAccess(xTextDocument->getText(),
                                                                    uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphs = xParagraphsAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xParagraph(xParagraphs->nextElement(),
                                                             uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPortions = xParagraph->createEnumeration();
    uno::Reference<beans::XPropertySet> xTextPortion(xPortions->nextElement(), uno::UNO_QUERY);
    OUString aPortionType;
    xTextPortion->getPropertyValue("TextPortionType") >>= aPortionType;
    CPPUNIT_ASSERT_EQUAL(OUString("ContentControl"), aPortionType);
    uno::Reference<text::XTextContent> xContentControl;
    xTextPortion->getPropertyValue("ContentControl") >>= xContentControl;
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    bool bPicture{};
    xContentControlProps->getPropertyValue("Picture") >>= bPicture;
    // Without the accompanying fix in place, this failed, as the picture attribute was ignored on
    // import.
    CPPUNIT_ASSERT(bPicture);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testDateContentControlExport)
{
    // Given a document with a date content control around a text portion:
    getComponent() = loadFromDesktop("private:factory/swriter");
    uno::Reference<lang::XMultiServiceFactory> xMSF(getComponent(), uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, "choose a date", /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance("com.sun.star.text.ContentControl"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    xContentControlProps->setPropertyValue("Date", uno::Any(true));
    xContentControlProps->setPropertyValue("DateFormat", uno::Any(OUString("YYYY-MM-DD")));
    xContentControlProps->setPropertyValue("DateLanguage", uno::Any(OUString("en-US")));
    xContentControlProps->setPropertyValue("CurrentDate",
                                           uno::Any(OUString("2022-05-25T00:00:00Z")));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When exporting to ODT:
    uno::Reference<frame::XStorable> xStorable(getComponent(), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProps = comphelper::InitPropertySequence({
        { "FilterName", uno::Any(OUString("writer8")) },
    });
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    xStorable->storeToURL(aTempFile.GetURL(), aStoreProps);
    validate(aTempFile.GetFileName(), test::ODF);

    // Then make sure the expected markup is used:
    std::unique_ptr<SvStream> pStream = parseExportStream(aTempFile, "content.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '//loext:content-control' no attribute 'date' exist
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    assertXPath(pXmlDoc, "//loext:content-control", "date", "true");
    assertXPath(pXmlDoc, "//loext:content-control", "date-format", "YYYY-MM-DD");
    assertXPath(pXmlDoc, "//loext:content-control", "date-rfc-language-tag", "en-US");
    assertXPath(pXmlDoc, "//loext:content-control", "current-date", "2022-05-25T00:00:00Z");
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testDateContentControlImport)
{
    // Given an ODF document with a date content control:
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "content-control-date.fodt";

    // When loading that document:
    getComponent() = loadFromDesktop(aURL);

    // Then make sure that the content control is not lost on import:
    uno::Reference<text::XTextDocument> xTextDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParagraphsAccess(xTextDocument->getText(),
                                                                    uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphs = xParagraphsAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xParagraph(xParagraphs->nextElement(),
                                                             uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPortions = xParagraph->createEnumeration();
    uno::Reference<beans::XPropertySet> xTextPortion(xPortions->nextElement(), uno::UNO_QUERY);
    OUString aPortionType;
    xTextPortion->getPropertyValue("TextPortionType") >>= aPortionType;
    CPPUNIT_ASSERT_EQUAL(OUString("ContentControl"), aPortionType);
    uno::Reference<text::XTextContent> xContentControl;
    xTextPortion->getPropertyValue("ContentControl") >>= xContentControl;
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    bool bDate{};
    xContentControlProps->getPropertyValue("Date") >>= bDate;
    // Without the accompanying fix in place, this test would have failed, the content control was
    // imported as a default rich text one.
    CPPUNIT_ASSERT(bDate);
    OUString aDateFormat;
    xContentControlProps->getPropertyValue("DateFormat") >>= aDateFormat;
    CPPUNIT_ASSERT_EQUAL(OUString("YYYY-MM-DD"), aDateFormat);
    OUString aDateLanguage;
    xContentControlProps->getPropertyValue("DateLanguage") >>= aDateLanguage;
    CPPUNIT_ASSERT_EQUAL(OUString("en-US"), aDateLanguage);
    OUString aCurrentDate;
    xContentControlProps->getPropertyValue("CurrentDate") >>= aCurrentDate;
    CPPUNIT_ASSERT_EQUAL(OUString("2022-05-25T00:00:00Z"), aCurrentDate);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
