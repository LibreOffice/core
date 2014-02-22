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

#include <cellatr.hxx>
#include <charfmt.hxx>
#include <cmdid.h>
#include <doc.hxx>
#include <editeng/colritem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/boxitem.hxx>
#include <fmtpdsc.hxx>
#include <hintids.hxx>
#include <istyleaccess.hxx>
#include <list.hxx>
#include <node.hxx>
#include <numrule.hxx>
#include <pagedesc.hxx>
#include <paratr.hxx>
#include <svl/whiter.hxx>
#include <svx/xtable.hxx>





SwAttrPool::SwAttrPool( SwDoc* pD )
    : SfxItemPool( OUString("SWG"),
                    POOLATTR_BEGIN, POOLATTR_END-1,
                    aSlotTab, aAttrTab ),
    pDoc( pD )
{
    SetVersionMap( 1, 1, 60, pVersionMap1 );
    SetVersionMap( 2, 1, 75, pVersionMap2 );
    SetVersionMap( 3, 1, 86, pVersionMap3 );
    SetVersionMap( 4, 1,121, pVersionMap4 );
    
    SetVersionMap( 5, 1,130, pVersionMap5 );
    SetVersionMap( 6, 1,136, pVersionMap6 );
    SetVersionMap( 7, 1,144, pVersionMap7 );
}

SwAttrPool::~SwAttrPool()
{
}





SwAttrSet::SwAttrSet( SwAttrPool& rPool, sal_uInt16 nWh1, sal_uInt16 nWh2 )
    : SfxItemSet( rPool, nWh1, nWh2 ), pOldSet( 0 ), pNewSet( 0 )
{
}

SwAttrSet::SwAttrSet( SwAttrPool& rPool, const sal_uInt16* nWhichPairTable )
    : SfxItemSet( rPool, nWhichPairTable ), pOldSet( 0 ), pNewSet( 0 )
{
}

SwAttrSet::SwAttrSet( const SwAttrSet& rSet )
    : SfxItemSet( rSet ), pOldSet( 0 ), pNewSet( 0 )
{
}

SfxItemSet* SwAttrSet::Clone( sal_Bool bItems, SfxItemPool *pToPool ) const
{
    if ( pToPool && pToPool != GetPool() )
    {
        SwAttrPool* pAttrPool = dynamic_cast< SwAttrPool* >(pToPool);
        SfxItemSet* pTmpSet = 0;
        if ( !pAttrPool )
            pTmpSet = SfxItemSet::Clone( bItems, pToPool );
        else
        {
            pTmpSet = new SwAttrSet( *pAttrPool, GetRanges() );
            if ( bItems )
            {
                SfxWhichIter aIter(*pTmpSet);
                sal_uInt16 nWhich = aIter.FirstWhich();
                while ( nWhich )
                {
                    const SfxPoolItem* pItem;
                    if ( SFX_ITEM_SET == GetItemState( nWhich, false, &pItem ) )
                        pTmpSet->Put( *pItem, pItem->Which() );
                    nWhich = aIter.NextWhich();
                }
            }
        }
        return pTmpSet;
    }
    else
        return bItems
                ? new SwAttrSet( *this )
                : new SwAttrSet( *GetPool(), GetRanges() );
}

bool SwAttrSet::Put_BC( const SfxPoolItem& rAttr,
                       SwAttrSet* pOld, SwAttrSet* pNew )
{
    pNewSet = pNew;
    pOldSet = pOld;
    bool nRet = 0 != SfxItemSet::Put( rAttr );
    pOldSet = pNewSet = 0;
    return nRet;
}


bool SwAttrSet::Put_BC( const SfxItemSet& rSet,
                       SwAttrSet* pOld, SwAttrSet* pNew )
{
    pNewSet = pNew;
    pOldSet = pOld;
    bool nRet = SfxItemSet::Put( rSet );
    pOldSet = pNewSet = 0;
    return nRet;
}

sal_uInt16 SwAttrSet::ClearItem_BC( sal_uInt16 nWhich,
                                    SwAttrSet* pOld, SwAttrSet* pNew )
{
    pNewSet = pNew;
    pOldSet = pOld;
    sal_uInt16 nRet = SfxItemSet::ClearItem( nWhich );
    pOldSet = pNewSet = 0;
    return nRet;
}

