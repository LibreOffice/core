/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <hintids.hxx>
#include <rtl/logfile.hxx>
#include <svl/itemiter.hxx>
#include <sfx2/app.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/brkitem.hxx>
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
#include <IDocumentUndoRedo.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <hints.hxx>            // for SwHyphenBug (in SetDefault)
#include <ndtxt.hxx>
#include <pam.hxx>
#include <UndoCore.hxx>
#include <UndoAttribute.hxx>
#include <ndgrf.hxx>
#include <pagedesc.hxx>         // For special treatment in InsFrmFmt
#include <rolbck.hxx>           // Undo-Attr
#include <mvsave.hxx>           // serve: Recognize changes
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
#include <docsh.hxx>

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

/*
 * Reset the text's hard formatting
 */

// Parameters for _Rst and lcl_SetTxtFmtColl
struct ParaRstFmt
{
    SwFmtColl* pFmtColl;
    SwHistory* pHistory;
    const SwPosition *pSttNd, *pEndNd;
    const SfxItemSet* pDelSet;
    sal_uInt16 nWhich;
    bool bReset;
    bool bResetListAttrs; // #i62575#
    bool bResetAll;
    bool bInclRefToxMark;

    bool bKeepOutlineLevelAttr;

    ParaRstFmt( const SwPosition* pStt, const SwPosition* pEnd,
                SwHistory* pHst, sal_uInt16 nWhch = 0, const SfxItemSet* pSet = 0 )
        : pFmtColl(0),
          pHistory(pHst),
          pSttNd(pStt),
          pEndNd(pEnd),
          pDelSet(pSet),
          nWhich(nWhch),
          bReset( false ), // #i62675#
          bResetListAttrs( false ),
          bResetAll( true ),
          bInclRefToxMark( false ),
          bKeepOutlineLevelAttr( false )
    {}

    ParaRstFmt( SwHistory* pHst )
        : pFmtColl(0),
          pHistory(pHst),
          pSttNd(0),
          pEndNd(0),
          pDelSet(0),
          nWhich(0),
          bReset( false ),
          bResetListAttrs( false ), // #i62675#
          bResetAll( true ),
          bInclRefToxMark( false ),
            bKeepOutlineLevelAttr( false )
    {}
};

/* pArgs contains the document's ChrFmtTable
 * Is need for selections at the beginning/end and with no SSelection.
 */

static bool lcl_RstTxtAttr( const SwNodePtr& rpNd, void* pArgs )
{
    ParaRstFmt* pPara = (ParaRstFmt*)pArgs;
    SwTxtNode * pTxtNode = (SwTxtNode*)rpNd->GetTxtNode();
    if( pTxtNode && pTxtNode->GetpSwpHints() )
    {
        SwIndex aSt( pTxtNode, 0 );
        sal_uInt16 nEnd = pTxtNode->Len();

        if( &pPara->pSttNd->nNode.GetNode() == pTxtNode &&
            pPara->pSttNd->nContent.GetIndex() )
            aSt = pPara->pSttNd->nContent.GetIndex();

        if( &pPara->pEndNd->nNode.GetNode() == rpNd )
            nEnd = pPara->pEndNd->nContent.GetIndex();

        if( pPara->pHistory )
        {
            // Save all attributes for the Undo.
            SwRegHistory aRHst( *pTxtNode, pPara->pHistory );
            pTxtNode->GetpSwpHints()->Register( &aRHst );
            pTxtNode->RstAttr( aSt, nEnd - aSt.GetIndex(), pPara->nWhich,
                                pPara->pDelSet, pPara->bInclRefToxMark );
            if( pTxtNode->GetpSwpHints() )
                pTxtNode->GetpSwpHints()->DeRegister();
        }
        else
            pTxtNode->RstAttr( aSt, nEnd - aSt.GetIndex(), pPara->nWhich,
                                pPara->pDelSet, pPara->bInclRefToxMark );
    }
    return true;
}

