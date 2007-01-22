/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: trvltbl.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: obo $ $Date: 2007-01-22 11:52:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif

#ifndef _CRSRSH_HXX
#include <crsrsh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _EDITSH_HXX
#include <editsh.hxx>       //EndAllAction gibts nur an der EditShell
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _VISCRS_HXX
#include <viscrs.hxx>
#endif
#ifndef _CALLNK_HXX
#include <callnk.hxx>
#endif
#ifndef _TABFRM_HXX
#include <tabfrm.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _SHELLRES_HXX
#include <shellres.hxx>
#endif
#ifndef _CELLATR_HXX
#include <cellatr.hxx>
#endif
#ifndef _CELLFRM_HXX //autogen
#include <cellfrm.hxx>
#endif
#ifndef _ROWFRM_HXX
#include <rowfrm.hxx>
#endif

// setze Crsr in die naechsten/vorherigen Celle
FASTBOOL SwCrsrShell::GoNextCell( BOOL bAppendLine )
{
    FASTBOOL bRet = FALSE;
    const SwTableNode* pTblNd;
    if( IsTableMode() || 0 != ( pTblNd = IsCrsrInTbl() ))
    {
        SwCursor* pCrsr = pTblCrsr ? pTblCrsr : pCurCrsr;
        SwCallLink aLk( *this );        // Crsr-Moves ueberwachen,
        bRet = TRUE;

        // folgt nach dem EndNode der Cell ein weiterer StartNode, dann
        // gibt es auch eine naechste Celle
        SwNodeIndex aCellStt( *pCrsr->GetNode()->FindTableBoxStartNode()->
                                EndOfSectionNode(), 1 );
        if( !aCellStt.GetNode().IsStartNode() )
        {
            if( pCrsr->HasMark() ||
                (!bAppendLine /*&& IsCrsrReadonly()*/ ))
                bRet = FALSE;
            else
            {
                // auf besonderen Wunsch: keine Line mehr vorhanden, dann
                // mache doch eine neue:
                const SwTableBox* pBox = pTblNd->GetTable().GetTblBox(
                                    pCrsr->GetPoint()->nNode.GetNode().
                                    StartOfSectionIndex() );
                ASSERT( pBox, "Box steht nicht in dieser Tabelle" );
                SwSelBoxes aBoxes;

                //Das Dokument veraendert sich evtl. ohne Action wuerden die Sichten
                //nichts mitbekommen.
                ((SwEditShell*)this)->StartAllAction();
                bRet = pDoc->InsertRow( pTblNd->GetTable().
                                    SelLineFromBox( pBox, aBoxes, FALSE ));
                ((SwEditShell*)this)->EndAllAction();
            }
        }
        if( bRet && 0 != ( bRet = pCrsr->GoNextCell() ))
            UpdateCrsr();                 // und den akt. Updaten
    }
    return bRet;
}


FASTBOOL SwCrsrShell::GoPrevCell()
{
    FASTBOOL bRet = FALSE;
    const SwTableNode* pTblNd;
    if( IsTableMode() || 0 != ( pTblNd = IsCrsrInTbl() ))
    {
        SwCursor* pCrsr = pTblCrsr ? pTblCrsr : pCurCrsr;
        SwCallLink aLk( *this );        // Crsr-Moves ueberwachen,
        bRet = pCrsr->GoPrevCell();
        if( bRet )
            UpdateCrsr();                 // und den akt. Updaten
    }
    return bRet;
}


FASTBOOL SwCrsrShell::GotoTblBox( const String& rName )
{
    SwShellCrsr* pCrsr = pTblCrsr ? pTblCrsr : pCurCrsr;
    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen,

    FASTBOOL bRet = pCrsr->GotoTblBox( rName );
    if( bRet )
    {
        //JP 28.10.97: Bug 45028 - die "oberste" Position setzen fuer
        //              wiederholte Kopfzeilen
        pCrsr->GetPtPos() = Point();
        UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                    SwCrsrShell::READONLY ); // und den akt. Updaten
    }
    return bRet;
}

