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

void ContentResultSetWrapper::impl_init_xRowOrigin(std::unique_lock<std::mutex>&)
{
    if(m_xRowOrigin.is())
        return;

    m_xRowOrigin.set(m_xResultSetOrigin, UNO_QUERY);
    OSL_ENSURE( m_xRowOrigin.is(), "interface XRow is required" );
}

void ContentResultSetWrapper::impl_init_xContentAccessOrigin(std::unique_lock<std::mutex>&)
{
    if(m_xContentAccessOrigin.is())
        return;

    m_xContentAccessOrigin.set(m_xResultSetOrigin, UNO_QUERY);
    OSL_ENSURE( m_xContentAccessOrigin.is(), "interface XContentAccess is required" );
}

void ContentResultSetWrapper::impl_init_xPropertySetOrigin(std::unique_lock<std::mutex>&)
{
    if( m_xPropertySetOrigin.is() )
        return;

    m_xPropertySetOrigin.set(m_xResultSetOrigin, UNO_QUERY);
    OSL_ENSURE( m_xPropertySetOrigin.is(), "interface XPropertySet is required" );
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
void ContentResultSetWrapper::impl_initPropertySetInfo(std::unique_lock<std::mutex>& rGuard)
{
    if( m_xPropertySetInfo.is() )
        return;

    impl_init_xPropertySetOrigin(rGuard);
    if( !m_xPropertySetOrigin.is() )
        return;

    m_xPropertySetInfo = m_xPropertySetOrigin->getPropertySetInfo();
}

void ContentResultSetWrapper::impl_EnsureNotDisposed(std::unique_lock<std::mutex>& /*rGuard*/)
{
    if( m_bDisposed )
        throw DisposedException();
}

void ContentResultSetWrapper::impl_notifyPropertyChangeListeners( std::unique_lock<std::mutex>& rGuard, const PropertyChangeEvent& rEvt )
{
    // Notify listeners interested especially in the changed property.
    OInterfaceContainerHelper4<XPropertyChangeListener>* pContainer =
            m_aPropertyChangeListeners.getContainer( rGuard, rEvt.PropertyName );
    if( pContainer )
    {
        pContainer->notifyEach( rGuard, &XPropertyChangeListener::propertyChange, rEvt );
    }

    // Notify listeners interested in all properties.
    pContainer = m_aPropertyChangeListeners.getContainer( rGuard, OUString() );
    if( pContainer )
    {
        pContainer->notifyEach( rGuard, &XPropertyChangeListener::propertyChange, rEvt );
    }
}

void ContentResultSetWrapper::impl_notifyVetoableChangeListeners( std::unique_lock<std::mutex>& rGuard, const PropertyChangeEvent& rEvt )
{
    // Notify listeners interested especially in the changed property.
    OInterfaceContainerHelper4<XVetoableChangeListener>* pContainer =
            m_aVetoableChangeListeners.getContainer( rGuard, rEvt.PropertyName );
    if( pContainer )
    {
        pContainer->notifyEach( rGuard, &XVetoableChangeListener::vetoableChange, rEvt );
    }

    // Notify listeners interested in all properties.
    pContainer = m_aVetoableChangeListeners.getContainer( rGuard, OUString() );
    if( pContainer )
    {
        pContainer->notifyEach( rGuard, &XVetoableChangeListener::vetoableChange, rEvt );
    }
}

bool ContentResultSetWrapper::impl_isForwardOnly(std::unique_lock<std::mutex>& /*rGuard*/)
{
    //m_nForwardOnly == 2 -> don't know
    //m_nForwardOnly == 1 -> YES
    //m_nForwardOnly == 0 -> NO

    //@todo replace this with lines in comment
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
    std::unique_lock aGuard(m_aMutex);
    impl_EnsureNotDisposed(aGuard);

    bool isCleared = false;
    if( m_bInDispose || m_bDisposed )
        return;
    m_bInDispose = true;

    if( m_xPropertySetOrigin.is() )
    {
        aGuard.unlock();
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
        aGuard.lock();
    if( m_aDisposeEventListeners.getLength(aGuard) )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XComponent * >( this );
        m_aDisposeEventListeners.disposeAndClear( aGuard, aEvt );
    }

    if( m_aPropertyChangeListeners.hasContainedTypes(aGuard) )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XPropertySet * >( this );
        m_aPropertyChangeListeners.disposeAndClear( aGuard, aEvt );
    }

    if( m_aVetoableChangeListeners.hasContainedTypes(aGuard) )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XPropertySet * >( this );
        m_aVetoableChangeListeners.disposeAndClear( aGuard, aEvt );
    }

    m_bDisposed = true;
    m_bInDispose = false;
}


