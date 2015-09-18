/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>

#include <rtl/ustring.hxx>
#include "cppunit/extensions/HelperMacros.h"

#include <com/sun/star/util/XSearchable.hpp>
#include <com/sun/star/util/XSearchDescriptor.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XArrayFormulaRange.hpp>
#include <com/sun/star/sheet/XCalculatable.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>
#include <com/sun/star/sheet/XNamedRange.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/sheet/Border.hpp>
#include <com/sun/star/sheet/NamedRangeFlag.hpp>

#include <test/callgrind.hxx>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

#define NUMBER_OF_TESTS 3

class ScPerfObj : public CalcUnoApiTest
{
public:

    ScPerfObj();

    virtual void setUp() SAL_OVERRIDE;
    virtual void tearDown() SAL_OVERRIDE;

    uno::Reference< uno::XInterface > init(const OUString& aFileName);

    CPPUNIT_TEST_SUITE(ScPerfObj);
    CPPUNIT_TEST(testSheetFindAll);
    CPPUNIT_TEST(testSheetNamedRanges);
    CPPUNIT_TEST(testSheets);
    CPPUNIT_TEST(testSum);
    CPPUNIT_TEST_SUITE_END();

private:

    static sal_Int32 nTest;
    static uno::Reference< lang::XComponent > mxComponent;

    // tests
    void testSheetFindAll();
    void testSheetNamedRanges();
    void testSheets();
    void testSum();

};

sal_Int32 ScPerfObj::nTest = 0;
uno::Reference< lang::XComponent > ScPerfObj::mxComponent;

ScPerfObj::ScPerfObj()
    : CalcUnoApiTest("sc/qa/perf/testdocuments/")
{
}

uno::Reference< uno::XInterface > ScPerfObj::init(const OUString& aFileName)
{
    if (mxComponent.is())
        closeDocument(mxComponent);

    OUString aFileURL;
    createFileURL(aFileName, aFileURL);

    mxComponent = loadFromDesktop(aFileURL);

    CPPUNIT_ASSERT(mxComponent.is());

    return mxComponent;
}

void ScPerfObj::setUp()
{
    nTest++;
    CalcUnoApiTest::setUp();
}

void ScPerfObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
    {
        if (mxComponent.is())
        {
            closeDocument(mxComponent);
            mxComponent.clear();
        }
    }
    CalcUnoApiTest::tearDown();
}

void ScPerfObj::testSheetFindAll()
{

    uno::Reference< sheet::XSpreadsheetDocument > xDoc(init("scBigFile.ods"), UNO_QUERY_THROW);

    CPPUNIT_ASSERT_MESSAGE("Problem in document loading" , xDoc.is());

    // get sheet
    uno::Reference< container::XIndexAccess > xSheetIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Problem in xSheetIndex" , xSheetIndex.is());

    uno::Reference< sheet::XSpreadsheet > xSheet( xSheetIndex->getByIndex(0), UNO_QUERY_THROW);

    // create descriptor
    uno::Reference< util::XSearchable > xSearchable(xSheet, UNO_QUERY_THROW);
    uno::Reference< util::XSearchDescriptor> xSearchDescr = xSearchable->createSearchDescriptor();

    // search for a value
    xSearchDescr->setSearchString(OUString("value_1"));

    callgrindStart();

    uno::Reference< container::XIndexAccess > xIndex = xSearchable->findAll(xSearchDescr);

    callgrindDump("sc:search_all_sheets_by_value");

    CPPUNIT_ASSERT(xIndex.is());

    int nCount = xIndex->getCount();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Count" , 20000, nCount);

    // search for style
    uno::Reference< sheet::XSpreadsheet > xSheet2( xSheetIndex->getByIndex(2), UNO_QUERY_THROW);
    uno::Reference< util::XSearchable > xSearchableStyle(xSheet2, UNO_QUERY_THROW);
    xSearchDescr = xSearchableStyle->createSearchDescriptor();

    uno::Reference< beans::XPropertySet > xSearchProp(xSearchDescr,UNO_QUERY_THROW);
    xSearchProp->setPropertyValue(OUString("SearchStyles"), makeAny(true));

    xSearchDescr->setSearchString(OUString("aCellStyle"));

    callgrindStart();

    uno::Reference< container::XIndexAccess > xIndex2 = xSearchableStyle->findAll(xSearchDescr);

    callgrindDump("sc:search_all_sheets_by_style");

    CPPUNIT_ASSERT(xIndex2.is());

    nCount = xIndex2->getCount();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Count" , 160, nCount);

}

