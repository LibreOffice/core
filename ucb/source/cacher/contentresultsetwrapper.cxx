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


#include <contentresultsetwrapper.hxx>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <com/sun/star/ucb/FetchError.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <rtl/ustring.hxx>
#include <osl/diagnose.h>

using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace cppu;








ContentResultSetWrapper::ContentResultSetWrapper(
                                Reference< XResultSet > xOrigin )
                : m_xResultSetOrigin( xOrigin )
                , m_xRowOrigin( NULL )
                , m_xContentAccessOrigin( NULL )
                , m_xPropertySetOrigin( NULL )
                , m_xPropertySetInfo( NULL )
                , m_nForwardOnly( 2 )
                , m_xMetaDataFromOrigin( NULL )
                , m_bDisposed( sal_False )
                , m_bInDispose( sal_False )
                , m_pDisposeEventListeners( NULL )
                , m_pPropertyChangeListeners( NULL )
                , m_pVetoableChangeListeners( NULL )
{
    m_pMyListenerImpl = new ContentResultSetWrapperListener( this );
    m_xMyListenerImpl = Reference< XPropertyChangeListener >( m_pMyListenerImpl );

    OSL_ENSURE( m_xResultSetOrigin.is(), "XResultSet is required" );

    
};


void SAL_CALL ContentResultSetWrapper::impl_init_xRowOrigin()
{
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if(m_xRowOrigin.is())
            return;
    }

    Reference< XRow > xOrgig =
        Reference< XRow >( m_xResultSetOrigin, UNO_QUERY );

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        m_xRowOrigin = xOrgig;
        OSL_ENSURE( m_xRowOrigin.is(), "interface XRow is required" );
    }
}

void SAL_CALL ContentResultSetWrapper::impl_init_xContentAccessOrigin()
{
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if(m_xContentAccessOrigin.is())
            return;
    }

    Reference< XContentAccess > xOrgig =
        Reference< XContentAccess >( m_xResultSetOrigin, UNO_QUERY );

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        m_xContentAccessOrigin = xOrgig;
        OSL_ENSURE( m_xContentAccessOrigin.is(), "interface XContentAccess is required" );
    }
}


void SAL_CALL ContentResultSetWrapper::impl_init_xPropertySetOrigin()
{
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if( m_xPropertySetOrigin.is() )
            return;
    }

    Reference< XPropertySet > xOrig =
        Reference< XPropertySet >( m_xResultSetOrigin, UNO_QUERY );

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        m_xPropertySetOrigin = xOrig;
        OSL_ENSURE( m_xPropertySetOrigin.is(), "interface XPropertySet is required" );
    }
}

void SAL_CALL ContentResultSetWrapper::impl_init()
{
    
    //

    
    Reference< XComponent > xComponentOrigin( m_xResultSetOrigin, UNO_QUERY );
    OSL_ENSURE( xComponentOrigin.is(), "interface XComponent is required" );
    xComponentOrigin->addEventListener( static_cast< XPropertyChangeListener * >( m_pMyListenerImpl ) );
}

ContentResultSetWrapper::~ContentResultSetWrapper()
{
    

    delete m_pDisposeEventListeners;
    delete m_pPropertyChangeListeners;
    delete m_pVetoableChangeListeners;
};

void SAL_CALL ContentResultSetWrapper::impl_deinit()
{
    
    //
    m_pMyListenerImpl->impl_OwnerDies();
}


void SAL_CALL ContentResultSetWrapper
    ::impl_initPropertySetInfo()
{
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if( m_xPropertySetInfo.is() )
            return;

        impl_init_xPropertySetOrigin();
        if( !m_xPropertySetOrigin.is() )
            return;
    }

    Reference< XPropertySetInfo > xOrig =
            m_xPropertySetOrigin->getPropertySetInfo();

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        m_xPropertySetInfo = xOrig;
    }
}

