/*************************************************************************
 *
 *  $RCSfile: tblsel.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:20 $
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

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _IPOBJ_HXX //autogen
#include <so3/ipobj.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _SCH_DLL_HXX //autogen
#include <sch/schdll.hxx>
#endif
#ifndef _SCH_MEMCHRT_HXX
#include <sch/memchrt.hxx>
#endif

#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _TBLSEL_HXX
#include <tblsel.hxx>
#endif
#ifndef _CRSRSH_HXX
#include <crsrsh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _NDOLE_HXX
#include <ndole.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
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
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _VISCRS_HXX
#include <viscrs.hxx>
#endif
#ifndef _SWCRSR_HXX
#include <swcrsr.hxx>
#endif
#ifndef _SWTBLFMT_HXX
#include <swtblfmt.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif
#ifndef _MVSAVE_HXX
#include <mvsave.hxx>
#endif

//siehe auch swtable.cxx
#define COLFUZZY 20L

// defines, die bestimmen, wie Tabellen Boxen gemergt werden:
//  - 1. alle leeren Zeilen entfernen, alle Boxen werden mit Blank,
//      alle Lines mit ParaBreak getrennt
//  - 2. alle leeren Zeilen und alle leeren Boxen am Anfang und Ende
//      entfernen, alle Boxen werden mit Blank,
//      alle Lines mit ParaBreak getrennt
//  - 3. alle leeren Boxen entfernen, alle Boxen werden mit Blank,
//      alle Lines mit ParaBreak getrennt

#undef      DEL_ONLY_EMPTY_LINES
#undef      DEL_EMPTY_BOXES_AT_START_AND_END
#define     DEL_ALL_EMPTY_BOXES


_SV_IMPL_SORTAR_ALG( SwSelBoxes, SwTableBoxPtr )
BOOL SwSelBoxes::Seek_Entry( const SwTableBoxPtr rSrch, USHORT* pFndPos ) const
{
    ULONG nIdx = rSrch->GetSttIdx();

    register USHORT nO = Count(), nM, nU = 0;
    if( nO > 0 )
    {
        nO--;
        while( nU <= nO )
        {
            nM = nU + ( nO - nU ) / 2;
            if( (*this)[ nM ]->GetSttNd() == rSrch->GetSttNd() )
            {
                if( pFndPos )
                    *pFndPos = nM;
                return TRUE;
            }
            else if( (*this)[ nM ]->GetSttIdx() < nIdx )
                nU = nM + 1;
            else if( nM == 0 )
            {
                if( pFndPos )
                    *pFndPos = nU;
                return FALSE;
            }
            else
                nO = nM - 1;
        }
    }
    if( pFndPos )
        *pFndPos = nU;
    return FALSE;
}


SV_IMPL_PTRARR( SwCellFrms, SwCellFrm* )

struct _CmpLPt
{
    const Point* pPos;
    const SwTableBox* pSelBox;

    _CmpLPt( const Point& rPt, const SwTableBox* pBox );

    BOOL operator==( const _CmpLPt& rCmp ) const
    {   return X() == rCmp.X() && Y() == rCmp.Y() ? TRUE : FALSE; }
    BOOL operator<( const _CmpLPt& rCmp ) const
    {   return Y() < rCmp.Y() || ( Y() == rCmp.Y() && X() < rCmp.X() )
                ? TRUE : FALSE; }

    long X() const { return pPos->X(); }
    long Y() const { return pPos->Y(); }
};

SV_DECL_VARARR_SORT( _MergePos, _CmpLPt, 0, 40 )
SV_IMPL_VARARR_SORT( _MergePos, _CmpLPt )

SV_IMPL_PTRARR( _FndBoxes, _FndBox* )
SV_IMPL_PTRARR( _FndLines, _FndLine* )


struct _Sort_CellFrm
{
    const SwCellFrm* pFrm;
    _Sort_CellFrm( const SwCellFrm& rCFrm ) : pFrm( &rCFrm ) {}

    int operator< ( const _Sort_CellFrm& rCmp ) const
    {
        return pFrm->Frm().Top() < rCmp.pFrm->Frm().Top() ||
                ( pFrm->Frm().Top() == rCmp.pFrm->Frm().Top() &&
                  pFrm->Frm().Left() < rCmp.pFrm->Frm().Left() );
    }
    int operator==( const _Sort_CellFrm& rCmp ) const
    {
        return pFrm->Frm().Top() == rCmp.pFrm->Frm().Top() &&
               pFrm->Frm().Left() == rCmp.pFrm->Frm().Left();
    }
};

SV_DECL_VARARR( _Sort_CellFrms, _Sort_CellFrm, 16, 16 )
SV_IMPL_VARARR( _Sort_CellFrms, _Sort_CellFrm )

SV_IMPL_PTRARR( SwChartBoxes, SwTableBoxPtr );
SV_IMPL_PTRARR( SwChartLines, SwChartBoxes* );

const SwLayoutFrm *lcl_FindCellFrm( const SwLayoutFrm *pLay )
{
    while ( pLay && !pLay->IsCellFrm() )
        pLay = pLay->GetUpper();
    return pLay;
}

const SwLayoutFrm *lcl_FindNextCellFrm( const SwLayoutFrm *pLay )
{
    //Dafuer sorgen, dass die Zelle auch verlassen wird (Bereiche)
    const SwLayoutFrm *pTmp = pLay;
    do {
        pTmp = pTmp->GetNextLayoutLeaf();
    } while( pLay->IsAnLower( pTmp ) );

    while( pTmp && !pTmp->IsCellFrm() )
        pTmp = pTmp->GetUpper();
    return pTmp;
}

void GetTblSelCrs( const SwCrsrShell &rShell, SwSelBoxes& rBoxes )
{
    if( rBoxes.Count() )
        rBoxes.Remove( USHORT(0), rBoxes.Count() );
    if( rShell.IsTableMode() && ((SwCrsrShell&)rShell).UpdateTblSelBoxes())
        rBoxes.Insert( &rShell.GetTableCrsr()->GetBoxes() );
}

void GetTblSelCrs( const SwTableCursor& rTblCrsr, SwSelBoxes& rBoxes )
{
    if( rBoxes.Count() )
        rBoxes.Remove( USHORT(0), rBoxes.Count() );

    if( rTblCrsr.IsChgd() || !rTblCrsr.GetBoxesCount() )
    {
        SwTableCursor* pTCrsr = (SwTableCursor*)&rTblCrsr;
        pTCrsr->GetDoc()->GetRootFrm()->MakeTblCrsrs( *pTCrsr );
    }

    if( rTblCrsr.GetBoxesCount() )
        rBoxes.Insert( &rTblCrsr.GetBoxes() );
}

void GetTblSel( const SwCrsrShell& rShell, SwSelBoxes& rBoxes,
                const SwTblSearchType eSearchType )
{
    //Start- und Endzelle besorgen und den naechsten fragen.
    if ( !rShell.IsTableMode() )
        rShell.GetCrsr();

    const SwShellCrsr *pCrsr = rShell.GetTableCrsr();
    if( !pCrsr )
        pCrsr = (SwShellCrsr*)*rShell.GetSwCrsr( FALSE );

    GetTblSel( *pCrsr, rBoxes, eSearchType );
}

void GetTblSel( const SwCursor& rCrsr, SwSelBoxes& rBoxes,
                const SwTblSearchType eSearchType )
{
    //Start- und Endzelle besorgen und den naechsten fragen.
    ASSERT( rCrsr.GetCntntNode() && rCrsr.GetCntntNode( FALSE ),
            "Tabselection nicht auf Cnt." );

    // Zeilen-Selektion:
    // teste ob Tabelle komplex ist. Wenn ja, dann immer uebers Layout
    // die selektierten Boxen zusammen suchen. Andernfalls ueber die
    // Tabellen-Struktur (fuer Makros !!)
    const SwTableNode* pTblNd;
    if( TBLSEARCH_ROW == ((~TBLSEARCH_PROTECT ) & eSearchType ) &&
        0 != ( pTblNd = rCrsr.GetNode()->FindTableNode() ) &&
        !pTblNd->GetTable().IsTblComplex() )
    {
        const SwTable& rTbl = pTblNd->GetTable();
        const SwTableLines& rLines = rTbl.GetTabLines();
        const SwTableLine* pLine = rTbl.GetTblBox(  rCrsr.GetNode(
                    FALSE )->StartOfSectionIndex() )->GetUpper();
        USHORT nSttPos = rLines.GetPos( pLine );
        ASSERT( USHRT_MAX != nSttPos, "Wo ist meine Zeile in der Tabelle?" );

        pLine = rTbl.GetTblBox( rCrsr.GetNode( TRUE )->StartOfSectionIndex() )
                                ->GetUpper();
        USHORT nEndPos = rLines.GetPos( pLine );
        ASSERT( USHRT_MAX != nEndPos, "Wo ist meine Zeile in der Tabelle?" );

        if( nEndPos < nSttPos )     // vertauschen
        {
            USHORT nTmp = nSttPos; nSttPos = nEndPos; nEndPos = nTmp;
        }

        int bChkProtected = TBLSEARCH_PROTECT & eSearchType;
        for( ; nSttPos <= nEndPos; ++nSttPos )
        {
            pLine = rLines[ nSttPos ];
            for( USHORT n = pLine->GetTabBoxes().Count(); n ; )
            {
                SwTableBox* pBox = pLine->GetTabBoxes()[ --n ];
                // Zellenschutzt beachten ??
                if( !bChkProtected ||
                    !pBox->GetFrmFmt()->GetProtect().IsCntntProtected() )
                    rBoxes.Insert( pBox );
            }
        }
    }
    else
    {
        Point aPtPos, aMkPos;
        const SwShellCrsr* pShCrsr = rCrsr;
        if( pShCrsr )
        {
            aPtPos = pShCrsr->GetPtPos();
            aMkPos = pShCrsr->GetMkPos();
        }
        const SwLayoutFrm *pStart = rCrsr.GetCntntNode()->GetFrm(
                                    &aPtPos )->GetUpper(),
                          *pEnd   = rCrsr.GetCntntNode(FALSE)->GetFrm(
                                    &aMkPos )->GetUpper();
        GetTblSel( pStart, pEnd, rBoxes, eSearchType );
    }
}

void GetTblSel( const SwLayoutFrm* pStart, const SwLayoutFrm* pEnd,
                SwSelBoxes& rBoxes, const SwTblSearchType eSearchType )
{
    //Muss ein HeadlineRepeat beachtet werden?
    const BOOL bRepeat = pStart->FindTabFrm()->GetTable()->IsHeadlineRepeat();
    int bChkProtected = TBLSEARCH_PROTECT & eSearchType;

    BOOL bTblIsValid;
    int nLoopMax = 100;     //JP 28.06.99: max 100 loops - Bug 67292
    do {
        bTblIsValid = TRUE;

        //Zuerst lassen wir uns die Tabellen und die Rechtecke heraussuchen.
        SwSelUnions aUnions;
        ::MakeSelUnions( aUnions, pStart, pEnd, eSearchType );

        //Jetzt zu jedem Eintrag die Boxen herausfischen und uebertragen.
        for( USHORT i = 0; i < aUnions.Count() && bTblIsValid; ++i )
        {
            SwSelUnion *pUnion = aUnions[i];
            const SwTabFrm *pTable = pUnion->GetTable();

            if( !pTable->IsValid() && nLoopMax )
            {
                bTblIsValid = FALSE;
                break;
            }

            const SwLayoutFrm *pRow = (const SwLayoutFrm*)pTable->Lower();
            //Wenn die Row eine wiederholte Headline ist wird sie nicht
            //beachtet.
            if( bRepeat && pTable->IsFollow() )
                pRow = (const SwLayoutFrm*)pRow->GetNext();

            while( pRow && bTblIsValid )
            {
                if( !pRow->IsValid() && nLoopMax )
                {
                    bTblIsValid = FALSE;
                    break;
                }

                if ( pRow->Frm().IsOver( pUnion->GetUnion() ) )
                {
                    const SwLayoutFrm *pCell = pRow->FirstCell();

                    while( bTblIsValid && pCell && pRow->IsAnLower( pCell ) )
                    {
                        if( !pCell->IsValid() && nLoopMax )
                        {
                            bTblIsValid = FALSE;
                            break;
                        }

                        ASSERT( pCell->IsCellFrm(), "Frame ohne Celle" );
                        if( ::IsFrmInTblSel( pUnion->GetUnion(), pCell ) )
                        {
                            SwTableBox* pBox = (SwTableBox*)
                                ((SwCellFrm*)pCell)->GetTabBox();
                            // Zellenschutzt beachten ??
                            if( !bChkProtected ||
                                !pBox->GetFrmFmt()->GetProtect().IsCntntProtected() )
                                rBoxes.Insert( pBox );
                        }
                        if ( pCell->GetNext() )
                        {
                            pCell = (const SwLayoutFrm*)pCell->GetNext();
                            if ( pCell->Lower()->IsRowFrm() )
                                pCell = pCell->FirstCell();
                        }
                        else
                            pCell = ::lcl_FindNextCellFrm( pCell );
                    }
                }
                pRow = (const SwLayoutFrm*)pRow->GetNext();
            }
        }

        if( bTblIsValid )
            break;

        // ansonsten das Layout der Tabelle kurz "kalkulieren" lassen
        // und nochmals neu aufsetzen

        for( i = 0; i < aUnions.Count(); ++i )
        {
            SwTabFrm *pTable = aUnions[i]->GetTable();
            if( pTable->IsValid() )
                pTable->InvalidatePos();
            pTable->SetONECalcLowers();
            pTable->Calc();
            pTable->SetCompletePaint();
        }
        i = 0;
        rBoxes.Remove( i, rBoxes.Count() );
        --nLoopMax;

    } while( TRUE );
    ASSERT( nLoopMax, "das Layout der Tabelle wurde nicht valide!" );
}



BOOL ChkChartSel( const SwNode& rSttNd, const SwNode& rEndNd,
                    SwChartLines* pGetCLines )
{
    const SwTableNode* pTNd = rSttNd.FindTableNode();
    if( !pTNd )
        return FALSE;

    Point aNullPos;
    SwNodeIndex aIdx( rSttNd );
    const SwCntntNode* pCNd = aIdx.GetNode().GetCntntNode();
    if( !pCNd )
        pCNd = aIdx.GetNodes().GoNextSection( &aIdx, TRUE, FALSE );

    const SwLayoutFrm *pStart = pCNd ? pCNd->GetFrm( &aNullPos )->GetUpper() : 0;
    ASSERT( pStart, "ohne Frame geht gar nichts" );

    aIdx = rEndNd;
    pCNd = aIdx.GetNode().GetCntntNode();
    if( !pCNd )
        pCNd = aIdx.GetNodes().GoNextSection( &aIdx, TRUE, FALSE );

    const SwLayoutFrm *pEnd = pCNd ? pCNd->GetFrm( &aNullPos )->GetUpper() : 0;
    ASSERT( pEnd, "ohne Frame geht gar nichts" );


    //Muss ein HeadlineRepeat beachtet werden?
    const BOOL bRepeat = pTNd->GetTable().IsHeadlineRepeat();

    BOOL bTblIsValid, bValidChartSel;
    int nLoopMax = 100;     //JP 28.06.99: max 100 loops - Bug 67292
    do {
        bTblIsValid = TRUE;
        bValidChartSel = TRUE;

        USHORT nRowCells = USHRT_MAX;

        //Zuerst lassen wir uns die Tabellen und die Rechtecke heraussuchen.
        SwSelUnions aUnions;
        ::MakeSelUnions( aUnions, pStart, pEnd, TBLSEARCH_NO_UNION_CORRECT );

        //Jetzt zu jedem Eintrag die Boxen herausfischen und uebertragen.
        for( USHORT i = 0; i < aUnions.Count() && bTblIsValid &&
                                    bValidChartSel; ++i )
        {
            SwSelUnion *pUnion = aUnions[i];
            const SwTabFrm *pTable = pUnion->GetTable();

            if( !pTable->IsValid() && nLoopMax  )
            {
                bTblIsValid = FALSE;
                break;
            }

            _Sort_CellFrms aCellFrms;

            const SwLayoutFrm *pRow = (const SwLayoutFrm*)pTable->Lower();
            //Wenn die Row eine wiederholte Headline ist wird sie nicht
            //beachtet.
            if( bRepeat && pTable->IsFollow() )
                pRow = (const SwLayoutFrm*)pRow->GetNext();

            while( pRow && bTblIsValid && bValidChartSel )
            {
                if( !pRow->IsValid() && nLoopMax )
                {
                    bTblIsValid = FALSE;
                    break;
                }

                if( pRow->Frm().IsOver( pUnion->GetUnion() ) )
                {
                    const SwLayoutFrm *pCell = pRow->FirstCell();

                    while( bValidChartSel && bTblIsValid && pCell &&
                            pRow->IsAnLower( pCell ) )
                    {
                        if( !pCell->IsValid() && nLoopMax  )
                        {
                            bTblIsValid = FALSE;
                            break;
                        }

                        ASSERT( pCell->IsCellFrm(), "Frame ohne Celle" );
                        const SwRect& rUnion = pUnion->GetUnion(),
                                    & rFrmRect = pCell->Frm();

                        const long nUnionRight = rUnion.Right();
                        const long nUnionBottom = rUnion.Bottom();
                        const long nFrmRight = rFrmRect.Right();
                        const long nFrmBottom = rFrmRect.Bottom();

                        // liegt das FrmRect ausserhalb der Union, kann es
                        // ignoriert werden.
                        if( !(  rUnion.Top() > nFrmBottom ||
                                nUnionBottom < rFrmRect.Top() ||
                                rUnion.Left() + 20 > nFrmRight ||
                                nUnionRight < rFrmRect.Left() + 20 ))
                        {
                            // wenn es aber nicht komplett in der Union liegt,
                            // dann ist es fuers Chart eine ungueltige
                            // Selektion.
                            if( rUnion.Left()   <= rFrmRect.Left() + 20 &&
                                rFrmRect.Left() <= nUnionRight &&
                                rUnion.Left()   <= nFrmRight &&
                                nFrmRight       <= nUnionRight + 20 &&
                                rUnion.Top()    <= rFrmRect.Top() &&
                                rFrmRect.Top()  <= nUnionBottom &&
                                rUnion.Top()    <= nFrmBottom &&
                                nFrmBottom      <= nUnionBottom )

                                aCellFrms.Insert(
                                        _Sort_CellFrm( *(SwCellFrm*)pCell ),
                                        aCellFrms.Count() );
                            else
                            {
                                bValidChartSel = FALSE;
                                break;
                            }
                        }
                        if ( pCell->GetNext() )
                        {
                            pCell = (const SwLayoutFrm*)pCell->GetNext();
                            if ( pCell->Lower()->IsRowFrm() )
                                pCell = pCell->FirstCell();
                        }
                        else
                            pCell = ::lcl_FindNextCellFrm( pCell );
                    }
                }
                pRow = (const SwLayoutFrm*)pRow->GetNext();
            }

            if( !bValidChartSel )
                break;

            // alle Zellen der (Teil-)Tabelle zusammen. Dann teste mal ob
            // all huebsch nebeneinander liegen.
            USHORT n, nEnd, nCellCnt = 0;
            long nYPos = LONG_MAX, nXPos, nHeight;

            for( n = 0, nEnd = aCellFrms.Count(); n < nEnd; ++n )
            {
                const _Sort_CellFrm& rCF = aCellFrms[ n ];
                if( rCF.pFrm->Frm().Top() != nYPos )
                {
                    // neue Zeile
                    if( n )
                    {
                        if( USHRT_MAX == nRowCells )        // 1. Zeilenwechsel
                            nRowCells = nCellCnt;
                        else if( nRowCells != nCellCnt )
                        {
                            bValidChartSel = FALSE;
                            break;
                        }
                    }
                    nCellCnt = 1;
                    nYPos = rCF.pFrm->Frm().Top();
                    nHeight = rCF.pFrm->Frm().Height();
                    nXPos = rCF.pFrm->Frm().Left() + rCF.pFrm->Frm().Width();
                }
                else if( nXPos == rCF.pFrm->Frm().Left() &&
                         nHeight == rCF.pFrm->Frm().Height() )
                {
                    nXPos += rCF.pFrm->Frm().Width();
                    ++nCellCnt;
                }
                else
                {
                    bValidChartSel = FALSE;
                    break;
                }
            }
            if( bValidChartSel )
            {
                if( USHRT_MAX == nRowCells )
                    nRowCells = nCellCnt;
                else if( nRowCells != nCellCnt )
                    bValidChartSel = FALSE;
            }

            if( bValidChartSel && pGetCLines )
            {
                nYPos = LONG_MAX;
                SwChartBoxes* pBoxes;
                for( n = 0, nEnd = aCellFrms.Count(); n < nEnd; ++n )
                {
                    const _Sort_CellFrm& rCF = aCellFrms[ n ];
                    if( rCF.pFrm->Frm().Top() != nYPos )
                    {
                        pBoxes = new SwChartBoxes( 255 < nRowCells
                                                    ? 255 : (BYTE)nRowCells);
                        pGetCLines->C40_INSERT( SwChartBoxes, pBoxes, pGetCLines->Count() );
                        nYPos = rCF.pFrm->Frm().Top();
                    }
                    SwTableBoxPtr pBox = (SwTableBox*)rCF.pFrm->GetTabBox();
                    pBoxes->Insert( pBox, pBoxes->Count() );
                }
            }
        }

        if( bTblIsValid )
            break;

        // ansonsten das Layout der Tabelle kurz "kalkulieren" lassen
        // und nochmals neu aufsetzen
        for( i = 0; i < aUnions.Count(); ++i )
        {
            SwTabFrm *pTable = aUnions[i]->GetTable();
            if( pTable->IsValid() )
                pTable->InvalidatePos();
            pTable->SetONECalcLowers();
            pTable->Calc();
            pTable->SetCompletePaint();
        }
        --nLoopMax;
        if( pGetCLines )
            pGetCLines->DeleteAndDestroy( 0, pGetCLines->Count() );
    } while( TRUE );

    ASSERT( nLoopMax, "das Layout der Tabelle wurde nicht valide!" );

    if( !bValidChartSel && pGetCLines )
        pGetCLines->DeleteAndDestroy( 0, pGetCLines->Count() );

    return bValidChartSel;
}


BOOL IsFrmInTblSel( const SwRect& rUnion, const SwFrm* pCell )
{
    return (
        rUnion.Top() <= pCell->Frm().Top() &&
        rUnion.Bottom() >= pCell->Frm().Bottom() &&

        (( rUnion.Left() <= pCell->Frm().Left()+20 &&
           rUnion.Right() > pCell->Frm().Left() ) ||

         ( rUnion.Left() >= pCell->Frm().Left() &&
           rUnion.Right() < pCell->Frm().Right() )) ? TRUE : FALSE );
}

BOOL GetAutoSumSel( const SwCrsrShell& rShell, SwCellFrms& rBoxes )
{
    SwShellCrsr *pCrsr = (SwShellCrsr*) rShell.IsTableMode() ?
                          rShell.pTblCrsr : rShell.pCurCrsr;

    const SwLayoutFrm *pStart = pCrsr->GetCntntNode()->GetFrm(
                      &pCrsr->GetPtPos() )->GetUpper(),
                      *pEnd   = pCrsr->GetCntntNode(FALSE)->GetFrm(
                      &pCrsr->GetMkPos() )->GetUpper();

    //Muss ein HeadlineRepeat beachtet werden?
    const BOOL bRepeat = pStart->FindTabFrm()->GetTable()->IsHeadlineRepeat();

    const SwLayoutFrm* pSttCell = pStart;
    while( pSttCell && !pSttCell->IsCellFrm() )
        pSttCell = pSttCell->GetUpper();

    //Zuerst lassen wir uns die Tabellen und die Rechtecke heraussuchen.
    SwSelUnions aUnions;

    // default erstmal nach oben testen, dann nach links
    ::MakeSelUnions( aUnions, pStart, pEnd, TBLSEARCH_COL );

    BOOL bTstRow = TRUE, bFound = FALSE;
    USHORT i;

    // 1. teste ob die darueber liegende Box Value/Formel enhaelt:
    for( i = 0; i < aUnions.Count(); ++i )
    {
        SwSelUnion *pUnion = aUnions[i];
        const SwTabFrm *pTable = pUnion->GetTable();

        const SwLayoutFrm *pRow = (const SwLayoutFrm*)pTable->Lower();
        //Wenn die Row eine wiederholte Headline ist wird sie nicht
        //beachtet.
        if( bRepeat && pTable->IsFollow() )
            pRow = (const SwLayoutFrm*)pRow->GetNext();

        while( pRow )
        {
            if( pRow->Frm().IsOver( pUnion->GetUnion() ) )
            {
                const SwCellFrm* pUpperCell = 0;
                const SwLayoutFrm *pCell = pRow->FirstCell();

                while( pCell && pRow->IsAnLower( pCell ) )
                {
                    if( pCell == pSttCell )
                    {
                        USHORT nWhichId = 0;
                        for( USHORT n = rBoxes.Count(); n; )
                            if( USHRT_MAX != ( nWhichId = rBoxes[ --n ]
                                ->GetTabBox()->IsFormulaOrValueBox() ))
                                break;

                        // alle Boxen zusammen, nicht mehr die Zeile
                        // pruefen, wenn eine Formel oder Value gefunden wurde
                        bTstRow = 0 == nWhichId || USHRT_MAX == nWhichId;
                        bFound = TRUE;
                        break;
                    }

                    ASSERT( pCell->IsCellFrm(), "Frame ohne Celle" );
                    if( ::IsFrmInTblSel( pUnion->GetUnion(), pCell ) )
                        pUpperCell = (SwCellFrm*)pCell;

                    if( pCell->GetNext() )
                    {
                        pCell = (const SwLayoutFrm*)pCell->GetNext();
                        if ( pCell->Lower()->IsRowFrm() )
                            pCell = pCell->FirstCell();
                    }
                    else
                        pCell = ::lcl_FindNextCellFrm( pCell );
                }

                if( pUpperCell )
                    rBoxes.Insert( pUpperCell, rBoxes.Count() );
            }
            if( bFound )
            {
                i = aUnions.Count();
                break;
            }
            pRow = (const SwLayoutFrm*)pRow->GetNext();
        }
    }


    // 2. teste ob die links liegende Box Value/Formel enhaelt:
    if( bTstRow )
    {
        bFound = FALSE;

        rBoxes.Remove( 0, rBoxes.Count() );
        aUnions.DeleteAndDestroy( 0, aUnions.Count() );
        ::MakeSelUnions( aUnions, pStart, pEnd, TBLSEARCH_ROW );

        for( i = 0; i < aUnions.Count(); ++i )
        {
            SwSelUnion *pUnion = aUnions[i];
            const SwTabFrm *pTable = pUnion->GetTable();

            const SwLayoutFrm *pRow = (const SwLayoutFrm*)pTable->Lower();
            //Wenn die Row eine wiederholte Headline ist wird sie nicht
            //beachtet.
            if( bRepeat && pTable->IsFollow() )
                pRow = (const SwLayoutFrm*)pRow->GetNext();

            while( pRow )
            {
                if( pRow->Frm().IsOver( pUnion->GetUnion() ) )
                {
                    const SwLayoutFrm *pCell = pRow->FirstCell();

                    while( pCell && pRow->IsAnLower( pCell ) )
                    {
                        if( pCell == pSttCell )
                        {
                            USHORT nWhichId = 0;
                            for( USHORT n = rBoxes.Count(); n; )
                                if( USHRT_MAX != ( nWhichId = rBoxes[ --n ]
                                    ->GetTabBox()->IsFormulaOrValueBox() ))
                                    break;

                            // alle Boxen zusammen, nicht mehr die Zeile
                            // pruefen, wenn eine Formel oder Value gefunden wurde
                            bFound = 0 != nWhichId && USHRT_MAX != nWhichId;
                            bTstRow = FALSE;
                            break;
                        }

                        ASSERT( pCell->IsCellFrm(), "Frame ohne Celle" );
                        if( ::IsFrmInTblSel( pUnion->GetUnion(), pCell ) )
                        {
                            const SwCellFrm* pC = (SwCellFrm*)pCell;
                            rBoxes.Insert( pC, rBoxes.Count() );
                        }
                        if( pCell->GetNext() )
                        {
                            pCell = (const SwLayoutFrm*)pCell->GetNext();
                            if ( pCell->Lower()->IsRowFrm() )
                                pCell = pCell->FirstCell();
                        }
                        else
                            pCell = ::lcl_FindNextCellFrm( pCell );
                    }
                }
                if( !bTstRow )
                {
                    i = aUnions.Count();
                    break;
                }

                pRow = (const SwLayoutFrm*)pRow->GetNext();
            }
        }
    }

    return bFound;
}

BOOL HasProtectedCells( const SwSelBoxes& rBoxes )
{
    BOOL bRet = FALSE;
    for( USHORT n = 0, nCnt = rBoxes.Count(); n < nCnt; ++n )
        if( rBoxes[ n ]->GetFrmFmt()->GetProtect().IsCntntProtected() )
        {
            bRet = TRUE;
            break;
        }
    return bRet;
}


_CmpLPt::_CmpLPt( const Point& rPt, const SwTableBox* pBox )
    : pPos( &rPt ), pSelBox( pBox )
{}

void lcl_InsTblBox( SwTableNode* pTblNd, SwDoc* pDoc, SwTableBox* pBox,
                        USHORT nInsPos, USHORT nCnt = 1 )
{
    ASSERT( pBox->GetSttNd(), "Box ohne Start-Node" );
    SwCntntNode* pCNd = pDoc->GetNodes()[ pBox->GetSttIdx() + 1 ]
                                ->GetCntntNode();
    if( pCNd && pCNd->IsTxtNode() )
        pDoc->GetNodes().InsBoxen( pTblNd, pBox->GetUpper(),
                (SwTableBoxFmt*)pBox->GetFrmFmt(),
                ((SwTxtNode*)pCNd)->GetTxtColl(),
                pCNd->GetpSwAttrSet(),
                nInsPos, nCnt );
    else
        pDoc->GetNodes().InsBoxen( pTblNd, pBox->GetUpper(),
                (SwTableBoxFmt*)pBox->GetFrmFmt(),
                (SwTxtFmtColl*)pDoc->GetDfltTxtFmtColl(), 0,
                nInsPos, nCnt );
}

BOOL lcl_IsEmptyBox( const SwTableBox& rBox, SwPaM& rPam )
{
    rPam.GetPoint()->nNode = *rBox.GetSttNd()->EndOfSectionNode();
    rPam.Move( fnMoveBackward, fnGoCntnt );
    rPam.SetMark();
    rPam.GetPoint()->nNode = *rBox.GetSttNd();
    rPam.Move( fnMoveForward, fnGoCntnt );

    BOOL bRet = *rPam.GetMark() == *rPam.GetPoint();
    if( bRet )
    {
        // dann teste mal auf absatzgebundenen Flys
        const SwSpzFrmFmts& rFmts = *rPam.GetDoc()->GetSpzFrmFmts();
        ULONG nSttIdx = rPam.GetPoint()->nNode.GetIndex(),
              nEndIdx = rBox.GetSttNd()->EndOfSectionIndex(),
              nIdx;

        for( USHORT n = 0; n < rFmts.Count(); ++n )
        {
            const SwPosition* pAPos;
            const SwFmtAnchor& rAnchor = rFmts[n]->GetAnchor();
            if( ( FLY_AT_CNTNT == rAnchor.GetAnchorId() ||
                  FLY_AUTO_CNTNT == rAnchor.GetAnchorId() ) &&
                0 != ( pAPos = rAnchor.GetCntntAnchor() ) &&
                nSttIdx <= ( nIdx = pAPos->nNode.GetIndex() ) &&
                nIdx < nEndIdx )
            {
                bRet = FALSE;
                break;
            }
        }
    }
    return bRet;
}


void GetMergeSel( const SwPaM& rPam, SwSelBoxes& rBoxes,
                SwTableBox** ppMergeBox, SwUndoTblMerge* pUndo )
{
    if( rBoxes.Count() )
        rBoxes.Remove( USHORT(0), rBoxes.Count() );

    //Zuerst lassen wir uns die Tabellen und die Rechtecke heraussuchen.
    ASSERT( rPam.GetCntntNode() && rPam.GetCntntNode( FALSE ),
            "Tabselection nicht auf Cnt." );

//JP 24.09.96: Merge mit wiederholenden TabellenHeadline funktioniert nicht
//              richtig. Warum nicht Point 0,0 benutzen? Dann ist garantiert,
//              das die 1. Headline mit drin ist.
//  Point aPt( rShell.GetCharRect().Pos() );
    Point aPt( 0, 0 );
    const SwLayoutFrm *pStart = rPam.GetCntntNode()->GetFrm(
                                                        &aPt )->GetUpper(),
                      *pEnd = rPam.GetCntntNode(FALSE)->GetFrm(
                                                        &aPt )->GetUpper();

    SwSelUnions aUnions;
    ::MakeSelUnions( aUnions, pStart, pEnd );
    if( !aUnions.Count() )
        return;

    const SwTable *pTable = aUnions[0]->GetTable()->GetTable();
    SwDoc* pDoc = (SwDoc*)pStart->GetFmt()->GetDoc();
    SwTableNode* pTblNd = (SwTableNode*)pTable->GetTabSortBoxes()[ 0 ]->
                                        GetSttNd()->FindTableNode();

    _MergePos aPosArr;      // Sort-Array mit den Positionen der Frames
    long nWidth;
    SwTableBox* pLastBox = 0;

    for ( USHORT i = 0; i < aUnions.Count(); ++i )
    {
        const SwTabFrm *pTabFrm = aUnions[i]->GetTable();
        SwRect &rUnion = aUnions[i]->GetUnion();
        const SwLayoutFrm *pRow = (const SwLayoutFrm*)pTabFrm->Lower();
        if ( pRow && pTabFrm->IsFollow() && pTable->IsHeadlineRepeat() )
            pRow = (SwLayoutFrm*)pRow->GetNext();

        while ( pRow )
        {
            if ( pRow->Frm().IsOver( rUnion ) )
            {
                const SwLayoutFrm *pCell = pRow->FirstCell();
//              while ( !pCell->IsCellFrm() )
//                  pCell = pCell->GetUpper();

                while ( pCell && pRow->IsAnLower( pCell ) )
                {
                    ASSERT( pCell->IsCellFrm(), "Frame ohne Celle" );
                        // in der vollen Breite ueberlappend ?
                    if( rUnion.Top() <= pCell->Frm().Top() &&
                        rUnion.Bottom() >= pCell->Frm().Bottom() )
                    {
                        SwTableBox* pBox =(SwTableBox*)((SwCellFrm*)pCell)->GetTabBox();

                        // nur nach rechts ueberlappend
                        if( ( rUnion.Left() - COLFUZZY ) <= pCell->Frm().Left() &&
                            ( rUnion.Right() - COLFUZZY ) > pCell->Frm().Left() )
                        {
                            if( ( rUnion.Right() + COLFUZZY ) < pCell->Frm().Right() )
                            {
                                USHORT nInsPos = pBox->GetUpper()->
                                                    GetTabBoxes().C40_GETPOS( SwTableBox, pBox )+1;
                                lcl_InsTblBox( pTblNd, pDoc, pBox, nInsPos );
                                pBox->ClaimFrmFmt();
                                SwFmtFrmSize aNew(
                                        pBox->GetFrmFmt()->GetFrmSize() );
                                nWidth = rUnion.Right() - pCell->Frm().Left();
                                nWidth = nWidth * aNew.GetWidth() /
                                         pCell->Frm().Width();
                                long nTmpWidth = aNew.GetWidth() - nWidth;
                                aNew.SetWidth( nWidth );
                                pBox->GetFrmFmt()->SetAttr( aNew );
                                // diese Box ist selektiert
                                pLastBox = pBox;
                                rBoxes.Insert( pBox );
                                aPosArr.Insert( _CmpLPt( pCell->Frm().Pos(), pBox ));

                                pBox = pBox->GetUpper()->GetTabBoxes()[ nInsPos ];
                                aNew.SetWidth( nTmpWidth );
                                pBox->ClaimFrmFmt();
                                pBox->GetFrmFmt()->SetAttr( aNew );

                                if( pUndo )
                                    pUndo->AddNewBox( pBox->GetSttIdx() );
                            }
                            else
                            {
                                // diese Box ist selektiert
                                pLastBox = pBox;
                                rBoxes.Insert( pBox );
                                aPosArr.Insert( _CmpLPt( pCell->Frm().Pos(), pBox ));
                            }
                        }
                        // oder rechts und links ueberlappend
                        else if( ( rUnion.Left() - COLFUZZY ) >= pCell->Frm().Left() &&
                                ( rUnion.Right() + COLFUZZY ) < pCell->Frm().Right() )
                        {
                            USHORT nInsPos = pBox->GetUpper()->GetTabBoxes().
                                            C40_GETPOS( SwTableBox, pBox )+1;
                            lcl_InsTblBox( pTblNd, pDoc, pBox, nInsPos, 2 );
                            pBox->ClaimFrmFmt();
                            SwFmtFrmSize aNew(
                                        pBox->GetFrmFmt()->GetFrmSize() );
                            long nLeft = rUnion.Left() - pCell->Frm().Left();
                            nLeft = nLeft * aNew.GetWidth() /
                                    pCell->Frm().Width();
                            long nRight = pCell->Frm().Right() - rUnion.Right();
                            nRight = nRight * aNew.GetWidth() /
                                     pCell->Frm().Width();
                            nWidth = aNew.GetWidth() - nLeft - nRight;

                            aNew.SetWidth( nLeft );
                            pBox->GetFrmFmt()->SetAttr( aNew );

                            {
                            const SfxPoolItem* pItem;
                            if( SFX_ITEM_SET == pBox->GetFrmFmt()->GetAttrSet()
                                        .GetItemState( RES_BOX, FALSE, &pItem ))
                            {
                                SvxBoxItem aBox( *(SvxBoxItem*)pItem );
                                aBox.SetLine( 0, BOX_LINE_RIGHT );
                                pBox->GetFrmFmt()->SetAttr( aBox );
                            }
                            }

                            pBox = pBox->GetUpper()->GetTabBoxes()[ nInsPos ];
                            aNew.SetWidth( nWidth );
                            pBox->ClaimFrmFmt();
                            pBox->GetFrmFmt()->SetAttr( aNew );

                            if( pUndo )
                                pUndo->AddNewBox( pBox->GetSttIdx() );

                            // diese Box ist selektiert
                            pLastBox = pBox;
                            rBoxes.Insert( pBox );
                            aPosArr.Insert( _CmpLPt( pCell->Frm().Pos(), pBox ));

                            pBox = pBox->GetUpper()->GetTabBoxes()[ nInsPos+1 ];
                            aNew.SetWidth( nRight );
                            pBox->ClaimFrmFmt();
                            pBox->GetFrmFmt()->SetAttr( aNew );

                            if( pUndo )
                                pUndo->AddNewBox( pBox->GetSttIdx() );
                        }
                        // oder reicht die rechte Kante der Box in den
                        // selektierten Bereich?
                        else if( ( pCell->Frm().Right() - COLFUZZY ) < rUnion.Right() &&
                                 ( pCell->Frm().Right() - COLFUZZY ) > rUnion.Left() &&
                                 ( pCell->Frm().Left() + COLFUZZY ) < rUnion.Left() )
                        {
                            // dann muss eine neue Box einfuegt und die
                            // Breiten angepasst werden
                            USHORT nInsPos = pBox->GetUpper()->GetTabBoxes().
                                            C40_GETPOS( SwTableBox, pBox )+1;
                            lcl_InsTblBox( pTblNd, pDoc, pBox, nInsPos, 1 );

                            SwFmtFrmSize aNew(pBox->GetFrmFmt()->GetFrmSize() );
                            long nLeft = rUnion.Left() - pCell->Frm().Left(),
                                nRight = pCell->Frm().Right() - rUnion.Left();

                            nLeft = nLeft * aNew.GetWidth() /
                                    pCell->Frm().Width();
                            nRight = nRight * aNew.GetWidth() /
                                    pCell->Frm().Width();

                            aNew.SetWidth( nLeft );
                            pBox->ClaimFrmFmt()->SetAttr( aNew );

                                // diese Box ist selektiert
                            pBox = pBox->GetUpper()->GetTabBoxes()[ nInsPos ];
                            aNew.SetWidth( nRight );
                            pBox->ClaimFrmFmt();
                            pBox->GetFrmFmt()->SetAttr( aNew );

                            pLastBox = pBox;
                            rBoxes.Insert( pBox );
                            aPosArr.Insert( _CmpLPt( Point( rUnion.Left(),
                                                pCell->Frm().Top()), pBox ));

                            if( pUndo )
                                pUndo->AddNewBox( pBox->GetSttIdx() );
                        }
                    }
                    if ( pCell->GetNext() )
                    {
                        pCell = (const SwLayoutFrm*)pCell->GetNext();
                        if ( pCell->Lower()->IsRowFrm() )
                            pCell = pCell->FirstCell();
                    }
                    else
                        pCell = ::lcl_FindNextCellFrm( pCell );
                }
            }
            pRow = (const SwLayoutFrm*)pRow->GetNext();
        }
    }

    // keine SSelection / keine gefundenen Boxen
    if( 1 >= rBoxes.Count() )
        return;

    // dann suche mal alle Boxen, die nebeneinander liegen, und verbinde
    // deren Inhalte mit Blanks. Alle untereinander liegende werden als
    // Absaetze zusammengefasst

    // 1. Loesung: gehe ueber das Array und
    //      alle auf der gleichen Y-Ebene werden mit Blanks getrennt
    //      alle anderen werden als Absaetze getrennt.
    BOOL bCalcWidth = TRUE;
    const SwTableBox* pFirstBox = aPosArr[ 0 ].pSelBox;

    // JP 27.03.98:  Optimierung - falls die Boxen einer Line leer sind,
    //              dann werden jetzt dafuer keine Blanks und
    //              kein Umbruch mehr eingefuegt.
    //Block damit SwPaM, SwPosition vom Stack geloescht werden
    {
        SwPaM aPam( pDoc->GetNodes() );

#if defined( DEL_ONLY_EMPTY_LINES )
        nWidth = pFirstBox->GetFrmFmt()->GetFrmSize().GetWidth();
        BOOL bEmptyLine = TRUE;
        USHORT n, nSttPos = 0;

        for( n = 0; n < aPosArr.Count(); ++n )
        {
            const _CmpLPt& rPt = aPosArr[ n ];
            if( n && aPosArr[ n - 1 ].Y() == rPt.Y() )  // gleiche Ebene ?
            {
                if( bEmptyLine && !lcl_IsEmptyBox( *rPt.pSelBox, aPam ))
                    bEmptyLine = FALSE;
                if( bCalcWidth )
                    nWidth += rPt.pSelBox->GetFrmFmt()->GetFrmSize().GetWidth();
            }
            else
            {
                if( bCalcWidth && n )
                    bCalcWidth = FALSE;     // eine Zeile fertig

                if( bEmptyLine && nSttPos < n )
                {
                    // dann ist die gesamte Line leer und braucht
                    // nicht mit Blanks aufgefuellt und als Absatz
                    // eingefuegt werden.
                    if( pUndo )
                        for( USHORT i = nSttPos; i < n; ++i )
                            pUndo->SaveCollection( *aPosArr[ i ].pSelBox );

                    aPosArr.Remove( nSttPos, n - nSttPos );
                    n = nSttPos;
                }
                else
                    nSttPos = n;

                bEmptyLine = lcl_IsEmptyBox( *aPosArr[n].pSelBox, aPam );
            }
        }
        if( bEmptyLine && nSttPos < n )
        {
            if( pUndo )
                for( USHORT i = nSttPos; i < n; ++i )
                    pUndo->SaveCollection( *aPosArr[ i ].pSelBox );
            aPosArr.Remove( nSttPos, n - nSttPos );
        }
#elsif defined( DEL_EMPTY_BOXES_AT_START_AND_END )

        nWidth = pFirstBox->GetFrmFmt()->GetFrmSize().GetWidth();
        USHORT n, nSttPos = 0, nSEndPos = 0, nESttPos = 0;

        for( n = 0; n < aPosArr.Count(); ++n )
        {
            const _CmpLPt& rPt = aPosArr[ n ];
            if( n && aPosArr[ n - 1 ].Y() == rPt.Y() )  // gleiche Ebene ?
            {
                BOOL bEmptyBox = lcl_IsEmptyBox( *rPt.pSelBox, aPam );
                if( bEmptyBox )
                {
                    if( nSEndPos == n )     // der Anfang ist leer
                        nESttPos = ++nSEndPos;
                }
                else                        // das Ende kann leer sein
                    nESttPos = n+1;

                if( bCalcWidth )
                    nWidth += rPt.pSelBox->GetFrmFmt()->GetFrmSize().GetWidth();
            }
            else
            {
                if( bCalcWidth && n )
                    bCalcWidth = FALSE;     // eine Zeile fertig

                // zuerst die vom Anfang
                if( nSttPos < nSEndPos )
                {
                    // dann ist der vorder Teil der Line leer und braucht
                    // nicht mit Blanks aufgefuellt werden.
                    if( pUndo )
                        for( USHORT i = nSttPos; i < nSEndPos; ++i )
                            pUndo->SaveCollection( *aPosArr[ i ].pSelBox );

                    USHORT nCnt = nSEndPos - nSttPos;
                    aPosArr.Remove( nSttPos, nCnt );
                    nESttPos -= nCnt;
                    n -= nCnt;
                }

                if( nESttPos < n )
                {
                    // dann ist der vorder Teil der Line leer und braucht
                    // nicht mit Blanks aufgefuellt werden.
                    if( pUndo )
                        for( USHORT i = nESttPos; i < n; ++i )
                            pUndo->SaveCollection( *aPosArr[ i ].pSelBox );

                    USHORT nCnt = n - nESttPos;
                    aPosArr.Remove( nESttPos, nCnt );
                    n -= nCnt;
                }

                nSttPos = nSEndPos = nESttPos = n;
                if( lcl_IsEmptyBox( *aPosArr[n].pSelBox, aPam ))
                    ++nSEndPos;
                else
                    ++nESttPos;
            }
        }

        // zuerst die vom Anfang
        if( nSttPos < nSEndPos )
        {
            // dann ist der vorder Teil der Line leer und braucht
            // nicht mit Blanks aufgefuellt werden.
            if( pUndo )
                for( USHORT i = nSttPos; i < nSEndPos; ++i )
                    pUndo->SaveCollection( *aPosArr[ i ].pSelBox );

            USHORT nCnt = nSEndPos - nSttPos;
            aPosArr.Remove( nSttPos, nCnt );
            nESttPos -= nCnt;
            n -= nCnt;
        }
        if( nESttPos < n )
        {
            // dann ist der vorder Teil der Line leer und braucht
            // nicht mit Blanks aufgefuellt werden.
            if( pUndo )
                for( USHORT i = nESttPos; i < n; ++i )
                    pUndo->SaveCollection( *aPosArr[ i ].pSelBox );

            USHORT nCnt = n - nESttPos;
            aPosArr.Remove( nESttPos, nCnt );
        }
#else
// DEL_ALL_EMPTY_BOXES

        nWidth = 0;
        long nY = aPosArr.Count() ? aPosArr[ 0 ].Y() : 0;
        for( USHORT n = 0; n < aPosArr.Count(); ++n )
        {
            const _CmpLPt& rPt = aPosArr[ n ];
            if( bCalcWidth )
            {
                if( nY == rPt.Y() )         // gleiche Ebene ?
                    nWidth += rPt.pSelBox->GetFrmFmt()->GetFrmSize().GetWidth();
                else
                    bCalcWidth = FALSE;     // eine Zeile fertig
            }

            if( lcl_IsEmptyBox( *rPt.pSelBox, aPam ) )
            {
                if( pUndo )
                    pUndo->SaveCollection( *rPt.pSelBox );

                aPosArr.Remove( n, 1 );
                --n;
            }
        }
#endif
    }

    // lege schon mal die neue Box an
    {
        SwTableBox* pTmpBox = rBoxes[0];
        SwTableLine* pInsLine = pTmpBox->GetUpper();
        USHORT nInsPos = pInsLine->GetTabBoxes().C40_GETPOS( SwTableBox, pTmpBox );

        lcl_InsTblBox( pTblNd, pDoc, pTmpBox, nInsPos );
        (*ppMergeBox) = pInsLine->GetTabBoxes()[ nInsPos ];
        pInsLine->GetTabBoxes().Remove( nInsPos );      // wieder austragen
        (*ppMergeBox)->SetUpper( 0 );
        (*ppMergeBox)->ClaimFrmFmt();

        // setze die Umrandung: von der 1. Box die linke/obere von der
        // letzten Box die rechte/untere Kante:
        if( pLastBox && pFirstBox )
        {
            SvxBoxItem aBox( pFirstBox->GetFrmFmt()->GetBox() );
            const SvxBoxItem& rBox = pLastBox->GetFrmFmt()->GetBox();
            aBox.SetLine( rBox.GetRight(), BOX_LINE_RIGHT );
            aBox.SetLine( rBox.GetBottom(), BOX_LINE_BOTTOM );
            if( aBox.GetLeft() || aBox.GetTop() ||
                aBox.GetRight() || aBox.GetBottom() )
                (*ppMergeBox)->GetFrmFmt()->SetAttr( aBox );
        }
    }

    //Block damit SwPaM, SwPosition vom Stack geloescht werden
    if( aPosArr.Count() )
    {
        SwTxtNode* pTxtNd;
        SwPosition aInsPos( *(*ppMergeBox)->GetSttNd() );
        SwNodeIndex& rInsPosNd = aInsPos.nNode;

        SwPaM aPam( aInsPos );

        for( USHORT n = 0; n < aPosArr.Count(); ++n )
        {
            const _CmpLPt& rPt = aPosArr[ n ];
            aPam.GetPoint()->nNode.Assign( *rPt.pSelBox->GetSttNd()->
                                            EndOfSectionNode(), -1 );
            SwCntntNode* pCNd = aPam.GetCntntNode();
            USHORT nL = pCNd ? pCNd->Len() : 0;
            aPam.GetPoint()->nContent.Assign( pCNd, nL );

            SwNodeIndex aSttNdIdx( *rPt.pSelBox->GetSttNd(), 1 );

            if( n && aPosArr[n-1].Y() == rPt.Y() && pTxtNd &&   // gleiche Ebene ?
                aSttNdIdx.GetNode().IsTxtNode() )
            {
                pTxtNd->Insert( '\x20', aInsPos.nContent );
                aPam.SetMark();
                aPam.GetPoint()->nNode = aSttNdIdx;
                aPam.GetPoint()->nContent.Assign(
                                aSttNdIdx.GetNode().GetCntntNode(), 0 );

                // alle absatzgebundenen Flys mitnehmen!
                _SaveFlyArr aSaveFlyArr;
                SwNodeIndex aIdx( rInsPosNd, -1 );
                {
                    SwNodeRange aRg( aPam.GetPoint()->nNode.GetNode(), 0,
                                *rPt.pSelBox->GetSttNd()->EndOfSectionNode() );
                    _SaveFlyInRange( aRg, aSaveFlyArr );
                }

                if( pUndo )
                    pUndo->MoveBoxCntnt( aPam, aInsPos, aSaveFlyArr );
                else
                    pDoc->Move( aPam, aInsPos );
                aPam.DeleteMark();
                if( bCalcWidth )
                    nWidth += rPt.pSelBox->GetFrmFmt()->GetFrmSize().GetWidth();

                if( aSaveFlyArr.Count() )
                {
                    aIdx++;
                    _RestFlyInRange( aSaveFlyArr, aIdx );
                }
            }
            else                                // Nodes moven
            {
                // ein Node muss in der Box erhalten bleiben (sonst wird beim
                // Move die gesamte Section geloescht)
                if( pUndo )
                    pDoc->DoUndo( FALSE );
                pDoc->AppendTxtNode( *aPam.GetPoint() );
                if( pUndo )
                    pDoc->DoUndo( TRUE );
                SwNodeRange aRg( aSttNdIdx, aPam.GetPoint()->nNode );
                rInsPosNd++;
                if( pUndo )
                    pUndo->MoveBoxCntnt( pDoc, aRg, rInsPosNd );
                else
                    pDoc->Move( aRg, rInsPosNd );
                // wo steht jetzt aInsPos ??

                if( bCalcWidth )
                    bCalcWidth = FALSE;     // eine Zeile fertig
            }

            // den initialen TextNode ueberspringen
            rInsPosNd.Assign( pDoc->GetNodes(),
                            rInsPosNd.GetNode().EndOfSectionIndex() - 2 );
            pTxtNd = rInsPosNd.GetNode().GetTxtNode();
            if( pTxtNd )
                aInsPos.nContent.Assign( pTxtNd, pTxtNd->GetTxt().Len() );
        }

        // in der MergeBox sollte jetzt der gesamte Text stehen
        // loesche jetzt noch den initialen TextNode
        ASSERT( (*ppMergeBox)->GetSttIdx()+2 <
                (*ppMergeBox)->GetSttNd()->EndOfSectionIndex(),
                    "leere Box" );
        SwNodeIndex aIdx( *(*ppMergeBox)->GetSttNd()->EndOfSectionNode(), -1 );
        pDoc->GetNodes().Delete( aIdx, 1 );
    }

    // setze die Breite der Box
    (*ppMergeBox)->GetFrmFmt()->SetAttr( SwFmtFrmSize( ATT_VAR_SIZE, nWidth, 0 ));
    if( pUndo )
        pUndo->AddNewBox( (*ppMergeBox)->GetSttIdx() );
}


BOOL lcl_CheckCol( const _FndBox*& rpFndBox, void* pPara );

BOOL lcl_CheckRow( const _FndLine*& rpFndLine, void* pPara )
{
    ((_FndLine*)rpFndLine)->GetBoxes().ForEach( &lcl_CheckCol, pPara );
    return *(BOOL*)pPara;
}

BOOL lcl_CheckCol( const _FndBox*& rpFndBox, void* pPara )
{
    if( !rpFndBox->GetBox()->GetSttNd() )
    {
        if( rpFndBox->GetLines().Count() !=
            rpFndBox->GetBox()->GetTabLines().Count() )
            *((BOOL*)pPara) = FALSE;
        else
            ((_FndBox*)rpFndBox)->GetLines().ForEach( &lcl_CheckRow, pPara );
    }
    // Box geschuetzt ??
    else if( rpFndBox->GetBox()->GetFrmFmt()->GetProtect().IsCntntProtected() )
        *((BOOL*)pPara) = FALSE;
    return *(BOOL*)pPara;
}


USHORT CheckMergeSel( const SwPaM& rPam )
{
    SwSelBoxes aBoxes;
//JP 24.09.96: Merge mit wiederholenden TabellenHeadline funktioniert nicht
//              richtig. Warum nicht Point 0,0 benutzen? Dann ist garantiert,
//              das die 1. Headline mit drin ist.
    Point aPt;
    const SwLayoutFrm *pStart = rPam.GetCntntNode()->GetFrm(
                                                    &aPt )->GetUpper(),
                        *pEnd = rPam.GetCntntNode(FALSE)->GetFrm(
                                                    &aPt )->GetUpper();
    GetTblSel( pStart, pEnd, aBoxes );
    return CheckMergeSel( aBoxes );
}

USHORT CheckMergeSel( const SwSelBoxes& rBoxes )
{
    USHORT eRet = TBLMERGE_NOSELECTION;
    if( rBoxes.Count() )
    {
        eRet = TBLMERGE_OK;

        _FndBox aFndBox( 0, 0 );
        _FndPara aPara( rBoxes, &aFndBox );
        const SwTableNode* pTblNd = aPara.rBoxes[0]->GetSttNd()->FindTableNode();
        ((SwTable&)pTblNd->GetTable()).GetTabLines().ForEach(
                    &_FndLineCopyCol, &aPara );
        if( aFndBox.GetLines().Count() )
        {
            BOOL bMergeSelOk = TRUE;
            _FndBox* pFndBox = &aFndBox;
            _FndLine* pFndLine = 0;
            while( pFndBox && 1 == pFndBox->GetLines().Count() )
            {
                pFndLine = pFndBox->GetLines()[0];
                if( 1 == pFndLine->GetBoxes().Count() )
                    pFndBox = pFndLine->GetBoxes()[0];
                else
                    pFndBox = 0;
            }
            if( pFndBox )
                pFndBox->GetLines().ForEach( &lcl_CheckRow, &bMergeSelOk );
            else if( pFndLine )
                pFndLine->GetBoxes().ForEach( &lcl_CheckCol, &bMergeSelOk );
            if( !bMergeSelOk )
                eRet = TBLMERGE_TOOCOMPLEX;
        }
        else
            eRet = TBLMERGE_NOSELECTION;
    }
    return eRet;
}

//Ermittelt die von einer Tabellenselektion betroffenen Tabellen und die
//Union-Rechteckte der Selektionen - auch fuer aufgespaltene Tabellen.
SV_IMPL_PTRARR( SwSelUnions, SwSelUnion* );

SwTwips lcl_CalcWish( const SwLayoutFrm *pCell, long nWish,
                                                const long nAct )
{
    SwTwips nRet = 0;
    const SwLayoutFrm *pTmp = pCell;
    if ( !nWish )
        nWish = 1;
    while ( pTmp )
    {
        while ( pTmp->GetPrev() )
        {   pTmp = (SwLayoutFrm*)pTmp->GetPrev();
            long nTmp = pTmp->GetFmt()->GetFrmSize().GetWidth();
            nRet += nTmp * nAct / nWish;
        }
        pTmp = pTmp->GetUpper()->GetUpper();
        if ( pTmp && !pTmp->IsCellFrm() )
            pTmp = 0;
    }
    return nRet;
}

/*  MA: 20. Sep. 93 wird nicht mehr gebraucht.
static const SwLayoutFrm *GetPrevCell( const SwLayoutFrm *pCell )
{
    const SwLayoutFrm *pLay = pCell->GetPrevLayoutLeaf();
    if ( pLay && pLay->IsLayoutFrm() && !pLay->IsTab() )
    {
        //GetPrevLayoutLeaf() liefert ggf. auch die Umgebung einer Tab zurueck
        //(naehmlich genau dann, wenn die Zelle noch Vorgaenger hat).
        const SwFrm *pFrm = pLay->Lower();
        while ( pFrm->GetNext() )
            pFrm = pFrm->GetNext();
        pLay = pFrm->IsTabFrm() ? (SwLayoutFrm*)pFrm : 0;
    }
    if ( pLay && pLay->IsTabFrm() )
    {
        //GetPrevLayoutLeaf() liefert ggf. auch Tabellen zurueck die letzte
        //Zelle dieser Tabelle ist das das gesuchte Blatt.
        pLay = ((SwTabFrm*)pLay)->FindLastCntnt()->GetUpper();
        while ( !pLay->IsCellFrm() )
            pLay = pLay->GetUpper();
    }
    return pLay;
}
*/

