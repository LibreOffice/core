/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/sheet/xcelladdressable.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/uno/XInterface.hpp>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

#define NUMBER_OF_TESTS 1

class ScCellObj : public CalcUnoApiTest, public apitest::XCellAddressable
{
public:
    ScCellObj();

    virtual uno::Reference< uno::XInterface > init() override;
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScCellObj);
    // XCellAddressable
    CPPUNIT_TEST(testGetCellAddress);
    CPPUNIT_TEST_SUITE_END();

private:

    static sal_Int32 nTest;
    static uno::Reference< lang::XComponent > mxComponent;

};

sal_Int32 ScCellObj::nTest = 0;
uno::Reference< lang::XComponent > ScCellObj::mxComponent;

ScCellObj::ScCellObj()
        : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference< uno::XInterface > ScCellObj::init()
{
    // create a calc document
    if (!mxComponent.is())
        mxComponent = loadFromDesktop("private:factory/scalc");

    uno::Reference< sheet::XSpreadsheetDocument > xSheetDoc(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document", xSheetDoc.is());

    // get getSheets
    uno::Reference< sheet::XSpreadsheets > xSheets (xSheetDoc->getSheets(), UNO_QUERY_THROW);
    uno::Any rSheet = xSheets->getByName("Sheet1");
    // query for the XCellRange interface
    uno::Reference< table::XCellRange > rCellRange(rSheet, UNO_QUERY);
    uno::Reference< table::XCellRange > xCellRange = rCellRange->getCellRangeByName("A1");

    return xCellRange->getCellByPosition(0, 0);
}

void ScCellObj::setUp()
{
    nTest++;
    CPPUNIT_ASSERT(nTest <= NUMBER_OF_TESTS);
    CalcUnoApiTest::setUp();
}

void ScCellObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
    {
        closeDocument(mxComponent);
        mxComponent.clear();
    }

    CalcUnoApiTest::tearDown();
}


CPPUNIT_TEST_SUITE_REGISTRATION(ScCellObj);

} // end namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
