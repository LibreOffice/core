/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: propertyset.cxx,v $
 * $Revision: 1.5 $
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

#include "oox/helper/propertyset.hxx"
#include <rtl/strbuf.hxx>
#include <osl/diagnose.h>
#include "oox/helper/propertymap.hxx"

using ::rtl::OUString;
using ::rtl::OStringBuffer;
using ::rtl::OUStringToOString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::beans::XPropertySet;

namespace oox {

// ============================================================================

void PropertySet::set( const Reference< XPropertySet >& rxPropSet )
{
    mxPropSet = rxPropSet;
    mxMultiPropSet.set( mxPropSet, UNO_QUERY );
}

// Get properties -------------------------------------------------------------

bool PropertySet::getAnyProperty( Any& orValue, sal_Int32 nPropId ) const
{
    return getAnyProperty( orValue, PropertyMap::getPropertyName( nPropId ) );
}

bool PropertySet::getBoolProperty( sal_Int32 nPropId ) const
{
    Any aAny;
    bool bValue = false;
    return getAnyProperty( aAny, nPropId ) && (aAny >>= bValue) && bValue;
}

void PropertySet::getProperties( Sequence< Any >& orValues, const Sequence< OUString >& rPropNames ) const
{
    if( mxMultiPropSet.is() )   // first try the XMultiPropertySet
    {
        try
        {
            orValues = mxMultiPropSet->getPropertyValues( rPropNames );
        }
        catch( Exception& )
        {
            OSL_ENSURE( false, "PropertySet::getProperties - cannot get all property values" );
        }
    }
    else if( mxPropSet.is() )
    {
        sal_Int32 nLen = rPropNames.getLength();
        const OUString* pPropName = rPropNames.getConstArray();
        const OUString* pPropNameEnd = pPropName + nLen;
        orValues.realloc( nLen );
        Any* pValue = orValues.getArray();
        for( ; pPropName != pPropNameEnd; ++pPropName, ++pValue )
            getAnyProperty( *pValue, *pPropName );
    }
}

// Set properties -------------------------------------------------------------

void PropertySet::setAnyProperty( sal_Int32 nPropId, const Any& rValue )
{
    setAnyProperty( PropertyMap::getPropertyName( nPropId ), rValue );
}

void PropertySet::setProperties( const Sequence< OUString >& rPropNames, const Sequence< Any >& rValues )
{
    OSL_ENSURE( rPropNames.getLength() == rValues.getLength(),
        "PropertySet::setProperties - length of sequences different" );

    if( mxMultiPropSet.is() )   // first try the XMultiPropertySet
    {
        try
        {
            mxMultiPropSet->setPropertyValues( rPropNames, rValues );
        }
        catch( Exception& )
        {
            OSL_ENSURE( false, "PropertySet::setProperties - cannot set all property values" );
        }
    }
    else if( mxPropSet.is() )
    {
        const OUString* pPropName = rPropNames.getConstArray();
        const OUString* pPropNameEnd = pPropName + rPropNames.getLength();
        const Any* pValue = rValues.getConstArray();
        for( ; pPropName != pPropNameEnd; ++pPropName, ++pValue )
            setAnyProperty( *pPropName, *pValue );
    }
}

void PropertySet::setProperties( const PropertyMap& rPropertyMap )
{
    if( !rPropertyMap.empty() )
    {
        Sequence< OUString > aPropNames;
        Sequence< Any > aValues;
        rPropertyMap.fillSequences( aPropNames, aValues );
        setProperties( aPropNames, aValues );
    }
}

// private --------------------------------------------------------------------

bool PropertySet::getAnyProperty( Any& orValue, const OUString& rPropName ) const
{
    bool bHasValue = false;
    try
    {
        if( mxPropSet.is() )
        {
            orValue = mxPropSet->getPropertyValue( rPropName );
            bHasValue = true;
        }
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, OStringBuffer( "PropertySet::getAnyProperty - cannot get property \"" ).
            append( OUStringToOString( rPropName, RTL_TEXTENCODING_ASCII_US ) ).append( '"' ).getStr() );
    }
    return bHasValue;
}

void PropertySet::setAnyProperty( const OUString& rPropName, const Any& rValue )
{
    try
    {
        if( mxPropSet.is() )
            mxPropSet->setPropertyValue( rPropName, rValue );
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, OStringBuffer( "PropertySet::setAnyProperty - cannot set property \"" ).
            append( OUStringToOString( rPropName, RTL_TEXTENCODING_ASCII_US ) ).append( '"' ).getStr() );
    }
}

// ============================================================================

} // namespace oox

