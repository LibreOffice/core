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
#include <svtools/toolboxcontroller.hxx>
#include <svtools/statusbarcontroller.hxx>

static pfunc_setToolBoxControllerCreator   pToolBoxControllerCreator   = nullptr;
static pfunc_setWeldToolBoxControllerCreator pWeldToolBoxControllerCreator = nullptr;
static pfunc_setStatusBarControllerCreator pStatusBarControllerCreator = nullptr;
static pfunc_getRefreshToolbars            pRefreshToolbars            = nullptr;
static pfunc_createDockingWindow           pCreateDockingWindow        = nullptr;
static pfunc_isDockingWindowVisible        pIsDockingWindowVisible     = nullptr;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;

namespace framework
{

pfunc_setToolBoxControllerCreator SetToolBoxControllerCreator( pfunc_setToolBoxControllerCreator pSetToolBoxControllerCreator )
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    pfunc_setToolBoxControllerCreator pOldSetToolBoxControllerCreator = pToolBoxControllerCreator;
    pToolBoxControllerCreator = pSetToolBoxControllerCreator;
    return pOldSetToolBoxControllerCreator;
}

rtl::Reference<svt::ToolboxController> CreateToolBoxController( const Reference< XFrame >& rFrame, ToolBox* pToolbox, ToolBoxItemId nID, const OUString& aCommandURL )
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

pfunc_setWeldToolBoxControllerCreator SetWeldToolBoxControllerCreator( pfunc_setWeldToolBoxControllerCreator pSetWeldToolBoxControllerCreator )
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    pfunc_setWeldToolBoxControllerCreator pOldSetToolBoxControllerCreator = pWeldToolBoxControllerCreator;
    pWeldToolBoxControllerCreator = pSetWeldToolBoxControllerCreator;
    return pOldSetToolBoxControllerCreator;
}

css::uno::Reference<css::frame::XToolbarController> CreateWeldToolBoxController( const Reference< XFrame >& rFrame, weld::Toolbar* pToolbar, weld::Builder* pBuilder, const OUString& aCommandURL )
{
    pfunc_setWeldToolBoxControllerCreator pFactory = nullptr;
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        pFactory = pWeldToolBoxControllerCreator;
    }

    if ( pFactory )
        return (*pFactory)( rFrame, pToolbar, pBuilder, aCommandURL );
    else
        return nullptr;
}

pfunc_setStatusBarControllerCreator SetStatusBarControllerCreator( pfunc_setStatusBarControllerCreator pSetStatusBarControllerCreator )
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    pfunc_setStatusBarControllerCreator pOldSetStatusBarControllerCreator = pSetStatusBarControllerCreator;
    pStatusBarControllerCreator = pSetStatusBarControllerCreator;
    return pOldSetStatusBarControllerCreator;
}

rtl::Reference<svt::StatusbarController> CreateStatusBarController( const Reference< XFrame >& rFrame, StatusBar* pStatusBar, unsigned short nID, const OUString& aCommandURL )
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

pfunc_getRefreshToolbars SetRefreshToolbars( pfunc_getRefreshToolbars pNewRefreshToolbarsFunc )
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    pfunc_getRefreshToolbars pOldFunc = pRefreshToolbars;
    pRefreshToolbars = pNewRefreshToolbarsFunc;

    return pOldFunc;
}

void RefreshToolbars( css::uno::Reference< css::frame::XFrame > const & rFrame )
{
    pfunc_getRefreshToolbars pCallback = nullptr;
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        pCallback = pRefreshToolbars;
    }

    if ( pCallback )
        (*pCallback)( rFrame );
}

pfunc_createDockingWindow SetDockingWindowCreator( pfunc_createDockingWindow pNewCreateDockingWindow )
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    pfunc_createDockingWindow pOldFunc = pCreateDockingWindow;
    pCreateDockingWindow = pNewCreateDockingWindow;

    return pOldFunc;
}

void CreateDockingWindow( const css::uno::Reference< css::frame::XFrame >& rFrame, const OUString& rResourceURL )
{
    pfunc_createDockingWindow pFactory = nullptr;
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        pFactory = pCreateDockingWindow;
    }

    if ( pFactory )
        (*pFactory)( rFrame, rResourceURL );
}

pfunc_isDockingWindowVisible SetIsDockingWindowVisible( pfunc_isDockingWindowVisible pNewIsDockingWindowVisible)
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    pfunc_isDockingWindowVisible pOldFunc = pIsDockingWindowVisible;
    pIsDockingWindowVisible = pNewIsDockingWindowVisible;

    return pOldFunc;
}

bool IsDockingWindowVisible( const css::uno::Reference< css::frame::XFrame >& rFrame, const OUString& rResourceURL )
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

using namespace ::com::sun::star;
uno::Reference<ui::XContextChangeEventListener> GetFirstListenerWith(
    css::uno::Reference<css::uno::XComponentContext> const & xComponentContext,
    uno::Reference<uno::XInterface> const& xEventFocus,
    std::function<bool (uno::Reference<ui::XContextChangeEventListener> const&)> const& rPredicate)
{
    return GetFirstListenerWith_Impl(xComponentContext, xEventFocus, rPredicate);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
