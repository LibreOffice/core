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


#ifdef WTC
#define private public
#endif

#include "hintids.hxx"
#include <editeng/lrspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <fmtornt.hxx>
#include <fmtfsize.hxx>
#include <fmtlsplt.hxx>
#include <fmtrowsplt.hxx>
#include <tabcol.hxx>
#include <frmatr.hxx>
#include <cellfrm.hxx>
#include <tabfrm.hxx>
#include <cntfrm.hxx>
#include <txtfrm.hxx>
#include <svx/svxids.hrc>
#include "doc.hxx"
#include "pam.hxx"
#include "swcrsr.hxx"
#include "viscrs.hxx"
#include "swtable.hxx"
#include "htmltbl.hxx"
#include "tblsel.hxx"
#include "swtblfmt.hxx"
#include "docary.hxx"
#include "ndindex.hxx"
#include "undobj.hxx"

using namespace ::com::sun::star;


extern void ClearFEShellTabCols();

//siehe auch swtable.cxx
#define COLFUZZY 20L

inline BOOL IsSame( long nA, long nB ) { return  Abs(nA-nB) <= COLFUZZY; }

class SwTblFmtCmp
{
public:
    SwFrmFmt *pOld,
             *pNew;
    INT16     nType;

    SwTblFmtCmp( SwFrmFmt *pOld, SwFrmFmt *pNew, INT16 nType );

    static SwFrmFmt *FindNewFmt( SvPtrarr &rArr, SwFrmFmt*pOld, INT16 nType );
    static void Delete( SvPtrarr &rArr );
};


SwTblFmtCmp::SwTblFmtCmp( SwFrmFmt *pO, SwFrmFmt *pN, INT16 nT )
    : pOld ( pO ), pNew ( pN ), nType( nT )
{
}

SwFrmFmt *SwTblFmtCmp::FindNewFmt( SvPtrarr &rArr, SwFrmFmt *pOld, INT16 nType )
{
    for ( USHORT i = 0; i < rArr.Count(); ++i )
    {
        SwTblFmtCmp *pCmp = (SwTblFmtCmp*)rArr[i];
        if ( pCmp->pOld == pOld && pCmp->nType == nType )
            return pCmp->pNew;
    }
    return 0;
}

void SwTblFmtCmp::Delete( SvPtrarr &rArr )
{
    for ( USHORT i = 0; i < rArr.Count(); ++i )
        delete (SwTblFmtCmp*)rArr[i];
}

void lcl_GetStartEndCell( const SwCursor& rCrsr,
                        SwLayoutFrm *&prStart, SwLayoutFrm *&prEnd )
{
    OSL_ENSURE( rCrsr.GetCntntNode() && rCrsr.GetCntntNode( FALSE ),
            "Tabselection nicht auf Cnt." );

    Point aPtPos, aMkPos;
    const SwShellCrsr* pShCrsr = dynamic_cast<const SwShellCrsr*>(&rCrsr);
    if( pShCrsr )
    {
        aPtPos = pShCrsr->GetPtPos();
        aMkPos = pShCrsr->GetMkPos();
    }

    // robust:
    SwCntntNode* pPointNd = rCrsr.GetCntntNode();
    SwCntntNode* pMarkNd  = rCrsr.GetCntntNode(FALSE);

    SwFrm* pPointFrm = pPointNd ? pPointNd->GetFrm( &aPtPos ) : 0;
    SwFrm* pMarkFrm  = pMarkNd  ? pMarkNd->GetFrm( &aMkPos )  : 0;

    prStart = pPointFrm ? pPointFrm->GetUpper() : 0;
    prEnd   = pMarkFrm  ? pMarkFrm->GetUpper() : 0;
}

BOOL lcl_GetBoxSel( const SwCursor& rCursor, SwSelBoxes& rBoxes,
                    BOOL bAllCrsr = FALSE )
{
    const SwTableCursor* pTblCrsr =
        dynamic_cast<const SwTableCursor*>(&rCursor);
    if( pTblCrsr )
        ::GetTblSelCrs( *pTblCrsr, rBoxes );
    else
    {
        const SwPaM *pCurPam = &rCursor, *pSttPam = pCurPam;
        do {
            const SwNode* pNd = pCurPam->GetNode()->FindTableBoxStartNode();
            if( pNd )
            {
                SwTableBox* pBox = (SwTableBox*)pNd->FindTableNode()->GetTable().
                                            GetTblBox( pNd->GetIndex() );
                rBoxes.Insert( pBox );
            }
        } while( bAllCrsr &&
                pSttPam != ( pCurPam = (SwPaM*)pCurPam->GetNext()) );
    }
    return 0 != rBoxes.Count();
}

/***********************************************************************
#*  Class      :  SwDoc
#*  Methoden   :  SetRowHeight(), GetRowHeight()
#***********************************************************************/
//Die Zeilenhoehe wird ausgehend von der Selektion ermittelt/gesetzt.
//Ausgehend von jeder Zelle innerhalb der Selektion werden nach oben alle
//Zeilen abgeklappert, die oberste Zeile erhaelt den gewuenschten Wert alle
//tieferliegenden Zeilen einen entsprechenden Wert der sich aus der
//Relation der alten und neuen Groesse der obersten Zeile und ihrer
//eigenen Groesse ergiebt.
//Alle veraenderten Zeilen erhalten ggf. ein eigenes FrmFmt.
//Natuerlich darf jede Zeile nur einmal angefasst werden.

inline void InsertLine( SvPtrarr& rLineArr, SwTableLine* pLine )
{
    if( USHRT_MAX == rLineArr.GetPos( pLine ) )
        rLineArr.Insert( pLine, rLineArr.Count() );
}

//-----------------------------------------------------------------------------

BOOL lcl_IsAnLower( const SwTableLine *pLine, const SwTableLine *pAssumed )
{
    const SwTableLine *pTmp = pAssumed->GetUpper() ?
                                    pAssumed->GetUpper()->GetUpper() : 0;
    while ( pTmp )
    {
        if ( pTmp == pLine )
            return TRUE;
        pTmp = pTmp->GetUpper() ? pTmp->GetUpper()->GetUpper() : 0;
    }
    return FALSE;
}
//-----------------------------------------------------------------------------

struct LinesAndTable
{
          SvPtrarr &rLines;
    const SwTable  &rTable;
          BOOL      bInsertLines;

    LinesAndTable( SvPtrarr &rL, const SwTable &rTbl ) :
          rLines( rL ), rTable( rTbl ), bInsertLines( TRUE ) {}
};


BOOL _FindLine( const _FndLine*& rpLine, void* pPara );

BOOL _FindBox( const _FndBox*& rpBox, void* pPara )
{
    if ( rpBox->GetLines().Count() )
    {
        ((LinesAndTable*)pPara)->bInsertLines = TRUE;
        ((_FndBox*)rpBox)->GetLines().ForEach( _FindLine, pPara );
        if ( ((LinesAndTable*)pPara)->bInsertLines )
        {
            const SwTableLines &rLines = rpBox->GetBox()
                                    ? rpBox->GetBox()->GetTabLines()
                                    : ((LinesAndTable*)pPara)->rTable.GetTabLines();
            if ( rpBox->GetLines().Count() == rLines.Count() )
            {
                for ( USHORT i = 0; i < rLines.Count(); ++i )
                    ::InsertLine( ((LinesAndTable*)pPara)->rLines,
                                  (SwTableLine*)rLines[i] );
            }
            else
                ((LinesAndTable*)pPara)->bInsertLines = FALSE;
        }
    }
    else if ( rpBox->GetBox() )
        ::InsertLine( ((LinesAndTable*)pPara)->rLines,
                      (SwTableLine*)rpBox->GetBox()->GetUpper() );
    return TRUE;
}

BOOL _FindLine( const _FndLine*& rpLine, void* pPara )
{
    ((_FndLine*)rpLine)->GetBoxes().ForEach( _FindBox, pPara );
    return TRUE;
}