void SAL_CALL ContentResultSetWrapper
::impl_EnsureNotDisposed()
    throw( DisposedException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    if( m_bDisposed )
        throw DisposedException();
}

ContentResultSetWrapper::PropertyChangeListenerContainer_Impl* SAL_CALL
    ContentResultSetWrapper
    ::impl_getPropertyChangeListenerContainer()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    if ( !m_pPropertyChangeListeners )
        m_pPropertyChangeListeners =
            new PropertyChangeListenerContainer_Impl( m_aContainerMutex );
    return m_pPropertyChangeListeners;
}

ContentResultSetWrapper::PropertyChangeListenerContainer_Impl* SAL_CALL
    ContentResultSetWrapper
    ::impl_getVetoableChangeListenerContainer()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    if ( !m_pVetoableChangeListeners )
        m_pVetoableChangeListeners =
            new PropertyChangeListenerContainer_Impl( m_aContainerMutex );
    return m_pVetoableChangeListeners;
}

void SAL_CALL ContentResultSetWrapper
    ::impl_notifyPropertyChangeListeners(
                    const PropertyChangeEvent& rEvt )
{
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if( !m_pPropertyChangeListeners )
            return;
    }

    
    OInterfaceContainerHelper* pContainer =
            m_pPropertyChangeListeners->getContainer( rEvt.PropertyName );
    if( pContainer )
    {
        OInterfaceIteratorHelper aIter( *pContainer );
        while( aIter.hasMoreElements() )
        {
            Reference< XPropertyChangeListener > xListener(
                                                    aIter.next(), UNO_QUERY );
            if( xListener.is() )
                xListener->propertyChange( rEvt );
        }
    }

    
    pContainer = m_pPropertyChangeListeners->getContainer( OUString() );
    if( pContainer )
    {
        OInterfaceIteratorHelper aIter( *pContainer );
        while( aIter.hasMoreElements() )
        {
            Reference< XPropertyChangeListener > xListener(
                                                    aIter.next(), UNO_QUERY );
            if( xListener.is() )
                xListener->propertyChange( rEvt );
        }
    }
}

void SAL_CALL ContentResultSetWrapper
    ::impl_notifyVetoableChangeListeners( const PropertyChangeEvent& rEvt )
    throw( PropertyVetoException,
           RuntimeException )
{
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if( !m_pVetoableChangeListeners )
            return;
    }

    
    OInterfaceContainerHelper* pContainer =
            m_pVetoableChangeListeners->getContainer( rEvt.PropertyName );
    if( pContainer )
    {
        OInterfaceIteratorHelper aIter( *pContainer );
        while( aIter.hasMoreElements() )
        {
            Reference< XVetoableChangeListener > xListener(
                                                    aIter.next(), UNO_QUERY );
            if( xListener.is() )
                xListener->vetoableChange( rEvt );
        }
    }

    
    pContainer = m_pVetoableChangeListeners->getContainer( OUString() );
    if( pContainer )
    {
        OInterfaceIteratorHelper aIter( *pContainer );
        while( aIter.hasMoreElements() )
        {
            Reference< XVetoableChangeListener > xListener(
                                                    aIter.next(), UNO_QUERY );
            if( xListener.is() )
                xListener->vetoableChange( rEvt );
        }
    }
}

sal_Bool SAL_CALL ContentResultSetWrapper
    ::impl_isForwardOnly()
{
    
    
    

    
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    m_nForwardOnly = 0;
    return false;


    /*
    ReacquireableGuard aGuard( m_aMutex );
    if( m_nForwardOnly == 2 )
    {
        aGuard.clear();
        if( !getPropertySetInfo().is() )
        {
            aGuard.reacquire();
            m_nForwardOnly = 0;
            return m_nForwardOnly;
        }
        aGuard.reacquire();

        OUString aName("ResultSetType");
        

        impl_init_xPropertySetOrigin();
        if( !m_xPropertySetOrigin.is() )
        {
            OSL_FAIL( "broadcaster was disposed already" );
            m_nForwardOnly = 0;
            return m_nForwardOnly;
        }

        aGuard.clear();
        Any aAny = m_xPropertySetOrigin->getPropertyValue( aName );

        aGuard.reacquire();
        long nResultSetType;
        if( ( aAny >>= nResultSetType ) &&
            ( nResultSetType == ResultSetType::FORWARD_ONLY ) )
            m_nForwardOnly = 1;
        else
            m_nForwardOnly = 0;
    }
    return m_nForwardOnly;
    */
}