static bool lcl_RstAttr( const SwNodePtr& rpNd, void* pArgs )
{
    ParaRstFmt* pPara = (ParaRstFmt*)pArgs;
    SwCntntNode* pNode = (SwCntntNode*)rpNd->GetCntntNode();
    if( pNode && pNode->HasSwAttrSet() )
    {
        const sal_Bool bLocked = pNode->IsModifyLocked();
        pNode->LockModify();

        SwDoc* pDoc = pNode->GetDoc();

        // remove unused attribute RES_LR_SPACE
        // add list attributes
        SfxItemSet aSet( pDoc->GetAttrPool(),
                         RES_PAGEDESC, RES_BREAK,
                         RES_PARATR_NUMRULE, RES_PARATR_NUMRULE,
                         RES_PARATR_OUTLINELEVEL,RES_PARATR_OUTLINELEVEL,//#outline level,removed by zhaojianwei
                         RES_PARATR_LIST_BEGIN, RES_PARATR_LIST_END - 1,
                         0 );
        const SfxItemSet* pSet = pNode->GetpSwAttrSet();

        std::vector<sal_uInt16> aClearWhichIds;
        // restoring all paragraph list attributes
        {
            SfxItemSet aListAttrSet( pDoc->GetAttrPool(), RES_PARATR_LIST_BEGIN, RES_PARATR_LIST_END - 1 );
            aListAttrSet.Set( *pSet );
            if ( aListAttrSet.Count() )
            {
                aSet.Put( aListAttrSet );
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

        sal_uInt16 const aSavIds[ 4 ] = { RES_PAGEDESC, RES_BREAK,  //->add by zhaojianwei
                                                RES_PARATR_NUMRULE,
                                                RES_PARATR_OUTLINELEVEL };
        for( sal_uInt16 n = 0; n < 4; ++n )                                     //<-end,zhaojianwei
        {
            if( SFX_ITEM_SET == pSet->GetItemState( aSavIds[ n ], sal_False, &pItem ))
            {
                bool bSave = false;
                switch( aSavIds[ n ] )
                {
                    case RES_PAGEDESC:
                        bSave = 0 != ((SwFmtPageDesc*)pItem)->GetPageDesc();
                    break;
                    case RES_BREAK:
                        bSave = SVX_BREAK_NONE != ((SvxFmtBreakItem*)pItem)->GetBreak();
                    break;
                    case RES_PARATR_NUMRULE:
                    {
                        bSave = 0 != ((SwNumRuleItem*)pItem)->GetValue().Len();
                    }
                    break;
                    case RES_PARATR_OUTLINELEVEL:               //#outline level,add by zhaojianwei
                    {
                        bSave = pPara && pPara->bKeepOutlineLevelAttr;
                    }
                    break;                                      //<-end,zhaojianwei
                }
                if( bSave )
                {
                    aSet.Put( *pItem );
                    aClearWhichIds.push_back( aSavIds[n] );
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
                while( sal_True )
                {
                    if ( ( pItem->Which() != RES_PAGEDESC &&
                           pItem->Which() != RES_BREAK &&
                           pItem->Which() != RES_PARATR_NUMRULE ) ||
                         ( aSet.GetItemState( pItem->Which(), sal_False ) != SFX_ITEM_SET ) )
                    {
                        pNode->ResetAttr( pItem->Which() );
                    }
                    if( aIter.IsAtEnd() )
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
        if ( bKeepAttributes && aSet.Count() )
        {
            pNode->LockModify();

            pNode->SetAttr( aSet );

            if( !bLocked )
                pNode->UnlockModify();
        }
    }
    return true;
}

void SwDoc::RstTxtAttrs(const SwPaM &rRg, bool bInclRefToxMark )
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
    ParaRstFmt aPara( pStt, pEnd, pHst );
    aPara.bInclRefToxMark = bInclRefToxMark;
    GetNodes().ForEach( pStt->nNode.GetIndex(), pEnd->nNode.GetIndex()+1,
                        lcl_RstTxtAttr, &aPara );
    SetModified();
}

void SwDoc::ResetAttrs( const SwPaM &rRg,
                        bool bTxtAttr,
                        const std::set<sal_uInt16> &rAttrs,
                        const bool bSendDataChangedEvents )
{
    SwPaM* pPam = (SwPaM*)&rRg;
    if( !bTxtAttr && !rAttrs.empty() && RES_TXTATR_END > *(rAttrs.begin()) )
        bTxtAttr = true;

    if( !rRg.HasMark() )
    {
        SwTxtNode* pTxtNd = rRg.GetPoint()->nNode.GetNode().GetTxtNode();
        if( !pTxtNd )
            return ;

        pPam = new SwPaM( *rRg.GetPoint() );

        SwIndex& rSt = pPam->GetPoint()->nContent;
        sal_uInt16 nMkPos, nPtPos = rSt.GetIndex();

        // Special case: if the Crsr is located within a URL attribute, we take over it's area
        SwTxtAttr const*const pURLAttr(
            pTxtNd->GetTxtAttrAt(rSt.GetIndex(), RES_TXTATR_INETFMT));
        if (pURLAttr && pURLAttr->GetINetFmt().GetValue().Len())
        {
            nMkPos = *pURLAttr->GetStart();
            nPtPos = *pURLAttr->GetEnd();
        }
        else
        {
            Boundary aBndry;
            if( pBreakIt->GetBreakIter().is() )
                aBndry = pBreakIt->GetBreakIter()->getWordBoundary(
                            pTxtNd->GetTxt(), nPtPos,
                            pBreakIt->GetLocale( pTxtNd->GetLang( nPtPos ) ),
                            WordType::ANY_WORD /*ANYWORD_IGNOREWHITESPACES*/,
                            sal_True );

            if( aBndry.startPos < nPtPos && nPtPos < aBndry.endPos )
            {
                nMkPos = (xub_StrLen)aBndry.startPos;
                nPtPos = (xub_StrLen)aBndry.endPos;
            }
            else
            {
                nPtPos = nMkPos = rSt.GetIndex();
                if( bTxtAttr )
                    pTxtNd->DontExpandFmt( rSt, sal_True );
            }
        }

        rSt = nMkPos;
        pPam->SetMark();
        pPam->GetPoint()->nContent = nPtPos;
    }

    // #i96644#
    // SwDataChanged aTmp( *pPam );
    std::auto_ptr< SwDataChanged > pDataChanged;
    if ( bSendDataChangedEvents )
    {
        pDataChanged.reset( new SwDataChanged( *pPam ) );
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
    ParaRstFmt aPara( pStt, pEnd, pHst );

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
        aTmpEnd++, bAdd = false;
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
        GetNodes().ForEach( pStt->nNode, aTmpEnd, lcl_RstTxtAttr, &aPara );
    }

    if( pPam != &rRg )
        delete pPam;

    SetModified();
}

#define DELETECHARSETS if ( bDelete ) { delete pCharSet; delete pOtherSet; }

// Insert Hints according to content types;
// Is used in SwDoc::Insert(..., SwFmtHint &rHt)

static bool
lcl_InsAttr(SwDoc *const pDoc, const SwPaM &rRg, const SfxItemSet& rChgSet,
            const SetAttrMode nFlags, SwUndoAttr *const pUndo)
{
    // Divide the Sets (for selections in Nodes)
    const SfxItemSet* pCharSet = 0;
    const SfxItemSet* pOtherSet = 0;
    bool bDelete = false;
    bool bCharAttr = false;
    bool bOtherAttr = false;

    // Check, if we can work with rChgSet or if we have to create additional SfxItemSets
    if ( 1 == rChgSet.Count() )
    {
        SfxItemIter aIter( rChgSet );
        const SfxPoolItem* pItem = aIter.FirstItem();
        if (!IsInvalidItem(pItem))
        {
            const sal_uInt16 nWhich = pItem->Which();

            if ( isCHRATR(nWhich) ||
                 (RES_TXTATR_CHARFMT == nWhich) ||
                 (RES_TXTATR_INETFMT == nWhich) ||
                 (RES_TXTATR_AUTOFMT == nWhich) ||
                 (RES_TXTATR_UNKNOWN_CONTAINER == nWhich) )
            {
                pCharSet  = &rChgSet;
                bCharAttr = true;
            }

            if (    isPARATR(nWhich)
                 || isPARATR_LIST(nWhich)
                 || isFRMATR(nWhich)
                 || isGRFATR(nWhich)
                 || isUNKNOWNATR(nWhich) )
            {
                pOtherSet = &rChgSet;
                bOtherAttr = true;
            }
        }
    }

    // Build new itemset if either
    // - rChgSet.Count() > 1 or
    // - The attribute in rChgSet does not belong to one of the above categories
    if ( !bCharAttr && !bOtherAttr )
    {
        SfxItemSet* pTmpCharItemSet = new SfxItemSet( pDoc->GetAttrPool(),
                                   RES_CHRATR_BEGIN, RES_CHRATR_END-1,
                                   RES_TXTATR_AUTOFMT, RES_TXTATR_AUTOFMT,
                                   RES_TXTATR_INETFMT, RES_TXTATR_INETFMT,
                                   RES_TXTATR_CHARFMT, RES_TXTATR_CHARFMT,
               RES_TXTATR_UNKNOWN_CONTAINER, RES_TXTATR_UNKNOWN_CONTAINER,
                                   0 );

        SfxItemSet* pTmpOtherItemSet = new SfxItemSet( pDoc->GetAttrPool(),
                                    RES_PARATR_BEGIN, RES_PARATR_END-1,
                                    RES_PARATR_LIST_BEGIN, RES_PARATR_LIST_END-1,
                                    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
                                    RES_GRFATR_BEGIN, RES_GRFATR_END-1,
                                    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
                                    0 );

        pTmpCharItemSet->Put( rChgSet );
        pTmpOtherItemSet->Put( rChgSet );

        pCharSet = pTmpCharItemSet;
        pOtherSet = pTmpOtherItemSet;

        bDelete = true;
    }

    SwHistory* pHistory = pUndo ? &pUndo->GetHistory() : 0;
    bool bRet = false;
    const SwPosition *pStt = rRg.Start(), *pEnd = rRg.End();
    SwCntntNode* pNode = pStt->nNode.GetNode().GetCntntNode();

    if( pNode && pNode->IsTxtNode() )
    {
        // #i27615#
        if (rRg.IsInFrontOfLabel())
        {
            SwTxtNode * pTxtNd = pNode->GetTxtNode();
            SwNumRule * pNumRule = pTxtNd->GetNumRule();

            // make code robust:
            if ( !pNumRule )
            {
                OSL_FAIL( "<InsAttr(..)> - PaM in front of label, but text node has no numbering rule set. This is a serious defect, please inform OD." );
                DELETECHARSETS
                return false;
            }

            SwNumFmt aNumFmt = pNumRule->Get(static_cast<sal_uInt16>(pTxtNd->GetActualListLevel()));
            SwCharFmt * pCharFmt =
                pDoc->FindCharFmtByName(aNumFmt.GetCharFmtName());

            if (pCharFmt)
            {
                if (pHistory)
                    pHistory->Add(pCharFmt->GetAttrSet(), *pCharFmt);

                if ( pCharSet )
                    pCharFmt->SetFmtAttr(*pCharSet);
            }

            DELETECHARSETS
            return true;
        }

        const SwIndex& rSt = pStt->nContent;

        // Attributes without an end do not have a range
        if ( !bCharAttr && !bOtherAttr )
        {
            SfxItemSet aTxtSet( pDoc->GetAttrPool(),
                        RES_TXTATR_NOEND_BEGIN, RES_TXTATR_NOEND_END-1 );
            aTxtSet.Put( rChgSet );
            if( aTxtSet.Count() )
            {
                SwRegHistory history( pNode, *pNode, pHistory );
                bRet = history.InsertItems(
                    aTxtSet, rSt.GetIndex(), rSt.GetIndex(), nFlags ) || bRet;

                if (bRet && (pDoc->IsRedlineOn() || (!pDoc->IsIgnoreRedline()
                                && !pDoc->GetRedlineTbl().empty())))
                {
                    SwPaM aPam( pStt->nNode, pStt->nContent.GetIndex()-1,
                                pStt->nNode, pStt->nContent.GetIndex() );

                    if( pUndo )
                        pUndo->SaveRedlineData( aPam, sal_True );

                    if( pDoc->IsRedlineOn() )
                        pDoc->AppendRedline( new SwRedline( nsRedlineType_t::REDLINE_INSERT, aPam ), true);
                    else
                        pDoc->SplitRedline( aPam );
                }
            }
        }

        // TextAttributes with an end never expand their range
        if ( !bCharAttr && !bOtherAttr )
        {
            // CharFmt and URL attributes are treated seperately!
            // TEST_TEMP ToDo: AutoFmt!
            SfxItemSet aTxtSet( pDoc->GetAttrPool(),
                                RES_TXTATR_REFMARK, RES_TXTATR_TOXMARK,
                                RES_TXTATR_META, RES_TXTATR_METAFIELD,
                                RES_TXTATR_CJK_RUBY, RES_TXTATR_CJK_RUBY,
                                0 );

            aTxtSet.Put( rChgSet );
            if( aTxtSet.Count() )
            {
                sal_uInt16 nInsCnt = rSt.GetIndex();
                sal_uInt16 nEnd = pStt->nNode == pEnd->nNode
                                ? pEnd->nContent.GetIndex()
                                : pNode->Len();
                SwRegHistory history( pNode, *pNode, pHistory );
                bRet = history.InsertItems( aTxtSet, nInsCnt, nEnd, nFlags )
                    || bRet;

                if (bRet && (pDoc->IsRedlineOn() || (!pDoc->IsIgnoreRedline()
                                && !pDoc->GetRedlineTbl().empty())))
                {
                    // Was text content inserted? (RefMark/TOXMarks without an end)
                    sal_Bool bTxtIns = nInsCnt != rSt.GetIndex();
                    // Was content inserted or set over the selection?
                    SwPaM aPam( pStt->nNode, bTxtIns ? nInsCnt + 1 : nEnd,
                                pStt->nNode, nInsCnt );
                    if( pUndo )
                        pUndo->SaveRedlineData( aPam, bTxtIns );

                    if( pDoc->IsRedlineOn() )
                        pDoc->AppendRedline( new SwRedline( bTxtIns
                                ? nsRedlineType_t::REDLINE_INSERT : nsRedlineType_t::REDLINE_FORMAT, aPam ), true);
                    else if( bTxtIns )
                        pDoc->SplitRedline( aPam );
                }
            }
        }
    }

    // We always have to set the auto flag for PageDescs that are set at the Node!
    if( pOtherSet && pOtherSet->Count() )
    {
        SwTableNode* pTblNd;
        const SwFmtPageDesc* pDesc;
        if( SFX_ITEM_SET == pOtherSet->GetItemState( RES_PAGEDESC,
                        sal_False, (const SfxPoolItem**)&pDesc ))
        {
            if( pNode )
            {
                // Set auto flag. Only in the template it's without auto!
                SwFmtPageDesc aNew( *pDesc );
                // 38479: AutoFlag is now being set in the WrtShell
                // aNew.SetAuto();

                // Tables now also know line breaks
                if( 0 == (nFlags & nsSetAttrMode::SETATTR_APICALL) &&
                    0 != ( pTblNd = pNode->FindTableNode() ) )
                {
                    SwTableNode* pCurTblNd = pTblNd;
                    while ( 0 != ( pCurTblNd = pCurTblNd->StartOfSectionNode()->FindTableNode() ) )
                        pTblNd = pCurTblNd;

                    // set the table format
                    SwFrmFmt* pFmt = pTblNd->GetTable().GetFrmFmt();
                    SwRegHistory aRegH( pFmt, *pTblNd, pHistory );
                    pFmt->SetFmtAttr( aNew );
                    bRet = true;
                }
                else
                {
                    SwRegHistory aRegH( pNode, *pNode, pHistory );
                    bRet = pNode->SetAttr( aNew ) || bRet;
                }
            }

            // bOtherAttr = true means that pOtherSet == rChgSet. In this case
            // we know, that there is only one attribute in pOtherSet. We cannot
            // perform the following operations, instead we return:
            if ( bOtherAttr )
                return bRet;

            const_cast<SfxItemSet*>(pOtherSet)->ClearItem( RES_PAGEDESC );
            if( !pOtherSet->Count() )
            {
                DELETECHARSETS
                return bRet;
            }
        }

        // Tables now also know line breaks
        const SvxFmtBreakItem* pBreak;
        if( pNode && 0 == (nFlags & nsSetAttrMode::SETATTR_APICALL) &&
            0 != (pTblNd = pNode->FindTableNode() ) &&
            SFX_ITEM_SET == pOtherSet->GetItemState( RES_BREAK,
                        sal_False, (const SfxPoolItem**)&pBreak ) )
        {
            SwTableNode* pCurTblNd = pTblNd;
            while ( 0 != ( pCurTblNd = pCurTblNd->StartOfSectionNode()->FindTableNode() ) )
                pTblNd = pCurTblNd;

             // set the table format
            SwFrmFmt* pFmt = pTblNd->GetTable().GetFrmFmt();
            SwRegHistory aRegH( pFmt, *pTblNd, pHistory );
            pFmt->SetFmtAttr( *pBreak );
            bRet = true;

            // bOtherAttr = true means that pOtherSet == rChgSet. In this case
            // we know, that there is only one attribute in pOtherSet. We cannot
            // perform the following operations, instead we return:
            if ( bOtherAttr )
                return bRet;

            const_cast<SfxItemSet*>(pOtherSet)->ClearItem( RES_BREAK );
            if( !pOtherSet->Count() )
            {
                DELETECHARSETS
                return bRet;
            }
        }

        {
            // If we have a PoolNumRule, create it if needed
            const SwNumRuleItem* pRule;
            sal_uInt16 nPoolId;
            if( SFX_ITEM_SET == pOtherSet->GetItemState( RES_PARATR_NUMRULE,
                                sal_False, (const SfxPoolItem**)&pRule ) &&
                !pDoc->FindNumRulePtr( pRule->GetValue() ) &&
                USHRT_MAX != (nPoolId = SwStyleNameMapper::GetPoolIdFromUIName ( pRule->GetValue(),
                                nsSwGetPoolIdFromName::GET_POOLID_NUMRULE )) )
                pDoc->GetNumRuleFromPool( nPoolId );
        }

    }

    if( !rRg.HasMark() )        // no range
    {
        if( !pNode )
        {
            DELETECHARSETS
            return bRet;
        }

        if( pNode->IsTxtNode() && pCharSet && pCharSet->Count() )
        {
            SwTxtNode* pTxtNd = static_cast<SwTxtNode*>(pNode);
            const SwIndex& rSt = pStt->nContent;
            sal_uInt16 nMkPos, nPtPos = rSt.GetIndex();
            const String& rStr = pTxtNd->GetTxt();

            // Special case: if the Crsr is located within a URL attribute, we take over it's area
            SwTxtAttr const*const pURLAttr(
                pTxtNd->GetTxtAttrAt(rSt.GetIndex(), RES_TXTATR_INETFMT));
            if (pURLAttr && pURLAttr->GetINetFmt().GetValue().Len())
            {
                nMkPos = *pURLAttr->GetStart();
                nPtPos = *pURLAttr->GetEnd();
            }
            else
            {
                Boundary aBndry;
                if( pBreakIt->GetBreakIter().is() )
                    aBndry = pBreakIt->GetBreakIter()->getWordBoundary(
                                pTxtNd->GetTxt(), nPtPos,
                                pBreakIt->GetLocale( pTxtNd->GetLang( nPtPos ) ),
                                WordType::ANY_WORD /*ANYWORD_IGNOREWHITESPACES*/,
                                sal_True );

                if( aBndry.startPos < nPtPos && nPtPos < aBndry.endPos )
                {
                    nMkPos = (xub_StrLen)aBndry.startPos;
                    nPtPos = (xub_StrLen)aBndry.endPos;
                }
                else
                    nPtPos = nMkPos = rSt.GetIndex();
            }

            // Remove the overriding attributes from the SwpHintsArray,
            // if the selection spans across the whole paragraph.
            // These attributes are inserted as FormatAttributes and
            // never override the TextAttributes!
            if( !(nFlags & nsSetAttrMode::SETATTR_DONTREPLACE ) &&
                pTxtNd->HasHints() && !nMkPos && nPtPos == rStr.Len() )
            {
                SwIndex aSt( pTxtNd );
                if( pHistory )
                {
                    // Save all attributes for the Undo.
                    SwRegHistory aRHst( *pTxtNd, pHistory );
                    pTxtNd->GetpSwpHints()->Register( &aRHst );
                    pTxtNd->RstAttr( aSt, nPtPos, 0, pCharSet );
                    if( pTxtNd->GetpSwpHints() )
                        pTxtNd->GetpSwpHints()->DeRegister();
                }
                else
                    pTxtNd->RstAttr( aSt, nPtPos, 0, pCharSet );
            }

            // the SwRegHistory inserts the attribute into the TxtNode!
            SwRegHistory history( pNode, *pNode, pHistory );
            bRet = history.InsertItems( *pCharSet, nMkPos, nPtPos, nFlags )
                || bRet;

            if( pDoc->IsRedlineOn() )
            {
                SwPaM aPam( *pNode, nMkPos, *pNode, nPtPos );

                if( pUndo )
                    pUndo->SaveRedlineData( aPam, sal_False );
                pDoc->AppendRedline( new SwRedline( nsRedlineType_t::REDLINE_FORMAT, aPam ), true);
            }
        }
        if( pOtherSet && pOtherSet->Count() )
        {
            SwRegHistory aRegH( pNode, *pNode, pHistory );
            bRet = pNode->SetAttr( *pOtherSet ) || bRet;
        }

        DELETECHARSETS
        return bRet;
    }

    if( pDoc->IsRedlineOn() && pCharSet && pCharSet->Count() )
    {
        if( pUndo )
            pUndo->SaveRedlineData( rRg, sal_False );
        pDoc->AppendRedline( new SwRedline( nsRedlineType_t::REDLINE_FORMAT, rRg ), true);
    }

    /* now if range */
    sal_uLong nNodes = 0;

    SwNodeIndex aSt( pDoc->GetNodes() );
    SwNodeIndex aEnd( pDoc->GetNodes() );
    SwIndex aCntEnd( pEnd->nContent );

    if( pNode )
    {
        sal_uInt16 nLen = pNode->Len();
        if( pStt->nNode != pEnd->nNode )
            aCntEnd.Assign( pNode, nLen );

        if( pStt->nContent.GetIndex() != 0 || aCntEnd.GetIndex() != nLen )
        {
            // the SwRegHistory inserts the attribute into the TxtNode!
            if( pNode->IsTxtNode() && pCharSet && pCharSet->Count() )
            {
                SwRegHistory history( pNode, *pNode, pHistory );
                bRet = history.InsertItems(*pCharSet,
                        pStt->nContent.GetIndex(), aCntEnd.GetIndex(), nFlags)
                    || bRet;
            }

            if( pOtherSet && pOtherSet->Count() )
            {
                SwRegHistory aRegH( pNode, *pNode, pHistory );
                bRet = pNode->SetAttr( *pOtherSet ) || bRet;
            }

            // Only selection in a Node.
            if( pStt->nNode == pEnd->nNode )
            {
                DELETECHARSETS
                return bRet;
            }
            ++nNodes;
            aSt.Assign( pStt->nNode.GetNode(), +1 );
        }
        else
            aSt = pStt->nNode;
        aCntEnd = pEnd->nContent; // aEnd was changed!
    }
    else
        aSt.Assign( pStt->nNode.GetNode(), +1 );

    // aSt points to the first full Node now

    /*
     * The selection spans more than one Node.
     */
    if( pStt->nNode < pEnd->nNode )
    {
        pNode = pEnd->nNode.GetNode().GetCntntNode();
        if(pNode)
        {
            sal_uInt16 nLen = pNode->Len();
            if( aCntEnd.GetIndex() != nLen )
            {
                // the SwRegHistory inserts the attribute into the TxtNode!
                if( pNode->IsTxtNode() && pCharSet && pCharSet->Count() )
                {
                    SwRegHistory history( pNode, *pNode, pHistory );
                    history.InsertItems(*pCharSet,
                            0, aCntEnd.GetIndex(), nFlags);
                }

                if( pOtherSet && pOtherSet->Count() )
                {
                    SwRegHistory aRegH( pNode, *pNode, pHistory );
                    pNode->SetAttr( *pOtherSet );
                }

                ++nNodes;
                aEnd = pEnd->nNode;
            }
            else
                aEnd.Assign( pEnd->nNode.GetNode(), +1 );
        }
        else
            aEnd = pEnd->nNode;
    }
    else
        aEnd.Assign( pEnd->nNode.GetNode(), +1 );

    // aEnd points BEHIND the last full node now

    /* Edit the fully selected Nodes. */
    // Reset all attributes from the set!
    if( pCharSet && pCharSet->Count() && !( nsSetAttrMode::SETATTR_DONTREPLACE & nFlags ) )
    {

        ParaRstFmt aPara( pStt, pEnd, pHistory, 0, pCharSet );
        pDoc->GetNodes().ForEach( aSt, aEnd, lcl_RstTxtAttr, &aPara );
    }

    bool bCreateSwpHints = pCharSet && (
        SFX_ITEM_SET == pCharSet->GetItemState( RES_TXTATR_CHARFMT, sal_False ) ||
        SFX_ITEM_SET == pCharSet->GetItemState( RES_TXTATR_INETFMT, sal_False ) );

    for(; aSt < aEnd; ++aSt )
    {
        pNode = aSt.GetNode().GetCntntNode();
        if( !pNode )
            continue;

        SwTxtNode* pTNd = pNode->GetTxtNode();
        if( pHistory )
        {
            SwRegHistory aRegH( pNode, *pNode, pHistory );
            SwpHints *pSwpHints;

            if( pTNd && pCharSet && pCharSet->Count() )
            {
                pSwpHints = bCreateSwpHints ? &pTNd->GetOrCreateSwpHints()
                                            : pTNd->GetpSwpHints();
                if( pSwpHints )
                    pSwpHints->Register( &aRegH );

                pTNd->SetAttr( *pCharSet, 0, pTNd->GetTxt().Len(), nFlags );
                if( pSwpHints )
                    pSwpHints->DeRegister();
            }
            if( pOtherSet && pOtherSet->Count() )
                pNode->SetAttr( *pOtherSet );
        }
        else
        {
            if( pTNd && pCharSet && pCharSet->Count() )
                pTNd->SetAttr( *pCharSet, 0, pTNd->GetTxt().Len(), nFlags );
            if( pOtherSet && pOtherSet->Count() )
                pNode->SetAttr( *pOtherSet );
        }
        ++nNodes;
    }

    DELETECHARSETS
    return (nNodes != 0) || bRet;
}


bool SwDoc::InsertPoolItem( const SwPaM &rRg, const SfxPoolItem &rHt,
                            const SetAttrMode nFlags )
{
    SwDataChanged aTmp( rRg );
    SwUndoAttr* pUndoAttr = 0;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().ClearRedo();
        pUndoAttr = new SwUndoAttr( rRg, rHt, nFlags );
    }

    SfxItemSet aSet( GetAttrPool(), rHt.Which(), rHt.Which() );
    aSet.Put( rHt );
    bool bRet = lcl_InsAttr( this, rRg, aSet, nFlags, pUndoAttr );

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo( pUndoAttr );
    }

    if( bRet )
        SetModified();
    return bRet;
}

bool SwDoc::InsertItemSet ( const SwPaM &rRg, const SfxItemSet &rSet,
                            const SetAttrMode nFlags )
{
    SwDataChanged aTmp( rRg );
    SwUndoAttr* pUndoAttr = 0;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().ClearRedo();
        pUndoAttr = new SwUndoAttr( rRg, rSet, nFlags );
    }

    bool bRet = lcl_InsAttr( this, rRg, rSet, nFlags, pUndoAttr );

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo( pUndoAttr );
    }

    if( bRet )
        SetModified();
    return bRet;
}


    // Set the attribute according to the stated format. If Undo is enabled, the old values is
    // added to the Undo history.
void SwDoc::SetAttr( const SfxPoolItem& rAttr, SwFmt& rFmt )
{
    SfxItemSet aSet( GetAttrPool(), rAttr.Which(), rAttr.Which() );
    aSet.Put( rAttr );
    SetAttr( aSet, rFmt );
}


     // Set the attribute according to the stated format. If Undo is enabled, the old values is
     // added to the Undo history.
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
    SetModified();
}

