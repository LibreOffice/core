/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/sheet/functiondescription.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/XFunctionDescriptions.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>

using namespace css;
using namespace css::uno;
using namespace com::sun::star;

namespace sc_apitest
{
class ScFunctionDescriptionObj : public CalcUnoApiTest, public apitest::FunctionDescription
{
public:
    ScFunctionDescriptionObj();

    virtual uno::Sequence<beans::PropertyValue> init() override;

    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScFunctionDescriptionObj);

    // FunctionDescription
    CPPUNIT_TEST(testFunctionDescriptionProperties);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<lang::XComponent> mxComponent;
};

ScFunctionDescriptionObj::ScFunctionDescriptionObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Sequence<beans::PropertyValue> ScFunctionDescriptionObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);

    uno::Reference<lang::XMultiServiceFactory> xMSF(xDoc, UNO_QUERY_THROW);
    uno::Reference<sheet::XFunctionDescriptions> xFDs(
        xMSF->createInstance("com.sun.star.sheet.FunctionDescriptions"), UNO_QUERY_THROW);

    uno::Reference<container::XNameAccess> xNA(xFDs, UNO_QUERY_THROW);
    uno::Sequence<OUString> names = xNA->getElementNames();

    uno::Sequence<beans::PropertyValue> sPropertyValues;
    CPPUNIT_ASSERT(xNA->getByName(names[0]) >>= sPropertyValues);
    return sPropertyValues;
}

void ScFunctionDescriptionObj::setUp()
{
    CalcUnoApiTest::setUp();
    // create a calc document
    mxComponent = loadFromDesktop("private:factory/scalc");
}

void ScFunctionDescriptionObj::tearDown()
{
    closeDocument(mxComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScFunctionDescriptionObj);

} // end namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
