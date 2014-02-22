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


#include <classes/actiontriggerpropertyset.hxx>
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
    HANDLE_COMMANDURL,
    HANDLE_HELPURL,
    HANDLE_IMAGE,
    HANDLE_SUBCONTAINER,
    HANDLE_TEXT,
    PROPERTYCOUNT
};

namespace framework
{

ActionTriggerPropertySet::ActionTriggerPropertySet()
    : ThreadHelpBase           ( &Application::GetSolarMutex()               )
    , OBroadcastHelper         ( m_aLock.getShareableOslMutex()              )
    ,   OPropertySetHelper       ( *(static_cast< OBroadcastHelper * >(this)))
    , OWeakObject              ()
    , m_xBitmap                ( 0 )
    , m_xActionTriggerContainer( 0 )
{
}

ActionTriggerPropertySet::~ActionTriggerPropertySet()
{
}


Any SAL_CALL ActionTriggerPropertySet::queryInterface( const Type& aType )
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

void SAL_CALL ActionTriggerPropertySet::acquire() throw ()
{
    OWeakObject::acquire();
}

void SAL_CALL ActionTriggerPropertySet::release() throw ()
{
    OWeakObject::release();
}


OUString SAL_CALL ActionTriggerPropertySet::getImplementationName()
throw ( RuntimeException )
{
    return OUString( IMPLEMENTATIONNAME_ACTIONTRIGGER );
}

sal_Bool SAL_CALL ActionTriggerPropertySet::supportsService( const OUString& ServiceName )
throw ( RuntimeException )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL ActionTriggerPropertySet::getSupportedServiceNames()
throw ( RuntimeException )
{
    Sequence< OUString > seqServiceNames( 1 );
    seqServiceNames[0] = OUString( SERVICENAME_ACTIONTRIGGER );
    return seqServiceNames;
}


Sequence< Type > SAL_CALL ActionTriggerPropertySet::getTypes() throw ( RuntimeException )
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

Sequence< sal_Int8 > SAL_CALL ActionTriggerPropertySet::getImplementationId() throw ( RuntimeException )
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

sal_Bool SAL_CALL ActionTriggerPropertySet::convertFastPropertyValue(
    Any&        aConvertedValue,
    Any&        aOldValue,
    sal_Int32   nHandle,
    const Any&  aValue  )
throw( IllegalArgumentException )
{
    
    
    
    
    
    sal_Bool bReturn = sal_False;

    switch( nHandle )
    {
        case HANDLE_COMMANDURL:
            bReturn = impl_tryToChangeProperty( m_aCommandURL, aValue, aOldValue, aConvertedValue );
            break;

        case HANDLE_HELPURL:
            bReturn = impl_tryToChangeProperty( m_aHelpURL, aValue, aOldValue, aConvertedValue ) ;
            break;

        case HANDLE_IMAGE:
            bReturn = impl_tryToChangeProperty( m_xBitmap, aValue, aOldValue, aConvertedValue ) ;
            break;

        case HANDLE_SUBCONTAINER:
            bReturn = impl_tryToChangeProperty( m_xActionTriggerContainer, aValue, aOldValue, aConvertedValue );
            break;

        case HANDLE_TEXT:
            bReturn = impl_tryToChangeProperty( m_aText, aValue, aOldValue, aConvertedValue ) ;
            break;
    }

    
    return bReturn;
}

void SAL_CALL ActionTriggerPropertySet::setFastPropertyValue_NoBroadcast(
    sal_Int32 nHandle, const Any& aValue )
throw( Exception )
{
    ::osl::MutexGuard aGuard( LockHelper::getGlobalLock().getShareableOslMutex() );

    
    switch( nHandle )
    {
        case HANDLE_COMMANDURL:
            aValue >>= m_aCommandURL;
            break;

        case HANDLE_HELPURL:
            aValue >>= m_aHelpURL;
            break;

        case HANDLE_IMAGE:
            aValue >>= m_xBitmap;
            break;

        case HANDLE_SUBCONTAINER:
            aValue >>= m_xActionTriggerContainer;
            break;

        case HANDLE_TEXT:
            aValue >>= m_aText;
            break;
    }
}

void SAL_CALL ActionTriggerPropertySet::getFastPropertyValue(
    Any& aValue, sal_Int32 nHandle ) const
{
    ::osl::MutexGuard aGuard( LockHelper::getGlobalLock().getShareableOslMutex() );

    
    switch( nHandle )
    {
        case HANDLE_COMMANDURL:
            aValue <<= m_aCommandURL;
            break;

        case HANDLE_HELPURL:
            aValue <<= m_aHelpURL;
            break;

        case HANDLE_IMAGE:
            aValue <<= m_xBitmap;
            break;

        case HANDLE_SUBCONTAINER:
            aValue <<= m_xActionTriggerContainer;
            break;

        case HANDLE_TEXT:
            aValue <<= m_aText;
            break;
    }
}

::cppu::IPropertyArrayHelper& SAL_CALL ActionTriggerPropertySet::getInfoHelper()
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

Reference< XPropertySetInfo > SAL_CALL ActionTriggerPropertySet::getPropertySetInfo()
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

const Sequence< Property > ActionTriggerPropertySet::impl_getStaticPropertyDescriptor()
{
    const Property pActionTriggerPropertys[] =
    {
        Property( OUString( "CommandURL"    ), HANDLE_COMMANDURL   , ::getCppuType((OUString*)0)                , PropertyAttribute::TRANSIENT  ),
        Property( OUString( "HelpURL"       ), HANDLE_HELPURL      , ::getCppuType((OUString*)0)                , PropertyAttribute::TRANSIENT  ),
        Property( OUString( "Image"         ), HANDLE_IMAGE     , ::getCppuType((Reference<XBitmap>*)0)     , PropertyAttribute::TRANSIENT  ),
        Property( OUString( "SubContainer"  ), HANDLE_SUBCONTAINER , ::getCppuType((OUString*)0)                , PropertyAttribute::TRANSIENT  ),
        Property( OUString( "Text"          ), HANDLE_TEXT         , ::getCppuType((Reference<XInterface>*)0)  , PropertyAttribute::TRANSIENT  )
    };

    
    const Sequence< Property > seqActionTriggerPropertyDescriptor( pActionTriggerPropertys, PROPERTYCOUNT );

    
    return seqActionTriggerPropertyDescriptor ;
}

sal_Bool ActionTriggerPropertySet::impl_tryToChangeProperty(
    const   OUString&    sCurrentValue   ,
    const   Any&        aNewValue       ,
    Any&        aOldValue       ,
    Any&        aConvertedValue )
throw( IllegalArgumentException )
{
    
    sal_Bool bReturn = sal_False;
    
    
    OUString sValue ;
    convertPropertyValue( sValue, aNewValue );

    
    if( sValue != sCurrentValue )
    {
        
        aOldValue       <<= sCurrentValue   ;
        aConvertedValue <<= sValue          ;
        
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

sal_Bool ActionTriggerPropertySet::impl_tryToChangeProperty(
    const Reference< XBitmap >  aCurrentValue   ,
    const Any&                  aNewValue       ,
    Any&                        aOldValue       ,
    Any&                        aConvertedValue )
throw( IllegalArgumentException )
{
    
    sal_Bool bReturn = sal_False;
    
    
    Reference< XBitmap > aValue ;
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

sal_Bool ActionTriggerPropertySet::impl_tryToChangeProperty(
    const Reference< XInterface >   aCurrentValue   ,
    const Any&                      aNewValue       ,
    Any&                            aOldValue       ,
    Any&                            aConvertedValue )
throw( IllegalArgumentException )
{
    
    sal_Bool bReturn = sal_False;
    
    
    Reference< XInterface > aValue ;
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
