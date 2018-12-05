/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/sheet/xcellrangereferrer.hxx>
#include <test/sheet/xviewpane.hxx>
#include <test/view/xcontrolaccess.hxx>

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

class ScViewPaneObj : public CalcUnoApiTest,
                      public apitest::XCellRangeReferrer,
                      public apitest::XControlAccess,
                      public apitest::XViewPane
{
public:
    ScViewPaneObj();

    virtual uno::Reference< uno::XInterface > init() override;
    virtual uno::Reference< uno::XInterface > getXComponent() override;
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScViewPaneObj);

    // XCellRangeReferrer
    CPPUNIT_TEST(testGetReferredCells);

    // XControlAccess
    CPPUNIT_TEST(testGetControl);

    // XViewPane
    CPPUNIT_TEST(testFirstVisibleColumn);
    CPPUNIT_TEST(testFirstVisibleRow);
    CPPUNIT_TEST(testVisibleRange);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<lang::XComponent> mxComponent;
};

ScViewPaneObj::ScViewPaneObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference<uno::XInterface> ScViewPaneObj::getXComponent()
{
    return mxComponent;
}

uno::Reference< uno::XInterface > ScViewPaneObj::init()
{
    uno::Reference< sheet::XSpreadsheetDocument > xSheetDoc(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference< frame::XModel > xModel(xSheetDoc, uno::UNO_QUERY_THROW);
    uno::Reference< frame::XController > xController = xModel->getCurrentController();
    uno::Reference< container::XIndexAccess > xIndexAccess(xController, uno::UNO_QUERY_THROW);
    uno::Reference< sheet::XViewPane > xViewPane (xIndexAccess->getByIndex(0), uno::UNO_QUERY_THROW);

    setCellRange(xViewPane->getVisibleRange());

    return xViewPane;
}

void ScViewPaneObj::setUp()
{
    CalcUnoApiTest::setUp();
    // create a calc document
    mxComponent = loadFromDesktop("private:factory/scalc");
}

void ScViewPaneObj::tearDown()
{
    closeDocument(mxComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScViewPaneObj);

} // End Namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
