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

#include <test/unoapi_test.hxx>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XCellRangesQuery.hpp>
#include <com/sun/star/sheet/XSheetCellRanges.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/sheet/CellFlags.hpp>

using namespace com::sun::star;


namespace ScCellRangeObj {

class ScXCellRangesQuery : public UnoApiTest
{
public:
    ScXCellRangesQuery();

    uno::Reference<sheet::XCellRangesQuery> init();

    void setRowVisible(bool bVisible);

    //Testcases
    void testQueryColumnDifference();
    void testQueryContentDifference();
    void testQueryEmptyCells();
    void testQueryFormulaCells();
    void testQueryIntersection();
    void testQueryRowDifference();
    void testQueryVisibleCells();

    CPPUNIT_TEST_SUITE(ScXCellRangesQuery);
    CPPUNIT_TEST(testQueryColumnDifference);
    CPPUNIT_TEST(testQueryContentDifference);
    CPPUNIT_TEST(testQueryEmptyCells);
    //looks broken
    //CPPUNIT_TEST(testQueryFormulaCells);
    CPPUNIT_TEST(testQueryIntersection);
    CPPUNIT_TEST(testQueryRowDifference);
    CPPUNIT_TEST_SUITE_END();

};

ScXCellRangesQuery::ScXCellRangesQuery()
{

}

uno::Reference<sheet::XCellRangesQuery> ScXCellRangesQuery::init()
{
    rtl::OUString aFileURL;
    const rtl::OUString aFileBase(RTL_CONSTASCII_USTRINGPARAM("xcellrangesquery.ods"));
    createFileURL(aFileBase, aFileURL);
    std::cout << rtl::OUStringToOString(aFileURL, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
    static uno::Reference< lang::XComponent > xComponent;
    if( !xComponent.is())
        xComponent = loadFromDesktop(aFileURL);
    uno::Reference< sheet::XSpreadsheetDocument> xDoc (xComponent, UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(0), UNO_QUERY_THROW);

    CPPUNIT_ASSERT_MESSAGE("Could not create interface of type XSpreadsheet", xSheet.is());

    uno::Reference<sheet::XCellRangesQuery> xReturn(xSheet->getCellRangeByPosition(0,0,3,4), UNO_QUERY_THROW);

    CPPUNIT_ASSERT_MESSAGE("Could not create object of type XCellRangesQuery", xReturn.is());
    return xReturn;
}

void ScXCellRangesQuery::testQueryColumnDifference()
{
    rtl::OUString aExpected(RTL_CONSTASCII_USTRINGPARAM("Sheet1.B1:C1,Sheet1.B3:C5"));
    uno::Reference<sheet::XCellRangesQuery> xCellRangesQuery = init();
    uno::Reference<sheet::XSheetCellRanges> xRanges = xCellRangesQuery->queryColumnDifferences(table::CellAddress(0, 1, 1));
    rtl::OUString aResult = xRanges->getRangeAddressesAsString();
    std::cout << "testQueryColumnDifference: Result: " << rtl::OUStringToOString(aResult, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
    CPPUNIT_ASSERT_MESSAGE("testQueryColumnDifference", aResult == aExpected);
}

void ScXCellRangesQuery::testQueryContentDifference()
{
    rtl::OUString aExpected(RTL_CONSTASCII_USTRINGPARAM("Sheet1.B2:B3"));
    uno::Reference<sheet::XCellRangesQuery> xCellRangesQuery = init();
    uno::Reference<sheet::XSheetCellRanges> xRanges = xCellRangesQuery->queryContentCells(sheet::CellFlags::VALUE);
    rtl::OUString aResult = xRanges->getRangeAddressesAsString();
    std::cout << "testQueryContentDifference: Result: " << rtl::OUStringToOString(aResult, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
    CPPUNIT_ASSERT_MESSAGE("testQueryContentDifference", aResult == aExpected);
}

void ScXCellRangesQuery::testQueryEmptyCells()
{
    rtl::OUString aExpected(RTL_CONSTASCII_USTRINGPARAM("Sheet1.A1:A5,Sheet1.B1:C1,Sheet1.B5,Sheet1.C3:C5,Sheet1.D1:D5"));
    uno::Reference<sheet::XCellRangesQuery> xCellRangesQuery = init();
    uno::Reference<sheet::XSheetCellRanges> xRanges = xCellRangesQuery->queryEmptyCells();
    rtl::OUString aResult = xRanges->getRangeAddressesAsString();
    std::cout << "testQueryEmptyCells: Result: " << rtl::OUStringToOString(aResult, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
    CPPUNIT_ASSERT_MESSAGE("testQueryEmptyCells", aResult == aExpected);
}

void ScXCellRangesQuery::testQueryFormulaCells()
{
    rtl::OUString aExpected(RTL_CONSTASCII_USTRINGPARAM("Sheet1.C2"));
    uno::Reference<sheet::XCellRangesQuery> xCellRangesQuery = init();
    uno::Reference<sheet::XSheetCellRanges> xRanges = xCellRangesQuery->queryFormulaCells(sheet::CellFlags::FORMULA);
    rtl::OUString aResult = xRanges->getRangeAddressesAsString();
    std::cout << "testQueryFormulaCells: Result: " << rtl::OUStringToOString(aResult, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
    CPPUNIT_ASSERT_MESSAGE("testQueryFormulaCells", aResult == aExpected);
}

void ScXCellRangesQuery::testQueryIntersection()
{
    rtl::OUString aExpected(RTL_CONSTASCII_USTRINGPARAM("Sheet1.D4:D5"));
    uno::Reference<sheet::XCellRangesQuery> xCellRangesQuery = init();
    uno::Reference<sheet::XSheetCellRanges> xRanges = xCellRangesQuery->queryIntersection(table::CellRangeAddress(0,3,3,7,7));
    rtl::OUString aResult = xRanges->getRangeAddressesAsString();
    std::cout << "testQueryIntersection: Result: " << rtl::OUStringToOString(aResult, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
    CPPUNIT_ASSERT_MESSAGE("testQueryFormulaCells", aResult == aExpected);
}

void ScXCellRangesQuery::testQueryRowDifference()
{
    rtl::OUString aExpected(RTL_CONSTASCII_USTRINGPARAM("Sheet1.A2:A4,Sheet1.C2:D4"));
    uno::Reference<sheet::XCellRangesQuery> xCellRangesQuery = init();
    uno::Reference<sheet::XSheetCellRanges> xRanges = xCellRangesQuery->queryRowDifferences(table::CellAddress(0,1,1));
    rtl::OUString aResult = xRanges->getRangeAddressesAsString();
    std::cout << "testQueryRowDifference: Result: " << rtl::OUStringToOString(aResult, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
    CPPUNIT_ASSERT_MESSAGE("testQueryFormulaCells", aResult == aExpected);
}

void ScXCellRangesQuery::testQueryVisibleCells()
{
    setRowVisible(false);
    rtl::OUString aExpected(RTL_CONSTASCII_USTRINGPARAM("Sheet1.A2"));
    uno::Reference<sheet::XCellRangesQuery> xCellRangesQuery = init();
    uno::Reference<sheet::XSheetCellRanges> xRanges = xCellRangesQuery->queryVisibleCells();
    rtl::OUString aResult = xRanges->getRangeAddressesAsString();
    std::cout << "testQueryVisibleCells: Result: " << rtl::OUStringToOString(aResult, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
    CPPUNIT_ASSERT_MESSAGE("testQueryFormulaCells", aResult == aExpected);
}

void ScXCellRangesQuery::setRowVisible(bool bVisible)
{

}

CPPUNIT_TEST_SUITE_REGISTRATION(ScXCellRangesQuery);

CPPUNIT_PLUGIN_IMPLEMENT();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
