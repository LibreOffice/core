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


#include <classes/actiontriggerseparatorpropertyset.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <cppuhelper/proptypehlp.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <vcl/svapp.hxx>


using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::awt;

// Handles for properties
// (PLEASE SORT THIS FIELD, IF YOU ADD NEW PROPERTIES!)
// We use an enum to define these handles, to use all numbers from 0 to nn and
// if you add someone, you don't must control this!
// But don't forget to change values of follow defines, if you do something with this enum!
enum EPROPERTIES
{
    HANDLE_TYPE,
    PROPERTYCOUNT
};

namespace framework
{

ActionTriggerSeparatorPropertySet::ActionTriggerSeparatorPropertySet()
        :   ThreadHelpBase          ( &Application::GetSolarMutex()                     )
        ,   OBroadcastHelper        ( m_aLock.getShareableOslMutex()                    )
        ,   OPropertySetHelper      ( *(static_cast< OBroadcastHelper * >(this))        )
        ,   OWeakObject             (                                                   )
        ,   m_nSeparatorType( 0 )
{
}

ActionTriggerSeparatorPropertySet::~ActionTriggerSeparatorPropertySet()
{
}

// XInterface
Any SAL_CALL ActionTriggerSeparatorPropertySet::queryInterface( const Type& aType )
throw ( RuntimeException )
{
    Any a = ::cppu::queryInterface(
                aType,
                (static_cast< XServiceInfo* >(this)),
                (static_cast< XTypeProvider* >(this)));

    if( a.hasValue() )
        return a;
    else
    {
        a = OPropertySetHelper::queryInterface( aType );

        if( a.hasValue() )
            return a;
    }

    return OWeakObject::queryInterface( aType );
}

void ActionTriggerSeparatorPropertySet::acquire() throw()
{
    OWeakObject::acquire();
}

void ActionTriggerSeparatorPropertySet::release() throw()
{
    OWeakObject::release();
}

// XServiceInfo
OUString SAL_CALL ActionTriggerSeparatorPropertySet::getImplementationName()
throw ( RuntimeException )
{
    return OUString( IMPLEMENTATIONNAME_ACTIONTRIGGERSEPARATOR );
}

sal_Bool SAL_CALL ActionTriggerSeparatorPropertySet::supportsService( const OUString& ServiceName )
throw ( RuntimeException )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL ActionTriggerSeparatorPropertySet::getSupportedServiceNames()
throw ( RuntimeException )
{
    Sequence< OUString > seqServiceNames( 1 );
    seqServiceNames[0] = OUString( SERVICENAME_ACTIONTRIGGERSEPARATOR );
    return seqServiceNames;
}

// XTypeProvider
Sequence< Type > SAL_CALL ActionTriggerSeparatorPropertySet::getTypes() throw ( RuntimeException )
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pTypeCollection is NULL - for the second call pTypeCollection is different from NULL!
    static ::cppu::OTypeCollection* pTypeCollection = NULL ;

    if ( pTypeCollection == NULL )
    {
        // Ready for multithreading; get global mutex for first call of this method only! see before
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

        // Control these pointer again ... it can be, that another instance will be faster then these!
        if ( pTypeCollection == NULL )
        {
            // Create a static typecollection ...
            static ::cppu::OTypeCollection aTypeCollection(
                        ::getCppuType(( const Reference< XPropertySet           >*)NULL ) ,
                        ::getCppuType(( const Reference< XFastPropertySet       >*)NULL ) ,
                        ::getCppuType(( const Reference< XMultiPropertySet      >*)NULL ) ,
                        ::getCppuType(( const Reference< XServiceInfo           >*)NULL ) ,
                        ::getCppuType(( const Reference< XTypeProvider          >*)NULL ) ) ;

            // ... and set his address to static pointer!
            pTypeCollection = &aTypeCollection ;
        }
    }

    return pTypeCollection->getTypes() ;
}

Sequence< sal_Int8 > SAL_CALL ActionTriggerSeparatorPropertySet::getImplementationId() throw ( RuntimeException )
{
    // Create one Id for all instances of this class.
    // Use ethernet address to do this! (sal_True)

    // Optimize this method
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pID is NULL - for the second call pID is different from NULL!
    static ::cppu::OImplementationId* pID = NULL ;

    if ( pID == NULL )
    {
        // Ready for multithreading; get global mutex for first call of this method only! see before
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

        // Control these pointer again ... it can be, that another instance will be faster then these!
        if ( pID == NULL )
        {
            // Create a new static ID ...
            static ::cppu::OImplementationId aID( false ) ;
            // ... and set his address to static pointer!
            pID = &aID ;
        }
    }

    return pID->getImplementationId() ;
}

sal_Bool SAL_CALL ActionTriggerSeparatorPropertySet::convertFastPropertyValue(
    Any&        aConvertedValue,
    Any&        aOldValue,
    sal_Int32   nHandle,
    const Any&  aValue  )
