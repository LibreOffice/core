/*************************************************************************
 *
 *  $RCSfile: undobj.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 14:38:18 $
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


#pragma hdrstop

#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FTNIDX_HXX //autogen
#include <ftnidx.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>           // fuer die UndoIds
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif
#ifndef _ROLBCK_HXX
#include <rolbck.hxx>
#endif
#ifndef _NDNOTXT_HXX
#include <ndnotxt.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>
#endif
#ifndef _MVSAVE_HXX
#include <mvsave.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif


class SwRedlineSaveData : public SwUndRng, public SwRedlineData,
                          private SwUndoSaveSection
{
public:
    SwRedlineSaveData( SwComparePosition eCmpPos,
                        const SwPosition& rSttPos, const SwPosition& rEndPos,
                        SwRedline& rRedl, BOOL bCopyNext );
    ~SwRedlineSaveData();
    void RedlineToDoc( SwPaM& rPam );
    SwNodeIndex* GetMvSttIdx() const
        { return SwUndoSaveSection::GetMvSttIdx(); }

#ifndef PRODUCT
    USHORT nRedlineCount;
#endif
};

SV_IMPL_PTRARR( SwUndos, SwUndo*)
SV_IMPL_PTRARR( SwRedlineSaveDatas, SwRedlineSaveDataPtr )

SwUndoIter::SwUndoIter( SwPaM* pPam, USHORT nId )
{
    nUndoId = nId;
    bWeiter = nId ? TRUE : FALSE;
    bUpdateAttr = FALSE;
    pAktPam = pPam;
    nEndCnt = 0;
    pSelFmt = 0;
    pMarkList = 0;
}
inline SwDoc& SwUndoIter::GetDoc() const { return *pAktPam->GetDoc(); }

//------------------------------------------------------------

// Diese Klasse speichert den Pam als USHORT's und kann diese wieder zu

// einem PaM zusammensetzen
SwUndRng::SwUndRng()
    : nSttNode( 0 ), nEndNode( 0 ), nSttCntnt( 0 ), nEndCntnt( 0 )
{
}

SwUndRng::SwUndRng( const SwPaM& rPam )
{
    SetValues( rPam );
}

void SwUndRng::SetValues( const SwPaM& rPam )
{
    const SwPosition *pStt = rPam.Start();
    if( rPam.HasMark() )
    {
        const SwPosition *pEnd = rPam.GetPoint() == pStt
                        ? rPam.GetMark()
                        : rPam.GetPoint();
        nEndNode = pEnd->nNode.GetIndex();
        nEndCntnt = pEnd->nContent.GetIndex();
    }
    else
        // keine Selektion !!
        nEndNode = 0, nEndCntnt = STRING_MAXLEN;

    nSttNode = pStt->nNode.GetIndex();
    nSttCntnt = pStt->nContent.GetIndex();
}

void SwUndRng::SetPaM( SwPaM & rPam, BOOL bCorrToCntnt ) const
{
    rPam.DeleteMark();
    rPam.GetPoint()->nNode = nSttNode;
    SwNode* pNd = rPam.GetNode();
    if( pNd->IsCntntNode() )
        rPam.GetPoint()->nContent.Assign( pNd->GetCntntNode(), nSttCntnt );
    else if( bCorrToCntnt )
        rPam.Move( fnMoveForward, fnGoCntnt );
    else
        rPam.GetPoint()->nContent.Assign( 0, 0 );

    if( !nEndNode && STRING_MAXLEN == nEndCntnt )       // keine Selection
        return ;

    rPam.SetMark();
    if( nSttNode == nEndNode && nSttCntnt == nEndCntnt )
        return;                             // nichts mehr zu tun

    rPam.GetPoint()->nNode = nEndNode;
    if( (pNd = rPam.GetNode())->IsCntntNode() )
        rPam.GetPoint()->nContent.Assign( pNd->GetCntntNode(), nEndCntnt );
    else if( bCorrToCntnt )
        rPam.Move( fnMoveBackward, fnGoCntnt );
    else
        rPam.GetPoint()->nContent.Assign( 0, 0 );
}

void SwUndRng::SetPaM( SwUndoIter& rIter, BOOL bCorrToCntnt ) const
{
    if( rIter.pAktPam )
        SetPaM( *rIter.pAktPam, bCorrToCntnt );
}

//------------------------------------------------------------


void SwUndo::RemoveIdxFromSection( SwDoc& rDoc, ULONG nSttIdx,
                                    ULONG* pEndIdx )
{
    SwNodeIndex aIdx( rDoc.GetNodes(), nSttIdx );
    SwNodeIndex aEndIdx( rDoc.GetNodes(), pEndIdx ? *pEndIdx
                                    : aIdx.GetNode().EndOfSectionIndex() );
    SwPosition aPos( rDoc.GetNodes().GetEndOfPostIts() );
    rDoc.CorrAbs( aIdx, aEndIdx, aPos, TRUE );
}

void SwUndo::RemoveIdxFromRange( SwPaM& rPam, BOOL bMoveNext )
{
    const SwPosition* pEnd = rPam.End();
    if( bMoveNext )
    {
        if( pEnd != rPam.GetPoint() )
            rPam.Exchange();

        SwNodeIndex aStt( rPam.GetMark()->nNode );
        SwNodeIndex aEnd( rPam.GetPoint()->nNode );

        if( !rPam.Move( fnMoveForward ) )
        {
            rPam.Exchange();
            if( !rPam.Move( fnMoveBackward ) )
            {
                rPam.GetPoint()->nNode = rPam.GetDoc()->GetNodes().GetEndOfPostIts();
                rPam.GetPoint()->nContent.Assign( 0, 0 );
            }
        }

        rPam.GetDoc()->CorrAbs( aStt, aEnd, *rPam.GetPoint(), TRUE );
    }
    else
        rPam.GetDoc()->CorrAbs( rPam, *pEnd, TRUE );
}

void SwUndo::RemoveIdxRel( ULONG nIdx, const SwPosition& rPos )
{
    // nur die Crsr verschieben; die Bookmarks/TOXMarks/.. werden vom
    // entsp. JoinNext/JoinPrev erledigt!
    SwNodeIndex aIdx( rPos.nNode.GetNode().GetNodes(), nIdx );
    ::PaMCorrRel( aIdx, rPos );
}

SwUndo::~SwUndo()
{
}

void SwUndo::Repeat( SwUndoIter& rIter )
{
    rIter.pLastUndoObj = this;
}

//------------------------------------------------------------

SwUndoSaveCntnt::SwUndoSaveCntnt()
    : pHistory( 0 )
{}

SwUndoSaveCntnt::~SwUndoSaveCntnt()
{
    delete pHistory;
}

    // wird fuer das Loeschen von Inhalt benoetigt. Fuer das ReDo werden
    // Inhalte in das UndoNodesArray verschoben. Diese Methoden fuegen
    // am Ende eines TextNodes fuer die Attribute einen Trenner ein.
    // Dadurch werden die Attribute nicht expandiert.
    // MoveTo..     verschiebt aus dem NodesArray in das UndoNodesArray
    // MoveFrom..   verschiebt aus dem UndoNodesArray in das NodesArray

    // 2.8.93:  ist pEndNdIdx angebenen, wird vom Undo/Redo -Ins/DelFly
    //          aufgerufen. Dann soll die gesamte Section verschoben werden.

void SwUndoSaveCntnt::MoveToUndoNds( SwPaM& rPaM, SwNodeIndex* pNodeIdx,
                    SwIndex* pCntIdx, ULONG* pEndNdIdx, xub_StrLen* pEndCntIdx )
{
    SwDoc& rDoc = *rPaM.GetDoc();
    BOOL bUndo = rDoc.DoesUndo();
    rDoc.DoUndo( FALSE );

    SwNoTxtNode* pCpyNd = rPaM.GetNode()->GetNoTxtNode();

    // jetzt kommt das eigentliche Loeschen(Verschieben)
    SwNodes& rNds = (SwNodes&)*rDoc.GetUndoNds();
    SwPosition aPos( pEndNdIdx ? rNds.GetEndOfPostIts()
                               : rNds.GetEndOfExtras() );
    aPos.nNode--;

    const SwPosition* pStt = rPaM.Start(), *pEnd = rPaM.End();

    if( pCpyNd || pEndNdIdx || !aPos.nNode.GetNode().GetCntntNode() ||
        (!pStt->nContent.GetIndex() && (pStt->nNode != pEnd->nNode ||
                (!pStt->nNode.GetNode().GetCntntNode() ||
                    pStt->nNode.GetNode().GetCntntNode()->Len() ==
                        pEnd->nContent.GetIndex() ) ) ) )
    {
        aPos.nNode++;
        aPos.nContent = 0;
    }
    else
        aPos.nNode.GetNode().GetCntntNode()->MakeEndIndex( &aPos.nContent );

    // als USHORT merken; die Indizies verschieben sich !!
    ULONG nTmpMvNode = aPos.nNode.GetIndex();
    xub_StrLen nTmpMvCntnt = aPos.nContent.GetIndex();

    if( pCpyNd || pEndNdIdx )
    {
        SwNodeRange aRg( pStt->nNode, 0, pEnd->nNode, 1 );
        rDoc.GetNodes()._MoveNodes( aRg, rNds, aPos.nNode, FALSE );
        aPos.nContent = 0;
        aPos.nNode--;
    }
    else
    {
        rDoc.GetNodes().Move( rPaM, aPos, rNds, FALSE );

        SwTxtNode* pTxtNd = aPos.nNode.GetNode().GetTxtNode();
        if( pTxtNd )        // fuege einen Trenner fuer die Attribute ein !
        {
            // weil aber beim Insert die Attribute angefasst/sprich
            // aus dem Array geloescht und wieder eingefuegt werden, koennen
            // dadurch Attribute verschwinden (z.B "Fett aus" von 10-20,
            // "Fett an" von 12-15, dann wird durchs Insert/Delete das
            // "Fett an" geloescht !! Ist hier aber nicht erwuenscht !!)
            // DARUM: nicht die Hints anfassen, direct den String manipulieren

            String& rStr = (String&)pTxtNd->GetTxt();
            // Zur Sicherheit lieber nur wenn wirklich am Ende steht
            if( rStr.Len() == aPos.nContent.GetIndex() )
            {
                rStr.Insert( ' ' );
                ++aPos.nContent;
            }
            else
                pTxtNd->Insert( ' ', aPos.nContent, INS_NOHINTEXPAND);
        }
    }
    if( pEndNdIdx )
        *pEndNdIdx = aPos.nNode.GetIndex();
    if( pEndCntIdx )
        *pEndCntIdx = aPos.nContent.GetIndex();

    // alte Position
    aPos.nNode = nTmpMvNode;
    if( pNodeIdx )
        *pNodeIdx = aPos.nNode;

    if( pCntIdx )
    {
        SwCntntNode* pCNd = aPos.nNode.GetNode().GetCntntNode();
        if( pCNd )
            pCntIdx->Assign( pCNd, nTmpMvCntnt );
        else
            pCntIdx->Assign( 0, 0 );
    }

    rDoc.DoUndo( bUndo );
}

void SwUndoSaveCntnt::MoveFromUndoNds( SwDoc& rDoc, ULONG nNodeIdx,
                            xub_StrLen nCntIdx, SwPosition& rInsPos,
                            ULONG* pEndNdIdx, xub_StrLen* pEndCntIdx )
{
    // jetzt kommt das wiederherstellen
    SwNodes& rNds = (SwNodes&)*rDoc.GetUndoNds();
    if( nNodeIdx == rNds.GetEndOfPostIts().GetIndex() )
        return;     // nichts gespeichert

    BOOL bUndo = rDoc.DoesUndo();
    rDoc.DoUndo( FALSE );

    SwPaM aPaM( rInsPos );
    if( pEndNdIdx )         // dann hole aus diesem den Bereich
        aPaM.GetPoint()->nNode.Assign( rNds, *pEndNdIdx );
    else
    {
        aPaM.GetPoint()->nNode = rNds.GetEndOfExtras();
        GoInCntnt( aPaM, fnMoveBackward );
    }

    SwTxtNode* pTxtNd = aPaM.GetNode()->GetTxtNode();
    if( !pEndNdIdx && pTxtNd )  // loesche den Trenner wieder
    {
        if( pEndCntIdx )
            aPaM.GetPoint()->nContent.Assign( pTxtNd, *pEndCntIdx );
        if( pTxtNd->GetTxt().Len() )
        {
            GoInCntnt( aPaM, fnMoveBackward );
            pTxtNd->Erase( aPaM.GetPoint()->nContent, 1 );
        }

        aPaM.SetMark();
        aPaM.GetPoint()->nNode = nNodeIdx;
        aPaM.GetPoint()->nContent.Assign( aPaM.GetCntntNode(), nCntIdx );

        _SaveRedlEndPosForRestore aRedlRest( rInsPos.nNode );

        rNds.Move( aPaM, rInsPos, rDoc.GetNodes() );

        // noch den letzen Node loeschen.
        if( !aPaM.GetPoint()->nContent.GetIndex() ||
            ( aPaM.GetPoint()->nNode++ &&       // noch leere Nodes am Ende ??
            &rNds.GetEndOfExtras() != &aPaM.GetPoint()->nNode.GetNode() ))
        {
            aPaM.GetPoint()->nContent.Assign( 0, 0 );
            aPaM.SetMark();
            rNds.Delete( aPaM.GetPoint()->nNode,
                        rNds.GetEndOfExtras().GetIndex() -
                        aPaM.GetPoint()->nNode.GetIndex() );
        }

        aRedlRest.Restore();
    }
    else if( pEndNdIdx || !pTxtNd )
    {
        SwNodeRange aRg( rNds, nNodeIdx, rNds, (pEndNdIdx
                        ? ((*pEndNdIdx) + 1)
                        : rNds.GetEndOfExtras().GetIndex() ) );
        rNds._MoveNodes( aRg, rDoc.GetNodes(), rInsPos.nNode, 0 == pEndNdIdx );

    }
    else
        ASSERT( FALSE, "was ist es denn nun?" );

    rDoc.DoUndo( bUndo );
}

// diese beiden Methoden bewegen den Point vom Pam zurueck/vor. Damit
// kann fuer ein Undo/Redo ein Bereich aufgespannt werden. (Der
// Point liegt dann vor dem manipuliertem Bereich !!)
// Das Flag gibt an, ob noch vorm Point Inhalt steht.

BOOL SwUndoSaveCntnt::MovePtBackward( SwPaM& rPam )
{
    rPam.SetMark();
    if( rPam.Move( fnMoveBackward ))
        return TRUE;

    // gibt es nach vorne keinen Inhalt mehr, so setze den Point einfach
    // auf die vorherige Position (Node und Content, damit der Content
    // abgemeldet wird !!)
    rPam.GetPoint()->nNode--;
    rPam.GetPoint()->nContent.Assign( 0, 0 );
    return FALSE;
}

void SwUndoSaveCntnt::MovePtForward( SwPaM& rPam, BOOL bMvBkwrd )
{
    // gab es noch Inhalt vor der Position ?
    if( bMvBkwrd )
        rPam.Move( fnMoveForward );
    else
    {                       // setzen Point auf die naechste Position
        rPam.GetPoint()->nNode++;
        SwCntntNode* pCNd = rPam.GetCntntNode();
        if( pCNd )
            pCNd->MakeStartIndex( &rPam.GetPoint()->nContent );
        else
            rPam.Move( fnMoveForward );
    }
}


/*
   JP 21.03.94: loesche alle Objecte, die ContentIndizies auf den ang.
                Bereich besitzen.
                Zur Zeit gibts folgende Objecte
                    - Fussnoten
                    - Flys
                    - Bookmarks
                    - Verzeichnisse
*/

