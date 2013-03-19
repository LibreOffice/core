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
 * Copyright (C) 2012 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
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
#include <test/sheet/xdatapilottable.hxx>
#include <test/sheet/xdatapilottable2.hxx>
#include <test/sheet/xdatapilotdescriptor.hxx>
#include <test/container/xnamed.hxx>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XDataPilotTablesSupplier.hpp>
#include <com/sun/star/sheet/XDataPilotTables.hpp>
#include <com/sun/star/sheet/XDataPilotTable.hpp>

namespace sc_apitest {

#define NUMBER_OF_TESTS 16

class ScDataPilotTableObj : public UnoApiTest, apitest::XDataPilotDescriptor, apitest::XDataPilotTable,
                                apitest::XNamed, apitest::XDataPilotTable2
{
public:
    ScDataPilotTableObj();

    virtual void setUp();
    virtual void tearDown();
    virtual uno::Reference< uno::XInterface > init();
    virtual uno::Reference< uno::XInterface > initDP2();
    virtual uno::Reference< uno::XInterface > getSheets();

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
    : UnoApiTest("/sc/qa/extras/testdocuments"),
      apitest::XNamed(rtl::OUString("DataPilotTable"))
{
}

uno::Reference< uno::XInterface > ScDataPilotTableObj::init()
{
    rtl::OUString aFileURL;
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

    uno::Reference< sheet::XDataPilotTable > xDPTable(xDPT->getByName(rtl::OUString("DataPilotTable")),UNO_QUERY_THROW);

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
    rtl::OUString aFileURL;
    createFileURL(rtl::OUString("ScDataPilotTableObj.ods"), aFileURL);
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

    uno::Reference< sheet::XDataPilotTable > xDPTable(xDPT->getByName(rtl::OUString("DataPilotTable2")),UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xDPTable.is());
    return xDPTable;
}

void ScDataPilotTableObj::setUp()
{
    nTest++;
    UnoApiTest::setUp();
}

void ScDataPilotTableObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
        closeDocument(mxComponent);

    UnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScDataPilotTableObj);

CPPUNIT_PLUGIN_IMPLEMENT();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
