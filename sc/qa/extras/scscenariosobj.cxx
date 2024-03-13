/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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
#include <test/container/xnameaccess.hxx>
#include <test/container/xindexaccess.hxx>
#include <test/lang/xserviceinfo.hxx>
#include <test/sheet/xscenarios.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XScenario.hpp>
#include <com/sun/star/sheet/XScenariosSupplier.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <cppu/unotype.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

using namespace css;
using namespace css::uno;
using namespace com::sun::star;

namespace sc_apitest
{
class ScScenariosObj : public UnoApiTest,
                       public apitest::XElementAccess,
                       public apitest::XEnumerationAccess,
                       public apitest::XNameAccess,
                       public apitest::XIndexAccess,
                       public apitest::XScenarios,
                       public apitest::XServiceInfo
{
public:
    ScScenariosObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(ScScenariosObj);

    // XElementAccess
    CPPUNIT_TEST(testGetElementType);
    CPPUNIT_TEST(testHasElements);

    // XEnumerationAccess
    CPPUNIT_TEST(testCreateEnumeration);

    // XNameAccess
    CPPUNIT_TEST(testGetByName);
    CPPUNIT_TEST(testGetElementNames);
    CPPUNIT_TEST(testHasByName);

    // XIndexAccess
    CPPUNIT_TEST(testGetByIndex);
    CPPUNIT_TEST(testGetCount);

    // XScenarios
    CPPUNIT_TEST(testAddNewByName);
    CPPUNIT_TEST(testRemoveByName);

    // XServiceInfo
    CPPUNIT_TEST(testGetImplementationName);
    CPPUNIT_TEST(testGetSupportedServiceNames);
    CPPUNIT_TEST(testSupportsService);

    CPPUNIT_TEST_SUITE_END();
};

ScScenariosObj::ScScenariosObj()
    : UnoApiTest("/sc/qa/extras/testdocuments")
    , XElementAccess(cppu::UnoType<sheet::XScenario>::get())
    , XNameAccess("ScScenarios")
    , XIndexAccess(1)
    , XServiceInfo("ScScenariosObj", "com.sun.star.sheet.Scenarios")
{
}

uno::Reference<uno::XInterface> ScScenariosObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<container::XIndexAccess> xIndex(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);

    xSheet->getCellByPosition(5, 5)->setValue(15);
    xSheet->getCellByPosition(1, 4)->setValue(10);
    xSheet->getCellByPosition(2, 0)->setValue(-5.15);

    uno::Reference<table::XCellRange> xCellRange(xSheet, uno::UNO_QUERY_THROW);
    uno::Reference<table::XCellRange> xCellRange2 = xCellRange->getCellRangeByName("A1:N4");
    uno::Reference<sheet::XCellRangeAddressable> xCRAddressable(xCellRange2, uno::UNO_QUERY_THROW);
    table::CellRangeAddress aCellRangeAddr = xCRAddressable->getRangeAddress();

    uno::Reference<sheet::XScenariosSupplier> xSupplier(xSheet, uno::UNO_QUERY_THROW);
    xSupplier->getScenarios()->addNewByName("ScScenarios", { aCellRangeAddr }, "Range");

    return xSupplier->getScenarios();
}

void ScScenariosObj::setUp()
{
    UnoApiTest::setUp();
    // create a calc document
    mxComponent = loadFromDesktop("private:factory/scalc");
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScScenariosObj);

} // end namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
