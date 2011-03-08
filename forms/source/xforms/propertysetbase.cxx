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
#include "precompiled_forms.hxx"

#include "propertysetbase.hxx"

#include <cppuhelper/typeprovider.hxx>  // for getImplementationId()

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <tools/debug.hxx>

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

oslInterlockedCount SAL_CALL PropertyAccessorBase::acquire()
{
    return ++m_refCount;
}

oslInterlockedCount SAL_CALL PropertyAccessorBase::release()
{
    if ( --m_refCount == 0 )
    {
        delete this;
        return 0;
    }
    return m_refCount;
}

PropertySetBase::PropertySetBase( )
    :m_pProperties( NULL )
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
        DBG_ASSERT( !m_aProperties.empty(), "PropertySetBase::getInfoHelper: no registered properties!" );
        m_pProperties = new cppu::OPropertyArrayHelper( &m_aProperties[0], m_aProperties.size(), sal_False );
    }
    return *m_pProperties;
}

Reference< XPropertySetInfo > SAL_CALL PropertySetBase::getPropertySetInfo(  ) throw(RuntimeException)
{
    return cppu::OPropertySetHelper::createPropertySetInfo( getInfoHelper() );
}

void PropertySetBase::registerProperty( const Property& rProperty,
    const ::rtl::Reference< PropertyAccessorBase >& rAccessor )
{
    DBG_ASSERT( rAccessor.get(), "PropertySetBase::registerProperty: invalid property accessor, this will crash!" );
    m_aAccessors.insert( PropertyAccessors::value_type( rProperty.Handle, rAccessor ) );

    DBG_ASSERT( ( rAccessor->isWriteable() == true )
                == ( ( rProperty.Attributes & com::sun::star::beans::PropertyAttribute::READONLY ) == 0 ),
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
            ::rtl::OUString sPropName;
            OSL_VERIFY( rPropertyMetaData.fillPropertyMembersByHandle( &sPropName, NULL, nHandle ) );
            Property aProperty = rPropertyMetaData.getPropertyByName( sPropName );
            // default construct a value of this type
            Any aEmptyValue( NULL, aProperty.Type );
            // insert into the cache
            aPos = m_aCache.insert( PropertyValueCache::value_type( nHandle, aEmptyValue ) ).first;
        }
        catch( Exception& )
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

#if OSL_DEBUG_LEVEL > 0
    ::std::pair< PropertyValueCache::iterator, bool > aInsertResult =
#endif
    m_aCache.insert( PropertyValueCache::value_type( nHandle, aCurrentValue ) );
    DBG_ASSERT( aInsertResult.second, "PropertySetBase::initializePropertyValueCache: already cached a value for this property!" );
}

PropertyAccessorBase& PropertySetBase::locatePropertyHandler( sal_Int32 nHandle ) const
{
    PropertyAccessors::const_iterator aPropertyPos = m_aAccessors.find( nHandle );
    DBG_ASSERT( aPropertyPos != m_aAccessors.end() && aPropertyPos->second.get(),
        "PropertySetBase::locatePropertyHandler: accessor map is corrupted!" );
        // neither should this be called for handles where there is no accessor, nor should a
        // NULL accssor be in the map
    return *aPropertyPos->second;
}

sal_Bool SAL_CALL PropertySetBase::convertFastPropertyValue( Any& rConvertedValue, Any& rOldValue, sal_Int32 nHandle,
    const Any& rValue )
    throw (IllegalArgumentException)
{
    PropertyAccessorBase& rAccessor = locatePropertyHandler( nHandle );
    if ( !rAccessor.approveValue( rValue ) )
        throw IllegalArgumentException( ::rtl::OUString(), *this, 0 );

    rAccessor.getValue( rOldValue );
    if ( rOldValue != rValue )
    {
        rConvertedValue = rValue;   // no conversion at all
        return sal_True;
    }
    return sal_False;
}

void SAL_CALL PropertySetBase::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue )
    throw (Exception)
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
