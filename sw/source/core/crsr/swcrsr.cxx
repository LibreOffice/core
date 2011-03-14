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
#include <editeng/protitem.hxx>

#include <com/sun/star/i18n/WordType.hdl>
#include <com/sun/star/i18n/CharType.hdl>

#include <unotools/charclass.hxx>
#include <svl/ctloptions.hxx>
#include <swmodule.hxx>
#include <fmtcntnt.hxx>
#include <swtblfmt.hxx>
#include <swcrsr.hxx>
#include <unocrsr.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <ndtxt.hxx>
#include <section.hxx>
#include <swtable.hxx>
#include <cntfrm.hxx>
#include <rootfrm.hxx>
#include <txtfrm.hxx>
#include <scriptinfo.hxx>
#include <crstate.hxx>
#include <docsh.hxx>
#include <frmatr.hxx>
#include <breakit.hxx>
#include <crsskip.hxx>
#include <vcl/msgbox.hxx>
#include <mdiexp.hxx>           // ...Percent()
#include <statstr.hrc>          // ResId fuer Statusleiste
#include <redline.hxx>      // SwRedline


using namespace ::com::sun::star::i18n;


static const USHORT coSrchRplcThreshold = 60000;

struct _PercentHdl
{
    SwDocShell* pDSh;
    ULONG nActPos;
    BOOL bBack, bNodeIdx;

    _PercentHdl( ULONG nStt, ULONG nEnd, SwDocShell* pSh )
        : pDSh( pSh )
    {
        nActPos = nStt;
        if( 0 != ( bBack = (nStt > nEnd )) )
        {
            ULONG n = nStt; nStt = nEnd; nEnd = n;
        }
        ::StartProgress( STR_STATSTR_SEARCH, nStt, nEnd, 0 );
    }

    _PercentHdl( const SwPaM& rPam )
        : pDSh( (SwDocShell*)rPam.GetDoc()->GetDocShell() )
    {
        ULONG nStt, nEnd;
        if( rPam.GetPoint()->nNode == rPam.GetMark()->nNode )
        {
            bNodeIdx = FALSE;
            nStt = rPam.GetMark()->nContent.GetIndex();
            nEnd = rPam.GetPoint()->nContent.GetIndex();
        }
        else
        {
            bNodeIdx = TRUE;
            nStt = rPam.GetMark()->nNode.GetIndex();
            nEnd = rPam.GetPoint()->nNode.GetIndex();
        }
        nActPos = nStt;
        if( 0 != ( bBack = (nStt > nEnd )) )
        {
            ULONG n = nStt; nStt = nEnd; nEnd = n;
        }
        ::StartProgress( STR_STATSTR_SEARCH, nStt, nEnd, pDSh );
    }

    ~_PercentHdl()                      { ::EndProgress( pDSh ); }

    void NextPos( ULONG nPos ) const
        { ::SetProgressState( bBack ? nActPos - nPos : nPos, pDSh ); }

    void NextPos( SwPosition& rPos ) const
        {
            ULONG nPos;
            if( bNodeIdx )
                nPos = rPos.nNode.GetIndex();
            else
                nPos = rPos.nContent.GetIndex();
            ::SetProgressState( bBack ? nActPos - nPos : nPos, pDSh );
        }
};

SwCursor::SwCursor( const SwPosition &rPos, SwPaM* pRing, bool bColumnSel )
    : SwPaM( rPos, pRing ), pSavePos( 0 ), mnRowSpanOffset( 0 ), nCursorBidiLevel( 0 ),
    mbColumnSelection( bColumnSel )
{
}

// @@@ semantic: no copy ctor.
SwCursor::SwCursor( SwCursor& rCpy )
    : SwPaM( rCpy ), pSavePos( 0 ), mnRowSpanOffset( rCpy.mnRowSpanOffset ),
    nCursorBidiLevel( rCpy.nCursorBidiLevel ), mbColumnSelection( rCpy.mbColumnSelection )
{
}

SwCursor::~SwCursor()
{
    while( pSavePos )
    {
        _SwCursor_SavePos* pNxt = pSavePos->pNext;
        delete pSavePos;
        pSavePos = pNxt;
    }
}

SwCursor* SwCursor::Create( SwPaM* pRing ) const
{
    return new SwCursor( *GetPoint(), pRing, false );
}

bool SwCursor::IsReadOnlyAvailable() const
{
    return false;
}

BOOL SwCursor::IsSkipOverHiddenSections() const
{
    return TRUE;
}

BOOL SwCursor::IsSkipOverProtectSections() const
{
    return !IsReadOnlyAvailable();
}


// Das CreateNewSavePos ist virtual, damit abgeleitete Klassen vom Cursor
// gegebenenfalls eigene SaveObjecte anlegen und in den virtuellen
// Check-Routinen verwenden koennen.
void SwCursor::SaveState()
{
    _SwCursor_SavePos* pNew = CreateNewSavePos();
    pNew->pNext = pSavePos;
    pSavePos = pNew;
}

void SwCursor::RestoreState()
{
    if( pSavePos ) // Robust
    {
        _SwCursor_SavePos* pDel = pSavePos;
        pSavePos = pSavePos->pNext;
        delete pDel;
    }
}

_SwCursor_SavePos* SwCursor::CreateNewSavePos() const
{
    return new _SwCursor_SavePos( *this );
}

// stelle fest, ob sich der Point ausserhalb des Content-Bereichs
// vom Nodes-Array befindet
BOOL SwCursor::IsNoCntnt() const
{
    return GetPoint()->nNode.GetIndex() <
            GetDoc()->GetNodes().GetEndOfExtras().GetIndex();
}

bool SwCursor::IsSelOvrCheck(int)
{
    return false;
}

// extracted from IsSelOvr()
bool SwTableCursor::IsSelOvrCheck(int eFlags)
{
    SwNodes& rNds = GetDoc()->GetNodes();
    // check sections of nodes array
    if( (nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION & eFlags)
        && HasMark() )
    {
        SwNodeIndex aOldPos( rNds, GetSavePos()->nNode );
        if( !CheckNodesRange( aOldPos, GetPoint()->nNode, TRUE ))
        {
            GetPoint()->nNode = aOldPos;
            GetPoint()->nContent.Assign( GetCntntNode(), GetSavePos()->nCntnt );
            return true;
        }
    }
    return SwCursor::IsSelOvrCheck(eFlags);
}