QUERYINTERFACE_IMPL_START( ContentResultSetWrapper )

    (static_cast< XComponent* >(this)),
    (static_cast< XCloseable* >(this)),
    (static_cast< XResultSetMetaDataSupplier* >(this)),
    (static_cast< XPropertySet* >(this)),

    (static_cast< XContentAccess* >(this)),
    (static_cast< XResultSet* >(this)),
    (static_cast< XRow* >(this))

QUERYINTERFACE_IMPL_END





void SAL_CALL ContentResultSetWrapper
    ::dispose() throw( RuntimeException )
{
    impl_EnsureNotDisposed();

    ReacquireableGuard aGuard( m_aMutex );
    if( m_bInDispose || m_bDisposed )
        return;
    m_bInDispose = sal_True;

    if( m_xPropertySetOrigin.is() )
    {
        aGuard.clear();
        try
        {
            m_xPropertySetOrigin->removePropertyChangeListener(
                OUString(), static_cast< XPropertyChangeListener * >( m_pMyListenerImpl ) );
        }
        catch( Exception& )
        {
            OSL_FAIL( "could not remove PropertyChangeListener" );
        }
        try
        {
            m_xPropertySetOrigin->removeVetoableChangeListener(
                OUString(), static_cast< XVetoableChangeListener * >( m_pMyListenerImpl ) );
        }
        catch( Exception& )
        {
            OSL_FAIL( "could not remove VetoableChangeListener" );
        }

        Reference< XComponent > xComponentOrigin( m_xResultSetOrigin, UNO_QUERY );
        OSL_ENSURE( xComponentOrigin.is(), "interface XComponent is required" );
        xComponentOrigin->removeEventListener( static_cast< XPropertyChangeListener * >( m_pMyListenerImpl ) );
    }

    aGuard.reacquire();
    if( m_pDisposeEventListeners && m_pDisposeEventListeners->getLength() )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XComponent * >( this );

        aGuard.clear();
        m_pDisposeEventListeners->disposeAndClear( aEvt );
    }

    aGuard.reacquire();
    if( m_pPropertyChangeListeners )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XPropertySet * >( this );

        aGuard.clear();
        m_pPropertyChangeListeners->disposeAndClear( aEvt );
    }

    aGuard.reacquire();
    if( m_pVetoableChangeListeners )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XPropertySet * >( this );

        aGuard.clear();
        m_pVetoableChangeListeners->disposeAndClear( aEvt );
    }

    aGuard.reacquire();
    m_bDisposed = sal_True;
    m_bInDispose = sal_False;
}



void SAL_CALL ContentResultSetWrapper
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



void SAL_CALL ContentResultSetWrapper
    ::removeEventListener( const Reference< XEventListener >& Listener )
    throw( RuntimeException )
{
    impl_EnsureNotDisposed();
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( m_pDisposeEventListeners )
        m_pDisposeEventListeners->removeInterface( Listener );
}





void SAL_CALL ContentResultSetWrapper
    ::close()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();
    dispose();
}





Reference< XResultSetMetaData > SAL_CALL ContentResultSetWrapper
    ::getMetaData()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    ReacquireableGuard aGuard( m_aMutex );
    if( !m_xMetaDataFromOrigin.is() && m_xResultSetOrigin.is() )
    {
        Reference< XResultSetMetaDataSupplier > xMetaDataSupplier
            = Reference< XResultSetMetaDataSupplier >(
                m_xResultSetOrigin, UNO_QUERY );

        if( xMetaDataSupplier.is() )
        {
            aGuard.clear();

            Reference< XResultSetMetaData > xMetaData
                = xMetaDataSupplier->getMetaData();

            aGuard.reacquire();
            m_xMetaDataFromOrigin = xMetaData;
        }
    }
    return m_xMetaDataFromOrigin;
}






