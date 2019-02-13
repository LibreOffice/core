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
#include <cppunit/extensions/HelperMacros.h>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XModel2.hpp>

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
#include <com/sun/star/sheet/XCellRangeFormula.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/sheet/Border.hpp>
#include <com/sun/star/sheet/XSubTotalCalculatable.hpp>
#include <com/sun/star/sheet/SubTotalColumn.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>

#include <test/callgrind.hxx>

#include <calcconfig.hxx>
#include <docsh.hxx>
#include <tabvwsh.hxx>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

class ScPerfObj : public CalcUnoApiTest
{
public:

    ScPerfObj();

    virtual void tearDown() override;

    uno::Reference< uno::XInterface > init(const OUString& aFileName);

    CPPUNIT_TEST_SUITE(ScPerfObj);
    CPPUNIT_TEST(testSheetFindAll);
    CPPUNIT_TEST(testFixedSum);
    CPPUNIT_TEST(testSheetNamedRanges);
    CPPUNIT_TEST(testSheets);
    CPPUNIT_TEST(testSum);
    CPPUNIT_TEST(testFTest);
    CPPUNIT_TEST(testChiTest);
    CPPUNIT_TEST(testSumX2PY2Test);
    CPPUNIT_TEST(testTTest);
    CPPUNIT_TEST(testLcm);
    CPPUNIT_TEST(testGcd);
    CPPUNIT_TEST(testPearson);
    CPPUNIT_TEST(testSubTotalWithFormulas);
    CPPUNIT_TEST(testSubTotalWithoutFormulas);
    CPPUNIT_TEST(testLoadingFileWithSingleBigSheet);
    CPPUNIT_TEST(testMatConcatSmall);
    CPPUNIT_TEST(testMatConcatLarge);
    CPPUNIT_TEST_SUITE_END();

private:

    uno::Reference< lang::XComponent > mxComponent;

    // tests
    void testSheetFindAll();
    void testSheetNamedRanges();
    void testSheets();
    void testSum();
    void testFTest();
    void testChiTest();
    void testSumX2PY2Test();
    void testTTest();
    void testLcm();
    void testGcd();
    void testPearson();
    void testSubTotalWithFormulas();
    void testSubTotalWithoutFormulas();
    void testLoadingFileWithSingleBigSheet();
    void testFixedSum();
    void testMatConcatSmall();
    void testMatConcatLarge();
};

ScPerfObj::ScPerfObj()
    : CalcUnoApiTest("sc/qa/perf/testdocuments/")
{
}

uno::Reference< uno::XInterface > ScPerfObj::init(const OUString& aFileName)
{
    OUString aFileURL;
    createFileURL(aFileName, aFileURL);

    mxComponent = loadFromDesktop(aFileURL);

    CPPUNIT_ASSERT(mxComponent.is());

    return mxComponent;
}

void ScPerfObj::tearDown()
{
    if (mxComponent.is())
    {
        closeDocument(mxComponent);
        mxComponent.clear();
    }
    CalcUnoApiTest::tearDown();
}

