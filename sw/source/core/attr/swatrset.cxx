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
#include <fmtpdsc.hxx>
#include <fmtautofmt.hxx>
#include <hintids.hxx>
#include <list.hxx>
#include <node.hxx>
#include <numrule.hxx>
#include <pagedesc.hxx>
#include <paratr.hxx>
#include <init.hxx>
#include <o3tl/unit_conversion.hxx>
#include <osl/diagnose.h>
#include <svl/whiter.hxx>

#include <svx/svdpool.hxx>
#include <svx/sxenditm.hxx>
#include <svx/sdsxyitm.hxx>



SwAttrPool::SwAttrPool(SwDoc* pD)
: SfxItemPool(u"SWG"_ustr)
, m_pDoc(pD)
{
    registerItemInfoPackage(getItemInfoPackageSwAttributes());

    // create SfxItemPool and EditEngine pool and add these in a chain. These
    // belong us and will be removed/destroyed in removeAndDeleteSecondaryPools() used from
    // the destructor
    rtl::Reference<SfxItemPool> pSdrPool = new SdrItemPool(this);

    // #75371# change DefaultItems for the SdrEdgeObj distance items
    // to TWIPS.
    constexpr tools::Long nDefEdgeDist
        = o3tl::convert(500, o3tl::Length::mm100, o3tl::Length::twip);

    pSdrPool->SetUserDefaultItem(SdrEdgeNode1HorzDistItem(nDefEdgeDist));
    pSdrPool->SetUserDefaultItem(SdrEdgeNode1VertDistItem(nDefEdgeDist));
    pSdrPool->SetUserDefaultItem(SdrEdgeNode2HorzDistItem(nDefEdgeDist));
    pSdrPool->SetUserDefaultItem(SdrEdgeNode2VertDistItem(nDefEdgeDist));

    // #i33700# // Set shadow distance defaults as PoolDefaultItems
    constexpr tools::Long nDefShadowDist
        = o3tl::convert(300, o3tl::Length::mm100, o3tl::Length::twip);
    pSdrPool->SetUserDefaultItem(makeSdrShadowXDistItem(nDefShadowDist));
    pSdrPool->SetUserDefaultItem(makeSdrShadowYDistItem(nDefShadowDist));

    rtl::Reference<SfxItemPool> pEEgPool = EditEngine::CreatePool();

    pSdrPool->SetSecondaryPool(pEEgPool.get());
}

SwAttrPool::~SwAttrPool()
{
    // cleanup secondary pools
    SfxItemPool* pSdrPool(GetSecondaryPool());

    // first delete the items, then break the linking
    pSdrPool->sendShutdownHint();
    SetSecondaryPool(nullptr);
}

/// Notification callback
void SwAttrSet::changeCallback(const SfxPoolItem* pOld, const SfxPoolItem* pNew) const
{
    // will have no effect, return
    if (nullptr == m_pOldSet && nullptr == m_pNewSet)
        return;

    // at least one SfxPoolItem has to be provided, else this is an error
    assert(nullptr != pOld || nullptr != pNew);
    sal_uInt16 nWhich(0);

    if (nullptr != pOld)
    {
        // do not handle if an invalid or disabled item is involved
        if (IsInvalidItem(pOld) || IsDisabledItem(pOld))
            return;

        // get WhichID from pOld
        nWhich = pOld->Which();
    }

    if (nullptr != pNew)
    {
        // do not handle if an invalid or disabled item is involved
        if (IsInvalidItem(pNew) || IsDisabledItem(pNew))
            return;

        if (0 == nWhich)
        {
            // get WhichID from pNew
            nWhich = pNew->Which();
        }
    }

    // all given items are valid. If we got no WhichID != 0 then
    // pOld == pNew == nullptr or IsDisabledItem and we have no
    // valid input. Also not needed if !IsWhich (aka > SFX_WHICH_MAX)
    if (0 == nWhich || !SfxItemPool::IsWhich(nWhich))
        return;

    if(m_pOldSet)
    {
        // old state shall be saved
        if (nullptr == pOld)
        {
            // no old value given, generate default from WhichID
            const SfxItemSet* pParent(GetParent());
            m_pOldSet->PutImpl(nullptr != pParent
                ? pParent->Get(nWhich)
                : GetPool()->GetUserOrPoolDefaultItem(nWhich), false);
        }
        else if (!IsInvalidItem(pOld))
        {
            // set/remember old value
            m_pOldSet->PutImpl(*pOld, false);
        }
    }

    if(m_pNewSet)
    {
        // old state shall be saved
        if (nullptr == pNew)
        {
            // no new value given, generate default from WhichID
            const SfxItemSet* pParent(GetParent());
            m_pNewSet->PutImpl(nullptr != pParent
                ? pParent->Get(nWhich)
                : GetPool()->GetUserOrPoolDefaultItem(nWhich), false);
        }
        else if (!IsInvalidItem(pNew))
        {
            // set/remember new value
            m_pNewSet->PutImpl(*pNew, false);
        }
    }
}

