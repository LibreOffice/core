/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#include <classes/actiontriggerpropertyset.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <cppuhelper/proptypehlp.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <vcl/svapp.hxx>


using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::awt;

//struct SAL_DLLPUBLIC_IMPORT ::cppu::OBroadcastHelperVar< OMultiTypeInterfaceContainerHelper, OMultiTypeInterfaceContainerHelper::keyType >;

// Handles for properties
// (PLEASE SORT THIS FIELD, IF YOU ADD NEW PROPERTIES!)
// We use an enum to define these handles, to use all numbers from 0 to nn and
// if you add someone, you don't must control this!
// But don't forget to change values of follow defines, if you do something with this enum!
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

ActionTriggerPropertySet::ActionTriggerPropertySet( const Reference< XMultiServiceFactory >& /*xServiceManager*/ )
    : ThreadHelpBase           ( &Application::GetSolarMutex()               )
    , OBroadcastHelper         ( m_aLock.getShareableOslMutex()              )
    ,   OPropertySetHelper       ( *SAL_STATIC_CAST( OBroadcastHelper *, this ))
    , OWeakObject              ()
    , m_xBitmap                ( 0 )
    , m_xActionTriggerContainer( 0 )
{
}

ActionTriggerPropertySet::~ActionTriggerPropertySet()
{
}

// XInterface
Any SAL_CALL ActionTriggerPropertySet::queryInterface( const Type& aType )
throw ( RuntimeException )
{
    Any a = ::cppu::queryInterface(
                aType ,
                SAL_STATIC_CAST( XServiceInfo*, this ));

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


// XServiceInfo
::rtl::OUString SAL_CALL ActionTriggerPropertySet::getImplementationName()
throw ( RuntimeException )
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATIONNAME_ACTIONTRIGGER ));
}

sal_Bool SAL_CALL ActionTriggerPropertySet::supportsService( const ::rtl::OUString& ServiceName )
throw ( RuntimeException )
{
    if ( ServiceName.equalsAscii( SERVICENAME_ACTIONTRIGGER ))
        return sal_True;

    return sal_False;
}

Sequence< ::rtl::OUString > SAL_CALL ActionTriggerPropertySet::getSupportedServiceNames()
throw ( RuntimeException )
{
    Sequence< ::rtl::OUString > seqServiceNames( 1 );
    seqServiceNames[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME_ACTIONTRIGGER ));
    return seqServiceNames;
}

// XTypeProvider
Sequence< Type > SAL_CALL ActionTriggerPropertySet::getTypes() throw ( RuntimeException )
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

Sequence< sal_Int8 > SAL_CALL ActionTriggerPropertySet::getImplementationId() throw ( RuntimeException )
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
            static ::cppu::OImplementationId aID( sal_False ) ;
            // ... and set his address to static pointer!
            pID = &aID ;
        }
    }

    return pID->getImplementationId() ;
}

//---------------------------------------------------------------------------------------------------------
//  OPropertySetHelper implementation
//---------------------------------------------------------------------------------------------------------

sal_Bool SAL_CALL ActionTriggerPropertySet::convertFastPropertyValue(
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

    // Return state of operation.
    return bReturn;
}


void SAL_CALL ActionTriggerPropertySet::setFastPropertyValue_NoBroadcast(
    sal_Int32 nHandle, const Any& aValue )
throw( Exception )
{
    ::osl::MutexGuard aGuard( LockHelper::getGlobalLock().getShareableOslMutex() );

    // Search for right handle ... and try to set property value.
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

    // Search for right handle ... and try to get property value.
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

Reference< XPropertySetInfo > SAL_CALL ActionTriggerPropertySet::getPropertySetInfo()
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

const Sequence< Property > ActionTriggerPropertySet::impl_getStaticPropertyDescriptor()
{
    static const Property pActionTriggerPropertys[] =
    {
        Property( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CommandURL"    )), HANDLE_COMMANDURL   , ::getCppuType((::rtl::OUString*)0)                , PropertyAttribute::TRANSIENT  ),
        Property( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HelpURL"       )), HANDLE_HELPURL      , ::getCppuType((::rtl::OUString*)0)                , PropertyAttribute::TRANSIENT  ),
        Property( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Image"        )), HANDLE_IMAGE     , ::getCppuType((Reference<XBitmap>*)0)     , PropertyAttribute::TRANSIENT  ),
        Property( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SubContainer"  )), HANDLE_SUBCONTAINER , ::getCppuType((::rtl::OUString*)0)                , PropertyAttribute::TRANSIENT  ),
        Property( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Text"          )), HANDLE_TEXT         , ::getCppuType((Reference<XInterface>*)0)  , PropertyAttribute::TRANSIENT  )
    };

    // Use it to initialize sequence!
    static const Sequence< Property > seqActionTriggerPropertyDescriptor( pActionTriggerPropertys, PROPERTYCOUNT );

    // Return static "PropertyDescriptor"
    return seqActionTriggerPropertyDescriptor ;
}


//******************************************************************************************************************************
//  private method
//******************************************************************************************************************************
sal_Bool ActionTriggerPropertySet::impl_tryToChangeProperty(
    const   ::rtl::OUString&    sCurrentValue   ,
    const   Any&        aNewValue       ,
    Any&        aOldValue       ,
    Any&        aConvertedValue )
throw( IllegalArgumentException )
{
    // Set default return value if method failed.
    sal_Bool bReturn = sal_False;
    // Get new value from any.
    // IllegalArgumentException() can be thrown!
    ::rtl::OUString sValue ;
    convertPropertyValue( sValue, aNewValue );

    // If value change ...
    if( sValue != sCurrentValue )
    {
        // ... set information of change.
        aOldValue       <<= sCurrentValue   ;
        aConvertedValue <<= sValue          ;
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


sal_Bool ActionTriggerPropertySet::impl_tryToChangeProperty(
    const Reference< XBitmap >  aCurrentValue   ,
    const Any&                  aNewValue       ,
    Any&                        aOldValue       ,
    Any&                        aConvertedValue )
throw( IllegalArgumentException )
{
    // Set default return value if method failed.
    sal_Bool bReturn = sal_False;
    // Get new value from any.
    // IllegalArgumentException() can be thrown!
    Reference< XBitmap > aValue ;
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

sal_Bool ActionTriggerPropertySet::impl_tryToChangeProperty(
    const Reference< XInterface >   aCurrentValue   ,
    const Any&                      aNewValue       ,
    Any&                            aOldValue       ,
    Any&                            aConvertedValue )
throw( IllegalArgumentException )
{
    // Set default return value if method failed.
    sal_Bool bReturn = sal_False;
    // Get new value from any.
    // IllegalArgumentException() can be thrown!
    Reference< XInterface > aValue ;
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
