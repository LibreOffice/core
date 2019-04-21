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
#include <test/table/xcellrange.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/table/XTableColumns.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;

namespace sc_apitest
{
class ScTableColumnObj : public CalcUnoApiTest, public apitest::XCellRange, public apitest::XNamed
{
public:
    ScTableColumnObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScTableColumnObj);

    // XCellRange
    CPPUNIT_TEST(testGetCellByPosition);
    CPPUNIT_TEST(testGetCellRangeByName);
    CPPUNIT_TEST(testGetCellRangeByPosition);

    // XNamed
    CPPUNIT_TEST(testGetName);
    // because TableColumnNames are fixed, test for an exception
    CPPUNIT_TEST(testSetNameThrowsException);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<lang::XComponent> m_xComponent;
};

ScTableColumnObj::ScTableColumnObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
    , XCellRange("K1:K1")
    , XNamed("K")
{
}

uno::Reference<uno::XInterface> ScTableColumnObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(m_xComponent, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA(xSheets, uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSpreadsheet> xSheet0(xIA->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<table::XColumnRowRange> xCRR(xSheet0, uno::UNO_QUERY_THROW);
    uno::Reference<table::XTableColumns> xTC(xCRR->getColumns(), uno::UNO_QUERY_THROW);

    uno::Reference<container::XIndexAccess> xIA_TC(xTC, uno::UNO_QUERY_THROW);
    uno::Reference<uno::XInterface> xReturn(xIA_TC->getByIndex(10), uno::UNO_QUERY_THROW);
    return xReturn;
}

void ScTableColumnObj::setUp()
{
    CalcUnoApiTest::setUp();
    // create calc document
    m_xComponent = loadFromDesktop("private:factory/scalc");
}

void ScTableColumnObj::tearDown()
{
    closeDocument(m_xComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScTableColumnObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
