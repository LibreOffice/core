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
#include <sfx2/app.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/rsiditem.hxx>
#include <editeng/colritem.hxx>
#include <svl/whiter.hxx>
#include <svl/zforlist.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/misccfg.hxx>
#include <com/sun/star/i18n/WordType.hpp>
#include <fmtpdsc.hxx>
#include <fmthdft.hxx>
#include <fmtcntnt.hxx>
#include <frmatr.hxx>
#include <doc.hxx>
#include <docfunc.hxx>
#include <drawdoc.hxx>
#include <MarkManager.hxx>
#include <IDocumentUndoRedo.hxx>
#include <DocumentContentOperationsManager.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <hints.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <UndoCore.hxx>
#include <UndoAttribute.hxx>
#include <UndoInsert.hxx>
#include <ndgrf.hxx>
#include <pagedesc.hxx>
#include <rolbck.hxx>
#include <mvsave.hxx>
#include <txatbase.hxx>
#include <swtable.hxx>
#include <swtblfmt.hxx>
#include <charfmt.hxx>
#include <docary.hxx>
#include <paratr.hxx>
#include <redline.hxx>
#include <reffld.hxx>
#include <txtinet.hxx>
#include <fmtinfmt.hxx>
#include <breakit.hxx>
#include <SwStyleNameMapper.hxx>
#include <fmtautofmt.hxx>
#include <istyleaccess.hxx>
#include <SwUndoFmt.hxx>
#include <UndoManager.hxx>
#include <docsh.hxx>
#include <swmodule.hxx>
#include <modcfg.hxx>
#include <memory>

using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
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

