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

#include <svx/svxids.hrc>
#include <editeng/langitem.hxx>
#include <fmtinfmt.hxx>
#include <txtatr.hxx>
#include <txtinet.hxx>
#include <editsh.hxx>
#include <doc.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <acorrect.hxx>
#include <shellio.hxx>
#include <swundo.hxx>
#include <viscrs.hxx>

#include <editeng/acorrcfg.hxx>

using namespace ::com::sun::star;


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
    : rEditSh( rEditShell ), rCrsr( rPam ), pIdx( 0 )
    , m_nEndUndoCounter(0)
    , bUndoIdInitialized( cIns ? false : true )
{
}


SwAutoCorrDoc::~SwAutoCorrDoc()
{
    for (int i = 0; i < m_nEndUndoCounter; ++i)
    {
        rEditSh.EndUndo();
    }
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
    {
        pDoc->DeleteRange( rDelPam );
    }
}

sal_Bool SwAutoCorrDoc::Delete( xub_StrLen nStt, xub_StrLen nEnd )
{
    const SwNodeIndex& rNd = rCrsr.GetPoint()->nNode;
    SwPaM aSel( rNd, nStt, rNd, nEnd );
    DeleteSel( aSel );

    if( bUndoIdInitialized )
        bUndoIdInitialized = true;
    return sal_True;
}


sal_Bool SwAutoCorrDoc::Insert( xub_StrLen nPos, const String& rTxt )
{
    SwPaM aPam( rCrsr.GetPoint()->nNode.GetNode(), nPos );
    rEditSh.GetDoc()->InsertString( aPam, rTxt );
    if( !bUndoIdInitialized )
    {
        bUndoIdInitialized = true;
        if( 1 == rTxt.Len() )
        {
            rEditSh.StartUndo( UNDO_AUTOCORRECT );
            ++m_nEndUndoCounter;
        }
    }
    return sal_True;
}


sal_Bool SwAutoCorrDoc::Replace( xub_StrLen nPos, const String& rTxt )
{
    return ReplaceRange( nPos, rTxt.Len(), rTxt );
}
sal_Bool SwAutoCorrDoc::ReplaceRange( xub_StrLen nPos, xub_StrLen nSourceLength, const String& rTxt )
{
    SwPaM* pPam = &rCrsr;
    if( pPam->GetPoint()->nContent.GetIndex() != nPos )
    {
        pPam = new SwPaM( *rCrsr.GetPoint() );
        pPam->GetPoint()->nContent = nPos;
    }

    SwTxtNode * const pNd = pPam->GetNode()->GetTxtNode();
    if ( !pNd )
    {
        return sal_False;
    }

    // text attributes with dummy characters must not be replaced!
    bool bDoReplace = true;
    xub_StrLen const nLen = rTxt.Len();
    for ( xub_StrLen n = 0; n < nLen; ++n )
    {
        sal_Unicode const Char = pNd->GetTxt().GetChar( n + nPos );
        if ( ( CH_TXTATR_BREAKWORD == Char || CH_TXTATR_INWORD == Char )
             && pNd->GetTxtAttrForCharAt( n + nPos ) )
        {
            bDoReplace = false;
            break;
        }
    }

    if ( bDoReplace )
    {
        SwDoc* pDoc = rEditSh.GetDoc();

        if( pDoc->IsAutoFmtRedline() )
        {
            if( nPos == pNd->GetTxt().Len() )       // am Ende erfolgt ein Insert
            {
                pDoc->InsertString( *pPam, rTxt );
            }
            else
            {
                _PaMIntoCrsrShellRing aTmp( rEditSh, rCrsr, *pPam );

                pPam->SetMark();
                pPam->GetPoint()->nContent = Min( pNd->GetTxt().Len(),
                                              xub_StrLen( nPos + nSourceLength ));
                pDoc->ReplaceRange( *pPam, rTxt, false );
                pPam->Exchange();
                pPam->DeleteMark();
            }
        }
        else
        {
            if( nSourceLength != rTxt.Len() )
            {
                pPam->SetMark();
                pPam->GetPoint()->nContent = Min( pNd->GetTxt().Len(),
                                              xub_StrLen( nPos + nSourceLength ));
                pDoc->ReplaceRange( *pPam, rTxt, false );
                pPam->Exchange();
                pPam->DeleteMark();
            }
            else
                pDoc->Overwrite( *pPam, rTxt );
        }

//      pDoc->SetRedlineMode_intern( eOld );
        if( bUndoIdInitialized )
        {
            bUndoIdInitialized = true;
            if( 1 == rTxt.Len() )
            {
                rEditSh.StartUndo( UNDO_AUTOCORRECT );
                ++m_nEndUndoCounter;
            }
        }
    }

    if( pPam != &rCrsr )
        delete pPam;

    return sal_True;
}



