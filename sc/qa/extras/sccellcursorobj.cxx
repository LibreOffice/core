/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/sheet/sheetcellrange.hxx>
#include <test/sheet/xarrayformularange.hxx>
#include <test/sheet/xcellformatrangessupplier.hxx>
#include <test/sheet/xcellrangeaddressable.hxx>
#include <test/sheet/xcellrangedata.hxx>
#include <test/sheet/xcellrangeformula.hxx>
#include <test/sheet/xcellseries.hxx>
#include <test/sheet/xformulaquery.hxx>
#include <test/sheet/xmultipleoperation.hxx>
#include <test/sheet/xsheetcellcursor.hxx>
#include <test/sheet/xsheetcellrange.hxx>
#include <test/sheet/xsheetfilterable.hxx>
#include <test/sheet/xsheetfilterableex.hxx>
#include <test/sheet/xsheetoperation.hxx>
#include <test/sheet/xsubtotalcalculatable.hxx>
#include <test/sheet/xuniquecellformatrangessupplier.hxx>
#include <test/sheet/xusedareacursor.hxx>
#include <test/table/xcellcursor.hxx>
#include <test/table/xcolumnrowrange.hxx>
#include <test/util/xindent.hxx>
#include <test/util/xmergeable.hxx>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/XCellCursor.hpp>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

class ScCellCursorObj : public CalcUnoApiTest, public apitest::SheetCellRange,
                                               public apitest::XArrayFormulaRange,
                                               public apitest::XCellCursor,
                                               public apitest::XCellFormatRangesSupplier,
                                               public apitest::XCellRangeAddressable,
                                               public apitest::XCellRangeData,
                                               public apitest::XCellRangeFormula,
                                               public apitest::XCellSeries,
                                               public apitest::XColumnRowRange,
                                               public apitest::XFormulaQuery,
                                               public apitest::XIndent,
                                               public apitest::XMergeable,
                                               public apitest::XMultipleOperation,
                                               public apitest::XSheetCellCursor,
                                               public apitest::XSheetCellRange,
                                               public apitest::XSheetFilterable,
                                               public apitest::XSheetFilterableEx,
                                               public apitest::XSheetOperation,
                                               public apitest::XSubTotalCalculatable,
                                               public apitest::XUniqueCellFormatRangesSupplier,
                                               public apitest::XUsedAreaCursor
{
public:
    ScCellCursorObj();

    virtual void setUp() override;
    virtual void tearDown() override;
    virtual uno::Reference< uno::XInterface > init() override;
    virtual uno::Reference< uno::XInterface > getXCellRangeData() override;
    virtual uno::Reference< uno::XInterface > getXSpreadsheet() override;

    CPPUNIT_TEST_SUITE(ScCellCursorObj);

    // SheetCellRange
    CPPUNIT_TEST(testSheetCellRangeProperties);

    // XArrayFormulaRange
    CPPUNIT_TEST(testGetSetArrayFormula);

    // XCellCursor
    CPPUNIT_TEST(testGoToNext);
    CPPUNIT_TEST(testGoToOffset);
    CPPUNIT_TEST(testGoToPrevious);
    CPPUNIT_TEST(testGoToStart);
    CPPUNIT_TEST(testGoToEnd);

    // XCellFormatRangesSupplier
    CPPUNIT_TEST(testGetCellFormatRanges);

    // XCellRangeAddressable
    CPPUNIT_TEST(testGetRangeAddress);

    // XCellRangeData
    CPPUNIT_TEST(testGetDataArray);
    CPPUNIT_TEST(testSetDataArray);

    // XCellRangeFormula
    CPPUNIT_TEST(testGetSetFormulaArray);

    // XCellSeries
    CPPUNIT_TEST(testFillAuto);
    CPPUNIT_TEST(testFillSeries);

    // XColumnRowRange
    CPPUNIT_TEST(testGetColumns);
    CPPUNIT_TEST(testGetRows);

    // XFormulaQuery
    CPPUNIT_TEST(testQueryDependents);
    CPPUNIT_TEST(testQueryPrecedents);

    // XIndent
    CPPUNIT_TEST(testIncrementIndent);
    CPPUNIT_TEST(testDecrementIndent);

    // XMergeable
    CPPUNIT_TEST(testGetIsMergedMerge);

    // XMultipleOperation
    CPPUNIT_TEST(testSetTableOperation);

    // XSheetCellCursor
    CPPUNIT_TEST(testCollapseToCurrentArray);
    CPPUNIT_TEST(testCollapseToCurrentRegion);
    CPPUNIT_TEST(testCollapseToMergedArea);
    CPPUNIT_TEST(testCollapseToSize);
    CPPUNIT_TEST(testExpandToEntireColumns);
    CPPUNIT_TEST(testExpandToEntireRows);

    // XSheetCellRange
    CPPUNIT_TEST(testGetSpreadsheet);

    // XSheetFilterable
    CPPUNIT_TEST(testCreateFilterDescriptor);
    CPPUNIT_TEST(testFilter);

    // XSheetFilterableEx
    CPPUNIT_TEST(testCreateFilterDescriptorByObject);

    // XSheetOperation
    CPPUNIT_TEST(testComputeFunction);
    CPPUNIT_TEST(testClearContents);

    // XSubTotalCalculatable
    CPPUNIT_TEST(testCreateSubTotalDescriptor);
    CPPUNIT_TEST(testApplyRemoveSubTotals);

    // XUsedAreaCursor
    CPPUNIT_TEST(testGotoStartOfUsedArea);
    CPPUNIT_TEST(testGotoEndOfUsedArea);

    // XUniqueCellFormatRangesSupplier
    CPPUNIT_TEST(testGetUniqueCellFormatRanges);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference< lang::XComponent > mxComponent;
};