// virtual
void SAL_CALL ContentResultSetWrapper::addEventListener( const Reference< XEventListener >& Listener )
{
    std::unique_lock aGuard( m_aMutex );
    impl_EnsureNotDisposed(aGuard);

    m_aDisposeEventListeners.addInterface( aGuard, Listener );
}


// virtual
void SAL_CALL ContentResultSetWrapper::removeEventListener( const Reference< XEventListener >& Listener )
{
    std::unique_lock aGuard( m_aMutex );
    impl_EnsureNotDisposed(aGuard);
    m_aDisposeEventListeners.removeInterface( aGuard, Listener );
}


//XCloseable methods.

//virtual
void SAL_CALL ContentResultSetWrapper::close()
{
    {
        std::unique_lock aGuard( m_aMutex );
        impl_EnsureNotDisposed(aGuard);
    }
    dispose();
}


//XResultSetMetaDataSupplier methods.

//virtual
Reference< XResultSetMetaData > SAL_CALL ContentResultSetWrapper::getMetaData()
{
    std::unique_lock aGuard(m_aMutex);
    impl_EnsureNotDisposed(aGuard);

    if( !m_xMetaDataFromOrigin.is() && m_xResultSetOrigin.is() )
    {
        Reference< XResultSetMetaDataSupplier > xMetaDataSupplier(
                m_xResultSetOrigin, UNO_QUERY );

        if( xMetaDataSupplier.is() )
        {
            aGuard.unlock();

            Reference< XResultSetMetaData > xMetaData
                = xMetaDataSupplier->getMetaData();

            aGuard.lock();
            m_xMetaDataFromOrigin = std::move(xMetaData);
        }
    }
    return m_xMetaDataFromOrigin;
}


// XPropertySet methods.

// virtual
Reference< XPropertySetInfo > SAL_CALL ContentResultSetWrapper::getPropertySetInfo()
{
    std::unique_lock aGuard( m_aMutex );
    return getPropertySetInfoImpl(aGuard);
}

// virtual
const Reference< XPropertySetInfo > & ContentResultSetWrapper::getPropertySetInfoImpl(std::unique_lock<std::mutex>& rGuard)
{
    impl_EnsureNotDisposed(rGuard);
    if( m_xPropertySetInfo.is() )
        return m_xPropertySetInfo;
    impl_initPropertySetInfo(rGuard);
    return m_xPropertySetInfo;
}

// virtual
void SAL_CALL ContentResultSetWrapper::setPropertyValue( const OUString& rPropertyName, const Any& rValue )
{
    std::unique_lock aGuard( m_aMutex );
    return setPropertyValueImpl(aGuard, rPropertyName, rValue);
}

// virtual
void ContentResultSetWrapper::setPropertyValueImpl( std::unique_lock<std::mutex>& rGuard, const OUString& rPropertyName, const Any& rValue )
{
    impl_EnsureNotDisposed(rGuard);
    impl_init_xPropertySetOrigin(rGuard);
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
    std::unique_lock aGuard( m_aMutex );
    impl_EnsureNotDisposed(aGuard);
    impl_init_xPropertySetOrigin(aGuard);
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
    std::unique_lock aGuard( m_aMutex );
    impl_EnsureNotDisposed(aGuard);

    if( !getPropertySetInfoImpl(aGuard).is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw UnknownPropertyException();
    }

    if( !aPropertyName.isEmpty() )
    {
        m_xPropertySetInfo->getPropertyByName( aPropertyName );
        //throws UnknownPropertyException, if so
    }

    bool bNeedRegister = !m_aPropertyChangeListeners.hasContainedTypes(aGuard);
    m_aPropertyChangeListeners.addInterface( aGuard, aPropertyName, xListener );
    if( !bNeedRegister )
        return;

    impl_init_xPropertySetOrigin(aGuard);
    if( !m_xPropertySetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        return;
    }
    try
    {
        m_xPropertySetOrigin->addPropertyChangeListener(
            OUString(), static_cast< XPropertyChangeListener * >( m_xMyListenerImpl.get() ) );
    }
    catch( Exception& )
    {
        m_aPropertyChangeListeners.removeInterface( aGuard, aPropertyName, xListener );
        throw;
    }
}


