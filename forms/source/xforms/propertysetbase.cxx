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


#include "propertysetbase.hxx"

#include <cppuhelper/typeprovider.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <tools/solar.h>

#include <vector>

using com::sun::star::uno::Any;
using com::sun::star::uno::Type;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::lang::IllegalArgumentException;
using com::sun::star::beans::Property;
using com::sun::star::beans::XPropertySetInfo;

PropertyAccessorBase::~PropertyAccessorBase()
{
}

PropertySetBase::PropertySetBase( )
    :m_pProperties( nullptr )
{
}

PropertySetBase::~PropertySetBase( )
{
    DELETEZ( m_pProperties );
}

cppu::IPropertyArrayHelper& SAL_CALL PropertySetBase::getInfoHelper()
{
    if ( !m_pProperties )
    {
        OSL_ENSURE( !m_aProperties.empty(), "PropertySetBase::getInfoHelper: no registered properties!" );
        m_pProperties = new cppu::OPropertyArrayHelper( &m_aProperties[0], m_aProperties.size(), sal_False );
    }
    return *m_pProperties;
}

Reference< XPropertySetInfo > SAL_CALL PropertySetBase::getPropertySetInfo(  ) throw(RuntimeException, std::exception)
{
    return cppu::OPropertySetHelper::createPropertySetInfo( getInfoHelper() );
}

void PropertySetBase::registerProperty( const Property& rProperty,
    const ::rtl::Reference< PropertyAccessorBase >& rAccessor )
{
    OSL_ENSURE( rAccessor.get(), "PropertySetBase::registerProperty: invalid property accessor, this will crash!" );
    m_aAccessors.insert( PropertyAccessors::value_type( rProperty.Handle, rAccessor ) );

    OSL_ENSURE( rAccessor->isWriteable()
                == ( ( rProperty.Attributes & css::beans::PropertyAttribute::READONLY ) == 0 ),
        "PropertySetBase::registerProperty: inconsistence!" );

    m_aProperties.push_back( rProperty );
}

void PropertySetBase::notifyAndCachePropertyValue( sal_Int32 nHandle )
{
    ::osl::ClearableMutexGuard aGuard( GetMutex() );

    PropertyValueCache::iterator aPos = m_aCache.find( nHandle );
    if ( aPos == m_aCache.end() )
    {   // method has never before been invoked for this property
        try
        {
            // determine the type of this property
            ::cppu::IPropertyArrayHelper& rPropertyMetaData = getInfoHelper();
            OUString sPropName;
            OSL_VERIFY( rPropertyMetaData.fillPropertyMembersByHandle( &sPropName, nullptr, nHandle ) );
            Property aProperty = rPropertyMetaData.getPropertyByName( sPropName );
            // default construct a value of this type
            Any aEmptyValue( nullptr, aProperty.Type );
            // insert into the cache
            aPos = m_aCache.insert( PropertyValueCache::value_type( nHandle, aEmptyValue ) ).first;
        }
        catch( const Exception& )
        {
            OSL_FAIL( "PropertySetBase::notifyAndCachePropertyValue: this is not expected to fail!" );
        }
    }
    Any aOldValue = aPos->second;
    // determine the current value
    Any aNewValue;
    getFastPropertyValue( aNewValue, nHandle );
    // remember the old value
    aPos->second = aNewValue;

    aGuard.clear();
    if ( aNewValue != aOldValue )
        firePropertyChange( nHandle, aNewValue, aOldValue );
}

void PropertySetBase::initializePropertyValueCache( sal_Int32 nHandle )
{
    Any aCurrentValue;
    getFastPropertyValue( aCurrentValue, nHandle );

    ::std::pair< PropertyValueCache::iterator, bool > aInsertResult =
          m_aCache.insert( PropertyValueCache::value_type( nHandle, aCurrentValue ) );
    OSL_ENSURE( aInsertResult.second, "PropertySetBase::initializePropertyValueCache: already cached a value for this property!" );
}

PropertyAccessorBase& PropertySetBase::locatePropertyHandler( sal_Int32 nHandle ) const
{
    PropertyAccessors::const_iterator aPropertyPos = m_aAccessors.find( nHandle );
    OSL_ENSURE( aPropertyPos != m_aAccessors.end() && aPropertyPos->second.get(),
        "PropertySetBase::locatePropertyHandler: accessor map is corrupted!" );
        // neither should this be called for handles where there is no accessor, nor should a
        // NULL accessor be in the map
    return *aPropertyPos->second;
}

sal_Bool SAL_CALL PropertySetBase::convertFastPropertyValue( Any& rConvertedValue, Any& rOldValue, sal_Int32 nHandle,
    const Any& rValue )
    throw (IllegalArgumentException)
{
    PropertyAccessorBase& rAccessor = locatePropertyHandler( nHandle );
    if ( !rAccessor.approveValue( rValue ) )
        throw IllegalArgumentException( OUString(), *this, 0 );

    rAccessor.getValue( rOldValue );
    if ( rOldValue != rValue )
    {
        rConvertedValue = rValue;   // no conversion at all
        return sal_True;
    }
    return sal_False;
}

void SAL_CALL PropertySetBase::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue )
    throw (Exception, std::exception)
{
    PropertyAccessorBase& rAccessor = locatePropertyHandler( nHandle );
    rAccessor.setValue( rValue );
}

void SAL_CALL PropertySetBase::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    PropertyAccessorBase& rAccessor = locatePropertyHandler( nHandle );
    rAccessor.getValue( rValue );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