BOOL SwCursor::IsSelOvr( int eFlags )
{
    SwDoc* pDoc = GetDoc();
    SwNodes& rNds = pDoc->GetNodes();

    BOOL bSkipOverHiddenSections = IsSkipOverHiddenSections();
    BOOL bSkipOverProtectSections = IsSkipOverProtectSections();

    if ( IsSelOvrCheck( eFlags ) )
    {
        return TRUE;
    }

// neu: Bereiche ueberpruefen
// Anfang
    if( pSavePos->nNode != GetPoint()->nNode.GetIndex() &&
        //JP 28.10.97: Bug 45129 - im UI-ReadOnly ist alles erlaubt
        ( !pDoc->GetDocShell() || !pDoc->GetDocShell()->IsReadOnlyUI() ))
    {
        // teste doch mal die neuen Sections:
        SwNodeIndex& rPtIdx = GetPoint()->nNode;
        const SwSectionNode* pSectNd = rPtIdx.GetNode().FindSectionNode();
        if( pSectNd &&
            ((bSkipOverHiddenSections && pSectNd->GetSection().IsHiddenFlag() ) ||
             (bSkipOverProtectSections && pSectNd->GetSection().IsProtectFlag() )))
        {
            if( 0 == ( nsSwCursorSelOverFlags::SELOVER_CHANGEPOS & eFlags ) )
            {
                // dann wars das schon
                RestoreSavePos();
                return TRUE;
            }

            // dann setze den Cursor auf die neue Position:
            SwNodeIndex aIdx( rPtIdx );
            xub_StrLen nCntntPos = pSavePos->nCntnt;
            int bGoNxt = pSavePos->nNode < rPtIdx.GetIndex();
            SwCntntNode* pCNd = bGoNxt
                    ? rNds.GoNextSection( &rPtIdx, bSkipOverHiddenSections, bSkipOverProtectSections)
                    : rNds.GoPrevSection( &rPtIdx, bSkipOverHiddenSections, bSkipOverProtectSections);
            if( !pCNd && ( nsSwCursorSelOverFlags::SELOVER_ENABLEREVDIREKTION & eFlags ))
            {
                bGoNxt = !bGoNxt;
                pCNd = bGoNxt ? rNds.GoNextSection( &rPtIdx, bSkipOverHiddenSections, bSkipOverProtectSections)
                              : rNds.GoPrevSection( &rPtIdx, bSkipOverHiddenSections, bSkipOverProtectSections);
            }

            int bIsValidPos = 0 != pCNd;
            BOOL bValidNodesRange = bIsValidPos &&
                                    ::CheckNodesRange( rPtIdx, aIdx, TRUE );
            if( !bValidNodesRange )
            {
                rPtIdx = pSavePos->nNode;
                if( 0 == ( pCNd = rPtIdx.GetNode().GetCntntNode() ) )
                {
                    bIsValidPos = FALSE;
                    nCntntPos = 0;
                    rPtIdx = aIdx;
                    if( 0 == ( pCNd = rPtIdx.GetNode().GetCntntNode() ) )
                    {
                        // dann auf den Anfang vom Doc
                        rPtIdx = rNds.GetEndOfExtras();
                        pCNd = rNds.GoNext( &rPtIdx );
                    }
                }
            }

            // ContentIndex noch anmelden:
            xub_StrLen nTmpPos = bIsValidPos ? (bGoNxt ? 0 : pCNd->Len()) : nCntntPos;
            GetPoint()->nContent.Assign( pCNd, nTmpPos );
            if( !bIsValidPos || !bValidNodesRange ||
                // sollten wir in einer Tabelle gelandet sein?
                IsInProtectTable( TRUE ) )
                return TRUE;
        }

        // oder sollte eine geschuetzte Section innerhalb der Selektion liegen?
        if( HasMark() && bSkipOverProtectSections)
        {
            ULONG nSttIdx = GetMark()->nNode.GetIndex(),
                  nEndIdx = GetPoint()->nNode.GetIndex();
            if( nEndIdx <= nSttIdx )
            {
                ULONG nTmp = nSttIdx;
                nSttIdx = nEndIdx;
                nEndIdx = nTmp;
            }

            const SwSectionFmts& rFmts = pDoc->GetSections();
            for( USHORT n = 0; n < rFmts.Count(); ++n )
            {
                const SwSectionFmt* pFmt = rFmts[n];
                const SvxProtectItem& rProtect = pFmt->GetProtect();
                if( rProtect.IsCntntProtected() )
                {
                    const SwFmtCntnt& rCntnt = pFmt->GetCntnt(FALSE);
                    OSL_ENSURE( rCntnt.GetCntntIdx(), "wo ist der SectionNode?" );
                    ULONG nIdx = rCntnt.GetCntntIdx()->GetIndex();
                    if( nSttIdx <= nIdx && nEndIdx >= nIdx )
                    {
                        // ist es keine gelinkte Section, dann kann sie auch
                        // nicht mitselektiert werden
                        const SwSection& rSect = *pFmt->GetSection();
                        if( CONTENT_SECTION == rSect.GetType() )
                        {
                            RestoreSavePos();
                            return TRUE;
                        }
                    }
                }
            }
        }

    }
// Ende
// neu: Bereiche ueberpruefen

    const SwNode* pNd = &GetPoint()->nNode.GetNode();
    if( pNd->IsCntntNode() && !dynamic_cast<SwUnoCrsr*>(this) )
    {
        const SwCntntFrm* pFrm = ((SwCntntNode*)pNd)->GetFrm();
        if( pFrm && pFrm->IsValid() && 0 == pFrm->Frm().Height() &&
            0 != ( nsSwCursorSelOverFlags::SELOVER_CHANGEPOS & eFlags ) )
        {
            // skip to the next / prev valid paragraph with a layout
            SwNodeIndex& rPtIdx = GetPoint()->nNode;
            int bGoNxt = pSavePos->nNode < rPtIdx.GetIndex();
            while( 0 != ( pFrm = ( bGoNxt ? pFrm->GetNextCntntFrm()
                                          : pFrm->GetPrevCntntFrm() )) &&
                    0 == pFrm->Frm().Height() )
                ;

            // --> LIJIAN/FME 2007-11-27 #i72394# skip to prev /next valid paragraph
            // with a layout in case the first search did not succeed:
            if( !pFrm )
            {
                bGoNxt = !bGoNxt;
                pFrm = ((SwCntntNode*)pNd)->GetFrm();
                while ( pFrm && 0 == pFrm->Frm().Height() )
                {
                    pFrm = bGoNxt ? pFrm->GetNextCntntFrm()
                        :   pFrm->GetPrevCntntFrm();
                }
            }
            // <--

            SwCntntNode* pCNd;
            if( pFrm && 0 != (pCNd = (SwCntntNode*)pFrm->GetNode()) )
            {
                // set this cntntNode as new position
                rPtIdx = *pCNd;
                pNd = pCNd;

                // ContentIndex noch anmelden:
                xub_StrLen nTmpPos = bGoNxt ? 0 : pCNd->Len();
                GetPoint()->nContent.Assign( pCNd, nTmpPos );

                    // sollten wir in einer Tabelle gelandet sein?
                if( IsInProtectTable( TRUE ) )
                    pFrm = 0;
            }
        }

        if( !pFrm )
        {
            DeleteMark();
            RestoreSavePos();
            return TRUE;        // ohne Frames geht gar nichts!
        }
    }

    // darf der Cursor in geschuetzen "Nodes" stehen?
    if( 0 == ( nsSwCursorSelOverFlags::SELOVER_CHANGEPOS & eFlags ) && !IsAtValidPos() )
    {
        DeleteMark();
        RestoreSavePos();
        return TRUE;
    }

    if( !HasMark() )
        return FALSE;

    //JP 19.08.98: teste mal auf ungueltige Selektion - sprich ueber
    //              GrundSections:
    if( !::CheckNodesRange( GetMark()->nNode, GetPoint()->nNode, TRUE ))
    {
        DeleteMark();
        RestoreSavePos();
        return TRUE;        // ohne Frames geht gar nichts!
    }

    const SwTableNode* pPtNd = pNd->FindTableNode();

    if( (pNd = &GetMark()->nNode.GetNode())->IsCntntNode() &&
        !((SwCntntNode*)pNd)->GetFrm() && !dynamic_cast<SwUnoCrsr*>(this) )
    {
        DeleteMark();
        RestoreSavePos();
        return TRUE;        // ohne Frames geht gar nichts!
    }

    const SwTableNode* pMrkNd = pNd->FindTableNode();

    // beide in keinem oder beide im gleichen TableNode
    if( ( !pMrkNd && !pPtNd ) || pPtNd == pMrkNd )
        return FALSE;

    // in unterschiedlichen Tabellen oder nur Mark in der Tabelle
    if( ( pPtNd && pMrkNd ) || pMrkNd )
    {                       // dann lasse das nicht zu, alte Pos zurueck
        RestoreSavePos();
        // Crsr bleibt an der alten Position
        return TRUE;
    }

    // ACHTUNG: dieses kann nicht im TableMode geschehen !!
    if( pPtNd )     // nur Point in Tabelle, dann gehe hinter/vor diese
    {
        if( nsSwCursorSelOverFlags::SELOVER_CHANGEPOS & eFlags )
        {
            BOOL bSelTop = GetPoint()->nNode.GetIndex() <
                    (( nsSwCursorSelOverFlags::SELOVER_TOGGLE & eFlags ) ? pSavePos->nNode
                                                 : GetMark()->nNode.GetIndex());

            do {
                // in Schleife fuer Tabelle hinter Tabelle
                ULONG nSEIdx = pPtNd->EndOfSectionIndex();
                ULONG nSttEndTbl = nSEIdx + 1; // dflt. Sel. nach unten

                if( bSelTop )                               // Sel. nach oben
                    nSttEndTbl = rNds[ nSEIdx ]->StartOfSectionIndex() - 1;

                GetPoint()->nNode = nSttEndTbl;
                const SwNode* pMyNd = GetNode();

                if( pMyNd->IsSectionNode() || ( pMyNd->IsEndNode() &&
                    pMyNd->StartOfSectionNode()->IsSectionNode() ) )
                {
                    // die lassen wir zu:
                    pMyNd = bSelTop
                        ? rNds.GoPrevSection( &GetPoint()->nNode,TRUE,FALSE )
                        : rNds.GoNextSection( &GetPoint()->nNode,TRUE,FALSE );

                    /* #i12312# Handle failure of Go{Prev|Next}Section */
                    if ( 0 == pMyNd)
                        break;

                    if( 0 != ( pPtNd = pMyNd->FindTableNode() ))
                        continue;
                }

                if( pMyNd->IsCntntNode() &&      // ist es ein ContentNode ??
                    ::CheckNodesRange( GetMark()->nNode,
                                       GetPoint()->nNode, TRUE ))
                {
                    // TABLE IN TABLE
                    const SwTableNode* pOuterTableNd = pMyNd->FindTableNode();
                    if ( pOuterTableNd )
                        pMyNd = pOuterTableNd;
                    else
                    {
                        SwCntntNode* pCNd = (SwCntntNode*)pMyNd;
                        xub_StrLen nTmpPos = bSelTop ? pCNd->Len() : 0;
                        GetPoint()->nContent.Assign( pCNd, nTmpPos );
                        return FALSE;
                    }
                }
                if( bSelTop
                    ? ( !pMyNd->IsEndNode() || 0 == ( pPtNd = pMyNd->FindTableNode() ))
                    : 0 == ( pPtNd = pMyNd->GetTableNode() ))
                    break;
            } while( TRUE );
        }

        // dann verbleibe auf der alten Position
        RestoreSavePos();
        return TRUE;        // Crsr bleibt an der alten Position
    }
    return FALSE;       // was bleibt noch ??
}

#if defined( UNX )
#define IDX     (*pCellStt)
#else
#define IDX     aCellStt
#endif


BOOL SwCursor::IsInProtectTable( BOOL bMove, BOOL bChgCrsr )
{
    SwCntntNode* pCNd = GetCntntNode();
    if( !pCNd )
        return FALSE;

    // No table, no protected cell:
    const SwTableNode* pTableNode = pCNd->FindTableNode();
    if ( !pTableNode )
        return FALSE;

    // Current position == last save position?
    if ( pSavePos->nNode == GetPoint()->nNode.GetIndex() )
        return FALSE;

    // Check for convered cell:
    bool bInCoveredCell = false;
    const SwStartNode* pTmpSttNode = pCNd->FindTableBoxStartNode();
    OSL_ENSURE( pTmpSttNode, "In table, therefore I expect to get a SwTableBoxStartNode" );
    const SwTableBox* pBox = pTmpSttNode ? pTableNode->GetTable().GetTblBox( pTmpSttNode->GetIndex() ) : 0; //Robust #151355
    if ( pBox && pBox->getRowSpan() < 1 ) // Robust #151270
        bInCoveredCell = true;

    // Positions of covered cells are not acceptable:
    if ( !bInCoveredCell )
    {
        // Position not protected?
        if ( !pCNd->IsProtect() )
            return FALSE;

        // Cursor in protected cells allowed?
        if ( IsReadOnlyAvailable() )
            return FALSE;
    }

    // If we reach this point, we are in a protected or covered table cell!

    if( !bMove )
    {
        if( bChgCrsr )
            // restore the last save position
            RestoreSavePos();
        return TRUE;        // Crsr bleibt an der alten Position
    }

    // wir stehen in einer geschuetzten TabellenZelle
    // von Oben nach Unten Traveln ?
    if( pSavePos->nNode < GetPoint()->nNode.GetIndex() )
    {
        // suche die naechste "gueltige" Box

        // folgt nach dem EndNode der Zelle ein weiterer StartNode, dann
        // gibt es auch eine naechste Zelle
#if defined( UNX )
        SwNodeIndex* pCellStt = new SwNodeIndex( *GetNode()->
                        FindTableBoxStartNode()->EndOfSectionNode(), 1 );
#else
        SwNodeIndex aCellStt( *GetNode()->FindTableBoxStartNode()->EndOfSectionNode(), 1 );
#endif
        BOOL bProt = TRUE;
GoNextCell:
        do {
            if( !IDX.GetNode().IsStartNode() )
                break;
            IDX++;
            if( 0 == ( pCNd = IDX.GetNode().GetCntntNode() ))
                pCNd = IDX.GetNodes().GoNext( &IDX );
            if( 0 == ( bProt = pCNd->IsProtect() ))
                break;
            IDX.Assign( *pCNd->FindTableBoxStartNode()->EndOfSectionNode(), 1 );
        } while( bProt );

SetNextCrsr:
        if( !bProt )        // eine freie Zelle gefunden
        {
            GetPoint()->nNode = IDX;
#if defined( UNX )
            delete pCellStt;
#endif
            SwCntntNode* pTmpCNd = GetCntntNode();
            if( pTmpCNd )
            {
                GetPoint()->nContent.Assign( pTmpCNd, 0 );
                return FALSE;
            }
            return IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                             nsSwCursorSelOverFlags::SELOVER_CHANGEPOS );
        }
        // am Ende der Tabelle, also setze hinter diese
        IDX++;     // auf den naechsten Node
        SwNode* pNd;
        if( ( pNd = &IDX.GetNode())->IsEndNode() || HasMark())
        {
            // Tabelle allein in einem FlyFrame oder SSelection,
            // dann verbleibe auf der alten Position
            if( bChgCrsr )
                RestoreSavePos();
#if defined( UNX )
            delete pCellStt;
#endif
            return TRUE;        // Crsr bleibt an der alten Position
        }
        else if( pNd->IsTableNode() && IDX++ )
            goto GoNextCell;

        bProt = FALSE;      // Index steht jetzt auf einem ContentNode
        goto SetNextCrsr;
    }

    // suche die vorherige "gueltige" Box
    {
        // liegt vor dem StartNode der Zelle ein weiterer EndNode, dann
        // gibt es auch eine vorherige Zelle
#if defined( UNX )
        SwNodeIndex* pCellStt = new SwNodeIndex(
                    *GetNode()->FindTableBoxStartNode(), -1 );
#else
        SwNodeIndex aCellStt( *GetNode()->FindTableBoxStartNode(), -1 );
#endif
        SwNode* pNd;
        BOOL bProt = TRUE;
GoPrevCell:
        do {
            if( !( pNd = &IDX.GetNode())->IsEndNode() )
                break;
            IDX.Assign( *pNd->StartOfSectionNode(), +1 );
            if( 0 == ( pCNd = IDX.GetNode().GetCntntNode() ))
                pCNd = pNd->GetNodes().GoNext( &IDX );
            if( 0 == ( bProt = pCNd->IsProtect() ))
                break;
            IDX.Assign( *pNd->FindTableBoxStartNode(), -1 );
        } while( bProt );

SetPrevCrsr:
        if( !bProt )        // eine freie Zelle gefunden
        {
            GetPoint()->nNode = IDX;
#if defined( UNX )
            delete pCellStt;
#endif
            SwCntntNode* pTmpCNd = GetCntntNode();
            if( pTmpCNd )
            {
                GetPoint()->nContent.Assign( pTmpCNd, 0 );
                return FALSE;
            }
            return IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                             nsSwCursorSelOverFlags::SELOVER_CHANGEPOS );
        }
        // am Start der Tabelle, also setze vor diese
        IDX--;     // auf den naechsten Node
        if( ( pNd = &IDX.GetNode())->IsStartNode() || HasMark() )
        {
            // Tabelle allein in einem FlyFrame oder Selektion,
            // dann verbleibe auf der alten Position
            if( bChgCrsr )
                RestoreSavePos();
#if defined( UNX )
            delete pCellStt;
#endif
            return TRUE;        // Crsr bleibt an der alten Position
        }
        else if( pNd->StartOfSectionNode()->IsTableNode() && IDX-- )
            goto GoPrevCell;

        bProt = FALSE;      // Index steht jetzt auf einem ContentNode
        goto SetPrevCrsr;
    }
}