void SwUndoSaveCntnt::DelCntntIndex( const SwPosition& rMark,
                                     const SwPosition& rPoint,
                                     DelCntntType nDelCntntType )
{
    const SwPosition *pStt = rMark < rPoint ? &rMark : &rPoint,
                    *pEnd = &rMark == pStt ? &rPoint : &rMark;

    SwDoc* pDoc = rMark.nNode.GetNode().GetDoc();
    BOOL bDoesUndo = pDoc->DoesUndo();
    pDoc->DoUndo( FALSE );

    // 1. Fussnoten
    if( DELCNT_FTN & nDelCntntType )
    {
        SwFtnIdxs& rFtnArr = pDoc->GetFtnIdxs();
        if( rFtnArr.Count() )
        {
            const SwNode* pFtnNd;
            USHORT nPos;
            rFtnArr.SeekEntry( pStt->nNode, &nPos );
            SwTxtFtn* pSrch;

            // loesche erstmal alle, die dahinter stehen
            while( nPos < rFtnArr.Count() && ( pFtnNd =
                &( pSrch = rFtnArr[ nPos ] )->GetTxtNode())->GetIndex()
                        <= pEnd->nNode.GetIndex() )
            {
                xub_StrLen nFtnSttIdx = *pSrch->GetStart();
                if( (DELCNT_CHKNOCNTNT & nDelCntntType )
                    ? (&pEnd->nNode.GetNode() == pFtnNd )
                    : (( &pStt->nNode.GetNode() == pFtnNd &&
                    pStt->nContent.GetIndex() > nFtnSttIdx) ||
                    ( &pEnd->nNode.GetNode() == pFtnNd &&
                    nFtnSttIdx >= pEnd->nContent.GetIndex() )) )
                {
                    ++nPos;     // weiter suchen
                    continue;
                }

                // es muss leider ein Index angelegt werden. Sonst knallts im
                // TextNode, weil im DTOR der SwFtn dieser geloescht wird !!
                SwTxtNode* pTxtNd = (SwTxtNode*)pFtnNd;
                if( !pHistory )
                    pHistory = new SwHistory;
                SwTxtAttr* pFtnHnt = pTxtNd->GetTxtAttr( nFtnSttIdx );
                ASSERT( pFtnHnt, "kein FtnAttribut" );
                SwIndex aIdx( pTxtNd, nFtnSttIdx );
                pHistory->Add( pFtnHnt, pTxtNd->GetIndex(), FALSE );
                pTxtNd->Erase( aIdx, 1 );
            }

            while( nPos-- && ( pFtnNd = &( pSrch = rFtnArr[ nPos ] )->
                    GetTxtNode())->GetIndex() >= pStt->nNode.GetIndex() )
            {
                xub_StrLen nFtnSttIdx = *pSrch->GetStart();
                if( !(DELCNT_CHKNOCNTNT & nDelCntntType) && (
                    ( &pStt->nNode.GetNode() == pFtnNd &&
                    pStt->nContent.GetIndex() > nFtnSttIdx ) ||
                    ( &pEnd->nNode.GetNode() == pFtnNd &&
                    nFtnSttIdx >= pEnd->nContent.GetIndex() )))
                    continue;               // weiter suchen

                // es muss leider ein Index angelegt werden. Sonst knallts im
                // TextNode, weil im DTOR der SwFtn dieser geloescht wird !!
                SwTxtNode* pTxtNd = (SwTxtNode*)pFtnNd;
                if( !pHistory )
                    pHistory = new SwHistory;
                SwTxtAttr* pFtnHnt = pTxtNd->GetTxtAttr( nFtnSttIdx );
                ASSERT( pFtnHnt, "kein FtnAttribut" );
                SwIndex aIdx( pTxtNd, nFtnSttIdx );
                pHistory->Add( pFtnHnt, pTxtNd->GetIndex(), FALSE );
                pTxtNd->Erase( aIdx, 1 );
            }
        }
    }

    // 2. Flys
    if( DELCNT_FLY & nDelCntntType )
    {
        USHORT nChainInsPos = pHistory ? pHistory->Count() : 0;
        const SwSpzFrmFmts& rSpzArr = *pDoc->GetSpzFrmFmts();
        if( rSpzArr.Count() )
        {
            const BOOL bDelFwrd = rMark.nNode.GetIndex() <= rPoint.nNode.GetIndex();
            SwFlyFrmFmt* pFmt;
            const SwFmtAnchor* pAnchor;
            USHORT n = rSpzArr.Count();
            const SwPosition* pAPos;

            while( n && rSpzArr.Count() )
            {
                pFmt = (SwFlyFrmFmt*)rSpzArr[--n];
                pAnchor = &pFmt->GetAnchor();
                switch( pAnchor->GetAnchorId() )
                {
                case FLY_IN_CNTNT:
                    if( 0 != (pAPos = pAnchor->GetCntntAnchor() ) &&
                        (( DELCNT_CHKNOCNTNT & nDelCntntType )
                        ? ( pStt->nNode <= pAPos->nNode &&
                            pAPos->nNode < pEnd->nNode )
                        : ( *pStt <= *pAPos && *pAPos < *pEnd )) )
                    {
                        if( !pHistory )
                            pHistory = new SwHistory;
                        SwTxtNode* pTxtNd = pDoc->GetNodes()[ pAPos->nNode]->GetTxtNode();
                        SwTxtAttr* pFlyHnt = pTxtNd->GetTxtAttr( pAPos->nContent.GetIndex());
                        ASSERT( pFlyHnt, "kein FlyAttribut" );
                        pHistory->Add( pFlyHnt, 0, FALSE );
                        // n wieder zurueck, damit nicht ein Format uebesprungen wird !
                        n = n >= rSpzArr.Count() ? rSpzArr.Count() : n+1;
                    }
                    break;
                case FLY_AT_CNTNT:

                    if( 0 != (pAPos = pAnchor->GetCntntAnchor() ) &&
                        (( DELCNT_CHKNOCNTNT & nDelCntntType )
                        ? ( pStt->nNode <= pAPos->nNode &&
                            pAPos->nNode < pEnd->nNode )
                        : ( bDelFwrd
                            ? rMark.nNode < pAPos->nNode &&
                                        pAPos->nNode <= rPoint.nNode
                            : rPoint.nNode <= pAPos->nNode &&
                                        pAPos->nNode < rMark.nNode )) )
                    {
                        if( !pHistory )
                            pHistory = new SwHistory;

                        // nur den Anker verchieben ??
                        if( !( DELCNT_CHKNOCNTNT & nDelCntntType ) &&
                            rPoint.nNode.GetIndex() == pAPos->nNode.GetIndex())
                        {
                            pHistory->Add( *pFmt );

                            SwFmtAnchor aAnch( *pAnchor );
                            SwPosition aPos( rMark.nNode );
                            aAnch.SetAnchor( &aPos );
                            pFmt->SetAttr( aAnch );
                        }
                        else
                        {
                            pHistory->Add( *pFmt, nChainInsPos );
                            // n wieder zurueck, damit nicht ein Format uebesprungen wird !
                            n = n >= rSpzArr.Count() ? rSpzArr.Count() : n+1;
                        }
                    }
                    break;
                case FLY_AUTO_CNTNT:
                    if( 0 != (pAPos = pAnchor->GetCntntAnchor() ) &&
                        (( DELCNT_CHKNOCNTNT & nDelCntntType )
                        ? ( pStt->nNode <= pAPos->nNode &&
                            pAPos->nNode < pEnd->nNode )
                        : ( *pStt <= *pAPos && *pAPos < *pEnd )) )
                    {
                        if( !pHistory )
                            pHistory = new SwHistory;
                        if( ( DELCNT_CHKNOCNTNT & nDelCntntType ) ||
                            ( ( pAPos->nNode < pEnd->nNode ) &&
                                ( pStt->nNode < pAPos->nNode ||
                                  !pStt->nContent.GetIndex() ) ) )
                        {
                            pHistory->Add( *pFmt, nChainInsPos );
                            n = n >= rSpzArr.Count() ? rSpzArr.Count() : n+1;
                        }
                        else
                        {
                            pHistory->Add( *pFmt );

                            SwFmtAnchor aAnch( *pAnchor );
                            aAnch.SetAnchor( &rMark );
                            pFmt->SetAttr( aAnch );
                        }
                    }
                    break;
                case FLY_AT_FLY:

                    if( 0 != (pAPos = pAnchor->GetCntntAnchor() ) &&
                        pStt->nNode == pAPos->nNode )
                    {
                        if( !pHistory )
                            pHistory = new SwHistory;

                        pHistory->Add( *pFmt, nChainInsPos );

                        // n wieder zurueck, damit nicht ein Format uebesprungen wird !
                        n = n >= rSpzArr.Count() ? rSpzArr.Count() : n+1;
                    }
                    break;
                }
            }
        }
    }

    // 3. Bookmarks
    if( DELCNT_BKM & nDelCntntType )
    {
        const SwBookmarks& rBkmkTbl = pDoc->GetBookmarks();
        if( rBkmkTbl.Count() )
        {
            const SwBookmark* pBkmk;

            for( USHORT n = 0; n < rBkmkTbl.Count(); ++n )
            {
                BYTE nTyp = 0;
                if( ( DELCNT_CHKNOCNTNT & nDelCntntType )
                    ? (pStt->nNode <= ( pBkmk = rBkmkTbl[ n ])->GetPos().nNode &&
                                    pBkmk->GetPos().nNode < pEnd->nNode )
                    : (*pStt <= ( pBkmk = rBkmkTbl[ n ])->GetPos() &&
                                    pBkmk->GetPos() < *pEnd ) )
                    nTyp = SwHstryBookmark::BKMK_POS;

                if( pBkmk->GetOtherPos() &&
                    (( DELCNT_CHKNOCNTNT & nDelCntntType )
                    ? (pStt->nNode <= pBkmk->GetOtherPos()->nNode &&
                        pBkmk->GetOtherPos()->nNode < pEnd->nNode )
                    : ( *pStt <= *pBkmk->GetOtherPos() &&
                        *pBkmk->GetOtherPos() < *pEnd )) )
                    nTyp |= SwHstryBookmark::BKMK_OTHERPOS;

                if( nTyp )
                {
                    if( !pHistory )
                        pHistory = new SwHistory;

                    pHistory->Add( *pBkmk, nTyp );
                    if( (SwHstryBookmark::BKMK_OTHERPOS|
                        SwHstryBookmark::BKMK_POS) == nTyp ||
                        ( SwHstryBookmark::BKMK_POS == nTyp
                            && !pBkmk->GetOtherPos() ))
                        pDoc->DelBookmark( n-- );
                }
            }
        }
    }

    pDoc->DoUndo( bDoesUndo );
}


