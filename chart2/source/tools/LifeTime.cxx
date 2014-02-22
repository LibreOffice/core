/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "LifeTime.hxx"
#include "macros.hxx"
#include <osl/diagnose.h>

#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/util/XCloseListener.hpp>

using namespace ::com::sun::star;

namespace apphelper
{

LifeTimeManager::LifeTimeManager( lang::XComponent* pComponent, sal_Bool bLongLastingCallsCancelable )
    : m_aListenerContainer( m_aAccessMutex )
    , m_pComponent(pComponent)
    , m_bLongLastingCallsCancelable(bLongLastingCallsCancelable)
{
    impl_init();
}

void LifeTimeManager::impl_init()
{
    m_bDisposed = sal_False;
    m_bInDispose = sal_False;
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
            sal_Bool LifeTimeManager
::impl_canStartApiCall()
{
    if( impl_isDisposed() )
        return sal_False; 

    
    return sal_True;
}

    void LifeTimeManager
::impl_registerApiCall(sal_Bool bLongLastingCall)
{
    
    
    m_nAccessCount++;
    if(m_nAccessCount==1)
        
        m_aNoAccessCountCondition.reset();

    if(bLongLastingCall)
        m_nLongLastingCallCount++;
    if(m_nLongLastingCallCount==1)
        m_aNoLongLastingCallCountCondition.reset();
}
    void LifeTimeManager
::impl_unregisterApiCall(sal_Bool bLongLastingCall)
{
    
    

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

        sal_Bool LifeTimeManager
::dispose() throw(uno::RuntimeException)
{
    
    {
        osl::Guard< osl::Mutex > aGuard( m_aAccessMutex );

        if( m_bDisposed || m_bInDispose )
        {
            OSL_TRACE( "This component is already disposed " );
            return sal_False; 
        }

        m_bInDispose = true;
        
        
        
    }
    

    
    {
        uno::Reference< lang::XComponent > xComponent =
            uno::Reference< lang::XComponent >(m_pComponent);;
        if(xComponent.is())
        {
            
            lang::EventObject aEvent( xComponent );
            m_aListenerContainer.disposeAndClear( aEvent );
        }
    }

    
    {
        osl::ClearableGuard< osl::Mutex > aGuard( m_aAccessMutex );
        OSL_ENSURE( !m_bDisposed, "dispose was called already" );
        m_bDisposed = sal_True;
        aGuard.clear();
    }
    

    
    
    m_aNoAccessCountCondition.wait();

    

    return sal_True;
    
}

CloseableLifeTimeManager::CloseableLifeTimeManager( ::com::sun::star::util::XCloseable* pCloseable
        , ::com::sun::star::lang::XComponent* pComponent
        , sal_Bool bLongLastingCallsCancelable )
        : LifeTimeManager( pComponent, bLongLastingCallsCancelable )
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
            (void)(bAssert);
        }
        return true;
    }
    return false;
}

        sal_Bool CloseableLifeTimeManager