static bool lcl_RstAttr( const SwNodePtr& rpNd, void* pArgs )
{
    const sw::DocumentContentOperationsManager::ParaRstFormat* pPara = static_cast<sw::DocumentContentOperationsManager::ParaRstFormat*>(pArgs);
    SwContentNode* pNode = rpNd->GetContentNode();
    if( pNode && pNode->HasSwAttrSet() )
    {
        const bool bLocked = pNode->IsModifyLocked();
        pNode->LockModify();

        SwDoc* pDoc = pNode->GetDoc();

        // remove unused attribute RES_LR_SPACE
        // add list attributes
        SfxItemSet aSavedAttrsSet(pDoc->GetAttrPool(), RES_PAGEDESC, RES_BREAK,
                                  RES_PARATR_NUMRULE, RES_PARATR_NUMRULE,
                                  RES_PARATR_LIST_BEGIN,
                                  RES_PARATR_LIST_END - 1, 0);
        const SfxItemSet* pAttrSetOfNode = pNode->GetpSwAttrSet();

        std::vector<sal_uInt16> aClearWhichIds;
        // restoring all paragraph list attributes
        {
            SfxItemSet aListAttrSet( pDoc->GetAttrPool(), RES_PARATR_LIST_BEGIN, RES_PARATR_LIST_END - 1 );
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

        const SfxPoolItem* pItem;

        sal_uInt16 const aSavIds[3] = { RES_PAGEDESC, RES_BREAK, RES_PARATR_NUMRULE };
        for (sal_uInt16 aSavId : aSavIds)
        {
            if (SfxItemState::SET == pAttrSetOfNode->GetItemState(aSavId, false, &pItem))
            {
                bool bSave = false;
                switch( aSavId )
                {
                    case RES_PAGEDESC:
                        bSave = nullptr != static_cast<const SwFormatPageDesc*>(pItem)->GetPageDesc();
                    break;
                    case RES_BREAK:
                        bSave = SvxBreak::NONE != static_cast<const SvxFormatBreakItem*>(pItem)->GetBreak();
                    break;
                    case RES_PARATR_NUMRULE:
                        bSave = !static_cast<const SwNumRuleItem*>(pItem)->GetValue().isEmpty();
                    break;
                }
                if( bSave )
                {
                    aSavedAttrsSet.Put(*pItem);
                    aClearWhichIds.push_back(aSavId);
                }
            }
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
                pItem = aIter.FirstItem();
                while(pItem)
                {
                    if ( ( pItem->Which() != RES_PAGEDESC &&
                           pItem->Which() != RES_BREAK &&
                           pItem->Which() != RES_PARATR_NUMRULE ) ||
                         ( aSavedAttrsSet.GetItemState( pItem->Which(), false ) != SfxItemState::SET ) )
                    {
                        pNode->ResetAttr( pItem->Which() );
                    }
                    if (aIter.IsAtEnd())
                        break;
                    pItem = aIter.NextItem();
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

void SwDoc::RstTextAttrs(const SwPaM &rRg, bool bInclRefToxMark, bool bExactRange )
{
    SwHistory* pHst = nullptr;
    SwDataChanged aTmp( rRg );
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndoResetAttr* pUndo = new SwUndoResetAttr( rRg, RES_CHRFMT );
        pHst = &pUndo->GetHistory();
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }
    const SwPosition *pStt = rRg.Start(), *pEnd = rRg.End();
    sw::DocumentContentOperationsManager::ParaRstFormat aPara( pStt, pEnd, pHst );
    aPara.bInclRefToxMark = bInclRefToxMark;
    aPara.bExactRange = bExactRange;
    GetNodes().ForEach( pStt->nNode.GetIndex(), pEnd->nNode.GetIndex()+1,
                        sw::DocumentContentOperationsManager::lcl_RstTextAttr, &aPara );
    getIDocumentState().SetModified();
}

void SwDoc::ResetAttrs( const SwPaM &rRg,
                        bool bTextAttr,
                        const std::set<sal_uInt16> &rAttrs,
                        const bool bSendDataChangedEvents )
{
    SwPaM* pPam = const_cast<SwPaM*>(&rRg);
    if( !bTextAttr && !rAttrs.empty() && RES_TXTATR_END > *(rAttrs.begin()) )
        bTextAttr = true;

    if( !rRg.HasMark() )
    {
        SwTextNode* pTextNd = rRg.GetPoint()->nNode.GetNode().GetTextNode();
        if( !pTextNd )
            return ;

        pPam = new SwPaM( *rRg.GetPoint() );

        SwIndex& rSt = pPam->GetPoint()->nContent;
        sal_Int32 nMkPos, nPtPos = rSt.GetIndex();

        // Special case: if the Cursor is located within a URL attribute, we take over it's area
        SwTextAttr const*const pURLAttr(
            pTextNd->GetTextAttrAt(rSt.GetIndex(), RES_TXTATR_INETFMT));
        if (pURLAttr && !pURLAttr->GetINetFormat().GetValue().isEmpty())
        {
            nMkPos = pURLAttr->GetStart();
            nPtPos = *pURLAttr->End();
        }
        else
        {
            Boundary aBndry;
            if( g_pBreakIt->GetBreakIter().is() )
                aBndry = g_pBreakIt->GetBreakIter()->getWordBoundary(
                            pTextNd->GetText(), nPtPos,
                            g_pBreakIt->GetLocale( pTextNd->GetLang( nPtPos ) ),
                            WordType::ANY_WORD /*ANYWORD_IGNOREWHITESPACES*/,
                            true );

            if( aBndry.startPos < nPtPos && nPtPos < aBndry.endPos )
            {
                nMkPos = aBndry.startPos;
                nPtPos = aBndry.endPos;
            }
            else
            {
                nPtPos = nMkPos = rSt.GetIndex();
                if( bTextAttr )
                    pTextNd->DontExpandFormat( rSt );
            }
        }

        rSt = nMkPos;
        pPam->SetMark();
        pPam->GetPoint()->nContent = nPtPos;
    }

    // #i96644#
    std::unique_ptr< SwDataChanged > xDataChanged;
    if ( bSendDataChangedEvents )
    {
        xDataChanged.reset( new SwDataChanged( *pPam ) );
    }
    SwHistory* pHst = nullptr;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndoResetAttr* pUndo = new SwUndoResetAttr( rRg,
            static_cast<sal_uInt16>(bTextAttr ? RES_CONDTXTFMTCOLL : RES_TXTFMTCOLL ));
        if( !rAttrs.empty() )
        {
            pUndo->SetAttrs( rAttrs );
        }
        pHst = &pUndo->GetHistory();
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    const SwPosition *pStt = pPam->Start(), *pEnd = pPam->End();
    sw::DocumentContentOperationsManager::ParaRstFormat aPara( pStt, pEnd, pHst );

    // mst: not including META here; it seems attrs with CH_TXTATR are omitted
    sal_uInt16 aResetableSetRange[] = {
        RES_FRMATR_BEGIN, RES_FRMATR_END-1,
        RES_CHRATR_BEGIN, RES_CHRATR_END-1,
        RES_PARATR_BEGIN, RES_PARATR_END-1,
        RES_PARATR_LIST_BEGIN, RES_PARATR_LIST_END-1,
        RES_TXTATR_INETFMT, RES_TXTATR_INETFMT,
        RES_TXTATR_CHARFMT, RES_TXTATR_CHARFMT,
        RES_TXTATR_CJK_RUBY, RES_TXTATR_CJK_RUBY,
        RES_TXTATR_UNKNOWN_CONTAINER, RES_TXTATR_UNKNOWN_CONTAINER,
        RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
        0
    };

    SfxItemSet aDelSet( GetAttrPool(), aResetableSetRange );
    if( !rAttrs.empty() )
    {
        for( std::set<sal_uInt16>::const_reverse_iterator it = rAttrs.rbegin(); it != rAttrs.rend(); ++it )
        {
            if( POOLATTR_END > *it )
                aDelSet.Put( *GetDfltAttr( *it ));
        }
        if( aDelSet.Count() )
            aPara.pDelSet = &aDelSet;
    }

    bool bAdd = true;
    SwNodeIndex aTmpStt( pStt->nNode );
    SwNodeIndex aTmpEnd( pEnd->nNode );
    if( pStt->nContent.GetIndex() )     // just one part
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
    if( pEnd->nContent.GetIndex() == pEnd->nNode.GetNode().GetContentNode()->Len() )
    {
         // set up a later, and all CharFormatAttr -> TextFormatAttr
        ++aTmpEnd;
        bAdd = false;
    }
    else if( pStt->nNode != pEnd->nNode || !pStt->nContent.GetIndex() )
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
        GetNodes().ForEach( pStt->nNode, aTmpEnd, lcl_RstAttr, &aPara );
    else if( !rRg.HasMark() )
    {
        aPara.bResetAll = false ;
        ::lcl_RstAttr( &pStt->nNode.GetNode(), &aPara );
        aPara.bResetAll = true ;
    }

    if( bTextAttr )
    {
        if( bAdd )
            ++aTmpEnd;
        GetNodes().ForEach( pStt->nNode, aTmpEnd, sw::DocumentContentOperationsManager::lcl_RstTextAttr, &aPara );
    }

    getIDocumentState().SetModified();

    xDataChanged.reset(); //before delete pPam

    if( pPam != &rRg )
        delete pPam;
}

/// Set the rsid of the next nLen symbols of rRg to the current session number
bool SwDoc::UpdateRsid( const SwPaM &rRg, const sal_Int32 nLen )
{
    if (!SW_MOD()->GetModuleConfig()->IsStoreRsid())
        return false;

    SwTextNode *pTextNode = rRg.GetPoint()->nNode.GetNode().GetTextNode();
    if (!pTextNode)
    {
        return false;
    }
    const sal_Int32 nStart(rRg.GetPoint()->nContent.GetIndex() - nLen);
    SvxRsidItem aRsid( mnRsid, RES_CHRATR_RSID );

    SfxItemSet aSet(GetAttrPool(), RES_CHRATR_RSID, RES_CHRATR_RSID);
    aSet.Put(aRsid);
    bool const bRet(pTextNode->SetAttr(aSet, nStart,
        rRg.GetPoint()->nContent.GetIndex()));

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
    return bRet;
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
    getIDocumentState().SetModified();
}

void SwDoc::ResetAttrAtFormat( const sal_uInt16 nWhichId,
                               SwFormat& rChangedFormat )
{
    SwUndo *const pUndo = (GetIDocumentUndoRedo().DoesUndo())
        ?   new SwUndoFormatResetAttr( rChangedFormat, nWhichId )
        :   nullptr;

    const bool bAttrReset = rChangedFormat.ResetFormatAttr( nWhichId );

    if ( bAttrReset )
    {
        if ( pUndo )
        {
            GetIDocumentUndoRedo().AppendUndo( pUndo );
        }

        getIDocumentState().SetModified();
    }
    else
        delete pUndo;
}

static bool lcl_SetNewDefTabStops( SwTwips nOldWidth, SwTwips nNewWidth,
                                SvxTabStopItem& rChgTabStop )
{
    // Set the default values of all TabStops to the new value.
    // Attention: we always work with the PoolAttribut here, so that
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

    SwModify aCallMod( nullptr );
    SwAttrSet aOld( GetAttrPool(), rSet.GetRanges() ),
            aNew( GetAttrPool(), rSet.GetRanges() );
    SfxItemIter aIter( rSet );
    const SfxPoolItem* pItem = aIter.GetCurItem();
    SfxItemPool* pSdrPool = GetAttrPool().GetSecondaryPool();
    while( true )
    {
        bool bCheckSdrDflt = false;
        const sal_uInt16 nWhich = pItem->Which();
        aOld.Put( GetAttrPool().GetDefaultItem( nWhich ) );
        GetAttrPool().SetPoolDefaultItem( *pItem );
        aNew.Put( GetAttrPool().GetDefaultItem( nWhich ) );

        if (isCHRATR(nWhich) || isTXTATR(nWhich))
        {
            aCallMod.Add( mpDfltTextFormatColl );
            aCallMod.Add( mpDfltCharFormat );
            bCheckSdrDflt = nullptr != pSdrPool;
        }
        else if ( isPARATR(nWhich) ||
                  isPARATR_LIST(nWhich) )
        {
            aCallMod.Add( mpDfltTextFormatColl );
            bCheckSdrDflt = nullptr != pSdrPool;
        }
        else if (isGRFATR(nWhich))
        {
            aCallMod.Add( mpDfltGrfFormatColl );
        }
        else if (isFRMATR(nWhich) || isDrawingLayerAttribute(nWhich) ) //UUUU
        {
            aCallMod.Add( mpDfltGrfFormatColl );
            aCallMod.Add( mpDfltTextFormatColl );
            aCallMod.Add( mpDfltFrameFormat );
        }
        else if (isBOXATR(nWhich))
        {
            aCallMod.Add( mpDfltFrameFormat );
        }

        // also copy the defaults
        if( bCheckSdrDflt )
        {
            sal_uInt16 nEdtWhich, nSlotId;
            if( 0 != (nSlotId = GetAttrPool().GetSlotId( nWhich ) ) &&
                nSlotId != nWhich &&
                0 != (nEdtWhich = pSdrPool->GetWhich( nSlotId )) &&
                nSlotId != nEdtWhich )
            {
                SfxPoolItem* pCpy = pItem->Clone();
                pCpy->SetWhich( nEdtWhich );
                pSdrPool->SetPoolDefaultItem( *pCpy );
                delete pCpy;
            }
        }

        if( aIter.IsAtEnd() )
            break;
        pItem = aIter.NextItem();
    }

    if( aNew.Count() && aCallMod.HasWriterListeners() )
    {
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            GetIDocumentUndoRedo().AppendUndo( new SwUndoDefaultAttr( aOld, this ) );
        }

        const SfxPoolItem* pTmpItem;
        if( ( SfxItemState::SET ==
                aNew.GetItemState( RES_PARATR_TABSTOP, false, &pTmpItem ) ) &&
            static_cast<const SvxTabStopItem*>(pTmpItem)->Count() )
        {
            // Set the default values of all TabStops to the new value.
            // Attention: we always work with the PoolAttribut here, so that
            // we don't calculate the same value on the same TabStop (pooled!) for all sets.
            // We send a FormatChg to modify.
            SwTwips nNewWidth = (*static_cast<const SvxTabStopItem*>(pTmpItem))[ 0 ].GetTabPos(),
                    nOldWidth = static_cast<const SvxTabStopItem&>(aOld.Get(RES_PARATR_TABSTOP))[ 0 ].GetTabPos();

            bool bChg = false;
            sal_uInt32 nMaxItems = GetAttrPool().GetItemCount2( RES_PARATR_TABSTOP );
            for( sal_uInt32 n = 0; n < nMaxItems; ++n )
                if( nullptr != (pTmpItem = GetAttrPool().GetItem2( RES_PARATR_TABSTOP, n ) ))
                    bChg |= lcl_SetNewDefTabStops( nOldWidth, nNewWidth,
                                                   *const_cast<SvxTabStopItem*>(static_cast<const SvxTabStopItem*>(pTmpItem)) );

            aNew.ClearItem( RES_PARATR_TABSTOP );
            aOld.ClearItem( RES_PARATR_TABSTOP );
            if( bChg )
            {
                SwFormatChg aChgFormat( mpDfltCharFormat );
                // notify the frames
                aCallMod.ModifyNotification( &aChgFormat, &aChgFormat );
            }
        }
    }

    if( aNew.Count() && aCallMod.HasWriterListeners() )
    {
        SwAttrSetChg aChgOld( aOld, aOld );
        SwAttrSetChg aChgNew( aNew, aNew );
        aCallMod.ModifyNotification( &aChgOld, &aChgNew );      // all changed are sent
    }

    // remove the default formats from the object again
    SwIterator<SwClient, SwModify> aClientIter(aCallMod);
    for(SwClient* pClient = aClientIter.First(); pClient; pClient = aClientIter.Next())
        aCallMod.Remove( pClient );

    getIDocumentState().SetModified();
}

/// Get the default attribute in this document
const SfxPoolItem& SwDoc::GetDefault( sal_uInt16 nFormatHint ) const
{
    return GetAttrPool().GetDefaultItem( nFormatHint );
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
        SwUndo * pUndo =
            new SwUndoCharFormatDelete(pDel, this);

        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    delete (*mpCharFormatTable)[nFormat];
    mpCharFormatTable->erase(mpCharFormatTable->begin() + nFormat);

    getIDocumentState().SetModified();
}

void SwDoc::DelCharFormat( SwCharFormat *pFormat, bool bBroadcast )
{
    size_t nFormat = mpCharFormatTable->GetPos( pFormat );
    OSL_ENSURE( SIZE_MAX != nFormat, "Format not found," );
    DelCharFormat( nFormat, bBroadcast );
}

void SwDoc::DelFrameFormat( SwFrameFormat *pFormat, bool bBroadcast )
{
    if( dynamic_cast<const SwTableBoxFormat*>( pFormat) != nullptr || dynamic_cast<const SwTableLineFormat*>( pFormat) != nullptr )
    {
        OSL_ENSURE( false, "Format is not in the DocArray any more, "
                       "so it can be deleted with delete" );
        delete pFormat;
    }
    else
    {
        // The format has to be in the one or the other, we'll see in which one.
        if ( mpFrameFormatTable->Contains( pFormat ) )
        {
            if (bBroadcast)
                BroadcastStyleOperation(pFormat->GetName(),
                                        SfxStyleFamily::Frame,
                                        SfxHintId::StyleSheetErased);

            if (GetIDocumentUndoRedo().DoesUndo())
            {
                SwUndo * pUndo = new SwUndoFrameFormatDelete(pFormat, this);

                GetIDocumentUndoRedo().AppendUndo(pUndo);
            }

            mpFrameFormatTable->erase( pFormat );
            delete pFormat;
        }
        else
        {
            bool contains = GetSpzFrameFormats()->Contains( pFormat );
            OSL_ENSURE( contains, "FrameFormat not found." );
            if( contains )
            {
                GetSpzFrameFormats()->erase( pFormat );
                delete pFormat;
            }
        }
    }
}

void SwDoc::DelTableFrameFormat( SwTableFormat *pFormat )
{
    SwFrameFormats::const_iterator it = mpTableFrameFormatTable->find( pFormat );
    OSL_ENSURE( it != mpTableFrameFormatTable->end(), "Format not found," );
    mpTableFrameFormatTable->erase( it );
    delete pFormat;
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
    {
        return mpTableFrameFormatTable->size();
    }

    SwAutoFormatGetDocNode aGetHt(&GetNodes());
    size_t nCount = 0;
    for (SwFrameFormat* const & pFormat : *mpTableFrameFormatTable)
    {
        if (!pFormat->GetInfo(aGetHt))
            nCount++;
    }
    return nCount;
}

SwFrameFormat& SwDoc::GetTableFrameFormat(size_t nFormat, bool bUsed) const
{
    if (!bUsed)
    {
        return *((*mpTableFrameFormatTable)[nFormat]);
    }

    SwAutoFormatGetDocNode aGetHt(&GetNodes());

    size_t index = 0;

    for (SwFrameFormat* const & pFormat : *mpTableFrameFormatTable)
    {
        if (!pFormat->GetInfo(aGetHt))
        {
            if (index == nFormat)
                return *pFormat;
            else
                index++;
        }
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
        SwUndo * pUndo = new SwUndoFrameFormatCreate(pFormat, pDerivedFrom, this);

        GetIDocumentUndoRedo().AppendUndo(pUndo);
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
    return dynamic_cast<SwFormat*>(pFrameFormat);
}

// #i40550# - add parameter <bAuto> - not relevant
SwCharFormat *SwDoc::MakeCharFormat( const OUString &rFormatName,
                               SwCharFormat *pDerivedFrom,
                               bool bBroadcast,
                               bool )
{
    SwCharFormat *pFormat = new SwCharFormat( GetAttrPool(), rFormatName, pDerivedFrom );
    mpCharFormatTable->push_back( pFormat );
    pFormat->SetAuto(false);
    getIDocumentState().SetModified();

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndo * pUndo = new SwUndoCharFormatCreate(pFormat, pDerivedFrom, this);

        GetIDocumentUndoRedo().AppendUndo(pUndo);
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
                            bool bBroadcast, bool bAuto)
{
    SwCharFormat *pCharFormat = dynamic_cast<SwCharFormat*>(pDerivedFrom);
    pCharFormat = MakeCharFormat( rFormatName, pCharFormat, bBroadcast, bAuto );
    return dynamic_cast<SwFormat*>(pCharFormat);
}

/// Create the FormatCollections
SwTextFormatColl* SwDoc::MakeTextFormatColl( const OUString &rFormatName,
                                     SwTextFormatColl *pDerivedFrom,
                                     bool bBroadcast,
                                     bool )
{
    SwTextFormatColl *pFormatColl = new SwTextFormatColl( GetAttrPool(), rFormatName,
                                                pDerivedFrom );
    mpTextFormatCollTable->push_back(pFormatColl);
    pFormatColl->SetAuto(false);
    getIDocumentState().SetModified();

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndo * pUndo = new SwUndoTextFormatCollCreate(pFormatColl, pDerivedFrom,
                                                    this);
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    if (bBroadcast)
        BroadcastStyleOperation(rFormatName, SfxStyleFamily::Para,
                                SfxHintId::StyleSheetCreated);

    return pFormatColl;
}

SwFormat *SwDoc::MakeTextFormatColl_(const OUString &rFormatName,
                            SwFormat *pDerivedFrom,
                            bool bBroadcast, bool bAuto)
{
    SwTextFormatColl *pTextFormatColl = dynamic_cast<SwTextFormatColl*>(pDerivedFrom);
    pTextFormatColl = MakeTextFormatColl( rFormatName, pTextFormatColl, bBroadcast, bAuto );
    return dynamic_cast<SwFormat*>(pTextFormatColl);
}

//FEATURE::CONDCOLL
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
        SwUndo * pUndo = new SwUndoCondTextFormatCollCreate(pFormatColl, pDerivedFrom,
                                                        this);
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    if (bBroadcast)
        BroadcastStyleOperation(rFormatName, SfxStyleFamily::Para,
                                SfxHintId::StyleSheetCreated);

    return pFormatColl;
}
//FEATURE::CONDCOLL

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
    OSL_ENSURE( nFormatColl, "Remove fuer Coll 0." );

    // Who has the to-be-deleted as their Next?
    SwTextFormatColl *pDel = (*mpTextFormatCollTable)[nFormatColl];
    if( mpDfltTextFormatColl == pDel )
        return;     // never delete default!

    if (bBroadcast)
        BroadcastStyleOperation(pDel->GetName(), SfxStyleFamily::Para,
                                SfxHintId::StyleSheetErased);

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndoTextFormatCollDelete * pUndo;
        if (RES_CONDTXTFMTCOLL == pDel->Which())
        {
            pUndo = new SwUndoCondTextFormatCollDelete(pDel, this);
        }
        else
        {
            pUndo = new SwUndoTextFormatCollDelete(pDel, this);
        }

        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    // Remove the FormatColl
    mpTextFormatCollTable->erase(mpTextFormatCollTable->begin() + nFormatColl);
    // Correct next
    for( SwTextFormatColls::const_iterator it = mpTextFormatCollTable->begin() + 1; it != mpTextFormatCollTable->end(); ++it )
        SetTextFormatCollNext( *it, pDel );
    delete pDel;
    getIDocumentState().SetModified();
}

void SwDoc::DelTextFormatColl( SwTextFormatColl *pColl, bool bBroadcast )
{
    size_t nFormat = mpTextFormatCollTable->GetPos( pColl );
    OSL_ENSURE( SIZE_MAX != nFormat, "Collection not found," );
    DelTextFormatColl( nFormat, bBroadcast );
}

static bool lcl_SetTextFormatColl( const SwNodePtr& rpNode, void* pArgs )
{
    SwContentNode* pCNd = static_cast<SwContentNode*>(rpNode->GetTextNode());

    if( pCNd == nullptr)
        return true;

    sw::DocumentContentOperationsManager::ParaRstFormat* pPara = static_cast<sw::DocumentContentOperationsManager::ParaRstFormat*>(pArgs);

    SwTextFormatColl* pFormat = static_cast<SwTextFormatColl*>(pPara->pFormatColl);
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
            SwTextNode* pTNd( dynamic_cast<SwTextNode*>(pCNd) );
            OSL_ENSURE( pTNd, "<lcl_SetTextFormatColl(..)> - text node expected -> crash" );
            {
                SwNumRule* pNumRuleAtParagraph( pTNd->GetNumRule() );
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
                    pRegH.reset( new SwRegHistory( pTNd, *pTNd, pPara->pHistory ) );
                }

                pCNd->ResetAttr( RES_PARATR_NUMRULE );

                // reset all list attributes
                pCNd->ResetAttr( RES_PARATR_LIST_LEVEL );
                pCNd->ResetAttr( RES_PARATR_LIST_ISRESTART );
                pCNd->ResetAttr( RES_PARATR_LIST_RESTARTVALUE );
                pCNd->ResetAttr( RES_PARATR_LIST_ISCOUNTED );
                pCNd->ResetAttr( RES_PARATR_LIST_ID );
            }
        }
    }

    // add to History so that old data is saved, if necessary
    if( pPara->pHistory )
        pPara->pHistory->Add( pCNd->GetFormatColl(), pCNd->GetIndex(),
                SwNodeType::Text );

    pCNd->ChgFormatColl( pFormat );

    pPara->nWhich++;

    return true;
}

