/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "hintids.hxx"
#include <editeng/lrspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
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

// See swtable.cxx too
#define COLFUZZY 20L

inline bool IsSame( long nA, long nB ) { return  std::abs(nA-nB) <= COLFUZZY; }

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

static void lcl_GetStartEndCell( const SwCursor& rCrsr,
                        SwLayoutFrm *&prStart, SwLayoutFrm *&prEnd )
{
    OSL_ENSURE( rCrsr.GetCntntNode() && rCrsr.GetCntntNode( sal_False ),
            "Tab selection not at ContentNode" );

    Point aPtPos, aMkPos;
    const SwShellCrsr* pShCrsr = dynamic_cast<const SwShellCrsr*>(&rCrsr);
    if( pShCrsr )
    {
        aPtPos = pShCrsr->GetPtPos();
        aMkPos = pShCrsr->GetMkPos();
    }

    // Robust:
    SwCntntNode* pPointNd = rCrsr.GetCntntNode();
    SwCntntNode* pMarkNd  = rCrsr.GetCntntNode(sal_False);

    SwFrm* pPointFrm = pPointNd ? pPointNd->getLayoutFrm( pPointNd->GetDoc()->GetCurrentLayout(), &aPtPos ) : 0;
    SwFrm* pMarkFrm  = pMarkNd  ? pMarkNd->getLayoutFrm( pMarkNd->GetDoc()->GetCurrentLayout(), &aMkPos )  : 0;

    prStart = pPointFrm ? pPointFrm->GetUpper() : 0;
    prEnd   = pMarkFrm  ? pMarkFrm->GetUpper() : 0;
}

static bool lcl_GetBoxSel( const SwCursor& rCursor, SwSelBoxes& rBoxes,
                    bool bAllCrsr = false )
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

/**
 * Class:  SwDoc
 * Methods:  SetRowHeight(), GetRowHeight()
 *
 * The line height is calculated from the Selection.
 * Starting with every Cell within the Selection, all Cells are iterated
 * through in an upwards fashion.
 *
 * The topmost Line gets the requested value, all Lines below it get
 * a respective value that is calculated from the relation of the old and
 * new size of the topmost Line in the lower line's own size.
 *
 * All changed Lines may get an own FrmFmt.
 * Of course we can only touch every Line once.
 */
inline void InsertLine( std::vector<SwTableLine*>& rLineArr, SwTableLine* pLine )
{
    if( rLineArr.end() == std::find( rLineArr.begin(), rLineArr.end(), pLine ) )
        rLineArr.push_back( pLine );
}


static bool lcl_IsAnLower( const SwTableLine *pLine, const SwTableLine *pAssumed )
{
    const SwTableLine *pTmp = pAssumed->GetUpper() ?
                                    pAssumed->GetUpper()->GetUpper() : 0;
    while ( pTmp )
    {
        if ( pTmp == pLine )
            return true;
        pTmp = pTmp->GetUpper() ? pTmp->GetUpper()->GetUpper() : 0;
    }
    return false;
}

struct LinesAndTable
{
    std::vector<SwTableLine*> &rLines;
    const SwTable             &rTable;
    bool                      bInsertLines;

    LinesAndTable( std::vector<SwTableLine*> &rL, const SwTable &rTbl ) :
          rLines( rL ), rTable( rTbl ), bInsertLines( true ) {}
};

bool _FindLine( _FndLine & rLine, LinesAndTable* pPara );

bool _FindBox( _FndBox & rBox, LinesAndTable* pPara )
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
    return true;
}

bool _FindLine( _FndLine& rLine, LinesAndTable* pPara )
{
    for (_FndBoxes::iterator it = rLine.GetBoxes().begin();
         it != rLine.GetBoxes().end(); ++it)
    {
        _FindBox(*it, pPara);
    }
    return true;
}