Reference< XPropertySetInfo > SAL_CALL ContentResultSetWrapper
    ::getPropertySetInfo() throw( RuntimeException )
{
    impl_EnsureNotDisposed();
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if( m_xPropertySetInfo.is() )
            return m_xPropertySetInfo;
    }
    impl_initPropertySetInfo();
    return m_xPropertySetInfo;
}


void SAL_CALL ContentResultSetWrapper
    ::setPropertyValue( const OUString& rPropertyName, const Any& rValue )
    throw( UnknownPropertyException,
           PropertyVetoException,
           IllegalArgumentException,
           WrappedTargetException,
           RuntimeException )
{
    impl_EnsureNotDisposed();
    impl_init_xPropertySetOrigin();
    if( !m_xPropertySetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw UnknownPropertyException();
    }
    m_xPropertySetOrigin->setPropertyValue( rPropertyName, rValue );
}



Any SAL_CALL ContentResultSetWrapper
    ::getPropertyValue( const OUString& rPropertyName )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
    impl_EnsureNotDisposed();
    impl_init_xPropertySetOrigin();
    if( !m_xPropertySetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw UnknownPropertyException();
    }
    return m_xPropertySetOrigin->getPropertyValue( rPropertyName );
}



void SAL_CALL ContentResultSetWrapper
    ::addPropertyChangeListener(
            const OUString& aPropertyName,
            const Reference< XPropertyChangeListener >& xListener )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !getPropertySetInfo().is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw UnknownPropertyException();
    }

    if( !aPropertyName.isEmpty() )
    {
        m_xPropertySetInfo->getPropertyByName( aPropertyName );
        
    }

    impl_getPropertyChangeListenerContainer();
    sal_Bool bNeedRegister = !m_pPropertyChangeListeners->
                        getContainedTypes().getLength();
    m_pPropertyChangeListeners->addInterface( aPropertyName, xListener );
    if( bNeedRegister )
    {
        impl_init_xPropertySetOrigin();
        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            if( !m_xPropertySetOrigin.is() )
            {
                OSL_FAIL( "broadcaster was disposed already" );
                return;
            }
        }
        try
        {
            m_xPropertySetOrigin->addPropertyChangeListener(
                OUString(), static_cast< XPropertyChangeListener * >( m_pMyListenerImpl ) );
        }
        catch( Exception& )
        {
            m_pPropertyChangeListeners->removeInterface( aPropertyName, xListener );
            throw;
        }
    }
}



void SAL_CALL ContentResultSetWrapper
    ::addVetoableChangeListener(
            const OUString& rPropertyName,
            const Reference< XVetoableChangeListener >& xListener )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !getPropertySetInfo().is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw UnknownPropertyException();
    }
    if( !rPropertyName.isEmpty() )
    {
        m_xPropertySetInfo->getPropertyByName( rPropertyName );
        
    }

    impl_getVetoableChangeListenerContainer();
    sal_Bool bNeedRegister = !m_pVetoableChangeListeners->
                        getContainedTypes().getLength();
    m_pVetoableChangeListeners->addInterface( rPropertyName, xListener );
    if( bNeedRegister )
    {
        impl_init_xPropertySetOrigin();
        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            if( !m_xPropertySetOrigin.is() )
            {
                OSL_FAIL( "broadcaster was disposed already" );
                return;
            }
        }
        try
        {
            m_xPropertySetOrigin->addVetoableChangeListener(
                OUString(), static_cast< XVetoableChangeListener * >( m_pMyListenerImpl ) );
        }
        catch( Exception& )
        {
            m_pVetoableChangeListeners->removeInterface( rPropertyName, xListener );
            throw;
        }
    }
}