sal_Bool SwAutoCorrDoc::SetAttr( xub_StrLen nStt, xub_StrLen nEnd, sal_uInt16 nSlotId,
                                        SfxPoolItem& rItem )
{
    const SwNodeIndex& rNd = rCrsr.GetPoint()->nNode;
    SwPaM aPam( rNd, nStt, rNd, nEnd );

    SfxItemPool& rPool = rEditSh.GetDoc()->GetAttrPool();
    sal_uInt16 nWhich = rPool.GetWhich( nSlotId, sal_False );
    if( nWhich )
    {
        rItem.SetWhich( nWhich );

        SfxItemSet aSet( rPool, aCharFmtSetRange );
        SetAllScriptItem( aSet, rItem );

        rEditSh.GetDoc()->SetFmtItemByAutoFmt( aPam, aSet );

        if( bUndoIdInitialized )
            bUndoIdInitialized = true;
    }
    return 0 != nWhich;
}



sal_Bool SwAutoCorrDoc::SetINetAttr( xub_StrLen nStt, xub_StrLen nEnd, const String& rURL )
{
    const SwNodeIndex& rNd = rCrsr.GetPoint()->nNode;
    SwPaM aPam( rNd, nStt, rNd, nEnd );

    SfxItemSet aSet( rEditSh.GetDoc()->GetAttrPool(),
                        RES_TXTATR_INETFMT, RES_TXTATR_INETFMT );
    aSet.Put( SwFmtINetFmt( rURL, aEmptyStr ));
    rEditSh.GetDoc()->SetFmtItemByAutoFmt( aPam, aSet );
    if( bUndoIdInitialized )
        bUndoIdInitialized = true;
    return sal_True;
}

    // returne den Text eines vorherigen Absatzes.
    // Dieser darf nicht leer sein!
    // Gibt es diesen nicht oder gibt es davor nur Leere, dann returne 0
    // Das Flag gibt an:
    //      sal_True: den, vor der normalen Einfuegeposition (sal_True)
    //      sal_False: den, in den das korrigierte Wort eingfuegt wurde.
    //              (Muss nicht der gleiche Absatz sein!!!!)
const String* SwAutoCorrDoc::GetPrevPara( sal_Bool bAtNormalPos )
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
    if( pTNd && 0 == pTNd->GetAttrOutlineLevel() )//#outline level,zhaojianwei
        pStr = &pTNd->GetTxt();

    if( bUndoIdInitialized )
        bUndoIdInitialized = true;
    return pStr;
}