// virtual
void SAL_CALL ContentResultSetWrapper::addVetoableChangeListener( const OUString& rPropertyName, const Reference< XVetoableChangeListener >& xListener )
{
    std::unique_lock aGuard( m_aMutex );
    impl_EnsureNotDisposed(aGuard);

    if( !getPropertySetInfoImpl(aGuard).is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw UnknownPropertyException();
    }
    if( !rPropertyName.isEmpty() )
    {
        m_xPropertySetInfo->getPropertyByName( rPropertyName );
        //throws UnknownPropertyException, if so
    }

    bool bNeedRegister = !m_aVetoableChangeListeners.hasContainedTypes(aGuard);
    m_aVetoableChangeListeners.addInterface( aGuard, rPropertyName, xListener );
    if( !bNeedRegister )
        return;

    impl_init_xPropertySetOrigin(aGuard);
    if( !m_xPropertySetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        return;
    }
    try
    {
        m_xPropertySetOrigin->addVetoableChangeListener(
            OUString(), static_cast< XVetoableChangeListener * >( m_xMyListenerImpl.get() ) );
    }
    catch( Exception& )
    {
        m_aVetoableChangeListeners.removeInterface( aGuard, rPropertyName, xListener );
        throw;
    }
}


// virtual
void SAL_CALL ContentResultSetWrapper::removePropertyChangeListener( const OUString& rPropertyName, const Reference< XPropertyChangeListener >& xListener )
{
    std::unique_lock aGuard( m_aMutex );
    impl_EnsureNotDisposed(aGuard);

    //noop, if no listener registered
    if( !m_aPropertyChangeListeners.hasContainedTypes(aGuard) )
        return;

    OInterfaceContainerHelper4<XPropertyChangeListener>* pContainer =
        m_aPropertyChangeListeners.getContainer( aGuard, rPropertyName );

    if( !pContainer )
    {
        if( !rPropertyName.isEmpty() )
        {
            if( !getPropertySetInfoImpl(aGuard).is() )
                throw UnknownPropertyException();

            m_xPropertySetInfo->getPropertyByName( rPropertyName );
            //throws UnknownPropertyException, if so
        }
        return; //the listener was not registered
    }

    m_aPropertyChangeListeners.removeInterface( aGuard, rPropertyName, xListener );

    if( m_aPropertyChangeListeners.hasContainedTypes(aGuard) )
        return;

    impl_init_xPropertySetOrigin(aGuard);
    if( !m_xPropertySetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        return;
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
    std::unique_lock aGuard( m_aMutex );
    impl_EnsureNotDisposed(aGuard);

    //noop, if no listener registered
    if( !m_aVetoableChangeListeners.hasContainedTypes(aGuard) )
        return;
    OInterfaceContainerHelper4<XVetoableChangeListener>* pContainer =
        m_aVetoableChangeListeners.getContainer( aGuard, rPropertyName );

    if( !pContainer )
    {
        if( !rPropertyName.isEmpty() )
        {
            if( !getPropertySetInfoImpl(aGuard).is() )
                throw UnknownPropertyException(rPropertyName);

            m_xPropertySetInfo->getPropertyByName( rPropertyName );
            //throws UnknownPropertyException, if so
        }
        return; //the listener was not registered
    }

    m_aVetoableChangeListeners.removeInterface( aGuard, rPropertyName, xListener );

    if( m_aVetoableChangeListeners.hasContainedTypes(aGuard) )
        return;

    impl_init_xPropertySetOrigin(aGuard);
    if( !m_xPropertySetOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        return;
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


//virtual, only called from ContentResultSetWrapperListener
void ContentResultSetWrapper::impl_disposing( const EventObject& )
{
    std::unique_lock aGuard(m_aMutex);

    impl_EnsureNotDisposed(aGuard);

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

//virtual, only called from ContentResultSetWrapperListener
void ContentResultSetWrapper::impl_propertyChange( const PropertyChangeEvent& rEvt )
{
    std::unique_lock aGuard(m_aMutex);

    impl_EnsureNotDisposed(aGuard);

    PropertyChangeEvent aEvt( rEvt );
    aEvt.Source = static_cast< XPropertySet * >( this );
    aEvt.Further = false;
    impl_notifyPropertyChangeListeners( aGuard, aEvt );
}

//virtual, only called from ContentResultSetWrapperListener
void ContentResultSetWrapper::impl_vetoableChange( const PropertyChangeEvent& rEvt )
{
    std::unique_lock aGuard(m_aMutex);
    impl_EnsureNotDisposed(aGuard);

    PropertyChangeEvent aEvt( rEvt );
    aEvt.Source = static_cast< XPropertySet * >( this );
    aEvt.Further = false;

    impl_notifyVetoableChangeListeners( aGuard, aEvt );
}


// XContentAccess methods.  ( -- position dependent )


// virtual
OUString SAL_CALL ContentResultSetWrapper::queryContentIdentifierString()
{
    std::unique_lock aGuard(m_aMutex);
    return queryContentIdentifierStringImpl(aGuard);
}

// virtual
OUString ContentResultSetWrapper::queryContentIdentifierStringImpl(std::unique_lock<std::mutex>& rGuard)
{
    impl_EnsureNotDisposed(rGuard);
    impl_init_xContentAccessOrigin(rGuard);
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
    std::unique_lock aGuard(m_aMutex);
    return queryContentIdentifierImpl(aGuard);
}

// virtual
Reference<XContentIdentifier> ContentResultSetWrapper::queryContentIdentifierImpl(std::unique_lock<std::mutex>& rGuard)
{
    impl_EnsureNotDisposed(rGuard);
    impl_init_xContentAccessOrigin(rGuard);
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
    std::unique_lock aGuard(m_aMutex);
    return queryContentImpl(aGuard);
}

Reference<XContent> ContentResultSetWrapper::queryContentImpl(std::unique_lock<std::mutex>& rGuard)
{
    impl_EnsureNotDisposed(rGuard);
    impl_init_xContentAccessOrigin(rGuard);
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
    std::unique_lock aGuard(m_aMutex);
    impl_EnsureNotDisposed(aGuard);

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
    std::unique_lock aGuard(m_aMutex);
    impl_EnsureNotDisposed(aGuard);

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
    std::unique_lock aGuard(m_aMutex);
    impl_EnsureNotDisposed(aGuard);

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
    std::unique_lock aGuard(m_aMutex);
    impl_EnsureNotDisposed(aGuard);

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
    std::unique_lock aGuard(m_aMutex);
    impl_EnsureNotDisposed(aGuard);

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
    std::unique_lock aGuard(m_aMutex);
    impl_EnsureNotDisposed(aGuard);

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
    std::unique_lock aGuard(m_aMutex);
    impl_EnsureNotDisposed(aGuard);

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
    std::unique_lock aGuard(m_aMutex);
    impl_EnsureNotDisposed(aGuard);

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
    std::unique_lock aGuard(m_aMutex);
    impl_EnsureNotDisposed(aGuard);

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
    std::unique_lock aGuard(m_aMutex);
    impl_EnsureNotDisposed(aGuard);

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
    std::unique_lock aGuard(m_aMutex);
    impl_EnsureNotDisposed(aGuard);

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
    std::unique_lock aGuard(m_aMutex);
    impl_EnsureNotDisposed(aGuard);

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
    std::unique_lock aGuard(m_aMutex);
    impl_EnsureNotDisposed(aGuard);

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
    std::unique_lock aGuard(m_aMutex);
    impl_EnsureNotDisposed(aGuard);

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
    std::unique_lock aGuard(m_aMutex);
    impl_EnsureNotDisposed(aGuard);

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
    std::unique_lock aGuard(m_aMutex);
    impl_EnsureNotDisposed(aGuard);

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
    std::unique_lock aGuard(m_aMutex);
    impl_EnsureNotDisposed(aGuard);

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
    std::unique_lock aGuard(m_aMutex);
    impl_EnsureNotDisposed(aGuard);
    //@todo ?return anything
    return Reference< XInterface >();
}


// XRow methods.


void ContentResultSetWrapper::verifyGet()
{
    std::unique_lock aGuard(m_aMutex);
    impl_EnsureNotDisposed(aGuard);
    impl_init_xRowOrigin(aGuard);
    if( !m_xRowOrigin.is() )
    {
        OSL_FAIL( "broadcaster was disposed already" );
        throw RuntimeException();
    }
}

//virtual
sal_Bool SAL_CALL ContentResultSetWrapper::wasNull()
{
    std::unique_lock aGuard(m_aMutex);
    impl_EnsureNotDisposed(aGuard);
    impl_init_xRowOrigin(aGuard);
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

    std::unique_lock aGuard(m_aMutex);
    impl_EnsureNotDisposed(aGuard);
    impl_init_xRowOrigin(aGuard);
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
