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
#include <test/container/xindexaccess.hxx>

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/table/XTableRows.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppu/unotype.hxx>

using namespace css;
using namespace css::uno;

namespace sc_apitest
{
class ScTableRowsObj : public CalcUnoApiTest,
                       public apitest::XElementAccess,
                       public apitest::XIndexAccess
{
public:
    ScTableRowsObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScTableRowsObj);

    // XElementAccess
    CPPUNIT_TEST(testGetElementType);
    CPPUNIT_TEST(testHasElements);

    // XIndexAccess
    CPPUNIT_TEST(testGetByIndex);
    CPPUNIT_TEST(testGetCount);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<lang::XComponent> m_xCompoment;
};

ScTableRowsObj::ScTableRowsObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
    , XElementAccess(cppu::UnoType<table::XCellRange>::get())
    , XIndexAccess(1048576)
{
}

uno::Reference<uno::XInterface> ScTableRowsObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(m_xCompoment, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document", xDoc.is());

    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<container::XNameAccess> xNA(xSheets, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet0(xSheets->getByName(xNA->getElementNames()[0]),
                                                uno::UNO_QUERY_THROW);

    uno::Reference<table::XColumnRowRange> xCRR(xSheet0, uno::UNO_QUERY_THROW);
    uno::Reference<table::XTableRows> xTR(xCRR->getRows(), uno::UNO_QUERY_THROW);

    return xTR;
}

void ScTableRowsObj::setUp()
{
    CalcUnoApiTest::setUp();
    m_xCompoment = loadFromDesktop("private:factory/scalc");
    CPPUNIT_ASSERT_MESSAGE("no component", m_xCompoment.is());
}

void ScTableRowsObj::tearDown()
{
    closeDocument(m_xCompoment);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScTableRowsObj);
} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
