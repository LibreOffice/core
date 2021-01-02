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

namespace
{
    // We need to override operator== here and specifically bypass the assert
    // in SfxPoolItem::operator== in order to make the FindItemSurrogate call
    // in SvxUnoNameItemTable::hasByName safe.
    class SampleItem : public NameOrIndex
    {
    public:
        SampleItem(sal_uInt16 nWhich, const OUString& rName) : NameOrIndex(nWhich, rName) {}

        bool operator==(const SfxPoolItem& rCmp) const
        {
            assert(dynamic_cast<const NameOrIndex*>(&rCmp) && "comparing different pool item subclasses");
            auto const & rOther = static_cast<const NameOrIndex&>(rCmp);
            return GetName() == rOther.GetName() && GetPalIndex() == rOther.GetPalIndex();
        }
    };

}


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
    if (rHint.GetId() != SfxHintId::ThisIsAnSdrHint)
        return;
    const SdrHint* pSdrHint = static_cast<const SdrHint*>(&rHint);
    if( SdrHintKind::ModelCleared == pSdrHint->GetKind() )
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

    if (mpModelPool)
    {
        NameOrIndex aSample(mnWhich, aName);
        for (const SfxPoolItem* pNameOrIndex : mpModelPool->FindItemSurrogate(mnWhich, aSample))
            if (isValid(static_cast<const NameOrIndex*>(pNameOrIndex)))
            {
                const_cast<SfxPoolItem*>(pNameOrIndex)->PutValue( aElement, mnMemberId );
                bFound = true;
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

    if (mpModelPool && !aName.isEmpty())
    {
        SampleItem aSample(mnWhich, aName);
        for (const SfxPoolItem* pFindItem : mpModelPool->FindItemSurrogate(mnWhich, aSample))
            if (isValid(static_cast<const NameOrIndex*>(pFindItem)))
            {
                uno::Any aAny;
                pFindItem->QueryValue( aAny, mnMemberId );
                return aAny;
            }
    }

    throw container::NoSuchElementException();
}

uno::Sequence< OUString > SAL_CALL SvxUnoNameItemTable::getElementNames(  )
{
    SolarMutexGuard aGuard;

    std::set< OUString > aNameSet;


    if (mpModelPool)
        for (const SfxPoolItem* pItem : mpModelPool->GetItemSurrogates(mnWhich))
        {
            const NameOrIndex *pNameOrIndex = static_cast<const NameOrIndex*>(pItem);

            if( !isValid( pNameOrIndex ) )
                continue;

            OUString aApiName = SvxUnogetApiNameForItem(mnWhich, pNameOrIndex->GetName());
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

    if (!mpModelPool)
        return false;

    SampleItem aSample(mnWhich, aName);
    for (const SfxPoolItem* pFindItem : mpModelPool->FindItemSurrogate(mnWhich, aSample))
        if (isValid(static_cast<const NameOrIndex*>(pFindItem)))
            return true;
    return false;
}

sal_Bool SAL_CALL SvxUnoNameItemTable::hasElements(  )
{
    SolarMutexGuard aGuard;

    if (mpModelPool)
        for (const SfxPoolItem* pItem : mpModelPool->GetItemSurrogates(mnWhich))
        {
            const NameOrIndex *pNameOrIndex = static_cast<const NameOrIndex*>(pItem);

            if( isValid( pNameOrIndex ) )
                return true;
        }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
