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

#include <memory>

#include <cellatr.hxx>
#include <charfmt.hxx>
#include <fchrfmt.hxx>
#include <doc.hxx>
#include <IDocumentListsAccess.hxx>
#include <editeng/editeng.hxx>
#include <fmtanchr.hxx>
#include <fmtLayoutInCell.hxx>
#include <fmtpdsc.hxx>
#include <fmtautofmt.hxx>
#include <hintids.hxx>
#include <list.hxx>
#include <node.hxx>
#include <numrule.hxx>
#include <pagedesc.hxx>
#include <paratr.hxx>
#include <o3tl/unit_conversion.hxx>
#include <osl/diagnose.h>
#include <svl/whiter.hxx>

#include <svx/svdpool.hxx>
#include <svx/sxenditm.hxx>
#include <svx/sdsxyitm.hxx>

SwAttrPool::SwAttrPool( SwDoc* pD )
    : SfxItemPool( "SWG",
                    POOLATTR_BEGIN, POOLATTR_END-1,
                    aSlotTab, &aAttrTab ),
    m_pDoc( pD )
{
    // create SfxItemPool and EditEngine pool and add these in a chain. These
    // belong us and will be removed/destroyed in removeAndDeleteSecondaryPools() used from
    // the destructor
    rtl::Reference<SfxItemPool> pSdrPool = new SdrItemPool(this);

    // #75371# change DefaultItems for the SdrEdgeObj distance items
    // to TWIPS.
    constexpr tools::Long nDefEdgeDist
        = o3tl::convert(500, o3tl::Length::mm100, o3tl::Length::twip);

    pSdrPool->SetPoolDefaultItem(SdrEdgeNode1HorzDistItem(nDefEdgeDist));
    pSdrPool->SetPoolDefaultItem(SdrEdgeNode1VertDistItem(nDefEdgeDist));
    pSdrPool->SetPoolDefaultItem(SdrEdgeNode2HorzDistItem(nDefEdgeDist));
    pSdrPool->SetPoolDefaultItem(SdrEdgeNode2VertDistItem(nDefEdgeDist));

    // #i33700# // Set shadow distance defaults as PoolDefaultItems
    constexpr tools::Long nDefShadowDist
        = o3tl::convert(300, o3tl::Length::mm100, o3tl::Length::twip);
    pSdrPool->SetPoolDefaultItem(makeSdrShadowXDistItem(nDefShadowDist));
    pSdrPool->SetPoolDefaultItem(makeSdrShadowYDistItem(nDefShadowDist));

    rtl::Reference<SfxItemPool> pEEgPool = EditEngine::CreatePool();

    pSdrPool->SetSecondaryPool(pEEgPool.get());

    if(GetFrozenIdRanges().empty())
    {
        FreezeIdRanges();
    }
    else
    {
        pSdrPool->FreezeIdRanges();
    }
}

SwAttrPool::~SwAttrPool()
{
    // cleanup secondary pools
    SfxItemPool *pSdrPool = GetSecondaryPool();
    // first delete the items, then break the linking
    pSdrPool->Delete();
    SetSecondaryPool(nullptr);
}

SwAttrSet::SwAttrSet( SwAttrPool& rPool, sal_uInt16 nWh1, sal_uInt16 nWh2 )
    : SfxItemSet( rPool, nWh1, nWh2 ), m_pOldSet( nullptr ), m_pNewSet( nullptr )
{
}

SwAttrSet::SwAttrSet( SwAttrPool& rPool, const WhichRangesContainer& nWhichPairTable )
    : SfxItemSet( rPool, nWhichPairTable )
    , m_pOldSet( nullptr ), m_pNewSet( nullptr )
{
}

SwAttrSet::SwAttrSet( const SwAttrSet& rSet )
    : SfxItemSet( rSet ), m_pOldSet( nullptr ), m_pNewSet( nullptr )
{
}

