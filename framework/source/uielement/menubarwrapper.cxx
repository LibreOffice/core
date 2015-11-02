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
#include <framework/actiontriggerhelper.hxx>
#include <services.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XSystemDependentMenuPeer.hpp>
#include <com/sun/star/awt/XMenuBar.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/util/URLTransformer.hpp>

#include <comphelper/processfactory.hxx>
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

//  XInterface, XTypeProvider
DEFINE_XINTERFACE_11    (   MenuBarWrapper                                                    ,
                            UIConfigElementWrapperBase                                        ,
                            DIRECT_INTERFACE( css::lang::XTypeProvider          ),
                            DIRECT_INTERFACE( css::ui::XUIElement               ),
                            DIRECT_INTERFACE( css::ui::XUIElementSettings       ),
                            DIRECT_INTERFACE( css::beans::XMultiPropertySet     ),
                            DIRECT_INTERFACE( css::beans::XFastPropertySet      ),
                            DIRECT_INTERFACE( css::beans::XPropertySet          ),
                            DIRECT_INTERFACE( css::lang::XInitialization        ),
                            DIRECT_INTERFACE( css::lang::XComponent             ),
                            DIRECT_INTERFACE( css::util::XUpdatable             ),
                            DIRECT_INTERFACE( css::ui::XUIConfigurationListener ),
                            DERIVED_INTERFACE( css::container::XNameAccess, css::container::XElementAccess )
                        )

DEFINE_XTYPEPROVIDER_11 (   MenuBarWrapper                                  ,
                            css::lang::XTypeProvider           ,
                            css::ui::XUIElement                ,
                            css::ui::XUIElementSettings        ,
                            css::beans::XMultiPropertySet      ,
                            css::beans::XFastPropertySet       ,
                            css::beans::XPropertySet           ,
                            css::lang::XInitialization         ,
                            css::lang::XComponent              ,
                            css::util::XUpdatable              ,
                            css::ui::XUIConfigurationListener  ,
                            css::container::XNameAccess
                        )

MenuBarWrapper::MenuBarWrapper(
    const css::uno::Reference< css::uno::XComponentContext >& rxContext
    )
:    UIConfigElementWrapperBase( UIElementType::MENUBAR ),
     m_bRefreshPopupControllerCache( true ),
     m_xContext( rxContext )
{
}

MenuBarWrapper::~MenuBarWrapper()
{
}

void SAL_CALL MenuBarWrapper::dispose() throw (css::uno::RuntimeException, std::exception)
{
    Reference< XComponent > xThis( static_cast< OWeakObject* >(this), UNO_QUERY );

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
void SAL_CALL MenuBarWrapper::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException, std::exception )
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        throw DisposedException();

    if ( !m_bInitialized )
    {
        OUString aModuleIdentifier;
        UIConfigElementWrapperBase::initialize( aArguments );

        Reference< XFrame > xFrame( m_xWeakFrame );
        if ( xFrame.is() && m_xConfigSource.is() )
        {
            // Create VCL menubar which will be filled with settings data
            MenuBar*        pVCLMenuBar = 0;
            VCLXMenuBar*    pAwtMenuBar = 0;
            {
                SolarMutexGuard aSolarMutexGuard;
                pVCLMenuBar = new MenuBar();
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
                m_xConfigData = m_xConfigSource->getSettings( m_aResourceURL, sal_False );
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
            for ( sal_Int32 n = 0; n < aArguments.getLength(); n++ )
            {
                PropertyValue aPropValue;
                if ( aArguments[n] >>= aPropValue )
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

                MenuBarManager* pMenuBarManager = new MenuBarManager( m_xContext,
                                                                      xFrame,
                                                                      xTrans,
                                                                      xDispatchProvider,
                                                                      aModuleIdentifier,
                                                                      pVCLMenuBar,
                                                                      false,
                                                                      true );

                m_xMenuBarManager.set( static_cast< OWeakObject *>( pMenuBarManager ), UNO_QUERY );
            }

            // Initialize toolkit menu bar implementation to have awt::XMenuBar for data exchange.
            // Don't use this toolkit menu bar or one of its functions. It is only used as a data container!
            pAwtMenuBar = new VCLXMenuBar( pVCLMenuBar );
            m_xMenuBar.set( static_cast< OWeakObject *>( pAwtMenuBar ), UNO_QUERY );
        }
    }
}

