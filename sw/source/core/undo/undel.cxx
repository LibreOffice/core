/*************************************************************************
 *
 *  $RCSfile: undel.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:27 $
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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _WORDSEL_HXX
#include <svtools/wordsel.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
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
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _MVSAVE_HXX
#include <mvsave.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif

inline SwDoc& SwUndoIter::GetDoc() const { return *pAktPam->GetDoc(); }


// DELETE

SwUndoDelete::SwUndoDelete( SwPaM& rPam, BOOL bFullPara )
    : SwUndo(UNDO_DELETE), SwUndRng( rPam ),
    pMvStt( 0 ), pSttStr(0), pEndStr(0), nNode( 0 ), nSectDiff( 0 ),
    pRedlData( 0 ), pRedlSaveData( 0 )
{
    bMvAroundSectNd = bSectNdFnd = bGroup = bBackSp = bTblDelLastNd =
        bResetPgDesc = bResetPgBrk = FALSE;

    bDelFullPara = bFullPara;

    SwDoc * pDoc = rPam.GetDoc();
#ifdef COMPACT
    pDoc->DelUndoGroups();
#endif

    if( !pDoc->IsIgnoreRedline() && pDoc->GetRedlineTbl().Count() )
    {
        pRedlSaveData = new SwRedlineSaveDatas;
        if( !FillSaveData( rPam, *pRedlSaveData ))
            delete pRedlSaveData, pRedlSaveData = 0;
    }

    if( !pHistory )
        pHistory = new SwHistory;

    // loesche erstmal alle Fussnoten
    const SwPosition *pStt = rPam.Start(),
                    *pEnd = rPam.GetPoint() == pStt
                        ? rPam.GetMark()
                        : rPam.GetPoint();

    if( bDelFullPara )
    {
        ASSERT( rPam.HasMark(), "PaM ohne Mark" );
        DelCntntIndex( *rPam.GetMark(), *rPam.GetPoint(),
                        DelCntntType(DELCNT_ALL | DELCNT_CHKNOCNTNT) );

        BOOL bDoesUndo = pDoc->DoesUndo();
        pDoc->DoUndo( FALSE );
        _DelBookmarks( pStt->nNode, pEnd->nNode );
        pDoc->DoUndo( bDoesUndo );
    }
    else
        DelCntntIndex( *rPam.GetMark(), *rPam.GetPoint() );
    nSetPos = pHistory ? pHistory->Count() : 0;

    // wurde schon was geloescht ??
    nNdDiff = nSttNode - pStt->nNode.GetIndex();

    bJoinNext = !bFullPara && pEnd == rPam.GetPoint();
    bBackSp = !bFullPara && !bJoinNext;

    SwTxtNode *pSttTxtNd = 0, *pEndTxtNd = 0;
    if( !bFullPara )
    {
        pSttTxtNd = pStt->nNode.GetNode().GetTxtNode();
        pEndTxtNd = nSttNode == nEndNode
                    ? pSttTxtNd
                    : pEndTxtNd = pEnd->nNode.GetNode().GetTxtNode();
    }

    BOOL bMoveNds = *pStt == *pEnd      // noch ein Bereich vorhanden ??
                ? FALSE
                : SaveCntnt( pStt, pEnd, pSttTxtNd, pEndTxtNd );

    if( pSttTxtNd && pEndTxtNd && pSttTxtNd != pEndTxtNd )
    {
        // zwei unterschiedliche TextNodes, also speicher noch die
        // TextFormatCollection fuers
        pHistory->Add( pSttTxtNd->GetTxtColl(),pStt->nNode.GetIndex(), ND_TEXTNODE );
        pHistory->Add( pEndTxtNd->GetTxtColl(),pEnd->nNode.GetIndex(), ND_TEXTNODE );

        if( !bJoinNext )        // Selection von Unten nach Oben
        {
            // Beim JoinPrev() werden die AUTO-PageBreak's richtig
            // kopiert. Um diese beim Undo wieder herzustellen, muss das
            // Auto-PageBreak aus dem EndNode zurueckgesetzt werden.
            // - fuer die PageDesc, ColBreak dito !
            if( pEndTxtNd->GetpSwAttrSet() )
            {
                SwRegHistory aRegHist( *pEndTxtNd, pHistory );
                if( SFX_ITEM_SET == pEndTxtNd->GetpSwAttrSet()->GetItemState(
                        RES_BREAK, FALSE ) )
                    pEndTxtNd->ResetAttr( RES_BREAK );
                if( pEndTxtNd->GetpSwAttrSet() &&
                    SFX_ITEM_SET == pEndTxtNd->GetpSwAttrSet()->GetItemState(
                        RES_PAGEDESC, FALSE ) )
                    pEndTxtNd->ResetAttr( RES_PAGEDESC );
            }
        }
    }


    // verschiebe jetzt noch den PaM !!!
    // der SPoint steht am Anfang der SSelection
    if( pEnd == rPam.GetPoint() && pSttTxtNd )
        rPam.Exchange();

    if( !pSttTxtNd && !pEndTxtNd )
        rPam.GetPoint()->nNode--;
    rPam.DeleteMark();          // der SPoint ist aus dem Bereich

    if( !pEndTxtNd )
        nEndCntnt = 0;
    if( !pSttTxtNd )
        nSttCntnt = 0;

    if( bMoveNds )      // sind noch Nodes zu verschieben ?
    {
        // verschiebe jetzt den Rest, also die vollstaendigen Nodes
        // ACHTUNG: pStt und pEnd koennen durch die Pam-Korrektur schon
        // ungueltig sein !!
        int nNdOff = 0;
        if( pSttTxtNd && ( pEndTxtNd || pSttTxtNd->GetTxt().Len() ))
            nNdOff++;

        SwNodeRange aRg( pDoc->GetNodes(), nSttNode - nNdDiff + nNdOff,
                         pDoc->GetNodes(), nEndNode - nNdDiff );
        if( !bFullPara && !pEndTxtNd &&
            &aRg.aEnd.GetNode() != &pDoc->GetNodes().GetEndOfContent() )
            aRg.aEnd++;

        SwNodes& rNds = (SwNodes&)*pDoc->GetUndoNds();
        SwNodes& rDocNds = pDoc->GetNodes();
        nNode = rNds.GetEndOfContent().GetIndex();

        // habe wir SectionNodes (Start/Ende) als 1. oder letzten
        // Nodes in der Selection ?
        SwNode* pTmpNd;
        if( bJoinNext )     // Selektion von oben -> unten
        {
            // Bedingung: - SectionNode und dessen Ende ist der naechste Node
            //            - EndNode einer Section und der Start steht ausserhalb
            if( pSttTxtNd && pEndTxtNd )
            {
                // am Ende erfolgt ein JoinNext, teste auf leere Sections
                // Bedingung: - hinter dem EndTextNode steht ein SectionEndNd
                //              dessen Start im Bereich liegt
                //            - SectionSttNd und der Start steht ausserhalb
                if( aRg.aEnd.GetIndex()+1 < rDocNds.Count() &&
                    ( (pTmpNd = rDocNds[ aRg.aEnd.GetIndex()+1 ])->IsEndNode()
                    && pTmpNd->FindStartNode()->IsSectionNode()
                    && pTmpNd->StartOfSectionIndex() >= aRg.aStart.GetIndex())
                )
                {
                    aRg.aEnd++;
                    bSectNdFnd = TRUE;
                }
                else if( ((pTmpNd = rDocNds[ aRg.aEnd.GetIndex()-1 ])->IsSectionNode()
                        && pTmpNd->EndOfSectionIndex()-1 == aRg.aEnd.GetIndex())
                    || ( pTmpNd->IsEndNode() &&
                    pTmpNd->FindStartNode()->IsSectionNode() &&
                    pTmpNd->FindStartNode()->GetIndex() < aRg.aStart.GetIndex())
                    )
                {
                    aRg.aEnd++;
                    bSectNdFnd = TRUE;
                }
            }
            while( aRg.aEnd.GetIndex() < rDocNds.Count()-1 &&
                // entstehen leere Sections ???
                ( (pTmpNd = &aRg.aEnd.GetNode())->IsEndNode() &&
                pTmpNd->FindStartNode()->IsSectionNode() &&
                pTmpNd->FindStartNode()->GetIndex()+1 >= aRg.aStart.GetIndex())
                )
            {
                aRg.aEnd++;
                bSectNdFnd = TRUE;
            }
        }
        else
        {
            if( pSttTxtNd && pEndTxtNd )
            {
                // am Ende erfolgt ein JoinPrev, teste auf leere Sections
                // Bedingung: - vor dem StartTextNode steht ein SectionSttNd
                //              dessen Ende im Bereich liegt
                //            - SectionEndNd und der Start steht ausserhalb
                if( 2 < aRg.aStart.GetIndex() &&
                    ( (pTmpNd = rDocNds[ aRg.aStart.GetIndex()-2 ])
                        ->IsSectionNode() &&
                    pTmpNd->EndOfSectionIndex() < aRg.aEnd.GetIndex())
                )
                {
                    aRg.aStart = *pTmpNd;
                    bSectNdFnd = TRUE;
                    nSectDiff++;
                }
                else if( aRg.aStart.GetIndex() &&
                    ((pTmpNd = &aRg.aStart.GetNode())->IsSectionNode()
                    && pTmpNd->EndOfSectionIndex() > aRg.aEnd.GetIndex() ) ||
                    (pTmpNd->IsEndNode() && pTmpNd->FindStartNode()->IsSectionNode() &&
                    pTmpNd->FindStartNode()->GetIndex() < aRg.aStart.GetIndex() )
                    )
                {
                    aRg.aStart--;
                    bSectNdFnd = TRUE;
                }
            }
            while( 1 < aRg.aStart.GetIndex() &&
                // entstehen leere Sections ???
                ( (pTmpNd = rDocNds[ aRg.aStart.GetIndex()-1 ])->IsSectionNode() &&
                pTmpNd->EndOfSectionIndex() < aRg.aEnd.GetIndex())
                )
            {
                aRg.aStart--;
                bSectNdFnd = TRUE;
            }
        }

        // ein Index auf den Start-/End-ContentNode, der mit verschoben wird,
        // um wieder an der Position eine Kopie anzulegen.
        if( bSectNdFnd && ( bJoinNext ? pEndTxtNd : pSttTxtNd ))
        {
            if( bJoinNext )
            {
                SwNodeRange aMvRg( *pEndTxtNd, 0, *pEndTxtNd, 1 );
                rDocNds.MakeTxtNode( aMvRg.aStart, pEndTxtNd->GetTxtColl() );
                rDocNds._MoveNodes( aMvRg, rDocNds, aRg.aStart );
            }
            else
            {
                SwNodeRange aMvRg( *pSttTxtNd, 0, *pSttTxtNd, 1 );
                SwNode* pNew = rDocNds.MakeTxtNode( aMvRg.aEnd,
                                            pSttTxtNd->GetTxtColl() );
                if( nSectDiff )
                {
                    aMvRg.aEnd--;
                    rDocNds._MoveNodes( aMvRg, rDocNds, aRg.aEnd );
                    aRg.aEnd--;
                }
                else
                    aRg.aStart = *pNew;
            }
        }

        rDocNds._MoveNodes( aRg, rNds, SwNodeIndex( rNds.GetEndOfContent() ));
        pMvStt = new SwNodeIndex( rNds, nNode );
        bMvAroundSectNd = FALSE;

        if( !bSectNdFnd )
        {
            nSectDiff = aRg.aEnd.GetIndex() - aRg.aStart.GetIndex();
            bMvAroundSectNd = 0 != nSectDiff;
        }

        nNode = rNds.GetEndOfContent().GetIndex() - nNode;      // Differenz merken !
    }
    else
        nNode = 0;      // kein Node verschoben -> keine Differenz zum Ende

    // wurden davor noch Nodes geloescht ?? (FootNotes haben ContentNodes!)
    if( !pSttTxtNd && !pEndTxtNd )
    {
        nNdDiff = nSttNode - rPam.GetPoint()->nNode.GetIndex() - (bFullPara ? 0 : 1);
        rPam.Move( fnMoveForward, fnGoNode );
    }
    else
        nNdDiff = nSttNode - rPam.GetPoint()->nNode.GetIndex();
    if( bSectNdFnd )
        nNdDiff -= nSectDiff;

    if( !rPam.GetNode()->IsCntntNode() )
        rPam.GetPoint()->nContent.Assign( 0, 0 );

    // wird die History ueberhaupt benoetigt ??
    if( pHistory && !pHistory->Count() )
        DELETEZ( pHistory );
}

BOOL SwUndoDelete::SaveCntnt( const SwPosition* pStt, const SwPosition* pEnd,
                    SwTxtNode* pSttTxtNd, SwTxtNode* pEndTxtNd )
{
    ULONG nNdIdx = pStt->nNode.GetIndex();
    // 1 - kopiere den Anfang in den Start-String
    if( pSttTxtNd )
    {
        BOOL bOneNode = nSttNode == nEndNode;
        xub_StrLen nLen = bOneNode ? nEndCntnt - nSttCntnt
                                : pSttTxtNd->GetTxt().Len() - nSttCntnt;
        SwRegHistory aRHst( *pSttTxtNd, pHistory );
        // immer alle TextAttribute sichern; ist fuers Undo mit voll-
        // staendiger Attributierung am besten, wegen den evt.
        // Ueberlappenden Bereichen von An/Aus.
        pHistory->CopyAttr( pSttTxtNd->GetpSwpHints(), nNdIdx,
                            0, pSttTxtNd->GetTxt().Len(), TRUE );
        if( !bOneNode && pSttTxtNd->GetpSwAttrSet() )
                pHistory->CopyFmtAttr( *pSttTxtNd->GetpSwAttrSet(), nNdIdx );

        // die Laenge kann sich veraendert haben (!!Felder!!)
        nLen = ( bOneNode ? pEnd->nContent.GetIndex() : pSttTxtNd->GetTxt().Len() )
                - pStt->nContent.GetIndex();

        // loesche jetzt noch den Text (alle Attribut-Aenderungen kommen in
        // die Undo-History
        pSttStr = (String*)new String( pSttTxtNd->GetTxt().Copy( nSttCntnt, nLen ));
        pSttTxtNd->Erase( pStt->nContent, nLen );
        if( pSttTxtNd->GetpSwpHints() )
            pSttTxtNd->GetpSwpHints()->DeRegister();

        if( bOneNode )
            return FALSE;           // keine Nodes mehr verschieben
    }


    // 2 - kopiere das Ende in den End-String
    if( pEndTxtNd )
    {
        SwIndex aEndIdx( pEndTxtNd );
        nNdIdx = pEnd->nNode.GetIndex();
        SwRegHistory aRHst( *pEndTxtNd, pHistory );

        // immer alle TextAttribute sichern; ist fuers Undo mit voll-
        // staendiger Attributierung am besten, wegen den evt.
        // Ueberlappenden Bereichen von An/Aus.
        pHistory->CopyAttr( pEndTxtNd->GetpSwpHints(), nNdIdx, 0,
                            pEndTxtNd->GetTxt().Len(), TRUE );

        if( pEndTxtNd->GetpSwAttrSet() )
            pHistory->CopyFmtAttr( *pEndTxtNd->GetpSwAttrSet(), nNdIdx );

        // loesche jetzt noch den Text (alle Attribut-Aenderungen kommen in
        // die Undo-History
        pEndStr = (String*)new String( pEndTxtNd->GetTxt().Copy( 0,
                                    pEnd->nContent.GetIndex() ));
        pEndTxtNd->Erase( aEndIdx, pEnd->nContent.GetIndex() );
        if( pEndTxtNd->GetpSwpHints() )
            pEndTxtNd->GetpSwpHints()->DeRegister();
    }

    // sind es nur zwei Nodes, dann ist schon alles erledigt.
    if( ( pSttTxtNd || pEndTxtNd ) && nSttNode + 1 == nEndNode )
        return FALSE;           // keine Nodes mehr verschieben

    return TRUE;                // verschiebe die dazwischen liegenden Nodes
}


BOOL SwUndoDelete::CanGrouping( SwDoc* pDoc, const SwPaM& rDelPam )
{
    // ist das Undo groesser als 1 Node ? (sprich: Start und EndString)
    if( pSttStr ? !pSttStr->Len() || pEndStr : TRUE )
        return FALSE;

    // es kann nur das Loeschen von einzelnen char's zusammengefasst werden
    if( nSttNode != nEndNode || ( !bGroup && nSttCntnt+1 != nEndCntnt ))
        return FALSE;

    const SwPosition *pStt = rDelPam.Start(),
                    *pEnd = rDelPam.GetPoint() == pStt
                        ? rDelPam.GetMark()
                        : rDelPam.GetPoint();

    if( pStt->nNode != pEnd->nNode ||
        pStt->nContent.GetIndex()+1 != pEnd->nContent.GetIndex() ||
        pEnd->nNode != nSttNode )
        return FALSE;

    // untercheide zwischen BackSpace und Delete. Es muss dann das
    // Undo-Array unterschiedlich aufgebaut werden !!
    if( pEnd->nContent == nSttCntnt )
    {
        if( bGroup && !bBackSp ) return FALSE;
        bBackSp = TRUE;
    }
    else if( pStt->nContent == nSttCntnt )
    {
        if( bGroup && bBackSp ) return FALSE;
        bBackSp = FALSE;
    }
    else
        return FALSE;

    // sind die beiden Nodes (Nodes-/Undo-Array) ueberhaupt TextNodes?
    SwTxtNode * pDelTxtNd = pStt->nNode.GetNode().GetTxtNode();
    if( !pDelTxtNd ) return FALSE;

    xub_StrLen nUChrPos = bBackSp ? 0 : pSttStr->Len()-1;
    sal_Unicode cDelChar = pDelTxtNd->GetTxt().GetChar( pStt->nContent.GetIndex() );
    if( ( CH_TXTATR_BREAKWORD == cDelChar && CH_TXTATR_INWORD == cDelChar ) ||
        WordSelection::IsNormalChar( cDelChar ) !=
        WordSelection::IsNormalChar( pSttStr->GetChar( nUChrPos ) ))
        return FALSE;

    {
        SwRedlineSaveDatas* pTmpSav = new SwRedlineSaveDatas;
        if( !FillSaveData( rDelPam, *pTmpSav, FALSE ))
            delete pTmpSav, pTmpSav = 0;

        BOOL bOk = ( !pRedlSaveData && !pTmpSav ) ||
                   ( pRedlSaveData && pTmpSav &&
                SwUndo::CanRedlineGroup( *pRedlSaveData, *pTmpSav, bBackSp ));
        delete pTmpSav;
        if( !bOk )
            return FALSE;

        pDoc->DeleteRedline( rDelPam, FALSE );
    }

    // Ok, die beiden 'Deletes' koennen zusammen gefasst werden, also
    // 'verschiebe' das enstprechende Zeichen
    if( bBackSp )
        nSttCntnt--;    // BackSpace: Zeichen in Array einfuegen !!
    else
    {
        nEndCntnt++;    // Delete: Zeichen am Ende anhaengen
        nUChrPos++;
    }
    pSttStr->Insert( cDelChar, nUChrPos );
    pDelTxtNd->Erase( pStt->nContent, 1 );

    bGroup = TRUE;
    return TRUE;
}



SwUndoDelete::~SwUndoDelete()
{
    delete pSttStr;
    delete pEndStr;
    if( pMvStt )        // loesche noch den Bereich aus dem UndoNodes Array
    {
        // Insert speichert den Inhalt in der IconSection
        pMvStt->GetNode().GetNodes().Delete( *pMvStt, nNode );
        delete pMvStt;
    }
    delete pRedlData;
    delete pRedlSaveData;
}



void SwUndoDelete::Undo( SwUndoIter& rUndoIter )
{
    SwDoc* pDoc = &rUndoIter.GetDoc();
    BOOL bUndo = pDoc->DoesUndo();
    pDoc->DoUndo( FALSE );

    ULONG nCalcStt = nSttNode - nNdDiff;
    if( bSectNdFnd )
        nCalcStt -= nSectDiff;
    SwNodeIndex aIdx( pDoc->GetNodes(), nCalcStt );

    // SectionNodes blieben stehen und es wurde danach zusammengefasst
    if( bMvAroundSectNd && !bJoinNext && pSttStr && pEndStr )
        pDoc->GetNodes().GoNext( &aIdx );
    SwNode* pInsNd = &aIdx.GetNode();

    {       // Block, damit der SwPosition beim loeschen vom Node
            // abgemeldet ist
        SwPosition aPos( aIdx );
        if( !bDelFullPara )
        {
            if( pInsNd->IsTableNode() )
            {
                pInsNd = pDoc->GetNodes().MakeTxtNode( aIdx,
                        (SwTxtFmtColl*)pDoc->GetDfltTxtFmtColl() );
                aIdx--;
                aPos.nNode = aIdx;
                aPos.nContent.Assign( pInsNd->GetCntntNode(), nSttCntnt );
            }
            else
            {
                if( pInsNd->IsCntntNode() )
                    aPos.nContent.Assign( (SwCntntNode*)pInsNd, nSttCntnt );
#ifndef PRODUCT
                else
                    ASSERT( bSectNdFnd, "vor welchen Node kopieren?" );
#endif
                if( !bTblDelLastNd )
                    pInsNd = 0;         // Node nicht loeschen !!
            }
        }
        else
            pInsNd = 0;         // Node nicht loeschen !!

        SwNodes* pUNds = (SwNodes*)pDoc->GetUndoNds();
        BOOL bNodeMove = 0 != nNode;

        // damit nicht die Attribute aus dem "Start"-Node kopiert werden,
        // splitte erstmal den Node (wenn noetig)
        if( bSectNdFnd )
        {
            if( bJoinNext )
                aPos.nNode++;
        }
        else if( pEndStr )
        {
            // alle Attribute verwerfen, wurden alle gespeichert!
            SwTxtNode* pTxtNd = aPos.nNode.GetNode().GetTxtNode();
            if( pTxtNd && pTxtNd->GetpSwAttrSet() )
                pTxtNd->ResetAllAttr();

            if( pTxtNd->GetpSwpHints() )
                pTxtNd->ClearSwpHintsArr( FALSE );

            if( pSttStr )
            {
                pDoc->SplitNode( aPos );
                pTxtNd = aPos.nNode.GetNode().GetTxtNode();
                if( bMvAroundSectNd )
                {
                    // leider muessen wir den Node noch verschieben
                    SwNodeIndex aMvIdx( pDoc->GetNodes(), ( bJoinNext
                            ? nEndNode - nNdDiff - nNode + nSectDiff + 1
                            : nCalcStt )
                            );
                    if( bJoinNext )
                        aPos.nNode++;       // auf den naechsten
                    SwNodeRange aRg( aPos.nNode, -1, aPos.nNode );
                    pDoc->GetNodes()._MoveNodes( aRg, pDoc->GetNodes(), aMvIdx, TRUE );

                    if( !bJoinNext )
                        aPos.nNode = aMvIdx;
                }
            }

            pTxtNd->Insert( *pEndStr, aPos.nContent, INS_NOHINTEXPAND );
            if( !bNodeMove && pSttStr )
                aPos.nNode = nSttNode - nNdDiff;
        }
        else if( pSttStr && bNodeMove )
        {
            SwTxtNode * pNd = aPos.nNode.GetNode().GetTxtNode();
            if( pNd )
            {
                if( nSttCntnt < pNd->GetTxt().Len() )
                    pDoc->SplitNode( aPos );
                else
                    aPos.nNode++;
            }
        }


        if( bNodeMove )
        {
            SwNodeRange aRg( *pMvStt, 0, *pMvStt, nNode );
            SwNodeIndex aPrevIdx( aPos.nNode, -1 );
            pUNds->_Copy( aRg, aPos.nNode );

            // SectionNode-Modus und von unten nach oben selektiert:
            //  -> im EndNode steht noch der Rest vom Join => loeschen
            if( bSectNdFnd )
            {
                SwPosition aSpPos( bJoinNext ? aPrevIdx : aPos.nNode );
                if( !bJoinNext && !nSectDiff )      // move to next content node
                    pDoc->GetNodes().GoNext( &aSpPos.nNode );

                aSpPos.nContent.Assign( aSpPos.nNode.GetNode().GetCntntNode(),
                                        nSttCntnt );
                pDoc->SplitNode( aSpPos );
                if( bJoinNext )
                {
                    aPrevIdx = aPos.nNode;
                    pDoc->GetNodes().GoPrevious( &aPrevIdx );
                }
                else
                {
                    aPrevIdx++;
                    if( nSectDiff )
                        aPrevIdx++;
                    aSpPos.nNode--;
                }
                SwNodeRange aMvRg( aSpPos.nNode, 0, aSpPos.nNode, 1 );
                pDoc->GetNodes()._MoveNodes( aMvRg, pDoc->GetNodes(),
                                            aPrevIdx );
                pDoc->GetNodes().Delete( aPrevIdx, 1 );

                if( pEndStr )
                {
                    aPos.nNode = nEndNode - nNdDiff;    // am Anfang manipulieren
                    SwTxtNode * pTxtNd = aPos.nNode.GetNode().GetTxtNode();
                    if( pTxtNd->GetpSwAttrSet() )
                        pTxtNd->ResetAllAttr();

                    aPos.nContent.Assign( pTxtNd, 0 );
                    pTxtNd->Insert( *pEndStr, aPos.nContent, INS_NOHINTEXPAND );
                }
            }

            aPos.nNode = nSttNode - nNdDiff;    // am Anfang manipulieren
        }
        if( pSttStr )
        {
            SwTxtNode * pTxtNd = aPos.nNode.GetNode().GetTxtNode();
            // wenn mehr als ein Node geloescht wurde, dann wurden auch
            // alle "Node"-Attribute gespeichert
            if( pTxtNd->GetpSwAttrSet() && bNodeMove && !pEndStr )
                pTxtNd->ResetAllAttr();

            if( pTxtNd->GetpSwpHints() )
                pTxtNd->ClearSwpHintsArr( FALSE );

            // SectionNode-Modus und von oben nach unten selektiert:
            //  -> im StartNode steht noch der Rest vom Join => loeschen
            aPos.nContent.Assign( pTxtNd, nSttCntnt );
            pTxtNd->Insert( *pSttStr, aPos.nContent, INS_NOHINTEXPAND );
        }

        if( pHistory )
        {
            pHistory->TmpRollback( pDoc, nSetPos, FALSE );
            if( nSetPos )       // es gab Fussnoten/FlyFrames
            {
                // gibts ausser diesen noch andere ?
                if( nSetPos < pHistory->Count() )
                {
                    // dann sicher die Attribute anderen Attribute
                    SwHistory aHstr;
                    aHstr.Move( 0, pHistory, nSetPos );
                    pHistory->Rollback( pDoc );
                    pHistory->Move( 0, &aHstr );
                }
                else
                {
                    pHistory->Rollback( pDoc );
                    DELETEZ( pHistory );
                }
            }
        }

        if( bResetPgDesc || bResetPgBrk )
        {
            USHORT nStt = bResetPgDesc ? RES_PAGEDESC : RES_BREAK;
            USHORT nEnd = bResetPgBrk ? RES_BREAK : RES_PAGEDESC;

            SwNode* pNode = pDoc->GetNodes()[ nEndNode + 1 ];
            if( pNode->IsCntntNode() )
                ((SwCntntNode*)pNode)->ResetAttr( nStt, nEnd );
            else if( pNode->IsTableNode() )
                ((SwTableNode*)pNode)->GetTable().GetFrmFmt()->ResetAttr( nStt, nEnd );
        }
    }

    // den temp. eingefuegten Node noch loeschen !!
    if( pInsNd )
        pDoc->GetNodes().Delete( aIdx, 1 );

    if( pRedlSaveData )
        SetSaveData( *pDoc, *pRedlSaveData );

    pDoc->DoUndo( bUndo );          // Undo wieder einschalten
    SetPaM( rUndoIter, TRUE );
}

void SwUndoDelete::Redo( SwUndoIter& rUndoIter )
{
    rUndoIter.SetUpdateAttr( TRUE );

    SwPaM& rPam = *rUndoIter.pAktPam;
    SwDoc& rDoc = *rPam.GetDoc();

    SetPaM( rPam );

    if( pRedlSaveData )
        rDoc.DeleteRedline( rPam, FALSE );

    if( !bDelFullPara )
    {
        RemoveIdxFromRange( rPam, FALSE );
        SetPaM( rPam );

        if( !bJoinNext )            // Dann Selektion von unten nach oben
            rPam.Exchange();        // wieder herstellen!
    }

    if( pHistory )      // wurden Attribute gesichert ?
    {
        pHistory->SetTmpEnd( pHistory->Count() );
        SwHistory aHstr;
        aHstr.Move( 0, pHistory );

        if( bDelFullPara )
        {
            ASSERT( rPam.HasMark(), "PaM ohne Mark" );
            DelCntntIndex( *rPam.GetMark(), *rPam.GetPoint(),
                            DelCntntType(DELCNT_ALL | DELCNT_CHKNOCNTNT) );

            _DelBookmarks( rPam.GetMark()->nNode, rPam.GetPoint()->nNode );
        }
        else
            DelCntntIndex( *rPam.GetMark(), *rPam.GetPoint() );
        nSetPos = pHistory ? pHistory->Count() : 0;

        pHistory->Move( nSetPos, &aHstr );
    }
    else
    {
        if( bDelFullPara )
        {
            ASSERT( rPam.HasMark(), "PaM ohne Mark" );
            DelCntntIndex( *rPam.GetMark(), *rPam.GetPoint(),
                            DelCntntType(DELCNT_ALL | DELCNT_CHKNOCNTNT) );

            _DelBookmarks( rPam.GetMark()->nNode, rPam.GetPoint()->nNode );
        }
        else
            DelCntntIndex( *rPam.GetMark(), *rPam.GetPoint() );
        nSetPos = pHistory ? pHistory->Count() : 0;
    }

    if( !pSttStr && !pEndStr )
    {
        SwNodeIndex& rSttIdx = ( bDelFullPara || bJoinNext )
                                    ? rPam.GetMark()->nNode
                                    : rPam.GetPoint()->nNode;
        SwTableNode* pTblNd = rSttIdx.GetNode().GetTableNode();
        if( pTblNd )
        {
            if( bTblDelLastNd )
            {
                // dann am Ende wieder einen Node einfuegen
                const SwNodeIndex aTmpIdx( *pTblNd->EndOfSectionNode(), 1 );
                rDoc.GetNodes().MakeTxtNode( aTmpIdx,
                        rDoc.GetTxtCollFromPool( RES_POOLCOLL_STANDARD ) );
            }

            SwCntntNode* pNextNd = rDoc.GetNodes()[
                    pTblNd->EndOfSectionIndex()+1 ]->GetCntntNode();
            if( pNextNd )
            {
                SwFrmFmt* pTableFmt = pTblNd->GetTable().GetFrmFmt();
                const SfxPoolItem *pItem;
                if( SFX_ITEM_SET == pTableFmt->GetItemState( RES_PAGEDESC,
                    FALSE, &pItem ) )
                    pNextNd->SetAttr( *pItem );

                if( SFX_ITEM_SET == pTableFmt->GetItemState( RES_BREAK,
                    FALSE, &pItem ) )
                    pNextNd->SetAttr( *pItem );
            }
            pTblNd->DelFrms();
        }

        rDoc.GetNodes().Delete( rSttIdx, nEndNode - nSttNode );

        rPam.DeleteMark();
        // setze den Cursor immer in einen ContentNode !!
        if( !rPam.Move( fnMoveBackward, fnGoCntnt ) &&
            !rPam.Move( fnMoveForward, fnGoCntnt ) )
            rPam.GetPoint()->nContent.Assign( rPam.GetCntntNode(), 0 );
    }
    else if( bDelFullPara )
    {
        // der Pam wurde am Point( == Ende) um eins erhoeht, um einen
        // Bereich fuers Undo zu haben. Der muss jetzt aber wieder entfernt
        // werden!!!
        rPam.End()->nNode--;
        if( rPam.GetPoint()->nNode == rPam.GetMark()->nNode )
            *rPam.GetMark() = *rPam.GetPoint();
        rDoc.DelFullPara( rPam );
    }
    else
        rDoc.DeleteAndJoin( rPam );
}

void SwUndoDelete::Repeat( SwUndoIter& rUndoIter )
{
    if( UNDO_DELETE == rUndoIter.GetLastUndoId() )
        return;

    SwPaM& rPam = *rUndoIter.pAktPam;
    SwDoc& rDoc = *rPam.GetDoc();
    BOOL bGroupUndo = rDoc.DoesGroupUndo();
    rDoc.DoGroupUndo( FALSE );
    if( !rPam.HasMark() )
    {
        rPam.SetMark();
        rPam.Move( fnMoveForward, fnGoCntnt );
    }
    if( bDelFullPara )
        rDoc.DelFullPara( rPam );
    else
        rDoc.DeleteAndJoin( rPam );
    rDoc.DoGroupUndo( bGroupUndo );
    rUndoIter.pLastUndoObj = this;
}

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/core/undo/undel.cxx,v 1.1.1.1 2000-09-19 00:08:27 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.91  2000/09/18 16:04:28  willem.vandorp
      OpenOffice header added.

      Revision 1.90  2000/07/25 19:47:22  jp
      Bug #73345#: Undo - restore the node inside the last section and in the first

      Revision 1.89  2000/07/20 13:15:28  jp
      change old txtatr-character to the two new characters

      Revision 1.88  2000/05/19 12:53:32  jp
      use WordSelection class for check chars

      Revision 1.87  2000/05/09 10:03:58  jp
      Changes for Unicode

      Revision 1.86  1999/12/16 22:51:13  jp
      Bug #70704#: content objects inside expanded ranges

      Revision 1.85  1998/08/24 14:29:40  JP
      Bug #55458#: SwUndoDelete - umsetzen vom PageDesc/-Break jetzt per Flag merken


      Rev 1.84   24 Aug 1998 16:29:40   JP
   Bug #55458#: SwUndoDelete - umsetzen vom PageDesc/-Break jetzt per Flag merken

      Rev 1.83   02 Apr 1998 15:12:50   JP
   Redo: Undo-Flag wird schon von der EditShell abgeschaltet

      Rev 1.82   31 Mar 1998 15:44:46   JP
   Redo: den richtigen NodeIndex beim Delete benutzen

      Rev 1.81   11 Mar 1998 18:15:28   JP
   Redo: Selektion sollte noch die richtige Richtung haben

      Rev 1.80   09 Feb 1998 21:02:20   JP
   Copy-Schnittstelle am Doc hat sich geaendert

      Rev 1.79   06 Feb 1998 18:06:40   JP
   SwUndoDel: nicht zuviele Nodes verschieben

      Rev 1.78   16 Jan 1998 12:01:16   JP
   Redlines beachten, auch wenns abgeschaltet ist

      Rev 1.77   06 Jan 1998 16:26:04   JP
   Redline beachten

      Rev 1.76   19 Dec 1997 12:14:22   JP
   Undo: Redlining beachten

      Rev 1.75   17 Dec 1997 14:11:22   JP
   Undo: Range nicht zu gross anlegen

      Rev 1.74   10 Dec 1997 16:21:18   JP
   Undo: ggfs. den ContentIndex abmelden

      Rev 1.73   10 Dec 1997 16:04:02   AMA
   New: SwSectionFrm-Einbau

*************************************************************************/

