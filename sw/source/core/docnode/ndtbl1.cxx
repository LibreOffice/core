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

#include <hintids.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <fesh.hxx>
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
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <pam.hxx>
#include <swcrsr.hxx>
#include <viscrs.hxx>
#include <swtable.hxx>
#include <htmltbl.hxx>
#include <tblsel.hxx>
#include <swtblfmt.hxx>
#include <docary.hxx>
#include <ndindex.hxx>
#include <undobj.hxx>
#include <calbck.hxx>
#include <UndoTable.hxx>
#include <o3tl/enumrange.hxx>
#include <o3tl/make_unique.hxx>

using ::editeng::SvxBorderLine;
using namespace ::com::sun::star;

// See swtable.cxx too
#define COLFUZZY 20L

inline bool IsSame( long nA, long nB ) { return  std::abs(nA-nB) <= COLFUZZY; }

// SwTableLine::ChgFrameFormat may delete old format which doesn't have writer listeners anymore.
// This may invalidate my pointers, and lead to use-after-free. For this reason, I register myself
// as a writer listener for the old format here, and take care to delete formats without listeners
// in my own dtor.
class SwTableFormatCmp : public SwClient
{
public:
    SwTableFormatCmp( SwFrameFormat *pOld, SwFrameFormat *pNew, sal_Int16 nType );
    ~SwTableFormatCmp() override;

    static SwFrameFormat* FindNewFormat(std::vector<std::unique_ptr<SwTableFormatCmp>>& rArr,
                                        SwFrameFormat const* pOld, sal_Int16 nType);

private:
    SwFrameFormat *pOld, *pNew;
    sal_Int16 nType;
};

SwTableFormatCmp::SwTableFormatCmp( SwFrameFormat *pO, SwFrameFormat *pN, sal_Int16 nT )
    : pOld ( pO ), pNew ( pN ), nType( nT )
{
    if (pOld)
        pOld->Add(this);
}

SwTableFormatCmp::~SwTableFormatCmp()
{
    if (pOld)
    {
        pOld->Remove(this);
        if (!pOld->HasWriterListeners())
            delete pOld;
    }
}

// static
SwFrameFormat* SwTableFormatCmp::FindNewFormat(std::vector<std::unique_ptr<SwTableFormatCmp>>& rArr,
                                               SwFrameFormat const* pOld, sal_Int16 nType)
{
    for (auto& pCmp : rArr)
    {
        if ( pCmp->pOld == pOld && pCmp->nType == nType )
            return pCmp->pNew;
    }
    return nullptr;
}

static void lcl_GetStartEndCell( const SwCursor& rCursor,
                        SwLayoutFrame *&prStart, SwLayoutFrame *&prEnd )
{
    OSL_ENSURE( rCursor.GetContentNode() && rCursor.GetContentNode( false ),
            "Tab selection not at ContentNode" );

    Point aPtPos, aMkPos;
    const SwShellCursor* pShCursor = dynamic_cast<const SwShellCursor*>(&rCursor);
    if( pShCursor )
    {
        aPtPos = pShCursor->GetPtPos();
        aMkPos = pShCursor->GetMkPos();
    }

    // Robust:
    SwContentNode* pPointNd = rCursor.GetContentNode();
    SwContentNode* pMarkNd  = rCursor.GetContentNode(false);

    SwFrame* pPointFrame = pPointNd ? pPointNd->getLayoutFrame( pPointNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &aPtPos ) : nullptr;
    SwFrame* pMarkFrame  = pMarkNd  ? pMarkNd->getLayoutFrame( pMarkNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &aMkPos )  : nullptr;

    prStart = pPointFrame ? pPointFrame->GetUpper() : nullptr;
    prEnd   = pMarkFrame  ? pMarkFrame->GetUpper() : nullptr;
}

static bool lcl_GetBoxSel( const SwCursor& rCursor, SwSelBoxes& rBoxes,
                    bool bAllCursor = false )
{
    const SwTableCursor* pTableCursor =
        dynamic_cast<const SwTableCursor*>(&rCursor);
    if( pTableCursor )
        ::GetTableSelCrs( *pTableCursor, rBoxes );
    else
    {
        const SwPaM *pCurPam = &rCursor, *pSttPam = pCurPam;
        do {
            const SwNode* pNd = pCurPam->GetNode().FindTableBoxStartNode();
            if( pNd )
            {
                SwTableBox* pBox = const_cast<SwTableBox*>(pNd->FindTableNode()->GetTable().
                                            GetTableBox( pNd->GetIndex() ));
                rBoxes.insert( pBox );
            }
        } while( bAllCursor &&
                pSttPam != ( pCurPam = pCurPam->GetNext()) );
    }
    return !rBoxes.empty();
}

inline void InsertLine( std::vector<SwTableLine*>& rLineArr, SwTableLine* pLine )
{
    if( rLineArr.end() == std::find( rLineArr.begin(), rLineArr.end(), pLine ) )
        rLineArr.push_back( pLine );
}

static bool lcl_IsAnLower( const SwTableLine *pLine, const SwTableLine *pAssumed )
{
    const SwTableLine *pTmp = pAssumed->GetUpper() ?
                                    pAssumed->GetUpper()->GetUpper() : nullptr;
    while ( pTmp )
    {
        if ( pTmp == pLine )
            return true;
        pTmp = pTmp->GetUpper() ? pTmp->GetUpper()->GetUpper() : nullptr;
    }
    return false;
}

struct LinesAndTable
{
    std::vector<SwTableLine*> &m_rLines;
    const SwTable             &m_rTable;
    bool                      m_bInsertLines;

    LinesAndTable(std::vector<SwTableLine*> &rL, const SwTable &rTable) :
          m_rLines(rL), m_rTable(rTable), m_bInsertLines(true) {}
};

bool FindLine_( FndLine_ & rLine, LinesAndTable* pPara );

bool FindBox_( FndBox_ & rBox, LinesAndTable* pPara )
{
    if (!rBox.GetLines().empty())
    {
        pPara->m_bInsertLines = true;
        for (auto const& rpFndLine : rBox.GetLines())
        {
            FindLine_(*rpFndLine, pPara);
        }

        if (pPara->m_bInsertLines)
        {
            const SwTableLines &rLines = (rBox.GetBox())
                                    ? rBox.GetBox()->GetTabLines()
                                    : pPara->m_rTable.GetTabLines();
            if (rBox.GetLines().size() == rLines.size())
            {
                for ( auto pLine : rLines )
                    ::InsertLine(pPara->m_rLines, pLine);
            }
            else
                pPara->m_bInsertLines = false;
        }
    }
    else if (rBox.GetBox())
    {
        ::InsertLine(pPara->m_rLines, rBox.GetBox()->GetUpper());
    }
    return true;
}

