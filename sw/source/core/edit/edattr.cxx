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


#include <hintids.hxx>

#include <editeng/tstpitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <com/sun/star/i18n/ScriptType.hdl>
#include <txatbase.hxx>
#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <editsh.hxx>
#include <edimp.hxx>    // fuer MACROS
#include <doc.hxx>
#include <swundo.hxx>   // fuer UNDO-Ids
#include <ndtxt.hxx>
#include <ftnidx.hxx>
#include <expfld.hxx>
#include <rootfrm.hxx>
#include <cntfrm.hxx>
#include <breakit.hxx>
#include <txtfld.hxx>
#include <fmtfld.hxx>
#include <crsskip.hxx>
#include <txtfrm.hxx>       // SwTxtFrm
#include <scriptinfo.hxx>
#include <svl/ctloptions.hxx>
#include <charfmt.hxx>  // #i27615#
#include <numrule.hxx>


/*************************************
 * harte Formatierung (Attribute)
 *************************************/

// wenn Selektion groesser Max Nodes oder mehr als Max Selektionen
// => keine Attribute
const USHORT& getMaxLookup()
{
    static const USHORT nMaxLookup = 1000;
    return nMaxLookup;
}

// --> OD 2008-01-16 #newlistlevelattrs#
BOOL SwEditShell::GetCurAttr( SfxItemSet& rSet,
                              const bool bMergeIndentValuesOfNumRule ) const
// <--
{
    if( GetCrsrCnt() > getMaxLookup() )
    {
        rSet.InvalidateAllItems();
        return FALSE;
    }

    SfxItemSet aSet( *rSet.GetPool(), rSet.GetRanges() );
    SfxItemSet *pSet = &rSet;

    FOREACHPAM_START(this)

        // #i27615# if the cursor is in front of the numbering label
        // the attributes to get are those from the numbering format.
        if (PCURCRSR->IsInFrontOfLabel())
        {
            SwTxtNode * pTxtNd =
                PCURCRSR->GetPoint()->nNode.GetNode().GetTxtNode();

            if (pTxtNd)
            {
                SwNumRule * pNumRule = pTxtNd->GetNumRule();

                if (pNumRule)
                {
                    const String & aCharFmtName =
                        pNumRule->Get(static_cast<USHORT>(pTxtNd->GetActualListLevel())).GetCharFmtName();
                    SwCharFmt * pCharFmt =
                        GetDoc()->FindCharFmtByName(aCharFmtName);

                    if (pCharFmt)
                        rSet.Put(pCharFmt->GetAttrSet());
                }
            }

            continue;
        }

        ULONG nSttNd = PCURCRSR->GetMark()->nNode.GetIndex(),
              nEndNd = PCURCRSR->GetPoint()->nNode.GetIndex();
        xub_StrLen nSttCnt = PCURCRSR->GetMark()->nContent.GetIndex(),
                   nEndCnt = PCURCRSR->GetPoint()->nContent.GetIndex();

        if( nSttNd > nEndNd || ( nSttNd == nEndNd && nSttCnt > nEndCnt ))
        {
            ULONG nTmp = nSttNd; nSttNd = nEndNd; nEndNd = nTmp;
            nTmp = nSttCnt; nSttCnt = nEndCnt; nEndCnt = (xub_StrLen)nTmp;
        }

        if( nEndNd - nSttNd >= getMaxLookup() )
        {
            rSet.ClearItem();
            rSet.InvalidateAllItems();
            return FALSE;
        }

        // beim 1.Node traegt der Node die Werte in den GetSet ein (Initial)
        // alle weiteren Nodes werden zum GetSet zu gemergt
        for( ULONG n = nSttNd; n <= nEndNd; ++n )
        {
            SwNode* pNd = GetDoc()->GetNodes()[ n ];
            switch( pNd->GetNodeType() )
            {
            case ND_TEXTNODE:
                {
                    xub_StrLen nStt = n == nSttNd ? nSttCnt : 0,
                                  nEnd = n == nEndNd ? nEndCnt
                                        : ((SwTxtNode*)pNd)->GetTxt().Len();
                    // --> OD 2008-01-16 #newlistlevelattrs#
                    ((SwTxtNode*)pNd)->GetAttr( *pSet, nStt, nEnd,
                                                FALSE, TRUE,
                                                bMergeIndentValuesOfNumRule );
                    // <--
                }
                break;
            case ND_GRFNODE:
            case ND_OLENODE:
                ((SwCntntNode*)pNd)->GetAttr( *pSet );
                break;

            default:
                pNd = 0;
            }

            if( pNd )
            {
                if( pSet != &rSet )
                    rSet.MergeValues( aSet );

                if( aSet.Count() )
                    aSet.ClearItem();
            }
            pSet = &aSet;
        }

    FOREACHPAM_END()

    return TRUE;
}