// TRUE: an die Position kann der Cursor gesetzt werden
BOOL SwCursor::IsAtValidPos( BOOL bPoint ) const
{
    const SwDoc* pDoc = GetDoc();
    const SwPosition* pPos = bPoint ? GetPoint() : GetMark();
    const SwNode* pNd = &pPos->nNode.GetNode();

    if( pNd->IsCntntNode() && !((SwCntntNode*)pNd)->GetFrm() &&
        !dynamic_cast<const SwUnoCrsr*>(this) )
    {
        return FALSE;
    }

        //JP 28.10.97: Bug 45129 - im UI-ReadOnly ist alles erlaubt
    if( !pDoc->GetDocShell() || !pDoc->GetDocShell()->IsReadOnlyUI() )
        return TRUE;

    BOOL bCrsrInReadOnly = IsReadOnlyAvailable();
    if( !bCrsrInReadOnly && pNd->IsProtect() )
        return FALSE;

    const SwSectionNode* pSectNd = pNd->FindSectionNode();
    if( pSectNd && (pSectNd->GetSection().IsHiddenFlag() ||
                    ( !bCrsrInReadOnly && pSectNd->GetSection().IsProtectFlag() )))
        return FALSE;

    return TRUE;
}

void SwCursor::SaveTblBoxCntnt( const SwPosition* ) {}

// setze den SRange fuer das Suchen im Dokument
SwMoveFnCollection* SwCursor::MakeFindRange( SwDocPositions nStart,
                                SwDocPositions nEnd, SwPaM* pRange ) const
{
    pRange->SetMark();
    FillFindPos( nStart, *pRange->GetMark() );
    FillFindPos( nEnd, *pRange->GetPoint() );

    // bestimme die Richtung, in der zu suchen ist
    // ( GetPoint > GetMark -> vorwaerts, sonst rueckwaerts )
    return ( DOCPOS_START == nStart || DOCPOS_OTHERSTART == nStart ||
              (DOCPOS_CURR == nStart &&
                (DOCPOS_END == nEnd || DOCPOS_OTHEREND == nEnd ) ))
                ? fnMoveForward : fnMoveBackward;
}


ULONG lcl_FindSelection( SwFindParas& rParas, SwCursor* pCurCrsr,
                        SwMoveFn fnMove, SwCursor*& pFndRing,
                        SwPaM& aRegion, FindRanges eFndRngs,
                        BOOL bInReadOnly, BOOL& bCancel )
{
    SwDoc* pDoc = pCurCrsr->GetDoc();
    BOOL bDoesUndo = pDoc->DoesUndo();
    int nFndRet = 0;
    ULONG nFound = 0;
    int bSrchBkwrd = fnMove == fnMoveBackward, bEnde = FALSE;
    SwPaM *pTmpCrsr = pCurCrsr, *pSaveCrsr = pCurCrsr;

    // only create progress-bar for ShellCrsr
    bool bIsUnoCrsr = 0 != dynamic_cast<SwUnoCrsr*>(pCurCrsr);
    _PercentHdl* pPHdl = 0;
    USHORT nCrsrCnt = 0;
    if( FND_IN_SEL & eFndRngs )
    {
        while( pCurCrsr != ( pTmpCrsr = (SwPaM*)pTmpCrsr->GetNext() ))
            ++nCrsrCnt;
        if( nCrsrCnt && !bIsUnoCrsr )
            pPHdl = new _PercentHdl( 0, nCrsrCnt, pDoc->GetDocShell() );
    }
    else
        pSaveCrsr = (SwPaM*)pSaveCrsr->GetPrev();

    do {
        aRegion.SetMark();
        // egal in welche Richtung, SPoint ist immer groesser als Mark,
        // wenn der Suchbereich gueltig ist !!
        SwPosition *pSttPos = aRegion.GetMark(),
                        *pEndPos = aRegion.GetPoint();
        *pSttPos = *pTmpCrsr->Start();
        *pEndPos = *pTmpCrsr->End();
        if( bSrchBkwrd )
            aRegion.Exchange();

        if( !nCrsrCnt && !pPHdl && !bIsUnoCrsr )
            pPHdl = new _PercentHdl( aRegion );

        // solange gefunden und nicht auf gleicher Position haengen bleibt
        while(  *pSttPos <= *pEndPos &&
                0 != ( nFndRet = rParas.Find( pCurCrsr, fnMove,
                                            &aRegion, bInReadOnly )) &&
                ( !pFndRing ||
                    *pFndRing->GetPoint() != *pCurCrsr->GetPoint() ||
                    *pFndRing->GetMark() != *pCurCrsr->GetMark() ))
        {
            if( !( FIND_NO_RING & nFndRet ))
            {
                // Bug 24084: Ring richtig herum aufbauen -> gleiche Mimik
                //            wie beim CreateCrsr !!!!

                SwCursor* pNew = pCurCrsr->Create( pFndRing );
                if( !pFndRing )
                    pFndRing = pNew;

                pNew->SetMark();
                *pNew->GetMark() = *pCurCrsr->GetMark();
            }

            ++nFound;

            if( !( eFndRngs & FND_IN_SELALL) )
            {
                bEnde = TRUE;
                break;
            }

            if( coSrchRplcThreshold == nFound && pDoc->DoesUndo()
                && rParas.IsReplaceMode())
            {
                short nRet = pCurCrsr->MaxReplaceArived();
                if( RET_YES == nRet )
                {
                    pDoc->DelAllUndoObj();
                    pDoc->DoUndo( FALSE );
                }
                else
                {
                    bEnde = TRUE;
                    if(RET_CANCEL == nRet)
                    {
                        bCancel = TRUE;
                        //unwind() ??
                    }
                    break;
                }
            }

            if( bSrchBkwrd )
                // bewege pEndPos vor den gefundenen Bereich
                *pEndPos = *pCurCrsr->Start();
            else
                // bewege pSttPos hinter den gefundenen Bereich
                *pSttPos = *pCurCrsr->End();

            if( *pSttPos == *pEndPos )      // im Bereich, aber am Ende
                break;                      // fertig

            if( !nCrsrCnt && pPHdl )
            {
                pPHdl->NextPos( *aRegion.GetMark() );
            }
        }

        if( bEnde || !( eFndRngs & ( FND_IN_SELALL | FND_IN_SEL )) )
            break;

        pTmpCrsr = ((SwPaM*)pTmpCrsr->GetNext());
        if( nCrsrCnt && pPHdl )
        {
            pPHdl->NextPos( ++pPHdl->nActPos );
        }

    } while( pTmpCrsr != pSaveCrsr );

    if( nFound && !pFndRing )       // falls kein Ring aufgebaut werden soll
        pFndRing = pCurCrsr->Create();

    delete pPHdl;
    pDoc->DoUndo( bDoesUndo );
    return nFound;
}


int lcl_MakeSelFwrd( const SwNode& rSttNd, const SwNode& rEndNd,
                        SwPaM& rPam, int bFirst )
{
    if( rSttNd.GetIndex() + 1 == rEndNd.GetIndex() )
        return FALSE;

    SwNodes& rNds = rPam.GetDoc()->GetNodes();
    rPam.DeleteMark();
    SwCntntNode* pCNd;
    if( !bFirst )
    {
        rPam.GetPoint()->nNode = rSttNd;
        pCNd = rNds.GoNext( &rPam.GetPoint()->nNode );
        if( !pCNd )
            return FALSE;
        pCNd->MakeStartIndex( &rPam.GetPoint()->nContent );
    }
    else if( rSttNd.GetIndex() > rPam.GetPoint()->nNode.GetIndex() ||
             rPam.GetPoint()->nNode.GetIndex() >= rEndNd.GetIndex() )
        return FALSE;       // steht nicht in dieser Section

    rPam.SetMark();
    rPam.GetPoint()->nNode = rEndNd;
    pCNd = rNds.GoPrevious( &rPam.GetPoint()->nNode );
    if( !pCNd )
        return FALSE;
    pCNd->MakeEndIndex( &rPam.GetPoint()->nContent );

    return *rPam.GetMark() < *rPam.GetPoint();
}


int lcl_MakeSelBkwrd( const SwNode& rSttNd, const SwNode& rEndNd,
                        SwPaM& rPam, int bFirst )
{
    if( rEndNd.GetIndex() + 1 == rSttNd.GetIndex() )
        return FALSE;

    SwNodes& rNds = rPam.GetDoc()->GetNodes();
    rPam.DeleteMark();
    SwCntntNode* pCNd;
    if( !bFirst )
    {
        rPam.GetPoint()->nNode = rSttNd;
        pCNd = rNds.GoPrevious( &rPam.GetPoint()->nNode );
        if( !pCNd )
            return FALSE;
        pCNd->MakeEndIndex( &rPam.GetPoint()->nContent );
    }
    else if( rEndNd.GetIndex() > rPam.GetPoint()->nNode.GetIndex() ||
             rPam.GetPoint()->nNode.GetIndex() >= rSttNd.GetIndex() )
        return FALSE;       // steht nicht in dieser Section

    rPam.SetMark();
    rPam.GetPoint()->nNode = rEndNd;
    pCNd = rNds.GoNext( &rPam.GetPoint()->nNode );
    if( !pCNd )
        return FALSE;
    pCNd->MakeStartIndex( &rPam.GetPoint()->nContent );

    return *rPam.GetPoint() < *rPam.GetMark();
}


