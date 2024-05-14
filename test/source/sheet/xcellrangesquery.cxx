/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xcellrangesquery.hxx>

#include <com/sun/star/sheet/XCellRangesQuery.hpp>
#include <com/sun/star/sheet/XSheetCellRanges.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/sheet/CellFlags.hpp>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;


namespace apitest {

void XCellRangesQuery::testQueryColumnDifference()
{
    uno::Reference<sheet::XCellRangesQuery> xCellRangesQuery(init(),UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetCellRanges> xRanges = xCellRangesQuery->queryColumnDifferences(table::CellAddress(0, 1, 1));
    OUString aResult = xRanges->getRangeAddressesAsString();
    std::cout << "testQueryColumnDifference: Result: " << aResult << std::endl;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("testQueryColumnDifference", u"Sheet1.B1:C1,Sheet1.B3:C5"_ustr, aResult);
}

void XCellRangesQuery::testQueryContentDifference()
{
    uno::Reference<sheet::XCellRangesQuery> xCellRangesQuery(init(),UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetCellRanges> xRanges = xCellRangesQuery->queryContentCells(sheet::CellFlags::VALUE);
    OUString aResult = xRanges->getRangeAddressesAsString();
    std::cout << "testQueryContentDifference: Result: " << aResult << std::endl;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("testQueryContentDifference", u"Sheet1.B3,Sheet1.C2"_ustr, aResult);
}

void XCellRangesQuery::testQueryEmptyCells()
{
    uno::Reference<sheet::XCellRangesQuery> xCellRangesQuery(init(),UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetCellRanges> xRanges = xCellRangesQuery->queryEmptyCells();
    OUString aResult = xRanges->getRangeAddressesAsString();
    std::cout << "testQueryEmptyCells: Result: " << aResult << std::endl;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("testQueryEmptyCells", u"Sheet1.A1:A5,Sheet1.B1:C1,Sheet1.B5,Sheet1.C3:C5,Sheet1.D1:E5"_ustr, aResult);
}

void XCellRangesQuery::testQueryFormulaCells()
{
    uno::Reference<sheet::XCellRangesQuery> xCellRangesQuery(init(),UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetCellRanges> xRanges = xCellRangesQuery->queryFormulaCells(sheet::CellFlags::FORMULA);
    OUString aResult = xRanges->getRangeAddressesAsString();
    std::cout << "testQueryFormulaCells: Result: " << aResult << std::endl;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("testQueryFormulaCells", u"Sheet1.B2"_ustr, aResult);
}

void XCellRangesQuery::testQueryIntersection()
{
    uno::Reference<sheet::XCellRangesQuery> xCellRangesQuery(init(),UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetCellRanges> xRanges = xCellRangesQuery->queryIntersection(table::CellRangeAddress(0,3,3,7,7));
    OUString aResult = xRanges->getRangeAddressesAsString();
    std::cout << "testQueryIntersection: Result: " << aResult << std::endl;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("testQueryIntersection", u"Sheet1.D4:E5"_ustr, aResult);
}

void XCellRangesQuery::testQueryRowDifference()
{
    uno::Reference<sheet::XCellRangesQuery> xCellRangesQuery(init(),UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetCellRanges> xRanges = xCellRangesQuery->queryRowDifferences(table::CellAddress(0,1,1));
    OUString aResult = xRanges->getRangeAddressesAsString();
    std::cout << "testQueryRowDifference: Result: " << aResult << std::endl;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("testQueryRowDifference", u"Sheet1.A2:A4,Sheet1.C2:E4"_ustr, aResult);
}

void XCellRangesQuery::testQueryVisibleCells()
{
    uno::Reference<sheet::XCellRangesQuery> xCellRangesQuery(init(),UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetCellRanges> xRanges = xCellRangesQuery->queryVisibleCells();
    OUString aResult = xRanges->getRangeAddressesAsString();
    std::cout << "testQueryVisibleCells: Result: " << aResult << std::endl;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("testQueryVisibleCells", u"Sheet1.A1:E5"_ustr, aResult);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
