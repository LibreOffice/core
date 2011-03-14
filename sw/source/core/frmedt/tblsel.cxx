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

#include <editeng/boxitem.hxx>
#include <editeng/protitem.hxx>

#include <hintids.hxx>
#include <fmtanchr.hxx>
#include <fmtfsize.hxx>
#include <frmatr.hxx>
#include <tblsel.hxx>
#include <crsrsh.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <docary.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <ndole.hxx>
#include <swtable.hxx>
#include <cntfrm.hxx>
#include <tabfrm.hxx>
#include <rowfrm.hxx>
#include <cellfrm.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <viscrs.hxx>
#include <swtblfmt.hxx>
#include <UndoTable.hxx>
#include <mvsave.hxx>
// OD 26.08.2003 #i18103#
#include <sectfrm.hxx>
#include <frmtool.hxx>
#include <deque>

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
sal_Bool SwSelBoxes::Seek_Entry( const SwTableBoxPtr rSrch, sal_uInt16* pFndPos ) const
{
    sal_uLong nIdx = rSrch->GetSttIdx();

    sal_uInt16 nO = Count(), nM, nU = 0;
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
                return sal_True;
            }
            else if( (*this)[ nM ]->GetSttIdx() < nIdx )
                nU = nM + 1;
            else if( nM == 0 )
            {
                if( pFndPos )
                    *pFndPos = nU;
                return sal_False;
            }
            else
                nO = nM - 1;
        }
    }
    if( pFndPos )
        *pFndPos = nU;
    return sal_False;
}


SV_IMPL_PTRARR( SwCellFrms, SwCellFrm* )

struct _CmpLPt
{
    Point aPos;
    const SwTableBox* pSelBox;
    sal_Bool bVert;

    _CmpLPt( const Point& rPt, const SwTableBox* pBox, sal_Bool bVertical );

    sal_Bool operator==( const _CmpLPt& rCmp ) const
    {   return X() == rCmp.X() && Y() == rCmp.Y() ? sal_True : sal_False; }

    sal_Bool operator<( const _CmpLPt& rCmp ) const
    {
        if ( bVert )
            return X() > rCmp.X() || ( X() == rCmp.X() && Y() < rCmp.Y() )
                    ? sal_True : sal_False;
        else
            return Y() < rCmp.Y() || ( Y() == rCmp.Y() && X() < rCmp.X() )
                    ? sal_True : sal_False;
    }

    long X() const { return aPos.X(); }
    long Y() const { return aPos.Y(); }
};


SV_DECL_VARARR_SORT( _MergePos, _CmpLPt, 0, 40 )
SV_IMPL_VARARR_SORT( _MergePos, _CmpLPt )

SV_IMPL_PTRARR( _FndBoxes, _FndBox* )
SV_IMPL_PTRARR( _FndLines, _FndLine* )


struct _Sort_CellFrm
{
    const SwCellFrm* pFrm;

    _Sort_CellFrm( const SwCellFrm& rCFrm )
        : pFrm( &rCFrm ) {}
};

typedef std::deque< _Sort_CellFrm > _Sort_CellFrms;

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
        rBoxes.Remove( sal_uInt16(0), rBoxes.Count() );
    if( rShell.IsTableMode() && ((SwCrsrShell&)rShell).UpdateTblSelBoxes())
        rBoxes.Insert( &rShell.GetTableCrsr()->GetBoxes() );
}

void GetTblSelCrs( const SwTableCursor& rTblCrsr, SwSelBoxes& rBoxes )
{
    if( rBoxes.Count() )
        rBoxes.Remove( sal_uInt16(0), rBoxes.Count() );

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

    GetTblSel( *rShell.getShellCrsr(false), rBoxes, eSearchType );
}

void GetTblSel( const SwCursor& rCrsr, SwSelBoxes& rBoxes,
                const SwTblSearchType eSearchType )
{
    //Start- und Endzelle besorgen und den naechsten fragen.
    OSL_ENSURE( rCrsr.GetCntntNode() && rCrsr.GetCntntNode( sal_False ),
            "Tabselection nicht auf Cnt." );

    // Zeilen-Selektion:
    // teste ob Tabelle komplex ist. Wenn ja, dann immer uebers Layout
    // die selektierten Boxen zusammen suchen. Andernfalls ueber die
    // Tabellen-Struktur (fuer Makros !!)
    const SwCntntNode* pContentNd = rCrsr.GetNode()->GetCntntNode();
    const SwTableNode* pTblNd = pContentNd ? pContentNd->FindTableNode() : 0;
    if( pTblNd && pTblNd->GetTable().IsNewModel() )
    {
        SwTable::SearchType eSearch;
        switch( nsSwTblSearchType::TBLSEARCH_COL & eSearchType )
        {
            case nsSwTblSearchType::TBLSEARCH_ROW: eSearch = SwTable::SEARCH_ROW; break;
            case nsSwTblSearchType::TBLSEARCH_COL: eSearch = SwTable::SEARCH_COL; break;
            default: eSearch = SwTable::SEARCH_NONE; break;
        }
        const bool bChkP = 0 != ( nsSwTblSearchType::TBLSEARCH_PROTECT & eSearchType );
        pTblNd->GetTable().CreateSelection( rCrsr, rBoxes, eSearch, bChkP );
        return;
    }
    if( nsSwTblSearchType::TBLSEARCH_ROW == ((~nsSwTblSearchType::TBLSEARCH_PROTECT ) & eSearchType ) &&
        pTblNd && !pTblNd->GetTable().IsTblComplex() )
    {
        const SwTable& rTbl = pTblNd->GetTable();
        const SwTableLines& rLines = rTbl.GetTabLines();

        const SwNode* pMarkNode = rCrsr.GetNode( sal_False );
        const sal_uLong nMarkSectionStart = pMarkNode->StartOfSectionIndex();
        const SwTableBox* pMarkBox = rTbl.GetTblBox( nMarkSectionStart );

        OSL_ENSURE( pMarkBox, "Point in table, mark outside?" );

        const SwTableLine* pLine = pMarkBox ? pMarkBox->GetUpper() : 0;
        sal_uInt16 nSttPos = rLines.GetPos( pLine );
        OSL_ENSURE( USHRT_MAX != nSttPos, "Wo ist meine Zeile in der Tabelle?" );
        pLine = rTbl.GetTblBox( rCrsr.GetNode( sal_True )->StartOfSectionIndex() )->GetUpper();
        sal_uInt16 nEndPos = rLines.GetPos( pLine );
        OSL_ENSURE( USHRT_MAX != nEndPos, "Wo ist meine Zeile in der Tabelle?" );
        // pb: #i20193# if tableintable then nSttPos == nEndPos == USHRT_MAX
        if ( nSttPos != USHRT_MAX && nEndPos != USHRT_MAX )
        {
            if( nEndPos < nSttPos )     // vertauschen
            {
                sal_uInt16 nTmp = nSttPos; nSttPos = nEndPos; nEndPos = nTmp;
            }

            int bChkProtected = nsSwTblSearchType::TBLSEARCH_PROTECT & eSearchType;
            for( ; nSttPos <= nEndPos; ++nSttPos )
            {
                pLine = rLines[ nSttPos ];
                for( sal_uInt16 n = pLine->GetTabBoxes().Count(); n ; )
                {
                    SwTableBox* pBox = pLine->GetTabBoxes()[ --n ];
                    // Zellenschutzt beachten ??
                    if( !bChkProtected ||
                        !pBox->GetFrmFmt()->GetProtect().IsCntntProtected() )
                        rBoxes.Insert( pBox );
                }
            }
        }
    }
    else
    {
        Point aPtPos, aMkPos;
        const SwShellCrsr* pShCrsr = dynamic_cast<const SwShellCrsr*>(&rCrsr);
        if( pShCrsr )
        {
            aPtPos = pShCrsr->GetPtPos();
            aMkPos = pShCrsr->GetMkPos();
        }
        const SwCntntNode *pCntNd = rCrsr.GetCntntNode();
        const SwLayoutFrm *pStart = pCntNd ?
            pCntNd->GetFrm( &aPtPos )->GetUpper() : 0;
        pCntNd = rCrsr.GetCntntNode(sal_False);
        const SwLayoutFrm *pEnd = pCntNd ?
            pCntNd->GetFrm( &aMkPos )->GetUpper() : 0;
        if( pStart && pEnd )
            GetTblSel( pStart, pEnd, rBoxes, 0, eSearchType );
    }
}

