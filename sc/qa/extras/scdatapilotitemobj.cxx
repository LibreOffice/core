/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/sheet/datapilotitem.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XDataPilotDescriptor.hpp>
#include <com/sun/star/sheet/XDataPilotField.hpp>
#include <com/sun/star/sheet/XDataPilotTablesSupplier.hpp>
#include <com/sun/star/sheet/XDataPilotTables.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Reference.hxx>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

class ScDataPilotItemObj : public CalcUnoApiTest, public apitest::DataPilotItem
{
public:
    virtual void setUp() override;
    virtual void tearDown() override;
    virtual uno::Reference< uno::XInterface > init() override;

    ScDataPilotItemObj();

    CPPUNIT_TEST_SUITE(ScDataPilotItemObj);

    // DataPilotItem
    CPPUNIT_TEST(testProperties);

    CPPUNIT_TEST_SUITE_END();

private:
    static sal_Int32 nMaxFieldIndex;
    uno::Reference< lang::XComponent > mxComponent;
};

sal_Int32 ScDataPilotItemObj::nMaxFieldIndex = 6;

ScDataPilotItemObj::ScDataPilotItemObj()
     : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference< uno::XInterface > ScDataPilotItemObj::init()
{
    table::CellRangeAddress sCellRangeAddress(0, 1, 0, nMaxFieldIndex - 1, nMaxFieldIndex - 1);
    table::CellAddress sCellAddress(0, 7, 8);

    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document", xDoc.is());

    // we need to sheets
    uno::Reference< sheet::XSpreadsheets > xSheets(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    xSheets->insertNewByName("Some Sheet", 0);

    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet1( xIndex->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet2( xIndex->getByIndex(1), uno::UNO_QUERY_THROW);

    for (auto i = 1; i < nMaxFieldIndex; i++)
    {
        xSheet1->getCellByPosition(i, 0)->setFormula(OUString("Col" + OUString::number(i)));
        xSheet1->getCellByPosition(0, 1)->setFormula(OUString("Row" + OUString::number(i)));
        xSheet2->getCellByPosition(i, 0)->setFormula(OUString("Col" + OUString::number(i)));
        xSheet2->getCellByPosition(0, 1)->setFormula(OUString("Row" + OUString::number(i)));
    }

    for (auto i = 1; i < nMaxFieldIndex; i++)
        for (auto j = 1; j < nMaxFieldIndex; j++)
        {
            xSheet1->getCellByPosition(i, j)->setValue(i * (j + 2));
            xSheet2->getCellByPosition(i, j)->setValue(i * (j + 2));
        }

    xSheet1->getCellByPosition(1,5);
    xSheet1->getCellByPosition(sCellAddress.Column, sCellAddress.Row + 3);

    uno::Reference< sheet::XDataPilotTablesSupplier > xDPTS(xSheet1, uno::UNO_QUERY_THROW);
    uno::Reference< sheet::XDataPilotTables > xDPT = xDPTS->getDataPilotTables();
    uno::Reference< sheet::XDataPilotDescriptor > xDPD = xDPT->createDataPilotDescriptor();
    xDPD->setSourceRange(sCellRangeAddress);

    uno::Reference< beans::XPropertySet > xDataPilotFieldProp(xDPD->getDataPilotFields()->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Any aGF; aGF <<= sheet::GeneralFunction_SUM;
    xDataPilotFieldProp->setPropertyValue("Function", aGF);
    uno::Any aDPFO; aDPFO <<= sheet::DataPilotFieldOrientation_DATA;
    xDataPilotFieldProp->setPropertyValue("Orientation", aDPFO);

    if (xDPT->hasByName("DataPilotTable"))
        xDPT->removeByName("DataPilotTable");

    xIndex = xDPD->getDataPilotFields();

    xDPT->insertNewByName("DataPilotTable", sCellAddress, xDPD);
    uno::Reference< sheet::XDataPilotField > xDPF(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);
    return xDPF->getItems();
}

void ScDataPilotItemObj::setUp()
{
    CalcUnoApiTest::setUp();
    // create calc document
    mxComponent = loadFromDesktop("private:factory/scalc");
}

void ScDataPilotItemObj::tearDown()
{
    closeDocument(mxComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScDataPilotItemObj);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