// sicher eine vollstaendige Section im Undo-Nodes-Array

SwUndoSaveSection::SwUndoSaveSection()
    : nStartPos( ULONG_MAX ), pMvStt( 0 ), nMvLen( 0 ), pRedlSaveData( 0 )
{
}

SwUndoSaveSection::~SwUndoSaveSection()
{
    if( pMvStt )        // loesche noch den Bereich aus dem UndoNodes Array
    {
        // SaveSection speichert den Inhalt in der PostIt-Section
        SwNodes& rUNds = pMvStt->GetNode().GetNodes();
        rUNds.Delete( *pMvStt, nMvLen );

        delete pMvStt;
    }
    delete pRedlSaveData;
}

void SwUndoSaveSection::SaveSection( SwDoc* pDoc, const SwNodeIndex& rSttIdx )
{
    SwNodeRange aRg( rSttIdx.GetNode(), *rSttIdx.GetNode().EndOfSectionNode() );
    SaveSection( pDoc, aRg );
}


void SwUndoSaveSection::SaveSection( SwDoc* pDoc, const SwNodeRange& rRange )
{
    SwPaM aPam( rRange.aStart, rRange.aEnd );

    // loesche alle Fussnoten / FlyFrames / Bookmarks / Verzeichnisse
    DelCntntIndex( *aPam.GetMark(), *aPam.GetPoint() );

    pRedlSaveData = new SwRedlineSaveDatas;
    if( !SwUndo::FillSaveData( aPam, *pRedlSaveData, TRUE, TRUE ))
        delete pRedlSaveData, pRedlSaveData = 0;

    nStartPos = rRange.aStart.GetIndex();

    aPam.GetPoint()->nNode--;
    aPam.GetMark()->nNode++;

    SwCntntNode* pCNd = aPam.GetCntntNode( FALSE );
    if( pCNd )
        aPam.GetMark()->nContent.Assign( pCNd, 0 );
    if( 0 != ( pCNd = aPam.GetCntntNode( TRUE )) )
        aPam.GetPoint()->nContent.Assign( pCNd, pCNd->Len() );

    // Positionen als SwIndex merken, damit im DTOR dieser Bereich
    // entfernt werden kann !!
    ULONG nEnd;
    pMvStt = new SwNodeIndex( rRange.aStart );
    MoveToUndoNds( aPam, pMvStt, 0, &nEnd, 0 );
    nMvLen = nEnd - pMvStt->GetIndex() + 1;
}

