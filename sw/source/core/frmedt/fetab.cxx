 /*************************************************************************
 *
 *  $RCSfile: fetab.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 14:03:33 $
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

#define ITEMID_BOXINFO SID_ATTR_BORDER_INNER
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef __RSC //autogen
#include <tools/errinf.hxx>
#endif
#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_FRMDIRITEM_HXX
#include <svx/frmdiritem.hxx>
#endif
#ifndef _RULER_HXX
#include <svtools/ruler.hxx>
#endif
#ifndef _SWWAIT_HXX
#include <swwait.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _FESH_HXX
#include <fesh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _TABFRM_HXX
#include <tabfrm.hxx>
#endif
#ifndef _ROWFRM_HXX
#include <rowfrm.hxx>
#endif
#ifndef _CELLFRM_HXX
#include <cellfrm.hxx>
#endif
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif
#ifndef _DFLYOBJ_HXX
#include <dflyobj.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _SWDDETBL_HXX
#include <swddetbl.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _CALC_HXX
#include <calc.hxx>
#endif
#ifndef _TABCOL_HXX
#include <tabcol.hxx>
#endif
#ifndef _CELLATR_HXX
#include <cellatr.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _VISCRS_HXX
#include <viscrs.hxx>
#endif
#ifndef _TBLSEL_HXX
#include <tblsel.hxx>
#endif
#ifndef _SWTBLFMT_HXX
#include <swtblfmt.hxx>
#endif

#ifndef _SWSWERROR_H
#include <swerror.h>
#endif

#include <node.hxx> // #i23726#
// OD 2004-05-24 #i28701#
#ifndef _SORTEDOBJS_HXX
#include <sortedobjs.hxx>
#endif

//siehe auch swtable.cxx
#define COLFUZZY 20L

inline BOOL IsSame( long nA, long nB ) { return  Abs(nA-nB) <= COLFUZZY; }

// table column cache
SwTabCols *pLastCols   = 0;
const SwTable   *pColumnCacheLastTable  = 0;
const SwTabFrm  *pColumnCacheLastTabFrm = 0;
const SwFrm     *pColumnCacheLastCellFrm = 0;

// table row cache
SwTabCols *pLastRows   = 0;
const SwTable   *pRowCacheLastTable  = 0;
const SwTabFrm  *pRowCacheLastTabFrm = 0;
const SwFrm     *pRowCacheLastCellFrm = 0;


class TblWait
{
    SwWait *pWait;
public:
    TblWait( USHORT nCnt, SwFrm *pFrm, SwDocShell &rDocShell, USHORT nCnt2 = 0);
    ~TblWait() { delete pWait; }
};

TblWait::TblWait( USHORT nCnt, SwFrm *pFrm, SwDocShell &rDocShell, USHORT nCnt2):
    pWait( 0 )
{
    BOOL bWait = 20 < nCnt || 20 < nCnt2 || (pFrm &&
                 20 < pFrm->ImplFindTabFrm()->GetTable()->GetTabLines().Count());
    if( bWait )
        pWait = new SwWait( rDocShell, TRUE );
}

inline const SwCursor& GetShellCursor( const SwCrsrShell& rShell )
{
    const SwShellCrsr *pCrsr = rShell.GetTableCrsr();
    if( !pCrsr )
        pCrsr = (SwShellCrsr*)*rShell.GetSwCrsr( FALSE );
    return *pCrsr;
}

void SwFEShell::ParkCursorInTab()
{
    SwCursor * pSwCrsr = GetSwCrsr();

    ASSERT(pSwCrsr, "no SwCursor");

    SwPosition aStartPos = *pSwCrsr->GetPoint(), aEndPos = aStartPos;

    SwCursor * pTmpCrsr = (SwCursor *) pSwCrsr;

    /* Search least and greatest position in current cursor ring.
     */
    do
    {
        const SwPosition * pPt = pTmpCrsr->GetPoint(),
            * pMk = pTmpCrsr->GetMark();

        if (*pPt < aStartPos)
            aStartPos = *pPt;

        if (*pPt > aEndPos)
            aEndPos = *pPt;

        if (*pMk < aStartPos)
            aStartPos = *pMk;

        if (*pMk > aEndPos)
            aEndPos = *pMk;

        pTmpCrsr = (SwCursor *) pTmpCrsr->GetNext();
    }
    while (pTmpCrsr != pSwCrsr);

    KillPams();

    /* Set cursor to end of selection to ensure IsLastCellInRow works
       properly. */
    {
        SwCursor aTmpCrsr(aEndPos);
        *pSwCrsr = aTmpCrsr;
    }

    /* Move the cursor out of the columns to delete and stay in the
       same row. If the table has only one column the cursor will
       stay in the row and the shell will take care of it. */
    if (IsLastCellInRow())
    {
        /* If the cursor is in the last row of the table, first
           try to move it to the previous cell. If that fails move
           it to the next cell. */

        {
            SwCursor aTmpCrsr(aStartPos);
            *pSwCrsr = aTmpCrsr;
        }

        if (! pSwCrsr->GoPrevCell())
        {
            SwCursor aTmpCrsr(aEndPos);
            *pSwCrsr = aTmpCrsr;
            pSwCrsr->GoNextCell();
        }
    }
    else
    {
        /* If the cursor is not in the last row of the table, first
           try to move it to the next cell. If that fails move it
           to the previous cell. */

        {
            SwCursor aTmpCrsr(aEndPos);
            *pSwCrsr = aTmpCrsr;
        }

        if (! pSwCrsr->GoNextCell())
        {
            SwCursor aTmpCrsr(aStartPos);
            *pSwCrsr = aTmpCrsr;
            pSwCrsr->GoPrevCell();
        }
    }
}

/***********************************************************************
#*  Class      :  SwFEShell
#*  Methoden   :  GetStartEndCell
#*  Datum      :  MA 03. May. 93
#*  Update     :  MA 23. May. 95
#***********************************************************************/
void SwFEShell::GetStartEndCell( SwLayoutFrm *&prStart, SwLayoutFrm *&prEnd )
{
    SwPaM* pPam = IsTableMode() ? GetTblCrs() : GetCrsr();
    SwShellCrsr *pICr = (SwShellCrsr*)*((SwCursor*)pPam);

    ASSERT( pICr->GetCntntNode() && pICr->GetCntntNode( FALSE ),
            "Tabselection nicht auf Cnt." );

    prStart = pICr->GetCntntNode()->GetFrm( &pICr->GetPtPos() )->GetUpper(),
    prEnd   = pICr->GetCntntNode(FALSE)->GetFrm( &pICr->GetMkPos() )->GetUpper();
}

/***********************************************************************
#*  Class      :  SwFEShell
#*  Methoden   :  InsertRow(), InsertCol
#*  Datum      :  MA 03. May. 93
#*  Update     :  MA 19. Apr. 95
#***********************************************************************/
BOOL SwFEShell::InsertRow( USHORT nCnt, BOOL bBehind )
{
    // pruefe ob vom aktuellen Crsr der Point/Mark in einer Tabelle stehen
    SwFrm *pFrm = GetCurrFrm();
    if( !pFrm->IsInTab() )
        return FALSE;

    if( pFrm->ImplFindTabFrm()->GetTable()->ISA( SwDDETable ))
    {
        ErrorHandler::HandleError( ERR_TBLDDECHG_ERROR,
                        ERRCODE_MSG_INFO | ERRCODE_BUTTON_DEF_OK );
        return FALSE;
    }

    SET_CURR_SHELL( this );
    StartAllAction();

    // lasse ueber das Layout die Boxen suchen
    SwSelBoxes aBoxes;
    GetTblSel( *this, aBoxes, TBLSEARCH_ROW );

    TblWait( nCnt, pFrm, *GetDoc()->GetDocShell(), aBoxes.Count() );

    BOOL bRet = FALSE;
    if ( aBoxes.Count() )
        bRet = GetDoc()->InsertRow( aBoxes, nCnt, bBehind );

    EndAllActionAndCall();
    return bRet;
}

BOOL SwFEShell::InsertCol( USHORT nCnt, BOOL bBehind )
{
    // pruefe ob vom aktuellen Crsr der Point/Mark in einer Tabelle stehen
    SwFrm *pFrm = GetCurrFrm();
    if( !pFrm->IsInTab() )
        return FALSE;

    if( pFrm->ImplFindTabFrm()->GetTable()->ISA( SwDDETable ))
    {
        ErrorHandler::HandleError( ERR_TBLDDECHG_ERROR,
                        ERRCODE_MSG_INFO | ERRCODE_BUTTON_DEF_OK );
        return FALSE;
    }

    SET_CURR_SHELL( this );

    if( !CheckSplitCells( *this, nCnt + 1, TBLSEARCH_COL ) )
    {
        ErrorHandler::HandleError( ERR_TBLINSCOL_ERROR,
                        ERRCODE_MSG_INFO | ERRCODE_BUTTON_DEF_OK );
        return FALSE;
    }

    StartAllAction();
    // lasse ueber das Layout die Boxen suchen
    SwSelBoxes aBoxes;
    GetTblSel( *this, aBoxes, TBLSEARCH_COL );

    TblWait( nCnt, pFrm, *GetDoc()->GetDocShell(), aBoxes.Count() );

    BOOL bRet = FALSE;
    if( aBoxes.Count() )
        bRet = GetDoc()->InsertCol( aBoxes, nCnt, bBehind );

    EndAllActionAndCall();
    return bRet;
}

