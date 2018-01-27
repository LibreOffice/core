/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/sheet/databaserange.hxx>
#include <test/sheet/xdatabaserange.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sheet/XDatabaseRange.hpp>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

class ScDatabaseRangeObj : public CalcUnoApiTest, public apitest::DatabaseRange, public apitest::XDatabaseRange
{
public:
    virtual void setUp() override;
    virtual void tearDown() override;

    virtual uno::Reference< uno::XInterface > init( const OUString& rDBName ) override;

    ScDatabaseRangeObj();

    CPPUNIT_TEST_SUITE(ScDatabaseRangeObj);

    // XDatabaseRange
    CPPUNIT_TEST(testDataArea);
    CPPUNIT_TEST(testGetSortDescriptor);
    CPPUNIT_TEST(testGetSubtotalDescriptor);
    CPPUNIT_TEST(testGetImportDescriptor);
    CPPUNIT_TEST(testGetFilterDescriptor);
    CPPUNIT_TEST(testRefresh);

    // DatabaseRange
    CPPUNIT_TEST(testMoveCells);
    CPPUNIT_TEST(testKeepFormats);
    CPPUNIT_TEST(testStripData);
    CPPUNIT_TEST(testAutoFilter);
    CPPUNIT_TEST(testUseFilterCriteriaSource);
    CPPUNIT_TEST(testFilterCriteriaSource);
    CPPUNIT_TEST(testRefreshPeriod);
    CPPUNIT_TEST(testFromSelection);
    CPPUNIT_TEST(testTokenIndex);
    CPPUNIT_TEST(testTotalsRow);
    CPPUNIT_TEST(testContainsHeader);

    CPPUNIT_TEST_SUITE_END();
private:
    uno::Reference< lang::XComponent > mxComponent;
};

ScDatabaseRangeObj::ScDatabaseRangeObj()
      : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference< uno::XInterface > ScDatabaseRangeObj::init( const OUString& rDBName )
{
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document", xDoc.is());

    uno::Reference< beans::XPropertySet > xPropSet(xDoc, UNO_QUERY_THROW);
    uno::Reference< container::XNameAccess > xNameAccess( xPropSet->getPropertyValue("DatabaseRanges"), UNO_QUERY_THROW);
    uno::Reference< sheet::XDatabaseRange > xDBRange( xNameAccess->getByName(rDBName), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDBRange.is());
    return xDBRange;
}

void ScDatabaseRangeObj::setUp()
{
    CalcUnoApiTest::setUp();

    OUString aFileURL;
    createFileURL("ScDatabaseRangeObj.ods", aFileURL);
    mxComponent = loadFromDesktop(aFileURL, "com.sun.star.sheet.SpreadsheetDocument");

}

void ScDatabaseRangeObj::tearDown()
{
    closeDocument(mxComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScDatabaseRangeObj);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
