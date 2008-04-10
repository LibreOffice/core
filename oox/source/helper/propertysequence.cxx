/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: propertysequence.cxx,v $
 * $Revision: 1.3 $
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

#include "oox/helper/propertysequence.hxx"
#include "oox/helper/propertyset.hxx"
#include <algorithm>
#include <osl/diagnose.h>
#include <com/sun/star/beans/PropertyValue.hpp>

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::PropertyValue;

namespace oox {

// ============================================================================

PropertySequence::PropertySequence( const sal_Char* const* ppcPropNames,
        const sal_Char* const* ppcPropNames2, const sal_Char* const* ppcPropNames3 ) :
    mnNextIndex( 0 )
{
    OSL_ENSURE( ppcPropNames, "PropertySequence::PropertySequence - no strings found" );

    // create OUStrings from ASCII property names
    typedef ::std::pair< OUString, size_t >     IndexedOUString;
    typedef ::std::vector< IndexedOUString >    IndexedOUStringVec;
    IndexedOUStringVec aPropNameVec;
    size_t nVecIdx = 0;
    while( *ppcPropNames )
    {
        OUString aPropName = OUString::createFromAscii( *ppcPropNames++ );
        aPropNameVec.push_back( IndexedOUString( aPropName, nVecIdx++ ) );
    }
    if( ppcPropNames2 ) while( *ppcPropNames2 )
    {
        OUString aPropName = OUString::createFromAscii( *ppcPropNames2++ );
        aPropNameVec.push_back( IndexedOUString( aPropName, nVecIdx++ ) );
    }
    if( ppcPropNames3 ) while( *ppcPropNames3 )
    {
        OUString aPropName = OUString::createFromAscii( *ppcPropNames3++ );
        aPropNameVec.push_back( IndexedOUString( aPropName, nVecIdx++ ) );
    }

    // sorts the pairs, which will be sorted by first component (the property name)
    ::std::sort( aPropNameVec.begin(), aPropNameVec.end() );

    // resize member sequences
    size_t nSize = aPropNameVec.size();
    maNameSeq.realloc( static_cast< sal_Int32 >( nSize ) );
    maValueSeq.realloc( static_cast< sal_Int32 >( nSize ) );
    maNameOrder.resize( nSize );

    // fill the property name sequence and store original sort order
    sal_Int32 nSeqIdx = 0;
    for( IndexedOUStringVec::const_iterator aIt = aPropNameVec.begin(),
            aEnd = aPropNameVec.end(); aIt != aEnd; ++aIt, ++nSeqIdx )
    {
        maNameSeq[ nSeqIdx ] = aIt->first;
        maNameOrder[ aIt->second ] = nSeqIdx;
    }
}

void PropertySequence::clearAllAnys()
{
    for( sal_Int32 nIdx = 0, nLen = maValueSeq.getLength(); nIdx < nLen; ++nIdx )
        maValueSeq[ nIdx ].clear();
}

// read properties ------------------------------------------------------------

void PropertySequence::readFromPropertySet( const PropertySet& rPropSet )
{
    rPropSet.getProperties( maValueSeq, maNameSeq );
    mnNextIndex = 0;
}

bool PropertySequence::readValue( Any& rAny )
{
    Any* pAny = getNextAny();
    if( pAny ) rAny = *pAny;
    return pAny != 0;
}

// write properties -----------------------------------------------------------

void PropertySequence::writeValue( const Any& rAny )
{
    if( Any* pAny = getNextAny() )
        *pAny = rAny;
}

void PropertySequence::writeToPropertySet( PropertySet& rPropSet )
{
    OSL_ENSURE( mnNextIndex == maNameOrder.size(), "PropertySequence::writeToPropertySet - sequence not complete" );
    rPropSet.setProperties( maNameSeq, maValueSeq );
    mnNextIndex = 0;
}

Sequence< PropertyValue > PropertySequence::createPropertySequence()
{
    OSL_ENSURE( mnNextIndex == maNameOrder.size(), "PropertySequence::createPropertySequence - sequence not complete" );
    Sequence< PropertyValue > aPropSeq( maNameSeq.getLength() );
    PropertyValue* pProp = aPropSeq.getArray();
    PropertyValue* pPropEnd = pProp + aPropSeq.getLength();
    const OUString* pName = maNameSeq.getConstArray();
    const Any* pValue = maValueSeq.getConstArray();
    for( ; pProp != pPropEnd; ++pProp, ++pName, ++pValue )
    {
        pProp->Name = *pName;
        pProp->Value = *pValue;
    }
    mnNextIndex = 0;
    return aPropSeq;
}

// private --------------------------------------------------------------------

Any* PropertySequence::getNextAny()
{
    OSL_ENSURE( mnNextIndex < maNameOrder.size(), "PropertySequence::getNextAny - sequence overflow" );
    Any* pAny = 0;
    if( mnNextIndex < maNameOrder.size() )
        pAny = &maValueSeq[ maNameOrder[ mnNextIndex++ ] ];
    return pAny;
}

// ============================================================================

} // namespace oox

