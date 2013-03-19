/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <test/unoapi_test.hxx>
#include <test/sheet/xdatabaserange.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sheet/XDatabaseRange.hpp>


namespace sc_apitest {

#define NUMBER_OF_TESTS 6

class ScDatabaseRangeObj : public UnoApiTest, apitest::XDatabaseRange
{
public:
    virtual void setUp();
    virtual void tearDown();

    virtual uno::Reference< uno::XInterface > init( const rtl::OUString& rDBName );

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
      : UnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference< uno::XInterface > ScDatabaseRangeObj::init( const rtl::OUString& rDBName )
{
    rtl::OUString aFileURL;
    createFileURL("ScDatabaseRangeObj.ods", aFileURL);
    if(!mxComponent.is())
        mxComponent = loadFromDesktop(aFileURL, "com.sun.star.sheet.SpreadsheetDocument");
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference< beans::XPropertySet > xPropSet(xDoc, UNO_QUERY_THROW);
    uno::Reference< container::XNameAccess > xNameAccess( xPropSet->getPropertyValue(rtl::OUString("DatabaseRanges")), UNO_QUERY_THROW);
    uno::Reference< sheet::XDatabaseRange > xDBRange( xNameAccess->getByName(rDBName), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDBRange.is());
    return xDBRange;
}

void ScDatabaseRangeObj::setUp()
{
    nTest++;
    UnoApiTest::setUp();
}

void ScDatabaseRangeObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
        closeDocument(mxComponent);

    UnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScDatabaseRangeObj);

CPPUNIT_PLUGIN_IMPLEMENT();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