/***********************************************************************
#*  Class      :  SwFEShell
#*  Methoden   :  DeleteRow(), DeleteCol()
#*  Datum      :  MA 03. May. 93
#*  Update     :  MA 19. Apr. 95
#***********************************************************************/

/**
   Determines if the current cursor is in the last row of the table.
*/
BOOL SwFEShell::IsLastCellInRow() const
{
    SwTabCols aTabCols;
    GetTabCols( aTabCols );
    BOOL bResult = FALSE;

    if (IsTableRightToLeft())
        /* If the table is right-to-left the last row is the most left one. */
        bResult = 0 == GetCurTabColNum();
    else
        /* If the table is left-to-right the last row is the most right one. */
        bResult = aTabCols.Count() == GetCurTabColNum();

    return bResult;
}

BOOL SwFEShell::DeleteCol()
{
    // pruefe ob vom aktuellen Crsr der SPoint/Mark in einer Tabelle stehen
    SwFrm *pFrm = GetCurrFrm();
    if( !pFrm->IsInTab() )
        return FALSE;

    if( pFrm->ImplFindTabFrm()->GetTable()->ISA( SwDDETable ))
    {
        ErrorHandler::HandleError( ERR_TBLDDECHG_ERROR,
                        ERRCODE_MSG_INFO | ERRCODE_BUTTON_DEF_OK );
        return FALSE;
    }

    SET_CURR_SHELL( this );
    StartAllAction();

    // lasse ueber das Layout die Boxen suchen
    BOOL bRet;
    SwSelBoxes aBoxes;
    GetTblSel( *this, aBoxes, TBLSEARCH_COL );
    if ( aBoxes.Count() )
    {
        TblWait( aBoxes.Count(), pFrm, *GetDoc()->GetDocShell() );

        // die Crsr muessen noch aus dem Loesch Bereich entfernt
        // werden. Setze sie immer hinter/auf die Tabelle; ueber die
        // Dokument-Position werden sie dann immer an die alte Position gesetzt.
        while( !pFrm->IsCellFrm() )
            pFrm = pFrm->GetUpper();

        ParkCursorInTab();

        // dann loesche doch die Spalten
        bRet = GetDoc()->DeleteRowCol( aBoxes );

    }
    else
        bRet = FALSE;

    EndAllActionAndCall();
    return bRet;
}

BOOL SwFEShell::DeleteRow()
{
    // pruefe ob vom aktuellen Crsr der SPoint/Mark in einer Tabelle stehen
    SwFrm *pFrm = GetCurrFrm();
    if( !pFrm->IsInTab() )
        return FALSE;

    if( pFrm->ImplFindTabFrm()->GetTable()->ISA( SwDDETable ))
    {
        ErrorHandler::HandleError( ERR_TBLDDECHG_ERROR,
                        ERRCODE_MSG_INFO | ERRCODE_BUTTON_DEF_OK );
        return FALSE;
    }

    SET_CURR_SHELL( this );
    StartAllAction();

    // lasse ueber das Layout die Boxen suchen
    BOOL bRet;
    SwSelBoxes aBoxes;
    GetTblSel( *this, aBoxes, TBLSEARCH_ROW );

    if( aBoxes.Count() )
    {
        TblWait( aBoxes.Count(), pFrm, *GetDoc()->GetDocShell() );

        // die Crsr aus dem Loeschbereich entfernen.
        // Der Cursor steht danach:
        //  - es folgt noch eine Zeile, in dieser
        //  - vorher steht noch eine Zeile, in dieser
        //  - sonst immer dahinter
        {
            SwTableNode* pTblNd = ((SwCntntFrm*)pFrm)->GetNode()->FindTableNode();

            // suche alle Boxen / Lines
            _FndBox aFndBox( 0, 0 );
            {
                _FndPara aPara( aBoxes, &aFndBox );
                pTblNd->GetTable().GetTabLines().ForEach( &_FndLineCopyCol, &aPara );
            }

            if( !aFndBox.GetLines().Count() )
            {
                EndAllActionAndCall();
                return FALSE;
            }

            KillPams();

            _FndBox* pFndBox = &aFndBox;
            while( 1 == pFndBox->GetLines().Count() &&
                    1 == pFndBox->GetLines()[0]->GetBoxes().Count() )
            {
                _FndBox* pTmp = pFndBox->GetLines()[0]->GetBoxes()[0];
                if( pTmp->GetBox()->GetSttNd() )
                    break;      // das ist sonst zu weit
                pFndBox = pTmp;
            }

            SwTableLine* pDelLine = pFndBox->GetLines()[
                            pFndBox->GetLines().Count()-1 ]->GetLine();
            SwTableBox* pDelBox = pDelLine->GetTabBoxes()[
                                pDelLine->GetTabBoxes().Count() - 1 ];
            while( !pDelBox->GetSttNd() )
            {
                SwTableLine* pLn = pDelBox->GetTabLines()[
                            pDelBox->GetTabLines().Count()-1 ];
                pDelBox = pLn->GetTabBoxes()[ pLn->GetTabBoxes().Count() - 1 ];
            }
            SwTableBox* pNextBox = pDelLine->FindNextBox( pTblNd->GetTable(),
                                                            pDelBox, TRUE );
            while( pNextBox &&
                    pNextBox->GetFrmFmt()->GetProtect().IsCntntProtected() )
                pNextBox = pNextBox->FindNextBox( pTblNd->GetTable(), pNextBox );

            if( !pNextBox )         // keine nachfolgende? dann die vorhergehende
            {
                pDelLine = pFndBox->GetLines()[ 0 ]->GetLine();
                pDelBox = pDelLine->GetTabBoxes()[ 0 ];
                while( !pDelBox->GetSttNd() )
                    pDelBox = pDelBox->GetTabLines()[0]->GetTabBoxes()[0];
                pNextBox = pDelLine->FindPreviousBox( pTblNd->GetTable(),
                                                            pDelBox, TRUE );
                while( pNextBox &&
                        pNextBox->GetFrmFmt()->GetProtect().IsCntntProtected() )
                    pNextBox = pNextBox->FindPreviousBox( pTblNd->GetTable(), pNextBox );
            }

            ULONG nIdx;
            if( pNextBox )      // dann den Cursor hier hinein
                nIdx = pNextBox->GetSttIdx() + 1;
            else                // ansonsten hinter die Tabelle
                nIdx = pTblNd->EndOfSectionIndex() + 1;

            SwNodeIndex aIdx( GetDoc()->GetNodes(), nIdx );
            SwCntntNode* pCNd = aIdx.GetNode().GetCntntNode();
            if( !pCNd )
                pCNd = GetDoc()->GetNodes().GoNext( &aIdx );

            if( pCNd )
            {
                SwPaM* pPam = GetCrsr();
                pPam->GetPoint()->nNode = aIdx;
                pPam->GetPoint()->nContent.Assign( pCNd, 0 );
                pPam->SetMark();            // beide wollen etwas davon haben
                pPam->DeleteMark();
            }
        }

        // dann loesche doch die Zeilen
        bRet = GetDoc()->DeleteRowCol( aBoxes );
    }
    else
        bRet = FALSE;

    EndAllActionAndCall();
    return bRet;
}

/***********************************************************************
#*  Class      :  SwFEShell
#*  Methoden   :  MergeTab(), SplitTab()
#*  Datum      :  MA 03. May. 93
#*  Update     :  MA 19. Apr. 95
#***********************************************************************/

USHORT SwFEShell::MergeTab()
{
    // pruefe ob vom aktuellen Crsr der SPoint/Mark in einer Tabelle stehen
    USHORT nRet = TBLMERGE_NOSELECTION;
    if( IsTableMode() )
    {
        SwShellTableCrsr* pTblCrsr = GetTableCrsr();
        const SwTableNode* pTblNd = pTblCrsr->GetNode()->FindTableNode();
        if( pTblNd->GetTable().ISA( SwDDETable ))
        {
            ErrorHandler::HandleError( ERR_TBLDDECHG_ERROR,
                            ERRCODE_MSG_INFO | ERRCODE_BUTTON_DEF_OK );
        }
        else
        {
            SET_CURR_SHELL( this );
            StartAllAction();

            TblWait( pTblCrsr->GetBoxesCount(), 0, *GetDoc()->GetDocShell(),
                     pTblNd->GetTable().GetTabLines().Count() );

            nRet = GetDoc()->MergeTbl( *pTblCrsr );

            KillPams();

            EndAllActionAndCall();
        }
    }
    return nRet;
}

