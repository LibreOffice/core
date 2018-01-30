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

class ScNamedRangesObj : public CalcUnoApiTest, public apitest::XNamedRanges
{
public:
    ScNamedRangesObj();

    virtual void setUp() override;
    virtual void tearDown() override;

    virtual uno::Reference< uno::XInterface > init(sal_Int32 nSheet = 0) override;

    CPPUNIT_TEST_SUITE(ScNamedRangesObj);

    // XNamedRanges
    CPPUNIT_TEST(testAddNewByName);
    CPPUNIT_TEST(testAddNewFromTitles);
    //CPPUNIT_TEST_EXCEPTION(testRemoveByName, uno::RuntimeException);
    CPPUNIT_TEST(testOutputList);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference< lang::XComponent > mxComponent;
};

ScNamedRangesObj::ScNamedRangesObj()
     : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference< uno::XInterface > ScNamedRangesObj::init(sal_Int32 nSheet)
{
    CPPUNIT_ASSERT_MESSAGE("no component loaded", mxComponent.is());

    uno::Reference< beans::XPropertySet > xPropSet (mxComponent, UNO_QUERY_THROW);
    uno::Reference< sheet::XNamedRanges > xNamedRanges(xPropSet->getPropertyValue("NamedRanges"), UNO_QUERY_THROW);

    //set value from xnamedranges.hxx
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndexAccess(xDoc->getSheets(), UNO_QUERY_THROW);
    xSheet.set(xIndexAccess->getByIndex(nSheet), UNO_QUERY_THROW);

    return xNamedRanges;
}

void ScNamedRangesObj::setUp()
{
    CalcUnoApiTest::setUp();
    // create a calc document
    OUString aFileURL;
    createFileURL("ScNamedRangeObj.ods", aFileURL);
    mxComponent = loadFromDesktop(aFileURL, "com.sun.star.sheet.SpreadsheetDocument");
}

void ScNamedRangesObj::tearDown()
{
    closeDocument(mxComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScNamedRangesObj);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
