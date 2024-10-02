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


#include "dynamicresultsetwrapper.hxx"
#include <cppuhelper/queryinterface.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/ucb/AlreadyInitializedException.hpp>
#include <com/sun/star/ucb/ListActionType.hpp>
#include <com/sun/star/ucb/ListenerAlreadySetException.hpp>
#include <com/sun/star/ucb/ServiceNotFoundException.hpp>
#include <com/sun/star/ucb/WelcomeDynamicResultSetStruct.hpp>
#include <com/sun/star/ucb/CachedDynamicResultSetStubFactory.hpp>

using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace comphelper;




DynamicResultSetWrapper::DynamicResultSetWrapper(
                    Reference< XDynamicResultSet > const & xOrigin
                    , const Reference< XComponentContext > & rxContext )

                : m_bDisposed( false )
                , m_bInDispose( false )
                , m_xContext( rxContext )
                , m_bStatic( false )
                , m_bGotWelcome( false )
                , m_xSource( xOrigin )
            //  , m_xSourceResultCurrent( NULL )
            //  , m_bUseOne( NULL )
{
    m_xMyListenerImpl = new DynamicResultSetWrapperListener( this );
    //call impl_init() at the end of constructor of derived class
};

void DynamicResultSetWrapper::impl_init()
{
    //call this at the end of constructor of derived class


    Reference< XDynamicResultSet > xSource;
    {
        std::unique_lock aGuard( m_aMutex );
        xSource = m_xSource;
        m_xSource = nullptr;
    }
    if( xSource.is() )
        setSource( xSource );
}

DynamicResultSetWrapper::~DynamicResultSetWrapper()
{
    //call impl_deinit() at start of destructor of derived class
};

void DynamicResultSetWrapper::impl_deinit()
{
    //call this at start of destructor of derived class

    m_xMyListenerImpl->impl_OwnerDies();
}

void DynamicResultSetWrapper::impl_EnsureNotDisposed(std::unique_lock<std::mutex>& /*rGuard*/)
{
    if( m_bDisposed )
        throw DisposedException();
}

//virtual
void DynamicResultSetWrapper::impl_InitResultSetOne( std::unique_lock<std::mutex>& /*rGuard*/, const Reference< XResultSet >& xResultSet )
{
    OSL_ENSURE( !m_xSourceResultOne.is(), "Source ResultSet One is set already" );
    m_xSourceResultOne = xResultSet;
    m_xMyResultOne = xResultSet;
}

//virtual
void DynamicResultSetWrapper::impl_InitResultSetTwo( std::unique_lock<std::mutex>& /*rGuard*/, const Reference< XResultSet >& xResultSet )
{
    OSL_ENSURE( !m_xSourceResultTwo.is(), "Source ResultSet Two is set already" );
    m_xSourceResultTwo = xResultSet;
    m_xMyResultTwo = xResultSet;
}

// XInterface methods.
css::uno::Any SAL_CALL DynamicResultSetWrapper::queryInterface( const css::uno::Type & rType )
{
    //list all interfaces inclusive baseclasses of interfaces
    css::uno::Any aRet = cppu::queryInterface( rType,
                                               static_cast< XComponent* >(this), //base of XDynamicResultSet
                                               static_cast< XDynamicResultSet* >(this),
                                               static_cast< XSourceInitialization* >(this)
                                               );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

// XComponent methods.

// virtual
void SAL_CALL DynamicResultSetWrapper::dispose()
{
    std::unique_lock aGuard( m_aMutex );
    impl_EnsureNotDisposed(aGuard);

    Reference< XComponent > xSourceComponent;
    if( m_bInDispose || m_bDisposed )
        return;
    m_bInDispose = true;

    xSourceComponent = m_xSource;

    if( m_aDisposeEventListeners.getLength(aGuard) )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XComponent * >( this );
        m_aDisposeEventListeners.disposeAndClear( aGuard, aEvt );
    }

    /* //@todo ?? ( only if java collection needs to long )
    if( xSourceComponent.is() )
        xSourceComponent->dispose();
    */

    m_bDisposed = true;
    m_bInDispose = false;
}


// virtual
void SAL_CALL DynamicResultSetWrapper::addEventListener( const Reference< XEventListener >& Listener )
{
    std::unique_lock aGuard( m_aMutex );
    impl_EnsureNotDisposed(aGuard);

    m_aDisposeEventListeners.addInterface( aGuard, Listener );
}