const SwFrm* lcl_FindMostUpperCellFrm( const SwFrm* pFrm )
{
    while ( pFrm &&
            ( !pFrm->IsCellFrm() ||
              !pFrm->GetUpper()->GetUpper()->IsTabFrm() ||
               pFrm->GetUpper()->GetUpper()->GetUpper()->IsInTab() ) )
    {
        pFrm = pFrm->GetUpper();
    }
    return pFrm;
}

FASTBOOL SwCrsrShell::_SelTblRowOrCol( bool bRow, bool bRowSimple )
{
    // pruefe ob vom aktuellen Crsr der SPoint/Mark in einer Tabelle stehen
    SwFrm *pFrm = GetCurrFrm();
    if( !pFrm->IsInTab() )
        return FALSE;

    SET_CURR_SHELL( this );

    const SwTableBox* pStt = 0;
    const SwTableBox* pEnd = 0;

    // lasse ueber das Layout die Boxen suchen
    SwSelBoxes aBoxes;
    SwTblSearchType eType = bRow ? TBLSEARCH_ROW : TBLSEARCH_COL;

    if( !IsReadOnlyAvailable() )
        eType = (SwTblSearchType)(eType | TBLSEARCH_PROTECT);

    if ( !bRowSimple )
    {
        GetTblSel( *this, aBoxes, eType );

        if( !aBoxes.Count() )
            return FALSE;

        pStt = aBoxes[0];
        pEnd = aBoxes[aBoxes.Count() - 1];
    }
    // --> FME 2004-07-30 #i32329# Enhanced table selection
    else
    {
        const SwShellCrsr *pCrsr = _GetCrsr();
        const SwFrm* pStartFrm = pFrm;
        const SwFrm* pEndFrm   = pCrsr->GetCntntNode( FALSE )->GetFrm( &pCrsr->GetMkPos() );

        if ( bRow )
        {
            pStartFrm = lcl_FindMostUpperCellFrm( pStartFrm );
            pEndFrm   = lcl_FindMostUpperCellFrm( pEndFrm   );
        }

        if ( !pStartFrm || !pEndFrm )
            return FALSE;

        const bool bVert = pFrm->ImplFindTabFrm()->IsVertical();

        // If we select upwards it is sufficient to set pStt and pEnd
        // to the first resp. last box of the selection obtained from
        // GetTblSel. However, selecting downwards requires the frames
        // located at the corners of the selection. This does not work
        // for column selections in vertical tables:
        const bool bSelectUp = ( bVert && !bRow ) ||
                                *pCrsr->GetPoint() <= *pCrsr->GetMark();
        SwCellFrms aCells;
        GetTblSel( static_cast<const SwCellFrm*>(pStartFrm),
                   static_cast<const SwCellFrm*>(pEndFrm),
                   aBoxes, bSelectUp ? 0 : &aCells, eType );

        if( !aBoxes.Count() || ( !bSelectUp && 4 != aCells.Count() ) )
            return FALSE;

        if ( bSelectUp )
        {
            pStt = aBoxes[0];
            pEnd = aBoxes[aBoxes.Count() - 1];
        }
        else
        {
            pStt = aCells[ bVert ? (bRow ? 0 : 3) : (bRow ? 2 : 1) ]->GetTabBox();  // will become point of table cursor
            pEnd = aCells[ bVert ? (bRow ? 3 : 0) : (bRow ? 1 : 2) ]->GetTabBox();  // will become mark of table cursor
        }
    }
    // <--

    // noch kein Tabellen-Cursor vorhanden, dann erzeuge einen
    if( !pTblCrsr )
    {
        pTblCrsr = new SwShellTableCrsr( *this, *pCurCrsr->GetPoint() );
        pCurCrsr->DeleteMark();
        pCurCrsr->SwSelPaintRects::Hide();
    }

    pTblCrsr->DeleteMark();

    // dann setze mal Anfang und Ende der Spalte
    pTblCrsr->GetPoint()->nNode = *pEnd->GetSttNd();
    pTblCrsr->Move( fnMoveForward, fnGoCntnt );
    pTblCrsr->SetMark();
    pTblCrsr->GetPoint()->nNode = *pStt->GetSttNd()->EndOfSectionNode();
    pTblCrsr->Move( fnMoveBackward, fnGoCntnt );

    UpdateCrsr();                 // und den akt. Updaten
    return TRUE;
}

