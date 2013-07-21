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


#include <uielement/statusbarwrapper.hxx>

#include <threadhelp/resetableguard.hxx>
#include <framework/actiontriggerhelper.hxx>
#include <uielement/constitemcontainer.hxx>
#include <uielement/rootitemcontainer.hxx>
#include <uielement/statusbar.hxx>
#include <helpid.hrc>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XSystemDependentMenuPeer.hpp>
#include <com/sun/star/awt/XMenuBar.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ui/UIElementType.hpp>

#include <comphelper/processfactory.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <tools/solar.h>
#include <vcl/svapp.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::frame;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::awt;
using namespace ::com::sun::star::ui;

namespace framework
{

StatusBarWrapper::StatusBarWrapper(
    const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext
    )
 :  UIConfigElementWrapperBase( UIElementType::STATUSBAR ),
    m_xContext( rxContext )
{
}

StatusBarWrapper::~StatusBarWrapper()
{
}

void SAL_CALL StatusBarWrapper::dispose() throw (::com::sun::star::uno::RuntimeException)
{
    Reference< XComponent > xThis( static_cast< OWeakObject* >(this), UNO_QUERY );

    com::sun::star::lang::EventObject aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    ResetableGuard aLock( m_aLock );
    if ( !m_bDisposed )
    {
        if ( m_xStatusBarManager.is() )
            m_xStatusBarManager->dispose();
        m_xStatusBarManager.clear();
        m_xConfigSource.clear();
        m_xConfigData.clear();
        m_xContext.clear();

        m_bDisposed = sal_True;
    }
    else
        throw DisposedException();
}

// XInitialization
void SAL_CALL StatusBarWrapper::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException )
{
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    if ( !m_bInitialized )
    {
        UIConfigElementWrapperBase::initialize( aArguments );

        Reference< XFrame > xFrame( m_xWeakFrame );
        if ( xFrame.is() && m_xConfigSource.is() )
        {
            // Create VCL based toolbar which will be filled with settings data
            StatusBar*        pStatusBar( 0 );
            StatusBarManager* pStatusBarManager( 0 );
            {
                SolarMutexGuard aSolarMutexGuard;
                Window* pWindow = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
                if ( pWindow )
                {
                    sal_uLong nStyles = WinBits( WB_LEFT | WB_3DLOOK );

                    pStatusBar = new FrameworkStatusBar( pWindow, nStyles );
                    pStatusBarManager = new StatusBarManager( m_xContext, xFrame, m_aResourceURL, pStatusBar );
                    ((FrameworkStatusBar*)pStatusBar)->SetStatusBarManager( pStatusBarManager );
                    m_xStatusBarManager = Reference< XComponent >( static_cast< OWeakObject *>( pStatusBarManager ), UNO_QUERY );
                    pStatusBar->SetUniqueId( HID_STATUSBAR );
                }
            }

            try
            {
                m_xConfigData = m_xConfigSource->getSettings( m_aResourceURL, sal_False );
                if ( m_xConfigData.is() && pStatusBar && pStatusBarManager )
                {
                    // Fill statusbar with container contents
                    pStatusBarManager->FillStatusBar( m_xConfigData );
                }
            }
            catch ( const NoSuchElementException& )
            {
            }
        }
    }
}

// XUIElementSettings
void SAL_CALL StatusBarWrapper::updateSettings() throw ( RuntimeException )
{
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_bPersistent &&
         m_xConfigSource.is() &&
         m_xStatusBarManager.is() )
    {
        try
        {
            StatusBarManager* pStatusBarManager = static_cast< StatusBarManager *>( m_xStatusBarManager.get() );

            m_xConfigData = m_xConfigSource->getSettings( m_aResourceURL, sal_False );
            if ( m_xConfigData.is() )
                pStatusBarManager->FillStatusBar( m_xConfigData );
        }
        catch ( const NoSuchElementException& )
        {
        }
    }
}

Reference< XInterface > SAL_CALL StatusBarWrapper::getRealInterface() throw ( RuntimeException )
{
    ResetableGuard aLock( m_aLock );

    if ( m_xStatusBarManager.is() )
    {
        StatusBarManager* pStatusBarManager = static_cast< StatusBarManager *>( m_xStatusBarManager.get() );
        if ( pStatusBarManager )
        {
            Window* pWindow = (Window *)pStatusBarManager->GetStatusBar();
            if ( pWindow )
                return Reference< XInterface >( VCLUnoHelper::GetInterface( pWindow ), UNO_QUERY );
        }
    }

    return Reference< XInterface >();
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
