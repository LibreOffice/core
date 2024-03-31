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

#include <algorithm>

#include <oox/helper/containerhelper.hxx>

#include <com/sun/star/container/XNameContainer.hpp>
#include <osl/diagnose.h>

namespace oox {

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::uno;

namespace {

struct ValueRangeComp
{
    bool operator()( const ValueRange& rLHS, const ValueRange& rRHS ) const
    {
        return rLHS.mnLast < rRHS.mnFirst;
    }
};

} // namespace

void ValueRangeSet::insert( const ValueRange& rRange )
{
    // find the first range that contains or follows the starting point of the passed range
    ValueRangeVector::iterator aBeg = maRanges.begin();
    ValueRangeVector::iterator aEnd = maRanges.end();
    ValueRangeVector::iterator aIt = ::std::lower_bound( aBeg, aEnd, rRange, ValueRangeComp() );
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

OUString ContainerHelper::getUnusedName(
        const Reference< XNameAccess >& rxNameAccess, const OUString& rSuggestedName,
        sal_Unicode cSeparator )
{
    OSL_ENSURE( rxNameAccess.is(), "ContainerHelper::getUnusedName - missing XNameAccess interface" );

    OUString aNewName = rSuggestedName;
    sal_Int32 nIndex = -1;
    while( rxNameAccess->hasByName( aNewName ) )
        aNewName = rSuggestedName + OUStringChar(cSeparator) + OUString::number( nIndex++ );
    return aNewName;
}

bool ContainerHelper::insertByName(
        const Reference< XNameContainer >& rxNameContainer,
        const OUString& rName, const Any& rObject )
{
    OSL_ENSURE( rxNameContainer.is(), "ContainerHelper::insertByName - missing XNameContainer interface" );
    bool bRet = false;
    try
    {
        if( rxNameContainer->hasByName( rName ) )
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
        const Any& rObject )
{
    OSL_ENSURE( rxNameContainer.is(), "ContainerHelper::insertByUnusedName - missing XNameContainer interface" );

    // find an unused name
    OUString aNewName = getUnusedName( rxNameContainer, rSuggestedName, cSeparator );

    // insert the new object and return its resulting name
    insertByName( rxNameContainer, aNewName, rObject );
    return aNewName;
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