SwTxtFmtColl* SwEditShell::GetCurTxtFmtColl() const
{
    SwTxtFmtColl *pFmt = 0;

    if ( GetCrsrCnt() > getMaxLookup() )
        return 0;

    FOREACHPAM_START(this)

        ULONG nSttNd = PCURCRSR->GetMark()->nNode.GetIndex(),
              nEndNd = PCURCRSR->GetPoint()->nNode.GetIndex();
        xub_StrLen nSttCnt = PCURCRSR->GetMark()->nContent.GetIndex(),
                   nEndCnt = PCURCRSR->GetPoint()->nContent.GetIndex();

        if( nSttNd > nEndNd || ( nSttNd == nEndNd && nSttCnt > nEndCnt ))
        {
            ULONG nTmp = nSttNd; nSttNd = nEndNd; nEndNd = nTmp;
            nTmp = nSttCnt; nSttCnt = nEndCnt; nEndCnt = (xub_StrLen)nTmp;
        }

        if( nEndNd - nSttNd >= getMaxLookup() )
        {
            pFmt = 0;
            break;
        }

        for( ULONG n = nSttNd; n <= nEndNd; ++n )
        {
            SwNode* pNd = GetDoc()->GetNodes()[ n ];
            if( pNd->IsTxtNode() )
            {
                if( !pFmt )
                    pFmt = ((SwTxtNode*)pNd)->GetTxtColl();
                else if( pFmt == ((SwTxtNode*)pNd)->GetTxtColl() ) // ???
                    break;
            }
        }

    FOREACHPAM_END()
    return pFmt;
}



BOOL SwEditShell::GetCurFtn( SwFmtFtn* pFillFtn )
{
    // der Cursor muss auf dem akt. Fussnoten-Anker stehen:
    SwPaM* pCrsr = GetCrsr();
    SwTxtNode* pTxtNd = pCrsr->GetNode()->GetTxtNode();
    if( !pTxtNd )
        return FALSE;

    SwTxtAttr *const pFtn = pTxtNd->GetTxtAttrForCharAt(
        pCrsr->GetPoint()->nContent.GetIndex(), RES_TXTATR_FTN);
    if( pFtn && pFillFtn )
    {
        // Daten vom Attribut uebertragen
        const SwFmtFtn &rFtn = ((SwTxtFtn*)pFtn)->GetFtn();
        pFillFtn->SetNumber( rFtn );
        pFillFtn->SetEndNote( rFtn.IsEndNote() );
    }
    return 0 != pFtn;
}


bool SwEditShell::SetCurFtn( const SwFmtFtn& rFillFtn )
{
    bool bChgd = false;
    StartAllAction();

    SwPaM* pCrsr = GetCrsr(), *pFirst = pCrsr;
    do {
        bChgd |=  pDoc->SetCurFtn( *pCrsr, rFillFtn.GetNumStr(),
                                            rFillFtn.GetNumber(),
                                            rFillFtn.IsEndNote() );

    } while( pFirst != ( pCrsr = (SwPaM*)pCrsr->GetNext() ));

    EndAllAction();
    return bChgd;
}



/*USHORT SwEditShell::GetFtnCnt( BOOL bEndNotes = FALSE ) const
{
    const SwFtnIdxs &rIdxs = pDoc->GetFtnIdxs();
    USHORT nCnt = 0;
    for ( USHORT i = 0; i < rIdxs.Count(); ++i )
    {
        const SwFmtFtn &rFtn = rIdxs[i]->GetFtn();
        if ( bEndNotes == rFtn.IsEndNote() )
            nCnt++;
    }
    return nCnt;
} */


bool SwEditShell::HasFtns( bool bEndNotes ) const
{
    const SwFtnIdxs &rIdxs = pDoc->GetFtnIdxs();
    for ( USHORT i = 0; i < rIdxs.Count(); ++i )
    {
        const SwFmtFtn &rFtn = rIdxs[i]->GetFtn();
        if ( bEndNotes == rFtn.IsEndNote() )
            return TRUE;
    }
    return FALSE;
}


    // gebe Liste aller Fussnoten und deren Anfangstexte
