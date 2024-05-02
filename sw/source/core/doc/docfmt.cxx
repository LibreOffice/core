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

#include <libxml/xmlwriter.h>
#include <hintids.hxx>
#include <svl/itemiter.hxx>
#include <svl/numformat.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/rsiditem.hxx>
#include <officecfg/Office/Common.hxx>
#include <osl/diagnose.h>
#include <svl/zforlist.hxx>
#include <svx/DocumentColorHelper.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/configmgr.hxx>
#include <sal/log.hxx>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <fmtpdsc.hxx>
#include <fmthdft.hxx>
#include <fmtcntnt.hxx>
#include <doc.hxx>
#include <docfunc.hxx>
#include <drawdoc.hxx>
#include <MarkManager.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentUndoRedo.hxx>
#include <DocumentContentOperationsManager.hxx>
#include <DocumentSettingManager.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <rootfrm.hxx>
#include <txtfrm.hxx>
#include <hints.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <UndoCore.hxx>
#include <UndoAttribute.hxx>
#include <UndoInsert.hxx>
#include <pagedesc.hxx>
#include <rolbck.hxx>
#include <mvsave.hxx>
#include <txatbase.hxx>
#include <swtblfmt.hxx>
#include <charfmt.hxx>
#include <docary.hxx>
#include <paratr.hxx>
#include <redline.hxx>
#include <reffld.hxx>
#include <fmtinfmt.hxx>
#include <breakit.hxx>
#include <SwUndoFmt.hxx>
#include <UndoManager.hxx>
#include <swmodule.hxx>
#include <modcfg.hxx>
#include <frameformats.hxx>
#include <textboxhelper.hxx>
#include <textcontentcontrol.hxx>
#include <memory>
#include <algorithm>
#include <functional>

using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;

/*
 * Internal functions
 */

static void SetTextFormatCollNext( SwTextFormatColl* pTextColl, const SwTextFormatColl* pDel )
{
    if ( &pTextColl->GetNextTextFormatColl() == pDel )
    {
        pTextColl->SetNextTextFormatColl( *pTextColl );
    }
}

static bool lcl_RstAttr( SwNode* pNd, void* pArgs )
{
    const sw::DocumentContentOperationsManager::ParaRstFormat* pPara = static_cast<sw::DocumentContentOperationsManager::ParaRstFormat*>(pArgs);
    SwContentNode* pNode = pNd->GetContentNode();
    if (pPara && pPara->pLayout && pPara->pLayout->HasMergedParas()
        && pNode && pNode->GetRedlineMergeFlag() == SwNode::Merge::Hidden)
    {
        return true;
    }
    if( pNode && pNode->HasSwAttrSet() )
    {
        const bool bLocked = pNode->IsModifyLocked();
        pNode->LockModify();

        SwDoc& rDoc = pNode->GetDoc();

        // remove unused attribute RES_LR_SPACE
        // add list attributes, except RES_PARATR_LIST_AUTOFMT
        SfxItemSetFixed<
                RES_PARATR_NUMRULE, RES_PARATR_NUMRULE,
                RES_PARATR_LIST_BEGIN, RES_PARATR_LIST_AUTOFMT - 1,
                RES_PAGEDESC, RES_BREAK,
                RES_FRMATR_STYLE_NAME, RES_FRMATR_CONDITIONAL_STYLE_NAME> aSavedAttrsSet(rDoc.GetAttrPool());
        const SfxItemSet* pAttrSetOfNode = pNode->GetpSwAttrSet();

        std::vector<sal_uInt16> aClearWhichIds;
        // restoring all paragraph list attributes
        {
            SfxItemSetFixed<RES_PARATR_LIST_BEGIN, RES_PARATR_LIST_AUTOFMT - 1> aListAttrSet( rDoc.GetAttrPool() );
            aListAttrSet.Set(*pAttrSetOfNode);
            if ( aListAttrSet.Count() )
            {
                aSavedAttrsSet.Put(aListAttrSet);
                SfxItemIter aIter( aListAttrSet );
                const SfxPoolItem* pItem = aIter.GetCurItem();
                while( pItem )
                {
                    aClearWhichIds.push_back( pItem->Which() );
                    pItem = aIter.NextItem();
                }
            }
        }

        if (auto pItem = pAttrSetOfNode->GetItemIfSet(RES_PARATR_NUMRULE, false);
            pItem && !pItem->GetValue().isEmpty())
        {
            aSavedAttrsSet.Put(*pItem);
            aClearWhichIds.push_back(RES_PARATR_NUMRULE);
        }
        if (auto pItem = pAttrSetOfNode->GetItemIfSet(RES_PAGEDESC, false);
            pItem && pItem->GetPageDesc())
        {
            aSavedAttrsSet.Put(*pItem);
            aClearWhichIds.push_back(RES_PAGEDESC);
        }
        if (auto pItem = pAttrSetOfNode->GetItemIfSet(RES_BREAK, false);
            pItem && pItem->GetBreak() != SvxBreak::NONE)
        {
            aSavedAttrsSet.Put(*pItem);
            aClearWhichIds.push_back(RES_BREAK);
        }
        if (auto pItem = pAttrSetOfNode->GetItemIfSet(RES_FRMATR_STYLE_NAME, false);
            pItem && !pItem->GetValue().isEmpty())
        {
            aSavedAttrsSet.Put(*pItem);
            aClearWhichIds.push_back(RES_FRMATR_STYLE_NAME);
        }
        if (auto pItem = pAttrSetOfNode->GetItemIfSet(RES_FRMATR_CONDITIONAL_STYLE_NAME, false);
            pItem && !pItem->GetValue().isEmpty())
        {
            aSavedAttrsSet.Put(*pItem);
            aClearWhichIds.push_back(RES_FRMATR_CONDITIONAL_STYLE_NAME);
        }

        // do not clear items directly from item set and only clear to be kept
        // attributes, if no deletion item set is found.
        const bool bKeepAttributes =
                    !pPara || !pPara->pDelSet || pPara->pDelSet->Count() == 0;
        if ( bKeepAttributes )
        {
            pNode->ResetAttr( aClearWhichIds );
        }

        if( !bLocked )
            pNode->UnlockModify();

        if( pPara )
        {
            SwRegHistory aRegH( pNode, *pNode, pPara->pHistory );

            if( pPara->pDelSet && pPara->pDelSet->Count() )
            {
                OSL_ENSURE( !bKeepAttributes,
                        "<lcl_RstAttr(..)> - certain attributes are kept, but not needed." );
                SfxItemIter aIter( *pPara->pDelSet );
                for (const SfxPoolItem* pItem = aIter.GetCurItem(); pItem; pItem = aIter.NextItem())
                {
                    if ( ( pItem->Which() != RES_PAGEDESC &&
                           pItem->Which() != RES_BREAK &&
                           pItem->Which() != RES_FRMATR_STYLE_NAME &&
                           pItem->Which() != RES_FRMATR_CONDITIONAL_STYLE_NAME &&
                           pItem->Which() != RES_PARATR_NUMRULE ) ||
                         ( aSavedAttrsSet.GetItemState( pItem->Which(), false ) != SfxItemState::SET ) )
                    {
                        pNode->ResetAttr( pItem->Which() );
                    }
                }
            }
            else if( pPara->bResetAll )
                pNode->ResetAllAttr();
            else
                pNode->ResetAttr( RES_PARATR_BEGIN, POOLATTR_END - 1 );
        }
        else
            pNode->ResetAllAttr();

        // only restore saved attributes, if needed
        if (bKeepAttributes && aSavedAttrsSet.Count())
        {
            pNode->LockModify();

            pNode->SetAttr(aSavedAttrsSet);

            if( !bLocked )
                pNode->UnlockModify();
        }
    }
    return true;
}

void SwDoc::RstTextAttrs(const SwPaM &rRg, bool bInclRefToxMark,
        bool bExactRange, SwRootFrame const*const pLayout)
{
    SwHistory* pHst = nullptr;
    SwDataChanged aTmp( rRg );
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        std::unique_ptr<SwUndoResetAttr> pUndo(new SwUndoResetAttr( rRg, RES_CHRFMT ));
        pHst = &pUndo->GetHistory();
        GetIDocumentUndoRedo().AppendUndo(std::move(pUndo));
    }
    auto [pStt, pEnd] = rRg.StartEnd(); // SwPosition*
    sw::DocumentContentOperationsManager::ParaRstFormat aPara(
            pStt, pEnd, pHst, nullptr, pLayout );
    aPara.bInclRefToxMark = bInclRefToxMark;
    aPara.bExactRange = bExactRange;
    GetNodes().ForEach( pStt->GetNodeIndex(), pEnd->GetNodeIndex()+1,
                        sw::DocumentContentOperationsManager::lcl_RstTextAttr, &aPara );
    getIDocumentState().SetModified();
}

