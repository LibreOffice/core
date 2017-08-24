/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>

#include <test/sheet/cellarealink.hxx>
#include <test/sheet/xarealink.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sheet/XAreaLink.hpp>
#include <com/sun/star/sheet/XAreaLinks.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/CellAddress.hpp>

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Reference.hxx>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

#define NUMBER_OF_TESTS 9

class ScAreaLink : public CalcUnoApiTest, public apitest::CellAreaLink, public apitest::XAreaLink
{
public:
    ScAreaLink();

    virtual uno::Reference< uno::XInterface > init() override;
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScAreaLink);

    // CellAreaLink
    CPPUNIT_TEST(testUrl);
    CPPUNIT_TEST(testFilter);
    CPPUNIT_TEST(testFilterOptions);
    CPPUNIT_TEST(testRefreshDelay);
    CPPUNIT_TEST(testRefreshPeriod);

    // XAreaLink
    CPPUNIT_TEST(testGetDestArea);
    CPPUNIT_TEST(testGetSourceArea);
    CPPUNIT_TEST(testSetSourceArea);
    CPPUNIT_TEST(testSetDestArea);

    CPPUNIT_TEST_SUITE_END();

private:
    static sal_Int32 nTest;
    static uno::Reference< lang::XComponent > mxComponent;

};

sal_Int32 ScAreaLink::nTest = 0;
uno::Reference< lang::XComponent > ScAreaLink::mxComponent;

ScAreaLink::ScAreaLink()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference< uno::XInterface > ScAreaLink::init()
{
    // create a calc document
    if (!mxComponent.is())
        mxComponent = loadFromDesktop("private:factory/scalc");

    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document", xDoc.is());

    uno::Reference< beans::XPropertySet > xPropSet(xDoc, uno::UNO_QUERY_THROW);
    uno::Reference< sheet::XAreaLinks > xLinks(xPropSet->getPropertyValue("AreaLinks"), uno::UNO_QUERY_THROW);

    table::CellAddress aCellAddress(1, 2, 3);
    xLinks->insertAtPosition(aCellAddress, "", "a1:c1", "", "");

    uno::Reference< sheet::XAreaLink > xLink( xLinks->getByIndex(0), uno::UNO_QUERY_THROW);
    return xLink;
}

void ScAreaLink::setUp()
{
    nTest++;
    CalcUnoApiTest::setUp();
}

void ScAreaLink::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
    {
        closeDocument(mxComponent);
        mxComponent.clear();
    }

    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScAreaLink);

} // end namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