void lcl_FindStartEndRow( const SwLayoutFrm *&rpStart,
                             const SwLayoutFrm *&rpEnd,
                             const int bChkProtected )
{
    //Start an den Anfang seiner Zeile setzen.
    //End an das Ende seiner Zeile setzen.
    rpStart = (SwLayoutFrm*)rpStart->GetUpper()->Lower();
    while ( rpEnd->GetNext() )
        rpEnd = (SwLayoutFrm*)rpEnd->GetNext();

    SvPtrarr aSttArr( 8, 8 ), aEndArr( 8, 8 );
    const SwLayoutFrm *pTmp;
    for( pTmp = rpStart; (FRM_CELL|FRM_ROW) & pTmp->GetType();
                pTmp = pTmp->GetUpper() )
    {
        void* p = (void*)pTmp;
        aSttArr.Insert( p, 0 );
    }
    for( pTmp = rpEnd; (FRM_CELL|FRM_ROW) & pTmp->GetType();
                pTmp = pTmp->GetUpper() )
    {
        void* p = (void*)pTmp;
        aEndArr.Insert( p, 0 );
    }

    for( USHORT n = 0; n < aEndArr.Count() && n < aSttArr.Count(); ++n )
        if( aSttArr[ n ] != aEndArr[ n ] )
        {
            // first unequal line or box - all odds are
            if( n & 1 )                 // 1, 3, 5, ... are boxes
            {
                rpStart = (SwLayoutFrm*)aSttArr[ n ];
                rpEnd = (SwLayoutFrm*)aEndArr[ n ];
            }
            else                                // 0, 2, 4, ... are lines
            {
                // check if start & end line are the first & last Line of the
                // box. If not return these cells.
                // Else the hole line with all Boxes has to be deleted.
                rpStart = (SwLayoutFrm*)aSttArr[ n+1 ];
                rpEnd = (SwLayoutFrm*)aEndArr[ n+1 ];
                if( n )
                {
                    const SwCellFrm* pCellFrm = (SwCellFrm*)aSttArr[ n-1 ];
                    const SwTableLines& rLns = pCellFrm->
                                                GetTabBox()->GetTabLines();
                    if( rLns[ 0 ] == ((SwRowFrm*)aSttArr[ n ])->GetTabLine() &&
                        rLns[ rLns.Count() - 1 ] ==
                                    ((SwRowFrm*)aEndArr[ n ])->GetTabLine() )
                    {
                        rpStart = rpEnd = pCellFrm;
                        while ( rpStart->GetPrev() )
                            rpStart = (SwLayoutFrm*)rpStart->GetPrev();
                        while ( rpEnd->GetNext() )
                            rpEnd = (SwLayoutFrm*)rpEnd->GetNext();
                    }
                }
            }
            break;
        }

    if( !bChkProtected )    // geschuetzte Zellen beachten ?
        return;


    //Anfang und Ende duerfen nicht auf geschuetzten Zellen liegen.
    while ( rpStart->GetFmt()->GetProtect().IsCntntProtected() )
        rpStart = (SwLayoutFrm*)rpStart->GetNext();
    while ( rpEnd->GetFmt()->GetProtect().IsCntntProtected() )
        rpEnd = (SwLayoutFrm*)rpEnd->GetPrev();
}

