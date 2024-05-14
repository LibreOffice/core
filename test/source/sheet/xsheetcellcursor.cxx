/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xsheetcellcursor.hxx>

#include <com/sun/star/sheet/XArrayFormulaRange.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XSheetCellCursor.hpp>
#include <com/sun/star/sheet/XSheetOperation.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/util/XMergeable.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void XSheetCellCursor::testCollapseToCurrentArray()
{
    uno::Reference<sheet::XSheetCellCursor> xSheetCellCursor(init(), UNO_QUERY_THROW);

    uno::Reference<sheet::XCellRangeAddressable> xCellRangeAddressable(xSheetCellCursor,
                                                                       UNO_QUERY_THROW);
    table::CellRangeAddress aCellRangeAddr = xCellRangeAddressable->getRangeAddress();
    const sal_Int32 nHeight = aCellRangeAddr.EndRow - aCellRangeAddr.StartRow + 1;

    uno::Reference<table::XCellRange> xCellRange
        = xSheetCellCursor->getCellRangeByPosition(0, 0, 0, nHeight - 1);
    uno::Reference<sheet::XArrayFormulaRange> xArrayFormulaRange(xCellRange, UNO_QUERY_THROW);
    xArrayFormulaRange->setArrayFormula("A1:A" + OUString::number(nHeight));

    xSheetCellCursor->collapseToSize(1, 1);
    xSheetCellCursor->collapseToCurrentArray();

    uno::Reference<table::XColumnRowRange> xColRowRange(xSheetCellCursor, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to collapseToCurrentArray (cols)", sal_Int32(1),
                                 xColRowRange->getColumns()->getCount());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to collapseToCurrentArray (rows)", nHeight,
                                 xColRowRange->getRows()->getCount());
    xArrayFormulaRange->setArrayFormula(u""_ustr);
}

void XSheetCellCursor::testCollapseToCurrentRegion()
{
    uno::Reference<sheet::XSheetCellCursor> xSheetCellCursor(init(), UNO_QUERY_THROW);

    const sal_Int32 nWidth = 4, nHeight = 4;
    uno::Reference<sheet::XSpreadsheet> xSheet = xSheetCellCursor->getSpreadsheet();
    uno::Reference<sheet::XSheetOperation> xSheetOp(xSheet, UNO_QUERY_THROW);
    xSheetOp->clearContents(65535);

    xSheetCellCursor->collapseToCurrentRegion();
    uno::Reference<table::XColumnRowRange> xColRowRange(xSheetCellCursor, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to collapseToCurrentRegion (cols)", nWidth,
                                 xColRowRange->getColumns()->getCount());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to collapseToCurrentRegion (rows)", nHeight,
                                 xColRowRange->getRows()->getCount());
}

void XSheetCellCursor::testCollapseToMergedArea()
{
    uno::Reference<sheet::XSheetCellCursor> xSheetCellCursor(init(), UNO_QUERY_THROW);
    xSheetCellCursor->collapseToSize(1, 1);

    const sal_Int32 nLeftCol = 0, nTopRow = 0, nWidth = 8, nHeight = 8;
    uno::Reference<sheet::XSpreadsheet> xSheet = xSheetCellCursor->getSpreadsheet();

    uno::Reference<table::XCellRange> xCellRange = xSheet->getCellRangeByPosition(
        nLeftCol + nWidth - 8, nTopRow + nHeight - 8, nLeftCol + nWidth, nTopRow + nHeight);

    uno::Reference<util::XMergeable> xMergeable(xCellRange, UNO_QUERY_THROW);
    xMergeable->merge(true);
    CPPUNIT_ASSERT_MESSAGE("Unable to merge area", xMergeable->getIsMerged());
    xSheetCellCursor->collapseToMergedArea();
    xMergeable->merge(false);

    uno::Reference<table::XColumnRowRange> xColRowRange(xSheetCellCursor, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to collapseToMergedArea (cols)", nWidth + 1,
                                 xColRowRange->getColumns()->getCount());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to collapseToMergedArea (rows)", nHeight + 1,
                                 xColRowRange->getRows()->getCount());
}

void XSheetCellCursor::testCollapseToSize()
{
    uno::Reference<sheet::XSheetCellCursor> xSheetCellCursor(init(), UNO_QUERY_THROW);

    const sal_Int32 nWidth = 1, nHeight = 1;
    xSheetCellCursor->collapseToSize(nWidth + 3, nHeight + 3);

    uno::Reference<table::XColumnRowRange> xColRowRange(xSheetCellCursor, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to collapseToSize (cols)", nWidth + 3,
                                 xColRowRange->getColumns()->getCount());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to collapseToSize (rows)", nHeight + 3,
                                 xColRowRange->getRows()->getCount());
}

void XSheetCellCursor::testExpandToEntireColumns()
{
    uno::Reference<sheet::XSheetCellCursor> xSheetCellCursor(init(), UNO_QUERY_THROW);

    xSheetCellCursor->expandToEntireColumns();

    uno::Reference<table::XColumnRowRange> xColRowRange(xSheetCellCursor, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to expandToEntireColumns (cols)", sal_Int32(4),
                                 xColRowRange->getColumns()->getCount());
    CPPUNIT_ASSERT_MESSAGE("Unable to expandToEntireColumns (rows)",
                           xColRowRange->getRows()->getCount() >= sal_Int32(32000));
}

void XSheetCellCursor::testExpandToEntireRows()
{
    uno::Reference<sheet::XSheetCellCursor> xSheetCellCursor(init(), UNO_QUERY_THROW);

    xSheetCellCursor->expandToEntireRows();

    uno::Reference<table::XColumnRowRange> xColRowRange(xSheetCellCursor, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Unable to expandToEntireRows (cols)",
                           xColRowRange->getColumns()->getCount() >= sal_Int32(256));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to expandToEntireRows (rows)", sal_Int32(4),
                                 xColRowRange->getRows()->getCount());
}
}

/* vim:set shiftnWidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
