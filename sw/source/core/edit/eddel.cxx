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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <hintids.hxx>
#include <doc.hxx>
#include <editsh.hxx>
#include <cntfrm.hxx>
#include <pam.hxx>
#include <swundo.hxx>       // fuer die UndoIds
#include <edimp.hxx>
#include <IMark.hxx>
#include <docary.hxx>
#include <SwRewriter.hxx>
#include <undobj.hxx>
#include <globals.hrc>

#include <comcore.hrc>
#include <list>

/************************************************************
 * Loeschen
 ************************************************************/

void SwEditShell::DeleteSel( SwPaM& rPam, BOOL* pUndo )
{
    // nur bei Selektion
    if( !rPam.HasMark() || *rPam.GetPoint() == *rPam.GetMark())
        return;

    // besteht eine Selection in einer Tabelle ?
    // dann nur den Inhalt der selektierten Boxen loeschen
    // jetzt gibt es 2 Faelle die beachtet werden muessen:
    //  1. Point und Mark stehen in einer Box, Selection normal loeschen
    //  2. Point und Mark stehen in unterschiedlichen Boxen, alle
    // selektierten Boxen suchen in den Inhalt loeschen
    if( rPam.GetNode()->FindTableNode() &&
        rPam.GetNode()->StartOfSectionNode() !=
        rPam.GetNode(FALSE)->StartOfSectionNode() )
    {
        // in Tabellen das Undo gruppieren
        if( pUndo && !*pUndo )
        {
            GetDoc()->StartUndo( UNDO_START, NULL );
            *pUndo = TRUE;
        }
        SwPaM aDelPam( *rPam.Start() );
        const SwPosition* pEndSelPos = rPam.End();
        do {
            aDelPam.SetMark();
            SwNode* pNd = aDelPam.GetNode();
            const SwNode& rEndNd = *pNd->EndOfSectionNode();
            if( pEndSelPos->nNode.GetIndex() <= rEndNd.GetIndex() )
            {
                *aDelPam.GetPoint() = *pEndSelPos;
                pEndSelPos = 0;     // Pointer als Flag missbrauchen
            }
            else
            {
                // dann ans Ende der Section
                aDelPam.GetPoint()->nNode = rEndNd;
                aDelPam.Move( fnMoveBackward, fnGoCntnt );
            }
                // geschuetze Boxen ueberspringen !
            if( !pNd->IsCntntNode() ||
                !((SwCntntNode*)pNd)->GetFrm()->IsProtected() )
            {
                // alles loeschen
                GetDoc()->DeleteAndJoin( aDelPam );
                SaveTblBoxCntnt( aDelPam.GetPoint() );
            }

            if( !pEndSelPos )               // am Ende der Selection
                break;
            aDelPam.DeleteMark();
            aDelPam.Move( fnMoveForward, fnGoCntnt );   // naechste Box
        } while( pEndSelPos );
    }
    else
    {
            // alles loeschen
        GetDoc()->DeleteAndJoin( rPam );
        SaveTblBoxCntnt( rPam.GetPoint() );
    }

    // Selection wird nicht mehr benoetigt.
    rPam.DeleteMark();
}


long SwEditShell::Delete()
{
    SET_CURR_SHELL( this );
    long nRet = 0;
    if( !HasReadonlySel() )
    {
        StartAllAction();

        BOOL bUndo = GetCrsr()->GetNext() != GetCrsr();
        if( bUndo )     // mehr als eine Selection ?
        {
            SwRewriter aRewriter;
            aRewriter.AddRule(UNDO_ARG1, String(SW_RES(STR_MULTISEL)));

            GetDoc()->StartUndo( UNDO_DELETE, &aRewriter );
        }

        FOREACHPAM_START(this)
            DeleteSel( *PCURCRSR, &bUndo );
        FOREACHPAM_END()

        // falls eine Undo-Klammerung, dann hier beenden
        if( bUndo )
            GetDoc()->EndUndo( UNDO_DELETE, NULL );
        EndAllAction();
        nRet = 1;
    }
    return nRet;
}

