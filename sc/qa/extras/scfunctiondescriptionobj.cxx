/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/sheet/functiondescription.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
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
class ScFunctionDescriptionObj : public UnoApiTest, public apitest::FunctionDescription
{
public:
    ScFunctionDescriptionObj();

    virtual uno::Sequence<beans::PropertyValue> init() override;

    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(ScFunctionDescriptionObj);

    // FunctionDescription
    CPPUNIT_TEST(testFunctionDescriptionProperties);

    CPPUNIT_TEST_SUITE_END();
};

ScFunctionDescriptionObj::ScFunctionDescriptionObj()
    : UnoApiTest(u"/sc/qa/extras/testdocuments"_ustr)
{
}

uno::Sequence<beans::PropertyValue> ScFunctionDescriptionObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);

    uno::Reference<lang::XMultiServiceFactory> xMSF(xDoc, UNO_QUERY_THROW);
    uno::Reference<sheet::XFunctionDescriptions> xFDs(
        xMSF->createInstance(u"com.sun.star.sheet.FunctionDescriptions"_ustr), UNO_QUERY_THROW);

    uno::Reference<container::XNameAccess> xNA(xFDs, UNO_QUERY_THROW);
    uno::Sequence<OUString> names = xNA->getElementNames();

    uno::Sequence<beans::PropertyValue> sPropertyValues;
    CPPUNIT_ASSERT(xNA->getByName(names[0]) >>= sPropertyValues);
    return sPropertyValues;
}

void ScFunctionDescriptionObj::setUp()
{
    UnoApiTest::setUp();
    // create a calc document
    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScFunctionDescriptionObj);

} // end namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