void SAL_CALL ContentResultSetWrapper
    ::removePropertyChangeListener(
            const OUString& rPropertyName,
            const Reference< XPropertyChangeListener >& xListener )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    {
        
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if( !m_pPropertyChangeListeners )
            return;
    }
    OInterfaceContainerHelper* pContainer =
        m_pPropertyChangeListeners->getContainer( rPropertyName );

    if( !pContainer )
    {
        if( !rPropertyName.isEmpty() )
        {
            if( !getPropertySetInfo().is() )
                throw UnknownPropertyException();

            m_xPropertySetInfo->getPropertyByName( rPropertyName );
            
        }
        return; 
    }

    m_pPropertyChangeListeners->removeInterface( rPropertyName, xListener );

    if( !m_pPropertyChangeListeners->getContainedTypes().getLength() )
    {
        impl_init_xPropertySetOrigin();
        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            if( !m_xPropertySetOrigin.is() )
            {
                OSL_FAIL( "broadcaster was disposed already" );
                return;
            }
        }
        try
        {
            m_xPropertySetOrigin->removePropertyChangeListener(
                OUString(), static_cast< XPropertyChangeListener * >( m_pMyListenerImpl ) );
        }
        catch( Exception& )
        {
            OSL_FAIL( "could not remove PropertyChangeListener" );
        }
    }
}



void SAL_CALL ContentResultSetWrapper
    ::removeVetoableChangeListener(
            const OUString& rPropertyName,
            const Reference< XVetoableChangeListener >& xListener )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    {
        
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if( !m_pVetoableChangeListeners )
            return;
    }
    OInterfaceContainerHelper* pContainer =
        m_pVetoableChangeListeners->getContainer( rPropertyName );

    if( !pContainer )
    {
        if( !rPropertyName.isEmpty() )
        {
            if( !getPropertySetInfo().is() )
                throw UnknownPropertyException();

            m_xPropertySetInfo->getPropertyByName( rPropertyName );
            
        }
        return; 
    }

    m_pVetoableChangeListeners->removeInterface( rPropertyName, xListener );

    if( !m_pVetoableChangeListeners->getContainedTypes().getLength() )
    {
        impl_init_xPropertySetOrigin();
        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            if( !m_xPropertySetOrigin.is() )
            {
                OSL_FAIL( "broadcaster was disposed already" );
                return;
            }
        }
        try
        {
            m_xPropertySetOrigin->removeVetoableChangeListener(
                OUString(), static_cast< XVetoableChangeListener * >( m_pMyListenerImpl ) );
        }
        catch( Exception& )
        {
            OSL_FAIL( "could not remove VetoableChangeListener" );
        }
    }
}






void SAL_CALL ContentResultSetWrapper
    ::impl_disposing( const EventObject& )
    throw( RuntimeException )
{
    impl_EnsureNotDisposed();

    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if( !m_xResultSetOrigin.is() )
        return;

    
    m_xResultSetOrigin.clear();
    if(m_xRowOrigin.is())
        m_xRowOrigin.clear();
    if(m_xContentAccessOrigin.is())
        m_xContentAccessOrigin.clear();
    if(m_xPropertySetOrigin.is())
        m_xPropertySetOrigin.clear();
    m_xMetaDataFromOrigin.clear();
    if(m_xPropertySetInfo.is())
        m_xPropertySetInfo.clear();
}


void SAL_CALL ContentResultSetWrapper
    ::impl_propertyChange( const PropertyChangeEvent& rEvt )
    throw( RuntimeException )
{
    impl_EnsureNotDisposed();

    PropertyChangeEvent aEvt( rEvt );
    aEvt.Source = static_cast< XPropertySet * >( this );
    aEvt.Further = sal_False;
    impl_notifyPropertyChangeListeners( aEvt );
}


