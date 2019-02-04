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
#include <comphelper/profilezone.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <svx/svdmodel.hxx>
#include "UnoNameItemTable.hxx"
#include <vcl/svapp.hxx>

#include <svx/unoapi.hxx>
#include <memory>

using namespace ::com::sun::star;
using namespace ::cppu;

SvxUnoNameItemTable::SvxUnoNameItemTable( SdrModel* pModel, sal_uInt16 nWhich, sal_uInt8 nMemberId ) throw()
: mpModel( pModel ),
  mpModelPool( pModel ? &pModel->GetItemPool() : nullptr ),
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
    maItemSetVector.clear();
}

void SvxUnoNameItemTable::Notify( SfxBroadcaster&, const SfxHint& rHint ) throw()
{
    const SdrHint* pSdrHint = dynamic_cast<const SdrHint*>(&rHint);

    if( pSdrHint && SdrHintKind::ModelCleared == pSdrHint->GetKind() )
        dispose();
}

sal_Bool SAL_CALL SvxUnoNameItemTable::supportsService( const  OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

void SvxUnoNameItemTable::ImplInsertByName( const OUString& aName, const uno::Any& aElement )
{
    maItemSetVector.push_back( std::make_unique< SfxItemSet >( *mpModelPool, std::initializer_list<SfxItemSet::Pair>{{mnWhich, mnWhich}} ) );

    std::unique_ptr<NameOrIndex> xNewItem(createItem());
    xNewItem->SetName(aName);
    xNewItem->PutValue(aElement, mnMemberId);
    xNewItem->SetWhich(mnWhich);
    maItemSetVector.back()->Put(*xNewItem);
}

// XNameContainer
void SAL_CALL SvxUnoNameItemTable::insertByName( const OUString& aApiName, const uno::Any& aElement )
{
    SolarMutexGuard aGuard;
    comphelper::ProfileZone aZone("SvxUnoNameItemTable::insertByName");

    if( hasByName( aApiName ) )
        throw container::ElementExistException();

    OUString aName = SvxUnogetInternalNameForItem(mnWhich, aApiName);

    ImplInsertByName( aName, aElement );
}


void SAL_CALL SvxUnoNameItemTable::removeByName( const OUString& aApiName )
{
    SolarMutexGuard aGuard;
    comphelper::ProfileZone aZone("SvxUnoNameItemTable::removeByName");

    // a little quickfix for 2.0 to let applications clear api
    // created items that are not used
    if ( aApiName == "~clear~" )
    {
        dispose();
        return;
    }

    OUString sName = SvxUnogetInternalNameForItem(mnWhich, aApiName);

    auto aIter = std::find_if(maItemSetVector.begin(), maItemSetVector.end(),
        [&](const std::unique_ptr<SfxItemSet>& rpItem) {
            const NameOrIndex *pItem = static_cast<const NameOrIndex *>(&(rpItem->Get( mnWhich ) ));
            return sName == pItem->GetName();
        });
    if (aIter != maItemSetVector.end())
    {
        maItemSetVector.erase( aIter );
        return;
    }

    if (!hasByName(sName))
        throw container::NoSuchElementException();
}

// XNameReplace
void SAL_CALL SvxUnoNameItemTable::replaceByName( const OUString& aApiName, const uno::Any& aElement )
{
    SolarMutexGuard aGuard;

    OUString aName = SvxUnogetInternalNameForItem(mnWhich, aApiName);

    auto aIter = std::find_if(maItemSetVector.begin(), maItemSetVector.end(),
        [&](const std::unique_ptr<SfxItemSet>& rpItem) {
            const NameOrIndex *pItem = static_cast<const NameOrIndex *>(&(rpItem->Get( mnWhich ) ));
            return aName == pItem->GetName();
        });
    if (aIter != maItemSetVector.end())
    {
        std::unique_ptr<NameOrIndex> xNewItem(createItem());
        xNewItem->SetName(aName);
        if (!xNewItem->PutValue(aElement, mnMemberId) || !isValid(xNewItem.get()))
            throw lang::IllegalArgumentException();
        (*aIter)->Put(*xNewItem);
        return;
    }

    // if it is not in our own sets, modify the pool!
    bool bFound = false;

    sal_uInt32 nSurrogate;
    sal_uInt32 nCount = mpModelPool ? mpModelPool->GetItemCount2( mnWhich ) : 0;
    for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
    {
        NameOrIndex *pItem = const_cast<NameOrIndex*>(static_cast<const NameOrIndex*>(mpModelPool->GetItem2( mnWhich, nSurrogate)));
        if (pItem && aName == pItem->GetName())
        {
            pItem->PutValue( aElement, mnMemberId );
            bFound = true;
            break;
        }
    }

    if( !bFound )
        throw container::NoSuchElementException();

    ImplInsertByName( aName, aElement );

    if( !hasByName( aName ) )
        throw container::NoSuchElementException();
}

// XNameAccess
uno::Any SAL_CALL SvxUnoNameItemTable::getByName( const OUString& aApiName )
{
    SolarMutexGuard aGuard;
    comphelper::ProfileZone aZone("SvxUnoNameItemTable::getByName");

    OUString aName = SvxUnogetInternalNameForItem(mnWhich, aApiName);

    uno::Any aAny;

    if (mpModelPool && !aName.isEmpty())
    {
        sal_uInt32 nSurrogate;

        sal_uInt32 nSurrogateCount = mpModelPool ? mpModelPool->GetItemCount2( mnWhich ) : 0;
        for( nSurrogate = 0; nSurrogate < nSurrogateCount; nSurrogate++ )
        {
            const NameOrIndex *pItem = static_cast<const NameOrIndex*>(mpModelPool->GetItem2( mnWhich, nSurrogate ));

            if (isValid(pItem) && aName == pItem->GetName())
            {
                pItem->QueryValue( aAny, mnMemberId );
                return aAny;
            }
        }
    }

    throw container::NoSuchElementException();
}

uno::Sequence< OUString > SAL_CALL SvxUnoNameItemTable::getElementNames(  )
{
    SolarMutexGuard aGuard;

    std::set< OUString > aNameSet;


    const sal_uInt32 nSurrogateCount = mpModelPool ? mpModelPool->GetItemCount2( mnWhich ) : 0;
    sal_uInt32 nSurrogate;
    for( nSurrogate = 0; nSurrogate < nSurrogateCount; nSurrogate++ )
    {
        const NameOrIndex *pItem = static_cast<const NameOrIndex*>(mpModelPool->GetItem2( mnWhich, nSurrogate ));

        if( !isValid( pItem ) )
            continue;

        OUString aApiName = SvxUnogetApiNameForItem(mnWhich, pItem->GetName());
        aNameSet.insert(aApiName);
    }

    return comphelper::containerToSequence(aNameSet);
}

sal_Bool SAL_CALL SvxUnoNameItemTable::hasByName( const OUString& aApiName )
{
    SolarMutexGuard aGuard;

    OUString aName = SvxUnogetInternalNameForItem(mnWhich, aApiName);

    if (aName.isEmpty())
        return false;

    sal_uInt32 nSurrogate;


    sal_uInt32 nCount = mpModelPool ? mpModelPool->GetItemCount2( mnWhich ) : 0;
    for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
    {
        const NameOrIndex *pItem = static_cast<const NameOrIndex*>(mpModelPool->GetItem2( mnWhich, nSurrogate ));
        if (isValid(pItem) && aName == pItem->GetName())
            return true;
    }

    return false;
}

sal_Bool SAL_CALL SvxUnoNameItemTable::hasElements(  )
{
    SolarMutexGuard aGuard;


    sal_uInt32 nSurrogate;
    const sal_uInt32 nSurrogateCount = mpModelPool ? mpModelPool->GetItemCount2( mnWhich ) : 0;
    for( nSurrogate = 0; nSurrogate < nSurrogateCount; nSurrogate++ )
    {
        const NameOrIndex *pItem = static_cast<const NameOrIndex*>(mpModelPool->GetItem2( mnWhich, nSurrogate ));

        if( isValid( pItem ) )
            return true;
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
