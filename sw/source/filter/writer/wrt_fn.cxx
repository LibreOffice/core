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
#include "precompiled_sw.hxx"
#include <svl/itemiter.hxx>
#include <svl/whiter.hxx>


#include "shellio.hxx"
#include "wrt_fn.hxx"
#include "pam.hxx"
#include "node.hxx"
#include "format.hxx"



Writer& Out( const SwAttrFnTab pTab, const SfxPoolItem& rHt, Writer & rWrt )
{
    sal_uInt16 nId = rHt.Which();
    ASSERT(  nId < POOLATTR_END && nId >= POOLATTR_BEGIN, "SwAttrFnTab::Out()" );
    FnAttrOut pOut;
    if( 0 != ( pOut = pTab[ nId - RES_CHRATR_BEGIN] ))
        (*pOut)( rWrt, rHt );
    return rWrt;

}

Writer& Out_SfxItemSet( const SwAttrFnTab pTab, Writer& rWrt,
                        const SfxItemSet& rSet, sal_Bool bDeep,
                        sal_Bool bTstForDefault )
{
    // erst die eigenen Attribute ausgeben
    const SfxItemPool& rPool = *rSet.GetPool();
    const SfxItemSet* pSet = &rSet;
    if( !pSet->Count() )        // Optimierung - leere Sets
    {
        if( !bDeep )
            return rWrt;
        while( 0 != ( pSet = pSet->GetParent() ) && !pSet->Count() )
            ;
        if( !pSet )
            return rWrt;
    }
    const SfxPoolItem* pItem;
    FnAttrOut pOut;
    if( !bDeep || !pSet->GetParent() )
    {
        ASSERT( rSet.Count(), "Wurde doch schon behandelt oder?" );
        SfxItemIter aIter( *pSet );
        pItem = aIter.GetCurItem();
        do {
            if( 0 != ( pOut = pTab[ pItem->Which() - RES_CHRATR_BEGIN] ))
                    (*pOut)( rWrt, *pItem );
        } while( !aIter.IsAtEnd() && 0 != ( pItem = aIter.NextItem() ) );
    }
    else
    {
        SfxWhichIter aIter( *pSet );
        sal_uInt16 nWhich = aIter.FirstWhich();
        while( nWhich )
        {
            if( SFX_ITEM_SET == pSet->GetItemState( nWhich, bDeep, &pItem ) &&
                ( !bTstForDefault || (
                    *pItem != rPool.GetDefaultItem( nWhich )
                    || ( pSet->GetParent() &&
                        *pItem != pSet->GetParent()->Get( nWhich ))
                )) && 0 != ( pOut = pTab[ nWhich - RES_CHRATR_BEGIN] ))
                    (*pOut)( rWrt, *pItem );
            nWhich = aIter.NextWhich();
        }
    }
    return rWrt;
}



Writer& Out( const SwNodeFnTab pTab, SwNode& rNode, Writer & rWrt )
{
    // es muss ein CntntNode sein !!
    SwCntntNode * pCNd = rNode.GetCntntNode();
    if( !pCNd )
        return rWrt;

    sal_uInt16 nId = RES_TXTNODE;
    switch (pCNd->GetNodeType())
    {
        case ND_TEXTNODE:
            nId = RES_TXTNODE;
             break;
        case ND_GRFNODE:
            nId = RES_GRFNODE;
            break;
        case ND_OLENODE:
            nId = RES_OLENODE;
            break;
        default:
            ASSERT(false, "was fuer ein Node ist es denn nun?");
            break;
    }
    FnNodeOut pOut;
    if( 0 != ( pOut = pTab[ nId - RES_NODE_BEGIN ] ))
        (*pOut)( rWrt, *pCNd );
    return rWrt;
}


