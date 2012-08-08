/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <uielement/toolbarwrapper.hxx>
#include <threadhelp/resetableguard.hxx>
#include <framework/actiontriggerhelper.hxx>
#include <uielement/constitemcontainer.hxx>
#include <uielement/rootitemcontainer.hxx>
#include <uielement/toolbarmanager.hxx>

#include <uielement/toolbar.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XSystemDependentMenuPeer.hpp>
#include <com/sun/star/awt/XMenuBar.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

#include <toolkit/unohlp.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <comphelper/processfactory.hxx>

#include <svtools/miscopt.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <rtl/logfile.hxx>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::frame;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::awt;
using namespace ::com::sun::star::ui;

namespace framework
{

ToolBarWrapper::ToolBarWrapper( const Reference< XMultiServiceFactory >& xServiceManager ) :
    UIConfigElementWrapperBase( UIElementType::TOOLBAR,xServiceManager )
{
}

ToolBarWrapper::~ToolBarWrapper()
{
}

// XInterface
void SAL_CALL ToolBarWrapper::acquire() throw()
{
    UIConfigElementWrapperBase::acquire();
}

void SAL_CALL ToolBarWrapper::release() throw()
{
    UIConfigElementWrapperBase::release();
}

uno::Any SAL_CALL ToolBarWrapper::queryInterface( const uno::Type & rType )
throw( ::com::sun::star::uno::RuntimeException )
{
    Any a = ::cppu::queryInterface(
                rType ,
                (static_cast< ::com::sun::star::ui::XUIFunctionListener* >(this)) );

    if( a.hasValue() )
        return a;

    return UIConfigElementWrapperBase::queryInterface( rType );
}

// XComponent
void SAL_CALL ToolBarWrapper::dispose() throw ( RuntimeException )
{
    Reference< XComponent > xThis( static_cast< OWeakObject* >(this), UNO_QUERY );

    {
        ResetableGuard aLock( m_aLock );
        if ( m_bDisposed )
            return;
    }

    com::sun::star::lang::EventObject aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    ResetableGuard aLock( m_aLock );

    if ( m_xToolBarManager.is() )
        m_xToolBarManager->dispose();
    m_xToolBarManager.clear();
    m_xConfigSource.clear();
    m_xConfigData.clear();
    m_xToolBarWindow.clear();

    m_bDisposed = sal_True;
}

// XInitialization
void SAL_CALL ToolBarWrapper::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException )
{
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    if ( !m_bInitialized )
    {
        UIConfigElementWrapperBase::initialize( aArguments );

        sal_Bool bPopupMode( sal_False );
        for ( sal_Int32 i = 0; i < aArguments.getLength(); i++ )
        {
            PropertyValue aPropValue;
            if ( aArguments[i] >>= aPropValue )
            {
                if ( aPropValue.Name.equalsAsciiL( "PopupMode", 9 ))
                {
                    aPropValue.Value >>= bPopupMode;
                    break;
                }
            }
        }

        Reference< XFrame > xFrame( m_xWeakFrame );
        if ( xFrame.is() && m_xConfigSource.is() )
        {
            // Create VCL based toolbar which will be filled with settings data
            ToolBar* pToolBar = 0;
            ToolBarManager* pToolBarManager = 0;
            {
                SolarMutexGuard aSolarMutexGuard;
                Window* pWindow = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
                if ( pWindow )
                {
                    sal_uLong nStyles = WB_LINESPACING | WB_BORDER | WB_SCROLL | WB_MOVEABLE | WB_3DLOOK | WB_DOCKABLE | WB_SIZEABLE | WB_CLOSEABLE;

                    pToolBar = new ToolBar( pWindow, nStyles );
                    m_xToolBarWindow = VCLUnoHelper::GetInterface( pToolBar );
                    pToolBarManager = new ToolBarManager( m_xServiceFactory, xFrame, m_aResourceURL, pToolBar );
                    pToolBar->SetToolBarManager( pToolBarManager );
                    m_xToolBarManager = Reference< XComponent >( static_cast< OWeakObject *>( pToolBarManager ), UNO_QUERY );
                    pToolBar->WillUsePopupMode( bPopupMode );
                }
            }

            try
            {
                m_xConfigData = m_xConfigSource->getSettings( m_aResourceURL, sal_False );
                if ( m_xConfigData.is() && pToolBar && pToolBarManager )
                {
                    // Fill toolbar with container contents
                    pToolBarManager->FillToolbar( m_xConfigData );
                    pToolBar->SetOutStyle( SvtMiscOptions().GetToolboxStyle() );
                    pToolBar->EnableCustomize( sal_True );
                    ::Size aActSize( pToolBar->GetSizePixel() );
                    ::Size aSize( pToolBar->CalcWindowSizePixel() );
                    aSize.Width() = aActSize.Width();
                    pToolBar->SetOutputSizePixel( aSize );
                }
            }
            catch ( const NoSuchElementException& )
            {
                // No settings in our configuration manager. This means we are
                // a transient toolbar which has no persistent settings.
                m_bPersistent = sal_False;
                if ( pToolBar && pToolBarManager )
                {
                    pToolBar->SetOutStyle( SvtMiscOptions().GetToolboxStyle() );
                    pToolBar->EnableCustomize( sal_True );
                    ::Size aActSize( pToolBar->GetSizePixel() );
                    ::Size aSize( pToolBar->CalcWindowSizePixel() );
                    aSize.Width() = aActSize.Width();
                    pToolBar->SetOutputSizePixel( aSize );
                }
            }
        }
    }
}

