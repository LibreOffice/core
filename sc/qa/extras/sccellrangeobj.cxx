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
 * Copyright (C) 2011 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
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
#include <test/sheet/xcellrangesquery.hxx>
#include <test/sheet/cellproperties.hxx>
#include <test/util/xreplaceable.hxx>
#include <test/util/xsearchable.hxx>
#include <test/sheet/xcellrangedata.hxx>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/XCellRange.hpp>

namespace sc_apitest {

#define NUMBER_OF_TESTS 14

class ScCellRangeObj : public UnoApiTest, apitest::XCellRangesQuery, apitest::CellProperties,
                        apitest::XSearchable, apitest::XReplaceable, apitest::XCellRangeData
{
public:
    ScCellRangeObj();

    virtual void setUp();
    virtual void tearDown();
    virtual uno::Reference< uno::XInterface > init();
    virtual uno::Reference< uno::XInterface > getXCellRangeData();

    CPPUNIT_TEST_SUITE(ScCellRangeObj);
    CPPUNIT_TEST(testQueryColumnDifference);
    CPPUNIT_TEST(testQueryContentDifference);
    CPPUNIT_TEST(testQueryEmptyCells);
    //CPPUNIT_TEST(testQueryFormulaCells);
    CPPUNIT_TEST(testQueryIntersection);
    CPPUNIT_TEST(testQueryRowDifference);
    CPPUNIT_TEST(testQueryVisibleCells);
    CPPUNIT_TEST(testVertJustify);
    CPPUNIT_TEST(testRotateReference);
    CPPUNIT_TEST(testFindAll);
    CPPUNIT_TEST(testFindFirst);
    CPPUNIT_TEST(testReplaceAll);
    CPPUNIT_TEST(testCreateReplaceDescriptor);
    CPPUNIT_TEST(testGetDataArray);
    CPPUNIT_TEST(testSetDataArray);
    CPPUNIT_TEST_SUITE_END();

private:
    static sal_Int32 nTest;
    static uno::Reference< lang::XComponent > mxComponent;
};

sal_Int32 ScCellRangeObj::nTest = 0;
uno::Reference< lang::XComponent > ScCellRangeObj::mxComponent;

ScCellRangeObj::ScCellRangeObj():
        UnoApiTest("/sc/qa/extras/testdocuments"),
        apitest::XSearchable(OUString("15"), 1),
        apitest::XReplaceable(OUString("15"), OUString("35"))
{
}

uno::Reference< uno::XInterface > ScCellRangeObj::init()
{
    OUString aFileURL;
    const OUString aFileBase("xcellrangesquery.ods");
    createFileURL(aFileBase, aFileURL);
    std::cout << OUStringToOString(aFileURL, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
    if( !mxComponent.is())
        mxComponent = loadFromDesktop(aFileURL, "com.sun.star.sheet.SpreadsheetDocument");
    uno::Reference< sheet::XSpreadsheetDocument> xDoc (mxComponent, UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(0), UNO_QUERY_THROW);

    CPPUNIT_ASSERT_MESSAGE("Could not create interface of type XSpreadsheet", xSheet.is());

    uno::Reference<table::XCellRange> xReturn(xSheet->getCellRangeByPosition(0,0,3,4), UNO_QUERY_THROW);

    CPPUNIT_ASSERT_MESSAGE("Could not create object of type XCellRangesQuery", xReturn.is());
    return xReturn;
}

uno::Reference< uno::XInterface > ScCellRangeObj::getXCellRangeData()
{
    OUString aFileURL;
    const OUString aFileBase("xcellrangesquery.ods");
    createFileURL(aFileBase, aFileURL);
    std::cout << OUStringToOString(aFileURL, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
    if( !mxComponent.is())
        mxComponent = loadFromDesktop(aFileURL, "com.sun.star.sheet.SpreadsheetDocument");
    uno::Reference< sheet::XSpreadsheetDocument> xDoc (mxComponent, UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(1), UNO_QUERY_THROW);

    CPPUNIT_ASSERT_MESSAGE("Could not create interface of type XSpreadsheet", xSheet.is());

    uno::Reference<table::XCellRange> xReturn(xSheet->getCellRangeByPosition(0,0,3,3), UNO_QUERY_THROW);

    CPPUNIT_ASSERT_MESSAGE("Could not create object of type XCellRangesQuery", xReturn.is());
    return xReturn;
}

void ScCellRangeObj::setUp()
{
    nTest++;
    CPPUNIT_ASSERT(nTest <= NUMBER_OF_TESTS);
    UnoApiTest::setUp();
}

void ScCellRangeObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
        closeDocument(mxComponent);

    UnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScCellRangeObj);

CPPUNIT_PLUGIN_IMPLEMENT();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
