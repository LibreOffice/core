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








DynamicResultSetWrapper::DynamicResultSetWrapper(
                    Reference< XDynamicResultSet > xOrigin
                    , const Reference< XComponentContext > & rxContext )

                : m_bDisposed( sal_False )
                , m_bInDispose( sal_False )
                , m_pDisposeEventListeners( NULL )
                , m_xContext( rxContext )
                , m_bStatic( sal_False )
                , m_bGotWelcome( sal_False )
                , m_xSource( xOrigin )
                , m_xSourceResultOne( NULL )
                , m_xSourceResultTwo( NULL )
            
            
                , m_xMyResultOne( NULL )
                , m_xMyResultTwo( NULL )
                , m_xListener( NULL )
{
    m_pMyListenerImpl = new DynamicResultSetWrapperListener( this );
    m_xMyListenerImpl = Reference< XDynamicResultSetListener >( m_pMyListenerImpl );
    
};

void SAL_CALL DynamicResultSetWrapper::impl_init()
{
    
    //

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
    

    delete m_pDisposeEventListeners;
};

void SAL_CALL DynamicResultSetWrapper::impl_deinit()
{
    
    //
    m_pMyListenerImpl->impl_OwnerDies();
}

void SAL_CALL DynamicResultSetWrapper
::impl_EnsureNotDisposed()
    throw( DisposedException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    if( m_bDisposed )
        throw DisposedException();
}


void SAL_CALL DynamicResultSetWrapper
::impl_InitResultSetOne( const Reference< XResultSet >& xResultSet )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    OSL_ENSURE( !m_xSourceResultOne.is(), "Source ResultSet One is set already" );
    m_xSourceResultOne = xResultSet;
    m_xMyResultOne = xResultSet;
}


void SAL_CALL DynamicResultSetWrapper
::impl_InitResultSetTwo( const Reference< XResultSet >& xResultSet )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    OSL_ENSURE( !m_xSourceResultTwo.is(), "Source ResultSet Two is set already" );
    m_xSourceResultTwo = xResultSet;
    m_xMyResultTwo = xResultSet;
}





QUERYINTERFACE_IMPL_START( DynamicResultSetWrapper )
    (static_cast< XComponent* >(this)) 
    , (static_cast< XDynamicResultSet* >(this))
    , (static_cast< XSourceInitialization* >(this))
QUERYINTERFACE_IMPL_END





void SAL_CALL DynamicResultSetWrapper
    ::dispose() throw( RuntimeException )
{
    impl_EnsureNotDisposed();

    Reference< XComponent > xSourceComponent;
    {
        osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );
        if( m_bInDispose || m_bDisposed )
            return;
        m_bInDispose = sal_True;

        xSourceComponent = Reference< XComponent >(m_xSource, UNO_QUERY);

        if( m_pDisposeEventListeners && m_pDisposeEventListeners->getLength() )
        {
            EventObject aEvt;
            aEvt.Source = static_cast< XComponent * >( this );

            aGuard.clear();
            m_pDisposeEventListeners->disposeAndClear( aEvt );
        }
    }

    /* 
    if( xSourceComponent.is() )
        xSourceComponent->dispose();
    */

    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    m_bDisposed = sal_True;
    m_bInDispose = sal_False;
}



void SAL_CALL DynamicResultSetWrapper
    ::addEventListener( const Reference< XEventListener >& Listener )
    throw( RuntimeException )
{
    impl_EnsureNotDisposed();
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( !m_pDisposeEventListeners )
        m_pDisposeEventListeners =
                    new OInterfaceContainerHelper( m_aContainerMutex );

    m_pDisposeEventListeners->addInterface( Listener );
}



void SAL_CALL DynamicResultSetWrapper
    ::removeEventListener( const Reference< XEventListener >& Listener )
    throw( RuntimeException )
{
    impl_EnsureNotDisposed();
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( m_pDisposeEventListeners )
        m_pDisposeEventListeners->removeInterface( Listener );
}






