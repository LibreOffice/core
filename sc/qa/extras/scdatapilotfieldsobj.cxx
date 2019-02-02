/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/container/xelementaccess.hxx>
#include <test/container/xenumerationaccess.hxx>
#include <test/container/xindexaccess.hxx>
#include <test/container/xnameaccess.hxx>
#include <test/lang/xserviceinfo.hxx>

#include <cppu/unotype.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sheet/XDataPilotDescriptor.hpp>
#include <com/sun/star/sheet/XDataPilotTables.hpp>
#include <com/sun/star/sheet/XDataPilotTablesSupplier.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;
namespace sc_apitest
{
class ScDataPilotFieldsObj : public CalcUnoApiTest,
                             public apitest::XElementAccess,
                             public apitest::XEnumerationAccess,
                             public apitest::XIndexAccess,
                             public apitest::XNameAccess,
                             public apitest::XServiceInfo
{
public:
    ScDataPilotFieldsObj();
    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScDataPilotFieldsObj);

    // XElementAccess
    CPPUNIT_TEST(testHasElements);
    CPPUNIT_TEST(testGetElementType);

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
    uno::Reference<lang::XComponent> m_xComponent;
};

ScDataPilotFieldsObj::ScDataPilotFieldsObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
    , XElementAccess(cppu::UnoType<beans::XPropertySet>::get())
    , XIndexAccess(6)
    , XNameAccess("")
    , XServiceInfo("ScDataPilotFieldsObj", "com.sun.star.sheet.DataPilotFields")
{
}

uno::Reference<uno::XInterface> ScDataPilotFieldsObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(m_xComponent, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA(xSheets, uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSpreadsheet> xSheet0(xIA->getByIndex(0), uno::UNO_QUERY_THROW);

    for (int i = 1; i < 4; ++i)
    {
        xSheet0->getCellByPosition(i, 0)->setFormula("Col" + OUString::number(i));
        xSheet0->getCellByPosition(0, i)->setFormula("Row" + OUString::number(i));
    }

    for (int i = 1; i < 4; ++i)
    {
        for (int j = 1; j < 4; ++j)
        {
            xSheet0->getCellByPosition(i, j)->setValue(i * (j + 1));
        }
    }

    uno::Reference<sheet::XDataPilotTablesSupplier> xDPTS(xSheet0, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XDataPilotTables> xDPT(xDPTS->getDataPilotTables(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XDataPilotDescriptor> xDPD(xDPT->createDataPilotDescriptor(),
                                                     uno::UNO_QUERY_THROW);

    xDPD->setSourceRange(table::CellRangeAddress(0, 0, 0, 4, 4));
    xDPT->insertNewByName("DataPilotTable", table::CellAddress(0, 5, 5), xDPD);

    return xDPD->getDataPilotFields();
}

void ScDataPilotFieldsObj::setUp()
{
    CalcUnoApiTest::setUp();
    // create a calc document
    m_xComponent = loadFromDesktop("private:factory/scalc");
}

void ScDataPilotFieldsObj::tearDown()
{
    closeDocument(m_xComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScDataPilotFieldsObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