void lcl_FindStartEndCol( const SwLayoutFrm *&rpStart,
                             const SwLayoutFrm *&rpEnd,
                             const int bChkProtected )
{
    //Start und End senkrecht bis an den Rand der Tabelle denken; es muss
    //die Gesamttabelle betrachtet werden, also inklusive Masters und
    //Follows.
    //Fuer den Start brauchen wir den Mutter-TabellenFrm.
    const SwTabFrm *pOrg = rpStart->FindTabFrm();
    const SwTabFrm *pTab = pOrg;
    const long nWish = pOrg->GetFmt()->GetFrmSize().GetWidth();
    while ( pTab->IsFollow() )
    {
        const SwFrm *pTmp = pTab->FindPrev();
        ASSERT( pTmp->IsTabFrm(), "Vorgaenger vom Follow nicht der Master." );
        pTab = (const SwTabFrm*)pTmp;
    }

    const SwTwips nSX = ::lcl_CalcWish( rpStart, nWish, pTab->Prt().Width() ) +
                        (pTab->Frm().Left() + pTab->Prt().Left());
    const SwTwips nSX2= nSX + (rpStart->GetFmt()->GetFrmSize().GetWidth() *
                                            pTab->Prt().Width() / nWish);

    const SwLayoutFrm *pTmp = pTab->FirstCell();
    while ( pTmp &&
            (!pTmp->IsCellFrm() ||
             (pTmp->Frm().Left() < nSX &&
              pTmp->Frm().Right()< nSX2)))
        pTmp = pTmp->GetNextLayoutLeaf();
    if ( pTmp )
        rpStart = pTmp;

    pTab = pOrg;
    while ( pTab->GetFollow() )
        pTab = pTab->GetFollow();
    const SwTwips nEX = ::lcl_CalcWish( rpEnd, nWish, pTab->Prt().Width() ) +
                        (pTab->Frm().Left() + pTab->Prt().Left());
    rpEnd = pTab->FindLastCntnt()->GetUpper();
    while( !rpEnd->IsCellFrm() )
        rpEnd = rpEnd->GetUpper();
    while ( rpEnd->Frm().Left() > nEX )
        rpEnd = rpEnd->GetPrevLayoutLeaf();

    if( !bChkProtected )    // geschuetzte Zellen beachten ?
        return;

    //Anfang und Ende duerfen nicht auf geschuetzten Zellen liegen.
    //Also muss ggf. nocheinmal rueckwaerts gesucht werden.
    while ( rpStart->GetFmt()->GetProtect().IsCntntProtected() )
    {
        const SwLayoutFrm *pTmp = rpStart;
        pTmp = pTmp->GetNextLayoutLeaf();
        while ( pTmp && pTmp->Frm().Left() > nEX )//erstmal die Zeile ueberspr.
            pTmp = pTmp->GetNextLayoutLeaf();
        while ( pTmp && pTmp->Frm().Left() < nSX &&
                        pTmp->Frm().Right()< nSX2 )
            pTmp = pTmp->GetNextLayoutLeaf();
        const SwTabFrm *pTab = rpStart->FindTabFrm();
        if ( !pTab->IsAnLower( pTmp ) )
        {
            pTab = pTab->GetFollow();
            rpStart = pTab->FirstCell();
            while ( rpStart->Frm().Left() < nSX &&
                    rpStart->Frm().Right()< nSX2 )
                rpStart = rpStart->GetNextLayoutLeaf();
        }
        else
            rpStart = pTmp;
    }
    while ( rpEnd->GetFmt()->GetProtect().IsCntntProtected() )
    {
        const SwLayoutFrm *pTmp = rpEnd;
        pTmp = pTmp->GetPrevLayoutLeaf();
        while ( pTmp && pTmp->Frm().Left() < nEX )//erstmal die Zeile ueberspr.
            pTmp = pTmp->GetPrevLayoutLeaf();
        while ( pTmp && pTmp->Frm().Left() > nEX )
            pTmp = pTmp->GetPrevLayoutLeaf();
        const SwTabFrm *pTab = rpEnd->FindTabFrm();
        if ( !pTmp || !pTab->IsAnLower( pTmp ) )
        {
            pTab = (const SwTabFrm*)pTab->FindPrev();
            ASSERT( pTab->IsTabFrm(), "Vorgaenger vom Follow nicht der Master.");
            rpEnd = pTab->FindLastCntnt()->GetUpper();
            while( !rpEnd->IsCellFrm() )
                rpEnd = rpEnd->GetUpper();
            while ( rpEnd->Frm().Left() > nEX )
                rpEnd = rpEnd->GetPrevLayoutLeaf();
        }
        else
            rpEnd = pTmp;
    }
}