// diese Methode "sucht" fuer alle Anwendungsfaelle, denn in SwFindParas
// steht immer die richtigen Parameter und die entsprechende Find-Methode

ULONG SwCursor::FindAll( SwFindParas& rParas,
                            SwDocPositions nStart, SwDocPositions nEnde,
                            FindRanges eFndRngs, BOOL& bCancel )
{
    bCancel = FALSE;
    SwCrsrSaveState aSaveState( *this );

    // Region erzeugen, ohne das diese in den Ring aufgenommen wird !
    SwPaM aRegion( *GetPoint() );
    SwMoveFn fnMove = MakeFindRange( nStart, nEnde, &aRegion );

    ULONG nFound = 0;
    int bMvBkwrd = fnMove == fnMoveBackward;
    BOOL bInReadOnly = IsReadOnlyAvailable();

    SwCursor* pFndRing = 0;
    SwNodes& rNds = GetDoc()->GetNodes();

    // suche in Bereichen ?
    if( FND_IN_SEL & eFndRngs )
    {
        // String nicht im Bereich gefunden, dann erhalte alle Bereiche,
        // der Cursor beleibt unveraendert
        if( 0 == ( nFound = lcl_FindSelection( rParas, this, fnMove,
                                                pFndRing, aRegion, eFndRngs,
                                                bInReadOnly, bCancel ) ))
            return nFound;

        // der String wurde ein- bis mehrmals gefunden. Das steht alles
        // im neuen Crsr-Ring. Darum hebe erstmal den alten Ring auf
        while( GetNext() != this )
            delete GetNext();

        *GetPoint() = *pFndRing->GetPoint();
        SetMark();
        *GetMark() = *pFndRing->GetMark();
        pFndRing->MoveRingTo( this );
        delete pFndRing;
    }
    else if( FND_IN_OTHER & eFndRngs )
    {
        // Cursor als Kopie vom akt. und in den Ring aufnehmen
        // Verkettung zeigt immer auf den zuerst erzeugten, also vorwaerts
        SwCursor* pSav = Create( this );    // sicher den aktuellen Crsr

        // wenn schon ausserhalb vom Bodytext, suche von der Position,
        // ansonsten beginne mit der 1. GrundSection
        if( bMvBkwrd
            ? lcl_MakeSelBkwrd( rNds.GetEndOfExtras(),
                    *rNds.GetEndOfPostIts().StartOfSectionNode(),
                     *this, rNds.GetEndOfExtras().GetIndex() >=
                    GetPoint()->nNode.GetIndex() )
            : lcl_MakeSelFwrd( *rNds.GetEndOfPostIts().StartOfSectionNode(),
                    rNds.GetEndOfExtras(), *this,
                    rNds.GetEndOfExtras().GetIndex() >=
                    GetPoint()->nNode.GetIndex() ))
        {
            nFound = lcl_FindSelection( rParas, this, fnMove, pFndRing,
                                        aRegion, eFndRngs, bInReadOnly, bCancel );
        }

        if( !nFound )
        {
            // den alten wieder zurueck
            *GetPoint() = *pSav->GetPoint();
            if( pSav->HasMark() )
            {
                SetMark();
                *GetMark() = *pSav->GetMark();
            }
            else
                DeleteMark();
            delete pSav;
            return 0;
        }

        delete pSav;
        if( !( FND_IN_SELALL & eFndRngs ))
        {
            // es sollte nur einer gesucht werden, also fuege in dazu
            // egal in welche Richtung, SPoint ist immer groesser als Mark,
            // wenn der Suchbereich gueltig ist !!
            *GetPoint() = *pFndRing->GetPoint();
            SetMark();
            *GetMark() = *pFndRing->GetMark();
        }
        else
        {
            // es  wurde ein- bis mehrmals gefunden. Das steht alles
            // im neuen Crsr-Ring. Darum hebe erstmal den alten Ring auf
            while( GetNext() != this )
                delete GetNext();

            *GetPoint() = *pFndRing->GetPoint();
            SetMark();
            *GetMark() = *pFndRing->GetMark();
            pFndRing->MoveRingTo( this );
        }
        delete pFndRing;
    }
    else if( FND_IN_SELALL & eFndRngs )
    {
        SwCursor* pSav = Create( this );    // sicher den aktuellen Crsr

        const SwNode* pSttNd = ( FND_IN_BODYONLY & eFndRngs )
                            ? rNds.GetEndOfContent().StartOfSectionNode()
                            : rNds.GetEndOfPostIts().StartOfSectionNode();

        if( bMvBkwrd
            ? lcl_MakeSelBkwrd( rNds.GetEndOfContent(), *pSttNd,*this, FALSE )
            : lcl_MakeSelFwrd( *pSttNd, rNds.GetEndOfContent(), *this, FALSE ))
        {
            nFound = lcl_FindSelection( rParas, this, fnMove, pFndRing,
                                        aRegion, eFndRngs, bInReadOnly, bCancel );
        }

        if( !nFound )
        {
            // den alten wieder zurueck
            *GetPoint() = *pSav->GetPoint();
            if( pSav->HasMark() )
            {
                SetMark();
                *GetMark() = *pSav->GetMark();
            }
            else
                DeleteMark();
            delete pSav;
            return 0;
        }
        // es  wurde ein- bis mehrmals gefunden. Das steht alles
        // im neuen Crsr-Ring. Darum hebe erstmal den alten Ring auf
        delete pSav;

        while( GetNext() != this )
            delete GetNext();

        *GetPoint() = *pFndRing->GetPoint();
        SetMark();
        *GetMark() = *pFndRing->GetMark();
        pFndRing->MoveRingTo( this );
        delete pFndRing;
    }
    else
    {
        // ist ein GetMark gesetzt, dann wird bei gefundenem Object
        // der GetMark beibehalten !! Dadurch kann ein Bereich mit der Suche
        // aufgespannt werden.
        SwPosition aMarkPos( *GetMark() );
        int bMarkPos = HasMark() && !eFndRngs;

        if( 0 != (nFound = rParas.Find( this, fnMove,
                                        &aRegion, bInReadOnly ) ? 1 : 0)
            && bMarkPos )
            *GetMark() = aMarkPos;
    }

    if( nFound && SwCursor::IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE ) )
        nFound = 0;
    return nFound;
}


void SwCursor::FillFindPos( SwDocPositions ePos, SwPosition& rPos ) const
{
    BOOL bIsStart = TRUE;
    SwCntntNode* pCNd = 0;
    SwNodes& rNds = GetDoc()->GetNodes();

    switch( ePos )
    {
    case DOCPOS_START:
        rPos.nNode = *rNds.GetEndOfContent().StartOfSectionNode();
        pCNd = rNds.GoNext( &rPos.nNode );
        break;

    case DOCPOS_END:
        rPos.nNode = rNds.GetEndOfContent();
        pCNd = rNds.GoPrevious( &rPos.nNode );
        bIsStart = FALSE;
        break;

    case DOCPOS_OTHERSTART:
        rPos.nNode = *rNds[ ULONG(0) ];
        pCNd = rNds.GoNext( &rPos.nNode );
        break;

    case DOCPOS_OTHEREND:
        rPos.nNode = *rNds.GetEndOfContent().StartOfSectionNode();
        pCNd = rNds.GoPrevious( &rPos.nNode );
        bIsStart = FALSE;
        break;

//  case DOCPOS_CURR:
    default:
        rPos = *GetPoint();
    }

    if( pCNd )
    {
        xub_StrLen nCPos = 0;
        if( !bIsStart )
            nCPos = pCNd->Len();
        rPos.nContent.Assign( pCNd, nCPos );
    }
}

short SwCursor::MaxReplaceArived()
{
    return RET_YES;
}


BOOL SwCursor::IsStartWord( sal_Int16 nWordType ) const
{
    return IsStartWordWT( nWordType );
}

BOOL SwCursor::IsEndWord( sal_Int16 nWordType ) const
{
    return IsEndWordWT( nWordType );
}

BOOL SwCursor::IsInWord( sal_Int16 nWordType ) const
{
    return IsInWordWT( nWordType );
}

BOOL SwCursor::GoStartWord()
{
    return GoStartWordWT( WordType::ANYWORD_IGNOREWHITESPACES );
}

BOOL SwCursor::GoEndWord()
{
    return GoEndWordWT( WordType::ANYWORD_IGNOREWHITESPACES );
}

BOOL SwCursor::GoNextWord()
{
    return GoNextWordWT( WordType::ANYWORD_IGNOREWHITESPACES );
}

BOOL SwCursor::GoPrevWord()
{
    return GoPrevWordWT( WordType::ANYWORD_IGNOREWHITESPACES );
}

BOOL SwCursor::SelectWord( const Point* pPt )
{
    return SelectWordWT( WordType::ANYWORD_IGNOREWHITESPACES, pPt );
}

BOOL SwCursor::IsStartWordWT( sal_Int16 nWordType ) const
{
    BOOL bRet = FALSE;
    const SwTxtNode* pTxtNd = GetNode()->GetTxtNode();
    if( pTxtNd && pBreakIt->GetBreakIter().is() )
    {
        xub_StrLen nPtPos = GetPoint()->nContent.GetIndex();
        bRet = pBreakIt->GetBreakIter()->isBeginWord(
                            pTxtNd->GetTxt(), nPtPos,
                            pBreakIt->GetLocale( pTxtNd->GetLang( nPtPos )),
                            nWordType );
    }
    return bRet;
}

BOOL SwCursor::IsEndWordWT( sal_Int16 nWordType ) const
{
    BOOL bRet = FALSE;
    const SwTxtNode* pTxtNd = GetNode()->GetTxtNode();
    if( pTxtNd && pBreakIt->GetBreakIter().is() )
    {
        xub_StrLen nPtPos = GetPoint()->nContent.GetIndex();
        bRet = pBreakIt->GetBreakIter()->isEndWord(
                            pTxtNd->GetTxt(), nPtPos,
                            pBreakIt->GetLocale( pTxtNd->GetLang( nPtPos ) ),
                            nWordType );

    }
    return bRet;
}