std::unique_ptr<SfxItemSet> SwAttrSet::Clone( bool bItems, SfxItemPool *pToPool ) const
{
    if ( pToPool && pToPool != GetPool() )
    {
        SwAttrPool* pAttrPool = dynamic_cast< SwAttrPool* >(pToPool);
        std::unique_ptr<SfxItemSet> pTmpSet;
        if ( !pAttrPool )
            pTmpSet = SfxItemSet::Clone( bItems, pToPool );
        else
        {
            pTmpSet.reset(new SwAttrSet( *pAttrPool, GetRanges() ));
            if ( bItems )
            {
                SfxWhichIter aIter(*pTmpSet);
                sal_uInt16 nWhich = aIter.FirstWhich();
                while ( nWhich )
                {
                    const SfxPoolItem* pItem;
                    if ( SfxItemState::SET == GetItemState( nWhich, false, &pItem ) )
                        pTmpSet->Put( *pItem );
                    nWhich = aIter.NextWhich();
                }
            }
        }
        return pTmpSet;
    }
    else
        return std::unique_ptr<SfxItemSet>(
                bItems
                ? new SwAttrSet( *this )
                : new SwAttrSet( *GetPool(), GetRanges() ));
}

bool SwAttrSet::Put_BC( const SfxPoolItem& rAttr,
                       SwAttrSet* pOld, SwAttrSet* pNew )
{
    m_pNewSet = pNew;
    m_pOldSet = pOld;
    bool bRet = nullptr != SfxItemSet::Put( rAttr );
    m_pOldSet = m_pNewSet = nullptr;
    return bRet;
}

bool SwAttrSet::Put_BC( const SfxItemSet& rSet,
                       SwAttrSet* pOld, SwAttrSet* pNew )
{
    m_pNewSet = pNew;
    m_pOldSet = pOld;
    bool bRet = SfxItemSet::Put( rSet );
    m_pOldSet = m_pNewSet = nullptr;
    return bRet;
}

sal_uInt16 SwAttrSet::ClearItem_BC( sal_uInt16 nWhich,
                                    SwAttrSet* pOld, SwAttrSet* pNew )
{
    m_pNewSet = pNew;
    m_pOldSet = pOld;
    sal_uInt16 nRet = SfxItemSet::ClearItem( nWhich );
    m_pOldSet = m_pNewSet = nullptr;
    return nRet;
}

sal_uInt16 SwAttrSet::ClearItem_BC( sal_uInt16 nWhich1, sal_uInt16 nWhich2,
                                    SwAttrSet* pOld, SwAttrSet* pNew )
{
    OSL_ENSURE( nWhich1 <= nWhich2, "no valid range" );
    m_pNewSet = pNew;
    m_pOldSet = pOld;
    sal_uInt16 nRet = 0;
    for( ; nWhich1 <= nWhich2; ++nWhich1 )
        nRet = nRet + SfxItemSet::ClearItem( nWhich1 );
    m_pOldSet = m_pNewSet = nullptr;
    return nRet;
}

int SwAttrSet::Intersect_BC( const SfxItemSet& rSet,
                             SwAttrSet* pOld, SwAttrSet* pNew )
{
    m_pNewSet = pNew;
    m_pOldSet = pOld;
    SfxItemSet::Intersect( rSet );
    m_pOldSet = m_pNewSet = nullptr;
    return pNew ? pNew->Count() : ( pOld ? pOld->Count() : 0 );
}

/// Notification callback
void  SwAttrSet::Changed( const SfxPoolItem& rOld, const SfxPoolItem& rNew )
{
    if( m_pOldSet )
        m_pOldSet->PutChgd( rOld );
    if( m_pNewSet )
        m_pNewSet->PutChgd( rNew );
}

