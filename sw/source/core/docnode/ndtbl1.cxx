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
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
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
#include "switerator.hxx"
#include <UndoTable.hxx>
#include <boost/foreach.hpp>

using ::editeng::SvxBorderLine;
using namespace ::com::sun::star;


extern void ClearFEShellTabCols();

//siehe auch swtable.cxx
#define COLFUZZY 20L

inline sal_Bool IsSame( long nA, long nB ) { return  Abs(nA-nB) <= COLFUZZY; }

class SwTblFmtCmp
{
public:
    SwFrmFmt *pOld,
             *pNew;
    sal_Int16     nType;

    SwTblFmtCmp( SwFrmFmt *pOld, SwFrmFmt *pNew, sal_Int16 nType );

    static SwFrmFmt *FindNewFmt( std::vector<SwTblFmtCmp*> &rArr, SwFrmFmt*pOld, sal_Int16 nType );
    static void Delete( std::vector<SwTblFmtCmp*> &rArr );
};


SwTblFmtCmp::SwTblFmtCmp( SwFrmFmt *pO, SwFrmFmt *pN, sal_Int16 nT )
    : pOld ( pO ), pNew ( pN ), nType( nT )
{
}

SwFrmFmt *SwTblFmtCmp::FindNewFmt( std::vector<SwTblFmtCmp*> &rArr, SwFrmFmt *pOld, sal_Int16 nType )
{
    for ( sal_uInt16 i = 0; i < rArr.size(); ++i )
    {
        SwTblFmtCmp *pCmp = rArr[i];
        if ( pCmp->pOld == pOld && pCmp->nType == nType )
            return pCmp->pNew;
    }
    return 0;
}

void SwTblFmtCmp::Delete( std::vector<SwTblFmtCmp*> &rArr )
{
    for ( sal_uInt16 i = 0; i < rArr.size(); ++i )
        delete rArr[i];
}

void lcl_GetStartEndCell( const SwCursor& rCrsr,
                        SwLayoutFrm *&prStart, SwLayoutFrm *&prEnd )
{
    OSL_ENSURE( rCrsr.GetCntntNode() && rCrsr.GetCntntNode( sal_False ),
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
    SwCntntNode* pMarkNd  = rCrsr.GetCntntNode(sal_False);

    SwFrm* pPointFrm = pPointNd ? pPointNd->getLayoutFrm( pPointNd->GetDoc()->GetCurrentLayout(), &aPtPos ) : 0;
    SwFrm* pMarkFrm  = pMarkNd  ? pMarkNd->getLayoutFrm( pMarkNd->GetDoc()->GetCurrentLayout(), &aMkPos )  : 0;

    prStart = pPointFrm ? pPointFrm->GetUpper() : 0;
    prEnd   = pMarkFrm  ? pMarkFrm->GetUpper() : 0;
}

sal_Bool lcl_GetBoxSel( const SwCursor& rCursor, SwSelBoxes& rBoxes,
                    sal_Bool bAllCrsr = sal_False )
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
                rBoxes.insert( pBox );
            }
        } while( bAllCrsr &&
                pSttPam != ( pCurPam = (SwPaM*)pCurPam->GetNext()) );
    }
    return !rBoxes.empty();
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

inline void InsertLine( std::vector<SwTableLine*>& rLineArr, SwTableLine* pLine )
{
    if( rLineArr.end() == std::find( rLineArr.begin(), rLineArr.end(), pLine ) )
        rLineArr.push_back( pLine );
}

//-----------------------------------------------------------------------------

sal_Bool lcl_IsAnLower( const SwTableLine *pLine, const SwTableLine *pAssumed )
{
    const SwTableLine *pTmp = pAssumed->GetUpper() ?
                                    pAssumed->GetUpper()->GetUpper() : 0;
    while ( pTmp )
    {
        if ( pTmp == pLine )
            return sal_True;
        pTmp = pTmp->GetUpper() ? pTmp->GetUpper()->GetUpper() : 0;
    }
    return sal_False;
}
//-----------------------------------------------------------------------------

struct LinesAndTable
{
    std::vector<SwTableLine*> &rLines;
    const SwTable             &rTable;
    bool                      bInsertLines;

    LinesAndTable( std::vector<SwTableLine*> &rL, const SwTable &rTbl ) :
          rLines( rL ), rTable( rTbl ), bInsertLines( true ) {}
};


sal_Bool _FindLine( _FndLine & rLine, LinesAndTable* pPara );

