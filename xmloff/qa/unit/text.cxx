/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapixml_test.hxx>

#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/text/BibliographyDataType.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>

#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <unotools/tempfile.hxx>
#include <docmodel/uno/UnoTheme.hxx>
#include <docmodel/theme/Theme.hxx>

using namespace ::com::sun::star;

/// Covers xmloff/source/text/ fixes.
class XmloffStyleTest : public UnoApiXmlTest
{
public:
    XmloffStyleTest();
};

XmloffStyleTest::XmloffStyleTest()
    : UnoApiXmlTest("/xmloff/qa/unit/data/")
{
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testMailMergeInEditeng)
{
    // Without the accompanying fix in place, this test would have failed, as unexpected
    // <text:database-display> in editeng text aborted the whole import process.
    loadFromFile(u"mail-merge-editeng.odt");
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testCommentProperty)
{
    mxComponent = loadFromDesktop("private:factory/swriter");
    uno::Sequence<beans::PropertyValue> aCommentProps = comphelper::InitPropertySequence({
        { "Text", uno::Any(OUString("comment")) },
    });
    dispatchCommand(mxComponent, ".uno:InsertAnnotation", aCommentProps);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPortionEnum = xPara->createEnumeration();
    uno::Reference<beans::XPropertySet> xPortion(xPortionEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xField(xPortion->getPropertyValue("TextField"),
                                               uno::UNO_QUERY);
    xField->setPropertyValue("Resolved", uno::Any(true));
    xField->setPropertyValue("ParentName", uno::Any(OUString("parent_comment_name")));

    saveAndReload("writer8");
    xTextDocument.set(mxComponent, uno::UNO_QUERY);
    xParaEnumAccess.set(xTextDocument->getText(), uno::UNO_QUERY);
    xParaEnum = xParaEnumAccess->createEnumeration();
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPortionEnum = xPara->createEnumeration();
    xPortion.set(xPortionEnum->nextElement(), uno::UNO_QUERY);
    xField.set(xPortion->getPropertyValue("TextField"), uno::UNO_QUERY);
    bool bResolved = false;
    xField->getPropertyValue("Resolved") >>= bResolved;
    OUString parentName;
    xField->getPropertyValue("ParentName") >>= parentName;
    CPPUNIT_ASSERT_EQUAL(
        OUString("parent_comment_name"),
        parentName); // Check if the parent comment name is written and read correctly.
    // Without the accompanying fix in place, this test would have failed, as the resolved state was
    // not saved for non-range comments.
    CPPUNIT_ASSERT(bResolved);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testBibliographyLocalUrl)
{
    // Given a document with a biblio field, with non-empty LocalURL:
    mxComponent = loadFromDesktop("private:factory/swriter");
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
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
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextContent> xContent(xField, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContent, /*bAbsorb=*/false);

    // When invoking ODT export + import on it:
    saveAndReload("writer8");
    // Without the accompanying fix in place, this test would have resulted in an assertion failure,
    // as LocalURL was mapped to XML_TOKEN_INVALID.

    // Then make sure that LocalURL is preserved:
    xTextDocument.set(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPortionEnum = xPara->createEnumeration();
    uno::Reference<beans::XPropertySet> xPortion(xPortionEnum->nextElement(), uno::UNO_QUERY);
    xField.set(xPortion->getPropertyValue("TextField"), uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aMap(xField->getPropertyValue("Fields"));
    CPPUNIT_ASSERT(aMap.contains("LocalURL"));
    auto aActual = aMap["LocalURL"].get<OUString>();
    CPPUNIT_ASSERT_EQUAL(OUString("file:///home/me/test.pdf"), aActual);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testBibliographyTargetURL1)
{
    // Given a document with a biblio field, with non-empty LocalURL:
    mxComponent = loadFromDesktop("private:factory/swriter");
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xField(
        xFactory->createInstance("com.sun.star.text.TextField.Bibliography"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aFields = {
        comphelper::makePropertyValue("Identifier", OUString("AT")),
        comphelper::makePropertyValue("URL", OUString("https://display.url/test1.pdf#page=1")),
        comphelper::makePropertyValue("TargetType", OUString("1")),
        comphelper::makePropertyValue("TargetURL", OUString("https://target.url/test2.pdf#page=2")),
    };
    xField->setPropertyValue("Fields", uno::Any(aFields));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextContent> xContent(xField, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContent, /*bAbsorb=*/false);

    // When invoking ODT export + import on it:
    saveAndReload("writer8");

    // Then make sure that URL, TargetURL and UseTargetURL are preserved and independent:
    xTextDocument.set(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPortionEnum = xPara->createEnumeration();
    uno::Reference<beans::XPropertySet> xPortion(xPortionEnum->nextElement(), uno::UNO_QUERY);
    xField.set(xPortion->getPropertyValue("TextField"), uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aMap(xField->getPropertyValue("Fields"));

    CPPUNIT_ASSERT(aMap.contains("URL"));
    CPPUNIT_ASSERT_EQUAL(OUString("https://display.url/test1.pdf#page=1"),
                         aMap["URL"].get<OUString>());

    CPPUNIT_ASSERT(aMap.contains("TargetURL"));
    CPPUNIT_ASSERT_EQUAL(OUString("https://target.url/test2.pdf#page=2"),
                         aMap["TargetURL"].get<OUString>());

    CPPUNIT_ASSERT(aMap.contains("TargetType"));
    CPPUNIT_ASSERT_EQUAL(OUString("1"), aMap["TargetType"].get<OUString>());
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testCommentTableBorder)
{
    // Without the accompanying fix in place, this failed to load, as a comment that started in a
    // table and ended outside a table aborted the whole importer.
    loadFromFile(u"comment-table-border.fodt");
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testParaStyleListLevel)
{
    // Given a document with style:list-level="...":
    loadFromFile(u"para-style-list-level.fodt");

    // Then make sure we map that to the paragraph style's numbering level:
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
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
    save("writer8");

    // Then make sure we save the style's numbering level:
    xmlDocUniquePtr pXmlDoc = parseExport("styles.xml");
    // Without the accompanying fix in place, this failed with:
    // - XPath '/office:document-styles/office:styles/style:style[@style:name='mystyle']' no attribute 'list-level' exist
    // i.e. a custom NumberingLevel was lost on save.
    assertXPath(pXmlDoc,
                "/office:document-styles/office:styles/style:style[@style:name='mystyle']"_ostr,
                "list-level"_ostr, "2");
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testContinueNumberingWord)
{
    // Given a document, which is produced by Word and contains text:continue-numbering="true":
    loadFromFile(u"continue-numbering-word.odt");

    // Then make sure that the numbering from the 1st para is continued on the 3rd para:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
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
    loadFromFile(u"list-id.fodt");

    // When storing that document as ODF:
    save("writer8");

    // Then make sure that unreferenced xml:id="..." attributes are not written:
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    // Without the accompanying fix in place, this failed with:
    // - XPath '//text:list' unexpected 'id' attribute
    // i.e. xml:id="..." was written unconditionally, even when no other list needed it.
    assertXPathNoAttribute(pXmlDoc, "//text:list"_ostr, "id"_ostr);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testListId2)
{
    // tdf#155823 Given a document with a list consisting of items having different list styles:
    loadFromFile(u"differentListStylesInOneList.fodt");

    auto xTextDocument(mxComponent.queryThrow<css::text::XTextDocument>());
    auto xParaEnumAccess(xTextDocument->getText().queryThrow<css::container::XEnumerationAccess>());
    auto xParaEnum(xParaEnumAccess->createEnumeration());

    auto xPara(xParaEnum->nextElement().queryThrow<beans::XPropertySet>());
    auto aActual(xPara->getPropertyValue("ListLabelString").get<OUString>());
    CPPUNIT_ASSERT_EQUAL(OUString("1."), aActual);
    xParaEnum->nextElement(); // Skip empty intermediate paragraph
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    aActual = xPara->getPropertyValue("ListLabelString").get<OUString>();
    CPPUNIT_ASSERT_EQUAL(OUString("2."), aActual);
    xParaEnum->nextElement(); // Skip empty intermediate paragraph
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    aActual = xPara->getPropertyValue("ListLabelString").get<OUString>();
    CPPUNIT_ASSERT_EQUAL(OUString("3."), aActual);
    xParaEnum->nextElement(); // Skip empty intermediate paragraph
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    aActual = xPara->getPropertyValue("ListLabelString").get<OUString>();
    CPPUNIT_ASSERT_EQUAL(OUString("4."), aActual);

    // When storing that document as ODF:
    // Without the fix in place, automatic validation would fail with:
    // Error: "list123456789012345" is referenced by an IDREF, but not defined.
    saveAndReload("writer8");

    xTextDocument.set(mxComponent.queryThrow<css::text::XTextDocument>());
    xParaEnumAccess.set(xTextDocument->getText().queryThrow<css::container::XEnumerationAccess>());
    xParaEnum.set(xParaEnumAccess->createEnumeration());

    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    aActual = xPara->getPropertyValue("ListLabelString").get<OUString>();
    CPPUNIT_ASSERT_EQUAL(OUString("1."), aActual);
    xParaEnum->nextElement(); // Skip empty intermediate paragraph
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    aActual = xPara->getPropertyValue("ListLabelString").get<OUString>();
    CPPUNIT_ASSERT_EQUAL(OUString("2."), aActual);
    xParaEnum->nextElement(); // Skip empty intermediate paragraph
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    aActual = xPara->getPropertyValue("ListLabelString").get<OUString>();
    CPPUNIT_ASSERT_EQUAL(OUString("3."), aActual);
    xParaEnum->nextElement(); // Skip empty intermediate paragraph

    // Check that the last item number is correct

    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    aActual = xPara->getPropertyValue("ListLabelString").get<OUString>();
    // Without the fix in place, this would fail with:
    // - Expected: 4.
    // - Actual  : 1.
    // i.e. the numbering was not continued.
    CPPUNIT_ASSERT_EQUAL(OUString("4."), aActual);

    // Then make sure that required xml:id="..." attributes is written when the style changes:
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the fix in place, this would fail,
    // i.e. xml:id="..." was omitted, even though it was needed for the next item.
    OUString id = getXPath(
        pXmlDoc, "/office:document-content/office:body/office:text/text:list[3]"_ostr, "id"_ostr);
    CPPUNIT_ASSERT(!id.isEmpty());
    assertXPath(pXmlDoc, "/office:document-content/office:body/office:text/text:list[4]"_ostr,
                "continue-list"_ostr, id);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testListIdState)
{
    // tdf#149668: given a document with 3 paragraphs: an outer numbering on para 1 & 3, an inner
    // numbering on para 2:
    mxComponent = loadFromDesktop("private:factory/swriter");
    auto xTextDocument(mxComponent.queryThrow<text::XTextDocument>());
    auto xText(xTextDocument->getText());
    xText->insertControlCharacter(xText->getEnd(), css::text::ControlCharacter::PARAGRAPH_BREAK,
                                  false);
    xText->insertControlCharacter(xText->getEnd(), css::text::ControlCharacter::PARAGRAPH_BREAK,
                                  false);

    auto paraEnumAccess(xText.queryThrow<container::XEnumerationAccess>());
    auto paraEnum(paraEnumAccess->createEnumeration());
    auto xParaProps(paraEnum->nextElement().queryThrow<beans::XPropertySet>());
    xParaProps->setPropertyValue("NumberingStyleName", css::uno::Any(OUString("Numbering ABC")));
    xParaProps.set(paraEnum->nextElement().queryThrow<beans::XPropertySet>());
    xParaProps->setPropertyValue("NumberingStyleName", css::uno::Any(OUString("Numbering 123")));
    xParaProps.set(paraEnum->nextElement().queryThrow<beans::XPropertySet>());
    xParaProps->setPropertyValue("NumberingStyleName", css::uno::Any(OUString("Numbering ABC")));

    // When storing that document as ODF:
    save("writer8");
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");

    // Make sure that xml:id="..." gets written for para 1, as it'll be continued in para 3.
    // Without the accompanying fix in place, this test would have failed,
    // i.e. para 1 didn't write an xml:id="..." but para 3 referred to it using continue-list="...",
    // which is inconsistent.
    OUString id = getXPath(
        pXmlDoc, "/office:document-content/office:body/office:text/text:list[1]"_ostr, "id"_ostr);
    CPPUNIT_ASSERT(!id.isEmpty());
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testListIdOnRestart)
{
    // Test that a restart of a continued list, by itself, does not introduce a unneeded xml:id
    // and text:continue-list, but uses text:continue-numbering, and is imported correctly.

    // Given a document with a list with a restart after break:
    loadFromFile(u"listRestartAfterBreak.fodt");

    auto xTextDocument(mxComponent.queryThrow<css::text::XTextDocument>());
    auto xParaEnumAccess(xTextDocument->getText().queryThrow<css::container::XEnumerationAccess>());
    auto xParaEnum(xParaEnumAccess->createEnumeration());

    auto xPara(xParaEnum->nextElement().queryThrow<beans::XPropertySet>());
    auto aActual(xPara->getPropertyValue("ListLabelString").get<OUString>());
    CPPUNIT_ASSERT_EQUAL(OUString("1."), aActual);
    OUString list_id = xPara->getPropertyValue("ListId").get<OUString>();
    xParaEnum->nextElement(); // Skip empty intermediate paragraph
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY_THROW);
    aActual = xPara->getPropertyValue("ListLabelString").get<OUString>();
    CPPUNIT_ASSERT_EQUAL(OUString("2."), aActual);
    CPPUNIT_ASSERT_EQUAL(list_id, xPara->getPropertyValue("ListId").get<OUString>());
    xParaEnum->nextElement(); // Skip empty intermediate paragraph
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    aActual = xPara->getPropertyValue("ListLabelString").get<OUString>();
    // Check that restart was applied correctly, with simple 'text:continue-numbering="true"'
    CPPUNIT_ASSERT_EQUAL(OUString("1."), aActual);
    CPPUNIT_ASSERT_EQUAL(list_id, xPara->getPropertyValue("ListId").get<OUString>());

    // When storing that document as ODF:
    saveAndReload("writer8");

    xTextDocument.set(mxComponent, uno::UNO_QUERY_THROW);
    xParaEnumAccess.set(xTextDocument->getText(), uno::UNO_QUERY_THROW);
    xParaEnum.set(xParaEnumAccess->createEnumeration());

    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY_THROW);
    aActual = xPara->getPropertyValue("ListLabelString").get<OUString>();
    CPPUNIT_ASSERT_EQUAL(OUString("1."), aActual);
    list_id = xPara->getPropertyValue("ListId").get<OUString>();
    xParaEnum->nextElement(); // Skip empty intermediate paragraph
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY_THROW);
    aActual = xPara->getPropertyValue("ListLabelString").get<OUString>();
    CPPUNIT_ASSERT_EQUAL(OUString("2."), aActual);
    CPPUNIT_ASSERT_EQUAL(list_id, xPara->getPropertyValue("ListId").get<OUString>());
    xParaEnum->nextElement(); // Skip empty intermediate paragraph
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY_THROW);
    aActual = xPara->getPropertyValue("ListLabelString").get<OUString>();
    CPPUNIT_ASSERT_EQUAL(OUString("1."), aActual);
    CPPUNIT_ASSERT_EQUAL(list_id, xPara->getPropertyValue("ListId").get<OUString>());

    // Then make sure that no xml:id="..." attribute is written, even in restarted case:
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "//text:list"_ostr, 3);
    assertXPathNoAttribute(pXmlDoc, "//text:list[1]"_ostr, "id"_ostr);
    assertXPathNoAttribute(pXmlDoc, "//text:list[2]"_ostr, "id"_ostr);
    assertXPathNoAttribute(pXmlDoc, "//text:list[3]"_ostr, "id"_ostr);
    assertXPathNoAttribute(pXmlDoc, "//text:list[3]"_ostr, "continue-list"_ostr);
    assertXPath(pXmlDoc, "//text:list[3]"_ostr, "continue-numbering"_ostr, "true");
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testClearingBreakExport)
{
    // Given a document with a clearing break:
    mxComponent = loadFromDesktop("private:factory/swriter");
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
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
    save("writer8");

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    // Without the accompanying fix in place, this failed with:
    // - XPath '//text:line-break' number of nodes is incorrect
    // i.e. the clearing break was lost on export.
    assertXPath(pXmlDoc, "//text:line-break"_ostr, "clear"_ostr, "all");
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testClearingBreakImport)
{
    // Given an ODF document with a clearing break:
    loadFromFile(u"clearing-break.fodt");

    // Then make sure that the "clear" attribute is not lost on import:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
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
    mxComponent = loadFromDesktop("private:factory/swriter");
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName("PageStyles"),
                                                        uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStyle(xStyleFamily->getByName("Standard"), uno::UNO_QUERY);
    // Body frame width is 6cm (2+2cm margin).
    xStyle->setPropertyValue("Width", uno::Any(static_cast<sal_Int32>(10000)));
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xTextFrame(
        xMSF->createInstance("com.sun.star.text.TextFrame"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextFrameProps(xTextFrame, uno::UNO_QUERY);
    xTextFrameProps->setPropertyValue("RelativeWidth", uno::Any(static_cast<sal_Int16>(50)));
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertTextContent(xCursor, xTextFrame, /*bAbsorb=*/false);
    // Body frame width is 16cm.
    xStyle->setPropertyValue("Width", uno::Any(static_cast<sal_Int32>(20000)));

    save("writer8");

    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    // Without the accompanying fix in place, this failed with:
    // - Expected: 3.15in (8cm)
    // - Actual  : 0.0161in (0.04 cm)
    // i.e. the fallback width value wasn't the expected half of the body frame width, but a smaller
    // value.
    assertXPath(pXmlDoc, "//draw:frame"_ostr, "width"_ostr, "3.15in");
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testScaleWidthAndHeight)
{
    // Given a broken document where both IsSyncHeightToWidth and IsSyncWidthToHeight are set to
    // true:
    mxComponent = loadFromDesktop("private:factory/swriter");
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xTextFrame(
        xMSF->createInstance("com.sun.star.text.TextFrame"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextFrameProps(xTextFrame, uno::UNO_QUERY);
    xTextFrameProps->setPropertyValue("Width", uno::Any(static_cast<sal_Int16>(2000)));
    xTextFrameProps->setPropertyValue("Height", uno::Any(static_cast<sal_Int16>(1000)));
    xTextFrameProps->setPropertyValue("IsSyncHeightToWidth", uno::Any(true));
    xTextFrameProps->setPropertyValue("IsSyncWidthToHeight", uno::Any(true));
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertTextContent(xCursor, xTextFrame, /*bAbsorb=*/false);

    // When exporting to ODT:
    save("writer8");

    // Then make sure that we still export a non-zero size:
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    // Without the accompanying fix in place, this failed with:
    // - Expected: 0.7874in
    // - Actual  : 0in
    // i.e. the exported size was 0, not 2000 mm100 in inches.
    assertXPath(pXmlDoc, "//draw:frame"_ostr, "width"_ostr, "0.7874in");
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testContentControlExport)
{
    // Given a document with a content control around one or more text portions:
    mxComponent = loadFromDesktop("private:factory/swriter");
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
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
    save("writer8");

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    // Without the accompanying fix in place, this failed with:
    // - XPath '//loext:content-control' number of nodes is incorrect
    // i.e. the content control was lost on export.
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "showing-place-holder"_ostr, "true");
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testContentControlImport)
{
    // Given an ODF document with a content control:
    loadFromFile(u"content-control.fodt");

    // Then make sure that the content control is not lost on import:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
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
    mxComponent = loadFromDesktop("private:factory/swriter");
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, u"☐"_ustr, /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance("com.sun.star.text.ContentControl"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    xContentControlProps->setPropertyValue("Checkbox", uno::Any(true));
    xContentControlProps->setPropertyValue("Checked", uno::Any(true));
    xContentControlProps->setPropertyValue("CheckedState", uno::Any(u"☒"_ustr));
    xContentControlProps->setPropertyValue("UncheckedState", uno::Any(u"☐"_ustr));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When exporting to ODT:
    save("writer8");

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "checkbox"_ostr, "true");
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "checked"_ostr, "true");
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "checked-state"_ostr, u"☒"_ustr);
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "unchecked-state"_ostr, u"☐"_ustr);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testCheckboxContentControlImport)
{
    // Given an ODF document with a checkbox content control:
    loadFromFile(u"content-control-checkbox.fodt");

    // Then make sure that the content control is not lost on import:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
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
    CPPUNIT_ASSERT_EQUAL(u"☒"_ustr, aCheckedState);
    OUString aUncheckedState;
    xContentControlProps->getPropertyValue("UncheckedState") >>= aUncheckedState;
    CPPUNIT_ASSERT_EQUAL(u"☐"_ustr, aUncheckedState);
    uno::Reference<text::XTextRange> xContentControlRange(xContentControl, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xContentControlRange->getText();
    uno::Reference<container::XEnumerationAccess> xContentEnumAccess(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xContentEnum = xContentEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xContent(xContentEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"☒"_ustr, xContent->getString());
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testDropdownContentControlExport)
{
    // Given a document with a dropdown content control around a text portion:
    mxComponent = loadFromDesktop("private:factory/swriter");
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, "choose an item", /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance("com.sun.star.text.ContentControl"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    {
        xContentControlProps->setPropertyValue("DropDown", uno::Any(true));
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
    save("writer8");

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "dropdown"_ostr, "true");
    // Without the accompanying fix in place, this failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//loext:content-control/loext:list-item[1]' number of nodes is incorrect
    // i.e. the list items were lost on export.
    assertXPath(pXmlDoc, "//loext:content-control/loext:list-item[1]"_ostr, "display-text"_ostr,
                "red");
    assertXPath(pXmlDoc, "//loext:content-control/loext:list-item[1]"_ostr, "value"_ostr, "R");
    assertXPath(pXmlDoc, "//loext:content-control/loext:list-item[2]"_ostr, "display-text"_ostr,
                "green");
    assertXPath(pXmlDoc, "//loext:content-control/loext:list-item[2]"_ostr, "value"_ostr, "G");
    assertXPath(pXmlDoc, "//loext:content-control/loext:list-item[3]"_ostr, "display-text"_ostr,
                "blue");
    assertXPath(pXmlDoc, "//loext:content-control/loext:list-item[3]"_ostr, "value"_ostr, "B");
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testDropdownContentControlImport)
{
    // Given an ODF document with a dropdown content control:
    loadFromFile(u"content-control-dropdown.fodt");

    // Then make sure that the content control is not lost on import:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
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
    mxComponent = loadFromDesktop("private:factory/swriter");
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
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
    save("writer8");

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '//loext:content-control' no attribute 'picture' exist
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "picture"_ostr, "true");
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testPictureContentControlImport)
{
    // Given an ODF document with a picture content control:
    loadFromFile(u"content-control-picture.fodt");

    // Then make sure that the content control is not lost on import:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
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
    mxComponent = loadFromDesktop("private:factory/swriter");
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
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
    save("writer8");

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '//loext:content-control' no attribute 'date' exist
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "date"_ostr, "true");
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "date-format"_ostr, "YYYY-MM-DD");
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "date-rfc-language-tag"_ostr, "en-US");
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "current-date"_ostr,
                "2022-05-25T00:00:00Z");
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testDateContentControlImport)
{
    // Given an ODF document with a date content control:
    loadFromFile(u"content-control-date.fodt");

    // Then make sure that the content control is not lost on import:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
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

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testPlainTextContentControlExport)
{
    // Given a document with a plain text content control around a text portion:
    mxComponent = loadFromDesktop("private:factory/swriter");
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, "test", /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance("com.sun.star.text.ContentControl"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    xContentControlProps->setPropertyValue("PlainText", uno::Any(true));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When exporting to ODT:
    save("writer8");

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '//loext:content-control' no attribute 'plain-text' exist
    // i.e. the plain text content control was turned into a rich text one on export.
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "plain-text"_ostr, "true");
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testPlainTextContentControlImport)
{
    // Given an ODF document with a plain-text content control:
    loadFromFile(u"content-control-plain-text.fodt");

    // Then make sure that the content control is not lost on import:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
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
    bool bPlainText{};
    xContentControlProps->getPropertyValue("PlainText") >>= bPlainText;
    // Without the accompanying fix in place, this test would have failed, the import result was a
    // rich text content control (not a plain text one).
    CPPUNIT_ASSERT(bPlainText);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testComboBoxContentControlExport)
{
    // Given a document with a combo box content control around a text portion:
    mxComponent = loadFromDesktop("private:factory/swriter");
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, "test", /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance("com.sun.star.text.ContentControl"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    xContentControlProps->setPropertyValue("ComboBox", uno::Any(true));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When exporting to ODT:
    save("writer8");

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '//loext:content-control' no attribute 'combobox' exist
    // i.e. the combo box content control was turned into a drop-down one on export.
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "combobox"_ostr, "true");
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testAliasContentControlExport)
{
    // Given a document with a content control and its alias around a text portion:
    mxComponent = loadFromDesktop("private:factory/swriter");
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, "test", /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance("com.sun.star.text.ContentControl"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    xContentControlProps->setPropertyValue("Alias", uno::Any(OUString("my alias")));
    xContentControlProps->setPropertyValue("Tag", uno::Any(OUString("my tag")));
    xContentControlProps->setPropertyValue("Id", uno::Any(static_cast<sal_Int32>(-2147483648)));
    xContentControlProps->setPropertyValue("TabIndex", uno::Any(sal_uInt32(3)));
    xContentControlProps->setPropertyValue("Lock", uno::Any(OUString("unlocked")));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When exporting to ODT:
    save("writer8");

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expression: prop
    // - XPath '//loext:content-control' no attribute 'alias' exist
    // i.e. alias was lost on export.
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "alias"_ostr, "my alias");
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "tag"_ostr, "my tag");
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "id"_ostr, "-2147483648");
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "tab-index"_ostr, "3");
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "lock"_ostr, "unlocked");
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testComboBoxContentControlImport)
{
    // Given an ODF document with a plain-text content control:
    loadFromFile(u"content-control-combo-box.fodt");

    // Then make sure that the content control is not lost on import:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
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
    bool bComboBox{};
    xContentControlProps->getPropertyValue("ComboBox") >>= bComboBox;
    // Without the accompanying fix in place, this test would have failed, the import result was a
    // drop-down content control (not a combo box one).
    CPPUNIT_ASSERT(bComboBox);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testAliasContentControlImport)
{
    // Given an ODF document with a content control and its alias/tag:
    loadFromFile(u"content-control-alias.fodt");

    // Then make sure that the content control is not lost on import:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
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
    OUString aAlias;
    xContentControlProps->getPropertyValue("Alias") >>= aAlias;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: my alias
    // - Actual  :
    // i.e. the alias was lost on import.
    CPPUNIT_ASSERT_EQUAL(OUString("my alias"), aAlias);
    OUString aTag;
    xContentControlProps->getPropertyValue("Tag") >>= aTag;
    CPPUNIT_ASSERT_EQUAL(OUString("my tag"), aTag);
    sal_Int32 nId = 0;
    xContentControlProps->getPropertyValue("Id") >>= nId;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2147483647), nId);
    sal_uInt32 nTabIndex;
    xContentControlProps->getPropertyValue("TabIndex") >>= nTabIndex;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(4), nTabIndex);
    OUString aLock;
    xContentControlProps->getPropertyValue("Lock") >>= aLock;
    CPPUNIT_ASSERT_EQUAL(OUString("sdtContentLocked"), aLock);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testDropdownContentControlAutostyleExport)
{
    // Given a document with a dropdown content control, and formatting that forms an autostyle in
    // ODT:
    loadFromFile(u"content-control-dropdown.docx");

    // When saving that document to ODT, then make sure no assertion failure happens:
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProps = comphelper::InitPropertySequence({
        { "FilterName", uno::Any(OUString("writer8")) },
    });
    // Without the accompanying fix in place, this test would have failed, we had duplicated XML
    // attributes.
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProps);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testScaleWidthRedline)
{
    // Given a document with change tracking enabled, one image is part of a delete redline:
    loadFromFile(u"scale-width-redline.fodt");
    dispatchCommand(mxComponent, ".uno:TrackChanges", {});
    dispatchCommand(mxComponent, ".uno:GoToEndOfLine", {});
    dispatchCommand(mxComponent, ".uno:EndOfParaSel", {});
    dispatchCommand(mxComponent, ".uno:Delete", {});

    // When saving to ODT:
    save("writer8");

    // Then make sure that a non-zero size is written to the output:
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 6.1728in
    // - Actual  : 0in
    // i.e. the deleted image had zero size, which is incorrect.
    assertXPath(pXmlDoc, "//draw:frame[@draw:name='Image45']"_ostr, "width"_ostr, "6.1728in");
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testThemeExport)
{
    mxComponent = loadFromDesktop("private:factory/swriter");

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xPageProps(xDrawPage, uno::UNO_QUERY);

    auto pTheme = std::make_shared<model::Theme>("My Theme");
    auto pColorSet = std::make_shared<model::ColorSet>("My Color Scheme");
    pColorSet->add(model::ThemeColorType::Dark1, 0x101010);
    pColorSet->add(model::ThemeColorType::Light1, 0x202020);
    pColorSet->add(model::ThemeColorType::Dark2, 0x303030);
    pColorSet->add(model::ThemeColorType::Light2, 0x404040);
    pColorSet->add(model::ThemeColorType::Accent1, 0x505050);
    pColorSet->add(model::ThemeColorType::Accent2, 0x606060);
    pColorSet->add(model::ThemeColorType::Accent3, 0x707070);
    pColorSet->add(model::ThemeColorType::Accent4, 0x808080);
    pColorSet->add(model::ThemeColorType::Accent5, 0x909090);
    pColorSet->add(model::ThemeColorType::Accent6, 0xa0a0a0);
    pColorSet->add(model::ThemeColorType::Hyperlink, 0xb0b0b0);
    pColorSet->add(model::ThemeColorType::FollowedHyperlink, 0xc0c0c0);
    pTheme->setColorSet(pColorSet);

    uno::Reference<util::XTheme> xTheme = model::theme::createXTheme(pTheme);
    xPageProps->setPropertyValue("Theme", uno::Any(xTheme));

    // Export to ODT:
    save("writer8");

    // Check if the 12 colors are written in the XML:
    xmlDocUniquePtr pXmlDoc = parseExport("styles.xml");
    OString aThemePath = "//office:styles/loext:theme/loext:theme-colors/loext:color"_ostr;
    assertXPath(pXmlDoc, aThemePath, 12);
    assertXPath(pXmlDoc, aThemePath + "[1]", "name"_ostr, "dark1");
    assertXPath(pXmlDoc, aThemePath + "[1]", "color"_ostr, "#101010");
    assertXPath(pXmlDoc, aThemePath + "[2]", "name"_ostr, "light1");
    assertXPath(pXmlDoc, aThemePath + "[2]", "color"_ostr, "#202020");
    assertXPath(pXmlDoc, aThemePath + "[12]", "name"_ostr, "followed-hyperlink");
    assertXPath(pXmlDoc, aThemePath + "[12]", "color"_ostr, "#c0c0c0");
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testFloatingTableExport)
{
    // Given a document with a floating table:
    mxComponent = loadFromDesktop("private:factory/swriter");
    // Insert a table:
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("Rows", static_cast<sal_Int32>(1)),
        comphelper::makePropertyValue("Columns", static_cast<sal_Int32>(1)),
    };
    dispatchCommand(mxComponent, ".uno:InsertTable", aArgs);
    // Select it:
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    // Wrap in a fly:
    aArgs = {
        comphelper::makePropertyValue("AnchorType", static_cast<sal_uInt16>(0)),
    };
    dispatchCommand(mxComponent, ".uno:InsertFrame", aArgs);
    // Mark it as a floating table:
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(
        xTextFramesSupplier->getTextFrames()->getByName("Frame1"), uno::UNO_QUERY);
    xFrame->setPropertyValue("IsSplitAllowed", uno::Any(true));

    // When saving to ODT:
    save("writer8");

    // Then make sure we write a floating table, not a textframe containing a table:
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '//draw:frame' no attribute 'may-break-between-pages' exist
    // i.e. no floating table was exported.
    assertXPath(pXmlDoc, "//draw:frame"_ostr, "may-break-between-pages"_ostr, "true");
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testFloatingTableImport)
{
    // Given a document with a floating table (loext:may-break-between-pages="true"), when importing
    // that document:
    loadFromFile(u"floattable.fodt");

    // Then make sure that the matching text frame property is set:
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(
        xTextFramesSupplier->getTextFrames()->getByName("Frame1"), uno::UNO_QUERY);
    bool bIsSplitAllowed = false;
    // Without the accompanying fix in place, this test would have failed, the property was false.
    xFrame->getPropertyValue("IsSplitAllowed") >>= bIsSplitAllowed;
    CPPUNIT_ASSERT(bIsSplitAllowed);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testParagraphScopedTabDistance)
{
    // Given a document with paragraph scoped default tab stop distance (loext:tab-stop-distance="0.5cm")
    loadFromFile(u"paragraph-tab-stop-distance.fodp");

    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);

    uno::Reference<beans::XPropertySet> xShape(xPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XText> xText
        = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY_THROW)->getText();

    uno::Reference<container::XEnumerationAccess> paraEnumAccess(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> paraEnum(paraEnumAccess->createEnumeration());
    uno::Reference<text::XTextRange> xParagraph(paraEnum->nextElement(), uno::UNO_QUERY_THROW);

    uno::Reference<container::XEnumerationAccess> runEnumAccess(xParagraph, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> runEnum = runEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xRun(runEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);

    // Make sure the tab stop default distance is imported correctly
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 10000
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(10000),
                         xPropSet->getPropertyValue("ParaTabStopDefaultDistance").get<sal_Int32>());

    // Save the imported file to test the export too
    save("impress8");

    // Then make sure we write the tab-stop-distance
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    assertXPath(pXmlDoc, "//style:style[@style:name='P1']/style:paragraph-properties"_ostr,
                "tab-stop-distance"_ostr, "10cm");

    assertXPath(pXmlDoc, "//text:p[@text:style-name='P1']"_ostr);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testNestedSpans)
{
    // Given a document with a first paragraph that has a nested span, the outer span setting the
    // boldness:
    // When importing that document:
    loadFromFile(u"nested-spans.odt");

    // Then make sure the text portion is bold, not normal:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParagraphsAccess(xTextDocument->getText(),
                                                                    uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphs = xParagraphsAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xParagraph(xParagraphs->nextElement(),
                                                             uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPortions = xParagraph->createEnumeration();
    uno::Reference<beans::XPropertySet> xTextPortion(xPortions->nextElement(), uno::UNO_QUERY);
    float fWeight{};
    xTextPortion->getPropertyValue("CharWeight") >>= fWeight;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 150 (awt::FontWeight::BOLD)
    // - Actual  : 100 (awt::FontWeight::NORMAL)
    // i.e. the boldness was lost on import.
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, fWeight);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
