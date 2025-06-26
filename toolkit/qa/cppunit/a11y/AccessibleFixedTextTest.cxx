/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/XFixedText.hpp>
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <test/a11y/AccessibilityTools.hxx>
#include <test/a11y/XAccessibleComponentTester.hxx>
#include <test/a11y/XAccessibleContextTester.hxx>
#include <test/a11y/XAccessibleExtendedComponentTester.hxx>
#include <test/a11y/XAccessibleEventBroadcasterTester.hxx>
#include <test/a11y/XAccessibleTextTester.hxx>
#include <test/a11y/accessibletestbase.hxx>
#include <vcl/scheduler.hxx>

using namespace css;

namespace
{
class AccessibleFixedTextTest : public test::AccessibleTestBase
{
private:
    void testFixedText();
    void runInterfaceTests(
        const css::uno::Reference<accessibility::XAccessibleContext>& rxFixedTextContext,
        const css::uno::Reference<css::awt::XWindow>& rxFixedTextWindow);

public:
    CPPUNIT_TEST_SUITE(AccessibleFixedTextTest);
    CPPUNIT_TEST(testFixedText);
    CPPUNIT_TEST_SUITE_END();
};

void AccessibleFixedTextTest::testFixedText()
{
    uno::Reference<lang::XMultiComponentFactory> xFactory(m_xContext->getServiceManager(),
                                                          uno::UNO_SET_THROW);
    uno::Reference<awt::XControlModel> xDlgModel(
        xFactory->createInstanceWithContext(u"com.sun.star.awt.UnoControlDialogModel"_ustr,
                                            m_xContext),
        uno::UNO_QUERY_THROW);

    css::uno::Reference<css::awt::XControl> xDlgControl(
        xFactory->createInstanceWithContext(u"com.sun.star.awt.UnoControlDialog"_ustr, m_xContext),
        css::uno::UNO_QUERY_THROW);
    xDlgControl->setModel(xDlgModel);

    css::uno::Reference<css::awt::XControlModel> xFixedTextModel(
        xFactory->createInstanceWithContext(u"com.sun.star.awt.UnoControlFixedTextModel"_ustr,
                                            m_xContext),
        css::uno::UNO_QUERY_THROW);

    css::uno::Reference<css::awt::XControl> xFixedTextControl(
        xFactory->createInstanceWithContext(u"com.sun.star.awt.UnoControlFixedText"_ustr,
                                            m_xContext),
        css::uno::UNO_QUERY_THROW);

    xFixedTextControl->setModel(xFixedTextModel);

    css::uno::Reference<css::awt::XFixedText> xFT(xFixedTextControl, css::uno::UNO_QUERY_THROW);
    xFT->setText("FxedText");

    /* Set the text control to its preferred size, otherwise it
     * defaults to the size hard coded in its constructor (100 x 12) */
    css::uno::Reference<css::awt::XLayoutConstrains> xLCTxt(xFixedTextControl,
                                                            css::uno::UNO_QUERY_THROW);
    css::awt::Size textSize = xLCTxt->getPreferredSize();
    css::uno::Reference<css::awt::XWindow> xWinTxt(xFixedTextControl, css::uno::UNO_QUERY_THROW);
    xWinTxt->setPosSize(0, 0, textSize.Width, textSize.Height, css::awt::PosSize::SIZE);

    css::uno::Reference<css::awt::XControlContainer> xControlContainer(xDlgControl,
                                                                       css::uno::UNO_QUERY_THROW);
    xControlContainer->addControl(u"Text"_ustr, xFixedTextControl);

    css::uno::Reference<css::awt::XWindow> xWinDlg(xDlgControl, css::uno::UNO_QUERY_THROW);
    xWinDlg->setVisible(true);
    xWinDlg->setPosSize(0, 0, 200, 100, css::awt::PosSize::SIZE);

    Scheduler::ProcessEventsToIdle();

    css::uno::Reference<css::accessibility::XAccessible> xRoot(xWinDlg, css::uno::UNO_QUERY_THROW);
    test::AccessibleTestBase::dumpA11YTree(xRoot);

    css::uno::Reference<css::accessibility::XAccessibleContext> xContext
        = AccessibilityTools::getAccessibleObjectForRole(xRoot,
                                                         css::accessibility::AccessibleRole::LABEL);
    runInterfaceTests(xContext, xWinTxt);
}

void AccessibleFixedTextTest::runInterfaceTests(
    const css::uno::Reference<accessibility::XAccessibleContext>& rxContext,
    const css::uno::Reference<css::awt::XWindow>& rxFixedTextWindow)
{
    XAccessibleContextTester aContextTester(rxContext);
    aContextTester.testAll();

    uno::Reference<accessibility::XAccessibleComponent> xAccessibleComponent(rxContext,
                                                                             uno::UNO_QUERY_THROW);
    XAccessibleComponentTester aComponentTester(xAccessibleComponent);
    aComponentTester.testAll();

    uno::Reference<accessibility::XAccessibleExtendedComponent> xAccessibleExtendedComponent(
        rxContext, uno::UNO_QUERY_THROW);
    XAccessibleExtendedComponentTester aExtendedComponentTester(xAccessibleExtendedComponent);
    aExtendedComponentTester.testAll();

    uno::Reference<accessibility::XAccessibleEventBroadcaster> xAccessibleEventBroadcaster(
        rxContext, uno::UNO_QUERY_THROW);
    auto aFireEventFunc = [&rxFixedTextWindow] {
        rxFixedTextWindow->setEnable(false);
        rxFixedTextWindow->setEnable(true);
    };
    XAccessibleEventBroadcasterTester aEventBroadcasterTester(xAccessibleEventBroadcaster,
                                                              aFireEventFunc);
    aEventBroadcasterTester.testAll();

    XAccessibleTextTester aTextTester(rxContext);
    aTextTester.testAll();
}

CPPUNIT_TEST_SUITE_REGISTRATION(AccessibleFixedTextTest);
} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
