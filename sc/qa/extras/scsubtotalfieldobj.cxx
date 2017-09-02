/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/sheet/xsubtotalfield.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/SubTotalColumn.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSubTotalCalculatable.hpp>
#include <com/sun/star/sheet/XSubTotalDescriptor.hpp>
#include <com/sun/star/sheet/XSubTotalField.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XInterface.hpp>

using namespace css;
using namespace css::uno;
using namespace com::sun::star;

namespace sc_apitest {

#define NUMBER_OF_TESTS 2

class ScSubTotalFieldObj : public CalcUnoApiTest, public apitest::XSubTotalField
{
public:
    ScSubTotalFieldObj();

    virtual uno::Reference< uno::XInterface > init() override;
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScSubTotalFieldObj);

    // XSubTotalField
    CPPUNIT_TEST(testGetSetGroupColumn);
    CPPUNIT_TEST(testGetSetTotalColumns);

    CPPUNIT_TEST_SUITE_END();

private:
    static sal_Int32 nTest;
    static uno::Reference< lang::XComponent > mxComponent;

};

sal_Int32 ScSubTotalFieldObj::nTest = 0;
uno::Reference< lang::XComponent > ScSubTotalFieldObj::mxComponent;

ScSubTotalFieldObj::ScSubTotalFieldObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference< uno::XInterface > ScSubTotalFieldObj::init()
{
    // create a calc document
    if (!mxComponent.is())
        mxComponent = loadFromDesktop("private:factory/scalc");

    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document", xDoc.is());

    uno::Reference< container::XIndexAccess > xIndex(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);

    uno::Reference< sheet::XSubTotalCalculatable > xSubTotalCalc(xSheet, uno::UNO_QUERY_THROW);
    uno::Reference< sheet::XSubTotalDescriptor > xSubTotalDesc = xSubTotalCalc->createSubTotalDescriptor(true);

    uno::Sequence< sheet::SubTotalColumn > xCols;
    xCols.realloc(1);
    xCols[0].Column = 5;
    xCols[0].Function = sheet::GeneralFunction_SUM;
    xSubTotalDesc->addNew(xCols, 1);

    uno::Reference< container::XIndexAccess > xDescIndex(xSubTotalDesc, uno::UNO_QUERY_THROW);
    uno::Reference< sheet::XSubTotalField > xSTF(xDescIndex->getByIndex(0), uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xSTF.is());
    return xSTF;
}

void ScSubTotalFieldObj::setUp()
{
    nTest++;
    CPPUNIT_ASSERT(nTest <= NUMBER_OF_TESTS);
    CalcUnoApiTest::setUp();
}

void ScSubTotalFieldObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
    {
        closeDocument(mxComponent);
        mxComponent.clear();
    }

    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScSubTotalFieldObj);

} // end namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
