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

#include <sal/config.h>

#include <memory>
#include <set>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>

#include <comphelper/sequence.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <svl/lstner.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/svdmodel.hxx>
#include <svx/xdef.hxx>

#include <vector>
#include <vcl/svapp.hxx>


#include <svx/unofill.hxx>

#include <svx/unoapi.hxx>

using namespace ::com::sun::star;
using namespace ::cppu;

typedef std::vector<std::unique_ptr<SfxItemSet>> ItemPoolVector;

namespace {

class SvxUnoMarkerTable : public WeakImplHelper< container::XNameContainer, lang::XServiceInfo >,
                          public SfxListener
{
private:
    SdrModel*       mpModel;
    SfxItemPool*    mpModelPool;

    ItemPoolVector maItemSetVector;

public:
    explicit SvxUnoMarkerTable( SdrModel* pModel ) throw();
    virtual ~SvxUnoMarkerTable() throw() override;

    void dispose();

    // SfxListener
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) throw () override;

    void ImplInsertByName( const OUString& aName, const uno::Any& aElement );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const  OUString& ServiceName ) override;
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // XNameContainer
    virtual void SAL_CALL insertByName( const  OUString& aName, const  uno::Any& aElement ) override;
    virtual void SAL_CALL removeByName( const  OUString& Name ) override;

    // XNameReplace
    virtual void SAL_CALL replaceByName( const  OUString& aName, const  uno::Any& aElement ) override;

    // XNameAccess
    virtual uno::Any SAL_CALL getByName( const  OUString& aName ) override;
    virtual uno::Sequence<  OUString > SAL_CALL getElementNames(  ) override;
    virtual sal_Bool SAL_CALL hasByName( const  OUString& aName ) override;

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;
};

}

SvxUnoMarkerTable::SvxUnoMarkerTable( SdrModel* pModel ) throw()
: mpModel( pModel ),
  mpModelPool( pModel ? &pModel->GetItemPool() : nullptr )
{
    if( pModel )
        StartListening( *pModel );
}

SvxUnoMarkerTable::~SvxUnoMarkerTable() throw()
{
    if( mpModel )
        EndListening( *mpModel );
    dispose();
}

void SvxUnoMarkerTable::dispose()
{
    maItemSetVector.clear();
}

// SfxListener
void SvxUnoMarkerTable::Notify( SfxBroadcaster&, const SfxHint& rHint ) throw()
{
    if (rHint.GetId() == SfxHintId::ThisIsAnSdrHint)
    {
        const SdrHint* pSdrHint = static_cast<const SdrHint*>(&rHint);
        if( SdrHintKind::ModelCleared == pSdrHint->GetKind() )
            dispose();
    }
}