throw( IllegalArgumentException )
{
    //  Check, if value of property will changed in method "setFastPropertyValue_NoBroadcast()".
    //  Return sal_True, if changed - else return sal_False.
    //  Attention: Method "impl_tryToChangeProperty()" can throw the IllegalArgumentException !!!
    //  Initialize return value with sal_False !!!
    //  (Handle can be invalid)
    sal_Bool bReturn = sal_False;

    switch( nHandle )
    {
        case HANDLE_TYPE:
            bReturn = impl_tryToChangeProperty( m_nSeparatorType, aValue, aOldValue, aConvertedValue );
            break;
    }

    // Return state of operation.
    return bReturn;
}

void SAL_CALL ActionTriggerSeparatorPropertySet::setFastPropertyValue_NoBroadcast(
    sal_Int32 nHandle, const Any& aValue )
throw( Exception )
{
    ::osl::MutexGuard aGuard( LockHelper::getGlobalLock().getShareableOslMutex() );

    // Search for right handle ... and try to set property value.
    switch( nHandle )
    {
        case HANDLE_TYPE:
            aValue >>= m_nSeparatorType;
            break;
    }
}

void SAL_CALL ActionTriggerSeparatorPropertySet::getFastPropertyValue(
    Any& aValue, sal_Int32 nHandle ) const
{
    ::osl::MutexGuard aGuard( LockHelper::getGlobalLock().getShareableOslMutex() );

    // Search for right handle ... and try to get property value.
    switch( nHandle )
    {
        case HANDLE_TYPE:
            aValue <<= m_nSeparatorType;
            break;
    }
}

::cppu::IPropertyArrayHelper& SAL_CALL ActionTriggerSeparatorPropertySet::getInfoHelper()
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pInfoHelper is NULL - for the second call pInfoHelper is different from NULL!
    static OPropertyArrayHelper* pInfoHelper = NULL;

    if( pInfoHelper == NULL )
    {
        // Ready for multithreading
        ::osl::MutexGuard aGuard( LockHelper::getGlobalLock().getShareableOslMutex() );
        // Control this pointer again, another instance can be faster then these!
        if( pInfoHelper == NULL )
        {
            // Define static member to give structure of properties to baseclass "OPropertySetHelper".
            // "impl_getStaticPropertyDescriptor" is a non exported and static funtion, who will define a static propertytable.
            // "sal_True" say: Table is sorted by name.
            static OPropertyArrayHelper aInfoHelper( impl_getStaticPropertyDescriptor(), sal_True );
            pInfoHelper = &aInfoHelper;
        }
    }

    return (*pInfoHelper);
}

Reference< XPropertySetInfo > SAL_CALL ActionTriggerSeparatorPropertySet::getPropertySetInfo()
throw ( RuntimeException )
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pInfo is NULL - for the second call pInfo is different from NULL!
    static Reference< XPropertySetInfo >* pInfo = NULL ;

    if( pInfo == NULL )
    {
        // Ready for multithreading
        ::osl::MutexGuard aGuard( LockHelper::getGlobalLock().getShareableOslMutex() );
        // Control this pointer again, another instance can be faster then these!
        if( pInfo == NULL )
        {
            // Create structure of propertysetinfo for baseclass "OPropertySetHelper".
            // (Use method "getInfoHelper()".)
            static Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
            pInfo = &xInfo;
        }
    }

    return (*pInfo);
}

const Sequence< Property > ActionTriggerSeparatorPropertySet::impl_getStaticPropertyDescriptor()
{
    const Property pActionTriggerPropertys[] =
    {
        Property( OUString( "SeparatorType" ), HANDLE_TYPE, ::getCppuType((sal_Int16*)0), PropertyAttribute::TRANSIENT )
    };

    // Use it to initialize sequence!
    const Sequence< Property > seqActionTriggerPropertyDescriptor( pActionTriggerPropertys, PROPERTYCOUNT );

    // Return "PropertyDescriptor"
    return seqActionTriggerPropertyDescriptor ;
}

sal_Bool ActionTriggerSeparatorPropertySet::impl_tryToChangeProperty(
    sal_Int16           aCurrentValue   ,
    const   Any&        aNewValue       ,
    Any&                aOldValue       ,
    Any&                aConvertedValue )
throw( IllegalArgumentException )
{
    // Set default return value if method failed.
    sal_Bool bReturn = sal_False;
    // Get new value from any.
    // IllegalArgumentException() can be thrown!
    sal_Int16 aValue = 0;
    convertPropertyValue( aValue, aNewValue );

    // If value change ...
    if( aValue != aCurrentValue )
    {
        // ... set information of change.
        aOldValue       <<= aCurrentValue   ;
        aConvertedValue <<= aValue          ;
        // Return OK - "value will be change ..."
        bReturn = sal_True;
    }
    else
    {
        // ... clear information of return parameter!
        aOldValue.clear         () ;
        aConvertedValue.clear   () ;
        // Return NOTHING - "value will not be change ..."
        bReturn = sal_False;
    }

    return bReturn;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