sal_Bool _FindBox( _FndBox & rBox, LinesAndTable* pPara )
{
    if (!rBox.GetLines().empty())
    {
        pPara->bInsertLines = sal_True;
        BOOST_FOREACH( _FndLine & rFndLine, rBox.GetLines() )
            _FindLine( rFndLine, pPara );
        if ( pPara->bInsertLines )
        {
            const SwTableLines &rLines = (rBox.GetBox())
                                    ? rBox.GetBox()->GetTabLines()
                                    : pPara->rTable.GetTabLines();
            if (rBox.GetLines().size() == rLines.size())
            {
                for ( sal_uInt16 i = 0; i < rLines.size(); ++i )
                    ::InsertLine( pPara->rLines,
                                  (SwTableLine*)rLines[i] );
            }
            else
                pPara->bInsertLines = sal_False;
        }
    }
    else if (rBox.GetBox())
    {
        ::InsertLine( pPara->rLines,
                      static_cast<SwTableLine*>(rBox.GetBox()->GetUpper()));
    }
    return sal_True;
}

sal_Bool _FindLine( _FndLine& rLine, LinesAndTable* pPara )
{
    for (_FndBoxes::iterator it = rLine.GetBoxes().begin();
         it != rLine.GetBoxes().end(); ++it)
    {
        _FindBox(*it, pPara);
    }
    return sal_True;
}

