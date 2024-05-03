/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <queue>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XBookmarksSupplier.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/TextGridMode.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontWeight.hpp>

#include <comphelper/scopeguard.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <officecfg/Office/Common.hxx>
#include <o3tl/string_view.hxx>
#include <comphelper/propertyvalue.hxx>

#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <rootfrm.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text") {}
};

DECLARE_OOXMLEXPORT_TEST(testTdf135164_cancelledNumbering, "tdf135164_cancelledNumbering.docx")
{
    uno::Reference<beans::XPropertySet> xPara(getParagraph(1, u"TBMM DÖNEMİ"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xPara, "ListLabelString"));

    xPara.set(getParagraph(2, "Numbering explicitly cancelled"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xPara, "ListLabelString"));

    xPara.set(getParagraph(6, "Default style has roman numbering"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("i"), getProperty<OUString>(xPara, "ListLabelString"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf147861_customField, "tdf147861_customField.docx")
{
    // These should each be specific values, not a shared DocProperty
    getParagraph(1, "CustomEditedTitle"); // edited
    // A couple of nulls at the end of the string thwarted all attempts at an "equals" comparison.
    CPPUNIT_ASSERT(getParagraph(2)->getString().startsWith(" INSERT Custom Title here"));
    getParagraph(3, "My Title"); // edited

    // Verify that these are fields, and not just plain text
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    auto xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<text::XTextField> xField(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("CustomEditedTitle"), xField->getPresentation(false));
    // The " (fixed)" part is unnecessary, but it must be consistent across a round-trip
    CPPUNIT_ASSERT_EQUAL(OUString("DocInformation:Title (fixed)"), xField->getPresentation(true));
}

DECLARE_OOXMLEXPORT_TEST(testTdf148380_createField, "tdf148380_createField.docx")
{
    // Verify that these are fields, and not just plain text
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    auto xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<text::XTextField> xField(xFields->nextElement(), uno::UNO_QUERY);
    // This should NOT be "Lorenzo Chavez", or a real date since the user hand-modified the result.
    CPPUNIT_ASSERT_EQUAL(OUString("Myself - that's who"), xField->getPresentation(false));
    xField.set(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("yesterday at noon"), xField->getPresentation(false));
}

DECLARE_OOXMLEXPORT_TEST(testTdf148380_fldLocked, "tdf148380_fldLocked.docx")
{
    getParagraph(2, "4/5/2022 4:29:00 PM");
    getParagraph(4, "1/23/4567 8:9:10 PM");

    // Verify that these are fields, and not just plain text
    // (import only, since export thankfully just dumps these fixed fields as plain text
    if (isExported())
        return;
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    auto xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<text::XTextField> xField(xFields->nextElement(), uno::UNO_QUERY);
    // This should NOT be updated at FILEOPEN to match the last modified time - it is locked.
    CPPUNIT_ASSERT_EQUAL(OUString("4/5/2022 4:29:00 PM"), xField->getPresentation(false));
    CPPUNIT_ASSERT_EQUAL(OUString("DocInformation:Modified (fixed)"), xField->getPresentation(true));
    xField.set(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("1/23/4567 8:9:10 PM"), xField->getPresentation(false));
    CPPUNIT_ASSERT_EQUAL(OUString("DocInformation:Last printed (fixed)"), xField->getPresentation(true));
}

DECLARE_OOXMLEXPORT_TEST(testTdf148380_usernameField, "tdf148380_usernameField.docx")
{
    // Verify that these are fields, and not just plain text
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    auto xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<text::XTextField> xField(xFields->nextElement(), uno::UNO_QUERY);
    // These should match the as-last-seen-in-the-text name, and not the application's user name
    CPPUNIT_ASSERT_EQUAL(OUString("Charlie Brown"), xField->getPresentation(false));
    xField.set(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("CB"), xField->getPresentation(false));
}

DECLARE_OOXMLEXPORT_TEST(testTdf148380_modifiedField, "tdf148380_modifiedField.docx")
{
    getParagraph(2, "4/5/2022 3:29:00 PM"); // default (unspecified) date format

    // Verify that these are fields, and not just plain text
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    auto xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<text::XTextField> xField(xFields->nextElement(), uno::UNO_QUERY);
    // unspecified SAVEDATE gets default US formatting because style.xml has w:lang w:val="en-US"
    CPPUNIT_ASSERT_EQUAL(OUString("4/5/2022 3:29:00 PM"), xField->getPresentation(false));
    xField.set(xFields->nextElement(), uno::UNO_QUERY);
    // This was hand-modified and really should be Charlie Brown, not Charles ...
    CPPUNIT_ASSERT_EQUAL(OUString("Charlie Brown"), xField->getPresentation(false));
}

DECLARE_OOXMLEXPORT_TEST(testTdf148380_printField, "tdf148380_printField.docx")
{
    // Verify that these are fields, and not just plain text
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    auto xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<text::XTextField> xField(xFields->nextElement(), uno::UNO_QUERY);
    // unspecified SAVEDATE gets default GB formatting because style.xml has w:lang w:val="en-GB"
    CPPUNIT_ASSERT_EQUAL(OUString("08/04/2022 07:10:00 AM"), xField->getPresentation(false));
    CPPUNIT_ASSERT_EQUAL(OUString("DocInformation:Modified"), xField->getPresentation(true));
    xField.set(xFields->nextElement(), uno::UNO_QUERY);
    // MS Word actually shows "8 o'clock-ish" until the document is reprinted,
    // but it seems best to actually show the real last-printed date since it can't be FIXEDFLD
    CPPUNIT_ASSERT_EQUAL(OUString("08/04/2022 06:47:00 AM"), xField->getPresentation(false));
    CPPUNIT_ASSERT_EQUAL(OUString("DocInformation:Last printed"), xField->getPresentation(true));
}

DECLARE_OOXMLEXPORT_TEST(testTdf132475_printField, "tdf132475_printField.docx")
{
    // The last printed date field: formatted two different ways
    getParagraph(2, "Thursday, March 17, 2022");
    getParagraph(3, "17-Mar-22");
    // Time zone affects the displayed time in MS Word. LO shows GMT time. Word only updated by F9
    getParagraph(5, "12:49");
    getParagraph(6, "12:49:00 PM");

    // Verify that these are fields, and not just plain text
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    auto xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<text::XTextField> xField(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Thursday, March 17, 2022"), xField->getPresentation(false));
    CPPUNIT_ASSERT_EQUAL(OUString("DocInformation:Last printed"), xField->getPresentation(true));
}

DECLARE_OOXMLEXPORT_TEST(testTdf114734_commentFormating, "tdf114734_commentFormating.docx")
{
    // Get the PostIt/Comment/Annotation
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    auto xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<text::XTextField> xField(xFields->nextElement(), uno::UNO_QUERY);

    uno::Reference<text::XText> xText = getProperty<uno::Reference<text::XText>>(xField, "TextRange");
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, xText);
    // Paragraph formatting was lost: should be right to left, and thus right-justified
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Right to Left comment",
                                 text::WritingMode2::RL_TB,
                                 getProperty<sal_Int16>(xParagraph, "WritingMode"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("literal right justified",
                                 sal_Int16(style::ParagraphAdjust_RIGHT),
                                 getProperty<sal_Int16>(xParagraph, "ParaAdjust"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf139759_commentHighlightBackground, "tdf139759_commentHighlightBackground.docx")
{
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    auto xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<text::XTextField> xField(xFields->nextElement(), uno::UNO_QUERY);

    uno::Reference<text::XText> xText = getProperty<uno::Reference<text::XText>>(xField, "TextRange");
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, xText);
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getProperty<Color>(getRun(xParagraph, 2), "CharBackColor"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf135906)
{
    loadAndReload("tdf135906.docx");
    // just test round-tripping. The document was exported as corrupt and didn't re-load.
}

DECLARE_OOXMLEXPORT_TEST(testTdf146802, "tdf146802.docx")
{
    // First check if the load failed, as before the fix.
    CPPUNIT_ASSERT(mxComponent);

    // There is a group shape with text box inside having an embedded VML formula,
    // check if something missing.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Where is the formula?", 2, getShapes());
    // Before the fix the bugdoc failed to load or the formula was missing.
}

CPPUNIT_TEST_FIXTURE(Test, testParaStyleNumLevel)
{
    loadAndSave("para-style-num-level.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/styles.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. a custom list level in a para style was lost on import+export.
    assertXPath(pXmlDoc, "/w:styles/w:style[@w:styleId='mystyle']/w:pPr/w:numPr/w:ilvl"_ostr, "val"_ostr, "1");
}

CPPUNIT_TEST_FIXTURE(Test, testClearingBreak)
{
    // Given a document with a clearing break, when saving to DOCX:
    loadAndSave("clearing-break.docx");

    // Then make sure that the clearing break is not lost:
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '/w:document/w:body/w:p/w:r/w:br' number of nodes is incorrect
    // i.e. first the clearing break was turned into a plain break, then it was completely lost.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:br"_ostr, "clear"_ostr, "all");
}

CPPUNIT_TEST_FIXTURE(Test, testContentControlExport)
{
    // Given a document with a content control around one or more text portions:
    createSwDoc();
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

    // When exporting to DOCX:
    save("Office Open XML Text");

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // XPath '//w:sdt/w:sdtPr/w:showingPlcHdr' number of nodes is incorrect
    // i.e. the SDT elements were missing on export.
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:showingPlcHdr"_ostr, 1);
    assertXPath(pXmlDoc, "//w:sdt/w:sdtContent"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testCheckboxContentControlExport)
{
    // Given a document with a checkbox content control around a text portion:
    createSwDoc();
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

    // When exporting to DOCX:
    save("Office Open XML Text");

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Without the fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//w:sdt/w:sdtPr/w14:checkbox/w14:checked' number of nodes is incorrect
    // i.e. <w14:checkbox> and its child elements were lost.
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w14:checkbox/w14:checked"_ostr, "val"_ostr, "1");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w14:checkbox/w14:checkedState"_ostr, "val"_ostr, "2612");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w14:checkbox/w14:uncheckedState"_ostr, "val"_ostr, "2610");
}

CPPUNIT_TEST_FIXTURE(Test, testDropdownContentControlExport)
{
    // Given a document with a dropdown content control around a text portion:
    createSwDoc();
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

    // When exporting to DOCX:
    save("Office Open XML Text");

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Without the fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//w:sdt/w:sdtPr/w:dropDownList/w:listItem[1]' number of nodes is incorrect
    // i.e. the list items were lost on export.
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:dropDownList/w:listItem[1]"_ostr, "displayText"_ostr, "red");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:dropDownList/w:listItem[1]"_ostr, "value"_ostr, "R");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:dropDownList/w:listItem[2]"_ostr, "displayText"_ostr, "green");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:dropDownList/w:listItem[2]"_ostr, "value"_ostr, "G");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:dropDownList/w:listItem[3]"_ostr, "displayText"_ostr, "blue");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:dropDownList/w:listItem[3]"_ostr, "value"_ostr, "B");
}

CPPUNIT_TEST_FIXTURE(Test, testPictureContentControlExport)
{
    // Given a document with a picture content control around a text portion:
    createSwDoc();
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

    // When exporting to DOCX:
    save("Office Open XML Text");

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Without the fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. <w:picture> was lost on export.
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:picture"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testDateContentControlExport)
{
    // Given a document with a date content control around a text portion:
    createSwDoc();
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
    xContentControlProps->setPropertyValue("Date", uno::Any(true));
    xContentControlProps->setPropertyValue("DateFormat", uno::Any(OUString("M/d/yyyy")));
    xContentControlProps->setPropertyValue("DateLanguage", uno::Any(OUString("en-US")));
    xContentControlProps->setPropertyValue("CurrentDate", uno::Any(OUString("2022-05-26T00:00:00Z")));
    xContentControlProps->setPropertyValue("PlaceholderDocPart", uno::Any(OUString("DefaultPlaceholder_-1854013437")));
    xContentControlProps->setPropertyValue("DataBindingPrefixMappings", uno::Any(OUString("xmlns:ns0='http://schemas.microsoft.com/vsto/samples' ")));
    xContentControlProps->setPropertyValue("DataBindingXpath", uno::Any(OUString("/ns0:employees[1]/ns0:employee[1]/ns0:hireDate[1]")));
    xContentControlProps->setPropertyValue("DataBindingStoreItemID", uno::Any(OUString("{241A8A02-7FFD-488D-8827-63FBE74E8BC9}")));
    xContentControlProps->setPropertyValue("Color", uno::Any(OUString("008000")));
    xContentControlProps->setPropertyValue("Appearance", uno::Any(OUString("hidden")));
    xContentControlProps->setPropertyValue("Alias", uno::Any(OUString("myalias")));
    xContentControlProps->setPropertyValue("Tag", uno::Any(OUString("mytag")));
    xContentControlProps->setPropertyValue("Id", uno::Any(static_cast<sal_Int32>(123)));
    xContentControlProps->setPropertyValue("TabIndex", uno::Any(sal_uInt32(4294967295))); // -1
    xContentControlProps->setPropertyValue("Lock", uno::Any(OUString("sdtLocked")));

    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When exporting to DOCX:
    save("Office Open XML Text");

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Without the fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//w:sdt/w:sdtPr/w:date/w:dateFormat' number of nodes is incorrect
    // i.e. the <w:date> was lost on export.
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:date/w:dateFormat"_ostr, "val"_ostr, "M/d/yyyy");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:date/w:lid"_ostr, "val"_ostr, "en-US");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:date"_ostr, "fullDate"_ostr, "2022-05-26T00:00:00Z");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:placeholder/w:docPart"_ostr, "val"_ostr, "DefaultPlaceholder_-1854013437");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:dataBinding"_ostr, "prefixMappings"_ostr, "xmlns:ns0='http://schemas.microsoft.com/vsto/samples' ");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:dataBinding"_ostr, "xpath"_ostr, "/ns0:employees[1]/ns0:employee[1]/ns0:hireDate[1]");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:dataBinding"_ostr, "storeItemID"_ostr, "{241A8A02-7FFD-488D-8827-63FBE74E8BC9}");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w15:color"_ostr, "val"_ostr, "008000");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w15:appearance"_ostr, "val"_ostr, "hidden");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:alias"_ostr, "val"_ostr, "myalias");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:tag"_ostr, "val"_ostr, "mytag");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:id"_ostr, "val"_ostr, "123");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:tabIndex"_ostr, "val"_ostr, "-1");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:lock"_ostr, "val"_ostr, "sdtLocked");
}