void MakeSelUnions( SwSelUnions& rUnions, const SwLayoutFrm *pStart,
                    const SwLayoutFrm *pEnd, const SwTblSearchType eSearchType )
{
    while ( !pStart->IsCellFrm() )
        pStart = pStart->GetUpper();
    while ( !pEnd->IsCellFrm() )
        pEnd = pEnd->GetUpper();

    const SwTabFrm *pTable = pStart->FindTabFrm();
    const SwTabFrm *pEndTable = pEnd->FindTabFrm();
    BOOL bExchange = FALSE;

    if ( pTable != pEndTable )
    {
        if ( !pTable->IsAnFollow( pEndTable ) )
        {
            ASSERT( pEndTable->IsAnFollow( pTable ), "Tabkette verknotet." );
            bExchange = TRUE;
        }
    }
    else if ( pStart->Frm().Top() > pEnd->Frm().Top() ||
             (pStart->Frm().Top() == pEnd->Frm().Top() &&
              pStart->Frm().Left() > pEnd->Frm().Left()) )
        bExchange = TRUE;
    if ( bExchange )
    {
        const SwLayoutFrm *pTmp = pStart;
        pStart = pEnd;
        pEnd = pTmp;
        //pTable und pEndTable nicht umsortieren, werden unten neu gesetzt.
        //MA: 28. Dec. 93 Bug: 5190
    }

    //Start und End sind jetzt huebsch sortiert, jetzt muessen sie falls
    //erwuenscht noch versetzt werden.
    if( TBLSEARCH_ROW == ((~TBLSEARCH_PROTECT ) & eSearchType ) )
        ::lcl_FindStartEndRow( pStart, pEnd, TBLSEARCH_PROTECT & eSearchType );
    else if( TBLSEARCH_COL == ((~TBLSEARCH_PROTECT ) & eSearchType ) )
        ::lcl_FindStartEndCol( pStart, pEnd, TBLSEARCH_PROTECT & eSearchType );

    //neu besorgen, da sie jetzt verschoben sind. MA: 28. Dec. 93 Bug 5190
    pTable = pStart->FindTabFrm();
    pEndTable = pEnd->FindTabFrm();

    const FASTBOOL bRepeat = pTable->GetTable()->IsHeadlineRepeat();

    const long nStSz = pStart->GetFmt()->GetFrmSize().GetWidth();
    const long nEdSz = pEnd->GetFmt()->GetFrmSize().GetWidth();
    const long nWish = Max( 1L, pTable->GetFmt()->GetFrmSize().GetWidth() );
    while ( pTable )
    {
        const long nOfst = pTable->Frm().Left() + pTable->Prt().Left();
        long nSt = ::lcl_CalcWish( pStart, nWish, pTable->Prt().Width() ) + nOfst;
        long nEd = ::lcl_CalcWish( pEnd,   nWish, pTable->Prt().Width() ) + nOfst;

        if ( nSt <= nEd )
            nEd += (long)((nEdSz * pTable->Prt().Width()) / nWish) - 1;
        else
            nSt += (long)((nStSz * pTable->Prt().Width()) / nWish) - 1;

        Point aSt( nSt, pStart->Frm().Top() ),
              aEd( nEd, pEnd->Frm().Bottom() );

        if ( !pTable->IsAnLower( pStart ) )
            aSt.Y() = pTable->Frm().Top();
        if ( !pTable->IsAnLower( pEnd ) )
            aEd.Y() = pTable->Frm().Bottom();

        SwRect aUnion( aSt, aEd );
        aUnion.Justify();

        // fuers
        if( !(TBLSEARCH_NO_UNION_CORRECT & eSearchType ))
        {
            //Leider ist die Union jetzt mit Rundungsfehlern behaftet und dadurch
            //wuerden beim Split/Merge fehlertraechtige Umstaende entstehen.
            //Um dies zu vermeiden werden jetzt fuer die Table die erste und
            //letzte Zelle innerhalb der Union ermittelt und aus genau deren
            //Werten wird die Union neu gebildet.
            const SwLayoutFrm *pRow = (SwLayoutFrm*)pTable->Lower();
            if ( bRepeat && pRow && pTable->IsFollow() )
                pRow = (SwLayoutFrm*)pRow->GetNext();
            while ( pRow && !pRow->Frm().IsOver( aUnion ) )
                pRow = (SwLayoutFrm*)pRow->GetNext();
            const SwLayoutFrm *pFirst = pRow ? pRow->FirstCell() : 0;
            while ( pFirst && !::IsFrmInTblSel( aUnion, pFirst ) )
            {
                if ( pFirst->GetNext() )
                {
                    pFirst = (const SwLayoutFrm*)pFirst->GetNext();
                    if ( pFirst->Lower()->IsRowFrm() )
                        pFirst = pFirst->FirstCell();
                }
                else
                    pFirst = ::lcl_FindNextCellFrm( pFirst );
            }
            const SwLayoutFrm *pLast = ::lcl_FindCellFrm( pTable->FindLastCntnt()->GetUpper());
            while ( pLast && !::IsFrmInTblSel( aUnion, pLast ) )
                pLast = ::lcl_FindCellFrm( pLast->GetPrevLayoutLeaf() );

            if ( pFirst && pLast ) //Robust
                aUnion = SwRect( pFirst->Frm().Pos(),
                                 Point( pLast->Frm().Right(),
                                         pLast->Frm().Bottom() ) );
            else
                aUnion.Width( 0 );
        }

        if( aUnion.Width() )
        {
            SwSelUnion *pTmp = new SwSelUnion( aUnion, (SwTabFrm*)pTable );
            rUnions.C40_INSERT( SwSelUnion, pTmp, rUnions.Count() );
        }

        pTable = pTable->GetFollow();
        if ( pTable != pEndTable && pEndTable->IsAnFollow( pTable ) )
            pTable = 0;
    }
}