void ScPerfObj::testSheetFindAll()
{

    uno::Reference< sheet::XSpreadsheetDocument > xDoc(init("scBigFile.ods"), UNO_QUERY_THROW);

    // get sheet
    uno::Reference< container::XIndexAccess > xSheetIndex (xDoc->getSheets(), UNO_QUERY_THROW);

    uno::Reference< sheet::XSpreadsheet > xSheet( xSheetIndex->getByIndex(0), UNO_QUERY_THROW);

    // create descriptor
    uno::Reference< util::XSearchable > xSearchable(xSheet, UNO_QUERY_THROW);
    uno::Reference< util::XSearchDescriptor> xSearchDescr = xSearchable->createSearchDescriptor();

    // search for a value
    xSearchDescr->setSearchString("value_1");

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
    xSearchProp->setPropertyValue("SearchStyles", makeAny(true));

    xSearchDescr->setSearchString("aCellStyle");

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
    uno::Reference< sheet::XSpreadsheet > xSheet( xSheetIndex->getByIndex(1), UNO_QUERY_THROW);

    // test removeByName
    OUString maNameToRemove("namedRange_4999");
    bool bHasIt = xNamedRanges->hasByName(maNameToRemove);
    CPPUNIT_ASSERT_MESSAGE("NamedRange initial1 does not exist, can't remove it", bHasIt);

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

    uno::Reference< sheet::XCalculatable > xCalculatable(xDoc, UNO_QUERY_THROW);

    // get getSheets
    uno::Reference< sheet::XSpreadsheets > xSheets (xDoc->getSheets(), UNO_QUERY_THROW);

    uno::Any rSheet = xSheets->getByName("SumSheet");

    // query for the XSpreadsheet interface
    uno::Reference< sheet::XSpreadsheet > xSheet (rSheet, UNO_QUERY);
    uno::Reference< table::XCell > xCell = xSheet->getCellByPosition(1, 0);


    callgrindStart();
    xCell->setFormula("=SUM(A1:A10000)");
    xCalculatable->calculate();
    callgrindDump("sc:sum_numbers_column");

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Sum" , 504178.0, xCell->getValue());

    // query for the XCellRange interface
    uno::Reference< table::XCellRange > rCellRange(rSheet, UNO_QUERY);
    // query the cell range
    uno::Reference< table::XCellRange > xCellRange = rCellRange->getCellRangeByName("B1");

    uno::Reference< sheet::XArrayFormulaRange > xArrayFormulaRange(xCellRange, UNO_QUERY_THROW);

    callgrindStart();
    xArrayFormulaRange->setArrayFormula("=SUM(A1:A10000=30)");
    xCalculatable->calculate();
    callgrindDump("sc:sum_with_array_formula_condition");

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Sum - number of elements equal 30" , 99.0, xCell->getValue());
}

void ScPerfObj::testFTest()
{
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(init("scMathFunctions.ods"), UNO_QUERY_THROW);

    uno::Reference< sheet::XCalculatable > xCalculatable(xDoc, UNO_QUERY_THROW);

    // get getSheets
    uno::Reference< sheet::XSpreadsheets > xSheets (xDoc->getSheets(), UNO_QUERY_THROW);

    uno::Any rSheet = xSheets->getByName("FTestSheet");

    // query for the XSpreadsheet interface
    uno::Reference< sheet::XSpreadsheet > xSheet (rSheet, UNO_QUERY);
    uno::Reference< table::XCell > xCell = xSheet->getCellByPosition(0, 0);

    callgrindStart();
    xCell->setFormula("=FTEST(B1:K10;L1:U10)");
    xCalculatable->calculate();
    callgrindDump("sc:ftest");

    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Wrong FTest result" , 0.8909, xCell->getValue(), 10e-4);
}

void ScPerfObj::testChiTest()
{
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(init("scMathFunctions.ods"), UNO_QUERY_THROW);

    uno::Reference< sheet::XCalculatable > xCalculatable(xDoc, UNO_QUERY_THROW);

    // get getSheets
    uno::Reference< sheet::XSpreadsheets > xSheets (xDoc->getSheets(), UNO_QUERY_THROW);

    uno::Any rSheet = xSheets->getByName("ChiTestSheet");

    // query for the XSpreadsheet interface
    uno::Reference< sheet::XSpreadsheet > xSheet (rSheet, UNO_QUERY);
    uno::Reference< table::XCell > xCell = xSheet->getCellByPosition(0, 0);

    callgrindStart();
    xCell->setFormula("=CHITEST(B1:CV100;CW1:GQ100)");
    xCalculatable->calculate();
    callgrindDump("sc:chitest");

    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Wrong ChiTest result" , 0.0, xCell->getValue(), 10e-4);
}

