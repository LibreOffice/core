/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <hintids.hxx>
#include <fmtanchr.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IShellCursorSupplier.hxx>
#include <docary.hxx>
#include <swundo.hxx>           // fuer die UndoIds
#include <pam.hxx>
#include <ndtxt.hxx>
#include <UndoCore.hxx>
#include <rolbck.hxx>
#include <redline.hxx>



SwUndoInserts::SwUndoInserts( SwUndoId nUndoId, const SwPaM& rPam )
    : SwUndo( nUndoId ), SwUndRng( rPam ),
    pTxtFmtColl( 0 ), pLastNdColl(0), pFrmFmts( 0 ), pRedlData( 0 ),
    bSttWasTxtNd( sal_True ), nNdDiff( 0 ), pPos( 0 ), nSetPos( 0 )
{
    pHistory = new SwHistory;
    SwDoc* pDoc = (SwDoc*)rPam.GetDoc();

    SwTxtNode* pTxtNd = rPam.GetPoint()->nNode.GetNode().GetTxtNode();
    if( pTxtNd )
    {
        pTxtFmtColl = pTxtNd->GetTxtColl();
        pHistory->CopyAttr( pTxtNd->GetpSwpHints(), nSttNode,
                            0, pTxtNd->GetTxt().Len(), false );
        if( pTxtNd->HasSwAttrSet() )
            pHistory->CopyFmtAttr( *pTxtNd->GetpSwAttrSet(), nSttNode );

        if( !nSttCntnt )    // dann werden Flys mitgenommen !!
        {
            sal_uInt16 nArrLen = pDoc->GetSpzFrmFmts()->Count();
            for( sal_uInt16 n = 0; n < nArrLen; ++n )
            {
                SwFrmFmt* pFmt = (*pDoc->GetSpzFrmFmts())[n];
                SwFmtAnchor const*const  pAnchor = &pFmt->GetAnchor();
                const SwPosition* pAPos = pAnchor->GetCntntAnchor();
                if (pAPos &&
                    (pAnchor->GetAnchorId() == FLY_AT_PARA) &&
                     nSttNode == pAPos->nNode.GetIndex() )
                {
                    if( !pFrmFmts )
                        pFrmFmts = new SvPtrarr;
                    pFrmFmts->Insert( pFmt, pFrmFmts->Count() );
                }
            }
        }
    }
    // Redline beachten
    if( pDoc->IsRedlineOn() )
    {
        pRedlData = new SwRedlineData( nsRedlineType_t::REDLINE_INSERT, pDoc->GetRedlineAuthor() );
        SetRedlineMode( pDoc->GetRedlineMode() );
    }
}

// setze den Destination-Bereich nach dem Einlesen.

void SwUndoInserts::SetInsertRange( const SwPaM& rPam, sal_Bool bScanFlys,
                                    sal_Bool bSttIsTxtNd )
{
    const SwPosition* pTmpPos = rPam.End();
    nEndNode = pTmpPos->nNode.GetIndex();
    nEndCntnt = pTmpPos->nContent.GetIndex();
    if( rPam.HasMark() )
    {
        if( pTmpPos == rPam.GetPoint() )
            pTmpPos = rPam.GetMark();
        else
            pTmpPos = rPam.GetPoint();

        nSttNode = pTmpPos->nNode.GetIndex();
        nSttCntnt = pTmpPos->nContent.GetIndex();

        if( !bSttIsTxtNd )      // wird eine Tabellenselektion eingefuegt,
        {
            ++nSttNode;         // dann stimmt der CopyPam nicht ganz
            bSttWasTxtNd = sal_False;
        }
    }

    if( bScanFlys && !nSttCntnt )
    {
        // dann alle neuen Flys zusammen sammeln !!
        SwDoc* pDoc = (SwDoc*)rPam.GetDoc();
        sal_uInt16 nFndPos, nArrLen = pDoc->GetSpzFrmFmts()->Count();
        for( sal_uInt16 n = 0; n < nArrLen; ++n )
        {
            SwFrmFmt* pFmt = (*pDoc->GetSpzFrmFmts())[n];
            SwFmtAnchor const*const pAnchor = &pFmt->GetAnchor();
            SwPosition const*const pAPos = pAnchor->GetCntntAnchor();
            if (pAPos &&
                (pAnchor->GetAnchorId() == FLY_AT_PARA) &&
                nSttNode == pAPos->nNode.GetIndex() )
            {
                if( !pFrmFmts ||
                    USHRT_MAX == ( nFndPos = pFrmFmts->GetPos( pFmt ) ) )
                {
                    ::boost::shared_ptr<SwUndoInsLayFmt> const pFlyUndo(
                        new SwUndoInsLayFmt(pFmt, 0, 0));
                    m_FlyUndos.push_back(pFlyUndo);
                }
                else
                    pFrmFmts->Remove( nFndPos );
            }
        }
        delete pFrmFmts, pFrmFmts = 0;
    }
}


