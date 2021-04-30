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


#include "contentresultsetwrapper.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <rtl/ustring.hxx>
#include <osl/diagnose.h>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/interfacecontainer.hxx>

using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace comphelper;
using namespace cppu;




ContentResultSetWrapper::ContentResultSetWrapper(
                                Reference< XResultSet > const & xOrigin )
                : m_xResultSetOrigin( xOrigin )
                , m_nForwardOnly( 2 )
                , m_bDisposed( false )
                , m_bInDispose( false )
{
    m_xMyListenerImpl = new ContentResultSetWrapperListener( this );

    OSL_ENSURE( m_xResultSetOrigin.is(), "XResultSet is required" );

    //!! call impl_init() at the end of constructor of derived class
};


void ContentResultSetWrapper::impl_init_xRowOrigin()
{
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if(m_xRowOrigin.is())
            return;
    }

    Reference< XRow > xOrgig( m_xResultSetOrigin, UNO_QUERY );

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        m_xRowOrigin = xOrgig;
        OSL_ENSURE( m_xRowOrigin.is(), "interface XRow is required" );
    }
}

void ContentResultSetWrapper::impl_init_xContentAccessOrigin()
{
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if(m_xContentAccessOrigin.is())
            return;
    }

    Reference< XContentAccess > xOrgig( m_xResultSetOrigin, UNO_QUERY );

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        m_xContentAccessOrigin = xOrgig;
        OSL_ENSURE( m_xContentAccessOrigin.is(), "interface XContentAccess is required" );
    }
}


void ContentResultSetWrapper::impl_init_xPropertySetOrigin()
{
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if( m_xPropertySetOrigin.is() )
            return;
    }

    Reference< XPropertySet > xOrig( m_xResultSetOrigin, UNO_QUERY );

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        m_xPropertySetOrigin = xOrig;
        OSL_ENSURE( m_xPropertySetOrigin.is(), "interface XPropertySet is required" );
    }
}

void ContentResultSetWrapper::impl_init()
{
    //call this at the end of constructor of derived class


    //listen to disposing from Origin:
    Reference< XComponent > xComponentOrigin( m_xResultSetOrigin, UNO_QUERY );
    OSL_ENSURE( xComponentOrigin.is(), "interface XComponent is required" );
    xComponentOrigin->addEventListener( static_cast< XPropertyChangeListener * >( m_xMyListenerImpl.get() ) );
}

ContentResultSetWrapper::~ContentResultSetWrapper()
{
    //call impl_deinit() at start of destructor of derived class
};

void ContentResultSetWrapper::impl_deinit()
{
    //call this at start of destructor of derived class

    m_xMyListenerImpl->impl_OwnerDies();
}

//virtual
void ContentResultSetWrapper::impl_initPropertySetInfo()
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

void ContentResultSetWrapper::impl_EnsureNotDisposed()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    if( m_bDisposed )
        throw DisposedException();
}

void ContentResultSetWrapper::impl_getPropertyChangeListenerContainer()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    if ( !m_pPropertyChangeListeners )
        m_pPropertyChangeListeners.reset(
            new PropertyChangeListenerContainer_Impl( m_aContainerMutex ) );
}

void ContentResultSetWrapper::impl_getVetoableChangeListenerContainer()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    if ( !m_pVetoableChangeListeners )
        m_pVetoableChangeListeners.reset(
            new PropertyChangeListenerContainer_Impl( m_aContainerMutex ) );
}

void ContentResultSetWrapper::impl_notifyPropertyChangeListeners( const PropertyChangeEvent& rEvt )
{
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if( !m_pPropertyChangeListeners )
            return;
    }

    // Notify listeners interested especially in the changed property.
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

    // Notify listeners interested in all properties.
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

void ContentResultSetWrapper::impl_notifyVetoableChangeListeners( const PropertyChangeEvent& rEvt )
{
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if( !m_pVetoableChangeListeners )
            return;
    }

    // Notify listeners interested especially in the changed property.
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

    // Notify listeners interested in all properties.
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

