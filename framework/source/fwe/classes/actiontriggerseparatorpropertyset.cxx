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


Sequence< Type > SAL_CALL ActionTriggerSeparatorPropertySet::getTypes() throw ( RuntimeException )
{
    
    
    
    static ::cppu::OTypeCollection* pTypeCollection = NULL ;

    if ( pTypeCollection == NULL )
    {
        
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

        
        if ( pTypeCollection == NULL )
        {
            
            static ::cppu::OTypeCollection aTypeCollection(
                        ::getCppuType(( const Reference< XPropertySet           >*)NULL ) ,
                        ::getCppuType(( const Reference< XFastPropertySet       >*)NULL ) ,
                        ::getCppuType(( const Reference< XMultiPropertySet      >*)NULL ) ,
                        ::getCppuType(( const Reference< XServiceInfo           >*)NULL ) ,
                        ::getCppuType(( const Reference< XTypeProvider          >*)NULL ) ) ;

            
            pTypeCollection = &aTypeCollection ;
        }
    }

    return pTypeCollection->getTypes() ;
}

Sequence< sal_Int8 > SAL_CALL ActionTriggerSeparatorPropertySet::getImplementationId() throw ( RuntimeException )
{
    
    

    
    
    
    static ::cppu::OImplementationId* pID = NULL ;

    if ( pID == NULL )
    {
        
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

        
        if ( pID == NULL )
        {
            
            static ::cppu::OImplementationId aID( false ) ;
            
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
    
    
    
    
    
    sal_Bool bReturn = sal_False;

    switch( nHandle )
    {
        case HANDLE_TYPE:
            bReturn = impl_tryToChangeProperty( m_nSeparatorType, aValue, aOldValue, aConvertedValue );
            break;
    }

    
    return bReturn;
}

void SAL_CALL ActionTriggerSeparatorPropertySet::setFastPropertyValue_NoBroadcast(
    sal_Int32 nHandle, const Any& aValue )
throw( Exception )
{
    ::osl::MutexGuard aGuard( LockHelper::getGlobalLock().getShareableOslMutex() );

    
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

    
    switch( nHandle )
    {
        case HANDLE_TYPE:
            aValue <<= m_nSeparatorType;
            break;
    }
}

::cppu::IPropertyArrayHelper& SAL_CALL ActionTriggerSeparatorPropertySet::getInfoHelper()
{
    
    
    
    static OPropertyArrayHelper* pInfoHelper = NULL;

    if( pInfoHelper == NULL )
    {
        
        ::osl::MutexGuard aGuard( LockHelper::getGlobalLock().getShareableOslMutex() );
        
        if( pInfoHelper == NULL )
        {
            
            
            
            static OPropertyArrayHelper aInfoHelper( impl_getStaticPropertyDescriptor(), sal_True );
            pInfoHelper = &aInfoHelper;
        }
    }

    return (*pInfoHelper);
}

Reference< XPropertySetInfo > SAL_CALL ActionTriggerSeparatorPropertySet::getPropertySetInfo()
throw ( RuntimeException )
{
    
    
    
    static Reference< XPropertySetInfo >* pInfo = NULL ;

    if( pInfo == NULL )
    {
        
        ::osl::MutexGuard aGuard( LockHelper::getGlobalLock().getShareableOslMutex() );
        
        if( pInfo == NULL )
        {
            
            
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

    
    const Sequence< Property > seqActionTriggerPropertyDescriptor( pActionTriggerPropertys, PROPERTYCOUNT );

    
    return seqActionTriggerPropertyDescriptor ;
}

sal_Bool ActionTriggerSeparatorPropertySet::impl_tryToChangeProperty(
    sal_Int16           aCurrentValue   ,
    const   Any&        aNewValue       ,
    Any&                aOldValue       ,
    Any&                aConvertedValue )
throw( IllegalArgumentException )
{
    
    sal_Bool bReturn = sal_False;
    
    
    sal_Int16 aValue = 0;
    convertPropertyValue( aValue, aNewValue );

    
    if( aValue != aCurrentValue )
    {
        
        aOldValue       <<= aCurrentValue   ;
        aConvertedValue <<= aValue          ;
        
        bReturn = sal_True;
    }
    else
    {
        
        aOldValue.clear         () ;
        aConvertedValue.clear   () ;
        
        bReturn = sal_False;
    }

    return bReturn;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
