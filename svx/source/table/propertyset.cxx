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


#include "propertyset.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

namespace sdr::table {

FastPropertySetInfo::FastPropertySetInfo( const PropertyVector& rProps )
{
    addProperties( rProps );
}


FastPropertySetInfo::~FastPropertySetInfo()
{
}


void FastPropertySetInfo::addProperties( const PropertyVector& rProps )
{
    sal_uInt32 nIndex = maProperties.size();
    sal_uInt32 nCount = rProps.size();
    maProperties.resize( nIndex + nCount );
    for( const Property& rProperty : rProps )
    {
        maProperties[nIndex] = rProperty;
        maMap[ rProperty.Name ] = nIndex++;
    }
}


const Property& FastPropertySetInfo::getProperty( const OUString& aName )
{
    PropertyMap::iterator aIter( maMap.find( aName ) );
    if( aIter == maMap.end() )
        throw UnknownPropertyException( aName, static_cast<cppu::OWeakObject*>(this));
    return maProperties[(*aIter).second];
}


const Property* FastPropertySetInfo::hasProperty( const OUString& aName )
{
    PropertyMap::iterator aIter( maMap.find( aName ) );
    if( aIter == maMap.end() )
        return nullptr;
    else
        return &maProperties[(*aIter).second];
}


// XPropertySetInfo


Sequence< Property > SAL_CALL FastPropertySetInfo::getProperties()
{
    return Sequence< Property >( maProperties.data(), maProperties.size() );
}


Property SAL_CALL FastPropertySetInfo::getPropertyByName( const OUString& aName )
{
    return getProperty( aName );
}


sal_Bool SAL_CALL FastPropertySetInfo::hasPropertyByName( const OUString& aName )
{
    return hasProperty( aName ) != nullptr;
}

FastPropertySet::FastPropertySet( const rtl::Reference< FastPropertySetInfo >& xInfo )
: mxInfo( xInfo )
{
}


FastPropertySet::~FastPropertySet()
{
}


// XPropertySet


Reference< XPropertySetInfo > SAL_CALL FastPropertySet::getPropertySetInfo(  )
{
    return mxInfo;
}


void SAL_CALL FastPropertySet::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
{
    setFastPropertyValue( mxInfo->getProperty( aPropertyName ).Handle, aValue );
}


Any SAL_CALL FastPropertySet::getPropertyValue( const OUString& aPropertyName )
{
    return getFastPropertyValue( mxInfo->getProperty( aPropertyName ).Handle );
}


void SAL_CALL FastPropertySet::addPropertyChangeListener( const OUString&, const Reference< XPropertyChangeListener >& )
{
}


void SAL_CALL FastPropertySet::removePropertyChangeListener( const OUString&, const Reference< XPropertyChangeListener >& )
{
}


void SAL_CALL FastPropertySet::addVetoableChangeListener( const OUString&, const Reference< XVetoableChangeListener >& )
{
}


void SAL_CALL FastPropertySet::removeVetoableChangeListener( const OUString&, const Reference< XVetoableChangeListener >& )
{
}


// XMultiPropertySet


void SAL_CALL FastPropertySet::setPropertyValues( const Sequence< OUString >& aPropertyNames, const Sequence< Any >& aValues )
{
    if( aPropertyNames.getLength() != aValues.getLength() )
        throw IllegalArgumentException();

    const Any* pValues = aValues.getConstArray();
    for( const OUString& rPropertyName : aPropertyNames )
    {
        const Property* pProperty = mxInfo->hasProperty( rPropertyName );
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


Sequence< Any > SAL_CALL FastPropertySet::getPropertyValues( const Sequence< OUString >& aPropertyNames )
{
    sal_Int32 nCount = aPropertyNames.getLength();
    Sequence< Any > aValues( nCount );

    Any* pValues = aValues.getArray();
    for( const OUString& rPropertyName : aPropertyNames )
    {
        const Property* pProperty = mxInfo->hasProperty( rPropertyName );
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


void SAL_CALL FastPropertySet::addPropertiesChangeListener( const Sequence< OUString >&, const Reference< XPropertiesChangeListener >& )
{
}


void SAL_CALL FastPropertySet::removePropertiesChangeListener( const Reference< XPropertiesChangeListener >& )
{
}


void SAL_CALL FastPropertySet::firePropertiesChangeEvent( const Sequence< OUString >&, const Reference< XPropertiesChangeListener >& )
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
