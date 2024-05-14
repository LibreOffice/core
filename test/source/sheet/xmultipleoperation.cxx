/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xmultipleoperation.hxx>

#include <com/sun/star/sheet/TableOperationMode.hpp>
#include <com/sun/star/sheet/XCellAddressable.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XMultipleOperation.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XMultipleOperation::testSetTableOperation()
{
    uno::Reference<sheet::XMultipleOperation> xMultipleOperation(init(), UNO_QUERY_THROW);

    uno::Reference<sheet::XSpreadsheet> xSheet(getXSpreadsheet(), UNO_QUERY_THROW);
    uno::Reference<table::XCellRange> xCellRange(xSheet->getCellRangeByName(u"$A$20:$A$20"_ustr),
                                                 UNO_SET_THROW);
    uno::Reference<sheet::XCellRangeAddressable> xCellRangeAddr(xCellRange, UNO_QUERY_THROW);

    uno::Reference<table::XCell> xCell = xSheet->getCellByPosition(0, 19);
    xCell->setFormula(u"=a18+a19"_ustr);

    uno::Reference<table::XCell> xCell1 = xSheet->getCellByPosition(0, 17);
    uno::Reference<sheet::XCellAddressable> xCellAddr1(xCell1, UNO_QUERY_THROW);

    uno::Reference<table::XCell> xCell2 = xSheet->getCellByPosition(0, 18);
    uno::Reference<sheet::XCellAddressable> xCellAddr2(xCell2, UNO_QUERY_THROW);

    fillCells(xSheet);
    xMultipleOperation->setTableOperation(
        xCellRangeAddr->getRangeAddress(), sheet::TableOperationMode_ROW,
        xCellAddr1->getCellAddress(), xCellAddr2->getCellAddress());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Check cell at position 1,1 (OpMode: ROW)", 5.0,
                                 xSheet->getCellByPosition(1, 1)->getValue());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Check cell at position 2,1 (OpMode: ROW)", 10.0,
                                 xSheet->getCellByPosition(2, 1)->getValue());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Check cell at position 3,1 (OpMode: ROW)", 15.0,
                                 xSheet->getCellByPosition(3, 1)->getValue());

    fillCells(xSheet);
    xMultipleOperation->setTableOperation(
        xCellRangeAddr->getRangeAddress(), sheet::TableOperationMode_COLUMN,
        xCellAddr1->getCellAddress(), xCellAddr2->getCellAddress());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Check cell at position 1,1 (OpMode: COLUMN)", 12.0,
                                 xSheet->getCellByPosition(1, 1)->getValue());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Check cell at position 1,2 (OpMode: COLUMN)", 24.0,
                                 xSheet->getCellByPosition(1, 2)->getValue());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Check cell at position 1,3 (OpMode: COLUMN)", 36.0,
                                 xSheet->getCellByPosition(1, 3)->getValue());

    fillCells(xSheet);
    xMultipleOperation->setTableOperation(
        xCellRangeAddr->getRangeAddress(), sheet::TableOperationMode_BOTH,
        xCellAddr1->getCellAddress(), xCellAddr2->getCellAddress());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Check cell at position 1,1 (OpMode: BOTH)", 17.0,
                                 xSheet->getCellByPosition(1, 1)->getValue());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Check cell at position 2,2 (OpMode: BOTH)", 34.0,
                                 xSheet->getCellByPosition(2, 2)->getValue());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Check cell at position 3,3 (OpMode: BOTH)", 51.0,
                                 xSheet->getCellByPosition(3, 3)->getValue());
}

void XMultipleOperation::fillCells(uno::Reference<sheet::XSpreadsheet> const& xSheet)
{
    for (unsigned int i = 1; i < 5; i++)
    {
        uno::Reference<table::XCell> xCellFill = xSheet->getCellByPosition(0, i);
        xCellFill->setValue(i * 12);
        xCellFill = xSheet->getCellByPosition(i, 0);
        xCellFill->setValue(i * 5);
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