void SAL_CALL ContentResultSetWrapper
    ::impl_vetoableChange( const PropertyChangeEvent& rEvt )
    throw( PropertyVetoException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    PropertyChangeEvent aEvt( rEvt );
    aEvt.Source = static_cast< XPropertySet * >( this );
    aEvt.Further = sal_False;

    impl_notifyVetoableChangeListeners( aEvt );
}






OUString SAL_CALL ContentResultSetWrapper
    ::queryContentIdentifierString()
    throw( RuntimeException )
{
    impl_EnsureNotDisposed();
    impl_init_xContentAccessOrigin();
    if( !m_xContentAccessOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xContentAccessOrigin->queryContentIdentifierString();
}



Reference< XContentIdentifier > SAL_CALL ContentResultSetWrapper
    ::queryContentIdentifier()
    throw( RuntimeException )
{
    impl_EnsureNotDisposed();
    impl_init_xContentAccessOrigin();
    if( !m_xContentAccessOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xContentAccessOrigin->queryContentIdentifier();
}



Reference< XContent > SAL_CALL ContentResultSetWrapper
    ::queryContent()
    throw( RuntimeException )
{
    impl_EnsureNotDisposed();
    impl_init_xContentAccessOrigin();
    if( !m_xContentAccessOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xContentAccessOrigin->queryContent();
}






sal_Bool SAL_CALL ContentResultSetWrapper
    ::next()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->next();
}


sal_Bool SAL_CALL ContentResultSetWrapper
    ::previous()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->previous();
}


sal_Bool SAL_CALL ContentResultSetWrapper
    ::absolute( sal_Int32 row )
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->absolute( row );
}


sal_Bool SAL_CALL ContentResultSetWrapper
    ::relative( sal_Int32 rows )
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->relative( rows );
}



sal_Bool SAL_CALL ContentResultSetWrapper
    ::first()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->first();
}


sal_Bool SAL_CALL ContentResultSetWrapper
    ::last()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->last();
}


void SAL_CALL ContentResultSetWrapper
    ::beforeFirst()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    m_xResultSetOrigin->beforeFirst();
}


void SAL_CALL ContentResultSetWrapper
    ::afterLast()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    m_xResultSetOrigin->afterLast();
}


sal_Bool SAL_CALL ContentResultSetWrapper
    ::isAfterLast()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->isAfterLast();
}


sal_Bool SAL_CALL ContentResultSetWrapper
    ::isBeforeFirst()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->isBeforeFirst();
}


sal_Bool SAL_CALL ContentResultSetWrapper
    ::isFirst()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->isFirst();
}


sal_Bool SAL_CALL ContentResultSetWrapper
    ::isLast()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->isLast();
}



sal_Int32 SAL_CALL ContentResultSetWrapper
    ::getRow()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->getRow();
}


void SAL_CALL ContentResultSetWrapper
    ::refreshRow()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    m_xResultSetOrigin->refreshRow();
}


sal_Bool SAL_CALL ContentResultSetWrapper
    ::rowUpdated()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->rowUpdated();
}

sal_Bool SAL_CALL ContentResultSetWrapper
    ::rowInserted()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->rowInserted();
}


sal_Bool SAL_CALL ContentResultSetWrapper
    ::rowDeleted()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->rowDeleted();
}


Reference< XInterface > SAL_CALL ContentResultSetWrapper
    ::getStatement()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();
    
    return Reference< XInterface >();
}





#define XROW_GETXXX( getXXX )                                   \
impl_EnsureNotDisposed();                                       \
impl_init_xRowOrigin();                                         \
if( !m_xRowOrigin.is() )                                        \
{                                                               \
    OSL_FAIL( "broadcaster was disposed already" );\
    throw RuntimeException();                                   \
}                                                               \
return m_xRowOrigin->getXXX( columnIndex );


sal_Bool SAL_CALL ContentResultSetWrapper
    ::wasNull()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();
    impl_init_xRowOrigin();
    if( !m_xRowOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xRowOrigin->wasNull();
}


