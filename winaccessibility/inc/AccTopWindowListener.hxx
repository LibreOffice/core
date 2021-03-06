/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <com/sun/star/awt/XTopWindowListener.hpp>
#include <com/sun/star/awt/XExtendedToolkit.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>

#include <cppuhelper/implbase.hxx>

#include "AccObjectManagerAgent.hxx"

/**
 * AccEventListener is the general event listener for all top windows. The top windows defined
 * in UNO are: FRAME, WINDOW, DIALOG, MENU, TOOLTIP.
 * It implements the methods of XTopWindowListener and the most important method is windowOpened().
 * In this method, all the accessible objects (including COM object and Uno objects) are created and
 * cached into bridge managers, and they are monitored by listeners for later accessible event handling.
 */
class AccTopWindowListener : public ::cppu::WeakImplHelper<css::awt::XTopWindowListener>
{
private:
    AccObjectManagerAgent accManagerAgent;

public:
    AccTopWindowListener();
    virtual ~AccTopWindowListener() override;

    // XTopWindowListener
    virtual void SAL_CALL windowOpened(const css::lang::EventObject& e) override;
    virtual void SAL_CALL windowClosing(const css::lang::EventObject& e) override;
    virtual void SAL_CALL windowClosed(const css::lang::EventObject& e) override;
    virtual void SAL_CALL windowMinimized(const css::lang::EventObject& e) override;
    virtual void SAL_CALL windowNormalized(const css::lang::EventObject& e) override;
    virtual void SAL_CALL windowActivated(const css::lang::EventObject& e) override;
    virtual void SAL_CALL windowDeactivated(const css::lang::EventObject& e) override;

    // XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;

    virtual void AddAllListeners(css::accessibility::XAccessible* pAccessible,
                                 css::accessibility::XAccessible* pParentXAcc, HWND pWND);
    //for On-Demand load.
    virtual void HandleWindowOpened(css::accessibility::XAccessible* pAccessible);

    sal_Int64 GetMSComPtr(sal_Int64 hWnd, sal_Int64 lParam, sal_Int64 wParam);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
