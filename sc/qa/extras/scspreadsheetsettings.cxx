/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/beans/xpropertyset.hxx>
#include <test/lang/xserviceinfo.hxx>
#include <test/sheet/globalsheetsettings.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;
using namespace css::uno;
using namespace com::sun::star;

namespace sc_apitest
{
class ScSpreadsheetSettings : public UnoApiTest,
                              public apitest::GlobalSheetSettings,
                              public apitest::XPropertySet,
                              public apitest::XServiceInfo
{
public:
    ScSpreadsheetSettings();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(ScSpreadsheetSettings);

    // GlobalSheetSettings
    CPPUNIT_TEST(testGlobalSheetSettingsProperties);

    // XPropertySet
    CPPUNIT_TEST(testGetPropertySetInfo);
    CPPUNIT_TEST(testGetPropertyValue);
    CPPUNIT_TEST(testSetPropertyValue);
    CPPUNIT_TEST(testPropertyChangeListener);
    CPPUNIT_TEST(testVetoableChangeListener);

    // XServiceInfo
    CPPUNIT_TEST(testGetImplementationName);
    CPPUNIT_TEST(testGetSupportedServiceNames);
    CPPUNIT_TEST(testSupportsService);

    CPPUNIT_TEST_SUITE_END();
};

ScSpreadsheetSettings::ScSpreadsheetSettings()
    : UnoApiTest(u"/sc/qa/extras/testdocuments"_ustr)
    , XPropertySet({ u"LinkUpdateMode"_ustr, u"UsePrinterMetrics"_ustr, u"UserLists"_ustr })
    , XServiceInfo(u"stardiv.StarCalc.ScSpreadsheetSettings"_ustr,
                   u"com.sun.star.sheet.GlobalSheetSettings"_ustr)
{
}

uno::Reference<uno::XInterface> ScSpreadsheetSettings::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference<lang::XMultiServiceFactory> xMSF(xDoc, UNO_QUERY_THROW);
    return xMSF->createInstance(u"com.sun.star.sheet.GlobalSheetSettings"_ustr);
}

void ScSpreadsheetSettings::setUp()
{
    UnoApiTest::setUp();
    // create a calc document
    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScSpreadsheetSettings);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
