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
#include <test/sheet/xcellformatrangessupplier.hxx>
#include <test/sheet/xcellrangeaddressable.hxx>
#include <test/sheet/xcellseries.hxx>
#include <test/sheet/xmultipleoperation.hxx>
#include <test/sheet/xsheetcellrange.hxx>
#include <test/sheet/xsheetfilterable.hxx>
#include <test/sheet/xsheetfilterableex.hxx>
#include <test/sheet/xsheetoperation.hxx>
#include <test/sheet/xsubtotalcalculatable.hxx>
#include <test/sheet/xusedareacursor.hxx>
#include <test/sheet/xuniquecellformatrangessupplier.hxx>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/XCellCursor.hpp>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

class ScCellCursorObj : public CalcUnoApiTest, public apitest::SheetCellRange,
                                               public apitest::XCellFormatRangesSupplier,
                                               public apitest::XCellRangeAddressable,
                                               public apitest::XCellSeries,
                                               public apitest::XMultipleOperation,
                                               public apitest::XSheetCellRange,
                                               public apitest::XSheetFilterable,
                                               public apitest::XSheetFilterableEx,
                                               public apitest::XSheetOperation,
                                               public apitest::XSubTotalCalculatable,
                                               public apitest::XUsedAreaCursor,
                                               public apitest::XUniqueCellFormatRangesSupplier
{
public:
    ScCellCursorObj();

    virtual void setUp() override;
    virtual void tearDown() override;
    virtual uno::Reference< uno::XInterface > init() override;
    virtual uno::Reference< uno::XInterface > getXSpreadsheet() override;

    CPPUNIT_TEST_SUITE(ScCellCursorObj);

    // SheetCellRange
    CPPUNIT_TEST(testSheetCellRangeProperties);

    // XUsedAreaCursor
    CPPUNIT_TEST(testGotoStartOfUsedArea);
    CPPUNIT_TEST(testGotoEndOfUsedArea);

    // XMultipleOperation
    CPPUNIT_TEST(testSetTableOperation);

    // XCellFormatRangesSupplier
    CPPUNIT_TEST(testGetCellFormatRanges);

    // XCellRangeAddressable
    CPPUNIT_TEST(testGetRangeAddress);

    // XCellSeries
    CPPUNIT_TEST(testFillAuto);
    CPPUNIT_TEST(testFillSeries);

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

    // XUniqueCellFormatRangesSupplier
    CPPUNIT_TEST(testGetUniqueCellFormatRanges);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference< lang::XComponent > mxComponent;
};

ScCellCursorObj::ScCellCursorObj():
    CalcUnoApiTest("/sc/qa/extras/testdocuments"),
    apitest::XCellSeries(0, 0)
{
}

uno::Reference< uno::XInterface > ScCellCursorObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document", xDoc.is());

    uno::Reference<container::XIndexAccess> xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), UNO_QUERY_THROW);

    uno::Reference<table::XCellRange> xCellRange = xSheet->getCellRangeByName("$A$1:$D$4");
    uno::Reference<sheet::XSheetCellRange> xSheetCellRange(xCellRange, UNO_QUERY_THROW);
    uno::Reference<table::XCellCursor> xCellCursor(xSheet->createCursorByRange(xSheetCellRange), UNO_QUERY_THROW);

    return xCellCursor;
}

uno::Reference< uno::XInterface > ScCellCursorObj::getXSpreadsheet()
{
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document", xDoc.is());

    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(0), UNO_QUERY_THROW);

    return xSheet;
}

void ScCellCursorObj::setUp()
{
    CalcUnoApiTest::setUp();

    OUString aFileURL;
    createFileURL("ScCellCursorObj.ods", aFileURL);
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
