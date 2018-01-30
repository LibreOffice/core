/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/sheet/xscenarios.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XScenariosSupplier.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>

#include <unonames.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XInterface.hpp>

using namespace css;
using namespace css::uno;
using namespace com::sun::star;

namespace sc_apitest
{
class ScScenariosObj : public CalcUnoApiTest, public apitest::XScenarios
{
public:
    ScScenariosObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScScenariosObj);

    // XScenarios
    CPPUNIT_TEST(testAddNewByName);
    CPPUNIT_TEST(testRemoveByName);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<lang::XComponent> mxComponent;
};

ScScenariosObj::ScScenariosObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference<uno::XInterface> ScScenariosObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document", xDoc.is());

    uno::Reference<container::XIndexAccess> xIndex(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);

    xSheet->getCellByPosition(5, 5)->setValue(15);
    xSheet->getCellByPosition(1, 4)->setValue(10);
    xSheet->getCellByPosition(2, 0)->setValue(-5.15);

    uno::Reference<table::XCellRange> xCellRange(xSheet, uno::UNO_QUERY_THROW);
    uno::Reference<table::XCellRange> xCellRange2 = xCellRange->getCellRangeByName("A1:N4");
    uno::Reference<sheet::XCellRangeAddressable> xCRAddressable(xCellRange2, uno::UNO_QUERY_THROW);
    table::CellRangeAddress aCellRangeAddr = xCRAddressable->getRangeAddress();

    uno::Sequence<table::CellRangeAddress> aCellRangeAddresses(1);
    aCellRangeAddresses[0] = aCellRangeAddr;
    uno::Reference<sheet::XScenariosSupplier> xSupplier(xSheet, uno::UNO_QUERY_THROW);
    xSupplier->getScenarios()->addNewByName("ScScenarios", aCellRangeAddresses, "Range");

    return xSupplier->getScenarios();
}

void ScScenariosObj::setUp()
{
    CalcUnoApiTest::setUp();
    // create a calc document
    mxComponent = loadFromDesktop("private:factory/scalc");
}

void ScScenariosObj::tearDown()
{
    closeDocument(mxComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScScenariosObj);

} // end namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
