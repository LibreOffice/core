/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/container/xnamed.hxx>
#include <test/sheet/datapilotitem.hxx>
#include <rtl/string.hxx>

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

namespace sc_apitest
{
class ScDataPilotItemObj : public CalcUnoApiTest,
                           public apitest::DataPilotItem,
                           public apitest::XNamed
{
public:
    virtual void setUp() override;
    virtual void tearDown() override;
    virtual uno::Reference<uno::XInterface> init() override;

    ScDataPilotItemObj();

    CPPUNIT_TEST_SUITE(ScDataPilotItemObj);

    // DataPilotItem
    CPPUNIT_TEST(testProperties);

    // XNamed
    CPPUNIT_TEST(testGetName);

    CPPUNIT_TEST_SUITE_END();

private:
    static const int m_nMaxFieldIndex = 6;
    uno::Reference<lang::XComponent> m_xComponent;
};

ScDataPilotItemObj::ScDataPilotItemObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
    , XNamed("2")
{
}

uno::Reference<uno::XInterface> ScDataPilotItemObj::init()
{
    table::CellRangeAddress aCellRangeAddress(0, 1, 0, m_nMaxFieldIndex - 1, m_nMaxFieldIndex - 1);
    table::CellAddress aCellAddress(0, 7, 8);

    uno::Reference<sheet::XSpreadsheetDocument> xDoc(m_xComponent, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), uno::UNO_QUERY_THROW);

    xSheets->insertNewByName("Some Sheet", 0);

    uno::Reference<container::XIndexAccess> xIA(xSheets, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet0(xIA->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet1(xIA->getByIndex(1), uno::UNO_QUERY_THROW);

    for (auto i = 1; i < m_nMaxFieldIndex; i++)
    {
        xSheet0->getCellByPosition(i, 0)->setFormula("Col" + OUString::number(i));
        xSheet0->getCellByPosition(0, i)->setFormula("Row" + OUString::number(i));
        xSheet1->getCellByPosition(i, 0)->setFormula("Col" + OUString::number(i));
        xSheet1->getCellByPosition(0, i)->setFormula("Row" + OUString::number(i));
    }

    for (auto i = 1; i < m_nMaxFieldIndex; i++)
        for (auto j = 1; j < m_nMaxFieldIndex; j++)
        {
            xSheet0->getCellByPosition(i, j)->setValue(i * (j + 1));
            xSheet1->getCellByPosition(i, j)->setValue(i * (j + 2));
        }

    xSheet0->getCellByPosition(1, 5);
    xSheet0->getCellByPosition(aCellAddress.Column, aCellAddress.Row + 3);

    uno::Reference<sheet::XDataPilotTablesSupplier> xDPTS(xSheet0, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XDataPilotTables> xDPT(xDPTS->getDataPilotTables(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XDataPilotDescriptor> xDPD(xDPT->createDataPilotDescriptor(),
                                                     uno::UNO_QUERY_THROW);
    xDPD->setSourceRange(aCellRangeAddress);

    uno::Reference<beans::XPropertySet> xDataPilotFieldProp(
        xDPD->getDataPilotFields()->getByIndex(0), uno::UNO_QUERY_THROW);
    xDataPilotFieldProp->setPropertyValue("Function", uno::makeAny(sheet::GeneralFunction_SUM));
    xDataPilotFieldProp->setPropertyValue("Orientation",
                                          uno::makeAny(sheet::DataPilotFieldOrientation_DATA));

    if (xDPT->hasByName("DataPilotTable"))
        xDPT->removeByName("DataPilotTable");

    uno::Reference<container::XIndexAccess> xIA_DPF(xDPD->getDataPilotFields(), uno::UNO_QUERY_THROW);

    xDPT->insertNewByName("DataPilotTable", aCellAddress, xDPD);
    uno::Reference<sheet::XDataPilotField> xDPF(xIA_DPF->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<uno::XInterface> xReturn(xDPF->getItems()->getByIndex(0), uno::UNO_QUERY_THROW);
    return xReturn;
}

void ScDataPilotItemObj::setUp()
{
    CalcUnoApiTest::setUp();
    // create calc document
    m_xComponent = loadFromDesktop("private:factory/scalc");
}

void ScDataPilotItemObj::tearDown()
{
    closeDocument(m_xComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScDataPilotItemObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