OUString SAL_CALL ContentResultSetWrapper
    ::getString( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getString );
}


sal_Bool SAL_CALL ContentResultSetWrapper
    ::getBoolean( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getBoolean );
}


sal_Int8 SAL_CALL ContentResultSetWrapper
    ::getByte( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getByte );
}


sal_Int16 SAL_CALL ContentResultSetWrapper
    ::getShort( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getShort );
}


sal_Int32 SAL_CALL ContentResultSetWrapper
    ::getInt( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getInt );
}


sal_Int64 SAL_CALL ContentResultSetWrapper
    ::getLong( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getLong );
}


float SAL_CALL ContentResultSetWrapper
    ::getFloat( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getFloat );
}


double SAL_CALL ContentResultSetWrapper
    ::getDouble( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getDouble );
}


Sequence< sal_Int8 > SAL_CALL ContentResultSetWrapper
    ::getBytes( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getBytes );
}


Date SAL_CALL ContentResultSetWrapper
    ::getDate( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getDate );
}


Time SAL_CALL ContentResultSetWrapper
    ::getTime( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getTime );
}


DateTime SAL_CALL ContentResultSetWrapper
    ::getTimestamp( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getTimestamp );
}


Reference< com::sun::star::io::XInputStream >
    SAL_CALL ContentResultSetWrapper
    ::getBinaryStream( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getBinaryStream );
}


Reference< com::sun::star::io::XInputStream >
    SAL_CALL ContentResultSetWrapper
    ::getCharacterStream( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getCharacterStream );
}


Any SAL_CALL ContentResultSetWrapper
    ::getObject( sal_Int32 columnIndex,
           const Reference<
            com::sun::star::container::XNameAccess >& typeMap )
    throw( SQLException,
           RuntimeException )
{
    
    

    impl_EnsureNotDisposed();
    impl_init_xRowOrigin();
    if( !m_xRowOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xRowOrigin->getObject( columnIndex, typeMap );
}


Reference< XRef > SAL_CALL ContentResultSetWrapper
    ::getRef( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getRef );
}


Reference< XBlob > SAL_CALL ContentResultSetWrapper
    ::getBlob( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getBlob );
}


Reference< XClob > SAL_CALL ContentResultSetWrapper
    ::getClob( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getClob );
}


Reference< XArray > SAL_CALL ContentResultSetWrapper
    ::getArray( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getArray );
}







ContentResultSetWrapperListener::ContentResultSetWrapperListener(
    ContentResultSetWrapper* pOwner )
    : m_pOwner( pOwner )
{
}

ContentResultSetWrapperListener::~ContentResultSetWrapperListener()
{
}





XINTERFACE_COMMON_IMPL( ContentResultSetWrapperListener )
QUERYINTERFACE_IMPL_START( ContentResultSetWrapperListener )

    static_cast< XEventListener * >(
                     static_cast< XPropertyChangeListener * >(this))
    , (static_cast< XPropertyChangeListener* >(this))
    , (static_cast< XVetoableChangeListener* >(this))

QUERYINTERFACE_IMPL_END







void SAL_CALL ContentResultSetWrapperListener
    ::disposing( const EventObject& rEventObject )
    throw( RuntimeException )
{
    if( m_pOwner )
        m_pOwner->impl_disposing( rEventObject );
}






void SAL_CALL ContentResultSetWrapperListener
    ::propertyChange( const PropertyChangeEvent& rEvt )
    throw( RuntimeException )
{
    if( m_pOwner )
        m_pOwner->impl_propertyChange( rEvt );
}





void SAL_CALL ContentResultSetWrapperListener
    ::vetoableChange( const PropertyChangeEvent& rEvt )
    throw( PropertyVetoException,
           RuntimeException )
{
    if( m_pOwner )
        m_pOwner->impl_vetoableChange( rEvt );
}

void SAL_CALL ContentResultSetWrapperListener
    ::impl_OwnerDies()
{
    m_pOwner = NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