bool SwDoc::SetTextFormatColl(const SwPaM &rRg,
                          SwTextFormatColl *pFormat,
                          const bool bReset,
                          const bool bResetListAttrs)
{
    SwDataChanged aTmp( rRg );
    const SwPosition *pStt = rRg.Start(), *pEnd = rRg.End();
    SwHistory* pHst = nullptr;
    bool bRet = true;

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndoFormatColl* pUndo = new SwUndoFormatColl( rRg, pFormat,
                                                  bReset,
                                                  bResetListAttrs );
        pHst = pUndo->GetHistory();
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    sw::DocumentContentOperationsManager::ParaRstFormat aPara( pStt, pEnd, pHst );
    aPara.pFormatColl = pFormat;
    aPara.bReset = bReset;
    // #i62675#
    aPara.bResetListAttrs = bResetListAttrs;

    GetNodes().ForEach( pStt->nNode.GetIndex(), pEnd->nNode.GetIndex()+1,
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
    SwTextFormatColl* pParent = mpDfltTextFormatColl;
    if( pParent != rColl.DerivedFrom() )
        pParent = CopyTextColl( *static_cast<SwTextFormatColl*>(rColl.DerivedFrom()) );

//FEATURE::CONDCOLL
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
//FEATURE::CONDCOLL
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
        const SfxPoolItem* pItem;
        if( SfxItemState::SET == pNewColl->GetItemState( RES_PARATR_NUMRULE,
            false, &pItem ))
        {
            const SwNumRule* pRule;
            const OUString& rName = static_cast<const SwNumRuleItem*>(pItem)->GetValue();
            if( !rName.isEmpty() &&
                nullptr != ( pRule = rColl.GetDoc()->FindNumRulePtr( rName )) &&
                !pRule->IsAutoRule() )
            {
                SwNumRule* pDestRule = FindNumRulePtr( rName );
                if( pDestRule )
                    pDestRule->SetInvalidRule( true );
                else
                    MakeNumRule( rName, pRule );
            }
        }
    }
    return pNewColl;
}

