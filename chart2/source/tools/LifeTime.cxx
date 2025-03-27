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

#include <LifeTime.hxx>
#include <osl/diagnose.h>

#include <com/sun/star/document/XStorageChangeListener.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/util/XCloseListener.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <comphelper/diagnose_ex.hxx>
#include <sal/log.hxx>

using namespace ::com::sun::star;

namespace apphelper
{

LifeTimeManager::LifeTimeManager( lang::XComponent* pComponent )
    : m_pComponent(pComponent)
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

void LifeTimeManager::impl_unregisterApiCall(std::unique_lock<std::mutex>& rGuard, bool bLongLastingCall)
{
    //Mutex needs to be acquired exactly once
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
        impl_apiCallCountReachedNull(rGuard);

    }
}

bool LifeTimeManager::dispose()
{
    //hold no mutex
    {
        std::unique_lock aGuard( m_aAccessMutex );

        if( m_bDisposed || m_bInDispose )
        {
            SAL_WARN("chart2",  "This component is already disposed " );
            return false; //behave passive if already disposed
        }

        m_bInDispose = true;
        //adding any listener is not allowed anymore
        //new calls will not be accepted
        //still running calls have the freedom to finish their work without crash

        uno::Reference< lang::XComponent > xComponent(m_pComponent);
        if(xComponent.is())
        {
            // notify XCLoseListeners
            lang::EventObject aEvent( xComponent );
            m_aCloseListeners.disposeAndClear( aGuard, aEvent );
            m_aModifyListeners.disposeAndClear( aGuard, aEvent );
            m_aStorageChangeListeners.disposeAndClear( aGuard, aEvent );
            m_aEventListeners.disposeAndClear( aGuard, aEvent );
            m_aSelectionChangeListeners.disposeAndClear( aGuard, aEvent );
        }

        OSL_ENSURE( !m_bDisposed, "dispose was called already" );
        m_bDisposed = true;
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
    m_bClosed = false;
    m_bInTryClose = false;
    m_bOwnership = false;
    m_aEndTryClosingCondition.set();
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
        }
        return true;
    }
    return false;
}

bool CloseableLifeTimeManager::g_close_startTryClose(bool bDeliverOwnership)
{
    //no mutex is allowed to be acquired
    {
        std::unique_lock aGuard( m_aAccessMutex );
        if( impl_isDisposedOrClosed(false) )
            return false;

        //Mutex needs to be acquired exactly once; will be released inbetween
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
        uno::Reference< util::XCloseable > xCloseable(m_pCloseable);
        if(xCloseable.is())
        {
            std::unique_lock aGuard( m_aAccessMutex );
            //--call queryClosing on all registered close listeners
            if( m_aCloseListeners.getLength(aGuard) )
            {
                lang::EventObject aEvent( xCloseable );
                m_aCloseListeners.forEach(aGuard,
                    [&aEvent, bDeliverOwnership](const uno::Reference<util::XCloseListener>& l)
                    {
                        l->queryClosing(aEvent, bDeliverOwnership);
                    });
            }
        }
    }
    catch( const uno::Exception& )
    {
        //no mutex is acquired
        g_close_endTryClose();
        throw;
    }
    return true;
}

void CloseableLifeTimeManager::g_close_endTryClose()
{
    //this method is called, if the try to close was not successful
    std::unique_lock aGuard( m_aAccessMutex );

    m_bOwnership = false;
    m_bInTryClose = false;
    m_aEndTryClosingCondition.set();

    //Mutex needs to be acquired exactly once
    //mutex may be released inbetween in special case of impl_apiCallCountReachedNull()
    impl_unregisterApiCall(aGuard, false);
}