void ScPerfObj::testSumX2PY2Test()
{
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(init("scMathFunctions2.ods"), UNO_QUERY_THROW);

    uno::Reference< sheet::XCalculatable > xCalculatable(xDoc, UNO_QUERY_THROW);

    // get getSheets
    uno::Reference< sheet::XSpreadsheets > xSheets (xDoc->getSheets(), UNO_QUERY_THROW);

    uno::Any rSheet = xSheets->getByName("SumX2PY2Sheet");

    // query for the XSpreadsheet interface
    uno::Reference< sheet::XSpreadsheet > xSheet (rSheet, UNO_QUERY);
    uno::Reference< table::XCell > xCell = xSheet->getCellByPosition(2, 0);

    callgrindStart();
    xCell->setFormula("=SUMX2PY2(A1:A10000;B1:B10000)");
    xCalculatable->calculate();
    callgrindDump("sc:sumx2py2");

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong SumX2PY2 result" , 574539.0, xCell->getValue());
}

void ScPerfObj::testTTest()
{
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(init("scMathFunctions2.ods"), UNO_QUERY_THROW);

    uno::Reference< sheet::XCalculatable > xCalculatable(xDoc, UNO_QUERY_THROW);

    // get getSheets
    uno::Reference< sheet::XSpreadsheets > xSheets (xDoc->getSheets(), UNO_QUERY_THROW);

    uno::Any rSheet = xSheets->getByName("TTestSheet");

    // query for the XSpreadsheet interface
    uno::Reference< sheet::XSpreadsheet > xSheet (rSheet, UNO_QUERY);
    uno::Reference< table::XCell > xCell = xSheet->getCellByPosition(0, 0);

    callgrindStart();
    xCell->setFormula("=T.TEST(B1:CV100;CW1:GQ100;2;1)");
    xCalculatable->calculate();
    callgrindDump("sc:ttest");

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong TTest result" , 0.0, xCell->getValue());
}

void ScPerfObj::testLcm()
{
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(init("scMathFunctions2.ods"), UNO_QUERY_THROW);

    uno::Reference< sheet::XCalculatable > xCalculatable(xDoc, UNO_QUERY_THROW);

    // get getSheets
    uno::Reference< sheet::XSpreadsheets > xSheets (xDoc->getSheets(), UNO_QUERY_THROW);

    uno::Any rSheet = xSheets->getByName("LCMSheet");

    // query for the XSpreadsheet interface
    uno::Reference< sheet::XSpreadsheet > xSheet (rSheet, UNO_QUERY);
    uno::Reference< table::XCell > xCell = xSheet->getCellByPosition(1, 0);

    callgrindStart();
    xCell->setFormula("=LCM(A1:A10000)");
    xCalculatable->calculate();
    callgrindDump("sc:lcm");

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong LCM" , 2520.0, xCell->getValue());
}

void ScPerfObj::testGcd()
{
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(init("scMathFunctions2.ods"), UNO_QUERY_THROW);

    uno::Reference< sheet::XCalculatable > xCalculatable(xDoc, UNO_QUERY_THROW);

    // get getSheets
    uno::Reference< sheet::XSpreadsheets > xSheets (xDoc->getSheets(), UNO_QUERY_THROW);

    uno::Any rSheet = xSheets->getByName("GCDSheet");

    // query for the XSpreadsheet interface
    uno::Reference< sheet::XSpreadsheet > xSheet (rSheet, UNO_QUERY);
    uno::Reference< table::XCell > xCell = xSheet->getCellByPosition(1, 0);

    callgrindStart();
    xCell->setFormula("=GCD(A1:A10000)");
    xCalculatable->calculate();
    callgrindDump("sc:gcd");

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong GCD", 3.0, xCell->getValue());
}

void ScPerfObj::testPearson()
{
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(init("scMathFunctions2.ods"), UNO_QUERY_THROW);

    uno::Reference< sheet::XCalculatable > xCalculatable(xDoc, UNO_QUERY_THROW);

    // get getSheets
    uno::Reference< sheet::XSpreadsheets > xSheets (xDoc->getSheets(), UNO_QUERY_THROW);

    uno::Any rSheet = xSheets->getByName("PearsonSheet");

    // query for the XSpreadsheet interface
    uno::Reference< sheet::XSpreadsheet > xSheet (rSheet, UNO_QUERY);
    uno::Reference< table::XCell > xCell = xSheet->getCellByPosition(0, 0);

    callgrindStart();
    xCell->setFormula("=PEARSON(B1:CV100;CW1:GQ100)");
    xCalculatable->calculate();
    callgrindDump("sc:pearson");

    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Wrong Pearson result" , 0.01255, xCell->getValue(), 10e-4);
}