// virtual
void SAL_CALL DynamicResultSetWrapper::removeEventListener( const Reference< XEventListener >& Listener )
{
    std::unique_lock aGuard( m_aMutex );
    impl_EnsureNotDisposed(aGuard);

    m_aDisposeEventListeners.removeInterface( aGuard, Listener );
}


// own methods


//virtual
void DynamicResultSetWrapper::impl_disposing( const EventObject& )
{
    std::unique_lock aGuard( m_aMutex );

    impl_EnsureNotDisposed(aGuard);

    if( !m_xSource.is() )
        return;

    //release all references to the broadcaster:
    m_xSource.clear();
    m_xSourceResultOne.clear();//?? or only when not static??
    m_xSourceResultTwo.clear();//??
    //@todo m_xMyResultOne.clear(); ???
    //@todo m_xMyResultTwo.clear(); ???
}

//virtual
void DynamicResultSetWrapper::impl_notify( const ListEvent& Changes )
{
    std::unique_lock aGuard( m_aMutex );
    impl_EnsureNotDisposed(aGuard);
    //@todo
    /*
    <p>The Listener is allowed to blockade this call, until he really want to go
    to the new version. The only situation, where the listener has to return the
    update call at once is, while he disposes his broadcaster or while he is
    removing himself as listener (otherwise you deadlock)!!!
    */
    // handle the actions in the list

    ListEvent aNewEvent;
    aNewEvent.Source = static_cast< XDynamicResultSet * >( this );
    aNewEvent.Changes = Changes.Changes;

    for( ListAction& rAction : asNonConstRange(aNewEvent.Changes) )
    {
        if (m_bGotWelcome)
            break;

        switch( rAction.ListActionType )
        {
            case ListActionType::WELCOME:
            {
                WelcomeDynamicResultSetStruct aWelcome;
                if( rAction.ActionInfo >>= aWelcome )
                {
                    impl_InitResultSetOne( aGuard, aWelcome.Old );
                    impl_InitResultSetTwo( aGuard, aWelcome.New );
                    m_bGotWelcome = true;

                    aWelcome.Old = m_xMyResultOne;
                    aWelcome.New = m_xMyResultTwo;

                    rAction.ActionInfo <<= aWelcome;
                }
                else
                {
                    OSL_FAIL( "ListActionType was WELCOME but ActionInfo didn't contain a WelcomeDynamicResultSetStruct" );
                    //throw RuntimeException();
                }
                break;
            }
        }
    }
    OSL_ENSURE( m_bGotWelcome, "first notification was without WELCOME" );

    aGuard.unlock();

    if( !m_xListener.is() )
        m_aListenerSet.wait();
    m_xListener->notify( aNewEvent );

    /*
    m_bUseOne = !m_bUseOne;
    if( m_bUseOne )
        m_xSourceResultCurrent = m_xSourceResultOne;
    else
        m_xSourceResultCurrent = m_xSourceResultTwo;
    */
}


// XSourceInitialization

//virtual
void SAL_CALL DynamicResultSetWrapper::setSource( const Reference< XInterface > & Source )
{
    std::unique_lock aGuard( m_aMutex );
    impl_EnsureNotDisposed(aGuard);
    if( m_xSource.is() )
        throw AlreadyInitializedException();

    Reference< XDynamicResultSet > xSourceDynamic( Source, UNO_QUERY );
    OSL_ENSURE( xSourceDynamic.is(),
        "the given source is not of required type XDynamicResultSet" );

    Reference< XDynamicResultSetListener > xListener;
    Reference< XDynamicResultSetListener > xMyListenerImpl;

    bool bStatic = false;
    m_xSource = xSourceDynamic;
    xListener = m_xListener;
    bStatic = m_bStatic;
    xMyListenerImpl = m_xMyListenerImpl.get();
    if( xListener.is() )
        xSourceDynamic->setListener( m_xMyListenerImpl );
    else if( bStatic )
    {
        Reference< XComponent > xSourceComponent( Source, UNO_QUERY );
        xSourceComponent->addEventListener( xMyListenerImpl );
    }
    m_aSourceSet.set();
}


// XDynamicResultSet