FASTBOOL SwCrsrShell::SelTbl()
{
    // pruefe ob vom aktuellen Crsr der SPoint/Mark in einer Tabelle stehen
    SwFrm *pFrm = GetCurrFrm();
    if( !pFrm->IsInTab() )
        return FALSE;

    SwTabFrm *pTblFrm = pFrm->ImplFindTabFrm();
    SwTableNode* pTblNd = pTblFrm->GetTable()->GetTableNode();

    SET_CURR_SHELL( this );

    if( !pTblCrsr )
    {
        pTblCrsr = new SwShellTableCrsr( *this, *pCurCrsr->GetPoint() );
        pCurCrsr->DeleteMark();
        pCurCrsr->SwSelPaintRects::Hide();
    }

    pTblCrsr->DeleteMark();
    pTblCrsr->GetPoint()->nNode = *pTblNd;
    pTblCrsr->Move( fnMoveForward, fnGoCntnt );
    pTblCrsr->SetMark();
    pTblCrsr->GetPoint()->nNode = *pTblNd->EndOfSectionNode();
    pTblCrsr->Move( fnMoveBackward, fnGoCntnt );
    UpdateCrsr();                 // und den akt. Updaten
    return TRUE;
}


FASTBOOL SwCrsrShell::SelTblBox()
{
    // if we're in a table, create a table cursor, and select the cell
    // that the current cursor's point resides in

    // search for start node of our table box. If not found, exit realy
    const SwStartNode* pStartNode =
        pCurCrsr->GetPoint()->nNode.GetNode().FindTableBoxStartNode();

#ifndef PRODUCT
    // the old code checks whether we're in a table by asking the
    // frame. This should yield the same result as searching for the
    // table box start node, right?
    SwFrm *pFrm = GetCurrFrm();
    DBG_ASSERT( !pFrm->IsInTab() == !(pStartNode != NULL),
                "Schroedinger's table: We're in a box, and also we aren't." )
#endif

    if( pStartNode == NULL )
        return FALSE;


    SET_CURR_SHELL( this );

    // create a table cursor, if there isn't one already
    if( !pTblCrsr )
    {
        pTblCrsr = new SwShellTableCrsr( *this, *pCurCrsr->GetPoint() );
        pCurCrsr->DeleteMark();
        pCurCrsr->SwSelPaintRects::Hide();
    }

    // select the complete box with our shiny new pTblCrsr
    // 1. delete mark, and move point to first content node in box
    // 2. set mark, and move point to last content node in box
    // 3. exchange

    pTblCrsr->DeleteMark();
    *(pTblCrsr->GetPoint()) = SwPosition( *pStartNode );
    pTblCrsr->Move( fnMoveForward, fnGoNode );

    pTblCrsr->SetMark();
    *(pTblCrsr->GetPoint()) = SwPosition( *(pStartNode->EndOfSectionNode()) );
    pTblCrsr->Move( fnMoveBackward, fnGoNode );

    pTblCrsr->Exchange();

    // with some luck, UpdateCrsr() will now update everything that
    // needs updateing
    UpdateCrsr();

    return TRUE;
}


// suche die naechste nicht geschuetzte Zelle innerhalb der Tabelle
// Parameter:
//      rIdx    - steht auf dem TabellenNode
//      SwDoc   -
//  return:
//      0   - Idx steht auf/in einer nicht geschuetzten Zelle
//      !0  - Node hinter der Tabelle


SwNode* lcl_FindNextCell( SwNodeIndex& rIdx, FASTBOOL bInReadOnly )
{
    // ueberpruefe geschuetzte Zellen
    SwCntntFrm* pFrm;
    SwNodeIndex aTmp( rIdx, 2 );        // TableNode + StartNode
    SwNodes& rNds = aTmp.GetNode().GetNodes();
    SwCntntNode* pCNd = aTmp.GetNode().GetCntntNode();
    if( !pCNd )
        pCNd = rNds.GoNext( &aTmp );

    if( 0 == ( pFrm = pCNd->GetFrm()) ||
        (!bInReadOnly && pFrm->IsProtected() ))
    {
        aTmp.Assign( *pCNd->EndOfSectionNode(), 1 );
        SwNode* pNd;
        for( ;; )
        {
            if( !( pNd = &aTmp.GetNode())->IsStartNode() )
                return pNd;
            aTmp++;
            if( 0 == (pCNd = aTmp.GetNode().GetCntntNode()) )
                pCNd = rNds.GoNext( &aTmp );

            if( 0 != ( pFrm = pCNd->GetFrm()) &&
                (bInReadOnly || !pFrm->IsProtected() ))
            {
                rIdx = *pNd;
                return 0;       // Ok, nicht geschuetzt
            }
            aTmp.Assign( *pCNd->EndOfSectionNode(), +1 );
        }
    }
    return 0;
}