void SwDoc::ResetAttrs( const SwPaM &rRg,
                        bool bTextAttr,
                        const o3tl::sorted_vector<sal_uInt16> &rAttrs,
                        const bool bSendDataChangedEvents,
                        SwRootFrame const*const pLayout)
{
    SwPaM* pPam = const_cast<SwPaM*>(&rRg);
    std::optional<SwPaM> oExtraPaM;
    if( !bTextAttr && !rAttrs.empty() && RES_TXTATR_END > *(rAttrs.begin()) )
        bTextAttr = true;

    if( !rRg.HasMark() )
    {
        SwTextNode* pTextNd = rRg.GetPoint()->GetNode().GetTextNode();
        if( !pTextNd )
            return ;

        oExtraPaM.emplace( *rRg.GetPoint() );
        pPam = &*oExtraPaM;

        SwPosition& rSt = *pPam->GetPoint();
        sal_Int32 nMkPos, nPtPos = rSt.GetContentIndex();

        // Special case: if the Cursor is located within a URL attribute, we take over it's area
        SwTextAttr const*const pURLAttr(
            pTextNd->GetTextAttrAt(rSt.GetContentIndex(), RES_TXTATR_INETFMT));
        if (pURLAttr && !pURLAttr->GetINetFormat().GetValue().isEmpty())
        {
            nMkPos = pURLAttr->GetStart();
            nPtPos = *pURLAttr->End();
        }
        else
        {
            assert(g_pBreakIt && g_pBreakIt->GetBreakIter().is());
            Boundary aBndry = g_pBreakIt->GetBreakIter()->getWordBoundary(
                                pTextNd->GetText(), nPtPos,
                                g_pBreakIt->GetLocale( pTextNd->GetLang( nPtPos ) ),
                                WordType::ANY_WORD /*ANYWORD_IGNOREWHITESPACES*/,
                                true);

            if( aBndry.startPos < nPtPos && nPtPos < aBndry.endPos )
            {
                nMkPos = aBndry.startPos;
                nPtPos = aBndry.endPos;
            }
            else
            {
                nPtPos = nMkPos = rSt.GetContentIndex();
                if( bTextAttr )
                    pTextNd->DontExpandFormat( nPtPos );
            }
        }

        rSt.SetContent(nMkPos);
        pPam->SetMark();
        pPam->GetPoint()->SetContent(nPtPos);
    }

    // #i96644#
    std::optional< SwDataChanged > oDataChanged;
    if ( bSendDataChangedEvents )
    {
        oDataChanged.emplace( *pPam );
    }
    SwHistory* pHst = nullptr;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        std::unique_ptr<SwUndoResetAttr> pUndo(new SwUndoResetAttr( rRg,
            bTextAttr ? sal_uInt16(RES_CONDTXTFMTCOLL) : sal_uInt16(RES_TXTFMTCOLL) ));
        if( !rAttrs.empty() )
        {
            pUndo->SetAttrs( o3tl::sorted_vector(rAttrs) );
        }
        pHst = &pUndo->GetHistory();
        GetIDocumentUndoRedo().AppendUndo(std::move(pUndo));
    }

    auto [pStt, pEnd] = pPam->StartEnd(); // SwPosition*
    sw::DocumentContentOperationsManager::ParaRstFormat aPara(
            pStt, pEnd, pHst, nullptr, pLayout);

    // mst: not including META here; it seems attrs with CH_TXTATR are omitted
    SfxItemSetFixed<RES_CHRATR_BEGIN, RES_CHRATR_END - 1,
                     RES_TXTATR_INETFMT, RES_TXTATR_UNKNOWN_CONTAINER,
                     RES_PARATR_BEGIN, RES_FRMATR_END - 1,
                     RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END - 1>
        aDelSet(GetAttrPool());
    for( auto it = rAttrs.rbegin(); it != rAttrs.rend(); ++it )
    {
        if( POOLATTR_END > *it )
            aDelSet.Put( *GetDfltAttr( *it ));
    }
    if( aDelSet.Count() )
        aPara.pDelSet = &aDelSet;

    bool bAdd = true;
    SwNodeIndex aTmpStt( pStt->GetNode() );
    SwNodeIndex aTmpEnd( pEnd->GetNode() );
    if( pStt->GetContentIndex() )     // just one part
    {
        // set up a later, and all CharFormatAttr -> TextFormatAttr
        SwTextNode* pTNd = aTmpStt.GetNode().GetTextNode();
        if( pTNd && pTNd->HasSwAttrSet() && pTNd->GetpSwAttrSet()->Count() )
        {
            if (pHst)
            {
                SwRegHistory history(pTNd, *pTNd, pHst);
                pTNd->FormatToTextAttr(pTNd);
            }
            else
            {
                pTNd->FormatToTextAttr(pTNd);
            }
        }

        ++aTmpStt;
    }
    if( pEnd->GetContentIndex() == pEnd->GetNode().GetContentNode()->Len() )
    {
         // set up a later, and all CharFormatAttr -> TextFormatAttr
        ++aTmpEnd;
        bAdd = false;
    }
    else if( pStt->GetNode() != pEnd->GetNode() || !pStt->GetContentIndex() )
    {
        SwTextNode* pTNd = aTmpEnd.GetNode().GetTextNode();
        if( pTNd && pTNd->HasSwAttrSet() && pTNd->GetpSwAttrSet()->Count() )
        {
            if (pHst)
            {
                SwRegHistory history(pTNd, *pTNd, pHst);
                pTNd->FormatToTextAttr(pTNd);
            }
            else
            {
                pTNd->FormatToTextAttr(pTNd);
            }
        }
    }

    if( aTmpStt < aTmpEnd )
        GetNodes().ForEach( pStt->GetNode(), aTmpEnd.GetNode(), lcl_RstAttr, &aPara );
    else if( !rRg.HasMark() )
    {
        aPara.bResetAll = false ;
        ::lcl_RstAttr( &pStt->GetNode(), &aPara );
        aPara.bResetAll = true ;
    }

    if( bTextAttr )
    {
        if( bAdd )
            ++aTmpEnd;
        GetNodes().ForEach( pStt->GetNode(), aTmpEnd.GetNode(), sw::DocumentContentOperationsManager::lcl_RstTextAttr, &aPara );
    }

    getIDocumentState().SetModified();

    oDataChanged.reset(); //before delete pPam
}

/// Set the rsid of the next nLen symbols of rRg to the current session number
void SwDoc::UpdateRsid( const SwPaM &rRg, const sal_Int32 nLen )
{
    if (!SW_MOD()->GetModuleConfig()->IsStoreRsid())
        return;

    SwTextNode *pTextNode = rRg.GetPoint()->GetNode().GetTextNode();
    if (!pTextNode)
    {
        return;
    }
    const sal_Int32 nStart(rRg.GetPoint()->GetContentIndex() - nLen);
    SvxRsidItem aRsid( mnRsid, RES_CHRATR_RSID );

    SfxItemSetFixed<RES_CHRATR_RSID, RES_CHRATR_RSID> aSet(GetAttrPool());
    aSet.Put(aRsid);
    bool const bRet(pTextNode->SetAttr(aSet, nStart,
        rRg.GetPoint()->GetContentIndex()));

    if (bRet && GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndo *const pLastUndo = GetUndoManager().GetLastUndo();
        SwUndoInsert *const pUndoInsert(dynamic_cast<SwUndoInsert*>(pLastUndo));
        // this function is called after Insert so expects to find SwUndoInsert
        assert(pUndoInsert);
        if (pUndoInsert)
        {
            pUndoInsert->SetWithRsid();
        }
    }
}

bool SwDoc::UpdateParRsid( SwTextNode *pTextNode, sal_uInt32 nVal )
{
    if (!SW_MOD()->GetModuleConfig()->IsStoreRsid())
        return false;

    if (!pTextNode)
    {
        return false;
    }

    SvxRsidItem aRsid( nVal ? nVal : mnRsid, RES_PARATR_RSID );
    return pTextNode->SetAttr( aRsid );
}

/// Set the attribute according to the stated format.
/// If Undo is enabled, the old values is added to the Undo history.
void SwDoc::SetAttr( const SfxPoolItem& rAttr, SwFormat& rFormat )
{
    SfxItemSet aSet( GetAttrPool(), rAttr.Which(), rAttr.Which() );
    aSet.Put( rAttr );
    SetAttr( aSet, rFormat );
}

/// Set the attribute according to the stated format.
/// If Undo is enabled, the old values is added to the Undo history.
void SwDoc::SetAttr( const SfxItemSet& rSet, SwFormat& rFormat )
{
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndoFormatAttrHelper aTmp( rFormat );
        rFormat.SetFormatAttr( rSet );
        if ( aTmp.GetUndo() )
        {
            GetIDocumentUndoRedo().AppendUndo( aTmp.ReleaseUndo() );
        }
        else
        {
            GetIDocumentUndoRedo().ClearRedo();
        }
    }
    else
    {
        rFormat.SetFormatAttr( rSet );
    }

    // If the format is a shape, and it has a textbox, sync.
    auto pShapeFormat = dynamic_cast<SwFrameFormat*>(&rFormat);
    if (pShapeFormat && SwTextBoxHelper::isTextBox(pShapeFormat, RES_DRAWFRMFMT))
    {
        if (auto pObj = pShapeFormat->FindRealSdrObject())
        {
            SwTextBoxHelper::syncFlyFrameAttr(*pShapeFormat, rSet, pObj);
            SwTextBoxHelper::changeAnchor(pShapeFormat, pObj);
        }
    }

    getIDocumentState().SetModified();
}

void SwDoc::ResetAttrAtFormat( const std::vector<sal_uInt16>& rIds,
                               SwFormat& rChangedFormat )
{
    std::unique_ptr<SwUndo> pUndo;
    if (GetIDocumentUndoRedo().DoesUndo())
        pUndo.reset(new SwUndoFormatResetAttr( rChangedFormat, rIds ));

    bool bAttrReset = false;
    for (const auto& nWhichId : rIds)
        bAttrReset = rChangedFormat.ResetFormatAttr(nWhichId) || bAttrReset;

    if ( bAttrReset )
    {
        if ( pUndo )
        {
            GetIDocumentUndoRedo().AppendUndo( std::move(pUndo) );
        }

        getIDocumentState().SetModified();
    }
}

static bool lcl_SetNewDefTabStops( SwTwips nOldWidth, SwTwips nNewWidth,
                                SvxTabStopItem& rChgTabStop )
{
    // Set the default values of all TabStops to the new value.
    // Attention: we always work with the PoolAttribute here, so that
    // we don't calculate the same value on the same TabStop (pooled!) for all sets.
    // We send a FormatChg to modify.

    sal_uInt16 nOldCnt = rChgTabStop.Count();
    if( !nOldCnt || nOldWidth == nNewWidth )
        return false;

    // Find the default's beginning
    sal_uInt16 n;
    for( n = nOldCnt; n ; --n )
        if( SvxTabAdjust::Default != rChgTabStop[n - 1].GetAdjustment() )
            break;
    ++n;
    if( n < nOldCnt )   // delete the DefTabStops
        rChgTabStop.Remove( n, nOldCnt - n );
    return true;
}

/// Set the attribute as new default attribute in this document.
/// If Undo is enabled, the old value is added to the Undo history.
void SwDoc::SetDefault( const SfxPoolItem& rAttr )
{
    SfxItemSet aSet( GetAttrPool(), rAttr.Which(), rAttr.Which() );
    aSet.Put( rAttr );
    SetDefault( aSet );
}