void GetTblSel( const SwLayoutFrm* pStart, const SwLayoutFrm* pEnd,
                SwSelBoxes& rBoxes, SwCellFrms* pCells,
                const SwTblSearchType eSearchType )
{
    const SwTabFrm* pStartTab = pStart->FindTabFrm();
    if ( !pStartTab )
    {
        OSL_ENSURE( false, "GetTblSel without start table" );
        return;
    }

    int bChkProtected = nsSwTblSearchType::TBLSEARCH_PROTECT & eSearchType;

    sal_Bool bTblIsValid;
    // #i55421# Reduced value 10
    int nLoopMax = 10;
    sal_uInt16 i;

    do {
        bTblIsValid = sal_True;

        //Zuerst lassen wir uns die Tabellen und die Rechtecke heraussuchen.
        SwSelUnions aUnions;
        ::MakeSelUnions( aUnions, pStart, pEnd, eSearchType );

        Point aCurrentTopLeft( LONG_MAX, LONG_MAX );
        Point aCurrentTopRight( 0, LONG_MAX );
        Point aCurrentBottomLeft( LONG_MAX, 0 );
        Point aCurrentBottomRight( 0, 0 );
        const SwCellFrm* pCurrentTopLeftFrm     = 0;
        const SwCellFrm* pCurrentTopRightFrm    = 0;
        const SwCellFrm* pCurrentBottomLeftFrm  = 0;
        const SwCellFrm* pCurrentBottomRightFrm  = 0;

        //Jetzt zu jedem Eintrag die Boxen herausfischen und uebertragen.
        for( i = 0; i < aUnions.Count() && bTblIsValid; ++i )
        {
            SwSelUnion *pUnion = aUnions[i];
            const SwTabFrm *pTable = pUnion->GetTable();
            if( !pTable->IsValid() && nLoopMax )
            {
                bTblIsValid = sal_False;
                break;
            }

            // Skip any repeated headlines in the follow:
            const SwLayoutFrm* pRow = pTable->IsFollow() ?
                                      pTable->GetFirstNonHeadlineRow() :
                                     (const SwLayoutFrm*)pTable->Lower();

            while( pRow && bTblIsValid )
            {
                if( !pRow->IsValid() && nLoopMax )
                {
                    bTblIsValid = sal_False;
                    break;
                }

                if ( pRow->Frm().IsOver( pUnion->GetUnion() ) )
                {
                    const SwLayoutFrm *pCell = pRow->FirstCell();

                    while( bTblIsValid && pCell && pRow->IsAnLower( pCell ) )
                    {
                        if( !pCell->IsValid() && nLoopMax )
                        {
                            bTblIsValid = sal_False;
                            break;
                        }

                        OSL_ENSURE( pCell->IsCellFrm(), "Frame ohne Celle" );
                        if( ::IsFrmInTblSel( pUnion->GetUnion(), pCell ) )
                        {
                            SwTableBox* pBox = (SwTableBox*)
                                ((SwCellFrm*)pCell)->GetTabBox();
                            // Zellenschutzt beachten ??
                            if( !bChkProtected ||
                                !pBox->GetFrmFmt()->GetProtect().IsCntntProtected() )
                                rBoxes.Insert( pBox );

                            if ( pCells )
                            {
                                const Point aTopLeft( pCell->Frm().TopLeft() );
                                const Point aTopRight( pCell->Frm().TopRight() );
                                const Point aBottomLeft( pCell->Frm().BottomLeft() );
                                const Point aBottomRight( pCell->Frm().BottomRight() );

                                if ( aTopLeft.Y() < aCurrentTopLeft.Y() ||
                                     ( aTopLeft.Y() == aCurrentTopLeft.Y() &&
                                       aTopLeft.X() <  aCurrentTopLeft.X() ) )
                                {
                                    aCurrentTopLeft = aTopLeft;
                                    pCurrentTopLeftFrm = static_cast<const SwCellFrm*>( pCell );
                                }

                                if ( aTopRight.Y() < aCurrentTopRight.Y() ||
                                     ( aTopRight.Y() == aCurrentTopRight.Y() &&
                                       aTopRight.X() >  aCurrentTopRight.X() ) )
                                {
                                    aCurrentTopRight = aTopRight;
                                    pCurrentTopRightFrm = static_cast<const SwCellFrm*>( pCell );
                                }

                                if ( aBottomLeft.Y() > aCurrentBottomLeft.Y() ||
                                     ( aBottomLeft.Y() == aCurrentBottomLeft.Y() &&
                                       aBottomLeft.X() <  aCurrentBottomLeft.X() ) )
                                {
                                    aCurrentBottomLeft = aBottomLeft;
                                    pCurrentBottomLeftFrm = static_cast<const SwCellFrm*>( pCell );
                                }

                                if ( aBottomRight.Y() > aCurrentBottomRight.Y() ||
                                     ( aBottomRight.Y() == aCurrentBottomRight.Y() &&
                                       aBottomRight.X() >  aCurrentBottomRight.X() ) )
                                {
                                    aCurrentBottomRight = aBottomRight;
                                    pCurrentBottomRightFrm = static_cast<const SwCellFrm*>( pCell );
                                }

                            }
                        }
                        if ( pCell->GetNext() )
                        {
                            pCell = (const SwLayoutFrm*)pCell->GetNext();
                            if ( pCell->Lower() && pCell->Lower()->IsRowFrm() )
                                pCell = pCell->FirstCell();
                        }
                        else
                            pCell = ::lcl_FindNextCellFrm( pCell );
                    }
                }
                pRow = (const SwLayoutFrm*)pRow->GetNext();
            }
        }

        if ( pCells )
        {
            pCells->Remove( 0, pCells->Count() );
            pCells->Insert( pCurrentTopLeftFrm, 0 );
            pCells->Insert( pCurrentTopRightFrm, 1 );
            pCells->Insert( pCurrentBottomLeftFrm, 2 );
            pCells->Insert( pCurrentBottomRightFrm, 3 );
        }

        if( bTblIsValid )
            break;

        SwDeletionChecker aDelCheck( pStart );

        // ansonsten das Layout der Tabelle kurz "kalkulieren" lassen
        // und nochmals neu aufsetzen
        SwTabFrm *pTable = aUnions[0]->GetTable();
        while( pTable )
        {
            if( pTable->IsValid() )
                pTable->InvalidatePos();
            pTable->SetONECalcLowers();
            pTable->Calc();
            pTable->SetCompletePaint();
            if( 0 == (pTable = pTable->GetFollow()) )
                break;
        }

        // --> Make code robust, check if pStart has
        // been deleted due to the formatting of the table:
        if ( aDelCheck.HasBeenDeleted() )
        {
            OSL_ENSURE( false, "Current box has been deleted during GetTblSel()" );
            break;
        }
        // <--

        i = 0;
        rBoxes.Remove( i, rBoxes.Count() );
        --nLoopMax;

    } while( sal_True );
    OSL_ENSURE( nLoopMax, "das Layout der Tabelle wurde nicht valide!" );
}