void lcl_CollectLines( SvPtrarr &rArr, const SwCursor& rCursor, bool bRemoveLines )
{
    //Zuerst die selektierten Boxen einsammeln.
    SwSelBoxes aBoxes;
    if( !::lcl_GetBoxSel( rCursor, aBoxes ))
        return ;

    //Die selektierte Struktur kopieren.
    const SwTable &rTable = aBoxes[0]->GetSttNd()->FindTableNode()->GetTable();
    LinesAndTable aPara( rArr, rTable );
    _FndBox aFndBox( 0, 0 );
    {
        _FndPara aTmpPara( aBoxes, &aFndBox );
        ((SwTableLines&)rTable.GetTabLines()).ForEach( &_FndLineCopyCol, &aTmpPara );
    }

    //Diejenigen Lines einsammeln, die nur selektierte Boxen enthalten.
    const _FndBox *pTmp = &aFndBox;
    ::_FindBox( pTmp, &aPara );

    // Remove lines, that have a common superordinate row.
    // (Not for row split)
    if ( bRemoveLines )
    {
        for ( USHORT i = 0; i < rArr.Count(); ++i )
        {
            SwTableLine *pUpLine = (SwTableLine*)rArr[i];
            for ( USHORT k = 0; k < rArr.Count(); ++k )
            {
                if ( k != i && ::lcl_IsAnLower( pUpLine, (SwTableLine*)rArr[k] ) )
                {
                    rArr.Remove( k );
                    if ( k <= i )
                        --i;
                    --k;
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------

void lcl_ProcessRowAttr( SvPtrarr& rFmtCmp, SwTableLine* pLine, const SfxPoolItem& rNew )
{
    SwFrmFmt *pNewFmt;
    if ( 0 != (pNewFmt = SwTblFmtCmp::FindNewFmt( rFmtCmp, pLine->GetFrmFmt(), 0 )))
        pLine->ChgFrmFmt( (SwTableLineFmt*)pNewFmt );
    else
    {
        SwFrmFmt *pOld = pLine->GetFrmFmt();
        SwFrmFmt *pNew = pLine->ClaimFrmFmt();
        pNew->SetFmtAttr( rNew );
        rFmtCmp.Insert( new SwTblFmtCmp( pOld, pNew, 0 ), rFmtCmp.Count());
    }
}

//-----------------------------------------------------------------------------

void lcl_ProcessBoxSize( SvPtrarr &rFmtCmp, SwTableBox *pBox, const SwFmtFrmSize &rNew );

void lcl_ProcessRowSize( SvPtrarr &rFmtCmp, SwTableLine *pLine, const SwFmtFrmSize &rNew )
{
    lcl_ProcessRowAttr( rFmtCmp, pLine, rNew );
    SwTableBoxes &rBoxes = pLine->GetTabBoxes();
    for ( USHORT i = 0; i < rBoxes.Count(); ++i )
        ::lcl_ProcessBoxSize( rFmtCmp, rBoxes[i], rNew );
}

//-----------------------------------------------------------------------------

void lcl_ProcessBoxSize( SvPtrarr &rFmtCmp, SwTableBox *pBox, const SwFmtFrmSize &rNew )
{
    SwTableLines &rLines = pBox->GetTabLines();
    if ( rLines.Count() )
    {
        SwFmtFrmSize aSz( rNew );
        aSz.SetHeight( rNew.GetHeight() ? rNew.GetHeight() / rLines.Count() : 0 );
        for ( USHORT i = 0; i < rLines.Count(); ++i )
            ::lcl_ProcessRowSize( rFmtCmp, rLines[i], aSz );
    }
}

//-----------------------------------------------------------------------------

/******************************************************************************
 *              void SwDoc::SetRowSplit()
 ******************************************************************************/
void SwDoc::SetRowSplit( const SwCursor& rCursor, const SwFmtRowSplit &rNew )
{
    SwTableNode* pTblNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    if( pTblNd )
    {
        SvPtrarr aRowArr( 25, 50 ); //Zum sammeln Lines.
        ::lcl_CollectLines( aRowArr, rCursor, false );

        if( aRowArr.Count() )
        {
            if( DoesUndo() )
            {
                ClearRedo();
                AppendUndo( new SwUndoAttrTbl( *pTblNd ));
            }

            SvPtrarr aFmtCmp( Max( BYTE(255), BYTE(aRowArr.Count()) ), 255 );

            for( USHORT i = 0; i < aRowArr.Count(); ++i )
                ::lcl_ProcessRowAttr( aFmtCmp, (SwTableLine*)aRowArr[i], rNew );

            SwTblFmtCmp::Delete( aFmtCmp );
            SetModified();
        }
    }
}


/******************************************************************************
 *               SwTwips SwDoc::GetRowSplit() const
 ******************************************************************************/
void SwDoc::GetRowSplit( const SwCursor& rCursor, SwFmtRowSplit *& rpSz ) const
{
    rpSz = 0;

    SwTableNode* pTblNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    if( pTblNd )
    {
        SvPtrarr aRowArr( 25, 50 ); //Zum sammeln der Lines.
        ::lcl_CollectLines( aRowArr, rCursor, false );

        if( aRowArr.Count() )
        {
            rpSz = &(SwFmtRowSplit&)((SwTableLine*)aRowArr[0])->
                                                GetFrmFmt()->GetRowSplit();

            for ( USHORT i = 1; i < aRowArr.Count() && rpSz; ++i )
            {
                if ( (*rpSz).GetValue() != ((SwTableLine*)aRowArr[i])->GetFrmFmt()->GetRowSplit().GetValue() )
                    rpSz = 0;
            }
            if ( rpSz )
                rpSz = new SwFmtRowSplit( *rpSz );
        }
    }
}


/******************************************************************************
 *              void SwDoc::SetRowHeight( SwTwips nNew )
 ******************************************************************************/
void SwDoc::SetRowHeight( const SwCursor& rCursor, const SwFmtFrmSize &rNew )
{
    SwTableNode* pTblNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    if( pTblNd )
    {
        SvPtrarr aRowArr( 25, 50 ); //Zum sammeln Lines.
        ::lcl_CollectLines( aRowArr, rCursor, true );

        if( aRowArr.Count() )
        {
            if( DoesUndo() )
            {
                ClearRedo();
                AppendUndo( new SwUndoAttrTbl( *pTblNd ));
            }

            SvPtrarr aFmtCmp( Max( BYTE(255), BYTE(aRowArr.Count()) ), 255 );
            for ( USHORT i = 0; i < aRowArr.Count(); ++i )
                ::lcl_ProcessRowSize( aFmtCmp, (SwTableLine*)aRowArr[i], rNew );
            SwTblFmtCmp::Delete( aFmtCmp );

            SetModified();
        }
    }
}


/******************************************************************************
 *               SwTwips SwDoc::GetRowHeight() const
 ******************************************************************************/
void SwDoc::GetRowHeight( const SwCursor& rCursor, SwFmtFrmSize *& rpSz ) const
{
    rpSz = 0;

    SwTableNode* pTblNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    if( pTblNd )
    {
        SvPtrarr aRowArr( 25, 50 ); //Zum sammeln der Lines.
        ::lcl_CollectLines( aRowArr, rCursor, true );

        if( aRowArr.Count() )
        {
            rpSz = &(SwFmtFrmSize&)((SwTableLine*)aRowArr[0])->
                                                GetFrmFmt()->GetFrmSize();

            for ( USHORT i = 1; i < aRowArr.Count() && rpSz; ++i )
            {
                if ( *rpSz != ((SwTableLine*)aRowArr[i])->GetFrmFmt()->GetFrmSize() )
                    rpSz = 0;
            }
            if ( rpSz )
                rpSz = new SwFmtFrmSize( *rpSz );
        }
    }
}

BOOL SwDoc::BalanceRowHeight( const SwCursor& rCursor, BOOL bTstOnly )
{
    BOOL bRet = FALSE;
    SwTableNode* pTblNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    if( pTblNd )
    {
        SvPtrarr aRowArr( 25, 50 ); //Zum sammeln der Lines.
        ::lcl_CollectLines( aRowArr, rCursor, true );

        if( 1 < aRowArr.Count() )
        {
            if( !bTstOnly )
            {
                long nHeight = 0;
                USHORT i;

                for ( i = 0; i < aRowArr.Count(); ++i )
                {
                    SwClientIter aIter( *((SwTableLine*)aRowArr[i])->GetFrmFmt() );
                    SwFrm* pFrm = (SwFrm*)aIter.First( TYPE(SwFrm) );
                    while ( pFrm )
                    {
                        nHeight = Max( nHeight, pFrm->Frm().Height() );
                        pFrm = (SwFrm*)aIter.Next();
                    }
                }
                SwFmtFrmSize aNew( ATT_MIN_SIZE, 0, nHeight );

                if( DoesUndo() )
                {
                    ClearRedo();
                    AppendUndo( new SwUndoAttrTbl( *pTblNd ));
                }

                SvPtrarr aFmtCmp( Max( BYTE(255), BYTE(aRowArr.Count()) ), 255 );
                for( i = 0; i < aRowArr.Count(); ++i )
                    ::lcl_ProcessRowSize( aFmtCmp, (SwTableLine*)aRowArr[i], aNew );
                SwTblFmtCmp::Delete( aFmtCmp );

                SetModified();
            }
            bRet = TRUE;
        }
    }
    return bRet;
}

/******************************************************************************
 *              void SwDoc::SetRowBackground()
 ******************************************************************************/
void SwDoc::SetRowBackground( const SwCursor& rCursor, const SvxBrushItem &rNew )
{
    SwTableNode* pTblNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    if( pTblNd )
    {
        SvPtrarr aRowArr( 25, 50 ); //Zum sammeln Lines.
        ::lcl_CollectLines( aRowArr, rCursor, true );

        if( aRowArr.Count() )
        {
            if( DoesUndo() )
            {
                ClearRedo();
                AppendUndo( new SwUndoAttrTbl( *pTblNd ));
            }

            SvPtrarr aFmtCmp( Max( BYTE(255), BYTE(aRowArr.Count()) ), 255 );

            for( USHORT i = 0; i < aRowArr.Count(); ++i )
                ::lcl_ProcessRowAttr( aFmtCmp, (SwTableLine*)aRowArr[i], rNew );

            SwTblFmtCmp::Delete( aFmtCmp );
            SetModified();
        }
    }
}

/******************************************************************************
 *               SwTwips SwDoc::GetRowBackground() const
 ******************************************************************************/
BOOL SwDoc::GetRowBackground( const SwCursor& rCursor, SvxBrushItem &rToFill ) const
{
    BOOL bRet = FALSE;
    SwTableNode* pTblNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    if( pTblNd )
    {
        SvPtrarr aRowArr( 25, 50 ); //Zum sammeln Lines.
        ::lcl_CollectLines( aRowArr, rCursor, true );

        if( aRowArr.Count() )
        {
            rToFill = ((SwTableLine*)aRowArr[0])->GetFrmFmt()->GetBackground();

            bRet = TRUE;
            for ( USHORT i = 1; i < aRowArr.Count(); ++i )
                if ( rToFill != ((SwTableLine*)aRowArr[i])->GetFrmFmt()->GetBackground() )
                {
                    bRet = FALSE;
                    break;
                }
        }
    }
    return bRet;
}

/***********************************************************************
#*  Class      :  SwDoc
#*  Methoden   :  SetTabBorders(), GetTabBorders()
#***********************************************************************/
inline void InsertCell( SvPtrarr& rCellArr, SwCellFrm* pCellFrm )
{
    if( USHRT_MAX == rCellArr.GetPos( pCellFrm ) )
        rCellArr.Insert( pCellFrm, rCellArr.Count() );
}

//-----------------------------------------------------------------------------
void lcl_CollectCells( SvPtrarr &rArr, const SwRect &rUnion,
                          SwTabFrm *pTab )
{
    SwLayoutFrm *pCell = pTab->FirstCell();
    do
    {
        // Wenn in der Zelle ein spaltiger Bereich sitzt, muessen wir
        // uns erst wieder zur Zelle hochhangeln
        while ( !pCell->IsCellFrm() )
            pCell = pCell->GetUpper();
        OSL_ENSURE( pCell, "Frame ist keine Zelle." );
        if ( rUnion.IsOver( pCell->Frm() ) )
            ::InsertCell( rArr, (SwCellFrm*)pCell );
        //Dafuer sorgen, dass die Zelle auch verlassen wird (Bereiche)
        SwLayoutFrm *pTmp = pCell;
        do
        {   pTmp = pTmp->GetNextLayoutLeaf();
        } while ( pCell->IsAnLower( pTmp ) );
        pCell = pTmp;
    } while( pCell && pTab->IsAnLower( pCell ) );
}

void SwDoc::SetTabBorders( const SwCursor& rCursor, const SfxItemSet& rSet )
{
    SwCntntNode* pCntNd = rCursor.GetPoint()->nNode.GetNode().GetCntntNode();
    SwTableNode* pTblNd = pCntNd ? pCntNd->FindTableNode() : 0;
    if( !pTblNd )
        return ;

    SwLayoutFrm *pStart, *pEnd;
    ::lcl_GetStartEndCell( rCursor, pStart, pEnd );

    SwSelUnions aUnions;
    ::MakeSelUnions( aUnions, pStart, pEnd );

    if( aUnions.Count() )
    {
        SwTable& rTable = pTblNd->GetTable();
        if( DoesUndo() )
        {
            ClearRedo();
            AppendUndo( new SwUndoAttrTbl( *pTblNd ));
        }

        SvPtrarr aFmtCmp( 255, 255 );
        const SvxBoxItem* pSetBox;
        const SvxBoxInfoItem *pSetBoxInfo;

        const SvxBorderLine* pLeft = 0;
        const SvxBorderLine* pRight = 0;
        const SvxBorderLine* pTop = 0;
        const SvxBorderLine* pBottom = 0;
        const SvxBorderLine* pHori = 0;
        const SvxBorderLine* pVert = 0;
        BOOL bHoriValid = TRUE, bVertValid = TRUE,
             bTopValid = TRUE, bBottomValid = TRUE,
             bLeftValid = TRUE, bRightValid = TRUE;

        // JP 21.07.95: die Flags im BoxInfo-Item entscheiden, wann eine
        //              BorderLine gueltig ist!!
        if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_BORDER_INNER, FALSE,
            (const SfxPoolItem**)&pSetBoxInfo) )
        {
            pHori = pSetBoxInfo->GetHori();
            pVert = pSetBoxInfo->GetVert();

            bHoriValid = pSetBoxInfo->IsValid(VALID_HORI);
            bVertValid = pSetBoxInfo->IsValid(VALID_VERT);

            // wollen wir die auswerten ??
            bTopValid = pSetBoxInfo->IsValid(VALID_TOP);
            bBottomValid = pSetBoxInfo->IsValid(VALID_BOTTOM);
            bLeftValid = pSetBoxInfo->IsValid(VALID_LEFT);
            bRightValid = pSetBoxInfo->IsValid(VALID_RIGHT);
        }

        if( SFX_ITEM_SET == rSet.GetItemState( RES_BOX, FALSE,
            (const SfxPoolItem**)&pSetBox) )
        {
            pLeft = pSetBox->GetLeft();
            pRight = pSetBox->GetRight();
            pTop = pSetBox->GetTop();
            pBottom = pSetBox->GetBottom();
        }
        else
        {
            // nicht gesetzt, also keine gueltigen Werte
            bTopValid = bBottomValid = bLeftValid = bRightValid = FALSE;
            pSetBox = 0;
        }

        BOOL bFirst = TRUE;
        for ( USHORT i = 0; i < aUnions.Count(); ++i )
        {
            SwSelUnion *pUnion = aUnions[i];
            SwTabFrm *pTab = pUnion->GetTable();
            const SwRect &rUnion = pUnion->GetUnion();
            const BOOL bLast  = i == aUnions.Count() - 1 ? TRUE : FALSE;

            SvPtrarr aCellArr( 255, 255 );
            ::lcl_CollectCells( aCellArr, pUnion->GetUnion(), pTab );

            //Alle Zellenkanten, die mit dem UnionRect uebereinstimmen oder
            //darueber hinausragen sind Aussenkanten. Alle anderen sind
            //Innenkanten.
            //neu: Die Aussenkanten koennen abhaengig davon, ob es sich um eine
            //Start/Mittlere/Folge -Tabelle (bei Selektionen ueber FollowTabs)
            //handelt doch keine Aussenkanten sein.
            //Aussenkanten werden links, rechts, oben und unten gesetzt.
            //Innenkanten werden nur oben und links gesetzt.
            for ( USHORT j = 0; j < aCellArr.Count(); ++j )
            {
                SwCellFrm *pCell = (SwCellFrm*)aCellArr[j];
                const sal_Bool bVert = pTab->IsVertical();
                const sal_Bool bRTL = pTab->IsRightToLeft();
                sal_Bool bTopOver, bLeftOver, bRightOver, bBottomOver;
                if ( bVert )
                {
                    bTopOver = pCell->Frm().Right() >= rUnion.Right();
                    bLeftOver = pCell->Frm().Top() <= rUnion.Top();
                    bRightOver = pCell->Frm().Bottom() >= rUnion.Bottom();
                    bBottomOver = pCell->Frm().Left() <= rUnion.Left();
                }
                else
                {
                    bTopOver = pCell->Frm().Top() <= rUnion.Top();
                    bLeftOver = pCell->Frm().Left() <= rUnion.Left();
                    bRightOver = pCell->Frm().Right() >= rUnion.Right();
                    bBottomOver = pCell->Frm().Bottom() >= rUnion.Bottom();
                }

                if ( bRTL )
                {
                    sal_Bool bTmp = bRightOver;
                    bRightOver = bLeftOver;
                    bLeftOver = bTmp;
                }

                //Grundsaetzlich nichts setzen in HeadlineRepeats.
                if ( pTab->IsFollow() &&
                     ( pTab->IsInHeadline( *pCell ) ||
                       // Same holds for follow flow rows.
                       pCell->IsInFollowFlowRow() ) )
                    continue;

                SvxBoxItem aBox( pCell->GetFmt()->GetBox() );

                INT16 nType = 0;

                //Obere Kante
                if( bTopValid )
                {
                    if ( bFirst && bTopOver )
                    {
                        aBox.SetLine( pTop, BOX_LINE_TOP );
                        nType |= 0x0001;
                    }
                    else if ( bHoriValid )
                    {
                        aBox.SetLine( 0, BOX_LINE_TOP );
                        nType |= 0x0002;
                    }
                }

                //Linke Kante
                if ( bLeftOver )
                {
                    if( bLeftValid )
                    {
                        aBox.SetLine( pLeft, BOX_LINE_LEFT );
                        nType |= 0x0004;
                    }
                }
                else if( bVertValid )
                {
                    aBox.SetLine( pVert, BOX_LINE_LEFT );
                    nType |= 0x0008;
                }

                //Rechte Kante
                if( bRightValid )
                {
                    if ( bRightOver )
                    {
                        aBox.SetLine( pRight, BOX_LINE_RIGHT );
                        nType |= 0x0010;
                    }
                    else if ( bVertValid )
                    {
                        aBox.SetLine( 0, BOX_LINE_RIGHT );
                        nType |= 0x0020;
                    }
                }

                //Untere Kante
                if ( bLast && bBottomOver )
                {
                    if( bBottomValid )
                    {
                        aBox.SetLine( pBottom, BOX_LINE_BOTTOM );
                        nType |= 0x0040;
                    }
                }
                else if( bHoriValid )
                {
                    aBox.SetLine( pHori, BOX_LINE_BOTTOM );
                    nType |= 0x0080;
                }

                if( pSetBox )
                {
                    static USHORT const aBorders[] = {
                        BOX_LINE_BOTTOM, BOX_LINE_TOP,
                        BOX_LINE_RIGHT, BOX_LINE_LEFT };
                    const USHORT* pBrd = aBorders;
                    for( int k = 0; k < 4; ++k, ++pBrd )
                        aBox.SetDistance( pSetBox->GetDistance( *pBrd ), *pBrd );
                }

                SwTableBox *pBox = (SwTableBox*)pCell->GetTabBox();
                SwFrmFmt *pNewFmt;
                if ( 0 != (pNewFmt = SwTblFmtCmp::FindNewFmt( aFmtCmp, pBox->GetFrmFmt(), nType )))
                    pBox->ChgFrmFmt( (SwTableBoxFmt*)pNewFmt );
                else
                {
                    SwFrmFmt *pOld = pBox->GetFrmFmt();
                    SwFrmFmt *pNew = pBox->ClaimFrmFmt();
                    pNew->SetFmtAttr( aBox );
                    aFmtCmp.Insert( new SwTblFmtCmp( pOld, pNew, nType ), aFmtCmp.Count());
                }
            }

            bFirst = FALSE;
        }

        SwHTMLTableLayout *pTableLayout = rTable.GetHTMLTableLayout();
        if( pTableLayout )
        {
            SwCntntFrm* pFrm = rCursor.GetCntntNode()->GetFrm();
            SwTabFrm* pTabFrm = pFrm->ImplFindTabFrm();

            pTableLayout->BordersChanged(
                pTableLayout->GetBrowseWidthByTabFrm( *pTabFrm ), TRUE );
        }
        SwTblFmtCmp::Delete( aFmtCmp );
        ::ClearFEShellTabCols();
        SetModified();
    }
}

void lcl_SetLineStyle( SvxBorderLine *pToSet,
                          const Color *pColor, const SvxBorderLine *pBorderLine)
{
    if ( pBorderLine )
    {
        if ( !pColor )
        {
            Color aTmp( pToSet->GetColor() );
            *pToSet = *pBorderLine;
            pToSet->SetColor( aTmp );
        }
        else
            *pToSet = *pBorderLine;
    }
    if ( pColor )
        pToSet->SetColor( *pColor );
}

void SwDoc::SetTabLineStyle( const SwCursor& rCursor,
                             const Color* pColor, BOOL bSetLine,
                             const SvxBorderLine* pBorderLine )
{
    SwCntntNode* pCntNd = rCursor.GetPoint()->nNode.GetNode().GetCntntNode();
    SwTableNode* pTblNd = pCntNd ? pCntNd->FindTableNode() : 0;
    if( !pTblNd )
        return ;

    SwLayoutFrm *pStart, *pEnd;
    ::lcl_GetStartEndCell( rCursor, pStart, pEnd );

    SwSelUnions aUnions;
    ::MakeSelUnions( aUnions, pStart, pEnd );

    if( aUnions.Count() )
    {
        SwTable& rTable = pTblNd->GetTable();
        if( DoesUndo() )
        {
            ClearRedo();
            AppendUndo( new SwUndoAttrTbl( *pTblNd ));
        }

        for( USHORT i = 0; i < aUnions.Count(); ++i )
        {
            SwSelUnion *pUnion = aUnions[i];
            SwTabFrm *pTab = pUnion->GetTable();
            SvPtrarr aCellArr( 255, 255 );
            ::lcl_CollectCells( aCellArr, pUnion->GetUnion(), pTab );

            for ( USHORT j = 0; j < aCellArr.Count(); ++j )
            {
                SwCellFrm *pCell = ( SwCellFrm* )aCellArr[j];

                //Grundsaetzlich nichts setzen in HeadlineRepeats.
                if ( pTab->IsFollow() && pTab->IsInHeadline( *pCell ) )
                    continue;

                ((SwTableBox*)pCell->GetTabBox())->ClaimFrmFmt();
                SwFrmFmt *pFmt = pCell->GetFmt();
                SvxBoxItem aBox( pFmt->GetBox() );

                if ( !pBorderLine && bSetLine )
                    aBox = *(SvxBoxItem*)::GetDfltAttr( RES_BOX );
                else
                {
                    if ( aBox.GetTop() )
                        ::lcl_SetLineStyle( (SvxBorderLine*)aBox.GetTop(),
                                        pColor, pBorderLine );
                    if ( aBox.GetBottom() )
                        ::lcl_SetLineStyle( (SvxBorderLine*)aBox.GetBottom(),
                                        pColor, pBorderLine );
                    if ( aBox.GetLeft() )
                        ::lcl_SetLineStyle( (SvxBorderLine*)aBox.GetLeft(),
                                        pColor, pBorderLine );
                    if ( aBox.GetRight() )
                        ::lcl_SetLineStyle( (SvxBorderLine*)aBox.GetRight(),
                                        pColor, pBorderLine );
                }
                pFmt->SetFmtAttr( aBox );
            }
        }

        SwHTMLTableLayout *pTableLayout = rTable.GetHTMLTableLayout();
        if( pTableLayout )
        {
            SwCntntFrm* pFrm = rCursor.GetCntntNode()->GetFrm();
            SwTabFrm* pTabFrm = pFrm->ImplFindTabFrm();

            pTableLayout->BordersChanged(
                pTableLayout->GetBrowseWidthByTabFrm( *pTabFrm ), TRUE );
        }
        ::ClearFEShellTabCols();
        SetModified();
    }
}

void SwDoc::GetTabBorders( const SwCursor& rCursor, SfxItemSet& rSet ) const
{
    SwCntntNode* pCntNd = rCursor.GetPoint()->nNode.GetNode().GetCntntNode();
    SwTableNode* pTblNd = pCntNd ? pCntNd->FindTableNode() : 0;
    if( !pTblNd )
        return ;

    SwLayoutFrm *pStart, *pEnd;
    ::lcl_GetStartEndCell( rCursor, pStart, pEnd );

    SwSelUnions aUnions;
    ::MakeSelUnions( aUnions, pStart, pEnd );

    if( aUnions.Count() )
    {
        SvxBoxItem     aSetBox    ((const SvxBoxItem    &) rSet.Get(RES_BOX    ));
        SvxBoxInfoItem aSetBoxInfo((const SvxBoxInfoItem&) rSet.Get(SID_ATTR_BORDER_INNER));

        BOOL bTopSet      = FALSE,
             bBottomSet   = FALSE,
             bLeftSet     = FALSE,
             bRightSet    = FALSE,
             bHoriSet     = FALSE,
             bVertSet     = FALSE,
             bDistanceSet = FALSE;

        aSetBoxInfo.ResetFlags();

        for ( USHORT i = 0; i < aUnions.Count(); ++i )
        {
            SwSelUnion *pUnion = aUnions[i];
            const SwTabFrm *pTab = pUnion->GetTable();
            const SwRect &rUnion = pUnion->GetUnion();
            const BOOL bFirst = i == 0 ? TRUE : FALSE;
            const BOOL bLast  = i == aUnions.Count() - 1 ? TRUE : FALSE;

            SvPtrarr aCellArr( 255, 255 );
            ::lcl_CollectCells( aCellArr, rUnion, (SwTabFrm*)pTab );

            for ( USHORT j = 0; j < aCellArr.Count(); ++j )
            {
                const SwCellFrm *pCell = (const SwCellFrm*)aCellArr[j];
                const sal_Bool bVert = pTab->IsVertical();
                const sal_Bool bRTL = pTab->IsRightToLeft();
                sal_Bool bTopOver, bLeftOver, bRightOver, bBottomOver;
                if ( bVert )
                {
                    bTopOver = pCell->Frm().Right() >= rUnion.Right();
                    bLeftOver = pCell->Frm().Top() <= rUnion.Top();
                    bRightOver = pCell->Frm().Bottom() >= rUnion.Bottom();
                    bBottomOver = pCell->Frm().Left() <= rUnion.Left();
                }
                else
                {
                    bTopOver = pCell->Frm().Top() <= rUnion.Top();
                    bLeftOver = pCell->Frm().Left() <= rUnion.Left();
                    bRightOver = pCell->Frm().Right() >= rUnion.Right();
                    bBottomOver = pCell->Frm().Bottom() >= rUnion.Bottom();
                }

                if ( bRTL )
                {
                    sal_Bool bTmp = bRightOver;
                    bRightOver = bLeftOver;
                    bLeftOver = bTmp;
                }

                const SwFrmFmt  *pFmt  = pCell->GetFmt();
                const SvxBoxItem  &rBox  = pFmt->GetBox();

                //Obere Kante
                if ( bFirst && bTopOver )
                {
                    if (aSetBoxInfo.IsValid(VALID_TOP))
                    {
                        if ( !bTopSet )
                        {   bTopSet = TRUE;
                            aSetBox.SetLine( rBox.GetTop(), BOX_LINE_TOP );
                        }
                        else if ((aSetBox.GetTop() && rBox.GetTop() &&
                                 !(*aSetBox.GetTop() == *rBox.GetTop())) ||
                                 ((!aSetBox.GetTop()) ^ (!rBox.GetTop()))) // XOR-Ausdruck ist TRUE, wenn genau einer der beiden Pointer 0 ist
                        {
                            aSetBoxInfo.SetValid(VALID_TOP, FALSE );
                            aSetBox.SetLine( 0, BOX_LINE_TOP );
                        }
                    }
                }

                //Linke Kante
                if ( bLeftOver )
                {
                    if (aSetBoxInfo.IsValid(VALID_LEFT))
                    {
                        if ( !bLeftSet )
                        {   bLeftSet = TRUE;
                            aSetBox.SetLine( rBox.GetLeft(), BOX_LINE_LEFT );
                        }
                        else if ((aSetBox.GetLeft() && rBox.GetLeft() &&
                                 !(*aSetBox.GetLeft() == *rBox.GetLeft())) ||
                                 ((!aSetBox.GetLeft()) ^ (!rBox.GetLeft())))
                        {
                            aSetBoxInfo.SetValid(VALID_LEFT, FALSE );
                            aSetBox.SetLine( 0, BOX_LINE_LEFT );
                        }
                    }
                }
                else
                {
                    if (aSetBoxInfo.IsValid(VALID_VERT))
                    {
                        if ( !bVertSet )
                        {   bVertSet = TRUE;
                            aSetBoxInfo.SetLine( rBox.GetLeft(), BOXINFO_LINE_VERT );
                        }
                        else if ((aSetBoxInfo.GetVert() && rBox.GetLeft() &&
                                 !(*aSetBoxInfo.GetVert() == *rBox.GetLeft())) ||
                                 ((!aSetBoxInfo.GetVert()) ^ (!rBox.GetLeft())))
                        {   aSetBoxInfo.SetValid( VALID_VERT, FALSE );
                            aSetBoxInfo.SetLine( 0, BOXINFO_LINE_VERT );
                        }
                    }
                }

                //Rechte Kante
                if ( aSetBoxInfo.IsValid(VALID_RIGHT) && bRightOver )
                {
                    if ( !bRightSet )
                    {   bRightSet = TRUE;
                        aSetBox.SetLine( rBox.GetRight(), BOX_LINE_RIGHT );
                    }
                    else if ((aSetBox.GetRight() && rBox.GetRight() &&
                             !(*aSetBox.GetRight() == *rBox.GetRight())) ||
                             (!aSetBox.GetRight() ^ !rBox.GetRight()))
                    {   aSetBoxInfo.SetValid( VALID_RIGHT, FALSE );
                        aSetBox.SetLine( 0, BOX_LINE_RIGHT );
                    }
                }

                //Untere Kante
                if ( bLast && bBottomOver )
                {
                    if ( aSetBoxInfo.IsValid(VALID_BOTTOM) )
                    {
                        if ( !bBottomSet )
                        {   bBottomSet = TRUE;
                            aSetBox.SetLine( rBox.GetBottom(), BOX_LINE_BOTTOM );
                        }
                        else if ((aSetBox.GetBottom() && rBox.GetBottom() &&
                                 !(*aSetBox.GetBottom() == *rBox.GetBottom())) ||
                                 (!aSetBox.GetBottom() ^ !rBox.GetBottom()))
                        {   aSetBoxInfo.SetValid( VALID_BOTTOM, FALSE );
                            aSetBox.SetLine( 0, BOX_LINE_BOTTOM );
                        }
                    }
                }
                //in allen Zeilen ausser der letzten werden die
                // horiz. Linien aus der Bottom-Linie entnommen
                else
                {
                    if (aSetBoxInfo.IsValid(VALID_HORI))
                    {
                        if ( !bHoriSet )
                        {   bHoriSet = TRUE;
                            aSetBoxInfo.SetLine( rBox.GetBottom(), BOXINFO_LINE_HORI );
                        }
                        else if ((aSetBoxInfo.GetHori() && rBox.GetBottom() &&
                                 !(*aSetBoxInfo.GetHori() == *rBox.GetBottom())) ||
                                 ((!aSetBoxInfo.GetHori()) ^ (!rBox.GetBottom())))
                        {
                            aSetBoxInfo.SetValid( VALID_HORI, FALSE );
                            aSetBoxInfo.SetLine( 0, BOXINFO_LINE_HORI );
                        }
                    }
                }

                // Abstand zum Text
                if (aSetBoxInfo.IsValid(VALID_DISTANCE))
                {
                    static USHORT const aBorders[] = {
                        BOX_LINE_BOTTOM, BOX_LINE_TOP,
                        BOX_LINE_RIGHT, BOX_LINE_LEFT };
                    const USHORT* pBrd = aBorders;

                    if( !bDistanceSet )     // bei 1. Durchlauf erstmal setzen
                    {
                        bDistanceSet = TRUE;
                        for( int k = 0; k < 4; ++k, ++pBrd )
                            aSetBox.SetDistance( rBox.GetDistance( *pBrd ),
                                                *pBrd );
                    }
                    else
                    {
                        for( int k = 0; k < 4; ++k, ++pBrd )
                            if( aSetBox.GetDistance( *pBrd ) !=
                                rBox.GetDistance( *pBrd ) )
                            {
                                aSetBoxInfo.SetValid( VALID_DISTANCE, FALSE );
                                aSetBox.SetDistance( (USHORT) 0 );
                                break;
                            }
                    }
                }
            }
        }
        rSet.Put( aSetBox );
        rSet.Put( aSetBoxInfo );
    }
}

/***********************************************************************
#*  Class      :  SwDoc
#*  Methoden   :  SetBoxAttr
#***********************************************************************/
void SwDoc::SetBoxAttr( const SwCursor& rCursor, const SfxPoolItem &rNew )
{
    SwTableNode* pTblNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    SwSelBoxes aBoxes;
    if( pTblNd && ::lcl_GetBoxSel( rCursor, aBoxes, TRUE ) )
    {
        SwTable& rTable = pTblNd->GetTable();
        if( DoesUndo() )
        {
            ClearRedo();
            AppendUndo( new SwUndoAttrTbl( *pTblNd ));
        }

        SvPtrarr aFmtCmp( Max( BYTE(255), BYTE(aBoxes.Count()) ), 255 );
        for ( USHORT i = 0; i < aBoxes.Count(); ++i )
        {
            SwTableBox *pBox = aBoxes[i];

            SwFrmFmt *pNewFmt;
            if ( 0 != (pNewFmt = SwTblFmtCmp::FindNewFmt( aFmtCmp, pBox->GetFrmFmt(), 0 )))
                pBox->ChgFrmFmt( (SwTableBoxFmt*)pNewFmt );
            else
            {
                SwFrmFmt *pOld = pBox->GetFrmFmt();
                SwFrmFmt *pNew = pBox->ClaimFrmFmt();
                pNew->SetFmtAttr( rNew );
                aFmtCmp.Insert( new SwTblFmtCmp( pOld, pNew, 0 ), aFmtCmp.Count());
            }
        }

        SwHTMLTableLayout *pTableLayout = rTable.GetHTMLTableLayout();
        if( pTableLayout )
        {
            SwCntntFrm* pFrm = rCursor.GetCntntNode()->GetFrm();
            SwTabFrm* pTabFrm = pFrm->ImplFindTabFrm();

            pTableLayout->Resize(
                pTableLayout->GetBrowseWidthByTabFrm( *pTabFrm ), TRUE );
        }
        SwTblFmtCmp::Delete( aFmtCmp );
        SetModified();
    }
}

/***********************************************************************
#*  Class      :  SwDoc
#*  Methoden   :  GetBoxAttr()
#***********************************************************************/

BOOL SwDoc::GetBoxAttr( const SwCursor& rCursor, SfxPoolItem& rToFill ) const
{
    BOOL bRet = FALSE;
    SwTableNode* pTblNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    SwSelBoxes aBoxes;
    if( pTblNd && lcl_GetBoxSel( rCursor, aBoxes ))
    {
        bRet = TRUE;
        BOOL bOneFound = FALSE;
        const USHORT nWhich = rToFill.Which();
        for( USHORT i = 0; i < aBoxes.Count(); ++i )
        {
            switch ( nWhich )
            {
                case RES_BACKGROUND:
                {
                    const SvxBrushItem &rBack =
                                    aBoxes[i]->GetFrmFmt()->GetBackground();
                    if( !bOneFound )
                    {
                        (SvxBrushItem&)rToFill = rBack;
                        bOneFound = TRUE;
                    }
                    else if( rToFill != rBack )
                        bRet = FALSE;
                }
                break;

                case RES_FRAMEDIR:
                {
                    const SvxFrameDirectionItem& rDir =
                                    aBoxes[i]->GetFrmFmt()->GetFrmDir();
                    if( !bOneFound )
                    {
                        (SvxFrameDirectionItem&)rToFill = rDir;
                        bOneFound = TRUE;
                    }
                    else if( rToFill != rDir )
                        bRet = FALSE;
                }
                case RES_VERT_ORIENT:
                {
                    const SwFmtVertOrient& rOrient =
                                    aBoxes[i]->GetFrmFmt()->GetVertOrient();
                    if( !bOneFound )
                    {
                        (SwFmtVertOrient&)rToFill = rOrient;
                        bOneFound = TRUE;
                    }
                    else if( rToFill != rOrient )
                        bRet = FALSE;
                }
            }

            if ( FALSE == bRet )
                break;
        }
    }
    return bRet;
}

/***********************************************************************
#*  Class      :  SwDoc
#*  Methoden   :  SetBoxAlign, SetBoxAlign
#***********************************************************************/
void SwDoc::SetBoxAlign( const SwCursor& rCursor, USHORT nAlign )
{
    OSL_ENSURE( nAlign == text::VertOrientation::NONE   ||
            nAlign == text::VertOrientation::CENTER ||
            nAlign == text::VertOrientation::BOTTOM, "wrong alignment" );
    SwFmtVertOrient aVertOri( 0, nAlign );
    SetBoxAttr( rCursor, aVertOri );
}

USHORT SwDoc::GetBoxAlign( const SwCursor& rCursor ) const
{
    USHORT nAlign = USHRT_MAX;
    SwTableNode* pTblNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    SwSelBoxes aBoxes;
    if( pTblNd && ::lcl_GetBoxSel( rCursor, aBoxes ))
        for( USHORT i = 0; i < aBoxes.Count(); ++i )
        {
            const SwFmtVertOrient &rOri =
                            aBoxes[i]->GetFrmFmt()->GetVertOrient();
            if( USHRT_MAX == nAlign )
                nAlign = static_cast<USHORT>(rOri.GetVertOrient());
            else if( rOri.GetVertOrient() != nAlign )
            {
                nAlign = USHRT_MAX;
                break;
            }
        }
    return nAlign;
}


/***********************************************************************
#*  Class      :  SwDoc
#*  Methoden   :  AdjustCellWidth()
#***********************************************************************/
USHORT lcl_CalcCellFit( const SwLayoutFrm *pCell )
{
    SwTwips nRet = 0;
    const SwFrm *pFrm = pCell->Lower(); //Die ganze Zelle.
    SWRECTFN( pCell )
    while ( pFrm )
    {
        const SwTwips nAdd = (pFrm->Frm().*fnRect->fnGetWidth)() -
                             (pFrm->Prt().*fnRect->fnGetWidth)();

        // pFrm does not necessarily have to be a SwTxtFrm!
        const SwTwips nCalcFitToContent = pFrm->IsTxtFrm() ?
                                          ((SwTxtFrm*)pFrm)->CalcFitToContent() :
                                          (pFrm->Prt().*fnRect->fnGetWidth)();

        nRet = Max( nRet, nCalcFitToContent + nAdd );
        pFrm = pFrm->GetNext();
    }
    //Umrandung und linker/rechter Rand wollen mit kalkuliert werden.
    nRet += (pCell->Frm().*fnRect->fnGetWidth)() -
            (pCell->Prt().*fnRect->fnGetWidth)();

    //Um Rechenungenauikeiten, die spaeter bei SwTable::SetTabCols enstehen,
    //auszugleichen, addieren wir noch ein bischen.
    nRet += COLFUZZY;
    return (USHORT)Max( long(MINLAY), nRet );
}

/*Die Zelle ist in der Selektion, wird aber nicht von den TabCols beschrieben.
 *Das bedeutet, dass die Zelle aufgrund der zweidimensionalen Darstellung von
 *anderen Zellen "geteilt" wurde. Wir muessen also den Wunsch- bzw. Minimalwert
 *der Zelle auf die Spalten, durch die sie geteilt wurde verteilen.
 *
 *Dazu sammeln wir zuerst die Spalten - nicht die Spaltentrenner! - ein, die
 *sich mit der Zelle ueberschneiden. Den Wunschwert der Zelle verteilen wir
 *dann anhand des Betrages der Ueberschneidung auf die Zellen.
 *Wenn eine Zelle bereits einen groesseren Wunschwert angemeldet hat, so bleibt
 *dieser erhalten, kleinere Wuensche werden ueberschrieben.
 */

void lcl_CalcSubColValues( SvUShorts &rToFill, const SwTabCols &rCols,
                              const SwLayoutFrm *pCell, const SwLayoutFrm *pTab,
                              BOOL bWishValues )
{
    const USHORT nWish = bWishValues ?
                    ::lcl_CalcCellFit( pCell ) :
                    MINLAY + USHORT(pCell->Frm().Width() - pCell->Prt().Width());

    SWRECTFN( pTab )

    for ( USHORT i = 0 ; i <= rCols.Count(); ++i )
    {
        long nColLeft  = i == 0             ? rCols.GetLeft()  : rCols[i-1];
        long nColRight = i == rCols.Count() ? rCols.GetRight() : rCols[i];
        nColLeft  += rCols.GetLeftMin();
        nColRight += rCols.GetLeftMin();

        //Werte auf die Verhaeltnisse der Tabelle (Follows) anpassen.
        if ( rCols.GetLeftMin() !=  USHORT((pTab->Frm().*fnRect->fnGetLeft)()) )
        {
            const long nDiff = (pTab->Frm().*fnRect->fnGetLeft)() - rCols.GetLeftMin();
            nColLeft  += nDiff;
            nColRight += nDiff;
        }
        const long nCellLeft  = (pCell->Frm().*fnRect->fnGetLeft)();
        const long nCellRight = (pCell->Frm().*fnRect->fnGetRight)();

        //Ueberschneidungsbetrag ermitteln.
        long nWidth = 0;
        if ( nColLeft <= nCellLeft && nColRight >= (nCellLeft+COLFUZZY) )
            nWidth = nColRight - nCellLeft;
        else if ( nColLeft <= (nCellRight-COLFUZZY) && nColRight >= nCellRight )
            nWidth = nCellRight - nColLeft;
        else if ( nColLeft >= nCellLeft && nColRight <= nCellRight )
            nWidth = nColRight - nColLeft;
        if ( nWidth && pCell->Frm().Width() )
        {
            long nTmp = nWidth * nWish / pCell->Frm().Width();
            if ( USHORT(nTmp) > rToFill[i] )
                rToFill[i] = USHORT(nTmp);
        }
    }
}

/*Besorgt neue Werte zu Einstellung der TabCols.
 *Es wird nicht ueber die Eintrage in den TabCols itereriert, sondern
 *quasi ueber die Zwischenraeume, die ja die Zellen beschreiben.
 *
 *bWishValues == TRUE:  Es werden zur aktuellen Selektion bzw. zur aktuellen
 *                      Zelle die Wunschwerte aller betroffen Zellen ermittelt.
 *                      Sind mehrere Zellen in einer Spalte, so wird der
 *                      groesste Wunschwert als Ergebnis geliefert.
 *                      Fuer die TabCol-Eintraege, zu denen keine Zellen
 *                      ermittelt wurden, werden 0-en eingetragen.
 *
 *bWishValues == FALSE: Die Selektion wird senkrecht ausgedehnt. Zu jeder
 *                      Spalte in den TabCols, die sich mit der Selektion
 *                      schneidet wird der Minimalwert ermittelt.
 */

void lcl_CalcColValues( SvUShorts &rToFill, const SwTabCols &rCols,
                           const SwLayoutFrm *pStart, const SwLayoutFrm *pEnd,
                           BOOL bWishValues )
{
    SwSelUnions aUnions;
    ::MakeSelUnions( aUnions, pStart, pEnd,
                    bWishValues ? nsSwTblSearchType::TBLSEARCH_NONE : nsSwTblSearchType::TBLSEARCH_COL );

    for ( USHORT i2 = 0; i2 < aUnions.Count(); ++i2 )
    {
        SwSelUnion *pSelUnion = aUnions[i2];
        const SwTabFrm *pTab = pSelUnion->GetTable();
        const SwRect &rUnion = pSelUnion->GetUnion();

        SWRECTFN( pTab )
        sal_Bool bRTL = pTab->IsRightToLeft();

        const SwLayoutFrm *pCell = pTab->FirstCell();
        do
        {
            if ( pCell->IsCellFrm() && pCell->FindTabFrm() == pTab && ::IsFrmInTblSel( rUnion, pCell ) )
            {
                const long nCLeft  = (pCell->Frm().*fnRect->fnGetLeft)();
                const long nCRight = (pCell->Frm().*fnRect->fnGetRight)();

                BOOL bNotInCols = TRUE;

                for ( USHORT i = 0; i <= rCols.Count(); ++i )
                {
                    USHORT nFit = rToFill[i];
                    long nColLeft  = i == 0             ? rCols.GetLeft()  : rCols[i-1];
                    long nColRight = i == rCols.Count() ? rCols.GetRight() : rCols[i];

                    if ( bRTL )
                    {
                        long nTmpRight = nColRight;
                        nColRight = rCols.GetRight() - nColLeft;
                        nColLeft = rCols.GetRight() - nTmpRight;
                    }

                    nColLeft  += rCols.GetLeftMin();
                    nColRight += rCols.GetLeftMin();

                    //Werte auf die Verhaeltnisse der Tabelle (Follows) anpassen.
                    long nLeftA  = nColLeft;
                    long nRightA = nColRight;
                    if ( rCols.GetLeftMin() !=  USHORT((pTab->Frm().*fnRect->fnGetLeft)()) )
                    {
                        const long nDiff = (pTab->Frm().*fnRect->fnGetLeft)() - rCols.GetLeftMin();
                        nLeftA  += nDiff;
                        nRightA += nDiff;
                    }

                    //Wir wollen nicht allzu genau hinsehen.
                    if ( ::IsSame(nCLeft, nLeftA) && ::IsSame(nCRight, nRightA))
                    {
                        bNotInCols = FALSE;
                        if ( bWishValues )
                        {
                            const USHORT nWish = ::lcl_CalcCellFit( pCell );
                            if ( nWish > nFit )
                                nFit = nWish;
                        }
                        else
                        {   const USHORT nMin = MINLAY + USHORT(pCell->Frm().Width() -
                                                                pCell->Prt().Width());
                            if ( !nFit || nMin < nFit )
                                nFit = nMin;
                        }
                        if ( rToFill[i] < nFit )
                            rToFill[i] = nFit;
                    }
                }
                if ( bNotInCols )
                    ::lcl_CalcSubColValues( rToFill, rCols, pCell, pTab, bWishValues );
            }
            do {
                pCell = pCell->GetNextLayoutLeaf();
            }while( pCell && pCell->Frm().Width() == 0 );
        } while ( pCell && pTab->IsAnLower( pCell ) );
    }
}


void SwDoc::AdjustCellWidth( const SwCursor& rCursor, BOOL bBalance )
{
    // pruefe ob vom aktuellen Crsr der Point/Mark in einer Tabelle stehen
    SwCntntNode* pCntNd = rCursor.GetPoint()->nNode.GetNode().GetCntntNode();
    SwTableNode* pTblNd = pCntNd ? pCntNd->FindTableNode() : 0;
    if( !pTblNd )
        return ;

    SwLayoutFrm *pStart, *pEnd;
    ::lcl_GetStartEndCell( rCursor, pStart, pEnd );

    //TabCols besorgen, den ueber diese stellen wir die Tabelle neu ein.
    SwFrm* pBoxFrm = pStart;
    while( pBoxFrm && !pBoxFrm->IsCellFrm() )
        pBoxFrm = pBoxFrm->GetUpper();

    if ( !pBoxFrm )
        return; // robust

    SwTabCols aTabCols;
    GetTabCols( aTabCols, 0, (SwCellFrm*)pBoxFrm );

    if ( ! aTabCols.Count() )
        return;

    const BYTE nTmp = (BYTE)Max( USHORT(255), USHORT(aTabCols.Count() + 1) );
    SvUShorts aWish( nTmp, nTmp ),
              aMins( nTmp, nTmp );
    USHORT i;

    for ( i = 0; i <= aTabCols.Count(); ++i )
    {
        aWish.Insert( USHORT(0), aWish.Count() );
        aMins.Insert( USHORT(0), aMins.Count() );
    }
    ::lcl_CalcColValues( aWish, aTabCols, pStart, pEnd, TRUE  );

    //Es ist Robuster wenn wir die Min-Werte fuer die ganze Tabelle berechnen.
    const SwTabFrm *pTab = pStart->ImplFindTabFrm();
    pStart = (SwLayoutFrm*)pTab->FirstCell();
    pEnd   = (SwLayoutFrm*)pTab->FindLastCntnt()->GetUpper();
    while( !pEnd->IsCellFrm() )
        pEnd = pEnd->GetUpper();
    ::lcl_CalcColValues( aMins, aTabCols, pStart, pEnd, FALSE );

    if( bBalance )
    {
        //Alle Spalten, die makiert sind haben jetzt einen Wunschwert
        //eingtragen. Wir addieren die aktuellen Werte, teilen das Ergebnis
        //durch die Anzahl und haben eine Wunschwert fuer den ausgleich.
        USHORT nWish = 0, nCnt = 0;
        for ( i = 0; i <= aTabCols.Count(); ++i )
        {
            int nDiff = aWish[i];
            if ( nDiff )
            {
                if ( i == 0 )
                    nWish = static_cast<USHORT>( nWish + aTabCols[i] - aTabCols.GetLeft() );
                else if ( i == aTabCols.Count() )
                    nWish = static_cast<USHORT>(nWish + aTabCols.GetRight() - aTabCols[i-1] );
                else
                    nWish = static_cast<USHORT>(nWish + aTabCols[i] - aTabCols[i-1] );
                ++nCnt;
            }
        }
        nWish = nWish / nCnt;
        for ( i = 0; i < aWish.Count(); ++i )
            if ( aWish[i] )
                aWish[i] = nWish;
    }

    const USHORT nOldRight = static_cast<USHORT>(aTabCols.GetRight());

    //Um die Impl. einfach zu gestalten, aber trotzdem in den meissten Faellen
    //den Platz richtig auszunutzen laufen wir zweimal.
    //Problem: Erste Spalte wird breiter, die anderen aber erst danach
    //schmaler. Die Wunschbreite der ersten Spalte wuerde abgelehnt, weil
    //mit ihr die max. Breite der Tabelle ueberschritten wuerde.
    for ( USHORT k= 0; k < 2; ++k )
    {
        for ( i = 0; i <= aTabCols.Count(); ++i )
        {
            int nDiff = aWish[i];
            if ( nDiff )
            {
                int nMin = aMins[i];
                if ( nMin > nDiff )
                    nDiff = nMin;

                if ( i == 0 )
                {
                    if( aTabCols.Count() )
                        nDiff -= aTabCols[0] - aTabCols.GetLeft();
                    else
                        nDiff -= aTabCols.GetRight() - aTabCols.GetLeft();
                }
                else if ( i == aTabCols.Count() )
                    nDiff -= aTabCols.GetRight() - aTabCols[i-1];
                else
                    nDiff -= aTabCols[i] - aTabCols[i-1];

                long nTabRight = aTabCols.GetRight() + nDiff;

                //Wenn die Tabelle zu breit wuerde begrenzen wir die Anpassung
                //auf das erlaubte Maximum.
                if ( !bBalance && nTabRight > aTabCols.GetRightMax() )
                {
                    const long nTmpD = nTabRight - aTabCols.GetRightMax();
                    nDiff     -= nTmpD;
                    nTabRight -= nTmpD;
                }
                for ( USHORT i2 = i; i2 < aTabCols.Count(); ++i2 )
                    aTabCols[i2] += nDiff;
                aTabCols.SetRight( nTabRight );
            }
        }
    }

    const USHORT nNewRight = static_cast<USHORT>(aTabCols.GetRight());

    SwFrmFmt *pFmt = pTblNd->GetTable().GetFrmFmt();
    const sal_Int16 nOriHori = pFmt->GetHoriOrient().GetHoriOrient();

    //So, die richtige Arbeit koennen wir jetzt der SwTable ueberlassen.
    SetTabCols( aTabCols, FALSE, 0, (SwCellFrm*)pBoxFrm );

    // i54248: lijian/fme
    // alignment might have been changed in SetTabCols, restore old value:
    const SwFmtHoriOrient &rHori = pFmt->GetHoriOrient();
    SwFmtHoriOrient aHori( rHori );
    if ( aHori.GetHoriOrient() != nOriHori )
    {
        aHori.SetHoriOrient( nOriHori );
        pFmt->SetFmtAttr( aHori );
    }

    //Bei Automatischer Breite wird auf Linksbuendig umgeschaltet.
    //Bei Randattributen wird der Rechte Rand angepasst.
    if( !bBalance && nNewRight < nOldRight )
    {
        if( aHori.GetHoriOrient() == text::HoriOrientation::FULL )
        {
            aHori.SetHoriOrient( text::HoriOrientation::LEFT );
            pFmt->SetFmtAttr( aHori );
        }
    }

    SetModified();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