// XUIElementSettings
void SAL_CALL MenuBarWrapper::updateSettings() throw ( RuntimeException, std::exception )
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_xMenuBarManager.is() )
    {
        if ( m_xConfigSource.is() && m_bPersistent )
        {
            try
            {
                MenuBarManager* pMenuBarManager = static_cast< MenuBarManager *>( m_xMenuBarManager.get() );

                m_xConfigData = m_xConfigSource->getSettings( m_aResourceURL, sal_False );
                if ( m_xConfigData.is() )
                    pMenuBarManager->SetItemContainer( m_xConfigData );
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
}
void MenuBarWrapper::impl_fillNewData()
{
    // Transient menubar => Fill menubar with new data
    MenuBarManager* pMenuBarManager = static_cast< MenuBarManager *>( m_xMenuBarManager.get() );

    if ( pMenuBarManager )
        pMenuBarManager->SetItemContainer( m_xConfigData );
}

void MenuBarWrapper::fillPopupControllerCache()
{
    if ( m_bRefreshPopupControllerCache )
    {
        MenuBarManager* pMenuBarManager = static_cast< MenuBarManager *>( m_xMenuBarManager.get() );
        if ( pMenuBarManager )
            pMenuBarManager->GetPopupController( m_aPopupControllerCache );
        if ( !m_aPopupControllerCache.empty() )
            m_bRefreshPopupControllerCache = false;
    }
}

// XElementAccess
Type SAL_CALL MenuBarWrapper::getElementType()
throw (css::uno::RuntimeException, std::exception)
{
    return cppu::UnoType<XDispatchProvider>::get();
}

sal_Bool SAL_CALL MenuBarWrapper::hasElements()
throw (css::uno::RuntimeException, std::exception)
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
throw ( container::NoSuchElementException,
        lang::WrappedTargetException,
        uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        throw DisposedException();

    fillPopupControllerCache();

    PopupControllerCache::const_iterator pIter = m_aPopupControllerCache.find( aName );
    if ( pIter != m_aPopupControllerCache.end() )
    {
        uno::Reference< frame::XDispatchProvider > xDispatchProvider;
        xDispatchProvider = pIter->second.m_xDispatchProvider;
        return uno::makeAny( xDispatchProvider );
    }
    else
        throw container::NoSuchElementException();
}

Sequence< OUString > SAL_CALL MenuBarWrapper::getElementNames()
throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        throw DisposedException();

    fillPopupControllerCache();

    Sequence< OUString > aSeq( m_aPopupControllerCache.size() );

    sal_Int32 i( 0 );
    PopupControllerCache::const_iterator pIter = m_aPopupControllerCache.begin();
    while ( pIter != m_aPopupControllerCache.end() )
    {
        aSeq[i++] = pIter->first;
        ++pIter;
    }

    return aSeq;
}

sal_Bool SAL_CALL MenuBarWrapper::hasByName(
    const OUString& aName )
throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        throw DisposedException();

    fillPopupControllerCache();

    PopupControllerCache::const_iterator pIter = m_aPopupControllerCache.find( aName );
    if ( pIter != m_aPopupControllerCache.end() )
        return sal_True;
    else
        return sal_False;
}

// XUIElement
Reference< XInterface > SAL_CALL MenuBarWrapper::getRealInterface() throw ( RuntimeException, std::exception )
{
    if ( m_bDisposed )
        throw DisposedException();

    return Reference< XInterface >( m_xMenuBarManager, UNO_QUERY );
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