bool ContentResultSetWrapper::impl_isForwardOnly()
{
    //m_nForwardOnly == 2 -> don't know
    //m_nForwardOnly == 1 -> YES
    //m_nForwardOnly == 0 -> NO

    //@todo replace this with lines in comment
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
        //find out, if we are ForwardOnly and cache the value:

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


// XInterface methods.

css::uno::Any SAL_CALL ContentResultSetWrapper::queryInterface( const css::uno::Type & rType )
{
    //list all interfaces inclusive baseclasses of interfaces
    css::uno::Any aRet = cppu::queryInterface( rType,
                                               static_cast< XComponent* >(this),
                                               static_cast< XCloseable* >(this),
                                               static_cast< XResultSetMetaDataSupplier* >(this),
                                               static_cast< XPropertySet* >(this),
                                               static_cast< XContentAccess* >(this),
                                               static_cast< XResultSet* >(this),
                                               static_cast< XRow* >(this)
                                               );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

// XComponent methods.

// virtual
void SAL_CALL ContentResultSetWrapper::dispose()
{
    impl_EnsureNotDisposed();

    bool isCleared = false;
    osl::ResettableMutexGuard aGuard(m_aMutex);
    if( m_bInDispose || m_bDisposed )
        return;
    m_bInDispose = true;

    if( m_xPropertySetOrigin.is() )
    {
        aGuard.clear();
        isCleared = true;
        try
        {
            m_xPropertySetOrigin->removePropertyChangeListener(
                OUString(), static_cast< XPropertyChangeListener * >( m_xMyListenerImpl.get() ) );
        }
        catch( Exception& )
        {
            OSL_FAIL( "could not remove PropertyChangeListener" );
        }
        try
        {
            m_xPropertySetOrigin->removeVetoableChangeListener(
                OUString(), static_cast< XVetoableChangeListener * >( m_xMyListenerImpl.get() ) );
        }
        catch( Exception& )
        {
            OSL_FAIL( "could not remove VetoableChangeListener" );
        }

        Reference< XComponent > xComponentOrigin( m_xResultSetOrigin, UNO_QUERY );
        OSL_ENSURE( xComponentOrigin.is(), "interface XComponent is required" );
        xComponentOrigin->removeEventListener( static_cast< XPropertyChangeListener * >( m_xMyListenerImpl.get() ) );
    }

    if (isCleared)
    {
        aGuard.reset();
        isCleared = false;
    }
    if( m_pDisposeEventListeners && m_pDisposeEventListeners->getLength() )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XComponent * >( this );

        aGuard.clear();
        isCleared = true;
        m_pDisposeEventListeners->disposeAndClear( aEvt );
    }

    if (isCleared)
    {
        aGuard.reset();
        isCleared = false;
    }
    if( m_pPropertyChangeListeners )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XPropertySet * >( this );

        aGuard.clear();
        isCleared = true;
        m_pPropertyChangeListeners->disposeAndClear( aEvt );
    }

    if (isCleared)
    {
        aGuard.reset();
        isCleared = false;
    }
    if( m_pVetoableChangeListeners )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XPropertySet * >( this );

        aGuard.clear();
        isCleared = true;
        m_pVetoableChangeListeners->disposeAndClear( aEvt );
    }

    if (isCleared)
    {
        aGuard.reset();
    }
    m_bDisposed = true;
    m_bInDispose = false;
}


// virtual
void SAL_CALL ContentResultSetWrapper::addEventListener( const Reference< XEventListener >& Listener )
{
    impl_EnsureNotDisposed();
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( !m_pDisposeEventListeners )
        m_pDisposeEventListeners.reset(
                    new OInterfaceContainerHelper2( m_aContainerMutex ) );

    m_pDisposeEventListeners->addInterface( Listener );
}


// virtual
void SAL_CALL ContentResultSetWrapper::removeEventListener( const Reference< XEventListener >& Listener )
{
    impl_EnsureNotDisposed();
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( m_pDisposeEventListeners )
        m_pDisposeEventListeners->removeInterface( Listener );
}


//XCloseable methods.

//virtual
void SAL_CALL ContentResultSetWrapper::close()
{
    impl_EnsureNotDisposed();
    dispose();
}


//XResultSetMetaDataSupplier methods.

//virtual
Reference< XResultSetMetaData > SAL_CALL ContentResultSetWrapper::getMetaData()
{
    impl_EnsureNotDisposed();

    osl::ResettableMutexGuard aGuard(m_aMutex);
    if( !m_xMetaDataFromOrigin.is() && m_xResultSetOrigin.is() )
    {
        Reference< XResultSetMetaDataSupplier > xMetaDataSupplier(
                m_xResultSetOrigin, UNO_QUERY );

        if( xMetaDataSupplier.is() )
        {
            aGuard.clear();

            Reference< XResultSetMetaData > xMetaData
                = xMetaDataSupplier->getMetaData();

            aGuard.reset();
            m_xMetaDataFromOrigin = xMetaData;
        }
    }
    return m_xMetaDataFromOrigin;
}


// XPropertySet methods.