/// copy the graphic nodes
SwGrfFormatColl* SwDoc::CopyGrfColl( const SwGrfFormatColl& rColl )
{
    SwGrfFormatColl* pNewColl = static_cast<SwGrfFormatColl*>(FindFormatByName( (SwFormatsBase&)*mpGrfFormatCollTable, rColl.GetName() ));
    if( pNewColl )
        return pNewColl;

     // Search for the "parent" first
    SwGrfFormatColl* pParent = mpDfltGrfFormatColl;
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
                        SwFormatsBase& rDestArr,
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

        if( nullptr == FindFormatByName( rDestArr, pSrc->GetName() ) )
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

        pDest = FindFormatByName( rDestArr, pSrc->GetName() );
        pDest->SetAuto(false);
        pDest->DelDiffs( *pSrc );

        // #i94285#: existing <SwFormatPageDesc> instance, before copying attributes
        const SfxPoolItem* pItem;
        if( &GetAttrPool() != pSrc->GetAttrSet().GetPool() &&
            SfxItemState::SET == pSrc->GetAttrSet().GetItemState(
            RES_PAGEDESC, false, &pItem ) &&
            static_cast<const SwFormatPageDesc*>(pItem)->GetPageDesc() )
        {
            SwFormatPageDesc aPageDesc( *static_cast<const SwFormatPageDesc*>(pItem) );
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
            pDest->SetDerivedFrom( FindFormatByName( rDestArr,
                                        pSrc->DerivedFrom()->GetName() ) );
        if( RES_TXTFMTCOLL == pSrc->Which() ||
            RES_CONDTXTFMTCOLL == pSrc->Which() )
        {
            SwTextFormatColl* pSrcColl = static_cast<SwTextFormatColl*>(pSrc),
                        * pDstColl = static_cast<SwTextFormatColl*>(pDest);
            if( &pSrcColl->GetNextTextFormatColl() != pSrcColl )
                pDstColl->SetNextTextFormatColl( *static_cast<SwTextFormatColl*>(FindFormatByName(
                    rDestArr, pSrcColl->GetNextTextFormatColl().GetName() ) ) );

            if(pSrcColl->IsAssignedToListLevelOfOutlineStyle())
                pDstColl->AssignToListLevelOfOutlineStyle(pSrcColl->GetAssignedOutlineStyleLevel());

//FEATURE::CONDCOLL
            if( RES_CONDTXTFMTCOLL == pSrc->Which() )
                // Copy the conditions, but delete the old ones first!
                static_cast<SwConditionTextFormatColl*>(pDstColl)->SetConditions(
                            static_cast<SwConditionTextFormatColl*>(pSrc)->GetCondColls() );
//FEATURE::CONDCOLL
        }
    }
}

