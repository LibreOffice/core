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

#include <svtools/statusbarcontroller.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/ui/XStatusbarItem.hpp>
#include <cppuhelper/queryinterface.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/status.hxx>
#include <svtools/imgdef.hxx>
#include <svtools/miscopt.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/interfacecontainer.hxx>

using namespace ::cppu;
using namespace css::awt;
using namespace css::uno;
using namespace css::util;
using namespace css::beans;
using namespace css::lang;
using namespace css::frame;

namespace svt
{

StatusbarController::StatusbarController(
    const Reference< XComponentContext >& rxContext,
    const Reference< XFrame >& xFrame,
    const OUString& aCommandURL,
    unsigned short nID ) :
    OWeakObject()
    ,   m_bInitialized( false )
    ,   m_bDisposed( false )
    ,   m_nID( nID )
    ,   m_xFrame( xFrame )
    ,   m_xContext( rxContext )
    ,   m_aCommandURL( aCommandURL )
    ,   m_aListenerContainer( m_aMutex )
{
}

StatusbarController::StatusbarController() :
    OWeakObject()
    ,   m_bInitialized( false )
    ,   m_bDisposed( false )
    ,   m_nID( 0 )
    ,   m_aListenerContainer( m_aMutex )
{
}

StatusbarController::~StatusbarController()
{
}

Reference< XFrame > StatusbarController::getFrameInterface() const
{
    SolarMutexGuard aSolarMutexGuard;
    return m_xFrame;
}

Reference< XURLTransformer > StatusbarController::getURLTransformer() const
{
    SolarMutexGuard aSolarMutexGuard;
    if ( !m_xURLTransformer.is() && m_xContext.is() )
    {
        m_xURLTransformer = css::util::URLTransformer::create( m_xContext );
    }

    return m_xURLTransformer;
}

// XInterface
Any SAL_CALL StatusbarController::queryInterface( const Type& rType )
{
    Any a = ::cppu::queryInterface(
                rType ,
                static_cast< XStatusbarController* >( this ),
                static_cast< XStatusListener* >( this ),
                static_cast< XEventListener* >( this ),
                static_cast< XInitialization* >( this ),
                static_cast< XComponent* >( this ),
                static_cast< XUpdatable* >( this ));

    if ( a.hasValue() )
        return a;

    return OWeakObject::queryInterface( rType );
}

void SAL_CALL StatusbarController::acquire() throw ()
{
    OWeakObject::acquire();
}

void SAL_CALL StatusbarController::release() throw ()
{
    OWeakObject::release();
}

void SAL_CALL StatusbarController::initialize( const Sequence< Any >& aArguments )
{
    bool bInitialized( true );

    {
        SolarMutexGuard aSolarMutexGuard;

        if ( m_bDisposed )
            throw DisposedException();

        bInitialized = m_bInitialized;
    }

    if ( bInitialized )
        return;

    SolarMutexGuard aSolarMutexGuard;
    m_bInitialized = true;

    PropertyValue aPropValue;
    for ( int i = 0; i < aArguments.getLength(); i++ )
    {
        if ( aArguments[i] >>= aPropValue )
        {
            if ( aPropValue.Name == "Frame" )
                aPropValue.Value >>= m_xFrame;
            else if ( aPropValue.Name == "CommandURL" )
                aPropValue.Value >>= m_aCommandURL;
            else if ( aPropValue.Name == "ServiceManager" )
            {
                Reference<XMultiServiceFactory> xMSF;
                aPropValue.Value >>= xMSF;
                if( xMSF.is() )
                    m_xContext = comphelper::getComponentContext(xMSF);
            }
            else if ( aPropValue.Name == "ParentWindow" )
                aPropValue.Value >>= m_xParentWindow;
            else if ( aPropValue.Name == "Identifier" )
                aPropValue.Value >>= m_nID;
            else if ( aPropValue.Name == "StatusbarItem" )
                aPropValue.Value >>= m_xStatusbarItem;
        }
    }

    if ( !m_aCommandURL.isEmpty() )
        m_aListenerMap.emplace( m_aCommandURL, Reference< XDispatch >() );
}

void SAL_CALL StatusbarController::update()
{
    {
        SolarMutexGuard aSolarMutexGuard;
        if ( m_bDisposed )
            throw DisposedException();
    }

    // Bind all registered listeners to their dispatch objects
    bindListener();
}

// XComponent
void SAL_CALL StatusbarController::dispose()
{
    Reference< XComponent > xThis( static_cast< OWeakObject* >(this), UNO_QUERY );

    {
        SolarMutexGuard aSolarMutexGuard;
        if ( m_bDisposed )
            throw DisposedException();
    }

    css::lang::EventObject aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    SolarMutexGuard aSolarMutexGuard;
    Reference< XStatusListener > xStatusListener( static_cast< OWeakObject* >( this ), UNO_QUERY );
    Reference< XURLTransformer > xURLTransformer = getURLTransformer();
    css::util::URL aTargetURL;
    for (auto const& listener : m_aListenerMap)
    {
        try
        {
            Reference< XDispatch > xDispatch(listener.second);
            aTargetURL.Complete = listener.first;
            xURLTransformer->parseStrict( aTargetURL );

            if ( xDispatch.is() && xStatusListener.is() )
                xDispatch->removeStatusListener( xStatusListener, aTargetURL );
        }
        catch ( Exception& )
        {
        }
    }

    // clear hash map
    m_aListenerMap.clear();

    // release references
    m_xURLTransformer.clear();
    m_xContext.clear();
    m_xFrame.clear();
    m_xParentWindow.clear();
    m_xStatusbarItem.clear();

    m_bDisposed = true;
}

void SAL_CALL StatusbarController::addEventListener( const Reference< XEventListener >& xListener )
{
    m_aListenerContainer.addInterface( cppu::UnoType<XEventListener>::get(), xListener );
}

void SAL_CALL StatusbarController::removeEventListener( const Reference< XEventListener >& aListener )
{
    m_aListenerContainer.removeInterface( cppu::UnoType<XEventListener>::get(), aListener );
}

// XEventListener
void SAL_CALL StatusbarController::disposing( const EventObject& Source )
{
    SolarMutexGuard aSolarMutexGuard;

    if ( m_bDisposed )
        return;

    Reference< XFrame > xFrame( Source.Source, UNO_QUERY );
    if ( xFrame.is() )
    {
        if ( xFrame == m_xFrame )
            m_xFrame.clear();
        return;
    }

    Reference< XDispatch > xDispatch( Source.Source, UNO_QUERY );
    if ( !xDispatch.is() )
        return;

    for (auto & listener : m_aListenerMap)
    {
        // Compare references and release dispatch references if they are equal.
        if ( xDispatch == listener.second )
            listener.second.clear();
    }
}

// XStatusListener
void SAL_CALL StatusbarController::statusChanged( const FeatureStateEvent& Event )
{
    SolarMutexGuard aSolarMutexGuard;

    if ( m_bDisposed )
        return;

    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( m_xParentWindow );
    if ( pWindow && pWindow->GetType() == WindowType::STATUSBAR && m_nID != 0 )
    {
        OUString   aStrValue;
        StatusBar* pStatusBar = static_cast<StatusBar *>(pWindow.get());

        if ( Event.State >>= aStrValue )
            pStatusBar->SetItemText( m_nID, aStrValue );
        else if ( !Event.State.hasValue() )
            pStatusBar->SetItemText( m_nID, "" );
    }
}

// XStatusbarController
sal_Bool SAL_CALL StatusbarController::mouseButtonDown(
    const css::awt::MouseEvent& )
{
    return false;
}

sal_Bool SAL_CALL StatusbarController::mouseMove(
    const css::awt::MouseEvent& )
{
    return false;
}

sal_Bool SAL_CALL StatusbarController::mouseButtonUp(
    const css::awt::MouseEvent& )
{
    return false;
}

void SAL_CALL StatusbarController::command(
    const css::awt::Point&,
    ::sal_Int32,
    sal_Bool,
    const css::uno::Any& )
{
}

void SAL_CALL StatusbarController::paint(
    const css::uno::Reference< css::awt::XGraphics >&,
    const css::awt::Rectangle&,
    ::sal_Int32 )
{
}

void SAL_CALL StatusbarController::click( const css::awt::Point& )
{
}

void SAL_CALL StatusbarController::doubleClick( const css::awt::Point& )
{
    SolarMutexGuard aSolarMutexGuard;

    if ( m_bDisposed )
        return;

    Sequence< PropertyValue > aArgs;
    execute( aArgs );
}

void StatusbarController::addStatusListener( const OUString& aCommandURL )
{
    Reference< XDispatch >       xDispatch;
    Reference< XStatusListener > xStatusListener;
    css::util::URL    aTargetURL;

    {
        SolarMutexGuard aSolarMutexGuard;
        URLToDispatchMap::iterator pIter = m_aListenerMap.find( aCommandURL );

        // Already in the list of status listener. Do nothing.
        if ( pIter != m_aListenerMap.end() )
            return;

        // Check if we are already initialized. Implementation starts adding itself as status listener when
        // initialize is called.
        if ( !m_bInitialized )
        {
            // Put into the unordered_map of status listener. Will be activated when initialized is called
            m_aListenerMap.emplace( aCommandURL, Reference< XDispatch >() );
            return;
        }
        else
        {
            // Add status listener directly as initialize has already been called.
            Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
            if ( m_xContext.is() && xDispatchProvider.is() )
            {
                Reference< XURLTransformer > xURLTransformer = getURLTransformer();
                aTargetURL.Complete = aCommandURL;
                xURLTransformer->parseStrict( aTargetURL );
                xDispatch = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );

                xStatusListener.set( static_cast< OWeakObject* >( this ), UNO_QUERY );
                URLToDispatchMap::iterator aIter = m_aListenerMap.find( aCommandURL );
                if ( aIter != m_aListenerMap.end() )
                {
                    Reference< XDispatch > xOldDispatch( aIter->second );
                    aIter->second = xDispatch;

                    try
                    {
                        if ( xOldDispatch.is() )
                            xOldDispatch->removeStatusListener( xStatusListener, aTargetURL );
                    }
                    catch ( Exception& )
                    {
                    }
                }
                else
                    m_aListenerMap.emplace( aCommandURL, xDispatch );
            }
        }
    }