/** special treatment for some attributes

    Set the Modify pointer (old pDefinedIn) for the following attributes:
     - SwFormatDropCaps
     - SwFormatPageDesc

    (Is called at inserts into formats/nodes)
*/
bool SwAttrSet::SetModifyAtAttr( const sw::BroadcastingModify* pModify )
{
    bool bSet = false;

    const SfxPoolItem* pItem;
    if( SfxItemState::SET == GetItemState( RES_PAGEDESC, false, &pItem ) &&
        pItem->StaticWhichCast(RES_PAGEDESC).GetDefinedIn() != pModify  )
    {
        const_cast<SwFormatPageDesc&>(pItem->StaticWhichCast(RES_PAGEDESC)).ChgDefinedIn( pModify );
        bSet = true;
    }

    if(SfxItemState::SET == GetItemState( RES_PARATR_DROP, false, &pItem ))
    {
        auto pDropDefiner = dynamic_cast<const sw::FormatDropDefiner*>(pModify);
        SwFormatDrop& rFormatDrop = const_cast<SwFormatDrop&>(pItem->StaticWhichCast(RES_PARATR_DROP));
        // If CharFormat is set and it is set in different attribute pools then
        // the CharFormat has to be copied.
        SwCharFormat* pCharFormat = rFormatDrop.GetCharFormat();
        if(pCharFormat && GetPool() != pCharFormat->GetAttrSet().GetPool())
        {
           pCharFormat = GetDoc()->CopyCharFormat(*pCharFormat);
           rFormatDrop.SetCharFormat(pCharFormat);
        }
        rFormatDrop.ChgDefinedIn(pDropDefiner);
        bSet = true;
    }

    if( SfxItemState::SET == GetItemState( RES_BOXATR_FORMULA, false, &pItem ) &&
        pItem->StaticWhichCast(RES_BOXATR_FORMULA).GetDefinedIn() != pModify )
    {
        const_cast<SwTableBoxFormula&>(pItem->StaticWhichCast(RES_BOXATR_FORMULA)).ChgDefinedIn( pModify );
        bSet = true;
    }

    return bSet;
}

