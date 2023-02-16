/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/document/XDocumentInsertable.hpp>

#include <vcl/scheduler.hxx>

using namespace ::com::sun::star;

namespace
{
/// Tests for writerfilter/source/dmapper/DomainMapper_Impl.cxx.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest("/writerfilter/qa/cppunittests/dmapper/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testPageBreakFooterTable)
{
    // Load a document which refers to a footer which ends with a table, and there is a page break
    // in the body text right after the footer reference.
    loadFromURL(u"page-break-footer-table.docx");

    // Check the last paragraph.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<beans::XPropertySet> xPara;
    while (xParaEnum->hasMoreElements())
    {
        xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    }
    style::BreakType eType = style::BreakType_NONE;
    xPara->getPropertyValue("BreakType") >>= eType;

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 4
    // - Actual  : 0
    // i.e. there was no page break before the last paragraph.
    CPPUNIT_ASSERT_EQUAL(style::BreakType_PAGE_BEFORE, eType);
}

CPPUNIT_TEST_FIXTURE(Test, testNumberingRestartStyleParent)
{
    loadFromURL(u"num-restart-style-parent.docx");

    // The paragraphs are A 1 2 B 1 2.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<beans::XPropertySet> xPara;
    static OUStringLiteral aProp(u"ListLabelString");
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("A."), xPara->getPropertyValue(aProp).get<OUString>());
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("1."), xPara->getPropertyValue(aProp).get<OUString>());
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("2."), xPara->getPropertyValue(aProp).get<OUString>());
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("B."), xPara->getPropertyValue(aProp).get<OUString>());
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1.
    // - Actual  : 3.
    // i.e. the numbering was not restarted after B.
    CPPUNIT_ASSERT_EQUAL(OUString("1."), xPara->getPropertyValue(aProp).get<OUString>());
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("2."), xPara->getPropertyValue(aProp).get<OUString>());
}

CPPUNIT_TEST_FIXTURE(Test, testFrameDirection)
{
    loadFromURL(u"frame-direction.docx");

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xFrame0(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame1(xDrawPage->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame2(xDrawPage->getByIndex(2), uno::UNO_QUERY);
    // Without the accompanying fix in place, all of the following values would be text::WritingMode2::CONTEXT
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::CONTEXT,
                         xFrame0->getPropertyValue("WritingMode").get<sal_Int16>());
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::BT_LR,
                         xFrame1->getPropertyValue("WritingMode").get<sal_Int16>());
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::TB_RL,
                         xFrame2->getPropertyValue("WritingMode").get<sal_Int16>());
}

