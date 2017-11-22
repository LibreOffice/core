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

#include <cppunit/extensions/HelperMacros.h>

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
    const sal_Int32 height = aCellRangeAddr.EndRow - aCellRangeAddr.StartRow + 1;

    uno::Reference<table::XCellRange> xCellRange
        = xSheetCellCursor->getCellRangeByPosition(0, 0, 0, height - 1);
    uno::Reference<sheet::XArrayFormulaRange> xArrayFormulaRange(xCellRange, UNO_QUERY_THROW);
    xArrayFormulaRange->setArrayFormula("A1:A" + OUString::number(height));

    xSheetCellCursor->collapseToSize(1, 1);
    xSheetCellCursor->collapseToCurrentArray();

    uno::Reference<table::XColumnRowRange> xColRowRange(xSheetCellCursor, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to collapseToCurrentArray (cols)", sal_Int32(1),
                                 xColRowRange->getColumns()->getCount());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to collapseToCurrentArray (rows)", height,
                                 xColRowRange->getRows()->getCount());
    xArrayFormulaRange->setArrayFormula("");
}

void XSheetCellCursor::testCollapseToCurrentRegion()
{
    uno::Reference<sheet::XSheetCellCursor> xSheetCellCursor(init(), UNO_QUERY_THROW);

    const sal_Int32 width = 4, height = 4;
    uno::Reference<sheet::XSpreadsheet> xSheet = xSheetCellCursor->getSpreadsheet();
    uno::Reference<sheet::XSheetOperation> xSheetOp(xSheet, UNO_QUERY_THROW);
    xSheetOp->clearContents(65535);

    xSheetCellCursor->collapseToCurrentRegion();
    uno::Reference<table::XColumnRowRange> xColRowRange(xSheetCellCursor, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to collapseToCurrentRegion (cols)", width,
                                 xColRowRange->getColumns()->getCount());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to collapseToCurrentRegion (rows)", height,
                                 xColRowRange->getRows()->getCount());

    xSheetCellCursor->collapseToSize(width, height);
}

void XSheetCellCursor::testCollapseToMergedArea()
{
    uno::Reference<sheet::XSheetCellCursor> xSheetCellCursor(init(), UNO_QUERY_THROW);

    const sal_Int32 leftCol = 0, topRow = 0, width = 1, height = 1;
    uno::Reference<sheet::XSpreadsheet> xSheet = xSheetCellCursor->getSpreadsheet();
    uno::Reference<table::XCellRange> xCellRange = xSheet->getCellRangeByPosition(
        leftCol + width - 1, topRow + height - 1, leftCol + width, topRow + height);

    uno::Reference<util::XMergeable> xMergeable(xCellRange, UNO_QUERY_THROW);
    xMergeable->merge(true);
    CPPUNIT_ASSERT_MESSAGE("Unable to merge area", xMergeable->getIsMerged());
    xSheetCellCursor->collapseToMergedArea();
    xMergeable->merge(false);

    uno::Reference<table::XColumnRowRange> xColRowRange(xSheetCellCursor, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to collapseToMergedArea (cols)", width + 1,
                                 xColRowRange->getColumns()->getCount());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to collapseToMergedArea (rows)", height + 3,
                                 xColRowRange->getRows()->getCount());

    xSheetCellCursor->collapseToSize(width, height);
}

void XSheetCellCursor::testCollapseToSize()
{
    uno::Reference<sheet::XSheetCellCursor> xSheetCellCursor(init(), UNO_QUERY_THROW);

    const sal_Int32 width = 1, height = 1;
    xSheetCellCursor->collapseToSize(width + 3, height + 3);

    uno::Reference<table::XColumnRowRange> xColRowRange(xSheetCellCursor, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to collapseToSize (cols)", width + 3,
                                 xColRowRange->getColumns()->getCount());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to collapseToSize (rows)", height + 3,
                                 xColRowRange->getRows()->getCount());

    xSheetCellCursor->collapseToSize(width, height);
}

void XSheetCellCursor::testExpandToEntireColumns()
{
    uno::Reference<sheet::XSheetCellCursor> xSheetCellCursor(init(), UNO_QUERY_THROW);

    xSheetCellCursor->expandToEntireColumns();

    uno::Reference<table::XColumnRowRange> xColRowRange(xSheetCellCursor, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to expandToEntireColumns (cols)", 1,
                                 xColRowRange->getColumns()->getCount());
    CPPUNIT_ASSERT_MESSAGE("Unable to expandToEntireColumns (rows)",
                           xColRowRange->getRows()->getCount() >= 32000);

    xSheetCellCursor->collapseToSize(1, 1);
}

void XSheetCellCursor::testExpandToEntireRows()
{
    uno::Reference<sheet::XSheetCellCursor> xSheetCellCursor(init(), UNO_QUERY_THROW);

    xSheetCellCursor->expandToEntireRows();

    uno::Reference<table::XColumnRowRange> xColRowRange(xSheetCellCursor, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Unable to expandToEntireColumns (cols)",
                           xColRowRange->getColumns()->getCount() >= 256);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to expandToEntireColumns (rows)", 1,
                                 xColRowRange->getRows()->getCount());

    xSheetCellCursor->collapseToSize(1, 1);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