void SAL_CALL DynamicResultSetWrapper
    ::impl_disposing( const EventObject& )
    throw( RuntimeException )
{
    impl_EnsureNotDisposed();

    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if( !m_xSource.is() )
        return;

    
    m_xSource.clear();
    m_xSourceResultOne.clear();
    m_xSourceResultTwo.clear();
    
    
}


void SAL_CALL DynamicResultSetWrapper
    ::impl_notify( const ListEvent& Changes )
    throw( RuntimeException )
{
    impl_EnsureNotDisposed();
    
    /*
    <p>The Listener is allowed to blockade this call, until he really want to go
    to the new version. The only situation, where the listener has to return the
    update call at once is, while he disposes his broadcaster or while he is
    removing himsef as listener (otherwise you deadlock)!!!
    */
    

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
                        m_bGotWelcome = sal_True;

                        aWelcome.Old = m_xMyResultOne;
                        aWelcome.New = m_xMyResultTwo;

                         rAction.ActionInfo <<= aWelcome;
                    }
                    else
                    {
                        OSL_FAIL( "ListActionType was WELCOME but ActionInfo didn't contain a WelcomeDynamicResultSetStruct" );
                        
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





void SAL_CALL DynamicResultSetWrapper
    ::setSource( const Reference< XInterface > & Source )
    throw( AlreadyInitializedException, RuntimeException )
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

    sal_Bool bStatic = sal_False;
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





Reference< XResultSet > SAL_CALL DynamicResultSetWrapper
    ::getStaticResultSet()
    throw( ListenerAlreadySetException, RuntimeException )
{
    impl_EnsureNotDisposed();

    Reference< XDynamicResultSet > xSource = NULL;
    Reference< XEventListener > xMyListenerImpl = NULL;
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if( m_xListener.is() )
            throw ListenerAlreadySetException();

        xSource = m_xSource;
        m_bStatic = sal_True;
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


void SAL_CALL DynamicResultSetWrapper
    ::setListener( const Reference<
    XDynamicResultSetListener > & Listener )
    throw( ListenerAlreadySetException, RuntimeException )
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


void SAL_CALL DynamicResultSetWrapper
    ::connectToCache( const Reference< XDynamicResultSet > & xCache )
    throw( ListenerAlreadySetException, AlreadyInitializedException, ServiceNotFoundException, RuntimeException )
{
    impl_EnsureNotDisposed();

    if( m_xListener.is() )
        throw ListenerAlreadySetException();
    if( m_bStatic )
        throw ListenerAlreadySetException();

    Reference< XSourceInitialization > xTarget( xCache, UNO_QUERY );
    OSL_ENSURE( xTarget.is(), "The given Target dosn't have the required interface 'XSourceInitialization'" );
    if( xTarget.is() && m_xContext.is() )
    {
        

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


sal_Int16 SAL_CALL DynamicResultSetWrapper
    ::getCapabilities()
    throw( RuntimeException )
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







DynamicResultSetWrapperListener::DynamicResultSetWrapperListener(
        DynamicResultSetWrapper* pOwner )
        : m_pOwner( pOwner )
{

}

DynamicResultSetWrapperListener::~DynamicResultSetWrapperListener()
{

}





XINTERFACE_IMPL_2( DynamicResultSetWrapperListener
                   , XDynamicResultSetListener
                   , XEventListener 
                   );





void SAL_CALL DynamicResultSetWrapperListener
    ::disposing( const EventObject& rEventObject )
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if( m_pOwner )
        m_pOwner->impl_disposing( rEventObject );
}


void SAL_CALL DynamicResultSetWrapperListener
    ::notify( const ListEvent& Changes )
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if( m_pOwner )
        m_pOwner->impl_notify( Changes );
}





void SAL_CALL DynamicResultSetWrapperListener
    ::impl_OwnerDies()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    m_pOwner = NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
