/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/sheet/xfunctiondescriptions.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XInterface.hpp>

using namespace css;
using namespace css::uno;
using namespace com::sun::star;

namespace sc_apitest
{
class ScFunctionListObj : public CalcUnoApiTest, public apitest::XFunctionDescriptions
{
public:
    ScFunctionListObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScFunctionListObj);

    // XFunctionDescriptions
    CPPUNIT_TEST(testGetById);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<lang::XComponent> mxComponent;
};

ScFunctionListObj::ScFunctionListObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference<uno::XInterface> ScFunctionListObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document", xDoc.is());

    uno::Reference<lang::XMultiServiceFactory> xMSF(xDoc, UNO_QUERY_THROW);
    return xMSF->createInstance("com.sun.star.sheet.FunctionDescriptions");
}

void ScFunctionListObj::setUp()
{
    CalcUnoApiTest::setUp();
    // create a calc document
    mxComponent = loadFromDesktop("private:factory/scalc");
}

void ScFunctionListObj::tearDown()
{
    closeDocument(mxComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScFunctionListObj);

} // end namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