SwUndoInserts::~SwUndoInserts()
{
    if( pPos )      // loesche noch den Bereich aus dem UndoNodes Array
    {
        // Insert speichert den Inhalt in der IconSection
        SwNodes& rUNds = pPos->nNode.GetNodes();
        if( pPos->nContent.GetIndex() )         // nicht den gesamten Node loeschen
        {
            SwTxtNode* pTxtNd = pPos->nNode.GetNode().GetTxtNode();
            ASSERT( pTxtNd, "kein TextNode, aus dem geloescht werden soll" );
            if( pTxtNd ) // Robust
            {
                pTxtNd->EraseText( pPos->nContent );
            }
            pPos->nNode++;
        }
        pPos->nContent.Assign( 0, 0 );
        rUNds.Delete( pPos->nNode, rUNds.GetEndOfExtras().GetIndex() -
                                    pPos->nNode.GetIndex() );
        delete pPos;
    }
    delete pFrmFmts;
    delete pRedlData;
}


void SwUndoInserts::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();
    SwPaM *const pPam = & AddUndoRedoPaM(rContext);

    if( IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() ))
        pDoc->DeleteRedline( *pPam, true, USHRT_MAX );

    // sind an Point/Mark 2 unterschiedliche TextNodes, dann muss ein
    // JoinNext ausgefuehrt werden.
    sal_Bool bJoinNext = nSttNode != nEndNode &&
                pPam->GetMark()->nNode.GetNode().GetTxtNode() &&
                pPam->GetPoint()->nNode.GetNode().GetTxtNode();


    // gibts ueberhaupt Inhalt ? (laden von Zeichenvorlagen hat kein Inhalt!)
    if( nSttNode != nEndNode || nSttCntnt != nEndCntnt )
    {
        if( nSttNode != nEndNode )
        {
            SwTxtNode* pTxtNd = pDoc->GetNodes()[ nEndNode ]->GetTxtNode();
            if( pTxtNd && pTxtNd->GetTxt().Len() == nEndCntnt )
                pLastNdColl = pTxtNd->GetTxtColl();
        }

        RemoveIdxFromRange( *pPam, sal_False );
        SetPaM(*pPam);

        // sind Fussnoten oder CntntFlyFrames im Text ??
        nSetPos = pHistory->Count();
        nNdDiff = pPam->GetMark()->nNode.GetIndex();
        DelCntntIndex( *pPam->GetMark(), *pPam->GetPoint() );
        nNdDiff -= pPam->GetMark()->nNode.GetIndex();

        if( *pPam->GetPoint() != *pPam->GetMark() )
        {
            pPos = new SwPosition( *pPam->GetPoint() );
            MoveToUndoNds( *pPam, &pPos->nNode, &pPos->nContent );

            if( !bSttWasTxtNd )
                pPam->Move( fnMoveBackward, fnGoCntnt );
        }
    }

    if (m_FlyUndos.size())
    {
        sal_uLong nTmp = pPam->GetPoint()->nNode.GetIndex();
        for (size_t n = m_FlyUndos.size(); 0 < n; --n)
        {
            m_FlyUndos[ n-1 ]->UndoImpl(rContext);
        }
        nNdDiff += nTmp - pPam->GetPoint()->nNode.GetIndex();
    }

    SwNodeIndex& rIdx = pPam->GetPoint()->nNode;
    SwTxtNode* pTxtNode = rIdx.GetNode().GetTxtNode();
    if( pTxtNode )
    {
        if( !pTxtFmtColl )      // falls 0, dann war hier auch kein TextNode,
        {                       // dann muss dieser geloescht werden,
            SwNodeIndex aDelIdx( rIdx );
            rIdx++;
            SwCntntNode* pCNd = rIdx.GetNode().GetCntntNode();
            xub_StrLen nCnt = 0; if( pCNd ) nCnt = pCNd->Len();
            pPam->GetPoint()->nContent.Assign( pCNd, nCnt );
            pPam->SetMark();
            pPam->DeleteMark();

            RemoveIdxRel( aDelIdx.GetIndex(), *pPam->GetPoint() );

            pDoc->GetNodes().Delete( aDelIdx, 1 );
        }
        else
        {
            if( bJoinNext && pTxtNode->CanJoinNext())
            {
                {
                    RemoveIdxRel( rIdx.GetIndex()+1, SwPosition( rIdx,
                        SwIndex( pTxtNode, pTxtNode->GetTxt().Len() )));
                }
                pTxtNode->JoinNext();
            }
            // reset all text attributes in the paragraph!
            pTxtNode->RstTxtAttr( SwIndex(pTxtNode, 0), pTxtNode->Len(), 0, 0, true );

            // setze alle Attribute im Node zurueck
            pTxtNode->ResetAllAttr();

            if( USHRT_MAX != pDoc->GetTxtFmtColls()->GetPos( pTxtFmtColl ))
                pTxtFmtColl = (SwTxtFmtColl*)pTxtNode->ChgFmtColl( pTxtFmtColl );

            pHistory->SetTmpEnd( nSetPos );
            pHistory->TmpRollback( pDoc, 0, false );
        }
    }
}

