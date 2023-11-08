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

#include <uielement/menubarwrapper.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/util/URLTransformer.hpp>

#include <comphelper/sequence.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <utility>
#include <vcl/svapp.hxx>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::frame;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::awt;
using namespace com::sun::star::util;
using namespace ::com::sun::star::ui;

namespace framework
{

MenuBarWrapper::MenuBarWrapper(
    css::uno::Reference< css::uno::XComponentContext > xContext
    )
:    MenuBarWrapper_Base( UIElementType::MENUBAR ),
     m_bRefreshPopupControllerCache( true ),
     m_xContext(std::move( xContext ))
{
}

MenuBarWrapper::~MenuBarWrapper()
{
}

void SAL_CALL MenuBarWrapper::dispose()
{
    Reference< XComponent > xThis(this);

    css::lang::EventObject aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    SolarMutexGuard g;

    m_xMenuBarManager->dispose();
    m_xMenuBarManager.clear();
    m_xConfigSource.clear();
    m_xConfigData.clear();

    m_xMenuBar.clear();
    m_bDisposed = true;
}

// XInitialization
void SAL_CALL MenuBarWrapper::initialize( const Sequence< Any >& aArguments )
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_bInitialized )
        return;

    OUString aModuleIdentifier;
    UIConfigElementWrapperBase::initialize( aArguments );

    Reference< XFrame > xFrame( m_xWeakFrame );
    if ( !(xFrame.is() && m_xConfigSource.is()) )
        return;

    // Create VCL menubar which will be filled with settings data
    VclPtr<MenuBar> pVCLMenuBar;
    {
        SolarMutexGuard aSolarMutexGuard;
        pVCLMenuBar = VclPtr<MenuBar>::Create();
    }

    Reference< XModuleManager2 > xModuleManager = ModuleManager::create( m_xContext );

    try
    {
        aModuleIdentifier = xModuleManager->identify( xFrame );
    }
    catch( const Exception& )
    {
    }

    Reference< XURLTransformer > xTrans;
    try
    {
        xTrans.set( URLTransformer::create(m_xContext) );
        m_xConfigData = m_xConfigSource->getSettings( m_aResourceURL, false );
        if ( m_xConfigData.is() )
        {
            // Fill menubar with container contents
            sal_uInt16 nId = 1;
            MenuBarManager::FillMenuWithConfiguration( nId, pVCLMenuBar, aModuleIdentifier, m_xConfigData, xTrans );
        }
    }
    catch ( const NoSuchElementException& )
    {
    }

    bool bMenuOnly( false );
    for ( const Any& rArg : aArguments )
    {
        PropertyValue aPropValue;
        if ( rArg >>= aPropValue )
        {
            if ( aPropValue.Name == "MenuOnly" )
                aPropValue.Value >>= bMenuOnly;
        }
    }

    if ( !bMenuOnly )
    {
        // Initialize menubar manager with our vcl menu bar. There are some situations where we only want to get the menu without any
        // interaction which is done by the menu bar manager. This must be requested by a special property called "MenuOnly". Be careful
        // a menu bar created with this property is not fully supported. It must be attached to a real menu bar manager to have full
        // support. This feature is currently used for "Inplace editing"!
        Reference< XDispatchProvider > xDispatchProvider;

        m_xMenuBarManager = new MenuBarManager( m_xContext,
                                                              xFrame,
                                                              xTrans,
                                                              xDispatchProvider,
                                                              aModuleIdentifier,
                                                              pVCLMenuBar,
                                                              false );
    }

    // Initialize toolkit menu bar implementation to have awt::XMenuBar for data exchange.
    // Don't use this toolkit menu bar or one of its functions. It is only used as a data container!
    m_xMenuBar = new VCLXMenuBar( pVCLMenuBar );
}

// XUIElementSettings
void SAL_CALL MenuBarWrapper::updateSettings()
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        throw DisposedException();

    if ( !m_xMenuBarManager.is() )
        return;

    if ( m_xConfigSource.is() && m_bPersistent )
    {
        try
        {
            m_xConfigData = m_xConfigSource->getSettings( m_aResourceURL, false );
            if ( m_xConfigData.is() )
                m_xMenuBarManager->SetItemContainer( m_xConfigData );
        }
        catch ( const NoSuchElementException& )
        {
        }
    }
    else if ( !m_bPersistent )
    {
        // Transient menubar: do nothing
    }
}
void MenuBarWrapper::impl_fillNewData()
{
    // Transient menubar => Fill menubar with new data
    if ( m_xMenuBarManager )
        m_xMenuBarManager->SetItemContainer( m_xConfigData );
}

void MenuBarWrapper::fillPopupControllerCache()
{
    if ( m_bRefreshPopupControllerCache )
    {
        if ( m_xMenuBarManager )
            m_xMenuBarManager->GetPopupController( m_aPopupControllerCache );
        if ( !m_aPopupControllerCache.empty() )
            m_bRefreshPopupControllerCache = false;
    }
}

// XElementAccess
Type SAL_CALL MenuBarWrapper::getElementType()
{
    return cppu::UnoType<XDispatchProvider>::get();
}

sal_Bool SAL_CALL MenuBarWrapper::hasElements()
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        throw DisposedException();

    fillPopupControllerCache();
    return ( !m_aPopupControllerCache.empty() );
}

// XNameAccess
Any SAL_CALL MenuBarWrapper::getByName(
    const OUString& aName )
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        throw DisposedException();

    fillPopupControllerCache();

    PopupControllerCache::const_iterator pIter = m_aPopupControllerCache.find( aName );
    if ( pIter == m_aPopupControllerCache.end() )
        throw container::NoSuchElementException();

    uno::Reference< frame::XDispatchProvider > xDispatchProvider = pIter->second.m_xDispatchProvider;
    return uno::Any( xDispatchProvider );
}

Sequence< OUString > SAL_CALL MenuBarWrapper::getElementNames()
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        throw DisposedException();

    fillPopupControllerCache();

    return comphelper::mapKeysToSequence( m_aPopupControllerCache );
}

sal_Bool SAL_CALL MenuBarWrapper::hasByName(
    const OUString& aName )
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        throw DisposedException();

    fillPopupControllerCache();

    PopupControllerCache::const_iterator pIter = m_aPopupControllerCache.find( aName );
    if ( pIter != m_aPopupControllerCache.end() )
        return true;
    else
        return false;
}

// XUIElement
Reference< XInterface > SAL_CALL MenuBarWrapper::getRealInterface()
{
    if ( m_bDisposed )
        throw DisposedException();

    return Reference< XInterface >( static_cast<cppu::OWeakObject*>(m_xMenuBarManager.get()), UNO_QUERY );
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
