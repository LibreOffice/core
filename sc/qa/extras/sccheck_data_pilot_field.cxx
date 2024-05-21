/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
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
#include <test/container/xnamed.hxx>
#include <test/beans/xpropertyset.hxx>
//check the DataPilot of Calc.

using namespace css;

namespace sc_apitest
{
class CheckDataPilotField : public UnoApiTest, public apitest::XNamed, public apitest::XPropertySet
{
public:
    CheckDataPilotField();

    virtual void setUp() override;

    uno::Reference<uno::XInterface> init() override;

    CPPUNIT_TEST_SUITE(CheckDataPilotField);

    // _XNamed
    CPPUNIT_TEST(testGetName);
    CPPUNIT_TEST(testSetName);

    // XPropertySet
    CPPUNIT_TEST(testGetPropertySetInfo);
    CPPUNIT_TEST(testSetPropertyValue);
    CPPUNIT_TEST(testGetPropertyValue);

    CPPUNIT_TEST_SUITE_END();

protected:
    virtual bool isPropertyIgnored(const OUString& rName) override;

private:
    uno::Reference<uno::XInterface> mxObject;
    int mMaxFieldIndex = 6;
};

bool CheckDataPilotField::isPropertyIgnored(const OUString& rName)
{
    return rName == "Function" || rName == "Subtotals" || rName == "Function2"
           || rName == "Subtotals2";
}

CheckDataPilotField::CheckDataPilotField()
    : UnoApiTest(u"/sc/qa/extras/testdocuments"_ustr)
    , apitest::XNamed(u"Col1"_ustr)
{
}

uno::Reference<uno::XInterface> CheckDataPilotField::init()
{
    // create a calc document
    if (!mxComponent.is())
        // Load an empty document.
        mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);
    else
        return mxObject;

    uno::Reference<sheet::XSpreadsheetDocument> xSheetDoc(mxComponent, uno::UNO_QUERY_THROW);

    // the cell range
    table::CellRangeAddress sCellRangeAddress;
    sCellRangeAddress.Sheet = 0;
    sCellRangeAddress.StartColumn = 1;
    sCellRangeAddress.StartRow = 0;
    sCellRangeAddress.EndColumn = mMaxFieldIndex - 1;
    sCellRangeAddress.EndRow = mMaxFieldIndex - 1;

    // position of the data pilot table
    table::CellAddress sCellAddress;
    sCellAddress.Sheet = 0;
    sCellAddress.Column = 7;
    sCellAddress.Row = 8;
    // Getting spreadsheet
    uno::Reference<sheet::XSpreadsheets> xSpreadsheets = xSheetDoc->getSheets();
    uno::Reference<container::XIndexAccess> oIndexAccess(xSpreadsheets, uno::UNO_QUERY_THROW);

    // Per default there's now just one sheet, make sure we have at least two, then
    xSpreadsheets->insertNewByName(u"Some Sheet"_ustr, 0);
    uno::Any aAny = oIndexAccess->getByIndex(0);
    uno::Reference<sheet::XSpreadsheet> oSheet;
    CPPUNIT_ASSERT(aAny >>= oSheet);

    uno::Any aAny2 = oIndexAccess->getByIndex(1);
    uno::Reference<sheet::XSpreadsheet> oSheet2;
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
    oChangeCell <<= oSheet->getCellByPosition(1, 5);
    int x = sCellAddress.Column;
    int y = sCellAddress.Row + 3;
    // cell of the data pilot output
    uno::Any oCheckCell;
    oCheckCell <<= oSheet->getCellByPosition(x, y);
    // create the test objects
    uno::Reference<sheet::XDataPilotTablesSupplier> DPTS(oSheet, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XDataPilotTables> DPT = DPTS->getDataPilotTables();
    uno::Reference<sheet::XDataPilotDescriptor> DPDsc = DPT->createDataPilotDescriptor();
    DPDsc->setSourceRange(sCellRangeAddress);

    uno::Any oDataPilotField = DPDsc->getDataPilotFields()->getByIndex(0);
    uno::Reference<beans::XPropertySet> fieldPropSet(oDataPilotField, uno::UNO_QUERY_THROW);

    uno::Any sum;
    sum <<= sheet::GeneralFunction_SUM;
    fieldPropSet->setPropertyValue(u"Function"_ustr, sum);

    uno::Any data;
    data <<= sheet::DataPilotFieldOrientation_DATA;
    fieldPropSet->setPropertyValue(u"Orientation"_ustr, data);

    //Insert the DataPilotTable
    if (DPT->hasByName(u"DataPilotField"_ustr))
        DPT->removeByName(u"DataPilotField"_ustr);
    DPT->insertNewByName(u"DataPilotTField"_ustr, sCellAddress, DPDsc);

    uno::Reference<container::XIndexAccess> IA = DPDsc->getDataPilotFields();
    uno::Reference<uno::XInterface> xDataPilotFieldObject;
    data = IA->getByIndex(0);
    CPPUNIT_ASSERT(data >>= xDataPilotFieldObject);
    mxObject = xDataPilotFieldObject;

    return xDataPilotFieldObject;
}

void CheckDataPilotField::setUp()
{
    UnoApiTest::setUp();
    init();
}

CPPUNIT_TEST_SUITE_REGISTRATION(CheckDataPilotField);
}

CPPUNIT_PLUGIN_IMPLEMENT();
