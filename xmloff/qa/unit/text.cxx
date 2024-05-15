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
    : UnoApiXmlTest(u"/xmloff/qa/unit/data/"_ustr)
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
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);
    uno::Sequence<beans::PropertyValue> aCommentProps = comphelper::InitPropertySequence({
        { "Text", uno::Any(u"comment"_ustr) },
    });
    dispatchCommand(mxComponent, u".uno:InsertAnnotation"_ustr, aCommentProps);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPortionEnum = xPara->createEnumeration();
    uno::Reference<beans::XPropertySet> xPortion(xPortionEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xField(xPortion->getPropertyValue(u"TextField"_ustr),
                                               uno::UNO_QUERY);
    xField->setPropertyValue(u"Resolved"_ustr, uno::Any(true));
    xField->setPropertyValue(u"ParentName"_ustr, uno::Any(u"parent_comment_name"_ustr));

    saveAndReload(u"writer8"_ustr);
    xTextDocument.set(mxComponent, uno::UNO_QUERY);
    xParaEnumAccess.set(xTextDocument->getText(), uno::UNO_QUERY);
    xParaEnum = xParaEnumAccess->createEnumeration();
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPortionEnum = xPara->createEnumeration();
    xPortion.set(xPortionEnum->nextElement(), uno::UNO_QUERY);
    xField.set(xPortion->getPropertyValue(u"TextField"_ustr), uno::UNO_QUERY);
    bool bResolved = false;
    xField->getPropertyValue(u"Resolved"_ustr) >>= bResolved;
    OUString parentName;
    xField->getPropertyValue(u"ParentName"_ustr) >>= parentName;
    CPPUNIT_ASSERT_EQUAL(
        u"parent_comment_name"_ustr,
        parentName); // Check if the parent comment name is written and read correctly.
    // Without the accompanying fix in place, this test would have failed, as the resolved state was
    // not saved for non-range comments.
    CPPUNIT_ASSERT(bResolved);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testBibliographyLocalUrl)
{
    // Given a document with a biblio field, with non-empty LocalURL:
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xField(
        xFactory->createInstance(u"com.sun.star.text.TextField.Bibliography"_ustr), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aFields = {
        comphelper::makePropertyValue(u"BibiliographicType"_ustr, text::BibliographyDataType::WWW),
        comphelper::makePropertyValue(u"Identifier"_ustr, u"AT"_ustr),
        comphelper::makePropertyValue(u"Author"_ustr, u"Author"_ustr),
        comphelper::makePropertyValue(u"Title"_ustr, u"Title"_ustr),
        comphelper::makePropertyValue(u"URL"_ustr, u"http://www.example.com/test.pdf#page=1"_ustr),
        comphelper::makePropertyValue(u"LocalURL"_ustr, u"file:///home/me/test.pdf"_ustr),
    };
    xField->setPropertyValue(u"Fields"_ustr, uno::Any(aFields));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextContent> xContent(xField, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContent, /*bAbsorb=*/false);

    // When invoking ODT export + import on it:
    saveAndReload(u"writer8"_ustr);
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
    xField.set(xPortion->getPropertyValue(u"TextField"_ustr), uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aMap(xField->getPropertyValue(u"Fields"_ustr));
    CPPUNIT_ASSERT(aMap.contains(u"LocalURL"_ustr));
    auto aActual = aMap[u"LocalURL"_ustr].get<OUString>();
    CPPUNIT_ASSERT_EQUAL(u"file:///home/me/test.pdf"_ustr, aActual);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testBibliographyTargetURL1)
{
    // Given a document with a biblio field, with non-empty LocalURL:
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xField(
        xFactory->createInstance(u"com.sun.star.text.TextField.Bibliography"_ustr), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aFields = {
        comphelper::makePropertyValue(u"Identifier"_ustr, u"AT"_ustr),
        comphelper::makePropertyValue(u"URL"_ustr, u"https://display.url/test1.pdf#page=1"_ustr),
        comphelper::makePropertyValue(u"TargetType"_ustr, u"1"_ustr),
        comphelper::makePropertyValue(u"TargetURL"_ustr,
                                      u"https://target.url/test2.pdf#page=2"_ustr),
    };
    xField->setPropertyValue(u"Fields"_ustr, uno::Any(aFields));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextContent> xContent(xField, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContent, /*bAbsorb=*/false);

    // When invoking ODT export + import on it:
    saveAndReload(u"writer8"_ustr);

    // Then make sure that URL, TargetURL and UseTargetURL are preserved and independent:
    xTextDocument.set(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPortionEnum = xPara->createEnumeration();
    uno::Reference<beans::XPropertySet> xPortion(xPortionEnum->nextElement(), uno::UNO_QUERY);
    xField.set(xPortion->getPropertyValue(u"TextField"_ustr), uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aMap(xField->getPropertyValue(u"Fields"_ustr));

    CPPUNIT_ASSERT(aMap.contains(u"URL"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"https://display.url/test1.pdf#page=1"_ustr,
                         aMap[u"URL"_ustr].get<OUString>());

    CPPUNIT_ASSERT(aMap.contains(u"TargetURL"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"https://target.url/test2.pdf#page=2"_ustr,
                         aMap[u"TargetURL"_ustr].get<OUString>());

    CPPUNIT_ASSERT(aMap.contains(u"TargetType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"1"_ustr, aMap[u"TargetType"_ustr].get<OUString>());
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
        xStyleFamilies->getByName(u"ParagraphStyles"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStyle(xStyleFamily->getByName(u"mystyle"_ustr),
                                               uno::UNO_QUERY);
    sal_Int16 nNumberingLevel{};
    CPPUNIT_ASSERT(xStyle->getPropertyValue(u"NumberingLevel"_ustr) >>= nNumberingLevel);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1), nNumberingLevel);

    // Test the export as well:
    save(u"writer8"_ustr);

    // Then make sure we save the style's numbering level:
    xmlDocUniquePtr pXmlDoc = parseExport(u"styles.xml"_ustr);
    // Without the accompanying fix in place, this failed with:
    // - XPath '/office:document-styles/office:styles/style:style[@style:name='mystyle']' no attribute 'list-level' exist
    // i.e. a custom NumberingLevel was lost on save.
    assertXPath(pXmlDoc,
                "/office:document-styles/office:styles/style:style[@style:name='mystyle']"_ostr,
                "list-level"_ostr, u"2"_ustr);
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
    auto aActual = xPara->getPropertyValue(u"ListLabelString"_ustr).get<OUString>();
    // Without the accompanying fix in place, this failed with:
    // - Expected: 2.
    // - Actual  : 1.
    // i.e. the numbering was not continued, like in Word.
    CPPUNIT_ASSERT_EQUAL(u"2."_ustr, aActual);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testListId)
{
    // Given a document with a simple list (no continue-list="..." attribute):
    loadFromFile(u"list-id.fodt");

    // When storing that document as ODF:
    save(u"writer8"_ustr);

    // Then make sure that unreferenced xml:id="..." attributes are not written:
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
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
    auto aActual(xPara->getPropertyValue(u"ListLabelString"_ustr).get<OUString>());
    CPPUNIT_ASSERT_EQUAL(u"1."_ustr, aActual);
    xParaEnum->nextElement(); // Skip empty intermediate paragraph
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    aActual = xPara->getPropertyValue(u"ListLabelString"_ustr).get<OUString>();
    CPPUNIT_ASSERT_EQUAL(u"2."_ustr, aActual);
    xParaEnum->nextElement(); // Skip empty intermediate paragraph
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    aActual = xPara->getPropertyValue(u"ListLabelString"_ustr).get<OUString>();
    CPPUNIT_ASSERT_EQUAL(u"3."_ustr, aActual);
    xParaEnum->nextElement(); // Skip empty intermediate paragraph
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    aActual = xPara->getPropertyValue(u"ListLabelString"_ustr).get<OUString>();
    CPPUNIT_ASSERT_EQUAL(u"4."_ustr, aActual);

    // When storing that document as ODF:
    // Without the fix in place, automatic validation would fail with:
    // Error: "list123456789012345" is referenced by an IDREF, but not defined.
    saveAndReload(u"writer8"_ustr);

    xTextDocument.set(mxComponent.queryThrow<css::text::XTextDocument>());
    xParaEnumAccess.set(xTextDocument->getText().queryThrow<css::container::XEnumerationAccess>());
    xParaEnum.set(xParaEnumAccess->createEnumeration());

    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    aActual = xPara->getPropertyValue(u"ListLabelString"_ustr).get<OUString>();
    CPPUNIT_ASSERT_EQUAL(u"1."_ustr, aActual);
    xParaEnum->nextElement(); // Skip empty intermediate paragraph
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    aActual = xPara->getPropertyValue(u"ListLabelString"_ustr).get<OUString>();
    CPPUNIT_ASSERT_EQUAL(u"2."_ustr, aActual);
    xParaEnum->nextElement(); // Skip empty intermediate paragraph
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    aActual = xPara->getPropertyValue(u"ListLabelString"_ustr).get<OUString>();
    CPPUNIT_ASSERT_EQUAL(u"3."_ustr, aActual);
    xParaEnum->nextElement(); // Skip empty intermediate paragraph

    // Check that the last item number is correct

    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    aActual = xPara->getPropertyValue(u"ListLabelString"_ustr).get<OUString>();
    // Without the fix in place, this would fail with:
    // - Expected: 4.
    // - Actual  : 1.
    // i.e. the numbering was not continued.
    CPPUNIT_ASSERT_EQUAL(u"4."_ustr, aActual);

    // Then make sure that required xml:id="..." attributes is written when the style changes:
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
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
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);
    auto xTextDocument(mxComponent.queryThrow<text::XTextDocument>());
    auto xText(xTextDocument->getText());
    xText->insertControlCharacter(xText->getEnd(), css::text::ControlCharacter::PARAGRAPH_BREAK,
                                  false);
    xText->insertControlCharacter(xText->getEnd(), css::text::ControlCharacter::PARAGRAPH_BREAK,
                                  false);

    auto paraEnumAccess(xText.queryThrow<container::XEnumerationAccess>());
    auto paraEnum(paraEnumAccess->createEnumeration());
    auto xParaProps(paraEnum->nextElement().queryThrow<beans::XPropertySet>());
    xParaProps->setPropertyValue(u"NumberingStyleName"_ustr, css::uno::Any(u"Numbering ABC"_ustr));
    xParaProps.set(paraEnum->nextElement().queryThrow<beans::XPropertySet>());
    xParaProps->setPropertyValue(u"NumberingStyleName"_ustr, css::uno::Any(u"Numbering 123"_ustr));
    xParaProps.set(paraEnum->nextElement().queryThrow<beans::XPropertySet>());
    xParaProps->setPropertyValue(u"NumberingStyleName"_ustr, css::uno::Any(u"Numbering ABC"_ustr));

    // When storing that document as ODF:
    save(u"writer8"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);

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
    auto aActual(xPara->getPropertyValue(u"ListLabelString"_ustr).get<OUString>());
    CPPUNIT_ASSERT_EQUAL(u"1."_ustr, aActual);
    OUString list_id = xPara->getPropertyValue(u"ListId"_ustr).get<OUString>();
    xParaEnum->nextElement(); // Skip empty intermediate paragraph
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY_THROW);
    aActual = xPara->getPropertyValue(u"ListLabelString"_ustr).get<OUString>();
    CPPUNIT_ASSERT_EQUAL(u"2."_ustr, aActual);
    CPPUNIT_ASSERT_EQUAL(list_id, xPara->getPropertyValue(u"ListId"_ustr).get<OUString>());
    xParaEnum->nextElement(); // Skip empty intermediate paragraph
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    aActual = xPara->getPropertyValue(u"ListLabelString"_ustr).get<OUString>();
    // Check that restart was applied correctly, with simple 'text:continue-numbering="true"'
    CPPUNIT_ASSERT_EQUAL(u"1."_ustr, aActual);
    CPPUNIT_ASSERT_EQUAL(list_id, xPara->getPropertyValue(u"ListId"_ustr).get<OUString>());

    // When storing that document as ODF:
    saveAndReload(u"writer8"_ustr);

    xTextDocument.set(mxComponent, uno::UNO_QUERY_THROW);
    xParaEnumAccess.set(xTextDocument->getText(), uno::UNO_QUERY_THROW);
    xParaEnum.set(xParaEnumAccess->createEnumeration());

    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY_THROW);
    aActual = xPara->getPropertyValue(u"ListLabelString"_ustr).get<OUString>();
    CPPUNIT_ASSERT_EQUAL(u"1."_ustr, aActual);
    list_id = xPara->getPropertyValue(u"ListId"_ustr).get<OUString>();
    xParaEnum->nextElement(); // Skip empty intermediate paragraph
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY_THROW);
    aActual = xPara->getPropertyValue(u"ListLabelString"_ustr).get<OUString>();
    CPPUNIT_ASSERT_EQUAL(u"2."_ustr, aActual);
    CPPUNIT_ASSERT_EQUAL(list_id, xPara->getPropertyValue(u"ListId"_ustr).get<OUString>());
    xParaEnum->nextElement(); // Skip empty intermediate paragraph
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY_THROW);
    aActual = xPara->getPropertyValue(u"ListLabelString"_ustr).get<OUString>();
    CPPUNIT_ASSERT_EQUAL(u"1."_ustr, aActual);
    CPPUNIT_ASSERT_EQUAL(list_id, xPara->getPropertyValue(u"ListId"_ustr).get<OUString>());

    // Then make sure that no xml:id="..." attribute is written, even in restarted case:
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "//text:list"_ostr, 3);
    assertXPathNoAttribute(pXmlDoc, "//text:list[1]"_ostr, "id"_ostr);
    assertXPathNoAttribute(pXmlDoc, "//text:list[2]"_ostr, "id"_ostr);
    assertXPathNoAttribute(pXmlDoc, "//text:list[3]"_ostr, "id"_ostr);
    assertXPathNoAttribute(pXmlDoc, "//text:list[3]"_ostr, "continue-list"_ostr);
    assertXPath(pXmlDoc, "//text:list[3]"_ostr, "continue-numbering"_ostr, u"true"_ustr);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testClearingBreakExport)
{
    // Given a document with a clearing break:
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xLineBreak(
        xMSF->createInstance(u"com.sun.star.text.LineBreak"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xLineBreakProps(xLineBreak, uno::UNO_QUERY);
    // SwLineBreakClear::ALL;
    sal_Int16 eClear = 3;
    xLineBreakProps->setPropertyValue(u"Clear"_ustr, uno::Any(eClear));
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertTextContent(xCursor, xLineBreak, /*bAbsorb=*/false);

    // When exporting to ODT:
    save(u"writer8"_ustr);

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    // Without the accompanying fix in place, this failed with:
    // - XPath '//text:line-break' number of nodes is incorrect
    // i.e. the clearing break was lost on export.
    assertXPath(pXmlDoc, "//text:line-break"_ostr, "clear"_ostr, u"all"_ustr);
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
    xPortion->getPropertyValue(u"TextPortionType"_ustr) >>= aTextPortionType;
    CPPUNIT_ASSERT_EQUAL(u"LineBreak"_ustr, aTextPortionType);
    uno::Reference<text::XTextContent> xLineBreak;
    xPortion->getPropertyValue(u"LineBreak"_ustr) >>= xLineBreak;
    uno::Reference<beans::XPropertySet> xLineBreakProps(xLineBreak, uno::UNO_QUERY);
    sal_Int16 eClear{};
    xLineBreakProps->getPropertyValue(u"Clear"_ustr) >>= eClear;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(3), eClear);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testRelativeWidth)
{
    // Given a document with an 50% wide text frame:
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xStyleFamily(
        xStyleFamilies->getByName(u"PageStyles"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStyle(xStyleFamily->getByName(u"Standard"_ustr),
                                               uno::UNO_QUERY);
    // Body frame width is 6cm (2+2cm margin).
    xStyle->setPropertyValue(u"Width"_ustr, uno::Any(static_cast<sal_Int32>(10000)));
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xTextFrame(
        xMSF->createInstance(u"com.sun.star.text.TextFrame"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextFrameProps(xTextFrame, uno::UNO_QUERY);
    xTextFrameProps->setPropertyValue(u"RelativeWidth"_ustr, uno::Any(static_cast<sal_Int16>(50)));
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertTextContent(xCursor, xTextFrame, /*bAbsorb=*/false);
    // Body frame width is 16cm.
    xStyle->setPropertyValue(u"Width"_ustr, uno::Any(static_cast<sal_Int32>(20000)));

    save(u"writer8"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    // Without the accompanying fix in place, this failed with:
    // - Expected: 3.1492in (8cm)
    // - Actual  : 0.0161in (0.04 cm)
    // i.e. the fallback width value wasn't the expected half of the body frame width, but a smaller
    // value.
    assertXPath(pXmlDoc, "//draw:frame"_ostr, "width"_ostr, u"3.1492in"_ustr);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testScaleWidthAndHeight)
{
    // Given a broken document where both IsSyncHeightToWidth and IsSyncWidthToHeight are set to
    // true:
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xTextFrame(
        xMSF->createInstance(u"com.sun.star.text.TextFrame"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextFrameProps(xTextFrame, uno::UNO_QUERY);
    xTextFrameProps->setPropertyValue(u"Width"_ustr, uno::Any(static_cast<sal_Int16>(2000)));
    xTextFrameProps->setPropertyValue(u"Height"_ustr, uno::Any(static_cast<sal_Int16>(1000)));
    xTextFrameProps->setPropertyValue(u"IsSyncHeightToWidth"_ustr, uno::Any(true));
    xTextFrameProps->setPropertyValue(u"IsSyncWidthToHeight"_ustr, uno::Any(true));
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertTextContent(xCursor, xTextFrame, /*bAbsorb=*/false);

    // When exporting to ODT:
    save(u"writer8"_ustr);

    // Then make sure that we still export a non-zero size:
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    // Without the accompanying fix in place, this failed with:
    // - Expected: 0.7874in
    // - Actual  : 0in
    // i.e. the exported size was 0, not 2000 mm100 in inches.
    assertXPath(pXmlDoc, "//draw:frame"_ostr, "width"_ostr, u"0.7874in"_ustr);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testContentControlExport)
{
    // Given a document with a content control around one or more text portions:
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, u"test"_ustr, /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance(u"com.sun.star.text.ContentControl"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    xContentControlProps->setPropertyValue(u"ShowingPlaceHolder"_ustr, uno::Any(true));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When exporting to ODT:
    save(u"writer8"_ustr);

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    // Without the accompanying fix in place, this failed with:
    // - XPath '//loext:content-control' number of nodes is incorrect
    // i.e. the content control was lost on export.
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "showing-place-holder"_ostr, u"true"_ustr);
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
    xTextPortion->getPropertyValue(u"TextPortionType"_ustr) >>= aPortionType;
    // Without the accompanying fix in place, this failed with:
    // - Expected: ContentControl
    // - Actual  : Text
    // i.e. the content control was lost on import.
    CPPUNIT_ASSERT_EQUAL(u"ContentControl"_ustr, aPortionType);
    uno::Reference<text::XTextContent> xContentControl;
    xTextPortion->getPropertyValue(u"ContentControl"_ustr) >>= xContentControl;
    uno::Reference<text::XTextRange> xContentControlRange(xContentControl, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xContentControlRange->getText();
    uno::Reference<container::XEnumerationAccess> xContentEnumAccess(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xContentEnum = xContentEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xContent(xContentEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"test"_ustr, xContent->getString());
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testCheckboxContentControlExport)
{
    // Given a document with a checkbox content control around a text portion:
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, u"☐"_ustr, /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance(u"com.sun.star.text.ContentControl"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    xContentControlProps->setPropertyValue(u"Checkbox"_ustr, uno::Any(true));
    xContentControlProps->setPropertyValue(u"Checked"_ustr, uno::Any(true));
    xContentControlProps->setPropertyValue(u"CheckedState"_ustr, uno::Any(u"☒"_ustr));
    xContentControlProps->setPropertyValue(u"UncheckedState"_ustr, uno::Any(u"☐"_ustr));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When exporting to ODT:
    save(u"writer8"_ustr);

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "checkbox"_ostr, u"true"_ustr);
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "checked"_ostr, u"true"_ustr);
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
    xTextPortion->getPropertyValue(u"TextPortionType"_ustr) >>= aPortionType;
    CPPUNIT_ASSERT_EQUAL(u"ContentControl"_ustr, aPortionType);
    uno::Reference<text::XTextContent> xContentControl;
    xTextPortion->getPropertyValue(u"ContentControl"_ustr) >>= xContentControl;
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    bool bCheckbox{};
    xContentControlProps->getPropertyValue(u"Checkbox"_ustr) >>= bCheckbox;
    // Without the accompanying fix in place, this failed, as the checkbox-related attributes were
    // ignored on import.
    CPPUNIT_ASSERT(bCheckbox);
    bool bChecked{};
    xContentControlProps->getPropertyValue(u"Checked"_ustr) >>= bChecked;
    CPPUNIT_ASSERT(bChecked);
    OUString aCheckedState;
    xContentControlProps->getPropertyValue(u"CheckedState"_ustr) >>= aCheckedState;
    CPPUNIT_ASSERT_EQUAL(u"☒"_ustr, aCheckedState);
    OUString aUncheckedState;
    xContentControlProps->getPropertyValue(u"UncheckedState"_ustr) >>= aUncheckedState;
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
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, u"choose an item"_ustr, /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance(u"com.sun.star.text.ContentControl"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    {
        xContentControlProps->setPropertyValue(u"DropDown"_ustr, uno::Any(true));
        uno::Sequence<beans::PropertyValues> aListItems = {
            {
                comphelper::makePropertyValue(u"DisplayText"_ustr, uno::Any(u"red"_ustr)),
                comphelper::makePropertyValue(u"Value"_ustr, uno::Any(u"R"_ustr)),
            },
            {
                comphelper::makePropertyValue(u"DisplayText"_ustr, uno::Any(u"green"_ustr)),
                comphelper::makePropertyValue(u"Value"_ustr, uno::Any(u"G"_ustr)),
            },
            {
                comphelper::makePropertyValue(u"DisplayText"_ustr, uno::Any(u"blue"_ustr)),
                comphelper::makePropertyValue(u"Value"_ustr, uno::Any(u"B"_ustr)),
            },
        };
        xContentControlProps->setPropertyValue(u"ListItems"_ustr, uno::Any(aListItems));
    }
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When exporting to ODT:
    save(u"writer8"_ustr);

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "dropdown"_ostr, u"true"_ustr);
    // Without the accompanying fix in place, this failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//loext:content-control/loext:list-item[1]' number of nodes is incorrect
    // i.e. the list items were lost on export.
    assertXPath(pXmlDoc, "//loext:content-control/loext:list-item[1]"_ostr, "display-text"_ostr,
                u"red"_ustr);
    assertXPath(pXmlDoc, "//loext:content-control/loext:list-item[1]"_ostr, "value"_ostr,
                u"R"_ustr);
    assertXPath(pXmlDoc, "//loext:content-control/loext:list-item[2]"_ostr, "display-text"_ostr,
                u"green"_ustr);
    assertXPath(pXmlDoc, "//loext:content-control/loext:list-item[2]"_ostr, "value"_ostr,
                u"G"_ustr);
    assertXPath(pXmlDoc, "//loext:content-control/loext:list-item[3]"_ostr, "display-text"_ostr,
                u"blue"_ustr);
    assertXPath(pXmlDoc, "//loext:content-control/loext:list-item[3]"_ostr, "value"_ostr,
                u"B"_ustr);
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
    xTextPortion->getPropertyValue(u"TextPortionType"_ustr) >>= aPortionType;
    CPPUNIT_ASSERT_EQUAL(u"ContentControl"_ustr, aPortionType);
    uno::Reference<text::XTextContent> xContentControl;
    xTextPortion->getPropertyValue(u"ContentControl"_ustr) >>= xContentControl;
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValues> aListItems;
    xContentControlProps->getPropertyValue(u"ListItems"_ustr) >>= aListItems;
    // Without the accompanying fix in place, this failed with:
    // - Expected: 3
    // - Actual  : 0
    // i.e. the list items were lost on import.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), aListItems.getLength());
    comphelper::SequenceAsHashMap aMap0(aListItems[0]);
    CPPUNIT_ASSERT_EQUAL(u"red"_ustr, aMap0[u"DisplayText"_ustr].get<OUString>());
    CPPUNIT_ASSERT_EQUAL(u"R"_ustr, aMap0[u"Value"_ustr].get<OUString>());
    comphelper::SequenceAsHashMap aMap1(aListItems[1]);
    CPPUNIT_ASSERT_EQUAL(u"green"_ustr, aMap1[u"DisplayText"_ustr].get<OUString>());
    CPPUNIT_ASSERT_EQUAL(u"G"_ustr, aMap1[u"Value"_ustr].get<OUString>());
    comphelper::SequenceAsHashMap aMap2(aListItems[2]);
    CPPUNIT_ASSERT_EQUAL(u"blue"_ustr, aMap2[u"DisplayText"_ustr].get<OUString>());
    CPPUNIT_ASSERT_EQUAL(u"B"_ustr, aMap2[u"Value"_ustr].get<OUString>());
    uno::Reference<text::XTextRange> xContentControlRange(xContentControl, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xContentControlRange->getText();
    uno::Reference<container::XEnumerationAccess> xContentEnumAccess(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xContentEnum = xContentEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xContent(xContentEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"choose a color"_ustr, xContent->getString());
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testPictureContentControlExport)
{
    // Given a document with a picture content control around an as-char image:
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<beans::XPropertySet> xTextGraphic(
        xMSF->createInstance(u"com.sun.star.text.TextGraphicObject"_ustr), uno::UNO_QUERY);
    xTextGraphic->setPropertyValue(u"AnchorType"_ustr,
                                   uno::Any(text::TextContentAnchorType_AS_CHARACTER));
    uno::Reference<text::XTextContent> xTextContent(xTextGraphic, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xTextContent, false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance(u"com.sun.star.text.ContentControl"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    xContentControlProps->setPropertyValue(u"Picture"_ustr, uno::Any(true));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When exporting to ODT:
    save(u"writer8"_ustr);

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '//loext:content-control' no attribute 'picture' exist
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "picture"_ostr, u"true"_ustr);
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
    xTextPortion->getPropertyValue(u"TextPortionType"_ustr) >>= aPortionType;
    CPPUNIT_ASSERT_EQUAL(u"ContentControl"_ustr, aPortionType);
    uno::Reference<text::XTextContent> xContentControl;
    xTextPortion->getPropertyValue(u"ContentControl"_ustr) >>= xContentControl;
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    bool bPicture{};
    xContentControlProps->getPropertyValue(u"Picture"_ustr) >>= bPicture;
    // Without the accompanying fix in place, this failed, as the picture attribute was ignored on
    // import.
    CPPUNIT_ASSERT(bPicture);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testDateContentControlExport)
{
    // Given a document with a date content control around a text portion:
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, u"choose a date"_ustr, /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance(u"com.sun.star.text.ContentControl"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    xContentControlProps->setPropertyValue(u"Date"_ustr, uno::Any(true));
    xContentControlProps->setPropertyValue(u"DateFormat"_ustr, uno::Any(u"YYYY-MM-DD"_ustr));
    xContentControlProps->setPropertyValue(u"DateLanguage"_ustr, uno::Any(u"en-US"_ustr));
    xContentControlProps->setPropertyValue(u"CurrentDate"_ustr,
                                           uno::Any(u"2022-05-25T00:00:00Z"_ustr));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When exporting to ODT:
    save(u"writer8"_ustr);

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '//loext:content-control' no attribute 'date' exist
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "date"_ostr, u"true"_ustr);
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "date-format"_ostr, u"YYYY-MM-DD"_ustr);
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "date-rfc-language-tag"_ostr,
                u"en-US"_ustr);
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "current-date"_ostr,
                u"2022-05-25T00:00:00Z"_ustr);
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
    xTextPortion->getPropertyValue(u"TextPortionType"_ustr) >>= aPortionType;
    CPPUNIT_ASSERT_EQUAL(u"ContentControl"_ustr, aPortionType);
    uno::Reference<text::XTextContent> xContentControl;
    xTextPortion->getPropertyValue(u"ContentControl"_ustr) >>= xContentControl;
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    bool bDate{};
    xContentControlProps->getPropertyValue(u"Date"_ustr) >>= bDate;
    // Without the accompanying fix in place, this test would have failed, the content control was
    // imported as a default rich text one.
    CPPUNIT_ASSERT(bDate);
    OUString aDateFormat;
    xContentControlProps->getPropertyValue(u"DateFormat"_ustr) >>= aDateFormat;
    CPPUNIT_ASSERT_EQUAL(u"YYYY-MM-DD"_ustr, aDateFormat);
    OUString aDateLanguage;
    xContentControlProps->getPropertyValue(u"DateLanguage"_ustr) >>= aDateLanguage;
    CPPUNIT_ASSERT_EQUAL(u"en-US"_ustr, aDateLanguage);
    OUString aCurrentDate;
    xContentControlProps->getPropertyValue(u"CurrentDate"_ustr) >>= aCurrentDate;
    CPPUNIT_ASSERT_EQUAL(u"2022-05-25T00:00:00Z"_ustr, aCurrentDate);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testPlainTextContentControlExport)
{
    // Given a document with a plain text content control around a text portion:
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, u"test"_ustr, /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance(u"com.sun.star.text.ContentControl"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    xContentControlProps->setPropertyValue(u"PlainText"_ustr, uno::Any(true));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When exporting to ODT:
    save(u"writer8"_ustr);

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '//loext:content-control' no attribute 'plain-text' exist
    // i.e. the plain text content control was turned into a rich text one on export.
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "plain-text"_ostr, u"true"_ustr);
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
    xTextPortion->getPropertyValue(u"TextPortionType"_ustr) >>= aPortionType;
    CPPUNIT_ASSERT_EQUAL(u"ContentControl"_ustr, aPortionType);
    uno::Reference<text::XTextContent> xContentControl;
    xTextPortion->getPropertyValue(u"ContentControl"_ustr) >>= xContentControl;
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    bool bPlainText{};
    xContentControlProps->getPropertyValue(u"PlainText"_ustr) >>= bPlainText;
    // Without the accompanying fix in place, this test would have failed, the import result was a
    // rich text content control (not a plain text one).
    CPPUNIT_ASSERT(bPlainText);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testComboBoxContentControlExport)
{
    // Given a document with a combo box content control around a text portion:
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, u"test"_ustr, /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance(u"com.sun.star.text.ContentControl"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    xContentControlProps->setPropertyValue(u"ComboBox"_ustr, uno::Any(true));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When exporting to ODT:
    save(u"writer8"_ustr);

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '//loext:content-control' no attribute 'combobox' exist
    // i.e. the combo box content control was turned into a drop-down one on export.
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "combobox"_ostr, u"true"_ustr);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testAliasContentControlExport)
{
    // Given a document with a content control and its alias around a text portion:
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, u"test"_ustr, /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance(u"com.sun.star.text.ContentControl"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    xContentControlProps->setPropertyValue(u"Alias"_ustr, uno::Any(u"my alias"_ustr));
    xContentControlProps->setPropertyValue(u"Tag"_ustr, uno::Any(u"my tag"_ustr));
    xContentControlProps->setPropertyValue(u"Id"_ustr,
                                           uno::Any(static_cast<sal_Int32>(-2147483648)));
    xContentControlProps->setPropertyValue(u"TabIndex"_ustr, uno::Any(sal_uInt32(3)));
    xContentControlProps->setPropertyValue(u"Lock"_ustr, uno::Any(u"unlocked"_ustr));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When exporting to ODT:
    save(u"writer8"_ustr);

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expression: prop
    // - XPath '//loext:content-control' no attribute 'alias' exist
    // i.e. alias was lost on export.
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "alias"_ostr, u"my alias"_ustr);
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "tag"_ostr, u"my tag"_ustr);
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "id"_ostr, u"-2147483648"_ustr);
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "tab-index"_ostr, u"3"_ustr);
    assertXPath(pXmlDoc, "//loext:content-control"_ostr, "lock"_ostr, u"unlocked"_ustr);
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
    xTextPortion->getPropertyValue(u"TextPortionType"_ustr) >>= aPortionType;
    CPPUNIT_ASSERT_EQUAL(u"ContentControl"_ustr, aPortionType);
    uno::Reference<text::XTextContent> xContentControl;
    xTextPortion->getPropertyValue(u"ContentControl"_ustr) >>= xContentControl;
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    bool bComboBox{};
    xContentControlProps->getPropertyValue(u"ComboBox"_ustr) >>= bComboBox;
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
    xTextPortion->getPropertyValue(u"TextPortionType"_ustr) >>= aPortionType;
    CPPUNIT_ASSERT_EQUAL(u"ContentControl"_ustr, aPortionType);
    uno::Reference<text::XTextContent> xContentControl;
    xTextPortion->getPropertyValue(u"ContentControl"_ustr) >>= xContentControl;
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    OUString aAlias;
    xContentControlProps->getPropertyValue(u"Alias"_ustr) >>= aAlias;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: my alias
    // - Actual  :
    // i.e. the alias was lost on import.
    CPPUNIT_ASSERT_EQUAL(u"my alias"_ustr, aAlias);
    OUString aTag;
    xContentControlProps->getPropertyValue(u"Tag"_ustr) >>= aTag;
    CPPUNIT_ASSERT_EQUAL(u"my tag"_ustr, aTag);
    sal_Int32 nId = 0;
    xContentControlProps->getPropertyValue(u"Id"_ustr) >>= nId;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2147483647), nId);
    sal_uInt32 nTabIndex;
    xContentControlProps->getPropertyValue(u"TabIndex"_ustr) >>= nTabIndex;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(4), nTabIndex);
    OUString aLock;
    xContentControlProps->getPropertyValue(u"Lock"_ustr) >>= aLock;
    CPPUNIT_ASSERT_EQUAL(u"sdtContentLocked"_ustr, aLock);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testDropdownContentControlAutostyleExport)
{
    // Given a document with a dropdown content control, and formatting that forms an autostyle in
    // ODT:
    loadFromFile(u"content-control-dropdown.docx");

    // When saving that document to ODT, then make sure no assertion failure happens:
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProps = comphelper::InitPropertySequence({
        { "FilterName", uno::Any(u"writer8"_ustr) },
    });
    // Without the accompanying fix in place, this test would have failed, we had duplicated XML
    // attributes.
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProps);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testScaleWidthRedline)
{
    // Given a document with change tracking enabled, one image is part of a delete redline:
    loadFromFile(u"scale-width-redline.fodt");
    dispatchCommand(mxComponent, u".uno:TrackChanges"_ustr, {});
    dispatchCommand(mxComponent, u".uno:GoToEndOfLine"_ustr, {});
    dispatchCommand(mxComponent, u".uno:EndOfParaSel"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});

    // When saving to ODT:
    save(u"writer8"_ustr);

    // Then make sure that a non-zero size is written to the output:
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 6.1728in
    // - Actual  : 0in
    // i.e. the deleted image had zero size, which is incorrect.
    assertXPath(pXmlDoc, "//draw:frame[@draw:name='Image45']"_ostr, "width"_ostr, u"6.1728in"_ustr);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testThemeExport)
{
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);

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
    xPageProps->setPropertyValue(u"Theme"_ustr, uno::Any(xTheme));

    // Export to ODT:
    save(u"writer8"_ustr);

    // Check if the 12 colors are written in the XML:
    xmlDocUniquePtr pXmlDoc = parseExport(u"styles.xml"_ustr);
    OString aThemePath = "//office:styles/loext:theme/loext:theme-colors/loext:color"_ostr;
    assertXPath(pXmlDoc, aThemePath, 12);
    assertXPath(pXmlDoc, aThemePath + "[1]", "name"_ostr, u"dark1"_ustr);
    assertXPath(pXmlDoc, aThemePath + "[1]", "color"_ostr, u"#101010"_ustr);
    assertXPath(pXmlDoc, aThemePath + "[2]", "name"_ostr, u"light1"_ustr);
    assertXPath(pXmlDoc, aThemePath + "[2]", "color"_ostr, u"#202020"_ustr);
    assertXPath(pXmlDoc, aThemePath + "[12]", "name"_ostr, u"followed-hyperlink"_ustr);
    assertXPath(pXmlDoc, aThemePath + "[12]", "color"_ostr, u"#c0c0c0"_ustr);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testFloatingTableExport)
{
    // Given a document with a floating table:
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);
    // Insert a table:
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"Rows"_ustr, static_cast<sal_Int32>(1)),
        comphelper::makePropertyValue(u"Columns"_ustr, static_cast<sal_Int32>(1)),
    };
    dispatchCommand(mxComponent, u".uno:InsertTable"_ustr, aArgs);
    // Select it:
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    // Wrap in a fly:
    aArgs = {
        comphelper::makePropertyValue(u"AnchorType"_ustr, static_cast<sal_uInt16>(0)),
    };
    dispatchCommand(mxComponent, u".uno:InsertFrame"_ustr, aArgs);
    // Mark it as a floating table:
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(
        xTextFramesSupplier->getTextFrames()->getByName(u"Frame1"_ustr), uno::UNO_QUERY);
    xFrame->setPropertyValue(u"IsSplitAllowed"_ustr, uno::Any(true));

    // When saving to ODT:
    save(u"writer8"_ustr);

    // Then make sure we write a floating table, not a textframe containing a table:
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '//draw:frame' no attribute 'may-break-between-pages' exist
    // i.e. no floating table was exported.
    assertXPath(pXmlDoc, "//draw:frame"_ostr, "may-break-between-pages"_ostr, u"true"_ustr);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testFloatingTableImport)
{
    // Given a document with a floating table (loext:may-break-between-pages="true"), when importing
    // that document:
    loadFromFile(u"floattable.fodt");

    // Then make sure that the matching text frame property is set:
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(
        xTextFramesSupplier->getTextFrames()->getByName(u"Frame1"_ustr), uno::UNO_QUERY);
    bool bIsSplitAllowed = false;
    // Without the accompanying fix in place, this test would have failed, the property was false.
    xFrame->getPropertyValue(u"IsSplitAllowed"_ustr) >>= bIsSplitAllowed;
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
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(10000),
        xPropSet->getPropertyValue(u"ParaTabStopDefaultDistance"_ustr).get<sal_Int32>());

    // Save the imported file to test the export too
    save(u"impress8"_ustr);

    // Then make sure we write the tab-stop-distance
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc, "//style:style[@style:name='P1']/style:paragraph-properties"_ostr,
                "tab-stop-distance"_ostr, u"10cm"_ustr);

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
    xTextPortion->getPropertyValue(u"CharWeight"_ustr) >>= fWeight;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 150 (awt::FontWeight::BOLD)
    // - Actual  : 100 (awt::FontWeight::NORMAL)
    // i.e. the boldness was lost on import.
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, fWeight);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
