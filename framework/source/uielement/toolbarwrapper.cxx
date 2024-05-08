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

#include <uielement/toolbarwrapper.hxx>
#include <uielement/toolbarmanager.hxx>

#include <com/sun/star/ui/ContextChangeEventMultiplexer.hpp>
#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

#include <toolkit/helper/vclunohelper.hxx>

#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/weldutils.hxx>
#include <vcl/wintypes.hxx>

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

ToolBarWrapper::ToolBarWrapper( const Reference< XComponentContext >& rxContext ) :
    ImplInheritanceHelper( UIElementType::TOOLBAR ),
    m_xContext( rxContext )
{
}

ToolBarWrapper::~ToolBarWrapper()
{
    m_xWeldedToolbar.reset(nullptr);
    m_xTopLevel.reset(nullptr);
    m_xBuilder.reset(nullptr);
}

// XComponent
void SAL_CALL ToolBarWrapper::dispose()
{
    Reference< XComponent > xThis(this);

    {
        SolarMutexGuard g;
        if ( m_bDisposed )
            return;
    }

    css::lang::EventObject aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    SolarMutexGuard g;

    auto xMultiplexer( ContextChangeEventMultiplexer::get( m_xContext ) );
    xMultiplexer->removeAllContextChangeEventListeners( this );

    Reference< XComponent > xComponent( m_xSubElement, UNO_QUERY );
    if ( xComponent.is() )
        xComponent->removeEventListener( Reference< XUIConfigurationListener >( this ));
    m_xSubElement.clear();

    if ( m_xToolBarManager.is() )
        m_xToolBarManager->dispose();
    m_xToolBarManager.clear();
    m_xConfigSource.clear();
    m_xConfigData.clear();

    m_bDisposed = true;
}

// XInitialization
void SAL_CALL ToolBarWrapper::initialize( const Sequence< Any >& aArguments )
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_bInitialized )
        return;

    UIConfigElementWrapperBase::initialize( aArguments );

    bool bPopupMode( false );
    Reference< XWindow > xParentWindow;
    for ( Any const & arg : aArguments )
    {
        PropertyValue aPropValue;
        if ( arg >>= aPropValue )
        {
            if ( aPropValue.Name == "PopupMode" )
                aPropValue.Value >>= bPopupMode;
            else if ( aPropValue.Name == "ParentWindow" )
                xParentWindow.set( aPropValue.Value, UNO_QUERY );
        }
    }

    Reference< XFrame > xFrame( m_xWeakFrame );
    if ( !(xFrame.is() && m_xConfigSource.is()) )
        return;

    OUString aContextPart;
    if ( m_aResourceURL.startsWith( "private:resource/toolbar/singlemode", &aContextPart ) && aContextPart.isEmpty() )
    {
        auto xMultiplexer( ContextChangeEventMultiplexer::get( m_xContext ) );
        try
        {
            xMultiplexer->addContextChangeEventListener( this, xFrame->getController() );
        }
        catch( const Exception& )
        {
        }
        // Avoid flickering on context change
        bPopupMode = true;
    }

    // Create VCL based toolbar which will be filled with settings data
    VclPtr<ToolBox> pToolBar;
    rtl::Reference<ToolBarManager> pToolBarManager;
    if ( aContextPart.isEmpty() )
    {
        SolarMutexGuard aSolarMutexGuard;
        if ( !xParentWindow.is() )
            xParentWindow.set( xFrame->getContainerWindow() );
        VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( xParentWindow );
        if ( pWindow )
        {
            WinBits nStyles = WB_BORDER | WB_SCROLL | WB_MOVEABLE | WB_3DLOOK | WB_DOCKABLE | WB_SIZEABLE | WB_CLOSEABLE;

            pToolBar = VclPtr<ToolBox>::Create( pWindow, nStyles );
            pToolBar->SetLineSpacing(true);
            pToolBarManager = new ToolBarManager( m_xContext, xFrame, m_aResourceURL, pToolBar );
            m_xToolBarManager = pToolBarManager;
            pToolBar->WillUsePopupMode( bPopupMode );
        }
        else if (weld::TransportAsXWindow* pTunnel = dynamic_cast<weld::TransportAsXWindow*>(xParentWindow.get()))
        {
            m_xBuilder = Application::CreateBuilder(pTunnel->getWidget(), u"svt/ui/managedtoolbar.ui"_ustr);
            m_xTopLevel = m_xBuilder->weld_container(u"toolbarcontainer"_ustr);
            m_xWeldedToolbar = m_xBuilder->weld_toolbar(u"managedtoolbar"_ustr);
            if ( m_xWeldedToolbar )
            {
                pToolBarManager = new ToolBarManager( m_xContext, xFrame, m_aResourceURL, m_xWeldedToolbar.get(), m_xBuilder.get() );
                m_xToolBarManager = pToolBarManager;
            }
        }
    }

    try
    {
        m_xConfigData = m_xConfigSource->getSettings( m_aResourceURL, false );
        if ( m_xConfigData.is() && (pToolBar || m_xWeldedToolbar) && pToolBarManager )
        {
            // Fill toolbar with container contents
            impl_fillNewData();
            if (pToolBar)
            {
                pToolBar->EnableCustomize();
                ::Size aActSize( pToolBar->GetSizePixel() );
                ::Size aSize( pToolBar->CalcWindowSizePixel() );
                aSize.setWidth( aActSize.Width() );
                pToolBar->SetOutputSizePixel( aSize );
            }
        }
    }
    catch ( const NoSuchElementException& )
    {
        // No settings in our configuration manager. This means we are
        // a transient toolbar which has no persistent settings.
        m_bPersistent = false;
        if ( pToolBar && pToolBarManager )
        {
            pToolBar->EnableCustomize();
            ::Size aActSize( pToolBar->GetSizePixel() );
            ::Size aSize( pToolBar->CalcWindowSizePixel() );
            aSize.setWidth( aActSize.Width() );
            pToolBar->SetOutputSizePixel( aSize );
        }
    }
}