BOOL SwFEShell::SplitTab( BOOL bVert, USHORT nCnt, BOOL bSameHeight )
{
    // pruefe ob vom aktuellen Crsr der SPoint/Mark in einer Tabelle stehen
    SwFrm *pFrm = GetCurrFrm();
    if( !pFrm->IsInTab() )
        return FALSE;

    if( pFrm->ImplFindTabFrm()->GetTable()->ISA( SwDDETable ))
    {
        ErrorHandler::HandleError( ERR_TBLDDECHG_ERROR,
                        ERRCODE_MSG_INFO | ERRCODE_BUTTON_DEF_OK );
        return FALSE;
    }

    SET_CURR_SHELL( this );

    if( bVert && !CheckSplitCells( *this, nCnt + 1 ) )
    {
        ErrorHandler::HandleError( ERR_TBLSPLIT_ERROR,
                        ERRCODE_MSG_INFO | ERRCODE_BUTTON_DEF_OK );
        return FALSE;
    }
    StartAllAction();
    // lasse ueber das Layout die Boxen suchen
    BOOL bRet;
    SwSelBoxes aBoxes;
    GetTblSel( *this, aBoxes );
    if( aBoxes.Count() )
    {
        TblWait( nCnt, pFrm, *GetDoc()->GetDocShell(), aBoxes.Count() );

        // dann loesche doch die Spalten
        bRet = GetDoc()->SplitTbl( aBoxes, bVert, nCnt, bSameHeight );

        DELETEZ( pLastCols );
        DELETEZ( pLastRows );
    }
    else
        bRet = FALSE;
    EndAllActionAndCall();
    return bRet;
}


/***********************************************************************
#*  Class      :  SwFEShell
#*  Methoden   :  _GetTabCols
#*  Datum      :  MA 30. Nov. 95
#*  Update     :  MA 08. Jan. 97
#***********************************************************************/
void SwFEShell::_GetTabCols( SwTabCols &rToFill, const SwFrm *pBox ) const
{
    const SwTabFrm *pTab = pBox->FindTabFrm();
    if ( pLastCols )
    {
        //Paar Kleinigkeiten muessen wir schon noch sicherstellen
        FASTBOOL bDel = TRUE;
        if ( pColumnCacheLastTable == pTab->GetTable() )
        {
            bDel = FALSE;
            SWRECTFN( pTab )

            const SwPageFrm* pPage = pTab->FindPageFrm();
            const ULONG nLeftMin = (pTab->Frm().*fnRect->fnGetLeft)() -
                                   (pPage->Frm().*fnRect->fnGetLeft)() +
                                   DOCUMENTBORDER;
            const ULONG nRightMax = (pTab->Frm().*fnRect->fnGetRight)() -
                                    (pPage->Frm().*fnRect->fnGetLeft)() +
                                   DOCUMENTBORDER;

            if ( pColumnCacheLastTabFrm != pTab )
            {
                //Wenn der TabFrm gewechselt hat, brauchen wir bei gleicher
                //Breite nur ein wenig shiften.
                SWRECTFNX( pColumnCacheLastTabFrm )
                if( (pColumnCacheLastTabFrm->Frm().*fnRect->fnGetWidth)() ==
                    (pTab->Frm().*fnRect->fnGetWidth)() )
                {
                    pLastCols->SetLeftMin( nLeftMin );

                    ASSERT( bVert ||
                            pLastCols->GetLeftMin() == (pTab->Frm().*fnRect->fnGetLeft)(),
                            "GetTabCols: wrong result" )

                    pColumnCacheLastTabFrm = pTab;
                }
                else
                    bDel = TRUE;
            }

            if ( !bDel &&
                 pLastCols->GetLeftMin () == (USHORT)nLeftMin &&
                 pLastCols->GetLeft    () == (USHORT)(pTab->Prt().*fnRect->fnGetLeft)() &&
                 pLastCols->GetRight   () == (USHORT)(pTab->Prt().*fnRect->fnGetRight)()&&
                 pLastCols->GetRightMax() == (USHORT)nRightMax - pLastCols->GetLeftMin() )
            {
                if ( pColumnCacheLastCellFrm != pBox )
                {
                    pTab->GetTable()->GetTabCols( *pLastCols,
                                        ((SwCellFrm*)pBox)->GetTabBox(), TRUE);
                    pColumnCacheLastCellFrm = pBox;
                }
                rToFill = *pLastCols;
            }
            else
                bDel = TRUE;
        }
        if ( bDel )
            DELETEZ(pLastCols);
    }
    if ( !pLastCols )
    {
        GetDoc()->GetTabCols( rToFill, 0, (SwCellFrm*)pBox );

        pLastCols   = new SwTabCols( rToFill );
        pColumnCacheLastTable  = pTab->GetTable();
        pColumnCacheLastTabFrm = pTab;
        pColumnCacheLastCellFrm= pBox;
    }

#if OSL_DEBUG_LEVEL > 1
    SwTabColsEntry aEntry;
    for ( int i = 0; i < rToFill.Count(); ++i )
    {
        aEntry = rToFill.GetEntry( i );
    }
#endif
}

/***********************************************************************
#*  Class      :  SwFEShell
#*  Methoden   :  _GetTabRows
#*  Datum      :  FME 2004-01-14
#*  Update     :
#***********************************************************************/
void SwFEShell::_GetTabRows( SwTabCols &rToFill, const SwFrm *pBox ) const
{
    const SwTabFrm *pTab = pBox->FindTabFrm();
    if ( pLastRows )
    {
        //Paar Kleinigkeiten muessen wir schon noch sicherstellen
        FASTBOOL bDel = TRUE;
        if ( pRowCacheLastTable == pTab->GetTable() )
        {
            bDel = FALSE;
            SWRECTFN( pTab )

            const SwPageFrm* pPage = pTab->FindPageFrm();
            ULONG nLeftMin = (*fnRect->fnYDiff)(
                                        (pTab->Frm().*fnRect->fnGetTop)(),
                                        (pPage->Frm().*fnRect->fnGetTop)() );
            nLeftMin += DOCUMENTBORDER;
            const ULONG nRightMax = LONG_MAX;

            if ( pRowCacheLastTabFrm != pTab ||
                 pRowCacheLastCellFrm != pBox )
                bDel = TRUE;

            if ( !bDel &&
                 pLastRows->GetLeftMin () == (USHORT)nLeftMin &&
                 pLastRows->GetLeft    () == (USHORT)(pTab->Prt().*fnRect->fnGetTop)() &&
                 pLastRows->GetRight   () == (USHORT)(pTab->Prt().*fnRect->fnGetBottom)()&&
                 pLastRows->GetRightMax() == nRightMax )
            {
                rToFill = *pLastRows;
            }
            else
                bDel = TRUE;
        }
        if ( bDel )
            DELETEZ(pLastRows);
    }
    if ( !pLastRows )
    {
        GetDoc()->GetTabRows( rToFill, 0, (SwCellFrm*)pBox );

        pLastRows   = new SwTabCols( rToFill );
        pRowCacheLastTable  = pTab->GetTable();
        pRowCacheLastTabFrm = pTab;
        pRowCacheLastCellFrm= pBox;
    }
}

/***********************************************************************
#*  Class      :  SwFEShell
#*  Methoden   :  SetTabCols(), GetTabCols()
#*  Datum      :  MA 03. May. 93
#*  Update     :  MA 18. May. 93
#***********************************************************************/
void SwFEShell::SetTabCols( const SwTabCols &rNew, BOOL bCurRowOnly )
{
    SwFrm *pBox = GetCurrFrm();
    if( !pBox->IsInTab() )
        return;

    SET_CURR_SHELL( this );
    StartAllAction();

    do {
        pBox = pBox->GetUpper();
    } while ( !pBox->IsCellFrm() );

    GetDoc()->SetTabCols( rNew, bCurRowOnly, 0, (SwCellFrm*)pBox );
    EndAllActionAndCall();
}

void SwFEShell::GetTabCols( SwTabCols &rToFill ) const
{
    const SwFrm *pFrm = GetCurrFrm();
    if( !pFrm->IsInTab() )
        return;
    do
    {   pFrm = pFrm->GetUpper();
    } while ( !pFrm->IsCellFrm() );

    _GetTabCols( rToFill, pFrm );
}

/*-- 19.01.2004 08:56:42---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwFEShell::GetTabRows( SwTabCols &rToFill ) const
{
    const SwFrm *pFrm = GetCurrFrm();
    if( !pFrm->IsInTab() )
        return;
    do
    {   pFrm = pFrm->GetUpper();
    } while ( !pFrm->IsCellFrm() );

    _GetTabRows( rToFill, pFrm );
}
/*-- 19.01.2004 08:56:44---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwFEShell::SetTabRows( const SwTabCols &rNew, BOOL bCurColOnly )
{
    SwFrm *pBox = GetCurrFrm();
    if( !pBox->IsInTab() )
        return;

    SET_CURR_SHELL( this );
    StartAllAction();

    do {
        pBox = pBox->GetUpper();
    } while ( !pBox->IsCellFrm() );

    GetDoc()->SetTabRows( rNew, bCurColOnly, 0, (SwCellFrm*)pBox );
    EndAllActionAndCall();
}
/*-- 19.01.2004 08:59:45---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwFEShell::GetMouseTabRows( SwTabCols &rToFill, const Point &rPt ) const
{
    const SwFrm *pBox = GetBox( rPt );
    if ( pBox )
        _GetTabRows( rToFill, pBox );
}
/*-- 19.01.2004 08:59:45---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwFEShell::SetMouseTabRows( const SwTabCols &rNew, BOOL bCurColOnly, const Point &rPt )
{
    const SwFrm *pBox = GetBox( rPt );
    if( pBox )
    {
        SET_CURR_SHELL( this );
        StartAllAction();
        GetDoc()->SetTabRows( rNew, bCurColOnly, 0, (SwCellFrm*)pBox );
        EndAllActionAndCall();
    }
}

/***********************************************************************
 *  Class      :  SwFEShell
 *  Methoden   :  SetRowSplit(), GetRowSplit()
 *  Datum      :  FME 13.11.2003
 ***********************************************************************/