void SwDoc::SetDefault( const SfxItemSet& rSet )
{
    if( !rSet.Count() )
        return;

    sw::BroadcastingModify aCallMod;
    SwAttrSet aOld( GetAttrPool(), rSet.GetRanges() ),
            aNew( GetAttrPool(), rSet.GetRanges() );
    SfxItemIter aIter( rSet );
    const SfxPoolItem* pItem = aIter.GetCurItem();
    SfxItemPool* pSdrPool = GetAttrPool().GetSecondaryPool();
    do
    {
        bool bCheckSdrDflt = false;
        const sal_uInt16 nWhich = pItem->Which();
        aOld.Put( GetAttrPool().GetUserOrPoolDefaultItem( nWhich ) );
        GetAttrPool().SetUserDefaultItem( *pItem );
        aNew.Put( GetAttrPool().GetUserOrPoolDefaultItem( nWhich ) );

        if (isCHRATR(nWhich) || isTXTATR(nWhich))
        {
            aCallMod.Add(*mpDfltTextFormatColl);
            aCallMod.Add(*mpDfltCharFormat);
            bCheckSdrDflt = nullptr != pSdrPool;
        }
        else if ( isPARATR(nWhich) ||
                  isPARATR_LIST(nWhich) )
        {
            aCallMod.Add(*mpDfltTextFormatColl);
            bCheckSdrDflt = nullptr != pSdrPool;
        }
        else if (isGRFATR(nWhich))
        {
            aCallMod.Add(*mpDfltGrfFormatColl);
        }
        else if (isFRMATR(nWhich) || isDrawingLayerAttribute(nWhich) )
        {
            aCallMod.Add(*mpDfltGrfFormatColl);
            aCallMod.Add(*mpDfltTextFormatColl);
            aCallMod.Add(*mpDfltFrameFormat);
        }
        else if (isBOXATR(nWhich))
        {
            aCallMod.Add(*mpDfltFrameFormat);
        }

        // also copy the defaults
        if( bCheckSdrDflt )
        {
            sal_uInt16 nSlotId = GetAttrPool().GetSlotId( nWhich );
            if( 0 != nSlotId && nSlotId != nWhich )
            {
                sal_uInt16 nEdtWhich = pSdrPool->GetWhichIDFromSlotID( nSlotId );
                if( 0 != nEdtWhich && nSlotId != nEdtWhich )
                {
                    std::unique_ptr<SfxPoolItem> pCpy(pItem->Clone());
                    pCpy->SetWhich( nEdtWhich );
                    pSdrPool->SetUserDefaultItem( *pCpy );
                }
            }
        }

        pItem = aIter.NextItem();
    } while (pItem);

    if( aNew.Count() && aCallMod.HasWriterListeners() )
    {
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            GetIDocumentUndoRedo().AppendUndo( std::make_unique<SwUndoDefaultAttr>( aOld, *this ) );
        }

        const SvxTabStopItem* pTmpItem = aNew.GetItemIfSet( RES_PARATR_TABSTOP, false );
        if( pTmpItem && pTmpItem->Count() )
        {
            // Set the default values of all TabStops to the new value.
            // Attention: we always work with the PoolAttribute here, so that
            // we don't calculate the same value on the same TabStop (pooled!) for all sets.
            // We send a FormatChg to modify.
            SwTwips nNewWidth = (*pTmpItem)[ 0 ].GetTabPos(),
                    nOldWidth = aOld.Get(RES_PARATR_TABSTOP)[ 0 ].GetTabPos();

            bool bChg = false;
            ItemSurrogates aSurrogates;
            GetAttrPool().GetItemSurrogates(aSurrogates, RES_PARATR_TABSTOP);
            for (const SfxPoolItem* pItem2 : aSurrogates)
            {
                // pItem2 and thus pTabStopItem is a evtl. shared & RefCounted
                // Item and *should* not be changed that way. lcl_SetNewDefTabStops
                // seems to change pTabStopItem (!). This may need to be changed
                // to use iterateItemSurrogates and a defined write cycle.
                if(auto pTabStopItem = pItem2->DynamicWhichCast(RES_PARATR_TABSTOP))
                    bChg |= lcl_SetNewDefTabStops( nOldWidth, nNewWidth,
                                                   *const_cast<SvxTabStopItem*>(pTabStopItem) );
            }

            aNew.ClearItem( RES_PARATR_TABSTOP );
            aOld.ClearItem( RES_PARATR_TABSTOP );
            if( bChg )
            {
                SwFormatChg aChgFormat( mpDfltCharFormat.get() );
                // notify the frames
                aCallMod.CallSwClientNotify(sw::LegacyModifyHint( &aChgFormat, &aChgFormat ));
            }
        }
    }

    if( aNew.Count() && aCallMod.HasWriterListeners() )
    {
        SwAttrSetChg aChgOld( aOld, aOld );
        SwAttrSetChg aChgNew( aNew, aNew );
        aCallMod.CallSwClientNotify(sw::LegacyModifyHint( &aChgOld, &aChgNew ));      // all changed are sent
    }

    // remove the default formats from the object again
    SwIterator<SwClient, sw::BroadcastingModify> aClientIter(aCallMod);
    for(SwClient* pClient = aClientIter.First(); pClient; pClient = aClientIter.Next())
        aCallMod.Remove(*pClient);

    getIDocumentState().SetModified();
}

/// Get the default attribute in this document
const SfxPoolItem& SwDoc::GetDefault( sal_uInt16 nFormatHint ) const
{
    return GetAttrPool().GetUserOrPoolDefaultItem( nFormatHint );
}

/// Delete the formats
void SwDoc::DelCharFormat(size_t nFormat, bool bBroadcast)
{
    SwCharFormat * pDel = (*mpCharFormatTable)[nFormat];

    if (bBroadcast)
        BroadcastStyleOperation(pDel->GetName(), SfxStyleFamily::Char,
                                SfxHintId::StyleSheetErased);

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo(
            std::make_unique<SwUndoCharFormatDelete>(pDel, *this));
    }

    delete (*mpCharFormatTable)[nFormat];
    mpCharFormatTable->erase(mpCharFormatTable->begin() + nFormat);

    getIDocumentState().SetModified();
}

void SwDoc::DelCharFormat( SwCharFormat const *pFormat, bool bBroadcast )
{
    size_t nFormat = mpCharFormatTable->GetPos( pFormat );
    OSL_ENSURE( SIZE_MAX != nFormat, "Format not found," );
    DelCharFormat( nFormat, bBroadcast );
}

void SwDoc::DelFrameFormat( SwFrameFormat *pFormat, bool bBroadcast )
{
    assert(pFormat && "ContainsFormat will always deref pFormat");
    if( dynamic_cast<const SwTableBoxFormat*>( pFormat) != nullptr || dynamic_cast<const SwTableLineFormat*>( pFormat) != nullptr )
    {
        OSL_ENSURE( false, "Format is not in the DocArray any more, "
                       "so it can be deleted with delete" );
        delete pFormat;
    }
    else
    {
        // The format has to be in the one or the other, we'll see in which one.
        if (mpFrameFormatTable->ContainsFormat(pFormat))
        {
            if (bBroadcast)
                BroadcastStyleOperation(pFormat->GetName(),
                                        SfxStyleFamily::Frame,
                                        SfxHintId::StyleSheetErased);

            if (GetIDocumentUndoRedo().DoesUndo())
            {
                GetIDocumentUndoRedo().AppendUndo(
                    std::make_unique<SwUndoFrameFormatDelete>(pFormat, *this));
            }

            mpFrameFormatTable->erase( pFormat );
            delete pFormat;
        }
        else
        {
            auto pSpz = static_cast<sw::SpzFrameFormat*>(pFormat);
            if(GetSpzFrameFormats()->ContainsFormat(pSpz))
            {
                GetSpzFrameFormats()->erase(pSpz);
                delete pSpz;
            }
            else
                SAL_WARN("sw", "FrameFormat not found.");
        }
    }
}

void SwDoc::DelTableFrameFormat( SwTableFormat *pFormat )
{
    auto it = mpTableFrameFormatTable->find( pFormat );
    OSL_ENSURE( it != mpTableFrameFormatTable->end(), "Format not found," );
    mpTableFrameFormatTable->erase( it );
    delete pFormat;
}

SwFrameFormat* SwDoc::FindFrameFormatByName( const OUString& rName ) const
{
    return static_cast<SwFrameFormat*>(mpFrameFormatTable->FindFormatByName(rName));
}

/// Create the formats
SwFlyFrameFormat *SwDoc::MakeFlyFrameFormat( const OUString &rFormatName,
                                    SwFrameFormat *pDerivedFrom )
{
    SwFlyFrameFormat *pFormat = new SwFlyFrameFormat( GetAttrPool(), rFormatName, pDerivedFrom );
    GetSpzFrameFormats()->push_back(pFormat);
    getIDocumentState().SetModified();
    return pFormat;
}

SwDrawFrameFormat *SwDoc::MakeDrawFrameFormat( const OUString &rFormatName,
                                     SwFrameFormat *pDerivedFrom )
{
    SwDrawFrameFormat *pFormat = new SwDrawFrameFormat( GetAttrPool(), rFormatName, pDerivedFrom);
    GetSpzFrameFormats()->push_back(pFormat);
    getIDocumentState().SetModified();
    return pFormat;
}

size_t SwDoc::GetTableFrameFormatCount(bool bUsed) const
{
    if (!bUsed)
        return mpTableFrameFormatTable->size();
    return std::count_if(mpTableFrameFormatTable->begin(), mpTableFrameFormatTable->end(),
            std::mem_fn(&SwFormat::IsUsed));
}

SwTableFormat& SwDoc::GetTableFrameFormat(size_t nFormat, bool bUsed) const
{
    if (!bUsed)
        return *const_cast<SwTableFormat*>((*mpTableFrameFormatTable)[nFormat]);
    for(SwTableFormat* pFormat: *mpTableFrameFormatTable)
    {
        if(!pFormat->IsUsed())
            continue;
        if(nFormat)
            --nFormat;
        else
            return *pFormat;
    }
    throw std::out_of_range("Format index out of range.");
}

SwTableFormat* SwDoc::MakeTableFrameFormat( const OUString &rFormatName,
                                    SwFrameFormat *pDerivedFrom )
{
    SwTableFormat* pFormat = new SwTableFormat( GetAttrPool(), rFormatName, pDerivedFrom );
    mpTableFrameFormatTable->push_back( pFormat );
    getIDocumentState().SetModified();

    return pFormat;
}

