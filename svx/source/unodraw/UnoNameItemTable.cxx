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


#include <set>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <svl/style.hxx>
#include <comphelper/stl_types.hxx>

#include <svx/svdmodel.hxx>
#include "UnoNameItemTable.hxx"
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

#include "svx/unoapi.hxx"

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::cppu;

SvxUnoNameItemTable::SvxUnoNameItemTable( SdrModel* pModel, sal_uInt16 nWhich, sal_uInt8 nMemberId ) throw()
: mpModel( pModel ),
  mpModelPool( pModel ? &pModel->GetItemPool() : NULL ),
  mnWhich( nWhich ), mnMemberId( nMemberId )
{
    if( pModel )
        StartListening( *pModel );
}

SvxUnoNameItemTable::~SvxUnoNameItemTable() throw()
{
    if( mpModel )
        EndListening( *mpModel );
    dispose();
}

bool SvxUnoNameItemTable::isValid( const NameOrIndex* pItem ) const
{
    return pItem && !pItem->GetName().isEmpty();
}

void SvxUnoNameItemTable::dispose()
{
    ItemPoolVector::iterator aIter = maItemSetVector.begin();
    const ItemPoolVector::iterator aEnd = maItemSetVector.end();

    while( aIter != aEnd )
    {
        delete (*aIter++);
    }

    maItemSetVector.clear();
}

void SvxUnoNameItemTable::Notify( SfxBroadcaster&, const SfxHint& rHint ) throw()
{
    const SdrHint* pSdrHint = PTR_CAST( SdrHint, &rHint );

    if( pSdrHint && HINT_MODELCLEARED == pSdrHint->GetKind() )
        dispose();
}

sal_Bool SAL_CALL SvxUnoNameItemTable::supportsService( const  OUString& ServiceName ) throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSNL( getSupportedServiceNames() );
    const OUString * pArray = aSNL.getConstArray();

    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;

    return sal_False;
}

void SAL_CALL SvxUnoNameItemTable::ImplInsertByName( const OUString& aName, const uno::Any& aElement )
{
    SfxItemSet* mpInSet = new SfxItemSet( *mpModelPool, mnWhich, mnWhich );
    maItemSetVector.push_back( mpInSet );

    NameOrIndex* pNewItem = createItem();
    pNewItem->SetName( String( aName ) );
    pNewItem->PutValue( aElement, mnMemberId );
    mpInSet->Put( *pNewItem, mnWhich );
    delete pNewItem;
}

// XNameContainer
void SAL_CALL SvxUnoNameItemTable::insertByName( const OUString& aApiName, const uno::Any& aElement )
    throw( lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    if( hasByName( aApiName ) )
        throw container::ElementExistException();

    OUString aName = SvxUnogetInternalNameForItem(mnWhich, aApiName);

    ImplInsertByName( aName, aElement );
}



void SAL_CALL SvxUnoNameItemTable::removeByName( const OUString& aApiName )
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    // a little quickfix for 2.0 to let applications clear api
    // created items that are not used
    if ( aApiName == "~clear~" )
    {
        dispose();
        return;
    }

    OUString sName = SvxUnogetInternalNameForItem(mnWhich, aApiName);

    ItemPoolVector::iterator aIter = maItemSetVector.begin();
    const ItemPoolVector::iterator aEnd = maItemSetVector.end();

    NameOrIndex *pItem;

    while( aIter != aEnd )
    {
        pItem = (NameOrIndex *)&((*aIter)->Get( mnWhich ) );
        if (sName.equals(pItem->GetName()))
        {
            delete (*aIter);
            maItemSetVector.erase( aIter );
            return;
        }
        ++aIter;
    }

    if (!hasByName(sName))
        throw container::NoSuchElementException();
}