void SwFEShell::SetRowSplit( const SwFmtRowSplit& rNew )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->SetRowSplit( GetShellCursor( *this ), rNew );
    EndAllActionAndCall();
}

void SwFEShell::GetRowSplit( SwFmtRowSplit*& rpSz ) const
{
    GetDoc()->GetRowSplit( GetShellCursor( *this ), rpSz );
}


/***********************************************************************
#*  Class      :  SwFEShell
#*  Methoden   :  SetRowHeight(), GetRowHeight()
#*  Datum      :  MA 17. May. 93
#*  Update     :  JP 29.04.98
#***********************************************************************/

void SwFEShell::SetRowHeight( const SwFmtFrmSize &rNew )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->SetRowHeight( GetShellCursor( *this ), rNew );
    EndAllActionAndCall();
}

/******************************************************************************
 *               SwTwips SwFEShell::GetRowHeight() const
 ******************************************************************************/
void SwFEShell::GetRowHeight( SwFmtFrmSize *& rpSz ) const
{
    GetDoc()->GetRowHeight( GetShellCursor( *this ), rpSz );
}

BOOL SwFEShell::BalanceRowHeight( BOOL bTstOnly )
{
    SET_CURR_SHELL( this );
    if( !bTstOnly )
        StartAllAction();
    BOOL bRet = GetDoc()->BalanceRowHeight( GetShellCursor( *this ), bTstOnly );
    if( !bTstOnly )
        EndAllActionAndCall();
    return bRet;
}

/******************************************************************************
 *              void SwFEShell::SetRowBackground()
 ******************************************************************************/
void SwFEShell::SetRowBackground( const SvxBrushItem &rNew )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->SetRowBackground( GetShellCursor( *this ), rNew );
    EndAllActionAndCall();
}

/******************************************************************************
 *               SwTwips SwFEShell::GetRowBackground() const
 ******************************************************************************/
BOOL SwFEShell::GetRowBackground( SvxBrushItem &rToFill ) const
{
    return GetDoc()->GetRowBackground( GetShellCursor( *this ), rToFill );
}

/***********************************************************************
#*  Class      :  SwFEShell
#*  Methoden   :  SetTabBorders(), GetTabBorders()
#*  Datum      :  MA 18. May. 93
#*  Update     :  JP 29.04.98
#***********************************************************************/

void SwFEShell::SetTabBorders( const SfxItemSet& rSet )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->SetTabBorders( GetShellCursor( *this ), rSet );
    EndAllActionAndCall();
}

void SwFEShell::SetTabLineStyle( const Color* pColor, BOOL bSetLine,
                                 const SvxBorderLine* pBorderLine )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->SetTabLineStyle( GetShellCursor( *this ),
                                pColor, bSetLine, pBorderLine );
    EndAllActionAndCall();
}

void SwFEShell::GetTabBorders( SfxItemSet& rSet ) const
{
    GetDoc()->GetTabBorders( GetShellCursor( *this ), rSet );
}


/***********************************************************************
#*  Class      :  SwFEShell
#*  Methoden   :  SetBoxBackground(), GetBoxBackground()
#*  Datum      :  MA 01. Jun. 93
#*  Update     :  MA 03. Jul. 96
#***********************************************************************/
void SwFEShell::SetBoxBackground( const SvxBrushItem &rNew )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->SetBoxAttr( GetShellCursor( *this ), rNew );
    EndAllActionAndCall();
}

BOOL SwFEShell::GetBoxBackground( SvxBrushItem &rToFill ) const
{
    return GetDoc()->GetBoxAttr( GetShellCursor( *this ), rToFill );
}

/***********************************************************************
#*  Class      :  SwFEShell
#*  Methoden   :  SetBoxDirection(), GetBoxDirection()
#*  Datum      :  FME 2004-02-03
#*  Update     :  FME 2004-02-03
#***********************************************************************/
void SwFEShell::SetBoxDirection( const SvxFrameDirectionItem& rNew )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->SetBoxAttr( GetShellCursor( *this ), rNew );
    EndAllActionAndCall();
}

BOOL SwFEShell::GetBoxDirection( SvxFrameDirectionItem&  rToFill ) const
{
    return GetDoc()->GetBoxAttr( GetShellCursor( *this ), rToFill );
}

/***********************************************************************
#*  Class      :  SwFEShell
#*  Methoden   :  SetBoxAlign, SetBoxAlign
#*  Datum      :  MA 18. Dec. 96
#*  Update     :  JP 29.04.98
#***********************************************************************/
void SwFEShell::SetBoxAlign( USHORT nAlign )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->SetBoxAlign( GetShellCursor( *this ), nAlign );
    EndAllActionAndCall();
}

USHORT SwFEShell::GetBoxAlign() const
{
    return GetDoc()->GetBoxAlign( GetShellCursor( *this ) );
}

/***********************************************************************
#*  Class      :  SwFEShell
#*  Methoden   :  SetTabBackground(), GetTabBackground()
#*  Datum      :  MA 08. Jul. 96
#*  Update     :  MA 08. Jul. 96
#***********************************************************************/
void SwFEShell::SetTabBackground( const SvxBrushItem &rNew )
{
    SwFrm *pFrm = GetCurrFrm();
    if( !pFrm->IsInTab() )
        return;

    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->SetAttr( rNew, *pFrm->ImplFindTabFrm()->GetFmt() );
    EndAllAction(); //Kein Call, denn es veraendert sich nichts!
    GetDoc()->SetModified();
}

void SwFEShell::GetTabBackground( SvxBrushItem &rToFill ) const
{
    SwFrm *pFrm = GetCurrFrm();
    if( pFrm->IsInTab() )
        rToFill = pFrm->ImplFindTabFrm()->GetFmt()->GetBackground();
}


/***********************************************************************
#*  Class      :  SwFEShell
#*  Methoden   :  HasWholeTabSelection()
#*  Datum      :  MA 18. May. 93
#*  Update     :  MA 20. Jul. 93
#***********************************************************************/
BOOL SwFEShell::HasWholeTabSelection() const
{
    //Ist die ganze Tabelle Selektiert?
    if ( IsTableMode() )
    {
        SwSelBoxes aBoxes;
        ::GetTblSelCrs( *this, aBoxes );
        if( aBoxes.Count() )
        {
            const SwTableNode *pTblNd = IsCrsrInTbl();
            return ( aBoxes[0]->GetSttIdx()-1 == pTblNd->
                EndOfSectionNode()->StartOfSectionIndex() &&
                aBoxes[aBoxes.Count()-1]->GetSttNd()->EndOfSectionIndex()+1
                ==  pTblNd->EndOfSectionIndex() );
        }
    }
    return FALSE;
}

BOOL SwFEShell::HasBoxSelection() const
{
    if(!IsCrsrInTbl())
        return FALSE;
    //Ist die ganze Tabelle Selektiert?
    if( IsTableMode() )
        return TRUE;
    SwPaM* pPam = GetCrsr();
        // leere Boxen gelten auch ohne Selektion als selektiert
//  if( !pPam->HasMark() )
//      return FALSE;
    BOOL bChg = FALSE;
    if( pPam->GetPoint() == pPam->End())
    {
        bChg = TRUE;
        pPam->Exchange();
    }
    SwNode* pNd;
    if( pPam->GetPoint()->nNode.GetIndex() -1 ==
        ( pNd = pPam->GetNode())->StartOfSectionIndex() &&
        !pPam->GetPoint()->nContent.GetIndex() &&
        pPam->GetMark()->nNode.GetIndex() + 1 ==
        pNd->EndOfSectionIndex())
    {
            SwNodeIndex aIdx( *pNd->EndOfSectionNode(), -1 );
            SwCntntNode* pCNd = GetDoc()->GetNodes()[ aIdx ]->GetCntntNode();
            if( !pCNd )
            {
                pCNd = GetDoc()->GetNodes().GoPrevious( &aIdx );
                ASSERT( pCNd, "kein ContentNode in der Box ??" );
            }
            if( pPam->GetMark()->nContent == pCNd->Len() )
            {
                if( bChg )
                    pPam->Exchange();
                return TRUE;
            }
    }
    if( bChg )
        pPam->Exchange();
    return FALSE;
}

/***********************************************************************
#*  Class      :  SwFEShell
#*  Methoden   :  ProtectCells(), UnProtectCells()
#*  Datum      :  MA 20. Jul. 93
#*  Update     :  JP 25. Sep. 93
#***********************************************************************/
void SwFEShell::ProtectCells()
{
    SvxProtectItem aProt;
    aProt.SetCntntProtect( TRUE );

    SET_CURR_SHELL( this );
    StartAllAction();

    GetDoc()->SetBoxAttr( GetShellCursor( *this ), aProt );

    if( !IsCrsrReadonly() )
    {
        if( IsTableMode() )
            ClearMark();
        ParkCursorInTab();
    }
    EndAllActionAndCall();
}

