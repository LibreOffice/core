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
#include <vcl/window.hxx>
#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#include <toolkit/awt/vclxwindow.hxx>

#include <vcl/sysdata.hxx>
#include <vcl/svapp.hxx>

#include <AccTopWindowListener.hxx>
#include <unomsaaevent.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>

using namespace com::sun::star::uno;

/**
 *  For the new opened window, generate all the UNO accessible's object, COM object and add
 *  accessible listener to monitor all these objects.
 *  @param pAccessible      the accessible of the new opened window
 */
void AccTopWindowListener::HandleWindowOpened( css::accessibility::XAccessible* pAccessible )
{
    //get SystemData from window
    VclPtr<vcl::Window> window;
    if (auto pvclwindow = dynamic_cast<VCLXWindow*>(pAccessible))
        window = pvclwindow->GetWindow();
    else if (auto pvclxcomponent = dynamic_cast<VCLXAccessibleComponent*>(pAccessible))
        window = pvclxcomponent->GetWindow();
    assert(window);
    // The SalFrame of window may be destructed at this time
    const SystemEnvData* systemdata = nullptr;
    try
    {
        systemdata = window->GetSystemData();
    }
    catch(...)
    {
        systemdata = nullptr;
    }
    Reference<css::accessibility::XAccessibleContext> xContext = pAccessible->getAccessibleContext();
    if(!xContext.is())
        return;

    css::accessibility::XAccessibleContext* pAccessibleContext = xContext.get();
    //Only AccessibleContext exist, add all listeners
    if(pAccessibleContext != nullptr && systemdata != nullptr)
    {
        m_aAccObjectManager.SaveTopWindowHandle(systemdata->hWnd, pAccessible);

        AddAllListeners(pAccessible,nullptr,systemdata->hWnd);

        if( window->GetStyle() & WB_MOVEABLE )
            m_aAccObjectManager.IncreaseState( pAccessible, static_cast<unsigned short>(-1) /* U_MOVEBLE */ );

        short role = pAccessibleContext->getAccessibleRole();

        if (role == css::accessibility::AccessibleRole::POPUP_MENU ||
                role == css::accessibility::AccessibleRole::MENU )
        {
            m_aAccObjectManager.NotifyAccEvent(pAccessible, UnoMSAAEvent::MENUPOPUPSTART);
        }

        if (role == css::accessibility::AccessibleRole::FRAME ||
                role == css::accessibility::AccessibleRole::DIALOG ||
                role == css::accessibility::AccessibleRole::WINDOW ||
                role == css::accessibility::AccessibleRole::ALERT)
        {
            m_aAccObjectManager.NotifyAccEvent(pAccessible, UnoMSAAEvent::SHOW);
        }
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

    Reference< css::accessibility::XAccessible > xAccessible ( e.Source, UNO_QUERY );
    css::accessibility::XAccessible* pAccessible = xAccessible.get();
    if ( !pAccessible )
        return;

    SolarMutexGuard g;

    HandleWindowOpened( pAccessible );
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
    css::accessibility::XAccessibleContext* pAccessibleContext = xContext.get();
    if(pAccessibleContext == nullptr)
    {
        return;
    }

    m_aAccObjectManager.InsertAccObj(pAccessible, pParentXAcc, pWND);

    if (!AccObjectWinManager::IsContainer(pAccessible))
    {
        return;
    }


    short role = pAccessibleContext->getAccessibleRole();
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

    sal_Int64 nCount = pAccessibleContext->getAccessibleChildCount();
    for (sal_Int64 i = 0; i < nCount; i++)
    {
        Reference<css::accessibility::XAccessible> xAccessible
            = pAccessibleContext->getAccessibleChild(i);

        css::accessibility::XAccessible* mpAccessible = xAccessible.get();
        if(mpAccessible != nullptr)
        {
            Reference<css::accessibility::XAccessibleContext> xAccessibleContext
                = mpAccessible->getAccessibleContext();
            css::accessibility::XAccessibleContext* mpContext = xAccessibleContext.get();
            if(mpContext != nullptr)
                AddAllListeners( mpAccessible, pAccessible, pWND);
        }
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

    Reference< css::accessibility::XAccessible > xAccessible ( e.Source, UNO_QUERY );
    css::accessibility::XAccessible* pAccessible = xAccessible.get();
    if ( pAccessible == nullptr)
        return;

    Reference<css::accessibility::XAccessibleContext> xContext = pAccessible->getAccessibleContext();
    if(!xContext.is())
    {
        return;
    }
    css::accessibility::XAccessibleContext* pAccessibleContext = xContext.get();

    short role = -1;
    if(pAccessibleContext != nullptr)
    {
        role = pAccessibleContext->getAccessibleRole();

        if (role == css::accessibility::AccessibleRole::POPUP_MENU ||
                role == css::accessibility::AccessibleRole::MENU)
        {
            m_aAccObjectManager.NotifyAccEvent(pAccessible, UnoMSAAEvent::MENUPOPUPEND);
        }
    }


    m_aAccObjectManager.DeleteChildrenAccObj( pAccessible );
    if( role != css::accessibility::AccessibleRole::POPUP_MENU )
        m_aAccObjectManager.DeleteAccObj( pAccessible );

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