SwFrameFormat *SwDoc::MakeFrameFormat(const OUString &rFormatName,
                            SwFrameFormat *pDerivedFrom,
                            bool bBroadcast, bool bAuto)
{
    SwFrameFormat *pFormat = new SwFrameFormat( GetAttrPool(), rFormatName, pDerivedFrom );

    pFormat->SetAuto(bAuto);
    mpFrameFormatTable->push_back( pFormat );
    getIDocumentState().SetModified();

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo(
            std::make_unique<SwUndoFrameFormatCreate>(pFormat, pDerivedFrom, *this));
    }

    if (bBroadcast)
    {
        BroadcastStyleOperation(rFormatName, SfxStyleFamily::Frame,
                                SfxHintId::StyleSheetCreated);
    }

    return pFormat;
}

SwFormat *SwDoc::MakeFrameFormat_(const OUString &rFormatName,
                            SwFormat *pDerivedFrom,
                            bool bBroadcast, bool bAuto)
{
    SwFrameFormat *pFrameFormat = dynamic_cast<SwFrameFormat*>(pDerivedFrom);
    pFrameFormat = MakeFrameFormat( rFormatName, pFrameFormat, bBroadcast, bAuto );
    return pFrameFormat;
}

SwCharFormat *SwDoc::MakeCharFormat( const OUString &rFormatName,
                               SwCharFormat *pDerivedFrom,
                               bool bBroadcast )
{
    SwCharFormat *pFormat = new SwCharFormat( GetAttrPool(), rFormatName, pDerivedFrom );
    mpCharFormatTable->insert( pFormat );
    pFormat->SetAuto(false);
    getIDocumentState().SetModified();

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo(
            std::make_unique<SwUndoCharFormatCreate>(pFormat, pDerivedFrom, *this));
    }

    if (bBroadcast)
    {
        BroadcastStyleOperation(rFormatName, SfxStyleFamily::Char,
                                SfxHintId::StyleSheetCreated);
    }

    return pFormat;
}

SwFormat *SwDoc::MakeCharFormat_(const OUString &rFormatName,
                            SwFormat *pDerivedFrom,
                            bool bBroadcast, bool /*bAuto*/)
{
    SwCharFormat *pCharFormat = dynamic_cast<SwCharFormat*>(pDerivedFrom);
    pCharFormat = MakeCharFormat( rFormatName, pCharFormat, bBroadcast );
    return pCharFormat;
}

/// Create the FormatCollections
SwTextFormatColl* SwDoc::MakeTextFormatColl( const OUString &rFormatName,
                                     SwTextFormatColl *pDerivedFrom,
                                     bool bBroadcast)
{
    SwTextFormatColl *pFormatColl = new SwTextFormatColl( GetAttrPool(), rFormatName,
                                                pDerivedFrom );
    mpTextFormatCollTable->push_back(pFormatColl);
    pFormatColl->SetAuto(false);
    getIDocumentState().SetModified();

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo(
            std::make_unique<SwUndoTextFormatCollCreate>(pFormatColl, pDerivedFrom,
                                                         *this));
    }

    if (bBroadcast)
        BroadcastStyleOperation(rFormatName, SfxStyleFamily::Para,
                                SfxHintId::StyleSheetCreated);

    return pFormatColl;
}

SwFormat *SwDoc::MakeTextFormatColl_(const OUString &rFormatName,
                            SwFormat *pDerivedFrom,
                            bool bBroadcast, bool /*bAuto*/)
{
    SwTextFormatColl *pTextFormatColl = dynamic_cast<SwTextFormatColl*>(pDerivedFrom);
    pTextFormatColl = MakeTextFormatColl( rFormatName, pTextFormatColl, bBroadcast );
    return pTextFormatColl;
}

SwConditionTextFormatColl* SwDoc::MakeCondTextFormatColl( const OUString &rFormatName,
                                                  SwTextFormatColl *pDerivedFrom,
                                                  bool bBroadcast)
{
    SwConditionTextFormatColl*pFormatColl = new SwConditionTextFormatColl( GetAttrPool(),
                                                    rFormatName, pDerivedFrom );
    mpTextFormatCollTable->push_back(pFormatColl);
    pFormatColl->SetAuto(false);
    getIDocumentState().SetModified();

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo(
            std::make_unique<SwUndoCondTextFormatCollCreate>(pFormatColl, pDerivedFrom,
                                                             *this));
    }

    if (bBroadcast)
        BroadcastStyleOperation(rFormatName, SfxStyleFamily::Para,
                                SfxHintId::StyleSheetCreated);

    return pFormatColl;
}

// GRF
SwGrfFormatColl* SwDoc::MakeGrfFormatColl( const OUString &rFormatName,
                                     SwGrfFormatColl *pDerivedFrom )
{
    SwGrfFormatColl *pFormatColl = new SwGrfFormatColl( GetAttrPool(), rFormatName,
                                                pDerivedFrom );
    mpGrfFormatCollTable->push_back( pFormatColl );
    pFormatColl->SetAuto(false);
    getIDocumentState().SetModified();
    return pFormatColl;
}

void SwDoc::DelTextFormatColl(size_t nFormatColl, bool bBroadcast)
{
    OSL_ENSURE( nFormatColl, "Remove of Coll 0." );

    // Who has the to-be-deleted as their Next?
    SwTextFormatColl *pDel = (*mpTextFormatCollTable)[nFormatColl];
    if( mpDfltTextFormatColl.get() == pDel )
        return;     // never delete default!

    if (bBroadcast)
        BroadcastStyleOperation(pDel->GetName(), SfxStyleFamily::Para,
                                SfxHintId::StyleSheetErased);

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        std::unique_ptr<SwUndoTextFormatCollDelete> pUndo;
        if (RES_CONDTXTFMTCOLL == pDel->Which())
        {
            pUndo.reset(new SwUndoCondTextFormatCollDelete(pDel, *this));
        }
        else
        {
            pUndo.reset(new SwUndoTextFormatCollDelete(pDel, *this));
        }

        GetIDocumentUndoRedo().AppendUndo(std::move(pUndo));
    }

    // Remove the FormatColl
    mpTextFormatCollTable->erase(mpTextFormatCollTable->begin() + nFormatColl);
    // Correct next
    for( SwTextFormatColls::const_iterator it = mpTextFormatCollTable->begin() + 1; it != mpTextFormatCollTable->end(); ++it )
        SetTextFormatCollNext( *it, pDel );
    delete pDel;
    getIDocumentState().SetModified();
}

void SwDoc::DelTextFormatColl( SwTextFormatColl const *pColl, bool bBroadcast )
{
    size_t nFormat = mpTextFormatCollTable->GetPos( pColl );
    OSL_ENSURE( SIZE_MAX != nFormat, "Collection not found," );
    DelTextFormatColl( nFormat, bBroadcast );
}

static bool lcl_SetTextFormatColl( SwNode* pNode, void* pArgs )
{
    SwContentNode* pCNd = pNode->GetTextNode();

    if( pCNd == nullptr)
        return true;

    sw::DocumentContentOperationsManager::ParaRstFormat* pPara = static_cast<sw::DocumentContentOperationsManager::ParaRstFormat*>(pArgs);

    if (pPara->pLayout && pPara->pLayout->HasMergedParas())
    {
        if (pCNd->GetRedlineMergeFlag() == SwNode::Merge::Hidden)
        {
            return true;
        }
        if (pCNd->IsTextNode())
        {
            pCNd = sw::GetParaPropsNode(*pPara->pLayout, *pCNd);
        }
    }

    SwTextFormatColl* pFormat = pPara->pFormatColl;
    if ( pPara->bReset )
    {
        lcl_RstAttr(pCNd, pPara);

        // #i62675# check, if paragraph style has changed
        if ( pPara->bResetListAttrs &&
             pFormat != pCNd->GetFormatColl() &&
             pFormat->GetItemState( RES_PARATR_NUMRULE ) == SfxItemState::SET )
        {
            // Check, if the list style of the paragraph will change.
            bool bChangeOfListStyleAtParagraph( true );
            SwTextNode& rTNd(*pCNd->GetTextNode());
            {
                SwNumRule* pNumRuleAtParagraph(rTNd.GetNumRule());
                if ( pNumRuleAtParagraph )
                {
                    const SwNumRuleItem& rNumRuleItemAtParagraphStyle =
                        pFormat->GetNumRule();
                    if ( rNumRuleItemAtParagraphStyle.GetValue() ==
                            pNumRuleAtParagraph->GetName() )
                    {
                        bChangeOfListStyleAtParagraph = false;
                    }
                }
            }

            if ( bChangeOfListStyleAtParagraph )
            {
                std::unique_ptr< SwRegHistory > pRegH;
                if ( pPara->pHistory )
                {
                    pRegH.reset(new SwRegHistory(&rTNd, rTNd, pPara->pHistory));
                }

                pCNd->ResetAttr( RES_PARATR_NUMRULE );

                // reset all list attributes
                pCNd->ResetAttr( RES_PARATR_LIST_LEVEL );
                pCNd->ResetAttr( RES_PARATR_LIST_ISRESTART );
                pCNd->ResetAttr( RES_PARATR_LIST_RESTARTVALUE );
                pCNd->ResetAttr( RES_PARATR_LIST_ISCOUNTED );
                pCNd->ResetAttr( RES_PARATR_LIST_ID );
            }
            else
            {
                // The List Level must be applied as direct formatting. The spec says:
                // 19.495 The style:list-level attribute specifies the list level value
                // of a list style that may be applied to any paragraph style.
                // It does not directly specify the paragraph's list level value,
                // but consumers can change the paragraph's list level value to the specified value
                // when the paragraph style is applied.
                pCNd->SetAttr(pFormat->GetFormatAttr(RES_PARATR_LIST_LEVEL));
            }
        }
    }

    // add to History so that old data is saved, if necessary
    if( pPara->pHistory )
        pPara->pHistory->AddColl(pCNd->GetFormatColl(), pCNd->GetIndex(),
                SwNodeType::Text );

    pCNd->ChgFormatColl( pFormat );

    pPara->nWhich++;

    return true;
}

