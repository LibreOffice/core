/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapixml_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/qa/XDumper.hpp>

using namespace ::com::sun::star;

namespace
{
/// Tests for sw/source/writerfilter/dmapper/DomainMapperTableHandler.cxx.
class Test : public UnoApiXmlTest
{
public:
    Test()
        : UnoApiXmlTest("/sw/qa/writerfilter/dmapper/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, test1cellInsidevRightborder)
{
    loadFromFile(u"1cell-insidev-rightborder.docx");
    uno::Reference<text::XTextTablesSupplier> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextDocument->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    table::BorderLine2 aBorder;
    xCell->getPropertyValue("RightBorder") >>= aBorder;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 18
    // i.e. the request to have no table-level right border was lost on import.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(0), aBorder.LineWidth);
}

CPPUNIT_TEST_FIXTURE(Test, testNestedFloatingTable)
{
    loadFromFile(u"nested-floating-table.docx");
    // Normal outer table, floating inner tables.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xIndexAccess->getCount());
}

CPPUNIT_TEST_FIXTURE(Test, testFloatingTableBreakBefore)
{
    // Given a 3 pages document: page break, then a multi-page floating table on pages 2 and 3:
    // When laying out that document:
    loadFromFile(u"floattable-break-before.docx");

    // Then make sure the page break property is on the anchor of the floating table, otherwise it
    // has no effect:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphs = xText->createEnumeration();
    xParagraphs->nextElement();
    xParagraphs->nextElement();
    uno::Reference<beans::XPropertySet> xParagraph(xParagraphs->nextElement(), uno::UNO_QUERY);
    style::BreakType eBreakType{};
    xParagraph->getPropertyValue("BreakType") >>= eBreakType;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 4 (style::BreakType_PAGE_BEFORE)
    // - Actual  : 0 (style::BreakType_NONE)
    // i.e. the page break was lost.
    CPPUNIT_ASSERT_EQUAL(style::BreakType_PAGE_BEFORE, eBreakType);
}

CPPUNIT_TEST_FIXTURE(Test, test3NestedFloatingTables)
{
    // Given a document with nested tables: outer and inner one is normal, middle one is floating:
    // When laying out that document:
    loadFromFile(u"floattable-nested-3tables.docx");

    // Then make sure we don't crash and create the 3 tables:
    // Without the accompanying fix in place, this would have crashed, layout can't handle nested
    // floating tables currently.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xIndexAccess->getCount());
}

CPPUNIT_TEST_FIXTURE(Test, testFloatingTablesOuterNonsplitInner)
{
    // Given a document with a normal table, 3 outer floating tables and an inner floating table in
    // the last floating table:
    loadFromFile(u"floattable-outer-nonsplit-inner.docx");

    // When counting the floating tables in the document:
    uno::Reference<text::XTextFramesSupplier> xFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFrames(xFramesSupplier->getTextFrames(),
                                                    uno::UNO_QUERY);

    // Then make sure no floating tables are missing:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 4
    // - Actual  : 3
    // i.e. the inner floating table was not floating.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), xFrames->getCount());
}

CPPUNIT_TEST_FIXTURE(Test, testDOCXFloatingTableHiddenAnchor)
{
    // Given a document with a floating table, anchored in a paragraph that is hidden:
    loadFromFile(u"floattable-hidden-anchor.docx");

    // When checking the visibility of the anchor paragraph:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphs = xText->createEnumeration();
    uno::Reference<beans::XPropertySet> xAnchor(xParagraphs->nextElement(), uno::UNO_QUERY);

    // Then make sure the anchor (and thus the table) is visible:
    bool bCharHidden{};
    CPPUNIT_ASSERT(xAnchor->getPropertyValue("CharHidden") >>= bCharHidden);
    // Without the accompanying fix in place, this test would have failed, the paragraph + table was
    // hidden.
    CPPUNIT_ASSERT(!bCharHidden);
}

CPPUNIT_TEST_FIXTURE(Test, testDOCXFloatingTableNested)
{
    // Given a document with nested, multi-page floating tables:
    // When loading that document:
    loadFromFile(u"floattable-nested.docx");

    // Then make sure that both floating tables are allowed to split:
    uno::Reference<text::XTextFramesSupplier> xFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFrames(xFramesSupplier->getTextFrames(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xFrames->getCount());
    // Outer frame:
    uno::Reference<beans::XPropertySet> xFrame1;
    xFrames->getByIndex(0) >>= xFrame1;
    bool bIsSplitAllowed = false;
    xFrame1->getPropertyValue("IsSplitAllowed") >>= bIsSplitAllowed;
    CPPUNIT_ASSERT(bIsSplitAllowed);
    // Inner frame:
    uno::Reference<beans::XPropertySet> xFrame2;
    xFrames->getByIndex(1) >>= xFrame2;
    bIsSplitAllowed = false;
    xFrame2->getPropertyValue("IsSplitAllowed") >>= bIsSplitAllowed;
    // Without the accompanying fix in place, this test would have failed, the inner frame could not
    // split.
    CPPUNIT_ASSERT(bIsSplitAllowed);
}

CPPUNIT_TEST_FIXTURE(Test, testDOCXFloatingTableHeader)
{
    // Given a document with a header that has a floating table and some large images in the body
    // text:
    loadFromFile(u"floattable-header.docx");

    // When breaking that document into pages:
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    xCursor->jumpToLastPage();

    // Then make sure we get 2 pages:
    sal_Int32 nLastPage = xCursor->getPage();
    // Without the accompanying fix in place, this test would have failed, the page count went to
    // 2233 pages and then there was a layout loop.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), nLastPage);
}

CPPUNIT_TEST_FIXTURE(Test, testDOCXFloatingTableFootnoteRedline)
{
    // Given a document with a floating table in a footnote, with track changes recorded (but not
    // visible):
    // When importing that document:
    loadFromFile(u"floattable-footnote-redline.docx");

    // Then make sure the table is imported as inline, till Writer layout is ready to handle
    // floating tables in footnotes:
    uno::Reference<drawing::XDrawPageSupplier> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xModel->getDrawPage();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), xDrawPage->getCount());
}

CPPUNIT_TEST_FIXTURE(Test, testDOCXFloatingTableHeaderBodyOverlap)
{
    // Given a document with a floating table in a header, the floating table extends the header
    // frame:
    // When importing that document:
    loadFromFile(u"floattable-header-overlap.docx");

    // Then make sure the fly bottom is less than the top of the body text:
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    css::uno::Reference<qa::XDumper> xDumper(xModel->getCurrentController(), uno::UNO_QUERY);
    OString aDump = xDumper->dump("layout").toUtf8();
    auto pCharBuffer = reinterpret_cast<const xmlChar*>(aDump.getStr());
    xmlDocUniquePtr pXmlDoc(xmlParseDoc(pCharBuffer));
    sal_Int32 nFlyBottom = getXPath(pXmlDoc, "//fly/infos/bounds"_ostr, "bottom"_ostr).toInt32();
    // Body text top is body top + height of the first line, that's just a fly portion (kind of a
    // top margin).
    sal_Int32 nBodyTop
        = getXPath(pXmlDoc, "//page[1]/body/txt[1]/infos/bounds"_ostr, "top"_ostr).toInt32();
    // Without the accompanying fix in place, this test would have failed, the first line was not a
    // fly portion but it was actual text, above the floating table.
    assertXPath(pXmlDoc, "//page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]/child::*"_ostr,
                "type"_ostr, "PortionType::Fly");
    sal_Int32 nBodyFlyPortionHeight
        = getXPath(pXmlDoc, "//page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]"_ostr,
                   "height"_ostr)
              .toInt32();
    sal_Int32 nBodyTextTop = nBodyTop + nBodyFlyPortionHeight;
    // Fly bottom was 3063, body text top was 7148.
    CPPUNIT_ASSERT_LESS(nBodyTextTop, nFlyBottom);
}

CPPUNIT_TEST_FIXTURE(Test, testTableStyleParaBorderSpacing)
{
    // Given a document with a table style, table style defines top and bottom border for
    // paragraphs:
    // When loading that document:
    loadFromFile(u"table-style-para-border-spacing.docx");

    // Then make sure the in-table paragraph gets its top border:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xText->createEnumeration();
    uno::Reference<text::XTextTable> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xCell(xPara->getCellByName("A1"), uno::UNO_QUERY);
    xParaEnum = xCell->createEnumeration();
    uno::Reference<beans::XPropertySet> xParaProps(xParaEnum->nextElement(), uno::UNO_QUERY);
    sal_Int32 nTopBorderDistance{};
    xParaProps->getPropertyValue("TopBorderDistance") >>= nTopBorderDistance;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 35
    // - Actual  : 0
    // i.e. the top and bottom border and its 1pt spacing was not set on the in-table paragraph.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(35), nTopBorderDistance);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