// XEventListener
void SAL_CALL ToolBarWrapper::disposing( const css::lang::EventObject& aEvent )
{
    if ( aEvent.Source == m_xSubElement )
        m_xSubElement.clear();
}

// XUpdatable
void SAL_CALL ToolBarWrapper::update()
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_xToolBarManager )
        m_xToolBarManager->CheckAndUpdateImages();
}

// XUIElementSettings
void SAL_CALL ToolBarWrapper::updateSettings()
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_xConfigSource.is() && m_bPersistent )
    {
        try
        {
            m_xConfigData = m_xConfigSource->getSettings( m_aResourceURL, false );
            if ( m_xConfigData.is() )
                impl_fillNewData();
        }
        catch ( const NoSuchElementException& )
        {
        }

        auto pContainer( m_aListenerContainer.getContainer( cppu::UnoType< XEventListener >::get() ) );
        if ( pContainer )
            pContainer->forEach< XUIElementSettings >([]( const Reference<XUIElementSettings>& xListener ){ xListener->updateSettings(); });
    }
    else if ( !m_bPersistent )
    {
        // Transient toolbar: do nothing
    }
}

void ToolBarWrapper::impl_fillNewData()
{
    if ( m_xToolBarManager )
    {
        Reference< XUIElementSettings > xUIElementSettings( m_xSubElement, UNO_QUERY );
        Reference< XIndexAccess > xContextData = xUIElementSettings.is() ? xUIElementSettings->getSettings( false ) : nullptr;
        OUString aContextToolbar = xContextData.is() ? m_xSubElement->getResourceURL() : OUString();
        m_xToolBarManager->FillToolbar( m_xConfigData, xContextData, aContextToolbar );
    }
}

//XContextChangeEventListener
void SAL_CALL ToolBarWrapper::notifyContextChangeEvent( const ContextChangeEventObject& aEvent )
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        throw DisposedException();

    if ( aEvent.ContextName.isEmpty() || aEvent.ContextName == "default" )
        return;

    const OUString aContextToolbar( m_aResourceURL + "-" + aEvent.ContextName.toAsciiLowerCase() );
    if ( m_xSubElement.is() && m_xSubElement->getResourceURL() == aContextToolbar )
        return;

    Reference< XComponent > xComponent( m_xSubElement, UNO_QUERY );
    if ( xComponent.is() )
        xComponent->removeEventListener( Reference< XUIConfigurationListener >( this ));
    m_xSubElement.clear();

    Reference< XLayoutManager > xLayoutManager;
    Reference< XPropertySet > xPropSet( m_xWeakFrame.get(), UNO_QUERY );
    if ( xPropSet.is() )
        xPropSet->getPropertyValue(u"LayoutManager"_ustr) >>= xLayoutManager;
    if ( !xLayoutManager.is() )
        return;

    xLayoutManager->createElement( aContextToolbar );
    m_xSubElement.set( xLayoutManager->getElement( aContextToolbar ) );
    xComponent.set( m_xSubElement, UNO_QUERY );
    if ( xComponent.is() )
        xComponent->addEventListener( Reference< XUIConfigurationListener >( this ));

    if ( m_xConfigData.is() )
    {
        xLayoutManager->lock();
        impl_fillNewData();
        xLayoutManager->unlock();
    }
}

// XUIElement interface
Reference< XInterface > SAL_CALL ToolBarWrapper::getRealInterface(  )
{
    SolarMutexGuard g;

    if ( m_xToolBarManager )
    {
        vcl::Window* pWindow = m_xToolBarManager->GetToolBar();
        return Reference< XInterface >( VCLUnoHelper::GetInterface( pWindow ), UNO_QUERY );
    }

    return Reference< XInterface >();
}

//XUIFunctionExecute
void SAL_CALL ToolBarWrapper::functionExecute(
    const OUString& aUIElementName,
    const OUString& aCommand )
{
    SolarMutexGuard g;

    if ( m_xToolBarManager )
        m_xToolBarManager->notifyRegisteredControllers( aUIElementName, aCommand );
}

void SAL_CALL ToolBarWrapper::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const css::uno::Any&  aValue )
{
    SolarMutexResettableGuard aLock;
    bool bNoClose( m_bNoClose );
    aLock.clear();

    UIConfigElementWrapperBase::setFastPropertyValue_NoBroadcast( nHandle, aValue );

    aLock.reset();

    bool bNewNoClose( m_bNoClose );
    if ( !(m_xToolBarManager.is() && !m_bDisposed && ( bNewNoClose != bNoClose )))
        return;

    if ( !m_xToolBarManager )
        return;

    ToolBox* pToolBox = m_xToolBarManager->GetToolBar();
    if ( !pToolBox )
        return;

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

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
