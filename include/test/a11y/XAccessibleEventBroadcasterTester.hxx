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

#include <test/testdllapi.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <vcl/window.hxx>

class OOO_DLLPUBLIC_TEST XAccessibleEventBroadcasterTester
{
private:
    const css::uno::Reference<css::accessibility::XAccessibleEventBroadcaster> mxBroadcaster;
    std::function<void()> m_aFireEventFunc;

    static bool isTransient(
        const css::uno::Reference<css::accessibility::XAccessibleEventBroadcaster>& xBroadcaster);
    static bool
    isTransient(const css::uno::Reference<css::accessibility::XAccessibleContext>& xCtx);

    void fireEvent();

public:
    /**
     * Creates an XAccessibleEventBroadcasterTester that will call @c rFireEventFunc to generate
     * an event.
     * @param xBroadcaster The XAccessibleEventBroadcaster implementation to test.
     * @param rFireEventFunc Function that triggers an accessible event for @c xBroadcaster.
     */
    XAccessibleEventBroadcasterTester(
        const css::uno::Reference<css::accessibility::XAccessibleEventBroadcaster>& xBroadcaster,
        const std::function<void()>& rFireEventFunc);

    void testAddEventListener();
    void testRemoveEventListener();

    void testAll()
    {
        testAddEventListener();
        testRemoveEventListener();
    }
};

/**
 * Specialized XAccessibleEventBroadcaster subclass that modifies the
 * passed vcl::Window so that accessible events are generated for it.
 */
class OOO_DLLPUBLIC_TEST WindowXAccessibleEventBroadcasterTester
    : public XAccessibleEventBroadcasterTester
{
public:
    /**
     * Creates an WindowXAccessibleEventBroadcasterTester that will modify @c pWindow to generate
     * at least one accessible event for it.
     * @param xBroadcaster The XAccessibleEventBroadcaster implementation to test.
     * @param pWindow Window for whose accessible events @c xBroadcaster is responsible.
     */
    WindowXAccessibleEventBroadcasterTester(
        const css::uno::Reference<css::accessibility::XAccessibleEventBroadcaster>& xBroadcaster,
        vcl::Window* pWindow);

private:
    static void triggerWindowEvent(vcl::Window* pWindow);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