bool FindLine_( FndLine_& rLine, LinesAndTable* pPara )
{
    for (auto const& it : rLine.GetBoxes())
    {
        FindBox_(*it, pPara);
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
    FndBox_ aFndBox( nullptr, nullptr );
    {
        FndPara aTmpPara( aBoxes, &aFndBox );
        ForEach_FndLineCopyCol( const_cast<SwTableLines&>(rTable.GetTabLines()), &aTmpPara );
    }

    // Collect the Lines which only contain selected Boxes
    ::FindBox_(aFndBox, &aPara);

    // Remove lines, that have a common superordinate row.
    // (Not for row split)
    if ( bRemoveLines )
    {
        for ( std::vector<SwTableLine*>::size_type i = 0; i < rArr.size(); ++i )
        {
            SwTableLine *pUpLine = rArr[i];
            for ( std::vector<SwTableLine*>::size_type k = 0; k < rArr.size(); ++k )
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

static void lcl_ProcessRowAttr(std::vector<std::unique_ptr<SwTableFormatCmp>>& rFormatCmp,
                               SwTableLine* pLine, const SfxPoolItem& rNew)
{
    SwFrameFormat *pNewFormat;
    if ( nullptr != (pNewFormat = SwTableFormatCmp::FindNewFormat( rFormatCmp, pLine->GetFrameFormat(), 0 )))
        pLine->ChgFrameFormat( static_cast<SwTableLineFormat*>(pNewFormat) );
    else
    {
        SwFrameFormat *pOld = pLine->GetFrameFormat();
        SwFrameFormat *pNew = pLine->ClaimFrameFormat();
        pNew->SetFormatAttr( rNew );
        rFormatCmp.push_back(o3tl::make_unique<SwTableFormatCmp>(pOld, pNew, 0));
    }
}

static void lcl_ProcessBoxSize(std::vector<std::unique_ptr<SwTableFormatCmp>>& rFormatCmp,
                               SwTableBox* pBox, const SwFormatFrameSize& rNew);

static void lcl_ProcessRowSize(std::vector<std::unique_ptr<SwTableFormatCmp>>& rFormatCmp,
                               SwTableLine* pLine, const SwFormatFrameSize& rNew)
{
    lcl_ProcessRowAttr( rFormatCmp, pLine, rNew );
    SwTableBoxes &rBoxes = pLine->GetTabBoxes();
    for ( auto & pBox : rBoxes )
        ::lcl_ProcessBoxSize( rFormatCmp, pBox.get(), rNew );
}

static void lcl_ProcessBoxSize(std::vector<std::unique_ptr<SwTableFormatCmp>>& rFormatCmp,
                               SwTableBox* pBox, const SwFormatFrameSize& rNew)
{
    SwTableLines &rLines = pBox->GetTabLines();
    if ( !rLines.empty() )
    {
        SwFormatFrameSize aSz( rNew );
        aSz.SetHeight( rNew.GetHeight() ? rNew.GetHeight() / rLines.size() : 0 );
        for ( auto pLine : rLines )
            ::lcl_ProcessRowSize( rFormatCmp, pLine, aSz );
    }
}

void SwDoc::SetRowSplit( const SwCursor& rCursor, const SwFormatRowSplit &rNew )
{
    SwTableNode* pTableNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    if( pTableNd )
    {
        std::vector<SwTableLine*> aRowArr; // For Lines collecting
        ::lcl_CollectLines( aRowArr, rCursor, false );

        if( !aRowArr.empty() )
        {
            if (GetIDocumentUndoRedo().DoesUndo())
            {
                GetIDocumentUndoRedo().AppendUndo(new SwUndoAttrTable(*pTableNd));
            }

            std::vector<std::unique_ptr<SwTableFormatCmp>> aFormatCmp;
            aFormatCmp.reserve( std::max( 255, static_cast<int>(aRowArr.size()) ) );

            for( auto pLn : aRowArr )
                ::lcl_ProcessRowAttr( aFormatCmp, pLn, rNew );

            getIDocumentState().SetModified();
        }
    }
}

void SwDoc::GetRowSplit( const SwCursor& rCursor, SwFormatRowSplit *& rpSz )
{
    rpSz = nullptr;

    SwTableNode* pTableNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    if( pTableNd )
    {
        std::vector<SwTableLine*> aRowArr; // For Lines collecting
        ::lcl_CollectLines( aRowArr, rCursor, false );

        if( !aRowArr.empty() )
        {
            rpSz = &const_cast<SwFormatRowSplit&>(aRowArr[0]->GetFrameFormat()->GetRowSplit());

            if (rpSz)
            {
                for ( auto pLn : aRowArr )
                {
                    if ( (*rpSz).GetValue() != pLn->GetFrameFormat()->GetRowSplit().GetValue() )
                    {
                        rpSz = nullptr;
                        break;
                    }
                }
            }
            if ( rpSz )
                rpSz = new SwFormatRowSplit( *rpSz );
        }
    }
}

/* Class:  SwDoc
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
 * All changed Lines may get an own FrameFormat.
 * Of course we can only touch every Line once.
 */

void SwDoc::SetRowHeight( const SwCursor& rCursor, const SwFormatFrameSize &rNew )
{
    SwTableNode* pTableNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    if( pTableNd )
    {
        std::vector<SwTableLine*> aRowArr; // For Lines collecting
        ::lcl_CollectLines( aRowArr, rCursor, true );

        if( !aRowArr.empty() )
        {
            if (GetIDocumentUndoRedo().DoesUndo())
            {
                GetIDocumentUndoRedo().AppendUndo(new SwUndoAttrTable(*pTableNd));
            }

            std::vector<std::unique_ptr<SwTableFormatCmp>> aFormatCmp;
            aFormatCmp.reserve( std::max( 255, static_cast<int>(aRowArr.size()) ) );
            for ( auto pLn : aRowArr )
                ::lcl_ProcessRowSize( aFormatCmp, pLn, rNew );

            getIDocumentState().SetModified();
        }
    }
}

void SwDoc::GetRowHeight( const SwCursor& rCursor, SwFormatFrameSize *& rpSz )
{
    rpSz = nullptr;

    SwTableNode* pTableNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    if( pTableNd )
    {
        std::vector<SwTableLine*> aRowArr; // For Lines collecting
        ::lcl_CollectLines( aRowArr, rCursor, true );

        if( !aRowArr.empty() )
        {
            rpSz = &const_cast<SwFormatFrameSize&>(aRowArr[0]->GetFrameFormat()->GetFrameSize());

            if (rpSz)
            {
                for ( auto pLn : aRowArr )
                {
                    if ( *rpSz != pLn->GetFrameFormat()->GetFrameSize() )
                    {
                        rpSz = nullptr;
                        break;
                    }
                }
            }
            if ( rpSz )
                rpSz = new SwFormatFrameSize( *rpSz );
        }
    }
}

bool SwDoc::BalanceRowHeight( const SwCursor& rCursor, bool bTstOnly )
{
    bool bRet = false;
    SwTableNode* pTableNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    if( pTableNd )
    {
        std::vector<SwTableLine*> aRowArr; // For Lines collecting
        ::lcl_CollectLines( aRowArr, rCursor, true );

        if( 1 < aRowArr.size() )
        {
            if( !bTstOnly )
            {
                long nHeight = 0;

                for ( auto pLn : aRowArr )
                {
                    SwIterator<SwFrame,SwFormat> aIter( *pLn->GetFrameFormat() );
                    SwFrame* pFrame = aIter.First();
                    while ( pFrame )
                    {
                        nHeight = std::max( nHeight, pFrame->getFrameArea().Height() );
                        pFrame = aIter.Next();
                    }
                }
                SwFormatFrameSize aNew( ATT_MIN_SIZE, 0, nHeight );

                if (GetIDocumentUndoRedo().DoesUndo())
                {
                    GetIDocumentUndoRedo().AppendUndo(
                            new SwUndoAttrTable(*pTableNd));
                }

                std::vector<std::unique_ptr<SwTableFormatCmp>> aFormatCmp;
                aFormatCmp.reserve( std::max( 255, static_cast<int>(aRowArr.size()) ) );
                for( auto pLn : aRowArr )
                    ::lcl_ProcessRowSize( aFormatCmp, pLn, aNew );

                getIDocumentState().SetModified();
            }
            bRet = true;
        }
    }
    return bRet;
}

void SwDoc::SetRowBackground( const SwCursor& rCursor, const SvxBrushItem &rNew )
{
    SwTableNode* pTableNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    if( pTableNd )
    {
        std::vector<SwTableLine*> aRowArr; // For Lines collecting
        ::lcl_CollectLines( aRowArr, rCursor, true );

        if( !aRowArr.empty() )
        {
            if (GetIDocumentUndoRedo().DoesUndo())
            {
                GetIDocumentUndoRedo().AppendUndo(new SwUndoAttrTable(*pTableNd));
            }

            std::vector<std::unique_ptr<SwTableFormatCmp>> aFormatCmp;
            aFormatCmp.reserve( std::max( 255, static_cast<int>(aRowArr.size()) ) );

            for( auto pLn : aRowArr )
                ::lcl_ProcessRowAttr( aFormatCmp, pLn, rNew );

            getIDocumentState().SetModified();
        }
    }
}

bool SwDoc::GetRowBackground( const SwCursor& rCursor, SvxBrushItem &rToFill )
{
    bool bRet = false;
    SwTableNode* pTableNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    if( pTableNd )
    {
        std::vector<SwTableLine*> aRowArr; // For Lines collecting
        ::lcl_CollectLines( aRowArr, rCursor, true );

        if( !aRowArr.empty() )
        {
            rToFill = aRowArr[0]->GetFrameFormat()->makeBackgroundBrushItem();

            bRet = true;
            for ( std::vector<SwTableLine*>::size_type i = 1; i < aRowArr.size(); ++i )
                if ( rToFill != aRowArr[i]->GetFrameFormat()->makeBackgroundBrushItem() )
                {
                    bRet = false;
                    break;
                }
        }
    }
    return bRet;
}

inline void InsertCell( std::vector<SwCellFrame*>& rCellArr, SwCellFrame* pCellFrame )
{
    if( rCellArr.end() == std::find( rCellArr.begin(), rCellArr.end(), pCellFrame ) )
        rCellArr.push_back( pCellFrame );
}

static void lcl_CollectCells( std::vector<SwCellFrame*> &rArr, const SwRect &rUnion,
                          SwTabFrame *pTab )
{
    SwLayoutFrame *pCell = pTab->FirstCell();
    do
    {
        // If the Cell contains a CellFrame, we need to use it
        // in order to get to the Cell
        while ( !pCell->IsCellFrame() )
            pCell = pCell->GetUpper();
        OSL_ENSURE( pCell, "Frame is not a Cell" );
        if ( rUnion.IsOver( pCell->getFrameArea() ) )
            ::InsertCell( rArr, static_cast<SwCellFrame*>(pCell) );

        // Make sure the Cell is left (Areas)
        SwLayoutFrame *pTmp = pCell;
        do
        {   pTmp = pTmp->GetNextLayoutLeaf();
        } while ( pCell->IsAnLower( pTmp ) );
        pCell = pTmp;
    } while( pCell && pTab->IsAnLower( pCell ) );
}

void SwDoc::SetTabBorders( const SwCursor& rCursor, const SfxItemSet& rSet )
{
    SwContentNode* pCntNd = rCursor.GetPoint()->nNode.GetNode().GetContentNode();
    SwTableNode* pTableNd = pCntNd ? pCntNd->FindTableNode() : nullptr;
    if( !pTableNd )
        return ;

    SwLayoutFrame *pStart, *pEnd;
    ::lcl_GetStartEndCell( rCursor, pStart, pEnd );

    SwSelUnions aUnions;
    ::MakeSelUnions( aUnions, pStart, pEnd );

    if( aUnions.empty() )
        return;

    SwTable& rTable = pTableNd->GetTable();
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo( new SwUndoAttrTable(*pTableNd) );
    }

    std::vector<std::unique_ptr<SwTableFormatCmp>> aFormatCmp;
    aFormatCmp.reserve( 255 );
    const SvxBoxItem* pSetBox;
    const SvxBoxInfoItem *pSetBoxInfo;

    const SvxBorderLine* pLeft = nullptr;
    const SvxBorderLine* pRight = nullptr;
    const SvxBorderLine* pTop = nullptr;
    const SvxBorderLine* pBottom = nullptr;
    const SvxBorderLine* pHori = nullptr;
    const SvxBorderLine* pVert = nullptr;
    bool bHoriValid = true, bVertValid = true,
         bTopValid = true, bBottomValid = true,
         bLeftValid = true, bRightValid = true;

    // The Flags in the BoxInfo Item decide whether a BorderLine is valid!
    if( SfxItemState::SET == rSet.GetItemState( SID_ATTR_BORDER_INNER, false,
        reinterpret_cast<const SfxPoolItem**>(&pSetBoxInfo)) )
    {
        pHori = pSetBoxInfo->GetHori();
        pVert = pSetBoxInfo->GetVert();

        bHoriValid = pSetBoxInfo->IsValid(SvxBoxInfoItemValidFlags::HORI);
        bVertValid = pSetBoxInfo->IsValid(SvxBoxInfoItemValidFlags::VERT);

        // Do we want to evaluate these?
        bTopValid = pSetBoxInfo->IsValid(SvxBoxInfoItemValidFlags::TOP);
        bBottomValid = pSetBoxInfo->IsValid(SvxBoxInfoItemValidFlags::BOTTOM);
        bLeftValid = pSetBoxInfo->IsValid(SvxBoxInfoItemValidFlags::LEFT);
        bRightValid = pSetBoxInfo->IsValid(SvxBoxInfoItemValidFlags::RIGHT);
    }

    if( SfxItemState::SET == rSet.GetItemState( RES_BOX, false,
        reinterpret_cast<const SfxPoolItem**>(&pSetBox)) )
    {
        pLeft = pSetBox->GetLeft();
        pRight = pSetBox->GetRight();
        pTop = pSetBox->GetTop();
        pBottom = pSetBox->GetBottom();
    }
    else
    {
        // Not set, thus not valid values
        bTopValid = bBottomValid = bLeftValid = bRightValid = false;
        pSetBox = nullptr;
    }

    bool bFirst = true;
    for ( SwSelUnions::size_type i = 0; i < aUnions.size(); ++i )
    {
        SwSelUnion *pUnion = &aUnions[i];
        SwTabFrame *pTab = pUnion->GetTable();
        const SwRect &rUnion = pUnion->GetUnion();
        const bool bLast  = (i == aUnions.size() - 1);

        std::vector<SwCellFrame*> aCellArr;
        aCellArr.reserve( 255 );
        ::lcl_CollectCells( aCellArr, pUnion->GetUnion(), pTab );

        // All Cell Borders that match the UnionRect or extend it are
        // Outer Borders. All others are Inner Borders.

        // New: The Outer Borders can, depending on whether it's a
        // Start/Middle/Follow Table (for Selection via FollowTabs),
        // also not be Outer Borders.
        // Outer Borders are set on the left, right, at the top and at the bottom.
        // Inner Borders are only set at the top and on the left.
        for ( auto pCell : aCellArr )
        {
            const bool bVert = pTab->IsVertical();
            const bool bRTL = pTab->IsRightToLeft();
            bool bTopOver, bLeftOver, bRightOver, bBottomOver;
            if ( bVert )
            {
                bTopOver = pCell->getFrameArea().Right() >= rUnion.Right();
                bLeftOver = pCell->getFrameArea().Top() <= rUnion.Top();
                bRightOver = pCell->getFrameArea().Bottom() >= rUnion.Bottom();
                bBottomOver = pCell->getFrameArea().Left() <= rUnion.Left();
            }
            else
            {
                bTopOver = pCell->getFrameArea().Top() <= rUnion.Top();
                bLeftOver = pCell->getFrameArea().Left() <= rUnion.Left();
                bRightOver = pCell->getFrameArea().Right() >= rUnion.Right();
                bBottomOver = pCell->getFrameArea().Bottom() >= rUnion.Bottom();
            }

            if ( bRTL )
            {
                bool bTmp = bRightOver;
                bRightOver = bLeftOver;
                bLeftOver = bTmp;
            }

            // Do not set anything by default in HeadlineRepeats
            if ( pTab->IsFollow() &&
                 ( pTab->IsInHeadline( *pCell ) ||
                   // Same holds for follow flow rows
                   pCell->IsInFollowFlowRow() ) )
                continue;

            SvxBoxItem aBox( pCell->GetFormat()->GetBox() );

            sal_Int16 nType = 0;

            // Top Border
            if( bTopValid )
            {
                if ( bFirst && bTopOver )
                {
                    aBox.SetLine( pTop, SvxBoxItemLine::TOP );
                    nType |= 0x0001;
                }
                else if ( bHoriValid )
                {
                    aBox.SetLine( nullptr, SvxBoxItemLine::TOP );
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
                            aBox.SetLine( pLeft, SvxBoxItemLine::RIGHT );
                            nType |= 0x0010;
                        }
                        if ( bRightValid )
                        {
                            aBox.SetLine( pRight, SvxBoxItemLine::LEFT );
                            nType |= 0x0004;
                        }
                    }
                    else
                    {
                        if ( bLeftValid )
                        {
                            aBox.SetLine( bRightOver ? pLeft : nullptr, SvxBoxItemLine::RIGHT );
                            bVertValid ? nType |= 0x0020 : nType |= 0x0010;
                        }
                        if ( bLeftOver )
                        {
                            if ( bRightValid )
                            {
                                aBox.SetLine( pRight, SvxBoxItemLine::LEFT );
                                nType |= 0x0004;
                            }
                        }
                        else if ( bVertValid )
                        {
                            aBox.SetLine( pVert, SvxBoxItemLine::LEFT );
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
                        aBox.SetLine( pLeft, SvxBoxItemLine::LEFT );
                        nType |= 0x0004;
                    }
                }
                else if( bVertValid )
                {
                    aBox.SetLine( pVert, SvxBoxItemLine::LEFT );
                    nType |= 0x0008;
                }

                // Right Border
                if( bRightValid )
                {
                    if ( bRightOver )
                    {
                        aBox.SetLine( pRight, SvxBoxItemLine::RIGHT );
                        nType |= 0x0010;
                    }
                    else if ( bVertValid )
                    {
                        aBox.SetLine( nullptr, SvxBoxItemLine::RIGHT );
                        nType |= 0x0020;
                    }
                }
            }

            // Bottom Border
            if ( bLast && bBottomOver )
            {
                if( bBottomValid )
                {
                    aBox.SetLine( pBottom, SvxBoxItemLine::BOTTOM );
                    nType |= 0x0040;
                }
            }
            else if( bHoriValid )
            {
                aBox.SetLine( pHori, SvxBoxItemLine::BOTTOM );
                nType |= 0x0080;
            }

            if( pSetBox )
            {
               for( SvxBoxItemLine k : o3tl::enumrange<SvxBoxItemLine>() )
                    aBox.SetDistance( pSetBox->GetDistance( k ), k );
            }

            SwTableBox *pBox = const_cast<SwTableBox*>(pCell->GetTabBox());
            SwFrameFormat *pNewFormat;
            if ( nullptr != (pNewFormat = SwTableFormatCmp::FindNewFormat( aFormatCmp, pBox->GetFrameFormat(), nType )))
                pBox->ChgFrameFormat( static_cast<SwTableBoxFormat*>(pNewFormat) );
            else
            {
                SwFrameFormat *pOld = pBox->GetFrameFormat();
                SwFrameFormat *pNew = pBox->ClaimFrameFormat();
                pNew->SetFormatAttr( aBox );
                aFormatCmp.push_back(o3tl::make_unique<SwTableFormatCmp>(pOld, pNew, nType));
            }
        }

        bFirst = false;
    }

    SwHTMLTableLayout *pTableLayout = rTable.GetHTMLTableLayout();
    if( pTableLayout )
    {
        SwContentFrame* pFrame = rCursor.GetContentNode()->getLayoutFrame( rCursor.GetContentNode()->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout() );
        SwTabFrame* pTabFrame = pFrame->ImplFindTabFrame();

        pTableLayout->BordersChanged(
            pTableLayout->GetBrowseWidthByTabFrame( *pTabFrame ) );
    }
    ::ClearFEShellTabCols(*this, nullptr);
    getIDocumentState().SetModified();
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
                             const Color* pColor, bool bSetLine,
                             const SvxBorderLine* pBorderLine )
{
    SwContentNode* pCntNd = rCursor.GetPoint()->nNode.GetNode().GetContentNode();
    SwTableNode* pTableNd = pCntNd ? pCntNd->FindTableNode() : nullptr;
    if( !pTableNd )
        return ;

    SwLayoutFrame *pStart, *pEnd;
    ::lcl_GetStartEndCell( rCursor, pStart, pEnd );

    SwSelUnions aUnions;
    ::MakeSelUnions( aUnions, pStart, pEnd );

    if( !aUnions.empty() )
    {
        SwTable& rTable = pTableNd->GetTable();
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            GetIDocumentUndoRedo().AppendUndo(new SwUndoAttrTable(*pTableNd));
        }

        for( auto &rU : aUnions )
        {
            SwSelUnion *pUnion = &rU;
            SwTabFrame *pTab = pUnion->GetTable();
            std::vector<SwCellFrame*> aCellArr;
            aCellArr.reserve( 255 );
            ::lcl_CollectCells( aCellArr, pUnion->GetUnion(), pTab );

            for ( auto pCell : aCellArr )
            {
                // Do not set anything by default in HeadlineRepeats
                if ( pTab->IsFollow() && pTab->IsInHeadline( *pCell ) )
                    continue;

                const_cast<SwTableBox*>(pCell->GetTabBox())->ClaimFrameFormat();
                SwFrameFormat *pFormat = pCell->GetFormat();
                SvxBoxItem aBox( pFormat->GetBox() );

                if ( !pBorderLine && bSetLine )
                    aBox = *::GetDfltAttr( RES_BOX );
                else
                {
                    if ( aBox.GetTop() )
                        ::lcl_SetLineStyle( const_cast<SvxBorderLine*>(aBox.GetTop()),
                                        pColor, pBorderLine );
                    if ( aBox.GetBottom() )
                        ::lcl_SetLineStyle( const_cast<SvxBorderLine*>(aBox.GetBottom()),
                                        pColor, pBorderLine );
                    if ( aBox.GetLeft() )
                        ::lcl_SetLineStyle( const_cast<SvxBorderLine*>(aBox.GetLeft()),
                                        pColor, pBorderLine );
                    if ( aBox.GetRight() )
                        ::lcl_SetLineStyle( const_cast<SvxBorderLine*>(aBox.GetRight()),
                                        pColor, pBorderLine );
                }
                pFormat->SetFormatAttr( aBox );
            }
        }

        SwHTMLTableLayout *pTableLayout = rTable.GetHTMLTableLayout();
        if( pTableLayout )
        {
            SwContentFrame* pFrame = rCursor.GetContentNode()->getLayoutFrame( rCursor.GetContentNode()->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout() );
            SwTabFrame* pTabFrame = pFrame->ImplFindTabFrame();

            pTableLayout->BordersChanged(
                pTableLayout->GetBrowseWidthByTabFrame( *pTabFrame ) );
        }
        ::ClearFEShellTabCols(*this, nullptr);
        getIDocumentState().SetModified();
    }
}

void SwDoc::GetTabBorders( const SwCursor& rCursor, SfxItemSet& rSet )
{
    SwContentNode* pCntNd = rCursor.GetPoint()->nNode.GetNode().GetContentNode();
    SwTableNode* pTableNd = pCntNd ? pCntNd->FindTableNode() : nullptr;
    if( !pTableNd )
        return ;

    SwLayoutFrame *pStart, *pEnd;
    ::lcl_GetStartEndCell( rCursor, pStart, pEnd );

    SwSelUnions aUnions;
    ::MakeSelUnions( aUnions, pStart, pEnd );

    if( !aUnions.empty() )
    {
        SvxBoxItem     aSetBox    ( rSet.Get(RES_BOX    ) );
        SvxBoxInfoItem aSetBoxInfo( rSet.Get(SID_ATTR_BORDER_INNER) );

        bool bTopSet      = false,
             bBottomSet   = false,
             bLeftSet     = false,
             bRightSet    = false,
             bHoriSet     = false,
             bVertSet     = false,
             bDistanceSet = false,
             bRTLTab      = false;

        aSetBoxInfo.ResetFlags();

        for ( SwSelUnions::size_type i = 0; i < aUnions.size(); ++i )
        {
            SwSelUnion *pUnion = &aUnions[i];
            const SwTabFrame *pTab = pUnion->GetTable();
            const SwRect &rUnion = pUnion->GetUnion();
            const bool bFirst = i == 0;
            const bool bLast  = (i == aUnions.size() - 1);

            std::vector<SwCellFrame*> aCellArr;
            aCellArr.reserve(255);
            ::lcl_CollectCells( aCellArr, rUnion, const_cast<SwTabFrame*>(pTab) );

            for ( auto pCell : aCellArr )
            {
                const bool bVert = pTab->IsVertical();
                const bool bRTL = bRTLTab = pTab->IsRightToLeft();
                bool bTopOver, bLeftOver, bRightOver, bBottomOver;
                if ( bVert )
                {
                    bTopOver = pCell->getFrameArea().Right() >= rUnion.Right();
                    bLeftOver = pCell->getFrameArea().Top() <= rUnion.Top();
                    bRightOver = pCell->getFrameArea().Bottom() >= rUnion.Bottom();
                    bBottomOver = pCell->getFrameArea().Left() <= rUnion.Left();
                }
                else
                {
                    bTopOver = pCell->getFrameArea().Top() <= rUnion.Top();
                    bLeftOver = pCell->getFrameArea().Left() <= rUnion.Left();
                    bRightOver = pCell->getFrameArea().Right() >= rUnion.Right();
                    bBottomOver = pCell->getFrameArea().Bottom() >= rUnion.Bottom();
                }

                if ( bRTL )
                {
                    bool bTmp = bRightOver;
                    bRightOver = bLeftOver;
                    bLeftOver = bTmp;
                }

                const SwFrameFormat  *pFormat  = pCell->GetFormat();
                const SvxBoxItem  &rBox  = pFormat->GetBox();

                // Top Border
                if ( bFirst && bTopOver )
                {
                    if (aSetBoxInfo.IsValid(SvxBoxInfoItemValidFlags::TOP))
                    {
                        if ( !bTopSet )
                        {   bTopSet = true;
                            aSetBox.SetLine( rBox.GetTop(), SvxBoxItemLine::TOP );
                        }
                        else if ((aSetBox.GetTop() && rBox.GetTop() &&
                                 !(*aSetBox.GetTop() == *rBox.GetTop())) ||
                                 ((!aSetBox.GetTop()) != (!rBox.GetTop()))) // != expression is true, if one and only one of the two pointers is !0
                        {
                            aSetBoxInfo.SetValid(SvxBoxInfoItemValidFlags::TOP, false );
                            aSetBox.SetLine( nullptr, SvxBoxItemLine::TOP );
                        }
                    }
                }

                // Left Border
                if ( bLeftOver )
                {
                    if (aSetBoxInfo.IsValid(SvxBoxInfoItemValidFlags::LEFT))
                    {
                        if ( !bLeftSet )
                        {   bLeftSet = true;
                            aSetBox.SetLine( rBox.GetLeft(), SvxBoxItemLine::LEFT );
                        }
                        else if ((aSetBox.GetLeft() && rBox.GetLeft() &&
                                 !(*aSetBox.GetLeft() == *rBox.GetLeft())) ||
                                 ((!aSetBox.GetLeft()) != (!rBox.GetLeft())))
                        {
                            aSetBoxInfo.SetValid(SvxBoxInfoItemValidFlags::LEFT, false );
                            aSetBox.SetLine( nullptr, SvxBoxItemLine::LEFT );
                        }
                    }
                }
                else
                {
                    if (aSetBoxInfo.IsValid(SvxBoxInfoItemValidFlags::VERT))
                    {
                        if ( !bVertSet )
                        {   bVertSet = true;
                            aSetBoxInfo.SetLine( rBox.GetLeft(), SvxBoxInfoItemLine::VERT );
                        }
                        else if ((aSetBoxInfo.GetVert() && rBox.GetLeft() &&
                                 !(*aSetBoxInfo.GetVert() == *rBox.GetLeft())) ||
                                 ((!aSetBoxInfo.GetVert()) != (!rBox.GetLeft())))
                        {   aSetBoxInfo.SetValid( SvxBoxInfoItemValidFlags::VERT, false );
                            aSetBoxInfo.SetLine( nullptr, SvxBoxInfoItemLine::VERT );
                        }
                    }
                }

                // Right Border
                if ( aSetBoxInfo.IsValid(SvxBoxInfoItemValidFlags::RIGHT) && bRightOver )
                {
                    if ( !bRightSet )
                    {   bRightSet = true;
                        aSetBox.SetLine( rBox.GetRight(), SvxBoxItemLine::RIGHT );
                    }
                    else if ((aSetBox.GetRight() && rBox.GetRight() &&
                             !(*aSetBox.GetRight() == *rBox.GetRight())) ||
                             (!aSetBox.GetRight() != !rBox.GetRight()))
                    {   aSetBoxInfo.SetValid( SvxBoxInfoItemValidFlags::RIGHT, false );
                        aSetBox.SetLine( nullptr, SvxBoxItemLine::RIGHT );
                    }
                }

                // Bottom Border
                if ( bLast && bBottomOver )
                {
                    if ( aSetBoxInfo.IsValid(SvxBoxInfoItemValidFlags::BOTTOM) )
                    {
                        if ( !bBottomSet )
                        {   bBottomSet = true;
                            aSetBox.SetLine( rBox.GetBottom(), SvxBoxItemLine::BOTTOM );
                        }
                        else if ((aSetBox.GetBottom() && rBox.GetBottom() &&
                                 !(*aSetBox.GetBottom() == *rBox.GetBottom())) ||
                                 (!aSetBox.GetBottom() != !rBox.GetBottom()))
                        {   aSetBoxInfo.SetValid( SvxBoxInfoItemValidFlags::BOTTOM, false );
                            aSetBox.SetLine( nullptr, SvxBoxItemLine::BOTTOM );
                        }
                    }
                }
                // In all Lines, except for the last one, the horizontal Line
                // is taken from the Bottom Line.
                else
                {
                    if (aSetBoxInfo.IsValid(SvxBoxInfoItemValidFlags::HORI))
                    {
                        if ( !bHoriSet )
                        {   bHoriSet = true;
                            aSetBoxInfo.SetLine( rBox.GetBottom(), SvxBoxInfoItemLine::HORI );
                        }
                        else if ((aSetBoxInfo.GetHori() && rBox.GetBottom() &&
                                 !(*aSetBoxInfo.GetHori() == *rBox.GetBottom())) ||
                                 ((!aSetBoxInfo.GetHori()) != (!rBox.GetBottom())))
                        {
                            aSetBoxInfo.SetValid( SvxBoxInfoItemValidFlags::HORI, false );
                            aSetBoxInfo.SetLine( nullptr, SvxBoxInfoItemLine::HORI );
                        }
                    }
                }

                // Distance to text
                if (aSetBoxInfo.IsValid(SvxBoxInfoItemValidFlags::DISTANCE))
                {
                    if( !bDistanceSet ) // Set on first iteration
                    {
                        bDistanceSet = true;
                        for( SvxBoxItemLine k : o3tl::enumrange<SvxBoxItemLine>() )
                            aSetBox.SetDistance( rBox.GetDistance( k ), k );
                    }
                    else
                    {
                        for( SvxBoxItemLine k : o3tl::enumrange<SvxBoxItemLine>() )
                            if( aSetBox.GetDistance( k ) !=
                                rBox.GetDistance( k ) )
                            {
                                aSetBoxInfo.SetValid( SvxBoxInfoItemValidFlags::DISTANCE, false );
                                aSetBox.SetAllDistances(0);
                                break;
                            }
                    }
                }
            }
        }

        // fdo#62470 fix the reading for table format.
        if ( bRTLTab )
        {
            SvxBoxItem     aTempBox    ( rSet.Get(RES_BOX    ) );
            SvxBoxInfoItem aTempBoxInfo( rSet.Get(SID_ATTR_BORDER_INNER) );

            aTempBox.SetLine( aSetBox.GetRight(), SvxBoxItemLine::RIGHT);
            aSetBox.SetLine( aSetBox.GetLeft(), SvxBoxItemLine::RIGHT);
            aSetBox.SetLine( aTempBox.GetRight(), SvxBoxItemLine::LEFT);

            aTempBoxInfo.SetValid( SvxBoxInfoItemValidFlags::LEFT, aSetBoxInfo.IsValid(SvxBoxInfoItemValidFlags::LEFT) );
            aSetBoxInfo.SetValid( SvxBoxInfoItemValidFlags::LEFT, aSetBoxInfo.IsValid(SvxBoxInfoItemValidFlags::RIGHT) );
            aSetBoxInfo.SetValid( SvxBoxInfoItemValidFlags::RIGHT, aTempBoxInfo.IsValid(SvxBoxInfoItemValidFlags::LEFT) );
        }

        rSet.Put( aSetBox );
        rSet.Put( aSetBoxInfo );
    }
}

void SwDoc::SetBoxAttr( const SwCursor& rCursor, const SfxPoolItem &rNew )
{
    SwTableNode* pTableNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    SwSelBoxes aBoxes;
    if( pTableNd && ::lcl_GetBoxSel( rCursor, aBoxes, true ) )
    {
        SwTable& rTable = pTableNd->GetTable();
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            GetIDocumentUndoRedo().AppendUndo( new SwUndoAttrTable(*pTableNd) );
        }

        std::vector<std::unique_ptr<SwTableFormatCmp>> aFormatCmp;
        aFormatCmp.reserve(std::max<size_t>(255, aBoxes.size()));
        for (size_t i = 0; i < aBoxes.size(); ++i)
        {
            SwTableBox *pBox = aBoxes[i];

            SwFrameFormat *pNewFormat;
            if ( nullptr != (pNewFormat = SwTableFormatCmp::FindNewFormat( aFormatCmp, pBox->GetFrameFormat(), 0 )))
                pBox->ChgFrameFormat( static_cast<SwTableBoxFormat*>(pNewFormat) );
            else
            {
                SwFrameFormat *pOld = pBox->GetFrameFormat();
                SwFrameFormat *pNew = pBox->ClaimFrameFormat();
                pNew->SetFormatAttr( rNew );
                aFormatCmp.push_back(o3tl::make_unique<SwTableFormatCmp>(pOld, pNew, 0));
            }

            pBox->SetDirectFormatting(true);
        }

        SwHTMLTableLayout *pTableLayout = rTable.GetHTMLTableLayout();
        if( pTableLayout )
        {
            SwContentFrame* pFrame = rCursor.GetContentNode()->getLayoutFrame( rCursor.GetContentNode()->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout() );
            SwTabFrame* pTabFrame = pFrame->ImplFindTabFrame();

            pTableLayout->Resize(
                pTableLayout->GetBrowseWidthByTabFrame( *pTabFrame ), true );
        }
        getIDocumentState().SetModified();
    }
}