void CloseableLifeTimeManager::g_close_isNeedToCancelLongLastingCalls( bool bDeliverOwnership, util::CloseVetoException const & ex )
{
    //this method is called when no closelistener has had a veto during queryclosing
    //the method returns false, if nothing stands against closing anymore
    //it returns true, if some longlasting calls are running, which might be cancelled
    //it throws the given exception, if long calls are running but not cancelable

    std::unique_lock aGuard( m_aAccessMutex );
    //this count cannot grow after try of close has started, because we wait in all those methods for end of try closing
    if( !m_nLongLastingCallCount )
        return;

    m_bOwnership = bDeliverOwnership;
    m_bInTryClose = false;
    m_aEndTryClosingCondition.set();

    //Mutex needs to be acquired exactly once
    //mutex may be released inbetween in special case of impl_apiCallCountReachedNull()
    impl_unregisterApiCall(aGuard, false);

    throw ex;
}

void CloseableLifeTimeManager::g_close_endTryClose_doClose()
{
    //this method is called, if the try to close was successful
    std::unique_lock aGuard( m_aAccessMutex );

    m_bInTryClose       = false;
    m_aEndTryClosingCondition.set();

    //Mutex needs to be acquired exactly once
    //mutex may be released inbetween in special case of impl_apiCallCountReachedNull()
    impl_unregisterApiCall(aGuard, false);
    impl_doClose(aGuard);
}

void CloseableLifeTimeManager::impl_apiCallCountReachedNull(std::unique_lock<std::mutex>& rGuard)
{
    //Mutex needs to be acquired exactly once
    //mutex will be released inbetween in impl_doClose()
    if( m_pCloseable && m_bOwnership )
        impl_doClose(rGuard);
}

void CloseableLifeTimeManager::impl_doClose(std::unique_lock<std::mutex>& rGuard)
{
    //Mutex needs to be acquired exactly once before calling impl_doClose()

    if(m_bClosed)
        return; //behave as passive as possible, if disposed or closed already
    if( m_bDisposed || m_bInDispose )
        return; //behave as passive as possible, if disposed or closed already

    m_bClosed = true;

    uno::Reference< util::XCloseable > xCloseable;
    xCloseable.set(m_pCloseable);
    try
    {
        if(xCloseable.is())
        {
            //--call notifyClosing on all registered close listeners
            if( m_aCloseListeners.getLength(rGuard) )
            {
                lang::EventObject aEvent( xCloseable );
                m_aCloseListeners.notifyEach(rGuard, &util::XCloseListener::notifyClosing, aEvent);
            }
        }
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    rGuard.unlock();
    if(xCloseable.is())
    {
        uno::Reference< lang::XComponent > xComponent( xCloseable, uno::UNO_QUERY );
        if(xComponent.is())
        {
            OSL_ENSURE( m_bClosed, "a not closed component will be disposed " );
            xComponent->dispose();
        }
    }
    rGuard.lock();
}

void CloseableLifeTimeManager::g_addCloseListener( const uno::Reference< util::XCloseListener > & xListener )
{
    std::unique_lock aGuard( m_aAccessMutex );
    //Mutex needs to be acquired exactly once; will be released inbetween
    if( !impl_canStartApiCall() )
        return;
    //mutex is acquired

    m_aCloseListeners.addInterface( aGuard, xListener );
    m_bOwnership = false;
}

bool CloseableLifeTimeManager::impl_canStartApiCall()
{
    //Mutex needs to be acquired exactly once before calling this method
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

        m_aAccessMutex.unlock();
        m_aEndTryClosingCondition.wait(); //@todo??? this may block??? try closing
        m_aAccessMutex.lock();
        if( m_bDisposed || m_bInDispose || m_bClosed )
            return false; //return if closed already
    }
    //mutex is acquired
    return true;
}

bool LifeTimeGuard::startApiCall(bool bLongLastingCall)
{
    //Mutex needs to be acquired exactly once; will be released inbetween
    //mutex is required due to constructor of LifeTimeGuard

    OSL_ENSURE( !m_bCallRegistered, "this method is only allowed ones" );
    if(m_bCallRegistered)
        return false;

    //Mutex needs to be acquired exactly once; will be released inbetween
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
        if (!m_guard.owns_lock())
            m_guard.lock();
        if(m_bCallRegistered)
        {
            //Mutex needs to be acquired exactly once
            //mutex may be released inbetween in special case of impl_apiCallCountReachedNull()
            m_rManager.impl_unregisterApiCall(m_guard, m_bLongLastingCallRegistered);
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
