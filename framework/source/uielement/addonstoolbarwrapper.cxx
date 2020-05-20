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
#include <uielement/addonstoolbarmanager.hxx>

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/ui/UIElementType.hpp>

#include <toolkit/helper/vclunohelper.hxx>

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
void SAL_CALL AddonsToolBarWrapper::dispose()
{
    Reference< XComponent > xThis( static_cast< OWeakObject* >(this), UNO_QUERY );

    css::lang::EventObject aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    SolarMutexGuard g;

    if ( m_xToolBarManager.is() )
        m_xToolBarManager->dispose();
    m_xToolBarManager.clear();

    m_bDisposed = true;
}

// XInitialization
void SAL_CALL AddonsToolBarWrapper::initialize( const Sequence< Any >& aArguments )
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_bInitialized )
        return;

    UIElementWrapperBase::initialize( aArguments );

    for ( const Any& rArg : aArguments )
    {
        PropertyValue aPropValue;
        if ( rArg >>= aPropValue )
        {
            if ( aPropValue.Name == "ConfigurationData" )
                aPropValue.Value >>= m_aConfigData;
        }
    }

    Reference< XFrame > xFrame( m_xWeakFrame );
    if ( !(xFrame.is() && m_aConfigData.hasElements()) )
        return;

    // Create VCL based toolbar which will be filled with settings data
    VclPtr<ToolBox> pToolBar;
    AddonsToolBarManager* pToolBarManager = nullptr;
    {
        SolarMutexGuard aSolarMutexGuard;
        VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
        if ( pWindow )
        {
            sal_uLong nStyles = WB_BORDER | WB_SCROLL | WB_MOVEABLE | WB_3DLOOK | WB_DOCKABLE | WB_SIZEABLE | WB_CLOSEABLE;

            pToolBar = VclPtr<ToolBox>::Create( pWindow, nStyles );
            pToolBar->SetLineSpacing(true);
            pToolBarManager = new AddonsToolBarManager( m_xContext, xFrame, m_aResourceURL, pToolBar );
            m_xToolBarManager.set( static_cast< OWeakObject *>( pToolBarManager ), UNO_QUERY );
        }
    }

    try
    {
        if ( m_aConfigData.hasElements() && pToolBar && pToolBarManager )
        {
            // Fill toolbar with container contents
            pToolBarManager->FillToolbar( m_aConfigData );
            pToolBar->SetOutStyle( SvtMiscOptions().GetToolboxStyle() );
            pToolBar->EnableCustomize();
            ::Size aActSize( pToolBar->GetSizePixel() );
            ::Size aSize( pToolBar->CalcWindowSizePixel() );
            aSize.setWidth( aActSize.Width() );
            pToolBar->SetSizePixel( aSize );
        }
    }
    catch ( const NoSuchElementException& )
    {
    }
}

// XUIElement interface
Reference< XInterface > SAL_CALL AddonsToolBarWrapper::getRealInterface()
{
    SolarMutexGuard g;

    if ( m_xToolBarManager.is() )
    {
        AddonsToolBarManager* pToolBarManager = static_cast< AddonsToolBarManager *>( m_xToolBarManager.get() );
        if ( pToolBarManager )
        {
            vcl::Window* pWindow = pToolBarManager->GetToolBar();
            return Reference< XInterface >( VCLUnoHelper::GetInterface( pWindow ), UNO_QUERY );
        }
    }

    return Reference< XInterface >();
}

// allow late population of images for add-on toolbars
void AddonsToolBarWrapper::populateImages()
{
    SolarMutexGuard g;

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