// XNameReplace
void SAL_CALL SvxUnoNameItemTable::replaceByName( const OUString& aApiName, const uno::Any& aElement )
    throw( lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    OUString aName = SvxUnogetInternalNameForItem(mnWhich, aApiName);

    ItemPoolVector::iterator aIter = maItemSetVector.begin();
    const ItemPoolVector::iterator aEnd = maItemSetVector.end();

    NameOrIndex *pItem;

    while( aIter != aEnd )
    {
        pItem = (NameOrIndex *)&((*aIter)->Get( mnWhich ) );
        if (aName.equals(pItem->GetName()))
        {
            NameOrIndex* pNewItem = createItem();
            pNewItem->SetName(aName);
            if( !pNewItem->PutValue( aElement, mnMemberId ) || !isValid( pNewItem ) )
                throw lang::IllegalArgumentException();

            (*aIter)->Put( *pNewItem );
            return;
        }
        ++aIter;
    }

    // if it is not in our own sets, modify the pool!
    bool bFound = false;

    sal_uInt32 nSurrogate;
    sal_uInt32 nCount = mpModelPool ? mpModelPool->GetItemCount2( mnWhich ) : 0;
    for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
    {
        pItem = (NameOrIndex*)mpModelPool->GetItem2( mnWhich, nSurrogate);
        if (pItem && aName.equals(pItem->GetName()))
        {
            pItem->PutValue( aElement, mnMemberId );
            bFound = true;
            break;
        }
    }

    if( bFound )
        ImplInsertByName( aName, aElement );
    else
        throw container::NoSuchElementException();

    if( !hasByName( aName ) )
        throw container::NoSuchElementException();
}

// XNameAccess
uno::Any SAL_CALL SvxUnoNameItemTable::getByName( const OUString& aApiName )
    throw( container::NoSuchElementException,  lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    OUString aName = SvxUnogetInternalNameForItem(mnWhich, aApiName);

    uno::Any aAny;

    if (mpModelPool && !aName.isEmpty())
    {
        NameOrIndex *pItem;
        sal_uInt32 nSurrogate;

        sal_uInt32 nSurrogateCount = mpModelPool ? mpModelPool->GetItemCount2( mnWhich ) : 0;
        for( nSurrogate = 0; nSurrogate < nSurrogateCount; nSurrogate++ )
        {
            pItem = (NameOrIndex*)mpModelPool->GetItem2( mnWhich, nSurrogate );

            if (isValid(pItem) && aName.equals(pItem->GetName()))
            {
                pItem->QueryValue( aAny, mnMemberId );
                return aAny;
            }
        }
    }

    throw container::NoSuchElementException();
}

uno::Sequence< OUString > SAL_CALL SvxUnoNameItemTable::getElementNames(  )
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    std::set< OUString, comphelper::UStringLess > aNameSet;

    NameOrIndex *pItem;

    const sal_uInt32 nSurrogateCount = mpModelPool ? mpModelPool->GetItemCount2( mnWhich ) : 0;
    sal_uInt32 nSurrogate;
    for( nSurrogate = 0; nSurrogate < nSurrogateCount; nSurrogate++ )
    {
        pItem = (NameOrIndex*)mpModelPool->GetItem2( mnWhich, nSurrogate );

        if( !isValid( pItem ) )
            continue;

        OUString aApiName = SvxUnogetApiNameForItem(mnWhich, pItem->GetName());
        aNameSet.insert(aApiName);
    }

    uno::Sequence< OUString > aSeq( aNameSet.size() );
    OUString* pNames = aSeq.getArray();

    std::set< OUString, comphelper::UStringLess >::iterator aIter( aNameSet.begin() );
    const std::set< OUString, comphelper::UStringLess >::iterator aEnd( aNameSet.end() );

    while( aIter != aEnd )
    {
        *pNames++ = *aIter++;
    }

    return aSeq;
}

sal_Bool SAL_CALL SvxUnoNameItemTable::hasByName( const OUString& aApiName )
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    OUString aName = SvxUnogetInternalNameForItem(mnWhich, aApiName);

    if (aName.isEmpty())
        return sal_False;

    sal_uInt32 nSurrogate;

    const NameOrIndex *pItem;

    sal_uInt32 nCount = mpModelPool ? mpModelPool->GetItemCount2( mnWhich ) : 0;
    for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
    {
        pItem = (NameOrIndex*)mpModelPool->GetItem2( mnWhich, nSurrogate );
        if (isValid(pItem) && aName.equals(pItem->GetName()))
            return sal_True;
    }

    return sal_False;
}

sal_Bool SAL_CALL SvxUnoNameItemTable::hasElements(  )
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    const NameOrIndex *pItem;

    sal_uInt32 nSurrogate;
    const sal_uInt32 nSurrogateCount = mpModelPool ? mpModelPool->GetItemCount2( mnWhich ) : 0;
    for( nSurrogate = 0; nSurrogate < nSurrogateCount; nSurrogate++ )
    {
        pItem = (NameOrIndex*)mpModelPool->GetItem2( mnWhich, nSurrogate );

        if( isValid( pItem ) )
            return sal_True;
    }

    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