// suche die vorherige nicht geschuetzte Zelle innerhalb der Tabelle
// Parameter:
//      rIdx    - steht auf dem EndNode der Tabelle
//      SwDoc   -
//  return:
//      0   - Idx steht auf/in einer nicht geschuetzten Zelle
//      !0  - Node hinter der Tabelle


SwNode* lcl_FindPrevCell( SwNodeIndex& rIdx, FASTBOOL bInReadOnly  )
{
    // ueberpruefe geschuetzte Zellen
    SwCntntFrm* pFrm;
    SwNodeIndex aTmp( rIdx, -2 );       // TableNode + EndNode
    SwNodes& rNds = aTmp.GetNode().GetNodes();
    SwCntntNode* pCNd = aTmp.GetNode().GetCntntNode();
    if( !pCNd )
        pCNd = rNds.GoPrevious( &aTmp );

    if( 0 == ( pFrm = pCNd->GetFrm()) ||
        (!bInReadOnly && pFrm->IsProtected() ))
    {
        aTmp.Assign( *pCNd->StartOfSectionNode(), -1 );
        SwNode* pNd;
        for( ;; )
        {
            if( !( pNd = &aTmp.GetNode())->IsEndNode() )
                return pNd;
            aTmp--;
            if( 0 == (pCNd = aTmp.GetNode().GetCntntNode()) )
                pCNd = rNds.GoPrevious( &aTmp );

            if( 0 != ( pFrm = pCNd->GetFrm()) &&
                (bInReadOnly || !pFrm->IsProtected() ))
            {
                rIdx = *pNd;
                return 0;       // Ok, nicht geschuetzt
            }
            aTmp.Assign( *pCNd->StartOfSectionNode(), - 1 );
        }
    }
    return 0;
}



FASTBOOL GotoPrevTable( SwPaM& rCurCrsr, SwPosTable fnPosTbl,
                        FASTBOOL bInReadOnly )
{
    SwNodeIndex aIdx( rCurCrsr.GetPoint()->nNode );

    SwTableNode* pTblNd = aIdx.GetNode().FindTableNode();
    if( pTblNd )
    {
        // #i26532#: If we are inside a table, we may not go backward
        // to the table start node, because we would miss any tables
        // inside this table.
        SwTableNode* pInnerTblNd = 0;
        SwNodeIndex aTmpIdx( aIdx );
        while( aTmpIdx.GetIndex() &&
                0 == ( pInnerTblNd = aTmpIdx.GetNode().StartOfSectionNode()->GetTableNode()) )
            aTmpIdx--;

        if( pInnerTblNd == pTblNd )
            aIdx.Assign( *pTblNd, - 1 );
    }

    do {
        while( aIdx.GetIndex() &&
            0 == ( pTblNd = aIdx.GetNode().StartOfSectionNode()->GetTableNode()) )
            aIdx--;

        if( pTblNd )        // gibt einen weiteren TableNode ?
        {
            if( fnPosTbl == fnMoveForward )         // an Anfang ?
            {
                aIdx = *aIdx.GetNode().StartOfSectionNode();
                if( lcl_FindNextCell( aIdx, bInReadOnly ))
                {
                    // Tabelle ueberspringen
                    aIdx.Assign( *pTblNd, -1 );
                    continue;
                }
            }
            else
            {
                // ueberpruefe geschuetzte Zellen
                if( lcl_FindNextCell( aIdx, bInReadOnly ))
                {
                    // Tabelle ueberspringen
                    aIdx.Assign( *pTblNd, -1 );
                    continue;
                }
            }
            rCurCrsr.GetPoint()->nNode = aIdx;
            rCurCrsr.Move( fnPosTbl, fnGoCntnt );
            return TRUE;
        }
    } while( pTblNd );

    return FALSE;
}


