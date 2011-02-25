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

#include "oox/helper/containerhelper.hxx"

#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <rtl/ustrbuf.hxx>
#include "oox/helper/helper.hxx"

namespace oox {

// ============================================================================

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

// ============================================================================

namespace {

struct ValueRangeComp
{
    inline bool         operator()( const ValueRange& rRange, sal_Int32 nValue ) const { return rRange.mnLast < nValue; }
};

} // namespace

// ----------------------------------------------------------------------------

void ValueRangeSet::insert( const ValueRange& rRange )
{
    // find the first range that contains or follows the starting point of the passed range
    ValueRangeVector::iterator aBeg = maRanges.begin();
    ValueRangeVector::iterator aEnd = maRanges.end();
    ValueRangeVector::iterator aIt = ::std::lower_bound( aBeg, aEnd, rRange.mnFirst, ValueRangeComp() );
    // nothing to do if found range contains passed range
    if( (aIt != aEnd) && aIt->contains( rRange ) ) return;
    // check if previous range can be used to merge with the passed range
    if( (aIt != aBeg) && ((aIt - 1)->mnLast + 1 == rRange.mnFirst) ) --aIt;
    // check if current range (aIt) can be used to merge with passed range
    if( (aIt != aEnd) && aIt->intersects( rRange ) )
    {
        // set new start value to existing range
        aIt->mnFirst = ::std::min( aIt->mnFirst, rRange.mnFirst );
        // search first range that cannot be merged anymore (aNext)
        ValueRangeVector::iterator aNext = aIt + 1;
        while( (aNext != aEnd) && aNext->intersects( rRange ) ) ++aNext;
        // set new end value to existing range
        aIt->mnLast = ::std::max( (aNext - 1)->mnLast, rRange.mnLast );
        // remove ranges covered by new existing range (aIt)
        maRanges.erase( aIt + 1, aNext );
    }
    else
    {
        // merging not possible: insert new range
        maRanges.insert( aIt, rRange );
    }
}

ValueRangeVector ValueRangeSet::getIntersection( const ValueRange& rRange ) const
{
    ValueRangeVector aRanges;
    // find the range that contains nFirst or the first range that follows nFirst
    ValueRangeVector::const_iterator aIt = ::std::lower_bound( maRanges.begin(), maRanges.end(), rRange.mnFirst, ValueRangeComp() );
    for( ValueRangeVector::const_iterator aEnd = maRanges.end(); (aIt != aEnd) && (aIt->mnFirst <= rRange.mnLast); ++aIt )
        aRanges.push_back( ValueRange( ::std::max( aIt->mnFirst, rRange.mnFirst ), ::std::min( aIt->mnLast, rRange.mnLast ) ) );
    return aRanges;
}

// ============================================================================

Reference< XIndexContainer > ContainerHelper::createIndexContainer( const Reference< XComponentContext >& rxContext )
{
    Reference< XIndexContainer > xContainer;
    if( rxContext.is() ) try
    {
        Reference< XMultiServiceFactory > xFactory( rxContext->getServiceManager(), UNO_QUERY_THROW );
        xContainer.set( xFactory->createInstance( CREATE_OUSTRING( "com.sun.star.document.IndexedPropertyValues" ) ), UNO_QUERY_THROW );
    }
    catch( Exception& )
    {
    }
    OSL_ENSURE( xContainer.is(), "ContainerHelper::createIndexContainer - cannot create container" );
    return xContainer;
}

bool ContainerHelper::insertByIndex(
        const Reference< XIndexContainer >& rxIndexContainer,
        sal_Int32 nIndex, const Any& rObject )
{
    OSL_ENSURE( rxIndexContainer.is(), "ContainerHelper::insertByIndex - missing XIndexContainer interface" );
    bool bRet = false;
    try
    {
        rxIndexContainer->insertByIndex( nIndex, rObject );
        bRet = true;
    }
    catch( Exception& )
    {
    }
    OSL_ENSURE( bRet, "ContainerHelper::insertByIndex - cannot insert object" );
    return bRet;
}

Reference< XNameContainer > ContainerHelper::createNameContainer( const Reference< XComponentContext >& rxContext )
{
    Reference< XNameContainer > xContainer;
    if( rxContext.is() ) try
    {
        Reference< XMultiServiceFactory > xFactory( rxContext->getServiceManager(), UNO_QUERY_THROW );
        xContainer.set( xFactory->createInstance( CREATE_OUSTRING( "com.sun.star.document.NamedPropertyValues" ) ), UNO_QUERY_THROW );
    }
    catch( Exception& )
    {
    }
    OSL_ENSURE( xContainer.is(), "ContainerHelper::createNameContainer - cannot create container" );
    return xContainer;
}

OUString ContainerHelper::getUnusedName(
        const Reference< XNameAccess >& rxNameAccess, const OUString& rSuggestedName,
        sal_Unicode cSeparator, sal_Int32 nFirstIndexToAppend )
{
    OSL_ENSURE( rxNameAccess.is(), "ContainerHelper::getUnusedName - missing XNameAccess interface" );

    OUString aNewName = rSuggestedName;
    sal_Int32 nIndex = nFirstIndexToAppend;
    while( rxNameAccess->hasByName( aNewName ) )
        aNewName = OUStringBuffer( rSuggestedName ).append( cSeparator ).append( nIndex++ ).makeStringAndClear();
    return aNewName;
}

bool ContainerHelper::insertByName(
        const Reference< XNameContainer >& rxNameContainer,
        const OUString& rName, const Any& rObject, bool bReplaceOldExisting )
{
    OSL_ENSURE( rxNameContainer.is(), "ContainerHelper::insertByName - missing XNameContainer interface" );
    bool bRet = false;
    try
    {
        if( bReplaceOldExisting && rxNameContainer->hasByName( rName ) )
            rxNameContainer->replaceByName( rName, rObject );
        else
            rxNameContainer->insertByName( rName, rObject );
        bRet = true;
    }
    catch( Exception& )
    {
    }
    OSL_ENSURE( bRet, "ContainerHelper::insertByName - cannot insert object" );
    return bRet;
}

OUString ContainerHelper::insertByUnusedName(
        const Reference< XNameContainer >& rxNameContainer,
        const OUString& rSuggestedName, sal_Unicode cSeparator,
        const Any& rObject, bool bRenameOldExisting )
{
    OSL_ENSURE( rxNameContainer.is(), "ContainerHelper::insertByUnusedName - missing XNameContainer interface" );

    // find an unused name
    Reference< XNameAccess > xNameAccess( rxNameContainer, UNO_QUERY );
    OUString aNewName = getUnusedName( xNameAccess, rSuggestedName, cSeparator );

    // rename existing object
    if( bRenameOldExisting && rxNameContainer->hasByName( rSuggestedName ) )
    {
        try
        {
            Any aOldObject = rxNameContainer->getByName( rSuggestedName );
            rxNameContainer->removeByName( rSuggestedName );
            rxNameContainer->insertByName( aNewName, aOldObject );
            aNewName = rSuggestedName;
        }
        catch( Exception& )
        {
            OSL_ENSURE( false, "ContainerHelper::insertByUnusedName - cannot rename old object" );
        }
    }

    // insert the new object and return its resulting name
    insertByName( rxNameContainer, aNewName, rObject );
    return aNewName;
}

// ============================================================================

} // namespace oox
