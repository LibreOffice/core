/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: statusbarwrapper.cxx,v $
 * $Revision: 1.7 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#include <uielement/statusbarwrapper.hxx>

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <threadhelp/resetableguard.hxx>
#include <helper/actiontriggerhelper.hxx>
#include <uielement/constitemcontainer.hxx>
#include <uielement/rootitemcontainer.hxx>
#include <uielement/statusbar.hxx>
#include <helpid.hrc>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XSystemDependentMenuPeer.hpp>
#include <com/sun/star/awt/XMenuBar.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ui/UIElementType.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <comphelper/processfactory.hxx>
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif

#include <tools/solar.h>
#include <vcl/svapp.hxx>
#include <rtl/logfile.hxx>

using namespace rtl;
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
    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xServiceManager
    )
 :  UIConfigElementWrapperBase( UIElementType::STATUSBAR ),
    m_xServiceFactory( xServiceManager )
{
}

StatusBarWrapper::~StatusBarWrapper()
{
}

const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& StatusBarWrapper::getServiceFactory()
{
    ResetableGuard aLock( m_aLock );
    if ( m_bDisposed )
        throw DisposedException();

    return m_xServiceFactory;
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
        m_xServiceFactory.clear();

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
                vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
                Window* pWindow = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
                if ( pWindow )
                {
                    ULONG nStyles = WinBits( WB_LEFT | WB_3DLOOK );

                    pStatusBar = new FrameworkStatusBar( pWindow, nStyles );
                    pStatusBarManager = new StatusBarManager( m_xServiceFactory, xFrame, m_aResourceURL, pStatusBar );
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
            catch ( NoSuchElementException& )
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
        catch ( NoSuchElementException& )
        {
        }
    }
}

void SAL_CALL StatusBarWrapper::setSettings( const Reference< XIndexAccess >& xSettings ) throw ( RuntimeException )
{
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    if ( xSettings.is() )
    {
        // Create a copy of the data if the container is not const
        Reference< XIndexReplace > xReplace( xSettings, UNO_QUERY );
        if ( xReplace.is() )
            m_xConfigData = Reference< XIndexAccess >( static_cast< OWeakObject * >( new ConstItemContainer( xSettings ) ), UNO_QUERY );
        else
            m_xConfigData = xSettings;

        if ( m_xConfigSource.is() && m_bPersistent )
        {
            OUString aResourceURL( m_aResourceURL );
            Reference< XUIConfigurationManager > xUICfgMgr( m_xConfigSource );

            aLock.unlock();

            try
            {
                xUICfgMgr->replaceSettings( aResourceURL, m_xConfigData );
            }
            catch( NoSuchElementException& )
            {
            }
        }
    }
}

Reference< XIndexAccess > SAL_CALL StatusBarWrapper::getSettings( sal_Bool bWriteable ) throw ( RuntimeException )
{
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    if ( bWriteable )
        return Reference< XIndexAccess >( static_cast< OWeakObject * >( new RootItemContainer( m_xConfigData ) ), UNO_QUERY );
    else
        return m_xConfigData;
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