CPPUNIT_TEST_FIXTURE(Test, testNegativePageBorder)
{
    // Given a document with a negative border distance:
    createSwDoc();
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDocShell* pDocShell = pTextDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert("test");
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"),
                                                   uno::UNO_QUERY);
    xPageStyle->setPropertyValue("TopMargin", uno::Any(static_cast<sal_Int32>(501)));
    table::BorderLine2 aBorder;
    aBorder.LineWidth = 159;
    aBorder.OuterLineWidth = 159;
    xPageStyle->setPropertyValue("TopBorder", uno::Any(aBorder));
    sal_Int32 nTopBorderDistance = -646;
    xPageStyle->setPropertyValue("TopBorderDistance", uno::Any(nTopBorderDistance));

    // When exporting to DOCX:
    save("Office Open XML Text");

    // Then make sure that the page edge -> border space is correct:
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "//w:pgMar"_ostr, "top"_ostr, "284");
    assertXPath(pXmlDoc, "//w:pgBorders/w:top"_ostr, "sz"_ostr, "36");
    // Without the fix in place, this test would have failed with:
    // - Expected: 28
    // - Actual  : 0
    // i.e. editeng::BorderDistancesToWord() mis-handled negative border distances.
    assertXPath(pXmlDoc, "//w:pgBorders/w:top"_ostr, "space"_ostr, "28");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf148494)
{
    loadAndSave("tdf148494.docx");

    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // Without the fix in place, this test would have failed with
    // - Expected:  MACROBUTTON AllCaps Hello World
    // - Actual  :  MACROBUTTONAllCaps Hello World
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:r[3]/w:instrText"_ostr, " MACROBUTTON AllCaps Hello World ");
}

