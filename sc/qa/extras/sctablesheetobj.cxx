/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/sheet/xcellseries.hxx>
#include <test/sheet/xdatapilottablessupplier.hxx>
#include <test/sheet/xmultipleoperation.hxx>
#include <test/sheet/xprintareas.hxx>
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

#define NUMBER_OF_TESTS 32

class ScTableSheetObj : public CalcUnoApiTest, public apitest::XCellSeries,
                                               public apitest::XDataPilotTablesSupplier,
                                               public apitest::XMultipleOperation,
                                               public apitest::XPrintAreas,
                                               public apitest::XReplaceable,
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
                                               public apitest::XUniqueCellFormatRangesSupplier
{
public:
    ScTableSheetObj();

    virtual void setUp() override;
    virtual void tearDown() override;

    virtual OUString getFileURL() override;

    virtual uno::Reference< uno::XInterface > init() override;
    virtual uno::Reference< uno::XInterface > getXSpreadsheet() override;
    virtual uno::Reference< uno::XInterface > getScenarioSpreadsheet() override;

    CPPUNIT_TEST_SUITE(ScTableSheetObj);

    // XCellSeries
    CPPUNIT_TEST(testFillAuto);
    CPPUNIT_TEST(testFillSeries);

    // XDataPilotTablesSupplier
    CPPUNIT_TEST(testGetDataPilotTables);

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

    // XUniqueCellFormatRangesSupplier
    CPPUNIT_TEST(testGetUniqueCellFormatRanges);

    CPPUNIT_TEST_SUITE_END();

private:
    OUString maFileURL;
    static sal_Int32 nTest;
    static uno::Reference< lang::XComponent > mxComponent;
};

sal_Int32 ScTableSheetObj::nTest = 0;
uno::Reference< lang::XComponent > ScTableSheetObj::mxComponent;

ScTableSheetObj::ScTableSheetObj():
    CalcUnoApiTest("/sc/qa/extras/testdocuments"),
    apitest::XCellSeries(1, 0),
    apitest::XReplaceable("searchReplaceString", "replaceReplaceString"),
    apitest::XSearchable("test", 4)
{
}

uno::Reference< uno::XInterface > ScTableSheetObj::init()
{
    createFileURL("ScTableSheetObj.ods", maFileURL);
    if (!mxComponent.is())
        mxComponent = loadFromDesktop(maFileURL, "com.sun.star.sheet.SpreadsheetDocument");
    CPPUNIT_ASSERT_MESSAGE("no calc document", mxComponent.is());

    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), UNO_QUERY_THROW);
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

    return xSheet;
}

uno::Reference<uno::XInterface> ScTableSheetObj::getScenarioSpreadsheet()
{
    createFileURL("ScTableSheetObj.ods", maFileURL);
    if (!mxComponent.is())
        mxComponent = loadFromDesktop(maFileURL, "com.sun.star.sheet.SpreadsheetDocument");
    CPPUNIT_ASSERT_MESSAGE("no calc document", mxComponent.is());

    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), UNO_QUERY_THROW);

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
    createFileURL("ScTableSheetObj.ods", maFileURL);
    if (!mxComponent.is())
        mxComponent = loadFromDesktop(maFileURL, "com.sun.star.sheet.SpreadsheetDocument");
    CPPUNIT_ASSERT_MESSAGE("no calc document", mxComponent.is());

    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), UNO_QUERY_THROW);

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
    return xSheet;
}

OUString ScTableSheetObj::getFileURL()
{
    return maFileURL;
}

void ScTableSheetObj::setUp()
{
    nTest++;
    CPPUNIT_ASSERT(nTest <= NUMBER_OF_TESTS);
    CalcUnoApiTest::setUp();
}

void ScTableSheetObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
    {
        closeDocument(mxComponent);
        mxComponent.clear();
    }

    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScTableSheetObj);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
