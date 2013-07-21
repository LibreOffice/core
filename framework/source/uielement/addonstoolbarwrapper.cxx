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

#include <uielement/addonstoolbarwrapper.hxx>
#include <threadhelp/resetableguard.hxx>
#include <framework/actiontriggerhelper.hxx>
#include <uielement/constitemcontainer.hxx>
#include <uielement/rootitemcontainer.hxx>
#include <uielement/addonstoolbarmanager.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XSystemDependentMenuPeer.hpp>
#include <com/sun/star/awt/XMenuBar.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ui/UIElementType.hpp>

#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <comphelper/processfactory.hxx>

#include <svtools/miscopt.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::frame;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::awt;
using namespace ::com::sun::star::ui;

namespace framework
{

AddonsToolBarWrapper::AddonsToolBarWrapper( const Reference< XComponentContext >& xContext ) :
    UIElementWrapperBase( UIElementType::TOOLBAR ),
    m_xContext( xContext ),
    m_bCreatedImages( false )
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
            ToolBox* pToolBar = 0;
            AddonsToolBarManager* pToolBarManager = 0;
            {
                SolarMutexGuard aSolarMutexGuard;
                Window* pWindow = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
                if ( pWindow )
                {
                    sal_uLong nStyles = WB_LINESPACING | WB_BORDER | WB_SCROLL | WB_MOVEABLE | WB_3DLOOK | WB_DOCKABLE | WB_SIZEABLE | WB_CLOSEABLE;

                    pToolBar = new ToolBox( pWindow, nStyles );
                    pToolBarManager = new AddonsToolBarManager( m_xContext, xFrame, m_aResourceURL, pToolBar );
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

// allow late population of images for add-on toolbars
void AddonsToolBarWrapper::populateImages()
{
    ResetableGuard aLock( m_aLock );

    if (m_bCreatedImages)
        return;

    if ( m_xToolBarManager.is() )
    {
        AddonsToolBarManager* pToolBarManager = static_cast< AddonsToolBarManager *>( m_xToolBarManager.get() );
        if (pToolBarManager)
        {
            pToolBarManager->RefreshImages();
            m_bCreatedImages = true;
        }
    }
}


} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
