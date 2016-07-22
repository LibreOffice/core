/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/sheet/xnamedranges.hxx>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

#define NUMBER_OF_TESTS 3

class ScNamedRangesObj : public CalcUnoApiTest, public apitest::XNamedRanges
{
public:
    virtual void setUp() override;
    virtual void tearDown() override;

    virtual uno::Reference< uno::XInterface > init(sal_Int32 nSheet = 0) override;

    ScNamedRangesObj();

    CPPUNIT_TEST_SUITE(ScNamedRangesObj);
    CPPUNIT_TEST(testAddNewByName);
    CPPUNIT_TEST(testAddNewFromTitles);
    //CPPUNIT_TEST_EXCEPTION(testRemoveByName, uno::RuntimeException);
    CPPUNIT_TEST(testOutputList);
    CPPUNIT_TEST_SUITE_END();

private:
    static sal_Int32 nTest;
    static uno::Reference< lang::XComponent > mxComponent;
};

sal_Int32 ScNamedRangesObj::nTest = 0;
uno::Reference< lang::XComponent > ScNamedRangesObj::mxComponent;

ScNamedRangesObj::ScNamedRangesObj()
     : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference< uno::XInterface > ScNamedRangesObj::init(sal_Int32 nSheet)
{
    OUString aFileURL;
    createFileURL("ScNamedRangeObj.ods", aFileURL);
    if(!mxComponent.is())
        mxComponent = loadFromDesktop(aFileURL, "com.sun.star.sheet.SpreadsheetDocument");
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference< beans::XPropertySet > xPropSet (mxComponent, UNO_QUERY_THROW);
    OUString aNamedRangesPropertyString("NamedRanges");
    uno::Reference< sheet::XNamedRanges > xNamedRanges(xPropSet->getPropertyValue(aNamedRangesPropertyString), UNO_QUERY_THROW);

    //set value from xnamedranges.hxx
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndexAccess(xDoc->getSheets(), UNO_QUERY_THROW);
    xSheet.set(xIndexAccess->getByIndex(nSheet),UNO_QUERY_THROW);

    return xNamedRanges;
}

void ScNamedRangesObj::setUp()
{
    nTest++;
    CalcUnoApiTest::setUp();
}

void ScNamedRangesObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
    {
        closeDocument(mxComponent);
        mxComponent.clear();
    }

    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScNamedRangesObj);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