    // Call without locked mutex as we are called back from dispatch implementation
    try
    {
        if ( xDispatch.is() )
            xDispatch->addStatusListener( xStatusListener, aTargetURL );
    }
    catch ( Exception& )
    {
    }
}

void StatusbarController::bindListener()
{
    std::vector< Listener > aDispatchVector;
    Reference< XStatusListener > xStatusListener;

    {
        SolarMutexGuard aSolarMutexGuard;

        if ( !m_bInitialized )
            return;

        // Collect all registered command URL's and store them temporary
        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
        if ( m_xContext.is() && xDispatchProvider.is() )
        {
            xStatusListener.set( static_cast< OWeakObject* >( this ), UNO_QUERY );
            for (auto & listener : m_aListenerMap)
            {
                Reference< XURLTransformer > xURLTransformer = getURLTransformer();
                css::util::URL aTargetURL;
                aTargetURL.Complete = listener.first;
                xURLTransformer->parseStrict( aTargetURL );

                Reference< XDispatch > xDispatch(listener.second);
                if ( xDispatch.is() )
                {
                    // We already have a dispatch object => we have to requery.
                    // Release old dispatch object and remove it as listener
                    try
                    {
                        xDispatch->removeStatusListener( xStatusListener, aTargetURL );
                    }
                    catch ( Exception& )
                    {
                    }
                }

                listener.second.clear();
                xDispatch.clear();

                // Query for dispatch object. Old dispatch will be released with this, too.
                try
                {
                    xDispatch = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );
                }
                catch ( Exception& )
                {
                }
                listener.second = xDispatch;

                Listener aListener( aTargetURL, xDispatch );
                aDispatchVector.push_back( aListener );
            }
        }
    }

    // Call without locked mutex as we are called back from dispatch implementation
    if ( !xStatusListener.is() )
        return;

    for (Listener & rListener : aDispatchVector)
    {
        try
        {
            if ( rListener.xDispatch.is() )
                rListener.xDispatch->addStatusListener( xStatusListener, rListener.aURL );
            else if ( rListener.aURL.Complete == m_aCommandURL )
            {
                // Send status changed for the main URL, if we cannot get a valid dispatch object.
                // UI disables the button. Catch exception as we release our mutex, it is possible
                // that someone else already disposed this instance!
                FeatureStateEvent aFeatureStateEvent;
                aFeatureStateEvent.IsEnabled = false;
                aFeatureStateEvent.FeatureURL = rListener.aURL;
                aFeatureStateEvent.State = Any();
                xStatusListener->statusChanged( aFeatureStateEvent );
            }
        }
        catch ( ... ){}
    }
}

