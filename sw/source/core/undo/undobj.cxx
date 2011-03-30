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


#include <IShellCursorSupplier.hxx>
#include <txtftn.hxx>
#include <fmtanchr.hxx>
#include <ftnidx.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <UndoManager.hxx>
#include <docary.hxx>
#include <swundo.hxx>           // fuer die UndoIds
#include <pam.hxx>
#include <ndtxt.hxx>
#include <UndoCore.hxx>
#include <rolbck.hxx>
#include <ndnotxt.hxx>
#include <IMark.hxx>
#include <mvsave.hxx>
#include <redline.hxx>
#include <crossrefbookmark.hxx>
#include <undo.hrc>
#include <comcore.hrc>
#include <docsh.hxx>

class SwRedlineSaveData : public SwUndRng, public SwRedlineData,
                          private SwUndoSaveSection
{
public:
    SwRedlineSaveData( SwComparePosition eCmpPos,
                        const SwPosition& rSttPos, const SwPosition& rEndPos,
                        SwRedline& rRedl, sal_Bool bCopyNext );
    ~SwRedlineSaveData();
    void RedlineToDoc( SwPaM& rPam );
    SwNodeIndex* GetMvSttIdx() const
        { return SwUndoSaveSection::GetMvSttIdx(); }

#ifdef DBG_UTIL
    sal_uInt16 nRedlineCount;
#endif
};

SV_IMPL_PTRARR( SwRedlineSaveDatas, SwRedlineSaveDataPtr )


//------------------------------------------------------------

// Diese Klasse speichert den Pam als sal_uInt16's und kann diese wieder zu

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

void SwUndRng::SetPaM( SwPaM & rPam, sal_Bool bCorrToCntnt ) const
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

SwPaM & SwUndRng::AddUndoRedoPaM(
        ::sw::UndoRedoContext & rContext, bool const bCorrToCntnt) const
{
    SwPaM & rPaM( rContext.GetCursorSupplier().CreateNewShellCursor() );
    SetPaM( rPaM, bCorrToCntnt );
    return rPaM;
}


//------------------------------------------------------------


void SwUndo::RemoveIdxFromSection( SwDoc& rDoc, sal_uLong nSttIdx,
                                    sal_uLong* pEndIdx )
{
    SwNodeIndex aIdx( rDoc.GetNodes(), nSttIdx );
    SwNodeIndex aEndIdx( rDoc.GetNodes(), pEndIdx ? *pEndIdx
                                    : aIdx.GetNode().EndOfSectionIndex() );
    SwPosition aPos( rDoc.GetNodes().GetEndOfPostIts() );
    rDoc.CorrAbs( aIdx, aEndIdx, aPos, sal_True );
}

void SwUndo::RemoveIdxFromRange( SwPaM& rPam, sal_Bool bMoveNext )
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

        rPam.GetDoc()->CorrAbs( aStt, aEnd, *rPam.GetPoint(), sal_True );
    }
    else
        rPam.GetDoc()->CorrAbs( rPam, *pEnd, sal_True );
}

void SwUndo::RemoveIdxRel( sal_uLong nIdx, const SwPosition& rPos )
{
    // nur die Crsr verschieben; die Bookmarks/TOXMarks/.. werden vom
    // entsp. JoinNext/JoinPrev erledigt!
    SwNodeIndex aIdx( rPos.nNode.GetNode().GetNodes(), nIdx );
    ::PaMCorrRel( aIdx, rPos );
}

SwUndo::SwUndo(SwUndoId const nId)
    : m_nId(nId), nOrigRedlineMode(nsRedlineMode_t::REDLINE_NONE),
      bCacheComment(true), pComment(NULL)
{
}

bool SwUndo::IsDelBox() const
{
    return GetId() == UNDO_COL_DELETE || GetId() == UNDO_ROW_DELETE ||
        GetId() == UNDO_TABLE_DELBOX;
}

SwUndo::~SwUndo()
{
    delete pComment;
}