long SwEditShell::Copy( SwEditShell* pDestShell )
{
    if( !pDestShell )
        pDestShell = this;

    SET_CURR_SHELL( pDestShell );

    // List of insert positions for smart insert of block selections
    std::list< boost::shared_ptr<SwPosition> > aInsertList;

    // Fill list of insert positions
    {
        SwPosition * pPos = 0;
        boost::shared_ptr<SwPosition> pInsertPos;
        USHORT nMove = 0;
        FOREACHPAM_START(this)

            if( !pPos )
            {
                if( pDestShell == this )
                {
                    // First cursor represents the target position!!
                    PCURCRSR->DeleteMark();
                    pPos = (SwPosition*)PCURCRSR->GetPoint();
                    continue;
                }
                else
                    pPos = pDestShell->GetCrsr()->GetPoint();
            }
            if( IsBlockMode() )
            {   // In block mode different insert positions will be calculated
                // by simulated cursor movements from the given first insert position
                if( nMove )
                {
                    SwCursor aCrsr( *pPos, 0, false);
                    if( aCrsr.UpDown( FALSE, nMove, 0, 0 ) )
                    {
                        pInsertPos.reset( new SwPosition( *aCrsr.GetPoint() ) );
                        aInsertList.push_back( pInsertPos );
                    }
                }
                else
                    pInsertPos.reset( new SwPosition( *pPos ) );
                ++nMove;
            }
            SwPosition *pTmp = IsBlockMode() ? pInsertPos.get() : pPos;
            // Check if a selection would be copied into itself
            if( pDestShell->GetDoc() == GetDoc() &&
                *PCURCRSR->Start() <= *pTmp && *pTmp < *PCURCRSR->End() )
                return FALSE;
        FOREACHPAM_END()
    }

    pDestShell->StartAllAction();
    SwPosition *pPos = 0;
    BOOL bRet = FALSE;
    BOOL bFirstMove = TRUE;
    SwNodeIndex aSttNdIdx( pDestShell->GetDoc()->GetNodes() );
    xub_StrLen nSttCntIdx = 0;
    // For block selection this list is filled with the insert positions
    std::list< boost::shared_ptr<SwPosition> >::iterator pNextInsert = aInsertList.begin();

    pDestShell->GetDoc()->StartUndo( UNDO_START, NULL );
    FOREACHPAM_START(this)

        if( !pPos )
        {
            if( pDestShell == this )
            {
                // First cursor represents the target position!!
                PCURCRSR->DeleteMark();
                pPos = (SwPosition*)PCURCRSR->GetPoint();
                continue;
            }
            else
                pPos = pDestShell->GetCrsr()->GetPoint();
        }
        if( !bFirstMove )
        {
            if( pNextInsert != aInsertList.end() )
            {
                pPos = pNextInsert->get();
                ++pNextInsert;
            }
            else if( IsBlockMode() )
                GetDoc()->SplitNode( *pPos, false );
        }

        // nur bei Selektion (nicht Textnodes haben Selection,
        // aber Point/GetMark sind gleich
        if( !PCURCRSR->HasMark() || *PCURCRSR->GetPoint() == *PCURCRSR->GetMark() )
            continue;

        if( bFirstMove )
        {
            // Anfangs-Position vom neuen Bereich merken
            aSttNdIdx = pPos->nNode.GetIndex()-1;
            nSttCntIdx = pPos->nContent.GetIndex();
            bFirstMove = FALSE;
        }

        const bool bSuccess( GetDoc()->CopyRange( *PCURCRSR, *pPos, false ) );
        if (!bSuccess)
            continue;

        SwPaM aInsertPaM(*pPos, SwPosition(aSttNdIdx));
        pDestShell->GetDoc()->MakeUniqueNumRules(aInsertPaM);

        bRet = TRUE;
    FOREACHPAM_END()


    // Maybe nothing has been moved?
    if( !bFirstMove )
    {
        SwPaM* pCrsr = pDestShell->GetCrsr();
        pCrsr->SetMark();
        pCrsr->GetPoint()->nNode = aSttNdIdx.GetIndex()+1;
        pCrsr->GetPoint()->nContent.Assign( pCrsr->GetCntntNode(),nSttCntIdx);
        pCrsr->Exchange();
    }
    else
    {
        // falls beim Move der Cursor "gewandert" ist, so setze hier auch
        // seinen GetMark um, damit dieser nie in den Wald zeigt.
        pDestShell->GetCrsr()->SetMark();
        pDestShell->GetCrsr()->DeleteMark();
    }
#if OSL_DEBUG_LEVEL > 1
// pruefe ob die Indizies auch in den richtigen Nodes angemeldet sind
{
    SwPaM* pCmp = (SwPaM*)pDestShell->GetCrsr();        // sicher den Pointer auf Cursor
    do {
        OSL_ENSURE( pCmp->GetPoint()->nContent.GetIdxReg()
                    == pCmp->GetCntntNode(), "Point im falschen Node" );
        OSL_ENSURE( pCmp->GetMark()->nContent.GetIdxReg()
                    == pCmp->GetCntntNode(FALSE), "Mark im falschen Node" );
        BOOL bTst = *pCmp->GetPoint() == *pCmp->GetMark();
        (void) bTst;
    } while( pDestShell->GetCrsr() != ( pCmp = (SwPaM*)pCmp->GetNext() ) );
}
#endif

    // Undo-Klammerung hier beenden
    pDestShell->GetDoc()->EndUndo( UNDO_END, NULL );
    pDestShell->EndAllAction();

    pDestShell->SaveTblBoxCntnt( pDestShell->GetCrsr()->GetPoint() );

    return (long)bRet;
}


    // Ersetz einen selektierten Bereich in einem TextNode mit dem
    // String. Ist fuers Suchen&Ersetzen gedacht.
    // bRegExpRplc - ersetze Tabs (\\t) und setze den gefundenen String
    //               ein ( nicht \& )
    //              z.B.: Fnd: "zzz", Repl: "xx\t\\t..&..\&"
    //                      --> "xx\t<Tab>..zzz..&"
