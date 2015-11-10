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

#include <framework/sfxhelperfunctions.hxx>
#include <framework/ContextChangeEventMultiplexerTunnel.hxx>
#include <helper/mischelper.hxx>

#include <tools/diagnose_ex.h>

static pfunc_setToolBoxControllerCreator   pToolBoxControllerCreator   = nullptr;
static pfunc_setStatusBarControllerCreator pStatusBarControllerCreator = nullptr;
static pfunc_getRefreshToolbars            pRefreshToolbars            = nullptr;
static pfunc_createDockingWindow           pCreateDockingWindow        = nullptr;
static pfunc_isDockingWindowVisible        pIsDockingWindowVisible     = nullptr;
static pfunc_activateToolPanel             pActivateToolPanel          = nullptr;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;

namespace framework
{

pfunc_setToolBoxControllerCreator SAL_CALL SetToolBoxControllerCreator( pfunc_setToolBoxControllerCreator pSetToolBoxControllerCreator )
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    pfunc_setToolBoxControllerCreator pOldSetToolBoxControllerCreator = pToolBoxControllerCreator;
    pToolBoxControllerCreator = pSetToolBoxControllerCreator;
    return pOldSetToolBoxControllerCreator;
}

svt::ToolboxController* SAL_CALL CreateToolBoxController( const Reference< XFrame >& rFrame, ToolBox* pToolbox, unsigned short nID, const OUString& aCommandURL )
{
    pfunc_setToolBoxControllerCreator pFactory = nullptr;
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        pFactory = pToolBoxControllerCreator;
    }

    if ( pFactory )
        return (*pFactory)( rFrame, pToolbox, nID, aCommandURL );
    else
        return nullptr;
}

pfunc_setStatusBarControllerCreator SAL_CALL SetStatusBarControllerCreator( pfunc_setStatusBarControllerCreator pSetStatusBarControllerCreator )
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    pfunc_setStatusBarControllerCreator pOldSetStatusBarControllerCreator = pSetStatusBarControllerCreator;
    pStatusBarControllerCreator = pSetStatusBarControllerCreator;
    return pOldSetStatusBarControllerCreator;
}

svt::StatusbarController* SAL_CALL CreateStatusBarController( const Reference< XFrame >& rFrame, StatusBar* pStatusBar, unsigned short nID, const OUString& aCommandURL )
{
    pfunc_setStatusBarControllerCreator pFactory = nullptr;
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        pFactory = pStatusBarControllerCreator;
    }

    if ( pFactory )
        return (*pFactory)( rFrame, pStatusBar, nID, aCommandURL );
    else
        return nullptr;
}

pfunc_getRefreshToolbars SAL_CALL SetRefreshToolbars( pfunc_getRefreshToolbars pNewRefreshToolbarsFunc )
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    pfunc_getRefreshToolbars pOldFunc = pRefreshToolbars;
    pRefreshToolbars = pNewRefreshToolbarsFunc;

    return pOldFunc;
}

void SAL_CALL RefreshToolbars( css::uno::Reference< css::frame::XFrame >& rFrame )
{
    pfunc_getRefreshToolbars pCallback = nullptr;
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        pCallback = pRefreshToolbars;
    }

    if ( pCallback )
        (*pCallback)( rFrame );
}

pfunc_createDockingWindow SAL_CALL SetDockingWindowCreator( pfunc_createDockingWindow pNewCreateDockingWindow )
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    pfunc_createDockingWindow pOldFunc = pCreateDockingWindow;
    pCreateDockingWindow = pNewCreateDockingWindow;

    return pOldFunc;
}

void SAL_CALL CreateDockingWindow( const css::uno::Reference< css::frame::XFrame >& rFrame, const OUString& rResourceURL )
{
    pfunc_createDockingWindow pFactory = nullptr;
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        pFactory = pCreateDockingWindow;
    }

    if ( pFactory )
        (*pFactory)( rFrame, rResourceURL );
}

pfunc_isDockingWindowVisible SAL_CALL SetIsDockingWindowVisible( pfunc_isDockingWindowVisible pNewIsDockingWindowVisible)
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    pfunc_isDockingWindowVisible pOldFunc = pIsDockingWindowVisible;
    pIsDockingWindowVisible = pNewIsDockingWindowVisible;

    return pOldFunc;
}

bool SAL_CALL IsDockingWindowVisible( const css::uno::Reference< css::frame::XFrame >& rFrame, const OUString& rResourceURL )
{
    pfunc_isDockingWindowVisible pCall = nullptr;
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        pCall = pIsDockingWindowVisible;
    }

    if ( pCall )
        return (*pCall)( rFrame, rResourceURL );
    else
        return false;
}

pfunc_activateToolPanel SAL_CALL SetActivateToolPanel( pfunc_activateToolPanel i_pActivator )
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    pfunc_activateToolPanel pOldFunc = pActivateToolPanel;
    pActivateToolPanel = i_pActivator;
    return pOldFunc;
}

void SAL_CALL ActivateToolPanel( const css::uno::Reference< css::frame::XFrame >& i_rFrame, const OUString& i_rPanelURL )
{
    pfunc_activateToolPanel pActivator = nullptr;
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        pActivator = pActivateToolPanel;
    }

    ENSURE_OR_RETURN_VOID( pActivator, "framework::ActivateToolPanel: no activator function!" );
    (*pActivator)( i_rFrame, i_rPanelURL );
}

using namespace ::com::sun::star;
uno::Reference<ui::XContextChangeEventListener> GetFirstListenerWith(
    uno::Reference<uno::XInterface> const& xEventFocus,
    std::function<bool (uno::Reference<ui::XContextChangeEventListener> const&)> const& rPredicate)
{
    return GetFirstListenerWith_Impl(xEventFocus, rPredicate);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