USHORT SwEditShell::GetSeqFtnList( SwSeqFldList& rList, bool bEndNotes )
{
    if( rList.Count() )
        rList.Remove( 0, rList.Count() );

    USHORT n, nFtnCnt = pDoc->GetFtnIdxs().Count();
    SwTxtFtn* pTxtFtn;
    for( n = 0; n < nFtnCnt; ++n )
    {
        pTxtFtn = pDoc->GetFtnIdxs()[ n ];
        const SwFmtFtn& rFtn = pTxtFtn->GetFtn();
        if ( rFtn.IsEndNote() != bEndNotes )
            continue;

        SwNodeIndex* pIdx = pTxtFtn->GetStartNode();
        if( pIdx )
        {
            SwNodeIndex aIdx( *pIdx, 1 );
            SwTxtNode* pTxtNd = aIdx.GetNode().GetTxtNode();
            if( !pTxtNd )
                pTxtNd = (SwTxtNode*)pDoc->GetNodes().GoNext( &aIdx );

            if( pTxtNd )
            {
                String sTxt( rFtn.GetViewNumStr( *pDoc ));
                if( sTxt.Len() )
                    sTxt += ' ';
                sTxt += pTxtNd->GetExpandTxt( 0, USHRT_MAX );

                _SeqFldLstElem* pNew = new _SeqFldLstElem( sTxt,
                                            pTxtFtn->GetSeqRefNo() );
                while( rList.InsertSort( pNew ) )
                    pNew->sDlgEntry += ' ';
            }
        }
    }

    return rList.Count();
}


// linken Rand ueber Objectleiste einstellen (aenhlich dem Stufen von
// Numerierungen)
BOOL SwEditShell::IsMoveLeftMargin( BOOL bRight, BOOL bModulus ) const
{
    BOOL bRet = TRUE;

    const SvxTabStopItem& rTabItem = (SvxTabStopItem&)GetDoc()->
                                GetDefault( RES_PARATR_TABSTOP );
    USHORT nDefDist = static_cast<USHORT>(rTabItem.Count() ? rTabItem[0].GetTabPos() : 1134);
    if( !nDefDist )
        return FALSE;

    FOREACHPAM_START(this)

        ULONG nSttNd = PCURCRSR->GetMark()->nNode.GetIndex(),
              nEndNd = PCURCRSR->GetPoint()->nNode.GetIndex();

        if( nSttNd > nEndNd )
        {
            ULONG nTmp = nSttNd; nSttNd = nEndNd; nEndNd = nTmp;
        }

        SwCntntNode* pCNd;
        for( ULONG n = nSttNd; bRet && n <= nEndNd; ++n )
            if( 0 != ( pCNd = GetDoc()->GetNodes()[ n ]->GetTxtNode() ))
            {
                const SvxLRSpaceItem& rLS = (SvxLRSpaceItem&)
                                            pCNd->GetAttr( RES_LR_SPACE );
                if( bRight )
                {
                    long nNext = rLS.GetTxtLeft() + nDefDist;
                    if( bModulus )
                        nNext = ( nNext / nDefDist ) * nDefDist;
                    SwFrm* pFrm = pCNd->GetFrm();
                    if ( pFrm )
                    {
                        const USHORT nFrmWidth = static_cast<USHORT>( pFrm->IsVertical() ?
                                                 pFrm->Frm().Height() :
                                                 pFrm->Frm().Width() );
                        bRet = nFrmWidth > ( nNext + MM50 );
                    }
                    else
                        bRet = FALSE;
                }
            }

        if( !bRet )
            break;

    FOREACHPAM_END()
    return bRet;
}

void SwEditShell::MoveLeftMargin( BOOL bRight, BOOL bModulus )
{
    StartAllAction();
    StartUndo( UNDO_START );

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr )         // Mehrfachselektion ?
    {
        SwPamRanges aRangeArr( *pCrsr );
        SwPaM aPam( *pCrsr->GetPoint() );
        for( USHORT n = 0; n < aRangeArr.Count(); ++n )
            GetDoc()->MoveLeftMargin( aRangeArr.SetPam( n, aPam ),
                                        bRight, bModulus );
    }
    else
        GetDoc()->MoveLeftMargin( *pCrsr, bRight, bModulus );

    EndUndo( UNDO_END );
    EndAllAction();
}


inline USHORT lcl_SetScriptFlags( USHORT nType )
{
    USHORT nRet;
       switch( nType )
    {
    case ::com::sun::star::i18n::ScriptType::LATIN:     nRet = SCRIPTTYPE_LATIN;    break;
    case ::com::sun::star::i18n::ScriptType::ASIAN:     nRet = SCRIPTTYPE_ASIAN;    break;
    case ::com::sun::star::i18n::ScriptType::COMPLEX:   nRet = SCRIPTTYPE_COMPLEX;  break;
    default: nRet = 0;
    }
    return nRet;
}