BOOL CheckSplitCells( const SwCrsrShell& rShell, USHORT nDiv,
                        const SwTblSearchType eSearchType )
{
    if( !rShell.IsTableMode() )
        rShell.GetCrsr();

    const SwShellCrsr *pCrsr = rShell.GetTableCrsr();
    if( !pCrsr )
        pCrsr = (SwShellCrsr*)*rShell.GetSwCrsr( FALSE );

    return CheckSplitCells( *pCrsr, nDiv, eSearchType );
}

BOOL CheckSplitCells( const SwCursor& rCrsr, USHORT nDiv,
                        const SwTblSearchType eSearchType )
{
    if( 1 >= nDiv )
        return FALSE;

    USHORT nMinValue = nDiv * MINLAY;

    //Start- und Endzelle besorgen und den naechsten fragen.
    Point aPtPos, aMkPos;
    const SwShellCrsr* pShCrsr = rCrsr;
    if( pShCrsr )
    {
        aPtPos = pShCrsr->GetPtPos();
        aMkPos = pShCrsr->GetMkPos();
    }
    const SwLayoutFrm *pStart = rCrsr.GetCntntNode()->GetFrm(
                                &aPtPos )->GetUpper(),
                      *pEnd   = rCrsr.GetCntntNode(FALSE)->GetFrm(
                                &aMkPos )->GetUpper();

    //Zuerst lassen wir uns die Tabellen und die Rechtecke heraussuchen.
    SwSelUnions aUnions;

    ::MakeSelUnions( aUnions, pStart, pEnd, eSearchType );

    //Muss ein HeadlineRepeat beachtet werden?
    const BOOL bRepeat = pStart->FindTabFrm()->GetTable()->IsHeadlineRepeat();

    //Jetzt zu jedem Eintrag die Boxen herausfischen und uebertragen.
    for ( USHORT i = 0; i < aUnions.Count(); ++i )
    {
        SwSelUnion *pUnion = aUnions[i];
        const SwTabFrm *pTable = pUnion->GetTable();

        const SwLayoutFrm *pRow = (const SwLayoutFrm*)pTable->Lower();
        //Wenn die Row eine wiederholte Headline ist wird sie nicht
        //beachtet.
        if ( bRepeat && pTable->IsFollow() )
            pRow = (const SwLayoutFrm*)pRow->GetNext();

        while ( pRow )
        {
            if ( pRow->Frm().IsOver( pUnion->GetUnion() ) )
            {
                const SwLayoutFrm *pCell = pRow->FirstCell();

                while ( pCell && pRow->IsAnLower( pCell ) )
                {
                    ASSERT( pCell->IsCellFrm(), "Frame ohne Celle" );
                    if( ::IsFrmInTblSel( pUnion->GetUnion(), pCell ) )
                    {
                        if( pCell->Frm().Width() < nMinValue )
                            return FALSE;
                    }

                    if ( pCell->GetNext() )
                    {
                        pCell = (const SwLayoutFrm*)pCell->GetNext();
                        if ( pCell->Lower()->IsRowFrm() )
                            pCell = pCell->FirstCell();
                    }
                    else
                        pCell = ::lcl_FindNextCellFrm( pCell );
                }
            }
            pRow = (const SwLayoutFrm*)pRow->GetNext();
        }
    }
    return TRUE;
}

