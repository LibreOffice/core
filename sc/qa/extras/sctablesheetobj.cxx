/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/sheet/scenario.hxx>
#include <test/sheet/sheetcellrange.hxx>
#include <test/sheet/spreadsheet.hxx>
#include <test/sheet/xarrayformularange.hxx>
#include <test/sheet/xcellformatrangessupplier.hxx>
#include <test/sheet/xcellrangeaddressable.hxx>
#include <test/sheet/xcellrangedata.hxx>
#include <test/sheet/xcellrangeformula.hxx>
#include <test/sheet/xcellrangemovement.hxx>
#include <test/sheet/xcellseries.hxx>
#include <test/sheet/xdatapilottablessupplier.hxx>
#include <test/sheet/xformulaquery.hxx>
#include <test/sheet/xmultipleoperation.hxx>
#include <test/sheet/xprintareas.hxx>
#include <test/sheet/xscenario.hxx>
#include <test/sheet/xscenarioenhanced.hxx>
#include <test/sheet/xscenariossupplier.hxx>
#include <test/sheet/xsheetannotationssupplier.hxx>
#include <test/sheet/xsheetauditing.hxx>
#include <test/sheet/xsheetcellrange.hxx>
#include <test/sheet/xsheetfilterable.hxx>
#include <test/sheet/xsheetfilterableex.hxx>
#include <test/sheet/xsheetlinkable.hxx>
#include <test/sheet/xsheetoperation.hxx>
#include <test/sheet/xsheetpagebreak.hxx>
#include <test/sheet/xspreadsheet.hxx>
#include <test/sheet/xsubtotalcalculatable.hxx>
#include <test/sheet/xuniquecellformatrangessupplier.hxx>
#include <test/table/xcolumnrowrange.hxx>
#include <test/table/xtablechartssupplier.hxx>
#include <test/util/xindent.hxx>
#include <test/util/xmergeable.hxx>
#include <test/util/xreplaceable.hxx>
#include <test/util/xsearchable.hxx>

#include <com/sun/star/sheet/XScenariosSupplier.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>

using namespace css;
using namespace css::uno;