void lcl_CollectLines( std::vector<SwTableLine*> &rArr, const SwCursor& rCursor, bool bRemoveLines )
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
        ForEach_FndLineCopyCol( (SwTableLines&)rTable.GetTabLines(), &aTmpPara );
    }

    //Diejenigen Lines einsammeln, die nur selektierte Boxen enthalten.
    ::_FindBox(aFndBox, &aPara);

    // Remove lines, that have a common superordinate row.
    // (Not for row split)
    if ( bRemoveLines )
    {
        for ( sal_uInt16 i = 0; i < rArr.size(); ++i )
        {
            SwTableLine *pUpLine = rArr[i];
            for ( sal_uInt16 k = 0; k < rArr.size(); ++k )
            {
                if ( k != i && ::lcl_IsAnLower( pUpLine, rArr[k] ) )
                {
                    rArr.erase( rArr.begin() + k );
                    if ( k <= i )
                        --i;
                    --k;
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------

void lcl_ProcessRowAttr( std::vector<SwTblFmtCmp*>& rFmtCmp, SwTableLine* pLine, const SfxPoolItem& rNew )
{
    SwFrmFmt *pNewFmt;
    if ( 0 != (pNewFmt = SwTblFmtCmp::FindNewFmt( rFmtCmp, pLine->GetFrmFmt(), 0 )))
        pLine->ChgFrmFmt( (SwTableLineFmt*)pNewFmt );
    else
    {
        SwFrmFmt *pOld = pLine->GetFrmFmt();
        SwFrmFmt *pNew = pLine->ClaimFrmFmt();
        pNew->SetFmtAttr( rNew );
        rFmtCmp.push_back( new SwTblFmtCmp( pOld, pNew, 0 ) );
    }
}

//-----------------------------------------------------------------------------

void lcl_ProcessBoxSize( std::vector<SwTblFmtCmp*> &rFmtCmp, SwTableBox *pBox, const SwFmtFrmSize &rNew );

void lcl_ProcessRowSize( std::vector<SwTblFmtCmp*> &rFmtCmp, SwTableLine *pLine, const SwFmtFrmSize &rNew )
{
    lcl_ProcessRowAttr( rFmtCmp, pLine, rNew );
    SwTableBoxes &rBoxes = pLine->GetTabBoxes();
    for ( sal_uInt16 i = 0; i < rBoxes.size(); ++i )
        ::lcl_ProcessBoxSize( rFmtCmp, rBoxes[i], rNew );
}

//-----------------------------------------------------------------------------

void lcl_ProcessBoxSize( std::vector<SwTblFmtCmp*> &rFmtCmp, SwTableBox *pBox, const SwFmtFrmSize &rNew )
{
    SwTableLines &rLines = pBox->GetTabLines();
    if ( !rLines.empty() )
    {
        SwFmtFrmSize aSz( rNew );
        aSz.SetHeight( rNew.GetHeight() ? rNew.GetHeight() / rLines.size() : 0 );
        for ( sal_uInt16 i = 0; i < rLines.size(); ++i )
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
        std::vector<SwTableLine*> aRowArr; //Zum sammeln Lines.
        ::lcl_CollectLines( aRowArr, rCursor, false );

        if( !aRowArr.empty() )
        {
            if (GetIDocumentUndoRedo().DoesUndo())
            {
                GetIDocumentUndoRedo().AppendUndo(new SwUndoAttrTbl(*pTblNd));
            }

            std::vector<SwTblFmtCmp*> aFmtCmp;
            aFmtCmp.reserve( Max( 255, (int)aRowArr.size() ) );

            for( sal_uInt16 i = 0; i < aRowArr.size(); ++i )
                ::lcl_ProcessRowAttr( aFmtCmp, aRowArr[i], rNew );

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
        std::vector<SwTableLine*> aRowArr; //Zum sammeln der Lines.
        ::lcl_CollectLines( aRowArr, rCursor, false );

        if( !aRowArr.empty() )
        {
            rpSz = &(SwFmtRowSplit&)aRowArr[0]->GetFrmFmt()->GetRowSplit();

            for ( sal_uInt16 i = 1; i < aRowArr.size() && rpSz; ++i )
            {
                if ( (*rpSz).GetValue() != aRowArr[i]->GetFrmFmt()->GetRowSplit().GetValue() )
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
        std::vector<SwTableLine*> aRowArr; //Zum sammeln Lines.
        ::lcl_CollectLines( aRowArr, rCursor, true );

        if( !aRowArr.empty() )
        {
            if (GetIDocumentUndoRedo().DoesUndo())
            {
                GetIDocumentUndoRedo().AppendUndo(new SwUndoAttrTbl(*pTblNd));
            }

            std::vector<SwTblFmtCmp*> aFmtCmp;
            aFmtCmp.reserve( Max( 255, (int)aRowArr.size() ) );
            for ( sal_uInt16 i = 0; i < aRowArr.size(); ++i )
                ::lcl_ProcessRowSize( aFmtCmp, aRowArr[i], rNew );
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
        std::vector<SwTableLine*> aRowArr; //Zum sammeln der Lines.
        ::lcl_CollectLines( aRowArr, rCursor, true );

        if( !aRowArr.empty() )
        {
            rpSz = &(SwFmtFrmSize&)aRowArr[0]->GetFrmFmt()->GetFrmSize();

            for ( sal_uInt16 i = 1; i < aRowArr.size() && rpSz; ++i )
            {
                if ( *rpSz != aRowArr[i]->GetFrmFmt()->GetFrmSize() )
                    rpSz = 0;
            }
            if ( rpSz )
                rpSz = new SwFmtFrmSize( *rpSz );
        }
    }
}

sal_Bool SwDoc::BalanceRowHeight( const SwCursor& rCursor, sal_Bool bTstOnly )
{
    sal_Bool bRet = sal_False;
    SwTableNode* pTblNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    if( pTblNd )
    {
        std::vector<SwTableLine*> aRowArr; //Zum sammeln der Lines.
        ::lcl_CollectLines( aRowArr, rCursor, true );

        if( 1 < aRowArr.size() )
        {
            if( !bTstOnly )
            {
                long nHeight = 0;
                sal_uInt16 i;

                for ( i = 0; i < aRowArr.size(); ++i )
                {
                    SwIterator<SwFrm,SwFmt> aIter( *aRowArr[i]->GetFrmFmt() );
                    SwFrm* pFrm = aIter.First();
                    while ( pFrm )
                    {
                        nHeight = Max( nHeight, pFrm->Frm().Height() );
                        pFrm = aIter.Next();
                    }
                }
                SwFmtFrmSize aNew( ATT_MIN_SIZE, 0, nHeight );

                if (GetIDocumentUndoRedo().DoesUndo())
                {
                    GetIDocumentUndoRedo().AppendUndo(
                            new SwUndoAttrTbl(*pTblNd));
                }

                std::vector<SwTblFmtCmp*> aFmtCmp;
                aFmtCmp.reserve( Max( 255, (int)aRowArr.size() ) );
                for( i = 0; i < aRowArr.size(); ++i )
                    ::lcl_ProcessRowSize( aFmtCmp, (SwTableLine*)aRowArr[i], aNew );
                SwTblFmtCmp::Delete( aFmtCmp );

                SetModified();
            }
            bRet = sal_True;
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
        std::vector<SwTableLine*> aRowArr; //Zum sammeln Lines.
        ::lcl_CollectLines( aRowArr, rCursor, true );

        if( !aRowArr.empty() )
        {
            if (GetIDocumentUndoRedo().DoesUndo())
            {
                GetIDocumentUndoRedo().AppendUndo(new SwUndoAttrTbl(*pTblNd));
            }

            std::vector<SwTblFmtCmp*> aFmtCmp;
            aFmtCmp.reserve( Max( 255, (int)aRowArr.size() ) );

            for( sal_uInt16 i = 0; i < aRowArr.size(); ++i )
                ::lcl_ProcessRowAttr( aFmtCmp, aRowArr[i], rNew );

            SwTblFmtCmp::Delete( aFmtCmp );
            SetModified();
        }
    }
}

/******************************************************************************
 *               SwTwips SwDoc::GetRowBackground() const
 ******************************************************************************/
sal_Bool SwDoc::GetRowBackground( const SwCursor& rCursor, SvxBrushItem &rToFill ) const
{
    sal_Bool bRet = sal_False;
    SwTableNode* pTblNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    if( pTblNd )
    {
        std::vector<SwTableLine*> aRowArr; //Zum sammeln Lines.
        ::lcl_CollectLines( aRowArr, rCursor, true );

        if( !aRowArr.empty() )
        {
            rToFill = aRowArr[0]->GetFrmFmt()->GetBackground();

            bRet = sal_True;
            for ( sal_uInt16 i = 1; i < aRowArr.size(); ++i )
                if ( rToFill != aRowArr[i]->GetFrmFmt()->GetBackground() )
                {
                    bRet = sal_False;
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
inline void InsertCell( std::vector<SwCellFrm*>& rCellArr, SwCellFrm* pCellFrm )
{
    if( rCellArr.end() == std::find( rCellArr.begin(), rCellArr.end(), pCellFrm ) )
        rCellArr.push_back( pCellFrm );
}

//-----------------------------------------------------------------------------
void lcl_CollectCells( std::vector<SwCellFrm*> &rArr, const SwRect &rUnion,
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

    if( !aUnions.empty() )
    {
        SwTable& rTable = pTblNd->GetTable();
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            GetIDocumentUndoRedo().AppendUndo( new SwUndoAttrTbl(*pTblNd) );
        }

        std::vector<SwTblFmtCmp*> aFmtCmp;
        aFmtCmp.reserve( 255 );
        const SvxBoxItem* pSetBox;
        const SvxBoxInfoItem *pSetBoxInfo;

        const SvxBorderLine* pLeft = 0;
        const SvxBorderLine* pRight = 0;
        const SvxBorderLine* pTop = 0;
        const SvxBorderLine* pBottom = 0;
        const SvxBorderLine* pHori = 0;
        const SvxBorderLine* pVert = 0;
        sal_Bool bHoriValid = sal_True, bVertValid = sal_True,
             bTopValid = sal_True, bBottomValid = sal_True,
             bLeftValid = sal_True, bRightValid = sal_True;

        // JP 21.07.95: die Flags im BoxInfo-Item entscheiden, wann eine
        //              BorderLine gueltig ist!!
        if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_BORDER_INNER, sal_False,
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

        if( SFX_ITEM_SET == rSet.GetItemState( RES_BOX, sal_False,
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
            bTopValid = bBottomValid = bLeftValid = bRightValid = sal_False;
            pSetBox = 0;
        }

        sal_Bool bFirst = sal_True;
        for ( sal_uInt16 i = 0; i < aUnions.size(); ++i )
        {
            SwSelUnion *pUnion = &aUnions[i];
            SwTabFrm *pTab = pUnion->GetTable();
            const SwRect &rUnion = pUnion->GetUnion();
            const sal_Bool bLast  = i == aUnions.size() - 1 ? sal_True : sal_False;

            std::vector<SwCellFrm*> aCellArr;
            aCellArr.reserve( 255 );
            ::lcl_CollectCells( aCellArr, pUnion->GetUnion(), pTab );

            //Alle Zellenkanten, die mit dem UnionRect uebereinstimmen oder
            //darueber hinausragen sind Aussenkanten. Alle anderen sind
            //Innenkanten.
            //neu: Die Aussenkanten koennen abhaengig davon, ob es sich um eine
            //Start/Mittlere/Folge -Tabelle (bei Selektionen ueber FollowTabs)
            //handelt doch keine Aussenkanten sein.
            //Aussenkanten werden links, rechts, oben und unten gesetzt.
            //Innenkanten werden nur oben und links gesetzt.
            for ( sal_uInt16 j = 0; j < aCellArr.size(); ++j )
            {
                SwCellFrm *pCell = aCellArr[j];
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

                sal_Int16 nType = 0;

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
                    static sal_uInt16 const aBorders[] = {
                        BOX_LINE_BOTTOM, BOX_LINE_TOP,
                        BOX_LINE_RIGHT, BOX_LINE_LEFT };
                    const sal_uInt16* pBrd = aBorders;
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
                    aFmtCmp.push_back( new SwTblFmtCmp( pOld, pNew, nType ) );
                }
            }

            bFirst = sal_False;
        }

        SwHTMLTableLayout *pTableLayout = rTable.GetHTMLTableLayout();
        if( pTableLayout )
        {
            SwCntntFrm* pFrm = rCursor.GetCntntNode()->getLayoutFrm( rCursor.GetCntntNode()->GetDoc()->GetCurrentLayout() );
            SwTabFrm* pTabFrm = pFrm->ImplFindTabFrm();

            pTableLayout->BordersChanged(
                pTableLayout->GetBrowseWidthByTabFrm( *pTabFrm ), sal_True );
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
                             const Color* pColor, sal_Bool bSetLine,
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

    if( !aUnions.empty() )
    {
        SwTable& rTable = pTblNd->GetTable();
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            GetIDocumentUndoRedo().AppendUndo(new SwUndoAttrTbl(*pTblNd));
        }

        for( sal_uInt16 i = 0; i < aUnions.size(); ++i )
        {
            SwSelUnion *pUnion = &aUnions[i];
            SwTabFrm *pTab = pUnion->GetTable();
            std::vector<SwCellFrm*> aCellArr;
            aCellArr.reserve( 255 );
            ::lcl_CollectCells( aCellArr, pUnion->GetUnion(), pTab );

            for ( sal_uInt16 j = 0; j < aCellArr.size(); ++j )
            {
                SwCellFrm *pCell = aCellArr[j];

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
            SwCntntFrm* pFrm = rCursor.GetCntntNode()->getLayoutFrm( rCursor.GetCntntNode()->GetDoc()->GetCurrentLayout() );
            SwTabFrm* pTabFrm = pFrm->ImplFindTabFrm();

            pTableLayout->BordersChanged(
                pTableLayout->GetBrowseWidthByTabFrm( *pTabFrm ), sal_True );
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

    if( !aUnions.empty() )
    {
        SvxBoxItem     aSetBox    ((const SvxBoxItem    &) rSet.Get(RES_BOX    ));
        SvxBoxInfoItem aSetBoxInfo((const SvxBoxInfoItem&) rSet.Get(SID_ATTR_BORDER_INNER));

        sal_Bool bTopSet      = sal_False,
             bBottomSet   = sal_False,
             bLeftSet     = sal_False,
             bRightSet    = sal_False,
             bHoriSet     = sal_False,
             bVertSet     = sal_False,
             bDistanceSet = sal_False;

        aSetBoxInfo.ResetFlags();

        for ( sal_uInt16 i = 0; i < aUnions.size(); ++i )
        {
            SwSelUnion *pUnion = &aUnions[i];
            const SwTabFrm *pTab = pUnion->GetTable();
            const SwRect &rUnion = pUnion->GetUnion();
            const sal_Bool bFirst = i == 0 ? sal_True : sal_False;
            const sal_Bool bLast  = i == aUnions.size() - 1 ? sal_True : sal_False;

            std::vector<SwCellFrm*> aCellArr;
            aCellArr.reserve(255);
            ::lcl_CollectCells( aCellArr, rUnion, (SwTabFrm*)pTab );

            for ( sal_uInt16 j = 0; j < aCellArr.size(); ++j )
            {
                SwCellFrm *pCell = aCellArr[j];
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
                        {   bTopSet = sal_True;
                            aSetBox.SetLine( rBox.GetTop(), BOX_LINE_TOP );
                        }
                        else if ((aSetBox.GetTop() && rBox.GetTop() &&
                                 !(*aSetBox.GetTop() == *rBox.GetTop())) ||
                                 ((!aSetBox.GetTop()) ^ (!rBox.GetTop()))) // XOR-Ausdruck ist sal_True, wenn genau einer der beiden Pointer 0 ist
                        {
                            aSetBoxInfo.SetValid(VALID_TOP, sal_False );
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
                        {   bLeftSet = sal_True;
                            aSetBox.SetLine( rBox.GetLeft(), BOX_LINE_LEFT );
                        }
                        else if ((aSetBox.GetLeft() && rBox.GetLeft() &&
                                 !(*aSetBox.GetLeft() == *rBox.GetLeft())) ||
                                 ((!aSetBox.GetLeft()) ^ (!rBox.GetLeft())))
                        {
                            aSetBoxInfo.SetValid(VALID_LEFT, sal_False );
                            aSetBox.SetLine( 0, BOX_LINE_LEFT );
                        }
                    }
                }
                else
                {
                    if (aSetBoxInfo.IsValid(VALID_VERT))
                    {
                        if ( !bVertSet )
                        {   bVertSet = sal_True;
                            aSetBoxInfo.SetLine( rBox.GetLeft(), BOXINFO_LINE_VERT );
                        }
                        else if ((aSetBoxInfo.GetVert() && rBox.GetLeft() &&
                                 !(*aSetBoxInfo.GetVert() == *rBox.GetLeft())) ||
                                 ((!aSetBoxInfo.GetVert()) ^ (!rBox.GetLeft())))
                        {   aSetBoxInfo.SetValid( VALID_VERT, sal_False );
                            aSetBoxInfo.SetLine( 0, BOXINFO_LINE_VERT );
                        }
                    }
                }

                //Rechte Kante
                if ( aSetBoxInfo.IsValid(VALID_RIGHT) && bRightOver )
                {
                    if ( !bRightSet )
                    {   bRightSet = sal_True;
                        aSetBox.SetLine( rBox.GetRight(), BOX_LINE_RIGHT );
                    }
                    else if ((aSetBox.GetRight() && rBox.GetRight() &&
                             !(*aSetBox.GetRight() == *rBox.GetRight())) ||
                             (!aSetBox.GetRight() ^ !rBox.GetRight()))
                    {   aSetBoxInfo.SetValid( VALID_RIGHT, sal_False );
                        aSetBox.SetLine( 0, BOX_LINE_RIGHT );
                    }
                }

                //Untere Kante
                if ( bLast && bBottomOver )
                {
                    if ( aSetBoxInfo.IsValid(VALID_BOTTOM) )
                    {
                        if ( !bBottomSet )
                        {   bBottomSet = sal_True;
                            aSetBox.SetLine( rBox.GetBottom(), BOX_LINE_BOTTOM );
                        }
                        else if ((aSetBox.GetBottom() && rBox.GetBottom() &&
                                 !(*aSetBox.GetBottom() == *rBox.GetBottom())) ||
                                 (!aSetBox.GetBottom() ^ !rBox.GetBottom()))
                        {   aSetBoxInfo.SetValid( VALID_BOTTOM, sal_False );
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
                        {   bHoriSet = sal_True;
                            aSetBoxInfo.SetLine( rBox.GetBottom(), BOXINFO_LINE_HORI );
                        }
                        else if ((aSetBoxInfo.GetHori() && rBox.GetBottom() &&
                                 !(*aSetBoxInfo.GetHori() == *rBox.GetBottom())) ||
                                 ((!aSetBoxInfo.GetHori()) ^ (!rBox.GetBottom())))
                        {
                            aSetBoxInfo.SetValid( VALID_HORI, sal_False );
                            aSetBoxInfo.SetLine( 0, BOXINFO_LINE_HORI );
                        }
                    }
                }

                // Abstand zum Text
                if (aSetBoxInfo.IsValid(VALID_DISTANCE))
                {
                    static sal_uInt16 const aBorders[] = {
                        BOX_LINE_BOTTOM, BOX_LINE_TOP,
                        BOX_LINE_RIGHT, BOX_LINE_LEFT };
                    const sal_uInt16* pBrd = aBorders;

                    if( !bDistanceSet )     // bei 1. Durchlauf erstmal setzen
                    {
                        bDistanceSet = sal_True;
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
                                aSetBoxInfo.SetValid( VALID_DISTANCE, sal_False );
                                aSetBox.SetDistance( (sal_uInt16) 0 );
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
    if( pTblNd && ::lcl_GetBoxSel( rCursor, aBoxes, sal_True ) )
    {
        SwTable& rTable = pTblNd->GetTable();
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            GetIDocumentUndoRedo().AppendUndo( new SwUndoAttrTbl(*pTblNd) );
        }

        std::vector<SwTblFmtCmp*> aFmtCmp;
        aFmtCmp.reserve(std::max<size_t>(255, aBoxes.size()));
        for (size_t i = 0; i < aBoxes.size(); ++i)
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
                aFmtCmp.push_back( new SwTblFmtCmp( pOld, pNew, 0 ) );
            }
        }

        SwHTMLTableLayout *pTableLayout = rTable.GetHTMLTableLayout();
        if( pTableLayout )
        {
            SwCntntFrm* pFrm = rCursor.GetCntntNode()->getLayoutFrm( rCursor.GetCntntNode()->GetDoc()->GetCurrentLayout() );
            SwTabFrm* pTabFrm = pFrm->ImplFindTabFrm();

            pTableLayout->Resize(
                pTableLayout->GetBrowseWidthByTabFrm( *pTabFrm ), sal_True );
        }
        SwTblFmtCmp::Delete( aFmtCmp );
        SetModified();
    }
}

/***********************************************************************
#*  Class      :  SwDoc
#*  Methoden   :  GetBoxAttr()
#***********************************************************************/

sal_Bool SwDoc::GetBoxAttr( const SwCursor& rCursor, SfxPoolItem& rToFill ) const
{
    sal_Bool bRet = sal_False;
    SwTableNode* pTblNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    SwSelBoxes aBoxes;
    if( pTblNd && lcl_GetBoxSel( rCursor, aBoxes ))
    {
        bRet = sal_True;
        sal_Bool bOneFound = sal_False;
        const sal_uInt16 nWhich = rToFill.Which();
        for (size_t i = 0; i < aBoxes.size(); ++i)
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
                        bOneFound = sal_True;
                    }
                    else if( rToFill != rBack )
                        bRet = sal_False;
                }
                break;

                case RES_FRAMEDIR:
                {
                    const SvxFrameDirectionItem& rDir =
                                    aBoxes[i]->GetFrmFmt()->GetFrmDir();
                    if( !bOneFound )
                    {
                        (SvxFrameDirectionItem&)rToFill = rDir;
                        bOneFound = sal_True;
                    }
                    else if( rToFill != rDir )
                        bRet = sal_False;
                }
                case RES_VERT_ORIENT:
                {
                    const SwFmtVertOrient& rOrient =
                                    aBoxes[i]->GetFrmFmt()->GetVertOrient();
                    if( !bOneFound )
                    {
                        (SwFmtVertOrient&)rToFill = rOrient;
                        bOneFound = sal_True;
                    }
                    else if( rToFill != rOrient )
                        bRet = sal_False;
                }
            }

            if ( sal_False == bRet )
                break;
        }
    }
    return bRet;
}

/***********************************************************************
#*  Class      :  SwDoc
#*  Methoden   :  SetBoxAlign, SetBoxAlign
#***********************************************************************/
void SwDoc::SetBoxAlign( const SwCursor& rCursor, sal_uInt16 nAlign )
{
    OSL_ENSURE( nAlign == text::VertOrientation::NONE   ||
            nAlign == text::VertOrientation::CENTER ||
            nAlign == text::VertOrientation::BOTTOM, "wrong alignment" );
    SwFmtVertOrient aVertOri( 0, nAlign );
    SetBoxAttr( rCursor, aVertOri );
}

sal_uInt16 SwDoc::GetBoxAlign( const SwCursor& rCursor ) const
{
    sal_uInt16 nAlign = USHRT_MAX;
    SwTableNode* pTblNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    SwSelBoxes aBoxes;
    if( pTblNd && ::lcl_GetBoxSel( rCursor, aBoxes ))
    {
        for (size_t i = 0; i < aBoxes.size(); ++i)
        {
            const SwFmtVertOrient &rOri =
                            aBoxes[i]->GetFrmFmt()->GetVertOrient();
            if( USHRT_MAX == nAlign )
                nAlign = static_cast<sal_uInt16>(rOri.GetVertOrient());
            else if( rOri.GetVertOrient() != nAlign )
            {
                nAlign = USHRT_MAX;
                break;
            }
        }
    }
    return nAlign;
}


/***********************************************************************
#*  Class      :  SwDoc
#*  Methoden   :  AdjustCellWidth()
#***********************************************************************/
sal_uInt16 lcl_CalcCellFit( const SwLayoutFrm *pCell )
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
    return (sal_uInt16)Max( long(MINLAY), nRet );
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

void lcl_CalcSubColValues( std::vector<sal_uInt16> &rToFill, const SwTabCols &rCols,
                              const SwLayoutFrm *pCell, const SwLayoutFrm *pTab,
                              sal_Bool bWishValues )
{
    const sal_uInt16 nWish = bWishValues ?
                    ::lcl_CalcCellFit( pCell ) :
                    MINLAY + sal_uInt16(pCell->Frm().Width() - pCell->Prt().Width());

    SWRECTFN( pTab )

    for ( sal_uInt16 i = 0 ; i <= rCols.Count(); ++i )
    {
        long nColLeft  = i == 0             ? rCols.GetLeft()  : rCols[i-1];
        long nColRight = i == rCols.Count() ? rCols.GetRight() : rCols[i];
        nColLeft  += rCols.GetLeftMin();
        nColRight += rCols.GetLeftMin();

        //Werte auf die Verhaeltnisse der Tabelle (Follows) anpassen.
        if ( rCols.GetLeftMin() !=  sal_uInt16((pTab->Frm().*fnRect->fnGetLeft)()) )
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
            if ( sal_uInt16(nTmp) > rToFill[i] )
                rToFill[i] = sal_uInt16(nTmp);
        }
    }
}

/*Besorgt neue Werte zu Einstellung der TabCols.
 *Es wird nicht ueber die Eintrage in den TabCols itereriert, sondern
 *quasi ueber die Zwischenraeume, die ja die Zellen beschreiben.
 *
 *bWishValues == sal_True:  Es werden zur aktuellen Selektion bzw. zur aktuellen
 *                      Zelle die Wunschwerte aller betroffen Zellen ermittelt.
 *                      Sind mehrere Zellen in einer Spalte, so wird der
 *                      groesste Wunschwert als Ergebnis geliefert.
 *                      Fuer die TabCol-Eintraege, zu denen keine Zellen
 *                      ermittelt wurden, werden 0-en eingetragen.
 *
 *bWishValues == sal_False: Die Selektion wird senkrecht ausgedehnt. Zu jeder
 *                      Spalte in den TabCols, die sich mit der Selektion
 *                      schneidet wird der Minimalwert ermittelt.
 */

void lcl_CalcColValues( std::vector<sal_uInt16> &rToFill, const SwTabCols &rCols,
                           const SwLayoutFrm *pStart, const SwLayoutFrm *pEnd,
                           sal_Bool bWishValues )
{
    SwSelUnions aUnions;
    ::MakeSelUnions( aUnions, pStart, pEnd,
                    bWishValues ? nsSwTblSearchType::TBLSEARCH_NONE : nsSwTblSearchType::TBLSEARCH_COL );

    for ( sal_uInt16 i2 = 0; i2 < aUnions.size(); ++i2 )
    {
        SwSelUnion *pSelUnion = &aUnions[i2];
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

                sal_Bool bNotInCols = sal_True;

                for ( sal_uInt16 i = 0; i <= rCols.Count(); ++i )
                {
                    sal_uInt16 nFit = rToFill[i];
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
                    if ( rCols.GetLeftMin() !=  sal_uInt16((pTab->Frm().*fnRect->fnGetLeft)()) )
                    {
                        const long nDiff = (pTab->Frm().*fnRect->fnGetLeft)() - rCols.GetLeftMin();
                        nLeftA  += nDiff;
                        nRightA += nDiff;
                    }

                    //Wir wollen nicht allzu genau hinsehen.
                    if ( ::IsSame(nCLeft, nLeftA) && ::IsSame(nCRight, nRightA))
                    {
                        bNotInCols = sal_False;
                        if ( bWishValues )
                        {
                            const sal_uInt16 nWish = ::lcl_CalcCellFit( pCell );
                            if ( nWish > nFit )
                                nFit = nWish;
                        }
                        else
                        {   const sal_uInt16 nMin = MINLAY + sal_uInt16(pCell->Frm().Width() -
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


void SwDoc::AdjustCellWidth( const SwCursor& rCursor, sal_Bool bBalance )
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

    std::vector<sal_uInt16> aWish(aTabCols.Count() + 1);
    std::vector<sal_uInt16> aMins(aTabCols.Count() + 1);

    sal_uInt16 i;

    ::lcl_CalcColValues( aWish, aTabCols, pStart, pEnd, sal_True  );

    //Es ist Robuster wenn wir die Min-Werte fuer die ganze Tabelle berechnen.
    const SwTabFrm *pTab = pStart->ImplFindTabFrm();
    pStart = (SwLayoutFrm*)pTab->FirstCell();
    pEnd   = (SwLayoutFrm*)pTab->FindLastCntnt()->GetUpper();
    while( !pEnd->IsCellFrm() )
        pEnd = pEnd->GetUpper();
    ::lcl_CalcColValues( aMins, aTabCols, pStart, pEnd, sal_False );

    if( bBalance )
    {
        //Alle Spalten, die makiert sind haben jetzt einen Wunschwert
        //eingtragen. Wir addieren die aktuellen Werte, teilen das Ergebnis
        //durch die Anzahl und haben eine Wunschwert fuer den ausgleich.
        sal_uInt16 nWish = 0, nCnt = 0;
        for ( i = 0; i <= aTabCols.Count(); ++i )
        {
            int nDiff = aWish[i];
            if ( nDiff )
            {
                if ( i == 0 )
                    nWish = static_cast<sal_uInt16>( nWish + aTabCols[i] - aTabCols.GetLeft() );
                else if ( i == aTabCols.Count() )
                    nWish = static_cast<sal_uInt16>(nWish + aTabCols.GetRight() - aTabCols[i-1] );
                else
                    nWish = static_cast<sal_uInt16>(nWish + aTabCols[i] - aTabCols[i-1] );
                ++nCnt;
            }
        }
        nWish = nWish / nCnt;
        for ( i = 0; i < aWish.size(); ++i )
            if ( aWish[i] )
                aWish[i] = nWish;
    }

    const sal_uInt16 nOldRight = static_cast<sal_uInt16>(aTabCols.GetRight());

    //Um die Impl. einfach zu gestalten, aber trotzdem in den meissten Faellen
    //den Platz richtig auszunutzen laufen wir zweimal.
    //Problem: Erste Spalte wird breiter, die anderen aber erst danach
    //schmaler. Die Wunschbreite der ersten Spalte wuerde abgelehnt, weil
    //mit ihr die max. Breite der Tabelle ueberschritten wuerde.
    for ( sal_uInt16 k= 0; k < 2; ++k )
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
                for ( sal_uInt16 i2 = i; i2 < aTabCols.Count(); ++i2 )
                    aTabCols[i2] += nDiff;
                aTabCols.SetRight( nTabRight );
            }
        }
    }

    const sal_uInt16 nNewRight = static_cast<sal_uInt16>(aTabCols.GetRight());

    SwFrmFmt *pFmt = pTblNd->GetTable().GetFrmFmt();
    const sal_Int16 nOriHori = pFmt->GetHoriOrient().GetHoriOrient();

    //So, die richtige Arbeit koennen wir jetzt der SwTable ueberlassen.
    SetTabCols( aTabCols, sal_False, 0, (SwCellFrm*)pBoxFrm );

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
