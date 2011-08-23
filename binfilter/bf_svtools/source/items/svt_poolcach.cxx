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

// MARKER(update_precomp.py): autogen include statement, do not remove


#include <limits.h>

#ifndef GCC
#endif

#include <bf_svtools/itempool.hxx>
#include <bf_svtools/itemset.hxx>
#include "poolcach.hxx"

namespace binfilter
{

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxItemPoolCache)


//------------------------------------------------------------------------

struct SfxItemModifyImpl
{
    const SfxSetItem  *pOrigItem;
    SfxSetItem		  *pPoolItem;
};

SV_DECL_VARARR( SfxItemModifyArr_Impl, SfxItemModifyImpl, 8, 8 )
SV_IMPL_VARARR( SfxItemModifyArr_Impl, SfxItemModifyImpl);

//------------------------------------------------------------------------

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

//------------------------------------------------------------------------

SfxItemPoolCache::~SfxItemPoolCache()
{
    DBG_DTOR(SfxItemPoolCache, 0);
    for ( USHORT nPos = 0; nPos < pCache->Count(); ++nPos ) {
        pPool->Remove( *(*pCache)[nPos].pPoolItem );
        pPool->Remove( *(*pCache)[nPos].pOrigItem );
    }
    delete pCache; pCache = 0;

    if ( pItemToPut )
        pPool->Remove( *pItemToPut );
}

//------------------------------------------------------------------------

const SfxSetItem& SfxItemPoolCache::ApplyTo( const SfxSetItem &rOrigItem, BOOL bNew )
{
    DBG_CHKTHIS(SfxItemPoolCache, 0);
    DBG_ASSERT( pPool == rOrigItem.GetItemSet().GetPool(), "invalid Pool" );
    DBG_ASSERT( IsDefaultItem( &rOrigItem ) || IsPooledItem( &rOrigItem ),
                "original not in pool" );

    // Suchen, ob diese Transformations schon einmal vorkam
    for ( USHORT nPos = 0; nPos < pCache->Count(); ++nPos )
    {
        SfxItemModifyImpl &rMapEntry = (*pCache)[nPos];
        if ( rMapEntry.pOrigItem == &rOrigItem )
        {
            // aendert sich ueberhaupt etwas?
            if ( rMapEntry.pPoolItem != &rOrigItem )
            {
                rMapEntry.pPoolItem->AddRef(2); // einen davon fuer den Cache
                if ( bNew )
                    pPool->Put( rOrigItem );	//! AddRef??
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
        pPool->Put( rOrigItem );	//! AddRef??

    // die Transformation im Cache eintragen
    SfxItemModifyImpl aModify;
    aModify.pOrigItem = &rOrigItem;
    aModify.pPoolItem = (SfxSetItem*) pNewPoolItem;
    pCache->Insert( aModify, pCache->Count() );

    DBG_ASSERT( !pItemToPut ||
                &pNewPoolItem->GetItemSet().Get( pItemToPut->Which() ) == pItemToPut,
                "wrong item in resulting set" );

    return *pNewPoolItem;
}

}