namespace sc_apitest
{

class ScTableSheetObj : public CalcUnoApiTest, public apitest::Scenario,
                                               public apitest::SheetCellRange,
                                               public apitest::Spreadsheet,
                                               public apitest::XArrayFormulaRange,
                                               public apitest::XCellFormatRangesSupplier,
                                               public apitest::XCellRangeAddressable,
                                               public apitest::XCellRangeData,
                                               public apitest::XCellRangeFormula,
                                               public apitest::XCellRangeMovement,
                                               public apitest::XCellSeries,
                                               public apitest::XColumnRowRange,
                                               public apitest::XDataPilotTablesSupplier,
                                               public apitest::XFormulaQuery,
                                               public apitest::XIndent,
                                               public apitest::XMergeable,
                                               public apitest::XMultipleOperation,
                                               public apitest::XPrintAreas,
                                               public apitest::XReplaceable,
                                               public apitest::XScenario,
                                               public apitest::XScenarioEnhanced,
                                               public apitest::XScenariosSupplier,
                                               public apitest::XSearchable,
                                               public apitest::XSheetAnnotationsSupplier,
                                               public apitest::XSheetAuditing,
                                               public apitest::XSheetCellRange,
                                               public apitest::XSheetFilterable,
                                               public apitest::XSheetFilterableEx,
                                               public apitest::XSheetLinkable,
                                               public apitest::XSheetOperation,
                                               public apitest::XSheetPageBreak,
                                               public apitest::XSpreadsheet,
                                               public apitest::XSubTotalCalculatable,
                                               public apitest::XTableChartsSupplier,
                                               public apitest::XUniqueCellFormatRangesSupplier
{
public:
    ScTableSheetObj();

    virtual void setUp() override;
    virtual void tearDown() override;

    virtual OUString getFileURL() override;

    virtual uno::Reference< uno::XInterface > init() override;
    virtual uno::Reference< uno::XInterface > getXCellRangeData() override;
    virtual uno::Reference< uno::XInterface > getXSpreadsheetDocument() override;
    virtual uno::Reference< uno::XInterface > getXSpreadsheet() override;
    virtual uno::Reference< uno::XInterface > getScenarioSpreadsheet() override;

    CPPUNIT_TEST_SUITE(ScTableSheetObj);

    // Scenario
    CPPUNIT_TEST(testScenarioProperties);

    // SheetCellRange
    CPPUNIT_TEST(testSheetCellRangeProperties);

    // Spreadsheet
    CPPUNIT_TEST(testSpreadsheetProperties);

    // XArrayFormulaRange
#if 0 // disable, because it makes no sense to set an ArrayFormula over the whole sheet
    CPPUNIT_TEST(testGetSetArrayFormula);
#endif

    // XCellFormatRangesSupplier
    CPPUNIT_TEST(testGetCellFormatRanges);

    // XCellRangeAddressable
    CPPUNIT_TEST(testGetRangeAddress);

    // XCellRangeData
    CPPUNIT_TEST(testGetDataArrayOnTableSheet);
    CPPUNIT_TEST(testSetDataArrayOnTableSheet);

    // XCellRangeFormula
#if 0 // disable, because it makes no sense to set a FormulaArray over the whole sheet
    CPPUNIT_TEST(testGetSetFormulaArray);
#endif

    // XCellRangeMovement
    CPPUNIT_TEST(testInsertCells);
    CPPUNIT_TEST(testCopyRange);
    CPPUNIT_TEST(testMoveRange);
    CPPUNIT_TEST(testRemoveRange);

    // XCellSeries
    CPPUNIT_TEST(testFillAuto);
    CPPUNIT_TEST(testFillSeries);

    // XColumnRowRange
    CPPUNIT_TEST(testGetColumns);
    CPPUNIT_TEST(testGetRows);

    // XDataPilotTablesSupplier
    CPPUNIT_TEST(testGetDataPilotTables);

    // XFormulaQuery
    CPPUNIT_TEST(testQueryDependents);
    CPPUNIT_TEST(testQueryPrecedents);

    // XIndent
    CPPUNIT_TEST(testIncrementIndent);
    CPPUNIT_TEST(testDecrementIndent);

    // XMergeable
    CPPUNIT_TEST(testGetIsMergedMerge);

    // XSearchable
    CPPUNIT_TEST(testFindAll);
    CPPUNIT_TEST(testFindNext);
    CPPUNIT_TEST(testFindFirst);

    // XMultipleOperation
#if 0 // disable, because test never finishes (see i87863)
    CPPUNIT_TEST(testSetTableOperation);
#endif

    // XPrintAreas
    CPPUNIT_TEST(testSetAndGetPrintTitleColumns);
    CPPUNIT_TEST(testSetAndGetPrintTitleRows);

    // XReplaceable
    CPPUNIT_TEST(testReplaceAll);
    CPPUNIT_TEST(testCreateReplaceDescriptor);

    // XScenario
    // test was disabled in qadevOOo/tests/java/ifc/sheet/_XScenario.java
    CPPUNIT_TEST(testScenario);

    // XScenarioEnhanced
    CPPUNIT_TEST(testGetRanges);

    // XScenariosSupplier
    CPPUNIT_TEST(testGetScenarios);

    // XSheetAnnotationsSupplier
    CPPUNIT_TEST(testGetAnnotations);

    // XSheetAuditing
    CPPUNIT_TEST(testShowHideDependents);
    CPPUNIT_TEST(testShowHidePrecedents);
    CPPUNIT_TEST(testClearArrows);
    CPPUNIT_TEST(testShowErrors);
    CPPUNIT_TEST(testShowInvalid);

    // XSheetCellRange
    CPPUNIT_TEST(testGetSpreadsheet);

    // XSheetFilterable
    CPPUNIT_TEST(testCreateFilterDescriptor);
    CPPUNIT_TEST(testFilter);

    // XSheetFilterableEx
#if 0 // temporarily disabled, takes too long (see i87876)
    CPPUNIT_TEST(testCreateFilterDescriptorByObject);
#endif

    // XSheetLinkable
    CPPUNIT_TEST(testSheetLinkable);

    // XSheetOperation
    CPPUNIT_TEST(testComputeFunction);
    CPPUNIT_TEST(testClearContents);

    // XSheetPageBreak
    CPPUNIT_TEST(testGetColumnPageBreaks);
    CPPUNIT_TEST(testGetRowPageBreaks);
    CPPUNIT_TEST(testRemoveAllManualPageBreaks);

    // XSpreadsheet
    CPPUNIT_TEST(testCreateCursor);
    CPPUNIT_TEST(testCreateCursorByRange);

    // XSubTotalCalculatable
    CPPUNIT_TEST(testCreateSubTotalDescriptor);
    CPPUNIT_TEST(testApplyRemoveSubTotals);

    // XTableChartsSupplier
    CPPUNIT_TEST(testGetCharts);

    // XUniqueCellFormatRangesSupplier
    CPPUNIT_TEST(testGetUniqueCellFormatRanges);

    CPPUNIT_TEST_SUITE_END();

private:
    OUString maFileURL;
    uno::Reference< lang::XComponent > mxComponent;
};

ScTableSheetObj::ScTableSheetObj():
    CalcUnoApiTest("/sc/qa/extras/testdocuments"),
    apitest::XCellSeries(1, 0),
    apitest::XFormulaQuery(table::CellRangeAddress(0, 0, 0, 1023, 1048575), table::CellRangeAddress(0, 0, 0, 1023, 1048575), 0, 0),
    apitest::XReplaceable("searchReplaceString", "replaceReplaceString"),
    apitest::XSearchable("test", 4)
{
}

uno::Reference< uno::XInterface > ScTableSheetObj::init()
{
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, UNO_QUERY_THROW);

    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), UNO_QUERY_THROW);

    xSheet->getCellByPosition(5 ,5)->setValue(15);
    xSheet->getCellByPosition(2 ,0)->setValue(-5.15);
    xSheet->getCellByPosition(2 ,0)->setFormula("= B5 + C1");

    xSheet->getCellByPosition(6, 6)->setValue(3);
    xSheet->getCellByPosition(7, 6)->setValue(3);
    xSheet->getCellByPosition(8, 6)->setFormula("= SUM(G7:H7)");
    xSheet->getCellByPosition(9, 6)->setFormula("= G7*I7");

    uno::Sequence<table::CellRangeAddress> aCellRangeAddr(1);
    aCellRangeAddr[0] = table::CellRangeAddress(0, 0, 0, 10, 10);

    uno::Reference<sheet::XScenariosSupplier> xScence(xSheet, UNO_QUERY_THROW);
    xScence->getScenarios()->addNewByName("Scenario", aCellRangeAddr, "Comment");
    xSheets->getByName("Scenario");

    return xSheet;
}

