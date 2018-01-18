/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/sheet/spreadsheetviewsettings.hxx>
#include <test/sheet/xspreadsheetview.hxx>
#include <test/sheet/xviewfreezable.hxx>
#include <test/sheet/xviewsplitable.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XViewSplitable.hpp>

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Reference.hxx>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

class ScTabViewObj : public CalcUnoApiTest,
                     public apitest::SpreadsheetViewSettings,
                     public apitest::XSpreadsheetView,
                     public apitest::XViewFreezable,
                     public apitest::XViewSplitable
{
public:
    ScTabViewObj();

    virtual uno::Reference< uno::XInterface > init() override;
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScTabViewObj);

    // SpreadsheetViewSettings
    CPPUNIT_TEST(testSpreadsheetViewSettingsProperties);

    // XSpreadsheetView
    CPPUNIT_TEST(testGetSetActiveSheet);

    // XViewFreezable
    CPPUNIT_TEST(testFreeze);

    // XViewSplitable
    CPPUNIT_TEST(testSplit);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference< lang::XComponent > mxComponent;
};

ScTabViewObj::ScTabViewObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference< uno::XInterface > ScTabViewObj::init()
{
    uno::Reference< sheet::XSpreadsheetDocument > xSheetDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference< frame::XModel > xModel(xSheetDoc, uno::UNO_QUERY_THROW);

    return xModel->getCurrentController();
}

void ScTabViewObj::setUp()
{
    CalcUnoApiTest::setUp();
    mxComponent = loadFromDesktop("private:factory/scalc");
}

void ScTabViewObj::tearDown()
{
    closeDocument(mxComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScTabViewObj);

} // end namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