void SwFEShell::UnProtectCells( const String& rTblName )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->UnProtectCells( rTblName );
    EndAllActionAndCall();
}

// die Tabellenselektion aufheben
void SwFEShell::UnProtectCells()
{
    SET_CURR_SHELL( this );
    StartAllAction();

    SwSelBoxes aBoxes;
    if( IsTableMode() )
        ::GetTblSelCrs( *this, aBoxes );
    else
    {
        SwFrm *pFrm = GetCurrFrm();
        do {
            pFrm = pFrm->GetUpper();
        } while ( pFrm && !pFrm->IsCellFrm() );
        if( pFrm )
        {
            SwTableBox *pBox = (SwTableBox*)((SwCellFrm*)pFrm)->GetTabBox();
            aBoxes.Insert( pBox );
        }
    }

    if( aBoxes.Count() )
        GetDoc()->UnProtectCells( aBoxes );

    EndAllActionAndCall();
}

void SwFEShell::UnProtectTbls()
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->UnProtectTbls( *GetCrsr() );
    EndAllActionAndCall();
}

BOOL SwFEShell::HasTblAnyProtection( const String* pTblName,
                                    BOOL* pFullTblProtection )
{
    return GetDoc()->HasTblAnyProtection( GetCrsr()->GetPoint(), pTblName,
                                        pFullTblProtection );
}

BOOL SwFEShell::CanUnProtectCells() const
{
    BOOL bUnProtectAvailable = FALSE;
    const SwTableNode *pTblNd = IsCrsrInTbl();
    if( pTblNd && !pTblNd->IsProtect() )
    {
        SwSelBoxes aBoxes;
        if( IsTableMode() )
            ::GetTblSelCrs( *this, aBoxes );
        else
        {
            SwFrm *pFrm = GetCurrFrm();
            do {
                pFrm = pFrm->GetUpper();
            } while ( pFrm && !pFrm->IsCellFrm() );
            if( pFrm )
            {
                SwTableBox *pBox = (SwTableBox*)((SwCellFrm*)pFrm)->GetTabBox();
                aBoxes.Insert( pBox );
            }
        }
        if( aBoxes.Count() )
            bUnProtectAvailable = ::HasProtectedCells( aBoxes );
    }
    return bUnProtectAvailable;
}

/***********************************************************************
#*  Class      :  SwFEShell
#*  Methoden   :  GetRowsToRepeat(), SetRowsToRepeat()
#***********************************************************************/
USHORT SwFEShell::GetRowsToRepeat() const
{
    const SwFrm *pFrm = GetCurrFrm();
    const SwTabFrm *pTab = pFrm->FindTabFrm();
    if( pTab )
        return pTab->GetTable()->GetRowsToRepeat();
    return 0;
}

void SwFEShell::SetRowsToRepeat( USHORT nSet )
{
    SwFrm    *pFrm = GetCurrFrm();
    SwTabFrm *pTab = pFrm->FindTabFrm();
    if( pTab && pTab->GetTable()->GetRowsToRepeat() != nSet )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), TRUE );
        SET_CURR_SHELL( this );
        StartAllAction();
        GetDoc()->SetRowsToRepeat( *pTab->GetTable(), nSet );
        EndAllActionAndCall();
    }
}
/*-- 30.06.2004 08:46:35---------------------------------------------------
    returns the number of rows consecutively selected from top
  -----------------------------------------------------------------------*/
USHORT lcl_GetRowNumber( const SwPosition& rPos )
{
    USHORT nRet = USHRT_MAX;
    Point aTmpPt;
    const SwCntntNode *pNd;
    const SwCntntFrm *pFrm;

    if( 0 != ( pNd = rPos.nNode.GetNode().GetCntntNode() ))
        pFrm = pNd->GetFrm( &aTmpPt, &rPos, FALSE );
    else
        pFrm = 0;

    if ( pFrm && pFrm->IsInTab() )
    {
        const SwFrm* pRow = pFrm->GetUpper();
        while ( !pRow->GetUpper()->IsTabFrm() )
            pRow = pRow->GetUpper();

        const SwTabFrm* pTabFrm = (const SwTabFrm*)pRow->GetUpper();
        const SwTableLine* pTabLine = static_cast<const SwRowFrm*>(pRow)->GetTabLine();

        USHORT nI = 0;
        while ( nI < pTabFrm->GetTable()->GetTabLines().Count() )
        {
            if ( pTabFrm->GetTable()->GetTabLines()[ nI ] == pTabLine )
            {
                nRet = nI;
                break;
            }
            ++nI;
        }
    }

    return nRet;
}
USHORT SwFEShell::GetRowSelectionFromTop() const
{
    USHORT nRet = 0;
    const SwPaM* pPaM = IsTableMode() ? GetTableCrsr() : _GetCrsr();
    const USHORT nPtLine = lcl_GetRowNumber( *pPaM->GetPoint() );

    if ( !IsTableMode() )
    {
        nRet = 0 == nPtLine ? 1 : 0;
    }
    else
    {
        const USHORT nMkLine = lcl_GetRowNumber( *pPaM->GetMark() );

        if ( ( nPtLine == 0 && nMkLine != USHRT_MAX ) ||
             ( nMkLine == 0 && nPtLine != USHRT_MAX ) )
        {
            nRet = Max( nPtLine, nMkLine ) + 1;
        }
    }

    return nRet;
}

/*
 * 1. case: bRepeat = true
 * returns true if the current frame is located inside a table headline in
 * a follow frame
 *
 * 2. case: bRepeat = false
 * returns true if the current frame is localed inside a table headline OR
 * inside the first line of a table!!!
 */
BOOL SwFEShell::CheckHeadline( bool bRepeat ) const
{
    BOOL bRet = FALSE;
    if ( !IsTableMode() )
    {
        SwFrm *pFrm = GetCurrFrm();  // DONE MULTIIHEADER
        if ( pFrm->IsInTab() )
        {
            SwTabFrm* pTab = pFrm->FindTabFrm();
            if ( bRepeat )
            {
                bRet = pTab->IsFollow() && pTab->IsInHeadline( *pFrm );
            }
            else
            {
                bRet =  ((SwLayoutFrm*)pTab->Lower())->IsAnLower( pFrm ) ||
                        pTab->IsInHeadline( *pFrm );
            }
        }
    }
    return bRet;
}

/***********************************************************************
#*  Class      :  SwFEShell
#*  Methoden   :  AdjustCellWidth()
#*  Datum      :  MA 20. Feb. 95
#*  Update     :  MA 27. Jul. 95
#***********************************************************************/

void SwFEShell::AdjustCellWidth( BOOL bBalance )
{
    SET_CURR_SHELL( this );
    StartAllAction();

    //WarteCrsr immer einschalten, weil sich im vorraus nicht so recht
    //ermitteln laesst wieviel Inhalt betroffen ist.
    TblWait aWait( USHRT_MAX, 0, *GetDoc()->GetDocShell() );

    GetDoc()->AdjustCellWidth( GetShellCursor( *this ), bBalance );
    EndAllActionAndCall();
}

BOOL SwFEShell::IsAdjustCellWidthAllowed( BOOL bBalance ) const
{
    //Es muss mindestens eine Zelle mit Inhalt in der Selektion enthalten
    //sein.

    SwFrm *pFrm = GetCurrFrm();
    if( !pFrm->IsInTab() )
        return FALSE;

    SwSelBoxes aBoxes;
    ::GetTblSelCrs( *this, aBoxes );

    if ( bBalance )
        return aBoxes.Count() > 1;

    if ( !aBoxes.Count() )
    {
        do
        {   pFrm = pFrm->GetUpper();
        } while ( !pFrm->IsCellFrm() );
        SwTableBox *pBox = (SwTableBox*)((SwCellFrm*)pFrm)->GetTabBox();
        aBoxes.Insert( pBox );
    }

    for ( USHORT i = 0; i < aBoxes.Count(); ++i )
    {
        SwTableBox *pBox = aBoxes[i];
        if ( pBox->GetSttNd() )
        {
            SwNodeIndex aIdx( *pBox->GetSttNd(), 1 );
            SwTxtNode* pCNd = aIdx.GetNode().GetTxtNode();
            if( !pCNd )
                pCNd = (SwTxtNode*)GetDoc()->GetNodes().GoNext( &aIdx );

            while ( pCNd )
            {
                if ( pCNd->GetTxt().Len() )
                    return TRUE;
                ++aIdx;
                pCNd = GetDoc()->GetNodes()[ aIdx ]->GetTxtNode();
            }
        }
    }
    return FALSE;
}

    // AutoFormat fuer die Tabelle/TabellenSelection