//virtual
Reference< XResultSet > SAL_CALL DynamicResultSetWrapper::getStaticResultSet()
{
    std::unique_lock aGuard( m_aMutex );
    impl_EnsureNotDisposed(aGuard);

    if( m_xListener.is() )
        throw ListenerAlreadySetException();

    Reference< XDynamicResultSet > xSource = m_xSource;
    Reference< XEventListener > xMyListenerImpl = m_xMyListenerImpl;
    m_bStatic = true;

    aGuard.unlock();

    if( xSource.is() )
    {
        xSource->addEventListener( xMyListenerImpl );
    }
    if( !xSource.is() )
        m_aSourceSet.wait();

    aGuard.lock();

    Reference< XResultSet > xResultSet = xSource->getStaticResultSet();
    impl_InitResultSetOne( aGuard, xResultSet );
    return m_xMyResultOne;
}

//virtual
void SAL_CALL DynamicResultSetWrapper::setListener( const Reference< XDynamicResultSetListener > & Listener )
{
    std::unique_lock aGuard( m_aMutex );
    impl_EnsureNotDisposed(aGuard);

    if( m_xListener.is() )
        throw ListenerAlreadySetException();
    if( m_bStatic )
        throw ListenerAlreadySetException();

    m_xListener = Listener;
    m_aDisposeEventListeners.addInterface( aGuard, Listener );

    Reference< XDynamicResultSet > xSource = m_xSource;
    Reference< XDynamicResultSetListener > xMyListenerImpl = m_xMyListenerImpl;

    aGuard.unlock();

    if ( xSource.is() )
        xSource->setListener( xMyListenerImpl );

    m_aListenerSet.set();
}

//virtual
void SAL_CALL DynamicResultSetWrapper::connectToCache( const Reference< XDynamicResultSet > & xCache )
{
    std::unique_lock aGuard( m_aMutex );
    impl_EnsureNotDisposed(aGuard);

    if( m_xListener.is() )
        throw ListenerAlreadySetException();
    if( m_bStatic )
        throw ListenerAlreadySetException();
    aGuard.unlock();

    Reference< XSourceInitialization > xTarget( xCache, UNO_QUERY );
    OSL_ENSURE( xTarget.is(), "The given Target doesn't have the required interface 'XSourceInitialization'" );
    if( xTarget.is() && m_xContext.is() )
    {
        //@todo m_aSourceSet.wait();?

        Reference< XCachedDynamicResultSetStubFactory > xStubFactory;
        try
        {
            xStubFactory = CachedDynamicResultSetStubFactory::create( m_xContext );
        }
        catch ( Exception const & )
        {
        }

        if( xStubFactory.is() )
        {
            xStubFactory->connectToCache(
                  this, xCache, Sequence< NumberedSortingInfo > (), nullptr );
            return;
        }
    }
    OSL_FAIL( "could not connect to cache" );
    throw ServiceNotFoundException();
}

//virtual
sal_Int16 SAL_CALL DynamicResultSetWrapper::getCapabilities()
{
    {
        std::unique_lock aGuard( m_aMutex );
        impl_EnsureNotDisposed(aGuard);
    }
    m_aSourceSet.wait();
    Reference< XDynamicResultSet > xSource;
    {
        std::unique_lock aGuard( m_aMutex );
        xSource = m_xSource;
    }
    return xSource->getCapabilities();
}




DynamicResultSetWrapperListener::DynamicResultSetWrapperListener(
        DynamicResultSetWrapper* pOwner )
        : m_pOwner( pOwner )
{

}

DynamicResultSetWrapperListener::~DynamicResultSetWrapperListener()
{

}


// XInterface methods.

void SAL_CALL DynamicResultSetWrapperListener::acquire()
    noexcept
{
    OWeakObject::acquire();
}

void SAL_CALL DynamicResultSetWrapperListener::release()
    noexcept
{
    OWeakObject::release();
}

css::uno::Any SAL_CALL DynamicResultSetWrapperListener::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = cppu::queryInterface( rType,
                                               static_cast< XDynamicResultSetListener* >(this),
                                               static_cast< XEventListener* >(this)
                                               );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

// XDynamicResultSetListener methods:

//virtual
void SAL_CALL DynamicResultSetWrapperListener::disposing( const EventObject& rEventObject )
{
    std::unique_lock aGuard( m_aMutex );

    if( m_pOwner )
        m_pOwner->impl_disposing( rEventObject );
}

//virtual
void SAL_CALL DynamicResultSetWrapperListener::notify( const ListEvent& Changes )
{
    std::unique_lock aGuard( m_aMutex );

    if( m_pOwner )
        m_pOwner->impl_notify( Changes );
}


// own methods:


void DynamicResultSetWrapperListener::impl_OwnerDies()
{
    std::unique_lock aGuard( m_aMutex );

    m_pOwner = nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
