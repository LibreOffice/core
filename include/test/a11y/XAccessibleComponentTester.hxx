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
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>

#include <test/a11y/AccessibilityTools.hxx>

class OOO_DLLPUBLIC_TEST XAccessibleComponentTester
{
private:
    const css::uno::Reference<css::accessibility::XAccessibleComponent> mxComponent;
    const css::uno::Reference<css::accessibility::XAccessibleContext> mxContext;

public:
    XAccessibleComponentTester(
        const css::uno::Reference<css::accessibility::XAccessibleComponent>& component)
        : mxComponent(component)
        , mxContext(component, css::uno::UNO_QUERY_THROW)
    {
    }

    void testBounds();
    void testSize();
    void testContainsPoint();
    void testAccessibleAtPoint();
    void testLocation();
    void testLocationOnScreen();
    void testGrabFocus();
    void testGetForeground();
    void testGetBackground();

    void testAll()
    {
        testBounds();
        testSize();
        testContainsPoint();
        testAccessibleAtPoint();
        testLocation();
        testLocationOnScreen();
        testGrabFocus();
        testGetForeground();
        testGetBackground();
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
