/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/sheet/xviewpane.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XViewPane.hpp>
#include <com/sun/star/uno/XInterface.hpp>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

#define NUMBER_OF_TESTS 3

class ScViewPaneObj : public CalcUnoApiTest, public apitest::XViewPane
{
public:
    ScViewPaneObj();

    virtual uno::Reference< uno::XInterface > init() override;
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScViewPaneObj);
    CPPUNIT_TEST(testFirstVisibleColumn);
    CPPUNIT_TEST(testFirstVisibleRow);
    CPPUNIT_TEST(testVisibleRange);
    CPPUNIT_TEST_SUITE_END();

private:

    static sal_Int32 nTest;
    static uno::Reference<lang::XComponent> mxComponent;

};

sal_Int32 ScViewPaneObj::nTest = 0;
uno::Reference< lang::XComponent > ScViewPaneObj::mxComponent;

ScViewPaneObj::ScViewPaneObj()
        : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference< uno::XInterface > ScViewPaneObj::init()
{
    // create a calc document
    if (!mxComponent.is())
        mxComponent = loadFromDesktop("private:factory/scalc");

    uno::Reference< sheet::XSpreadsheetDocument > xSheetDoc(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document!", xSheetDoc.is());

    uno::Reference< frame::XModel > xModel(xSheetDoc, uno::UNO_QUERY_THROW);
    uno::Reference< frame::XController > xController = xModel->getCurrentController();
    uno::Reference< container::XIndexAccess > xIndexAccess(xController, uno::UNO_QUERY_THROW);
    uno::Reference< sheet::XViewPane > xViewPane (xIndexAccess->getByIndex(0), uno::UNO_QUERY_THROW);
    return xViewPane;
}

void ScViewPaneObj::setUp()
{
    nTest++;
    CPPUNIT_ASSERT(nTest <= NUMBER_OF_TESTS);
    CalcUnoApiTest::setUp();
}

void ScViewPaneObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
    {
        closeDocument(mxComponent);
        mxComponent.clear();
    }

    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScViewPaneObj);

} // End Namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