void SwAttrSet::CopyToModify( sw::BroadcastingModify& rMod ) const
{
    // copy attributes across multiple documents if needed
    SwContentNode* pCNd = dynamic_cast<SwContentNode*>( &rMod  );
    SwFormat* pFormat = dynamic_cast<SwFormat*>( &rMod  );

    if( pCNd || pFormat )
    {
        if( Count() )
        {
            // #i92811#
            std::unique_ptr<SfxStringItem> pNewListIdItem;

            const SfxPoolItem* pItem;
            const SwDoc *pSrcDoc = GetDoc();
            SwDoc *pDstDoc = pCNd ? &pCNd->GetDoc() : pFormat->GetDoc();

            // Does the NumRule has to be copied?
            if( pSrcDoc != pDstDoc &&
                SfxItemState::SET == GetItemState( RES_PARATR_NUMRULE, false, &pItem ) )
            {
                const OUString& rNm = pItem->StaticWhichCast(RES_PARATR_NUMRULE).GetValue();
                if( !rNm.isEmpty() )
                {
                    SwNumRule* pDestRule = pDstDoc->FindNumRulePtr( rNm );
                    if( pDestRule )
                        pDestRule->SetInvalidRule( true );
                    else
                        pDstDoc->MakeNumRule( rNm, pSrcDoc->FindNumRulePtr( rNm ) );
                }
            }

            // copy list and if needed also the corresponding list style
            // for text nodes
            if ( pSrcDoc != pDstDoc &&
                 pCNd && pCNd->IsTextNode() &&
                 GetItemState( RES_PARATR_LIST_ID, false, &pItem ) == SfxItemState::SET )
            {
                auto pStrItem = dynamic_cast<const SfxStringItem*>(pItem);
                assert(pStrItem);
                const OUString& sListId = pStrItem->GetValue();
                if ( !sListId.isEmpty() &&
                     !pDstDoc->getIDocumentListsAccess().getListByName( sListId ) )
                {
                    const SwList* pList = pSrcDoc->getIDocumentListsAccess().getListByName( sListId );
                    // copy list style, if needed
                    const OUString& sDefaultListStyleName =
                                            pList->GetDefaultListStyleName();
                    // #i92811#
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
                        // If list id of text node equals the list style's
                        // default list id in the source document, the same
                        // should be hold in the destination document.
                        // Thus, create new list id item.
                        if (pSrcDocNumRule && sListId == pSrcDocNumRule->GetDefaultListId())
                        {
                            pNewListIdItem.reset(new SfxStringItem (
                                            RES_PARATR_LIST_ID,
                                            pDstDocNumRule->GetDefaultListId() ));
                        }
                    }
                    // check again, if list exist, because <SwDoc::MakeNumRule(..)>
                    // could have also created it.
                    if ( pNewListIdItem == nullptr &&
                         !pDstDoc->getIDocumentListsAccess().getListByName( sListId ) )
                    {
                        // copy list
                        pDstDoc->getIDocumentListsAccess().createList( sListId, sDefaultListStyleName );
                    }
                }
            }

            std::optional< SfxItemSet > tmpSet;

            if( pSrcDoc != pDstDoc && SfxItemState::SET == GetItemState(
                                            RES_PAGEDESC, false, &pItem ))
            {
                const SwPageDesc* pPgDesc = pItem->StaticWhichCast(RES_PAGEDESC).GetPageDesc();
                if( pPgDesc )
                {
                    tmpSet.emplace(*this);

                    SwPageDesc* pDstPgDesc = pDstDoc->FindPageDesc(pPgDesc->GetName());
                    if( !pDstPgDesc )
                    {
                        pDstPgDesc = pDstDoc->MakePageDesc(pPgDesc->GetName());
                        pDstDoc->CopyPageDesc( *pPgDesc, *pDstPgDesc );
                    }
                    SwFormatPageDesc aDesc( pDstPgDesc );
                    aDesc.SetNumOffset( pItem->StaticWhichCast(RES_PAGEDESC).GetNumOffset() );
                    tmpSet->Put( aDesc );
                }
            }

            if( pSrcDoc != pDstDoc && SfxItemState::SET == GetItemState( RES_ANCHOR, false, &pItem )
                && pItem->StaticWhichCast(RES_ANCHOR).GetContentAnchor() != nullptr )
            {
                if( !tmpSet )
                    tmpSet.emplace( *this );
                // Anchors at any node position cannot be copied to another document, because the SwPosition
                // would still point to the old document. It needs to be fixed up explicitly.
                tmpSet->ClearItem( RES_ANCHOR );
            }

            if (pSrcDoc != pDstDoc &&
                SfxItemState::SET == GetItemState(RES_PARATR_LIST_AUTOFMT, false, &pItem))
            {
                SfxItemSet const& rAutoStyle(*pItem->StaticWhichCast(RES_PARATR_LIST_AUTOFMT).GetStyleHandle());
                std::shared_ptr<SfxItemSet> const pNewSet(
                    rAutoStyle.SfxItemSet::Clone(true, &pDstDoc->GetAttrPool()));

                // fix up character style, it contains pointers to pSrcDoc
                if (SfxItemState::SET == pNewSet->GetItemState(RES_TXTATR_CHARFMT, false, &pItem))
                {
                    const auto& rChar(pItem->StaticWhichCast(RES_TXTATR_CHARFMT));
                    SwCharFormat *const pCopy(pDstDoc->CopyCharFormat(*rChar.GetCharFormat()));
                    const_cast<SwFormatCharFormat&>(rChar).SetCharFormat(pCopy);
                }

                SwFormatAutoFormat item(RES_PARATR_LIST_AUTOFMT);
                // TODO: for ODF export we'd need to add it to the autostyle pool
                item.SetStyleHandle(pNewSet);
                if (!tmpSet)
                {
                    tmpSet.emplace(*this);
                }
                tmpSet->Put(item);
            }

            if( tmpSet )
            {
                if( pCNd )
                {
                    // #i92811#
                    if ( pNewListIdItem != nullptr )
                    {
                        tmpSet->Put( *pNewListIdItem );
                    }
                    pCNd->SetAttr( *tmpSet );
                }
                else
                {
                    pFormat->SetFormatAttr( *tmpSet );
                }
            }
            else if( pCNd )
            {
                // #i92811#
                if ( pNewListIdItem != nullptr )
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
                pFormat->SetFormatAttr( *this );
            }
        }
    }
#if OSL_DEBUG_LEVEL > 0
    else
        OSL_FAIL("neither Format nor ContentNode - no Attributes copied");
#endif
}

/// check if ID is in range of attribute set IDs
bool IsInRange( const WhichRangesContainer& pRange, const sal_uInt16 nId )
{
    for(const auto& rPair : pRange)
    {
        if( rPair.first <= nId && nId <= rPair.second )
            return true;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
