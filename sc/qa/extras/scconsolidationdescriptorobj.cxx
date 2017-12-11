/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/sheet/xconsolidationdescriptor.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sheet/XConsolidatable.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;
using namespace css::uno;
using namespace com::sun::star;

namespace sc_apitest
{
#define NUMBER_OF_TESTS 12

class ScConsolidationDescriptorObj : public CalcUnoApiTest, public apitest::XConsolidationDescriptor
{
public:
    ScConsolidationDescriptorObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScConsolidationDescriptorObj);

    // XConsolidationDescriptor
    CPPUNIT_TEST(testGetFunction);
    CPPUNIT_TEST(testSetFunction);
    CPPUNIT_TEST(testGetSources);
    CPPUNIT_TEST(testSetSources);
    CPPUNIT_TEST(testGetStartOutputPosition);
    CPPUNIT_TEST(testSetStartOutputPosition);
    CPPUNIT_TEST(testGetUseColumnHeaders);
    CPPUNIT_TEST(testSetUseColumnHeaders);
    CPPUNIT_TEST(testGetUseRowHeaders);
    CPPUNIT_TEST(testSetUseRowHeaders);
    CPPUNIT_TEST(testGetInsertLinks);
    CPPUNIT_TEST(testSetInsertLinks);

    CPPUNIT_TEST_SUITE_END();

private:
    static sal_Int32 nTest;
    static uno::Reference<lang::XComponent> mxComponent;
};

sal_Int32 ScConsolidationDescriptorObj::nTest = 0;
uno::Reference<lang::XComponent> ScConsolidationDescriptorObj::mxComponent;

ScConsolidationDescriptorObj::ScConsolidationDescriptorObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference<uno::XInterface> ScConsolidationDescriptorObj::init()
{
    // create a calc document
    if (!mxComponent.is())
        mxComponent = loadFromDesktop("private:factory/scalc");

    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document", xDoc.is());

    uno::Reference<sheet::XConsolidatable> xConsolidatable(xDoc, UNO_QUERY_THROW);
    return xConsolidatable->createConsolidationDescriptor(true);
}

void ScConsolidationDescriptorObj::setUp()
{
    nTest++;
    CPPUNIT_ASSERT(nTest <= NUMBER_OF_TESTS);
    CalcUnoApiTest::setUp();
}

void ScConsolidationDescriptorObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
    {
        closeDocument(mxComponent);
        mxComponent.clear();
    }

    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScConsolidationDescriptorObj);

} // end namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