::g_close_startTryClose(sal_Bool bDeliverOwnership)
    throw ( uno::Exception )
{
    
    {
        osl::ResettableGuard< osl::Mutex > aGuard( m_aAccessMutex );
        if( impl_isDisposedOrClosed(false) )
            return sal_False;

        
        if( !impl_canStartApiCall() )
            return sal_False;
        

        
        m_bInTryClose = sal_True;
        m_aEndTryClosingCondition.reset();

        impl_registerApiCall(sal_False);
    }

    

    
    

    

    try
    {
        uno::Reference< util::XCloseable > xCloseable =
            uno::Reference< util::XCloseable >(m_pCloseable);;
        if(xCloseable.is())
        {
            
            ::cppu::OInterfaceContainerHelper* pIC = m_aListenerContainer.getContainer(
                        ::getCppuType((const uno::Reference< util::XCloseListener >*)0) );;
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
        
        g_close_endTryClose(bDeliverOwnership, sal_False);
        throw;
    }
    return sal_True;
}

    void CloseableLifeTimeManager
::g_close_endTryClose(sal_Bool bDeliverOwnership, sal_Bool /* bMyVeto */ )
{
    
    osl::Guard< osl::Mutex > aGuard( m_aAccessMutex );
    impl_setOwnership( bDeliverOwnership, sal_False );

    m_bInTryClose = sal_False;
    m_aEndTryClosingCondition.set();

    
    
    impl_unregisterApiCall(sal_False);
}

    sal_Bool CloseableLifeTimeManager
::g_close_isNeedToCancelLongLastingCalls( sal_Bool bDeliverOwnership, util::CloseVetoException& ex )
    throw ( util::CloseVetoException )
{
    
    
    
    

    osl::Guard< osl::Mutex > aGuard( m_aAccessMutex );
    
    if( !m_nLongLastingCallCount )
        return sal_False;

      if(m_bLongLastingCallsCancelable)
        return sal_True;

    impl_setOwnership( bDeliverOwnership, sal_True );

    m_bInTryClose = sal_False;
    m_aEndTryClosingCondition.set();

    
    
    impl_unregisterApiCall(sal_False);

    throw ex;
}

    void CloseableLifeTimeManager
::g_close_endTryClose_doClose()
{
    
    osl::ResettableGuard< osl::Mutex > aGuard( m_aAccessMutex );

    m_bInTryClose       = sal_False;
    m_aEndTryClosingCondition.set();

    
    
    impl_unregisterApiCall(sal_False);
    impl_doClose();
}

    void CloseableLifeTimeManager
::impl_setOwnership( sal_Bool bDeliverOwnership, sal_Bool bMyVeto )
{
    m_bOwnership            = bDeliverOwnership && bMyVeto;
    m_bOwnershipIsWellKnown = sal_True;
}
    sal_Bool CloseableLifeTimeManager
::impl_shouldCloseAtNextChance()
{
    return m_bOwnership;
}

    void CloseableLifeTimeManager
::impl_apiCallCountReachedNull()
{
    
    
    if( m_pCloseable && impl_shouldCloseAtNextChance() )
        impl_doClose();
}

    void CloseableLifeTimeManager
::impl_doClose()
{
    

    if(m_bClosed)
        return; 
    if( m_bDisposed || m_bInDispose )
        return; 

    
    m_bClosed = sal_True;

    NegativeGuard< osl::Mutex > aNegativeGuard( m_aAccessMutex );
    

    uno::Reference< util::XCloseable > xCloseable=NULL;
    try
    {
        xCloseable = uno::Reference< util::XCloseable >(m_pCloseable);;
        if(xCloseable.is())
        {
            
            ::cppu::OInterfaceContainerHelper* pIC = m_aListenerContainer.getContainer(
                        ::getCppuType((const uno::Reference< util::XCloseListener >*)0) );;
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
    
}

    sal_Bool CloseableLifeTimeManager
::g_addCloseListener( const uno::Reference< util::XCloseListener > & xListener )
    throw(uno::RuntimeException)
{
    osl::Guard< osl::Mutex > aGuard( m_aAccessMutex );
    
    if( !impl_canStartApiCall() )
        return sal_False;
    

    m_aListenerContainer.addInterface( ::getCppuType((const uno::Reference< util::XCloseListener >*)0),xListener );
    m_bOwnership = sal_False;
    return sal_True;
}

    sal_Bool CloseableLifeTimeManager
::impl_canStartApiCall()
{
    
    

    if( impl_isDisposed() )
        return sal_False; 
    if( m_bClosed )
        return sal_False; 

    
    while( m_bInTryClose )
    {
        
        
        

        m_aAccessMutex.release();
        m_aEndTryClosingCondition.wait(); 
        m_aAccessMutex.acquire();
        if( m_bDisposed || m_bInDispose || m_bClosed )
            return sal_False; 
    }
    
    return sal_True;
}

    sal_Bool LifeTimeGuard
::startApiCall(sal_Bool bLongLastingCall)
{
    
    

    OSL_ENSURE( !m_bCallRegistered, "this method is only allowed ones" );
    if(m_bCallRegistered)
        return sal_False;

    
    if( !m_rManager.impl_canStartApiCall() )
        return sal_False;
    

    m_bCallRegistered = sal_True;
    m_bLongLastingCallRegistered = bLongLastingCall;
    m_rManager.impl_registerApiCall(bLongLastingCall);
    return sal_True;
}

LifeTimeGuard::~LifeTimeGuard()
{
    try
    {
        
        osl::MutexGuard g(m_rManager.m_aAccessMutex);
        if(m_bCallRegistered)
        {
            
            
            m_rManager.impl_unregisterApiCall(m_bLongLastingCallRegistered);
        }
    }
    catch( uno::Exception& ex )
    {
        
        ex.Context.is(); 
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