ScCellCursorObj::ScCellCursorObj():
    CalcUnoApiTest("/sc/qa/extras/testdocuments"),
    apitest::XCellSeries(0, 0),
    apitest::XFormulaQuery(table::CellRangeAddress(0, 15, 15, 15, 15), table::CellRangeAddress(0, 0, 15, 0, 15))
{
}

uno::Reference< uno::XInterface > ScCellCursorObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);

    uno::Reference<container::XIndexAccess> xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), UNO_QUERY_THROW);

    uno::Reference<table::XCellRange> xCellRange = xSheet->getCellRangeByName("$A$1:$D$4");
    uno::Reference<sheet::XSheetCellRange> xSheetCellRange(xCellRange, UNO_QUERY_THROW);
    uno::Reference<table::XCellCursor> xCellCursor(xSheet->createCursorByRange(xSheetCellRange), UNO_QUERY_THROW);

    xSheet->getCellByPosition(1, 1)->setValue(1);
    xSheet->getCellByPosition(4, 5)->setValue(1);
    xSheet->getCellByPosition(3, 2)->setFormula("xTextDoc");
    xSheet->getCellByPosition(3, 3)->setFormula("xTextDoc");

    return xCellCursor;
}

uno::Reference< uno::XInterface > ScCellCursorObj::getXCellRangeData()
{
    return init();
}

uno::Reference< uno::XInterface > ScCellCursorObj::getXSpreadsheet()
{
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, UNO_QUERY_THROW);

    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(0), UNO_QUERY_THROW);

    setXCell(xSheet->getCellByPosition(15, 15));

    return xSheet;
}

void ScCellCursorObj::setUp()
{
    CalcUnoApiTest::setUp();

    OUString aFileURL;
    createFileURL(u"ScCellCursorObj.ods", aFileURL);
    mxComponent = loadFromDesktop(aFileURL, "com.sun.star.sheet.SpreadsheetDocument");
}

void ScCellCursorObj::tearDown()
{
    closeDocument(mxComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScCellCursorObj);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
