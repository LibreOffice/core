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


#include <limits.h>

#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <svl/poolcach.hxx>

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxItemPoolCache)


SfxItemPoolCache::SfxItemPoolCache( SfxItemPool *pItemPool,
                                    const SfxPoolItem *pPutItem ):
    pPool(pItemPool),
    pCache(new SfxItemModifyArr_Impl),
    pSetToPut( 0 ),
    pItemToPut( &pItemPool->Put(*pPutItem) )
{
    DBG_CTOR(SfxItemPoolCache, 0);
    DBG_ASSERT(pItemPool, "kein Pool angegeben");
}


SfxItemPoolCache::SfxItemPoolCache( SfxItemPool *pItemPool,
                                    const SfxItemSet *pPutSet ):
    pPool(pItemPool),
    pCache(new SfxItemModifyArr_Impl),
    pSetToPut( pPutSet ),
    pItemToPut( 0 )
{
    DBG_CTOR(SfxItemPoolCache, 0);
    DBG_ASSERT(pItemPool, "kein Pool angegeben");
}


SfxItemPoolCache::~SfxItemPoolCache()
{
    DBG_DTOR(SfxItemPoolCache, 0);
    for ( size_t nPos = 0; nPos < pCache->size(); ++nPos ) {
        pPool->Remove( *(*pCache)[nPos].pPoolItem );
        pPool->Remove( *(*pCache)[nPos].pOrigItem );
    }
    delete pCache; pCache = 0;

    if ( pItemToPut )
        pPool->Remove( *pItemToPut );
}


const SfxSetItem& SfxItemPoolCache::ApplyTo( const SfxSetItem &rOrigItem, sal_Bool bNew )
{
    DBG_CHKTHIS(SfxItemPoolCache, 0);
    DBG_ASSERT( pPool == rOrigItem.GetItemSet().GetPool(), "invalid Pool" );
    DBG_ASSERT( IsDefaultItem( &rOrigItem ) || IsPooledItem( &rOrigItem ),
                "original not in pool" );

    // Find whether this Transformations ever occurred
    for ( size_t nPos = 0; nPos < pCache->size(); ++nPos )
    {
        SfxItemModifyImpl &rMapEntry = (*pCache)[nPos];
        if ( rMapEntry.pOrigItem == &rOrigItem )
        {
            // aendert sich ueberhaupt etwas?
            if ( rMapEntry.pPoolItem != &rOrigItem )
            {
                rMapEntry.pPoolItem->AddRef(2); // einen davon fuer den Cache
                if ( bNew )
                    pPool->Put( rOrigItem );    //! AddRef??
            }
            return *rMapEntry.pPoolItem;
        }
    }

    // die neue Attributierung in einem neuen Set eintragen
    SfxSetItem *pNewItem = (SfxSetItem *)rOrigItem.Clone();
    if ( pItemToPut )
    {
        pNewItem->GetItemSet().PutDirect( *pItemToPut );
        DBG_ASSERT( &pNewItem->GetItemSet().Get( pItemToPut->Which() ) == pItemToPut,
                    "wrong item in temporary set" );
    }
    else
        pNewItem->GetItemSet().Put( *pSetToPut );
    const SfxSetItem* pNewPoolItem = (const SfxSetItem*) &pPool->Put( *pNewItem );
    DBG_ASSERT( pNewPoolItem != pNewItem, "Pool: rein == raus?" );
    delete pNewItem;

    // Refernzzaehler anpassen, je einen davon fuer den Cache
    pNewPoolItem->AddRef( pNewPoolItem != &rOrigItem ? 2 : 1 );
    if ( bNew )
        pPool->Put( rOrigItem );    //! AddRef??

    // die Transformation im Cache eintragen
    SfxItemModifyImpl aModify;
    aModify.pOrigItem = &rOrigItem;
    aModify.pPoolItem = (SfxSetItem*) pNewPoolItem;
    pCache->push_back( aModify );

    DBG_ASSERT( !pItemToPut ||
                &pNewPoolItem->GetItemSet().Get( pItemToPut->Which() ) == pItemToPut,
                "wrong item in resulting set" );

    return *pNewPoolItem;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