void SwDoc::CopyPageDescHeaderFooterImpl( bool bCpyHeader,
                                const SwFrameFormat& rSrcFormat, SwFrameFormat& rDestFormat )
{
    // Treat the header and footer attributes in the right way:
    // Copy content nodes across documents!
    sal_uInt16 nAttr = static_cast<sal_uInt16>( bCpyHeader ? RES_HEADER : RES_FOOTER );
    const SfxPoolItem* pItem;
    if( SfxItemState::SET != rSrcFormat.GetAttrSet().GetItemState( nAttr, false, &pItem ))
        return ;

    // The header only contains the reference to the format from the other document!
    SfxPoolItem* pNewItem = pItem->Clone();

    SwFrameFormat* pOldFormat;
    if( bCpyHeader )
         pOldFormat = static_cast<SwFormatHeader*>(pNewItem)->GetHeaderFormat();
    else
         pOldFormat = static_cast<SwFormatFooter*>(pNewItem)->GetFooterFormat();

    if( pOldFormat )
    {
        SwFrameFormat* pNewFormat = new SwFrameFormat( GetAttrPool(), "CpyDesc",
                                            GetDfltFrameFormat() );
        pNewFormat->CopyAttrs( *pOldFormat );

        if( SfxItemState::SET == pNewFormat->GetAttrSet().GetItemState(
            RES_CNTNT, false, &pItem ))
        {
            const SwFormatContent* pContent = static_cast<const SwFormatContent*>(pItem);
            if( pContent->GetContentIdx() )
            {
                SwNodeIndex aTmpIdx( GetNodes().GetEndOfAutotext() );
                const SwNodes& rSrcNds = rSrcFormat.GetDoc()->GetNodes();
                SwStartNode* pSttNd = SwNodes::MakeEmptySection( aTmpIdx,
                                                bCpyHeader
                                                    ? SwHeaderStartNode
                                                    : SwFooterStartNode );
                const SwNode& rCSttNd = pContent->GetContentIdx()->GetNode();
                SwNodeRange aRg( rCSttNd, 0, *rCSttNd.EndOfSectionNode() );
                aTmpIdx = *pSttNd->EndOfSectionNode();
                rSrcNds.Copy_( aRg, aTmpIdx );
                aTmpIdx = *pSttNd;
                rSrcFormat.GetDoc()->GetDocumentContentOperationsManager().CopyFlyInFlyImpl( aRg, 0, aTmpIdx );
                pNewFormat->SetFormatAttr( SwFormatContent( pSttNd ));
            }
            else
                pNewFormat->ResetFormatAttr( RES_CNTNT );
        }
        if( bCpyHeader )
            static_cast<SwFormatHeader*>(pNewItem)->RegisterToFormat(*pNewFormat);
        else
            static_cast<SwFormatFooter*>(pNewItem)->RegisterToFormat(*pNewFormat);
        rDestFormat.SetFormatAttr( *pNewItem );
    }
    delete pNewItem;
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
    //  a) in rtf export don't export our hideous pgdsctbl
    //  extension to rtf anymore
    //  b) in sd rtf import (View::InsertData) don't use
    //  a super-fragile test for mere presence of \trowd to
    //  indicate import of rtf into a table
    //  c) then drop use of bIncludePageStyles
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
                pNew->CopyNumRule( this, rR );
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

SwFormat* SwDoc::FindFormatByName( const SwFormatsBase& rFormatArr,
                                   const OUString& rName )
{
    SwFormat* pFnd = nullptr;
    for( size_t n = 0; n < rFormatArr.GetFormatCount(); ++n )
    {
        // Does the Doc already contain the template?
        if( rFormatArr.GetFormat(n)->HasName( rName ) )
        {
            pFnd = rFormatArr.GetFormat(n);
            break;
        }
    }
    return pFnd;
}

void SwDoc::MoveLeftMargin( const SwPaM& rPam, bool bRight, bool bModulus )
{
    SwHistory* pHistory = nullptr;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndoMoveLeftMargin* pUndo = new SwUndoMoveLeftMargin( rPam, bRight,
                                                                bModulus );
        pHistory = &pUndo->GetHistory();
        GetIDocumentUndoRedo().AppendUndo( pUndo );
    }

    const SvxTabStopItem& rTabItem = static_cast<const SvxTabStopItem&>(GetDefault( RES_PARATR_TABSTOP ));
    const sal_Int32 nDefDist = rTabItem.Count() ? rTabItem[0].GetTabPos() : 1134;
    const SwPosition &rStt = *rPam.Start(), &rEnd = *rPam.End();
    SwNodeIndex aIdx( rStt.nNode );
    while( aIdx <= rEnd.nNode )
    {
        SwTextNode* pTNd = aIdx.GetNode().GetTextNode();
        if( pTNd )
        {
            SvxLRSpaceItem aLS( static_cast<const SvxLRSpaceItem&>(pTNd->SwContentNode::GetAttr( RES_LR_SPACE )) );

            // #i93873# See also lcl_MergeListLevelIndentAsLRSpaceItem in thints.cxx
            if ( pTNd->AreListLevelIndentsApplicable() )
            {
                const SwNumRule* pRule = pTNd->GetNumRule();
                if ( pRule )
                {
                    const int nListLevel = pTNd->GetActualListLevel();
                    if ( nListLevel >= 0 )
                    {
                        const SwNumFormat& rFormat = pRule->Get(static_cast<sal_uInt16>(nListLevel));
                        if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
                        {
                            aLS.SetTextLeft( rFormat.GetIndentAt() );
                            aLS.SetTextFirstLineOfst( static_cast<short>(rFormat.GetFirstLineIndent()) );
                        }
                    }
                }
            }

            long nNext = aLS.GetTextLeft();
            if( bModulus )
                nNext = ( nNext / nDefDist ) * nDefDist;

            if( bRight )
                nNext += nDefDist;
            else
                if(nNext >0) // fdo#75936 set limit for decreasing indent
                    nNext -= nDefDist;

            aLS.SetTextLeft( nNext );

            SwRegHistory aRegH( pTNd, *pTNd, pHistory );
            pTNd->SetAttr( aLS );
        }
        ++aIdx;
    }
    getIDocumentState().SetModified();
}