void SwUndoInserts::RedoImpl(::sw::UndoRedoContext & rContext)
{
    // setze noch den Cursor auf den Redo-Bereich
    SwPaM *const pPam(& rContext.GetCursorSupplier().CreateNewShellCursor());
    SwDoc* pDoc = pPam->GetDoc();
    pPam->DeleteMark();
    pPam->GetPoint()->nNode = nSttNode - nNdDiff;
    SwCntntNode* pCNd = pPam->GetCntntNode();
    pPam->GetPoint()->nContent.Assign( pCNd, nSttCntnt );

    SwTxtFmtColl* pSavTxtFmtColl = pTxtFmtColl;
    if( pTxtFmtColl && pCNd && pCNd->IsTxtNode() )
        pSavTxtFmtColl = ((SwTxtNode*)pCNd)->GetTxtColl();

    pHistory->SetTmpEnd( nSetPos );

    // alte Anfangs-Position fuers Rollback zurueckholen
    if( ( nSttNode != nEndNode || nSttCntnt != nEndCntnt ) && pPos )
    {
        sal_Bool bMvBkwrd = MovePtBackward( *pPam );

        // Inhalt wieder einfuegen. (erst pPos abmelden !!)
        sal_uLong nMvNd = pPos->nNode.GetIndex();
        xub_StrLen nMvCnt = pPos->nContent.GetIndex();
        DELETEZ( pPos );
        MoveFromUndoNds( *pDoc, nMvNd, nMvCnt, *pPam->GetMark() );
        if( bSttWasTxtNd )
            MovePtForward( *pPam, bMvBkwrd );
        pPam->Exchange();
    }

    if( USHRT_MAX != pDoc->GetTxtFmtColls()->GetPos( pTxtFmtColl ))
    {
        SwTxtNode* pTxtNd = pPam->GetMark()->nNode.GetNode().GetTxtNode();
        if( pTxtNd )
            pTxtNd->ChgFmtColl( pTxtFmtColl );
    }
    pTxtFmtColl = pSavTxtFmtColl;

    if( pLastNdColl && USHRT_MAX != pDoc->GetTxtFmtColls()->GetPos( pLastNdColl ) &&
        pPam->GetPoint()->nNode != pPam->GetMark()->nNode )
    {
        SwTxtNode* pTxtNd = pPam->GetPoint()->nNode.GetNode().GetTxtNode();
        if( pTxtNd )
            pTxtNd->ChgFmtColl( pLastNdColl );
    }

    for (size_t n = m_FlyUndos.size(); 0 < n; --n)
    {
        m_FlyUndos[ n-1 ]->RedoImpl(rContext);
    }

    pHistory->Rollback( pDoc, nSetPos );

    if( pRedlData && IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() ))
    {
        RedlineMode_t eOld = pDoc->GetRedlineMode();
        pDoc->SetRedlineMode_intern((RedlineMode_t)( eOld & ~nsRedlineMode_t::REDLINE_IGNORE ));
        pDoc->AppendRedline( new SwRedline( *pRedlData, *pPam ), true);
        pDoc->SetRedlineMode_intern( eOld );
    }
    else if( !( nsRedlineMode_t::REDLINE_IGNORE & GetRedlineMode() ) &&
            pDoc->GetRedlineTbl().Count() )
        pDoc->SplitRedline( *pPam );
}

void SwUndoInserts::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwPaM aPam( rContext.GetDoc().GetNodes().GetEndOfContent() );
    SetPaM( aPam );
    SwPaM & rRepeatPaM( rContext.GetRepeatPaM() );
    aPam.GetDoc()->CopyRange( aPam, *rRepeatPaM.GetPoint(), false );
}


//////////////////////////////////////////////////////////////////////////

SwUndoInsDoc::SwUndoInsDoc( const SwPaM& rPam )
    : SwUndoInserts( UNDO_INSDOKUMENT, rPam )
{
}

SwUndoCpyDoc::SwUndoCpyDoc( const SwPaM& rPam )
    : SwUndoInserts( UNDO_COPY, rPam )
{
}