// -------------------------------------------------------------------
// Diese Klassen kopieren die aktuelle Tabellen-Selektion (rBoxes)
// unter Beibehaltung der Tabellen-Struktur in eine eigene Struktur
// neu: SS zum gezielten Loeschen/Retaurieren des Layouts.

void lcl_InsertRow( SwTableLine &rLine, SwLayoutFrm *pUpper, SwFrm *pSibling )
{
    SwRowFrm *pRow = new SwRowFrm( rLine );
    if ( pUpper->IsTabFrm() && ((SwTabFrm*)pUpper)->IsFollow() )
    {
        ((SwTabFrm*)pUpper)->FindMaster()->InvalidatePos(); //kann die Zeile vielleicht aufnehmen
        if ( ((SwTabFrm*)pUpper)->GetTable()->IsHeadlineRepeat() &&
             pSibling && !pSibling->GetPrev() )
        {
            //Nicht vor die Headline-Wiederholung pasten.
            pSibling = pSibling->GetNext();
        }
    }
    pRow->Paste( pUpper, pSibling );
    pRow->RegistFlys();
}


BOOL _FndBoxCopyCol( const SwTableBox*& rpBox, void* pPara )
{
    _FndPara* pFndPara = (_FndPara*)pPara;
    _FndBox* pFndBox = new _FndBox( (SwTableBox*)rpBox, pFndPara->pFndLine );
    if( rpBox->GetTabLines().Count() )
    {
        _FndPara aPara( *pFndPara, pFndBox );
        pFndBox->GetBox()->GetTabLines().ForEach( &_FndLineCopyCol, &aPara );
        if( !pFndBox->GetLines().Count() )
        {
            delete pFndBox;
            return TRUE;
        }
    }
    else
    {
        SwTableBoxPtr pSrch = (SwTableBoxPtr)rpBox;
        USHORT nFndPos;
        if( !pFndPara->rBoxes.Seek_Entry( pSrch, &nFndPos ))
        {
            delete pFndBox;
            return TRUE;
        }
    }
    pFndPara->pFndLine->GetBoxes().C40_INSERT( _FndBox, pFndBox,
                    pFndPara->pFndLine->GetBoxes().Count() );
    return TRUE;
}

