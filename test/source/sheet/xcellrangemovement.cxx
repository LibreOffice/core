/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xcellrangemovement.hxx>

#include <com/sun/star/sheet/CellDeleteMode.hpp>
#include <com/sun/star/sheet/CellInsertMode.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XCellRangeMovement.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void XCellRangeMovement::testInsertCells()
{
    uno::Reference<sheet::XCellRangeMovement> xCRM(init(), UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xCRM, UNO_QUERY_THROW);

    uno::Reference<sheet::XCellRangeAddressable> xCRA(xCRM, UNO_QUERY_THROW);
    const sal_Int16 nSheet = xCRA->getRangeAddress().Sheet;

    xSheet->getCellByPosition(0, 20)->setValue(100);
    xSheet->getCellByPosition(1, 20)->setValue(100);
    xSheet->getCellByPosition(2, 20)->setValue(100);
    xSheet->getCellByPosition(3, 20)->setValue(100);
    xSheet->getCellByPosition(0, 21)->setValue(200);
    xSheet->getCellByPosition(1, 21)->setValue(200);
    xSheet->getCellByPosition(2, 21)->setValue(200);
    xSheet->getCellByPosition(3, 21)->setValue(200);

    table::CellRangeAddress aSrcCellRangeAddr(nSheet, 0, 21, 5, 21);
    xCRM->insertCells(aSrcCellRangeAddr, sheet::CellInsertMode_DOWN);

    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Unable to insert cells", 0.0,
                                         xSheet->getCellByPosition(1, 21)->getValue(), 0.0);
}

void XCellRangeMovement::testCopyRange()
{
    uno::Reference<sheet::XCellRangeMovement> xCRM(init(), UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xCRM, UNO_QUERY_THROW);

    xSheet->getCellByPosition(1, 1)->setValue(100);
    xSheet->getCellByPosition(1, 2)->setValue(200);
    xSheet->getCellByPosition(2, 1)->setValue(300);
    xSheet->getCellByPosition(2, 2)->setValue(400);

    uno::Reference<sheet::XCellRangeAddressable> xCRA(xCRM, UNO_QUERY_THROW);
    const sal_Int16 nSheet = xCRA->getRangeAddress().Sheet;

    table::CellRangeAddress aSrcCellRangeAddr(nSheet, 1, 1, 2, 2);
    table::CellAddress aDstCellAddr(nSheet, 1, 10);

    xCRM->copyRange(aDstCellAddr, aSrcCellRangeAddr);

    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Value was not copied from position 1,1 to 1,10", 100.0,
                                         xSheet->getCellByPosition(1, 10)->getValue(), 0.1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Value was not copied from position 1,2 to 1,11", 200.0,
                                         xSheet->getCellByPosition(1, 11)->getValue(), 0.1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Value was not copied from position 2,1 to 2,10", 300.0,
                                         xSheet->getCellByPosition(2, 10)->getValue(), 0.1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Value was not copied from position 2,2 to 2,11", 400.0,
                                         xSheet->getCellByPosition(2, 11)->getValue(), 0.1);
}
void XCellRangeMovement::testMoveRange()
{
    uno::Reference<sheet::XCellRangeMovement> xCRM(init(), UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xCRM, UNO_QUERY_THROW);

    xSheet->getCellByPosition(4, 0)->setValue(111);
    xSheet->getCellByPosition(4, 1)->setValue(222);

    uno::Reference<sheet::XCellRangeAddressable> xCRA(xCRM, UNO_QUERY_THROW);
    const sal_Int16 nSheet = xCRA->getRangeAddress().Sheet;

    table::CellRangeAddress aSrcCellRangeAddr(nSheet, 4, 0, 4, 1);
    table::CellAddress aDstCellAddr(nSheet, 4, 4);

    xCRM->moveRange(aDstCellAddr, aSrcCellRangeAddr);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Unable to move range", 333.0,
                                         xSheet->getCellByPosition(4, 4)->getValue()
                                             + xSheet->getCellByPosition(4, 5)->getValue(),
                                         0.0);
}
void XCellRangeMovement::testRemoveRange()
{
    uno::Reference<sheet::XCellRangeMovement> xCRM(init(), UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xCRM, UNO_QUERY_THROW);

    xSheet->getCellByPosition(5, 0)->setValue(333);
    xSheet->getCellByPosition(5, 1)->setValue(444);

    uno::Reference<sheet::XCellRangeAddressable> xCRA(xCRM, UNO_QUERY_THROW);
    const sal_Int16 nSheet = xCRA->getRangeAddress().Sheet;

    table::CellRangeAddress aSrcCellRangeAddr(nSheet, 5, 0, 5, 1);

    xCRM->removeRange(aSrcCellRangeAddr, sheet::CellDeleteMode_UP);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Unable to remove range", 0.0,
                                         xSheet->getCellByPosition(5, 0)->getValue()
                                             + xSheet->getCellByPosition(5, 1)->getValue(),
                                         0.0);
}
} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
