/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/text/XTextDocument.hpp>

#include <docsh.hxx>
#include <formatcontentcontrol.hxx>
#include <wrtsh.hxx>

namespace
{
/**
 * Covers sw/source/filter/ww8/ fixes.
 *
 * Note that these tests are meant to be simple: either load a file and assert some result or build
 * a document model with code, export and assert that result.
 *
 * Keep using the various sw_<format>import/export suites for multiple filter calls inside a single
 * test.
 */
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/filter/ww8/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testNegativePageBorderDocImport)
{
    // Given a document with a border distance that is larger than the margin, when loading that
    // document:
    createSwDoc("negative-page-border.doc");

    // Then make sure we map that to a negative border distance (move border from the edge of body
    // frame towards the center of the page, not towards the edge of the page):
    uno::Reference<container::XNameAccess> xStyleFamily = getStyles("PageStyles");
    uno::Reference<beans::XPropertySet> xStyle(xStyleFamily->getByName("Standard"), uno::UNO_QUERY);
    auto nTopMargin = xStyle->getPropertyValue("TopMargin").get<sal_Int32>();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 501
    // - Actual  : 342
    // i.e. the border properties influenced the margin, which was 284 twips in the sprmSDyaTop
    // SPRM.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(501), nTopMargin);
    auto aTopBorder = xStyle->getPropertyValue("TopBorder").get<table::BorderLine2>();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(159), aTopBorder.LineWidth);
    auto nTopBorderDistance = xStyle->getPropertyValue("TopBorderDistance").get<sal_Int32>();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-646), nTopBorderDistance);
}

CPPUNIT_TEST_FIXTURE(Test, testPlainTextContentControlExport)
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

    // When exporting to DOCX:
    save("Office Open XML Text");

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//w:sdt/w:sdtPr/w:text' number of nodes is incorrect
    // i.e. the plain text content control was turned into a rich text one on export.
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:text", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testDocxComboBoxContentControlExport)
{
    // Given a document with a combo box content control around a text portion:
    mxComponent = loadFromDesktop("private:factory/swriter");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::COMBO_BOX);

    // When exporting to DOCX:
    save("Office Open XML Text");

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//w:sdt/w:sdtPr/w:comboBox' number of nodes is incorrect
    // i.e. the combo box content control was turned into a drop-down one on export.
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:comboBox", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testDocxHyperlinkShape)
{
    // Given a document with a hyperlink at char positions 0 -> 6 and a shape with text anchored at
    // char position 6:
    mxComponent = loadFromDesktop("private:factory/swriter");
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, "beforeafter", /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->goRight(/*nCount=*/6, /*bExpand=*/true);
    uno::Reference<beans::XPropertySet> xCursorProps(xCursor, uno::UNO_QUERY);
    xCursorProps->setPropertyValue("HyperLinkURL", uno::Any(OUString("http://www.example.com/")));
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->goRight(/*nCount=*/6, /*bExpand=*/false);
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance("com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY);
    xShape->setSize(awt::Size(5000, 5000));
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    xShapeProps->setPropertyValue("AnchorType", uno::Any(text::TextContentAnchorType_AT_CHARACTER));
    uno::Reference<text::XTextContent> xShapeContent(xShape, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xShapeContent, /*bAbsorb=*/false);
    xShapeProps->setPropertyValue("TextBox", uno::Any(true));

    // When saving this document to DOCX, then make sure we don't crash on export (due to an
    // assertion failure for not-well-formed XML output):
    save("Office Open XML Text");
}

CPPUNIT_TEST_FIXTURE(Test, testDocxContentControlDropdownEmptyDisplayText)
{
    // Given a document with a dropdown content control, the only list item has no display text
    // (only a value):
    mxComponent = loadFromDesktop("private:factory/swriter");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::DROP_DOWN_LIST);

    // When saving to DOCX:
    save("Office Open XML Text");

    // Then make sure that no display text attribute is written:
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '//w:sdt/w:sdtPr/w:dropDownList/w:listItem' unexpected 'displayText' attribute
    // i.e. we wrote an empty attribute instead of omitting it.
    assertXPathNoAttribute(pXmlDoc, "//w:sdt/w:sdtPr/w:dropDownList/w:listItem", "displayText");
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