BOOL _FndLineCopyCol( const SwTableLine*& rpLine, void* pPara )
{
    _FndPara* pFndPara = (_FndPara*)pPara;
    _FndLine* pFndLine = new _FndLine( (SwTableLine*)rpLine, pFndPara->pFndBox );
    _FndPara aPara( *pFndPara, pFndLine );
    pFndLine->GetLine()->GetTabBoxes().ForEach( &_FndBoxCopyCol, &aPara );
    if( pFndLine->GetBoxes().Count() )
    {
        pFndPara->pFndBox->GetLines().C40_INSERT( _FndLine, pFndLine,
                pFndPara->pFndBox->GetLines().Count() );
    }
    else
        delete pFndLine;
    return TRUE;
}

void _FndBox::SetTableLines( const SwSelBoxes &rBoxes, const SwTable &rTable )
{
    //Pointer auf die Lines vor und hinter den zu verarbeitenden Bereich
    //setzen. Wenn die erste/letzte Zeile in den Bereich eingeschlossen
    //sind, so bleiben die Pointer eben einfach 0.
    //Gesucht werden zunachst die Positionen der ersten/letzten betroffenen
    //Line im Array der SwTable. Damit die 0 fuer 'keine Line' verwand werden
    //kann werden die Positionen um 1 nach oben versetzt!

    USHORT nStPos = USHRT_MAX;
    USHORT nEndPos= 0;

    for ( USHORT i = 0; i < rBoxes.Count(); ++i )
    {
        SwTableLine *pLine = rBoxes[i]->GetUpper();
        while ( pLine->GetUpper() )
            pLine = pLine->GetUpper()->GetUpper();
        const USHORT nPos = rTable.GetTabLines().GetPos(
                    (const SwTableLine*&)pLine ) + 1;

        ASSERT( nPos != USHRT_MAX, "TableLine not found." );

        if( nStPos > nPos )
            nStPos = nPos;

        if( nEndPos < nPos )
            nEndPos = nPos;
    }
    if ( nStPos > 1 )
        pLineBefore = rTable.GetTabLines()[nStPos - 2];
    if ( nEndPos < rTable.GetTabLines().Count() )
        pLineBehind = rTable.GetTabLines()[nEndPos];
}

void _FndBox::SetTableLines( const SwTable &rTable )
{
    // Pointer auf die Lines vor und hinter den zu verarbeitenden Bereich
    // setzen. Wenn die erste/letzte Zeile in den Bereich eingeschlossen
    // sind, so bleiben die Pointer eben einfach 0.
    // Die Positionen der ersten/letzten betroffenen Line im Array der
    // SwTable steht in der FndBox. Damit die 0 fuer 'keine Line' verwand
    // werdenkann werden die Positionen um 1 nach oben versetzt!

    if( !GetLines().Count() )
        return;

    SwTableLine* pTmpLine = GetLines()[0]->GetLine();
    USHORT nPos = rTable.GetTabLines().C40_GETPOS( SwTableLine, pTmpLine );
    ASSERT( USHRT_MAX != nPos, "Line steht nicht in der Tabelle" );
    if( nPos )
        pLineBefore = rTable.GetTabLines()[ nPos - 1 ];

    pTmpLine = GetLines()[GetLines().Count()-1]->GetLine();
    nPos = rTable.GetTabLines().C40_GETPOS( SwTableLine, pTmpLine );
    ASSERT( USHRT_MAX != nPos, "Line steht nicht in der Tabelle" );
    if( ++nPos < rTable.GetTabLines().Count() )
        pLineBehind = rTable.GetTabLines()[nPos];
}

inline void UnsetFollow( SwFlowFrm *pTab )
{
    pTab->bIsFollow = FALSE;
}

void _FndBox::DelFrms( SwTable &rTable )
{
    //Alle Lines zwischen pLineBefore und pLineBehind muessen aus dem
    //Layout ausgeschnitten und geloescht werden.
    //Entstehen dabei leere Follows so muessen diese vernichtet werden.
    //Wird ein Master vernichtet, so muss der Follow Master werden.
    //Ein TabFrm muss immer uebrigbleiben.

    USHORT nStPos = 0;
    USHORT nEndPos= rTable.GetTabLines().Count() - 1;
    if ( pLineBefore )
    {
        nStPos = rTable.GetTabLines().GetPos(
                        (const SwTableLine*&)pLineBefore );
        ASSERT( nStPos != USHRT_MAX, "Fuchs Du hast die Line gestohlen!" );
        ++nStPos;
    }
    if ( pLineBehind )
    {
        nEndPos = rTable.GetTabLines().GetPos(
                        (const SwTableLine*&)pLineBehind );
        ASSERT( nEndPos != USHRT_MAX, "Fuchs Du hast die Line gestohlen!" );
        --nEndPos;
    }
    for ( USHORT i = nStPos; i <= nEndPos; ++i)
    {
        SwFrmFmt *pFmt = rTable.GetTabLines()[i]->GetFrmFmt();
        SwClientIter aIter( *pFmt );
        SwClient* pLast = aIter.GoStart();
        if( pLast )
        {
            do {
                SwFrm *pFrm = PTR_CAST( SwFrm, pLast );
                if ( pFrm &&
                     ((SwRowFrm*)pFrm)->GetTabLine() == rTable.GetTabLines()[i] )
                {
                    BOOL bDel = TRUE;
                    SwTabFrm *pUp = !pFrm->GetPrev() && !pFrm->GetNext() ?
                                            (SwTabFrm*)pFrm->GetUpper() : 0;
                    if ( !pUp )
                    {
                        if ( ((SwTabFrm*)pFrm->GetUpper())->GetTable()->IsHeadlineRepeat() &&
                             ((SwTabFrm*)pFrm->GetUpper())->IsFollow() )
                        {
                            if ( !pFrm->GetNext() && pFrm->GetPrev() &&
                                 !pFrm->GetPrev()->GetPrev() )
                            {
                                pUp = (SwTabFrm*)pFrm->GetUpper();
                            }
                        }
                    }
                    if ( pUp )
                    {
                        SwTabFrm *pFollow = pUp->GetFollow();
                        SwTabFrm *pPrev   = pUp->IsFollow() ? pUp : 0;
                        if ( pPrev )
                        {
                            SwFrm *pTmp = pPrev->FindPrev();
                            ASSERT( pTmp->IsTabFrm(),
                                    "Vorgaenger vom Follow kein Master.");
                            pPrev = (SwTabFrm*)pTmp;
                        }
                        if ( pPrev )
                            pPrev->SetFollow( pFollow );
                        else if ( pFollow )
                            ::UnsetFollow( pFollow );

                        //Ein TabellenFrm muss immer stehenbleiben!
                        if ( pPrev || pFollow )
                        {
                            pUp->Cut();
                            delete pUp;
                            bDel = FALSE;//Die Row wird mit in den Abgrund
                                         //gerissen.
                        }
                    }
                    if ( bDel )
                    {
                        pFrm->Cut();
                        delete pFrm;
                    }
                }
            } while( 0 != ( pLast = aIter++ ));
        }
    }
}

BOOL lcl_IsLineOfTblFrm( const SwTabFrm& rTable, const SwFrm& rChk )
{
    const SwTabFrm* pTblFrm = rChk.FindTabFrm();
    while( pTblFrm->IsFollow() )
        pTblFrm = pTblFrm->FindMaster();
    return &rTable == pTblFrm;
}

void _FndBox::MakeFrms( SwTable &rTable )
{
    //Alle Lines zwischen pLineBefore und pLineBehind muessen im Layout
    //wieder neu erzeugt werden.
    //Und Zwar fuer alle Auspraegungen der Tabelle (mehrere z.B. im Kopf/Fuss).

    USHORT nStPos = 0;
    USHORT nEndPos= rTable.GetTabLines().Count() - 1;
    if ( pLineBefore )
    {
        nStPos = rTable.GetTabLines().GetPos(
                        (const SwTableLine*&)pLineBefore );
        ASSERT( nStPos != USHRT_MAX, "Fuchs Du hast die Line gestohlen!" );
        ++nStPos;

    }
    if ( pLineBehind )
    {
        nEndPos = rTable.GetTabLines().GetPos(
                        (const SwTableLine*&)pLineBehind );
        ASSERT( nEndPos != USHRT_MAX, "Fuchs Du hast die Line gestohlen!" );
        --nEndPos;
    }
    //Jetzt die grosse Einfuegeoperation fuer alle Tabllen.
    SwClientIter aTabIter( *rTable.GetFrmFmt() );
    for ( SwTabFrm *pTable = (SwTabFrm*)aTabIter.First( TYPE(SwFrm) ); pTable;
          pTable = (SwTabFrm*)aTabIter.Next() )
    {
        if ( !pTable->IsFollow() )
        {
            SwFrm  *pSibling = 0;
            SwFrm  *pUpper   = 0;
            int i;
            for ( i = rTable.GetTabLines().Count()-1;
                    i >= 0 && !pSibling; --i )
            {
                SwTableLine *pLine = pLineBehind ? pLineBehind :
                                                    rTable.GetTabLines()[i];
                SwClientIter aIter( *pLine->GetFrmFmt() );
                for ( pSibling = (SwFrm*)aIter.First( TYPE(SwFrm) );
                      pSibling && (
                        ((SwRowFrm*)pSibling)->GetTabLine() != pLine ||
                        !lcl_IsLineOfTblFrm( *pTable, *pSibling ) );
                      pSibling = (SwFrm*)aIter.Next() )
                    /* do nothing */;
            }
            if ( pSibling )
            {
                pUpper = pSibling->GetUpper();
                if ( !pLineBehind )
                    pSibling = 0;
            }
            else
// ???? oder das der Letzte Follow der Tabelle ????
                pUpper = pTable;

            for ( i = nStPos; (USHORT)i <= nEndPos; ++i )
                ::lcl_InsertRow( *rTable.GetTabLines()[i],
                                (SwLayoutFrm*)pUpper, pSibling );
            if ( pUpper->IsTabFrm() )
                ((SwTabFrm*)pUpper)->SetCalcLowers();
        }
        else if ( nStPos == 0 && rTable.IsHeadlineRepeat() )
        {
            //Headline in den Follow einsetzen
            SwRowFrm *pRow = new SwRowFrm( *rTable.GetTabLines()[0] );
            pRow->Paste( pTable, pTable->Lower() );
            pRow->RegistFlys();
            pTable->SetCalcLowers();
        }
    }
}

