/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "oox/helper/containerhelper.hxx"

#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <rtl/ustrbuf.hxx>
#include "oox/helper/helper.hxx"

namespace oox {



using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;



namespace {

struct ValueRangeComp
{
    inline bool operator()( const ValueRange& rLHS, const ValueRange& rRHS ) const
    {
        return rLHS.mnLast < rRHS.mnFirst;
    }
};

} 



void ValueRangeSet::insert( const ValueRange& rRange )
{
    
    ValueRangeVector::iterator aBeg = maRanges.begin();
    ValueRangeVector::iterator aEnd = maRanges.end();
    ValueRangeVector::iterator aIt = ::std::lower_bound( aBeg, aEnd, rRange, ValueRangeComp() );
    
    if( (aIt != aEnd) && aIt->contains( rRange ) ) return;
    
    if( (aIt != aBeg) && ((aIt - 1)->mnLast + 1 == rRange.mnFirst) ) --aIt;
    
    if( (aIt != aEnd) && aIt->intersects( rRange ) )
    {
        
        aIt->mnFirst = ::std::min( aIt->mnFirst, rRange.mnFirst );
        
        ValueRangeVector::iterator aNext = aIt + 1;
        while( (aNext != aEnd) && aNext->intersects( rRange ) ) ++aNext;
        
        aIt->mnLast = ::std::max( (aNext - 1)->mnLast, rRange.mnLast );
        
        maRanges.erase( aIt + 1, aNext );
    }
    else
    {
        
        maRanges.insert( aIt, rRange );
    }
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

    
    OUString aNewName = getUnusedName( rxNameContainer, rSuggestedName, cSeparator );

    
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
            OSL_FAIL( "ContainerHelper::insertByUnusedName - cannot rename old object" );
        }
    }

    
    insertByName( rxNameContainer, aNewName, rObject );
    return aNewName;
}



} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