class UndoRedoRedlineGuard
{
public:
    UndoRedoRedlineGuard(::sw::UndoRedoContext & rContext, SwUndo & rUndo)
        : m_rRedlineAccess(rContext.GetDoc())
        , m_eMode(m_rRedlineAccess.GetRedlineMode())
    {
        RedlineMode_t const eTmpMode =
            static_cast<RedlineMode_t>(rUndo.GetRedlineMode());
        if ((nsRedlineMode_t::REDLINE_SHOW_MASK & eTmpMode) !=
            (nsRedlineMode_t::REDLINE_SHOW_MASK & m_eMode))
        {
            m_rRedlineAccess.SetRedlineMode( eTmpMode );
        }
        m_rRedlineAccess.SetRedlineMode_intern( static_cast<RedlineMode_t>(
                eTmpMode | nsRedlineMode_t::REDLINE_IGNORE) );
    }
    ~UndoRedoRedlineGuard()
    {
        m_rRedlineAccess.SetRedlineMode(m_eMode);
    }
private:
    IDocumentRedlineAccess & m_rRedlineAccess;
    RedlineMode_t const m_eMode;
};

void SwUndo::Undo()
{
    OSL_ENSURE(false, "SwUndo::Undo(): ERROR: must call Undo(context) instead");
}

void SwUndo::Redo()
{
    OSL_ENSURE(false, "SwUndo::Redo(): ERROR: must call Redo(context) instead");
}

void SwUndo::UndoWithContext(SfxUndoContext & rContext)
{
    ::sw::UndoRedoContext *const pContext(
            dynamic_cast< ::sw::UndoRedoContext * >(& rContext));
    OSL_ASSERT(pContext);
    if (!pContext) { return; }
    UndoRedoRedlineGuard(*pContext, *this);
    UndoImpl(*pContext);
}

void SwUndo::RedoWithContext(SfxUndoContext & rContext)
{
    ::sw::UndoRedoContext *const pContext(
            dynamic_cast< ::sw::UndoRedoContext * >(& rContext));
    OSL_ASSERT(pContext);
    if (!pContext) { return; }
    UndoRedoRedlineGuard(*pContext, *this);
    RedoImpl(*pContext);
}

void SwUndo::Repeat(SfxRepeatTarget & rContext)
{
    ::sw::RepeatContext *const pRepeatContext(
            dynamic_cast< ::sw::RepeatContext * >(& rContext));
    OSL_ASSERT(pRepeatContext);
    if (!pRepeatContext) { return; }
    RepeatImpl(*pRepeatContext);
}

sal_Bool SwUndo::CanRepeat(SfxRepeatTarget & rContext) const
{
    ::sw::RepeatContext *const pRepeatContext(
            dynamic_cast< ::sw::RepeatContext * >(& rContext));
    OSL_ASSERT(pRepeatContext);
    if (!pRepeatContext) { return false; }
    return CanRepeatImpl(*pRepeatContext);
}

void SwUndo::RepeatImpl( ::sw::RepeatContext & )
{
}

bool SwUndo::CanRepeatImpl( ::sw::RepeatContext & ) const
{
//    return false;
    return ((REPEAT_START <= GetId()) && (GetId() < REPEAT_END));
}

String SwUndo::GetComment() const
{
    String aResult;

    if (bCacheComment)
    {
        if (! pComment)
        {
            pComment = new String(SW_RES(UNDO_BASE + GetId()));

            SwRewriter aRewriter = GetRewriter();

            *pComment = aRewriter.Apply(*pComment);
        }

        aResult = *pComment;
    }
    else
    {
        aResult = String(SW_RES(UNDO_BASE + GetId()));

        SwRewriter aRewriter = GetRewriter();

        aResult = aRewriter.Apply(aResult);
    }

    return aResult;
}

SwRewriter SwUndo::GetRewriter() const
{
    SwRewriter aResult;

    return aResult;
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
                    SwIndex* pCntIdx, sal_uLong* pEndNdIdx, xub_StrLen* pEndCntIdx )
{
    SwDoc& rDoc = *rPaM.GetDoc();
    ::sw::UndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());

    SwNoTxtNode* pCpyNd = rPaM.GetNode()->GetNoTxtNode();

    // jetzt kommt das eigentliche Loeschen(Verschieben)
    SwNodes & rNds = rDoc.GetUndoManager().GetUndoNodes();
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

    // als sal_uInt16 merken; die Indizies verschieben sich !!
    sal_uLong nTmpMvNode = aPos.nNode.GetIndex();
    xub_StrLen nTmpMvCntnt = aPos.nContent.GetIndex();

    if( pCpyNd || pEndNdIdx )
    {
        SwNodeRange aRg( pStt->nNode, 0, pEnd->nNode, 1 );
        rDoc.GetNodes()._MoveNodes( aRg, rNds, aPos.nNode, sal_False );
        aPos.nContent = 0;
        aPos.nNode--;
    }
    else
    {
        rDoc.GetNodes().MoveRange( rPaM, aPos, rNds );

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
            {
                pTxtNd->InsertText( sal_Unicode(' '), aPos.nContent,
                        IDocumentContentOperations::INS_NOHINTEXPAND );
            }
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
}