void SwDoc::ResetAttrAtFormat( const sal_uInt16 nWhichId,
                               SwFmt& rChangedFormat )
{
    SwUndo *const pUndo = (GetIDocumentUndoRedo().DoesUndo())
        ?   new SwUndoFmtResetAttr( rChangedFormat, nWhichId )
        :   0;

    const sal_Bool bAttrReset = rChangedFormat.ResetFmtAttr( nWhichId );

    if ( bAttrReset )
    {
        if ( pUndo )
        {
            GetIDocumentUndoRedo().AppendUndo( pUndo );
        }

        SetModified();
    }
    else
        delete pUndo;
}

static int lcl_SetNewDefTabStops( SwTwips nOldWidth, SwTwips nNewWidth,
                                SvxTabStopItem& rChgTabStop )
{
    // Set the default values of all TabStops to the new value.
    // Attention: we always work with the PoolAttribut here, so that
    // we don't calculate the same value on the same TabStop (pooled!) for all sets.
    // We send a FmtChg to modify.

    sal_uInt16 nOldCnt = rChgTabStop.Count();
    if( !nOldCnt || nOldWidth == nNewWidth )
        return sal_False;

    // Find the default's beginning
    sal_uInt16 n;
    for( n = nOldCnt; n ; --n )
        if( SVX_TAB_ADJUST_DEFAULT != rChgTabStop[n - 1].GetAdjustment() )
            break;
    ++n;
    if( n < nOldCnt )   // delete the DefTabStops
        rChgTabStop.Remove( n, nOldCnt - n );
    return sal_True;
}