FASTBOOL GotoNextTable( SwPaM& rCurCrsr, SwPosTable fnPosTbl,
                        FASTBOOL bInReadOnly )
{
    SwNodeIndex aIdx( rCurCrsr.GetPoint()->nNode );
    SwTableNode* pTblNd = aIdx.GetNode().FindTableNode();
    if( pTblNd )
        aIdx.Assign( *pTblNd->EndOfSectionNode(), 1 );

    ULONG nLastNd = rCurCrsr.GetDoc()->GetNodes().Count() - 1;
    do {
        while( aIdx.GetIndex() < nLastNd &&
                0 == ( pTblNd = aIdx.GetNode().GetTableNode()) )
            aIdx++;
        if( pTblNd )        // gibt einen weiteren TableNode ?
        {
            if( fnPosTbl == fnMoveForward )         // an Anfang ?
            {
                if( lcl_FindNextCell( aIdx, bInReadOnly ))
                {
                    // Tabelle ueberspringen
                    aIdx.Assign( *pTblNd->EndOfSectionNode(), + 1 );
                    continue;
                }
            }
            else
            {
                aIdx = *aIdx.GetNode().EndOfSectionNode();
                // ueberpruefe geschuetzte Zellen
                if( lcl_FindNextCell( aIdx, bInReadOnly ))
                {
                    // Tabelle ueberspringen
                    aIdx.Assign( *pTblNd->EndOfSectionNode(), + 1 );
                    continue;
                }
            }
            rCurCrsr.GetPoint()->nNode = aIdx;
            rCurCrsr.Move( fnPosTbl, fnGoCntnt );
            return TRUE;
        }
    } while( pTblNd );

    return FALSE;
}


FASTBOOL GotoCurrTable( SwPaM& rCurCrsr, SwPosTable fnPosTbl,
                        FASTBOOL bInReadOnly )
{
    SwTableNode* pTblNd = rCurCrsr.GetPoint()->nNode.GetNode().FindTableNode();
    if( !pTblNd )
        return FALSE;

    if( fnPosTbl == fnMoveBackward )    // ans Ende der Tabelle
    {
        SwNodeIndex aIdx( *pTblNd->EndOfSectionNode() );
        if( lcl_FindPrevCell( aIdx, bInReadOnly ))
            return FALSE;
        rCurCrsr.GetPoint()->nNode = aIdx;
    }
    else
    {
        SwNodeIndex aIdx( *pTblNd );
        if( lcl_FindNextCell( aIdx, bInReadOnly ))
            return FALSE;
        rCurCrsr.GetPoint()->nNode = aIdx;
    }
    rCurCrsr.Move( fnPosTbl, fnGoCntnt );
    return TRUE;
}


FASTBOOL SwCursor::MoveTable( SwWhichTable fnWhichTbl, SwPosTable fnPosTbl )
{
    FASTBOOL bRet = FALSE;
    SwTableCursor* pTblCrsr = (SwTableCursor*)*this;

    if( pTblCrsr || !HasMark() )    // nur wenn kein Mark oder ein TblCrsr
    {
        SwCrsrSaveState aSaveState( *this );
        bRet = (*fnWhichTbl)( *this, fnPosTbl, IsReadOnlyAvailable() ) &&
                !IsSelOvr( SELOVER_CHECKNODESSECTION | SELOVER_TOGGLE );
    }
    return bRet;
}

