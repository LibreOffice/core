/*************************************************************************
 *
 *  $RCSfile: acorrect.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-25 15:32:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#define _STD_VAR_ARRAYS

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <svx/langitem.hxx>
#endif

#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _TXTATR_HXX //autogen
#include <txtatr.hxx>
#endif
#ifndef _TXTINET_HXX //autogen
#include <txtinet.hxx>
#endif
#ifndef _FMTHBSH_HXX //autogen
#include <fmthbsh.hxx>
#endif
#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _ACORRECT_HXX
#include <acorrect.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _TEMPAUTO_HXX
#include <tempauto.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif
#ifndef _VISCRS_HXX
#include <viscrs.hxx>
#endif
#ifndef _SWFONT_HXX
#include <swfont.hxx>
#endif

class _PaMIntoCrsrShellRing
{
    SwCrsrShell& rSh;
    SwPaM &rDelPam, &rCrsr;
    Ring *pPrevDelPam, *pPrevCrsr;

    void RemoveFromRing( SwPaM& rPam, Ring* pPrev );
public:
    _PaMIntoCrsrShellRing( SwCrsrShell& rSh, SwPaM& rCrsr, SwPaM& rPam );
    ~_PaMIntoCrsrShellRing();
};

_PaMIntoCrsrShellRing::_PaMIntoCrsrShellRing( SwCrsrShell& rCSh,
                                            SwPaM& rShCrsr, SwPaM& rPam )
    : rSh( rCSh ), rDelPam( rPam ), rCrsr( rShCrsr )
{
    SwPaM* pShCrsr = rSh._GetCrsr();

    pPrevDelPam = rDelPam.GetPrev();
    pPrevCrsr = rCrsr.GetPrev();

    rDelPam.MoveRingTo( pShCrsr );
    rCrsr.MoveRingTo( pShCrsr );
}
_PaMIntoCrsrShellRing::~_PaMIntoCrsrShellRing()
{
    // und den Pam wieder herausnehmen:
    RemoveFromRing( rDelPam, pPrevDelPam );
    RemoveFromRing( rCrsr, pPrevCrsr );
}
void _PaMIntoCrsrShellRing::RemoveFromRing( SwPaM& rPam, Ring* pPrev )
{
    Ring *p, *pNext = (Ring*)&rPam;
    do {
        p = pNext;
        pNext = p->GetNext();
        p->MoveTo( &rPam );
    } while( p != pPrev );
}


SwAutoCorrDoc::SwAutoCorrDoc( SwEditShell& rEditShell, SwPaM& rPam,
                                sal_Unicode cIns )
    : rEditSh( rEditShell ), rCrsr( rPam ), pIdx( 0 ),
    nUndoId( cIns ? 0 : USHRT_MAX )
{
}


SwAutoCorrDoc::~SwAutoCorrDoc()
{
    if( nUndoId && USHRT_MAX != nUndoId )
        rEditSh.EndUndo( nUndoId );
    delete pIdx;
}

void SwAutoCorrDoc::DeleteSel( SwPaM& rDelPam )
{
    SwDoc* pDoc = rEditSh.GetDoc();
    if( pDoc->IsAutoFmtRedline() )
    {
        // damit der DelPam auch verschoben wird, in den Shell-Cursr-Ring
        // mit aufnehmen !!
        _PaMIntoCrsrShellRing aTmp( rEditSh, rCrsr, rDelPam );
        pDoc->DeleteAndJoin( rDelPam );
    }
    else
        pDoc->Delete( rDelPam );
}

BOOL SwAutoCorrDoc::Delete( xub_StrLen nStt, xub_StrLen nEnd )
{
    const SwNodeIndex& rNd = rCrsr.GetPoint()->nNode;
    SwPaM aSel( rNd, nStt, rNd, nEnd );
    DeleteSel( aSel );

    if( !nUndoId )
        nUndoId = USHRT_MAX;
    return TRUE;
}


BOOL SwAutoCorrDoc::Insert( xub_StrLen nPos, const String& rTxt )
{
    SwPaM aPam( rCrsr.GetPoint()->nNode.GetNode(), nPos );
    rEditSh.GetDoc()->Insert( aPam, rTxt );
    if( !nUndoId )
    {
        if( 1 == rTxt.Len() )
            rEditSh.StartUndo( nUndoId = UNDO_AUTOCORRECT );
        else
            nUndoId = USHRT_MAX;
    }
    return TRUE;
}


BOOL SwAutoCorrDoc::Replace( xub_StrLen nPos, const String& rTxt )
{
    SwPaM* pPam = &rCrsr;
    if( pPam->GetPoint()->nContent.GetIndex() != nPos )
    {
        pPam = new SwPaM( *rCrsr.GetPoint() );
        pPam->GetPoint()->nContent = nPos;
    }

    BOOL bChg = TRUE;
    SwTxtNode* pNd = pPam->GetNode()->GetTxtNode();
    if( pNd )
    {
        // TextAttribute ohne Ende duerfen nie ersetzt werden!
        sal_Unicode cChr;
        for( xub_StrLen n = 0, nLen = rTxt.Len(); n < nLen; ++n )
            if( ( CH_TXTATR_BREAKWORD == (cChr = pNd->GetTxt().
                    GetChar( n + nPos )) || CH_TXTATR_INWORD == cChr ) &&
                pNd->GetTxtAttr( n + nPos ) )
            {
                bChg = FALSE;
                break;
            }
    }

    if( bChg )
    {
        SwDoc* pDoc = rEditSh.GetDoc();
        SwRedlineMode eOld = pDoc->GetRedlineMode();

//      if( !pDoc->IsAutoFmtRedline() &&
//          pPam != &rCrsr )    // nur an akt. Position das Redline sichern
//          pDoc->SetRedlineMode_intern( eOld | REDLINE_IGNORE );

        if( pDoc->IsAutoFmtRedline() )
        {
            if( nPos == pNd->GetTxt().Len() )       // am Ende erfolgt ein Insert
                pDoc->Insert( *pPam, rTxt );
            else
            {
                _PaMIntoCrsrShellRing aTmp( rEditSh, rCrsr, *pPam );

                pPam->SetMark();
                pPam->GetPoint()->nContent = Min( pNd->GetTxt().Len(),
                                              xub_StrLen( nPos + rTxt.Len() ));
                pDoc->Replace( *pPam, rTxt, FALSE );
                pPam->Exchange();
                pPam->DeleteMark();
            }
        }
        else
            pDoc->Overwrite( *pPam, rTxt );

//      pDoc->SetRedlineMode_intern( eOld );
        if( !nUndoId )
        {
            if( 1 == rTxt.Len() )
                rEditSh.StartUndo( nUndoId = UNDO_AUTOCORRECT );
            else
                nUndoId = USHRT_MAX;
        }
    }

    if( pPam != &rCrsr )
        delete pPam;

    return TRUE;
}



BOOL SwAutoCorrDoc::SetAttr( xub_StrLen nStt, xub_StrLen nEnd, USHORT nSlotId,
                                        SfxPoolItem& rItem )
{
    const SwNodeIndex& rNd = rCrsr.GetPoint()->nNode;
    SwPaM aPam( rNd, nStt, rNd, nEnd );

    USHORT nWhich = rEditSh.GetDoc()->GetAttrPool().GetWhich( nSlotId, FALSE );
    if( nWhich )
    {
        rItem.SetWhich( nWhich );
        rEditSh.GetDoc()->SetFmtItemByAutoFmt( aPam, rItem );

        if( !nUndoId )
            nUndoId = USHRT_MAX;
    }
    return 0 != nWhich;
}



BOOL SwAutoCorrDoc::SetINetAttr( xub_StrLen nStt, xub_StrLen nEnd, const String& rURL )
{
    const SwNodeIndex& rNd = rCrsr.GetPoint()->nNode;
    SwPaM aPam( rNd, nStt, rNd, nEnd );

    rEditSh.GetDoc()->SetFmtItemByAutoFmt( aPam,
                                            SwFmtINetFmt( rURL, aEmptyStr ));
    if( !nUndoId )
        nUndoId = USHRT_MAX;
    return TRUE;
}

    // returne den Text eines vorherigen Absatzes.
    // Dieser darf nicht leer sein!
    // Gibt es diesen nicht oder gibt es davor nur Leere, dann returne 0
    // Das Flag gibt an:
    //      TRUE: den, vor der normalen Einfuegeposition (TRUE)
    //      FALSE: den, in den das korrigierte Wort eingfuegt wurde.
    //              (Muss nicht der gleiche Absatz sein!!!!)
const String* SwAutoCorrDoc::GetPrevPara( BOOL bAtNormalPos )
{
    const String* pStr = 0;

    if( bAtNormalPos || !pIdx )
        pIdx = new SwNodeIndex( rCrsr.GetPoint()->nNode, -1 );
    else
        (*pIdx)--;

    SwTxtNode* pTNd = pIdx->GetNode().GetTxtNode();
    while( pTNd && !pTNd->GetTxt().Len() )
    {
        (*pIdx)--;
        pTNd = pIdx->GetNode().GetTxtNode();
    }

    if( pTNd && NO_NUMBERING == pTNd->GetTxtColl()->GetOutlineLevel() )
        pStr = &pTNd->GetTxt();

    if( !nUndoId )
        nUndoId = USHRT_MAX;
    return pStr;
}


BOOL SwAutoCorrDoc::ChgAutoCorrWord( xub_StrLen & rSttPos, xub_StrLen nEndPos,
                                            SvxAutoCorrect& rACorrect,
                                            const String** ppPara )
{
    if( !nUndoId )
        nUndoId = USHRT_MAX;

    // Absatz-Anfang oder ein Blank gefunden, suche nach dem Wort
    // Kuerzel im Auto
    SwTxtNode* pTxtNd = rCrsr.GetNode()->GetTxtNode();
    ASSERT( pTxtNd, "wo ist denn der TextNode?" );

    BOOL bRet = FALSE;
    if( nEndPos == rSttPos )
        return bRet;

    LanguageType eLang = GetLanguage(nEndPos, FALSE);
    if(LANGUAGE_SYSTEM == eLang)
        eLang = ::GetSystemLang();

    //JP 22.04.99: Bug 63883 - Sonderbehandlung fuer Punkte.
    BOOL bLastCharIsPoint = nEndPos < pTxtNd->GetTxt().Len() &&
                            '.' == pTxtNd->GetTxt().GetChar( nEndPos );

    const SvxAutocorrWord* pFnd = rACorrect.SearchWordsInList(
                                pTxtNd->GetTxt(), rSttPos, nEndPos, *this, eLang );
    SwDoc* pDoc = rEditSh.GetDoc();
    if( pFnd )
    {
        const SwNodeIndex& rNd = rCrsr.GetPoint()->nNode;
        SwPaM aPam( rNd, rSttPos, rNd, nEndPos );

        if( pFnd->IsTextOnly() )
        {
            //JP 22.04.99: Bug 63883 - Sonderbehandlung fuer Punkte.
            if( !bLastCharIsPoint || !pFnd->GetLong().Len() ||
                '.' != pFnd->GetLong().GetChar( pFnd->GetLong().Len() - 1 ) )
            {
                // dann mal ersetzen
                DeleteSel( aPam );
                pDoc->DontExpandFmt( *aPam.GetPoint() );
                pDoc->Insert( aPam, pFnd->GetLong() );
                bRet = TRUE;
            }
        }
        else
        {
            SwTextBlocks aTBlks( rACorrect.GetAutoCorrFileName( eLang ));
            USHORT nPos = aTBlks.GetIndex( pFnd->GetShort() );
            if( USHRT_MAX != nPos && aTBlks.BeginGetDoc( nPos ) )
            {
                DeleteSel( aPam );
                pDoc->DontExpandFmt( *aPam.GetPoint() );

                if( ppPara )
                {
                    ASSERT( !pIdx, "wer hat seinen Index nicht geloescht?" );
                    pIdx = new SwNodeIndex( rCrsr.GetPoint()->nNode, -1 );
                }

                //
                SwDoc* pAutoDoc = aTBlks.GetDoc();
                SwNodeIndex aSttIdx( pAutoDoc->GetNodes().GetEndOfExtras(), 1 );
                SwCntntNode* pCntntNd = pAutoDoc->GetNodes().GoNext( &aSttIdx );
                SwPaM aCpyPam( aSttIdx );

                const SwTableNode* pTblNd = pCntntNd->FindTableNode();
                if( pTblNd )
                {
                    aCpyPam.GetPoint()->nContent.Assign( 0, 0 );
                    aCpyPam.GetPoint()->nNode = *pTblNd;
                }
                aCpyPam.SetMark();

                // dann bis zum Ende vom Nodes Array
                aCpyPam.GetPoint()->nNode.Assign( pAutoDoc->GetNodes().GetEndOfContent(), -1 );
                pCntntNd = aCpyPam.GetCntntNode();
                aCpyPam.GetPoint()->nContent.Assign( pCntntNd, pCntntNd->Len() );

                SwDontExpandItem aExpItem;
                aExpItem.SaveDontExpandItems( *aPam.GetPoint() );

                pAutoDoc->Copy( aCpyPam, *aPam.GetPoint() );

                aExpItem.RestoreDontExpandItems( *aPam.GetPoint() );

                if( ppPara )
                {
                    (*pIdx)++;
                    pTxtNd = pIdx->GetNode().GetTxtNode();
                }
                bRet = TRUE;
            }
            aTBlks.EndGetDoc();
        }
    }
    else if( pTempAuto )
    {
        String sKurz( pTxtNd->GetTxt().Copy( rSttPos, nEndPos - rSttPos ));
        // die temporaere Autokorrektur schlaegt zu
        const SwCorrection* pCorr = pTempAuto->Replaceable( sKurz );
        //JP 22.04.99: Bug 63883 - Sonderbehandlung fuer Punkte.
        if( pCorr && ( !bLastCharIsPoint || !pCorr->Correct().Len() ||
            '.' != pCorr->Correct().GetChar( pCorr->Correct().Len() - 1 )) )
        {
            const SwNodeIndex& rNd = rCrsr.GetPoint()->nNode;
            SwPaM aPam( rNd, rSttPos, rNd, nEndPos );

            DeleteSel( aPam );

            pDoc->DontExpandFmt( *aPam.GetPoint() );
            pDoc->Insert( aPam, pCorr->Correct() );
            bRet = TRUE;
        }
    }

    if( bRet && ppPara && pTxtNd )
        *ppPara = &pTxtNd->GetTxt();

    return bRet;
}


    // wird nach dem austauschen der Zeichen von den Funktionen
    //  - FnCptlSttWrd
    //  - FnCptlSttSntnc
    // gerufen. Dann koennen die Worte ggfs. in die Ausnahmelisten
    // aufgenommen werden.
void SwAutoCorrDoc::SaveCpltSttWord( ULONG nFlag, xub_StrLen nPos,
                                            const String& rExceptWord,
                                            sal_Unicode cChar )
{
    ULONG nNode = pIdx ? pIdx->GetIndex() : rCrsr.GetPoint()->nNode.GetIndex();
    LanguageType eLang = GetLanguage(nPos, FALSE);
    rEditSh.GetDoc()->SetAutoCorrExceptWord( new SwAutoCorrExceptWord( nFlag,
                                        nNode, nPos, rExceptWord, cChar, eLang ));
}

LanguageType SwAutoCorrDoc::GetLanguage( xub_StrLen nPos, BOOL bPrevPara ) const
{
    LanguageType eRet = LANGUAGE_SYSTEM;
    ULONG nNode = pIdx ? pIdx->GetIndex() : rCrsr.GetPoint()->nNode.GetIndex();

    SwTxtNode* pNd = (( bPrevPara && pIdx )
                            ? *pIdx
                            : rCrsr.GetPoint()->nNode ).GetNode().GetTxtNode();

    if( pNd )
    {
        SfxItemSet aSet( rEditSh.GetDoc()->GetAttrPool(),
                        RES_CHRATR_LANGUAGE, RES_CHRATR_LANGUAGE );
        pNd->GetAttr( aSet, nPos, nPos );
        eRet = ((const SvxLanguageItem&)aSet.Get( RES_CHRATR_LANGUAGE ))
                                .GetLanguage();
    }
    if(LANGUAGE_SYSTEM == eRet)
        eRet = ::GetSystemLang();
    return eRet;
}

void SwAutoCorrExceptWord::CheckChar( const SwPosition& rPos, sal_Unicode cChr )
{
    // nur testen ob es eine Verbesserung ist. Wenn ja, dann das Wort
    // in die Ausnahmeliste aufnehmen.
    if( cChar == cChr && rPos.nNode.GetIndex() == nNode &&
        rPos.nContent.GetIndex() == nCntnt )
    {
        // die akt. Autokorrektur besorgen:
        SvxAutoCorrect* pACorr = OFF_APP()->GetAutoCorrect();

        // dann in die Liste aufnehmen:
        if( CptlSttWrd & nFlags )
            pACorr->AddWrtSttException( sWord, eLanguage );
        else if( CptlSttSntnc & nFlags )
            pACorr->AddCplSttException( sWord, eLanguage );
    }
}


BOOL SwAutoCorrExceptWord::CheckDelChar( const SwPosition& rPos )
{
    BOOL bRet = FALSE;
    if( !bDeleted && rPos.nNode.GetIndex() == nNode &&
        rPos.nContent.GetIndex() == nCntnt )
        bDeleted = bRet = TRUE;
    return bRet;
}

SwDontExpandItem::~SwDontExpandItem()
{
    delete pDontExpItems;
}

void SwDontExpandItem::SaveDontExpandItems( const SwPosition& rPos )
{
    const SwTxtNode* pTxtNd = rPos.nNode.GetNode().GetTxtNode();
    if( pTxtNd )
    {
        pDontExpItems = new SfxItemSet( ((SwDoc*)pTxtNd->GetDoc())->GetAttrPool(),
                                            aCharFmtSetRange );
        xub_StrLen n = rPos.nContent.GetIndex();
        if( !pTxtNd->GetAttr( *pDontExpItems, n, n,
                                n != pTxtNd->GetTxt().Len() ))
            delete pDontExpItems, pDontExpItems = 0;
    }
}

void SwDontExpandItem::RestoreDontExpandItems( const SwPosition& rPos )
{
    SwTxtNode* pTxtNd = rPos.nNode.GetNode().GetTxtNode();
    if( pTxtNd )
    {
        xub_StrLen nStart = rPos.nContent.GetIndex();
        if( nStart == pTxtNd->GetTxt().Len() )
            pTxtNd->FmtToTxtAttr( pTxtNd );

        if( pTxtNd->GetpSwpHints() && pTxtNd->GetpSwpHints()->Count() )
        {
            const USHORT nSize = pTxtNd->GetpSwpHints()->Count();
            register USHORT n;
            xub_StrLen nAttrStart;
            register const xub_StrLen* pAttrEnd;

            for( n = 0; n < nSize; ++n )
            {
                SwTxtAttr* pHt = pTxtNd->GetpSwpHints()->GetHt( n );
                nAttrStart = *pHt->GetStart();
                if( nAttrStart > nStart )       // ueber den Bereich hinaus
                    break;

                if( 0 != ( pAttrEnd = pHt->GetEnd() ) &&
                    ( ( nAttrStart < nStart &&
                        ( pHt->DontExpand() ? nStart < *pAttrEnd
                                            : nStart <= *pAttrEnd )) ||
                      ( nStart == nAttrStart &&
                        ( nAttrStart == *pAttrEnd || !nStart ))) )
                {
                    const SfxPoolItem* pItem;
                    if( !pDontExpItems || SFX_ITEM_SET != pDontExpItems->
                        GetItemState( pHt->Which(), FALSE, &pItem ) ||
                        *pItem != pHt->GetAttr() )
                    {
                        // das Attribut war vorher nicht in dieser Form im Absatz
                        // gesetzt, also kann es nur durchs einfuegen/kopieren erzeugt
                        // worden sein. Damit ist es ein Kandiadat fuers DontExpand
                        pHt->SetDontExpand( TRUE );
                    }
                }
            }
        }
    }
}