::tools::Rectangle StatusbarController::getControlRect() const
{
    ::tools::Rectangle aRect;

    {
        SolarMutexGuard aSolarMutexGuard;

        if ( m_bDisposed )
            throw DisposedException();

        if ( m_xParentWindow.is() )
        {
            VclPtr< StatusBar > pStatusBar = dynamic_cast< StatusBar* >( VCLUnoHelper::GetWindow( m_xParentWindow ).get() );
            if ( pStatusBar && pStatusBar->GetType() == WindowType::STATUSBAR )
                aRect = pStatusBar->GetItemRect( m_nID );
        }
    }

    return aRect;
}

void StatusbarController::execute( const css::uno::Sequence< css::beans::PropertyValue >& aArgs )
{
    Reference< XDispatch >       xDispatch;
    Reference< XURLTransformer > xURLTransformer;
    OUString                aCommandURL;

    {
        SolarMutexGuard aSolarMutexGuard;

        if ( m_bDisposed )
            throw DisposedException();

        if ( m_bInitialized &&
             m_xFrame.is() &&
             m_xContext.is() &&
             !m_aCommandURL.isEmpty() )
        {
            xURLTransformer = getURLTransformer();
            aCommandURL = m_aCommandURL;
            URLToDispatchMap::iterator pIter = m_aListenerMap.find( m_aCommandURL );
            if ( pIter != m_aListenerMap.end() )
                xDispatch = pIter->second;
        }
    }

    if ( !(xDispatch.is() && xURLTransformer.is()) )
        return;

    try
    {
        css::util::URL aTargetURL;

        aTargetURL.Complete = aCommandURL;
        xURLTransformer->parseStrict( aTargetURL );
        xDispatch->dispatch( aTargetURL, aArgs );
    }
    catch ( DisposedException& )
    {
    }
}