void SwUndoSaveSection::RestoreSection( SwDoc* pDoc, SwNodeIndex* pIdx,
                                        USHORT nSectType )
{
    if( ULONG_MAX != nStartPos )        // gab es ueberhaupt Inhalt ?
    {
        // ueberpruefe, ob der Inhalt an der alten Position steht
        SwNodeIndex aSttIdx( pDoc->GetNodes(), nStartPos );
        ASSERT( !pDoc->GetNodes()[ aSttIdx ]->GetCntntNode(),
                "Position in irgendeiner Section" );

        // move den Inhalt aus dem UndoNodes-Array in den Fly
        SwStartNode* pSttNd = pDoc->GetNodes().MakeEmptySection( aSttIdx,
                                                (SwStartNodeType)nSectType );

        RestoreSection( pDoc, SwNodeIndex( *pSttNd->EndOfSectionNode() ));

        if( pIdx )
            *pIdx = *pSttNd;
    }
}

void SwUndoSaveSection::RestoreSection( SwDoc* pDoc, const SwNodeIndex& rInsPos )
{
    if( ULONG_MAX != nStartPos )        // gab es ueberhaupt Inhalt ?
    {
        SwPosition aInsPos( rInsPos );
        ULONG nEnd = pMvStt->GetIndex() + nMvLen - 1;
        MoveFromUndoNds( *pDoc, pMvStt->GetIndex(), 0, aInsPos, &nEnd, 0 );

        // Indizies wieder zerstoren, Inhalt ist aus dem UndoNodes-Array
        // entfernt worden.
        DELETEZ( pMvStt );
        nMvLen = 0;

        if( pRedlSaveData )
        {
            SwUndo::SetSaveData( *pDoc, *pRedlSaveData );
            delete pRedlSaveData, pRedlSaveData = 0;
        }
    }
}