uno::Reference< uno::XInterface > ScTableSheetObj::getXCellRangeData()
{
    return init();
}

uno::Reference<uno::XInterface> ScTableSheetObj::getXSpreadsheetDocument()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
    return xDoc;
}

uno::Reference<uno::XInterface> ScTableSheetObj::getScenarioSpreadsheet()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);

    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), UNO_QUERY_THROW);

    xSheet->getCellByPosition(5 ,5)->setValue(15);
    xSheet->getCellByPosition(2 ,0)->setValue(-5.15);
    xSheet->getCellByPosition(2 ,0)->setFormula("= B5 + C1");

    xSheet->getCellByPosition(6, 6)->setValue(3);
    xSheet->getCellByPosition(7, 6)->setValue(3);
    xSheet->getCellByPosition(8, 6)->setFormula("= SUM(G7:H7)");
    xSheet->getCellByPosition(9, 6)->setFormula("= G7*I7");

    uno::Sequence<table::CellRangeAddress> aCellRangeAddr(1);
    aCellRangeAddr[0] = table::CellRangeAddress(0, 0, 0, 10, 10);

    uno::Reference<sheet::XScenariosSupplier> xScence(xSheet, UNO_QUERY_THROW);
    xScence->getScenarios()->addNewByName("Scenario", aCellRangeAddr, "Comment");
    uno::Reference<sheet::XSpreadsheet> sSheet(xSheets->getByName("Scenario"), UNO_QUERY_THROW);

    return sSheet;
}

uno::Reference< uno::XInterface > ScTableSheetObj::getXSpreadsheet()
{
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, UNO_QUERY_THROW);

    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), UNO_SET_THROW);
    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(0), UNO_QUERY_THROW);

    xSheet->getCellByPosition(5 ,5)->setValue(15);
    xSheet->getCellByPosition(2 ,0)->setValue(-5.15);
    xSheet->getCellByPosition(2 ,0)->setFormula("= B5 + C1");

    xSheet->getCellByPosition(6, 6)->setValue(3);
    xSheet->getCellByPosition(7, 6)->setValue(3);
    xSheet->getCellByPosition(8, 6)->setFormula("= SUM(G7:H7)");
    xSheet->getCellByPosition(9, 6)->setFormula("= G7*I7");

    uno::Sequence<table::CellRangeAddress> aCellRangeAddr(1);
    aCellRangeAddr[0] = table::CellRangeAddress(0, 0, 0, 10, 10);
    uno::Reference<sheet::XScenariosSupplier> xScence(xSheet, UNO_QUERY_THROW);
    xScence->getScenarios()->addNewByName("Scenario", aCellRangeAddr, "Comment");
    xSheets->getByName("Scenario");

    setXCell(xSheet->getCellByPosition(15, 15));
    return xSheet;
}

OUString ScTableSheetObj::getFileURL()
{
    return maFileURL;
}

void ScTableSheetObj::setUp()
{
    CalcUnoApiTest::setUp();
    createFileURL(u"ScTableSheetObj.ods", maFileURL);
    mxComponent = loadFromDesktop(maFileURL, "com.sun.star.sheet.SpreadsheetDocument");
}

void ScTableSheetObj::tearDown()
{
    closeDocument(mxComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScTableSheetObj);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
