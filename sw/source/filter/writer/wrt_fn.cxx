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

#include <svl/itemiter.hxx>
#include <svl/whiter.hxx>

// tdf#94088 SdrAllFillAttributesHelper needed
#include <svx/unobrushitemhelper.hxx>

#include "shellio.hxx"
#include "wrt_fn.hxx"
#include "node.hxx"
#include "format.hxx"

Writer& Out( const SwAttrFnTab pTab, const SfxPoolItem& rHt, Writer & rWrt )
{
    sal_uInt16 nId = rHt.Which();
    OSL_ENSURE(  nId < POOLATTR_END && nId >= POOLATTR_BEGIN, "SwAttrFnTab::Out()" );
    FnAttrOut pOut;
    if( nullptr != ( pOut = pTab[ nId - RES_CHRATR_BEGIN] ))
        (*pOut)( rWrt, rHt );
    return rWrt;

}

Writer& Out_SfxItemSet( const SwAttrFnTab pTab, Writer& rWrt,
                        const SfxItemSet& rSet, bool bDeep,
                        bool bTstForDefault )
{
    // at first give the own attributes out
    const SfxItemPool& rPool = *rSet.GetPool();
    const SfxItemSet* pSet = &rSet;
    if( !pSet->Count() )        // Optimizing - empty Sets
    {
        if( !bDeep )
            return rWrt;
        while( nullptr != ( pSet = pSet->GetParent() ) && !pSet->Count() )
            ;
        if( !pSet )
            return rWrt;
    }
    const SfxPoolItem* pItem(nullptr);
    FnAttrOut pOut;

    // tdf#94088 check if any FillAttribute is used [XATTR_FILL_FIRST .. XATTR_FILL_LAST]
    // while processing the items
    bool bFillItemUsed = false;

    if( !bDeep || !pSet->GetParent() )
    {
        OSL_ENSURE( rSet.Count(), "It has been handled already, right?" );
        SfxItemIter aIter( *pSet );
        pItem = aIter.GetCurItem();
        do {
            // pTab only covers POOLATTR_BEGIN..POOLATTR_END.
            if( pItem->Which() <= POOLATTR_END )
            {
                if( nullptr != ( pOut = pTab[ pItem->Which() - RES_CHRATR_BEGIN]) )
                {
                    (*pOut)( rWrt, *pItem );
                }
            }
            else if(XATTR_FILLSTYLE == pItem->Which())
            {
                bFillItemUsed = true;
            }
        } while( !aIter.IsAtEnd() && nullptr != ( pItem = aIter.NextItem() ) );
    }
    else
    {
        SfxWhichIter aIter( *pSet );
        sal_uInt16 nWhich = aIter.FirstWhich();
        while( nWhich )
        {
            if( SfxItemState::SET == pSet->GetItemState( nWhich, bDeep, &pItem ) &&
                ( !bTstForDefault || (
                    *pItem != rPool.GetDefaultItem( nWhich )
                    || ( pSet->GetParent() &&
                        *pItem != pSet->GetParent()->Get( nWhich ))
                )))
            {
                if( nullptr != ( pOut = pTab[ nWhich - RES_CHRATR_BEGIN] ))
                {
                    (*pOut)( rWrt, *pItem );
                }
                else if(XATTR_FILLSTYLE == pItem->Which())
                {
                    bFillItemUsed = true;
                }
            }
            nWhich = aIter.NextWhich();
        }
    }

    if(bFillItemUsed)
    {
        // tdf#94088 if used, construct a SvxBrushItem and export it using the
        // existing mechanisms.
        // This is the right place in the future if the adapted fill attributes
        // may be handled more directly in HTML export to handle them.
        const SvxBrushItem aSvxBrushItem = getSvxBrushItemFromSourceSet(*pSet, RES_BACKGROUND, bDeep);

        if( nullptr != ( pOut = pTab[RES_BACKGROUND - RES_CHRATR_BEGIN] ))
        {
            (*pOut)( rWrt, aSvxBrushItem );
        }
    }

    return rWrt;
}

Writer& Out( const SwNodeFnTab pTab, SwNode& rNode, Writer & rWrt )
{
    // It must be a ContentNode!
    SwContentNode * pCNd = rNode.GetContentNode();
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
            OSL_FAIL("What kind of node is it now?");
            break;
    }
    FnNodeOut pOut;
    if( nullptr != ( pOut = pTab[ nId - RES_NODE_BEGIN ] ))
        (*pOut)( rWrt, *pCNd );
    return rWrt;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