// virtual
Reference< XPropertySetInfo > SAL_CALL ContentResultSetWrapper::getPropertySetInfo()
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

// virtual
void SAL_CALL ContentResultSetWrapper::setPropertyValue( const OUString& rPropertyName, const Any& rValue )
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


// virtual
Any SAL_CALL ContentResultSetWrapper::getPropertyValue( const OUString& rPropertyName )
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


// virtual
void SAL_CALL ContentResultSetWrapper::addPropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener )
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
        //throws UnknownPropertyException, if so
    }

    impl_getPropertyChangeListenerContainer();
    bool bNeedRegister = !m_pPropertyChangeListeners->
                        getContainedTypes().hasElements();
    m_pPropertyChangeListeners->addInterface( aPropertyName, xListener );
    if( !bNeedRegister )
        return;

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
            OUString(), static_cast< XPropertyChangeListener * >( m_xMyListenerImpl.get() ) );
    }
    catch( Exception& )
    {
        m_pPropertyChangeListeners->removeInterface( aPropertyName, xListener );
        throw;
    }
}


// virtual
void SAL_CALL ContentResultSetWrapper::addVetoableChangeListener( const OUString& rPropertyName, const Reference< XVetoableChangeListener >& xListener )
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
        //throws UnknownPropertyException, if so
    }

    impl_getVetoableChangeListenerContainer();
    bool bNeedRegister = !m_pVetoableChangeListeners->
                        getContainedTypes().hasElements();
    m_pVetoableChangeListeners->addInterface( rPropertyName, xListener );
    if( !bNeedRegister )
        return;

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
            OUString(), static_cast< XVetoableChangeListener * >( m_xMyListenerImpl.get() ) );
    }
    catch( Exception& )
    {
        m_pVetoableChangeListeners->removeInterface( rPropertyName, xListener );
        throw;
    }
}


// virtual
void SAL_CALL ContentResultSetWrapper::removePropertyChangeListener( const OUString& rPropertyName, const Reference< XPropertyChangeListener >& xListener )
{
    impl_EnsureNotDisposed();

    {
        //noop, if no listener registered
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
            //throws UnknownPropertyException, if so
        }
        return; //the listener was not registered
    }

    m_pPropertyChangeListeners->removeInterface( rPropertyName, xListener );

    if( m_pPropertyChangeListeners->getContainedTypes().hasElements() )
        return;

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
            OUString(), static_cast< XPropertyChangeListener * >( m_xMyListenerImpl.get() ) );
    }
    catch( Exception& )
    {
        OSL_FAIL( "could not remove PropertyChangeListener" );
    }
}


// virtual
void SAL_CALL ContentResultSetWrapper::removeVetoableChangeListener( const OUString& rPropertyName, const Reference< XVetoableChangeListener >& xListener )
{
    impl_EnsureNotDisposed();

    {
        //noop, if no listener registered
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
                throw UnknownPropertyException(rPropertyName);

            m_xPropertySetInfo->getPropertyByName( rPropertyName );
            //throws UnknownPropertyException, if so
        }
        return; //the listener was not registered
    }

    m_pVetoableChangeListeners->removeInterface( rPropertyName, xListener );

    if( m_pVetoableChangeListeners->getContainedTypes().hasElements() )
        return;

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
            OUString(), static_cast< XVetoableChangeListener * >( m_xMyListenerImpl.get() ) );
    }
    catch( Exception& )
    {
        OSL_FAIL( "could not remove VetoableChangeListener" );
    }
}


// own methods.


//virtual
void ContentResultSetWrapper::impl_disposing( const EventObject& )
{
    impl_EnsureNotDisposed();

    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if( !m_xResultSetOrigin.is() )
        return;

    //release all references to the broadcaster:
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

//virtual
void ContentResultSetWrapper::impl_propertyChange( const PropertyChangeEvent& rEvt )
{
    impl_EnsureNotDisposed();

    PropertyChangeEvent aEvt( rEvt );
    aEvt.Source = static_cast< XPropertySet * >( this );
    aEvt.Further = false;
    impl_notifyPropertyChangeListeners( aEvt );
}

//virtual
void ContentResultSetWrapper::impl_vetoableChange( const PropertyChangeEvent& rEvt )
{
    impl_EnsureNotDisposed();

    PropertyChangeEvent aEvt( rEvt );
    aEvt.Source = static_cast< XPropertySet * >( this );
    aEvt.Further = false;

    impl_notifyVetoableChangeListeners( aEvt );
}


// XContentAccess methods.  ( -- position dependent )


// virtual
OUString SAL_CALL ContentResultSetWrapper::queryContentIdentifierString()
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


// virtual
Reference< XContentIdentifier > SAL_CALL ContentResultSetWrapper::queryContentIdentifier()
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


// virtual
Reference< XContent > SAL_CALL ContentResultSetWrapper::queryContent()
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


// XResultSet methods.

//virtual

sal_Bool SAL_CALL ContentResultSetWrapper::next()
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->next();
}