BOOL lcl_IsNoEndTxtAttrAtPos( const SwTxtNode& rTNd, xub_StrLen nPos,
                            USHORT &rScrpt, BOOL bInSelection, BOOL bNum )
{
    BOOL bRet = FALSE;
    const String& rTxt = rTNd.GetTxt();
    String sExp;

    // consider numbering
    if ( bNum )
    {
        bRet = FALSE;

        // --> OD 2008-03-19 #refactorlists#
        if ( rTNd.IsInList() )
        {
            ASSERT( rTNd.GetNumRule(),
                    "<lcl_IsNoEndTxtAttrAtPos(..)> - no list style found at text node. Serious defect -> please inform OD." );
            const SwNumRule* pNumRule = rTNd.GetNumRule();
            const SwNumFmt &rNumFmt = pNumRule->Get( static_cast<USHORT>(rTNd.GetActualListLevel()) );
            if( SVX_NUM_BITMAP != rNumFmt.GetNumberingType() )
            {
                if ( SVX_NUM_CHAR_SPECIAL == rNumFmt.GetNumberingType() )
                    sExp = rNumFmt.GetBulletChar();
                else
                    sExp = rTNd.GetNumString();
            }
        }
    }

    // and fields
    if ( CH_TXTATR_BREAKWORD == rTxt.GetChar( nPos ) )
    {
        const SwTxtAttr* const pAttr = rTNd.GetTxtAttrForCharAt( nPos );
        if (pAttr)
        {
            bRet = TRUE; // all other than fields can be
                         // defined as weak-script ?
            if ( RES_TXTATR_FIELD == pAttr->Which() )
            {
                const SwField* const pFld = pAttr->GetFld().GetFld();
                if (pFld)
                {
                    sExp += pFld->ExpandField(true);
                }
            }
        }
    }

    xub_StrLen nEnd = sExp.Len();
    if ( nEnd )
    {
        xub_StrLen n;
        if( bInSelection )
        {
            USHORT nScript;
            for( n = 0; n < nEnd; n = (xub_StrLen)
                    pBreakIt->GetBreakIter()->endOfScript( sExp, n, nScript ))
            {
                nScript = pBreakIt->GetBreakIter()->getScriptType( sExp, n );
                rScrpt |= lcl_SetScriptFlags( nScript );
            }
        }
        else
            rScrpt |= lcl_SetScriptFlags( pBreakIt->GetBreakIter()->
                                        getScriptType( sExp, nEnd-1 ));
    }

    return bRet;
}