DECLARE_OOXMLEXPORT_TEST(testTdf137466, "tdf137466.docx")
{
    if (!isExported())
       return; // initial import, no further checks
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // Ensure that we have <w:placeholder><w:docPart v:val="xxxx"/></w:placeholder>
    OUString sDocPart = getXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:placeholder/w:docPart"_ostr, "val"_ostr);
    CPPUNIT_ASSERT_EQUAL(OUString("DefaultPlaceholder_-1854013440"), sDocPart);

    // Ensure that we have <w15:color v:val="xxxx"/>
    OUString sColor = getXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w15:color"_ostr, "val"_ostr);
    CPPUNIT_ASSERT_EQUAL(OUString("FF0000"), sColor);
}

DECLARE_OOXMLEXPORT_TEST(testParaListRightIndent, "testParaListRightIndent.docx")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), getProperty<sal_Int32>(getParagraph(1), "ParaRightMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5001), getProperty<sal_Int32>(getParagraph(2), "ParaRightMargin"));
}

CPPUNIT_TEST_FIXTURE(Test, testDontAddNewStyles)
{
    // Given a document that lacks builtin styles, and addition of them is disabled:
    {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Load::DisableBuiltinStyles::set(true, pBatch);
        pBatch->commit();
    }
    comphelper::ScopeGuard g([] {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Load::DisableBuiltinStyles::set(false, pBatch);
        pBatch->commit();
    });

    // When saving that document:
    loadAndSave("dont-add-new-styles.docx");

    // Then make sure that export doesn't have additional styles, Caption was one of them:
    xmlDocUniquePtr pXmlDoc = parseExport("word/styles.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. builtin styles were added to the export result, even if we opted out.
    assertXPath(pXmlDoc, "/w:styles/w:style[@w:styleId='Caption']"_ostr, 0);
}

DECLARE_OOXMLEXPORT_TEST(TestWPGZOrder, "testWPGZOrder.docx")
{
    // Check if the load failed.
    CPPUNIT_ASSERT(mxComponent);

    // Get the WPG
    uno::Reference<drawing::XShapes> xGroup(getShape(1), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xGroupProperties(xGroup, uno::UNO_QUERY_THROW);

    // Initialize a queue for subgroups
    std::queue<uno::Reference<drawing::XShapes>> xGroupList;
    xGroupList.push(xGroup);

    // Every textbox shall be visible.
    while (xGroupList.size())
    {
        // Get the first group
        xGroup = xGroupList.front();
        xGroupList.pop();
        for (sal_Int32 i = 0; i < xGroup->getCount(); ++i)
        {
            // Get the child shape
            uno::Reference<beans::XPropertySet> xChildShapeProperties(xGroup->getByIndex(i),
                uno::UNO_QUERY_THROW);
            // Check for textbox
            if (!xChildShapeProperties->getPropertyValue("TextBox").get<bool>())
            {
                // Is this a Group Shape? Put it into the queue.
                uno::Reference<drawing::XShapes> xInnerGroup(xGroup->getByIndex(i), uno::UNO_QUERY);
                if (xInnerGroup)
                    xGroupList.push(xInnerGroup);
                continue;
            }

            // Get the textbox properties
            uno::Reference<beans::XPropertySet> xTextBoxFrameProperties(
                xChildShapeProperties->getPropertyValue("TextBoxContent"), uno::UNO_QUERY_THROW);

            // Assert that the textbox ZOrder greater than the groupshape
            CPPUNIT_ASSERT_GREATER(xGroupProperties->getPropertyValue("ZOrder").get<long>(),
                xTextBoxFrameProperties->getPropertyValue("ZOrder").get<long>());
            // Before the fix, this failed because that was less, and the textboxes were covered.
        }

    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf148720)
{
    loadAndReload("tdf148720.odt");
    const auto& pLayout = parseLayoutDump();

    const OString sShapeXPaths[] =
    {
        "/root/page/body/txt/anchored/SwAnchoredDrawObject/SdrObjGroup/SdrObjList/SdrObject[1]"_ostr,
        "/root/page/body/txt/anchored/SwAnchoredDrawObject/SdrObjGroup/SdrObjList/SdrObjGroup/SdrObjList/SdrObjGroup/SdrObjList/SdrObject[1]"_ostr,
        "/root/page/body/txt/anchored/SwAnchoredDrawObject/SdrObjGroup/SdrObjList/SdrObjGroup/SdrObjList/SdrObjGroup/SdrObjList/SdrObject[2]"_ostr,
        "/root/page/body/txt/anchored/SwAnchoredDrawObject/SdrObjGroup/SdrObjList/SdrObject[2]"_ostr
    };

    const OString sTextXPaths[] =
    {
        "/root/page/body/txt/anchored/fly[1]/infos/bounds"_ostr,
        "/root/page/body/txt/anchored/fly[2]/infos/bounds"_ostr,
        "/root/page/body/txt/anchored/fly[3]/infos/bounds"_ostr,
        "/root/page/body/txt/anchored/fly[4]/infos/bounds"_ostr
    };

    const OString sAttribs[] =
    {
        "left"_ostr,
        "top"_ostr,
        "width"_ostr,
        "height"_ostr
    };

    for (sal_Int32 i = 0; i < 4; ++i)
    {
        OUString aShapeVals[4];
        int aTextVals[4] = {0, 0, 0, 0};

        const auto aOutRect = getXPath(pLayout, sShapeXPaths[i], "aOutRect"_ostr);

        sal_uInt16 nCommaPos[4] = {0, 0, 0, 0};
        nCommaPos[1] = aOutRect.indexOf(",");
        nCommaPos[2] = aOutRect.indexOf(",", nCommaPos[1] + 1);
        nCommaPos[3] = aOutRect.indexOf(",", nCommaPos[2] + 1);


        aShapeVals[0] = aOutRect.copy(nCommaPos[0], nCommaPos[1] - nCommaPos[0]);
        aShapeVals[1] = aOutRect.copy(nCommaPos[1] + 2, nCommaPos[2] - nCommaPos[1] - 2);
        aShapeVals[2] = aOutRect.copy(nCommaPos[2] + 2, nCommaPos[3] - nCommaPos[2] - 2);
        aShapeVals[3] = aOutRect.copy(nCommaPos[3] + 2, aOutRect.getLength() - nCommaPos[3] - 2);

        for (int ii = 0; ii < 4; ++ii)
        {
            aTextVals[ii] = getXPath(pLayout, sTextXPaths[i], sAttribs[ii]).toInt32();
        }

        tools::Rectangle ShapeArea(Point(aShapeVals[0].toInt32(), aShapeVals[1].toInt32()), Size(aShapeVals[2].toInt32() + 5, aShapeVals[3].toInt32() + 5));

        tools::Rectangle TextArea(Point(aTextVals[0], aTextVals[1]), Size(aTextVals[2], aTextVals[3]));

        CPPUNIT_ASSERT(ShapeArea.Contains(TextArea));
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf126287, "tdf126287.docx")
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf123642_BookmarkAtDocEnd, "tdf123642.docx")
{
    // get bookmark interface
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(), uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xBookmarksByName = xBookmarksSupplier->getBookmarks();

    // check: we have 1 bookmark (previously there were 0)
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xBookmarksByIdx->getCount());
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Bookmark1"));

    // and it is really in exported DOCX (let's ensure)
    if (!isExported())
       return; // initial import, no further checks
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    CPPUNIT_ASSERT_EQUAL(OUString("Bookmark1"), getXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:bookmarkStart[1]"_ostr, "name"_ostr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf148361, "tdf148361.docx")
{
    // Plain text Block SDT is imported as content control
    OUString aActual = getParagraph(1)->getString();
    // This was "itadmin".
    CPPUNIT_ASSERT_EQUAL(OUString("itadmin"), aActual);

    aActual = getParagraph(2)->getString();
    // This was "itadmin".
    CPPUNIT_ASSERT_EQUAL(OUString("[Type text]"), aActual);
}

DECLARE_OOXMLEXPORT_TEST(testTdf153082_semicolon, "custom-styles-TOC-semicolon.docx")
{
    uno::Reference<text::XDocumentIndexesSupplier> xIndexSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexes = xIndexSupplier->getDocumentIndexes();
    uno::Reference<text::XDocumentIndex> xTOC(xIndexes->getByIndex(0), uno::UNO_QUERY);
    // check styles
    uno::Reference<container::XIndexAccess> xParaStyles =
        getProperty<uno::Reference<container::XIndexAccess>>(xTOC, "LevelParagraphStyles");
    uno::Sequence<OUString> styles;
    xParaStyles->getByIndex(0) >>= styles;
    CPPUNIT_ASSERT_EQUAL(uno::Sequence<OUString>{}, styles);
    xParaStyles->getByIndex(1) >>= styles;
    CPPUNIT_ASSERT_EQUAL(uno::Sequence<OUString>{}, styles);
    xParaStyles->getByIndex(2) >>= styles;
    // the first one is built-in Word style "Intense Quote" that was localised DE "Intensives Zitat" in the file
    CPPUNIT_ASSERT_EQUAL((uno::Sequence<OUString>{"Intensives Zitat", "Custom1", "_MyStyle0"}), styles);
    xTOC->update();
    OUString const tocContent(xTOC->getAnchor()->getString());
    CPPUNIT_ASSERT(tocContent.startsWith("Table of Contents"));
    CPPUNIT_ASSERT(tocContent.indexOf("Lorem ipsum dolor sit amet, consectetuer adipiscing elit.") != -1);
    CPPUNIT_ASSERT(tocContent.indexOf("Fusce posuere, magna sed pulvinar ultricies, purus lectus malesuada libero, sit amet commodo magna eros quis urna.") != -1);
    CPPUNIT_ASSERT(tocContent.indexOf("Proin pharetra nonummy pede. Mauris et orci.") != -1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf153082_comma, "custom-styles-TOC-comma.docx")
{
    uno::Reference<text::XDocumentIndexesSupplier> xIndexSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexes = xIndexSupplier->getDocumentIndexes();
    uno::Reference<text::XDocumentIndex> xTOC(xIndexes->getByIndex(0), uno::UNO_QUERY);
    // check styles
    uno::Reference<container::XIndexAccess> xParaStyles =
        getProperty<uno::Reference<container::XIndexAccess>>(xTOC, "LevelParagraphStyles");
    uno::Sequence<OUString> styles;
    xParaStyles->getByIndex(0) >>= styles;
    CPPUNIT_ASSERT_EQUAL(uno::Sequence<OUString>{"_MyStyle0"}, styles);
    xParaStyles->getByIndex(1) >>= styles;
    CPPUNIT_ASSERT_EQUAL(uno::Sequence<OUString>{"Custom1"}, styles);
    xParaStyles->getByIndex(2) >>= styles;
    // the first one is built-in Word style "Intense Quote" that was localised DE "Intensives Zitat" in the file
    CPPUNIT_ASSERT_EQUAL(uno::Sequence<OUString>{"Intensives Zitat"}, styles);
    xTOC->update();
    OUString const tocContent(xTOC->getAnchor()->getString());
    CPPUNIT_ASSERT(tocContent.startsWith("Table of Contents"));
    CPPUNIT_ASSERT(tocContent.indexOf("Lorem ipsum dolor sit amet, consectetuer adipiscing elit.") != -1);
    CPPUNIT_ASSERT(tocContent.indexOf("Fusce posuere, magna sed pulvinar ultricies, purus lectus malesuada libero, sit amet commodo magna eros quis urna.") != -1);
    CPPUNIT_ASSERT(tocContent.indexOf("Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas.") != -1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf160402, "StyleRef-DE.docx")
{
    xmlDocUniquePtr pLayout = parseLayoutDump();
    assertXPath(pLayout, "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, "expand"_ostr, "Heading 1");
    assertXPath(pLayout, "/root/page[2]/header/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, "expand"_ostr, "Nunc viverra imperdiet enim. Fusce est. Vivamus a tellus.");
    assertXPath(pLayout, "/root/page[3]/header/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, "expand"_ostr, "Error: Reference source not found"); // TODO
    assertXPath(pLayout, "/root/page[4]/header/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, "expand"_ostr, "Nunc viverra imperdiet enim. Fusce est. Vivamus a tellus.");
    assertXPath(pLayout, "/root/page[5]/header/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, "expand"_ostr, "Error: Reference source not found"); // TODO
}

DECLARE_OOXMLEXPORT_TEST(testTdf142407, "tdf142407.docx")
{
    uno::Reference<container::XNameAccess> xPageStyles = getStyles("PageStyles");
    uno::Reference<beans::XPropertySet> xPageStyle(xPageStyles->getByName("Standard"), uno::UNO_QUERY);
    sal_Int16 nGridLines;
    xPageStyle->getPropertyValue("GridLines") >>= nGridLines;
    CPPUNIT_ASSERT_EQUAL( sal_Int16(36), nGridLines);   // was 23, left large space before text.
}

DECLARE_OOXMLEXPORT_TEST(testWPGBodyPr, "WPGbodyPr.docx")
{
    // Is load successful?
    CPPUNIT_ASSERT(mxComponent);

    // There are a WPG shape and a picture
    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    // Get the WPG shape
    uno::Reference<drawing::XShapes> xGroup(getShape(1), uno::UNO_QUERY);
    // And the embed WPG
    uno::Reference<drawing::XShapes> xEmbedGroup(xGroup->getByIndex(1), uno::UNO_QUERY);

    // Get the properties of the shapes
    uno::Reference<beans::XPropertySet> xOuterShape(xGroup->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xMiddleShape(xEmbedGroup->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xInnerShape(xEmbedGroup->getByIndex(1), uno::UNO_QUERY);

    // Get the properties of the textboxes too
    uno::Reference<beans::XPropertySet> xOuterTextBox(
        xOuterShape->getPropertyValue("TextBoxContent"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xMiddleTextBox(
        xMiddleShape->getPropertyValue("TextBoxContent"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xInnerTextBox(
        xInnerShape->getPropertyValue("TextBoxContent"), uno::UNO_QUERY);

    // Check the alignments
    CPPUNIT_ASSERT_EQUAL(css::drawing::TextVerticalAdjust::TextVerticalAdjust_TOP,
                         xOuterTextBox->getPropertyValue("TextVerticalAdjust")
                             .get<css::drawing::TextVerticalAdjust>());
    CPPUNIT_ASSERT_EQUAL(css::drawing::TextVerticalAdjust::TextVerticalAdjust_TOP,
                         xMiddleTextBox->getPropertyValue("TextVerticalAdjust")
                             .get<css::drawing::TextVerticalAdjust>());
    CPPUNIT_ASSERT_EQUAL(css::drawing::TextVerticalAdjust::TextVerticalAdjust_CENTER,
                         xInnerTextBox->getPropertyValue("TextVerticalAdjust")
                             .get<css::drawing::TextVerticalAdjust>());

    // Check the inset margins, all were 0 before the fix
    CPPUNIT_ASSERT_EQUAL(sal_Int32(499),
                         xInnerShape->getPropertyValue("TextLowerDistance").get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(499),
                         xInnerShape->getPropertyValue("TextUpperDistance").get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1000),
                         xInnerShape->getPropertyValue("TextLeftDistance").get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(254),
                         xInnerShape->getPropertyValue("TextRightDistance").get<sal_Int32>());
}

DECLARE_OOXMLEXPORT_TEST(testTdf146851_1, "tdf146851_1.docx")
{
    uno::Reference<beans::XPropertySet> xPara;

    xPara.set(getParagraph(1, "qwerty"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("1."), getProperty<OUString>(xPara, "ListLabelString"));

    xPara.set(getParagraph(2, "asdfg"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("1/"), getProperty<OUString>(xPara, "ListLabelString"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf146851_2, "tdf146851_2.docx")
{
    // Ensure numbering on second para
    uno::Reference<beans::XPropertySet> xPara;
    xPara.set(getParagraph(2, "."), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Schedule"), getProperty<OUString>(xPara, "ListLabelString"));

    // Refresh fields and ensure cross-reference to numbered para is okay
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());

    uno::Reference<util::XRefreshable>(xFieldsAccess, uno::UNO_QUERY_THROW)->refresh();

    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    CPPUNIT_ASSERT(xFields->hasMoreElements());
    uno::Reference<text::XTextField> xTextField(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Schedule"), xTextField->getPresentation(false));
}

DECLARE_OOXMLEXPORT_TEST(testTdf148052, "tdf148052.docx")
{
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());

    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    CPPUNIT_ASSERT(xFields->hasMoreElements());

    uno::Reference<text::XTextField> xTextField(xFields->nextElement(), uno::UNO_QUERY);

    // Without the fix in place, this test would have failed with
    // - Expected: 14. Aug 18
    // - Actual  : 11. Apr 22
    CPPUNIT_ASSERT_EQUAL(OUString("14. Aug 18"), xTextField->getPresentation(false));
}

DECLARE_OOXMLEXPORT_TEST(testTdf148111, "tdf148111.docx")
{
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());

    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    std::vector<OUString> aExpectedValues = {
        // These field values are NOT in order in document: getTextFields did provide
        // fields in a strange but fixed order
        "Title", "Placeholder", "Placeholder", "Placeholder",
        "Placeholder", "Placeholder", "Placeholder", "Placeholder",
        "Placeholder", "Placeholder", "Placeholder", "Placeholder",
        "Placeholder", "Placeholder", "Placeholder", "Placeholder",
        "Placeholder", "Title", "Title", "Title",
        "Title", "Title", "Title", "Title"
    };

    sal_uInt16 nIndex = 0;
    while (xFields->hasMoreElements())
    {
        uno::Reference<text::XTextField> xTextField(xFields->nextElement(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(aExpectedValues[nIndex++], xTextField->getPresentation(false));
    }

    // No more fields
    CPPUNIT_ASSERT(!xFields->hasMoreElements());

    if (!isExported())
        return;
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // ShowingPlaceholder should be off for 0, false and "on". (This was 21 before the fix)
    assertXPath(pXmlDoc,"//w:p/w:sdt/w:sdtPr/w:showingPlcHdr"_ostr, 12);
}

DECLARE_OOXMLEXPORT_TEST(TestTdf73499, "tdf73499.docx")
{
    // Ensure, the bugdoc is opened
    CPPUNIT_ASSERT(mxComponent);
    // Get the groupshape
    uno::Reference<drawing::XShapes> xGroup(getShape(1), uno::UNO_QUERY_THROW);

    // Get the textboxes of the groupshape
    uno::Reference<text::XText> xTextBox1(xGroup->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<text::XText> xTextBox2(xGroup->getByIndex(1), uno::UNO_QUERY_THROW);

    // Get the properties of the textboxes
    uno::Reference<beans::XPropertySet> xTextBox1Properties(xTextBox1, uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xTextBox2Properties(xTextBox2, uno::UNO_QUERY_THROW);

    // Get the name of the textboxes
    uno::Reference<container::XNamed> xTextBox1Name(xTextBox1, uno::UNO_QUERY_THROW);
    uno::Reference<container::XNamed> xTextBox2Name(xTextBox2, uno::UNO_QUERY_THROW);

    // Check for the links, before the fix that were missing
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Link name missing!", xTextBox2Name->getName(),
        xTextBox1Properties->getPropertyValue("ChainNextName").get<OUString>());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Link name missing!", xTextBox1Name->getName(),
        xTextBox2Properties->getPropertyValue("ChainPrevName").get<OUString>());
}

DECLARE_OOXMLEXPORT_TEST(testTdf81507, "tdf81507.docx")
{
    if (!isExported())
       return; // initial import, no further checks
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // Ensure that we have <w:text w:multiLine="1"/>
    CPPUNIT_ASSERT_EQUAL(OUString("1"), getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:sdt/w:sdtPr/w:text"_ostr, "multiLine"_ostr));

    // Ensure that we have <w:text w:multiLine="0"/>
    CPPUNIT_ASSERT_EQUAL(OUString("0"), getXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:sdt/w:sdtPr/w:text"_ostr, "multiLine"_ostr));

    // Ensure that we have <w:text/>
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:sdt/w:sdtPr/w:text"_ostr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:sdt/w:sdtPr/w:text"_ostr);
}

DECLARE_OOXMLEXPORT_TEST(testTdf139948, "tdf139948.docx")
{
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0),
        getProperty<table::BorderLine2>(getParagraph(1, "No border"), "TopBorder").LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0),
        getProperty<table::BorderLine2>(getParagraph(2, "Border below"), "TopBorder").LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(88),
        getProperty<table::BorderLine2>(getParagraph(3, "Borders below and above"), "TopBorder").LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(88),
        getProperty<table::BorderLine2>(getParagraph(4, "Border above"), "TopBorder").LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0),
        getProperty<table::BorderLine2>(getParagraph(5, "No border"), "TopBorder").LineWidth);


    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0),
        getProperty<table::BorderLine2>(getParagraph(1), "BottomBorder").LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0),
        getProperty<table::BorderLine2>(getParagraph(2), "BottomBorder").LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0),
        getProperty<table::BorderLine2>(getParagraph(3), "BottomBorder").LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0),
        getProperty<table::BorderLine2>(getParagraph(4), "BottomBorder").LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0),
        getProperty<table::BorderLine2>(getParagraph(5), "BottomBorder").LineWidth);
}

DECLARE_OOXMLEXPORT_TEST(testTdf144563, "tdf144563.docx")
{
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());

    // Refresh all cross-reference fields
    uno::Reference<util::XRefreshable>(xFieldsAccess, uno::UNO_QUERY_THROW)->refresh();

    // Verify values
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    std::vector<OUString> aExpectedValues = {
        // These field values are NOT in order in document: getTextFields did provide
        // fields in a strange but fixed order
        "1", "1", "1", "1", "1/", "1/", "1/", "1)", "1)", "1)", "1.)",
        "1.)", "1.)", "1..", "1..", "1..", "1.", "1.", "1.", "1", "1"
    };

    sal_uInt16 nIndex = 0;
    while (xFields->hasMoreElements())
    {
        uno::Reference<text::XTextField> xTextField(xFields->nextElement(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(aExpectedValues[nIndex++], xTextField->getPresentation(false));
    }
}

// broken test document?
#if !defined(_WIN32)
CPPUNIT_TEST_FIXTURE(Test, testTdf146955)
{
    loadAndReload("tdf146955.odt");
    // import of a (broken?) DOCX export with dozens of frames raised a SAX exception,
    // when the code tried to access to a non-existent footnote
    uno::Reference<text::XFootnotesSupplier> xNotes(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xNotes->getFootnotes()->getCount());
}
#endif

CPPUNIT_TEST_FIXTURE(Test, testTdf144668)
{
    loadAndReload("tdf144668.odt");
    uno::Reference<beans::XPropertySet> xPara1(getParagraph(1, u"level1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("[0001]"), getProperty<OUString>(xPara1, "ListLabelString"));

    uno::Reference<beans::XPropertySet> xPara2(getParagraph(2, u"level2"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("[001]"), getProperty<OUString>(xPara2, "ListLabelString"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf148455_1, "tdf148455_1.docx")
{
    uno::Reference<beans::XPropertySet> xPara2(getParagraph(3, u"1.1.1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("1.1.1."), getProperty<OUString>(xPara2, "ListLabelString"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf148455_2, "tdf148455_2.docx")
{
    if (!isExported())
       return; // initial import, no further checks
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // Find list id for restarted list
    sal_Int32 nListId = getXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:numPr/w:numId"_ostr, "val"_ostr).toInt32();

    xmlDocUniquePtr pNumberingDoc = parseExport("word/numbering.xml");

    // Ensure we have empty lvlOverride for levels 0 - 1
    assertXPath(pNumberingDoc, "/w:numbering/w:num[@w:numId='" + OString::number(nListId) +"']/w:lvlOverride[@w:ilvl='0']");
    assertXPath(pNumberingDoc, "/w:numbering/w:num[@w:numId='" + OString::number(nListId) +"']/w:lvlOverride[@w:ilvl='1']");
    // And normal override for level 2
    getXPath(pNumberingDoc, "/w:numbering/w:num[@w:numId='" + OString::number(nListId) +"']/w:lvlOverride[@w:ilvl='2']/w:startOverride", "val"_ostr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf147978enhancedPathABVW)
{
    createSwDoc("tdf147978_enhancedPath_commandABVW.odt");
    saveAndReload("Office Open XML Text");
    // Make sure the new implemented export for commands A,B,V and W use the correct arc between
    // the given two points, here the short one.
    for (sal_Int16 i = 1 ; i <= 4; ++i)
    {
        uno::Reference<drawing::XShape> xShape = getShape(i);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(506), getProperty<awt::Rectangle>(xShape, "BoundRect").Height);
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf148132, "tdf148132.docx")
{
    {
        uno::Reference<text::XTextRange> xParagraph = getParagraph(1);
        auto xLevels = getProperty< uno::Reference<container::XIndexAccess> >(xParagraph, "NumberingRules");
        // Get level 2 char style
        comphelper::SequenceAsHashMap levelProps(xLevels->getByIndex(1));
        OUString aCharStyleName = levelProps["CharStyleName"].get<OUString>();
        // Ensure that numbering in this paragraph is 24pt bold italic
        // Previously it got overridden by paragraph properties and became 6pt, no bold, no italic
        uno::Reference<beans::XPropertySet> xStyle(getStyles("CharacterStyles")->getByName(aCharStyleName), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(24.f, getProperty<float>(xStyle, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xStyle, "CharWeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC, getProperty<awt::FontSlant>(xStyle, "CharPosture"));
    }
    // And do the same for second paragraph. Numbering should be identical
    {
        uno::Reference<text::XTextRange> xParagraph = getParagraph(2);
        auto xLevels = getProperty< uno::Reference<container::XIndexAccess> >(xParagraph, "NumberingRules");
        comphelper::SequenceAsHashMap levelProps(xLevels->getByIndex(1));
        OUString aCharStyleName = levelProps["CharStyleName"].get<OUString>();

        uno::Reference<beans::XPropertySet> xStyle(getStyles("CharacterStyles")->getByName(aCharStyleName), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(24.f, getProperty<float>(xStyle, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xStyle, "CharWeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC, getProperty<awt::FontSlant>(xStyle, "CharPosture"));
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf154481, "tdf154481.docx")
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Missing pages!", 7, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf149200)
{
    loadAndSave("tdf149200.docx");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // Ensure there is no unexpected invalid structure <w14:textFill>
    // There is just one run property
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "count(/w:document/w:body/w:p[1]/w:r[1]/w:rPr/*)"_ostr);
    CPPUNIT_ASSERT(pXmlObj);
    CPPUNIT_ASSERT_EQUAL(double(1), pXmlObj->floatval);
    // And it is a color definition with themeColor
    CPPUNIT_ASSERT_EQUAL(OUString("dark1"), getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w:color"_ostr, "themeColor"_ostr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf149313, "tdf149313.docx")
{
    // only 2, but not 3 pages in document
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // And ensure that pages are with correct sections (have correct dimensions)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4989), getXPath(pXmlDoc, "/root/page[1]/infos/bounds"_ostr, "height"_ostr).toInt32());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4989), getXPath(pXmlDoc, "/root/page[1]/infos/bounds"_ostr, "width"_ostr).toInt32());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4989), getXPath(pXmlDoc, "/root/page[2]/infos/bounds"_ostr, "height"_ostr).toInt32());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8000), getXPath(pXmlDoc, "/root/page[2]/infos/bounds"_ostr, "width"_ostr).toInt32());
}

DECLARE_OOXMLEXPORT_TEST(testTdf148360, "tdf148360.docx")
{
    const auto& pLayout = parseLayoutDump();

    // Ensure first element is a tab
    assertXPath(pLayout, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/child::*[1]"_ostr, "type"_ostr, "PortionType::TabLeft");
    // and only then goes content
    assertXPath(pLayout, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/child::*[2]"_ostr, "type"_ostr, "PortionType::Text");
}

DECLARE_OOXMLEXPORT_TEST(testTdf135923, "tdf135923-min.docx")
{
    uno::Reference<text::XText> xShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, xShape);

    CPPUNIT_ASSERT_EQUAL(COL_WHITE, getProperty<Color>(getRun(xParagraph, 1), "CharColor"));
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, getProperty<Color>(getRun(xParagraph, 2), "CharColor"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf148273_sectionBulletFormatLeak, "tdf148273_sectionBulletFormatLeak.docx")
{
    // get a paragraph with bullet point after section break
    uno::Reference<text::XTextRange> xParagraph = getParagraph(4);
    uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);

    // Make sure that the bullet has no ListAutoFormat inherited from
    // the empty paragraph before the section break
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. empty paragraph formats from the first section leaked to the bullet's formatting
    uno::Any aValue = xProps->getPropertyValue("ListAutoFormat");
    CPPUNIT_ASSERT_EQUAL(false, aValue.hasValue());
}

DECLARE_OOXMLEXPORT_TEST(testTdf149089, "tdf149089.docx")
{
    uno::Reference<container::XNameAccess> xPageStyles = getStyles("PageStyles");
    uno::Reference<beans::XPropertySet> xPageStyle(xPageStyles->getByName("Standard"), uno::UNO_QUERY);
    sal_Int16 nGridMode;
    xPageStyle->getPropertyValue("GridMode") >>= nGridMode;
    CPPUNIT_ASSERT_EQUAL( sal_Int16(text::TextGridMode::LINES), nGridMode);   // was LINES_AND_CHARS
}

CPPUNIT_TEST_FIXTURE(Test, testTdf139128)
{
    loadAndReload("tdf139128.odt");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 0
    // i.e. the line break was lost on export.
    assertXPath(pXmlDoc, "//w:br"_ostr, 2);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
