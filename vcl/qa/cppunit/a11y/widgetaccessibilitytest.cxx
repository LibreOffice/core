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

#define VCL_INTERNALS

#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleExtendedAttributes.hpp>
#include <com/sun/star/accessibility/XAccessibleExtendedComponent.hpp>
#include <test/a11y/AccessibilityTools.hxx>
#include <test/a11y/accessibletestbase.hxx>
#include <test/a11y/XAccessibleActionTester.hxx>
#include <test/a11y/XAccessibleComponentTester.hxx>
#include <test/a11y/XAccessibleContextTester.hxx>
#include <test/a11y/XAccessibleExtendedComponentTester.hxx>
#include <vcl/toolkit/dialog.hxx>
#include <vcl/toolkit/lstbox.hxx>

CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, AccessibleDropDownListBox)
{
    // create and show dialog that contains a dropdown listbox
    ScopedVclPtr<::Dialog> pDialog = VclPtr<::Dialog>::Create(nullptr, WB_MOVEABLE | WB_CLOSEABLE,
                                                              ::Dialog::InitFlag::NoParent);
    VclPtr<ListBox> pListBox = VclPtr<ListBox>::Create(pDialog, WB_DROPDOWN);
    pListBox->InsertEntry(u"First Entry"_ustr);
    pListBox->InsertEntry(u"Second Entry"_ustr);
    pListBox->SetSizePixel(pListBox->get_preferred_size());
    pListBox->Show();
    pDialog->Show();

    // perform various a11y checks on the dropdown listbox
    css::uno::Reference<css::accessibility::XAccessible> xListBoxAcc = pListBox->GetAccessible();
    CPPUNIT_ASSERT(xListBoxAcc.is());
    css::uno::Reference<css::accessibility::XAccessibleContext> xContext
        = xListBoxAcc->getAccessibleContext();
    CPPUNIT_ASSERT(xContext.is());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Drop down listbox has incorrect role",
                                 css::accessibility::AccessibleRole::COMBO_BOX,
                                 xContext->getAccessibleRole());

    XAccessibleContextTester aContextTester(xContext);
    aContextTester.testAll();

    css::uno::Reference<css::accessibility::XAccessibleComponent> xAccessibleComponent(
        xContext, css::uno::UNO_QUERY_THROW);
    XAccessibleComponentTester aComponentTester(xAccessibleComponent);
    aComponentTester.testAll();

    css::uno::Reference<css::accessibility::XAccessibleExtendedComponent>
        xAccessibleExtendedComponent(xContext, css::uno::UNO_QUERY_THROW);
    XAccessibleExtendedComponentTester aExtendedComponentTester(xAccessibleExtendedComponent);
    aExtendedComponentTester.testAll();

    css::uno::Reference<css::accessibility::XAccessibleAction> xAccessibleAction(
        xContext, css::uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("No accessible action provided",
                           xAccessibleAction->getAccessibleActionCount() > 0);
    XAccessibleActionTester aActionTester(xAccessibleAction);
    aActionTester.testAll();
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
