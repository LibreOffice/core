/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/lang/xserviceinfo.hxx>
#include <test/sheet/xcellrangereferrer.hxx>
#include <test/sheet/xviewpane.hxx>
#include <test/view/xcontrolaccess.hxx>
#include <test/view/xformlayeraccess.hxx>
#include <test/helper/form.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/form/XFormsSupplier.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XViewPane.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;

namespace sc_apitest
{
class ScViewPaneObj : public UnoApiTest,
                      public apitest::XCellRangeReferrer,
                      public apitest::XControlAccess,
                      public apitest::XFormLayerAccess,
                      public apitest::XServiceInfo,
                      public apitest::XViewPane
{
public:
    ScViewPaneObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual uno::Reference<uno::XInterface> getXComponent() override;
    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(ScViewPaneObj);

    // XCellRangeReferrer
    CPPUNIT_TEST(testGetReferredCells);

    // XControlAccess
    CPPUNIT_TEST(testGetControl);

    // XFormLayerAccess
    CPPUNIT_TEST(testGetFormController);
    CPPUNIT_TEST(testIsFormDesignMode);
    CPPUNIT_TEST(testSetFormDesignMode);

    // XServiceInfo
    CPPUNIT_TEST(testGetImplementationName);
    CPPUNIT_TEST(testGetSupportedServiceNames);
    CPPUNIT_TEST(testSupportsService);

    // XViewPane
    CPPUNIT_TEST(testFirstVisibleColumn);
    CPPUNIT_TEST(testFirstVisibleRow);
    CPPUNIT_TEST(testVisibleRange);

    CPPUNIT_TEST_SUITE_END();
};

ScViewPaneObj::ScViewPaneObj()
    : UnoApiTest("/sc/qa/extras/testdocuments")
    , XServiceInfo("ScViewPaneObj", "com.sun.star.sheet.SpreadsheetViewPane")
{
}

uno::Reference<uno::XInterface> ScViewPaneObj::getXComponent() { return mxComponent; }

uno::Reference<uno::XInterface> ScViewPaneObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<frame::XModel> xModel(xDoc, uno::UNO_QUERY_THROW);
    uno::Reference<frame::XController> xController(xModel->getCurrentController(),
                                                   uno::UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIA(xController, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XViewPane> xViewPane(xIA->getByIndex(0), uno::UNO_QUERY_THROW);

    uno::Reference<drawing::XDrawPagesSupplier> xDPS(xDoc, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPages> xDP(xDPS->getDrawPages(), uno::UNO_SET_THROW);
    xDP->insertNewByIndex(1);
    xDP->insertNewByIndex(2);

    uno::Reference<drawing::XDrawPage> xDrawPage(xDP->getByIndex(0), uno::UNO_QUERY_THROW);
    xDrawPage->add(
        apitest::helper::form::createCommandButton(mxComponent, 15000, 10000, 3000, 4500));

    uno::Reference<form::XFormsSupplier> xFS(xDrawPage, uno::UNO_QUERY_THROW);
    uno::Reference<container::XNameContainer> xNC(xFS->getForms(), uno::UNO_SET_THROW);

    // XFormLayerAccess
    uno::Reference<form::XForm> xForm(xNC->getByName("Form"), uno::UNO_QUERY_THROW);
    setForm(xForm);
    // XCellRangeReferrer
    setCellRange(xViewPane->getVisibleRange());

    return xViewPane;
}

void ScViewPaneObj::setUp()
{
    UnoApiTest::setUp();
    // create a calc document
    mxComponent = loadFromDesktop("private:factory/scalc");
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScViewPaneObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
