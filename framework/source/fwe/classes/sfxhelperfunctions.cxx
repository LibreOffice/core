/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#include <framework/sfxhelperfunctions.hxx>

#include <tools/diagnose_ex.h>

static pfunc_setToolBoxControllerCreator   pToolBoxControllerCreator   = NULL;
static pfunc_setStatusBarControllerCreator pStatusBarControllerCreator = NULL;
static pfunc_getRefreshToolbars            pRefreshToolbars            = NULL;
static pfunc_createDockingWindow           pCreateDockingWindow        = NULL;
static pfunc_isDockingWindowVisible        pIsDockingWindowVisible     = NULL;
static pfunc_activateToolPanel             pActivateToolPanel          = NULL;



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

svt::ToolboxController* SAL_CALL CreateToolBoxController( const Reference< XFrame >& rFrame, ToolBox* pToolbox, unsigned short nID, const ::rtl::OUString& aCommandURL )
{
    pfunc_setToolBoxControllerCreator pFactory = NULL;
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        pFactory = pToolBoxControllerCreator;
    }

    if ( pFactory )
        return (*pFactory)( rFrame, pToolbox, nID, aCommandURL );
    else
        return NULL;
}

pfunc_setStatusBarControllerCreator SAL_CALL SetStatusBarControllerCreator( pfunc_setStatusBarControllerCreator pSetStatusBarControllerCreator )
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    pfunc_setStatusBarControllerCreator pOldSetStatusBarControllerCreator = pSetStatusBarControllerCreator;
    pStatusBarControllerCreator = pSetStatusBarControllerCreator;
    return pOldSetStatusBarControllerCreator;
}

svt::StatusbarController* SAL_CALL CreateStatusBarController( const Reference< XFrame >& rFrame, StatusBar* pStatusBar, unsigned short nID, const ::rtl::OUString& aCommandURL )
{
    pfunc_setStatusBarControllerCreator pFactory = NULL;
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        pFactory = pStatusBarControllerCreator;
    }

    if ( pFactory )
        return (*pFactory)( rFrame, pStatusBar, nID, aCommandURL );
    else
        return NULL;
}

pfunc_getRefreshToolbars SAL_CALL SetRefreshToolbars( pfunc_getRefreshToolbars pNewRefreshToolbarsFunc )
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    pfunc_getRefreshToolbars pOldFunc = pRefreshToolbars;
    pRefreshToolbars = pNewRefreshToolbarsFunc;

    return pOldFunc;
}

void SAL_CALL RefreshToolbars( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame )
{
    pfunc_getRefreshToolbars pCallback = NULL;
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

void SAL_CALL CreateDockingWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, const ::rtl::OUString& rResourceURL )
{
    pfunc_createDockingWindow pFactory = NULL;
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

bool SAL_CALL IsDockingWindowVisible( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, const ::rtl::OUString& rResourceURL )
{
    pfunc_isDockingWindowVisible pCall = NULL;
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        pCall = pIsDockingWindowVisible;
    }

    if ( pIsDockingWindowVisible )
        return (*pIsDockingWindowVisible)( rFrame, rResourceURL );
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

void SAL_CALL ActivateToolPanel( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& i_rFrame, const ::rtl::OUString& i_rPanelURL )
{
    pfunc_activateToolPanel pActivator = NULL;
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        pActivator = pActivateToolPanel;
    }

    ENSURE_OR_RETURN_VOID( pActivator, "framework::ActivateToolPanel: no activator function!" );
    (*pActivator)( i_rFrame, i_rPanelURL );
}

}