// Set the attribute as new default attribute in this document.
// If Undi is enabled, the old value is added to the Undo history.
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
    sal_uInt16 nWhich;
    const SfxPoolItem* pItem = aIter.GetCurItem();
    SfxItemPool* pSdrPool = GetAttrPool().GetSecondaryPool();
    while( sal_True )
    {
        bool bCheckSdrDflt = false;
        nWhich = pItem->Which();
        aOld.Put( GetAttrPool().GetDefaultItem( nWhich ) );
        GetAttrPool().SetPoolDefaultItem( *pItem );
        aNew.Put( GetAttrPool().GetDefaultItem( nWhich ) );

        if (isCHRATR(nWhich) || isTXTATR(nWhich))
        {
            aCallMod.Add( pDfltTxtFmtColl );
            aCallMod.Add( pDfltCharFmt );
            bCheckSdrDflt = 0 != pSdrPool;
        }
        else if ( isPARATR(nWhich) ||
                  isPARATR_LIST(nWhich) )
        {
            aCallMod.Add( pDfltTxtFmtColl );
            bCheckSdrDflt = 0 != pSdrPool;
        }
        else if (isGRFATR(nWhich))
        {
            aCallMod.Add( pDfltGrfFmtColl );
        }
        else if (isFRMATR(nWhich))
        {
            aCallMod.Add( pDfltGrfFmtColl );
            aCallMod.Add( pDfltTxtFmtColl );
            aCallMod.Add( pDfltFrmFmt );
        }
        else if (isBOXATR(nWhich))
        {
            aCallMod.Add( pDfltFrmFmt );
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

    if( aNew.Count() && aCallMod.GetDepends() )
    {
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            GetIDocumentUndoRedo().AppendUndo( new SwUndoDefaultAttr( aOld ) );
        }

        const SfxPoolItem* pTmpItem;
        if( ( SFX_ITEM_SET ==
                aNew.GetItemState( RES_PARATR_TABSTOP, sal_False, &pTmpItem ) ) &&
            ((SvxTabStopItem*)pTmpItem)->Count() )
        {
            // Set the default values of all TabStops to the new value.
            // Attention: we always work with the PoolAttribut here, so that
            // we don't calculate the same value on the same TabStop (pooled!) for all sets.
            // We send a FmtChg to modify.
            SwTwips nNewWidth = (*(SvxTabStopItem*)pTmpItem)[ 0 ].GetTabPos(),
                    nOldWidth = ((SvxTabStopItem&)aOld.Get(RES_PARATR_TABSTOP))[ 0 ].GetTabPos();

            int bChg = sal_False;
            sal_uInt32 nMaxItems = GetAttrPool().GetItemCount2( RES_PARATR_TABSTOP );
            for( sal_uInt32 n = 0; n < nMaxItems; ++n )
                if( 0 != (pTmpItem = GetAttrPool().GetItem2( RES_PARATR_TABSTOP, n ) ))
                    bChg |= lcl_SetNewDefTabStops( nOldWidth, nNewWidth,
                                                *(SvxTabStopItem*)pTmpItem );

            aNew.ClearItem( RES_PARATR_TABSTOP );
            aOld.ClearItem( RES_PARATR_TABSTOP );
            if( bChg )
            {
                SwFmtChg aChgFmt( pDfltCharFmt );
                // notify the frames
                aCallMod.ModifyNotification( &aChgFmt, &aChgFmt );
            }
        }
    }

    if( aNew.Count() && aCallMod.GetDepends() )
    {
        SwAttrSetChg aChgOld( aOld, aOld );
        SwAttrSetChg aChgNew( aNew, aNew );
        aCallMod.ModifyNotification( &aChgOld, &aChgNew );      // all changed are sent
    }

    // remove the default formats from the object again
    SwClient* pDep;
    while( 0 != ( pDep = (SwClient*)aCallMod.GetDepends()) )
        aCallMod.Remove( pDep );

    SetModified();
}

// Get the default attribute in this document
const SfxPoolItem& SwDoc::GetDefault( sal_uInt16 nFmtHint ) const
{
    return GetAttrPool().GetDefaultItem( nFmtHint );
}

/*
 * Delete the formats
 */
void SwDoc::DelCharFmt(sal_uInt16 nFmt, bool bBroadcast)
{
    SwCharFmt * pDel = (*pCharFmtTbl)[nFmt];

    if (bBroadcast)
        BroadcastStyleOperation(pDel->GetName(), SFX_STYLE_FAMILY_CHAR,
                                SFX_STYLESHEET_ERASED);

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndo * pUndo =
            new SwUndoCharFmtDelete(pDel, this);

        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    delete (*pCharFmtTbl)[nFmt];
    pCharFmtTbl->erase(pCharFmtTbl->begin() + nFmt);

    SetModified();
}

void SwDoc::DelCharFmt( SwCharFmt *pFmt, bool bBroadcast )
{
    sal_uInt16 nFmt = pCharFmtTbl->GetPos( pFmt );
    OSL_ENSURE( USHRT_MAX != nFmt, "Fmt not found," );
    DelCharFmt( nFmt, bBroadcast );
}

