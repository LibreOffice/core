/*************************************************************************
 *
 *  $RCSfile: poolcach.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:01 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <limits.h>

#pragma hdrstop

#include "itempool.hxx"
#include "itemset.hxx"
#include "poolcach.hxx"

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxItemPoolCache);


//------------------------------------------------------------------------

struct SfxItemModifyImpl
{
    const SfxSetItem  *pOrigItem;
    SfxSetItem        *pPoolItem;
};

SV_DECL_VARARR( SfxItemModifyArr_Impl, SfxItemModifyImpl, 8, 8 );
SV_IMPL_VARARR( SfxItemModifyArr_Impl, SfxItemModifyImpl);

//------------------------------------------------------------------------

SfxItemPoolCache::SfxItemPoolCache( SfxItemPool *pItemPool,
                                    const SfxPoolItem *pPutItem ):
    pPool(pItemPool),
    pCache(new SfxItemModifyArr_Impl),
    pItemToPut( &pItemPool->Put(*pPutItem) ),
    pSetToPut( 0 )
{
    DBG_CTOR(SfxItemPoolCache, 0);
    DBG_ASSERT(pItemPool, "kein Pool angegeben");
}

//------------------------------------------------------------------------

SfxItemPoolCache::SfxItemPoolCache( SfxItemPool *pItemPool,
                                    const SfxItemSet *pPutSet ):
    pPool(pItemPool),
    pCache(new SfxItemModifyArr_Impl),
    pItemToPut( 0 ),
    pSetToPut( pPutSet )
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
    pCache->Insert( aModify, pCache->Count() );

    DBG_ASSERT( !pItemToPut ||
                &pNewPoolItem->GetItemSet().Get( pItemToPut->Which() ) == pItemToPut,
                "wrong item in resulting set" );

    return *pNewPoolItem;
}