BOOL SwFEShell::SetTableAutoFmt( const SwTableAutoFmt& rNew )
{
    SwTableNode *pTblNd = (SwTableNode*)IsCrsrInTbl();
    if( !pTblNd || pTblNd->GetTable().IsTblComplex() )
        return FALSE;

    SwSelBoxes aBoxes;

    if ( !IsTableMode() )       // falls Crsr noch nicht akt. sind
        GetCrsr();

    // gesamte Tabelle oder nur auf die akt. Selektion
    if( IsTableMode() )
        ::GetTblSelCrs( *this, aBoxes );
    else
    {
        const SwTableSortBoxes& rTBoxes = pTblNd->GetTable().GetTabSortBoxes();
        for( USHORT n = 0; n < rTBoxes.Count(); ++n )
        {
            SwTableBox* pBox = rTBoxes[ n ];
            aBoxes.Insert( pBox );
        }
    }

    BOOL bRet;
    if( aBoxes.Count() )
    {
        SET_CURR_SHELL( this );
        StartAllAction();
        bRet = GetDoc()->SetTableAutoFmt( aBoxes, rNew );
        DELETEZ( pLastCols );
        DELETEZ( pLastRows );
        EndAllActionAndCall();
    }
    else
        bRet = FALSE;
    return bRet;
}

BOOL SwFEShell::GetTableAutoFmt( SwTableAutoFmt& rGet )
{
    const SwTableNode *pTblNd = IsCrsrInTbl();
    if( !pTblNd || pTblNd->GetTable().IsTblComplex() )
        return FALSE;

    SwSelBoxes aBoxes;

    if ( !IsTableMode() )       // falls Crsr noch nicht akt. sind
        GetCrsr();

    // gesamte Tabelle oder nur auf die akt. Selektion
    if( IsTableMode() )
        ::GetTblSelCrs( *this, aBoxes );
    else
    {
        const SwTableSortBoxes& rTBoxes = pTblNd->GetTable().GetTabSortBoxes();
        for( USHORT n = 0; n < rTBoxes.Count(); ++n )
        {
            SwTableBox* pBox = rTBoxes[ n ];
            aBoxes.Insert( pBox );
        }
    }

    return GetDoc()->GetTableAutoFmt( aBoxes, rGet );
}

/***********************************************************************
#*  Class      :  SwFEShell
#*  Methoden   :  DeleteTblSel()
#*  Datum      :  MA 03. May. 93
#*  Update     :  MA 19. Apr. 95
#***********************************************************************/
BOOL SwFEShell::DeleteTblSel()
{
    // pruefe ob vom aktuellen Crsr der SPoint/Mark in einer Tabelle stehen
    SwFrm *pFrm = GetCurrFrm();
    if( !pFrm->IsInTab() )
        return FALSE;

    if( pFrm->ImplFindTabFrm()->GetTable()->ISA( SwDDETable ))
    {
        ErrorHandler::HandleError( ERR_TBLDDECHG_ERROR,
                        ERRCODE_MSG_INFO | ERRCODE_BUTTON_DEF_OK );
        return FALSE;
    }

    SET_CURR_SHELL( this );
    StartAllAction();

    // lasse ueber das Layout die Boxen suchen
    BOOL bRet;
    SwSelBoxes aBoxes;
    GetTblSelCrs( *this, aBoxes );
    if( aBoxes.Count() )
    {
        TblWait( aBoxes.Count(), pFrm, *GetDoc()->GetDocShell() );

        // die Crsr muessen noch aus dem Loesch Bereich entfernt
        // werden. Setze sie immer hinter/auf die Tabelle; ueber die
        // Dokument-Position werden sie dann immer an die alte Position gesetzt.
        while( !pFrm->IsCellFrm() )
            pFrm = pFrm->GetUpper();
        ParkCrsr( SwNodeIndex( *((SwCellFrm*)pFrm)->GetTabBox()->GetSttNd() ));

        bRet = GetDoc()->DeleteRowCol( aBoxes );

        DELETEZ( pLastCols );
        DELETEZ( pLastRows );
    }
    else
        bRet = FALSE;
    EndAllActionAndCall();
    return bRet;
}

/*************************************************************************
|*
|*  SwFEShell::GetCurTabColNum()
|*
|*  Ersterstellung      MA 03. Feb. 95
|*  Letzte Aenderung    MA 21. May. 95
|
|*************************************************************************/
USHORT SwFEShell::GetCurTabColNum() const
{
    //!!!GetCurMouseTabColNum() mitpflegen!!!!
    USHORT nRet = 0;

    SwFrm *pFrm = GetCurrFrm();
    ASSERT( pFrm, "Crsr geparkt?" );

    // pruefe ob vom aktuellen Crsr der SPoint/Mark in einer Tabelle stehen
    if( pFrm->IsInTab() )
    {
        do {            // JP 26.09.95: warum mit dem CntntFrame und nicht mit
                        //              dem CellFrame vergleichen????
            pFrm = pFrm->GetUpper();
        } while ( !pFrm->IsCellFrm() );
        SWRECTFN( pFrm )
        const long nX = (pFrm->Frm().*fnRect->fnGetLeft)();

        //TabCols besorgen, den nur ueber diese erreichen wir die Position.
        SwTabCols aTabCols;
        GetTabCols( aTabCols );

        if( pFrm->FindTabFrm()->IsRightToLeft() )
        {
            long nX = (pFrm->Frm().*fnRect->fnGetRight)();
            const long nRight = aTabCols.GetLeftMin() + aTabCols.GetRight();;

            if ( !::IsSame( nX, nRight ) )
            {
                nX = nRight - nX + aTabCols.GetLeft();
                for ( USHORT i = 0; i < aTabCols.Count(); ++i )
                    if ( ::IsSame( nX, aTabCols[i] ) )
                    {
                        nRet = i + 1;
                        break;
                    }
            }
        }
        else
        {
            const long nX = (pFrm->Frm().*fnRect->fnGetLeft)();
            const long nLeft = aTabCols.GetLeftMin();

            if ( !::IsSame( nX, nLeft + aTabCols.GetLeft() ) )
            {
                for ( USHORT i = 0; i < aTabCols.Count(); ++i )
                    if ( ::IsSame( nX, nLeft + aTabCols[i] ) )
                    {
                        nRet = i + 1;
                        break;
                    }
            }
        }
    }
    return nRet;
}

/*************************************************************************
|*
|*  SwFEShell::GetBox()
|*
|*  Ersterstellung      MA 22. Jun. 95
|*  Letzte Aenderung    MA 21. Nov. 96
|*
|*************************************************************************/

const SwFrm *lcl_FindFrmInTab( const SwLayoutFrm *pLay, const Point &rPt, SwTwips nFuzzy )
{
    const SwFrm *pFrm = pLay->Lower();

    while( pFrm && pLay->IsAnLower( pFrm ) )
    {
        if ( pFrm->Frm().IsNear( rPt, nFuzzy ) )
        {
            if ( pFrm->IsCellFrm() && ((SwCellFrm*)pFrm)->Lower() &&
                 ( !((SwCellFrm*)pFrm)->Lower()->IsLayoutFrm() ||
                    ((SwCellFrm*)pFrm)->Lower()->IsSctFrm() ) )
                return pFrm;
            if ( pFrm->IsLayoutFrm() )
            {
                const SwFrm *pTmp = ::lcl_FindFrmInTab( (SwLayoutFrm*)pFrm, rPt, nFuzzy);
                if ( pTmp )
                    return pTmp;
            }
            break;
        }
        pFrm = pFrm->FindNext();
    };

    return 0;
}

const SwFrm *lcl_FindFrm( const SwLayoutFrm *pLay, const Point &rPt,
                          SwTwips nFuzzy, bool* pbRow )
{
    const SwFrm *pFrm = pLay->ContainsCntnt();
    if ( pFrm )
    {
        do
        {
            if ( pFrm->IsInTab() )
                pFrm = ((SwFrm*)pFrm)->ImplFindTabFrm();
            if ( pFrm->IsTabFrm() )
            {
                const SwFrm *pTmp = ::lcl_FindFrmInTab( (SwLayoutFrm*)pFrm, rPt, nFuzzy );
                if ( pTmp )
                {
                    pFrm = pTmp;
                    break;
                }
            }
            pFrm = pFrm->FindNextCnt();

        } while ( pFrm && pLay->IsAnLower( pFrm ) );
    }
    if ( pFrm && pFrm->IsInTab() && pLay->IsAnLower( pFrm ) )
    {
        do
        {   while ( pFrm && !pFrm->IsCellFrm() )
                pFrm = pFrm->GetUpper();
            if ( pFrm )
            {
                if ( ::IsSame(pFrm->Frm().Left(), rPt.X()) ||
                     ::IsSame(pFrm->Frm().Right(),rPt.X()) )
                {
                    if ( pbRow ) *pbRow = false;
                    return pFrm;
                }
                if ( ::IsSame(pFrm->Frm().Top(), rPt.Y()) ||
                     ::IsSame(pFrm->Frm().Bottom(),rPt.Y()) )
                {
                    if ( pbRow ) *pbRow = true;
                    return pFrm;
                }
                pFrm = pFrm->GetUpper();
            }
        } while ( pFrm );
    }
    return 0;
}