void SwDoc::DelFrmFmt( SwFrmFmt *pFmt, bool bBroadcast )
{
    if( pFmt->ISA( SwTableBoxFmt ) || pFmt->ISA( SwTableLineFmt ))
    {
        OSL_ENSURE( !this, "Format is not in the DocArray any more, "
                       "so it can be deleted with delete" );
        delete pFmt;
    }
    else
    {

        // The format has to be in the one or the other, we'll see in which one.
        SwFrmFmts::iterator it = std::find( pFrmFmtTbl->begin(), pFrmFmtTbl->end(), pFmt );
        if ( it != pFrmFmtTbl->end() )
        {
            if (bBroadcast)
                BroadcastStyleOperation(pFmt->GetName(),
                                        SFX_STYLE_FAMILY_FRAME,
                                        SFX_STYLESHEET_ERASED);

            if (GetIDocumentUndoRedo().DoesUndo())
            {
                SwUndo * pUndo = new SwUndoFrmFmtDelete(pFmt, this);

                GetIDocumentUndoRedo().AppendUndo(pUndo);
            }

            delete *it;
            pFrmFmtTbl->erase(it);
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
    SwFrmFmts::iterator it = std::find( pTblFrmFmtTbl->begin(), pTblFrmFmtTbl->end(), pFmt );
    OSL_ENSURE( it != pTblFrmFmtTbl->end(), "Fmt not found," );
    delete *it;
    pTblFrmFmtTbl->erase(it);
}

/*
 * Create the formats
 */
SwFlyFrmFmt *SwDoc::MakeFlyFrmFmt( const String &rFmtName,
                                    SwFrmFmt *pDerivedFrom )
{
    SwFlyFrmFmt *pFmt = new SwFlyFrmFmt( GetAttrPool(), rFmtName, pDerivedFrom );
    GetSpzFrmFmts()->push_back(pFmt);
    SetModified();
    return pFmt;
}

SwDrawFrmFmt *SwDoc::MakeDrawFrmFmt( const String &rFmtName,
                                     SwFrmFmt *pDerivedFrom )
{
    SwDrawFrmFmt *pFmt = new SwDrawFrmFmt( GetAttrPool(), rFmtName, pDerivedFrom);
    GetSpzFrmFmts()->push_back(pFmt);
    SetModified();
    return pFmt;
}


sal_uInt16 SwDoc::GetTblFrmFmtCount(bool bUsed) const
{
    sal_uInt16 nCount = pTblFrmFmtTbl->size();
    if(bUsed)
    {
        SwAutoFmtGetDocNode aGetHt( &GetNodes() );
        for ( sal_uInt16 i = nCount; i; )
        {
            if((*pTblFrmFmtTbl)[--i]->GetInfo( aGetHt ))
                --nCount;
        }
    }

    return nCount;
}


SwFrmFmt& SwDoc::GetTblFrmFmt(sal_uInt16 nFmt, bool bUsed ) const
{
    sal_uInt16 nRemoved = 0;
    if(bUsed)
    {
        SwAutoFmtGetDocNode aGetHt( &GetNodes() );
        for ( sal_uInt16 i = 0; i <= nFmt; i++ )
        {
            while ( (*pTblFrmFmtTbl)[ i + nRemoved]->GetInfo( aGetHt ))
            {
                nRemoved++;
            }
        }
    }
    return *((*pTblFrmFmtTbl)[nRemoved + nFmt]);
}

SwTableFmt* SwDoc::MakeTblFrmFmt( const String &rFmtName,
                                    SwFrmFmt *pDerivedFrom )
{
    SwTableFmt* pFmt = new SwTableFmt( GetAttrPool(), rFmtName, pDerivedFrom );
    pTblFrmFmtTbl->push_back( pFmt );
    SetModified();

    return pFmt;
}

SwFrmFmt *SwDoc::MakeFrmFmt(const String &rFmtName,
                            SwFrmFmt *pDerivedFrom,
                            sal_Bool bBroadcast, sal_Bool bAuto)
{

    SwFrmFmt *pFmt = new SwFrmFmt( GetAttrPool(), rFmtName, pDerivedFrom );

    pFmt->SetAuto(bAuto);
    pFrmFmtTbl->push_back( pFmt );
    SetModified();

    if (bBroadcast)
    {
        BroadcastStyleOperation(rFmtName, SFX_STYLE_FAMILY_PARA,
                                SFX_STYLESHEET_CREATED);

        if (GetIDocumentUndoRedo().DoesUndo())
        {
            SwUndo * pUndo = new SwUndoFrmFmtCreate(pFmt, pDerivedFrom, this);

            GetIDocumentUndoRedo().AppendUndo(pUndo);
        }
    }

    return pFmt;
}

SwFmt *SwDoc::_MakeFrmFmt(const String &rFmtName,
                            SwFmt *pDerivedFrom,
                            sal_Bool bBroadcast, sal_Bool bAuto)
{
    SwFrmFmt *pFrmFmt = dynamic_cast<SwFrmFmt*>(pDerivedFrom);
    pFrmFmt = MakeFrmFmt( rFmtName, pFrmFmt, bBroadcast, bAuto );
    return dynamic_cast<SwFmt*>(pFrmFmt);
}


// #i40550# - add parameter <bAuto> - not relevant
SwCharFmt *SwDoc::MakeCharFmt( const String &rFmtName,
                               SwCharFmt *pDerivedFrom,
                               sal_Bool bBroadcast,
                               sal_Bool )
{
    SwCharFmt *pFmt = new SwCharFmt( GetAttrPool(), rFmtName, pDerivedFrom );
    pCharFmtTbl->push_back( pFmt );
    pFmt->SetAuto( sal_False );
    SetModified();

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndo * pUndo = new SwUndoCharFmtCreate(pFmt, pDerivedFrom, this);

        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    if (bBroadcast)
    {
        BroadcastStyleOperation(rFmtName, SFX_STYLE_FAMILY_CHAR,
                                SFX_STYLESHEET_CREATED);
    }

    return pFmt;
}

SwFmt *SwDoc::_MakeCharFmt(const String &rFmtName,
                            SwFmt *pDerivedFrom,
                            sal_Bool bBroadcast, sal_Bool bAuto)
{
    SwCharFmt *pCharFmt = dynamic_cast<SwCharFmt*>(pDerivedFrom);
    pCharFmt = MakeCharFmt( rFmtName, pCharFmt, bBroadcast, bAuto );
    return dynamic_cast<SwFmt*>(pCharFmt);
}


/*
 * Create the FormatCollections
 */
// TXT
// #i40550# - add parameter <bAuto> - not relevant
SwTxtFmtColl* SwDoc::MakeTxtFmtColl( const String &rFmtName,
                                     SwTxtFmtColl *pDerivedFrom,
                                     sal_Bool bBroadcast,
                                     sal_Bool )
{
    SwTxtFmtColl *pFmtColl = new SwTxtFmtColl( GetAttrPool(), rFmtName,
                                                pDerivedFrom );
    pTxtFmtCollTbl->push_back(pFmtColl);
    pFmtColl->SetAuto( sal_False );
    SetModified();

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndo * pUndo = new SwUndoTxtFmtCollCreate(pFmtColl, pDerivedFrom,
                                                    this);
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    if (bBroadcast)
        BroadcastStyleOperation(rFmtName, SFX_STYLE_FAMILY_PARA,
                                SFX_STYLESHEET_CREATED);

    return pFmtColl;
}

SwFmt *SwDoc::_MakeTxtFmtColl(const String &rFmtName,
                            SwFmt *pDerivedFrom,
                            sal_Bool bBroadcast, sal_Bool bAuto)
{
    SwTxtFmtColl *pTxtFmtColl = dynamic_cast<SwTxtFmtColl*>(pDerivedFrom);
    pTxtFmtColl = MakeTxtFmtColl( rFmtName, pTxtFmtColl, bBroadcast, bAuto );
    return dynamic_cast<SwFmt*>(pTxtFmtColl);
}


//FEATURE::CONDCOLL
SwConditionTxtFmtColl* SwDoc::MakeCondTxtFmtColl( const String &rFmtName,
                                                  SwTxtFmtColl *pDerivedFrom,
                                                  sal_Bool bBroadcast)
{
    SwConditionTxtFmtColl*pFmtColl = new SwConditionTxtFmtColl( GetAttrPool(),
                                                    rFmtName, pDerivedFrom );
    pTxtFmtCollTbl->push_back(pFmtColl);
    pFmtColl->SetAuto( sal_False );
    SetModified();

    if (bBroadcast)
        BroadcastStyleOperation(rFmtName, SFX_STYLE_FAMILY_PARA,
                                SFX_STYLESHEET_CREATED);

    return pFmtColl;
}
//FEATURE::CONDCOLL

// GRF

SwGrfFmtColl* SwDoc::MakeGrfFmtColl( const String &rFmtName,
                                     SwGrfFmtColl *pDerivedFrom )
{
    SwGrfFmtColl *pFmtColl = new SwGrfFmtColl( GetAttrPool(), rFmtName,
                                                pDerivedFrom );
    pGrfFmtCollTbl->push_back( pFmtColl );
    pFmtColl->SetAuto( sal_False );
    SetModified();
    return pFmtColl;
}

void SwDoc::DelTxtFmtColl(sal_uInt16 nFmtColl, sal_Bool bBroadcast)
{
    OSL_ENSURE( nFmtColl, "Remove fuer Coll 0." );

    // Who has the to-be-deleted as their Next?
    SwTxtFmtColl *pDel = (*pTxtFmtCollTbl)[nFmtColl];
    if( pDfltTxtFmtColl == pDel )
        return;     // never delete default!

    if (bBroadcast)
        BroadcastStyleOperation(pDel->GetName(), SFX_STYLE_FAMILY_PARA,
                                SFX_STYLESHEET_ERASED);

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndoTxtFmtCollDelete * pUndo =
            new SwUndoTxtFmtCollDelete(pDel, this);

        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    // Remove the FmtColl
    pTxtFmtCollTbl->erase(pTxtFmtCollTbl->begin() + nFmtColl);
    // Correct next
    for( SwTxtFmtColls::const_iterator it = pTxtFmtCollTbl->begin() + 1; it != pTxtFmtCollTbl->end(); ++it )
        SetTxtFmtCollNext( *it, pDel );
    delete pDel;
    SetModified();
}

void SwDoc::DelTxtFmtColl( SwTxtFmtColl *pColl, sal_Bool bBroadcast )
{
    sal_uInt16 nFmt = pTxtFmtCollTbl->GetPos( pColl );
    OSL_ENSURE( USHRT_MAX != nFmt, "Collection not found," );
    DelTxtFmtColl( nFmt, bBroadcast );
}

static bool lcl_SetTxtFmtColl( const SwNodePtr& rpNode, void* pArgs )
{
    // ParaSetFmtColl * pPara = (ParaSetFmtColl*)pArgs;
    SwCntntNode* pCNd = (SwCntntNode*)rpNode->GetTxtNode();
    if( pCNd )
    {
        ParaRstFmt* pPara = (ParaRstFmt*)pArgs;

        SwTxtFmtColl* pFmt = static_cast<SwTxtFmtColl*>(pPara->pFmtColl);
        if ( pPara->bReset )
        {

            if( pFmt->GetAttrOutlineLevel() == 0 && pPara )
                pPara->bKeepOutlineLevelAttr = true;

            lcl_RstAttr( pCNd, pPara );

            // #i62675# check, if paragraph style has changed
            if ( pPara->bResetListAttrs &&
                 pFmt != pCNd->GetFmtColl() &&
                 pFmt->GetItemState( RES_PARATR_NUMRULE ) == SFX_ITEM_SET )
            {
                // Check, if the list style of the paragraph will change.
                bool bChangeOfListStyleAtParagraph( true );
                SwTxtNode* pTNd( dynamic_cast<SwTxtNode*>(pCNd) );
                OSL_ENSURE( pTNd,
                        "<lcl_SetTxtFmtColl(..)> - text node expected -> crash" );
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
                    std::auto_ptr< SwRegHistory > pRegH;
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
    }
    return true;
}

sal_Bool SwDoc::SetTxtFmtColl( const SwPaM &rRg,
                           SwTxtFmtColl *pFmt,
                           bool bReset,
                           bool bResetListAttrs )
{
    SwDataChanged aTmp( rRg );
    const SwPosition *pStt = rRg.Start(), *pEnd = rRg.End();
    SwHistory* pHst = 0;
    sal_Bool bRet = sal_True;

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndoFmtColl* pUndo = new SwUndoFmtColl( rRg, pFmt,
                                                  bReset,
                                                  bResetListAttrs );
        pHst = pUndo->GetHistory();
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    ParaRstFmt aPara( pStt, pEnd, pHst );
    aPara.pFmtColl = pFmt;
    aPara.bReset = bReset;
    // #i62675#
    aPara.bResetListAttrs = bResetListAttrs;

    GetNodes().ForEach( pStt->nNode.GetIndex(), pEnd->nNode.GetIndex()+1,
                        lcl_SetTxtFmtColl, &aPara );
    if( !aPara.nWhich )
        bRet = sal_False;           // didn't find a valid Node

    if( bRet )
        SetModified();
    return bRet;
}


// ---- Copy the formats to itself (SwDoc) ----------------------

SwFmt* SwDoc::CopyFmt( const SwFmt& rFmt,
                        const SwFmtsBase& rFmtArr,
                        FNCopyFmt fnCopyFmt, const SwFmt& rDfltFmt )
{
    // It's no autoformat, default format or collection format,
    // then search for it.
    if( !rFmt.IsAuto() || !rFmt.GetRegisteredIn() )
        for( sal_uInt16 n = 0; n < rFmtArr.GetFmtCount(); n++ )
        {
            // Does the Doc already contain the template?
            if( rFmtArr.GetFmt(n)->GetName().Equals( rFmt.GetName() ))
                return (SwFmt*)rFmtArr.GetFmt(n);
        }

    // Search for the "parent" first
    SwFmt* pParent = (SwFmt*)&rDfltFmt;
    if( rFmt.DerivedFrom() && pParent != rFmt.DerivedFrom() )
        pParent = CopyFmt( *rFmt.DerivedFrom(), rFmtArr,
                                fnCopyFmt, rDfltFmt );

    // Create the format and copy the attributes
    // #i40550#
    SwFmt* pNewFmt = (this->*fnCopyFmt)( rFmt.GetName(), pParent, sal_False, sal_True );
    pNewFmt->SetAuto( rFmt.IsAuto() );
    pNewFmt->CopyAttrs( rFmt, sal_True );           // copy the attributes

    pNewFmt->SetPoolFmtId( rFmt.GetPoolFmtId() );
    pNewFmt->SetPoolHelpId( rFmt.GetPoolHelpId() );

    // Always set the HelpFile Id to dflt!
    pNewFmt->SetPoolHlpFileId( UCHAR_MAX );

    return pNewFmt;
}


// ---- copy the frame format --------
SwFrmFmt* SwDoc::CopyFrmFmt( const SwFrmFmt& rFmt )
{

    return (SwFrmFmt*)CopyFmt( rFmt, *GetFrmFmts(), &SwDoc::_MakeFrmFmt,
                                *GetDfltFrmFmt() );
}

// ---- copy the char format --------
SwCharFmt* SwDoc::CopyCharFmt( const SwCharFmt& rFmt )
{
    return (SwCharFmt*)CopyFmt( rFmt, *GetCharFmts(),
                                &SwDoc::_MakeCharFmt,
                                *GetDfltCharFmt() );
}


// --- copy TextNodes ----

SwTxtFmtColl* SwDoc::CopyTxtColl( const SwTxtFmtColl& rColl )
{
    SwTxtFmtColl* pNewColl = FindTxtFmtCollByName( rColl.GetName() );
    if( pNewColl )
        return pNewColl;

    // search for the "parent" first
    SwTxtFmtColl* pParent = pDfltTxtFmtColl;
    if( pParent != rColl.DerivedFrom() )
        pParent = CopyTxtColl( *(SwTxtFmtColl*)rColl.DerivedFrom() );


//FEATURE::CONDCOLL
    if( RES_CONDTXTFMTCOLL == rColl.Which() )
    {
        pNewColl = new SwConditionTxtFmtColl( GetAttrPool(), rColl.GetName(),
                                                pParent);
        pTxtFmtCollTbl->push_back( pNewColl );
        pNewColl->SetAuto( sal_False );
        SetModified();

        // copy the conditions
        ((SwConditionTxtFmtColl*)pNewColl)->SetConditions(
                            ((SwConditionTxtFmtColl&)rColl).GetCondColls() );
    }
    else
//FEATURE::CONDCOLL
        pNewColl = MakeTxtFmtColl( rColl.GetName(), pParent );

    // copy the auto formats or the attributes
    pNewColl->CopyAttrs( rColl, sal_True );

    if(rColl.IsAssignedToListLevelOfOutlineStyle())
        pNewColl->AssignToListLevelOfOutlineStyle(rColl.GetAssignedOutlineStyleLevel());//<-end,zhaojianwei
    //<-end
    pNewColl->SetPoolFmtId( rColl.GetPoolFmtId() );
    pNewColl->SetPoolHelpId( rColl.GetPoolHelpId() );

    // Always set the HelpFile Id to dflt!
    pNewColl->SetPoolHlpFileId( UCHAR_MAX );

    if( &rColl.GetNextTxtFmtColl() != &rColl )
        pNewColl->SetNextTxtFmtColl( *CopyTxtColl( rColl.GetNextTxtFmtColl() ));

    // create the NumRule if necessary
    if( this != rColl.GetDoc() )
    {
        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == pNewColl->GetItemState( RES_PARATR_NUMRULE,
            sal_False, &pItem ))
        {
            const SwNumRule* pRule;
            const String& rName = ((SwNumRuleItem*)pItem)->GetValue();
            if( rName.Len() &&
                0 != ( pRule = rColl.GetDoc()->FindNumRulePtr( rName )) &&
                !pRule->IsAutoRule() )
            {
                SwNumRule* pDestRule = FindNumRulePtr( rName );
                if( pDestRule )
                    pDestRule->SetInvalidRule( sal_True );
                else
                    MakeNumRule( rName, pRule );
            }
        }
    }
    return pNewColl;
}

// --- copy the graphic nodes ----

SwGrfFmtColl* SwDoc::CopyGrfColl( const SwGrfFmtColl& rColl )
{
    SwGrfFmtColl* pNewColl = FindGrfFmtCollByName( rColl.GetName() );
    if( pNewColl )
        return pNewColl;

     // Search for the "parent" first
    SwGrfFmtColl* pParent = pDfltGrfFmtColl;
    if( pParent != rColl.DerivedFrom() )
        pParent = CopyGrfColl( *(SwGrfFmtColl*)rColl.DerivedFrom() );

    // if not, copy them
    pNewColl = MakeGrfFmtColl( rColl.GetName(), pParent );

    // copy the attributes
    pNewColl->CopyAttrs( rColl );

    pNewColl->SetPoolFmtId( rColl.GetPoolFmtId() );
    pNewColl->SetPoolHelpId( rColl.GetPoolHelpId() );

    // Always set the HelpFile Id to dflt!
    pNewColl->SetPoolHlpFileId( UCHAR_MAX );

    return pNewColl;
}

static SwPageDesc* lcl_FindPageDesc( const SwPageDescs& rArr, const String& rName )
{
    for( sal_uInt16 n = rArr.size(); n; )
    {
        SwPageDesc* pDesc = rArr[ --n ];
        if( pDesc->GetName() == rName )
            return pDesc;
    }
    return 0;
}

void SwDoc::CopyFmtArr( const SwFmtsBase& rSourceArr,
                        SwFmtsBase& rDestArr,
                        FNCopyFmt fnCopyFmt,
                        SwFmt& rDfltFmt )
{
    sal_uInt16 nSrc;
    SwFmt* pSrc, *pDest;

    // 1st step: Create all formats (skip the 0th - it's the default one)
    for( nSrc = rSourceArr.GetFmtCount(); nSrc > 1; )
    {
        pSrc = (SwFmt*)rSourceArr.GetFmt( --nSrc );
        if( pSrc->IsDefault() || pSrc->IsAuto() )
            continue;

        if( 0 == FindFmtByName( rDestArr, pSrc->GetName() ) )
        {
            if( RES_CONDTXTFMTCOLL == pSrc->Which() )
                MakeCondTxtFmtColl( pSrc->GetName(), (SwTxtFmtColl*)&rDfltFmt );
            else
                // #i40550#
                (this->*fnCopyFmt)( pSrc->GetName(), &rDfltFmt, sal_False, sal_True );
        }
    }

    // 2nd step: Copy all attributes, set the right parents
    for( nSrc = rSourceArr.GetFmtCount(); nSrc > 1; )
    {
        pSrc = (SwFmt*)rSourceArr.GetFmt( --nSrc );
        if( pSrc->IsDefault() || pSrc->IsAuto() )
            continue;

        pDest = FindFmtByName( rDestArr, pSrc->GetName() );
        pDest->SetAuto( sal_False );
        pDest->DelDiffs( *pSrc );

        // #i94285#: existing <SwFmtPageDesc> instance, before copying attributes
        const SfxPoolItem* pItem;
        if( &GetAttrPool() != pSrc->GetAttrSet().GetPool() &&
            SFX_ITEM_SET == pSrc->GetAttrSet().GetItemState(
            RES_PAGEDESC, sal_False, &pItem ) &&
            ((SwFmtPageDesc*)pItem)->GetPageDesc() )
        {
            SwFmtPageDesc aPageDesc( *(SwFmtPageDesc*)pItem );
            const String& rNm = aPageDesc.GetPageDesc()->GetName();
            SwPageDesc* pPageDesc = ::lcl_FindPageDesc( aPageDescs, rNm );
            if( !pPageDesc )
            {
                pPageDesc = aPageDescs[ MakePageDesc( rNm ) ];
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

        // Always set the HelpFile Id to dflt!
        pDest->SetPoolHlpFileId( UCHAR_MAX );

        if( pSrc->DerivedFrom() )
            pDest->SetDerivedFrom( FindFmtByName( rDestArr,
                                        pSrc->DerivedFrom()->GetName() ) );
        if( RES_TXTFMTCOLL == pSrc->Which() ||
            RES_CONDTXTFMTCOLL == pSrc->Which() )
        {
            SwTxtFmtColl* pSrcColl = (SwTxtFmtColl*)pSrc,
                        * pDstColl = (SwTxtFmtColl*)pDest;
            if( &pSrcColl->GetNextTxtFmtColl() != pSrcColl )
                pDstColl->SetNextTxtFmtColl( *(SwTxtFmtColl*)FindFmtByName(
                    rDestArr, pSrcColl->GetNextTxtFmtColl().GetName() ) );

            if(pSrcColl->IsAssignedToListLevelOfOutlineStyle())
                pDstColl->AssignToListLevelOfOutlineStyle(pSrcColl->GetAssignedOutlineStyleLevel());//<-end,zhaojianwei
            //<-end

//FEATURE::CONDCOLL
            if( RES_CONDTXTFMTCOLL == pSrc->Which() )
                // Copy the conditions, but delete the old ones first!
                ((SwConditionTxtFmtColl*)pDstColl)->SetConditions(
                            ((SwConditionTxtFmtColl*)pSrc)->GetCondColls() );
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
    if( SFX_ITEM_SET != rSrcFmt.GetAttrSet().GetItemState( nAttr, sal_False, &pItem ))
        return ;

    // The header only contains the reference to the format from the other document!
    SfxPoolItem* pNewItem = pItem->Clone();

    SwFrmFmt* pOldFmt;
    if( bCpyHeader )
         pOldFmt = ((SwFmtHeader*)pNewItem)->GetHeaderFmt();
    else
         pOldFmt = ((SwFmtFooter*)pNewItem)->GetFooterFmt();

    if( pOldFmt )
    {
        SwFrmFmt* pNewFmt = new SwFrmFmt( GetAttrPool(), "CpyDesc",
                                            GetDfltFrmFmt() );
        pNewFmt->CopyAttrs( *pOldFmt, sal_True );

        if( SFX_ITEM_SET == pNewFmt->GetAttrSet().GetItemState(
            RES_CNTNT, sal_False, &pItem ))
        {
            SwFmtCntnt* pCntnt = (SwFmtCntnt*)pItem;
            if( pCntnt->GetCntntIdx() )
            {
                SwNodeIndex aTmpIdx( GetNodes().GetEndOfAutotext() );
                const SwNodes& rSrcNds = rSrcFmt.GetDoc()->GetNodes();
                SwStartNode* pSttNd = GetNodes().MakeEmptySection( aTmpIdx,
                                                bCpyHeader
                                                    ? SwHeaderStartNode
                                                    : SwFooterStartNode );
                const SwNode& rCSttNd = pCntnt->GetCntntIdx()->GetNode();
                SwNodeRange aRg( rCSttNd, 0, *rCSttNd.EndOfSectionNode() );
                aTmpIdx = *pSttNd->EndOfSectionNode();
                rSrcNds._Copy( aRg, aTmpIdx );
                aTmpIdx = *pSttNd;
                rSrcFmt.GetDoc()->CopyFlyInFlyImpl( aRg, 0, aTmpIdx );
                pNewFmt->SetFmtAttr( SwFmtCntnt( pSttNd ));
            }
            else
                pNewFmt->ResetFmtAttr( RES_CNTNT );
        }
        if( bCpyHeader )
            ((SwFmtHeader*)pNewItem)->RegisterToFormat(*pNewFmt);
        else
            ((SwFmtFooter*)pNewItem)->RegisterToFormat(*pNewFmt);
        rDestFmt.SetFmtAttr( *pNewItem );
    }
    delete pNewItem;
}

void SwDoc::CopyPageDesc( const SwPageDesc& rSrcDesc, SwPageDesc& rDstDesc,
                            sal_Bool bCopyPoolIds )
{
    bool bNotifyLayout = false;
    SwRootFrm* pTmpRoot = GetCurrentLayout();//swmod 080219

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
        // Always set the HelpFile Id to dflt!
        rDstDesc.SetPoolHlpFileId( UCHAR_MAX );
    }

    if( rSrcDesc.GetFollow() != &rSrcDesc )
    {
        SwPageDesc* pFollow = ::lcl_FindPageDesc( aPageDescs,
                                    rSrcDesc.GetFollow()->GetName() );
        if( !pFollow )
        {
            // copy
            sal_uInt16 nPos = MakePageDesc( rSrcDesc.GetFollow()->GetName() );
            pFollow = aPageDescs[ nPos ];
            CopyPageDesc( *rSrcDesc.GetFollow(), *pFollow );
        }
        rDstDesc.SetFollow( pFollow );
        bNotifyLayout = true;
    }

    // the header and footer attributes are copied seperately
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
    }

    CopyHeader( rSrcDesc.GetMaster(), rDstDesc.GetMaster() );
    CopyFooter( rSrcDesc.GetMaster(), rDstDesc.GetMaster() );
    if( !rDstDesc.IsHeaderShared() )
        CopyHeader( rSrcDesc.GetLeft(), rDstDesc.GetLeft() );
    else
        rDstDesc.GetLeft().SetFmtAttr( rDstDesc.GetMaster().GetHeader() );
    if( !rDstDesc.IsFirstShared() )
        CopyHeader( rSrcDesc.GetFirst(), rDstDesc.GetFirst() );
    else
        rDstDesc.GetFirst().SetFmtAttr( rDstDesc.GetMaster().GetHeader() );

    if( !rDstDesc.IsFooterShared() )
        CopyFooter( rSrcDesc.GetLeft(), rDstDesc.GetLeft() );
    else
        rDstDesc.GetLeft().SetFmtAttr( rDstDesc.GetMaster().GetFooter() );
    if( !rDstDesc.IsFirstShared() )
        CopyFooter( rSrcDesc.GetFirst(), rDstDesc.GetFirst() );
    else
        rDstDesc.GetFirst().SetFmtAttr( rDstDesc.GetMaster().GetFooter() );

    if( bNotifyLayout && pTmpRoot )
    {
        std::set<SwRootFrm*> aAllLayouts = GetAllLayouts();//swmod 080225
        std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::mem_fun(&SwRootFrm::AllCheckPageDescs));//swmod 080226
    }

    // If foot notes change the pages have to be triggered
    if( !(rDstDesc.GetFtnInfo() == rSrcDesc.GetFtnInfo()) )
    {
        rDstDesc.SetFtnInfo( rSrcDesc.GetFtnInfo() );
        SwMsgPoolItem  aInfo( RES_PAGEDESC_FTNINFO );
        {
            rDstDesc.GetMaster().ModifyBroadcast( &aInfo, 0, TYPE(SwFrm) );
        }
        {
            rDstDesc.GetLeft().ModifyBroadcast( &aInfo, 0, TYPE(SwFrm) );
        }
    }
}

void SwDoc::ReplaceStyles( const SwDoc& rSource, bool bIncludePageStyles )
{
    ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

    CopyFmtArr( *rSource.pCharFmtTbl, *pCharFmtTbl,
                &SwDoc::_MakeCharFmt, *pDfltCharFmt );
    CopyFmtArr( *rSource.pFrmFmtTbl, *pFrmFmtTbl,
                &SwDoc::_MakeFrmFmt, *pDfltFrmFmt );
    CopyFmtArr( *rSource.pTxtFmtCollTbl, *pTxtFmtCollTbl,
                &SwDoc::_MakeTxtFmtColl, *pDfltTxtFmtColl );

    sal_uInt16 nCnt;

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
        nCnt = rSource.aPageDescs.size();
        if( nCnt )
        {
            // a different Doc -> Number formatter needs to be merged
            SwTblNumFmtMerge aTNFM( rSource, *this );

            // 1st step: Create all formats (skip the 0th - it's the default!)
            while( nCnt )
            {
                SwPageDesc *pSrc = rSource.aPageDescs[ --nCnt ];
                if( 0 == ::lcl_FindPageDesc( aPageDescs, pSrc->GetName() ) )
                    MakePageDesc( pSrc->GetName() );
            }

            // 2nd step: Copy all attributes, set the right parents
            for( nCnt = rSource.aPageDescs.size(); nCnt; )
            {
                SwPageDesc *pSrc = rSource.aPageDescs[ --nCnt ];
                CopyPageDesc( *pSrc, *::lcl_FindPageDesc( aPageDescs, pSrc->GetName() ));
            }
        }
    }

    // then there are the numbering templates
    nCnt = rSource.GetNumRuleTbl().size();
    if( nCnt )
    {
        const SwNumRuleTbl& rArr = rSource.GetNumRuleTbl();
        for( sal_uInt16 n = 0; n < nCnt; ++n )
        {
            const SwNumRule& rR = *rArr[ n ];
            if( !rR.IsAutoRule() )
            {
                SwNumRule* pNew = FindNumRulePtr( rR.GetName());
                if( pNew )
                    pNew->CopyNumRule( this, rR );
                else
                    MakeNumRule( rR.GetName(), &rR );
            }
        }
    }

    if (undoGuard.UndoWasEnabled())
    {
        // nodes array was modified!
        GetIDocumentUndoRedo().DelAllUndoObj();
    }

    SetModified();
}

SwFmt* SwDoc::FindFmtByName( const SwFmtsBase& rFmtArr,
                                    const String& rName ) const
{
    SwFmt* pFnd = 0;
    for( sal_uInt16 n = 0; n < rFmtArr.GetFmtCount(); n++ )
    {
        // Does the Doc already contain the template?
        if( rFmtArr.GetFmt(n)->GetName() == rName )
        {
            pFnd = (SwFmt*)rFmtArr.GetFmt(n);
            break;
        }
    }
    return pFnd;
}

void SwDoc::MoveLeftMargin( const SwPaM& rPam, sal_Bool bRight, sal_Bool bModulus )
{
    SwHistory* pHistory = 0;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndoMoveLeftMargin* pUndo = new SwUndoMoveLeftMargin( rPam, bRight,
                                                                bModulus );
        pHistory = &pUndo->GetHistory();
        GetIDocumentUndoRedo().AppendUndo( pUndo );
    }

    const SvxTabStopItem& rTabItem = (SvxTabStopItem&)GetDefault( RES_PARATR_TABSTOP );
    sal_uInt16 nDefDist = rTabItem.Count() ?
        static_cast<sal_uInt16>(rTabItem[0].GetTabPos()) : 1134;
    const SwPosition &rStt = *rPam.Start(), &rEnd = *rPam.End();
    SwNodeIndex aIdx( rStt.nNode );
    while( aIdx <= rEnd.nNode )
    {
        SwTxtNode* pTNd = aIdx.GetNode().GetTxtNode();
        if( pTNd )
        {
            SvxLRSpaceItem aLS( (SvxLRSpaceItem&)pTNd->SwCntntNode::GetAttr( RES_LR_SPACE ) );

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
                nNext -= nDefDist;

            aLS.SetTxtLeft( nNext );

            SwRegHistory aRegH( pTNd, *pTNd, pHistory );
            pTNd->SetAttr( aLS );
        }
        ++aIdx;
    }
    SetModified();
}

sal_Bool SwDoc::DontExpandFmt( const SwPosition& rPos, sal_Bool bFlag )
{
    sal_Bool bRet = sal_False;
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
    SwTableBoxFmt* pFmt = new SwTableBoxFmt( GetAttrPool(), aEmptyStr,
                                                pDfltFrmFmt );
    SetModified();
    return pFmt;
}

SwTableLineFmt* SwDoc::MakeTableLineFmt()
{
    SwTableLineFmt* pFmt = new SwTableLineFmt( GetAttrPool(), aEmptyStr,
                                                pDfltFrmFmt );
    SetModified();
    return pFmt;
}

void SwDoc::_CreateNumberFormatter()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDoc::_CreateNumberFormatter" );

    OSL_ENSURE( !pNumberFormatter, "is already there" );


    LanguageType eLang = LANGUAGE_SYSTEM;

    Reference< XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
    pNumberFormatter = new SvNumberFormatter( xMSF, eLang );
    pNumberFormatter->SetEvalDateFormat( NF_EVALDATEFORMAT_FORMAT_INTL );
    pNumberFormatter->SetYear2000(static_cast<sal_uInt16>(::utl::MiscCfg().GetYear2000()));

}

SwTblNumFmtMerge::SwTblNumFmtMerge( const SwDoc& rSrc, SwDoc& rDest )
    : pNFmt( 0 )
{
    // a different Doc -> Number formatter needs to be merged
    SvNumberFormatter* pN;
    if( &rSrc != &rDest && 0 != ( pN = ((SwDoc&)rSrc).GetNumberFormatter( sal_False ) ))
        ( pNFmt = rDest.GetNumberFormatter( sal_True ))->MergeFormatter( *pN );

    if( &rSrc != &rDest )
        ((SwGetRefFieldType*)rSrc.GetSysFldType( RES_GETREFFLD ))->
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

    if( mbIsAutoFmtRedline && pTNd )
    {
        // create the redline object
        const SwTxtFmtColl& rColl = *pTNd->GetTxtColl();
        SwRedline* pRedl = new SwRedline( nsRedlineType_t::REDLINE_FMTCOLL, aPam );
        pRedl->SetMark();

        // Only those items that are not set by the Set again in the Node
        // are of interest. Thus, we take the difference.
        SwRedlineExtraData_FmtColl aExtraData( rColl.GetName(),
                                                rColl.GetPoolFmtId() );
        if( pSet && pTNd->HasSwAttrSet() )
        {
            SfxItemSet aTmp( *pTNd->GetpSwAttrSet() );
            aTmp.Differentiate( *pSet );
            // we handle the adjust item seperately
            const SfxPoolItem* pItem;
            if( SFX_ITEM_SET == pTNd->GetpSwAttrSet()->GetItemState(
                    RES_PARATR_ADJUST, sal_False, &pItem ))
                aTmp.Put( *pItem );
            aExtraData.SetItemSet( aTmp );
        }
        pRedl->SetExtraData( &aExtraData );

        //TODO: Undo is still missing!
        AppendRedline( pRedl, true );
    }

    SetTxtFmtColl( aPam, GetTxtCollFromPool( nPoolId ) );

    if( pSet && pTNd && pSet->Count() )
    {
        aPam.SetMark();
        aPam.GetMark()->nContent.Assign( pTNd, pTNd->GetTxt().Len() );
        InsertItemSet( aPam, *pSet, 0 );
    }
}

void SwDoc::SetFmtItemByAutoFmt( const SwPaM& rPam, const SfxItemSet& rSet )
{
    SwTxtNode* pTNd = rPam.GetPoint()->nNode.GetNode().GetTxtNode();

    RedlineMode_t eOld = GetRedlineMode();

    if( mbIsAutoFmtRedline && pTNd )
    {
        // create the redline object
        SwRedline* pRedl = new SwRedline( nsRedlineType_t::REDLINE_FORMAT, rPam );
        if( !pRedl->HasMark() )
            pRedl->SetMark();

        // Only those items that are not set by the Set again in the Node
        // are of interest. Thus, we take the difference.
        SwRedlineExtraData_Format aExtraData( rSet );

        pRedl->SetExtraData( &aExtraData );

        //TODO: Undo is still missing!
        AppendRedline( pRedl, true );

        SetRedlineMode_intern( (RedlineMode_t)(eOld | nsRedlineMode_t::REDLINE_IGNORE));
    }

    InsertItemSet( rPam, rSet, nsSetAttrMode::SETATTR_DONTEXPAND );
    SetRedlineMode_intern( eOld );
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

void SwDoc::RenameFmt(SwFmt & rFmt, const String & sNewName,
                      sal_Bool bBroadcast)
{
    SfxStyleFamily eFamily = SFX_STYLE_FAMILY_ALL;

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndo * pUndo = NULL;

        switch (rFmt.Which())
        {
        case RES_CHRFMT:
            pUndo = new SwUndoRenameCharFmt(rFmt.GetName(), sNewName, this);
            eFamily = SFX_STYLE_FAMILY_PARA;
            break;
        case RES_TXTFMTCOLL:
            pUndo = new SwUndoRenameFmtColl(rFmt.GetName(), sNewName, this);
            eFamily = SFX_STYLE_FAMILY_CHAR;
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
        BroadcastStyleOperation(sNewName, eFamily, SFX_STYLESHEET_MODIFIED);
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
            const sal_uInt16 nCount = pTxtFmtColls->size();
            for ( sal_uInt16 i = 0; i < nCount; ++i )
            {
                SwTxtFmtColl* pTxtFmtColl = (*pTxtFmtColls)[i];

                if ( pTxtFmtColl->IsDefault() ||
                    ! pTxtFmtColl->IsAssignedToListLevelOfOutlineStyle() )  //<-end,zhaojianwei
                {
                    continue;
                }

                const SwTxtFmtColl* pParentTxtFmtColl =
                   dynamic_cast<const SwTxtFmtColl*>( pTxtFmtColl->DerivedFrom());
                if ( !pParentTxtFmtColl )
                    continue;

                if ( SFX_ITEM_SET == pParentTxtFmtColl->GetItemState( RES_PARATR_NUMRULE ) )
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

SwFmtsBase::~SwFmtsBase() {}

sal_uInt16 SwFrmFmts::GetPos(const SwFrmFmt* p) const
{
    const_iterator it = std::find(begin(), end(), p);
    return it == end() ? USHRT_MAX : it - begin();
}

bool SwFrmFmts::Contains(const SwFrmFmt* p) const
{
    return std::find(begin(), end(), p) != end();
}

SwFrmFmts::~SwFrmFmts()
{
    for(const_iterator it = begin(); it != end(); ++it)
        delete *it;
}

sal_uInt16 SwCharFmts::GetPos(const SwCharFmt* p) const
{
    const_iterator it = std::find(begin(), end(), p);
    return it == end() ? USHRT_MAX : it - begin();
}

bool SwCharFmts::Contains(const SwCharFmt* p) const
{
    return std::find(begin(), end(), p) != end();
}

SwCharFmts::~SwCharFmts()
{
    for(const_iterator it = begin(); it != end(); ++it)
        delete *it;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
