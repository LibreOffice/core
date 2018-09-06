/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <test/calc_unoapi_test.hxx>
#include <test/table/xtablechart.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/table/XTableChart.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;
using namespace css::uno;
using namespace com::sun::star;

namespace sc_apitest
{
class ScChartObj : public CalcUnoApiTest, public apitest::xtablechart
{
public:
    ScChartObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScChartObj);

    // XTableChart
    CPPUNIT_TEST(testSetGetHasColumnHeaders);
    CPPUNIT_TEST(testSetGetHasRowHeaders);
    CPPUNIT_TEST(testSetGetRanges);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<lang::XComponent> mxComponent;
};

ScChartObj::ScChartObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference<uno::XInterface> ScChartObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xSpreadsheetDocument(mxComponent, UNO_QUERY_THROW);

    uno::Reference<sheet::XSpreadsheets> xSpreadsheets(xSpreadsheetDocument->getSheets(),
                                                       UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIndexAccess(xSpreadsheets, UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSpreadsheet(xIndexAccess->getByIndex(0), UNO_QUERY_THROW);

    uno::Reference<table::XTableChart> xTableChart(xSpreadsheet, UNO_QUERY_THROW);
    return xTableChart;
}

void ScChartObj::setUp()
{
    CalcUnoApiTest::setUp();
    // create a calc document
    mxComponent = loadFromDesktop("private:factory/scalc");
}

void ScChartObj::tearDown()
{
    closeDocument(mxComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScChartObj);

} // end namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
