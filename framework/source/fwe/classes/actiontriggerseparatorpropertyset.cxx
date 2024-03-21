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
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <vcl/svapp.hxx>

using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::awt;

namespace {

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

}

namespace framework
{

ActionTriggerSeparatorPropertySet::ActionTriggerSeparatorPropertySet()
        :   OBroadcastHelper        ( m_aMutex )
        ,   OPropertySetHelper      ( *static_cast< OBroadcastHelper * >(this) )
        ,   m_nSeparatorType( 0 )
{
}

ActionTriggerSeparatorPropertySet::~ActionTriggerSeparatorPropertySet()
{
}

// XInterface
Any SAL_CALL ActionTriggerSeparatorPropertySet::queryInterface( const Type& aType )
{
    Any a = ::cppu::queryInterface(
                aType,
                static_cast< XServiceInfo* >(this),
                static_cast< XTypeProvider* >(this));

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

void ActionTriggerSeparatorPropertySet::acquire() noexcept
{
    OWeakObject::acquire();
}

void ActionTriggerSeparatorPropertySet::release() noexcept
{
    OWeakObject::release();
}

// XServiceInfo
OUString SAL_CALL ActionTriggerSeparatorPropertySet::getImplementationName()
{
    return IMPLEMENTATIONNAME_ACTIONTRIGGERSEPARATOR;
}

sal_Bool SAL_CALL ActionTriggerSeparatorPropertySet::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL ActionTriggerSeparatorPropertySet::getSupportedServiceNames()
{
    Sequence<OUString> seqServiceNames { SERVICENAME_ACTIONTRIGGERSEPARATOR };
    return seqServiceNames;
}

// XTypeProvider
Sequence< Type > SAL_CALL ActionTriggerSeparatorPropertySet::getTypes()
{
    // Create a static typecollection ...
    static ::cppu::OTypeCollection ourTypeCollection(
                        cppu::UnoType<XPropertySet>::get(),
                        cppu::UnoType<XFastPropertySet>::get(),
                        cppu::UnoType<XMultiPropertySet>::get(),
                        cppu::UnoType<XServiceInfo>::get(),
                        cppu::UnoType<XTypeProvider>::get());

    return ourTypeCollection.getTypes();
}

Sequence< sal_Int8 > SAL_CALL ActionTriggerSeparatorPropertySet::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

sal_Bool SAL_CALL ActionTriggerSeparatorPropertySet::convertFastPropertyValue(
    Any&        aConvertedValue,
    Any&        aOldValue,
    sal_Int32   nHandle,
    const Any&  aValue  )
{
    //  Check, if value of property will changed in method "setFastPropertyValue_NoBroadcast()".
    //  Return sal_True, if changed - else return sal_False.
    //  Attention: Method "impl_tryToChangeProperty()" can throw the IllegalArgumentException !!!
    //  Initialize return value with sal_False !!!
    //  (Handle can be invalid)
    bool bReturn = false;

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
{
    SolarMutexGuard aGuard;

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
    SolarMutexGuard aGuard;

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
    // Define static member to give structure of properties to baseclass "OPropertySetHelper".
    // "impl_getStaticPropertyDescriptor" is a non exported and static function, who will define a static propertytable.
    // "true" indicates: Table is sorted by name.
    static OPropertyArrayHelper ourInfoHelper( impl_getStaticPropertyDescriptor(), true );

    return ourInfoHelper;
}

Reference< XPropertySetInfo > SAL_CALL ActionTriggerSeparatorPropertySet::getPropertySetInfo()
{
    // Create structure of propertysetinfo for baseclass "OPropertySetHelper".
    // (Use method "getInfoHelper()".)
    static Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );

    return xInfo;
}

Sequence< Property > ActionTriggerSeparatorPropertySet::impl_getStaticPropertyDescriptor()
{
    return
    {
        Property( "SeparatorType", HANDLE_TYPE, cppu::UnoType<sal_Int16>::get(), PropertyAttribute::TRANSIENT )
    };
}

// static
bool ActionTriggerSeparatorPropertySet::impl_tryToChangeProperty(
    sal_Int16           aCurrentValue   ,
    const   Any&        aNewValue       ,
    Any&                aOldValue       ,
    Any&                aConvertedValue )
{
    // Set default return value if method failed.
    bool bReturn = false;
    // Get new value from any.
    // IllegalArgumentException() can be thrown!
    sal_Int16 aValue = 0;
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