// START
SwUndoStart::SwUndoStart( USHORT nId )
    : SwUndo( UNDO_START ), nUserId( nId ), nEndOffset( 0 )
{
}

void SwUndoStart::Undo( SwUndoIter& rUndoIter )
{
    if( !( --rUndoIter.nEndCnt ) && rUndoIter.bWeiter &&
        ( rUndoIter.GetId() ? ( rUndoIter.GetId() == nUserId ||
        ( UNDO_END == rUndoIter.GetId() && UNDO_START == GetId() )) : TRUE ))
        rUndoIter.bWeiter = FALSE;
}

void SwUndoStart::Redo( SwUndoIter& rUndoIter )
{
    rUndoIter.bWeiter = TRUE;
    ++rUndoIter.nEndCnt;
}

void SwUndoStart::Repeat( SwUndoIter& rUndoIter )
{
    rUndoIter.bWeiter = FALSE;
}


// END
SwUndoEnd::SwUndoEnd( USHORT nId )
    : SwUndo( UNDO_END ), nUserId( nId ), nSttOffset( 0 )
{
}

void SwUndoEnd::Undo( SwUndoIter& rUndoIter )
{
    if( rUndoIter.GetId() == GetId() || !rUndoIter.GetId() )
        rUndoIter.bWeiter = TRUE;
    if( rUndoIter.bWeiter )
        ++rUndoIter.nEndCnt;
}

