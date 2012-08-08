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

#include <uielement/addonstoolbarwrapper.hxx>
#include <threadhelp/resetableguard.hxx>
#include <framework/actiontriggerhelper.hxx>
#include <uielement/constitemcontainer.hxx>
#include <uielement/rootitemcontainer.hxx>
#include <uielement/addonstoolbarmanager.hxx>

#include <uielement/toolbar.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XSystemDependentMenuPeer.hpp>
#include <com/sun/star/awt/XMenuBar.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ui/UIElementType.hpp>

#include <toolkit/unohlp.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <comphelper/processfactory.hxx>

#include <svtools/miscopt.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <rtl/logfile.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::frame;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::awt;
using namespace ::com::sun::star::ui;

namespace framework
{

AddonsToolBarWrapper::AddonsToolBarWrapper( const Reference< XMultiServiceFactory >& xServiceManager ) :
    UIElementWrapperBase( UIElementType::TOOLBAR ),
    m_xServiceManager( xServiceManager )
{
}

AddonsToolBarWrapper::~AddonsToolBarWrapper()
{
}

// XComponent
void SAL_CALL AddonsToolBarWrapper::dispose() throw ( RuntimeException )
{
    Reference< XComponent > xThis( static_cast< OWeakObject* >(this), UNO_QUERY );

    com::sun::star::lang::EventObject aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    ResetableGuard aLock( m_aLock );

    if ( m_xToolBarManager.is() )
        m_xToolBarManager->dispose();
    m_xToolBarManager.clear();
    m_xToolBarWindow.clear();

    m_bDisposed = sal_True;
}

// XInitialization
void SAL_CALL AddonsToolBarWrapper::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException )
{
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    if ( !m_bInitialized )
    {
        UIElementWrapperBase::initialize( aArguments );

        for ( sal_Int32 n = 0; n < aArguments.getLength(); n++ )
        {
            PropertyValue aPropValue;
            if ( aArguments[n] >>= aPropValue )
            {
                if ( aPropValue.Name == "ConfigurationData" )
                    aPropValue.Value >>= m_aConfigData;
            }
        }

        Reference< XFrame > xFrame( m_xWeakFrame );
        if ( xFrame.is() && m_aConfigData.getLength() > 0 )
        {
            // Create VCL based toolbar which will be filled with settings data
            ToolBar* pToolBar = 0;
            AddonsToolBarManager* pToolBarManager = 0;
            {
                SolarMutexGuard aSolarMutexGuard;
                Window* pWindow = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
                if ( pWindow )
                {
                    sal_uLong nStyles = WB_LINESPACING | WB_BORDER | WB_SCROLL | WB_MOVEABLE | WB_3DLOOK | WB_DOCKABLE | WB_SIZEABLE | WB_CLOSEABLE;

                    pToolBar = new ToolBar( pWindow, nStyles );
                    m_xToolBarWindow = VCLUnoHelper::GetInterface( pToolBar );
                    pToolBarManager = new AddonsToolBarManager( m_xServiceManager, xFrame, m_aResourceURL, pToolBar );
                    pToolBar->SetToolBarManager( pToolBarManager );
                    m_xToolBarManager = Reference< XComponent >( static_cast< OWeakObject *>( pToolBarManager ), UNO_QUERY );
                }
            }

            try
            {
                if (( m_aConfigData.getLength() > 0 ) && pToolBar && pToolBarManager )
                {
                    // Fill toolbar with container contents
                    pToolBarManager->FillToolbar( m_aConfigData );
                    pToolBar->SetOutStyle( SvtMiscOptions().GetToolboxStyle() );
                    pToolBar->EnableCustomize( sal_True );
                    ::Size aActSize( pToolBar->GetSizePixel() );
                    ::Size aSize( pToolBar->CalcWindowSizePixel() );
                    aSize.Width() = aActSize.Width();
                    pToolBar->SetSizePixel( aSize );
                }
            }
            catch ( const NoSuchElementException& )
            {
            }
        }
    }
}

// XUIElement interface
Reference< XInterface > SAL_CALL AddonsToolBarWrapper::getRealInterface() throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    if ( m_xToolBarManager.is() )
    {
        AddonsToolBarManager* pToolBarManager = static_cast< AddonsToolBarManager *>( m_xToolBarManager.get() );
        if ( pToolBarManager )
        {
            Window* pWindow = (Window *)pToolBarManager->GetToolBar();
            return Reference< XInterface >( VCLUnoHelper::GetInterface( pWindow ), UNO_QUERY );
        }
    }

    return Reference< XInterface >();
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