BOOL SwCursor::IsInWordWT( sal_Int16 nWordType ) const
{
    BOOL bRet = FALSE;
    const SwTxtNode* pTxtNd = GetNode()->GetTxtNode();
    if( pTxtNd && pBreakIt->GetBreakIter().is() )
    {
        xub_StrLen nPtPos = GetPoint()->nContent.GetIndex();
        Boundary aBoundary = pBreakIt->GetBreakIter()->getWordBoundary(
                            pTxtNd->GetTxt(), nPtPos,
                            pBreakIt->GetLocale( pTxtNd->GetLang( nPtPos ) ),
                            nWordType,
                            TRUE );

        bRet = aBoundary.startPos != aBoundary.endPos &&
                aBoundary.startPos <= nPtPos &&
                    nPtPos <= aBoundary.endPos;
        if(bRet)
        {
            const CharClass& rCC = GetAppCharClass();
            bRet = rCC.isLetterNumeric( pTxtNd->GetTxt(), static_cast<xub_StrLen>(aBoundary.startPos) );
        }
    }
    return bRet;
}

BOOL SwCursor::IsStartEndSentence( bool bEnd ) const
{
    BOOL bRet = bEnd ?
                    GetCntntNode() && GetPoint()->nContent == GetCntntNode()->Len() :
                    GetPoint()->nContent.GetIndex() == 0;

    if( !bRet )
    {
        SwCursor aCrsr(*GetPoint(), 0, false);
        SwPosition aOrigPos = *aCrsr.GetPoint();
        aCrsr.GoSentence( bEnd ? SwCursor::END_SENT : SwCursor::START_SENT );
        bRet = aOrigPos == *aCrsr.GetPoint();
    }

    return bRet;
}

BOOL SwCursor::GoStartWordWT( sal_Int16 nWordType )
{
    BOOL bRet = FALSE;
    const SwTxtNode* pTxtNd = GetNode()->GetTxtNode();
    if( pTxtNd && pBreakIt->GetBreakIter().is() )
    {
        SwCrsrSaveState aSave( *this );
        xub_StrLen nPtPos = GetPoint()->nContent.GetIndex();
        nPtPos = (xub_StrLen)pBreakIt->GetBreakIter()->getWordBoundary(
                            pTxtNd->GetTxt(), nPtPos,
                            pBreakIt->GetLocale( pTxtNd->GetLang( nPtPos ) ),
                            nWordType,
                            FALSE ).startPos;

        if( nPtPos < pTxtNd->GetTxt().Len() )
        {
            GetPoint()->nContent = nPtPos;
            if( !IsSelOvr() )
                bRet = TRUE;
        }
    }
    return bRet;
}

BOOL SwCursor::GoEndWordWT( sal_Int16 nWordType )
{
    BOOL bRet = FALSE;
    const SwTxtNode* pTxtNd = GetNode()->GetTxtNode();
    if( pTxtNd && pBreakIt->GetBreakIter().is() )
    {
        SwCrsrSaveState aSave( *this );
        xub_StrLen nPtPos = GetPoint()->nContent.GetIndex();
        nPtPos = (xub_StrLen)pBreakIt->GetBreakIter()->getWordBoundary(
                            pTxtNd->GetTxt(), nPtPos,
                            pBreakIt->GetLocale( pTxtNd->GetLang( nPtPos ) ),
                            nWordType,
                            TRUE ).endPos;

        if( nPtPos <= pTxtNd->GetTxt().Len() &&
            GetPoint()->nContent.GetIndex() != nPtPos )
        {
            GetPoint()->nContent = nPtPos;
            if( !IsSelOvr() )
                bRet = TRUE;
        }
    }
    return bRet;
}

BOOL SwCursor::GoNextWordWT( sal_Int16 nWordType )
{
    BOOL bRet = FALSE;
    const SwTxtNode* pTxtNd = GetNode()->GetTxtNode();
    if( pTxtNd && pBreakIt->GetBreakIter().is() )
    {
        SwCrsrSaveState aSave( *this );
        xub_StrLen nPtPos = GetPoint()->nContent.GetIndex();

        nPtPos = (xub_StrLen)pBreakIt->GetBreakIter()->nextWord(
                                pTxtNd->GetTxt(), nPtPos,
            pBreakIt->GetLocale( pTxtNd->GetLang( nPtPos, 1 ) ),
                    nWordType ).startPos;

        if( nPtPos < pTxtNd->GetTxt().Len() )
        {
            GetPoint()->nContent = nPtPos;
            if( !IsSelOvr() )
                bRet = TRUE;
        }
    }
    return bRet;
}

BOOL SwCursor::GoPrevWordWT( sal_Int16 nWordType )
{
    BOOL bRet = FALSE;
    const SwTxtNode* pTxtNd = GetNode()->GetTxtNode();
    if( pTxtNd && pBreakIt->GetBreakIter().is() )
    {
        SwCrsrSaveState aSave( *this );
        xub_StrLen nPtPos = GetPoint()->nContent.GetIndex();
        const xub_StrLen nPtStart = nPtPos;

        if( nPtPos )
            --nPtPos;
        nPtPos = (xub_StrLen)pBreakIt->GetBreakIter()->previousWord(
                                pTxtNd->GetTxt(), nPtStart,
            pBreakIt->GetLocale( pTxtNd->GetLang( nPtPos, 1 ) ),
                    nWordType ).startPos;

        if( nPtPos < pTxtNd->GetTxt().Len() )
        {
            GetPoint()->nContent = nPtPos;
            if( !IsSelOvr() )
                bRet = TRUE;
        }
    }
    return bRet;
}

BOOL SwCursor::SelectWordWT( sal_Int16 nWordType, const Point* pPt )
{
    SwCrsrSaveState aSave( *this );

    BOOL bRet = FALSE;
    BOOL bForward = TRUE;
    DeleteMark();
    SwRootFrm* pLayout;
    if( pPt && 0 != (pLayout = GetDoc()->GetRootFrm()) )
    {
        // set the cursor to the layout position
        Point aPt( *pPt );
        pLayout->GetCrsrOfst( GetPoint(), aPt );
    }

    const SwTxtNode* pTxtNd = GetNode()->GetTxtNode();
    if( pTxtNd && pBreakIt->GetBreakIter().is() )
    {
        // Should we select the whole fieldmark?
        const IDocumentMarkAccess* pMarksAccess = GetDoc()->getIDocumentMarkAccess( );
        sw::mark::IMark* pMark = GetPoint() ? pMarksAccess->getFieldmarkFor( *GetPoint( ) ) : NULL;
        if ( pMark )
        {
            const SwPosition rStart = pMark->GetMarkStart();
            GetPoint()->nNode = rStart.nNode;
            GetPoint()->nContent = rStart.nContent;
            GetPoint()->nContent++; // Don't select the start delimiter

            const SwPosition rEnd = pMark->GetMarkEnd();

            if ( rStart != rEnd )
            {
                SetMark();
                GetMark()->nNode = rEnd.nNode;
                GetMark()->nContent = rEnd.nContent;
                GetMark()->nContent--; //Don't select the end delimiter
            }
            bRet = TRUE;
        }
        else
        {
            xub_StrLen nPtPos = GetPoint()->nContent.GetIndex();
            Boundary aBndry( pBreakIt->GetBreakIter()->getWordBoundary(
                                pTxtNd->GetTxt(), nPtPos,
                                pBreakIt->GetLocale( pTxtNd->GetLang( nPtPos ) ),
                                nWordType,
                                bForward ));

            if( aBndry.startPos != aBndry.endPos )
            {
                GetPoint()->nContent = (xub_StrLen)aBndry.endPos;
                if( !IsSelOvr() )
                {
                    SetMark();
                    GetMark()->nContent = (xub_StrLen)aBndry.startPos;
                    if( !IsSelOvr() )
                        bRet = TRUE;
                }
            }
        }
    }

    if( !bRet )
    {
        DeleteMark();
        RestoreSavePos();
    }
    return bRet;
}

//-----------------------------------------------------------------------------

static String lcl_MaskDeletedRedlines( const SwTxtNode* pTxtNd )
{
    String aRes;
    if (pTxtNd)
    {
        //mask deleted redlines
        String sNodeText(pTxtNd->GetTxt());
        const SwDoc& rDoc = *pTxtNd->GetDoc();
        const bool nShowChg = IDocumentRedlineAccess::IsShowChanges( rDoc.GetRedlineMode() );
        if ( nShowChg )
        {
            USHORT nAct = rDoc.GetRedlinePos( *pTxtNd, USHRT_MAX );
            for ( ; nAct < rDoc.GetRedlineTbl().Count(); nAct++ )
            {
                const SwRedline* pRed = rDoc.GetRedlineTbl()[ nAct ];
                if ( pRed->Start()->nNode > pTxtNd->GetIndex() )
                    break;

                if( nsRedlineType_t::REDLINE_DELETE == pRed->GetType() )
                {
                    xub_StrLen nStart, nEnd;
                    pRed->CalcStartEnd( pTxtNd->GetIndex(), nStart, nEnd );

                    while ( nStart < nEnd && nStart < sNodeText.Len() )
                        sNodeText.SetChar( nStart++, CH_TXTATR_INWORD );
                }
            }
        }
        aRes = sNodeText;
    }
    return aRes;
}

BOOL SwCursor::GoSentence( SentenceMoveType eMoveType )
{
    BOOL bRet = FALSE;
    const SwTxtNode* pTxtNd = GetNode()->GetTxtNode();
    if( pTxtNd && pBreakIt->GetBreakIter().is() )
    {
        String sNodeText( lcl_MaskDeletedRedlines( pTxtNd ) );

        SwCrsrSaveState aSave( *this );
        xub_StrLen nPtPos = GetPoint()->nContent.GetIndex();
        switch ( eMoveType )
        {
        case START_SENT: /* when modifying: see also ExpandToSentenceBorders below! */
            nPtPos = (xub_StrLen)pBreakIt->GetBreakIter()->beginOfSentence(
                                    sNodeText,
                                    nPtPos, pBreakIt->GetLocale(
                                            pTxtNd->GetLang( nPtPos ) ));
            break;
        case END_SENT: /* when modifying: see also ExpandToSentenceBorders below! */
            nPtPos = (xub_StrLen)pBreakIt->GetBreakIter()->endOfSentence(
                                    sNodeText,
                                    nPtPos, pBreakIt->GetLocale(
                                                pTxtNd->GetLang( nPtPos ) ));
            break;
        case NEXT_SENT:
            {
                nPtPos = (xub_StrLen)pBreakIt->GetBreakIter()->endOfSentence(
                                        sNodeText,
                                        nPtPos, pBreakIt->GetLocale(
                                                    pTxtNd->GetLang( nPtPos ) ));
                while (nPtPos != (USHORT) -1 && ++nPtPos < sNodeText.Len()
                       && sNodeText.GetChar(nPtPos)== ' ' /*isWhiteSpace( aTxt.GetChar(nPtPos)*/ )
                    ;
                break;
            }
        case PREV_SENT:
            nPtPos = (xub_StrLen)pBreakIt->GetBreakIter()->beginOfSentence(
                                    sNodeText,
                                    nPtPos, pBreakIt->GetLocale(
                                                pTxtNd->GetLang( nPtPos ) ));
            if (nPtPos == 0)
                return FALSE;   // the previous sentence is not in this paragraph
            if (nPtPos > 0)
                nPtPos = (xub_StrLen)pBreakIt->GetBreakIter()->beginOfSentence(
                                    sNodeText,
                                    nPtPos - 1, pBreakIt->GetLocale(
                                                pTxtNd->GetLang( nPtPos ) ));
            break;
        }

        // it is allowed to place the PaM just behind the last
        // character in the text thus <= ...Len
        if( nPtPos <= pTxtNd->GetTxt().Len() )
        {
            GetPoint()->nContent = nPtPos;
            if( !IsSelOvr() )
                bRet = TRUE;
        }
    }
    return bRet;
}


