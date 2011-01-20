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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#include "uielement/buttontoolbarcontroller.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include "uielement/toolbar.hxx"

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include "com/sun/star/util/XMacroExpander.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#include <rtl/uri.hxx>
#include <osl/mutex.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/window.hxx>
#include <vcl/graph.hxx>
#include <vcl/bitmap.hxx>
#include <svtools/filter.hxx>
#include <svtools/miscopt.hxx>
#include <dispatch/uieventloghelper.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;

namespace framework
{

ButtonToolbarController::ButtonToolbarController(
    const uno::Reference< lang::XMultiServiceFactory >& rServiceManager,
    ToolBox*                                            pToolBar,
    const rtl::OUString&                                aCommand ) :
    cppu::OWeakObject(),
    m_bInitialized( sal_False ),
    m_bDisposed( sal_False ),
    m_aCommandURL( aCommand ),
    m_xServiceManager( rServiceManager ),
    m_pToolbar( pToolBar )
{
}

ButtonToolbarController::~ButtonToolbarController()
{
}

        // XInterface
uno::Any SAL_CALL ButtonToolbarController::queryInterface( const uno::Type& rType )
throw (::com::sun::star::uno::RuntimeException)
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
    const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    const rtl::OUString aFrameName( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
    const rtl::OUString aCommandURLName( RTL_CONSTASCII_USTRINGPARAM( "CommandURL" ));
    const rtl::OUString aServiceManagerName( RTL_CONSTASCII_USTRINGPARAM( "ServiceManager" ));

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
        m_bInitialized = sal_True;

        PropertyValue aPropValue;
        for ( int i = 0; i < aArguments.getLength(); i++ )
        {
            if ( aArguments[i] >>= aPropValue )
            {
                if ( aPropValue.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Frame" ) ))
                    m_xFrame.set(aPropValue.Value,UNO_QUERY);
                else if ( aPropValue.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "CommandURL" ) ))
                    aPropValue.Value >>= m_aCommandURL;
                else if ( aPropValue.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ServiceManager" ) ))
                    m_xServiceManager.set(aPropValue.Value,UNO_QUERY);
            }
        }
    }
}

// XComponent
void SAL_CALL ButtonToolbarController::dispose() throw (::com::sun::star::uno::RuntimeException)
{
    Reference< XComponent > xThis( static_cast< OWeakObject* >(this), UNO_QUERY );

    {
        SolarMutexGuard aSolarMutexGuard;
        if ( m_bDisposed )
            throw DisposedException();

        m_xServiceManager.clear();
        m_xURLTransformer.clear();
        m_xFrame.clear();
        m_pToolbar = 0;
        m_bDisposed = sal_True;
    }
}

void SAL_CALL ButtonToolbarController::addEventListener(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& )
throw (::com::sun::star::uno::RuntimeException)
{
    // do nothing
}

void SAL_CALL ButtonToolbarController::removeEventListener(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& )
throw (::com::sun::star::uno::RuntimeException)
{
    // do nothing
}

// XUpdatable
void SAL_CALL ButtonToolbarController::update()
throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarMutexGuard;
    if ( m_bDisposed )
        throw DisposedException();
}

// XEventListener
void SAL_CALL ButtonToolbarController::disposing(
    const com::sun::star::lang::EventObject& Source )
throw ( ::com::sun::star::uno::RuntimeException )
{
    uno::Reference< uno::XInterface > xSource( Source.Source );

    SolarMutexGuard aSolarMutexGuard;

    if ( m_bDisposed )
        return;

    uno::Reference< uno::XInterface > xIfac( m_xFrame, uno::UNO_QUERY );
    if ( xIfac == xSource )
        m_xFrame.clear();
}

void SAL_CALL ButtonToolbarController::statusChanged( const ::com::sun::star::frame::FeatureStateEvent& )
throw ( ::com::sun::star::uno::RuntimeException )
{
    // do nothing
    if ( m_bDisposed )
        throw DisposedException();
}

// XToolbarController
void SAL_CALL ButtonToolbarController::execute( sal_Int16 KeyModifier )
throw (::com::sun::star::uno::RuntimeException)
{
    uno::Reference< frame::XDispatch >      xDispatch;
    uno::Reference< frame::XFrame >         xFrame;
    uno::Reference< util::XURLTransformer > xURLTransformer;
    rtl::OUString                           aCommandURL;
    ::com::sun::star::util::URL             aTargetURL;

    {
        SolarMutexGuard aSolarMutexGuard;

        if ( m_bDisposed )
            throw DisposedException();

        if ( m_bInitialized &&
             m_xFrame.is() &&
             m_xServiceManager.is() &&
             m_aCommandURL.getLength() )
        {
            if ( !m_xURLTransformer.is() )
            {
                m_xURLTransformer = uno::Reference< util::XURLTransformer >(
                    m_xServiceManager->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                    uno::UNO_QUERY_THROW );
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
        xDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );
    }

    if ( xDispatch.is() )
    {
        try
        {
            Sequence<PropertyValue>   aArgs( 1 );

            // Provide key modifier information to dispatch function
            aArgs[0].Name   = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "KeyModifier" ));
            aArgs[0].Value  <<= KeyModifier;

            if(::comphelper::UiEventsLogger::isEnabled()) //#i88653#
                UiEventLogHelper(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ButtonToolbarController"))).log(m_xServiceManager, m_xFrame, aTargetURL, aArgs);
            xDispatch->dispatch( aTargetURL, aArgs );
        }
        catch ( DisposedException& )
        {
        }
    }
}

void SAL_CALL ButtonToolbarController::click()
throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarMutexGuard;

    if ( m_bDisposed )
        throw DisposedException();

    sal_Int16   nKeyModifier( (sal_Int16)m_pToolbar->GetModifier() );
    execute( nKeyModifier );
}

void SAL_CALL ButtonToolbarController::doubleClick()
throw (::com::sun::star::uno::RuntimeException)
{
    // do nothing
    if ( m_bDisposed )
        throw DisposedException();
}

uno::Reference< awt::XWindow > SAL_CALL ButtonToolbarController::createPopupWindow()
throw (::com::sun::star::uno::RuntimeException)
{
    if ( m_bDisposed )
        throw DisposedException();

    return uno::Reference< awt::XWindow >();
}

uno::Reference< awt::XWindow > SAL_CALL ButtonToolbarController::createItemWindow(
    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& )
throw (::com::sun::star::uno::RuntimeException)
{
    if ( m_bDisposed )
        throw DisposedException();

    return uno::Reference< awt::XWindow >();
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
