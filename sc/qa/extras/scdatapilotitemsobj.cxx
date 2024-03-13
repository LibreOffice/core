/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/container/xelementaccess.hxx>
#include <test/container/xenumerationaccess.hxx>
#include <test/container/xindexaccess.hxx>
#include <test/container/xnameaccess.hxx>
#include <test/lang/xserviceinfo.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/XDataPilotDescriptor.hpp>
#include <com/sun/star/sheet/XDataPilotField.hpp>
#include <com/sun/star/sheet/XDataPilotTables.hpp>
#include <com/sun/star/sheet/XDataPilotTablesSupplier.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;

namespace sc_apitest
{
class ScDataPilotItemsObj : public UnoApiTest,
                            public apitest::XElementAccess,
                            public apitest::XEnumerationAccess,
                            public apitest::XIndexAccess,
                            public apitest::XNameAccess,
                            public apitest::XServiceInfo
{
public:
    ScDataPilotItemsObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(ScDataPilotItemsObj);

    // XElementAccess
    CPPUNIT_TEST(testGetElementType);
    CPPUNIT_TEST(testHasElements);

    // XEnumerationAccess
    CPPUNIT_TEST(testCreateEnumeration);

    // XIndexAccess
    CPPUNIT_TEST(testGetByIndex);
    CPPUNIT_TEST(testGetCount);

    // XNameAccess
    CPPUNIT_TEST(testGetByName);
    CPPUNIT_TEST(testGetElementNames);
    CPPUNIT_TEST(testHasByName);

    // XServiceInfo
    CPPUNIT_TEST(testGetImplementationName);
    CPPUNIT_TEST(testGetSupportedServiceNames);
    CPPUNIT_TEST(testSupportsService);

    CPPUNIT_TEST_SUITE_END();

private:
    static const int m_nMaxFieldIndex = 6;
};

ScDataPilotItemsObj::ScDataPilotItemsObj()
    : UnoApiTest("/sc/qa/extras/testdocuments")
    , XElementAccess(cppu::UnoType<beans::XPropertySet>::get())
    , XIndexAccess(5)
    , XNameAccess("2")
    , XServiceInfo("ScDataPilotItemsObj", "com.sun.star.sheet.DataPilotItems")
{
}

uno::Reference<uno::XInterface> ScDataPilotItemsObj::init()
{
    table::CellRangeAddress aCellRangeAddress(0, 1, 0, m_nMaxFieldIndex - 1, m_nMaxFieldIndex - 1);
    table::CellAddress aCellAddress(0, 7, 8);

    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), uno::UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIA(xSheets, uno::UNO_QUERY_THROW);
    xSheets->insertNewByName("Some Sheet", 0);

    uno::Reference<sheet::XSpreadsheet> xSheet0(xIA->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet1(xIA->getByIndex(1), uno::UNO_QUERY_THROW);

    for (auto i = 1; i < m_nMaxFieldIndex; ++i)
    {
        xSheet0->getCellByPosition(i, 0)->setFormula("Col" + OUString::number(i));
        xSheet0->getCellByPosition(0, i)->setFormula("Row" + OUString::number(i));
        xSheet1->getCellByPosition(i, 0)->setFormula("Col" + OUString::number(i));
        xSheet1->getCellByPosition(0, i)->setFormula("Row" + OUString::number(i));
    }

    for (auto i = 1; i < m_nMaxFieldIndex; ++i)
    {
        for (auto j = 1; j < m_nMaxFieldIndex; ++j)
        {
            xSheet0->getCellByPosition(i, j)->setValue(i * (j + 1));
            xSheet1->getCellByPosition(i, j)->setValue(i * (j + 2));
        }
    }

    xSheet0->getCellByPosition(1, 5);
    xSheet0->getCellByPosition(aCellAddress.Column, aCellAddress.Row + 3);

    uno::Reference<sheet::XDataPilotTablesSupplier> xDPTS(xSheet0, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XDataPilotTables> xDPT(xDPTS->getDataPilotTables(), uno::UNO_SET_THROW);
    uno::Reference<sheet::XDataPilotDescriptor> xDPD(xDPT->createDataPilotDescriptor(),
                                                     uno::UNO_SET_THROW);

    xDPD->setSourceRange(aCellRangeAddress);

    uno::Reference<beans::XPropertySet> xPropertySet(xDPD->getDataPilotFields()->getByIndex(1),
                                                     uno::UNO_QUERY_THROW);
    xPropertySet->setPropertyValue("Function", uno::Any(sheet::GeneralFunction_SUM));
    xPropertySet->setPropertyValue("Orientation", uno::Any(sheet::DataPilotFieldOrientation_DATA));

    xDPT->insertNewByName("DataPilotTable", aCellAddress, xDPD);

    uno::Reference<container::XIndexAccess> xIA_DPF(xDPD->getDataPilotFields(), uno::UNO_SET_THROW);
    uno::Reference<sheet::XDataPilotField> xDPF(xIA_DPF->getByIndex(0), uno::UNO_QUERY_THROW);

    return xDPF->getItems();
}

void ScDataPilotItemsObj::setUp()
{
    UnoApiTest::setUp();
    // create a calc document
    mxComponent = loadFromDesktop("private:factory/scalc");
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScDataPilotItemsObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