bool SwDoc::GetBoxAttr( const SwCursor& rCursor, SfxPoolItem& rToFill )
{
    bool bRet = false;
    SwTableNode* pTableNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    SwSelBoxes aBoxes;
    if( pTableNd && lcl_GetBoxSel( rCursor, aBoxes ))
    {
        bRet = true;
        bool bOneFound = false;
        const sal_uInt16 nWhich = rToFill.Which();
        for (size_t i = 0; i < aBoxes.size(); ++i)
        {
            switch ( nWhich )
            {
                case RES_BACKGROUND:
                {
                    SvxBrushItem aBack =
                        aBoxes[i]->GetFrameFormat()->makeBackgroundBrushItem();
                    if( !bOneFound )
                    {
                        static_cast<SvxBrushItem&>(rToFill) = aBack;
                        bOneFound = true;
                    }
                    else if( rToFill != aBack )
                        bRet = false;
                }
                break;

                case RES_FRAMEDIR:
                {
                    const SvxFrameDirectionItem& rDir =
                                    aBoxes[i]->GetFrameFormat()->GetFrameDir();
                    if( !bOneFound )
                    {
                        static_cast<SvxFrameDirectionItem&>(rToFill) = rDir;
                        bOneFound = true;
                    }
                    else if( rToFill != rDir )
                        bRet = false;
                }
                break;
                case RES_VERT_ORIENT:
                {
                    const SwFormatVertOrient& rOrient =
                                    aBoxes[i]->GetFrameFormat()->GetVertOrient();
                    if( !bOneFound )
                    {
                        static_cast<SwFormatVertOrient&>(rToFill) = rOrient;
                        bOneFound = true;
                    }
                    else if( rToFill != rOrient )
                        bRet = false;
                }
                break;
            }

            if ( !bRet )
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
    SwFormatVertOrient aVertOri( 0, nAlign );
    SetBoxAttr( rCursor, aVertOri );
}

sal_uInt16 SwDoc::GetBoxAlign( const SwCursor& rCursor )
{
    sal_uInt16 nAlign = USHRT_MAX;
    SwTableNode* pTableNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
    SwSelBoxes aBoxes;
    if( pTableNd && ::lcl_GetBoxSel( rCursor, aBoxes ))
    {
        for (size_t i = 0; i < aBoxes.size(); ++i)
        {
            const SwFormatVertOrient &rOri =
                            aBoxes[i]->GetFrameFormat()->GetVertOrient();
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

static sal_uInt16 lcl_CalcCellFit( const SwLayoutFrame *pCell )
{
    SwTwips nRet = 0;
    const SwFrame *pFrame = pCell->Lower(); // The whole Line
    SwRectFnSet aRectFnSet(pCell);
    while ( pFrame )
    {
        const SwTwips nAdd = aRectFnSet.GetWidth(pFrame->getFrameArea()) -
                             aRectFnSet.GetWidth(pFrame->getFramePrintArea());

        // pFrame does not necessarily have to be a SwTextFrame!
        const SwTwips nCalcFitToContent = pFrame->IsTextFrame() ?
                                          const_cast<SwTextFrame*>(static_cast<const SwTextFrame*>(pFrame))->CalcFitToContent() :
                                          aRectFnSet.GetWidth(pFrame->getFramePrintArea());

        nRet = std::max( nRet, nCalcFitToContent + nAdd );
        pFrame = pFrame->GetNext();
    }
    // Surrounding border as well as left and Right Border also need to be respected
    nRet += aRectFnSet.GetWidth(pCell->getFrameArea()) -
            aRectFnSet.GetWidth(pCell->getFramePrintArea());

    // To compensate for the accuracy of calculation later on in SwTable::SetTabCols
    // we keep adding up a little.
    nRet += COLFUZZY;
    return static_cast<sal_uInt16>(std::max( long(MINLAY), nRet ));
}

/* The Line is within the Selection but not outlined by the TabCols.
 *
 * That means that the Line has been "split" by other Cells due to the
 * two-dimensional representation used. Thus, we have to distribute the cell's
 * default or minimum value amongst the Cell it has been split by.
 *
 * First, we collect the Columns (not the Column separators) which overlap
 * with the Cell. We then distribute the desired value according to the
 * amount of overlapping amongst the Cells.
 *
 * A Cell's default value stays the same if it already has a larger value than
 * the desired one. It's overwritten if it's smaller.
 */
static void lcl_CalcSubColValues( std::vector<sal_uInt16> &rToFill, const SwTabCols &rCols,
                              const SwLayoutFrame *pCell, const SwLayoutFrame *pTab,
                              bool bWishValues )
{
    const sal_uInt16 nWish = bWishValues ?
                    ::lcl_CalcCellFit( pCell ) :
                    MINLAY + sal_uInt16(pCell->getFrameArea().Width() - pCell->getFramePrintArea().Width());

    SwRectFnSet aRectFnSet(pTab);

    for ( size_t i = 0 ; i <= rCols.Count(); ++i )
    {
        long nColLeft  = i == 0             ? rCols.GetLeft()  : rCols[i-1];
        long nColRight = i == rCols.Count() ? rCols.GetRight() : rCols[i];
        nColLeft  += rCols.GetLeftMin();
        nColRight += rCols.GetLeftMin();

        // Adapt values to the proportions of the Table (Follows)
        if ( rCols.GetLeftMin() != aRectFnSet.GetLeft(pTab->getFrameArea()) )
        {
            const long nDiff = aRectFnSet.GetLeft(pTab->getFrameArea()) - rCols.GetLeftMin();
            nColLeft  += nDiff;
            nColRight += nDiff;
        }
        const long nCellLeft  = aRectFnSet.GetLeft(pCell->getFrameArea());
        const long nCellRight = aRectFnSet.GetRight(pCell->getFrameArea());

        // Calculate overlapping value
        long nWidth = 0;
        if ( nColLeft <= nCellLeft && nColRight >= (nCellLeft+COLFUZZY) )
            nWidth = nColRight - nCellLeft;
        else if ( nColLeft <= (nCellRight-COLFUZZY) && nColRight >= nCellRight )
            nWidth = nCellRight - nColLeft;
        else if ( nColLeft >= nCellLeft && nColRight <= nCellRight )
            nWidth = nColRight - nColLeft;
        if ( nWidth && pCell->getFrameArea().Width() )
        {
            long nTmp = nWidth * nWish / pCell->getFrameArea().Width();
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
 * @param bWishValues == true:     We calculate the desired value of all affected
 *                                 Cells for the current Selection/current Cell.
 *                                 If more Cells are within a Column, the highest
 *                                 desired value is returned.
 *                                 We set TabCol entries for which we did not calculate
 *                                 Cells to 0.
 *
 * @param bWishValues == false:     The Selection is expanded vertically.
 *                                  We calculate the minimum value for every
 *                                  Column in the TabCols that intersects with the
 *                                  Selection.
 */
static void lcl_CalcColValues( std::vector<sal_uInt16> &rToFill, const SwTabCols &rCols,
                           const SwLayoutFrame *pStart, const SwLayoutFrame *pEnd,
                           bool bWishValues )
{
    SwSelUnions aUnions;
    ::MakeSelUnions( aUnions, pStart, pEnd,
                    bWishValues ? SwTableSearchType::NONE : SwTableSearchType::Col );

    for ( auto &rU : aUnions )
    {
        SwSelUnion *pSelUnion = &rU;
        const SwTabFrame *pTab = pSelUnion->GetTable();
        const SwRect &rUnion = pSelUnion->GetUnion();

        SwRectFnSet aRectFnSet(pTab);
        bool bRTL = pTab->IsRightToLeft();

        const SwLayoutFrame *pCell = pTab->FirstCell();
        if (!pCell)
            continue;
        do
        {
            if ( pCell->IsCellFrame() && pCell->FindTabFrame() == pTab && ::IsFrameInTableSel( rUnion, pCell ) )
            {
                const long nCLeft  = aRectFnSet.GetLeft(pCell->getFrameArea());
                const long nCRight = aRectFnSet.GetRight(pCell->getFrameArea());

                bool bNotInCols = true;

                for ( size_t i = 0; i <= rCols.Count(); ++i )
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
                    if ( rCols.GetLeftMin() !=  sal_uInt16(aRectFnSet.GetLeft(pTab->getFrameArea())) )
                    {
                        const long nDiff = aRectFnSet.GetLeft(pTab->getFrameArea()) - rCols.GetLeftMin();
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
                        {   const sal_uInt16 nMin = MINLAY + sal_uInt16(pCell->getFrameArea().Width() -
                                                                pCell->getFramePrintArea().Width());
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
            } while( pCell && pCell->getFrameArea().Width() == 0 );
        } while ( pCell && pTab->IsAnLower( pCell ) );
    }
}

void SwDoc::AdjustCellWidth( const SwCursor& rCursor, bool bBalance )
{
    // Check whether the current Cursor has it's Point/Mark in a Table
    SwContentNode* pCntNd = rCursor.GetPoint()->nNode.GetNode().GetContentNode();
    SwTableNode* pTableNd = pCntNd ? pCntNd->FindTableNode() : nullptr;
    if( !pTableNd )
        return ;

    SwLayoutFrame *pStart, *pEnd;
    ::lcl_GetStartEndCell( rCursor, pStart, pEnd );

    // Collect TabCols; we reset the Table with them
    SwFrame* pBoxFrame = pStart;
    while( pBoxFrame && !pBoxFrame->IsCellFrame() )
        pBoxFrame = pBoxFrame->GetUpper();

    if ( !pBoxFrame )
        return; // Robust

    SwTabCols aTabCols;
    GetTabCols( aTabCols, static_cast<SwCellFrame*>(pBoxFrame) );

    if ( ! aTabCols.Count() )
        return;

    std::vector<sal_uInt16> aWish(aTabCols.Count() + 1);
    std::vector<sal_uInt16> aMins(aTabCols.Count() + 1);

    ::lcl_CalcColValues( aWish, aTabCols, pStart, pEnd, true  );

    // It's more robust if we calculate the minimum values for the whole Table
    const SwTabFrame *pTab = pStart->ImplFindTabFrame();
    pStart = const_cast<SwLayoutFrame*>(static_cast<SwLayoutFrame const *>(pTab->FirstCell()));
    pEnd   = const_cast<SwLayoutFrame*>(pTab->FindLastContent()->GetUpper());
    while( !pEnd->IsCellFrame() )
        pEnd = pEnd->GetUpper();
    ::lcl_CalcColValues( aMins, aTabCols, pStart, pEnd, false );

    if( bBalance )
    {
        // All Columns, which are now selected, have a desired value.
        // We add up the current values, divide the result by their
        // count and get a desired value for balancing.
        sal_uInt16 nWish = 0, nCnt = 0;
        for ( size_t i = 0; i <= aTabCols.Count(); ++i )
        {
            int nDiff = aWish[i];
            if ( nDiff )
            {
                if ( i == 0 )
                    nWish += aTabCols[i] - aTabCols.GetLeft();
                else if ( i == aTabCols.Count() )
                    nWish += aTabCols.GetRight() - aTabCols[i-1];
                else
                    nWish += aTabCols[i] - aTabCols[i-1];
                ++nCnt;
            }
        }
        nWish /= nCnt;
        for (sal_uInt16 & rn : aWish)
            if ( rn )
                rn = nWish;
    }

    const long nOldRight = aTabCols.GetRight();

    // In order to make the implementation easier, but still use the available
    // space properly, we do this twice.

    // The problem: The first column is getting wider, the others get slimmer
    // only afterwards.
    // The first column's desired width would be discarded as it would cause
    // the Table's width to exceed the maximum width.
    for ( int k = 0; k < 2; ++k )
    {
        for ( size_t i = 0; i <= aTabCols.Count(); ++i )
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
                for ( size_t i2 = i; i2 < aTabCols.Count(); ++i2 )
                    aTabCols[i2] += nDiff;
                aTabCols.SetRight( nTabRight );
            }
        }
    }

    const long nNewRight = aTabCols.GetRight();

    SwFrameFormat *pFormat = pTableNd->GetTable().GetFrameFormat();
    const sal_Int16 nOriHori = pFormat->GetHoriOrient().GetHoriOrient();

    // We can leave the "real" work to the SwTable now
    SetTabCols( aTabCols, false, static_cast<SwCellFrame*>(pBoxFrame) );

    // Alignment might have been changed in SetTabCols; restore old value
    const SwFormatHoriOrient &rHori = pFormat->GetHoriOrient();
    SwFormatHoriOrient aHori( rHori );
    if ( aHori.GetHoriOrient() != nOriHori )
    {
        aHori.SetHoriOrient( nOriHori );
        pFormat->SetFormatAttr( aHori );
    }

    // We switch to left-adjusted for automatic width
    // We adjust the right border for Border attributes
    if( !bBalance && nNewRight < nOldRight )
    {
        if( aHori.GetHoriOrient() == text::HoriOrientation::FULL )
        {
            aHori.SetHoriOrient( text::HoriOrientation::LEFT );
            pFormat->SetFormatAttr( aHori );
        }
    }

    getIDocumentState().SetModified();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