SwAttrSet::SwAttrSet( SwAttrPool& rPool, sal_uInt16 nWh1, sal_uInt16 nWh2 )
    : SfxItemSet( rPool, nWh1, nWh2 )
    , m_pOldSet( nullptr )
    , m_pNewSet( nullptr )
    , m_aCallbackHolder(this)
{
}

SwAttrSet::SwAttrSet( SwAttrPool& rPool, const WhichRangesContainer& nWhichPairTable )
    : SfxItemSet( rPool, nWhichPairTable )
    , m_pOldSet( nullptr )
    , m_pNewSet( nullptr )
    , m_aCallbackHolder(this)
{
}

SwAttrSet::SwAttrSet( const SwAttrSet& rSet )
    : SfxItemSet( rSet )
    , m_pOldSet( nullptr )
    , m_pNewSet( nullptr )
    , m_aCallbackHolder(this)
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

SwAttrSet SwAttrSet::CloneAsValue( bool bItems ) const
{
    if (bItems)
        return *this;
    else
        return SwAttrSet( *GetPool(), GetRanges() );
}

bool SwAttrSet::Put_BC( const SfxPoolItem& rAttr,
                       SwAttrSet* pOld, SwAttrSet* pNew )
{
    // direct call when neither pOld nor pNew is set, no need for callback
    if (nullptr == pOld && nullptr == pNew)
        return nullptr != SfxItemSet::Put( rAttr );

    m_pNewSet = pNew;
    m_pOldSet = pOld;
    setCallback(m_aCallbackHolder);
    bool bRet = nullptr != SfxItemSet::Put( rAttr );
    clearCallback();
    m_pOldSet = m_pNewSet = nullptr;
    return bRet;
}

bool SwAttrSet::Put_BC( const SfxItemSet& rSet,
                       SwAttrSet* pOld, SwAttrSet* pNew )
{
    // direct call when neither pOld nor pNew is set, no need for callback
    if (nullptr == pOld && nullptr == pNew)
        return SfxItemSet::Put( rSet );

    m_pNewSet = pNew;
    m_pOldSet = pOld;
    setCallback(m_aCallbackHolder);
    bool bRet = SfxItemSet::Put( rSet );
    clearCallback();
    m_pOldSet = m_pNewSet = nullptr;
    return bRet;
}

sal_uInt16 SwAttrSet::ClearItem_BC( sal_uInt16 nWhich,
                                    SwAttrSet* pOld, SwAttrSet* pNew )
{
    // direct call when neither pOld nor pNew is set, no need for callback
    if (nullptr == pOld && nullptr == pNew)
        return SfxItemSet::ClearItem( nWhich );

    m_pNewSet = pNew;
    m_pOldSet = pOld;
    setCallback(m_aCallbackHolder);
    sal_uInt16 nRet = SfxItemSet::ClearItem( nWhich );
    clearCallback();
    m_pOldSet = m_pNewSet = nullptr;
    return nRet;
}

sal_uInt16 SwAttrSet::ClearItem_BC( sal_uInt16 nWhich1, sal_uInt16 nWhich2,
                                    SwAttrSet* pOld, SwAttrSet* pNew )
{
    OSL_ENSURE( nWhich1 <= nWhich2, "no valid range" );
    sal_uInt16 nRet = 0;

    // direct call when neither pOld nor pNew is set, no need for callback
    if (nullptr == pOld && nullptr == pNew)
    {
        for( ; nWhich1 <= nWhich2; ++nWhich1 )
            nRet = nRet + SfxItemSet::ClearItem( nWhich1 );
        return nRet;
    }

    m_pNewSet = pNew;
    m_pOldSet = pOld;
    setCallback(m_aCallbackHolder);
    for( ; nWhich1 <= nWhich2; ++nWhich1 )
        nRet = nRet + SfxItemSet::ClearItem( nWhich1 );
    clearCallback();
    m_pOldSet = m_pNewSet = nullptr;
    return nRet;
}