void ScPerfObj::testSubTotalWithFormulas()
{
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(init("scBigSingleSheet200.ods"), UNO_QUERY_THROW);

    uno::Reference< sheet::XCalculatable > xCalculatable(xDoc, UNO_QUERY_THROW);

    // get getSheets
    uno::Reference< sheet::XSpreadsheets > xSheets (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Any rSheet = xSheets->getByName("scBigSingleSheet");

    // query for the XCellRange interface
    uno::Reference< table::XCellRange > rCellRange(rSheet, UNO_QUERY);

    // query the cell range
    uno::Reference< table::XCellRange > xCellRange = rCellRange->getCellRangeByPosition(0,0,1023,1000000);

    // Create the subtotal interface
    uno::Reference< sheet::XSubTotalCalculatable > xSub(xCellRange, UNO_QUERY_THROW);
    uno::Reference< sheet::XSubTotalDescriptor > xSubDesc = xSub->createSubTotalDescriptor(true);

    // Create the column used for subtotal
    uno::Sequence < sheet::SubTotalColumn > xSubTotalColumns;
    xSubTotalColumns.realloc(1);

    // Add the column to the descriptor
    xSubTotalColumns[0].Column   = 1;
    xSubTotalColumns[0].Function = sheet::GeneralFunction_SUM;

    xSubDesc->addNew(xSubTotalColumns, 0);

    // Run the subtotal function
    callgrindStart();
    xSub->applySubTotals(xSubDesc, true);
    callgrindDump("sc:doSubTotal_on_large_sheet_with_formulas");
}

void ScPerfObj::testSubTotalWithoutFormulas()
{
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(init("scBigSingleSheet200.ods"), UNO_QUERY_THROW);

    uno::Reference< sheet::XCalculatable > xCalculatable(xDoc, UNO_QUERY_THROW);

    // get getSheets
    uno::Reference< sheet::XSpreadsheets > xSheets (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Any rSheet = xSheets->getByName("scBigSingleSheet");

    // query for the XCellRange interface
    uno::Reference< table::XCellRange > rCellRange(rSheet, UNO_QUERY);

    // query the cell range
    uno::Reference< table::XCellRange > xCellRange = rCellRange->getCellRangeByPosition(0,0,10,1000000);

    // Create the subtotal interface
    uno::Reference< sheet::XSubTotalCalculatable > xSub(xCellRange, UNO_QUERY_THROW);
    uno::Reference< sheet::XSubTotalDescriptor > xSubDesc = xSub->createSubTotalDescriptor(true);

    // Create the column used for subtotal
    uno::Sequence < sheet::SubTotalColumn> xSubTotalColumns;
    xSubTotalColumns.realloc(1);

    // Add the column to the descriptor
    xSubTotalColumns[0].Column   = 1;
    xSubTotalColumns[0].Function = sheet::GeneralFunction_SUM;

    xSubDesc->addNew(xSubTotalColumns, 0);

    // Run the subtotal function
    callgrindStart();
    xSub->applySubTotals(xSubDesc, true);
    callgrindDump("sc:doSubTotal_on_large_sheet_without_formulas");
}

void ScPerfObj::testLoadingFileWithSingleBigSheet()
{
    callgrindStart();
    uno::Reference< sheet::XSpreadsheetDocument > xDoc1(init("scBigSingleSheet2000.ods"), UNO_QUERY_THROW);
    uno::Reference< sheet::XCalculatable > xCalculatable1(xDoc1, UNO_QUERY_THROW);
    callgrindDump("sc:loadingFileWithSingleBigSheetdoSubTotal_2000lines");
}

namespace {
    void setupBlockFormula(
        const uno::Reference< sheet::XSpreadsheetDocument > & xDoc,
        const OUString &rSheetName,
        const OUString &rCellRange,
        const OUString &rFormula)
    {
        uno::Reference< sheet::XSpreadsheets > xSheets (xDoc->getSheets(), UNO_QUERY_THROW);

        uno::Any aSheet = xSheets->getByName(rSheetName);
        uno::Reference< table::XCellRange > xSheetCellRange(aSheet, UNO_QUERY);
        uno::Reference< sheet::XCellRangeFormula > xCellRange(
            xSheetCellRange->getCellRangeByName(rCellRange), UNO_QUERY);

        uno::Sequence< uno::Sequence< OUString > > aFormulae(1000);
        for (sal_Int32 i = 0; i < 1000; ++i)
        {
            uno::Sequence< OUString > aRow(1);
            aRow[0] = rFormula;
            aFormulae[i] = aRow;
        }

        // NB. not set Array (matrix) formula
        xCellRange->setFormulaArray(aFormulae);
    }
}

void ScPerfObj::testFixedSum()
{
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(init("scMathFunctions3.ods"), UNO_QUERY_THROW);

    uno::Reference< sheet::XCalculatable > xCalculatable(xDoc, UNO_QUERY_THROW);

    setupBlockFormula(xDoc, "FixedSumSheet", "B1:B1000", "=SUM(A$1:A$1000)");

    callgrindStart();
    xCalculatable->calculateAll();
    callgrindDump("sc:sum_with_fixed_array_formula");

    uno::Reference< sheet::XSpreadsheets > xSheets (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Any aSheet = xSheets->getByName("FixedSumSheet");
    uno::Reference< sheet::XSpreadsheet > xSheet (aSheet, UNO_QUERY);

    for( sal_Int32 i = 0; i < 1000; ++i )
    {
        uno::Reference< table::XCell > xCell = xSheet->getCellByPosition(1, i);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(50206.0, xCell->getValue(), 1e-12);
    }
}

void ScPerfObj::testMatConcatSmall()
{
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(init("empty.ods"), UNO_QUERY_THROW);

    uno::Reference< sheet::XCalculatable > xCalculatable(xDoc, UNO_QUERY_THROW);

    // get getSheets
    uno::Reference< sheet::XSpreadsheets > xSheets (xDoc->getSheets(), UNO_QUERY_THROW);

    uno::Any rSheet = xSheets->getByName("Sheet1");

    // query for the XSpreadsheet interface
    uno::Reference< sheet::XSpreadsheet > xSheet (rSheet, UNO_QUERY);

    // query for the XCellRange interface
    uno::Reference< table::XCellRange > rCellRange(rSheet, UNO_QUERY);
    // query the cell range
    uno::Reference< table::XCellRange > xCellRange = rCellRange->getCellRangeByName("C1");

    uno::Reference< sheet::XArrayFormulaRange > xArrayFormulaRange(xCellRange, UNO_QUERY_THROW);

    xArrayFormulaRange->setArrayFormula("=A1:A20&B1:B20");
    callgrindStart();
    xCalculatable->calculateAll();
    callgrindDump("sc:mat_concat_small");
}

void ScPerfObj::testMatConcatLarge()
{
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(init("empty.ods"), UNO_QUERY_THROW);

    uno::Reference< sheet::XCalculatable > xCalculatable(xDoc, UNO_QUERY_THROW);

    // get getSheets
    uno::Reference< sheet::XSpreadsheets > xSheets (xDoc->getSheets(), UNO_QUERY_THROW);

    uno::Any rSheet = xSheets->getByName("Sheet1");

    // query for the XSpreadsheet interface
    uno::Reference< sheet::XSpreadsheet > xSheet (rSheet, UNO_QUERY);

    // query for the XCellRange interface
    uno::Reference< table::XCellRange > rCellRange(rSheet, UNO_QUERY);
    // query the cell range
    uno::Reference< table::XCellRange > xCellRange = rCellRange->getCellRangeByName("C1");

    uno::Reference< sheet::XArrayFormulaRange > xArrayFormulaRange(xCellRange, UNO_QUERY_THROW);

    callgrindStart();
    xArrayFormulaRange->setArrayFormula("=A1:A30000&B1:B30000");
    xCalculatable->calculate();
    callgrindDump("sc:mat_concat");
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScPerfObj);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