BOOL SwEditShell::Replace( const String& rNewStr, BOOL bRegExpRplc )
{
    SET_CURR_SHELL( this );

    BOOL bRet = FALSE;
    if( !HasReadonlySel() )
    {
        StartAllAction();
        GetDoc()->StartUndo(UNDO_EMPTY, NULL);

        FOREACHPAM_START(this)
            if( PCURCRSR->HasMark() && *PCURCRSR->GetMark() != *PCURCRSR->GetPoint() )
            {
                bRet = GetDoc()->ReplaceRange( *PCURCRSR, rNewStr, bRegExpRplc )
                    || bRet;
                SaveTblBoxCntnt( PCURCRSR->GetPoint() );
            }
        FOREACHPAM_END()

        // Undo-Klammerung hier beenden
        GetDoc()->EndUndo(UNDO_EMPTY, NULL);
        EndAllAction();
    }
    return bRet;
}


    // Special-Methode fuer JOE's- Wizzards
BOOL SwEditShell::DelFullPara()
{
    BOOL bRet = FALSE;
    if( !IsTableMode() )
    {
        SwPaM* pCrsr = GetCrsr();
        // keine Mehrfach-Selection
        if( pCrsr->GetNext() == pCrsr && !HasReadonlySel() )
        {
            SET_CURR_SHELL( this );
            StartAllAction();
            bRet = GetDoc()->DelFullPara( *pCrsr );
            EndAllAction();
        }
    }
    return bRet;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
