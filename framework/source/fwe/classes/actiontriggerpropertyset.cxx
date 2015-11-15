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

#include <classes/actiontriggerpropertyset.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <cppuhelper/proptypehlp.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/supportsservice.hxx>
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

ActionTriggerPropertySet::ActionTriggerPropertySet()
    : OBroadcastHelper         ( m_aMutex )
    ,   OPropertySetHelper       ( *(static_cast< OBroadcastHelper * >(this)))
    , OWeakObject              ()
    , m_xBitmap                ( nullptr )
    , m_xActionTriggerContainer( nullptr )
{
}

ActionTriggerPropertySet::~ActionTriggerPropertySet()
{
}

// XInterface
Any SAL_CALL ActionTriggerPropertySet::queryInterface( const Type& aType )
throw ( RuntimeException, std::exception )
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

// XServiceInfo
OUString SAL_CALL ActionTriggerPropertySet::getImplementationName()
throw ( RuntimeException, std::exception )
{
    return OUString( IMPLEMENTATIONNAME_ACTIONTRIGGER );
}

sal_Bool SAL_CALL ActionTriggerPropertySet::supportsService( const OUString& ServiceName )
throw ( RuntimeException, std::exception )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL ActionTriggerPropertySet::getSupportedServiceNames()
throw ( RuntimeException, std::exception )
{
    Sequence<OUString> seqServiceNames { SERVICENAME_ACTIONTRIGGER };
    return seqServiceNames;
}

// XTypeProvider
Sequence< Type > SAL_CALL ActionTriggerPropertySet::getTypes() throw ( RuntimeException, std::exception )
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pTypeCollection is NULL - for the second call pTypeCollection is different from NULL!
    static ::cppu::OTypeCollection* pTypeCollection = nullptr;

    if ( pTypeCollection == nullptr )
    {
        // Ready for multithreading; get global mutex for first call of this method only! see before
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() );

        // Control these pointer again ... it can be, that another instance will be faster then these!
        if ( pTypeCollection == nullptr )
        {
            // Create a static typecollection ...
            static ::cppu::OTypeCollection aTypeCollection(
                        cppu::UnoType<XPropertySet>::get(),
                        cppu::UnoType<XFastPropertySet>::get(),
                        cppu::UnoType<XMultiPropertySet>::get(),
                        cppu::UnoType<XServiceInfo>::get(),
                        cppu::UnoType<XTypeProvider>::get());

            // ... and set his address to static pointer!
            pTypeCollection = &aTypeCollection;
        }
    }

    return pTypeCollection->getTypes();
}

Sequence< sal_Int8 > SAL_CALL ActionTriggerPropertySet::getImplementationId() throw ( RuntimeException, std::exception )
{
    return css::uno::Sequence<sal_Int8>();
}

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
    bool bReturn = false;

    switch( nHandle )
    {
        case HANDLE_COMMANDURL:
            bReturn = impl_tryToChangeProperty( m_aCommandURL, aValue, aOldValue, aConvertedValue );
            break;

        case HANDLE_HELPURL:
            bReturn = impl_tryToChangeProperty( m_aHelpURL, aValue, aOldValue, aConvertedValue );
            break;

        case HANDLE_IMAGE:
            bReturn = impl_tryToChangeProperty( m_xBitmap, aValue, aOldValue, aConvertedValue );
            break;

        case HANDLE_SUBCONTAINER:
            bReturn = impl_tryToChangeProperty( m_xActionTriggerContainer, aValue, aOldValue, aConvertedValue );
            break;

        case HANDLE_TEXT:
            bReturn = impl_tryToChangeProperty( m_aText, aValue, aOldValue, aConvertedValue );
            break;
    }

    // Return state of operation.
    return bReturn;
}

void SAL_CALL ActionTriggerPropertySet::setFastPropertyValue_NoBroadcast(
    sal_Int32 nHandle, const Any& aValue )
throw( Exception, std::exception )
{
    SolarMutexGuard aGuard;

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
    SolarMutexGuard aGuard;

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
    static OPropertyArrayHelper* pInfoHelper = nullptr;

    if( pInfoHelper == nullptr )
    {
        SolarMutexGuard aGuard;
        // Control this pointer again, another instance can be faster then these!
        if( pInfoHelper == nullptr )
        {
            // Define static member to give structure of properties to baseclass "OPropertySetHelper".
            // "impl_getStaticPropertyDescriptor" is a non exported and static function, who will define a static propertytable.
            // "sal_True" say: Table is sorted by name.
            static OPropertyArrayHelper aInfoHelper( impl_getStaticPropertyDescriptor(), sal_True );
            pInfoHelper = &aInfoHelper;
        }
    }

    return (*pInfoHelper);
}