sal_uInt16 SwAttrSet::ClearItem_BC( sal_uInt16 nWhich1, sal_uInt16 nWhich2,
                                    SwAttrSet* pOld, SwAttrSet* pNew )
{
    OSL_ENSURE( nWhich1 <= nWhich2, "no valid range" );
    pNewSet = pNew;
    pOldSet = pOld;
    sal_uInt16 nRet = 0;
    for( ; nWhich1 <= nWhich2; ++nWhich1 )
        nRet = nRet + SfxItemSet::ClearItem( nWhich1 );
    pOldSet = pNewSet = 0;
    return nRet;
}

int SwAttrSet::Intersect_BC( const SfxItemSet& rSet,
                             SwAttrSet* pOld, SwAttrSet* pNew )
{
    pNewSet = pNew;
    pOldSet = pOld;
    SfxItemSet::Intersect( rSet );
    pOldSet = pNewSet = 0;
    return pNew ? pNew->Count() : ( pOld ? pOld->Count() : 0 );
}


void  SwAttrSet::Changed( const SfxPoolItem& rOld, const SfxPoolItem& rNew )
{
    if( pOldSet )
        pOldSet->PutChgd( rOld );
    if( pNewSet )
        pNewSet->PutChgd( rNew );
}

/** special treatment for some attributes

    Set the Modify pointer (old pDefinedIn) for the following attributes:
     - SwFmtDropCaps
     - SwFmtPageDesc

    (Is called at inserts into formats/nodes)
*/
bool SwAttrSet::SetModifyAtAttr( const SwModify* pModify )
{
    bool bSet = false;

    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == GetItemState( RES_PAGEDESC, false, &pItem ) &&
        ((SwFmtPageDesc*)pItem)->GetDefinedIn() != pModify  )
    {
        ((SwFmtPageDesc*)pItem)->ChgDefinedIn( pModify );
        bSet = true;
    }

    if( SFX_ITEM_SET == GetItemState( RES_PARATR_DROP, false, &pItem ) &&
        ((SwFmtDrop*)pItem)->GetDefinedIn() != pModify )
    {
        
        
        SwCharFmt* pCharFmt;
        if( 0 != ( pCharFmt = ((SwFmtDrop*)pItem)->GetCharFmt() )
            && GetPool() != pCharFmt->GetAttrSet().GetPool() )
        {
           pCharFmt = GetDoc()->CopyCharFmt( *pCharFmt );
           ((SwFmtDrop*)pItem)->SetCharFmt( pCharFmt );
        }
        ((SwFmtDrop*)pItem)->ChgDefinedIn( pModify );
        bSet = true;
    }

    if( SFX_ITEM_SET == GetItemState( RES_BOXATR_FORMULA, false, &pItem ) &&
        ((SwTblBoxFormula*)pItem)->GetDefinedIn() != pModify )
    {
        ((SwTblBoxFormula*)pItem)->ChgDefinedIn( pModify );
        bSet = true;
    }

    return bSet;
}