bool SwDoc::SetTextFormatColl(const SwPaM &rRg,
                          SwTextFormatColl *pFormat,
                          const bool bReset,
                          const bool bResetListAttrs,
                          SwRootFrame const*const pLayout)
{
    SwDataChanged aTmp( rRg );
    auto [pStt, pEnd] = rRg.StartEnd(); // SwPosition*
    SwHistory* pHst = nullptr;
    bool bRet = true;

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        std::unique_ptr<SwUndoFormatColl> pUndo(new SwUndoFormatColl( rRg, pFormat,
                                                  bReset,
                                                  bResetListAttrs ));
        pHst = pUndo->GetHistory();
        GetIDocumentUndoRedo().AppendUndo(std::move(pUndo));
    }

    sw::DocumentContentOperationsManager::ParaRstFormat aPara(
            pStt, pEnd, pHst, nullptr, pLayout);
    aPara.pFormatColl = pFormat;
    aPara.bReset = bReset;
    // #i62675#
    aPara.bResetListAttrs = bResetListAttrs;

    GetNodes().ForEach( pStt->GetNodeIndex(), pEnd->GetNodeIndex()+1,
                        lcl_SetTextFormatColl, &aPara );
    if( !aPara.nWhich )
        bRet = false;           // didn't find a valid Node

    if (bRet)
    {
        getIDocumentState().SetModified();
    }

    return bRet;
}

/// Copy the formats to itself
SwFormat* SwDoc::CopyFormat( const SwFormat& rFormat,
                        const SwFormatsBase& rFormatArr,
                        FNCopyFormat fnCopyFormat, const SwFormat& rDfltFormat )
{
    // It's no autoformat, default format or collection format,
    // then search for it.
    if( !rFormat.IsAuto() || !rFormat.GetRegisteredIn() )
        for( size_t n = 0; n < rFormatArr.GetFormatCount(); ++n )
        {
            // Does the Doc already contain the template?
            if( rFormatArr.GetFormat(n)->GetName()==rFormat.GetName() )
                return rFormatArr.GetFormat(n);
        }

    // Search for the "parent" first
    SwFormat* pParent = const_cast<SwFormat*>(&rDfltFormat);
    if( rFormat.DerivedFrom() && pParent != rFormat.DerivedFrom() )
        pParent = CopyFormat( *rFormat.DerivedFrom(), rFormatArr,
                                fnCopyFormat, rDfltFormat );

    // Create the format and copy the attributes
    // #i40550#
    SwFormat* pNewFormat = (this->*fnCopyFormat)( rFormat.GetName(), pParent, false, true );
    pNewFormat->SetAuto( rFormat.IsAuto() );
    pNewFormat->CopyAttrs( rFormat );           // copy the attributes

    pNewFormat->SetPoolFormatId( rFormat.GetPoolFormatId() );
    pNewFormat->SetPoolHelpId( rFormat.GetPoolHelpId() );

    // Always set the HelpFile Id to default!
    pNewFormat->SetPoolHlpFileId( UCHAR_MAX );

    return pNewFormat;
}

/// copy the frame format
SwFrameFormat* SwDoc::CopyFrameFormat( const SwFrameFormat& rFormat )
{
    return static_cast<SwFrameFormat*>(CopyFormat( rFormat, *GetFrameFormats(), &SwDoc::MakeFrameFormat_,
                                *GetDfltFrameFormat() ));
}

/// copy the char format
SwCharFormat* SwDoc::CopyCharFormat( const SwCharFormat& rFormat )
{
    return static_cast<SwCharFormat*>(CopyFormat( rFormat, *GetCharFormats(),
                                            &SwDoc::MakeCharFormat_,
                                            *GetDfltCharFormat() ));
}

/// copy TextNodes
SwTextFormatColl* SwDoc::CopyTextColl( const SwTextFormatColl& rColl )
{
    SwTextFormatColl* pNewColl = FindTextFormatCollByName( rColl.GetName() );
    if( pNewColl )
        return pNewColl;

    // search for the "parent" first
    SwTextFormatColl* pParent = mpDfltTextFormatColl.get();
    if( pParent != rColl.DerivedFrom() )
        pParent = CopyTextColl( *static_cast<SwTextFormatColl*>(rColl.DerivedFrom()) );

    if( RES_CONDTXTFMTCOLL == rColl.Which() )
    {
        pNewColl = new SwConditionTextFormatColl( GetAttrPool(), rColl.GetName(),
                                                pParent);
        mpTextFormatCollTable->push_back( pNewColl );
        pNewColl->SetAuto(false);
        getIDocumentState().SetModified();

        // copy the conditions
        static_cast<SwConditionTextFormatColl*>(pNewColl)->SetConditions(
                            static_cast<const SwConditionTextFormatColl&>(rColl).GetCondColls() );
    }
    else
        pNewColl = MakeTextFormatColl( rColl.GetName(), pParent );

    // copy the auto formats or the attributes
    pNewColl->CopyAttrs( rColl );

    if(rColl.IsAssignedToListLevelOfOutlineStyle())
        pNewColl->AssignToListLevelOfOutlineStyle(rColl.GetAssignedOutlineStyleLevel());
    pNewColl->SetPoolFormatId( rColl.GetPoolFormatId() );
    pNewColl->SetPoolHelpId( rColl.GetPoolHelpId() );

    // Always set the HelpFile Id to default!
    pNewColl->SetPoolHlpFileId( UCHAR_MAX );

    if( &rColl.GetNextTextFormatColl() != &rColl )
        pNewColl->SetNextTextFormatColl( *CopyTextColl( rColl.GetNextTextFormatColl() ));

    // create the NumRule if necessary
    if( this != rColl.GetDoc() )
    {
        const SwNumRuleItem* pItem = pNewColl->GetItemIfSet( RES_PARATR_NUMRULE,
            false );
        if( pItem )
        {
            const OUString& rName = pItem->GetValue();
            if( !rName.isEmpty() )
            {
                const SwNumRule* pRule = rColl.GetDoc()->FindNumRulePtr( rName );
                if( pRule && !pRule->IsAutoRule() )
                {
                    SwNumRule* pDestRule = FindNumRulePtr( rName );
                    if( pDestRule )
                        pDestRule->Invalidate();
                    else
                        MakeNumRule( rName, pRule );
                }
            }
        }
    }
    return pNewColl;
}

/// copy the graphic nodes
SwGrfFormatColl* SwDoc::CopyGrfColl( const SwGrfFormatColl& rColl )
{
    SwGrfFormatColl* pNewColl = mpGrfFormatCollTable->FindFormatByName( rColl.GetName() );
    if( pNewColl )
        return pNewColl;

     // Search for the "parent" first
    SwGrfFormatColl* pParent = mpDfltGrfFormatColl.get();
    if( pParent != rColl.DerivedFrom() )
        pParent = CopyGrfColl( *static_cast<SwGrfFormatColl*>(rColl.DerivedFrom()) );

    // if not, copy them
    pNewColl = MakeGrfFormatColl( rColl.GetName(), pParent );

    // copy the attributes
    pNewColl->CopyAttrs( rColl );

    pNewColl->SetPoolFormatId( rColl.GetPoolFormatId() );
    pNewColl->SetPoolHelpId( rColl.GetPoolHelpId() );

    // Always set the HelpFile Id to default!
    pNewColl->SetPoolHlpFileId( UCHAR_MAX );

    return pNewColl;
}

void SwDoc::CopyFormatArr( const SwFormatsBase& rSourceArr,
                        SwFormatsBase const & rDestArr,
                        FNCopyFormat fnCopyFormat,
                        SwFormat& rDfltFormat )
{
    SwFormat* pSrc, *pDest;

    // 1st step: Create all formats (skip the 0th - it's the default one)
    for( size_t nSrc = rSourceArr.GetFormatCount(); nSrc > 1; )
    {
        pSrc = rSourceArr.GetFormat( --nSrc );
        if( pSrc->IsDefault() || pSrc->IsAuto() )
            continue;

        if( nullptr == rDestArr.FindFormatByName( pSrc->GetName() ) )
        {
            if( RES_CONDTXTFMTCOLL == pSrc->Which() )
                MakeCondTextFormatColl( pSrc->GetName(), static_cast<SwTextFormatColl*>(&rDfltFormat) );
            else
                // #i40550#
                (this->*fnCopyFormat)( pSrc->GetName(), &rDfltFormat, false, true );
        }
    }

    // 2nd step: Copy all attributes, set the right parents
    for( size_t nSrc = rSourceArr.GetFormatCount(); nSrc > 1; )
    {
        pSrc = rSourceArr.GetFormat( --nSrc );
        if( pSrc->IsDefault() || pSrc->IsAuto() )
            continue;

        pDest = rDestArr.FindFormatByName( pSrc->GetName() );
        pDest->SetAuto(false);
        pDest->DelDiffs( *pSrc );

        // #i94285#: existing <SwFormatPageDesc> instance, before copying attributes
        const SwFormatPageDesc* pItem;
        if( &GetAttrPool() != pSrc->GetAttrSet().GetPool()
                && (pItem = pSrc->GetAttrSet().GetItemIfSet( RES_PAGEDESC, false ))
                && pItem->GetPageDesc() )
        {
            SwFormatPageDesc aPageDesc( *pItem );
            const OUString& rNm = aPageDesc.GetPageDesc()->GetName();
            SwPageDesc* pPageDesc = FindPageDesc( rNm );
            if( !pPageDesc )
            {
                pPageDesc = MakePageDesc(rNm);
            }
            aPageDesc.RegisterToPageDesc( *pPageDesc );
            SwAttrSet aTmpAttrSet( pSrc->GetAttrSet() );
            aTmpAttrSet.Put( aPageDesc );
            pDest->SetFormatAttr( aTmpAttrSet );
        }
        else
        {
            pDest->SetFormatAttr( pSrc->GetAttrSet() );
        }

        pDest->SetPoolFormatId( pSrc->GetPoolFormatId() );
        pDest->SetPoolHelpId( pSrc->GetPoolHelpId() );

        // Always set the HelpFile Id to default!
        pDest->SetPoolHlpFileId( UCHAR_MAX );

        if( pSrc->DerivedFrom() )
            pDest->SetDerivedFrom( rDestArr.FindFormatByName(
                                        pSrc->DerivedFrom()->GetName() ) );
        if( RES_TXTFMTCOLL == pSrc->Which() ||
            RES_CONDTXTFMTCOLL == pSrc->Which() )
        {
            SwTextFormatColl* pSrcColl = static_cast<SwTextFormatColl*>(pSrc),
                        * pDstColl = static_cast<SwTextFormatColl*>(pDest);
            if( &pSrcColl->GetNextTextFormatColl() != pSrcColl )
                pDstColl->SetNextTextFormatColl(
                    *static_cast<SwTextFormatColl*>(rDestArr.FindFormatByName( pSrcColl->GetNextTextFormatColl().GetName() )) );

            if(pSrcColl->IsAssignedToListLevelOfOutlineStyle())
                pDstColl->AssignToListLevelOfOutlineStyle(pSrcColl->GetAssignedOutlineStyleLevel());

            if( RES_CONDTXTFMTCOLL == pSrc->Which() )
            {
                if (pDstColl->Which() != RES_CONDTXTFMTCOLL)
                {
                    // Target already had a style with a matching name, but it's not a conditional
                    // style, then don't copy the conditions.
                    continue;
                }

                // Copy the conditions, but delete the old ones first!
                static_cast<SwConditionTextFormatColl*>(pDstColl)->SetConditions(
                            static_cast<SwConditionTextFormatColl*>(pSrc)->GetCondColls() );
            }
        }
    }
}