Reference< XPropertySetInfo > SAL_CALL ActionTriggerPropertySet::getPropertySetInfo()
throw ( RuntimeException, std::exception )
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pInfo is NULL - for the second call pInfo is different from NULL!
    static Reference< XPropertySetInfo >* pInfo = nullptr;

    if( pInfo == nullptr )
    {
        SolarMutexGuard aGuard;
        // Control this pointer again, another instance can be faster then these!
        if( pInfo == nullptr )
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
    const Property pActionTriggerPropertys[] =
    {
        Property( OUString( "CommandURL"    ), HANDLE_COMMANDURL   , cppu::UnoType<OUString>::get(), PropertyAttribute::TRANSIENT  ),
        Property( OUString( "HelpURL"       ), HANDLE_HELPURL      , cppu::UnoType<OUString>::get(), PropertyAttribute::TRANSIENT  ),
        Property( OUString( "Image"         ), HANDLE_IMAGE     , cppu::UnoType<XBitmap>::get(), PropertyAttribute::TRANSIENT  ),
        Property( OUString( "SubContainer"  ), HANDLE_SUBCONTAINER , cppu::UnoType<OUString>::get(), PropertyAttribute::TRANSIENT  ),
        Property( OUString( "Text"          ), HANDLE_TEXT         , cppu::UnoType<XInterface>::get(), PropertyAttribute::TRANSIENT  )
    };

    // Use it to initialize sequence!
    const Sequence< Property > seqActionTriggerPropertyDescriptor( pActionTriggerPropertys, PROPERTYCOUNT );

    // Return "PropertyDescriptor"
    return seqActionTriggerPropertyDescriptor;
}

bool ActionTriggerPropertySet::impl_tryToChangeProperty(
    const   OUString&    sCurrentValue   ,
    const   Any&        aNewValue       ,
    Any&        aOldValue       ,
    Any&        aConvertedValue )
throw( IllegalArgumentException )
{
    // Set default return value if method failed.
    bool bReturn = false;
    // Get new value from any.
    // IllegalArgumentException() can be thrown!
    OUString sValue;
    convertPropertyValue( sValue, aNewValue );

    // If value change ...
    if( sValue != sCurrentValue )
    {
        // ... set information of change.
        aOldValue       <<= sCurrentValue;
        aConvertedValue <<= sValue;
        // Return OK - "value will be change ..."
        bReturn = true;
    }
    else
    {
        // ... clear information of return parameter!
        aOldValue.clear         ();
        aConvertedValue.clear   ();
        // Return NOTHING - "value will not be change ..."
        bReturn = false;
    }

    return bReturn;
}

bool ActionTriggerPropertySet::impl_tryToChangeProperty(
    const Reference< XBitmap >  aCurrentValue   ,
    const Any&                  aNewValue       ,
    Any&                        aOldValue       ,
    Any&                        aConvertedValue )
throw( IllegalArgumentException )
{
    // Set default return value if method failed.
    bool bReturn = false;
    // Get new value from any.
    // IllegalArgumentException() can be thrown!
    Reference< XBitmap > aValue;
    convertPropertyValue( aValue, aNewValue );

    // If value change ...
    if( aValue != aCurrentValue )
    {
        // ... set information of change.
        aOldValue       <<= aCurrentValue;
        aConvertedValue <<= aValue;
        // Return OK - "value will be change ..."
        bReturn = true;
    }
    else
    {
        // ... clear information of return parameter!
        aOldValue.clear         ();
        aConvertedValue.clear   ();
        // Return NOTHING - "value will not be change ..."
        bReturn = false;
    }

    return bReturn;
}

bool ActionTriggerPropertySet::impl_tryToChangeProperty(
    const Reference< XInterface >   aCurrentValue   ,
    const Any&                      aNewValue       ,
    Any&                            aOldValue       ,
    Any&                            aConvertedValue )
throw( IllegalArgumentException )
{
    // Set default return value if method failed.
    bool bReturn = false;
    // Get new value from any.
    // IllegalArgumentException() can be thrown!
    Reference< XInterface > aValue;
    convertPropertyValue( aValue, aNewValue );

    // If value change ...
    if( aValue != aCurrentValue )
    {
        // ... set information of change.
        aOldValue       <<= aCurrentValue;
        aConvertedValue <<= aValue;
        // Return OK - "value will be change ..."
        bReturn = true;
    }
    else
    {
        // ... clear information of return parameter!
        aOldValue.clear         ();
        aConvertedValue.clear   ();
        // Return NOTHING - "value will not be change ..."
        bReturn = false;
    }

    return bReturn;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