FASTBOOL SwCrsrShell::MoveTable( SwWhichTable fnWhichTbl, SwPosTable fnPosTbl )
{
    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen

    SwShellCrsr* pCrsr = pTblCrsr ? pTblCrsr : pCurCrsr;
    FASTBOOL bCheckPos, bRet;
    ULONG nPtNd;
    xub_StrLen nPtCnt;

    if( !pTblCrsr && pCurCrsr->HasMark() )      // wenn Mark und kein TblCrsr,
    {
        // dann auf jedenfall in den Tabellen-Modus schalten
        pTblCrsr = new SwShellTableCrsr( *this, *pCurCrsr->GetPoint() );
        pCurCrsr->DeleteMark();
        pCurCrsr->SwSelPaintRects::Hide();
        pTblCrsr->SetMark();
        pCrsr = pTblCrsr;
        bCheckPos = FALSE;
    }
    else
    {
        bCheckPos = TRUE;
        nPtNd = pCrsr->GetPoint()->nNode.GetIndex();
        nPtCnt = pCrsr->GetPoint()->nContent.GetIndex();
    }

    bRet = pCrsr->MoveTable( fnWhichTbl, fnPosTbl );

    if( bRet )
    {
        //JP 28.10.97: Bug 45028 - die "oberste" Position setzen fuer
        //              wiederholte Kopfzeilen
        pCrsr->GetPtPos() = Point();

        UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);

        if( bCheckPos &&
            pCrsr->GetPoint()->nNode.GetIndex() == nPtNd &&
            pCrsr->GetPoint()->nContent.GetIndex() == nPtCnt )
            bRet = FALSE;
    }
    return bRet;
}


FASTBOOL SwCrsrShell::IsTblComplex() const
{
    SwFrm *pFrm = GetCurrFrm( FALSE );
    if ( pFrm && pFrm->IsInTab() )
        return pFrm->FindTabFrm()->GetTable()->IsTblComplex();
    return FALSE;
}


FASTBOOL SwCrsrShell::IsTblComplexForChart()
{
    FASTBOOL bRet = FALSE;

    StartAction();  // IsTblComplexForChart() may trigger table formatting
                    // we better do that inside an action

    const SwTableNode* pTNd = pCurCrsr->GetPoint()->nNode.GetNode().FindTableNode();
    if( pTNd )
    {
        // wir stehen in der Tabelle, dann teste mal, ob die Tabelle oder die
        // Selektion ausgeglichen ist.
        String sSel;
        if( pTblCrsr )
            sSel = GetBoxNms();
        bRet = pTNd->GetTable().IsTblComplexForChart( sSel );
    }

    EndAction();

    return bRet;
}

String SwCrsrShell::GetBoxNms() const
{
    String sNm;
    const SwPosition* pPos;
    SwFrm* pFrm;

    if( IsTableMode() )
    {
        pFrm = pTblCrsr->Start()->nNode.GetNode().GetCntntNode()->GetFrm();

        do {
            pFrm = pFrm->GetUpper();
        } while ( pFrm && !pFrm->IsCellFrm() );

        ASSERT( pFrm, "kein Frame zur Box" );
        sNm = ((SwCellFrm*)pFrm)->GetTabBox()->GetName();
        sNm += ':';
        pPos = pTblCrsr->End();
    }
    else
    {
        const SwTableNode* pTblNd = IsCrsrInTbl();
        if( !pTblNd )
            return sNm;
        pPos = GetCrsr()->GetPoint();
    }

    pFrm = pPos->nNode.GetNode().GetCntntNode()->GetFrm();

    do {
        pFrm = pFrm->GetUpper();
    } while ( pFrm && !pFrm->IsCellFrm() );

    if( pFrm )
        sNm += ((SwCellFrm*)pFrm)->GetTabBox()->GetName();

    return sNm;
}


FASTBOOL SwCrsrShell::GotoTable( const String& rName )
{
    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen,
    FASTBOOL bRet = !pTblCrsr && pCurCrsr->GotoTable( rName );
    if( bRet )
    {
        pCurCrsr->GetPtPos() = Point();
        UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                    SwCrsrShell::READONLY ); // und den akt. Updaten
    }
    return bRet;
}