bool SwDoc::DontExpandFormat( const SwPosition& rPos, bool bFlag )
{
    bool bRet = false;
    SwTextNode* pTextNd = rPos.nNode.GetNode().GetTextNode();
    if( pTextNd )
    {
        bRet = pTextNd->DontExpandFormat( rPos.nContent, bFlag );
        if( bRet && GetIDocumentUndoRedo().DoesUndo() )
        {
            GetIDocumentUndoRedo().AppendUndo( new SwUndoDontExpandFormat(rPos) );
        }
    }
    return bRet;
}

SwTableBoxFormat* SwDoc::MakeTableBoxFormat()
{
    SwTableBoxFormat* pFormat = new SwTableBoxFormat( GetAttrPool(), aEmptyOUStr,
                                                mpDfltFrameFormat );
    getIDocumentState().SetModified();
    return pFormat;
}

SwTableLineFormat* SwDoc::MakeTableLineFormat()
{
    SwTableLineFormat* pFormat = new SwTableLineFormat( GetAttrPool(), aEmptyOUStr,
                                                mpDfltFrameFormat );
    getIDocumentState().SetModified();
    return pFormat;
}

void SwDoc::CreateNumberFormatter()
{
    OSL_ENSURE( !mpNumberFormatter, "is already there" );

    LanguageType eLang = LANGUAGE_SYSTEM;

    mpNumberFormatter = new SvNumberFormatter( comphelper::getProcessComponentContext(), eLang );
    mpNumberFormatter->SetEvalDateFormat( NF_EVALDATEFORMAT_FORMAT_INTL );
    if (!utl::ConfigManager::IsAvoidConfig())
        mpNumberFormatter->SetYear2000(static_cast<sal_uInt16>(::utl::MiscCfg().GetYear2000()));
}