void SwAttrSet::CopyToModify( SwModify& rMod ) const
{
    
    SwCntntNode* pCNd = PTR_CAST( SwCntntNode, &rMod );
    SwFmt* pFmt = PTR_CAST( SwFmt, &rMod );

    if( pCNd || pFmt )
    {
        if( Count() )
        {
            
            SfxStringItem* pNewListIdItem( 0 );

            const SfxPoolItem* pItem;
            const SwDoc *pSrcDoc = GetDoc();
            SwDoc *pDstDoc = pCNd ? pCNd->GetDoc() : pFmt->GetDoc();

            
            if( pSrcDoc != pDstDoc &&
                SFX_ITEM_SET == GetItemState( RES_PARATR_NUMRULE, false, &pItem ) )
            {
                const OUString& rNm = ((SwNumRuleItem*)pItem)->GetValue();
                if( !rNm.isEmpty() )
                {
                    SwNumRule* pDestRule = pDstDoc->FindNumRulePtr( rNm );
                    if( pDestRule )
                        pDestRule->SetInvalidRule( sal_True );
                    else
                        pDstDoc->MakeNumRule( rNm, pSrcDoc->FindNumRulePtr( rNm ) );
                }
            }

            
            
            if ( pSrcDoc != pDstDoc &&
                 pCNd && pCNd->IsTxtNode() &&
                 GetItemState( RES_PARATR_LIST_ID, false, &pItem ) == SFX_ITEM_SET )
            {
                const OUString& sListId =
                        dynamic_cast<const SfxStringItem*>(pItem)->GetValue();
                if ( !sListId.isEmpty() &&
                     !pDstDoc->getListByName( sListId ) )
                {
                    const SwList* pList = pSrcDoc->getListByName( sListId );
                    
                    const OUString sDefaultListStyleName =
                                            pList->GetDefaultListStyleName();
                    
                    const SwNumRule* pDstDocNumRule =
                                pDstDoc->FindNumRulePtr( sDefaultListStyleName );
                    if ( !pDstDocNumRule )
                    {
                        pDstDoc->MakeNumRule( sDefaultListStyleName,
                                              pSrcDoc->FindNumRulePtr( sDefaultListStyleName ) );
                    }
                    else
                    {
                        const SwNumRule* pSrcDocNumRule =
                                pSrcDoc->FindNumRulePtr( sDefaultListStyleName );
                        
                        
                        
                        
                        if ( sListId == pSrcDocNumRule->GetDefaultListId() )
                        {
                            pNewListIdItem = new SfxStringItem (
                                            RES_PARATR_LIST_ID,
                                            pDstDocNumRule->GetDefaultListId() );
                        }
                    }
                    
                    
                    if ( pNewListIdItem == 0 &&
                         !pDstDoc->getListByName( sListId ) )
                    {
                        
                        pDstDoc->createList( sListId, sDefaultListStyleName );
                    }
                }
            }

            const SwPageDesc* pPgDesc;
            if( pSrcDoc != pDstDoc && SFX_ITEM_SET == GetItemState(
                                            RES_PAGEDESC, false, &pItem ) &&
                0 != ( pPgDesc = ((SwFmtPageDesc*)pItem)->GetPageDesc()) )
            {
                SfxItemSet aTmpSet( *this );

                SwPageDesc* pDstPgDesc = pDstDoc->FindPageDescByName(
                                                    pPgDesc->GetName() );
                if( !pDstPgDesc )
                {
                    pDstPgDesc = &pDstDoc->GetPageDesc(
                                   pDstDoc->MakePageDesc( pPgDesc->GetName() ));
                    pDstDoc->CopyPageDesc( *pPgDesc, *pDstPgDesc );
                }
                SwFmtPageDesc aDesc( pDstPgDesc );
                aDesc.SetNumOffset( ((SwFmtPageDesc*)pItem)->GetNumOffset() );
                aTmpSet.Put( aDesc );

                if( pCNd )
                {
                    
                    if ( pNewListIdItem != 0 )
                    {
                        aTmpSet.Put( *pNewListIdItem );
                    }
                    pCNd->SetAttr( aTmpSet );
                }
                else
                {
                    pFmt->SetFmtAttr( aTmpSet );
                }
            }
            else if( pCNd )
            {
                
                if ( pNewListIdItem != 0 )
                {
                    SfxItemSet aTmpSet( *this );
                    aTmpSet.Put( *pNewListIdItem );
                    pCNd->SetAttr( aTmpSet );
                }
                else
                {
                    pCNd->SetAttr( *this );
                }
            }
            else
            {
                pFmt->SetFmtAttr( *this );
            }

            
            delete pNewListIdItem;
            pNewListIdItem = 0;
        }
    }
#if OSL_DEBUG_LEVEL > 0
    else
        OSL_FAIL("neither Format nor ContentNode - no Attributes copied");
#endif
}


bool IsInRange( const sal_uInt16* pRange, const sal_uInt16 nId )
{
    while( *pRange )
    {
        if( *pRange <= nId && nId <= *(pRange+1) )
            return true;
        pRange += 2;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
