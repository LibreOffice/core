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

#pragma once

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/awt/XWindow.hpp>

class XAccessibleEventBroadcasterTester
{
private:
    const css::uno::Reference<css::accessibility::XAccessibleEventBroadcaster> mxBroadcaster;
    const css::uno::Reference<css::awt::XWindow> mxWindow;

    static bool isTransient(
        const css::uno::Reference<css::accessibility::XAccessibleEventBroadcaster> xBroadcaster);
    static bool isTransient(const css::uno::Reference<css::accessibility::XAccessibleContext> xCtx);

    void fireEvent();

public:
    XAccessibleEventBroadcasterTester(
        const css::uno::Reference<css::accessibility::XAccessibleEventBroadcaster>& xBroadcaster,
        const css::uno::Reference<css::awt::XWindow>& xWindow);

    void testAddEventListener();
    void testRemoveEventListener();

    void testAll()
    {
        testAddEventListener();
        testRemoveEventListener();
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