void SwUndoSaveCntnt::MoveFromUndoNds( SwDoc& rDoc, sal_uLong nNodeIdx,
                            xub_StrLen nCntIdx, SwPosition& rInsPos,
                            sal_uLong* pEndNdIdx, xub_StrLen* pEndCntIdx )
{
    // jetzt kommt das wiederherstellen
    SwNodes & rNds = rDoc.GetUndoManager().GetUndoNodes();
    if( nNodeIdx == rNds.GetEndOfPostIts().GetIndex() )
        return;     // nichts gespeichert

    ::sw::UndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());

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
            pTxtNd->EraseText( aPaM.GetPoint()->nContent, 1 );
        }

        aPaM.SetMark();
        aPaM.GetPoint()->nNode = nNodeIdx;
        aPaM.GetPoint()->nContent.Assign( aPaM.GetCntntNode(), nCntIdx );

        _SaveRedlEndPosForRestore aRedlRest( rInsPos.nNode, rInsPos.nContent.GetIndex() );

        rNds.MoveRange( aPaM, rInsPos, rDoc.GetNodes() );

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
    else {
        ASSERT( sal_False, "was ist es denn nun?" );
    }
}

// diese beiden Methoden bewegen den Point vom Pam zurueck/vor. Damit
// kann fuer ein Undo/Redo ein Bereich aufgespannt werden. (Der
// Point liegt dann vor dem manipuliertem Bereich !!)
// Das Flag gibt an, ob noch vorm Point Inhalt steht.

sal_Bool SwUndoSaveCntnt::MovePtBackward( SwPaM& rPam )
{
    rPam.SetMark();
    if( rPam.Move( fnMoveBackward ))
        return sal_True;

    // gibt es nach vorne keinen Inhalt mehr, so setze den Point einfach
    // auf die vorherige Position (Node und Content, damit der Content
    // abgemeldet wird !!)
    rPam.GetPoint()->nNode--;
    rPam.GetPoint()->nContent.Assign( 0, 0 );
    return sal_False;
}