sal_Bool SAL_CALL SvxUnoMarkerTable::supportsService( const  OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

OUString SAL_CALL SvxUnoMarkerTable::getImplementationName()
{
    return "SvxUnoMarkerTable";
}

uno::Sequence< OUString > SAL_CALL SvxUnoMarkerTable::getSupportedServiceNames(  )
{
    uno::Sequence<OUString> aSNS { "com.sun.star.drawing.MarkerTable" };
    return aSNS;
}

void SvxUnoMarkerTable::ImplInsertByName( const OUString& aName, const uno::Any& aElement )
{
    maItemSetVector.push_back(
        std::make_unique<SfxItemSet>( *mpModelPool, svl::Items<XATTR_LINESTART, XATTR_LINEEND>{} ));
    auto pInSet = maItemSetVector.back().get();

    XLineEndItem aEndMarker(XATTR_LINEEND);
    aEndMarker.SetName( aName );
    aEndMarker.PutValue( aElement, 0 );

    pInSet->Put( aEndMarker );

    XLineStartItem aStartMarker(XATTR_LINESTART);
    aStartMarker.SetName( aName );
    aStartMarker.PutValue( aElement, 0 );

    pInSet->Put( aStartMarker );
}

// XNameContainer
void SAL_CALL SvxUnoMarkerTable::insertByName( const OUString& aApiName, const uno::Any& aElement )
{
    SolarMutexGuard aGuard;

    if( hasByName( aApiName ) )
        throw container::ElementExistException();

    OUString aName = SvxUnogetInternalNameForItem(XATTR_LINEEND, aApiName);

    ImplInsertByName( aName, aElement );
}

void SAL_CALL SvxUnoMarkerTable::removeByName( const OUString& aApiName )
{
    SolarMutexGuard aGuard;

    // a little quickfix for 2.0 to let applications clear api
    // created items that are not used
    if ( aApiName == "~clear~" )
    {
        dispose();
        return;
    }

    OUString aName = SvxUnogetInternalNameForItem(XATTR_LINEEND, aApiName);

    auto aIter = std::find_if(maItemSetVector.begin(), maItemSetVector.end(),
        [&aName](const std::unique_ptr<SfxItemSet>& rpItem) {
            const NameOrIndex *pItem = &(rpItem->Get( XATTR_LINEEND ) );
            return pItem->GetName() == aName;
        });
    if (aIter != maItemSetVector.end())
    {
        maItemSetVector.erase( aIter );
        return;
    }

    if( !hasByName( aName ) )
        throw container::NoSuchElementException();
}

// XNameReplace
void SAL_CALL SvxUnoMarkerTable::replaceByName( const OUString& aApiName, const uno::Any& aElement )
{
    SolarMutexGuard aGuard;

    const OUString aName = SvxUnogetInternalNameForItem(XATTR_LINEEND, aApiName);

    auto aIter = std::find_if(maItemSetVector.begin(), maItemSetVector.end(),
        [&aName](const std::unique_ptr<SfxItemSet>& rpItem) {
            const NameOrIndex *pItem = &(rpItem->Get( XATTR_LINEEND ) );
            return pItem->GetName() == aName;
        });
    if (aIter != maItemSetVector.end())
    {
        XLineEndItem aEndMarker(XATTR_LINEEND);
        aEndMarker.SetName( aName );
        if( !aEndMarker.PutValue( aElement, 0 ) )
            throw lang::IllegalArgumentException();

        (*aIter)->Put( aEndMarker );

        XLineStartItem aStartMarker(XATTR_LINESTART);
        aStartMarker.SetName( aName );
        aStartMarker.PutValue( aElement, 0 );

        (*aIter)->Put( aStartMarker );
        return;
    }

    // if it is not in our own sets, modify the pool!
    bool bFound = false;

    if (mpModelPool)
        for (const SfxPoolItem* p : mpModelPool->GetItemSurrogates(XATTR_LINESTART))
        {
            NameOrIndex *pItem = const_cast<NameOrIndex*>(static_cast<const NameOrIndex*>(p));
            if( pItem && pItem->GetName() == aName )
            {
                pItem->PutValue( aElement, 0 );
                bFound = true;
                break;
            }
        }

    if (mpModelPool)
        for (const SfxPoolItem* p : mpModelPool->GetItemSurrogates(XATTR_LINEEND))
        {
            NameOrIndex *pItem = const_cast<NameOrIndex*>(static_cast<const NameOrIndex*>(p));
            if( pItem && pItem->GetName() == aName )
            {
                pItem->PutValue( aElement, 0 );
                bFound = true;
                break;
            }
        }

    if( !bFound )
        throw container::NoSuchElementException();

    ImplInsertByName( aName, aElement );
}

static bool getByNameFromPool( const OUString& rSearchName, SfxItemPool const * pPool, sal_uInt16 nWhich, uno::Any& rAny )
{
    if (pPool)
        for (const SfxPoolItem* p : pPool->GetItemSurrogates(nWhich))
        {
            const NameOrIndex *pItem = static_cast<const NameOrIndex*>(p);

            if( pItem && pItem->GetName() == rSearchName )
            {
                pItem->QueryValue( rAny );
                return true;
            }
        }

    return false;
}

// XNameAccess
uno::Any SAL_CALL SvxUnoMarkerTable::getByName( const OUString& aApiName )
{
    SolarMutexGuard aGuard;

    OUString aName = SvxUnogetInternalNameForItem(XATTR_LINEEND, aApiName);

    uno::Any aAny;

    if (mpModelPool && !aName.isEmpty())
    {
        do
        {
            if (getByNameFromPool(aName, mpModelPool, XATTR_LINESTART, aAny))
                break;

            if (getByNameFromPool(aName, mpModelPool, XATTR_LINEEND, aAny))
                break;

            throw container::NoSuchElementException();
        }
        while(false);
    }

    return aAny;
}

static void createNamesForPool( SfxItemPool const * pPool, sal_uInt16 nWhich, std::set< OUString >& rNameSet )
{
    for (const SfxPoolItem* p : pPool->GetItemSurrogates(nWhich))
    {
        const NameOrIndex* pItem = static_cast<const NameOrIndex*>(p);

        if( pItem == nullptr || pItem->GetName().isEmpty() )
            continue;

        OUString aName = SvxUnogetApiNameForItem(XATTR_LINEEND, pItem->GetName());
        rNameSet.insert( aName );
    }
}

uno::Sequence< OUString > SAL_CALL SvxUnoMarkerTable::getElementNames()
{
    SolarMutexGuard aGuard;

    std::set< OUString > aNameSet;

    // search model pool for line starts
    createNamesForPool( mpModelPool, XATTR_LINESTART, aNameSet );

    // search model pool for line ends
    createNamesForPool( mpModelPool, XATTR_LINEEND, aNameSet );

    return comphelper::containerToSequence(aNameSet);
}

sal_Bool SAL_CALL SvxUnoMarkerTable::hasByName( const OUString& aName )
{
    SolarMutexGuard aGuard;

    if( aName.isEmpty() )
        return false;

    OUString aSearchName;

    const NameOrIndex *pItem;

    aSearchName = SvxUnogetInternalNameForItem(XATTR_LINESTART, aName);
    if (mpModelPool)
        for (const SfxPoolItem* p : mpModelPool->GetItemSurrogates(XATTR_LINESTART))
        {
            pItem = static_cast<const NameOrIndex*>(p);
            if( pItem && pItem->GetName() == aSearchName )
                return true;
        }

    aSearchName = SvxUnogetInternalNameForItem(XATTR_LINEEND, aName);
    if (mpModelPool)
        for (const SfxPoolItem* p : mpModelPool->GetItemSurrogates(XATTR_LINEEND))
        {
            pItem = static_cast<const NameOrIndex*>(p);
            if( pItem && pItem->GetName() == aSearchName )
                return true;
        }

    return false;
}

// XElementAccess
uno::Type SAL_CALL SvxUnoMarkerTable::getElementType(  )
{
    return cppu::UnoType<drawing::PointSequence>::get();
}

sal_Bool SAL_CALL SvxUnoMarkerTable::hasElements(  )
{
    SolarMutexGuard aGuard;

    const NameOrIndex *pItem;

    if (mpModelPool)
        for (const SfxPoolItem* p : mpModelPool->GetItemSurrogates(XATTR_LINESTART))
        {
            pItem = static_cast<const NameOrIndex*>(p);
            if( pItem && !pItem->GetName().isEmpty() )
                return true;
        }

    if (mpModelPool)
        for (const SfxPoolItem* p : mpModelPool->GetItemSurrogates(XATTR_LINEEND))
        {
            pItem = static_cast<const NameOrIndex*>(p);
            if( pItem && !pItem->GetName().isEmpty() )
                return true;
        }

    return false;
}

/**
 * Create a hatchtable
 */
uno::Reference< uno::XInterface > SvxUnoMarkerTable_createInstance( SdrModel* pModel )
{
    return *new SvxUnoMarkerTable(pModel);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