SwTableNumFormatMerge::SwTableNumFormatMerge( const SwDoc& rSrc, SwDoc& rDest )
    : pNFormat( nullptr )
{
    // a different Doc -> Number formatter needs to be merged
    SvNumberFormatter* pN;
    if( &rSrc != &rDest && nullptr != ( pN = const_cast<SwDoc&>(rSrc).GetNumberFormatter( false ) ))
        ( pNFormat = rDest.GetNumberFormatter())->MergeFormatter( *pN );

    if( &rSrc != &rDest )
        static_cast<SwGetRefFieldType*>(rSrc.getIDocumentFieldsAccess().GetSysFieldType( RES_GETREFFLD ))->
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
    SwTextNode* pTNd = rPos.nNode.GetNode().GetTextNode();
    assert(pTNd);

    if (mbIsAutoFormatRedline)
    {
        // create the redline object
        const SwTextFormatColl& rColl = *pTNd->GetTextColl();
        SwRangeRedline* pRedl = new SwRangeRedline( nsRedlineType_t::REDLINE_FMTCOLL, aPam );
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
        aPam.GetMark()->nContent.Assign(pTNd, pTNd->GetText().getLength());
        getIDocumentContentOperations().InsertItemSet( aPam, *pSet );
    }
}

void SwDoc::SetFormatItemByAutoFormat( const SwPaM& rPam, const SfxItemSet& rSet )
{
    SwTextNode* pTNd = rPam.GetPoint()->nNode.GetNode().GetTextNode();
    assert(pTNd);

    RedlineFlags eOld = getIDocumentRedlineAccess().GetRedlineFlags();

    if (mbIsAutoFormatRedline)
    {
        // create the redline object
        SwRangeRedline* pRedl = new SwRangeRedline( nsRedlineType_t::REDLINE_FORMAT, rPam );
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

    const sal_Int32 nEnd(rPam.End()->nContent.GetIndex());
    std::vector<sal_uInt16> whichIds;
    SfxItemIter iter(rSet);
    for (SfxPoolItem const* pItem = iter.FirstItem();
            pItem; pItem = iter.NextItem())
    {
        whichIds.push_back(pItem->Which());
        whichIds.push_back(pItem->Which());
    }
    whichIds.push_back(0);
    SfxItemSet currentSet(GetAttrPool(), &whichIds[0]);
    pTNd->GetAttr(currentSet, nEnd, nEnd);
    for (size_t i = 0; whichIds[i]; i += 2)
    {   // yuk - want to explicitly set the pool defaults too :-/
        currentSet.Put(currentSet.Get(whichIds[i]));
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

            const SfxPoolItem * pItem = aIter.FirstItem();
            while (pItem != nullptr)
            {
                aOldSet.InvalidateItem(pItem->Which());

                pItem = aIter.NextItem();
            }
        }

        SwUndo * pUndo = new SwUndoFormatAttr(aOldSet, rFormat);

        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    rFormat.SetFormatAttr(rSet);
}

void SwDoc::RenameFormat(SwFormat & rFormat, const OUString & sNewName,
                      bool bBroadcast)
{
    SfxStyleFamily eFamily = SfxStyleFamily::All;

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndo * pUndo = nullptr;

        switch (rFormat.Which())
        {
        case RES_CHRFMT:
            pUndo = new SwUndoRenameCharFormat(rFormat.GetName(), sNewName, this);
            eFamily = SfxStyleFamily::Char;
            break;
        case RES_TXTFMTCOLL:
            pUndo = new SwUndoRenameFormatColl(rFormat.GetName(), sNewName, this);
            eFamily = SfxStyleFamily::Para;
            break;
        case RES_FRMFMT:
            pUndo = new SwUndoRenameFrameFormat(rFormat.GetName(), sNewName, this);
            eFamily = SfxStyleFamily::Frame;
            break;

        default:
            break;
        }

        if (pUndo)
        {
            GetIDocumentUndoRedo().AppendUndo(pUndo);
        }
    }

    rFormat.SetName(sNewName);

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
        xmlTextWriterSetIndentString(pWriter, BAD_CAST("  "));
        xmlTextWriterStartDocument(pWriter, nullptr, nullptr, nullptr);
        bOwns = true;
    }
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwDoc"));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);

    m_pNodes->dumpAsXml(pWriter);
    maDBData.dumpAsXml(pWriter);
    mpMarkManager->dumpAsXml(pWriter);
    m_pUndoManager->dumpAsXml(pWriter);
    getIDocumentFieldsAccess().GetFieldTypes()->dumpAsXml(pWriter);
    mpTextFormatCollTable->dumpAsXml(pWriter);
    mpCharFormatTable->dumpAsXml(pWriter);
    mpFrameFormatTable->dumpAsXml(pWriter, "frmFormatTable");
    mpSpzFrameFormatTable->dumpAsXml(pWriter, "spzFrameFormatTable");
    mpSectionFormatTable->dumpAsXml(pWriter);
    mpNumRuleTable->dumpAsXml(pWriter);
    getIDocumentRedlineAccess().GetRedlineTable().dumpAsXml(pWriter);
    getIDocumentRedlineAccess().GetExtraRedlineTable().dumpAsXml(pWriter);
    if (const SdrModel* pModel = getIDocumentDrawModelAccess().GetDrawModel())
        pModel->dumpAsXml(pWriter);

    xmlTextWriterStartElement(pWriter, BAD_CAST("mbModified"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(OString::boolean(getIDocumentState().IsModified()).getStr()));
    xmlTextWriterEndElement(pWriter);

    xmlTextWriterEndElement(pWriter);
    if (bOwns)
    {
        xmlTextWriterEndDocument(pWriter);
        xmlFreeTextWriter(pWriter);
    }
}