//virtual
sal_Bool SAL_CALL ContentResultSetWrapper::previous()
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->previous();
}

//virtual
sal_Bool SAL_CALL ContentResultSetWrapper::absolute( sal_Int32 row )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->absolute( row );
}

//virtual
sal_Bool SAL_CALL ContentResultSetWrapper::relative( sal_Int32 rows )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->relative( rows );
}


//virtual
sal_Bool SAL_CALL ContentResultSetWrapper::first()
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->first();
}

//virtual
sal_Bool SAL_CALL ContentResultSetWrapper::last()
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->last();
}

//virtual
void SAL_CALL ContentResultSetWrapper::beforeFirst()
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    m_xResultSetOrigin->beforeFirst();
}

//virtual
void SAL_CALL ContentResultSetWrapper::afterLast()
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    m_xResultSetOrigin->afterLast();
}

//virtual
sal_Bool SAL_CALL ContentResultSetWrapper::isAfterLast()
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->isAfterLast();
}

//virtual
sal_Bool SAL_CALL ContentResultSetWrapper::isBeforeFirst()
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->isBeforeFirst();
}

//virtual
sal_Bool SAL_CALL ContentResultSetWrapper::isFirst()
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->isFirst();
}

//virtual
sal_Bool SAL_CALL ContentResultSetWrapper::isLast()
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->isLast();
}


//virtual
sal_Int32 SAL_CALL ContentResultSetWrapper::getRow()
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->getRow();
}

//virtual
void SAL_CALL ContentResultSetWrapper::refreshRow()
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    m_xResultSetOrigin->refreshRow();
}

//virtual
sal_Bool SAL_CALL ContentResultSetWrapper::rowUpdated()
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->rowUpdated();
}

//virtual
sal_Bool SAL_CALL ContentResultSetWrapper::rowInserted()
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->rowInserted();
}

//virtual
sal_Bool SAL_CALL ContentResultSetWrapper::rowDeleted()
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->rowDeleted();
}

//virtual
Reference< XInterface > SAL_CALL ContentResultSetWrapper::getStatement()
{
    impl_EnsureNotDisposed();
    //@todo ?return anything
    return Reference< XInterface >();
}


// XRow methods.


