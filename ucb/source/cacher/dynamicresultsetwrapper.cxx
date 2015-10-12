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


#include <dynamicresultsetwrapper.hxx>
#include <ucbhelper/macros.hxx>
#include <osl/diagnose.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/ucb/ListActionType.hpp>
#include <com/sun/star/ucb/WelcomeDynamicResultSetStruct.hpp>
#include <com/sun/star/ucb/CachedDynamicResultSetStubFactory.hpp>

using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace cppu;




// class DynamicResultSetWrapper



DynamicResultSetWrapper::DynamicResultSetWrapper(
                    Reference< XDynamicResultSet > xOrigin
                    , const Reference< XComponentContext > & rxContext )

                : m_bDisposed( false )
                , m_bInDispose( false )
                , m_pDisposeEventListeners( NULL )
                , m_xContext( rxContext )
                , m_bStatic( false )
                , m_bGotWelcome( false )
                , m_xSource( xOrigin )
                , m_xSourceResultOne( NULL )
                , m_xSourceResultTwo( NULL )
            //  , m_xSourceResultCurrent( NULL )
            //  , m_bUseOne( NULL )
                , m_xMyResultOne( NULL )
                , m_xMyResultTwo( NULL )
                , m_xListener( NULL )
{
    m_pMyListenerImpl = new DynamicResultSetWrapperListener( this );
    m_xMyListenerImpl = Reference< XDynamicResultSetListener >( m_pMyListenerImpl );
    //call impl_init() at the end of constructor of derived class
};

void SAL_CALL DynamicResultSetWrapper::impl_init()
{
    //call this at the end of constructor of derived class


    Reference< XDynamicResultSet > xSource = NULL;
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        xSource = m_xSource;
        m_xSource = NULL;
    }
    if( xSource.is() )
        setSource( xSource );
}

DynamicResultSetWrapper::~DynamicResultSetWrapper()
{
    //call impl_deinit() at start of destructor of derived class

    delete m_pDisposeEventListeners;
};

void SAL_CALL DynamicResultSetWrapper::impl_deinit()
{
    //call this at start of destructor of derived class

    m_pMyListenerImpl->impl_OwnerDies();
}

void SAL_CALL DynamicResultSetWrapper::impl_EnsureNotDisposed()
    throw( DisposedException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    if( m_bDisposed )
        throw DisposedException();
}

//virtual
void SAL_CALL DynamicResultSetWrapper::impl_InitResultSetOne( const Reference< XResultSet >& xResultSet )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    OSL_ENSURE( !m_xSourceResultOne.is(), "Source ResultSet One is set already" );
    m_xSourceResultOne = xResultSet;
    m_xMyResultOne = xResultSet;
}

//virtual
void SAL_CALL DynamicResultSetWrapper::impl_InitResultSetTwo( const Reference< XResultSet >& xResultSet )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    OSL_ENSURE( !m_xSourceResultTwo.is(), "Source ResultSet Two is set already" );
    m_xSourceResultTwo = xResultSet;
    m_xMyResultTwo = xResultSet;
}

// XInterface methods.
css::uno::Any SAL_CALL DynamicResultSetWrapper::queryInterface( const css::uno::Type & rType )
    throw( css::uno::RuntimeException, std::exception )
{
    //list all interfaces inclusive baseclasses of interfaces
    css::uno::Any aRet = cppu::queryInterface( rType,
                                               (static_cast< XComponent* >(this)), //base of XDynamicResultSet
                                               (static_cast< XDynamicResultSet* >(this)),
                                               (static_cast< XSourceInitialization* >(this))
                                               );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

// XComponent methods.

// virtual
void SAL_CALL DynamicResultSetWrapper::dispose()
    throw( RuntimeException, std::exception )
{
    impl_EnsureNotDisposed();

    Reference< XComponent > xSourceComponent;
    {
        osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );
        if( m_bInDispose || m_bDisposed )
            return;
        m_bInDispose = true;

        xSourceComponent = Reference< XComponent >(m_xSource, UNO_QUERY);

        if( m_pDisposeEventListeners && m_pDisposeEventListeners->getLength() )
        {
            EventObject aEvt;
            aEvt.Source = static_cast< XComponent * >( this );

            aGuard.clear();
            m_pDisposeEventListeners->disposeAndClear( aEvt );
        }
    }

    /* //@todo ?? ( only if java collection needs to long )
    if( xSourceComponent.is() )
        xSourceComponent->dispose();
    */

    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    m_bDisposed = true;
    m_bInDispose = false;
}


// virtual
void SAL_CALL DynamicResultSetWrapper::addEventListener( const Reference< XEventListener >& Listener )
    throw( RuntimeException, std::exception )
{
    impl_EnsureNotDisposed();
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( !m_pDisposeEventListeners )
        m_pDisposeEventListeners =
                    new OInterfaceContainerHelper( m_aContainerMutex );

    m_pDisposeEventListeners->addInterface( Listener );
}


// virtual
void SAL_CALL DynamicResultSetWrapper::removeEventListener( const Reference< XEventListener >& Listener )
    throw( RuntimeException, std::exception )
{
    impl_EnsureNotDisposed();
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( m_pDisposeEventListeners )
        m_pDisposeEventListeners->removeInterface( Listener );
}


// own methods


