/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <test/table/xcellcursor.hxx>

#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XSheetCellCursor.hpp>
#include <com/sun/star/sheet/XSheetCellRange.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/XCellCursor.hpp>
#include <com/sun/star/table/XCellRange.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void XCellCursor::testGoToNext()
{
    uno::Reference<table::XCellCursor> xCellCursor(init(), UNO_QUERY_THROW);

    uno::Reference<sheet::XCellRangeAddressable> xCellRangeAddressable(xCellCursor,
                                                                       UNO_QUERY_THROW);
    table::CellRangeAddress aCellRangeAddr = xCellRangeAddressable->getRangeAddress();
    const sal_Int32 startCol = aCellRangeAddr.StartColumn;

    xCellCursor->gotoNext();

    aCellRangeAddr = xCellRangeAddressable->getRangeAddress();
    const sal_Int32 startCol2 = aCellRangeAddr.StartColumn;

    CPPUNIT_ASSERT_MESSAGE("Successfully able to go to Next", startCol != startCol2);
}

void XCellCursor::testGoToOffset()
{
    uno::Reference<table::XCellCursor> xCellCursor(init(), UNO_QUERY_THROW);

    uno::Reference<sheet::XCellRangeAddressable> xCellRangeAddressable(xCellCursor,
                                                                       UNO_QUERY_THROW);
    table::CellRangeAddress aCellRangeAddr = xCellRangeAddressable->getRangeAddress();
    const sal_Int32 startRow = aCellRangeAddr.StartRow;
    const sal_Int32 startCol = aCellRangeAddr.StartColumn;

    xCellCursor->gotoOffset(4, 4);

    aCellRangeAddr = xCellRangeAddressable->getRangeAddress();
    const sal_Int32 startRow2 = aCellRangeAddr.StartRow;
    const sal_Int32 startCol2 = aCellRangeAddr.StartColumn;

    CPPUNIT_ASSERT_MESSAGE("Successfully able to go to Offset",
                           (startCol != startCol2) || (startRow == startRow2));
}

void XCellCursor::testGoToPrevious()
{
    uno::Reference<table::XCellCursor> xCellCursor(init(), UNO_QUERY_THROW);

    uno::Reference<sheet::XCellRangeAddressable> xCellRangeAddressable(xCellCursor,
                                                                       UNO_QUERY_THROW);
    xCellCursor->gotoOffset(4, 4);

    table::CellRangeAddress aCellRangeAddr = xCellRangeAddressable->getRangeAddress();
    const sal_Int32 startCol = aCellRangeAddr.StartColumn;

    xCellCursor->gotoPrevious();

    aCellRangeAddr = xCellRangeAddressable->getRangeAddress();
    const sal_Int32 startCol2 = aCellRangeAddr.StartColumn;
    CPPUNIT_ASSERT_MESSAGE("Successfully able to go to Previous", startCol != startCol2);
}

void XCellCursor::testGoToStart()
{
    uno::Reference<table::XCellCursor> xCellCursor(init(), UNO_QUERY_THROW);

    uno::Reference<sheet::XCellRangeAddressable> xCellRangeAddressable(xCellCursor,
                                                                       UNO_QUERY_THROW);
    xCellCursor->gotoStart();

    table::CellRangeAddress aCellRangeAddr = xCellRangeAddressable->getRangeAddress();

    const sal_Int32 startRow = aCellRangeAddr.StartRow;
    const sal_Int32 startCol = aCellRangeAddr.StartColumn;
    const sal_Int32 endRow = aCellRangeAddr.EndRow;
    const sal_Int32 endCol = aCellRangeAddr.EndColumn;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Successfully able to go to Start", startCol, endCol);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Successfully able to go to Start", endRow, startRow);
}

void XCellCursor::testGoToEnd()
{
    uno::Reference<table::XCellCursor> xCellCursor(init(), UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSpreadsheet(getXSpreadsheet(), UNO_QUERY_THROW);
    uno::Reference<table::XCellRange> xCellRange(xCellCursor, UNO_QUERY_THROW);
    xCellRange = xSpreadsheet->getCellRangeByName("$A$1:$g$7");
    uno::Reference<sheet::XSheetCellRange> xSheetCellRange(xCellCursor, UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetCellCursor> xSheetCellCursor(xCellCursor, UNO_QUERY_THROW);
    xSheetCellCursor = xSpreadsheet->createCursorByRange(xSheetCellRange);
    uno::Reference<sheet::XCellRangeAddressable> xCellRangeAddressable(xCellCursor,
                                                                       UNO_QUERY_THROW);

    xCellCursor->gotoEnd();

    table::CellRangeAddress aCellRangeAddr = xCellRangeAddressable->getRangeAddress();
    const sal_Int32 startRow = aCellRangeAddr.StartRow;
    const sal_Int32 startCol = aCellRangeAddr.StartColumn;
    const sal_Int32 endRow = aCellRangeAddr.EndRow;
    const sal_Int32 endCol = aCellRangeAddr.EndColumn;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Successfully able to go to End", startCol, endCol);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Successfully able to go to End", endRow, startRow);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
