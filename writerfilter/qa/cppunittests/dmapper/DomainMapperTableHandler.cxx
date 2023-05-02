/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/style/BreakType.hpp>

using namespace ::com::sun::star;

namespace
{
/// Tests for writerfilter/source/dmapper/DomainMapperTableHandler.cxx.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest("/writerfilter/qa/cppunittests/dmapper/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, test1cellInsidevRightborder)
{
    loadFromURL(u"1cell-insidev-rightborder.docx");
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
    loadFromURL(u"nested-floating-table.docx");
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
    loadFromURL(u"floattable-break-before.docx");

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
    loadFromURL(u"floattable-nested-3tables.docx");

    // Then make sure we don't crash and create the 3 tables:
    // Without the accompanying fix in place, this would have crashed, layout can't handle nested
    // floating tables currently.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xIndexAccess->getCount());
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
