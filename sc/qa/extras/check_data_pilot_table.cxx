/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/XDataPilotTable.hpp>
#include <com/sun/star/sheet/XDataPilotDescriptor.hpp>
#include <com/sun/star/sheet/XDataPilotTables.hpp>
#include <com/sun/star/sheet/XDataPilotTablesSupplier.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>

#include "test/container/xnamed.hxx"
#include "test/sheet/xdatapilottable.hxx"
#include "test/sheet/xdatapilotdescriptor.hxx"
#include "test/beans/xpropertyset.hxx"

// check the DataPilot of Calc

using namespace css;
using namespace css::lang;
using namespace css::frame;

namespace sc_apitest {

class CheckDataPilotTable : public CalcUnoApiTest, public apitest::XNamed, public apitest::XDataPilotTable, public apitest::XPropertySet, public apitest::XDataPilotDescriptor
{
public:
    CheckDataPilotTable();

    virtual void setUp() override;
    virtual void tearDown() override;

    uno::Reference< uno::XInterface > init() override;

    CPPUNIT_TEST_SUITE(CheckDataPilotTable);
    // _XNamed
    CPPUNIT_TEST(testGetName);
    CPPUNIT_TEST(testSetName);

    // _XDataPilotTable
    CPPUNIT_TEST(testGetOutputRange);

    // _XDataPilotDescriptor
    CPPUNIT_TEST(testTag);
    CPPUNIT_TEST(testGetFilterDescriptor);
    CPPUNIT_TEST(testGetDataPilotFields);
    CPPUNIT_TEST(testGetColumnFields);
    CPPUNIT_TEST(testGetRowFields);
    // CPPUNIT_TEST(testGetDataFields);
    // CPPUNIT_TEST(testGetHiddenFields);
    CPPUNIT_TEST(testGetPageFields);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<lang::XComponent> mxComponent;
    uno::Reference<uno::XInterface> mxObject;
    int mMaxFieldIndex = 6;
};

CheckDataPilotTable::CheckDataPilotTable()
     : CalcUnoApiTest("/sc/qa/extras/testdocuments"),
       apitest::XNamed(OUString("DataPilotTable"))
{
}

uno::Reference< uno::XInterface > CheckDataPilotTable::init()
{
    // create a calc document
    if (!mxComponent.is())
        // Load an empty document.
        mxComponent = loadFromDesktop("private:factory/scalc");
    else
        return mxObject;

    uno::Reference< sheet::XSpreadsheetDocument > xSheetDoc( mxComponent, uno::UNO_QUERY );
    CPPUNIT_ASSERT_MESSAGE( "no calc document", xSheetDoc.is() );

    // the cell range
    table::CellRangeAddress sCellRangeAdress;
    sCellRangeAdress.Sheet = 0;
    sCellRangeAdress.StartColumn = 1;
    sCellRangeAdress.StartRow = 0;
    sCellRangeAdress.EndColumn = mMaxFieldIndex-1;
    sCellRangeAdress.EndRow = mMaxFieldIndex - 1;

    // position of the data pilot table
    table::CellAddress sCellAdress;
    sCellAdress.Sheet = 0;
    sCellAdress.Column = 7;
    sCellAdress.Row = 8;
    // Getting spreadsheet
    uno::Reference< sheet::XSpreadsheets > xSpreadsheets = xSheetDoc->getSheets();
    uno::Reference< container::XIndexAccess > oIndexAccess(xSpreadsheets, uno::UNO_QUERY_THROW);

    // Per default there's now just one sheet, make sure we have at least two, then
    xSpreadsheets->insertNewByName("Some Sheet", 0);
    uno::Any aAny = oIndexAccess->getByIndex(0);
    uno::Reference< sheet::XSpreadsheet > oSheet;
    CPPUNIT_ASSERT(aAny >>= oSheet);

    uno::Any aAny2 = oIndexAccess->getByIndex(1);
    uno::Reference< sheet::XSpreadsheet > oSheet2;
    CPPUNIT_ASSERT(aAny2 >>= oSheet2);

    //Filling a table
    for (int i = 1; i < mMaxFieldIndex; i++)
    {
        oSheet->getCellByPosition(i, 0)->setFormula("Col" + OUString::number(i));
        oSheet->getCellByPosition(0, i)->setFormula("Row" + OUString::number(i));
        oSheet2->getCellByPosition(i, 0)->setFormula("Col" + OUString::number(i));
        oSheet2->getCellByPosition(0, i)->setFormula("Row" + OUString::number(i));
    }

    for (int i = 1; i < mMaxFieldIndex; i++)
    {
        for (int j = 1; j < mMaxFieldIndex; j++)
        {
            oSheet->getCellByPosition(i, j)->setValue(i * (j + 1));
            oSheet2->getCellByPosition(i, j)->setValue(i * (j + 2));
         }
    }

    // change a value of a cell and check the change in the data pilot
    // cell of data
    uno::Any oChangeCell;
    oChangeCell<<= oSheet->getCellByPosition(1, 5);
    int x = sCellAdress.Column;
    int y = sCellAdress.Row + 3;
    // cell of the data pilot output
    uno::Any oCheckCell;
    oCheckCell<<= oSheet->getCellByPosition(x, y);
    // create the test objects
    uno::Reference< sheet::XDataPilotTablesSupplier> DPTS(oSheet, uno::UNO_QUERY_THROW);
    uno::Reference< sheet::XDataPilotTables> DPT = DPTS->getDataPilotTables();
    uno::Reference< sheet::XDataPilotDescriptor> DPDsc = DPT->createDataPilotDescriptor();
    DPDsc->setSourceRange(sCellRangeAdress);

    uno::Any oDataPilotField = DPDsc->getDataPilotFields()->getByIndex(0);
    uno::Reference<beans::XPropertySet> fieldPropSet(oDataPilotField,  uno::UNO_QUERY_THROW);

    uno::Any sum;
    sum<<= sheet::GeneralFunction_SUM;
    fieldPropSet->setPropertyValue("Function", sum );

    uno::Any data;
    data<<= sheet::DataPilotFieldOrientation_DATA;
    fieldPropSet->setPropertyValue("Orientation", data);

    //Insert the DataPilotTable
    if (DPT->hasByName("DataPilotTable"))
        DPT->removeByName("DataPilotTable");
    DPT->insertNewByName("DataPilotTable", sCellAdress, DPDsc);

    uno::Reference<uno::XInterface> xDataPilotTableObject;
    data = DPT->getByName( DPT->getElementNames()[0] );
    CPPUNIT_ASSERT(data >>= xDataPilotTableObject);
    mxObject = xDataPilotTableObject;

    return xDataPilotTableObject;
}

void CheckDataPilotTable::setUp()
{
    CalcUnoApiTest::setUp();
    init();
}

void CheckDataPilotTable::tearDown()
{
    closeDocument(mxComponent);
    mxComponent.clear();
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(CheckDataPilotTable);

}

CPPUNIT_PLUGIN_IMPLEMENT();