const SwFrm *SwFEShell::GetBox( const Point &rPt, bool* pbRow ) const
{
    const SwPageFrm *pPage = (SwPageFrm*)GetLayout()->Lower();
    Window* pOutWin = GetWin();
    SwTwips nFuzzy = COLFUZZY;
    if(pOutWin)
    {
        Size aTmp(RULER_MOUSE_MARGINWIDTH, RULER_MOUSE_MARGINWIDTH);
        aTmp = pOutWin->PixelToLogic(aTmp);
        nFuzzy = aTmp.Width();
    }
    while ( pPage && !pPage->Frm().IsNear( rPt, nFuzzy ) )
        pPage = (SwPageFrm*)pPage->GetNext();

    const SwFrm *pFrm = 0;
    if ( pPage )
    {
        //Per GetCrsrOfst oder GetCntntPos koennen wir hier die Box leider
        //nicht suchen. Das wuerde zu einem Performance-Zusammenbruch bei
        //Dokumenten mit vielen Absaetzen/Tabellen auf einer Seite fuehren
        //(BrowseMode!)

        //Erst die Flys checken.
        if ( pPage->GetSortedObjs() )
        {
            for ( USHORT i = 0; !pFrm && i < pPage->GetSortedObjs()->Count(); ++i )
            {
                SwAnchoredObject* pObj = (*pPage->GetSortedObjs())[i];
                if ( pObj->ISA(SwFlyFrm) )
                {
                    pFrm = lcl_FindFrm( static_cast<SwFlyFrm*>(pObj),
                                        rPt, nFuzzy, pbRow );
                }
            }
        }
        const SwLayoutFrm *pLay = (SwLayoutFrm*)pPage->Lower();
        while ( pLay && !pFrm )
        {
            pFrm = lcl_FindFrm( pLay, rPt, nFuzzy, pbRow );
            pLay = (SwLayoutFrm*)pLay->GetNext();
        }
    }
    return pFrm;
}


/*************************************************************************
|*
|*  SwFEShell::WhichMouseTabCol()
|*
|*  Ersterstellung      MA 22. Jun. 95
|*  Last change         AMA 12. Jun. 02
|
|*************************************************************************/
BYTE SwFEShell::WhichMouseTabCol( const Point &rPt ) const
{
    bool bRow;
    SwCellFrm* pFrm = (SwCellFrm*)GetBox( rPt, &bRow );
    if( pFrm )
    {
        while( pFrm->Lower()->IsRowFrm() )
            pFrm = (SwCellFrm*)((SwLayoutFrm*)pFrm->Lower())->Lower();
        if( pFrm && pFrm->GetTabBox()->GetSttNd() &&
            pFrm->GetTabBox()->GetSttNd()->IsInProtectSect() )
            pFrm = 0;
    }
    if( pFrm )
    {
        if ( pFrm->IsVertical() )
            return bRow ? SW_TABCOL_VERT : SW_TABROW_VERT;
        else
            return bRow ? SW_TABROW_HORI : SW_TABCOL_HORI;
    }
    return SW_TABCOL_NONE;
}

// -> #i23726#
SwTxtNode * SwFEShell::GetNumRuleNodeAtPos( const Point &rPt)
{
    SwTxtNode * pResult = NULL;

    SwContentAtPos aCntntAtPos
        (SwContentAtPos::SW_NUMLABEL);

    if( GetContentAtPos(rPt, aCntntAtPos) && aCntntAtPos.aFnd.pNode)
        pResult = aCntntAtPos.aFnd.pNode->GetTxtNode();

    return pResult;
}

BOOL SwFEShell::IsNumLabel( const Point &rPt, int nMaxOffset )
{
    BOOL bResult = FALSE;

    SwContentAtPos aCntntAtPos
        (SwContentAtPos::SW_NUMLABEL);

    if( GetContentAtPos(rPt, aCntntAtPos))
    {
        if ((nMaxOffset >= 0 && aCntntAtPos.nDist <= nMaxOffset) ||
            (nMaxOffset < 0))
            bResult = TRUE;
    }

    return bResult;
}
// <- #i23726#

/*************************************************************************
|*
|*  SwFEShell::GetMouseTabCols()
|*
|*  Ersterstellung      MA 22. Jun. 95
|*  Letzte Aenderung    MA 27. Aug. 96
|
|*************************************************************************/
void SwFEShell::GetMouseTabCols( SwTabCols &rToFill, const Point &rPt ) const
{
    const SwFrm *pBox = GetBox( rPt );
    if ( pBox )
        _GetTabCols( rToFill, pBox );
}

void SwFEShell::SetMouseTabCols( const SwTabCols &rNew, BOOL bCurRowOnly,
                                 const Point &rPt )
{
    const SwFrm *pBox = GetBox( rPt );
    if( pBox )
    {
        SET_CURR_SHELL( this );
        StartAllAction();
        GetDoc()->SetTabCols( rNew, bCurRowOnly, 0, (SwCellFrm*)pBox );
        EndAllActionAndCall();
    }
}

/*************************************************************************
|*
|*  SwFEShell::GetMouseColNum(), GetMouseTabColNum()
|*
|*  Ersterstellung      MA 04. Jul. 95
|*  Letzte Aenderung    MA 04. Jul. 95
|
|*************************************************************************/
USHORT SwFEShell::GetCurMouseColNum( const Point &rPt,
                                    SwGetCurColNumPara* pPara ) const
{
    return _GetCurColNum( GetBox( rPt ), pPara );
}

USHORT SwFEShell::GetCurMouseTabColNum( const Point &rPt ) const
{
    //!!!GetCurTabColNum() mitpflegen!!!!
    USHORT nRet = 0;

    const SwFrm *pFrm = GetBox( rPt );
    ASSERT( pFrm, "Table not found" );
    if( pFrm )
    {
        const long nX = pFrm->Frm().Left();

        //TabCols besorgen, den nur ueber diese erreichen wir die Position.
        SwTabCols aTabCols;
        GetMouseTabCols( aTabCols, rPt );

        const long nLeft = aTabCols.GetLeftMin();

        if ( !::IsSame( nX, nLeft + aTabCols.GetLeft() ) )
        {
            for ( USHORT i = 0; i < aTabCols.Count(); ++i )
                if ( ::IsSame( nX, nLeft + aTabCols[i] ) )
                {
                    nRet = i + 1;
                    break;
                }
        }
    }
    return nRet;
}

void ClearFEShellTabCols()
{
    DELETEZ( pLastCols );
    DELETEZ( pLastRows );
}

/*************************************************************************
|*
|*  SwFEShell::GetTblAttr(), SetTblAttr()
|*
|*  Ersterstellung      MA 09. Dec. 96
|*  Letzte Aenderung    MA 09. Dec. 96
|
|*************************************************************************/
void SwFEShell::GetTblAttr( SfxItemSet &rSet ) const
{
    SwFrm *pFrm = GetCurrFrm();
    if( pFrm->IsInTab() )
        rSet.Put( pFrm->ImplFindTabFrm()->GetFmt()->GetAttrSet() );
}

void SwFEShell::SetTblAttr( const SfxItemSet &rNew )
{
    SwFrm *pFrm = GetCurrFrm();
    if( pFrm->IsInTab() )
    {
        SET_CURR_SHELL( this );
        StartAllAction();
        SwTabFrm *pTab = pFrm->FindTabFrm();
        pTab->GetTable()->SetHTMLTableLayout( 0 );
        GetDoc()->SetAttr( rNew, *pTab->GetFmt() );
        GetDoc()->SetModified();
        EndAllActionAndCall();
    }
}

/** move cursor within a table into previous/next row (same column)
 * @param pShell cursor shell whose cursor is to be moved
 * @param bUp true: move up, false: move down
 * @returns true if successful
 */
bool lcl_GoTableRow( SwCrsrShell* pShell, bool bUp )
{
    ASSERT( pShell != NULL, "need shell" );

    bool bRet = false;

    SwPaM* pPam = pShell->GetCrsr();
    const SwStartNode* pTableBox = pPam->GetNode()->FindTableBoxStartNode();
    ASSERT( pTableBox != NULL, "I'm living in a box... NOT!" );

    // move cursor to start node of table box
    pPam->GetPoint()->nNode = pTableBox->GetIndex();
    pPam->GetPoint()->nContent.Assign( NULL, 0 );
    GoInCntnt( *pPam, fnMoveForward );

    // go to beginning end of table box
    SwPosSection fnPosSect = bUp ? fnSectionStart : fnSectionEnd;
    pShell->MoveSection( fnSectionCurr, fnPosSect );

    // and go up/down into next content
    bRet = bUp ? pShell->Up() : pShell->Down();

    return bRet;
}

    // aender eine  Zellenbreite/-Hoehe/Spaltenbreite/Zeilenhoehe
