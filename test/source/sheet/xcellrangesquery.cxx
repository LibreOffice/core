/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * [ Copyright (C) 2011 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer) ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <test/sheet/xcellrangesquery.hxx>

#include <com/sun/star/sheet/XCellRangesQuery.hpp>
#include <com/sun/star/sheet/XSheetCellRanges.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/sheet/CellFlags.hpp>

#include "cppunit/extensions/HelperMacros.h"

using namespace com::sun::star;
using namespace com::sun::star::uno;


namespace apitest {

void XCellRangesQuery::testQueryColumnDifference()
{
    rtl::OUString aExpected(RTL_CONSTASCII_USTRINGPARAM("Sheet1.B1:C1,Sheet1.B3:C5"));
    uno::Reference<sheet::XCellRangesQuery> xCellRangesQuery(init(),UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetCellRanges> xRanges = xCellRangesQuery->queryColumnDifferences(table::CellAddress(0, 1, 1));
    rtl::OUString aResult = xRanges->getRangeAddressesAsString();
    std::cout << "testQueryColumnDifference: Result: " << rtl::OUStringToOString(aResult, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
    CPPUNIT_ASSERT_MESSAGE("testQueryColumnDifference", aResult == aExpected);
}

void XCellRangesQuery::testQueryContentDifference()
{
    rtl::OUString aExpected(RTL_CONSTASCII_USTRINGPARAM("Sheet1.B2:B3"));
    uno::Reference<sheet::XCellRangesQuery> xCellRangesQuery(init(),UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetCellRanges> xRanges = xCellRangesQuery->queryContentCells(sheet::CellFlags::VALUE);
    rtl::OUString aResult = xRanges->getRangeAddressesAsString();
    std::cout << "testQueryContentDifference: Result: " << rtl::OUStringToOString(aResult, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
    CPPUNIT_ASSERT_MESSAGE("testQueryContentDifference", aResult == aExpected);
}

void XCellRangesQuery::testQueryEmptyCells()
{
    rtl::OUString aExpected(RTL_CONSTASCII_USTRINGPARAM("Sheet1.A1:A5,Sheet1.B1:C1,Sheet1.B5,Sheet1.C3:C5,Sheet1.D1:D5"));
    uno::Reference<sheet::XCellRangesQuery> xCellRangesQuery(init(),UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetCellRanges> xRanges = xCellRangesQuery->queryEmptyCells();
    rtl::OUString aResult = xRanges->getRangeAddressesAsString();
    std::cout << "testQueryEmptyCells: Result: " << rtl::OUStringToOString(aResult, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
    CPPUNIT_ASSERT_MESSAGE("testQueryEmptyCells", aResult == aExpected);
}

void XCellRangesQuery::testQueryFormulaCells()
{
    rtl::OUString aExpected(RTL_CONSTASCII_USTRINGPARAM("Sheet1.C2"));
    uno::Reference<sheet::XCellRangesQuery> xCellRangesQuery(init(),UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetCellRanges> xRanges = xCellRangesQuery->queryFormulaCells(sheet::CellFlags::FORMULA);
    rtl::OUString aResult = xRanges->getRangeAddressesAsString();
    std::cout << "testQueryFormulaCells: Result: " << rtl::OUStringToOString(aResult, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
    CPPUNIT_ASSERT_MESSAGE("testQueryFormulaCells", aResult == aExpected);
}

void XCellRangesQuery::testQueryIntersection()
{
    rtl::OUString aExpected(RTL_CONSTASCII_USTRINGPARAM("Sheet1.D4:D5"));
    uno::Reference<sheet::XCellRangesQuery> xCellRangesQuery(init(),UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetCellRanges> xRanges = xCellRangesQuery->queryIntersection(table::CellRangeAddress(0,3,3,7,7));
    rtl::OUString aResult = xRanges->getRangeAddressesAsString();
    std::cout << "testQueryIntersection: Result: " << rtl::OUStringToOString(aResult, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
    CPPUNIT_ASSERT_MESSAGE("testQueryFormulaCells", aResult == aExpected);
}

void XCellRangesQuery::testQueryRowDifference()
{
    rtl::OUString aExpected(RTL_CONSTASCII_USTRINGPARAM("Sheet1.A2:A4,Sheet1.C2:D4"));
    uno::Reference<sheet::XCellRangesQuery> xCellRangesQuery(init(),UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetCellRanges> xRanges = xCellRangesQuery->queryRowDifferences(table::CellAddress(0,1,1));
    rtl::OUString aResult = xRanges->getRangeAddressesAsString();
    std::cout << "testQueryRowDifference: Result: " << rtl::OUStringToOString(aResult, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
    CPPUNIT_ASSERT_MESSAGE("testQueryFormulaCells", aResult == aExpected);
}

void XCellRangesQuery::testQueryVisibleCells()
{
    rtl::OUString aExpected(RTL_CONSTASCII_USTRINGPARAM("Sheet1.A1:D5"));
    uno::Reference<sheet::XCellRangesQuery> xCellRangesQuery(init(),UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetCellRanges> xRanges = xCellRangesQuery->queryVisibleCells();
    rtl::OUString aResult = xRanges->getRangeAddressesAsString();
    std::cout << "testQueryVisibleCells: Result: " << rtl::OUStringToOString(aResult, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
    CPPUNIT_ASSERT_MESSAGE("testQueryFormulaCells", aResult == aExpected);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
