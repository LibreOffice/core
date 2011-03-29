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
#include "precompiled_svx.hxx"

#include "propertyset.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

namespace comphelper {

// -----------------------------------------------------------------------------
// FastPropertySetInfo
// -----------------------------------------------------------------------------

FastPropertySetInfo::FastPropertySetInfo()
{
}

// -----------------------------------------------------------------------------

FastPropertySetInfo::FastPropertySetInfo( const PropertyVector& rProps )
{
    addProperties( rProps );
}

// -----------------------------------------------------------------------------

FastPropertySetInfo::~FastPropertySetInfo()
{
}

// -----------------------------------------------------------------------------

void FastPropertySetInfo::addProperty( const Property& rProperty )
{
    maProperties.push_back( rProperty );
    maMap[ rProperty.Name ] = maProperties.size() - 1;
}

// -----------------------------------------------------------------------------

void FastPropertySetInfo::addProperties( const PropertyVector& rProps )
{
    sal_uInt32 nIndex = maProperties.size();
    sal_uInt32 nCount = rProps.size();
    maProperties.resize( nIndex + nCount );
    PropertyVector::const_iterator aIter( rProps.begin() );
    while( nCount-- )
    {
        const Property& rProperty = (*aIter++);
        maProperties[nIndex] = rProperty;
        maMap[ rProperty.Name ] = nIndex++;
    }
}

// -----------------------------------------------------------------------------

const Property& FastPropertySetInfo::getProperty( const OUString& aName ) throw (UnknownPropertyException )
{
    PropertyMap::iterator aIter( maMap.find( aName ) );
    if( aIter == maMap.end() )
        throw UnknownPropertyException();
    return maProperties[(*aIter).second];
}

// -----------------------------------------------------------------------------

const Property* FastPropertySetInfo::hasProperty( const OUString& aName )
{
    PropertyMap::iterator aIter( maMap.find( aName ) );
    if( aIter == maMap.end() )
        return 0;
    else
        return &maProperties[(*aIter).second];
}

// -----------------------------------------------------------------------------
// XPropertySetInfo
// -----------------------------------------------------------------------------

Sequence< Property > SAL_CALL FastPropertySetInfo::getProperties() throw (RuntimeException)
{
    return Sequence< Property >( &maProperties[0], maProperties.size() );
}

// -----------------------------------------------------------------------------

Property SAL_CALL FastPropertySetInfo::getPropertyByName( const OUString& aName ) throw (UnknownPropertyException, RuntimeException)
{
    return getProperty( aName );
}

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL FastPropertySetInfo::hasPropertyByName( const OUString& aName ) throw (RuntimeException)
{
    return hasProperty( aName ) != 0 ? sal_True : sal_False;;
}

// -----------------------------------------------------------------------------
// FastPropertySet
// -----------------------------------------------------------------------------

FastPropertySet::FastPropertySet( const rtl::Reference< FastPropertySetInfo >& xInfo )
: mxInfo( xInfo )
{
}

// -----------------------------------------------------------------------------

FastPropertySet::~FastPropertySet()
{
}

// -----------------------------------------------------------------------------
// XPropertySet
// -----------------------------------------------------------------------------

Reference< XPropertySetInfo > SAL_CALL FastPropertySet::getPropertySetInfo(  ) throw (RuntimeException)
{
    return Reference< XPropertySetInfo >( mxInfo.get() );
}

// -----------------------------------------------------------------------------

void SAL_CALL FastPropertySet::setPropertyValue( const OUString& aPropertyName, const Any& aValue ) throw (UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    setFastPropertyValue( mxInfo->getProperty( aPropertyName ).Handle, aValue );
}

// -----------------------------------------------------------------------------

Any SAL_CALL FastPropertySet::getPropertyValue( const OUString& aPropertyName ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    return getFastPropertyValue( mxInfo->getProperty( aPropertyName ).Handle );
}

// -----------------------------------------------------------------------------

void SAL_CALL FastPropertySet::addPropertyChangeListener( const OUString&, const Reference< XPropertyChangeListener >& ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

// -----------------------------------------------------------------------------

void SAL_CALL FastPropertySet::removePropertyChangeListener( const OUString&, const Reference< XPropertyChangeListener >& ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

// -----------------------------------------------------------------------------

void SAL_CALL FastPropertySet::addVetoableChangeListener( const OUString&, const Reference< XVetoableChangeListener >& ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

// -----------------------------------------------------------------------------

void SAL_CALL FastPropertySet::removeVetoableChangeListener( const OUString&, const Reference< XVetoableChangeListener >& ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

// -----------------------------------------------------------------------------
// XMultiPropertySet
// -----------------------------------------------------------------------------

void SAL_CALL FastPropertySet::setPropertyValues( const Sequence< OUString >& aPropertyNames, const Sequence< Any >& aValues ) throw (PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    const OUString* pPropertyNames = aPropertyNames.getConstArray();
    const Any* pValues = aValues.getConstArray();
    sal_Int32 nCount = aPropertyNames.getLength();
    if( nCount != aValues.getLength() )
        throw IllegalArgumentException();

    while( nCount-- )
    {
        const Property* pProperty = mxInfo->hasProperty( *pPropertyNames++ );
        if( pProperty ) try
        {
            setFastPropertyValue( pProperty->Handle, *pValues );
        }
        catch( UnknownPropertyException& )
        {
        }
        pValues++;
    }
}

// -----------------------------------------------------------------------------

Sequence< Any > SAL_CALL FastPropertySet::getPropertyValues( const Sequence< OUString >& aPropertyNames ) throw (RuntimeException)
{
    sal_Int32 nCount = aPropertyNames.getLength();
    Sequence< Any > aValues( nCount );

    const OUString* pPropertyNames = aPropertyNames.getConstArray();
    Any* pValues = aValues.getArray();
    while( nCount-- )
    {
        const Property* pProperty = mxInfo->hasProperty( *pPropertyNames++ );
        if( pProperty ) try
        {
            *pValues = getFastPropertyValue( pProperty->Handle );
        }
        catch( UnknownPropertyException& )
        {
        }
        pValues++;
    }
    return aValues;
}

// -----------------------------------------------------------------------------

void SAL_CALL FastPropertySet::addPropertiesChangeListener( const Sequence< OUString >&, const Reference< XPropertiesChangeListener >& ) throw (RuntimeException)
{
}

// -----------------------------------------------------------------------------

void SAL_CALL FastPropertySet::removePropertiesChangeListener( const Reference< XPropertiesChangeListener >& ) throw (RuntimeException)
{
}

// -----------------------------------------------------------------------------

void SAL_CALL FastPropertySet::firePropertiesChangeEvent( const Sequence< OUString >&, const Reference< XPropertiesChangeListener >& ) throw (RuntimeException)
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