sal_Bool ChkChartSel( const SwNode& rSttNd, const SwNode& rEndNd,
                    SwChartLines* pGetCLines )
{
    const SwTableNode* pTNd = rSttNd.FindTableNode();
    if( !pTNd )
        return sal_False;

    Point aNullPos;
    SwNodeIndex aIdx( rSttNd );
    const SwCntntNode* pCNd = aIdx.GetNode().GetCntntNode();
    if( !pCNd )
        pCNd = aIdx.GetNodes().GoNextSection( &aIdx, sal_False, sal_False );

    // if table is invisible, return
    // (layout needed for forming table selection further down, so we can't
    //  continue with invisible tables)
    // #i22135# - Also the content of the table could be
    //                          invisible - e.g. in a hidden section
    // Robust: check, if content was found (e.g. empty table cells)
    if ( !pCNd || pCNd->GetFrm() == NULL )
            return sal_False;

    const SwLayoutFrm *pStart = pCNd ? pCNd->GetFrm( &aNullPos )->GetUpper() : 0;
    OSL_ENSURE( pStart, "ohne Frame geht gar nichts" );

    aIdx = rEndNd;
    pCNd = aIdx.GetNode().GetCntntNode();
    if( !pCNd )
        pCNd = aIdx.GetNodes().GoNextSection( &aIdx, sal_False, sal_False );

    // #i22135# - Robust: check, if content was found and if it's visible
    if ( !pCNd || pCNd->GetFrm() == NULL )
    {
        return sal_False;
    }

    const SwLayoutFrm *pEnd = pCNd ? pCNd->GetFrm( &aNullPos )->GetUpper() : 0;
    OSL_ENSURE( pEnd, "ohne Frame geht gar nichts" );


    sal_Bool bTblIsValid, bValidChartSel;
    // --> FME 2006-01-25 #i55421# Reduced value 10
    int nLoopMax = 10;      //JP 28.06.99: max 100 loops - Bug 67292
    // <--
    sal_uInt16 i = 0;

    do {
        bTblIsValid = sal_True;
        bValidChartSel = sal_True;

        sal_uInt16 nRowCells = USHRT_MAX;

        //Zuerst lassen wir uns die Tabellen und die Rechtecke heraussuchen.
        SwSelUnions aUnions;
        ::MakeSelUnions( aUnions, pStart, pEnd, nsSwTblSearchType::TBLSEARCH_NO_UNION_CORRECT );

        //Jetzt zu jedem Eintrag die Boxen herausfischen und uebertragen.
        for( i = 0; i < aUnions.Count() && bTblIsValid &&
                                    bValidChartSel; ++i )
        {
            SwSelUnion *pUnion = aUnions[i];
            const SwTabFrm *pTable = pUnion->GetTable();

            SWRECTFN( pTable )
            sal_Bool bRTL = pTable->IsRightToLeft();

            if( !pTable->IsValid() && nLoopMax  )
            {
                bTblIsValid = sal_False;
                break;
            }

            _Sort_CellFrms aCellFrms;

            // Skip any repeated headlines in the follow:
            const SwLayoutFrm* pRow = pTable->IsFollow() ?
                                      pTable->GetFirstNonHeadlineRow() :
                                      (const SwLayoutFrm*)pTable->Lower();

            while( pRow && bTblIsValid && bValidChartSel )
            {
                if( !pRow->IsValid() && nLoopMax )
                {
                    bTblIsValid = sal_False;
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
                            bTblIsValid = sal_False;
                            break;
                        }

                        OSL_ENSURE( pCell->IsCellFrm(), "Frame ohne Celle" );
                        const SwRect& rUnion = pUnion->GetUnion(),
                                    & rFrmRect = pCell->Frm();

                        const long nUnionRight = rUnion.Right();
                        const long nUnionBottom = rUnion.Bottom();
                        const long nFrmRight = rFrmRect.Right();
                        const long nFrmBottom = rFrmRect.Bottom();

                        // liegt das FrmRect ausserhalb der Union, kann es
                        // ignoriert werden.

                        const long nXFuzzy = bVert ? 0 : 20;
                        const long nYFuzzy = bVert ? 20 : 0;

                        if( !(  rUnion.Top()  + nYFuzzy > nFrmBottom ||
                                nUnionBottom < rFrmRect.Top() + nYFuzzy ||
                                rUnion.Left() + nXFuzzy > nFrmRight ||
                                nUnionRight < rFrmRect.Left() + nXFuzzy ))
                        {
                            // ok, rUnion is _not_ completely outside of rFrmRect

                            // wenn es aber nicht komplett in der Union liegt,
                            // dann ist es fuers Chart eine ungueltige
                            // Selektion.
                            if( rUnion.Left()   <= rFrmRect.Left() + nXFuzzy &&
                                rFrmRect.Left() <= nUnionRight &&
                                rUnion.Left()   <= nFrmRight &&
                                nFrmRight       <= nUnionRight + nXFuzzy &&
                                rUnion.Top()    <= rFrmRect.Top() + nYFuzzy &&
                                rFrmRect.Top()  <= nUnionBottom &&
                                rUnion.Top()    <= nFrmBottom &&
                                nFrmBottom      <= nUnionBottom+ nYFuzzy )

                                aCellFrms.push_back(
                                        _Sort_CellFrm( *(SwCellFrm*)pCell) );
                            else
                            {
                                bValidChartSel = sal_False;
                                break;
                            }
                        }
                        if ( pCell->GetNext() )
                        {
                            pCell = (const SwLayoutFrm*)pCell->GetNext();
                            if ( pCell->Lower() && pCell->Lower()->IsRowFrm() )
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
            size_t n, nCellCnt = 0;
            long nYPos = LONG_MAX;
            long nXPos = 0;
            long nHeight = 0;

            for( n = 0 ; n < aCellFrms.size(); ++n )
            {
                const _Sort_CellFrm& rCF = aCellFrms[ n ];
                if( (rCF.pFrm->Frm().*fnRect->fnGetTop)() != nYPos )
                {
                    // neue Zeile
                    if( n )
                    {
                        if( USHRT_MAX == nRowCells )        // 1. Zeilenwechsel
                            nRowCells = nCellCnt;
                        else if( nRowCells != nCellCnt )
                        {
                            bValidChartSel = sal_False;
                            break;
                        }
                    }
                    nCellCnt = 1;
                    nYPos = (rCF.pFrm->Frm().*fnRect->fnGetTop)();
                    nHeight = (rCF.pFrm->Frm().*fnRect->fnGetHeight)();

                    nXPos = bRTL ?
                            (rCF.pFrm->Frm().*fnRect->fnGetLeft)() :
                            (rCF.pFrm->Frm().*fnRect->fnGetRight)();
                }
                else if( nXPos == ( bRTL ?
                                    (rCF.pFrm->Frm().*fnRect->fnGetRight)() :
                                    (rCF.pFrm->Frm().*fnRect->fnGetLeft)() ) &&
                         nHeight == (rCF.pFrm->Frm().*fnRect->fnGetHeight)() )
                {
                    nXPos += ( bRTL ? (-1) : 1 ) *
                             (rCF.pFrm->Frm().*fnRect->fnGetWidth)();
                    ++nCellCnt;
                }
                else
                {
                    bValidChartSel = sal_False;
                    break;
                }
            }
            if( bValidChartSel )
            {
                if( USHRT_MAX == nRowCells )
                    nRowCells = nCellCnt;
                else if( nRowCells != nCellCnt )
                    bValidChartSel = sal_False;
            }

            if( bValidChartSel && pGetCLines )
            {
                nYPos = LONG_MAX;
                SwChartBoxes* pBoxes = 0;
                for( n = 0; n < aCellFrms.size(); ++n )
                {
                    const _Sort_CellFrm& rCF = aCellFrms[ n ];
                    if( (rCF.pFrm->Frm().*fnRect->fnGetTop)() != nYPos )
                    {
                        pBoxes = new SwChartBoxes( 255 < nRowCells
                                                    ? 255 : (sal_uInt8)nRowCells);
                        pGetCLines->C40_INSERT( SwChartBoxes, pBoxes, pGetCLines->Count() );
                        nYPos = (rCF.pFrm->Frm().*fnRect->fnGetTop)();
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
        SwTabFrm *pTable = aUnions[0]->GetTable();
        for( i = 0; i < aUnions.Count(); ++i )
        {
            if( pTable->IsValid() )
                pTable->InvalidatePos();
            pTable->SetONECalcLowers();
            pTable->Calc();
            pTable->SetCompletePaint();
            if( 0 == (pTable = pTable->GetFollow()) )
                break;
        }
        --nLoopMax;
        if( pGetCLines )
            pGetCLines->DeleteAndDestroy( 0, pGetCLines->Count() );
    } while( sal_True );

    OSL_ENSURE( nLoopMax, "das Layout der Tabelle wurde nicht valide!" );

    if( !bValidChartSel && pGetCLines )
        pGetCLines->DeleteAndDestroy( 0, pGetCLines->Count() );

    return bValidChartSel;
}


sal_Bool IsFrmInTblSel( const SwRect& rUnion, const SwFrm* pCell )
{
    OSL_ENSURE( pCell->IsCellFrm(), "Frame ohne Gazelle" );

    if( pCell->FindTabFrm()->IsVertical() )
        return ( rUnion.Right() >= pCell->Frm().Right() &&
                 rUnion.Left() <= pCell->Frm().Left() &&
            (( rUnion.Top() <= pCell->Frm().Top()+20 &&
               rUnion.Bottom() > pCell->Frm().Top() ) ||
             ( rUnion.Top() >= pCell->Frm().Top() &&
               rUnion.Bottom() < pCell->Frm().Bottom() )) ? sal_True : sal_False );

    return (
        rUnion.Top() <= pCell->Frm().Top() &&
        rUnion.Bottom() >= pCell->Frm().Bottom() &&

        (( rUnion.Left() <= pCell->Frm().Left()+20 &&
           rUnion.Right() > pCell->Frm().Left() ) ||

         ( rUnion.Left() >= pCell->Frm().Left() &&
           rUnion.Right() < pCell->Frm().Right() )) ? sal_True : sal_False );
}

sal_Bool GetAutoSumSel( const SwCrsrShell& rShell, SwCellFrms& rBoxes )
{
    SwShellCrsr* pCrsr = rShell.pCurCrsr;
    if ( rShell.IsTableMode() )
        pCrsr = rShell.pTblCrsr;

    const SwLayoutFrm *pStart = pCrsr->GetCntntNode()->GetFrm(
                      &pCrsr->GetPtPos() )->GetUpper(),
                      *pEnd   = pCrsr->GetCntntNode(sal_False)->GetFrm(
                      &pCrsr->GetMkPos() )->GetUpper();

    const SwLayoutFrm* pSttCell = pStart;
    while( pSttCell && !pSttCell->IsCellFrm() )
        pSttCell = pSttCell->GetUpper();

    //Zuerst lassen wir uns die Tabellen und die Rechtecke heraussuchen.
    SwSelUnions aUnions;

    // default erstmal nach oben testen, dann nach links
    ::MakeSelUnions( aUnions, pStart, pEnd, nsSwTblSearchType::TBLSEARCH_COL );

    sal_Bool bTstRow = sal_True, bFound = sal_False;
    sal_uInt16 i;

    // 1. teste ob die darueber liegende Box Value/Formel enhaelt:
    for( i = 0; i < aUnions.Count(); ++i )
    {
        SwSelUnion *pUnion = aUnions[i];
        const SwTabFrm *pTable = pUnion->GetTable();

        // Skip any repeated headlines in the follow:
        const SwLayoutFrm* pRow = pTable->IsFollow() ?
                                  pTable->GetFirstNonHeadlineRow() :
                                  (const SwLayoutFrm*)pTable->Lower();

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
                        sal_uInt16 nWhichId = 0;
                        for( sal_uInt16 n = rBoxes.Count(); n; )
                            if( USHRT_MAX != ( nWhichId = rBoxes[ --n ]
                                ->GetTabBox()->IsFormulaOrValueBox() ))
                                break;

                        // alle Boxen zusammen, nicht mehr die Zeile
                        // pruefen, wenn eine Formel oder Value gefunden wurde
                        bTstRow = 0 == nWhichId || USHRT_MAX == nWhichId;
                        bFound = sal_True;
                        break;
                    }

                    OSL_ENSURE( pCell->IsCellFrm(), "Frame ohne Celle" );
                    if( ::IsFrmInTblSel( pUnion->GetUnion(), pCell ) )
                        pUpperCell = (SwCellFrm*)pCell;

                    if( pCell->GetNext() )
                    {
                        pCell = (const SwLayoutFrm*)pCell->GetNext();
                        if ( pCell->Lower() && pCell->Lower()->IsRowFrm() )
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
        bFound = sal_False;

        rBoxes.Remove( 0, rBoxes.Count() );
        aUnions.DeleteAndDestroy( 0, aUnions.Count() );
        ::MakeSelUnions( aUnions, pStart, pEnd, nsSwTblSearchType::TBLSEARCH_ROW );

        for( i = 0; i < aUnions.Count(); ++i )
        {
            SwSelUnion *pUnion = aUnions[i];
            const SwTabFrm *pTable = pUnion->GetTable();

            // Skip any repeated headlines in the follow:
            const SwLayoutFrm* pRow = pTable->IsFollow() ?
                                      pTable->GetFirstNonHeadlineRow() :
                                      (const SwLayoutFrm*)pTable->Lower();

            while( pRow )
            {
                if( pRow->Frm().IsOver( pUnion->GetUnion() ) )
                {
                    const SwLayoutFrm *pCell = pRow->FirstCell();

                    while( pCell && pRow->IsAnLower( pCell ) )
                    {
                        if( pCell == pSttCell )
                        {
                            sal_uInt16 nWhichId = 0;
                            for( sal_uInt16 n = rBoxes.Count(); n; )
                                if( USHRT_MAX != ( nWhichId = rBoxes[ --n ]
                                    ->GetTabBox()->IsFormulaOrValueBox() ))
                                    break;

                            // alle Boxen zusammen, nicht mehr die Zeile
                            // pruefen, wenn eine Formel oder Value gefunden wurde
                            bFound = 0 != nWhichId && USHRT_MAX != nWhichId;
                            bTstRow = sal_False;
                            break;
                        }

                        OSL_ENSURE( pCell->IsCellFrm(), "Frame ohne Celle" );
                        if( ::IsFrmInTblSel( pUnion->GetUnion(), pCell ) )
                        {
                            const SwCellFrm* pC = (SwCellFrm*)pCell;
                            rBoxes.Insert( pC, rBoxes.Count() );
                        }
                        if( pCell->GetNext() )
                        {
                            pCell = (const SwLayoutFrm*)pCell->GetNext();
                            if ( pCell->Lower() && pCell->Lower()->IsRowFrm() )
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

sal_Bool HasProtectedCells( const SwSelBoxes& rBoxes )
{
    sal_Bool bRet = sal_False;
    for( sal_uInt16 n = 0, nCnt = rBoxes.Count(); n < nCnt; ++n )
        if( rBoxes[ n ]->GetFrmFmt()->GetProtect().IsCntntProtected() )
        {
            bRet = sal_True;
            break;
        }
    return bRet;
}


_CmpLPt::_CmpLPt( const Point& rPt, const SwTableBox* pBox, sal_Bool bVertical )
    : aPos( rPt ), pSelBox( pBox ), bVert( bVertical )
{}

void lcl_InsTblBox( SwTableNode* pTblNd, SwDoc* pDoc, SwTableBox* pBox,
                        sal_uInt16 nInsPos, sal_uInt16 nCnt = 1 )
{
    OSL_ENSURE( pBox->GetSttNd(), "Box ohne Start-Node" );
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

sal_Bool IsEmptyBox( const SwTableBox& rBox, SwPaM& rPam )
{
    rPam.GetPoint()->nNode = *rBox.GetSttNd()->EndOfSectionNode();
    rPam.Move( fnMoveBackward, fnGoCntnt );
    rPam.SetMark();
    rPam.GetPoint()->nNode = *rBox.GetSttNd();
    rPam.Move( fnMoveForward, fnGoCntnt );
    sal_Bool bRet = *rPam.GetMark() == *rPam.GetPoint()
        && ( rBox.GetSttNd()->GetIndex() + 1 == rPam.GetPoint()->nNode.GetIndex() );

    if( bRet )
    {
        // dann teste mal auf absatzgebundenen Flys
        const SwSpzFrmFmts& rFmts = *rPam.GetDoc()->GetSpzFrmFmts();
        sal_uLong nSttIdx = rPam.GetPoint()->nNode.GetIndex(),
              nEndIdx = rBox.GetSttNd()->EndOfSectionIndex(),
              nIdx;

        for( sal_uInt16 n = 0; n < rFmts.Count(); ++n )
        {
            const SwFmtAnchor& rAnchor = rFmts[n]->GetAnchor();
            const SwPosition* pAPos = rAnchor.GetCntntAnchor();
            if (pAPos &&
                ((FLY_AT_PARA == rAnchor.GetAnchorId()) ||
                 (FLY_AT_CHAR == rAnchor.GetAnchorId())) &&
                nSttIdx <= ( nIdx = pAPos->nNode.GetIndex() ) &&
                nIdx < nEndIdx )
            {
                bRet = sal_False;
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
        rBoxes.Remove( sal_uInt16(0), rBoxes.Count() );

    //Zuerst lassen wir uns die Tabellen und die Rechtecke heraussuchen.
    OSL_ENSURE( rPam.GetCntntNode() && rPam.GetCntntNode( sal_False ),
            "Tabselection nicht auf Cnt." );

//JP 24.09.96: Merge mit wiederholenden TabellenHeadline funktioniert nicht
//              richtig. Warum nicht Point 0,0 benutzen? Dann ist garantiert,
//              das die 1. Headline mit drin ist.
    Point aPt( 0, 0 );
    const SwLayoutFrm *pStart = rPam.GetCntntNode()->GetFrm(
                                                        &aPt )->GetUpper(),
                      *pEnd = rPam.GetCntntNode(sal_False)->GetFrm(
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

    SWRECTFN( pStart->GetUpper() )

    for ( sal_uInt16 i = 0; i < aUnions.Count(); ++i )
    {
        const SwTabFrm *pTabFrm = aUnions[i]->GetTable();

        SwRect &rUnion = aUnions[i]->GetUnion();

        // Skip any repeated headlines in the follow:
        const SwLayoutFrm* pRow = pTabFrm->IsFollow() ?
                                  pTabFrm->GetFirstNonHeadlineRow() :
                                  (const SwLayoutFrm*)pTabFrm->Lower();

        while ( pRow )
        {
            if ( pRow->Frm().IsOver( rUnion ) )
            {
                const SwLayoutFrm *pCell = pRow->FirstCell();

                while ( pCell && pRow->IsAnLower( pCell ) )
                {
                    OSL_ENSURE( pCell->IsCellFrm(), "Frame ohne Celle" );
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
                                sal_uInt16 nInsPos = pBox->GetUpper()->
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
                                pBox->GetFrmFmt()->SetFmtAttr( aNew );
                                // diese Box ist selektiert
                                pLastBox = pBox;
                                rBoxes.Insert( pBox );
                                aPosArr.Insert(
                                    _CmpLPt( (pCell->Frm().*fnRect->fnGetPos)(),
                                    pBox, bVert ) );

                                pBox = pBox->GetUpper()->GetTabBoxes()[ nInsPos ];
                                aNew.SetWidth( nTmpWidth );
                                pBox->ClaimFrmFmt();
                                pBox->GetFrmFmt()->SetFmtAttr( aNew );

                                if( pUndo )
                                    pUndo->AddNewBox( pBox->GetSttIdx() );
                            }
                            else
                            {
                                // diese Box ist selektiert
                                pLastBox = pBox;
                                rBoxes.Insert( pBox );
#if OSL_DEBUG_LEVEL > 1
                                Point aInsPoint( (pCell->Frm().*fnRect->fnGetPos)() );
#endif
                                aPosArr.Insert(
                                    _CmpLPt( (pCell->Frm().*fnRect->fnGetPos)(),
                                    pBox, bVert ) );
                            }
                        }
                        // oder rechts und links ueberlappend
                        else if( ( rUnion.Left() - COLFUZZY ) >= pCell->Frm().Left() &&
                                ( rUnion.Right() + COLFUZZY ) < pCell->Frm().Right() )
                        {
                            sal_uInt16 nInsPos = pBox->GetUpper()->GetTabBoxes().
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
                            pBox->GetFrmFmt()->SetFmtAttr( aNew );

                            {
                            const SfxPoolItem* pItem;
                            if( SFX_ITEM_SET == pBox->GetFrmFmt()->GetAttrSet()
                                        .GetItemState( RES_BOX, sal_False, &pItem ))
                            {
                                SvxBoxItem aBox( *(SvxBoxItem*)pItem );
                                aBox.SetLine( 0, BOX_LINE_RIGHT );
                                pBox->GetFrmFmt()->SetFmtAttr( aBox );
                            }
                            }

                            pBox = pBox->GetUpper()->GetTabBoxes()[ nInsPos ];
                            aNew.SetWidth( nWidth );
                            pBox->ClaimFrmFmt();
                            pBox->GetFrmFmt()->SetFmtAttr( aNew );

                            if( pUndo )
                                pUndo->AddNewBox( pBox->GetSttIdx() );

                            // diese Box ist selektiert
                            pLastBox = pBox;
                            rBoxes.Insert( pBox );
                            aPosArr.Insert(
                                _CmpLPt( (pCell->Frm().*fnRect->fnGetPos)(),
                                pBox, bVert ) );

                            pBox = pBox->GetUpper()->GetTabBoxes()[ nInsPos+1 ];
                            aNew.SetWidth( nRight );
                            pBox->ClaimFrmFmt();
                            pBox->GetFrmFmt()->SetFmtAttr( aNew );

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
                            sal_uInt16 nInsPos = pBox->GetUpper()->GetTabBoxes().
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
                            pBox->ClaimFrmFmt()->SetFmtAttr( aNew );

                                // diese Box ist selektiert
                            pBox = pBox->GetUpper()->GetTabBoxes()[ nInsPos ];
                            aNew.SetWidth( nRight );
                            pBox->ClaimFrmFmt();
                            pBox->GetFrmFmt()->SetFmtAttr( aNew );

                            pLastBox = pBox;
                            rBoxes.Insert( pBox );
                            aPosArr.Insert( _CmpLPt( Point( rUnion.Left(),
                                                pCell->Frm().Top()), pBox, bVert ));

                            if( pUndo )
                                pUndo->AddNewBox( pBox->GetSttIdx() );
                        }
                    }
                    if ( pCell->GetNext() )
                    {
                        pCell = (const SwLayoutFrm*)pCell->GetNext();
                        // --> Check if table cell is not empty
                        if ( pCell->Lower() && pCell->Lower()->IsRowFrm() )
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
    sal_Bool bCalcWidth = sal_True;
    const SwTableBox* pFirstBox = aPosArr[ 0 ].pSelBox;

    // JP 27.03.98:  Optimierung - falls die Boxen einer Line leer sind,
    //              dann werden jetzt dafuer keine Blanks und
    //              kein Umbruch mehr eingefuegt.
    //Block damit SwPaM, SwPosition vom Stack geloescht werden
    {
        SwPaM aPam( pDoc->GetNodes() );

#if defined( DEL_ONLY_EMPTY_LINES )
        nWidth = pFirstBox->GetFrmFmt()->GetFrmSize().GetWidth();
        sal_Bool bEmptyLine = sal_True;
        sal_uInt16 n, nSttPos = 0;

        for( n = 0; n < aPosArr.Count(); ++n )
        {
            const _CmpLPt& rPt = aPosArr[ n ];
            if( n && aPosArr[ n - 1 ].Y() == rPt.Y() )  // gleiche Ebene ?
            {
                if( bEmptyLine && !IsEmptyBox( *rPt.pSelBox, aPam ))
                    bEmptyLine = sal_False;
                if( bCalcWidth )
                    nWidth += rPt.pSelBox->GetFrmFmt()->GetFrmSize().GetWidth();
            }
            else
            {
                if( bCalcWidth && n )
                    bCalcWidth = sal_False;     // eine Zeile fertig

                if( bEmptyLine && nSttPos < n )
                {
                    // dann ist die gesamte Line leer und braucht
                    // nicht mit Blanks aufgefuellt und als Absatz
                    // eingefuegt werden.
                    if( pUndo )
                        for( sal_uInt16 i = nSttPos; i < n; ++i )
                            pUndo->SaveCollection( *aPosArr[ i ].pSelBox );

                    aPosArr.Remove( nSttPos, n - nSttPos );
                    n = nSttPos;
                }
                else
                    nSttPos = n;

                bEmptyLine = IsEmptyBox( *aPosArr[n].pSelBox, aPam );
            }
        }
        if( bEmptyLine && nSttPos < n )
        {
            if( pUndo )
                for( sal_uInt16 i = nSttPos; i < n; ++i )
                    pUndo->SaveCollection( *aPosArr[ i ].pSelBox );
            aPosArr.Remove( nSttPos, n - nSttPos );
        }
#elsif defined( DEL_EMPTY_BOXES_AT_START_AND_END )

        nWidth = pFirstBox->GetFrmFmt()->GetFrmSize().GetWidth();
        sal_uInt16 n, nSttPos = 0, nSEndPos = 0, nESttPos = 0;

        for( n = 0; n < aPosArr.Count(); ++n )
        {
            const _CmpLPt& rPt = aPosArr[ n ];
            if( n && aPosArr[ n - 1 ].Y() == rPt.Y() )  // gleiche Ebene ?
            {
                sal_Bool bEmptyBox = IsEmptyBox( *rPt.pSelBox, aPam );
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
                    bCalcWidth = sal_False;     // eine Zeile fertig

                // zuerst die vom Anfang
                if( nSttPos < nSEndPos )
                {
                    // dann ist der vorder Teil der Line leer und braucht
                    // nicht mit Blanks aufgefuellt werden.
                    if( pUndo )
                        for( sal_uInt16 i = nSttPos; i < nSEndPos; ++i )
                            pUndo->SaveCollection( *aPosArr[ i ].pSelBox );

                    sal_uInt16 nCnt = nSEndPos - nSttPos;
                    aPosArr.Remove( nSttPos, nCnt );
                    nESttPos -= nCnt;
                    n -= nCnt;
                }

                if( nESttPos < n )
                {
                    // dann ist der vorder Teil der Line leer und braucht
                    // nicht mit Blanks aufgefuellt werden.
                    if( pUndo )
                        for( sal_uInt16 i = nESttPos; i < n; ++i )
                            pUndo->SaveCollection( *aPosArr[ i ].pSelBox );

                    sal_uInt16 nCnt = n - nESttPos;
                    aPosArr.Remove( nESttPos, nCnt );
                    n -= nCnt;
                }

                nSttPos = nSEndPos = nESttPos = n;
                if( IsEmptyBox( *aPosArr[n].pSelBox, aPam ))
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
                for( sal_uInt16 i = nSttPos; i < nSEndPos; ++i )
                    pUndo->SaveCollection( *aPosArr[ i ].pSelBox );

            sal_uInt16 nCnt = nSEndPos - nSttPos;
            aPosArr.Remove( nSttPos, nCnt );
            nESttPos -= nCnt;
            n -= nCnt;
        }
        if( nESttPos < n )
        {
            // dann ist der vorder Teil der Line leer und braucht
            // nicht mit Blanks aufgefuellt werden.
            if( pUndo )
                for( sal_uInt16 i = nESttPos; i < n; ++i )
                    pUndo->SaveCollection( *aPosArr[ i ].pSelBox );

            sal_uInt16 nCnt = n - nESttPos;
            aPosArr.Remove( nESttPos, nCnt );
        }
#else
// DEL_ALL_EMPTY_BOXES

        nWidth = 0;
        long nY = aPosArr.Count() ?
                    ( bVert ?
                      aPosArr[ 0 ].X() :
                      aPosArr[ 0 ].Y() ) :
                  0;

        for( sal_uInt16 n = 0; n < aPosArr.Count(); ++n )
        {
            const _CmpLPt& rPt = aPosArr[ n ];
            if( bCalcWidth )
            {
                if( nY == ( bVert ? rPt.X() : rPt.Y() ) )            // gleiche Ebene ?
                    nWidth += rPt.pSelBox->GetFrmFmt()->GetFrmSize().GetWidth();
                else
                    bCalcWidth = sal_False;     // eine Zeile fertig
            }

            if( IsEmptyBox( *rPt.pSelBox, aPam ) )
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
        sal_uInt16 nInsPos = pInsLine->GetTabBoxes().C40_GETPOS( SwTableBox, pTmpBox );

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
                (*ppMergeBox)->GetFrmFmt()->SetFmtAttr( aBox );
        }
    }

    //Block damit SwPaM, SwPosition vom Stack geloescht werden
    if( aPosArr.Count() )
    {
        SwTxtNode* pTxtNd = 0;
        SwPosition aInsPos( *(*ppMergeBox)->GetSttNd() );
        SwNodeIndex& rInsPosNd = aInsPos.nNode;

        SwPaM aPam( aInsPos );

        for( sal_uInt16 n = 0; n < aPosArr.Count(); ++n )
        {
            const _CmpLPt& rPt = aPosArr[ n ];
            aPam.GetPoint()->nNode.Assign( *rPt.pSelBox->GetSttNd()->
                                            EndOfSectionNode(), -1 );
            SwCntntNode* pCNd = aPam.GetCntntNode();
            sal_uInt16 nL = pCNd ? pCNd->Len() : 0;
            aPam.GetPoint()->nContent.Assign( pCNd, nL );

            SwNodeIndex aSttNdIdx( *rPt.pSelBox->GetSttNd(), 1 );
            // ein Node muss in der Box erhalten bleiben (sonst wird beim
            // Move die gesamte Section geloescht)
            bool const bUndo(pDoc->GetIDocumentUndoRedo().DoesUndo());
            if( pUndo )
            {
                pDoc->GetIDocumentUndoRedo().DoUndo(false);
            }
            pDoc->AppendTxtNode( *aPam.GetPoint() );
            if( pUndo )
            {
                pDoc->GetIDocumentUndoRedo().DoUndo(bUndo);
            }
            SwNodeRange aRg( aSttNdIdx, aPam.GetPoint()->nNode );
            rInsPosNd++;
            if( pUndo )
                pUndo->MoveBoxCntnt( pDoc, aRg, rInsPosNd );
            else
            {
                pDoc->MoveNodeRange( aRg, rInsPosNd,
                    IDocumentContentOperations::DOC_MOVEDEFAULT );
            }
            // wo steht jetzt aInsPos ??

            if( bCalcWidth )
                bCalcWidth = sal_False;     // eine Zeile fertig

            // den initialen TextNode ueberspringen
            rInsPosNd.Assign( pDoc->GetNodes(),
                            rInsPosNd.GetNode().EndOfSectionIndex() - 2 );
            pTxtNd = rInsPosNd.GetNode().GetTxtNode();
            if( pTxtNd )
                aInsPos.nContent.Assign( pTxtNd, pTxtNd->GetTxt().Len() );
        }

        // in der MergeBox sollte jetzt der gesamte Text stehen
        // loesche jetzt noch den initialen TextNode
        OSL_ENSURE( (*ppMergeBox)->GetSttIdx()+2 <
                (*ppMergeBox)->GetSttNd()->EndOfSectionIndex(),
                    "leere Box" );
        SwNodeIndex aIdx( *(*ppMergeBox)->GetSttNd()->EndOfSectionNode(), -1 );
        pDoc->GetNodes().Delete( aIdx, 1 );
    }

    // setze die Breite der Box
    (*ppMergeBox)->GetFrmFmt()->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE, nWidth, 0 ));
    if( pUndo )
        pUndo->AddNewBox( (*ppMergeBox)->GetSttIdx() );
}


static sal_Bool lcl_CheckCol( const _FndBox*& rpFndBox, void* pPara );

static sal_Bool lcl_CheckRow( const _FndLine*& rpFndLine, void* pPara )
{
    ((_FndLine*)rpFndLine)->GetBoxes().ForEach( &lcl_CheckCol, pPara );
    return *(sal_Bool*)pPara;
}

static sal_Bool lcl_CheckCol( const _FndBox*& rpFndBox, void* pPara )
{
    if( !rpFndBox->GetBox()->GetSttNd() )
    {
        if( rpFndBox->GetLines().Count() !=
            rpFndBox->GetBox()->GetTabLines().Count() )
            *((sal_Bool*)pPara) = sal_False;
        else
            ((_FndBox*)rpFndBox)->GetLines().ForEach( &lcl_CheckRow, pPara );
    }
    // Box geschuetzt ??
    else if( rpFndBox->GetBox()->GetFrmFmt()->GetProtect().IsCntntProtected() )
        *((sal_Bool*)pPara) = sal_False;
    return *(sal_Bool*)pPara;
}


sal_uInt16 CheckMergeSel( const SwPaM& rPam )
{
    SwSelBoxes aBoxes;
//JP 24.09.96: Merge mit wiederholenden TabellenHeadline funktioniert nicht
//              richtig. Warum nicht Point 0,0 benutzen? Dann ist garantiert,
//              das die 1. Headline mit drin ist.
    Point aPt;
    const SwLayoutFrm *pStart = rPam.GetCntntNode()->GetFrm(
                                                    &aPt )->GetUpper(),
                        *pEnd = rPam.GetCntntNode(sal_False)->GetFrm(
                                                    &aPt )->GetUpper();
    GetTblSel( pStart, pEnd, aBoxes, 0 );
    return CheckMergeSel( aBoxes );
}

sal_uInt16 CheckMergeSel( const SwSelBoxes& rBoxes )
{
    sal_uInt16 eRet = TBLMERGE_NOSELECTION;
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
            sal_Bool bMergeSelOk = sal_True;
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
    const SwLayoutFrm *pTmp = pCell;
    if ( !nWish )
        nWish = 1;

    const sal_Bool bRTL = pCell->IsRightToLeft();
    SwTwips nRet = bRTL ?
        nAct - pCell->Frm().Width() :
        0;

    while ( pTmp )
    {
        while ( pTmp->GetPrev() )
        {
            pTmp = (SwLayoutFrm*)pTmp->GetPrev();
            long nTmp = pTmp->GetFmt()->GetFrmSize().GetWidth();
            nRet += ( bRTL ? ( -1 ) : 1 ) * nTmp * nAct / nWish;
        }
        pTmp = pTmp->GetUpper()->GetUpper();
        if ( pTmp && !pTmp->IsCellFrm() )
            pTmp = 0;
    }
    return nRet;
}

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

    for( sal_uInt16 n = 0; n < aEndArr.Count() && n < aSttArr.Count(); ++n )
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
    if( !rpStart )
        return;
    const SwTabFrm *pOrg = rpStart->FindTabFrm();
    const SwTabFrm *pTab = pOrg;

    SWRECTFN( pTab )

    sal_Bool bRTL = pTab->IsRightToLeft();
    const long nTmpWish = pOrg->GetFmt()->GetFrmSize().GetWidth();
    const long nWish = ( nTmpWish > 0 ) ? nTmpWish : 1;

    while ( pTab->IsFollow() )
    {
        const SwFrm *pTmp = pTab->FindPrev();
        OSL_ENSURE( pTmp->IsTabFrm(), "Vorgaenger vom Follow nicht der Master." );
        pTab = (const SwTabFrm*)pTmp;
    }

    SwTwips nSX  = 0;
    SwTwips nSX2 = 0;

    if ( pTab->GetTable()->IsNewModel() )
    {
        nSX  = (rpStart->Frm().*fnRect->fnGetLeft )();
        nSX2 = (rpStart->Frm().*fnRect->fnGetRight)();
    }
    else
    {
        const SwTwips nPrtWidth = (pTab->Prt().*fnRect->fnGetWidth)();
        nSX = ::lcl_CalcWish( rpStart, nWish, nPrtWidth ) + (pTab->*fnRect->fnGetPrtLeft)();
        nSX2 = nSX + (rpStart->GetFmt()->GetFrmSize().GetWidth() * nPrtWidth / nWish);
    }

    const SwLayoutFrm *pTmp = pTab->FirstCell();

    while ( pTmp &&
            (!pTmp->IsCellFrm() ||
             ( ( ! bRTL && (pTmp->Frm().*fnRect->fnGetLeft)() < nSX &&
                           (pTmp->Frm().*fnRect->fnGetRight)()< nSX2 ) ||
               (   bRTL && (pTmp->Frm().*fnRect->fnGetLeft)() > nSX &&
                           (pTmp->Frm().*fnRect->fnGetRight)()> nSX2 ) ) ) )
        pTmp = pTmp->GetNextLayoutLeaf();

    if ( pTmp )
        rpStart = pTmp;

    pTab = pOrg;

    const SwTabFrm* pLastValidTab = pTab;
    while ( pTab->GetFollow() )
    {
        //
        // Check if pTab->GetFollow() is a valid follow table:
        // Only follow tables with at least on non-FollowFlowLine
        // should be considered.
        //
        if ( pTab->HasFollowFlowLine() )
        {
            pTab = pTab->GetFollow();
            const SwFrm* pTmpRow = pTab->GetFirstNonHeadlineRow();
            if ( pTmpRow && pTmpRow->GetNext() )
                pLastValidTab = pTab;
        }
        else
            pLastValidTab = pTab = pTab->GetFollow();
    }
    pTab = pLastValidTab;

    SwTwips nEX = 0;

    if ( pTab->GetTable()->IsNewModel() )
    {
        nEX = (rpEnd->Frm().*fnRect->fnGetLeft )();
    }
    else
    {
        const SwTwips nPrtWidth = (pTab->Prt().*fnRect->fnGetWidth)();
        nEX = ::lcl_CalcWish( rpEnd, nWish, nPrtWidth ) + (pTab->*fnRect->fnGetPrtLeft)();
    }

    const SwCntntFrm* pLastCntnt = pTab->FindLastCntnt();
    rpEnd = pLastCntnt ? pLastCntnt->GetUpper() : 0;
    // --> Made code robust. If pTab does not have a lower,
    // we would crash here.
    if ( !pLastCntnt ) return;
    // <--

    while( !rpEnd->IsCellFrm() )
        rpEnd = rpEnd->GetUpper();

    while ( (   bRTL && (rpEnd->Frm().*fnRect->fnGetLeft)() < nEX ) ||
            ( ! bRTL && (rpEnd->Frm().*fnRect->fnGetLeft)() > nEX ) )
    {
        const SwLayoutFrm* pTmpLeaf = rpEnd->GetPrevLayoutLeaf();
        if( !pTmpLeaf || !pTab->IsAnLower( pTmpLeaf ) )
            break;
        rpEnd = pTmpLeaf;
    }

    if( !bChkProtected )    // geschuetzte Zellen beachten ?
        return;

    //Anfang und Ende duerfen nicht auf geschuetzten Zellen liegen.
    //Also muss ggf. nocheinmal rueckwaerts gesucht werden.
    while ( rpStart->GetFmt()->GetProtect().IsCntntProtected() )
    {
        const SwLayoutFrm *pTmpLeaf = rpStart;
        pTmpLeaf = pTmpLeaf->GetNextLayoutLeaf();
        while ( pTmpLeaf && (pTmpLeaf->Frm().*fnRect->fnGetLeft)() > nEX )//erstmal die Zeile ueberspr.
            pTmpLeaf = pTmpLeaf->GetNextLayoutLeaf();
        while ( pTmpLeaf && (pTmpLeaf->Frm().*fnRect->fnGetLeft)() < nSX &&
                            (pTmpLeaf->Frm().*fnRect->fnGetRight)()< nSX2 )
            pTmpLeaf = pTmpLeaf->GetNextLayoutLeaf();
        const SwTabFrm *pTmpTab = rpStart->FindTabFrm();
        if ( !pTmpTab->IsAnLower( pTmpLeaf ) )
        {
            pTmpTab = pTmpTab->GetFollow();
            rpStart = pTmpTab->FirstCell();
            while ( (rpStart->Frm().*fnRect->fnGetLeft)() < nSX &&
                    (rpStart->Frm().*fnRect->fnGetRight)()< nSX2 )
                rpStart = rpStart->GetNextLayoutLeaf();
        }
        else
            rpStart = pTmpLeaf;
    }
    while ( rpEnd->GetFmt()->GetProtect().IsCntntProtected() )
    {
        const SwLayoutFrm *pTmpLeaf = rpEnd;
        pTmpLeaf = pTmpLeaf->GetPrevLayoutLeaf();
        while ( pTmpLeaf && (pTmpLeaf->Frm().*fnRect->fnGetLeft)() < nEX )//erstmal die Zeile ueberspr.
            pTmpLeaf = pTmpLeaf->GetPrevLayoutLeaf();
        while ( pTmpLeaf && (pTmpLeaf->Frm().*fnRect->fnGetLeft)() > nEX )
            pTmpLeaf = pTmpLeaf->GetPrevLayoutLeaf();
        const SwTabFrm *pTmpTab = rpEnd->FindTabFrm();
        if ( !pTmpLeaf || !pTmpTab->IsAnLower( pTmpLeaf ) )
        {
            pTmpTab = (const SwTabFrm*)pTmpTab->FindPrev();
            OSL_ENSURE( pTmpTab->IsTabFrm(), "Vorgaenger vom Follow nicht der Master.");
            rpEnd = pTmpTab->FindLastCntnt()->GetUpper();
            while( !rpEnd->IsCellFrm() )
                rpEnd = rpEnd->GetUpper();
            while ( (rpEnd->Frm().*fnRect->fnGetLeft)() > nEX )
                rpEnd = rpEnd->GetPrevLayoutLeaf();
        }
        else
            rpEnd = pTmpLeaf;
    }
}


void MakeSelUnions( SwSelUnions& rUnions, const SwLayoutFrm *pStart,
                    const SwLayoutFrm *pEnd, const SwTblSearchType eSearchType )
{
    while ( pStart && !pStart->IsCellFrm() )
        pStart = pStart->GetUpper();
    while ( pEnd && !pEnd->IsCellFrm() )
        pEnd = pEnd->GetUpper();

    if ( !pStart || !pEnd )
    {
        OSL_ENSURE( false, "MakeSelUnions with pStart or pEnd not in CellFrm" );
        return;
    }

    const SwTabFrm *pTable = pStart->FindTabFrm();
    const SwTabFrm *pEndTable = pEnd->FindTabFrm();
    if( !pTable || !pEndTable )
        return;
    sal_Bool bExchange = sal_False;

    if ( pTable != pEndTable )
    {
        if ( !pTable->IsAnFollow( pEndTable ) )
        {
            OSL_ENSURE( pEndTable->IsAnFollow( pTable ), "Tabkette verknotet." );
            bExchange = sal_True;
        }
    }
    else
    {
        SWRECTFN( pTable )
        long nSttTop = (pStart->Frm().*fnRect->fnGetTop)();
        long nEndTop = (pEnd->Frm().*fnRect->fnGetTop)();
        if( nSttTop == nEndTop )
        {
            if( (pStart->Frm().*fnRect->fnGetLeft)() >
                (pEnd->Frm().*fnRect->fnGetLeft)() )
                bExchange = sal_True;
        }
        else if( bVert == ( nSttTop < nEndTop ) )
            bExchange = sal_True;
    }
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
    if( nsSwTblSearchType::TBLSEARCH_ROW == ((~nsSwTblSearchType::TBLSEARCH_PROTECT ) & eSearchType ) )
        ::lcl_FindStartEndRow( pStart, pEnd, nsSwTblSearchType::TBLSEARCH_PROTECT & eSearchType );
    else if( nsSwTblSearchType::TBLSEARCH_COL == ((~nsSwTblSearchType::TBLSEARCH_PROTECT ) & eSearchType ) )
        ::lcl_FindStartEndCol( pStart, pEnd, nsSwTblSearchType::TBLSEARCH_PROTECT & eSearchType );

    if ( !pEnd ) return; // Made code robust.

    //neu besorgen, da sie jetzt verschoben sind.
    pTable = pStart->FindTabFrm();
    pEndTable = pEnd->FindTabFrm();

    const long nStSz = pStart->GetFmt()->GetFrmSize().GetWidth();
    const long nEdSz = pEnd->GetFmt()->GetFrmSize().GetWidth();
    const long nWish = Max( 1L, pTable->GetFmt()->GetFrmSize().GetWidth() );
    while ( pTable )
    {
        SWRECTFN( pTable )
        const long nOfst = (pTable->*fnRect->fnGetPrtLeft)();
        const long nPrtWidth = (pTable->Prt().*fnRect->fnGetWidth)();
        long nSt1 = ::lcl_CalcWish( pStart, nWish, nPrtWidth ) + nOfst;
        long nEd1 = ::lcl_CalcWish( pEnd,   nWish, nPrtWidth ) + nOfst;

        if ( nSt1 <= nEd1 )
            nEd1 += (long)((nEdSz * nPrtWidth) / nWish) - 1;
        else
            nSt1 += (long)((nStSz * nPrtWidth) / nWish) - 1;

        long nSt2;
        long nEd2;
        if( pTable->IsAnLower( pStart ) )
            nSt2 = (pStart->Frm().*fnRect->fnGetTop)();
        else
            nSt2 = (pTable->Frm().*fnRect->fnGetTop)();
        if( pTable->IsAnLower( pEnd ) )
            nEd2 = (pEnd->Frm().*fnRect->fnGetBottom)();
        else
            nEd2 = (pTable->Frm().*fnRect->fnGetBottom)();
        Point aSt, aEd;
        if( nSt1 > nEd1 )
        {
            long nTmp = nSt1;
            nSt1 = nEd1;
            nEd1 = nTmp;
        }
        if( nSt2 > nEd2 )
        {
            long nTmp = nSt2;
            nSt2 = nEd2;
            nEd2 = nTmp;
        }
        if( bVert )
        {
            aSt = Point( nSt2, nSt1 );
            aEd = Point( nEd2, nEd1 );
        }
        else
        {
            aSt = Point( nSt1, nSt2 );
            aEd = Point( nEd1, nEd2 );
        }

        const Point aDiff( aEd - aSt );
        SwRect aUnion( aSt, Size( aDiff.X(), aDiff.Y() ) );
        aUnion.Justify();

        // fuers
        if( !(nsSwTblSearchType::TBLSEARCH_NO_UNION_CORRECT & eSearchType ))
        {
            //Leider ist die Union jetzt mit Rundungsfehlern behaftet und dadurch
            //wuerden beim Split/Merge fehlertraechtige Umstaende entstehen.
            //Um dies zu vermeiden werden jetzt fuer die Table die erste und
            //letzte Zelle innerhalb der Union ermittelt und aus genau deren
            //Werten wird die Union neu gebildet.
            const SwLayoutFrm* pRow = pTable->IsFollow() ?
                                      pTable->GetFirstNonHeadlineRow() :
                                      (const SwLayoutFrm*)pTable->Lower();

            while ( pRow && !pRow->Frm().IsOver( aUnion ) )
                pRow = (SwLayoutFrm*)pRow->GetNext();

            // --> FME 2004-07-26 #i31976#
            // A follow flow row may contain emtpy cells. These are not
            // considered by FirstCell(). Therefore we have to find
            // the first cell manually:
            const SwFrm* pTmpCell = 0;
            if ( pTable->IsFollow() && pRow && pRow->IsInFollowFlowRow() )
            {
                const SwFrm* pTmpRow = pRow;
                while ( pTmpRow && pTmpRow->IsRowFrm() )
                {
                    pTmpCell = static_cast<const SwRowFrm*>(pTmpRow)->Lower();
                    pTmpRow  = static_cast<const SwCellFrm*>(pTmpCell)->Lower();
                }
                OSL_ENSURE( !pTmpCell || pTmpCell->IsCellFrm(), "Lower of rowframe != cellframe?!" );
            }
            // <--

            const SwLayoutFrm* pFirst = pTmpCell ?
                                        static_cast<const SwLayoutFrm*>(pTmpCell) :
                                        pRow ?
                                        pRow->FirstCell() :
                                        0;

            while ( pFirst && !::IsFrmInTblSel( aUnion, pFirst ) )
            {
                if ( pFirst->GetNext() )
                {
                    pFirst = (const SwLayoutFrm*)pFirst->GetNext();
                    if ( pFirst->Lower() && pFirst->Lower()->IsRowFrm() )
                        pFirst = pFirst->FirstCell();
                }
                else
                    pFirst = ::lcl_FindNextCellFrm( pFirst );
            }
            const SwLayoutFrm* pLast = 0;
            const SwFrm* pLastCntnt = pTable->FindLastCntnt();
            if ( pLastCntnt )
                pLast = ::lcl_FindCellFrm( pLastCntnt->GetUpper() );

            while ( pLast && !::IsFrmInTblSel( aUnion, pLast ) )
                pLast = ::lcl_FindCellFrm( pLast->GetPrevLayoutLeaf() );

            if ( pFirst && pLast ) //Robust
            {
                aUnion = pFirst->Frm();
                aUnion.Union( pLast->Frm() );
            }
            else
                aUnion.Width( 0 );
        }

        if( (aUnion.*fnRect->fnGetWidth)() )
        {
            SwSelUnion *pTmp = new SwSelUnion( aUnion, (SwTabFrm*)pTable );
            rUnions.C40_INSERT( SwSelUnion, pTmp, rUnions.Count() );
        }

        pTable = pTable->GetFollow();
        if ( pTable != pEndTable && pEndTable->IsAnFollow( pTable ) )
            pTable = 0;
    }
}

sal_Bool CheckSplitCells( const SwCrsrShell& rShell, sal_uInt16 nDiv,
                        const SwTblSearchType eSearchType )
{
    if( !rShell.IsTableMode() )
        rShell.GetCrsr();

    return CheckSplitCells( *rShell.getShellCrsr(false), nDiv, eSearchType );
}

sal_Bool CheckSplitCells( const SwCursor& rCrsr, sal_uInt16 nDiv,
                        const SwTblSearchType eSearchType )
{
    if( 1 >= nDiv )
        return sal_False;

    sal_uInt16 nMinValue = nDiv * MINLAY;

    //Start- und Endzelle besorgen und den naechsten fragen.
    Point aPtPos, aMkPos;
    const SwShellCrsr* pShCrsr = dynamic_cast<const SwShellCrsr*>(&rCrsr);
    if( pShCrsr )
    {
        aPtPos = pShCrsr->GetPtPos();
        aMkPos = pShCrsr->GetMkPos();
    }
    const SwLayoutFrm *pStart = rCrsr.GetCntntNode()->GetFrm(
                                &aPtPos )->GetUpper(),
                      *pEnd   = rCrsr.GetCntntNode(sal_False)->GetFrm(
                                &aMkPos )->GetUpper();

    SWRECTFN( pStart->GetUpper() )

    //Zuerst lassen wir uns die Tabellen und die Rechtecke heraussuchen.
    SwSelUnions aUnions;

    ::MakeSelUnions( aUnions, pStart, pEnd, eSearchType );

    //Jetzt zu jedem Eintrag die Boxen herausfischen und uebertragen.
    for ( sal_uInt16 i = 0; i < aUnions.Count(); ++i )
    {
        SwSelUnion *pUnion = aUnions[i];
        const SwTabFrm *pTable = pUnion->GetTable();

        // Skip any repeated headlines in the follow:
        const SwLayoutFrm* pRow = pTable->IsFollow() ?
                                  pTable->GetFirstNonHeadlineRow() :
                                  (const SwLayoutFrm*)pTable->Lower();

        while ( pRow )
        {
            if ( pRow->Frm().IsOver( pUnion->GetUnion() ) )
            {
                const SwLayoutFrm *pCell = pRow->FirstCell();

                while ( pCell && pRow->IsAnLower( pCell ) )
                {
                    OSL_ENSURE( pCell->IsCellFrm(), "Frame ohne Celle" );
                    if( ::IsFrmInTblSel( pUnion->GetUnion(), pCell ) )
                    {
                        if( (pCell->Frm().*fnRect->fnGetWidth)() < nMinValue )
                            return sal_False;
                    }

                    if ( pCell->GetNext() )
                    {
                        pCell = (const SwLayoutFrm*)pCell->GetNext();
                        if ( pCell->Lower() && pCell->Lower()->IsRowFrm() )
                            pCell = pCell->FirstCell();
                    }
                    else
                        pCell = ::lcl_FindNextCellFrm( pCell );
                }
            }
            pRow = (const SwLayoutFrm*)pRow->GetNext();
        }
    }
    return sal_True;
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
        SwTabFrm* pTabFrm = (SwTabFrm*)pUpper;
        pTabFrm->FindMaster()->InvalidatePos(); //kann die Zeile vielleicht aufnehmen

        if ( pSibling && pTabFrm->IsInHeadline( *pSibling ) )
        {
            // Skip any repeated headlines in the follow:
            pSibling = pTabFrm->GetFirstNonHeadlineRow();
        }
    }
    pRow->Paste( pUpper, pSibling );
    pRow->RegistFlys();
}


sal_Bool _FndBoxCopyCol( const SwTableBox*& rpBox, void* pPara )
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
            return sal_True;
        }
    }
    else
    {
        SwTableBoxPtr pSrch = (SwTableBoxPtr)rpBox;
        sal_uInt16 nFndPos;
        if( !pFndPara->rBoxes.Seek_Entry( pSrch, &nFndPos ))
        {
            delete pFndBox;
            return sal_True;
        }
    }
    pFndPara->pFndLine->GetBoxes().C40_INSERT( _FndBox, pFndBox,
                    pFndPara->pFndLine->GetBoxes().Count() );
    return sal_True;
}

sal_Bool _FndLineCopyCol( const SwTableLine*& rpLine, void* pPara )
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
    return sal_True;
}

void _FndBox::SetTableLines( const SwSelBoxes &rBoxes, const SwTable &rTable )
{
    //Pointer auf die Lines vor und hinter den zu verarbeitenden Bereich
    //setzen. Wenn die erste/letzte Zeile in den Bereich eingeschlossen
    //sind, so bleiben die Pointer eben einfach 0.
    //Gesucht werden zunachst die Positionen der ersten/letzten betroffenen
    //Line im Array der SwTable. Damit die 0 fuer 'keine Line' verwand werden
    //kann werden die Positionen um 1 nach oben versetzt!

    sal_uInt16 nStPos = USHRT_MAX;
    sal_uInt16 nEndPos= 0;

    for ( sal_uInt16 i = 0; i < rBoxes.Count(); ++i )
    {
        SwTableLine *pLine = rBoxes[i]->GetUpper();
        while ( pLine->GetUpper() )
            pLine = pLine->GetUpper()->GetUpper();
        const sal_uInt16 nPos = rTable.GetTabLines().GetPos(
                    (const SwTableLine*&)pLine ) + 1;

        OSL_ENSURE( nPos != USHRT_MAX, "TableLine not found." );

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
    sal_uInt16 nPos = rTable.GetTabLines().C40_GETPOS( SwTableLine, pTmpLine );
    OSL_ENSURE( USHRT_MAX != nPos, "Line steht nicht in der Tabelle" );
    if( nPos )
        pLineBefore = rTable.GetTabLines()[ nPos - 1 ];

    pTmpLine = GetLines()[GetLines().Count()-1]->GetLine();
    nPos = rTable.GetTabLines().C40_GETPOS( SwTableLine, pTmpLine );
    OSL_ENSURE( USHRT_MAX != nPos, "Line steht nicht in der Tabelle" );
    if( ++nPos < rTable.GetTabLines().Count() )
        pLineBehind = rTable.GetTabLines()[nPos];
}

inline void UnsetFollow( SwFlowFrm *pTab )
{
    pTab->bIsFollow = sal_False;
}

void _FndBox::DelFrms( SwTable &rTable )
{
    //Alle Lines zwischen pLineBefore und pLineBehind muessen aus dem
    //Layout ausgeschnitten und geloescht werden.
    //Entstehen dabei leere Follows so muessen diese vernichtet werden.
    //Wird ein Master vernichtet, so muss der Follow Master werden.
    //Ein TabFrm muss immer uebrigbleiben.

    sal_uInt16 nStPos = 0;
    sal_uInt16 nEndPos= rTable.GetTabLines().Count() - 1;
    if( rTable.IsNewModel() && pLineBefore )
        rTable.CheckRowSpan( pLineBefore, true );
    if ( pLineBefore )
    {
        nStPos = rTable.GetTabLines().GetPos(
                        (const SwTableLine*&)pLineBefore );
        OSL_ENSURE( nStPos != USHRT_MAX, "Fuchs Du hast die Line gestohlen!" );
        ++nStPos;
    }
    if( rTable.IsNewModel() && pLineBehind )
        rTable.CheckRowSpan( pLineBehind, false );
    if ( pLineBehind )
    {
        nEndPos = rTable.GetTabLines().GetPos(
                        (const SwTableLine*&)pLineBehind );
        OSL_ENSURE( nEndPos != USHRT_MAX, "Fuchs Du hast die Line gestohlen!" );
        --nEndPos;
    }

    for ( sal_uInt16 i = nStPos; i <= nEndPos; ++i)
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
                    sal_Bool bDel = sal_True;
                    SwTabFrm *pUp = !pFrm->GetPrev() && !pFrm->GetNext() ?
                                            (SwTabFrm*)pFrm->GetUpper() : 0;
                    if ( !pUp )
                    {
                        const sal_uInt16 nRepeat =
                                ((SwTabFrm*)pFrm->GetUpper())->GetTable()->GetRowsToRepeat();
                        if ( nRepeat > 0 &&
                             ((SwTabFrm*)pFrm->GetUpper())->IsFollow() )
                        {
                            if ( !pFrm->GetNext() )
                            {
                                SwRowFrm* pFirstNonHeadline =
                                    ((SwTabFrm*)pFrm->GetUpper())->GetFirstNonHeadlineRow();
                                if ( pFirstNonHeadline == pFrm )
                                {
                                    pUp = (SwTabFrm*)pFrm->GetUpper();
                                }
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
                            OSL_ENSURE( pTmp->IsTabFrm(),
                                    "Vorgaenger vom Follow kein Master.");
                            pPrev = (SwTabFrm*)pTmp;
                        }
                        if ( pPrev )
                        {
                            pPrev->SetFollow( pFollow );
                            // --> FME 2006-01-31 #i60340# Do not transfer the
                            // flag from pUp to pPrev. pUp may still have the
                            // flag set although there is not more follow flow
                            // line associated with pUp.
                            pPrev->SetFollowFlowLine( sal_False );
                            // <--
                        }
                        else if ( pFollow )
                            ::UnsetFollow( pFollow );

                        //Ein TabellenFrm muss immer stehenbleiben!
                        if ( pPrev || pFollow )
                        {
                            // OD 26.08.2003 #i18103# - if table is in a section,
                            // lock the section, to avoid its delete.
                            {
                                SwSectionFrm* pSctFrm = pUp->FindSctFrm();
                                bool bOldSectLock = false;
                                if ( pSctFrm )
                                {
                                    bOldSectLock = pSctFrm->IsColLocked();
                                    pSctFrm->ColLock();
                                }
                                pUp->Cut();
                                if ( pSctFrm && !bOldSectLock )
                                {
                                    pSctFrm->ColUnlock();
                                }
                            }
                            delete pUp;
                            bDel = sal_False;//Die Row wird mit in den Abgrund
                                         //gerissen.
                        }
                    }
                    if ( bDel )
                    {
                        SwFrm* pTabFrm = pFrm->GetUpper();
                        if ( pTabFrm->IsTabFrm() &&
                            !pFrm->GetNext() &&
                             ((SwTabFrm*)pTabFrm)->GetFollow() )
                        {
                            // We do not delete the follow flow line,
                            // this will be done automatically in the
                            // next turn.
                            ((SwTabFrm*)pTabFrm)->SetFollowFlowLine( sal_False );
                        }

                        pFrm->Cut();
                        delete pFrm;
                    }
                }
            } while( 0 != ( pLast = aIter++ ));
        }
    }
}

sal_Bool lcl_IsLineOfTblFrm( const SwTabFrm& rTable, const SwFrm& rChk )
{
    const SwTabFrm* pTblFrm = rChk.FindTabFrm();
    if( pTblFrm->IsFollow() )
        pTblFrm = pTblFrm->FindMaster( true );
    return &rTable == pTblFrm;
}

/*
 * lcl_UpdateRepeatedHeadlines
 */
void lcl_UpdateRepeatedHeadlines( SwTabFrm& rTabFrm, bool bCalcLowers )
{
    OSL_ENSURE( rTabFrm.IsFollow(), "lcl_UpdateRepeatedHeadlines called for non-follow tab" );

    // Delete remaining headlines:
    SwRowFrm* pLower = 0;
    while ( 0 != ( pLower = (SwRowFrm*)rTabFrm.Lower() ) && pLower->IsRepeatedHeadline() )
    {
        pLower->Cut();
        delete pLower;
    }

    // Insert fresh set of headlines:
    pLower = (SwRowFrm*)rTabFrm.Lower();
    SwTable& rTable = *rTabFrm.GetTable();
    const sal_uInt16 nRepeat = rTable.GetRowsToRepeat();
    for ( sal_uInt16 nIdx = 0; nIdx < nRepeat; ++nIdx )
    {
        SwRowFrm* pHeadline = new SwRowFrm(
                                *rTable.GetTabLines()[ nIdx ] );
        pHeadline->SetRepeatedHeadline( true );
        pHeadline->Paste( &rTabFrm, pLower );
        pHeadline->RegistFlys();
    }

    if ( bCalcLowers )
        rTabFrm.SetCalcLowers();
}

void _FndBox::MakeFrms( SwTable &rTable )
{
    //Alle Lines zwischen pLineBefore und pLineBehind muessen im Layout
    //wieder neu erzeugt werden.
    //Und Zwar fuer alle Auspraegungen der Tabelle (mehrere z.B. im Kopf/Fuss).

    sal_uInt16 nStPos = 0;
    sal_uInt16 nEndPos= rTable.GetTabLines().Count() - 1;
    if ( pLineBefore )
    {
        nStPos = rTable.GetTabLines().GetPos(
                        (const SwTableLine*&)pLineBefore );
        OSL_ENSURE( nStPos != USHRT_MAX, "Fuchs Du hast die Line gestohlen!" );
        ++nStPos;

    }
    if ( pLineBehind )
    {
        nEndPos = rTable.GetTabLines().GetPos(
                        (const SwTableLine*&)pLineBehind );
        OSL_ENSURE( nEndPos != USHRT_MAX, "Fuchs Du hast die Line gestohlen!" );
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
            SwFrm  *pUpperFrm  = 0;
            int i;
            for ( i = rTable.GetTabLines().Count()-1;
                    i >= 0 && !pSibling; --i )
            {
                SwTableLine *pLine = pLineBehind ? pLineBehind :
                                                    rTable.GetTabLines()[static_cast<sal_uInt16>(i)];
                SwClientIter aIter( *pLine->GetFrmFmt() );
                pSibling = (SwFrm*)aIter.First( TYPE(SwFrm) );
                while ( pSibling && (
                            static_cast<SwRowFrm*>(pSibling)->GetTabLine() != pLine ||
                            !lcl_IsLineOfTblFrm( *pTable, *pSibling ) ||
                            static_cast<SwRowFrm*>(pSibling)->IsRepeatedHeadline() ||
                            // --> FME 2005-08-24 #i53647# If !pLineBehind,
                            // IsInSplitTableRow() should be checked.
                            ( pLineBehind && pSibling->IsInFollowFlowRow() ) ||
                            (!pLineBehind && pSibling->IsInSplitTableRow() ) ) )
                            // <--
                {
                    pSibling = (SwFrm*)aIter.Next();
                }
            }
            if ( pSibling )
            {
                pUpperFrm = pSibling->GetUpper();
                if ( !pLineBehind )
                    pSibling = 0;
            }
            else
// ???? oder das der Letzte Follow der Tabelle ????
                pUpperFrm = pTable;

            for ( i = nStPos; (sal_uInt16)i <= nEndPos; ++i )
                ::lcl_InsertRow( *rTable.GetTabLines()[static_cast<sal_uInt16>(i)],
                                (SwLayoutFrm*)pUpperFrm, pSibling );
            if ( pUpperFrm->IsTabFrm() )
                ((SwTabFrm*)pUpperFrm)->SetCalcLowers();
        }
        else if ( rTable.GetRowsToRepeat() > 0 )
        {
            // Insert new headlines:
            lcl_UpdateRepeatedHeadlines( *pTable, true );
        }
    }
}

void _FndBox::MakeNewFrms( SwTable &rTable, const sal_uInt16 nNumber,
                                            const sal_Bool bBehind )
{
    //Frms fuer neu eingefuege Zeilen erzeugen.
    //bBehind == sal_True:  vor     pLineBehind
    //        == sal_False: hinter  pLineBefore
    const sal_uInt16 nBfPos = pLineBefore ?
        rTable.GetTabLines().GetPos( (const SwTableLine*&)pLineBefore ) :
        USHRT_MAX;
    const sal_uInt16 nBhPos = pLineBehind ?
        rTable.GetTabLines().GetPos( (const SwTableLine*&)pLineBehind ) :
        USHRT_MAX;

    //nNumber: wie oft ist eingefuegt worden.
    //nCnt:    wieviele sind nNumber mal eingefuegt worden.

    const sal_uInt16 nCnt =
        ((nBhPos != USHRT_MAX ? nBhPos : rTable.GetTabLines().Count()) -
         (nBfPos != USHRT_MAX ? nBfPos + 1 : 0)) / (nNumber + 1);

    //Den Master-TabFrm suchen
    SwClientIter aTabIter( *rTable.GetFrmFmt() );
    SwTabFrm *pTable;
    for ( pTable = (SwTabFrm*)aTabIter.First( TYPE(SwFrm) ); pTable;
          pTable = (SwTabFrm*)aTabIter.Next() )
    {
        if( !pTable->IsFollow() )
        {
            SwFrm       *pSibling = 0;
            SwLayoutFrm *pUpperFrm   = 0;
            if ( bBehind )
            {
                if ( pLineBehind )
                {
                    SwClientIter aIter( *pLineBehind->GetFrmFmt() );
                    pSibling = (SwFrm*)aIter.First( TYPE(SwFrm) );
                    while ( pSibling && (
                                // only consider row frames associated with pLineBehind:
                                static_cast<SwRowFrm*>(pSibling)->GetTabLine() != pLineBehind ||
                                // only consider row frames that are in pTables Master-Follow chain:
                                !lcl_IsLineOfTblFrm( *pTable, *pSibling ) ||
                                // only consider row frames that are not repeated headlines:
                                static_cast<SwRowFrm*>(pSibling)->IsRepeatedHeadline() ||
                                // only consider row frames that are not follow flow rows
                                pSibling->IsInFollowFlowRow() ) )
                    {
                          pSibling = (SwFrm*)aIter.Next();
                    }
                }
                if ( pSibling )
                    pUpperFrm = pSibling->GetUpper();
                else
                {
                    while( pTable->GetFollow() )
                        pTable = pTable->GetFollow();
                    pUpperFrm = pTable;
                }
                const sal_uInt16 nMax = nBhPos != USHRT_MAX ?
                                    nBhPos : rTable.GetTabLines().Count();

                sal_uInt16 i = nBfPos != USHRT_MAX ? nBfPos + 1 + nCnt : nCnt;

                for ( ; i < nMax; ++i )
                    ::lcl_InsertRow( *rTable.GetTabLines()[i], pUpperFrm, pSibling );
                if ( pUpperFrm->IsTabFrm() )
                    ((SwTabFrm*)pUpperFrm)->SetCalcLowers();
            }
            else //davor einfuegen
            {
                sal_uInt16 i;

                // We are looking for the frame that is behind the row frame
                // that should be inserted.
                for ( i = 0; !pSibling; ++i )
                {
                    SwTableLine* pLine = pLineBefore ? pLineBefore : rTable.GetTabLines()[i];

                    SwClientIter aIter( *pLine->GetFrmFmt() );
                    pSibling = (SwFrm*)aIter.First( TYPE(SwFrm) );

                    while ( pSibling && (
                            // only consider row frames associated with pLineBefore:
                            static_cast<SwRowFrm*>(pSibling)->GetTabLine() != pLine ||
                            // only consider row frames that are in pTables Master-Follow chain:
                            !lcl_IsLineOfTblFrm( *pTable, *pSibling ) ||
                            // only consider row frames that are not repeated headlines:
                            static_cast<SwRowFrm*>(pSibling)->IsRepeatedHeadline() ||
                            // 1. case: pLineBefore == 0:
                            // only consider row frames that are not follow flow rows
                            // 2. case: pLineBefore != 0:
                            // only consider row frames that are not split table rows
                            // --> FME 2004-11-23 #i37476# If !pLineBefore,
                            // check IsInFollowFlowRow instead of IsInSplitTableRow.
                            ( ( !pLineBefore && pSibling->IsInFollowFlowRow() ) ||
                              (  pLineBefore && pSibling->IsInSplitTableRow() ) ) ) )
                            // <--
                    {
                        pSibling = (SwFrm*)aIter.Next();
                    }
                }

                pUpperFrm = pSibling->GetUpper();
                if ( pLineBefore )
                    pSibling = pSibling->GetNext();

                sal_uInt16 nMax = nBhPos != USHRT_MAX ?
                                    nBhPos - nCnt :
                                    rTable.GetTabLines().Count() - nCnt;

                i = nBfPos != USHRT_MAX ? nBfPos + 1 : 0;
                for ( ; i < nMax; ++i )
                    ::lcl_InsertRow( *rTable.GetTabLines()[i],
                                pUpperFrm, pSibling );
                if ( pUpperFrm->IsTabFrm() )
                    ((SwTabFrm*)pUpperFrm)->SetCalcLowers();
            }
        }
    }

    //Die Headlines mussen ggf. auch verarbeitet werden. Um gut arbeitenden
    //Code nicht zu zerfasern wird hier nochmals iteriert.
    const sal_uInt16 nRowsToRepeat = rTable.GetRowsToRepeat();
    if ( nRowsToRepeat > 0 &&
         ( ( !bBehind && ( nBfPos == USHRT_MAX || nBfPos + 1 < nRowsToRepeat ) ) ||
           (  bBehind && ( ( nBfPos == USHRT_MAX && nRowsToRepeat > 1 ) || nBfPos + 2 < nRowsToRepeat ) ) ) )
    {
        for ( pTable = (SwTabFrm*)aTabIter.First( TYPE(SwFrm) ); pTable;
              pTable = (SwTabFrm*)aTabIter.Next() )
        {
            if ( pTable->Lower() )
            {
                if ( pTable->IsFollow() )
                {
                    lcl_UpdateRepeatedHeadlines( *pTable, true );
                }

                OSL_ENSURE( ((SwRowFrm*)pTable->Lower())->GetTabLine() ==
                        rTable.GetTabLines()[0], "MakeNewFrms: Table corruption!" );
            }
        }
    }
}

sal_Bool _FndBox::AreLinesToRestore( const SwTable &rTable ) const
{
    //Lohnt es sich MakeFrms zu rufen?

    if ( !pLineBefore && !pLineBehind && rTable.GetTabLines().Count() )
        return sal_True;

    sal_uInt16 nBfPos;
    if(pLineBefore)
    {
        const SwTableLine* rLBefore = (const SwTableLine*)pLineBefore;
        nBfPos = rTable.GetTabLines().GetPos( rLBefore );
    }
    else
        nBfPos = USHRT_MAX;

    sal_uInt16 nBhPos;
    if(pLineBehind)
    {
        const SwTableLine* rLBehind = (const SwTableLine*)pLineBehind;
        nBhPos = rTable.GetTabLines().GetPos( rLBehind );
    }
    else
        nBhPos = USHRT_MAX;

    if ( nBfPos == nBhPos ) //Duerfte eigentlich nie vorkommen.
    {
        OSL_ENSURE( sal_False, "Table, Loeschen auf keinem Bereich !?!" );
        return sal_False;
    }

    if ( rTable.GetRowsToRepeat() > 0 )
    {
        // ups. sollte unsere zu wiederholende Kopfzeile geloescht worden
        // sein??
        SwClientIter aIter( *rTable.GetFrmFmt() );
        for( SwTabFrm* pTable = (SwTabFrm*)aIter.First( TYPE( SwFrm ));
             pTable; pTable = (SwTabFrm*)aIter.Next() )
        {
            if( pTable->IsFollow() )
            {
                // Insert new headlines:
                lcl_UpdateRepeatedHeadlines( *pTable, false );
            }
        }
    }

    // Some adjacent lines at the beginning of the table have been deleted:
    if ( nBfPos == USHRT_MAX && nBhPos == 0 )
        return sal_False;

    // Some adjacent lines at the end of the table have been deleted:
    if ( nBhPos == USHRT_MAX && nBfPos == (rTable.GetTabLines().Count() - 1) )
        return sal_False;

    // Some adjacent lines in the middle of the table have been deleted:
    if ( nBfPos != USHRT_MAX && nBhPos != USHRT_MAX && (nBfPos + 1) == nBhPos )
        return sal_False;

    // The structure of the deleted lines is more complex due to split lines.
    // A call of MakeFrms() is necessary.
    return sal_True;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