void SwDoc::CopyPageDescHeaderFooterImpl( bool bCpyHeader,
                                const SwFrameFormat& rSrcFormat, SwFrameFormat& rDestFormat )
{
    // Treat the header and footer attributes in the right way:
    // Copy content nodes across documents!
    sal_uInt16 nAttr = bCpyHeader ? sal_uInt16(RES_HEADER) : sal_uInt16(RES_FOOTER);
    const SfxPoolItem* pItem;
    if( SfxItemState::SET != rSrcFormat.GetAttrSet().GetItemState( nAttr, false, &pItem ))
        return ;

    // The header only contains the reference to the format from the other document!
    std::unique_ptr<SfxPoolItem> pNewItem(pItem->Clone());

    SwFrameFormat* pOldFormat;
    if( bCpyHeader )
         pOldFormat = pNewItem->StaticWhichCast(RES_HEADER).GetHeaderFormat();
    else
         pOldFormat = pNewItem->StaticWhichCast(RES_FOOTER).GetFooterFormat();

    if( !pOldFormat )
        return;

    SwFrameFormat* pNewFormat = new SwFrameFormat( GetAttrPool(), "CpyDesc",
                                        GetDfltFrameFormat() );
    pNewFormat->CopyAttrs( *pOldFormat );

    if( const SwFormatContent* pContent = pNewFormat->GetAttrSet().GetItemIfSet(
        RES_CNTNT, false ) )
    {
        if( pContent->GetContentIdx() )
        {
            const SwNodes& rSrcNds = rSrcFormat.GetDoc()->GetNodes();
            SwStartNode* pSttNd = SwNodes::MakeEmptySection( GetNodes().GetEndOfAutotext(),
                                            bCpyHeader
                                                ? SwHeaderStartNode
                                                : SwFooterStartNode );
            const SwNode& rCSttNd = pContent->GetContentIdx()->GetNode();
            SwNodeRange aRg( rCSttNd, SwNodeOffset(0), *rCSttNd.EndOfSectionNode() );
            rSrcNds.Copy_( aRg, *pSttNd->EndOfSectionNode() );
            rSrcFormat.GetDoc()->GetDocumentContentOperationsManager().CopyFlyInFlyImpl(aRg, nullptr, *pSttNd);
            // TODO: investigate calling CopyWithFlyInFly?
            SwPaM const source(aRg.aStart, aRg.aEnd);
            SwPosition dest(*pSttNd);
            sw::CopyBookmarks(source, dest);
            pNewFormat->SetFormatAttr( SwFormatContent( pSttNd ));
        }
        else
            pNewFormat->ResetFormatAttr( RES_CNTNT );
    }
    if( bCpyHeader )
        pNewItem->StaticWhichCast(RES_HEADER).RegisterToFormat(*pNewFormat);
    else
        pNewItem->StaticWhichCast(RES_FOOTER).RegisterToFormat(*pNewFormat);
    rDestFormat.SetFormatAttr( *pNewItem );
}

void SwDoc::CopyPageDesc( const SwPageDesc& rSrcDesc, SwPageDesc& rDstDesc,
                            bool bCopyPoolIds )
{
    bool bNotifyLayout = false;
    SwRootFrame* pTmpRoot = getIDocumentLayoutAccess().GetCurrentLayout();

    rDstDesc.SetLandscape( rSrcDesc.GetLandscape() );
    rDstDesc.SetNumType( rSrcDesc.GetNumType() );
    if( rDstDesc.ReadUseOn() != rSrcDesc.ReadUseOn() )
    {
        rDstDesc.WriteUseOn( rSrcDesc.ReadUseOn() );
        bNotifyLayout = true;
    }

    if( bCopyPoolIds )
    {
        rDstDesc.SetPoolFormatId( rSrcDesc.GetPoolFormatId() );
        rDstDesc.SetPoolHelpId( rSrcDesc.GetPoolHelpId() );
        // Always set the HelpFile Id to default!
        rDstDesc.SetPoolHlpFileId( UCHAR_MAX );
    }

    if( rSrcDesc.GetFollow() != &rSrcDesc )
    {
        const SwPageDesc* pSrcFollow = rSrcDesc.GetFollow();
        SwPageDesc* pFollow = FindPageDesc( pSrcFollow->GetName() );
        if( !pFollow )
        {
            // copy
            pFollow = MakePageDesc( pSrcFollow->GetName() );
            CopyPageDesc( *pSrcFollow, *pFollow );
        }
        rDstDesc.SetFollow( pFollow );
        bNotifyLayout = true;
    }

    // the header and footer attributes are copied separately
    // the content sections have to be copied in their entirety
    {
        SfxItemSet aAttrSet( rSrcDesc.GetMaster().GetAttrSet() );
        aAttrSet.ClearItem( RES_HEADER );
        aAttrSet.ClearItem( RES_FOOTER );

        rDstDesc.GetMaster().DelDiffs( aAttrSet );
        rDstDesc.GetMaster().SetFormatAttr( aAttrSet );

        aAttrSet.ClearItem();
        aAttrSet.Put( rSrcDesc.GetLeft().GetAttrSet() );
        aAttrSet.ClearItem( RES_HEADER );
        aAttrSet.ClearItem( RES_FOOTER );

        rDstDesc.GetLeft().DelDiffs( aAttrSet );
        rDstDesc.GetLeft().SetFormatAttr( aAttrSet );

        aAttrSet.ClearItem();
        aAttrSet.Put( rSrcDesc.GetFirstMaster().GetAttrSet() );
        aAttrSet.ClearItem( RES_HEADER );
        aAttrSet.ClearItem( RES_FOOTER );

        rDstDesc.GetFirstMaster().DelDiffs( aAttrSet );
        rDstDesc.GetFirstMaster().SetFormatAttr( aAttrSet );

        aAttrSet.ClearItem();
        aAttrSet.Put( rSrcDesc.GetFirstLeft().GetAttrSet() );
        aAttrSet.ClearItem( RES_HEADER );
        aAttrSet.ClearItem( RES_FOOTER );

        rDstDesc.GetFirstLeft().DelDiffs( aAttrSet );
        rDstDesc.GetFirstLeft().SetFormatAttr( aAttrSet );
    }

    CopyHeader( rSrcDesc.GetMaster(), rDstDesc.GetMaster() );
    CopyFooter( rSrcDesc.GetMaster(), rDstDesc.GetMaster() );
    if( !rDstDesc.IsHeaderShared() )
        CopyHeader( rSrcDesc.GetLeft(), rDstDesc.GetLeft() );
    else
        rDstDesc.GetLeft().SetFormatAttr( rDstDesc.GetMaster().GetHeader() );
    if( !rDstDesc.IsFirstShared() )
    {
        CopyHeader( rSrcDesc.GetFirstMaster(), rDstDesc.GetFirstMaster() );
        rDstDesc.GetFirstLeft().SetFormatAttr(rDstDesc.GetFirstMaster().GetHeader());
    }
    else
    {
        rDstDesc.GetFirstMaster().SetFormatAttr( rDstDesc.GetMaster().GetHeader() );
        rDstDesc.GetFirstLeft().SetFormatAttr(rDstDesc.GetLeft().GetHeader());
    }

    if( !rDstDesc.IsFooterShared() )
        CopyFooter( rSrcDesc.GetLeft(), rDstDesc.GetLeft() );
    else
        rDstDesc.GetLeft().SetFormatAttr( rDstDesc.GetMaster().GetFooter() );
    if( !rDstDesc.IsFirstShared() )
    {
        CopyFooter( rSrcDesc.GetFirstMaster(), rDstDesc.GetFirstMaster() );
        rDstDesc.GetFirstLeft().SetFormatAttr(rDstDesc.GetFirstMaster().GetFooter());
    }
    else
    {
        rDstDesc.GetFirstMaster().SetFormatAttr( rDstDesc.GetMaster().GetFooter() );
        rDstDesc.GetFirstLeft().SetFormatAttr(rDstDesc.GetLeft().GetFooter());
    }

    if( bNotifyLayout && pTmpRoot )
    {
        for( auto aLayout : GetAllLayouts() )
            aLayout->AllCheckPageDescs();
    }

    // If foot notes change the pages have to be triggered
    if( !(rDstDesc.GetFootnoteInfo() == rSrcDesc.GetFootnoteInfo()) )
    {
        sw::PageFootnoteHint aHint;
        rDstDesc.SetFootnoteInfo( rSrcDesc.GetFootnoteInfo() );
        rDstDesc.GetMaster().CallSwClientNotify(aHint);
        rDstDesc.GetLeft().CallSwClientNotify(aHint);
        rDstDesc.GetFirstMaster().CallSwClientNotify(aHint);
        rDstDesc.GetFirstLeft().CallSwClientNotify(aHint);
    }

    // Copy the stashed formats as well between the page descriptors...
    for (bool bFirst : { true, false })
    {
        for (bool bLeft : { true, false })
        {
            for (bool bHeader : { true, false })
            {
                if (!bLeft && !bFirst)
                    continue;

                // Copy format only if it exists
                if (auto pStashedFormatSrc = rSrcDesc.GetStashedFrameFormat(bHeader, bLeft, bFirst))
                {
                    if (pStashedFormatSrc->GetDoc() != this)
                    {
                        SwFrameFormat newFormat(GetAttrPool(), "CopyDesc", GetDfltFrameFormat());

                        SfxItemSet aAttrSet(pStashedFormatSrc->GetAttrSet());
                        aAttrSet.ClearItem(RES_HEADER);
                        aAttrSet.ClearItem(RES_FOOTER);

                        newFormat.DelDiffs(aAttrSet);
                        newFormat.SetFormatAttr(aAttrSet);

                        if (bHeader)
                            CopyHeader(*pStashedFormatSrc, newFormat);
                        else
                            CopyFooter(*pStashedFormatSrc, newFormat);

                        rDstDesc.StashFrameFormat(newFormat, bHeader, bLeft, bFirst);
                    }
                    else
                    {
                        rDstDesc.StashFrameFormat(*pStashedFormatSrc, bHeader, bLeft, bFirst);
                    }
                }
            }
        }
    }
}

