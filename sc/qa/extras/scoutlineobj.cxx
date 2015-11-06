/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/sheet/xsheetoutline.hxx>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

#define NUMBER_OF_TESTS 6

class ScOutlineObj : public CalcUnoApiTest, apitest::XSheetOutline
{
public:
    ScOutlineObj();

    virtual void setUp() override;
    virtual void tearDown() override;

    virtual uno::Reference< uno::XInterface > init() override;

    CPPUNIT_TEST_SUITE(ScOutlineObj);
    CPPUNIT_TEST(testHideDetail);
    CPPUNIT_TEST(testShowDetail);
    CPPUNIT_TEST(testShowLevel);
    CPPUNIT_TEST(testUngroup);
    CPPUNIT_TEST(testGroup);
  //  CPPUNIT_TEST(testAutoOutline);
    CPPUNIT_TEST(testClearOutline);
    CPPUNIT_TEST_SUITE_END();
private:

    static sal_Int32 nTest;
    static uno::Reference< lang::XComponent > mxComponent;
};

sal_Int32 ScOutlineObj::nTest = 0;
uno::Reference< lang::XComponent > ScOutlineObj::mxComponent;

ScOutlineObj::ScOutlineObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference< uno::XInterface > ScOutlineObj::init()
{
    // get the test file
    OUString aFileURL;
    createFileURL("ScOutlineObj.ods", aFileURL);
    if(!mxComponent.is())
        mxComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT_MESSAGE("Component not loaded",mxComponent.is());

    // get the first sheet
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(0), UNO_QUERY_THROW);

    return xSheet;
}

void ScOutlineObj::setUp()
{
    nTest++;
    CalcUnoApiTest::setUp();
}

void ScOutlineObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
    {
        closeDocument(mxComponent);
        mxComponent.clear();
    }

    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScOutlineObj);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