void StatusbarController::execute(
    const OUString& aCommandURL,
    const Sequence< css::beans::PropertyValue >& aArgs )
{
    Reference< XDispatch >      xDispatch;
    css::util::URL   aTargetURL;

    {
        SolarMutexGuard aSolarMutexGuard;

        if ( m_bDisposed )
            throw DisposedException();

        if ( m_bInitialized &&
             m_xFrame.is() &&
             m_xContext.is() &&
             !m_aCommandURL.isEmpty() )
        {
            Reference< XURLTransformer > xURLTransformer( getURLTransformer() );
            aTargetURL.Complete = aCommandURL;
            xURLTransformer->parseStrict( aTargetURL );

            URLToDispatchMap::iterator pIter = m_aListenerMap.find( aCommandURL );
            if ( pIter != m_aListenerMap.end() )
                xDispatch = pIter->second;
            else
            {
                Reference< css::frame::XDispatchProvider > xDispatchProvider(
                    m_xFrame->getController(), UNO_QUERY );
                if ( xDispatchProvider.is() )
                    xDispatch = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );
            }
        }
    }

    if ( xDispatch.is() )
    {
        try
        {
            xDispatch->dispatch( aTargetURL, aArgs );
        }
        catch ( DisposedException& )
        {
        }
    }
}

} // svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