// XEventListener
void SAL_CALL ToolBarWrapper::disposing( const ::com::sun::star::lang::EventObject& ) throw (::com::sun::star::uno::RuntimeException)
{
    // nothing todo
}

// XUpdatable
void SAL_CALL ToolBarWrapper::update() throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    ToolBarManager* pToolBarManager = static_cast< ToolBarManager *>( m_xToolBarManager.get() );
    if ( pToolBarManager )
        pToolBarManager->CheckAndUpdateImages();
}

// XUIElementSettings
void SAL_CALL ToolBarWrapper::updateSettings() throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_xToolBarManager.is() )
    {
        if ( m_xConfigSource.is() && m_bPersistent )
        {
            try
            {
                ToolBarManager* pToolBarManager = static_cast< ToolBarManager *>( m_xToolBarManager.get() );

                m_xConfigData = m_xConfigSource->getSettings( m_aResourceURL, sal_False );
                if ( m_xConfigData.is() )
                    pToolBarManager->FillToolbar( m_xConfigData );
            }
            catch ( const NoSuchElementException& )
            {
            }
        }
        else if ( !m_bPersistent )
        {
            // Transient toolbar: do nothing
        }
    }
}

void ToolBarWrapper::impl_fillNewData()
{
    // Transient toolbar => Fill toolbar with new data
    ToolBarManager* pToolBarManager = static_cast< ToolBarManager *>( m_xToolBarManager.get() );
    if ( pToolBarManager )
        pToolBarManager->FillToolbar( m_xConfigData );
}

// XUIElement interface
Reference< XInterface > SAL_CALL ToolBarWrapper::getRealInterface(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    if ( m_xToolBarManager.is() )
    {
        ToolBarManager* pToolBarManager = static_cast< ToolBarManager *>( m_xToolBarManager.get() );
        if ( pToolBarManager )
        {
            Window* pWindow = (Window *)pToolBarManager->GetToolBar();
            return Reference< XInterface >( VCLUnoHelper::GetInterface( pWindow ), UNO_QUERY );
        }
    }

    return Reference< XInterface >();
}

//XUIFunctionExecute
void SAL_CALL ToolBarWrapper::functionExecute(
    const ::rtl::OUString& aUIElementName,
    const ::rtl::OUString& aCommand )
throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    if ( m_xToolBarManager.is() )
    {
        ToolBarManager* pToolBarManager = static_cast< ToolBarManager *>( m_xToolBarManager.get() );
        if ( pToolBarManager )
            pToolBarManager->notifyRegisteredControllers( aUIElementName, aCommand );
    }
}

void SAL_CALL ToolBarWrapper::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const com::sun::star::uno::Any&  aValue ) throw( com::sun::star::uno::Exception )
{
    ResetableGuard aLock( m_aLock );
    sal_Bool bNoClose( m_bNoClose );
    aLock.unlock();

    UIConfigElementWrapperBase::setFastPropertyValue_NoBroadcast( nHandle, aValue );

    aLock.lock();

    sal_Bool bNewNoClose( m_bNoClose );
    if ( m_xToolBarManager.is() && !m_bDisposed && ( bNewNoClose != bNoClose ))
    {
        ToolBarManager* pToolBarManager = static_cast< ToolBarManager *>( m_xToolBarManager.get() );
        if ( pToolBarManager )
        {
            ToolBox* pToolBox = pToolBarManager->GetToolBar();
            if ( pToolBox )
            {
                if ( bNewNoClose )
                {
                    pToolBox->SetStyle( pToolBox->GetStyle() & ~WB_CLOSEABLE );
                    pToolBox->SetFloatStyle( pToolBox->GetFloatStyle() & ~WB_CLOSEABLE );
                }
                else
                {
                    pToolBox->SetStyle( pToolBox->GetStyle() | WB_CLOSEABLE );
                    pToolBox->SetFloatStyle( pToolBox->GetFloatStyle() | WB_CLOSEABLE );
                }
            }
        }
    }
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