void _FndBox::MakeNewFrms( SwTable &rTable, const USHORT nNumber,
                                            const BOOL bBehind )
{
    //Frms fuer neu eingefuege Zeilen erzeugen.
    //bBehind == TRUE:  vor     pLineBehind
    //        == FALSE: hinter  pLineBefore
    const USHORT nBfPos = pLineBefore ?
        rTable.GetTabLines().GetPos( (const SwTableLine*&)pLineBefore ) :
        USHRT_MAX;
    const USHORT nBhPos = pLineBehind ?
        rTable.GetTabLines().GetPos( (const SwTableLine*&)pLineBehind ) :
        USHRT_MAX;

    //nNumber: wie oft ist eingefuegt worden.
    //nCnt:    wieviele sind nNumber mal eingefuegt worden.

    const USHORT nCnt =
        ((nBhPos != USHRT_MAX ? nBhPos : rTable.GetTabLines().Count()) -
         (nBfPos != USHRT_MAX ? nBfPos + 1 : 0)) / (nNumber + 1);

    //Den Master-TabFrm suchen
    SwClientIter aTabIter( *rTable.GetFrmFmt() );
    SwTabFrm *pTable;
    for ( pTable = (SwTabFrm*)aTabIter.First( TYPE(SwFrm) ); pTable;
          pTable = (SwTabFrm*)aTabIter.Next() )
        if( !pTable->IsFollow() )
        {
            SwFrm       *pSibling = 0;
            SwLayoutFrm *pUpper   = 0;
            if ( bBehind )
            {
                if ( pLineBehind )
                {
                    SwClientIter aIter( *pLineBehind->GetFrmFmt() );
                    for ( pSibling = (SwFrm*)aIter.First( TYPE(SwFrm) );
                          pSibling && (
                            ((SwRowFrm*)pSibling)->GetTabLine() != pLineBehind ||
                            !lcl_IsLineOfTblFrm( *pTable, *pSibling ) );
                          pSibling = (SwFrm*)aIter.Next() )
                        /* do nothing */;
                }
                if ( pSibling )
                    pUpper = pSibling->GetUpper();
                else
                {
                    while( pTable->GetFollow() )
                        pTable = pTable->GetFollow();
                    pUpper = pTable;
                }
                const USHORT nMax = nBhPos != USHRT_MAX ?
                                    nBhPos : rTable.GetTabLines().Count();

                USHORT i = nBfPos != USHRT_MAX ? nBfPos + 1 + nCnt : nCnt;

                for ( ; i < nMax; ++i )
                    ::lcl_InsertRow( *rTable.GetTabLines()[i], pUpper, pSibling );
                if ( pUpper->IsTabFrm() )
                    ((SwTabFrm*)pUpper)->SetCalcLowers();
            }
            else //davor einfuegen
            {
                USHORT i;
                for ( i = 0; !pSibling; ++i )
                {
                    SwTableLine *pLine = pLineBefore ? pLineBefore :
                                                    rTable.GetTabLines()[i];

                    SwClientIter aIter( *pLine->GetFrmFmt() );
                    for ( pSibling = (SwFrm*)aIter.First( TYPE(SwFrm) );
                          pSibling && (
                            ((SwRowFrm*)pSibling)->GetTabLine() != pLine ||
                            !lcl_IsLineOfTblFrm( *pTable, *pSibling ) ||
                            ((!pLineBefore || pLine == rTable.GetTabLines()[0]) &&
                             pSibling->FindTabFrm() != pTable)); // Master finden!
                          pSibling = (SwFrm*)aIter.Next() )
                        /* do nothing */;
                }
                pUpper = pSibling->GetUpper();
                if ( pLineBefore )
                    pSibling = pSibling->GetNext();

                USHORT nMax = nBhPos != USHRT_MAX ?
                                    nBhPos - nCnt :
                                    rTable.GetTabLines().Count() - nCnt;

                i = nBfPos != USHRT_MAX ? nBfPos + 1 : 0;
                for ( ; i < nMax; ++i )
                    ::lcl_InsertRow( *rTable.GetTabLines()[i],
                                pUpper, pSibling );
                if ( pUpper->IsTabFrm() )
                    ((SwTabFrm*)pUpper)->SetCalcLowers();
            }
        }

    //Die Headlines mussen ggf. auch verarbeitet werden. Um gut arbeitenden
    //Code nicht zu zerfasern wird hier nochmals iteriert.
    if ( !bBehind && nBfPos == USHRT_MAX && rTable.IsHeadlineRepeat() )
    {
        SwTabFrm *pTab = (SwTabFrm*)aTabIter.First( TYPE(SwFrm) );
        if ( pTab->Lower() )
        {
            if ( pTab->IsFollow() )
            {
                //Alte Headline vernichten
                SwFrm *pLow = pTab->Lower();
                pLow->Cut();
                delete pLow;
            }
            if ( ((SwRowFrm*)pTab->Lower())->GetTabLine() !=
                 rTable.GetTabLines()[0] )
            {
                //Neue Headline einsetzen
                SwRowFrm *pRow = new SwRowFrm( *rTable.GetTabLines()[0]);
                pRow->Paste( pTab, pTab->Lower() );
                pRow->RegistFlys();
                pTab->SetCalcLowers();
            }
        }
    }
}

BOOL _FndBox::AreLinesToRestore( const SwTable &rTable ) const
{
    //Lohnt es sich MakeFrms zu rufen?

    if ( !pLineBefore && !pLineBehind && rTable.GetTabLines().Count() )
        return TRUE;

    USHORT nBfPos;
    if(pLineBefore)
    {
        const SwTableLine* rLBefore = (const SwTableLine*)pLineBefore;
        nBfPos = rTable.GetTabLines().GetPos( rLBefore );
    }
    else
        nBfPos = USHRT_MAX;

    USHORT nBhPos;
    if(pLineBehind)
    {
        const SwTableLine* rLBehind = (const SwTableLine*)pLineBehind;
        nBhPos = rTable.GetTabLines().GetPos( rLBehind );
    }
    else
        nBhPos = USHRT_MAX;

    if ( nBfPos == nBhPos ) //Duerfte eigentlich nie vorkommen.
    {
        ASSERT( FALSE, "Table, Loeschen auf keinem Bereich !?!" );
        return FALSE;
    }

    if ( nBfPos == USHRT_MAX && nBhPos == 0 )
    {
        // ups. sollte unsere zu wiederholende Kopfzeile geloescht worden
        // sein??
        if( rTable.IsHeadlineRepeat() )
        {
            SwClientIter aIter( *rTable.GetFrmFmt() );
            for( SwTabFrm* pTable = (SwTabFrm*)aIter.First( TYPE( SwFrm ));
                    pTable; pTable = (SwTabFrm*)aIter.Next() )
                if( pTable->IsFollow() )
                {
                    //Headline in den Follow einsetzen
                    SwRowFrm *pRow = new SwRowFrm( *rTable.GetTabLines()[0] );
                    pRow->Paste( pTable, pTable->Lower() );
                    pRow->RegistFlys();
                }
        }
        return FALSE;
    }

    if ( nBhPos == USHRT_MAX && nBfPos == (rTable.GetTabLines().Count() - 1) )
        return FALSE;

    if ( nBfPos != USHRT_MAX && nBhPos != USHRT_MAX && (nBfPos + 1) == nBhPos )
        return FALSE;

    return TRUE;
}


//Save- und RestoreChartData:
//Zu der Tabelle werden alle Charts gesucht. Die Namentliche Addressierung der
//Boxen in der Tabelle (etwa: <A1:C3>) wird ausgelesen. Die Addressen der
//Boxen werden im Chart festgehalten. Im Restore wird versucht zu den Pointern
//die Boxen wiederzufinden. Wenn dies gelingt, wird die neue Addressierung
//wieder in das Chart geschrieben. Wenn sie nicht gefunden werden gibt es
//einen FallBack auf die erste/letzte Box.

const SwTableBox *lcl_FindFirstBox( const SwTable &rTable )
{
    const SwTableLines *pLines = &rTable.GetTabLines();
    const SwTableBox *pBox;
    do
    {   pBox = (*pLines)[0]->GetTabBoxes()[0];
        if ( pBox->GetSttNd() )
            pLines = 0;
        else
            pLines = &pBox->GetTabLines();

    } while ( pLines );
    return pBox;
}

const SwTableBox *lcl_FindLastBox( const SwTable &rTable )
{
    const SwTableLines *pLines = &rTable.GetTabLines();
    const SwTableBox *pBox;
    do
    {   const SwTableBoxes &rBoxes = (*pLines)[pLines->Count()-1]->GetTabBoxes();
        pBox = rBoxes[rBoxes.Count()-1];
        if ( pBox->GetSttNd() )
            pLines = 0;
        else
            pLines = &pBox->GetTabLines();

    } while ( pLines );

    return pBox;
}


//GPF bei Tab in letzer Zelle mit MSC4
#pragma optimize("",off)

void _FndBox::SaveChartData( const SwTable &rTable )
{
    SwDoc *pDoc = rTable.GetFrmFmt()->GetDoc();
    SwClientIter aIter( *(SwModify*)pDoc->GetDfltGrfFmtColl() );
    SwClient *pCli;
    if ( 0 != (pCli = aIter.First( TYPE(SwCntntNode) )) )
        do
        {   if ( !((SwCntntNode*)pCli)->GetOLENode() )
                continue;
            SwOLENode *pONd = (SwOLENode*)pCli;
            if ( rTable.GetFrmFmt()->GetName() == pONd->GetChartTblName() )
            {
                SwOLEObj& rOObj = pONd->GetOLEObj();
                SchMemChart *pData = SchDLL::GetChartData( rOObj.GetOleRef() );
                if ( pData )
                {
                    String &rStr = pData->SomeData1();
                    xub_StrLen nTmp = rStr.Search( ':' );
                    String aBox( rStr.Copy( 1, nTmp - 1 ) );
                    //const this, weil Borland so dumm ist!
                    const SwTableBox *pSttBox = rTable.GetTblBox( aBox );
                    if ( !pSttBox )
                        pSttBox = rTable.GetTabLines()[0]->GetTabBoxes()[0];
                    aBox = rStr.Copy( nTmp + 1, rStr.Len()-2 - nTmp);
                    const SwTableBox *pEndBox = rTable.GetTblBox( aBox );
                    if ( !pEndBox )
                    {
                        SwTableLine *pLine =
                            rTable.GetTabLines()[rTable.GetTabLines().Count()-1];
                        pEndBox = pLine->GetTabBoxes()[pLine->GetTabBoxes().Count()-1];
                    }
                    pData->SomeData3() = String::CreateFromInt32(
                                        pSttBox != ::lcl_FindFirstBox(rTable)
                                            ? long(pSttBox)
                                            : LONG_MAX );
                    pData->SomeData4() = String::CreateFromInt32(
                                        pEndBox != ::lcl_FindLastBox(rTable)
                                            ? long(pEndBox)
                                            : LONG_MAX );
                }
            }
        } while ( 0 != (pCli = aIter.Next()) );
}

void _FndBox::RestoreChartData( const SwTable &rTable )
{
    SwDoc *pDoc = rTable.GetFrmFmt()->GetDoc();
    SwClientIter aIter( *(SwModify*)pDoc->GetDfltGrfFmtColl() );
    SwClient *pCli;
    if ( 0 != (pCli = aIter.First( TYPE(SwCntntNode) )) )
        do
        {   if ( !((SwCntntNode*)pCli)->GetOLENode() )
                continue;
            SwOLENode *pONd = (SwOLENode*)pCli;
            if ( rTable.GetFrmFmt()->GetName() == pONd->GetChartTblName() )
            {
                SwOLEObj& rOObj = pONd->GetOLEObj();
                SchMemChart *pData = SchDLL::GetChartData( rOObj.GetOleRef() );
                if ( pData )
                {
                    const SwTableBox *pSttBox = (SwTableBox*)
                                                pData->SomeData3().ToInt32();
                    if ( long(pSttBox) == LONG_MAX )
                        pSttBox = ::lcl_FindFirstBox( rTable );
                    const SwTableBox *pEndBox = (SwTableBox*)
                                                pData->SomeData4().ToInt32();
                    if ( long(pEndBox) == LONG_MAX )
                        pEndBox = ::lcl_FindLastBox( rTable );
                    FASTBOOL bSttFound = FALSE, bEndFound = FALSE;
                    const SwTableSortBoxes &rBoxes = rTable.GetTabSortBoxes();
                    for ( USHORT i = 0; i < rBoxes.Count(); ++i )
                    {
                        const SwTableBox *pTmp = rBoxes[i];
                        if ( pTmp == pSttBox )
                            bSttFound = TRUE;
                        if ( pTmp == pEndBox )
                            bEndFound = TRUE;
                    }
                    if ( !bSttFound )
                        pSttBox = rTable.GetTabLines()[0]->GetTabBoxes()[0];
                    if ( !bEndFound )
                    {
                        SwTableLine *pLine =
                            rTable.GetTabLines()[rTable.GetTabLines().Count()-1];
                        pEndBox = pLine->GetTabBoxes()[pLine->GetTabBoxes().Count()-1];
                    }
                    String &rStr = pData->SomeData1();
                    rStr = '<'; rStr += pSttBox->GetName(); rStr += ':';
                    rStr += pEndBox->GetName(); rStr += '>';
                    pData->SomeData3().Erase(); pData->SomeData4().Erase();
                    SchDLL::Update( rOObj.GetOleRef(), pData );
                }
            }
        } while ( 0 != (pCli = aIter.Next()) );
}

#pragma optimize("",on)