//virtual
void SAL_CALL DynamicResultSetWrapper::impl_disposing( const EventObject& )
    throw( RuntimeException )
{
    impl_EnsureNotDisposed();

    osl::Guard< osl::Mutex > aGuard( m_aMutex );

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
void SAL_CALL DynamicResultSetWrapper::impl_notify( const ListEvent& Changes )
    throw( RuntimeException )
{
    impl_EnsureNotDisposed();
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

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        for( long i=0; !m_bGotWelcome && i<Changes.Changes.getLength(); i++ )
        {
            ListAction& rAction = aNewEvent.Changes[i];
            switch( rAction.ListActionType )
            {
                case ListActionType::WELCOME:
                {
                    WelcomeDynamicResultSetStruct aWelcome;
                    if( rAction.ActionInfo >>= aWelcome )
                    {
                        impl_InitResultSetOne( aWelcome.Old );
                        impl_InitResultSetTwo( aWelcome.New );
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
    }

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
    throw( AlreadyInitializedException, RuntimeException, std::exception )
{
    impl_EnsureNotDisposed();
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if( m_xSource.is() )
        {
            throw AlreadyInitializedException();
        }
    }

    Reference< XDynamicResultSet > xSourceDynamic( Source, UNO_QUERY );
    OSL_ENSURE( xSourceDynamic.is(),
        "the given source is not of required type XDynamicResultSet" );

    Reference< XDynamicResultSetListener > xListener = NULL;
    Reference< XDynamicResultSetListener > xMyListenerImpl = NULL;

    bool bStatic = false;
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        m_xSource = xSourceDynamic;
        xListener = m_xListener;
        bStatic = m_bStatic;
        xMyListenerImpl = m_xMyListenerImpl;
    }
    if( xListener.is() )
        xSourceDynamic->setListener( m_xMyListenerImpl );
    else if( bStatic )
    {
        Reference< XComponent > xSourceComponent( Source, UNO_QUERY );
        xSourceComponent->addEventListener( Reference< XEventListener > ::query( xMyListenerImpl ) );
    }
    m_aSourceSet.set();
}


// XDynamicResultSet

//virtual
Reference< XResultSet > SAL_CALL DynamicResultSetWrapper::getStaticResultSet()
    throw( ListenerAlreadySetException, RuntimeException, std::exception )
{
    impl_EnsureNotDisposed();

    Reference< XDynamicResultSet > xSource = NULL;
    Reference< XEventListener > xMyListenerImpl = NULL;
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if( m_xListener.is() )
            throw ListenerAlreadySetException();

        xSource = m_xSource;
        m_bStatic = true;
        xMyListenerImpl = Reference< XEventListener > ::query( m_xMyListenerImpl );
    }

    if( xSource.is() )
    {
        xSource->addEventListener( xMyListenerImpl );
    }
    if( !xSource.is() )
        m_aSourceSet.wait();


    Reference< XResultSet > xResultSet = xSource->getStaticResultSet();
    impl_InitResultSetOne( xResultSet );
    return m_xMyResultOne;
}

//virtual
void SAL_CALL DynamicResultSetWrapper::setListener( const Reference< XDynamicResultSetListener > & Listener )
    throw( ListenerAlreadySetException, RuntimeException, std::exception )
{
    impl_EnsureNotDisposed();

    Reference< XDynamicResultSet > xSource = NULL;
    Reference< XDynamicResultSetListener > xMyListenerImpl = NULL;
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if( m_xListener.is() )
            throw ListenerAlreadySetException();
        if( m_bStatic )
            throw ListenerAlreadySetException();

        m_xListener = Listener;
        addEventListener( Reference< XEventListener >::query( Listener ) );

        xSource = m_xSource;
        xMyListenerImpl = m_xMyListenerImpl;
    }
    if ( xSource.is() )
        xSource->setListener( xMyListenerImpl );

    m_aListenerSet.set();
}

//virtual
void SAL_CALL DynamicResultSetWrapper::connectToCache( const Reference< XDynamicResultSet > & xCache )
    throw( ListenerAlreadySetException, AlreadyInitializedException, ServiceNotFoundException, RuntimeException, std::exception )
{
    impl_EnsureNotDisposed();

    if( m_xListener.is() )
        throw ListenerAlreadySetException();
    if( m_bStatic )
        throw ListenerAlreadySetException();

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
                  this, xCache, Sequence< NumberedSortingInfo > (), NULL );
            return;
        }
    }
    OSL_FAIL( "could not connect to cache" );
    throw ServiceNotFoundException();
}

//virtual
sal_Int16 SAL_CALL DynamicResultSetWrapper::getCapabilities()
    throw( RuntimeException, std::exception )
{
    impl_EnsureNotDisposed();

    m_aSourceSet.wait();
    Reference< XDynamicResultSet > xSource = NULL;
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        xSource = m_xSource;
    }
    return xSource->getCapabilities();
}



// class DynamicResultSetWrapperListener



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
    throw()
{
    OWeakObject::acquire();
}

void SAL_CALL DynamicResultSetWrapperListener::release()
    throw()
{
    OWeakObject::release();
}

css::uno::Any SAL_CALL DynamicResultSetWrapperListener::queryInterface( const css::uno::Type & rType )
    throw( css::uno::RuntimeException, std::exception )
{
    css::uno::Any aRet = cppu::queryInterface( rType,
                                               (static_cast< XDynamicResultSetListener* >(this)),
                                               (static_cast< XEventListener* >(this))
                                               );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

// XDynamicResultSetListener methods:

//virtual
void SAL_CALL DynamicResultSetWrapperListener::disposing( const EventObject& rEventObject )
    throw( RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if( m_pOwner )
        m_pOwner->impl_disposing( rEventObject );
}

//virtual
void SAL_CALL DynamicResultSetWrapperListener::notify( const ListEvent& Changes )
    throw( RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if( m_pOwner )
        m_pOwner->impl_notify( Changes );
}


// own methods:


void SAL_CALL DynamicResultSetWrapperListener::impl_OwnerDies()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    m_pOwner = NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