FASTBOOL SwCrsrShell::CheckTblBoxCntnt( const SwPosition* pPos )
{
    if( !pBoxIdx || !pBoxPtr || IsSelTblCells() || !IsAutoUpdateCells() )
        return FALSE;

    // ueberpruefe, ob der Box Inhalt mit dem angegebenen Format der Box
    // ueber einstimmt. Wenn nicht, setze neu
    SwTableBox* pChkBox = 0;
    SwStartNode* pSttNd;
    if( !pPos )
    {
        // gesicherte Position heraus holen.
        if( pBoxIdx && pBoxPtr &&
            0 != ( pSttNd = pBoxIdx->GetNode().GetStartNode() ) &&
            SwTableBoxStartNode == pSttNd->GetStartNodeType() &&
            pBoxPtr == pSttNd->FindTableNode()->GetTable().
                        GetTblBox( pBoxIdx->GetIndex() ) )
            pChkBox = pBoxPtr;
    }
    else if( 0 != ( pSttNd = pPos->nNode.GetNode().
                                FindSttNodeByType( SwTableBoxStartNode )) )
    {
        pChkBox = pSttNd->FindTableNode()->GetTable().GetTblBox( pSttNd->GetIndex() );
    }


    // Box mehr als 1 Absatz?
    if( pChkBox && pSttNd->GetIndex() + 2 != pSttNd->EndOfSectionIndex() )
        pChkBox = 0;

    // jetzt sollten wir mal die Pointer zerstoeren, bevor eine erneute
    // Actionklammerung kommt.
    if( !pPos && !pChkBox )
        ClearTblBoxCntnt();

    // liegt der Cursor nicht mehr in dem Bereich ?
    if( pChkBox && !pPos &&
        ( pCurCrsr->HasMark() || pCurCrsr->GetNext() != pCurCrsr ||
          pSttNd->GetIndex() + 1 == pCurCrsr->GetPoint()->nNode.GetIndex() ))
        pChkBox = 0;

    //JP 12.01.99: hat sich der Inhalt der Box ueberhaupt veraendert?
    // Ist wichtig, wenn z.B. Undo nicht den richtigen Inhalt wieder
    // herstellen konnte.
    if( pChkBox )
    {
        const SwTxtNode* pNd = GetDoc()->GetNodes()[
                                    pSttNd->GetIndex() + 1 ]->GetTxtNode();
        if( !pNd ||
            ( pNd->GetTxt() == ViewShell::GetShellRes()->aCalc_Error &&
              SFX_ITEM_SET == pChkBox->GetFrmFmt()->
                            GetItemState( RES_BOXATR_FORMULA )) )
            pChkBox = 0;
    }

    if( pChkBox )
    {
        // jetzt sollten wir mal die Pointer zerstoeren, bevor ein weiterer
        // aufruf kommt.
        ClearTblBoxCntnt();
        StartAction();
        GetDoc()->ChkBoxNumFmt( *pChkBox, TRUE );
        EndAction();
    }

    return 0 != pChkBox;
}


void SwCrsrShell::SaveTblBoxCntnt( const SwPosition* pPos )
{
    if( IsSelTblCells() || !IsAutoUpdateCells() )
        return ;

    if( !pPos )
        pPos = pCurCrsr->GetPoint();

    SwStartNode* pSttNd = pPos->nNode.GetNode().FindSttNodeByType( SwTableBoxStartNode );

    BOOL bCheckBox = FALSE;
    if( pSttNd && pBoxIdx )
    {
        if( pSttNd == &pBoxIdx->GetNode() )
            pSttNd = 0;     // die haben wir schon
        else
            bCheckBox = TRUE;
    }
    else
        bCheckBox = 0 != pBoxIdx;

    if( bCheckBox )
    {
        // pBoxIdx Checken
        SwPosition aPos( *pBoxIdx );
        CheckTblBoxCntnt( &aPos );
    }

    if( pSttNd )
    {
        pBoxPtr = pSttNd->FindTableNode()->GetTable().GetTblBox( pSttNd->GetIndex() );

        if( pBoxIdx )
            *pBoxIdx = *pSttNd;
        else
            pBoxIdx = new SwNodeIndex( *pSttNd );
    }
}


void SwCrsrShell::ClearTblBoxCntnt()
{
    delete pBoxIdx, pBoxIdx = 0;
    pBoxPtr = 0;
}

FASTBOOL SwCrsrShell::EndAllTblBoxEdit()
{
    FASTBOOL bRet = FALSE;
    ViewShell *pSh = this;
    do {
        if( pSh->IsA( TYPE( SwCrsrShell ) ) )
            bRet |= ((SwCrsrShell*)pSh)->CheckTblBoxCntnt(
                        ((SwCrsrShell*)pSh)->pCurCrsr->GetPoint() );

    } while( this != (pSh = (ViewShell *)pSh->GetNext()) );
    return bRet;
}