void SwUndoSaveCntnt::MovePtForward( SwPaM& rPam, sal_Bool bMvBkwrd )
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
// --> OD 2007-10-17 #i81002# - extending method:
// delete certain (not all) cross-reference bookmarks at text node of <rMark>
// and at text node of <rPoint>, if these text nodes aren't the same.
void SwUndoSaveCntnt::DelCntntIndex( const SwPosition& rMark,
                                     const SwPosition& rPoint,
                                     DelCntntType nDelCntntType )
{
    const SwPosition *pStt = rMark < rPoint ? &rMark : &rPoint,
                    *pEnd = &rMark == pStt ? &rPoint : &rMark;

    SwDoc* pDoc = rMark.nNode.GetNode().GetDoc();

    ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());

    // 1. Fussnoten
    if( nsDelCntntType::DELCNT_FTN & nDelCntntType )
    {
        SwFtnIdxs& rFtnArr = pDoc->GetFtnIdxs();
        if( rFtnArr.Count() )
        {
            const SwNode* pFtnNd;
            sal_uInt16 nPos;
            rFtnArr.SeekEntry( pStt->nNode, &nPos );
            SwTxtFtn* pSrch;

            // loesche erstmal alle, die dahinter stehen
            while( nPos < rFtnArr.Count() && ( pFtnNd =
                &( pSrch = rFtnArr[ nPos ] )->GetTxtNode())->GetIndex()
                        <= pEnd->nNode.GetIndex() )
            {
                xub_StrLen nFtnSttIdx = *pSrch->GetStart();
                if( (nsDelCntntType::DELCNT_CHKNOCNTNT & nDelCntntType )
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
                SwTxtAttr* const pFtnHnt =
                    pTxtNd->GetTxtAttrForCharAt( nFtnSttIdx );
                ASSERT( pFtnHnt, "kein FtnAttribut" );
                SwIndex aIdx( pTxtNd, nFtnSttIdx );
                pHistory->Add( pFtnHnt, pTxtNd->GetIndex(), false );
                pTxtNd->EraseText( aIdx, 1 );
            }

            while( nPos-- && ( pFtnNd = &( pSrch = rFtnArr[ nPos ] )->
                    GetTxtNode())->GetIndex() >= pStt->nNode.GetIndex() )
            {
                xub_StrLen nFtnSttIdx = *pSrch->GetStart();
                if( !(nsDelCntntType::DELCNT_CHKNOCNTNT & nDelCntntType) && (
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
                SwTxtAttr* const pFtnHnt =
                    pTxtNd->GetTxtAttrForCharAt( nFtnSttIdx );
                ASSERT( pFtnHnt, "kein FtnAttribut" );
                SwIndex aIdx( pTxtNd, nFtnSttIdx );
                pHistory->Add( pFtnHnt, pTxtNd->GetIndex(), false );
                pTxtNd->EraseText( aIdx, 1 );
            }
        }
    }

    // 2. Flys
    if( nsDelCntntType::DELCNT_FLY & nDelCntntType )
    {
        sal_uInt16 nChainInsPos = pHistory ? pHistory->Count() : 0;
        const SwSpzFrmFmts& rSpzArr = *pDoc->GetSpzFrmFmts();
        if( rSpzArr.Count() )
        {
            const sal_Bool bDelFwrd = rMark.nNode.GetIndex() <= rPoint.nNode.GetIndex();
            SwFlyFrmFmt* pFmt;
            const SwFmtAnchor* pAnchor;
            sal_uInt16 n = rSpzArr.Count();
            const SwPosition* pAPos;

            while( n && rSpzArr.Count() )
            {
                pFmt = (SwFlyFrmFmt*)rSpzArr[--n];
                pAnchor = &pFmt->GetAnchor();
                switch( pAnchor->GetAnchorId() )
                {
                case FLY_AS_CHAR:
                    if( 0 != (pAPos = pAnchor->GetCntntAnchor() ) &&
                        (( nsDelCntntType::DELCNT_CHKNOCNTNT & nDelCntntType )
                        ? ( pStt->nNode <= pAPos->nNode &&
                            pAPos->nNode < pEnd->nNode )
                        : ( *pStt <= *pAPos && *pAPos < *pEnd )) )
                    {
                        if( !pHistory )
                            pHistory = new SwHistory;
                        SwTxtNode *const pTxtNd =
                            pAPos->nNode.GetNode().GetTxtNode();
                        SwTxtAttr* const pFlyHnt = pTxtNd->GetTxtAttrForCharAt(
                            pAPos->nContent.GetIndex());
                        ASSERT( pFlyHnt, "kein FlyAttribut" );
                        pHistory->Add( pFlyHnt, 0, false );
                        // n wieder zurueck, damit nicht ein Format uebesprungen wird !
                        n = n >= rSpzArr.Count() ? rSpzArr.Count() : n+1;
                    }
                    break;
                case FLY_AT_PARA:
                    {
                        pAPos =  pAnchor->GetCntntAnchor();
                        if( pAPos )
                        {
                            bool bTmp;
                            if( nsDelCntntType::DELCNT_CHKNOCNTNT & nDelCntntType )
                                bTmp = pStt->nNode <= pAPos->nNode && pAPos->nNode < pEnd->nNode;
                            else
                            {
                                if (bDelFwrd)
                                    bTmp = rMark.nNode < pAPos->nNode &&
                                        pAPos->nNode <= rPoint.nNode;
                                else
                                    bTmp = rPoint.nNode <= pAPos->nNode &&
                                        pAPos->nNode < rMark.nNode;
                            }

                            if (bTmp)
                            {
                                if( !pHistory )
                                    pHistory = new SwHistory;

                                // Moving the anchor?
                                if( !( nsDelCntntType::DELCNT_CHKNOCNTNT & nDelCntntType ) &&
                                    ( rPoint.nNode.GetIndex() == pAPos->nNode.GetIndex() ) )
                                {
                                    // Do not try to move the anchor to a table!
                                    if( rMark.nNode.GetNode().GetTxtNode() )
                                    {
                                        pHistory->Add( *pFmt );
                                        SwFmtAnchor aAnch( *pAnchor );
                                        SwPosition aPos( rMark.nNode );
                                        aAnch.SetAnchor( &aPos );
                                        pFmt->SetFmtAttr( aAnch );
                                    }
                                }
                                else
                                {
                                    pHistory->Add( *pFmt, nChainInsPos );
                                    // n wieder zurueck, damit nicht ein
                                    // Format uebesprungen wird !
                                    n = n >= rSpzArr.Count() ?
                                        rSpzArr.Count() : n+1;
                                }
                            }
                        }
                    }
                    break;
                case FLY_AT_CHAR:
                    if( 0 != (pAPos = pAnchor->GetCntntAnchor() ) &&
                        ( pStt->nNode <= pAPos->nNode && pAPos->nNode <= pEnd->nNode ) )
                    {
                        if( !pHistory )
                            pHistory = new SwHistory;
                        if (IsDestroyFrameAnchoredAtChar(
                                *pAPos, *pStt, *pEnd, nDelCntntType))
                        {
                            pHistory->Add( *pFmt, nChainInsPos );
                            n = n >= rSpzArr.Count() ? rSpzArr.Count() : n+1;
                        }
                        else if( !( nsDelCntntType::DELCNT_CHKNOCNTNT & nDelCntntType ) )
                        {
                            if( *pStt <= *pAPos && *pAPos < *pEnd )
                            {
                                // These are the objects anchored
                                // between section start and end position
                                // Do not try to move the anchor to a table!
                                if( rMark.nNode.GetNode().GetTxtNode() )
                                {
                                    pHistory->Add( *pFmt );
                                    SwFmtAnchor aAnch( *pAnchor );
                                    aAnch.SetAnchor( &rMark );
                                    pFmt->SetFmtAttr( aAnch );
                                }
                            }
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
                default: break;
                }
            }
        }
    }

    // 3. Bookmarks
    if( nsDelCntntType::DELCNT_BKM & nDelCntntType )
    {
        IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
        if( pMarkAccess->getMarksCount() )
        {

            for( sal_uInt16 n = 0; n < pMarkAccess->getMarksCount(); ++n )
            {
                // --> OD 2007-10-17 #i81002#
                bool bSavePos = false;
                bool bSaveOtherPos = false;
                const ::sw::mark::IMark* pBkmk = (pMarkAccess->getMarksBegin() + n)->get();
                if( nsDelCntntType::DELCNT_CHKNOCNTNT & nDelCntntType )
                {
                    if( pStt->nNode <= pBkmk->GetMarkPos().nNode &&
                        pBkmk->GetMarkPos().nNode < pEnd->nNode )
                        bSavePos = true;
                    if( pBkmk->IsExpanded() &&
                        pStt->nNode <= pBkmk->GetOtherMarkPos().nNode &&
                        pBkmk->GetOtherMarkPos().nNode < pEnd->nNode )
                        bSaveOtherPos = true;
                }
                else
                {
                    // --> OD 2009-08-06 #i92125#
                    bool bKeepCrossRefBkmk( false );
                    {
                        if ( rMark.nNode == rPoint.nNode &&
                             ( IDocumentMarkAccess::GetType(*pBkmk) ==
                                IDocumentMarkAccess::CROSSREF_HEADING_BOOKMARK ||
                               IDocumentMarkAccess::GetType(*pBkmk) ==
                                IDocumentMarkAccess::CROSSREF_NUMITEM_BOOKMARK ) )
                        {
                            bKeepCrossRefBkmk = true;
                        }
                    }
                    if ( !bKeepCrossRefBkmk )
                    {
                        bool bMaybe = false;
                        if ( *pStt <= pBkmk->GetMarkPos() && pBkmk->GetMarkPos() <= *pEnd )
                        {
                            if( pBkmk->GetMarkPos() == *pEnd ||
                                ( *pStt == pBkmk->GetMarkPos() && pBkmk->IsExpanded() ) )
                                bMaybe = true;
                            else
                                bSavePos = true;
                        }
                        if( pBkmk->IsExpanded() &&
                            *pStt <= pBkmk->GetOtherMarkPos() && pBkmk->GetOtherMarkPos() <= *pEnd )
                        {
                            if( bSavePos || bSaveOtherPos ||
                                ( pBkmk->GetOtherMarkPos() < *pEnd && pBkmk->GetOtherMarkPos() > *pStt ) )
                            {
                                if( bMaybe )
                                    bSavePos = true;
                                bSaveOtherPos = true;
                            }
                        }
                    }
                    // <--

                    // --> OD 2007-10-17 #i81002#
                    const bool bDifferentTxtNodesAtMarkAndPoint(
                                        rMark.nNode != rPoint.nNode &&
                                        rMark.nNode.GetNode().GetTxtNode() &&
                                        rPoint.nNode.GetNode().GetTxtNode() );
                    // <--
                    if( !bSavePos && !bSaveOtherPos && bDifferentTxtNodesAtMarkAndPoint &&
                        dynamic_cast< const ::sw::mark::CrossRefBookmark* >(pBkmk))
                    {
                        // delete cross-reference bookmark at <pStt>, if only part of
                        // <pEnd> text node content is deleted.
                        if( pStt->nNode == pBkmk->GetMarkPos().nNode &&
                            pEnd->nContent.GetIndex() !=
                                pEnd->nNode.GetNode().GetTxtNode()->Len() )
                        {
                            bSavePos = true;
                            bSaveOtherPos = false;
                        }
                        // delete cross-reference bookmark at <pEnd>, if only part of
                        // <pStt> text node content is deleted.
                        else if( pEnd->nNode == pBkmk->GetMarkPos().nNode &&
                            pStt->nContent.GetIndex() != 0 )
                        {
                            bSavePos = true;
                            bSaveOtherPos = false;
                        }
                    }
                }
                if( bSavePos || bSaveOtherPos )
                {
                    if( !pHistory )
                        pHistory = new SwHistory;

                    pHistory->Add( *pBkmk, bSavePos, bSaveOtherPos );
                    if(bSavePos &&
                        (bSaveOtherPos || !pBkmk->IsExpanded()))
                    {
                        pMarkAccess->deleteMark(pMarkAccess->getMarksBegin()+n);
                        n--;
                    }
                }
            }
        }
    }
}


// sicher eine vollstaendige Section im Undo-Nodes-Array

SwUndoSaveSection::SwUndoSaveSection()
    : pMvStt( 0 ), pRedlSaveData( 0 ), nMvLen( 0 ), nStartPos( ULONG_MAX )
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


void SwUndoSaveSection::SaveSection( SwDoc* , const SwNodeRange& rRange )
{
    SwPaM aPam( rRange.aStart, rRange.aEnd );

    // loesche alle Fussnoten / FlyFrames / Bookmarks / Verzeichnisse
    DelCntntIndex( *aPam.GetMark(), *aPam.GetPoint() );

    pRedlSaveData = new SwRedlineSaveDatas;
    if( !SwUndo::FillSaveData( aPam, *pRedlSaveData, sal_True, sal_True ))
        delete pRedlSaveData, pRedlSaveData = 0;

    nStartPos = rRange.aStart.GetIndex();

    aPam.GetPoint()->nNode--;
    aPam.GetMark()->nNode++;

    SwCntntNode* pCNd = aPam.GetCntntNode( sal_False );
    if( pCNd )
        aPam.GetMark()->nContent.Assign( pCNd, 0 );
    if( 0 != ( pCNd = aPam.GetCntntNode( sal_True )) )
        aPam.GetPoint()->nContent.Assign( pCNd, pCNd->Len() );

    // Positionen als SwIndex merken, damit im DTOR dieser Bereich
    // entfernt werden kann !!
    sal_uLong nEnd;
    pMvStt = new SwNodeIndex( rRange.aStart );
    MoveToUndoNds( aPam, pMvStt, 0, &nEnd, 0 );
    nMvLen = nEnd - pMvStt->GetIndex() + 1;
}

void SwUndoSaveSection::RestoreSection( SwDoc* pDoc, SwNodeIndex* pIdx,
                                        sal_uInt16 nSectType )
{
    if( ULONG_MAX != nStartPos )        // gab es ueberhaupt Inhalt ?
    {
        // ueberpruefe, ob der Inhalt an der alten Position steht
        SwNodeIndex aSttIdx( pDoc->GetNodes(), nStartPos );
        OSL_ENSURE(!aSttIdx.GetNode().GetCntntNode(),
                "RestoreSection(): Position on content node");

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
        sal_uLong nEnd = pMvStt->GetIndex() + nMvLen - 1;
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

        // sicher und setze die RedlineDaten

SwRedlineSaveData::SwRedlineSaveData( SwComparePosition eCmpPos,
                                        const SwPosition& rSttPos,
                                        const SwPosition& rEndPos,
                                        SwRedline& rRedl,
                                        sal_Bool bCopyNext )
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

#ifdef DBG_UTIL
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
    rDoc.DeleteRedline( *pRedl, false, USHRT_MAX );

    RedlineMode_t eOld = rDoc.GetRedlineMode();
    rDoc.SetRedlineMode_intern((RedlineMode_t)(eOld | nsRedlineMode_t::REDLINE_DONTCOMBINE_REDLINES));
    //#i92154# let UI know about a new redline with comment
    if (rDoc.GetDocShell() && (pRedl->GetComment() != String(::rtl::OUString::createFromAscii(""))) )
        rDoc.GetDocShell()->Broadcast(SwRedlineHint(pRedl,SWREDLINE_INSERTED));
    //
#if OSL_DEBUG_LEVEL > 0
    bool const bSuccess =
#endif
        rDoc.AppendRedline( pRedl, true );
    OSL_ENSURE(bSuccess,
        "SwRedlineSaveData::RedlineToDoc: insert redline failed");
    rDoc.SetRedlineMode_intern( eOld );
}

sal_Bool SwUndo::FillSaveData( const SwPaM& rRange, SwRedlineSaveDatas& rSData,
                            sal_Bool bDelRange, sal_Bool bCopyNext )
{
    if( rSData.Count() )
        rSData.DeleteAndDestroy( 0, rSData.Count() );

    SwRedlineSaveData* pNewData;
    const SwPosition *pStt = rRange.Start(), *pEnd = rRange.End();
    const SwRedlineTbl& rTbl = rRange.GetDoc()->GetRedlineTbl();
    sal_uInt16 n = 0;
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
        rRange.GetDoc()->DeleteRedline( rRange, false, USHRT_MAX );
    return 0 != rSData.Count();
}

sal_Bool SwUndo::FillSaveDataForFmt( const SwPaM& rRange, SwRedlineSaveDatas& rSData )
{
    if( rSData.Count() )
        rSData.DeleteAndDestroy( 0, rSData.Count() );

    SwRedlineSaveData* pNewData;
    const SwPosition *pStt = rRange.Start(), *pEnd = rRange.End();
    const SwRedlineTbl& rTbl = rRange.GetDoc()->GetRedlineTbl();
    sal_uInt16 n = 0;
    rRange.GetDoc()->GetRedline( *pStt, &n );
    for( ; n < rTbl.Count(); ++n )
    {
        SwRedline* pRedl = rTbl[ n ];
        if( nsRedlineType_t::REDLINE_FORMAT == pRedl->GetType() )
        {
            const SwPosition *pRStt = pRedl->Start(), *pREnd = pRedl->End();

            SwComparePosition eCmpPos = ComparePosition( *pStt, *pEnd, *pRStt, *pREnd );
            if( POS_BEFORE != eCmpPos && POS_BEHIND != eCmpPos &&
                POS_COLLIDE_END != eCmpPos && POS_COLLIDE_START != eCmpPos )
            {
                pNewData = new SwRedlineSaveData( eCmpPos, *pStt, *pEnd,
                                                    *pRedl, sal_True );
                rSData.Insert( pNewData, rSData.Count() );
            }


        }
    }
    return 0 != rSData.Count();
}

void SwUndo::SetSaveData( SwDoc& rDoc, const SwRedlineSaveDatas& rSData )
{
    RedlineMode_t eOld = rDoc.GetRedlineMode();
    rDoc.SetRedlineMode_intern( (RedlineMode_t)(( eOld & ~nsRedlineMode_t::REDLINE_IGNORE) | nsRedlineMode_t::REDLINE_ON ));
    SwPaM aPam( rDoc.GetNodes().GetEndOfContent() );

    for( sal_uInt16 n = rSData.Count(); n; )
        rSData[ --n ]->RedlineToDoc( aPam );

    // check redline count against count saved in RedlineSaveData object
    DBG_ASSERT( (rSData.Count() == 0) ||
                (rSData[0]->nRedlineCount == rDoc.GetRedlineTbl().Count()),
                "redline count not restored properly" );

    rDoc.SetRedlineMode_intern( eOld );
}

sal_Bool SwUndo::HasHiddenRedlines( const SwRedlineSaveDatas& rSData )
{
    for( sal_uInt16 n = rSData.Count(); n; )
        if( rSData[ --n ]->GetMvSttIdx() )
            return sal_True;
    return sal_False;
}

sal_Bool SwUndo::CanRedlineGroup( SwRedlineSaveDatas& rCurr,
                        const SwRedlineSaveDatas& rCheck, sal_Bool bCurrIsEnd )
{
    sal_Bool bRet = sal_False;
    sal_uInt16 n;

    if( rCurr.Count() == rCheck.Count() )
    {
        bRet = sal_True;
        for( n = 0; n < rCurr.Count(); ++n )
        {
            const SwRedlineSaveData& rSet = *rCurr[ n ];
            const SwRedlineSaveData& rGet = *rCheck[ n ];
            if( rSet.nSttNode != rGet.nSttNode ||
                rSet.GetMvSttIdx() || rGet.GetMvSttIdx() ||
                ( bCurrIsEnd ? rSet.nSttCntnt != rGet.nEndCntnt
                             : rSet.nEndCntnt != rGet.nSttCntnt ) ||
                !rGet.CanCombine( rSet ) )
            {
                bRet = sal_False;
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

// #111827#
String ShortenString(const String & rStr, xub_StrLen nLength, const String & rFillStr)
{
    ASSERT( nLength - rFillStr.Len() >= 2, "improper arguments")

    String aResult;

    if (rStr.Len() <= nLength)
        aResult = rStr;
    else
    {
        long nTmpLength = nLength - rFillStr.Len();
        if ( nTmpLength < 2 )
            nTmpLength = 2;

        nLength = static_cast<xub_StrLen>(nTmpLength);

        const xub_StrLen nFrontLen = nLength - nLength / 2;
        const xub_StrLen nBackLen = nLength - nFrontLen;

        aResult += rStr.Copy(0, nFrontLen);
        aResult += rFillStr;
        aResult += rStr.Copy(rStr.Len() - nBackLen, nBackLen);
    }

    return aResult;
}

static bool lcl_IsSpecialCharacter(sal_Unicode nChar)
{
    switch (nChar)
    {
    case CH_TXTATR_BREAKWORD:
    case CH_TXTATR_INWORD:
    case CH_TXTATR_TAB:
    case CH_TXTATR_NEWLINE:
        return true;

    default:
        break;
    }

    return false;
}

static String lcl_DenotedPortion(String rStr, xub_StrLen nStart,
                                 xub_StrLen nEnd)
{
    String aResult;

    if (nEnd - nStart > 0)
    {
        sal_Unicode cLast = rStr.GetChar(nEnd - 1);
        if (lcl_IsSpecialCharacter(cLast))
        {
            switch(cLast)
            {
            case CH_TXTATR_TAB:
                aResult += String(SW_RES(STR_UNDO_TABS));

                break;
            case CH_TXTATR_NEWLINE:
                aResult += String(SW_RES(STR_UNDO_NLS));

                break;

            case CH_TXTATR_INWORD:
            case CH_TXTATR_BREAKWORD:
                aResult += UNDO_ARG2;

                break;

            }
            SwRewriter aRewriter;
            aRewriter.AddRule(UNDO_ARG1,
                              String::CreateFromInt32(nEnd - nStart));
            aResult = aRewriter.Apply(aResult);
        }
        else
        {
            aResult = String(SW_RES(STR_START_QUOTE));
            aResult += rStr.Copy(nStart, nEnd - nStart);
            aResult += String(SW_RES(STR_END_QUOTE));
        }
    }

    return aResult;
}

String DenoteSpecialCharacters(const String & rStr)
{
    String aResult;

    if (rStr.Len() > 0)
    {
        bool bStart = false;
        xub_StrLen nStart = 0;
        sal_Unicode cLast = 0;

        for (xub_StrLen i = 0; i < rStr.Len(); i++)
        {
            if (lcl_IsSpecialCharacter(rStr.GetChar(i)))
            {
                if (cLast != rStr.GetChar(i))
                    bStart = true;

            }
            else
            {
                if (lcl_IsSpecialCharacter(cLast))
                    bStart = true;
            }

            if (bStart)
            {
                aResult += lcl_DenotedPortion(rStr, nStart, i);

                nStart = i;
                bStart = false;
            }

            cLast = rStr.GetChar(i);
        }

        aResult += lcl_DenotedPortion(rStr, nStart, rStr.Len());
    }
    else
        aResult = UNDO_ARG2;

    return aResult;
}

bool IsDestroyFrameAnchoredAtChar(SwPosition const & rAnchorPos,
        SwPosition const & rStart, SwPosition const & rEnd,
        DelCntntType const nDelCntntType)
{

    // Here we identified the objects to destroy:
    // - anchored between start and end of the selection
    // - anchored in start of the selection with "CheckNoContent"
    // - anchored in start of sel. and the selection start at pos 0
    return  (rAnchorPos.nNode < rEnd.nNode)
         && (   (nsDelCntntType::DELCNT_CHKNOCNTNT & nDelCntntType)
            ||  (rStart.nNode < rAnchorPos.nNode)
            ||  !rStart.nContent.GetIndex()
            );
}

