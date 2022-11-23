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
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xFrame(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    bool bIsFollowingTextFlow = false;
    xFrame->getPropertyValue("IsFollowingTextFlow") >>= bIsFollowingTextFlow;
    // Without the accompanying fix in place, this test would have failed, the nested floating table
    // was partly positioned outside the table cell, leading to overlapping text.
    CPPUNIT_ASSERT(bIsFollowingTextFlow);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