BOOL SwCursor::ExpandToSentenceBorders()
{
    BOOL bRes = FALSE;
    const SwTxtNode* pStartNd = Start()->nNode.GetNode().GetTxtNode();
    const SwTxtNode* pEndNd   = End()->nNode.GetNode().GetTxtNode();
    if (pStartNd && pEndNd && pBreakIt->GetBreakIter().is())
    {
        if (!HasMark())
            SetMark();

        String sStartText( lcl_MaskDeletedRedlines( pStartNd ) );
        String sEndText( pStartNd == pEndNd? sStartText : lcl_MaskDeletedRedlines( pEndNd ) );

        SwCrsrSaveState aSave( *this );
        xub_StrLen nStartPos = Start()->nContent.GetIndex();
        xub_StrLen nEndPos   = End()->nContent.GetIndex();

        nStartPos = (xub_StrLen)pBreakIt->GetBreakIter()->beginOfSentence(
                                sStartText, nStartPos,
                                pBreakIt->GetLocale( pStartNd->GetLang( nStartPos ) ) );
        nEndPos   = (xub_StrLen)pBreakIt->GetBreakIter()->endOfSentence(
                                sEndText, nEndPos,
                                pBreakIt->GetLocale( pEndNd->GetLang( nEndPos ) ) );

        // it is allowed to place the PaM just behind the last
        // character in the text thus <= ...Len
        bool bChanged = false;
        if (nStartPos <= pStartNd->GetTxt().Len())
        {
            GetMark()->nContent = nStartPos;
            bChanged = true;
        }
        if (nEndPos <= pEndNd->GetTxt().Len())
        {
            GetPoint()->nContent = nEndPos;
            bChanged = true;
        }
        if (bChanged && !IsSelOvr())
            bRes = TRUE;
    }
    return bRes;
}


BOOL SwTableCursor::LeftRight( BOOL bLeft, USHORT nCnt, USHORT /*nMode*/,
    BOOL /*bVisualAllowed*/, BOOL /*bSkipHidden*/, BOOL /*bInsertCrsr*/ )
{
    return bLeft ? GoPrevCell( nCnt )
                 : GoNextCell( nCnt );
}


// calculate cursor bidi level: extracted from LeftRight()
const SwCntntFrm*
SwCursor::DoSetBidiLevelLeftRight(
    BOOL & io_rbLeft, BOOL bVisualAllowed, BOOL bInsertCrsr)
{
    // calculate cursor bidi level
    const SwCntntFrm* pSttFrm = NULL;
    SwNode& rNode = GetPoint()->nNode.GetNode();

    if( rNode.IsTxtNode() )
    {
        const SwTxtNode& rTNd = *rNode.GetTxtNode();
        SwIndex& rIdx = GetPoint()->nContent;
        xub_StrLen nPos = rIdx.GetIndex();

        const SvtCTLOptions& rCTLOptions = SW_MOD()->GetCTLOptions();
        if ( bVisualAllowed && rCTLOptions.IsCTLFontEnabled() &&
             SvtCTLOptions::MOVEMENT_VISUAL ==
             rCTLOptions.GetCTLCursorMovement() )
        {
            // for visual cursor travelling (used in bidi layout)
            // we first have to convert the logic to a visual position
            Point aPt;
            pSttFrm = rTNd.GetFrm( &aPt, GetPoint() );
            if( pSttFrm )
            {
                BYTE nCrsrLevel = GetCrsrBidiLevel();
                sal_Bool bForward = ! io_rbLeft;
                ((SwTxtFrm*)pSttFrm)->PrepareVisualMove( nPos, nCrsrLevel,
                                                         bForward, bInsertCrsr );
                rIdx = nPos;
                SetCrsrBidiLevel( nCrsrLevel );
                io_rbLeft = ! bForward;
            }
        }
        else
        {
            const SwScriptInfo* pSI = SwScriptInfo::GetScriptInfo( rTNd );
            if ( pSI )
            {
                const xub_StrLen nMoveOverPos = io_rbLeft ?
                                               ( nPos ? nPos - 1 : 0 ) :
                                                nPos;
                SetCrsrBidiLevel( pSI->DirType( nMoveOverPos ) );
            }
        }
    }
    return pSttFrm;
}

BOOL SwCursor::LeftRight( BOOL bLeft, USHORT nCnt, USHORT nMode,
                          BOOL bVisualAllowed,BOOL bSkipHidden, BOOL bInsertCrsr )
{
    // calculate cursor bidi level
    SwNode& rNode = GetPoint()->nNode.GetNode();
    const SwCntntFrm* pSttFrm = // may side-effect bLeft!
        DoSetBidiLevelLeftRight(bLeft, bVisualAllowed, bInsertCrsr);

    // kann der Cursor n-mal weiterverschoben werden ?
    SwCrsrSaveState aSave( *this );
    SwMoveFn fnMove = bLeft ? fnMoveBackward : fnMoveForward;

    SwGoInDoc fnGo;
    if ( bSkipHidden )
        fnGo = CRSR_SKIP_CELLS == nMode ? fnGoCntntCellsSkipHidden : fnGoCntntSkipHidden;
    else
        fnGo = CRSR_SKIP_CELLS == nMode ? fnGoCntntCells : fnGoCntnt;

    // OSL_ENSURE( not in covered cell )

    while( nCnt )
    {
        SwNodeIndex aOldNodeIdx( GetPoint()->nNode );

        bool bSuccess = Move( fnMove, fnGo );
        if ( !bSuccess )
            break;

        // If we were located inside a covered cell but our position has been
        // corrected, we check if the last move has moved the cursor to a different
        // table cell. In this case we set the cursor to the stored covered position
        // and redo the move:
        if ( mnRowSpanOffset )
        {
            const SwNode* pOldTabBoxSttNode = aOldNodeIdx.GetNode().FindTableBoxStartNode();
            const SwTableNode* pOldTabSttNode = pOldTabBoxSttNode ? pOldTabBoxSttNode->FindTableNode() : 0;
            const SwNode* pNewTabBoxSttNode = GetPoint()->nNode.GetNode().FindTableBoxStartNode();
            const SwTableNode* pNewTabSttNode = pNewTabBoxSttNode ? pNewTabBoxSttNode->FindTableNode() : 0;

            const bool bCellChanged = pOldTabSttNode && pNewTabSttNode &&
                                      pOldTabSttNode == pNewTabSttNode &&
                                      pOldTabBoxSttNode && pNewTabBoxSttNode &&
                                      pOldTabBoxSttNode != pNewTabBoxSttNode;

            if ( bCellChanged )
            {
                // Set cursor to start/end of covered cell:
                SwTableBox* pTableBox = pOldTabBoxSttNode->GetTblBox();
                const long nRowSpan = pTableBox->getRowSpan();
                if ( nRowSpan > 1 )
                {
                    pTableBox = & pTableBox->FindEndOfRowSpan( pOldTabSttNode->GetTable(), (USHORT)(pTableBox->getRowSpan() + mnRowSpanOffset ) );
                    SwNodeIndex& rPtIdx = GetPoint()->nNode;
                    SwNodeIndex aNewIdx( *pTableBox->GetSttNd() );
                    rPtIdx = aNewIdx;

                    GetDoc()->GetNodes().GoNextSection( &rPtIdx, FALSE, FALSE );
                    SwCntntNode* pCntntNode = GetCntntNode();
                    if ( pCntntNode )
                    {
                        const xub_StrLen nTmpPos = bLeft ? pCntntNode->Len() : 0;
                        GetPoint()->nContent.Assign( pCntntNode, nTmpPos );

                        // Redo the move:
                        bSuccess = Move( fnMove, fnGo );
                        if ( !bSuccess )
                            break;
                    }
                }

                mnRowSpanOffset = 0;
            }
        }

        // Check if I'm inside a covered cell. Correct cursor if necessary and
        // store covered cell:
        const SwNode* pTableBoxStartNode = GetPoint()->nNode.GetNode().FindTableBoxStartNode();
        if ( pTableBoxStartNode )
        {
            const SwTableBox* pTableBox = pTableBoxStartNode->GetTblBox();
            if ( pTableBox && pTableBox->getRowSpan() < 1 )
            {
                // Store the row span offset:
                mnRowSpanOffset = pTableBox->getRowSpan();

                // Move cursor to non-covered cell:
                const SwTableNode* pTblNd = pTableBoxStartNode->FindTableNode();
                pTableBox = & pTableBox->FindStartOfRowSpan( pTblNd->GetTable(), USHRT_MAX );
                 SwNodeIndex& rPtIdx = GetPoint()->nNode;
                SwNodeIndex aNewIdx( *pTableBox->GetSttNd() );
                rPtIdx = aNewIdx;

                GetDoc()->GetNodes().GoNextSection( &rPtIdx, FALSE, FALSE );
                SwCntntNode* pCntntNode = GetCntntNode();
                if ( pCntntNode )
                {
                    const xub_StrLen nTmpPos = bLeft ? pCntntNode->Len() : 0;
                       GetPoint()->nContent.Assign( pCntntNode, nTmpPos );
                }
            }
        }

        --nCnt;
    }

    // here come some special rules for visual cursor travelling
    if ( pSttFrm )
    {
        SwNode& rTmpNode = GetPoint()->nNode.GetNode();
        if ( &rTmpNode != &rNode && rTmpNode.IsTxtNode() )
        {
            Point aPt;
            const SwCntntFrm* pEndFrm = ((SwTxtNode&)rTmpNode).GetFrm( &aPt, GetPoint() );
            if ( pEndFrm )
            {
                if ( ! pEndFrm->IsRightToLeft() != ! pSttFrm->IsRightToLeft() )
                {
                    if ( ! bLeft )
                        pEndFrm->RightMargin( this );
                    else
                        pEndFrm->LeftMargin( this );
                }
            }
        }
    }

    return 0 == nCnt && !IsInProtectTable( TRUE ) &&
            !IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                       nsSwCursorSelOverFlags::SELOVER_CHANGEPOS );
}

