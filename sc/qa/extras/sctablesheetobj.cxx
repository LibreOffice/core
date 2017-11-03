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
#include <test/sheet/xprintareas.hxx>
#include <test/sheet/xsheetcellrange.hxx>
#include <test/sheet/xsheetfilterable.hxx>
#include <test/sheet/xsheetlinkable.hxx>
#include <test/sheet/xsheetoperation.hxx>
#include <test/sheet/xsheetpagebreak.hxx>
#include <test/sheet/xspreadsheet.hxx>
#include <test/sheet/xsubtotalcalculatable.hxx>
#include <test/sheet/xuniquecellformatrangessupplier.hxx>
#include <test/util/xreplaceable.hxx>
#include <test/util/xsearchable.hxx>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>

using namespace css;
using namespace css::uno;

namespace sc_apitest
{

#define NUMBER_OF_TESTS 23

class ScTableSheetObj : public CalcUnoApiTest, public apitest::XCellSeries,
                                               public apitest::XPrintAreas,
                                               public apitest::XReplaceable,
                                               public apitest::XSearchable,
                                               public apitest::XSheetCellRange,
                                               public apitest::XSheetFilterable,
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

    CPPUNIT_TEST_SUITE(ScTableSheetObj);

    // XCellSeries
    CPPUNIT_TEST(testFillAuto);
    CPPUNIT_TEST(testFillSeries);

    // XPrintAreas
    CPPUNIT_TEST(testSetAndGetPrintTitleColumns);
    CPPUNIT_TEST(testSetAndGetPrintTitleRows);

    // XReplaceable
    CPPUNIT_TEST(testReplaceAll);
    CPPUNIT_TEST(testCreateReplaceDescriptor);

    // XSearchable
    CPPUNIT_TEST(testFindAll);
    CPPUNIT_TEST(testFindNext);
    CPPUNIT_TEST(testFindFirst);

    // XSheetCellRange
    CPPUNIT_TEST(testGetSpreadsheet);

    // XSheetFilterable
    CPPUNIT_TEST(testCreateFilterDescriptor);
    CPPUNIT_TEST(testFilter);

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
    //OUString aFileURL;
    createFileURL("ScTableSheetObj.ods", maFileURL);
    if(!mxComponent.is())
        mxComponent = loadFromDesktop(maFileURL, "com.sun.star.sheet.SpreadsheetDocument");
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(0), UNO_QUERY_THROW);

    return xSheet;
}

uno::Reference< uno::XInterface > ScTableSheetObj::getXSpreadsheet()
{
    OUString aFileURL;
    createFileURL("ScTableSheetObj.ods", aFileURL);
    if(!mxComponent.is())
        mxComponent = loadFromDesktop(aFileURL, "com.sun.star.sheet.SpreadsheetDocument");
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(1), UNO_QUERY_THROW);

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
