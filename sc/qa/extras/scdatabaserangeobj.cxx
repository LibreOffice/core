/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/sheet/xdatabaserange.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sheet/XDatabaseRange.hpp>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

#define NUMBER_OF_TESTS 6

class ScDatabaseRangeObj : public CalcUnoApiTest, apitest::XDatabaseRange
{
public:
    virtual void setUp() override;
    virtual void tearDown() override;

    virtual uno::Reference< uno::XInterface > init( const OUString& rDBName ) override;

    ScDatabaseRangeObj();

    CPPUNIT_TEST_SUITE(ScDatabaseRangeObj);
    CPPUNIT_TEST(testDataArea);
    CPPUNIT_TEST(testGetSortDescriptor);
    CPPUNIT_TEST(testGetSubtotalDescriptor);
    CPPUNIT_TEST(testGetImportDescriptor);
    CPPUNIT_TEST(testGetFilterDescriptor);
    CPPUNIT_TEST(testRefresh);
    CPPUNIT_TEST_SUITE_END();
private:
    static sal_Int32 nTest;
    static uno::Reference< lang::XComponent > mxComponent;
};

sal_Int32 ScDatabaseRangeObj::nTest = 0;
uno::Reference< lang::XComponent > ScDatabaseRangeObj::mxComponent;

ScDatabaseRangeObj::ScDatabaseRangeObj()
      : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference< uno::XInterface > ScDatabaseRangeObj::init( const OUString& rDBName )
{
    OUString aFileURL;
    createFileURL("ScDatabaseRangeObj.ods", aFileURL);
    if(!mxComponent.is())
        mxComponent = loadFromDesktop(aFileURL, "com.sun.star.sheet.SpreadsheetDocument");
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference< beans::XPropertySet > xPropSet(xDoc, UNO_QUERY_THROW);
    uno::Reference< container::XNameAccess > xNameAccess( xPropSet->getPropertyValue("DatabaseRanges"), UNO_QUERY_THROW);
    uno::Reference< sheet::XDatabaseRange > xDBRange( xNameAccess->getByName(rDBName), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDBRange.is());
    return xDBRange;
}

void ScDatabaseRangeObj::setUp()
{
    nTest++;
    CalcUnoApiTest::setUp();
}

void ScDatabaseRangeObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
    {
        closeDocument(mxComponent);
        mxComponent.clear();
    }

    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScDatabaseRangeObj);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
