/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XConditionalFormats.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <unonames.hxx>

using namespace css;

namespace sc_apitest {

#define NUMBER_OF_TESTS 1

class ScConditionalFormatTest : public CalcUnoApiTest
{
public:
    ScConditionalFormatTest();

    virtual void setUp() SAL_OVERRIDE;
    virtual void tearDown() SAL_OVERRIDE;

    uno::Reference< uno::XInterface > init();
    void testRequestCondFormatListFromSheet();

    CPPUNIT_TEST_SUITE(ScConditionalFormatTest);
    CPPUNIT_TEST(testRequestCondFormatListFromSheet);
    CPPUNIT_TEST_SUITE_END();
private:

    static sal_Int32 nTest;
    static uno::Reference< lang::XComponent > mxComponent;
};

sal_Int32 ScConditionalFormatTest::nTest = 0;
uno::Reference< lang::XComponent > ScConditionalFormatTest::mxComponent;

ScConditionalFormatTest::ScConditionalFormatTest()
    : CalcUnoApiTest("sc/qa/extras/testdocuments/")
{
}

uno::Reference< uno::XInterface > ScConditionalFormatTest::init()
{
    if(!mxComponent.is())
    {
        // get the test file
        OUString aFileURL;
        createFileURL(OUString("new_cond_format_api.ods"), aFileURL);
        mxComponent = loadFromDesktop(aFileURL);
    }
    CPPUNIT_ASSERT_MESSAGE("Component not loaded", mxComponent.is());

    // get the first sheet
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(0), uno::UNO_QUERY_THROW);

    return xSheet;
}

void ScConditionalFormatTest::testRequestCondFormatListFromSheet()
{
    uno::Reference<sheet::XSpreadsheet> xSheet(init(), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xProps(xSheet, uno::UNO_QUERY_THROW);
    uno::Any aAny = xProps->getPropertyValue("ConditionalFormats");
    uno::Reference<sheet::XConditionalFormats> xCondFormats;
    CPPUNIT_ASSERT(aAny >>= xCondFormats);
    CPPUNIT_ASSERT(xCondFormats.is());
}

void ScConditionalFormatTest::setUp()
{
    nTest++;
    CalcUnoApiTest::setUp();
}

void ScConditionalFormatTest::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
    {
        closeDocument(mxComponent);
        mxComponent.clear();
    }

    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScConditionalFormatTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