// calculate cursor bidi level: extracted from UpDown()
void SwCursor::DoSetBidiLevelUpDown()
{
    SwNode& rNode = GetPoint()->nNode.GetNode();
    if ( rNode.IsTxtNode() )
    {
        const SwScriptInfo* pSI =
            SwScriptInfo::GetScriptInfo( (SwTxtNode&)rNode );
        if ( pSI )
        {
            SwIndex& rIdx = GetPoint()->nContent;
            xub_StrLen nPos = rIdx.GetIndex();

            if( nPos && nPos < ((SwTxtNode&)rNode).GetTxt().Len() )
            {
                const BYTE nCurrLevel = pSI->DirType( nPos );
                const BYTE nPrevLevel = pSI->DirType( nPos - 1 );

                if ( nCurrLevel % 2 != nPrevLevel % 2 )
                {
                    // set cursor level to the lower of the two levels
                    SetCrsrBidiLevel( Min( nCurrLevel, nPrevLevel ) );
                }
                else
                    SetCrsrBidiLevel( nCurrLevel );
            }
        }
    }
}

BOOL SwCursor::UpDown( BOOL bUp, USHORT nCnt,
                            Point* pPt, long nUpDownX )
{
    SwTableCursor* pTblCrsr = dynamic_cast<SwTableCursor*>(this);
    sal_Bool bAdjustTableCrsr = sal_False;

    // vom Tabellen Crsr Point/Mark in der gleichen Box ??
    // dann stelle den Point an den Anfang der Box
    if( pTblCrsr && GetNode( TRUE )->StartOfSectionNode() ==
                    GetNode( FALSE )->StartOfSectionNode() )
    {
        if ( End() != GetPoint() )
            Exchange();
        bAdjustTableCrsr = sal_True;
    }

    BOOL bRet = FALSE;
    Point aPt;
    if( pPt )
        aPt = *pPt;
    SwCntntFrm* pFrm = GetCntntNode()->GetFrm( &aPt, GetPoint() );

    if( pFrm )
    {
        SwCrsrSaveState aSave( *this );

        if( !pPt )
        {
            SwRect aTmpRect;
            pFrm->GetCharRect( aTmpRect, *GetPoint() );
            aPt = aTmpRect.Pos();

            nUpDownX = pFrm->IsVertical() ?
                aPt.Y() - pFrm->Frm().Top() :
                aPt.X() - pFrm->Frm().Left();
        }

        // Bei Fussnoten ist auch die Bewegung in eine andere Fussnote erlaubt.
        // aber keine Selection!!
        const BOOL bChkRange = pFrm->IsInFtn() && !HasMark()
                                    ? FALSE : TRUE;
        const SwPosition aOldPos( *GetPoint() );
        BOOL bInReadOnly = IsReadOnlyAvailable();

        if ( bAdjustTableCrsr && !bUp )
        {
            // Special case: We have a table cursor but the start box
            // has more than one paragraph. If we want to go down, we have to
            // set the point to the last frame in the table box. This is
            // only necessary if we do not already have a table selection
            const SwStartNode* pTblNd = GetNode( TRUE )->FindTableBoxStartNode();
            OSL_ENSURE( pTblNd, "pTblCrsr without SwTableNode?" );

            if ( pTblNd ) // safety first
            {
                const SwNode* pEndNd = pTblNd->EndOfSectionNode();
                GetPoint()->nNode = *pEndNd;
                pTblCrsr->Move( fnMoveBackward, fnGoNode );
                   pFrm = GetCntntNode()->GetFrm( &aPt, GetPoint() );
            }
        }

        while( nCnt &&
               (bUp ? pFrm->UnitUp( this, nUpDownX, bInReadOnly )
                    : pFrm->UnitDown( this, nUpDownX, bInReadOnly ) ) &&
                CheckNodesRange( aOldPos.nNode, GetPoint()->nNode, bChkRange ))
        {
               pFrm = GetCntntNode()->GetFrm( &aPt, GetPoint() );
            --nCnt;
        }

        if( !nCnt && !IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                                nsSwCursorSelOverFlags::SELOVER_CHANGEPOS ) )  // die gesamte Anzahl durchlaufen ?
        {
            if( !pTblCrsr )
            {
                // dann versuche den Cursor auf die Position zu setzen,
                // auf halber Heohe vom Char-Rectangle
                pFrm = GetCntntNode()->GetFrm( &aPt, GetPoint() );
                SwCrsrMoveState eTmpState( MV_UPDOWN );
                eTmpState.bSetInReadOnly = bInReadOnly;
                SwRect aTmpRect;
                pFrm->GetCharRect( aTmpRect, *GetPoint(), &eTmpState );
                if ( pFrm->IsVertical() )
                {
                    aPt.X() = aTmpRect.Center().X();
                    pFrm->Calc();
                    aPt.Y() = pFrm->Frm().Top() + nUpDownX;
                }
                else
                {
                    aPt.Y() = aTmpRect.Center().Y();
                    pFrm->Calc();
                    aPt.X() = pFrm->Frm().Left() + nUpDownX;
                }
                pFrm->GetCrsrOfst( GetPoint(), aPt, &eTmpState );
            }
            bRet = TRUE;
        }
        else
            *GetPoint() = aOldPos;

        DoSetBidiLevelUpDown(); // calculate cursor bidi level
    }

    return bRet;
}

BOOL SwCursor::LeftRightMargin( BOOL bLeft, BOOL bAPI )
{
    Point aPt;
    SwCntntFrm * pFrm = GetCntntNode()->GetFrm( &aPt, GetPoint() );

    // calculate cursor bidi level
    if ( pFrm )
        SetCrsrBidiLevel( pFrm->IsRightToLeft() ? 1 : 0 );

    return pFrm && (bLeft ? pFrm->LeftMargin( this ) :
                            pFrm->RightMargin( this, bAPI ) );
}

BOOL SwCursor::IsAtLeftRightMargin( BOOL bLeft, BOOL bAPI ) const
{
    BOOL bRet = FALSE;
    Point aPt;
    SwCntntFrm * pFrm = GetCntntNode()->GetFrm( &aPt, GetPoint() );
    if( pFrm )
    {
        SwPaM aPam( *GetPoint() );
        if( !bLeft && aPam.GetPoint()->nContent.GetIndex() )
            aPam.GetPoint()->nContent--;
        bRet = (bLeft ? pFrm->LeftMargin( &aPam )
                      : pFrm->RightMargin( &aPam, bAPI ))
                && *aPam.GetPoint() == *GetPoint();
    }
    return bRet;
}

BOOL SwCursor::SttEndDoc( BOOL bStt )
{
    SwCrsrSaveState aSave( *this );

    // Springe beim Selektieren nie ueber Section-Grenzen !!
    // kann der Cursor weiterverschoben werden ?
    SwMoveFn fnMove = bStt ? fnMoveBackward : fnMoveForward;
    BOOL bRet = (!HasMark() || !IsNoCntnt() ) &&
                    Move( fnMove, fnGoDoc ) &&
                    !IsInProtectTable( TRUE ) &&
                    !IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                               nsSwCursorSelOverFlags::SELOVER_CHANGEPOS |
                               nsSwCursorSelOverFlags::SELOVER_ENABLEREVDIREKTION );

    return bRet;
}

BOOL SwCursor::GoPrevNextCell( BOOL bNext, USHORT nCnt )
{
    const SwTableNode* pTblNd = GetPoint()->nNode.GetNode().FindTableNode();
    if( !pTblNd )
        return FALSE;

    // liegt vor dem StartNode der Cell ein weiterer EndNode, dann
    // gibt es auch eine vorherige Celle
    SwCrsrSaveState aSave( *this );
    SwNodeIndex& rPtIdx = GetPoint()->nNode;

    while( nCnt-- )
    {
        const SwNode* pTableBoxStartNode = rPtIdx.GetNode().FindTableBoxStartNode();
        const SwTableBox* pTableBox = pTableBoxStartNode->GetTblBox();

        // Check if we have to move the cursor to a covered cell before
        // proceeding:
        if ( mnRowSpanOffset )
        {
            if ( pTableBox->getRowSpan() > 1 )
            {
                pTableBox = & pTableBox->FindEndOfRowSpan( pTblNd->GetTable(), (USHORT)(pTableBox->getRowSpan() + mnRowSpanOffset) );
                SwNodeIndex aNewIdx( *pTableBox->GetSttNd() );
                rPtIdx = aNewIdx;
                pTableBoxStartNode = rPtIdx.GetNode().FindTableBoxStartNode();
            }
            mnRowSpanOffset = 0;
        }

        const SwNode* pTmpNode = bNext ?
                                 pTableBoxStartNode->EndOfSectionNode() :
                                 pTableBoxStartNode;

        SwNodeIndex aCellIdx( *pTmpNode, bNext ? 1 : -1 );
        if(  (bNext && !aCellIdx.GetNode().IsStartNode()) ||
            (!bNext && !aCellIdx.GetNode().IsEndNode()) )
            return FALSE;

        rPtIdx = bNext ? aCellIdx : SwNodeIndex(*aCellIdx.GetNode().StartOfSectionNode());

        pTableBoxStartNode = rPtIdx.GetNode().FindTableBoxStartNode();
        pTableBox = pTableBoxStartNode->GetTblBox();
        if ( pTableBox->getRowSpan() < 1 )
        {
            mnRowSpanOffset = pTableBox->getRowSpan();
            // move cursor to non-covered cell:
            pTableBox = & pTableBox->FindStartOfRowSpan( pTblNd->GetTable(), USHRT_MAX );
            SwNodeIndex aNewIdx( *pTableBox->GetSttNd() );
            rPtIdx = aNewIdx;
        }
    }

    rPtIdx++;
    if( !rPtIdx.GetNode().IsCntntNode() )
        GetDoc()->GetNodes().GoNextSection( &rPtIdx, TRUE, FALSE );
    GetPoint()->nContent.Assign( GetCntntNode(), 0 );

    return !IsInProtectTable( TRUE );
}

BOOL SwTableCursor::GotoTable( const String& /*rName*/ )
{
    return FALSE; // invalid action
}

BOOL SwCursor::GotoTable( const String& rName )
{
    BOOL bRet = FALSE;
    if ( !HasMark() )
    {
        SwTable* pTmpTbl = SwTable::FindTable( GetDoc()->FindTblFmtByName( rName ) );
        if( pTmpTbl )
        {
            // eine Tabelle im normalen NodesArr
            SwCrsrSaveState aSave( *this );
            GetPoint()->nNode = *pTmpTbl->GetTabSortBoxes()[ 0 ]->
                                GetSttNd()->FindTableNode();
            Move( fnMoveForward, fnGoCntnt );
            bRet = !IsSelOvr();
        }
    }
    return bRet;
}

BOOL SwCursor::GotoTblBox( const String& rName )
{
    BOOL bRet = FALSE;
    const SwTableNode* pTblNd = GetPoint()->nNode.GetNode().FindTableNode();
    if( pTblNd )
    {
        // erfrage die Box, mit dem Nanen
        const SwTableBox* pTblBox = pTblNd->GetTable().GetTblBox( rName );
        if( pTblBox && pTblBox->GetSttNd() &&
            ( !pTblBox->GetFrmFmt()->GetProtect().IsCntntProtected() ||
              IsReadOnlyAvailable() ) )
        {
            SwCrsrSaveState aSave( *this );
            GetPoint()->nNode = *pTblBox->GetSttNd();
            Move( fnMoveForward, fnGoCntnt );
            bRet = !IsSelOvr();
        }
    }
    return bRet;
}