void ScPerfObj::testSheetNamedRanges()
{

    uno::Reference< lang::XComponent > xComponent (init("scBigFile.ods"), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(xComponent, UNO_QUERY_THROW);

    // get NamedRanges
    uno::Reference< beans::XPropertySet > xPropSet (xComponent, UNO_QUERY_THROW);
    OUString aNamedRangesPropertyString("NamedRanges");
    uno::Reference< sheet::XNamedRanges > xNamedRanges(xPropSet->getPropertyValue(aNamedRangesPropertyString), UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndexNamedRanges(xNamedRanges, UNO_QUERY_THROW);

    // get sheet - named ranges are defined in sheet #1
    uno::Reference< container::XIndexAccess > xSheetIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Problem in xSheetIndex" , xSheetIndex.is());
    uno::Reference< sheet::XSpreadsheet > xSheet( xSheetIndex->getByIndex(1), UNO_QUERY_THROW);

    // test removeByName
    OUString maNameToRemove("namedRange_4999");
    bool bHasIt = xNamedRanges->hasByName(maNameToRemove);
    CPPUNIT_ASSERT_MESSAGE("NamedRange initial1 des not exits, can't remove it", bHasIt);

    if (bHasIt)
    {
        callgrindStart();
        xNamedRanges->removeByName(maNameToRemove);
        callgrindDump("sc:remove_named_range_by_name");
    }

    // test addNewByName
    table::CellAddress aBaseAddress = table::CellAddress(0,0,0);

    sal_Int32 nType = 0;
    OUString aContent1("B4999");
    OUString aName1("single_added");
    callgrindStart();
    xNamedRanges->addNewByName(aName1, aContent1, aBaseAddress, nType);
    callgrindDump("sc:add_named_range_by_name");

    //test AddNewFromTitles with some 1000 new named ranges
    table::CellRangeAddress aCellRangeAddress = table::CellRangeAddress(0,1,0,2,999);
    callgrindStart();
    xNamedRanges->addNewFromTitles(aCellRangeAddress, sheet::Border_LEFT);
    callgrindDump("sc:add_named_range_from_titles");

}

void ScPerfObj::testSheets()
{

    uno::Reference< sheet::XSpreadsheetDocument > xDoc(init("scBigFile.ods"), UNO_QUERY_THROW);

    CPPUNIT_ASSERT_MESSAGE("Problem in document loading" , xDoc.is());

    // get getSheets
    uno::Reference< sheet::XSpreadsheets > xSheets (xDoc->getSheets(), UNO_QUERY_THROW);

    OUString aSheetName = "NewSheet";

    callgrindStart();
    xSheets->insertNewByName(aSheetName, 80);
    callgrindDump("sc:insert_new_sheet_by_name");

    OUString aSourceSheetName = "aSheet_2";
    OUString aTargetSheetName = "aCopiedSheet";

    callgrindStart();
    xSheets->copyByName(aSourceSheetName, aTargetSheetName, 70);
    callgrindDump("sc:copy_sheet_by_name");

    uno::Reference< sheet::XSpreadsheets > xSheetsNameContainer (xSheets, UNO_QUERY_THROW);

    callgrindStart();
    xSheetsNameContainer->removeByName(aSheetName);
    callgrindDump("sc:remove_sheet_by_name");

}

void ScPerfObj::testSum()
{
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(init("scMathFunctions.ods"), UNO_QUERY_THROW);

    CPPUNIT_ASSERT_MESSAGE("Problem in document loading" , xDoc.is());
    uno::Reference< sheet::XCalculatable > xCalculatable(xDoc, UNO_QUERY_THROW);

    // get getSheets
    uno::Reference< sheet::XSpreadsheets > xSheets (xDoc->getSheets(), UNO_QUERY_THROW);

    uno::Any rSheet = xSheets->getByName(OUString::createFromAscii("SumSheet"));

    // query for the XSpreadsheet interface
    uno::Reference< sheet::XSpreadsheet > xSheet (rSheet, UNO_QUERY);
    uno::Reference< table::XCell > xCell = xSheet->getCellByPosition(1, 0);


    callgrindStart();
    xCell->setFormula(OUString::createFromAscii("=SUM(A1:A10000)"));
    xCalculatable->calculate();
    callgrindDump("sc:sum_numbers_column");

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Sum" , 504178.0, xCell->getValue());

    // query for the XCellRange interface
    uno::Reference< table::XCellRange > rCellRange(rSheet, UNO_QUERY);
    // query the cell range
    uno::Reference< table::XCellRange > xCellRange = rCellRange->getCellRangeByName(OUString::createFromAscii("B1"));

    uno::Reference< sheet::XArrayFormulaRange > xArrayFormulaRange(xCellRange, UNO_QUERY_THROW);

    callgrindStart();
    xArrayFormulaRange->setArrayFormula("=SUM(A1:A10000=30)");
    xCalculatable->calculate();
    callgrindDump("sc:sum_with_array_formula_condition");

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Sum - number of elements equal 30" , 99.0, xCell->getValue());
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScPerfObj);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
