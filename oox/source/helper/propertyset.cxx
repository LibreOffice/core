/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include "oox/helper/propertyset.hxx"

#include <osl/diagnose.h>
#include <rtl/strbuf.hxx>
#include "oox/helper/propertymap.hxx"

namespace oox {

// ============================================================================

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

using ::rtl::OStringBuffer;
using ::rtl::OUString;
using ::rtl::OUStringToOString;

// ============================================================================

void PropertySet::set( const Reference< XPropertySet >& rxPropSet )
{
    mxPropSet = rxPropSet;
    mxMultiPropSet.set( mxPropSet, UNO_QUERY );
    if( mxPropSet.is() ) try
    {
        mxPropSetInfo = mxPropSet->getPropertySetInfo();
    }
    catch( Exception& )
    {
    }
}

bool PropertySet::hasProperty( sal_Int32 nPropId ) const
{
    if( mxPropSetInfo.is() ) try
    {
        const OUString& rPropName = PropertyMap::getPropertyName( nPropId );
        return mxPropSetInfo->hasPropertyByName( rPropName );
    }
    catch( Exception& )
    {
    }
    return false;
}

// Get properties -------------------------------------------------------------

Any PropertySet::getAnyProperty( sal_Int32 nPropId ) const
{
    Any aValue;
    return implGetPropertyValue( aValue, PropertyMap::getPropertyName( nPropId ) ) ? aValue : Any();
}

void PropertySet::getProperties( Sequence< Any >& orValues, const Sequence< OUString >& rPropNames ) const
{
    if( mxMultiPropSet.is() ) try
    {
        orValues = mxMultiPropSet->getPropertyValues( rPropNames );
        return;
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "PropertySet::getProperties - cannot get all property values - fallback to single mode" );
    }

    if( mxPropSet.is() )
    {
        sal_Int32 nLen = rPropNames.getLength();
        const OUString* pPropName = rPropNames.getConstArray();
        const OUString* pPropNameEnd = pPropName + nLen;
        orValues.realloc( nLen );
        Any* pValue = orValues.getArray();
        for( ; pPropName != pPropNameEnd; ++pPropName, ++pValue )
            implGetPropertyValue( *pValue, *pPropName );
    }
}

// Set properties -------------------------------------------------------------

bool PropertySet::setAnyProperty( sal_Int32 nPropId, const Any& rValue )
{
    return implSetPropertyValue( PropertyMap::getPropertyName( nPropId ), rValue );
}

void PropertySet::setProperties( const Sequence< OUString >& rPropNames, const Sequence< Any >& rValues )
{
    OSL_ENSURE( rPropNames.getLength() == rValues.getLength(),
        "PropertySet::setProperties - length of sequences different" );

    if( mxMultiPropSet.is() ) try
    {
        mxMultiPropSet->setPropertyValues( rPropNames, rValues );
        return;
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "PropertySet::setProperties - cannot set all property values, fallback to single mode" );
    }

    if( mxPropSet.is() )
    {
        const OUString* pPropName = rPropNames.getConstArray();
        const OUString* pPropNameEnd = pPropName + rPropNames.getLength();
        const Any* pValue = rValues.getConstArray();
        for( ; pPropName != pPropNameEnd; ++pPropName, ++pValue )
            implSetPropertyValue( *pPropName, *pValue );
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

bool PropertySet::implGetPropertyValue( Any& orValue, const OUString& rPropName ) const
{
    if( mxPropSet.is() ) try
    {
        orValue = mxPropSet->getPropertyValue( rPropName );
        return true;
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, OStringBuffer( "PropertySet::implGetPropertyValue - cannot get property \"" ).
            append( OUStringToOString( rPropName, RTL_TEXTENCODING_ASCII_US ) ).append( '"' ).getStr() );
    }
    return false;
}

bool PropertySet::implSetPropertyValue( const OUString& rPropName, const Any& rValue )
{
    if( mxPropSet.is() ) try
    {
        mxPropSet->setPropertyValue( rPropName, rValue );
        return true;
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, OStringBuffer( "PropertySet::implSetPropertyValue - cannot set property \"" ).
            append( OUStringToOString( rPropName, RTL_TEXTENCODING_ASCII_US ) ).append( '"' ).getStr() );
    }
    return false;
}

// ============================================================================

} // namespace oox