CPPUNIT_TEST_FIXTURE(Test, testAltChunk)
{
    loadFromURL(u"alt-chunk.docx");
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara;
    uno::Reference<beans::XPropertySet> xParaProps;
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xParaProps.set(xPara, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("outer, before sect break"), xPara->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         xParaProps->getPropertyValue("PageStyleName").get<OUString>());
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xParaProps.set(xPara, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("outer, after sect break"), xPara->getString());

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Converted1
    // - Actual  : Standard
    // i.e. the page break between the first and the second paragraph was missing.
    CPPUNIT_ASSERT_EQUAL(OUString("Converted1"),
                         xParaProps->getPropertyValue("PageStyleName").get<OUString>());

    // Without the accompanying fix in place, this test would have failed with a
    // container.NoSuchElementException, as the document had only 2 paragraphs, all the "inner"
    // content was lost.
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("inner doc, first para"), xPara->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testFieldIfInsideIf)
{
    // Load a document with a field in a table cell: it contains an IF field with various nested
    // fields.
    loadFromURL(u"field-if-inside-if.docx");
    uno::Reference<text::XTextTablesSupplier> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextDocument->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    // Get the result of the topmost field.
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 0** Expression is faulty **2
    // i.e. some of the inner fields escaped outside the outer field.
    CPPUNIT_ASSERT_EQUAL(OUString("2"), xCell->getString());

    // Test the second cell: it contains "IF ", not the usual " IF ".
    xCell.set(xTable->getCellByName("A2"), uno::UNO_QUERY);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 25
    // - Actual  : 025
    // i.e. some of the inner fields escaped outside the outer field.
    CPPUNIT_ASSERT_EQUAL(OUString("25"), xCell->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testCreateDatePreserve)
{
    loadFromURL(u"create-date-preserve.docx");
    // Trigger idle layout.
    Scheduler::ProcessEventsToIdle();
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xPortionEnumAccess(xParaEnum->nextElement(),
                                                                     uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPortionEnum = xPortionEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPortion(xPortionEnum->nextElement(), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 7/7/2020 10:11:00 AM
    // - Actual  : 07/07/2020
    // i.e. the formatting of the create date field was lost.
    CPPUNIT_ASSERT_EQUAL(OUString("7/7/2020 10:11:00 AM"), xPortion->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testChartZOrder)
{
    // Given a document with a chart and a shape on it:
    loadFromURL(u"chart-zorder.docx");

    // Then make sure the shape is on top of the chart:
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<lang::XServiceInfo> xChart(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed, as the chart was on top
    // of the shape.
    CPPUNIT_ASSERT(xChart->supportsService("com.sun.star.text.TextEmbeddedObject"));
}

CPPUNIT_TEST_FIXTURE(Test, testPTab)
{
    // Given a document that has a <w:ptab> to render a linebreak:
    loadFromURL(u"ptab.docx");

    // Then make sure that the Writer doc model contains that linebreak:
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName("PageStyles"),
                                                        uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStyle(xStyleFamily->getByName("Standard"), uno::UNO_QUERY);
    auto xFooter = xStyle->getPropertyValue("FooterText").get<uno::Reference<text::XTextRange>>();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: <space><newline>1\n
    // - Actual:   <space><tab>1\n
    // i.e. the layout height of the footer text was incorrect, the page number field was not
    // visually inside the background shape.
    CPPUNIT_ASSERT_EQUAL(OUString(" \n1" SAL_NEWLINE_STRING), xFooter->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testPasteOle)
{
    // Given an empty document:
    mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");

    // When pasting RTF into that document:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<document::XDocumentInsertable> xCursor(
        xText->createTextCursorByRange(xText->getStart()), uno::UNO_QUERY);
    OUString aURL = createFileURL(u"paste-ole.rtf");
    xCursor->insertDocumentFromURL(aURL, {});

    // Then make sure that all the 3 paragraphs of the paste data (empty para, OLE obj, text) are
    // inserted to the document:
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    xParaEnum->nextElement();
    // Without the accompanying fix in place, this test would have failed, as the paste result was a
    // single paragraph, containing the OLE object, and the content after the OLE object was lost.
    CPPUNIT_ASSERT(xParaEnum->hasMoreElements());
    xParaEnum->nextElement();
    CPPUNIT_ASSERT(xParaEnum->hasMoreElements());
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("hello"), xPara->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testClearingBreak)
{
    // Given a document with a clearing break:
    loadFromURL(u"clearing-break.docx");

    // Then make sure that the clear property of the break is not ignored:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphs = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xParagraph(xParagraphs->nextElement(),
                                                             uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPortions = xParagraph->createEnumeration();
    xPortions->nextElement();
    xPortions->nextElement();
    // Without the accompanying fix in place, this test would have failed with:
    // An uncaught exception of type com.sun.star.container.NoSuchElementException
    // i.e. the first para was just a fly + text portion, the clearing break was lost.
    uno::Reference<beans::XPropertySet> xPortion(xPortions->nextElement(), uno::UNO_QUERY);
    OUString aPortionType;
    xPortion->getPropertyValue("TextPortionType") >>= aPortionType;
    CPPUNIT_ASSERT_EQUAL(OUString("LineBreak"), aPortionType);
    uno::Reference<text::XTextContent> xLineBreak;
    xPortion->getPropertyValue("LineBreak") >>= xLineBreak;
    sal_Int16 eClear{};
    uno::Reference<beans::XPropertySet> xLineBreakProps(xLineBreak, uno::UNO_QUERY);
    xLineBreakProps->getPropertyValue("Clear") >>= eClear;
    // SwLineBreakClear::ALL
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(3), eClear);
}

CPPUNIT_TEST_FIXTURE(Test, testContentControlDateDataBinding)
{
    // Given a document with date content control and data binding, data binding date is 2012,
    // in-document date is 2022:
    loadFromURL(u"content-control-date-data-binding.docx");

    // Then make sure that the date is from the data binding, not from document.xml:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphs = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xParagraph(xParagraphs->nextElement(), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 4/26/2012
    // - Actual  : 4/26/2022
    // i.e. the date was from document.xml, which is considered outdated.
    CPPUNIT_ASSERT_EQUAL(OUString("4/26/2012"), xParagraph->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testContentControlDataBindingColor)
{
    // Given a document with an inline content control with data binding, placeholder char color is
    // set to red, when loading that document:
    loadFromURL(u"content-control-data-binding-color.docx");

    // Then make sure that the placeholder char color is not in the document, since data binding is
    // active:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xCursor->gotoEnd(/*bExpand=*/false);
    xCursor->goLeft(/*nCount=*/1, /*bExpand=*/false);
    uno::Reference<beans::XPropertySet> xCursorProps(xCursor, uno::UNO_QUERY);
    Color nColor;
    CPPUNIT_ASSERT(xCursorProps->getPropertyValue("CharColor") >>= nColor);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: rgba[ffffff00]
    // - Actual  : rgba[ff0000ff]
    // i.e. the char color was red, not the default / automatic.
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, nColor);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