void SwUndoEnd::Redo( SwUndoIter& rUndoIter )
{
    if( !( --rUndoIter.nEndCnt ) && rUndoIter.bWeiter &&
        ( rUndoIter.GetId() ? ( rUndoIter.GetId() == nUserId ||
        ( UNDO_END == rUndoIter.GetId() && UNDO_START == GetId() )) : TRUE ))
        rUndoIter.bWeiter = FALSE;
}

void SwUndoEnd::Repeat( SwUndoIter& rUndoIter )
{
    rUndoIter.bWeiter = FALSE;
}

/*  */
        // sicher und setze die RedlineDaten

SwRedlineSaveData::SwRedlineSaveData( SwComparePosition eCmpPos,
                                        const SwPosition& rSttPos,
                                        const SwPosition& rEndPos,
                                        SwRedline& rRedl,
                                        BOOL bCopyNext )
    : SwUndRng( rRedl ),
    SwRedlineData( rRedl.GetRedlineData(), bCopyNext )
{
    ASSERT( POS_OUTSIDE == eCmpPos ||
            !rRedl.GetContentIdx(), "Redline mit Content" );

    switch( eCmpPos )
    {
    case POS_OVERLAP_BEFORE:        // Pos1 ueberlappt Pos2 am Anfang
        nEndNode = rEndPos.nNode.GetIndex();
        nEndCntnt = rEndPos.nContent.GetIndex();
        break;
    case POS_OVERLAP_BEHIND:        // Pos1 ueberlappt Pos2 am Ende
        nSttNode = rSttPos.nNode.GetIndex();
        nSttCntnt = rSttPos.nContent.GetIndex();
        break;

    case POS_INSIDE:                // Pos1 liegt vollstaendig in Pos2
        nSttNode = rSttPos.nNode.GetIndex();
        nSttCntnt = rSttPos.nContent.GetIndex();
        nEndNode = rEndPos.nNode.GetIndex();
        nEndCntnt = rEndPos.nContent.GetIndex();
        break;

    case POS_OUTSIDE:               // Pos2 liegt vollstaendig in Pos1
        if( rRedl.GetContentIdx() )
        {
            // dann den Bereich ins UndoArray verschieben und merken
            SaveSection( rRedl.GetDoc(), *rRedl.GetContentIdx() );
            rRedl.SetContentIdx( 0 );
        }
        break;

    case POS_EQUAL:                 // Pos1 ist genauso gross wie Pos2
        break;

    default:
        ASSERT( !this, "keine gueltigen Daten!" )
    }

#ifndef PRODUCT
    nRedlineCount = rSttPos.nNode.GetNode().GetDoc()->GetRedlineTbl().Count();
#endif
}

