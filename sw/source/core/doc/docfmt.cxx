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
#include <boost/scoped_ptr.hpp>

using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

/*
 * Internal functions
 */

static void SetTxtFmtCollNext( SwTxtFmtColl* pTxtColl, const SwTxtFmtColl* pDel )
{
    if ( &pTxtColl->GetNextTxtFmtColl() == pDel )
    {
        pTxtColl->SetNextTxtFmtColl( *pTxtColl );
    }
}

static bool lcl_RstAttr( const SwNodePtr& rpNd, void* pArgs )
{
    const sw::DocumentContentOperationsManager::ParaRstFmt* pPara = static_cast<sw::DocumentContentOperationsManager::ParaRstFmt*>(pArgs);
    SwCntntNode* pNode = rpNd->GetCntntNode();
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
        for (int n = 0; n < 3; ++n)
        {
            if (SfxItemState::SET == pAttrSetOfNode->GetItemState(aSavIds[n], false, &pItem))
            {
                bool bSave = false;
                switch( aSavIds[ n ] )
                {
                    case RES_PAGEDESC:
                        bSave = 0 != static_cast<const SwFmtPageDesc*>(pItem)->GetPageDesc();
                    break;
                    case RES_BREAK:
                        bSave = SVX_BREAK_NONE != static_cast<const SvxFmtBreakItem*>(pItem)->GetBreak();
                    break;
                    case RES_PARATR_NUMRULE:
                        bSave = !static_cast<const SwNumRuleItem*>(pItem)->GetValue().isEmpty();
                    break;
                }
                if( bSave )
                {
                    aSavedAttrsSet.Put(*pItem);
                    aClearWhichIds.push_back(aSavIds[n]);
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
                        "<lcl_RstAttr(..)> - certain attributes are kept, but not needed. -> please inform OD" );
                SfxItemIter aIter( *pPara->pDelSet );
                pItem = aIter.FirstItem();
                while( true )
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

void SwDoc::RstTxtAttrs(const SwPaM &rRg, bool bInclRefToxMark, bool bExactRange )
{
    SwHistory* pHst = 0;
    SwDataChanged aTmp( rRg );
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndoResetAttr* pUndo = new SwUndoResetAttr( rRg, RES_CHRFMT );
        pHst = &pUndo->GetHistory();
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }
    const SwPosition *pStt = rRg.Start(), *pEnd = rRg.End();
    sw::DocumentContentOperationsManager::ParaRstFmt aPara( pStt, pEnd, pHst );
    aPara.bInclRefToxMark = bInclRefToxMark;
    aPara.bExactRange = bExactRange;
    GetNodes().ForEach( pStt->nNode.GetIndex(), pEnd->nNode.GetIndex()+1,
                        sw::DocumentContentOperationsManager::lcl_RstTxtAttr, &aPara );
    getIDocumentState().SetModified();
}

void SwDoc::ResetAttrs( const SwPaM &rRg,
                        bool bTxtAttr,
                        const std::set<sal_uInt16> &rAttrs,
                        const bool bSendDataChangedEvents )
{
    SwPaM* pPam = const_cast<SwPaM*>(&rRg);
    if( !bTxtAttr && !rAttrs.empty() && RES_TXTATR_END > *(rAttrs.begin()) )
        bTxtAttr = true;

    if( !rRg.HasMark() )
    {
        SwTxtNode* pTxtNd = rRg.GetPoint()->nNode.GetNode().GetTxtNode();
        if( !pTxtNd )
            return ;

        pPam = new SwPaM( *rRg.GetPoint() );

        SwIndex& rSt = pPam->GetPoint()->nContent;
        sal_Int32 nMkPos, nPtPos = rSt.GetIndex();

        // Special case: if the Crsr is located within a URL attribute, we take over it's area
        SwTxtAttr const*const pURLAttr(
            pTxtNd->GetTxtAttrAt(rSt.GetIndex(), RES_TXTATR_INETFMT));
        if (pURLAttr && !pURLAttr->GetINetFmt().GetValue().isEmpty())
        {
            nMkPos = pURLAttr->GetStart();
            nPtPos = *pURLAttr->End();
        }
        else
        {
            Boundary aBndry;
            if( g_pBreakIt->GetBreakIter().is() )
                aBndry = g_pBreakIt->GetBreakIter()->getWordBoundary(
                            pTxtNd->GetTxt(), nPtPos,
                            g_pBreakIt->GetLocale( pTxtNd->GetLang( nPtPos ) ),
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
                if( bTxtAttr )
                    pTxtNd->DontExpandFmt( rSt, true );
            }
        }

        rSt = nMkPos;
        pPam->SetMark();
        pPam->GetPoint()->nContent = nPtPos;
    }

    // #i96644#
    boost::scoped_ptr< SwDataChanged > xDataChanged;
    if ( bSendDataChangedEvents )
    {
        xDataChanged.reset( new SwDataChanged( *pPam ) );
    }
    SwHistory* pHst = 0;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndoResetAttr* pUndo = new SwUndoResetAttr( rRg,
            static_cast<sal_uInt16>(bTxtAttr ? RES_CONDTXTFMTCOLL : RES_TXTFMTCOLL ));
        if( !rAttrs.empty() )
        {
            pUndo->SetAttrs( rAttrs );
        }
        pHst = &pUndo->GetHistory();
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    const SwPosition *pStt = pPam->Start(), *pEnd = pPam->End();
    sw::DocumentContentOperationsManager::ParaRstFmt aPara( pStt, pEnd, pHst );

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
        // set up a later, and all CharFmtAttr -> TxtFmtAttr
        SwTxtNode* pTNd = aTmpStt.GetNode().GetTxtNode();
        if( pTNd && pTNd->HasSwAttrSet() && pTNd->GetpSwAttrSet()->Count() )
        {
            if (pHst)
            {
                SwRegHistory history(pTNd, *pTNd, pHst);
                pTNd->FmtToTxtAttr(pTNd);
            }
            else
            {
                pTNd->FmtToTxtAttr(pTNd);
            }
        }

        ++aTmpStt;
    }
    if( pEnd->nContent.GetIndex() == pEnd->nNode.GetNode().GetCntntNode()->Len() )
         // set up a later, and all CharFmtAttr -> TxtFmtAttr
        ++aTmpEnd, bAdd = false;
    else if( pStt->nNode != pEnd->nNode || !pStt->nContent.GetIndex() )
    {
        SwTxtNode* pTNd = aTmpEnd.GetNode().GetTxtNode();
        if( pTNd && pTNd->HasSwAttrSet() && pTNd->GetpSwAttrSet()->Count() )
        {
            if (pHst)
            {
                SwRegHistory history(pTNd, *pTNd, pHst);
                pTNd->FmtToTxtAttr(pTNd);
            }
            else
            {
                pTNd->FmtToTxtAttr(pTNd);
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

    if( bTxtAttr )
    {
        if( bAdd )
            ++aTmpEnd;
        GetNodes().ForEach( pStt->nNode, aTmpEnd, sw::DocumentContentOperationsManager::lcl_RstTxtAttr, &aPara );
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

    SwTxtNode *pTxtNode = rRg.GetPoint()->nNode.GetNode().GetTxtNode();
    if (!pTxtNode)
    {
        return false;
    }
    const sal_Int32 nStart(rRg.GetPoint()->nContent.GetIndex() - nLen);
    SvxRsidItem aRsid( mnRsid, RES_CHRATR_RSID );

    SfxItemSet aSet(GetAttrPool(), RES_CHRATR_RSID, RES_CHRATR_RSID);
    aSet.Put(aRsid);
    bool const bRet(pTxtNode->SetAttr(aSet, nStart,
        rRg.GetPoint()->nContent.GetIndex(), SetAttrMode::DEFAULT));

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

bool SwDoc::UpdateParRsid( SwTxtNode *pTxtNode, sal_uInt32 nVal )
{
    if (!SW_MOD()->GetModuleConfig()->IsStoreRsid())
        return false;

    if (!pTxtNode)
    {
        return false;
    }

    SvxRsidItem aRsid( nVal ? nVal : mnRsid, RES_PARATR_RSID );
    return pTxtNode->SetAttr( aRsid );
}

/// Set the attribute according to the stated format.
/// If Undo is enabled, the old values is added to the Undo history.
void SwDoc::SetAttr( const SfxPoolItem& rAttr, SwFmt& rFmt )
{
    SfxItemSet aSet( GetAttrPool(), rAttr.Which(), rAttr.Which() );
    aSet.Put( rAttr );
    SetAttr( aSet, rFmt );
}

/// Set the attribute according to the stated format.
/// If Undo is enabled, the old values is added to the Undo history.
void SwDoc::SetAttr( const SfxItemSet& rSet, SwFmt& rFmt )
{
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndoFmtAttrHelper aTmp( rFmt );
        rFmt.SetFmtAttr( rSet );
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
        rFmt.SetFmtAttr( rSet );
    }
    getIDocumentState().SetModified();
}

void SwDoc::ResetAttrAtFormat( const sal_uInt16 nWhichId,
                               SwFmt& rChangedFormat )
{
    SwUndo *const pUndo = (GetIDocumentUndoRedo().DoesUndo())
        ?   new SwUndoFmtResetAttr( rChangedFormat, nWhichId )
        :   0;

    const bool bAttrReset = rChangedFormat.ResetFmtAttr( nWhichId );

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
    // We send a FmtChg to modify.

    sal_uInt16 nOldCnt = rChgTabStop.Count();
    if( !nOldCnt || nOldWidth == nNewWidth )
        return false;

    // Find the default's beginning
    sal_uInt16 n;
    for( n = nOldCnt; n ; --n )
        if( SVX_TAB_ADJUST_DEFAULT != rChgTabStop[n - 1].GetAdjustment() )
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

    SwModify aCallMod( 0 );
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
            aCallMod.Add( mpDfltTxtFmtColl );
            aCallMod.Add( mpDfltCharFmt );
            bCheckSdrDflt = 0 != pSdrPool;
        }
        else if ( isPARATR(nWhich) ||
                  isPARATR_LIST(nWhich) )
        {
            aCallMod.Add( mpDfltTxtFmtColl );
            bCheckSdrDflt = 0 != pSdrPool;
        }
        else if (isGRFATR(nWhich))
        {
            aCallMod.Add( mpDfltGrfFmtColl );
        }
        else if (isFRMATR(nWhich) || isDrawingLayerAttribute(nWhich) ) //UUUU
        {
            aCallMod.Add( mpDfltGrfFmtColl );
            aCallMod.Add( mpDfltTxtFmtColl );
            aCallMod.Add( mpDfltFrmFmt );
        }
        else if (isBOXATR(nWhich))
        {
            aCallMod.Add( mpDfltFrmFmt );
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
            GetIDocumentUndoRedo().AppendUndo( new SwUndoDefaultAttr( aOld ) );
        }

        const SfxPoolItem* pTmpItem;
        if( ( SfxItemState::SET ==
                aNew.GetItemState( RES_PARATR_TABSTOP, false, &pTmpItem ) ) &&
            static_cast<const SvxTabStopItem*>(pTmpItem)->Count() )
        {
            // Set the default values of all TabStops to the new value.
            // Attention: we always work with the PoolAttribut here, so that
            // we don't calculate the same value on the same TabStop (pooled!) for all sets.
            // We send a FmtChg to modify.
            SwTwips nNewWidth = (*static_cast<const SvxTabStopItem*>(pTmpItem))[ 0 ].GetTabPos(),
                    nOldWidth = static_cast<const SvxTabStopItem&>(aOld.Get(RES_PARATR_TABSTOP))[ 0 ].GetTabPos();

            bool bChg = false;
            sal_uInt32 nMaxItems = GetAttrPool().GetItemCount2( RES_PARATR_TABSTOP );
            for( sal_uInt32 n = 0; n < nMaxItems; ++n )
                if( 0 != (pTmpItem = GetAttrPool().GetItem2( RES_PARATR_TABSTOP, n ) ))
                    bChg |= lcl_SetNewDefTabStops( nOldWidth, nNewWidth,
                                                   *const_cast<SvxTabStopItem*>(static_cast<const SvxTabStopItem*>(pTmpItem)) );

            aNew.ClearItem( RES_PARATR_TABSTOP );
            aOld.ClearItem( RES_PARATR_TABSTOP );
            if( bChg )
            {
                SwFmtChg aChgFmt( mpDfltCharFmt );
                // notify the frames
                aCallMod.ModifyNotification( &aChgFmt, &aChgFmt );
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
const SfxPoolItem& SwDoc::GetDefault( sal_uInt16 nFmtHint ) const
{
    return GetAttrPool().GetDefaultItem( nFmtHint );
}

/// Delete the formats
void SwDoc::DelCharFmt(size_t nFmt, bool bBroadcast)
{
    SwCharFmt * pDel = (*mpCharFmtTbl)[nFmt];

    if (bBroadcast)
        BroadcastStyleOperation(pDel->GetName(), SFX_STYLE_FAMILY_CHAR,
                                SfxStyleSheetHintId::ERASED);

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndo * pUndo =
            new SwUndoCharFmtDelete(pDel, this);

        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    delete (*mpCharFmtTbl)[nFmt];
    mpCharFmtTbl->erase(mpCharFmtTbl->begin() + nFmt);

    getIDocumentState().SetModified();
}

void SwDoc::DelCharFmt( SwCharFmt *pFmt, bool bBroadcast )
{
    size_t nFmt = mpCharFmtTbl->GetPos( pFmt );
    OSL_ENSURE( SIZE_MAX != nFmt, "Fmt not found," );
    DelCharFmt( nFmt, bBroadcast );
}

void SwDoc::DelFrmFmt( SwFrmFmt *pFmt, bool bBroadcast )
{
    if( pFmt->ISA( SwTableBoxFmt ) || pFmt->ISA( SwTableLineFmt ))
    {
        OSL_ENSURE( false, "Format is not in the DocArray any more, "
                       "so it can be deleted with delete" );
        delete pFmt;
    }
    else
    {

        // The format has to be in the one or the other, we'll see in which one.
        SwFrmFmts::iterator it = std::find( mpFrmFmtTbl->begin(), mpFrmFmtTbl->end(), pFmt );
        if ( it != mpFrmFmtTbl->end() )
        {
            if (bBroadcast)
                BroadcastStyleOperation(pFmt->GetName(),
                                        SFX_STYLE_FAMILY_FRAME,
                                        SfxStyleSheetHintId::ERASED);

            if (GetIDocumentUndoRedo().DoesUndo())
            {
                SwUndo * pUndo = new SwUndoFrmFmtDelete(pFmt, this);

                GetIDocumentUndoRedo().AppendUndo(pUndo);
            }

            delete *it;
            mpFrmFmtTbl->erase(it);
        }
        else
        {
            SwFrmFmts::iterator it2 = std::find( GetSpzFrmFmts()->begin(), GetSpzFrmFmts()->end(), pFmt );
            OSL_ENSURE( it2 != GetSpzFrmFmts()->end(), "FrmFmt not found." );
            if( it2 != GetSpzFrmFmts()->end() )
            {
                delete *it2;
                GetSpzFrmFmts()->erase( it2 );
            }
        }
    }
}

void SwDoc::DelTblFrmFmt( SwTableFmt *pFmt )
{
    SwFrmFmts::iterator it = std::find( mpTblFrmFmtTbl->begin(), mpTblFrmFmtTbl->end(), pFmt );
    OSL_ENSURE( it != mpTblFrmFmtTbl->end(), "Fmt not found," );
    delete *it;
    mpTblFrmFmtTbl->erase(it);
}

/// Create the formats
SwFlyFrmFmt *SwDoc::MakeFlyFrmFmt( const OUString &rFmtName,
                                    SwFrmFmt *pDerivedFrom )
{
    SwFlyFrmFmt *pFmt = new SwFlyFrmFmt( GetAttrPool(), rFmtName, pDerivedFrom );
    GetSpzFrmFmts()->push_back(pFmt);
    getIDocumentState().SetModified();
    return pFmt;
}

SwDrawFrmFmt *SwDoc::MakeDrawFrmFmt( const OUString &rFmtName,
                                     SwFrmFmt *pDerivedFrom )
{
    SwDrawFrmFmt *pFmt = new SwDrawFrmFmt( GetAttrPool(), rFmtName, pDerivedFrom);
    GetSpzFrmFmts()->push_back(pFmt);
    getIDocumentState().SetModified();
    return pFmt;
}

size_t SwDoc::GetTblFrmFmtCount(bool bUsed) const
{
    size_t nCount = mpTblFrmFmtTbl->size();
    if(bUsed)
    {
        SwAutoFmtGetDocNode aGetHt( &GetNodes() );
        for ( size_t i = nCount; i; )
        {
            if((*mpTblFrmFmtTbl)[--i]->GetInfo( aGetHt ))
                --nCount;
        }
    }
    return nCount;
}

SwFrmFmt& SwDoc::GetTblFrmFmt(size_t nFmt, bool bUsed ) const
{
    size_t nRemoved = 0;
    if(bUsed)
    {
        SwAutoFmtGetDocNode aGetHt( &GetNodes() );
        for ( size_t i = 0; i <= nFmt; ++i )
        {
            while ( (*mpTblFrmFmtTbl)[ i + nRemoved]->GetInfo( aGetHt ))
            {
                nRemoved++;
            }
        }
    }
    return *((*mpTblFrmFmtTbl)[nRemoved + nFmt]);
}

SwTableFmt* SwDoc::MakeTblFrmFmt( const OUString &rFmtName,
                                    SwFrmFmt *pDerivedFrom )
{
    SwTableFmt* pFmt = new SwTableFmt( GetAttrPool(), rFmtName, pDerivedFrom );
    mpTblFrmFmtTbl->push_back( pFmt );
    getIDocumentState().SetModified();

    return pFmt;
}

SwFrmFmt *SwDoc::MakeFrmFmt(const OUString &rFmtName,
                            SwFrmFmt *pDerivedFrom,
                            bool bBroadcast, bool bAuto)
{
    SwFrmFmt *pFmt = new SwFrmFmt( GetAttrPool(), rFmtName, pDerivedFrom );

    pFmt->SetAuto(bAuto);
    mpFrmFmtTbl->push_back( pFmt );
    getIDocumentState().SetModified();

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndo * pUndo = new SwUndoFrmFmtCreate(pFmt, pDerivedFrom, this);

        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    if (bBroadcast)
    {
        BroadcastStyleOperation(rFmtName, SFX_STYLE_FAMILY_FRAME,
                                SfxStyleSheetHintId::CREATED);
    }

    return pFmt;
}

SwFmt *SwDoc::_MakeFrmFmt(const OUString &rFmtName,
                            SwFmt *pDerivedFrom,
                            bool bBroadcast, bool bAuto)
{
    SwFrmFmt *pFrmFmt = dynamic_cast<SwFrmFmt*>(pDerivedFrom);
    pFrmFmt = MakeFrmFmt( rFmtName, pFrmFmt, bBroadcast, bAuto );
    return dynamic_cast<SwFmt*>(pFrmFmt);
}

// #i40550# - add parameter <bAuto> - not relevant
SwCharFmt *SwDoc::MakeCharFmt( const OUString &rFmtName,
                               SwCharFmt *pDerivedFrom,
                               bool bBroadcast,
                               bool )
{
    SwCharFmt *pFmt = new SwCharFmt( GetAttrPool(), rFmtName, pDerivedFrom );
    mpCharFmtTbl->push_back( pFmt );
    pFmt->SetAuto( false );
    getIDocumentState().SetModified();

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndo * pUndo = new SwUndoCharFmtCreate(pFmt, pDerivedFrom, this);

        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    if (bBroadcast)
    {
        BroadcastStyleOperation(rFmtName, SFX_STYLE_FAMILY_CHAR,
                                SfxStyleSheetHintId::CREATED);
    }

    return pFmt;
}

SwFmt *SwDoc::_MakeCharFmt(const OUString &rFmtName,
                            SwFmt *pDerivedFrom,
                            bool bBroadcast, bool bAuto)
{
    SwCharFmt *pCharFmt = dynamic_cast<SwCharFmt*>(pDerivedFrom);
    pCharFmt = MakeCharFmt( rFmtName, pCharFmt, bBroadcast, bAuto );
    return dynamic_cast<SwFmt*>(pCharFmt);
}

/// Create the FormatCollections
SwTxtFmtColl* SwDoc::MakeTxtFmtColl( const OUString &rFmtName,
                                     SwTxtFmtColl *pDerivedFrom,
                                     bool bBroadcast,
                                     bool )
{
    SwTxtFmtColl *pFmtColl = new SwTxtFmtColl( GetAttrPool(), rFmtName,
                                                pDerivedFrom );
    mpTxtFmtCollTbl->push_back(pFmtColl);
    pFmtColl->SetAuto( false );
    getIDocumentState().SetModified();

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndo * pUndo = new SwUndoTxtFmtCollCreate(pFmtColl, pDerivedFrom,
                                                    this);
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    if (bBroadcast)
        BroadcastStyleOperation(rFmtName, SFX_STYLE_FAMILY_PARA,
                                SfxStyleSheetHintId::CREATED);

    return pFmtColl;
}

SwFmt *SwDoc::_MakeTxtFmtColl(const OUString &rFmtName,
                            SwFmt *pDerivedFrom,
                            bool bBroadcast, bool bAuto)
{
    SwTxtFmtColl *pTxtFmtColl = dynamic_cast<SwTxtFmtColl*>(pDerivedFrom);
    pTxtFmtColl = MakeTxtFmtColl( rFmtName, pTxtFmtColl, bBroadcast, bAuto );
    return dynamic_cast<SwFmt*>(pTxtFmtColl);
}

//FEATURE::CONDCOLL
SwConditionTxtFmtColl* SwDoc::MakeCondTxtFmtColl( const OUString &rFmtName,
                                                  SwTxtFmtColl *pDerivedFrom,
                                                  bool bBroadcast)
{
    SwConditionTxtFmtColl*pFmtColl = new SwConditionTxtFmtColl( GetAttrPool(),
                                                    rFmtName, pDerivedFrom );
    mpTxtFmtCollTbl->push_back(pFmtColl);
    pFmtColl->SetAuto( false );
    getIDocumentState().SetModified();

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndo * pUndo = new SwUndoCondTxtFmtCollCreate(pFmtColl, pDerivedFrom,
                                                        this);
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    if (bBroadcast)
        BroadcastStyleOperation(rFmtName, SFX_STYLE_FAMILY_PARA,
                                SfxStyleSheetHintId::CREATED);

    return pFmtColl;
}
//FEATURE::CONDCOLL

// GRF
SwGrfFmtColl* SwDoc::MakeGrfFmtColl( const OUString &rFmtName,
                                     SwGrfFmtColl *pDerivedFrom )
{
    SwGrfFmtColl *pFmtColl = new SwGrfFmtColl( GetAttrPool(), rFmtName,
                                                pDerivedFrom );
    mpGrfFmtCollTbl->push_back( pFmtColl );
    pFmtColl->SetAuto( false );
    getIDocumentState().SetModified();
    return pFmtColl;
}

void SwDoc::DelTxtFmtColl(size_t nFmtColl, bool bBroadcast)
{
    OSL_ENSURE( nFmtColl, "Remove fuer Coll 0." );

    // Who has the to-be-deleted as their Next?
    SwTxtFmtColl *pDel = (*mpTxtFmtCollTbl)[nFmtColl];
    if( mpDfltTxtFmtColl == pDel )
        return;     // never delete default!

    if (bBroadcast)
        BroadcastStyleOperation(pDel->GetName(), SFX_STYLE_FAMILY_PARA,
                                SfxStyleSheetHintId::ERASED);

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndoTxtFmtCollDelete * pUndo;
        if (RES_CONDTXTFMTCOLL == pDel->Which())
        {
            pUndo = new SwUndoCondTxtFmtCollDelete(pDel, this);
        }
        else
        {
            pUndo = new SwUndoTxtFmtCollDelete(pDel, this);
        }

        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    // Remove the FmtColl
    mpTxtFmtCollTbl->erase(mpTxtFmtCollTbl->begin() + nFmtColl);
    // Correct next
    for( SwTxtFmtColls::const_iterator it = mpTxtFmtCollTbl->begin() + 1; it != mpTxtFmtCollTbl->end(); ++it )
        SetTxtFmtCollNext( *it, pDel );
    delete pDel;
    getIDocumentState().SetModified();
}

void SwDoc::DelTxtFmtColl( SwTxtFmtColl *pColl, bool bBroadcast )
{
    size_t nFmt = mpTxtFmtCollTbl->GetPos( pColl );
    OSL_ENSURE( SIZE_MAX != nFmt, "Collection not found," );
    DelTxtFmtColl( nFmt, bBroadcast );
}

static bool lcl_SetTxtFmtColl( const SwNodePtr& rpNode, void* pArgs )
{
    SwCntntNode* pCNd = static_cast<SwCntntNode*>(rpNode->GetTxtNode());

    if( pCNd == NULL)
        return true;

    sw::DocumentContentOperationsManager::ParaRstFmt* pPara = static_cast<sw::DocumentContentOperationsManager::ParaRstFmt*>(pArgs);

    SwTxtFmtColl* pFmt = static_cast<SwTxtFmtColl*>(pPara->pFmtColl);
    if ( pPara->bReset )
    {
        lcl_RstAttr(pCNd, pPara);

        // #i62675# check, if paragraph style has changed
        if ( pPara->bResetListAttrs &&
             pFmt != pCNd->GetFmtColl() &&
             pFmt->GetItemState( RES_PARATR_NUMRULE ) == SfxItemState::SET )
        {
            // Check, if the list style of the paragraph will change.
            bool bChangeOfListStyleAtParagraph( true );
            SwTxtNode* pTNd( dynamic_cast<SwTxtNode*>(pCNd) );
            OSL_ENSURE( pTNd, "<lcl_SetTxtFmtColl(..)> - text node expected -> crash" );
            {
                SwNumRule* pNumRuleAtParagraph( pTNd->GetNumRule() );
                if ( pNumRuleAtParagraph )
                {
                    const SwNumRuleItem& rNumRuleItemAtParagraphStyle =
                        pFmt->GetNumRule();
                    if ( rNumRuleItemAtParagraphStyle.GetValue() ==
                            pNumRuleAtParagraph->GetName() )
                    {
                        bChangeOfListStyleAtParagraph = false;
                    }
                }
            }

            if ( bChangeOfListStyleAtParagraph )
            {
                boost::scoped_ptr< SwRegHistory > pRegH;
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
        pPara->pHistory->Add( pCNd->GetFmtColl(), pCNd->GetIndex(),
                ND_TEXTNODE );

    pCNd->ChgFmtColl( pFmt );

    pPara->nWhich++;

    return true;
}

bool SwDoc::SetTxtFmtColl(const SwPaM &rRg,
                          SwTxtFmtColl *pFmt,
                          const bool bReset,
                          const bool bResetListAttrs)
{
    SwDataChanged aTmp( rRg );
    const SwPosition *pStt = rRg.Start(), *pEnd = rRg.End();
    SwHistory* pHst = 0;
    bool bRet = true;

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndoFmtColl* pUndo = new SwUndoFmtColl( rRg, pFmt,
                                                  bReset,
                                                  bResetListAttrs );
        pHst = pUndo->GetHistory();
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    sw::DocumentContentOperationsManager::ParaRstFmt aPara( pStt, pEnd, pHst );
    aPara.pFmtColl = pFmt;
    aPara.bReset = bReset;
    // #i62675#
    aPara.bResetListAttrs = bResetListAttrs;

    GetNodes().ForEach( pStt->nNode.GetIndex(), pEnd->nNode.GetIndex()+1,
                        lcl_SetTxtFmtColl, &aPara );
    if( !aPara.nWhich )
        bRet = false;           // didn't find a valid Node

    if (bRet)
    {
        getIDocumentState().SetModified();
    }

    return bRet;
}

/// Copy the formats to itself
SwFmt* SwDoc::CopyFmt( const SwFmt& rFmt,
                        const SwFmtsBase& rFmtArr,
                        FNCopyFmt fnCopyFmt, const SwFmt& rDfltFmt )
{
    // It's no autoformat, default format or collection format,
    // then search for it.
    if( !rFmt.IsAuto() || !rFmt.GetRegisteredIn() )
        for( size_t n = 0; n < rFmtArr.GetFmtCount(); ++n )
        {
            // Does the Doc already contain the template?
            if( rFmtArr.GetFmt(n)->GetName()==rFmt.GetName() )
                return rFmtArr.GetFmt(n);
        }

    // Search for the "parent" first
    SwFmt* pParent = const_cast<SwFmt*>(&rDfltFmt);
    if( rFmt.DerivedFrom() && pParent != rFmt.DerivedFrom() )
        pParent = CopyFmt( *rFmt.DerivedFrom(), rFmtArr,
                                fnCopyFmt, rDfltFmt );

    // Create the format and copy the attributes
    // #i40550#
    SwFmt* pNewFmt = (this->*fnCopyFmt)( rFmt.GetName(), pParent, false, true );
    pNewFmt->SetAuto( rFmt.IsAuto() );
    pNewFmt->CopyAttrs( rFmt, true );           // copy the attributes

    pNewFmt->SetPoolFmtId( rFmt.GetPoolFmtId() );
    pNewFmt->SetPoolHelpId( rFmt.GetPoolHelpId() );

    // Always set the HelpFile Id to default!
    pNewFmt->SetPoolHlpFileId( UCHAR_MAX );

    return pNewFmt;
}

/// copy the frame format
SwFrmFmt* SwDoc::CopyFrmFmt( const SwFrmFmt& rFmt )
{
    return static_cast<SwFrmFmt*>(CopyFmt( rFmt, *GetFrmFmts(), &SwDoc::_MakeFrmFmt,
                                *GetDfltFrmFmt() ));
}

/// copy the char format
SwCharFmt* SwDoc::CopyCharFmt( const SwCharFmt& rFmt )
{
    return static_cast<SwCharFmt*>(CopyFmt( rFmt, *GetCharFmts(),
                                            &SwDoc::_MakeCharFmt,
                                            *GetDfltCharFmt() ));
}

/// copy TextNodes
SwTxtFmtColl* SwDoc::CopyTxtColl( const SwTxtFmtColl& rColl )
{
    SwTxtFmtColl* pNewColl = FindTxtFmtCollByName( rColl.GetName() );
    if( pNewColl )
        return pNewColl;

    // search for the "parent" first
    SwTxtFmtColl* pParent = mpDfltTxtFmtColl;
    if( pParent != rColl.DerivedFrom() )
        pParent = CopyTxtColl( *static_cast<SwTxtFmtColl*>(rColl.DerivedFrom()) );

//FEATURE::CONDCOLL
    if( RES_CONDTXTFMTCOLL == rColl.Which() )
    {
        pNewColl = new SwConditionTxtFmtColl( GetAttrPool(), rColl.GetName(),
                                                pParent);
        mpTxtFmtCollTbl->push_back( pNewColl );
        pNewColl->SetAuto( false );
        getIDocumentState().SetModified();

        // copy the conditions
        static_cast<SwConditionTxtFmtColl*>(pNewColl)->SetConditions(
                            static_cast<const SwConditionTxtFmtColl&>(rColl).GetCondColls() );
    }
    else
//FEATURE::CONDCOLL
        pNewColl = MakeTxtFmtColl( rColl.GetName(), pParent );

    // copy the auto formats or the attributes
    pNewColl->CopyAttrs( rColl, true );

    if(rColl.IsAssignedToListLevelOfOutlineStyle())
        pNewColl->AssignToListLevelOfOutlineStyle(rColl.GetAssignedOutlineStyleLevel());
    pNewColl->SetPoolFmtId( rColl.GetPoolFmtId() );
    pNewColl->SetPoolHelpId( rColl.GetPoolHelpId() );

    // Always set the HelpFile Id to default!
    pNewColl->SetPoolHlpFileId( UCHAR_MAX );

    if( &rColl.GetNextTxtFmtColl() != &rColl )
        pNewColl->SetNextTxtFmtColl( *CopyTxtColl( rColl.GetNextTxtFmtColl() ));

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
                0 != ( pRule = rColl.GetDoc()->FindNumRulePtr( rName )) &&
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
SwGrfFmtColl* SwDoc::CopyGrfColl( const SwGrfFmtColl& rColl )
{
    SwGrfFmtColl* pNewColl = FindGrfFmtCollByName( rColl.GetName() );
    if( pNewColl )
        return pNewColl;

     // Search for the "parent" first
    SwGrfFmtColl* pParent = mpDfltGrfFmtColl;
    if( pParent != rColl.DerivedFrom() )
        pParent = CopyGrfColl( *static_cast<SwGrfFmtColl*>(rColl.DerivedFrom()) );

    // if not, copy them
    pNewColl = MakeGrfFmtColl( rColl.GetName(), pParent );

    // copy the attributes
    pNewColl->CopyAttrs( rColl );

    pNewColl->SetPoolFmtId( rColl.GetPoolFmtId() );
    pNewColl->SetPoolHelpId( rColl.GetPoolHelpId() );

    // Always set the HelpFile Id to default!
    pNewColl->SetPoolHlpFileId( UCHAR_MAX );

    return pNewColl;
}

void SwDoc::CopyFmtArr( const SwFmtsBase& rSourceArr,
                        SwFmtsBase& rDestArr,
                        FNCopyFmt fnCopyFmt,
                        SwFmt& rDfltFmt )
{
    SwFmt* pSrc, *pDest;

    // 1st step: Create all formats (skip the 0th - it's the default one)
    for( size_t nSrc = rSourceArr.GetFmtCount(); nSrc > 1; )
    {
        pSrc = rSourceArr.GetFmt( --nSrc );
        if( pSrc->IsDefault() || pSrc->IsAuto() )
            continue;

        if( 0 == FindFmtByName( rDestArr, pSrc->GetName() ) )
        {
            if( RES_CONDTXTFMTCOLL == pSrc->Which() )
                MakeCondTxtFmtColl( pSrc->GetName(), static_cast<SwTxtFmtColl*>(&rDfltFmt) );
            else
                // #i40550#
                (this->*fnCopyFmt)( pSrc->GetName(), &rDfltFmt, false, true );
        }
    }

    // 2nd step: Copy all attributes, set the right parents
    for( size_t nSrc = rSourceArr.GetFmtCount(); nSrc > 1; )
    {
        pSrc = rSourceArr.GetFmt( --nSrc );
        if( pSrc->IsDefault() || pSrc->IsAuto() )
            continue;

        pDest = FindFmtByName( rDestArr, pSrc->GetName() );
        pDest->SetAuto( false );
        pDest->DelDiffs( *pSrc );

        // #i94285#: existing <SwFmtPageDesc> instance, before copying attributes
        const SfxPoolItem* pItem;
        if( &GetAttrPool() != pSrc->GetAttrSet().GetPool() &&
            SfxItemState::SET == pSrc->GetAttrSet().GetItemState(
            RES_PAGEDESC, false, &pItem ) &&
            static_cast<const SwFmtPageDesc*>(pItem)->GetPageDesc() )
        {
            SwFmtPageDesc aPageDesc( *static_cast<const SwFmtPageDesc*>(pItem) );
            const OUString& rNm = aPageDesc.GetPageDesc()->GetName();
            SwPageDesc* pPageDesc = FindPageDesc( rNm );
            if( !pPageDesc )
            {
                pPageDesc = MakePageDesc(rNm);
            }
            aPageDesc.RegisterToPageDesc( *pPageDesc );
            SwAttrSet aTmpAttrSet( pSrc->GetAttrSet() );
            aTmpAttrSet.Put( aPageDesc );
            pDest->SetFmtAttr( aTmpAttrSet );
        }
        else
        {
            pDest->SetFmtAttr( pSrc->GetAttrSet() );
        }

        pDest->SetPoolFmtId( pSrc->GetPoolFmtId() );
        pDest->SetPoolHelpId( pSrc->GetPoolHelpId() );

        // Always set the HelpFile Id to default!
        pDest->SetPoolHlpFileId( UCHAR_MAX );

        if( pSrc->DerivedFrom() )
            pDest->SetDerivedFrom( FindFmtByName( rDestArr,
                                        pSrc->DerivedFrom()->GetName() ) );
        if( RES_TXTFMTCOLL == pSrc->Which() ||
            RES_CONDTXTFMTCOLL == pSrc->Which() )
        {
            SwTxtFmtColl* pSrcColl = static_cast<SwTxtFmtColl*>(pSrc),
                        * pDstColl = static_cast<SwTxtFmtColl*>(pDest);
            if( &pSrcColl->GetNextTxtFmtColl() != pSrcColl )
                pDstColl->SetNextTxtFmtColl( *static_cast<SwTxtFmtColl*>(FindFmtByName(
                    rDestArr, pSrcColl->GetNextTxtFmtColl().GetName() ) ) );

            if(pSrcColl->IsAssignedToListLevelOfOutlineStyle())
                pDstColl->AssignToListLevelOfOutlineStyle(pSrcColl->GetAssignedOutlineStyleLevel());

//FEATURE::CONDCOLL
            if( RES_CONDTXTFMTCOLL == pSrc->Which() )
                // Copy the conditions, but delete the old ones first!
                static_cast<SwConditionTxtFmtColl*>(pDstColl)->SetConditions(
                            static_cast<SwConditionTxtFmtColl*>(pSrc)->GetCondColls() );
//FEATURE::CONDCOLL
        }
    }
}

void SwDoc::CopyPageDescHeaderFooterImpl( bool bCpyHeader,
                                const SwFrmFmt& rSrcFmt, SwFrmFmt& rDestFmt )
{
    // Treat the header and footer attributes in the right way:
    // Copy content nodes across documents!
    sal_uInt16 nAttr = static_cast<sal_uInt16>( bCpyHeader ? RES_HEADER : RES_FOOTER );
    const SfxPoolItem* pItem;
    if( SfxItemState::SET != rSrcFmt.GetAttrSet().GetItemState( nAttr, false, &pItem ))
        return ;

    // The header only contains the reference to the format from the other document!
    SfxPoolItem* pNewItem = pItem->Clone();

    SwFrmFmt* pOldFmt;
    if( bCpyHeader )
         pOldFmt = static_cast<SwFmtHeader*>(pNewItem)->GetHeaderFmt();
    else
         pOldFmt = static_cast<SwFmtFooter*>(pNewItem)->GetFooterFmt();

    if( pOldFmt )
    {
        SwFrmFmt* pNewFmt = new SwFrmFmt( GetAttrPool(), "CpyDesc",
                                            GetDfltFrmFmt() );
        pNewFmt->CopyAttrs( *pOldFmt, true );

        if( SfxItemState::SET == pNewFmt->GetAttrSet().GetItemState(
            RES_CNTNT, false, &pItem ))
        {
            const SwFmtCntnt* pCntnt = static_cast<const SwFmtCntnt*>(pItem);
            if( pCntnt->GetCntntIdx() )
            {
                SwNodeIndex aTmpIdx( GetNodes().GetEndOfAutotext() );
                const SwNodes& rSrcNds = rSrcFmt.GetDoc()->GetNodes();
                SwStartNode* pSttNd = SwNodes::MakeEmptySection( aTmpIdx,
                                                bCpyHeader
                                                    ? SwHeaderStartNode
                                                    : SwFooterStartNode );
                const SwNode& rCSttNd = pCntnt->GetCntntIdx()->GetNode();
                SwNodeRange aRg( rCSttNd, 0, *rCSttNd.EndOfSectionNode() );
                aTmpIdx = *pSttNd->EndOfSectionNode();
                rSrcNds._Copy( aRg, aTmpIdx );
                aTmpIdx = *pSttNd;
                rSrcFmt.GetDoc()->GetDocumentContentOperationsManager().CopyFlyInFlyImpl( aRg, 0, aTmpIdx );
                pNewFmt->SetFmtAttr( SwFmtCntnt( pSttNd ));
            }
            else
                pNewFmt->ResetFmtAttr( RES_CNTNT );
        }
        if( bCpyHeader )
            static_cast<SwFmtHeader*>(pNewItem)->RegisterToFormat(*pNewFmt);
        else
            static_cast<SwFmtFooter*>(pNewItem)->RegisterToFormat(*pNewFmt);
        rDestFmt.SetFmtAttr( *pNewItem );
    }
    delete pNewItem;
}

void SwDoc::CopyPageDesc( const SwPageDesc& rSrcDesc, SwPageDesc& rDstDesc,
                            bool bCopyPoolIds )
{
    bool bNotifyLayout = false;
    SwRootFrm* pTmpRoot = getIDocumentLayoutAccess().GetCurrentLayout();

    rDstDesc.SetLandscape( rSrcDesc.GetLandscape() );
    rDstDesc.SetNumType( rSrcDesc.GetNumType() );
    if( rDstDesc.ReadUseOn() != rSrcDesc.ReadUseOn() )
    {
        rDstDesc.WriteUseOn( rSrcDesc.ReadUseOn() );
        bNotifyLayout = true;
    }

    if( bCopyPoolIds )
    {
        rDstDesc.SetPoolFmtId( rSrcDesc.GetPoolFmtId() );
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
        rDstDesc.GetMaster().SetFmtAttr( aAttrSet );

        aAttrSet.ClearItem();
        aAttrSet.Put( rSrcDesc.GetLeft().GetAttrSet() );
        aAttrSet.ClearItem( RES_HEADER );
        aAttrSet.ClearItem( RES_FOOTER );

        rDstDesc.GetLeft().DelDiffs( aAttrSet );
        rDstDesc.GetLeft().SetFmtAttr( aAttrSet );

        aAttrSet.ClearItem();
        aAttrSet.Put( rSrcDesc.GetFirstMaster().GetAttrSet() );
        aAttrSet.ClearItem( RES_HEADER );
        aAttrSet.ClearItem( RES_FOOTER );

        rDstDesc.GetFirstMaster().DelDiffs( aAttrSet );
        rDstDesc.GetFirstMaster().SetFmtAttr( aAttrSet );

        aAttrSet.ClearItem();
        aAttrSet.Put( rSrcDesc.GetFirstLeft().GetAttrSet() );
        aAttrSet.ClearItem( RES_HEADER );
        aAttrSet.ClearItem( RES_FOOTER );

        rDstDesc.GetFirstLeft().DelDiffs( aAttrSet );
        rDstDesc.GetFirstLeft().SetFmtAttr( aAttrSet );
    }

    CopyHeader( rSrcDesc.GetMaster(), rDstDesc.GetMaster() );
    CopyFooter( rSrcDesc.GetMaster(), rDstDesc.GetMaster() );
    if( !rDstDesc.IsHeaderShared() )
        CopyHeader( rSrcDesc.GetLeft(), rDstDesc.GetLeft() );
    else
        rDstDesc.GetLeft().SetFmtAttr( rDstDesc.GetMaster().GetHeader() );
    if( !rDstDesc.IsFirstShared() )
    {
        CopyHeader( rSrcDesc.GetFirstMaster(), rDstDesc.GetFirstMaster() );
        rDstDesc.GetFirstLeft().SetFmtAttr(rDstDesc.GetFirstMaster().GetHeader());
    }
    else
    {
        rDstDesc.GetFirstMaster().SetFmtAttr( rDstDesc.GetMaster().GetHeader() );
        rDstDesc.GetFirstLeft().SetFmtAttr(rDstDesc.GetLeft().GetHeader());
    }

    if( !rDstDesc.IsFooterShared() )
        CopyFooter( rSrcDesc.GetLeft(), rDstDesc.GetLeft() );
    else
        rDstDesc.GetLeft().SetFmtAttr( rDstDesc.GetMaster().GetFooter() );
    if( !rDstDesc.IsFirstShared() )
    {
        CopyFooter( rSrcDesc.GetFirstMaster(), rDstDesc.GetFirstMaster() );
        rDstDesc.GetFirstLeft().SetFmtAttr(rDstDesc.GetFirstMaster().GetFooter());
    }
    else
    {
        rDstDesc.GetFirstMaster().SetFmtAttr( rDstDesc.GetMaster().GetFooter() );
        rDstDesc.GetFirstLeft().SetFmtAttr(rDstDesc.GetLeft().GetFooter());
    }

    if( bNotifyLayout && pTmpRoot )
    {
        std::set<SwRootFrm*> aAllLayouts = GetAllLayouts();
        std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::mem_fun(&SwRootFrm::AllCheckPageDescs));
    }

    // If foot notes change the pages have to be triggered
    if( !(rDstDesc.GetFtnInfo() == rSrcDesc.GetFtnInfo()) )
    {
        sw::PageFootnoteHint aHint;
        rDstDesc.SetFtnInfo( rSrcDesc.GetFtnInfo() );
        rDstDesc.GetMaster().CallSwClientNotify(aHint);
        rDstDesc.GetLeft().CallSwClientNotify(aHint);
        rDstDesc.GetFirstMaster().CallSwClientNotify(aHint);
        rDstDesc.GetFirstLeft().CallSwClientNotify(aHint);
    }
}

void SwDoc::ReplaceStyles( const SwDoc& rSource, bool bIncludePageStyles )
{
    ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

    CopyFmtArr( *rSource.mpCharFmtTbl, *mpCharFmtTbl,
                &SwDoc::_MakeCharFmt, *mpDfltCharFmt );
    CopyFmtArr( *rSource.mpFrmFmtTbl, *mpFrmFmtTbl,
                &SwDoc::_MakeFrmFmt, *mpDfltFrmFmt );
    CopyFmtArr( *rSource.mpTxtFmtCollTbl, *mpTxtFmtCollTbl,
                &SwDoc::_MakeTxtFmtColl, *mpDfltTxtFmtColl );

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
        SwPageDescs::size_type nCnt = rSource.maPageDescs.size();
        if( nCnt )
        {
            // a different Doc -> Number formatter needs to be merged
            SwTblNumFmtMerge aTNFM( rSource, *this );

            // 1st step: Create all formats (skip the 0th - it's the default!)
            while( nCnt )
            {
                const SwPageDesc &rSrc = rSource.maPageDescs[ --nCnt ];
                if( 0 == FindPageDesc( rSrc.GetName() ) )
                    MakePageDesc( rSrc.GetName() );
            }

            // 2nd step: Copy all attributes, set the right parents
            for( nCnt = rSource.maPageDescs.size(); nCnt; )
            {
                const SwPageDesc &rSrc = rSource.maPageDescs[ --nCnt ];
                SwPageDesc* pDesc = FindPageDesc( rSrc.GetName() );
                CopyPageDesc( rSrc, *pDesc);
            }
        }
    }

    // then there are the numbering templates
    const SwPageDescs::size_type nCnt = rSource.GetNumRuleTbl().size();
    if( nCnt )
    {
        const SwNumRuleTbl& rArr = rSource.GetNumRuleTbl();
        for( SwPageDescs::size_type n = 0; n < nCnt; ++n )
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
                    SAL_WARN( "sw.styles", "Found unknown auto SwNumRule during reset!" );
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

SwFmt* SwDoc::FindFmtByName( const SwFmtsBase& rFmtArr,
                             const OUString& rName )
{
    SwFmt* pFnd = 0;
    for( size_t n = 0; n < rFmtArr.GetFmtCount(); ++n )
    {
        // Does the Doc already contain the template?
        if( rFmtArr.GetFmt(n)->GetName() == rName )
        {
            pFnd = rFmtArr.GetFmt(n);
            break;
        }
    }
    return pFnd;
}

void SwDoc::MoveLeftMargin( const SwPaM& rPam, bool bRight, bool bModulus )
{
    SwHistory* pHistory = 0;
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
        SwTxtNode* pTNd = aIdx.GetNode().GetTxtNode();
        if( pTNd )
        {
            SvxLRSpaceItem aLS( static_cast<const SvxLRSpaceItem&>(pTNd->SwCntntNode::GetAttr( RES_LR_SPACE )) );

            // #i93873# See also lcl_MergeListLevelIndentAsLRSpaceItem in thints.cxx
            if ( pTNd->AreListLevelIndentsApplicable() )
            {
                const SwNumRule* pRule = pTNd->GetNumRule();
                if ( pRule )
                {
                    const int nListLevel = pTNd->GetActualListLevel();
                    if ( nListLevel >= 0 )
                    {
                        const SwNumFmt& rFmt = pRule->Get(static_cast<sal_uInt16>(nListLevel));
                        if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
                        {
                            aLS.SetTxtLeft( rFmt.GetIndentAt() );
                            aLS.SetTxtFirstLineOfst( static_cast<short>(rFmt.GetFirstLineIndent()) );
                        }
                    }
                }
            }

            long nNext = aLS.GetTxtLeft();
            if( bModulus )
                nNext = ( nNext / nDefDist ) * nDefDist;

            if( bRight )
                nNext += nDefDist;
            else
                if(nNext >0) // fdo#75936 set limit for decreasing indent
                    nNext -= nDefDist;

            aLS.SetTxtLeft( nNext );

            SwRegHistory aRegH( pTNd, *pTNd, pHistory );
            pTNd->SetAttr( aLS );
        }
        ++aIdx;
    }
    getIDocumentState().SetModified();
}

bool SwDoc::DontExpandFmt( const SwPosition& rPos, bool bFlag )
{
    bool bRet = false;
    SwTxtNode* pTxtNd = rPos.nNode.GetNode().GetTxtNode();
    if( pTxtNd )
    {
        bRet = pTxtNd->DontExpandFmt( rPos.nContent, bFlag );
        if( bRet && GetIDocumentUndoRedo().DoesUndo() )
        {
            GetIDocumentUndoRedo().AppendUndo( new SwUndoDontExpandFmt(rPos) );
        }
    }
    return bRet;
}

SwTableBoxFmt* SwDoc::MakeTableBoxFmt()
{
    SwTableBoxFmt* pFmt = new SwTableBoxFmt( GetAttrPool(), aEmptyOUStr,
                                                mpDfltFrmFmt );
    getIDocumentState().SetModified();
    return pFmt;
}

SwTableLineFmt* SwDoc::MakeTableLineFmt()
{
    SwTableLineFmt* pFmt = new SwTableLineFmt( GetAttrPool(), aEmptyOUStr,
                                                mpDfltFrmFmt );
    getIDocumentState().SetModified();
    return pFmt;
}

void SwDoc::_CreateNumberFormatter()
{
    OSL_ENSURE( !mpNumberFormatter, "is already there" );

    LanguageType eLang = LANGUAGE_SYSTEM;

    mpNumberFormatter = new SvNumberFormatter( comphelper::getProcessComponentContext(), eLang );
    mpNumberFormatter->SetEvalDateFormat( NF_EVALDATEFORMAT_FORMAT_INTL );
    mpNumberFormatter->SetYear2000(static_cast<sal_uInt16>(::utl::MiscCfg().GetYear2000()));
}

SwTblNumFmtMerge::SwTblNumFmtMerge( const SwDoc& rSrc, SwDoc& rDest )
    : pNFmt( 0 )
{
    // a different Doc -> Number formatter needs to be merged
    SvNumberFormatter* pN;
    if( &rSrc != &rDest && 0 != ( pN = const_cast<SwDoc&>(rSrc).GetNumberFormatter( false ) ))
        ( pNFmt = rDest.GetNumberFormatter( true ))->MergeFormatter( *pN );

    if( &rSrc != &rDest )
        static_cast<SwGetRefFieldType*>(rSrc.getIDocumentFieldsAccess().GetSysFldType( RES_GETREFFLD ))->
            MergeWithOtherDoc( rDest );
}

SwTblNumFmtMerge::~SwTblNumFmtMerge()
{
    if( pNFmt )
        pNFmt->ClearMergeTable();
}

void SwDoc::SetTxtFmtCollByAutoFmt( const SwPosition& rPos, sal_uInt16 nPoolId,
                                    const SfxItemSet* pSet )
{
    SwPaM aPam( rPos );
    SwTxtNode* pTNd = rPos.nNode.GetNode().GetTxtNode();
    assert(pTNd);

    if (mbIsAutoFmtRedline)
    {
        // create the redline object
        const SwTxtFmtColl& rColl = *pTNd->GetTxtColl();
        SwRangeRedline* pRedl = new SwRangeRedline( nsRedlineType_t::REDLINE_FMTCOLL, aPam );
        pRedl->SetMark();

        // Only those items that are not set by the Set again in the Node
        // are of interest. Thus, we take the difference.
        SwRedlineExtraData_FmtColl aExtraData( rColl.GetName(),
                                                rColl.GetPoolFmtId() );
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

    SetTxtFmtColl( aPam, getIDocumentStylePoolAccess().GetTxtCollFromPool( nPoolId ) );

    if (pSet && pSet->Count())
    {
        aPam.SetMark();
        aPam.GetMark()->nContent.Assign(pTNd, pTNd->GetTxt().getLength());
        getIDocumentContentOperations().InsertItemSet( aPam, *pSet );
    }
}

void SwDoc::SetFmtItemByAutoFmt( const SwPaM& rPam, const SfxItemSet& rSet )
{
    SwTxtNode* pTNd = rPam.GetPoint()->nNode.GetNode().GetTxtNode();
    assert(pTNd);

    RedlineMode_t eOld = getIDocumentRedlineAccess().GetRedlineMode();

    if (mbIsAutoFmtRedline)
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

        getIDocumentRedlineAccess().SetRedlineMode_intern( (RedlineMode_t)(eOld | nsRedlineMode_t::REDLINE_IGNORE));
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
    pTNd->GetAttr(currentSet, nEnd, nEnd, false, true, false);
    for (size_t i = 0; whichIds[i]; i += 2)
    {   // yuk - want to explicitly set the pool defaults too :-/
        currentSet.Put(currentSet.Get(whichIds[i], true));
    }

    getIDocumentContentOperations().InsertItemSet( rPam, rSet, SetAttrMode::DONTEXPAND );

    // fdo#62536: DONTEXPAND does not work when there is already an AUTOFMT
    // here, so insert the old attributes as an empty hint to stop expand
    SwPaM endPam(*pTNd, nEnd);
    endPam.SetMark();
    getIDocumentContentOperations().InsertItemSet(endPam, currentSet, SetAttrMode::DEFAULT);

    getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
}

void SwDoc::ChgFmt(SwFmt & rFmt, const SfxItemSet & rSet)
{
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        // copying <rSet> to <aSet>
        SfxItemSet aSet(rSet);
        // remove from <aSet> all items, which are already set at the format
        aSet.Differentiate(rFmt.GetAttrSet());
        // <aSet> contains now all *new* items for the format

        // copying current format item set to <aOldSet>
        SfxItemSet aOldSet(rFmt.GetAttrSet());
        // insert new items into <aOldSet>
        aOldSet.Put(aSet);
        // invalidate all new items in <aOldSet> in order to clear these items,
        // if the undo action is triggered.
        {
            SfxItemIter aIter(aSet);

            const SfxPoolItem * pItem = aIter.FirstItem();
            while (pItem != NULL)
            {
                aOldSet.InvalidateItem(pItem->Which());

                pItem = aIter.NextItem();
            }
        }

        SwUndo * pUndo = new SwUndoFmtAttr(aOldSet, rFmt);

        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    rFmt.SetFmtAttr(rSet);
}

void SwDoc::RenameFmt(SwFmt & rFmt, const OUString & sNewName,
                      bool bBroadcast)
{
    SfxStyleFamily eFamily = SFX_STYLE_FAMILY_ALL;

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndo * pUndo = NULL;

        switch (rFmt.Which())
        {
        case RES_CHRFMT:
            pUndo = new SwUndoRenameCharFmt(rFmt.GetName(), sNewName, this);
            eFamily = SFX_STYLE_FAMILY_CHAR;
            break;
        case RES_TXTFMTCOLL:
            pUndo = new SwUndoRenameFmtColl(rFmt.GetName(), sNewName, this);
            eFamily = SFX_STYLE_FAMILY_PARA;
            break;
        case RES_FRMFMT:
            pUndo = new SwUndoRenameFrmFmt(rFmt.GetName(), sNewName, this);
            eFamily = SFX_STYLE_FAMILY_FRAME;
            break;

        default:
            break;
        }

        if (pUndo)
        {
            GetIDocumentUndoRedo().AppendUndo(pUndo);
        }
    }

    rFmt.SetName(sNewName);

    if (bBroadcast)
        BroadcastStyleOperation(sNewName, eFamily, SfxStyleSheetHintId::MODIFIED);
}

void SwDoc::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    bool bOwns = false;
    if (!pWriter)
    {
        pWriter = xmlNewTextWriterFilename("nodes.xml", 0);
        xmlTextWriterStartDocument(pWriter, NULL, NULL, NULL);
        bOwns = true;
    }
    xmlTextWriterStartElement(pWriter, BAD_CAST("swDoc"));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);

    m_pNodes->dumpAsXml(pWriter);
    maDBData.dumpAsXml(pWriter);
    mpMarkManager->dumpAsXml(pWriter);
    m_pUndoManager->dumpAsXml(pWriter);
    getIDocumentFieldsAccess().GetFldTypes()->dumpAsXml(pWriter);
    mpTxtFmtCollTbl->dumpAsXml(pWriter);
    mpCharFmtTbl->dumpAsXml(pWriter);
    mpFrmFmtTbl->dumpAsXml(pWriter, "frmFmtTbl");
    mpSpzFrmFmtTbl->dumpAsXml(pWriter, "spzFrmFmtTbl");
    mpSectionFmtTbl->dumpAsXml(pWriter);
    mpNumRuleTbl->dumpAsXml(pWriter);
    getIDocumentRedlineAccess().GetRedlineTbl().dumpAsXml(pWriter);
    getIDocumentRedlineAccess().GetExtraRedlineTbl().dumpAsXml(pWriter);
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
    xmlTextWriterStartElement(pWriter, BAD_CAST("swDBData"));

    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("sDataSource"), BAD_CAST(sDataSource.toUtf8().getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("sCommand"), BAD_CAST(sCommand.toUtf8().getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nCommandType"), BAD_CAST(OString::number(nCommandType).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("sEmbeddedName"), BAD_CAST(sEmbeddedName.toUtf8().getStr()));

    xmlTextWriterEndElement(pWriter);
}

std::set<Color> SwDoc::GetDocColors()
{
    std::set<Color> aDocColors;
    SwAttrPool& rPool = GetAttrPool();
    const sal_uInt16 pAttribs[] = {RES_CHRATR_COLOR, RES_CHRATR_HIGHLIGHT, RES_BACKGROUND};
    for (size_t i=0; i<SAL_N_ELEMENTS(pAttribs); i++)
    {
        const sal_uInt16 nAttrib = pAttribs[i];
        const sal_uInt32 nCount = rPool.GetItemCount2(nAttrib);
        for (sal_uInt32 j=0; j<nCount; j++)
        {
            const SvxColorItem *pItem = static_cast<const SvxColorItem*>(rPool.GetItem2(nAttrib, j));
            if (pItem == 0)
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
        // If a parent paragraph style of one of the parargraph styles, which
        // are assigned to the list levels of the outline style, has a list style
        // set or inherits a list style from its parent style, the outline style
        // has to be written as a normal list style to the OpenDocument file
        // format or the OpenOffice.org file format.
        bool bRet( false );

        const SwTxtFmtColls* pTxtFmtColls( rDoc.GetTxtFmtColls() );
        if ( pTxtFmtColls )
        {
            for ( auto pTxtFmtColl : *pTxtFmtColls )
            {
                if ( pTxtFmtColl->IsDefault() ||
                    ! pTxtFmtColl->IsAssignedToListLevelOfOutlineStyle() )
                {
                    continue;
                }

                const SwTxtFmtColl* pParentTxtFmtColl =
                   dynamic_cast<const SwTxtFmtColl*>( pTxtFmtColl->DerivedFrom());
                if ( !pParentTxtFmtColl )
                    continue;

                if ( SfxItemState::SET == pParentTxtFmtColl->GetItemState( RES_PARATR_NUMRULE ) )
                {
                    // #i106218# consider that the outline style is set
                    const SwNumRuleItem& rDirectItem = pParentTxtFmtColl->GetNumRule();
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
