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

#include "LifeTime.hxx"
#include "macros.hxx"
#include <osl/diagnose.h>

#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/util/XCloseListener.hpp>

using namespace ::com::sun::star;

namespace apphelper
{

LifeTimeManager::LifeTimeManager( lang::XComponent* pComponent )
    : m_aListenerContainer( m_aAccessMutex )
    , m_pComponent(pComponent)
{
    impl_init();
}

void LifeTimeManager::impl_init()
{
    m_bDisposed = false;
    m_bInDispose = false;
    m_nAccessCount = 0;
    m_nLongLastingCallCount = 0;
    m_aNoAccessCountCondition.set();
    m_aNoLongLastingCallCountCondition.set();
}

LifeTimeManager::~LifeTimeManager()
{
}

bool LifeTimeManager::impl_isDisposed( bool bAssert )
{
    if( m_bDisposed || m_bInDispose )
    {
        if( bAssert )
        {
            OSL_FAIL( "This component is already disposed " );
            (void)(bAssert);
        }
        return true;
    }
    return false;
}

bool LifeTimeManager::impl_canStartApiCall()
{
    if( impl_isDisposed() )
        return false; //behave passive if already disposed

    //mutex is acquired
    return true;
}

void LifeTimeManager::impl_registerApiCall(bool bLongLastingCall)
{
    //only allowed if not disposed
    //do not acquire the mutex here because it will be acquired already
    m_nAccessCount++;
    if(m_nAccessCount==1)
        //@todo? is it ok to wake some threads here while we have acquired the mutex?
        m_aNoAccessCountCondition.reset();

    if(bLongLastingCall)
        m_nLongLastingCallCount++;
    if(m_nLongLastingCallCount==1)
        m_aNoLongLastingCallCountCondition.reset();
}

void LifeTimeManager::impl_unregisterApiCall(bool bLongLastingCall)
{
    //Mutex needs to be acquired exactly ones
    //mutex may be released inbetween in special case of impl_apiCallCountReachedNull()

    OSL_ENSURE( m_nAccessCount>0, "access count mismatch" );
    m_nAccessCount--;
    if(bLongLastingCall)
        m_nLongLastingCallCount--;
    if( m_nLongLastingCallCount==0 )
    {
        m_aNoLongLastingCallCountCondition.set();
    }
    if( m_nAccessCount== 0)
    {
        m_aNoAccessCountCondition.set();
        impl_apiCallCountReachedNull();

    }
}

bool LifeTimeManager::dispose()
    throw(uno::RuntimeException)
{
    //hold no mutex
    {
        osl::Guard< osl::Mutex > aGuard( m_aAccessMutex );

        if( m_bDisposed || m_bInDispose )
        {
            OSL_TRACE( "This component is already disposed " );
            return false; //behave passive if already disposed
        }

        m_bInDispose = true;
        //adding any listener is not allowed anymore
        //new calls will not be accepted
        //still running calls have the freedom to finish their work without crash
    }
    //no mutex is acquired

    //--do the disposing of listeners after calling this method
    {
        uno::Reference< lang::XComponent > xComponent =
            uno::Reference< lang::XComponent >(m_pComponent);
        if(xComponent.is())
        {
            // notify XCLoseListeners
            lang::EventObject aEvent( xComponent );
            m_aListenerContainer.disposeAndClear( aEvent );
        }
    }

    //no mutex is acquired
    {
        osl::ClearableGuard< osl::Mutex > aGuard( m_aAccessMutex );
        OSL_ENSURE( !m_bDisposed, "dispose was called already" );
        m_bDisposed = true;
        aGuard.clear();
    }
    //no mutex is acquired

    //wait until all still running calls have finished
    //the accessCount cannot grow anymore, because all calls will return after checking m_bDisposed
    m_aNoAccessCountCondition.wait();

    //we are the only ones working on our data now

    return true;
    //--release all resources and references after calling this method successful
}

CloseableLifeTimeManager::CloseableLifeTimeManager( css::util::XCloseable* pCloseable
        , css::lang::XComponent* pComponent )
        : LifeTimeManager( pComponent )
        , m_pCloseable(pCloseable)
{
    impl_init();
}

CloseableLifeTimeManager::~CloseableLifeTimeManager()
{
}

bool CloseableLifeTimeManager::impl_isDisposedOrClosed( bool bAssert )
{
    if( impl_isDisposed( bAssert ) )
        return true;

    if( m_bClosed )
    {
        if( bAssert )
        {
            OSL_FAIL( "This object is already closed" );
            (void)(bAssert);//avoid warnings
        }
        return true;
    }
    return false;
}

bool CloseableLifeTimeManager::g_close_startTryClose(bool bDeliverOwnership)
    throw ( uno::Exception )
{
    //no mutex is allowed to be acquired
    {
        osl::ResettableGuard< osl::Mutex > aGuard( m_aAccessMutex );
        if( impl_isDisposedOrClosed(false) )
            return false;

        //Mutex needs to be acquired exactly ones; will be released inbetween
        if( !impl_canStartApiCall() )
            return false;
        //mutex is acquired

        //not closed already -> we try to close again
        m_bInTryClose = true;
        m_aEndTryClosingCondition.reset();

        impl_registerApiCall(false);
    }

    //no mutex is acquired

    //only remove listener calls will be worked on until end of tryclose
    //all other new calls will wait till end of try close // @todo? is that really ok

    //?? still running calls have the freedom to finish their work without crash

    try
    {
        uno::Reference< util::XCloseable > xCloseable =
            uno::Reference< util::XCloseable >(m_pCloseable);
        if(xCloseable.is())
        {
            //--call queryClosing on all registered close listeners
            ::cppu::OInterfaceContainerHelper* pIC = m_aListenerContainer.getContainer(
                        cppu::UnoType<util::XCloseListener>::get());
            if( pIC )
            {
                lang::EventObject aEvent( xCloseable );
                ::cppu::OInterfaceIteratorHelper aIt( *pIC );
                while( aIt.hasMoreElements() )
                {
                    uno::Reference< util::XCloseListener > xCloseListener( aIt.next(), uno::UNO_QUERY );
                    if(xCloseListener.is())
                        xCloseListener->queryClosing( aEvent, bDeliverOwnership );
                }
            }
        }
    }
    catch( const uno::Exception& )
    {
        //no mutex is acquired
        g_close_endTryClose(bDeliverOwnership, false);
        throw;
    }
    return true;
}

void CloseableLifeTimeManager::g_close_endTryClose(bool bDeliverOwnership, bool /* bMyVeto */ )
{
    //this method is called, if the try to close was not successful
    osl::Guard< osl::Mutex > aGuard( m_aAccessMutex );
    impl_setOwnership( bDeliverOwnership, false );

    m_bInTryClose = false;
    m_aEndTryClosingCondition.set();

    //Mutex needs to be acquired exactly ones
    //mutex may be released inbetween in special case of impl_apiCallCountReachedNull()
    impl_unregisterApiCall(false);
}

bool CloseableLifeTimeManager::g_close_isNeedToCancelLongLastingCalls( bool bDeliverOwnership, util::CloseVetoException& ex )
    throw ( util::CloseVetoException )
{
    //this method is called when no closelistener has had a veto during queryclosing
    //the method returns false, if nothing stands against closing anymore
    //it returns true, if some longlasting calls are running, which might be cancelled
    //it throws the given exception, if long calls are running but not cancelable

    osl::Guard< osl::Mutex > aGuard( m_aAccessMutex );
    //this count cannot grow after try of close has started, because we wait in all those methods for end of try closing
    if( !m_nLongLastingCallCount )
        return false;

    impl_setOwnership( bDeliverOwnership, true );

    m_bInTryClose = false;
    m_aEndTryClosingCondition.set();

    //Mutex needs to be acquired exactly ones
    //mutex may be released inbetween in special case of impl_apiCallCountReachedNull()
    impl_unregisterApiCall(false);

    throw ex;
}

void CloseableLifeTimeManager::g_close_endTryClose_doClose()
{
    //this method is called, if the try to close was successful
    osl::ResettableGuard< osl::Mutex > aGuard( m_aAccessMutex );

    m_bInTryClose       = false;
    m_aEndTryClosingCondition.set();

    //Mutex needs to be acquired exactly ones
    //mutex may be released inbetween in special case of impl_apiCallCountReachedNull()
    impl_unregisterApiCall(false);
    impl_doClose();
}

void CloseableLifeTimeManager::impl_setOwnership( bool bDeliverOwnership, bool bMyVeto )
{
    m_bOwnership            = bDeliverOwnership && bMyVeto;
}

void CloseableLifeTimeManager::impl_apiCallCountReachedNull()
{
    //Mutex needs to be acquired exactly ones
    //mutex will be released inbetween in impl_doClose()
    if( m_pCloseable && impl_shouldCloseAtNextChance() )
        impl_doClose();
}

void CloseableLifeTimeManager::impl_doClose()
{
    //Mutex needs to be acquired exactly ones before calling impl_doClose()

    if(m_bClosed)
        return; //behave as passive as possible, if disposed or closed already
    if( m_bDisposed || m_bInDispose )
        return; //behave as passive as possible, if disposed or closed already

    m_bClosed = true;

    NegativeGuard< osl::Mutex > aNegativeGuard( m_aAccessMutex );
    //mutex is not acquired, mutex will be reacquired at the end of this method automatically

    uno::Reference< util::XCloseable > xCloseable=nullptr;
    try
    {
        xCloseable.set(m_pCloseable);
        if(xCloseable.is())
        {
            //--call notifyClosing on all registered close listeners
            ::cppu::OInterfaceContainerHelper* pIC = m_aListenerContainer.getContainer(
                        cppu::UnoType<util::XCloseListener>::get());
            if( pIC )
            {
                lang::EventObject aEvent( xCloseable );
                ::cppu::OInterfaceIteratorHelper aIt( *pIC );
                while( aIt.hasMoreElements() )
                {
                    uno::Reference< util::XCloseListener > xListener( aIt.next(), uno::UNO_QUERY );
                    if( xListener.is() )
                        xListener->notifyClosing( aEvent );
                }
            }
        }
    }
    catch( const uno::Exception& ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    if(xCloseable.is())
    {
        uno::Reference< lang::XComponent > xComponent =
            uno::Reference< lang::XComponent >( xCloseable, uno::UNO_QUERY );
        if(xComponent.is())
        {
            OSL_ENSURE( m_bClosed, "a not closed component will be disposed " );
            xComponent->dispose();
        }
    }
    //mutex will be reacquired in destructor of aNegativeGuard
}

void CloseableLifeTimeManager::g_addCloseListener( const uno::Reference< util::XCloseListener > & xListener )
    throw(uno::RuntimeException)
{
    osl::Guard< osl::Mutex > aGuard( m_aAccessMutex );
    //Mutex needs to be acquired exactly ones; will be released inbetween
    if( !impl_canStartApiCall() )
        return;
    //mutex is acquired

    m_aListenerContainer.addInterface( cppu::UnoType<util::XCloseListener>::get(),xListener );
    m_bOwnership = false;
}

bool CloseableLifeTimeManager::impl_canStartApiCall()
{
    //Mutex needs to be acquired exactly ones before calling this method
    //the mutex will be released inbetween and reacquired

    if( impl_isDisposed() )
        return false; //behave passive if already disposed
    if( m_bClosed )
        return false; //behave passive if closing is already done

    //during try-close most calls need to wait for the decision
    while( m_bInTryClose )
    {
        //if someone tries to close this object at the moment
        //we need to wait for his end because the result of the preceding call
        //is relevant for our behaviour here

        m_aAccessMutex.release();
        m_aEndTryClosingCondition.wait(); //@todo??? this may block??? try closing
        m_aAccessMutex.acquire();
        if( m_bDisposed || m_bInDispose || m_bClosed )
            return false; //return if closed already
    }
    //mutex is acquired
    return true;
}

bool LifeTimeGuard::startApiCall(bool bLongLastingCall)
{
    //Mutex needs to be acquired exactly ones; will be released inbetween
    //mutex is requiered due to constructor of LifeTimeGuard

    OSL_ENSURE( !m_bCallRegistered, "this method is only allowed ones" );
    if(m_bCallRegistered)
        return false;

    //Mutex needs to be acquired exactly ones; will be released inbetween
    if( !m_rManager.impl_canStartApiCall() )
        return false;
    //mutex is acquired

    m_bCallRegistered = true;
    m_bLongLastingCallRegistered = bLongLastingCall;
    m_rManager.impl_registerApiCall(bLongLastingCall);
    return true;
}

LifeTimeGuard::~LifeTimeGuard()
{
    try
    {
        //do acquire the mutex if it was cleared before
        osl::MutexGuard g(m_rManager.m_aAccessMutex);
        if(m_bCallRegistered)
        {
            //Mutex needs to be acquired exactly ones
            //mutex may be released inbetween in special case of impl_apiCallCountReachedNull()
            m_rManager.impl_unregisterApiCall(m_bLongLastingCallRegistered);
        }
    }
    catch( uno::Exception& ex )
    {
        //@todo ? allow a uno::RuntimeException from dispose to travel through??
        ex.Context.is(); //to avoid compilation warnings
    }
}

}//end namespace apphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