SwRedlineSaveData::~SwRedlineSaveData()
{
}

void SwRedlineSaveData::RedlineToDoc( SwPaM& rPam )
{
    SwDoc& rDoc = *rPam.GetDoc();
    SwRedline* pRedl = new SwRedline( *this, rPam );

    if( GetMvSttIdx() )
    {
        SwNodeIndex aIdx( rDoc.GetNodes() );
        RestoreSection( &rDoc, &aIdx, SwNormalStartNode );
        if( GetHistory() )
            GetHistory()->Rollback( &rDoc );
        pRedl->SetContentIdx( &aIdx );
    }
    SetPaM( *pRedl );
    // erstmal die "alten" entfernen, damit im Append keine unerwarteten
    // Dinge passieren, wie z.B. eine Delete in eigenen Insert. Dann wird
    // naehmlich das gerade restaurierte wieder geloescht - nicht das gewollte
    rDoc.DeleteRedline( *pRedl, FALSE );

    SwRedlineMode eOld = rDoc.GetRedlineMode();
    rDoc.SetRedlineMode_intern( eOld | REDLINE_DONTCOMBINE_REDLINES );
    rDoc.AppendRedline( pRedl );
    rDoc.SetRedlineMode_intern( eOld );
}

BOOL SwUndo::FillSaveData( const SwPaM& rRange, SwRedlineSaveDatas& rSData,
                            BOOL bDelRange, BOOL bCopyNext )
{
    if( rSData.Count() )
        rSData.DeleteAndDestroy( 0, rSData.Count() );

    SwRedlineSaveData* pNewData;
    const SwPosition *pStt = rRange.Start(), *pEnd = rRange.End();
    const SwRedlineTbl& rTbl = rRange.GetDoc()->GetRedlineTbl();
    USHORT n = 0;
    rRange.GetDoc()->GetRedline( *pStt, &n );
    for( ; n < rTbl.Count(); ++n )
    {
        SwRedline* pRedl = rTbl[ n ];
        const SwPosition *pRStt = pRedl->Start(), *pREnd = pRedl->End();

        SwComparePosition eCmpPos = ComparePosition( *pStt, *pEnd, *pRStt, *pREnd );
        if( POS_BEFORE != eCmpPos && POS_BEHIND != eCmpPos &&
            POS_COLLIDE_END != eCmpPos && POS_COLLIDE_START != eCmpPos )
        {
            pNewData = new SwRedlineSaveData( eCmpPos, *pStt, *pEnd,
                                                *pRedl, bCopyNext );
            rSData.Insert( pNewData, rSData.Count() );
        }
    }
    if( rSData.Count() && bDelRange )
        rRange.GetDoc()->DeleteRedline( rRange, FALSE );
    return 0 != rSData.Count();
}

