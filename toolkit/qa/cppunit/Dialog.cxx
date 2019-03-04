/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppuhelper/implbase.hxx>
#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>

#include <com/sun/star/awt/UnoControlDialog.hpp>
#include <com/sun/star/awt/XUnoControlDialog.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>

#include <comphelper/processfactory.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/window.hxx>

using namespace css;

namespace
{
/// Test dialogs constructed via UNO
class DialogTest : public test::BootstrapFixture, public unotest::MacrosTest
{
protected:
    uno::Reference<uno::XComponentContext> mxContext;

public:
    virtual void setUp() override;
};

void DialogTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxContext.set(comphelper::getComponentContext(getMultiServiceFactory()));
}

CPPUNIT_TEST_FIXTURE(DialogTest, testDialogSizeable)
{
    uno::Reference<awt::XDialog> xDialog;
    uno::Reference<lang::XMultiComponentFactory> xFactory(mxContext->getServiceManager(),
                                                          uno::UNO_QUERY_THROW);
    uno::Reference<awt::XControlModel> xControlModel(
        xFactory->createInstanceWithContext("com.sun.star.awt.UnoControlDialogModel", mxContext),
        uno::UNO_QUERY_THROW);

    uno::Reference<beans::XPropertySet> xPropSet(xControlModel, uno::UNO_QUERY_THROW);
    xPropSet->setPropertyValue("Sizeable", uno::Any(true));

    uno::Reference<awt::XUnoControlDialog> xControl = awt::UnoControlDialog::create(mxContext);
    xControl->setModel(xControlModel);
    uno::Reference<awt::XWindow> xWindow(xControl, uno::UNO_QUERY);
    xWindow->setVisible(true);
    xDialog.set(xControl, uno::UNO_QUERY_THROW);
    xDialog->execute();

    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow(xControl->getPeer());
    CPPUNIT_ASSERT(pWindow);
    CPPUNIT_ASSERT(pWindow->GetStyle() & WB_SIZEABLE);

    xDialog->endExecute();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
