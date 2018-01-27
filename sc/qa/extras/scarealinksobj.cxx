/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/sheet/xarealinks.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XAreaLinks.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Reference.hxx>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

class ScAreaLinksObj : public CalcUnoApiTest, public apitest::XAreaLinks
{
public:
    ScAreaLinksObj();

    virtual uno::Reference< uno::XInterface > init() override;
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScAreaLinksObj);

    // XAreaLinks
    CPPUNIT_TEST(testInsertAtPosition);
    CPPUNIT_TEST(testRemoveByIndex);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference< lang::XComponent > mxComponent;

};

ScAreaLinksObj::ScAreaLinksObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference< uno::XInterface > ScAreaLinksObj::init()
{
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document", xDoc.is());

    uno::Reference< beans::XPropertySet > xPropSet(xDoc, uno::UNO_QUERY_THROW);
    uno::Reference< sheet::XAreaLinks > xLinks(xPropSet->getPropertyValue("AreaLinks"), uno::UNO_QUERY_THROW);

    return xLinks;
}

void ScAreaLinksObj::setUp()
{
    CalcUnoApiTest::setUp();
    mxComponent = loadFromDesktop("private:factory/scalc");
}

void ScAreaLinksObj::tearDown()
{
    closeDocument(mxComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScAreaLinksObj);

} // end namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