BOOL SwUndo::FillSaveDataForFmt( const SwPaM& rRange, SwRedlineSaveDatas& rSData )
{
    if( rSData.Count() )
        rSData.DeleteAndDestroy( 0, rSData.Count() );

    SwRedlineSaveData* pNewData;
    const SwPosition *pStt = rRange.Start(), *pEnd = rRange.End();
    const SwRedlineTbl& rTbl = rRange.GetDoc()->GetRedlineTbl();
    USHORT n = 0;
    rRange.GetDoc()->GetRedline( *pStt, &n );
    for( ; n < rTbl.Count(); ++n )
    {
        SwRedline* pRedl = rTbl[ n ];
        if( REDLINE_FORMAT == pRedl->GetType() )
        {
            const SwPosition *pRStt = pRedl->Start(), *pREnd = pRedl->End();

            SwComparePosition eCmpPos = ComparePosition( *pStt, *pEnd, *pRStt, *pREnd );
            if( POS_BEFORE != eCmpPos && POS_BEHIND != eCmpPos &&
                POS_COLLIDE_END != eCmpPos && POS_COLLIDE_START != eCmpPos )
            {
                pNewData = new SwRedlineSaveData( eCmpPos, *pStt, *pEnd,
                                                    *pRedl, TRUE );
                rSData.Insert( pNewData, rSData.Count() );
            }


        }
    }
    return 0 != rSData.Count();
}

void SwUndo::SetSaveData( SwDoc& rDoc, const SwRedlineSaveDatas& rSData )
{
    SwRedlineMode eOld = rDoc.GetRedlineMode();
    rDoc.SetRedlineMode_intern( ( eOld & ~REDLINE_IGNORE) | REDLINE_ON );
    SwPaM aPam( rDoc.GetNodes().GetEndOfContent() );

    for( USHORT n = rSData.Count(); n; )
        rSData[ --n ]->RedlineToDoc( aPam );

    // check redline count against count saved in RedlineSaveData object
    DBG_ASSERT( (rSData.Count() == 0) ||
                (rSData[0]->nRedlineCount == rDoc.GetRedlineTbl().Count()),
                "redline count not restored properly" );

    rDoc.SetRedlineMode_intern( eOld );
}

BOOL SwUndo::HasHiddenRedlines( const SwRedlineSaveDatas& rSData )
{
    for( USHORT n = rSData.Count(); n; )
        if( rSData[ --n ]->GetMvSttIdx() )
            return TRUE;
    return FALSE;
}

BOOL SwUndo::CanRedlineGroup( SwRedlineSaveDatas& rCurr,
                        const SwRedlineSaveDatas& rCheck, BOOL bCurrIsEnd )
{
    BOOL bRet = FALSE;
    if( rCurr.Count() == rCheck.Count() )
    {
        bRet = TRUE;
        for( USHORT n = 0; n < rCurr.Count(); ++n )
        {
            const SwRedlineSaveData& rSet = *rCurr[ n ];
            const SwRedlineSaveData& rGet = *rCheck[ n ];
            if( rSet.nSttNode != rGet.nSttNode ||
                rSet.GetMvSttIdx() || rGet.GetMvSttIdx() ||
                ( bCurrIsEnd ? rSet.nSttCntnt != rGet.nEndCntnt
                             : rSet.nEndCntnt != rGet.nSttCntnt ) ||
                !rGet.CanCombine( rSet ) )
            {
                bRet = FALSE;
                break;
            }
        }

        if( bRet )
            for( n = 0; n < rCurr.Count(); ++n )
            {
                SwRedlineSaveData& rSet = *rCurr[ n ];
                const SwRedlineSaveData& rGet = *rCheck[ n ];
                if( bCurrIsEnd )
                    rSet.nSttCntnt = rGet.nSttCntnt;
                else
                    rSet.nEndCntnt = rGet.nEndCntnt;
            }
    }
    return bRet;
}