void SwDBData::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwDBData"));

    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("sDataSource"), BAD_CAST(sDataSource.toUtf8().getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("sCommand"), BAD_CAST(sCommand.toUtf8().getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nCommandType"), BAD_CAST(OString::number(nCommandType).getStr()));

    xmlTextWriterEndElement(pWriter);
}

std::set<Color> SwDoc::GetDocColors()
{
    std::set<Color> aDocColors;
    SwAttrPool& rPool = GetAttrPool();
    const sal_uInt16 pAttribs[] = {RES_CHRATR_COLOR, RES_CHRATR_HIGHLIGHT, RES_BACKGROUND};
    for (sal_uInt16 nAttrib : pAttribs)
    {
        const sal_uInt32 nCount = rPool.GetItemCount2(nAttrib);
        for (sal_uInt32 j=0; j<nCount; j++)
        {
            const SvxColorItem *pItem = static_cast<const SvxColorItem*>(rPool.GetItem2(nAttrib, j));
            if (pItem == nullptr)
                continue;
            Color aColor( pItem->GetValue() );
            if (COL_AUTO != aColor.GetColor())
                aDocColors.insert(aColor);
        }
    }
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

SwFrameFormats::SwFrameFormats()
    : m_PosIndex( m_Array.get<0>() )
    , m_TypeAndNameIndex( m_Array.get<1>() )
{
}

SwFrameFormats::~SwFrameFormats()
{
    DeleteAndDestroyAll();
}

SwFrameFormats::iterator SwFrameFormats::find( const value_type& x ) const
{
    ByTypeAndName::iterator it = m_TypeAndNameIndex.find(
        boost::make_tuple(x->Which(), x->GetName(), x) );
    return m_Array.project<0>( it );
}

std::pair<SwFrameFormats::const_range_iterator,SwFrameFormats::const_range_iterator>
SwFrameFormats::rangeFind( sal_uInt16 type, const OUString& name ) const
{
    return m_TypeAndNameIndex.equal_range( boost::make_tuple(type, name) );
}

std::pair<SwFrameFormats::const_range_iterator,SwFrameFormats::const_range_iterator>
SwFrameFormats::rangeFind( const value_type& x ) const
{
    return rangeFind( x->Which(), x->GetName() );
}

void SwFrameFormats::DeleteAndDestroyAll( bool keepDefault )
{
    if ( empty() )
        return;
    const int _offset = keepDefault ? 1 : 0;
    for( const_iterator it = begin() + _offset; it != end(); ++it )
        delete *it;
    if ( _offset )
        m_PosIndex.erase( begin() + _offset, end() );
    else
        m_Array.clear();
}

std::pair<SwFrameFormats::const_iterator,bool> SwFrameFormats::push_back( const value_type& x )
{
    SAL_WARN_IF(x->m_ffList != nullptr, "sw.core", "Inserting already assigned item");
    assert(x->m_ffList == nullptr);
    x->m_ffList = this;
    return m_PosIndex.push_back( x );
}

bool SwFrameFormats::erase( const value_type& x )
{
    const_iterator const ret = find( x );
    SAL_WARN_IF(x->m_ffList != this, "sw.core", "Removing invalid / unassigned item");
    if (ret != end()) {
        assert( x == *ret );
        m_PosIndex.erase( ret );
        x->m_ffList = nullptr;
        return true;
    }
    return false;
}

void SwFrameFormats::erase( size_type index_ )
{
    erase( begin() + index_ );
}

void SwFrameFormats::erase( const_iterator const& position )
{
    (*position)->m_ffList = nullptr;
    m_PosIndex.erase( begin() + (position - begin()) );
}

bool SwFrameFormats::Contains( const SwFrameFormats::value_type& x ) const
{
    return (x->m_ffList == this);
}

bool SwFrameFormats::newDefault( const value_type& x )
{
    std::pair<iterator,bool> res = m_PosIndex.push_front( x );
    if( ! res.second )
        newDefault( res.first );
    return res.second;
}

void SwFrameFormats::newDefault( const_iterator const& position )
{
    if (position == begin())
        return;
    m_PosIndex.relocate( begin(), position );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
