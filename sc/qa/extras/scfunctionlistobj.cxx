/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/container/xelementaccess.hxx>
#include <test/container/xenumerationaccess.hxx>
#include <test/container/xindexaccess.hxx>
#include <test/container/xnameaccess.hxx>
#include <test/lang/xserviceinfo.hxx>
#include <test/sheet/xfunctiondescriptions.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <cppu/unotype.hxx>

using namespace css;
using namespace css::uno;
using namespace com::sun::star;

namespace sc_apitest
{
class ScFunctionListObj : public UnoApiTest,
                          public apitest::XElementAccess,
                          public apitest::XEnumerationAccess,
                          public apitest::XFunctionDescriptions,
                          public apitest::XIndexAccess,
                          public apitest::XNameAccess,
                          public apitest::XServiceInfo
{
public:
    ScFunctionListObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(ScFunctionListObj);

    // XElementAccess
    CPPUNIT_TEST(testGetElementType);
    CPPUNIT_TEST(testHasElements);

    // XEnumerationAccess
    CPPUNIT_TEST(testCreateEnumeration);

    // XFunctionDescriptions
    CPPUNIT_TEST(testGetById);

    // XIndexAccess
    CPPUNIT_TEST(testGetByIndex);
    CPPUNIT_TEST(testGetCount);

    // XNameAccess
    CPPUNIT_TEST(testGetByName);
    CPPUNIT_TEST(testGetElementNames);
    CPPUNIT_TEST(testHasByName);

    // XServiceInfo
    CPPUNIT_TEST(testGetImplementationName);
    CPPUNIT_TEST(testGetSupportedServiceNames);
    CPPUNIT_TEST(testSupportsService);

    CPPUNIT_TEST_SUITE_END();
};

ScFunctionListObj::ScFunctionListObj()
    : UnoApiTest(u"/sc/qa/extras/testdocuments"_ustr)
    , XElementAccess(cppu::UnoType<uno::Sequence<beans::PropertyValue>>::get())
    , XIndexAccess(405)
    , XNameAccess(u"IF"_ustr)
    , XServiceInfo(u"stardiv.StarCalc.ScFunctionListObj"_ustr,
                   u"com.sun.star.sheet.FunctionDescriptions"_ustr)
{
}

uno::Reference<uno::XInterface> ScFunctionListObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference<lang::XMultiServiceFactory> xMSF(xDoc, UNO_QUERY_THROW);
    return xMSF->createInstance(u"com.sun.star.sheet.FunctionDescriptions"_ustr);
}

void ScFunctionListObj::setUp()
{
    UnoApiTest::setUp();
    // create a calc document
    loadFromURL(u"private:factory/scalc"_ustr);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScFunctionListObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
