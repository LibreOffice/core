/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/sheet/xlabelrange.hxx>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XLabelRange.hpp>
#include <com/sun/star/sheet/XLabelRanges.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/uno/XInterface.hpp>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

#define NUMBER_OF_TESTS 2

class ScLabelRangeObj : public CalcUnoApiTest, public apitest::XLabelRange
{
public:
    virtual void setUp() override;
    virtual void tearDown() override;

    virtual uno::Reference< uno::XInterface > init() override;

    ScLabelRangeObj();

    CPPUNIT_TEST_SUITE(ScLabelRangeObj);
    CPPUNIT_TEST(testDataArea);
    CPPUNIT_TEST(testLabelArea);
    CPPUNIT_TEST_SUITE_END();
private:
    static sal_Int32 nTest;
    static uno::Reference< lang::XComponent > mxComponent;
};

sal_Int32 ScLabelRangeObj::nTest = 0;
uno::Reference< lang::XComponent > ScLabelRangeObj::mxComponent;

ScLabelRangeObj::ScLabelRangeObj()
         : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference< uno::XInterface > ScLabelRangeObj::init()
{
    if (!mxComponent.is())
        mxComponent = loadFromDesktop("private:factory/scalc");
    uno::Reference< sheet::XSpreadsheetDocument > xSheetDoc(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document!", xSheetDoc.is());

    uno::Reference< beans::XPropertySet > xPropSet(xSheetDoc, UNO_QUERY_THROW);
    uno::Reference< container::XNameAccess > xNameAccess( xPropSet->getPropertyValue("ColumnLabelRanges"), UNO_QUERY_THROW);
    uno::Reference< sheet::XLabelRanges > lRanges (xNameAccess, uno::UNO_QUERY_THROW);

    table::CellRangeAddress aRange2 = table::CellRangeAddress(0, 0, 1, 0, 6);
    table::CellRangeAddress aRange1 = table::CellRangeAddress(0, 0, 1, 0, 6);

    lRanges->addNew(aRange1, aRange2);

    uno::Reference< sheet::XLabelRange > lRange (lRanges->getByIndex(0), uno::UNO_QUERY_THROW);
    return lRange;

}

void ScLabelRangeObj::setUp()
{
    nTest++;
    CalcUnoApiTest::setUp();
}

void ScLabelRangeObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
    {
        closeDocument(mxComponent);
        mxComponent.clear();
    }

    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScLabelRangeObj);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