static void lcl_CollectLines( std::vector<SwTableLine*> &rArr, const SwCursor& rCursor, bool bRemoveLines )
{
    // Collect the selected Boxes first
    SwSelBoxes aBoxes;
    if( !::lcl_GetBoxSel( rCursor, aBoxes ))
        return ;

    // Copy the selected structure
    const SwTable &rTable = aBoxes[0]->GetSttNd()->FindTableNode()->GetTable();
    LinesAndTable aPara( rArr, rTable );
    _FndBox aFndBox( 0, 0 );
    {
        _FndPara aTmpPara( aBoxes, &aFndBox );
        ForEach_FndLineCopyCol( (SwTableLines&)rTable.GetTabLines(), &aTmpPara );
    }

    // Collect the Lines which only contain selected Boxes
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

static void lcl_ProcessRowAttr( std::vector<SwTblFmtCmp*>& rFmtCmp, SwTableLine* pLine, const SfxPoolItem& rNew )
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

static void lcl_ProcessBoxSize( std::vector<SwTblFmtCmp*> &rFmtCmp, SwTableBox *pBox, const SwFmtFrmSize &rNew );

static void lcl_ProcessRowSize( std::vector<SwTblFmtCmp*> &rFmtCmp, SwTableLine *pLine, const SwFmtFrmSize &rNew )
{
    lcl_ProcessRowAttr( rFmtCmp, pLine, rNew );
    SwTableBoxes &rBoxes = pLine->GetTabBoxes();
    for ( sal_uInt16 i = 0; i < rBoxes.size(); ++i )
        ::lcl_ProcessBoxSize( rFmtCmp, rBoxes[i], rNew );
}

static void lcl_ProcessBoxSize( std::vector<SwTblFmtCmp*> &rFmtCmp, SwTableBox *pBox, const SwFmtFrmSize &rNew )
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

void SwDoc::SetRowSplit( const SwCursor& rCursor, const SwFmtRowSplit &rNew )
{
    SwTableNode* pTblNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    if( pTblNd )
    {
        std::vector<SwTableLine*> aRowArr; // For Lines collecting
        ::lcl_CollectLines( aRowArr, rCursor, false );

        if( !aRowArr.empty() )
        {
            if (GetIDocumentUndoRedo().DoesUndo())
            {
                GetIDocumentUndoRedo().AppendUndo(new SwUndoAttrTbl(*pTblNd));
            }

            std::vector<SwTblFmtCmp*> aFmtCmp;
            aFmtCmp.reserve( std::max( 255, (int)aRowArr.size() ) );

            for( sal_uInt16 i = 0; i < aRowArr.size(); ++i )
                ::lcl_ProcessRowAttr( aFmtCmp, aRowArr[i], rNew );

            SwTblFmtCmp::Delete( aFmtCmp );
            SetModified();
        }
    }
}

void SwDoc::GetRowSplit( const SwCursor& rCursor, SwFmtRowSplit *& rpSz ) const
{
    rpSz = 0;

    SwTableNode* pTblNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    if( pTblNd )
    {
        std::vector<SwTableLine*> aRowArr; // For Lines collecting
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

void SwDoc::SetRowHeight( const SwCursor& rCursor, const SwFmtFrmSize &rNew )
{
    SwTableNode* pTblNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    if( pTblNd )
    {
        std::vector<SwTableLine*> aRowArr; // For Lines collecting
        ::lcl_CollectLines( aRowArr, rCursor, true );

        if( !aRowArr.empty() )
        {
            if (GetIDocumentUndoRedo().DoesUndo())
            {
                GetIDocumentUndoRedo().AppendUndo(new SwUndoAttrTbl(*pTblNd));
            }

            std::vector<SwTblFmtCmp*> aFmtCmp;
            aFmtCmp.reserve( std::max( 255, (int)aRowArr.size() ) );
            for ( sal_uInt16 i = 0; i < aRowArr.size(); ++i )
                ::lcl_ProcessRowSize( aFmtCmp, aRowArr[i], rNew );
            SwTblFmtCmp::Delete( aFmtCmp );

            SetModified();
        }
    }
}

void SwDoc::GetRowHeight( const SwCursor& rCursor, SwFmtFrmSize *& rpSz ) const
{
    rpSz = 0;

    SwTableNode* pTblNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    if( pTblNd )
    {
        std::vector<SwTableLine*> aRowArr; // For Lines collecting
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
        std::vector<SwTableLine*> aRowArr; // For Lines collecting
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
                        nHeight = std::max( nHeight, pFrm->Frm().Height() );
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
                aFmtCmp.reserve( std::max( 255, (int)aRowArr.size() ) );
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

void SwDoc::SetRowBackground( const SwCursor& rCursor, const SvxBrushItem &rNew )
{
    SwTableNode* pTblNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    if( pTblNd )
    {
        std::vector<SwTableLine*> aRowArr; // For Lines collecting
        ::lcl_CollectLines( aRowArr, rCursor, true );

        if( !aRowArr.empty() )
        {
            if (GetIDocumentUndoRedo().DoesUndo())
            {
                GetIDocumentUndoRedo().AppendUndo(new SwUndoAttrTbl(*pTblNd));
            }

            std::vector<SwTblFmtCmp*> aFmtCmp;
            aFmtCmp.reserve( std::max( 255, (int)aRowArr.size() ) );

            for( sal_uInt16 i = 0; i < aRowArr.size(); ++i )
                ::lcl_ProcessRowAttr( aFmtCmp, aRowArr[i], rNew );

            SwTblFmtCmp::Delete( aFmtCmp );
            SetModified();
        }
    }
}

sal_Bool SwDoc::GetRowBackground( const SwCursor& rCursor, SvxBrushItem &rToFill ) const
{
    sal_Bool bRet = sal_False;
    SwTableNode* pTblNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    if( pTblNd )
    {
        std::vector<SwTableLine*> aRowArr; // For Lines collecting
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

/**
 * Class: SwDoc
 * Methods: SetTabBorders(), GetTabBorders()
 */
inline void InsertCell( std::vector<SwCellFrm*>& rCellArr, SwCellFrm* pCellFrm )
{
    if( rCellArr.end() == std::find( rCellArr.begin(), rCellArr.end(), pCellFrm ) )
        rCellArr.push_back( pCellFrm );
}

static void lcl_CollectCells( std::vector<SwCellFrm*> &rArr, const SwRect &rUnion,
                          SwTabFrm *pTab )
{
    SwLayoutFrm *pCell = pTab->FirstCell();
    do
    {
        // If the Cell contains a CellFrame, we need to use it
        // in order to get to the Cell
        while ( !pCell->IsCellFrm() )
            pCell = pCell->GetUpper();
        OSL_ENSURE( pCell, "Frame is not a Cell" );
        if ( rUnion.IsOver( pCell->Frm() ) )
            ::InsertCell( rArr, (SwCellFrm*)pCell );

        // Make sure the Cell is left (Areas)
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

        // The Flags in the BoxInfo Item decide whether a BorderLine is valid!
        if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_BORDER_INNER, sal_False,
            (const SfxPoolItem**)&pSetBoxInfo) )
        {
            pHori = pSetBoxInfo->GetHori();
            pVert = pSetBoxInfo->GetVert();

            bHoriValid = pSetBoxInfo->IsValid(VALID_HORI);
            bVertValid = pSetBoxInfo->IsValid(VALID_VERT);

            // Do we want to evaluate these?
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
            // Not set, thus not valid values
            bTopValid = bBottomValid = bLeftValid = bRightValid = sal_False;
            pSetBox = 0;
        }

        bool bFirst = true;
        for ( sal_uInt16 i = 0; i < aUnions.size(); ++i )
        {
            SwSelUnion *pUnion = &aUnions[i];
            SwTabFrm *pTab = pUnion->GetTable();
            const SwRect &rUnion = pUnion->GetUnion();
            const bool bLast  = (i == aUnions.size() - 1);

            std::vector<SwCellFrm*> aCellArr;
            aCellArr.reserve( 255 );
            ::lcl_CollectCells( aCellArr, pUnion->GetUnion(), pTab );

            // All Cell Borders that match the UnionRect or extend it are
            // Outer Borders. All others are Inner Borders.
            //
            // New: The Outer Borders can, depending on whether it's a
            // Start/Middle/Follow Table (for Selection via FollowTabs),
            // also not be Outer Borders.
            // Outer Borders are set on the left, right, at the top and at the bottom.
            // Inner Borders are only set at the top and on the left.
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

                // Do not set anything by default in HeadlineRepeats
                if ( pTab->IsFollow() &&
                     ( pTab->IsInHeadline( *pCell ) ||
                       // Same holds for follow flow rows
                       pCell->IsInFollowFlowRow() ) )
                    continue;

                SvxBoxItem aBox( pCell->GetFmt()->GetBox() );

                sal_Int16 nType = 0;

                // Top Border
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

                // Fix fdo#62470 correct the input for RTL table
                if (bRTL)
                {
                        if( bLeftOver && bRightOver)
                        {
                            if ( bLeftValid )
                            {
                                aBox.SetLine( pLeft, BOX_LINE_RIGHT );
                                nType |= 0x0010;
                            }
                            if ( bRightValid )
                            {
                                aBox.SetLine( pRight, BOX_LINE_LEFT );
                                nType |= 0x0004;
                            }
                        }
                        else
                        {
                            if ( bLeftValid )
                            {
                                aBox.SetLine( bRightOver ? pLeft : 0, BOX_LINE_RIGHT );
                                bVertValid ? nType |= 0x0020 : nType |= 0x0010;
                            }
                            if ( bLeftOver )
                            {
                                if ( bRightValid )
                                {
                                    aBox.SetLine( pRight, BOX_LINE_LEFT );
                                    nType |= 0x0004;
                                }
                            }
                            else if ( bVertValid )
                            {
                                aBox.SetLine( pVert, BOX_LINE_LEFT );
                                nType |= 0x0008;
                            }
                        }
                }
                else
                {
                    // Left Border
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

                    // Right Border
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
                }

                // Bottom Border
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

            bFirst = false;
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

static void lcl_SetLineStyle( SvxBorderLine *pToSet,
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

                // Do not set anything by default in HeadlineRepeats
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
             bDistanceSet = sal_False,
             bRTLTab      = sal_False;

        aSetBoxInfo.ResetFlags();

        for ( sal_uInt16 i = 0; i < aUnions.size(); ++i )
        {
            SwSelUnion *pUnion = &aUnions[i];
            const SwTabFrm *pTab = pUnion->GetTable();
            const SwRect &rUnion = pUnion->GetUnion();
            const bool bFirst = i == 0;
            const bool bLast  = (i == aUnions.size() - 1);

            std::vector<SwCellFrm*> aCellArr;
            aCellArr.reserve(255);
            ::lcl_CollectCells( aCellArr, rUnion, (SwTabFrm*)pTab );

            for ( sal_uInt16 j = 0; j < aCellArr.size(); ++j )
            {
                SwCellFrm *pCell = aCellArr[j];
                const sal_Bool bVert = pTab->IsVertical();
                const sal_Bool bRTL = bRTLTab = pTab->IsRightToLeft();
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

                // Top Border
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
                                 ((!aSetBox.GetTop()) ^ (!rBox.GetTop()))) // XOR expression is sal_True, if one of the two pointers is 0
                        {
                            aSetBoxInfo.SetValid(VALID_TOP, sal_False );
                            aSetBox.SetLine( 0, BOX_LINE_TOP );
                        }
                    }
                }

                // Left Border
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

                // Right Border
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

                // Bottom Border
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
                // In all Lines, except for the last one, the horizontal Line
                // is taken from the Bottom Line.
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

                // Distance to text
                if (aSetBoxInfo.IsValid(VALID_DISTANCE))
                {
                    static sal_uInt16 const aBorders[] = {
                        BOX_LINE_BOTTOM, BOX_LINE_TOP,
                        BOX_LINE_RIGHT, BOX_LINE_LEFT };
                    const sal_uInt16* pBrd = aBorders;

                    if( !bDistanceSet ) // Set on first iteration
                    {
                        bDistanceSet = sal_True;
                        for( int k = 0; k < 4; ++k, ++pBrd )
                            aSetBox.SetDistance( rBox.GetDistance( *pBrd ), *pBrd );
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

        // fdo#62470 fix the reading for table format.
        if ( bRTLTab )
        {
            SvxBoxItem     aTempBox    ((const SvxBoxItem    &) rSet.Get(RES_BOX    ));
            SvxBoxInfoItem aTempBoxInfo((const SvxBoxInfoItem&) rSet.Get(SID_ATTR_BORDER_INNER));

            aTempBox.SetLine( aSetBox.GetRight(), BOX_LINE_RIGHT);
            aSetBox.SetLine( aSetBox.GetLeft(), BOX_LINE_RIGHT);
            aSetBox.SetLine( aTempBox.GetRight(), BOX_LINE_LEFT);

            aTempBoxInfo.SetValid( VALID_LEFT, aSetBoxInfo.IsValid(VALID_LEFT) );
            aSetBoxInfo.SetValid( VALID_LEFT, aSetBoxInfo.IsValid(VALID_RIGHT) );
            aSetBoxInfo.SetValid( VALID_RIGHT, aTempBoxInfo.IsValid(VALID_LEFT) );
        }

        rSet.Put( aSetBox );
        rSet.Put( aSetBoxInfo );
    }
}

void SwDoc::SetBoxAttr( const SwCursor& rCursor, const SfxPoolItem &rNew )
{
    SwTableNode* pTblNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    SwSelBoxes aBoxes;
    if( pTblNd && ::lcl_GetBoxSel( rCursor, aBoxes, true ) )
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

sal_Bool SwDoc::GetBoxAttr( const SwCursor& rCursor, SfxPoolItem& rToFill ) const
{
    sal_Bool bRet = sal_False;
    SwTableNode* pTblNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    SwSelBoxes aBoxes;
    if( pTblNd && lcl_GetBoxSel( rCursor, aBoxes ))
    {
        bRet = sal_True;
        bool bOneFound = false;
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
                        bOneFound = true;
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
                        bOneFound = true;
                    }
                    else if( rToFill != rDir )
                        bRet = sal_False;
                }
            }

            if ( sal_False == bRet )
                break;
        }
    }
    return bRet;
}

void SwDoc::SetBoxAlign( const SwCursor& rCursor, sal_uInt16 nAlign )
{
    OSL_ENSURE( nAlign == text::VertOrientation::NONE   ||
            nAlign == text::VertOrientation::CENTER ||
            nAlign == text::VertOrientation::BOTTOM, "Wrong alignment" );
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

/**
 * Class: SwDoc
 * Methods: AdjustCellWidth()
 */
static sal_uInt16 lcl_CalcCellFit( const SwLayoutFrm *pCell )
{
    SwTwips nRet = 0;
    const SwFrm *pFrm = pCell->Lower(); // The whole Line
    SWRECTFN( pCell )
    while ( pFrm )
    {
        const SwTwips nAdd = (pFrm->Frm().*fnRect->fnGetWidth)() -
                             (pFrm->Prt().*fnRect->fnGetWidth)();

        // pFrm does not necessarily have to be a SwTxtFrm!
        const SwTwips nCalcFitToContent = pFrm->IsTxtFrm() ?
                                          ((SwTxtFrm*)pFrm)->CalcFitToContent() :
                                          (pFrm->Prt().*fnRect->fnGetWidth)();

        nRet = std::max( nRet, nCalcFitToContent + nAdd );
        pFrm = pFrm->GetNext();
    }
    // Surrounding border as well as left and Right Border also need to be respected
    nRet += (pCell->Frm().*fnRect->fnGetWidth)() -
            (pCell->Prt().*fnRect->fnGetWidth)();

    // To compensate for the accuracy of calculation later on in SwTable::SetTabCols
    // we keep adding up a little.
    nRet += COLFUZZY;
    return (sal_uInt16)std::max( long(MINLAY), nRet );
}

/**
 * The Line is within the Selection but not outlined by the TabCols.
 *
 * That means that the Line has been "split" by other Cells due to the
 * two-dimensional representation used. Thus, we have to distribute the cell's
 * default or minium value amongst the Cell it has been split by.
 *
 * First, we collect the Columns (not the Column separators) which overlap
 * with the Cell. We then distribute the desired value according to the
 * amount of overlapping amongst the Cells.
 *
 * A Cell's default value stays the same if it already has a larger value than
 * the desired one. It's overwritten if it's smaller.
 */
static void lcl_CalcSubColValues( std::vector<sal_uInt16> &rToFill, const SwTabCols &rCols,
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

        // Adapt values to the proportions of the Table (Follows)
        if ( rCols.GetLeftMin() !=  sal_uInt16((pTab->Frm().*fnRect->fnGetLeft)()) )
        {
            const long nDiff = (pTab->Frm().*fnRect->fnGetLeft)() - rCols.GetLeftMin();
            nColLeft  += nDiff;
            nColRight += nDiff;
        }
        const long nCellLeft  = (pCell->Frm().*fnRect->fnGetLeft)();
        const long nCellRight = (pCell->Frm().*fnRect->fnGetRight)();

        // Calculate overlapping value
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

/**
 * Retrievs new values to set the TabCols.
 *
 * We do not iterate over the TabCols' entries, but over the gaps that describe Cells.
 *
 * @param bWishValues == sal_True: We calculate the desired value of all affected
 *                                 Cells for the current Selection/current Cell.
 *                                 If more Cells are within a Column, the highest
 *                                 desired value is returned.
 *                                 We set TabCol entries for which we did not calculate
 *                                 Cells to 0.
 *
 * @param bWishValues == sal_False: The Selection is expanded vertically.
 *                                  We calculate the minimum value for every
 *                                  Column in the TabCols that intersects with the
 *                                  Selection.
 */
static void lcl_CalcColValues( std::vector<sal_uInt16> &rToFill, const SwTabCols &rCols,
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

                bool bNotInCols = true;

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

                    // Adapt values to the proportions of the Table (Follows)
                    long nLeftA  = nColLeft;
                    long nRightA = nColRight;
                    if ( rCols.GetLeftMin() !=  sal_uInt16((pTab->Frm().*fnRect->fnGetLeft)()) )
                    {
                        const long nDiff = (pTab->Frm().*fnRect->fnGetLeft)() - rCols.GetLeftMin();
                        nLeftA  += nDiff;
                        nRightA += nDiff;
                    }

                    // We don't want to take a too close look
                    if ( ::IsSame(nCLeft, nLeftA) && ::IsSame(nCRight, nRightA))
                    {
                        bNotInCols = false;
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
            } while( pCell && pCell->Frm().Width() == 0 );
        } while ( pCell && pTab->IsAnLower( pCell ) );
    }
}

void SwDoc::AdjustCellWidth( const SwCursor& rCursor, sal_Bool bBalance )
{
    // Check whether the current Crsr has it's Point/Mark in a Table
    SwCntntNode* pCntNd = rCursor.GetPoint()->nNode.GetNode().GetCntntNode();
    SwTableNode* pTblNd = pCntNd ? pCntNd->FindTableNode() : 0;
    if( !pTblNd )
        return ;

    SwLayoutFrm *pStart, *pEnd;
    ::lcl_GetStartEndCell( rCursor, pStart, pEnd );

    // Collect TabCols; we reset the Table with them
    SwFrm* pBoxFrm = pStart;
    while( pBoxFrm && !pBoxFrm->IsCellFrm() )
        pBoxFrm = pBoxFrm->GetUpper();

    if ( !pBoxFrm )
        return; // Robust

    SwTabCols aTabCols;
    GetTabCols( aTabCols, 0, (SwCellFrm*)pBoxFrm );

    if ( ! aTabCols.Count() )
        return;

    std::vector<sal_uInt16> aWish(aTabCols.Count() + 1);
    std::vector<sal_uInt16> aMins(aTabCols.Count() + 1);

    sal_uInt16 i;

    ::lcl_CalcColValues( aWish, aTabCols, pStart, pEnd, sal_True  );

    // It's more robust if we calculate the minimum values for the whole Table
    const SwTabFrm *pTab = pStart->ImplFindTabFrm();
    pStart = (SwLayoutFrm*)pTab->FirstCell();
    pEnd   = (SwLayoutFrm*)pTab->FindLastCntnt()->GetUpper();
    while( !pEnd->IsCellFrm() )
        pEnd = pEnd->GetUpper();
    ::lcl_CalcColValues( aMins, aTabCols, pStart, pEnd, sal_False );

    if( bBalance )
    {
        // All Columns, which are now selected, have a desired value.
        // We add up the current values, divide the result by their
        // count and get a desired value for balancing.
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

    // In order to make the implementation easier, but still use the available
    // space properly, we do this twice.
    //
    // The problem: The first column is getting wider, the others get slimmer
    // only afterwards.
    // The first column's desired width would be discarded as it would cause
    // the Table's width to exceed the maximum width.
    for ( sal_uInt16 k = 0; k < 2; ++k )
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

                // If the Table would become too wide, we restrict the
                // adjusted amount to the allowed maximum.
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

    // We can leave the "real" work to the SwTable now
    SetTabCols( aTabCols, sal_False, 0, (SwCellFrm*)pBoxFrm );

    // Alignment might have been changed in SetTabCols; restore old value
    const SwFmtHoriOrient &rHori = pFmt->GetHoriOrient();
    SwFmtHoriOrient aHori( rHori );
    if ( aHori.GetHoriOrient() != nOriHori )
    {
        aHori.SetHoriOrient( nOriHori );
        pFmt->SetFmtAttr( aHori );
    }

    // We switch to left-adjusted for automatic width
    // We adjust the right border for Border attributes
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