void ContentResultSetWrapper::verifyGet()
{
    impl_EnsureNotDisposed();
    impl_init_xRowOrigin();
    if( !m_xRowOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
}

//virtual
sal_Bool SAL_CALL ContentResultSetWrapper::wasNull()
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

//virtual
OUString SAL_CALL ContentResultSetWrapper::getString( sal_Int32 columnIndex )
{
    verifyGet();
    return m_xRowOrigin->getString( columnIndex );
}

//virtual
sal_Bool SAL_CALL ContentResultSetWrapper::getBoolean( sal_Int32 columnIndex )
{
    verifyGet();
    return m_xRowOrigin->getBoolean( columnIndex );
}

//virtual
sal_Int8 SAL_CALL ContentResultSetWrapper::getByte( sal_Int32 columnIndex )
{
    verifyGet();
    return m_xRowOrigin->getByte( columnIndex );
}

//virtual
sal_Int16 SAL_CALL ContentResultSetWrapper::getShort( sal_Int32 columnIndex )
{
    verifyGet();
    return m_xRowOrigin->getShort( columnIndex );
}

//virtual
sal_Int32 SAL_CALL ContentResultSetWrapper::getInt( sal_Int32 columnIndex )
{
    verifyGet();
    return m_xRowOrigin->getInt( columnIndex );
}

//virtual
sal_Int64 SAL_CALL ContentResultSetWrapper::getLong( sal_Int32 columnIndex )
{
    verifyGet();
    return m_xRowOrigin->getLong( columnIndex );
}

//virtual
float SAL_CALL ContentResultSetWrapper::getFloat( sal_Int32 columnIndex )
{
    verifyGet();
    return m_xRowOrigin->getFloat( columnIndex );
}

//virtual
double SAL_CALL ContentResultSetWrapper::getDouble( sal_Int32 columnIndex )
{
    verifyGet();
    return m_xRowOrigin->getDouble( columnIndex );
}

//virtual
Sequence< sal_Int8 > SAL_CALL ContentResultSetWrapper::getBytes( sal_Int32 columnIndex )
{
    verifyGet();
    return m_xRowOrigin->getBytes( columnIndex );
}

//virtual
Date SAL_CALL ContentResultSetWrapper::getDate( sal_Int32 columnIndex )
{
    verifyGet();
    return m_xRowOrigin->getDate( columnIndex );
}

//virtual
Time SAL_CALL ContentResultSetWrapper::getTime( sal_Int32 columnIndex )
{
    verifyGet();
    return m_xRowOrigin->getTime( columnIndex );
}

//virtual
DateTime SAL_CALL ContentResultSetWrapper::getTimestamp( sal_Int32 columnIndex )
{
    verifyGet();
    return m_xRowOrigin->getTimestamp( columnIndex );
}

//virtual
Reference< css::io::XInputStream > SAL_CALL ContentResultSetWrapper::getBinaryStream( sal_Int32 columnIndex )
{
    verifyGet();
    return m_xRowOrigin->getBinaryStream( columnIndex );
}

//virtual
Reference< css::io::XInputStream > SAL_CALL ContentResultSetWrapper::getCharacterStream( sal_Int32 columnIndex )
{
    verifyGet();
    return m_xRowOrigin->getCharacterStream( columnIndex );
}

//virtual
Any SAL_CALL ContentResultSetWrapper::getObject( sal_Int32 columnIndex, const Reference< css::container::XNameAccess >& typeMap )
{
    //if you change this macro please pay attention to
    //define XROW_GETXXX, where this is similar implemented

    impl_EnsureNotDisposed();
    impl_init_xRowOrigin();
    if( !m_xRowOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xRowOrigin->getObject( columnIndex, typeMap );
}

//virtual
Reference< XRef > SAL_CALL ContentResultSetWrapper::getRef( sal_Int32 columnIndex )
{
    verifyGet();
    return m_xRowOrigin->getRef( columnIndex );
}

//virtual
Reference< XBlob > SAL_CALL ContentResultSetWrapper::getBlob( sal_Int32 columnIndex )
{
    verifyGet();
    return m_xRowOrigin->getBlob( columnIndex );
}

//virtual
Reference< XClob > SAL_CALL ContentResultSetWrapper::getClob( sal_Int32 columnIndex )
{
    verifyGet();
    return m_xRowOrigin->getClob( columnIndex );
}

//virtual
Reference< XArray > SAL_CALL ContentResultSetWrapper::getArray( sal_Int32 columnIndex )
{
    verifyGet();
    return m_xRowOrigin->getArray( columnIndex );
}




ContentResultSetWrapperListener::ContentResultSetWrapperListener(
    ContentResultSetWrapper* pOwner )
    : m_pOwner( pOwner )
{
}

ContentResultSetWrapperListener::~ContentResultSetWrapperListener()
{
}


// XInterface methods.
void SAL_CALL ContentResultSetWrapperListener::acquire()
    noexcept
{
    OWeakObject::acquire();
}

void SAL_CALL ContentResultSetWrapperListener::release()
    noexcept
{
    OWeakObject::release();
}

css::uno::Any SAL_CALL ContentResultSetWrapperListener::queryInterface( const css::uno::Type & rType )
{
    //list all interfaces inclusive baseclasses of interfaces
    css::uno::Any aRet = cppu::queryInterface( rType,
                                               static_cast< XEventListener * >(
                                                       static_cast< XPropertyChangeListener * >(this)),
                                               static_cast< XPropertyChangeListener* >(this),
                                               static_cast< XVetoableChangeListener* >(this)
        );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

//XEventListener methods.


//virtual
void SAL_CALL ContentResultSetWrapperListener::disposing( const EventObject& rEventObject )
{
    if( m_pOwner )
        m_pOwner->impl_disposing( rEventObject );
}


//XPropertyChangeListener methods.


//virtual
void SAL_CALL ContentResultSetWrapperListener::propertyChange( const PropertyChangeEvent& rEvt )
{
    if( m_pOwner )
        m_pOwner->impl_propertyChange( rEvt );
}


//XVetoableChangeListener methods.

//virtual
void SAL_CALL ContentResultSetWrapperListener::vetoableChange( const PropertyChangeEvent& rEvt )
{
    if( m_pOwner )
        m_pOwner->impl_vetoableChange( rEvt );
}

void ContentResultSetWrapperListener::impl_OwnerDies()
{
    m_pOwner = nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
