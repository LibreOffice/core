/*************************************************************************
 *
 *  $RCSfile: trvltbl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:17 $
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
#ifndef _NODE_HXX //autogen
#include <node.hxx>
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


FASTBOOL SwCrsrShell::SelTblRow()
{
    // pruefe ob vom aktuellen Crsr der SPoint/Mark in einer Tabelle stehen
    SwFrm *pFrm = GetCurrFrm();
    if( !pFrm->IsInTab() )
        return FALSE;

    const SwTabFrm *pTblFrm = pFrm->ImplFindTabFrm();

    SET_CURR_SHELL( this );

    // lasse ueber das Layout die Boxen suchen
    Point aPt( pTblFrm->Frm().Left(), pTblFrm->Frm().Right() );
    SwSelBoxes aBoxes;

    SwTblSearchType eType = TBLSEARCH_ROW;
    if( !IsReadOnlyAvailable() )
        eType = (SwTblSearchType)(eType | TBLSEARCH_PROTECT);
    GetTblSel( *this, aBoxes, eType );

    if( !aBoxes.Count() )
        return FALSE;

    // noch kein Tabellen-Cursor vorhanden, dann erzeuge einen
    if( !pTblCrsr )
    {
        pTblCrsr = new SwShellTableCrsr( *this, *pCurCrsr->GetPoint() );
        pCurCrsr->DeleteMark();
        pCurCrsr->SwSelPaintRects::Hide();
    }

//  SwCallLink aLk( *this );        // Crsr-Moves ueberwachen,
    pTblCrsr->DeleteMark();
    // dann setze mal Anfang und Ende der Spalte
    pTblCrsr->GetPoint()->nNode = *aBoxes[0]->GetSttNd();
    pTblCrsr->Move( fnMoveForward, fnGoCntnt );
    pTblCrsr->SetMark();
    pTblCrsr->GetPoint()->nNode = *aBoxes[aBoxes.Count()-1]->GetSttNd();
    pTblCrsr->Move( fnMoveForward, fnGoCntnt );
    UpdateCrsr();                 // und den akt. Updaten
    return TRUE;
}


FASTBOOL SwCrsrShell::SelTblCol()
{
    SwFrm *pFrm = GetCurrFrm();
    if( !pFrm->IsInTab() )
        return FALSE;

    const SwTabFrm *pTblFrm = pFrm->ImplFindTabFrm();

    SET_CURR_SHELL( this );

    // lasse ueber das Layout die Boxen suchen
    Point aPt( pTblFrm->Frm().Top(), pTblFrm->Frm().Bottom() );
    SwSelBoxes aBoxes;
    SwTblSearchType eType = TBLSEARCH_COL;
    if( !IsReadOnlyAvailable() )
        eType = (SwTblSearchType)(eType | TBLSEARCH_PROTECT);
    GetTblSel( *this, aBoxes, eType );

    if( !aBoxes.Count() )
        return FALSE;

    // noch kein Tabellen-Cursor vorhanden, dann erzeuge einen
    if( !pTblCrsr )
    {
        pTblCrsr = new SwShellTableCrsr( *this, *pCurCrsr->GetPoint() );
        pCurCrsr->DeleteMark();
        pCurCrsr->SwSelPaintRects::Hide();
    }

//  SwCallLink aLk( *this );        // Crsr-Moves ueberwachen,
    pTblCrsr->DeleteMark();
    // dann setze mal Anfang und Ende der Spalte
    pTblCrsr->GetPoint()->nNode = *aBoxes[0]->GetSttNd();
    pTblCrsr->Move( fnMoveForward, fnGoCntnt );
    pTblCrsr->SetMark();
    pTblCrsr->GetPoint()->nNode = *aBoxes[aBoxes.Count()-1]->GetSttNd();
    pTblCrsr->Move( fnMoveForward, fnGoCntnt );
    UpdateCrsr();                 // und den akt. Updaten
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
        pCNd = rNds.GoNext( &aTmp );

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
        aIdx.Assign( *pTblNd, - 1 );

    do {
        while( aIdx.GetIndex() &&
            0 == ( pTblNd = aIdx.GetNode().FindStartNode()->GetTableNode()) )
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
                !IsSelOvr();
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


FASTBOOL SwCrsrShell::IsTblComplexForChart() const
{
    FASTBOOL bRet = FALSE;
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