void SwDoc::ReplaceStyles( const SwDoc& rSource, bool bIncludePageStyles )
{
    ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

    CopyFormatArr( *rSource.mpCharFormatTable, *mpCharFormatTable,
                &SwDoc::MakeCharFormat_, *mpDfltCharFormat );
    CopyFormatArr( *rSource.mpFrameFormatTable, *mpFrameFormatTable,
                &SwDoc::MakeFrameFormat_, *mpDfltFrameFormat );
    CopyFormatArr( *rSource.mpTextFormatCollTable, *mpTextFormatCollTable,
                &SwDoc::MakeTextFormatColl_, *mpDfltTextFormatColl );

    //To-Do:
    //  a) in sd rtf import (View::InsertData) don't use
    //  a super-fragile test for mere presence of \trowd to
    //  indicate import of rtf into a table
    //  b) then drop use of bIncludePageStyles
    if (bIncludePageStyles)
    {
        // and now the page templates
        SwPageDescs::size_type nCnt = rSource.m_PageDescs.size();
        if( nCnt )
        {
            // a different Doc -> Number formatter needs to be merged
            SwTableNumFormatMerge aTNFM( rSource, *this );

            // 1st step: Create all formats (skip the 0th - it's the default!)
            while( nCnt )
            {
                const SwPageDesc &rSrc = *rSource.m_PageDescs[ --nCnt ];
                if( nullptr == FindPageDesc( rSrc.GetName() ) )
                    MakePageDesc( rSrc.GetName() );
            }

            // 2nd step: Copy all attributes, set the right parents
            for (SwPageDescs::size_type i = rSource.m_PageDescs.size(); i; )
            {
                const SwPageDesc &rSrc = *rSource.m_PageDescs[ --i ];
                SwPageDesc* pDesc = FindPageDesc( rSrc.GetName() );
                CopyPageDesc( rSrc, *pDesc);
            }
        }
    }

    // then there are the numbering templates
    const SwNumRuleTable::size_type nCnt = rSource.GetNumRuleTable().size();
    if( nCnt )
    {
        const SwNumRuleTable& rArr = rSource.GetNumRuleTable();
        for( SwNumRuleTable::size_type n = 0; n < nCnt; ++n )
        {
            const SwNumRule& rR = *rArr[ n ];
            SwNumRule* pNew = FindNumRulePtr( rR.GetName());
            if( pNew )
                pNew->CopyNumRule(*this, rR);
            else
            {
                if( !rR.IsAutoRule() )
                    MakeNumRule( rR.GetName(), &rR );
                else
                {
                    // as we reset all styles, there shouldn't be any unknown
                    // automatic SwNumRules, because all should have been
                    // created by the style copying!
                    // So just warn and ignore.
                    SAL_WARN( "sw.core", "Found unknown auto SwNumRule during reset!" );
                }
            }
        }
    }

    if (undoGuard.UndoWasEnabled())
    {
        // nodes array was modified!
        GetIDocumentUndoRedo().DelAllUndoObj();
    }

    getIDocumentState().SetModified();
}

void SwDoc::MoveLeftMargin(const SwPaM& rPam, bool bRight, bool bModulus,
        SwRootFrame const*const pLayout)
{
    SwHistory* pHistory = nullptr;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        std::unique_ptr<SwUndoMoveLeftMargin> pUndo(new SwUndoMoveLeftMargin( rPam, bRight,
                                                                bModulus ));
        pHistory = &pUndo->GetHistory();
        GetIDocumentUndoRedo().AppendUndo( std::move(pUndo) );
    }

    const SvxTabStopItem& rTabItem = GetDefault( RES_PARATR_TABSTOP );
    const sal_Int32 nDefDist = rTabItem.Count() ? rTabItem[0].GetTabPos() : 1134;
    const SwPosition &rStt = *rPam.Start(), &rEnd = *rPam.End();
    SwNodeIndex aIdx( rStt.GetNode() );
    while( aIdx <= rEnd.GetNode() )
    {
        SwTextNode* pTNd = aIdx.GetNode().GetTextNode();
        if( pTNd )
        {
            pTNd = sw::GetParaPropsNode(*pLayout, aIdx.GetNode());
            SvxFirstLineIndentItem firstLine(pTNd->SwContentNode::GetAttr(RES_MARGIN_FIRSTLINE));
            SvxTextLeftMarginItem leftMargin(pTNd->SwContentNode::GetAttr(RES_MARGIN_TEXTLEFT));

            // #i93873# See also lcl_MergeListLevelIndentAsLRSpaceItem in thints.cxx
            ::sw::ListLevelIndents const indents(pTNd->AreListLevelIndentsApplicable());
            if (indents != ::sw::ListLevelIndents::No)
            {
                const SwNumRule* pRule = pTNd->GetNumRule();
                if ( pRule )
                {
                    const int nListLevel = pTNd->GetActualListLevel();
                    if ( nListLevel >= 0 )
                    {
                        const SwNumFormat& rFormat = pRule->Get(o3tl::narrowing<sal_uInt16>(nListLevel));
                        if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
                        {
                            if (indents & ::sw::ListLevelIndents::LeftMargin)
                            {
                                leftMargin.SetTextLeft(rFormat.GetIndentAt());
                            }
                            if (indents & ::sw::ListLevelIndents::FirstLine)
                            {
                                firstLine.SetTextFirstLineOffset(static_cast<short>(rFormat.GetFirstLineIndent()));
                            }
                        }
                    }
                }
            }

            tools::Long nNext = leftMargin.GetTextLeft();
            if( bModulus )
                nNext = ( nNext / nDefDist ) * nDefDist;

            if( bRight )
                nNext += nDefDist;
            else
                if(nNext >0) // fdo#75936 set limit for decreasing indent
                    nNext -= nDefDist;

            leftMargin.SetTextLeft( nNext );

            SwRegHistory aRegH( pTNd, *pTNd, pHistory );
            pTNd->SetAttr(firstLine);
            pTNd->SetAttr(leftMargin);
            aIdx = *sw::GetFirstAndLastNode(*pLayout, aIdx.GetNode()).second;
        }
        ++aIdx;
    }
    getIDocumentState().SetModified();
}

bool SwDoc::DontExpandFormat( const SwPosition& rPos, bool bFlag )
{
    bool bRet = false;
    SwTextNode* pTextNd = rPos.GetNode().GetTextNode();
    if( pTextNd )
    {
        bRet = pTextNd->DontExpandFormat( rPos.GetContentIndex(), bFlag );
        if( bRet && GetIDocumentUndoRedo().DoesUndo() )
        {
            GetIDocumentUndoRedo().AppendUndo( std::make_unique<SwUndoDontExpandFormat>(rPos) );
        }
    }
    return bRet;
}

SwTableBoxFormat* SwDoc::MakeTableBoxFormat()
{
    SwTableBoxFormat* pFormat = new SwTableBoxFormat( GetAttrPool(), mpDfltFrameFormat.get() );
    pFormat->SetFormatName("TableBox" + OUString::number(reinterpret_cast<sal_IntPtr>(pFormat)));
    getIDocumentState().SetModified();
    return pFormat;
}

SwTableLineFormat* SwDoc::MakeTableLineFormat()
{
    SwTableLineFormat* pFormat = new SwTableLineFormat( GetAttrPool(), mpDfltFrameFormat.get() );
    pFormat->SetFormatName("TableLine" + OUString::number(reinterpret_cast<sal_IntPtr>(pFormat)));
    getIDocumentState().SetModified();
    return pFormat;
}

void SwDoc::EnsureNumberFormatter()
{
    if (mpNumberFormatter == nullptr)
    {
        LanguageType eLang = LANGUAGE_SYSTEM;
        mpNumberFormatter = new SvNumberFormatter(comphelper::getProcessComponentContext(), eLang);
        mpNumberFormatter->SetEvalDateFormat( NF_EVALDATEFORMAT_FORMAT_INTL );
        if (!comphelper::IsFuzzing())
            mpNumberFormatter->SetYear2000(
                officecfg::Office::Common::DateFormat::TwoDigitYear::get());
    };
}

SwTableNumFormatMerge::SwTableNumFormatMerge( const SwDoc& rSrc, SwDoc& rDest )
    : pNFormat( nullptr )
{
    // a different Doc -> Number formatter needs to be merged
    if( &rSrc != &rDest )
    {
        SvNumberFormatter* pN = const_cast<SwDoc&>(rSrc).GetNumberFormatter( false );
        if( pN )
        {
            pNFormat = rDest.GetNumberFormatter();
            pNFormat->MergeFormatter( *pN );
        }
    }

    if( &rSrc != &rDest )
        static_cast<SwGetRefFieldType*>(rSrc.getIDocumentFieldsAccess().GetSysFieldType( SwFieldIds::GetRef ))->
            MergeWithOtherDoc( rDest );
}

SwTableNumFormatMerge::~SwTableNumFormatMerge()
{
    if( pNFormat )
        pNFormat->ClearMergeTable();
}