BOOL SwFEShell::SetColRowWidthHeight( USHORT eType, USHORT nDiff )
{
    SwFrm *pFrm = GetCurrFrm();
    if( !pFrm->IsInTab() )
        return FALSE;

    if( WH_FLAG_INSDEL & eType &&
        pFrm->ImplFindTabFrm()->GetTable()->ISA( SwDDETable ))
    {
        ErrorHandler::HandleError( ERR_TBLDDECHG_ERROR,
                        ERRCODE_MSG_INFO | ERRCODE_BUTTON_DEF_OK );
        return FALSE;
    }

    SET_CURR_SHELL( this );
    StartAllAction();

    do {
        pFrm = pFrm->GetUpper();
    } while( !pFrm->IsCellFrm() );

    SwTabFrm *pTab = pFrm->ImplFindTabFrm();

    // sollte die Tabelle noch auf relativen Werten (USHRT_MAX) stehen
    // dann muss es jetzt auf absolute umgerechnet werden.
    const SwFmtFrmSize& rTblFrmSz = pTab->GetFmt()->GetFrmSize();
    SWRECTFN( pTab )
    long nPrtWidth = (pTab->Prt().*fnRect->fnGetWidth)();
    if( TBLVAR_CHGABS == pTab->GetTable()->GetTblChgMode() &&
        ( eType & WH_COL_LEFT || eType & WH_COL_RIGHT ) &&
        HORI_NONE == pTab->GetFmt()->GetHoriOrient().GetHoriOrient() &&
        nPrtWidth != rTblFrmSz.GetWidth() )
    {
        SwFmtFrmSize aSz( rTblFrmSz );
        aSz.SetWidth( pTab->Prt().Width() );
        pTab->GetFmt()->SetAttr( aSz );
    }

    if( (eType & (WH_FLAG_BIGGER | WH_FLAG_INSDEL)) ==
        (WH_FLAG_BIGGER | WH_FLAG_INSDEL) )
    {
        nDiff = USHORT((pFrm->Frm().*fnRect->fnGetWidth)());

        // we must move the cursor outside the current cell before
        // deleting the cells.
        TblChgWidthHeightType eTmp =
            static_cast<TblChgWidthHeightType>( eType & 0xfff );
        switch( eTmp )
        {
        case WH_ROW_TOP:
            lcl_GoTableRow( this, true );
            break;
        case WH_ROW_BOTTOM:
            lcl_GoTableRow( this, false );
            break;
        case WH_COL_LEFT:
            GoPrevCell();
            break;
        case WH_COL_RIGHT:
            GoNextCell();
            break;
        default:
            break;
        }
    }

    SwTwips nLogDiff = nDiff;
    nLogDiff *= pTab->GetFmt()->GetFrmSize().GetWidth();
    nLogDiff /= nPrtWidth;

    /** The cells are destroyed in here */
    BOOL bRet = GetDoc()->SetColRowWidthHeight(
                    *(SwTableBox*)((SwCellFrm*)pFrm)->GetTabBox(),
                    eType, nDiff, nLogDiff );

    delete pLastCols, pLastCols = 0;
    EndAllActionAndCall();

    if( bRet && (eType & (WH_FLAG_BIGGER | WH_FLAG_INSDEL)) == WH_FLAG_INSDEL )
    {
        switch(eType & ~(WH_FLAG_BIGGER | WH_FLAG_INSDEL))
        {
        case WH_CELL_LEFT:
        case WH_COL_LEFT:
                GoPrevCell();
                break;

        case WH_CELL_RIGHT:
        case WH_COL_RIGHT:
                GoNextCell();
                break;

        case WH_CELL_TOP:
        case WH_ROW_TOP:
                lcl_GoTableRow( this, true );
                break;

        case WH_CELL_BOTTOM:
        case WH_ROW_BOTTOM:
                lcl_GoTableRow( this, false );
                break;
        }
    }

    return bRet;
}

BOOL lcl_IsFormulaSelBoxes( const SwTable& rTbl, const SwTblBoxFormula& rFml,
                            SwCellFrms& rCells )
{
    SwTblBoxFormula aTmp( rFml );
    SwSelBoxes aBoxes;
    for( USHORT nSelBoxes = aTmp.GetBoxesOfFormula( rTbl,aBoxes ); nSelBoxes; )
    {
        SwTableBox* pBox = aBoxes[ --nSelBoxes ];
        USHORT i;
        for( i = 0; i < rCells.Count(); ++i )
            if( rCells[ i ]->GetTabBox() == pBox )
                break;      // gefunden

        if( i == rCells.Count() )
            return FALSE;
    }

    return TRUE;
}

    // erfrage die Formel fuer die Autosumme
BOOL SwFEShell::GetAutoSum( String& rFml ) const
{
    SwFrm *pFrm = GetCurrFrm();
    SwTabFrm *pTab = pFrm->ImplFindTabFrm();
    if( !pTab )
        return FALSE;

    rFml = String::CreateFromAscii( sCalc_Sum );

    SwCellFrms aCells;
    if( ::GetAutoSumSel( *this, aCells ))
    {
        USHORT nW = 0, nInsPos = 0;
        for( USHORT n = aCells.Count(); n; )
        {
            SwCellFrm* pCFrm = aCells[ --n ];
            USHORT nBoxW = pCFrm->GetTabBox()->IsFormulaOrValueBox();
            if( !nBoxW )
                break;

            if( !nW )
            {
                if( USHRT_MAX == nBoxW )
                    continue;       // leere am Anfang ueberspringen

                rFml += '(';
                nInsPos = rFml.Len();

                // Formeln nur wenn diese Boxen enthalten
                if( RES_BOXATR_FORMULA == nBoxW &&
                    !::lcl_IsFormulaSelBoxes( *pTab->GetTable(), pCFrm->
                    GetTabBox()->GetFrmFmt()->GetTblBoxFormula(), aCells))
                {
                    nW = RES_BOXATR_VALUE;
                    // alle vorhierigen Leere wieder mit aufnehmen !
                    for( USHORT i = aCells.Count(); n+1 < i; )
                    {
                        String sTmp( String::CreateFromAscii(
                                RTL_CONSTASCII_STRINGPARAM( "|<" )) );
                        sTmp += aCells[ --i ]->GetTabBox()->GetName();
                        sTmp += '>';
                        rFml.Insert( sTmp, nInsPos );
                    }
                }
                else
                    nW = nBoxW;
            }
            else if( RES_BOXATR_VALUE == nW )
            {
                // values werden gesucht, Value/Formel/Text gefunden -> aufn.
                if( RES_BOXATR_FORMULA == nBoxW &&
                    ::lcl_IsFormulaSelBoxes( *pTab->GetTable(), pCFrm->
                        GetTabBox()->GetFrmFmt()->GetTblBoxFormula(), aCells ))
                    break;
                else if( USHRT_MAX != nBoxW )
                    rFml.Insert( cListDelim, nInsPos );
                else
                    break;
            }
            else if( RES_BOXATR_FORMULA == nW )
            {
                // bei Formeln nur weiter suchen, wenn die akt. Formel auf
                // alle Boxen verweist, die sich in der Selektion befinden
                if( RES_BOXATR_FORMULA == nBoxW )
                {
                    if( !::lcl_IsFormulaSelBoxes( *pTab->GetTable(), pCFrm->
                        GetTabBox()->GetFrmFmt()->GetTblBoxFormula(), aCells ))
                    {
                        // dann noch mal von vorne und nur die Values!

                        nW = RES_BOXATR_VALUE;
                        rFml.Erase( nInsPos );
                        // alle vorhierigen Leere wieder mit aufnehmen !
                        for( USHORT i = aCells.Count(); n+1 < i; )
                        {
                            String sTmp( String::CreateFromAscii(
                                    RTL_CONSTASCII_STRINGPARAM( "|<" )) );
                            sTmp += aCells[ --i ]->GetTabBox()->GetName();
                            sTmp += '>';
                            rFml.Insert( sTmp, nInsPos );
                        }
                    }
                    else
                        rFml.Insert( cListDelim, nInsPos );
                }
                else if( USHRT_MAX == nBoxW )
                    break;
                else
                    continue;       // diese Boxen ignorieren
            }
            else
                // alles andere beendet die Schleife
// evt. Texte noch zu lassen??
                break;

            String sTmp( '<' );
            sTmp += pCFrm->GetTabBox()->GetName();
            sTmp += '>';
            rFml.Insert( sTmp, nInsPos );
        }
        if( nW )
        {
            rFml += ')';

/*
            // TabellenSelektion erzeugen??
            SwTblBoxFormula aTmp( rFml );
            SwSelBoxes aBoxes;
            for( USHORT nSelBoxes = aTmp.GetBoxesOfFormula( rTbl,aBoxes );
                    nSelBoxes; )
            {
            }
*/
        }
    }

    return TRUE;
}
/* -----------------------------22.08.2002 12:50------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwFEShell::IsTableRightToLeft() const
{
    SwFrm *pFrm = GetCurrFrm();
    if( !pFrm->IsInTab() )
        return FALSE;

    return pFrm->ImplFindTabFrm()->IsRightToLeft();
}

/* -----------------------------22.08.2002 12:50------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwFEShell::IsMouseTableRightToLeft(const Point &rPt) const
{
    SwFrm *pFrm = (SwFrm *)GetBox( rPt );
    const SwTabFrm*  pTabFrm = pFrm ? pFrm->ImplFindTabFrm() : 0;
    ASSERT( pTabFrm, "Table not found" );
    return pTabFrm ? pTabFrm->IsRightToLeft() : FALSE;
}

/* -----------------------------11.02.2004 12:50------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwFEShell::IsTableVertical() const
{
    SwFrm *pFrm = GetCurrFrm();
    if( !pFrm->IsInTab() )
        return FALSE;

    return pFrm->ImplFindTabFrm()->IsVertical();
}