// returns the scripttpye of the selection
USHORT SwEditShell::GetScriptType() const
{
    USHORT nRet = 0;
    //if( pBreakIt->GetBreakIter().is() )
    {
        FOREACHPAM_START(this)

            const SwPosition *pStt = PCURCRSR->Start(),
                             *pEnd = pStt == PCURCRSR->GetMark()
                                    ? PCURCRSR->GetPoint()
                                    : PCURCRSR->GetMark();
            if( pStt == pEnd || *pStt == *pEnd )
            {
                const SwTxtNode* pTNd = pStt->nNode.GetNode().GetTxtNode();
                if( pTNd )
                {
                    // try to get SwScriptInfo
                    const SwScriptInfo* pScriptInfo = SwScriptInfo::GetScriptInfo( *pTNd );

                    xub_StrLen nPos = pStt->nContent.GetIndex();
                    //Task 90448: we need the scripttype of the previous
                    //              position, if no selection exist!
                    if( nPos )
                    {
                        SwIndex aIdx( pStt->nContent );
                        if( pTNd->GoPrevious( &aIdx, CRSR_SKIP_CHARS ) )
                            nPos = aIdx.GetIndex();
                    }

                    USHORT nScript;

                    if ( pTNd->GetTxt().Len() )
                    {
                        nScript = pScriptInfo ?
                                  pScriptInfo->ScriptType( nPos ) :
                                  pBreakIt->GetBreakIter()->getScriptType( pTNd->GetTxt(), nPos );
                    }
                    else
                        nScript = GetI18NScriptTypeOfLanguage( (USHORT)GetAppLanguage() );

                    if( !lcl_IsNoEndTxtAttrAtPos( *pTNd, nPos, nRet, FALSE, FALSE ))
                        nRet |= lcl_SetScriptFlags( nScript );
                }
            }
            else if ( pBreakIt->GetBreakIter().is() )
            {
                ULONG nEndIdx = pEnd->nNode.GetIndex();
                SwNodeIndex aIdx( pStt->nNode );
                for( ; aIdx.GetIndex() <= nEndIdx; aIdx++ )
                    if( aIdx.GetNode().IsTxtNode() )
                    {
                        const SwTxtNode* pTNd = aIdx.GetNode().GetTxtNode();
                        const String& rTxt = pTNd->GetTxt();

                        // try to get SwScriptInfo
                        const SwScriptInfo* pScriptInfo = SwScriptInfo::GetScriptInfo( *pTNd );

                        xub_StrLen nChg = aIdx == pStt->nNode
                                                ? pStt->nContent.GetIndex()
                                                : 0,
                                    nEndPos = aIdx == nEndIdx
                                                ? pEnd->nContent.GetIndex()
                                                : rTxt.Len();

                        ASSERT( nEndPos <= rTxt.Len(), "Index outside the range - endless loop!" );
                        if( nEndPos > rTxt.Len() )
                            nEndPos = rTxt.Len();

                        USHORT nScript;
                        while( nChg < nEndPos )
                        {
                            nScript = pScriptInfo ?
                                      pScriptInfo->ScriptType( nChg ) :
                                      pBreakIt->GetBreakIter()->getScriptType(
                                                                rTxt, nChg );

                            if( !lcl_IsNoEndTxtAttrAtPos( *pTNd, nChg, nRet, TRUE,
                                                          0 == nChg && rTxt.Len() == nEndPos ) )
                                nRet |= lcl_SetScriptFlags( nScript );

                            if( (SCRIPTTYPE_LATIN | SCRIPTTYPE_ASIAN |
                                SCRIPTTYPE_COMPLEX) == nRet )
                                break;

                            xub_StrLen nFldPos = nChg+1;

                            nChg = pScriptInfo ?
                                   pScriptInfo->NextScriptChg( nChg ) :
                                   (xub_StrLen)pBreakIt->GetBreakIter()->endOfScript(
                                                    rTxt, nChg, nScript );

                            nFldPos = rTxt.Search(
                                            CH_TXTATR_BREAKWORD, nFldPos );
                            if( nFldPos < nChg )
                                nChg = nFldPos;
                        }
                        if( (SCRIPTTYPE_LATIN | SCRIPTTYPE_ASIAN |
                                SCRIPTTYPE_COMPLEX) == nRet )
                            break;
                    }
            }
            if( (SCRIPTTYPE_LATIN | SCRIPTTYPE_ASIAN |
                                SCRIPTTYPE_COMPLEX) == nRet )
                break;

        FOREACHPAM_END()
    }
    if( !nRet )
        nRet = SvtLanguageOptions::GetScriptTypeOfLanguage( LANGUAGE_SYSTEM );
    return nRet;
}


USHORT SwEditShell::GetCurLang() const
{
    const SwPaM* pCrsr = GetCrsr();
    const SwPosition& rPos = *pCrsr->GetPoint();
    const SwTxtNode* pTNd = rPos.nNode.GetNode().GetTxtNode();
    USHORT nLang;
    if( pTNd )
    {
        //JP 24.9.2001: if exist no selection, then get the language before
        //              the current character!
        xub_StrLen nPos = rPos.nContent.GetIndex();
        if( nPos && !pCrsr->HasMark() )
            --nPos;
        nLang = pTNd->GetLang( nPos );
    }
    else
        nLang = LANGUAGE_DONTKNOW;
    return nLang;
}

USHORT SwEditShell::GetScalingOfSelectedText() const
{
    const SwPaM* pCrsr = GetCrsr();
    const SwPosition* pStt = pCrsr->Start();
    const SwTxtNode* pTNd = pStt->nNode.GetNode().GetTxtNode();
    ASSERT( pTNd, "no textnode available" );

    USHORT nScaleWidth;
    if( pTNd )
    {
        xub_StrLen nStt = pStt->nContent.GetIndex(), nEnd;
        const SwPosition* pEnd = pStt == pCrsr->GetPoint()
                                        ? pCrsr->GetMark()
                                        : pCrsr->GetPoint();
        if( pStt->nNode == pEnd->nNode )
            nEnd = pEnd->nContent.GetIndex();
        else
            nEnd = pTNd->GetTxt().Len();
        nScaleWidth = pTNd->GetScalingOfSelectedText( nStt, nEnd );
    }
    else
        nScaleWidth = 100;              // default are no scaling -> 100%
    return nScaleWidth;
}