int SwAttrSet::Intersect_BC( const SfxItemSet& rSet,
                             SwAttrSet* pOld, SwAttrSet* pNew )
{
    // direct call when neither pOld nor pNew is set, no need for callback
    if (nullptr == pOld && nullptr == pNew)
    {
        SfxItemSet::Intersect( rSet );
        return 0; // as below when neither pOld nor pNew is set
    }

    m_pNewSet = pNew;
    m_pOldSet = pOld;
    setCallback(m_aCallbackHolder);
    SfxItemSet::Intersect( rSet );
    clearCallback();
    m_pOldSet = m_pNewSet = nullptr;
    return pNew ? pNew->Count() : pOld->Count();
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

    const SwFormatPageDesc* pPageDescItem = GetItemIfSet( RES_PAGEDESC, false );
    if( pPageDescItem &&
        pPageDescItem->GetDefinedIn() != pModify  )
    {
        const_cast<SwFormatPageDesc&>(*pPageDescItem).ChgDefinedIn( pModify );
        bSet = true;
    }

    if(SwFormatDrop* pFormatDrop = const_cast<SwFormatDrop*>(GetItemIfSet( RES_PARATR_DROP, false )))
    {
        auto pDropDefiner = dynamic_cast<const sw::FormatDropDefiner*>(pModify);
        // If CharFormat is set and it is set in different attribute pools then
        // the CharFormat has to be copied.
        SwCharFormat* pCharFormat = pFormatDrop->GetCharFormat();
        if(pCharFormat && GetPool() != pCharFormat->GetAttrSet().GetPool())
        {
           pCharFormat = GetDoc()->CopyCharFormat(*pCharFormat);
           pFormatDrop->SetCharFormat(pCharFormat);
        }
        pFormatDrop->ChgDefinedIn(pDropDefiner);
        bSet = true;
    }

    const SwTableBoxFormula* pBoxFormula = GetItemIfSet( RES_BOXATR_FORMULA, false );
    if( pBoxFormula && pBoxFormula->GetDefinedIn() != pModify )
    {
        const_cast<SwTableBoxFormula&>(*pBoxFormula).ChgDefinedIn( pModify );
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

            const SwDoc *pSrcDoc = GetDoc();
            SwDoc *pDstDoc = pCNd ? &pCNd->GetDoc() : pFormat->GetDoc();

            // Does the NumRule has to be copied?
            const SwNumRuleItem* pNumRuleItem;
            if( pSrcDoc != pDstDoc &&
                (pNumRuleItem = GetItemIfSet( RES_PARATR_NUMRULE, false )) )
            {
                const OUString& rNm = pNumRuleItem->GetValue();
                if( !rNm.isEmpty() )
                {
                    SwNumRule* pDestRule = pDstDoc->FindNumRulePtr( rNm );
                    if( pDestRule )
                        pDestRule->Invalidate();
                    else
                        pDstDoc->MakeNumRule( rNm, pSrcDoc->FindNumRulePtr( rNm ) );
                }
            }

            // copy list and if needed also the corresponding list style
            // for text nodes
            const SfxStringItem* pStrItem;
            if ( pSrcDoc != pDstDoc &&
                 pCNd && pCNd->IsTextNode() &&
                 (pStrItem = GetItemIfSet( RES_PARATR_LIST_ID, false )) )
            {
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
            const SwFormatPageDesc* pPageDescItem;
            if( pSrcDoc != pDstDoc && (pPageDescItem = GetItemIfSet(
                                            RES_PAGEDESC, false )))
            {
                const SwPageDesc* pPgDesc = pPageDescItem->GetPageDesc();
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
                    aDesc.SetNumOffset( pPageDescItem->GetNumOffset() );
                    tmpSet->Put( aDesc );
                }
            }

            const SwFormatAnchor* pAnchorItem;
            if( pSrcDoc != pDstDoc && (pAnchorItem = GetItemIfSet( RES_ANCHOR, false ))
                && pAnchorItem->GetAnchorNode() != nullptr )
            {
                if( !tmpSet )
                    tmpSet.emplace( *this );
                // Anchors at any node position cannot be copied to another document, because the SwPosition
                // would still point to the old document. It needs to be fixed up explicitly.
                tmpSet->ClearItem( RES_ANCHOR );
            }

            const SwFormatAutoFormat* pAutoFormatItem;
            if (pSrcDoc != pDstDoc &&
                (pAutoFormatItem = GetItemIfSet(RES_PARATR_LIST_AUTOFMT, false)) &&
                pAutoFormatItem->GetStyleHandle())
            {
                SfxItemSet const& rAutoStyle(*pAutoFormatItem->GetStyleHandle());
                std::shared_ptr<SfxItemSet> const pNewSet(
                    rAutoStyle.SfxItemSet::Clone(true, &pDstDoc->GetAttrPool()));

                // fix up character style, it contains pointers to pSrcDoc
                if (const SwFormatCharFormat* pCharFormatItem = pNewSet->GetItemIfSet(RES_TXTATR_CHARFMT, false))
                {
                    SwCharFormat *const pCopy(pDstDoc->CopyCharFormat(*pCharFormatItem->GetCharFormat()));
                    const_cast<SwFormatCharFormat&>(*pCharFormatItem).SetCharFormat(pCopy);
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
                        tmpSet->Put( std::move(pNewListIdItem) );
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
                    aTmpSet.Put( std::move(pNewListIdItem) );
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
        if (pCNd && pCNd->HasSwAttrSet())
        {
            SfxWhichIter it(*this);
            std::vector<sal_uInt16> toClear;
            for (sal_uInt16 nWhich = it.FirstWhich(); nWhich != 0; nWhich = it.NextWhich())
            {
                if (GetItemState(nWhich, false) != SfxItemState::SET
                    && pCNd->GetSwAttrSet().GetItemState(nWhich, false) == SfxItemState::SET)
                {
                    toClear.emplace_back(nWhich);
                }
            }
            if (!toClear.empty())
            {
                pCNd->ResetAttr(toClear);
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
