/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/sheet/xdatapilottable.hxx>
#include <test/sheet/xdatapilottable2.hxx>
#include <test/sheet/xdatapilotdescriptor.hxx>
#include <test/container/xnamed.hxx>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XDataPilotTablesSupplier.hpp>
#include <com/sun/star/sheet/XDataPilotTables.hpp>
#include <com/sun/star/sheet/XDataPilotTable.hpp>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

#define NUMBER_OF_TESTS 16

class ScDataPilotTableObj : public CalcUnoApiTest, apitest::XDataPilotDescriptor, apitest::XDataPilotTable,
                                apitest::XNamed, apitest::XDataPilotTable2
{
public:
    ScDataPilotTableObj();

    virtual void setUp() override;
    virtual void tearDown() override;
    virtual uno::Reference< uno::XInterface > init() override;
    virtual uno::Reference< uno::XInterface > initDP2() override;
    virtual uno::Reference< uno::XInterface > getSheets() override;

    CPPUNIT_TEST_SUITE(ScDataPilotTableObj);
    CPPUNIT_TEST(testRefresh);
    //CPPUNIT_TEST(testGetHiddenFields);
    CPPUNIT_TEST(testGetOutputRange);
    CPPUNIT_TEST(testSourceRange);
    CPPUNIT_TEST(testTag);
    CPPUNIT_TEST(testGetFilterDescriptor);
    CPPUNIT_TEST(testGetDataPilotFields);
    CPPUNIT_TEST(testGetColumnFields);
    CPPUNIT_TEST(testGetRowFields);
    CPPUNIT_TEST(testGetPageFields);
    CPPUNIT_TEST(testGetDataFields);
    CPPUNIT_TEST(testGetName);
    CPPUNIT_TEST(testSetName);
    CPPUNIT_TEST(testGetDrillDownData);
    CPPUNIT_TEST(testInsertDrillDownSheet);
    CPPUNIT_TEST(testGetPositionData);
    CPPUNIT_TEST(testGetOutputRangeByType);
    CPPUNIT_TEST_SUITE_END();

private:
    static sal_Int32 nTest;
    static uno::Reference< lang::XComponent > mxComponent;
};

sal_Int32 ScDataPilotTableObj::nTest = 0;
uno::Reference< lang::XComponent > ScDataPilotTableObj::mxComponent;

ScDataPilotTableObj::ScDataPilotTableObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments"),
      apitest::XNamed(OUString("DataPilotTable"))
{
}

uno::Reference< uno::XInterface > ScDataPilotTableObj::init()
{
    OUString aFileURL;
    createFileURL("ScDataPilotTableObj.ods", aFileURL);
    if(!mxComponent.is())
        mxComponent = loadFromDesktop(aFileURL, "com.sun.star.sheet.SpreadsheetDocument");
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(0), UNO_QUERY_THROW);

    // set variables from xdatapilottable.[ch]xx
    xCellForChange = xSheet->getCellByPosition( 1, 5 );
    xCellForCheck = xSheet->getCellByPosition( 7, 11 );
    CPPUNIT_ASSERT(xCellForCheck.is());
    CPPUNIT_ASSERT(xCellForChange.is());

    CPPUNIT_ASSERT_MESSAGE("Could not create interface of type XSpreadsheet", xSheet.is());
    uno::Reference< sheet::XDataPilotTablesSupplier > xDPTS(xSheet, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDPTS.is());
    uno::Reference< sheet::XDataPilotTables > xDPT = xDPTS->getDataPilotTables();
    CPPUNIT_ASSERT(xDPT.is());

    uno::Reference< sheet::XDataPilotTable > xDPTable(xDPT->getByName("DataPilotTable"),UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xDPTable.is());
    return xDPTable;
}

uno::Reference< uno::XInterface > ScDataPilotTableObj::getSheets()
{
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference< uno::XInterface > xSheets(xDoc->getSheets());
    return xSheets;
}

uno::Reference< uno::XInterface > ScDataPilotTableObj::initDP2()
{
    OUString aFileURL;
    createFileURL(OUString("ScDataPilotTableObj.ods"), aFileURL);
    if(!mxComponent.is())
        mxComponent = loadFromDesktop(aFileURL, "com.sun.star.sheet.SpreadsheetDocument");
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(0), UNO_QUERY_THROW);

    // set variables from xdatapilottable.[ch]xx
    xCellForChange = xSheet->getCellByPosition( 1, 5 );
    xCellForCheck = xSheet->getCellByPosition( 7, 11 );
    CPPUNIT_ASSERT(xCellForCheck.is());
    CPPUNIT_ASSERT(xCellForChange.is());

    CPPUNIT_ASSERT_MESSAGE("Could not create interface of type XSpreadsheet", xSheet.is());
    uno::Reference< sheet::XDataPilotTablesSupplier > xDPTS(xSheet, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDPTS.is());
    uno::Reference< sheet::XDataPilotTables > xDPT = xDPTS->getDataPilotTables();
    CPPUNIT_ASSERT(xDPT.is());

    uno::Reference< sheet::XDataPilotTable > xDPTable(xDPT->getByName("DataPilotTable2"),UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xDPTable.is());
    return xDPTable;
}

void ScDataPilotTableObj::setUp()
{
    nTest++;
    CalcUnoApiTest::setUp();
}

void ScDataPilotTableObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
    {
        closeDocument(mxComponent);
        mxComponent.clear();
    }

    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScDataPilotTableObj);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
