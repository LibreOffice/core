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

#include "uielement/buttontoolbarcontroller.hxx"

#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <rtl/uri.hxx>
#include <osl/mutex.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/window.hxx>
#include <vcl/graph.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/toolbox.hxx>
#include <svtools/miscopt.hxx>

using namespace ::com::sun::star;
using namespace css::awt;
using namespace css::uno;
using namespace css::beans;
using namespace css::lang;
using namespace css::frame;
using namespace css::util;

namespace framework
{

ButtonToolbarController::ButtonToolbarController(
    const uno::Reference< uno::XComponentContext >& rxContext,
    ToolBox*                                            pToolBar,
    const OUString&                                aCommand ) :
    cppu::OWeakObject(),
    m_bInitialized( false ),
    m_bDisposed( false ),
    m_aCommandURL( aCommand ),
    m_xContext( rxContext ),
    m_pToolbar( pToolBar )
{
}

ButtonToolbarController::~ButtonToolbarController()
{
}

        // XInterface
uno::Any SAL_CALL ButtonToolbarController::queryInterface( const uno::Type& rType )
throw (css::uno::RuntimeException, std::exception)
{
    Any a = ::cppu::queryInterface(
                rType ,
                static_cast< frame::XStatusListener* >( this ),
                static_cast< frame::XToolbarController* >( this ),
                static_cast< lang::XInitialization* >( this ),
                static_cast< lang::XComponent* >( this ),
                static_cast< util::XUpdatable* >( this ));

    if ( a.hasValue() )
        return a;

    return cppu::OWeakObject::queryInterface( rType );
}

void SAL_CALL ButtonToolbarController::acquire() throw ()
{
    cppu::OWeakObject::acquire();
}

void SAL_CALL ButtonToolbarController::release() throw ()
{
    cppu::OWeakObject::release();
}

// XInitialization
void SAL_CALL ButtonToolbarController::initialize(
    const css::uno::Sequence< css::uno::Any >& aArguments )
throw (css::uno::Exception, css::uno::RuntimeException, std::exception)
{
    bool bInitialized( true );

    {
        SolarMutexGuard aSolarMutexGuard;

        if ( m_bDisposed )
            throw DisposedException();

        bInitialized = m_bInitialized;
    }

    if ( !bInitialized )
    {
        SolarMutexGuard aSolarMutexGuard;
        m_bInitialized = true;

        PropertyValue aPropValue;
        for ( int i = 0; i < aArguments.getLength(); i++ )
        {
            if ( aArguments[i] >>= aPropValue )
            {
                if ( aPropValue.Name == "Frame" )
                    m_xFrame.set(aPropValue.Value,UNO_QUERY);
                else if ( aPropValue.Name == "CommandURL" )
                    aPropValue.Value >>= m_aCommandURL;
                else if ( aPropValue.Name == "ServiceManager" )
                {
                    Reference<XMultiServiceFactory> xServiceManager(aPropValue.Value,UNO_QUERY);
                    m_xContext = comphelper::getComponentContext(xServiceManager);
                }
            }
        }
    }
}

// XComponent
void SAL_CALL ButtonToolbarController::dispose() throw (css::uno::RuntimeException, std::exception)
{
    Reference< XComponent > xThis( static_cast< OWeakObject* >(this), UNO_QUERY );

    {
        SolarMutexGuard aSolarMutexGuard;
        if ( m_bDisposed )
            throw DisposedException();

        m_xContext.clear();
        m_xURLTransformer.clear();
        m_xFrame.clear();
        m_pToolbar.clear();
        m_bDisposed = true;
    }
}

void SAL_CALL ButtonToolbarController::addEventListener(
    const css::uno::Reference< css::lang::XEventListener >& )
throw (css::uno::RuntimeException, std::exception)
{
    // do nothing
}

void SAL_CALL ButtonToolbarController::removeEventListener(
    const css::uno::Reference< css::lang::XEventListener >& )
throw (css::uno::RuntimeException, std::exception)
{
    // do nothing
}

// XUpdatable
void SAL_CALL ButtonToolbarController::update()
throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarMutexGuard;
    if ( m_bDisposed )
        throw DisposedException();
}

// XEventListener
void SAL_CALL ButtonToolbarController::disposing(
    const css::lang::EventObject& Source )
throw ( css::uno::RuntimeException, std::exception )
{
    uno::Reference< uno::XInterface > xSource( Source.Source );

    SolarMutexGuard aSolarMutexGuard;

    if ( m_bDisposed )
        return;

    uno::Reference< uno::XInterface > xIfac( m_xFrame, uno::UNO_QUERY );
    if ( xIfac == xSource )
        m_xFrame.clear();
}

void SAL_CALL ButtonToolbarController::statusChanged( const css::frame::FeatureStateEvent& )
throw ( css::uno::RuntimeException, std::exception )
{
    // do nothing
    if ( m_bDisposed )
        throw DisposedException();
}

// XToolbarController
void SAL_CALL ButtonToolbarController::execute( sal_Int16 KeyModifier )
throw (css::uno::RuntimeException, std::exception)
{
    uno::Reference< frame::XDispatch >      xDispatch;
    uno::Reference< frame::XFrame >         xFrame;
    uno::Reference< util::XURLTransformer > xURLTransformer;
    OUString                           aCommandURL;
    css::util::URL             aTargetURL;

    {
        SolarMutexGuard aSolarMutexGuard;

        if ( m_bDisposed )
            throw DisposedException();

        if ( m_bInitialized &&
             m_xFrame.is() &&
             m_xContext.is() &&
             !m_aCommandURL.isEmpty() )
        {
            if ( !m_xURLTransformer.is() )
            {
                m_xURLTransformer = util::URLTransformer::create( m_xContext );
            }

            xFrame          = m_xFrame;
            aCommandURL     = m_aCommandURL;
            xURLTransformer = m_xURLTransformer;
        }
    }

    uno::Reference< frame::XDispatchProvider > xDispatchProvider( xFrame, uno::UNO_QUERY );
    if ( xDispatchProvider.is() )
    {
        aTargetURL.Complete = aCommandURL;
        xURLTransformer->parseStrict( aTargetURL );
        xDispatch = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );
    }

    if ( xDispatch.is() )
    {
        try
        {
            Sequence<PropertyValue>   aArgs( 1 );

            // Provide key modifier information to dispatch function
            aArgs[0].Name   = "KeyModifier";
            aArgs[0].Value  <<= KeyModifier;

            xDispatch->dispatch( aTargetURL, aArgs );
        }
        catch ( const DisposedException& )
        {
        }
    }
}

void SAL_CALL ButtonToolbarController::click()
throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarMutexGuard;

    if ( m_bDisposed )
        throw DisposedException();

    sal_Int16   nKeyModifier( (sal_Int16)m_pToolbar->GetModifier() );
    execute( nKeyModifier );
}

void SAL_CALL ButtonToolbarController::doubleClick()
throw (css::uno::RuntimeException, std::exception)
{
    // do nothing
    if ( m_bDisposed )
        throw DisposedException();
}

uno::Reference< awt::XWindow > SAL_CALL ButtonToolbarController::createPopupWindow()
throw (css::uno::RuntimeException, std::exception)
{
    if ( m_bDisposed )
        throw DisposedException();

    return uno::Reference< awt::XWindow >();
}

uno::Reference< awt::XWindow > SAL_CALL ButtonToolbarController::createItemWindow(
    const css::uno::Reference< css::awt::XWindow >& )
throw (css::uno::RuntimeException, std::exception)
{
    if ( m_bDisposed )
        throw DisposedException();

    return uno::Reference< awt::XWindow >();
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