sal_Bool SwAutoCorrDoc::ChgAutoCorrWord( xub_StrLen & rSttPos, xub_StrLen nEndPos,
                                            SvxAutoCorrect& rACorrect,
                                            const String** ppPara )
{
    if( bUndoIdInitialized )
        bUndoIdInitialized = true;

    // Absatz-Anfang oder ein Blank gefunden, suche nach dem Wort
    // Kuerzel im Auto
    SwTxtNode* pTxtNd = rCrsr.GetNode()->GetTxtNode();
    OSL_ENSURE( pTxtNd, "wo ist denn der TextNode?" );

    sal_Bool bRet = sal_False;
    if( nEndPos == rSttPos )
        return bRet;

    LanguageType eLang = GetLanguage(nEndPos, sal_False);
    if(LANGUAGE_SYSTEM == eLang)
        eLang = (LanguageType)GetAppLanguage();

    //JP 22.04.99: Bug 63883 - Sonderbehandlung fuer Punkte.
    sal_Bool bLastCharIsPoint = nEndPos < pTxtNd->GetTxt().Len() &&
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
                // replace the selection
                pDoc->ReplaceRange( aPam, pFnd->GetLong(), false);
                bRet = sal_True;
            }
        }
        else
        {
            SwTextBlocks aTBlks( rACorrect.GetAutoCorrFileName( eLang, sal_False, sal_True ));
            sal_uInt16 nPos = aTBlks.GetIndex( pFnd->GetShort() );
            if( USHRT_MAX != nPos && aTBlks.BeginGetDoc( nPos ) )
            {
                DeleteSel( aPam );
                pDoc->DontExpandFmt( *aPam.GetPoint() );

                if( ppPara )
                {
                    OSL_ENSURE( !pIdx, "wer hat seinen Index nicht geloescht?" );
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

                pAutoDoc->CopyRange( aCpyPam, *aPam.GetPoint(), false );

                aExpItem.RestoreDontExpandItems( *aPam.GetPoint() );

                if( ppPara )
                {
                    ++(*pIdx);
                    pTxtNd = pIdx->GetNode().GetTxtNode();
                }
                bRet = sal_True;
            }
            aTBlks.EndGetDoc();
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
void SwAutoCorrDoc::SaveCpltSttWord( sal_uLong nFlag, xub_StrLen nPos,
                                            const String& rExceptWord,
                                            sal_Unicode cChar )
{
    sal_uLong nNode = pIdx ? pIdx->GetIndex() : rCrsr.GetPoint()->nNode.GetIndex();
    LanguageType eLang = GetLanguage(nPos, sal_False);
    rEditSh.GetDoc()->SetAutoCorrExceptWord( new SwAutoCorrExceptWord( nFlag,
                                        nNode, nPos, rExceptWord, cChar, eLang ));
}

LanguageType SwAutoCorrDoc::GetLanguage( xub_StrLen nPos, sal_Bool bPrevPara ) const
{
    LanguageType eRet = LANGUAGE_SYSTEM;

    SwTxtNode* pNd = (( bPrevPara && pIdx )
                            ? *pIdx
                            : rCrsr.GetPoint()->nNode ).GetNode().GetTxtNode();

    if( pNd )
        eRet = pNd->GetLang( nPos, 0 );
    if(LANGUAGE_SYSTEM == eRet)
        eRet = (LanguageType)GetAppLanguage();
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
        SvxAutoCorrect* pACorr = SvxAutoCorrCfg::Get().GetAutoCorrect();

        // dann in die Liste aufnehmen:
        if( CptlSttWrd & nFlags )
            pACorr->AddWrtSttException( sWord, eLanguage );
        else if( CptlSttSntnc & nFlags )
            pACorr->AddCplSttException( sWord, eLanguage );
    }
}


sal_Bool SwAutoCorrExceptWord::CheckDelChar( const SwPosition& rPos )
{
    sal_Bool bRet = sal_False;
    if( !bDeleted && rPos.nNode.GetIndex() == nNode &&
        rPos.nContent.GetIndex() == nCntnt )
        bDeleted = bRet = sal_True;
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
            const sal_uInt16 nSize = pTxtNd->GetpSwpHints()->Count();
            sal_uInt16 n;
            xub_StrLen nAttrStart;
            const xub_StrLen* pAttrEnd;

            for( n = 0; n < nSize; ++n )
            {
                SwTxtAttr* pHt = pTxtNd->GetpSwpHints()->GetTextHint( n );
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
                        GetItemState( pHt->Which(), sal_False, &pItem ) ||
                        *pItem != pHt->GetAttr() )
                    {
                        // das Attribut war vorher nicht in dieser Form im Absatz
                        // gesetzt, also kann es nur durchs einfuegen/kopieren erzeugt
                        // worden sein. Damit ist es ein Kandiadat fuers DontExpand
                        pHt->SetDontExpand( sal_True );
                    }
                }
            }
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