void SwDoc::SetTextFormatCollByAutoFormat( const SwPosition& rPos, sal_uInt16 nPoolId,
                                    const SfxItemSet* pSet )
{
    SwPaM aPam( rPos );
    SwTextNode* pTNd = rPos.GetNode().GetTextNode();
    assert(pTNd);

    if (mbIsAutoFormatRedline)
    {
        // create the redline object
        const SwTextFormatColl& rColl = *pTNd->GetTextColl();
        SwRangeRedline* pRedl = new SwRangeRedline( RedlineType::FmtColl, aPam );
        pRedl->SetMark();

        // Only those items that are not set by the Set again in the Node
        // are of interest. Thus, we take the difference.
        SwRedlineExtraData_FormatColl aExtraData( rColl.GetName(),
                                                rColl.GetPoolFormatId() );
        if( pSet && pTNd->HasSwAttrSet() )
        {
            SfxItemSet aTmp( *pTNd->GetpSwAttrSet() );
            aTmp.Differentiate( *pSet );
            // we handle the adjust item separately
            const SfxPoolItem* pItem;
            if( SfxItemState::SET == pTNd->GetpSwAttrSet()->GetItemState(
                    RES_PARATR_ADJUST, false, &pItem ))
                aTmp.Put( *pItem );
            aExtraData.SetItemSet( aTmp );
        }
        pRedl->SetExtraData( &aExtraData );

        //TODO: Undo is still missing!
        getIDocumentRedlineAccess().AppendRedline( pRedl, true );
    }

    SetTextFormatColl( aPam, getIDocumentStylePoolAccess().GetTextCollFromPool( nPoolId ) );

    if (pSet && pSet->Count())
    {
        aPam.SetMark();
        aPam.GetMark()->SetContent(pTNd->GetText().getLength());
        // sw_redlinehide: don't need layout currently because the only caller
        // passes in the properties node
        assert(static_cast<SwTextFrame const*>(pTNd->getLayoutFrame(nullptr))->GetTextNodeForParaProps() == pTNd);
        getIDocumentContentOperations().InsertItemSet( aPam, *pSet );
    }
}

void SwDoc::SetFormatItemByAutoFormat( const SwPaM& rPam, const SfxItemSet& rSet )
{
    SwTextNode* pTNd = rPam.GetPoint()->GetNode().GetTextNode();
    assert(pTNd);

    RedlineFlags eOld = getIDocumentRedlineAccess().GetRedlineFlags();

    if (mbIsAutoFormatRedline)
    {
        // create the redline object
        SwRangeRedline* pRedl = new SwRangeRedline( RedlineType::Format, rPam );
        if( !pRedl->HasMark() )
            pRedl->SetMark();

        // Only those items that are not set by the Set again in the Node
        // are of interest. Thus, we take the difference.
        SwRedlineExtraData_Format aExtraData( rSet );

        pRedl->SetExtraData( &aExtraData );

        //TODO: Undo is still missing!
        getIDocumentRedlineAccess().AppendRedline( pRedl, true );

        getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld | RedlineFlags::Ignore );
    }

    const sal_Int32 nEnd(rPam.End()->GetContentIndex());
    std::vector<WhichPair> whichIds;
    SfxItemIter iter(rSet);
    for (SfxPoolItem const* pItem = iter.GetCurItem(); pItem; pItem = iter.NextItem())
    {
        whichIds.push_back({pItem->Which(), pItem->Which()});
    }
    SfxItemSet currentSet(GetAttrPool(), WhichRangesContainer(whichIds.data(), whichIds.size()));
    pTNd->GetParaAttr(currentSet, nEnd, nEnd);
    for (const WhichPair& rPair : whichIds)
    {   // yuk - want to explicitly set the pool defaults too :-/
        currentSet.Put(currentSet.Get(rPair.first));
    }

    getIDocumentContentOperations().InsertItemSet( rPam, rSet, SetAttrMode::DONTEXPAND );

    // fdo#62536: DONTEXPAND does not work when there is already an AUTOFMT
    // here, so insert the old attributes as an empty hint to stop expand
    SwPaM endPam(*pTNd, nEnd);
    endPam.SetMark();
    getIDocumentContentOperations().InsertItemSet(endPam, currentSet);

    getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
}

void SwDoc::ChgFormat(SwFormat & rFormat, const SfxItemSet & rSet)
{
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        // copying <rSet> to <aSet>
        SfxItemSet aSet(rSet);
        // remove from <aSet> all items, which are already set at the format
        aSet.Differentiate(rFormat.GetAttrSet());
        // <aSet> contains now all *new* items for the format

        // copying current format item set to <aOldSet>
        SfxItemSet aOldSet(rFormat.GetAttrSet());
        // insert new items into <aOldSet>
        aOldSet.Put(aSet);
        // invalidate all new items in <aOldSet> in order to clear these items,
        // if the undo action is triggered.
        {
            SfxItemIter aIter(aSet);

            for (const SfxPoolItem* pItem = aIter.GetCurItem(); pItem; pItem = aIter.NextItem())
            {
                aOldSet.InvalidateItem(pItem->Which());
            }
        }

        GetIDocumentUndoRedo().AppendUndo(
            std::make_unique<SwUndoFormatAttr>(std::move(aOldSet), rFormat, /*bSaveDrawPt*/true));
    }

    rFormat.SetFormatAttr(rSet);
}

void SwDoc::RenameFormat(SwFormat & rFormat, const OUString & sNewName,
                      bool bBroadcast)
{
    SfxStyleFamily eFamily = SfxStyleFamily::All;

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        std::unique_ptr<SwUndo> pUndo;

        switch (rFormat.Which())
        {
        case RES_CHRFMT:
            pUndo.reset(new SwUndoRenameCharFormat(rFormat.GetName(), sNewName, *this));
            eFamily = SfxStyleFamily::Char;
            break;
        case RES_TXTFMTCOLL:
            pUndo.reset(new SwUndoRenameFormatColl(rFormat.GetName(), sNewName, *this));
            eFamily = SfxStyleFamily::Para;
            break;
        case RES_FRMFMT:
            pUndo.reset(new SwUndoRenameFrameFormat(rFormat.GetName(), sNewName, *this));
            eFamily = SfxStyleFamily::Frame;
            break;

        default:
            break;
        }

        if (pUndo)
        {
            GetIDocumentUndoRedo().AppendUndo(std::move(pUndo));
        }
    }

    // name change means the o3tl::sorted_array is not property sorted
    if (rFormat.Which() == RES_CHRFMT)
        mpCharFormatTable->SetFormatNameAndReindex(static_cast<SwCharFormat*>(&rFormat), sNewName);
    else
        rFormat.SetFormatName(sNewName);

    if (bBroadcast)
        BroadcastStyleOperation(sNewName, eFamily, SfxHintId::StyleSheetModified);
}

void SwDoc::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    bool bOwns = false;
    if (!pWriter)
    {
        pWriter = xmlNewTextWriterFilename("nodes.xml", 0);
        xmlTextWriterSetIndent(pWriter,1);
        (void)xmlTextWriterSetIndentString(pWriter, BAD_CAST("  "));
        (void)xmlTextWriterStartDocument(pWriter, nullptr, nullptr, nullptr);
        bOwns = true;
    }
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwDoc"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);

    m_pNodes->dumpAsXml(pWriter);
    m_PageDescs.dumpAsXml(pWriter);
    maDBData.dumpAsXml(pWriter);
    mpMarkManager->dumpAsXml(pWriter);
    m_pContentControlManager->dumpAsXml(pWriter);
    m_pUndoManager->dumpAsXml(pWriter);
    m_pDocumentSettingManager->dumpAsXml(pWriter);
    getIDocumentFieldsAccess().GetFieldTypes()->dumpAsXml(pWriter);
    mpTextFormatCollTable->dumpAsXml(pWriter);
    mpCharFormatTable->dumpAsXml(pWriter);
    mpFrameFormatTable->dumpAsXml(pWriter, "frmFormatTable");
    mpSpzFrameFormatTable->dumpAsXml(pWriter, "spzFrameFormatTable");
    mpSectionFormatTable->dumpAsXml(pWriter);
    mpTableFrameFormatTable->dumpAsXml(pWriter, "tableFrameFormatTable");
    mpNumRuleTable->dumpAsXml(pWriter);
    getIDocumentRedlineAccess().GetRedlineTable().dumpAsXml(pWriter);
    getIDocumentRedlineAccess().GetExtraRedlineTable().dumpAsXml(pWriter);
    if (const SdrModel* pModel = getIDocumentDrawModelAccess().GetDrawModel())
        pModel->dumpAsXml(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbModified"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(OString::boolean(getIDocumentState().IsModified()).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterEndElement(pWriter);
    if (bOwns)
    {
        (void)xmlTextWriterEndDocument(pWriter);
        xmlFreeTextWriter(pWriter);
    }
}

void SwDBData::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwDBData"));

    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("sDataSource"), BAD_CAST(sDataSource.toUtf8().getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("sCommand"), BAD_CAST(sCommand.toUtf8().getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nCommandType"), BAD_CAST(OString::number(nCommandType).getStr()));

    (void)xmlTextWriterEndElement(pWriter);
}

std::set<Color> SwDoc::GetDocColors()
{
    std::set<Color> aDocColors;
    SwAttrPool& rPool = GetAttrPool();

    svx::DocumentColorHelper::queryColors<SvxColorItem>(RES_CHRATR_COLOR, &rPool, aDocColors);
    svx::DocumentColorHelper::queryColors<SvxBrushItem>(RES_CHRATR_HIGHLIGHT, &rPool, aDocColors);
    svx::DocumentColorHelper::queryColors<SvxBrushItem>(RES_CHRATR_BACKGROUND, &rPool, aDocColors);

    return aDocColors;
}

// #i69627#
namespace docfunc
{
    bool HasOutlineStyleToBeWrittenAsNormalListStyle( SwDoc& rDoc )
    {
        // If a parent paragraph style of one of the paragraph styles, which
        // are assigned to the list levels of the outline style, has a list style
        // set or inherits a list style from its parent style, the outline style
        // has to be written as a normal list style to the OpenDocument file
        // format or the OpenOffice.org file format.
        bool bRet( false );

        const SwTextFormatColls* pTextFormatColls( rDoc.GetTextFormatColls() );
        if ( pTextFormatColls )
        {
            for ( auto pTextFormatColl : *pTextFormatColls )
            {
                if ( pTextFormatColl->IsDefault() ||
                    ! pTextFormatColl->IsAssignedToListLevelOfOutlineStyle() )
                {
                    continue;
                }

                const SwTextFormatColl* pParentTextFormatColl =
                   dynamic_cast<const SwTextFormatColl*>( pTextFormatColl->DerivedFrom());
                if ( !pParentTextFormatColl )
                    continue;

                if ( SfxItemState::SET == pParentTextFormatColl->GetItemState( RES_PARATR_NUMRULE ) )
                {
                    // #i106218# consider that the outline style is set
                    const SwNumRuleItem& rDirectItem = pParentTextFormatColl->GetNumRule();
                    if ( rDirectItem.GetValue() != rDoc.GetOutlineNumRule()->GetName() )
                    {
                        bRet = true;
                        break;
                    }
                }
            }

        }
        return bRet;
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