BOOL SwCursor::MovePara(SwWhichPara fnWhichPara, SwPosPara fnPosPara )
{
    //for optimization test something before
    const SwNode* pNd = &GetPoint()->nNode.GetNode();
    bool bShortCut = false;
    if ( fnWhichPara == fnParaCurr )
    {
        // --> FME 2005-02-21 #i41048#
        // If fnWhichPara == fnParaCurr, (*fnWhichPara)( *this, fnPosPara )
        // can already move the cursor to a different text node. In this case
        // we better check if IsSelOvr().
        const SwCntntNode* pCntntNd = pNd->GetCntntNode();
        if ( pCntntNd )
        {
            const xub_StrLen nSttEnd = fnPosPara == fnMoveForward ? 0 : pCntntNd->Len();
            if ( GetPoint()->nContent.GetIndex() != nSttEnd )
                bShortCut = true;
        }
        // <--
    }
    else
    {
        if ( pNd->IsTxtNode() &&
             pNd->GetNodes()[ pNd->GetIndex() +
                    (fnWhichPara == fnParaNext ? 1 : -1 ) ]->IsTxtNode() )
            bShortCut = true;
    }

    if ( bShortCut )
        return (*fnWhichPara)( *this, fnPosPara );

    // else we must use the SaveStructure, because the next/prev is not
    // a same node type.
    SwCrsrSaveState aSave( *this );
    return (*fnWhichPara)( *this, fnPosPara ) &&
            !IsInProtectTable( TRUE ) &&
            !IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                       nsSwCursorSelOverFlags::SELOVER_CHANGEPOS );
}


BOOL SwCursor::MoveSection( SwWhichSection fnWhichSect,
                                SwPosSection fnPosSect)
{
    SwCrsrSaveState aSave( *this );
    return (*fnWhichSect)( *this, fnPosSect ) &&
            !IsInProtectTable( TRUE ) &&
            !IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                       nsSwCursorSelOverFlags::SELOVER_CHANGEPOS );
}

void SwCursor::RestoreSavePos()
{
    if( pSavePos )
    {
        GetPoint()->nNode = pSavePos->nNode;
        GetPoint()->nContent.Assign( GetCntntNode(), pSavePos->nCntnt );
    }
}


/*  */

SwTableCursor::SwTableCursor( const SwPosition &rPos, SwPaM* pRing )
    : SwCursor( rPos, pRing, false )
{
    bParked = FALSE;
    bChg = FALSE;
    nTblPtNd = 0, nTblMkNd = 0;
    nTblPtCnt = 0, nTblMkCnt = 0;
}

SwTableCursor::~SwTableCursor() {}


BOOL lcl_SeekEntry( const SwSelBoxes& rTmp, const SwStartNode* pSrch, USHORT& rFndPos )
{
    ULONG nIdx = pSrch->GetIndex();

    USHORT nO = rTmp.Count();
    if( nO > 0 )
    {
        nO--;
        USHORT nU = 0;
        while( nU <= nO )
        {
            USHORT nM = nU + ( nO - nU ) / 2;
            if( rTmp[ nM ]->GetSttNd() == pSrch )
            {
                rFndPos = nM;
                return TRUE;
            }
            else if( rTmp[ nM ]->GetSttIdx() < nIdx )
                nU = nM + 1;
            else if( nM == 0 )
                return FALSE;
            else
                nO = nM - 1;
        }
    }
    return FALSE;
}


SwCursor* SwTableCursor::MakeBoxSels( SwCursor* pAktCrsr )
{
    if( bChg )      // ???
    {
        if( bParked )
        {
            // wieder in den Inhalt schieben
            Exchange();
            Move( fnMoveForward );
            Exchange();
            Move( fnMoveForward );
            bParked = FALSE;
        }

        bChg = FALSE;

        // temp Kopie anlegen, damit alle Boxen, fuer die schon Cursor
        // existieren, entfernt werden koennen.
        SwSelBoxes aTmp;
        aTmp.Insert( &aSelBoxes );

        //Jetzt die Alten und die neuen abgleichen.
        SwNodes& rNds = pAktCrsr->GetDoc()->GetNodes();
        USHORT nPos;
        const SwStartNode* pSttNd;
        SwPaM* pCur = pAktCrsr;
        do {
            BOOL bDel = FALSE;
            pSttNd = pCur->GetPoint()->nNode.GetNode().FindTableBoxStartNode();
            if( !pCur->HasMark() || !pSttNd ||
                pSttNd != pCur->GetMark()->nNode.GetNode().FindTableBoxStartNode() )
                bDel = TRUE;

            else if( lcl_SeekEntry( aTmp, pSttNd, nPos ))
            {
                SwNodeIndex aIdx( *pSttNd, 1 );
                const SwNode* pNd = &aIdx.GetNode();
                if( !pNd->IsCntntNode() )
                    pNd = rNds.GoNextSection( &aIdx, TRUE, FALSE );

                SwPosition* pPos = pCur->GetMark();
                if( pNd != &pPos->nNode.GetNode() )
                    pPos->nNode = *pNd;
                pPos->nContent.Assign( (SwCntntNode*)pNd, 0 );

                aIdx.Assign( *pSttNd->EndOfSectionNode(), - 1 );
                if( !( pNd = &aIdx.GetNode())->IsCntntNode() )
                    pNd = rNds.GoPrevSection( &aIdx, TRUE, FALSE );

                pPos = pCur->GetPoint();
                if( pNd != &pPos->nNode.GetNode() )
                    pPos->nNode = *pNd;
                pPos->nContent.Assign( (SwCntntNode*)pNd, ((SwCntntNode*)pNd)->Len() );

                aTmp.Remove( nPos );
            }
            else
                bDel = TRUE;

            pCur = (SwPaM*)pCur->GetNext();
            if( bDel )
            {
                SwPaM* pDel = (SwPaM*)pCur->GetPrev();

                if( pDel == pAktCrsr )
                    pAktCrsr->DeleteMark();
                else
                    delete pDel;
            }
        } while ( pAktCrsr != pCur );

        for( nPos = 0; nPos < aTmp.Count(); ++nPos )
        {
            pSttNd = aTmp[ nPos ]->GetSttNd();

            SwNodeIndex aIdx( *pSttNd, 1 );
            if( &aIdx.GetNodes() != &rNds )
                break;
            const SwNode* pNd = &aIdx.GetNode();
            if( !pNd->IsCntntNode() )
                pNd = rNds.GoNextSection( &aIdx, TRUE, FALSE );

            SwPaM* pNew;
            if( pAktCrsr->GetNext() == pAktCrsr && !pAktCrsr->HasMark() )
            {
                pNew = pAktCrsr;
                pNew->GetPoint()->nNode = *pNd;
                pNew->GetPoint()->nContent.Assign( (SwCntntNode*)pNd, 0 );
            }
            else
            {
                pNew = pAktCrsr->Create( pAktCrsr );
                pNew->GetPoint()->nNode = *pNd;
                pNew->GetPoint()->nContent.Assign( (SwCntntNode*)pNd, 0 );
            }
            pNew->SetMark();

            SwPosition* pPos = pNew->GetPoint();
            pPos->nNode.Assign( *pSttNd->EndOfSectionNode(), - 1 );
            if( !( pNd = &pPos->nNode.GetNode())->IsCntntNode() )
                pNd = rNds.GoPrevSection( &pPos->nNode, TRUE, FALSE );

            pPos->nContent.Assign( (SwCntntNode*)pNd, ((SwCntntNode*)pNd)->Len() );
        }
    }
    return pAktCrsr;
}


void SwTableCursor::InsertBox( const SwTableBox& rTblBox )
{
    SwTableBox* pBox = (SwTableBox*)&rTblBox;
    aSelBoxes.Insert( pBox );
    bChg = TRUE;
}

bool SwTableCursor::NewTableSelection()
{
    bool bRet = false;
    const SwNode *pStart = GetCntntNode()->FindTableBoxStartNode();
    const SwNode *pEnd = GetCntntNode(FALSE)->FindTableBoxStartNode();
    if( pStart && pEnd )
    {
        const SwTableNode *pTableNode = pStart->FindTableNode();
        if( pTableNode == pEnd->FindTableNode() &&
            pTableNode->GetTable().IsNewModel() )
        {
            bRet = true;
            SwSelBoxes aNew;
            aNew.Insert( &aSelBoxes );
            pTableNode->GetTable().CreateSelection( pStart, pEnd, aNew,
                SwTable::SEARCH_NONE, false );
            ActualizeSelection( aNew );
        }
    }
    return bRet;
}

void SwTableCursor::ActualizeSelection( const SwSelBoxes &rNew )
{
    USHORT nOld = 0, nNew = 0;
    while ( nOld < aSelBoxes.Count() && nNew < rNew.Count() )
    {
        const SwTableBox* pPOld = *( aSelBoxes.GetData() + nOld );
        const SwTableBox* pPNew = *( rNew.GetData() + nNew );
        if( pPOld == pPNew )
        {   // this box will stay
            ++nOld;
            ++nNew;
        }
        else if( pPOld->GetSttIdx() < pPNew->GetSttIdx() )
            DeleteBox( nOld ); // this box has to go
        else
        {
            InsertBox( *pPNew ); // this is a new one
            ++nOld;
            ++nNew;
        }
    }

    while( nOld < aSelBoxes.Count() )
        DeleteBox( nOld ); // some more to delete

    for( ; nNew < rNew.Count(); ++nNew ) // some more to insert
        InsertBox( **( rNew.GetData() + nNew ) );
}

BOOL SwTableCursor::IsCrsrMovedUpdt()
{
    if( !IsCrsrMoved() )
        return FALSE;

    nTblMkNd = GetMark()->nNode.GetIndex();
    nTblPtNd = GetPoint()->nNode.GetIndex();
    nTblMkCnt = GetMark()->nContent.GetIndex();
    nTblPtCnt = GetPoint()->nContent.GetIndex();
    return TRUE;
}


// Parke den Tabellen-Cursor auf dem StartNode der Boxen.
void SwTableCursor::ParkCrsr()
{
    // Index aus dem TextNode abmelden
    SwNode* pNd = &GetPoint()->nNode.GetNode();
    if( !pNd->IsStartNode() )
        pNd = pNd->StartOfSectionNode();
    GetPoint()->nNode = *pNd;
    GetPoint()->nContent.Assign( 0, 0 );

    pNd = &GetMark()->nNode.GetNode();
    if( !pNd->IsStartNode() )
        pNd = pNd->StartOfSectionNode();
    GetMark()->nNode = *pNd;
    GetMark()->nContent.Assign( 0, 0 );

    bChg = TRUE;
    bParked = TRUE;
}


BOOL SwTableCursor::HasReadOnlyBoxSel() const
{
    BOOL bRet = FALSE;
    for( USHORT n = aSelBoxes.Count(); n;  )
        if( aSelBoxes[ --n ]->GetFrmFmt()->GetProtect().IsCntntProtected() )
        {
            bRet = TRUE;
            break;
        }
    return bRet;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
