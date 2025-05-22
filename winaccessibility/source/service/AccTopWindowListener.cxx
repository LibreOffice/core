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


#include <sal/log.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>
#include <vcl/accessibility/vclxaccessiblecomponent.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/svapp.hxx>

#include <AccTopWindowListener.hxx>
#include <unomsaaevent.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/awt/XWindow.hpp>

using namespace com::sun::star::uno;

/**
 *  For the new opened window, generate all the UNO accessible's object, COM object and add
 *  accessible listener to monitor all these objects.
 *  @param pWindow      the new opened window
 */
void AccTopWindowListener::HandleWindowOpened(vcl::Window* pWindow)
{
    assert(pWindow);

    const SystemEnvData* pSystemData = pWindow->GetSystemData();
    if (!pSystemData)
        return;

    Reference<css::accessibility::XAccessible> xAccessible = pWindow->GetAccessible();
    if (!xAccessible.is())
        return;

    Reference<css::accessibility::XAccessibleContext> xContext = xAccessible->getAccessibleContext();
    if(!xContext.is())
        return;

    // add all listeners
    m_aAccObjectManager.SaveTopWindowHandle(pSystemData->hWnd, xAccessible.get());

    AddAllListeners(xAccessible.get(), nullptr, pSystemData->hWnd);

    if (pWindow->GetStyle() & WB_MOVEABLE)
        m_aAccObjectManager.IncreaseState(xAccessible.get(), static_cast<unsigned short>(-1) /* U_MOVEBLE */ );

    short role = xContext->getAccessibleRole();

    if (role == css::accessibility::AccessibleRole::POPUP_MENU ||
            role == css::accessibility::AccessibleRole::MENU )
    {
        m_aAccObjectManager.NotifyAccEvent(xAccessible.get(), UnoMSAAEvent::MENUPOPUPSTART);
    }

    if (role == css::accessibility::AccessibleRole::FRAME ||
            role == css::accessibility::AccessibleRole::DIALOG ||
            role == css::accessibility::AccessibleRole::WINDOW ||
            role == css::accessibility::AccessibleRole::ALERT)
    {
        m_aAccObjectManager.NotifyAccEvent(xAccessible.get(), UnoMSAAEvent::SHOW);
    }
}

AccTopWindowListener::AccTopWindowListener()
    : m_aAccObjectManager()
{
}

AccTopWindowListener::~AccTopWindowListener()
{
}

/**
 *  It is invoked when a new window is opened, the source of this EventObject is the window
 */
void AccTopWindowListener::windowOpened( const css::lang::EventObject& e )
{
    SAL_INFO( "iacc2", "windowOpened triggered" );

    if ( !e.Source.is())
        return;

    SolarMutexGuard g;

    css::uno::Reference<css::awt::XWindow> xWindow(e.Source, UNO_QUERY_THROW);
    vcl::Window* pWindow = VCLUnoHelper::GetWindow(xWindow);
    assert(pWindow);

    HandleWindowOpened(pWindow);
}

/**
 *  Add the accessible event listener to object and all its children objects.
 *  @param  pAccessible     the accessible object
 *  @param  pParentXAcc     the parent of current accessible object
 *  @param  pWND            the handle of top window which current object resides
 */
void AccTopWindowListener::AddAllListeners(css::accessibility::XAccessible* pAccessible, css::accessibility::XAccessible* pParentXAcc, HWND pWND)
{
    Reference<css::accessibility::XAccessibleContext> xContext = pAccessible->getAccessibleContext();
    if(!xContext.is())
    {
        return;
    }

    m_aAccObjectManager.InsertAccObj(pAccessible, pParentXAcc, pWND);

    if (!AccObjectWinManager::IsContainer(pAccessible))
    {
        return;
    }


    short role = xContext->getAccessibleRole();
    if(css::accessibility::AccessibleRole::DOCUMENT == role ||
            css::accessibility::AccessibleRole::DOCUMENT_PRESENTATION == role ||
            css::accessibility::AccessibleRole::DOCUMENT_SPREADSHEET == role ||
            css::accessibility::AccessibleRole::DOCUMENT_TEXT == role)
    {
        if(AccObjectWinManager::IsStateManageDescendant(pAccessible))
        {
            return ;
        }
    }

    sal_Int64 nCount = xContext->getAccessibleChildCount();
    for (sal_Int64 i = 0; i < nCount; i++)
    {
        Reference<css::accessibility::XAccessible> xAccessible
            = xContext->getAccessibleChild(i);

        if (xAccessible.is())
            AddAllListeners(xAccessible.get(), pAccessible, pWND);
    }
}

void AccTopWindowListener::windowClosing( const css::lang::EventObject& )
{
    SAL_INFO( "iacc2", "windowClosing triggered" );
}

/**
 *  Invoke this method when the top window is closed, remove all the objects and its children
 *  from current manager's cache, and remove the COM object and the accessible event listener
 *  assigned to the accessible objects.
 */
void AccTopWindowListener::windowClosed( const css::lang::EventObject& e )
{
    SAL_INFO( "iacc2", "windowClosed triggered" );

    if ( !e.Source.is())
        return;

    css::uno::Reference<css::awt::XWindow> xWindow(e.Source, UNO_QUERY_THROW);
    vcl::Window* pWindow = VCLUnoHelper::GetWindow(xWindow);
    assert(pWindow);

    Reference<css::accessibility::XAccessible> xAccessible = pWindow->GetAccessible();
    if (!xAccessible.is())
        return;

    Reference<css::accessibility::XAccessibleContext> xContext = xAccessible->getAccessibleContext();
    if(!xContext.is())
        return;

    short role = xContext->getAccessibleRole();
    if (role == css::accessibility::AccessibleRole::POPUP_MENU ||
            role == css::accessibility::AccessibleRole::MENU)
    {
        m_aAccObjectManager.NotifyAccEvent(xAccessible.get(), UnoMSAAEvent::MENUPOPUPEND);
    }

    m_aAccObjectManager.DeleteChildrenAccObj(xAccessible.get());
    m_aAccObjectManager.DeleteAccObj(xAccessible.get());
}

void AccTopWindowListener::windowMinimized( const css::lang::EventObject& )
{
}

void AccTopWindowListener::windowNormalized( const css::lang::EventObject& )
{
}

void AccTopWindowListener::windowActivated( const css::lang::EventObject& )
{
}

void AccTopWindowListener::windowDeactivated( const css::lang::EventObject& )
{
}

void AccTopWindowListener::disposing( const css::lang::EventObject&  )
{
}

sal_Int64 AccTopWindowListener::GetMSComPtr(
        sal_Int64 hWnd, sal_Int64 lParam, sal_Int64 wParam)
{
    return m_aAccObjectManager.Get_ToATInterface(hWnd, lParam, wParam);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
