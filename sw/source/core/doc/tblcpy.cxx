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

#include <svl/zforlist.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <DocumentContentOperationsManager.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <cntfrm.hxx>
#include <pam.hxx>
#include <swtable.hxx>
#include <ndtxt.hxx>
#include <fldbas.hxx>
#include <tblsel.hxx>
#include <tabfrm.hxx>
#include <poolfmt.hxx>
#include <cellatr.hxx>
#include <mvsave.hxx>
#include <docary.hxx>
#include <fmtanchr.hxx>
#include <hints.hxx>
#include <UndoTable.hxx>
#include <redline.hxx>
#include <fmtfsize.hxx>
#include <deque>
#include <memory>
#include <o3tl/make_unique.hxx>

static void lcl_CpyBox( const SwTable& rCpyTable, const SwTableBox* pCpyBox,
                    SwTable& rDstTable, SwTableBox* pDstBox,
                    bool bDelContent, SwUndoTableCpyTable* pUndo );

// The following type will be used by table copy functions to describe
// the structure of tables (or parts of tables).
// It's for new table model only.

namespace
{
    struct BoxSpanInfo
    {
        SwTableBox* mpBox;
        SwTableBox* mpCopy;
        sal_uInt16 mnColSpan;
        bool mbSelected;
    };

    typedef std::vector< BoxSpanInfo > BoxStructure;
    typedef std::vector< BoxStructure > LineStructure;
    typedef std::deque< sal_uLong > ColumnStructure;

    struct SubBox
    {
        SwTableBox *mpBox;
        bool mbCovered;
    };

    typedef std::vector< SubBox > SubLine;
    typedef std::vector< SubLine > SubTable;

    class TableStructure
    {
    public:
        LineStructure maLines;
        ColumnStructure maCols;
        sal_uInt16 mnStartCol;
        sal_uInt16 mnAddLine;
        void addLine( sal_uInt16 &rLine, const SwTableBoxes&, const SwSelBoxes*,
                      bool bNewModel );
        void addBox( sal_uInt16 nLine, const SwSelBoxes*, SwTableBox *pBox,
                     sal_uLong &rnB, sal_uInt16 &rnC, ColumnStructure::iterator& rpCl,
                     BoxStructure::iterator& rpSel, bool &rbSel, bool bCover );
        void incColSpan( sal_uInt16 nLine, sal_uInt16 nCol );
        explicit TableStructure( const SwTable& rTable );
        TableStructure( const SwTable& rTable, FndBox_ &rFndBox,
                        const SwSelBoxes& rSelBoxes,
                        LineStructure::size_type nMinSize );
        LineStructure::size_type getLineCount() const
            { return maLines.size(); }
        void moreLines( const SwTable& rTable );
        void assignBoxes( const TableStructure &rSource );
        void copyBoxes( const SwTable& rSource, SwTable& rDstTable,
                        SwUndoTableCpyTable* pUndo ) const;
    };

    SubTable::iterator insertSubLine( SubTable& rSubTable, SwTableLine& rLine,
        const SubTable::iterator& pStartLn );

    SubTable::iterator insertSubBox( SubTable& rSubTable, SwTableBox& rBox,
        SubTable::iterator pStartLn, const SubTable::iterator& pEndLn )
    {
        if( !rBox.GetTabLines().empty() )
        {
            SubTable::size_type nSize = static_cast<SubTable::size_type>(std::distance( pStartLn, pEndLn ));
            if( nSize < rBox.GetTabLines().size() )
            {
                SubLine aSubLine;
                SubLine::iterator pBox = pStartLn->begin();
                SubLine::iterator pEnd = pStartLn->end();
                while( pBox != pEnd )
                {
                    SubBox aSub;
                    aSub.mpBox = pBox->mpBox;
                    aSub.mbCovered = true;
                    aSubLine.push_back( aSub );
                    ++pBox;
                }
                do
                {
                    rSubTable.insert( pEndLn, aSubLine );
                } while( ++nSize < rBox.GetTabLines().size() );
            }
            for( auto pLine : rBox.GetTabLines() )
                pStartLn = insertSubLine( rSubTable, *pLine, pStartLn );
            OSL_ENSURE( pStartLn == pEndLn, "Sub line confusion" );
        }
        else
        {
            SubBox aSub;
            aSub.mpBox = &rBox;
            aSub.mbCovered = false;
            while( pStartLn != pEndLn )
            {
                pStartLn->push_back( aSub );
                aSub.mbCovered = true;
                ++pStartLn;
            }
        }
        return pStartLn;
    }

    SubTable::iterator insertSubLine( SubTable& rSubTable, SwTableLine& rLine,
        const SubTable::iterator& pStartLn )
    {
        SubTable::iterator pMax = pStartLn;
        ++pMax;
        SubTable::difference_type nMax = 1;
        for( auto & pBox : rLine.GetTabBoxes() )
        {
            SubTable::iterator pTmp = insertSubBox( rSubTable, *pBox, pStartLn, pMax );
            SubTable::difference_type nTmp = std::distance( pStartLn, pTmp );
            if( nTmp > nMax )
            {
                pMax = pTmp;
                nMax = nTmp;
            }
        }
        return pMax;
    }

    TableStructure::TableStructure( const SwTable& rTable ) :
        maLines( rTable.GetTabLines().size() ), mnStartCol(USHRT_MAX),
        mnAddLine(0)
    {
        maCols.push_front(0);
        sal_uInt16 nCnt = 0;
        for( auto pLine : rTable.GetTabLines() )
            addLine( nCnt, pLine->GetTabBoxes(), nullptr, rTable.IsNewModel() );
    }

    TableStructure::TableStructure( const SwTable& rTable,
        FndBox_ &rFndBox, const SwSelBoxes& rSelBoxes,
        LineStructure::size_type nMinSize )
        : mnStartCol(USHRT_MAX), mnAddLine(0)
    {
        if( !rFndBox.GetLines().empty() )
        {
            bool bNoSelection = rSelBoxes.size() < 2;
            FndLines_t &rFndLines = rFndBox.GetLines();
            maCols.push_front(0);
            const SwTableLine* pLine = rFndLines.front()->GetLine();
            const sal_uInt16 nStartLn = rTable.GetTabLines().GetPos( pLine );
            SwTableLines::size_type nEndLn = nStartLn;
            if( rFndLines.size() > 1 )
            {
                pLine = rFndLines.back()->GetLine();
                nEndLn = rTable.GetTabLines().GetPos( pLine );
            }
            if( nStartLn < USHRT_MAX && nEndLn < USHRT_MAX )
            {
                const SwTableLines &rLines = rTable.GetTabLines();
                if( bNoSelection && nMinSize > nEndLn - nStartLn + 1 )
                {
                    SwTableLines::size_type nNewEndLn = nStartLn + nMinSize - 1;
                    if( nNewEndLn >= rLines.size() )
                    {
                        mnAddLine = nNewEndLn - rLines.size() + 1;
                        nNewEndLn = rLines.size() - 1;
                    }
                    while( nEndLn < nNewEndLn )
                    {
                        SwTableLine *pLine2 = rLines[ ++nEndLn ];
                        SwTableBox *pTmpBox = pLine2->GetTabBoxes()[0].get();
                        FndLine_ *pInsLine = new FndLine_( pLine2, &rFndBox );
                        pInsLine->GetBoxes().insert(pInsLine->GetBoxes().begin(), o3tl::make_unique<FndBox_>(pTmpBox, pInsLine));
                        rFndLines.push_back(std::unique_ptr<FndLine_>(pInsLine));
                    }
                }
                maLines.resize( nEndLn - nStartLn + 1 );
                const SwSelBoxes* pSelBoxes = &rSelBoxes;
                sal_uInt16 nCnt = 0;
                for( SwTableLines::size_type nLine = nStartLn; nLine <= nEndLn; ++nLine )
                {
                    addLine( nCnt, rLines[nLine]->GetTabBoxes(),
                             pSelBoxes, rTable.IsNewModel() );
                    if( bNoSelection )
                        pSelBoxes = nullptr;
                }
            }
            if( bNoSelection && mnStartCol < USHRT_MAX )
            {
                BoxStructure::iterator pC = maLines[0].begin();
                BoxStructure::iterator pEnd = maLines[0].end();
                sal_uInt16 nIdx = mnStartCol;
                mnStartCol = 0;
                while( nIdx && pC != pEnd )
                {
                    mnStartCol += pC->mnColSpan;
                    --nIdx;
                    ++pC;
                }
            }
            else
                mnStartCol = USHRT_MAX;
        }
    }

    void TableStructure::addLine( sal_uInt16 &rLine, const SwTableBoxes& rBoxes,
        const SwSelBoxes* pSelBoxes, bool bNewModel )
    {
        bool bComplex = false;
        if( !bNewModel )
            for( SwTableBoxes::size_type nBox = 0; !bComplex && nBox < rBoxes.size(); ++nBox )
                bComplex = !rBoxes[nBox]->GetTabLines().empty();
        if( bComplex )
        {
            SubTable aSubTable;
            SubLine aSubLine;
            aSubTable.push_back( aSubLine );
            SubTable::iterator pStartLn = aSubTable.begin();
            SubTable::iterator pEndLn = aSubTable.end();
            for( auto & pBox : rBoxes )
                insertSubBox( aSubTable, *pBox, pStartLn, pEndLn );
            SubTable::size_type nSize = aSubTable.size();
            if( nSize )
            {
                maLines.resize( maLines.size() + nSize - 1 );
                while( pStartLn != pEndLn )
                {
                    bool bSelected = false;
                    sal_uLong nBorder = 0;
                    sal_uInt16 nCol = 0;
                    maLines[rLine].reserve( pStartLn->size() );
                    BoxStructure::iterator pSel = maLines[rLine].end();
                    ColumnStructure::iterator pCol = maCols.begin();
                    SubLine::iterator pBox = pStartLn->begin();
                    SubLine::iterator pEnd = pStartLn->end();
                    while( pBox != pEnd )
                    {
                        addBox( rLine, pSelBoxes, pBox->mpBox, nBorder, nCol,
                            pCol, pSel, bSelected, pBox->mbCovered );
                        ++pBox;
                    }
                    ++rLine;
                    ++pStartLn;
                }
            }
        }
        else
        {
            bool bSelected = false;
            sal_uLong nBorder = 0;
            sal_uInt16 nCol = 0;
            maLines[rLine].reserve( rBoxes.size() );
            ColumnStructure::iterator pCol = maCols.begin();
            BoxStructure::iterator pSel = maLines[rLine].end();
            for( auto & pBox : rBoxes )
                addBox( rLine, pSelBoxes, pBox.get(), nBorder, nCol,
                        pCol, pSel, bSelected, false );
            ++rLine;
        }
    }

    void TableStructure::addBox( sal_uInt16 nLine, const SwSelBoxes* pSelBoxes,
        SwTableBox *pBox, sal_uLong &rnBorder, sal_uInt16 &rnCol,
        ColumnStructure::iterator& rpCol, BoxStructure::iterator& rpSel,
        bool &rbSelected, bool bCovered )
    {
        BoxSpanInfo aInfo;
        if( pSelBoxes &&
            pSelBoxes->end() != pSelBoxes->find( pBox ) )
        {
            aInfo.mbSelected = true;
            if( mnStartCol == USHRT_MAX )
            {
                mnStartCol = static_cast<sal_uInt16>(maLines[nLine].size());
                if( pSelBoxes->size() < 2 )
                {
                    pSelBoxes = nullptr;
                    aInfo.mbSelected = false;
                }
            }
        }
        else
            aInfo.mbSelected = false;
        rnBorder += pBox->GetFrameFormat()->GetFrameSize().GetWidth();
        const sal_uInt16 nLeftCol = rnCol;
        while( rpCol != maCols.end() && *rpCol < rnBorder )
        {
            ++rnCol;
            ++rpCol;
        }
        if( rpCol == maCols.end() || *rpCol > rnBorder )
        {
            rpCol = maCols.insert( rpCol, rnBorder );
            incColSpan( nLine, rnCol );
        }
        aInfo.mnColSpan = rnCol - nLeftCol;
        aInfo.mpCopy = nullptr;
        aInfo.mpBox = bCovered ? nullptr : pBox;
        maLines[nLine].push_back( aInfo );
        if( aInfo.mbSelected )
        {
            if( rbSelected )
            {
                while( rpSel != maLines[nLine].end() )
                {
                    rpSel->mbSelected = true;
                    ++rpSel;
                }
            }
            else
            {
                rpSel = maLines[nLine].end();
                rbSelected = true;
            }
            --rpSel;
        }
    }

    void TableStructure::moreLines( const SwTable& rTable )
    {
        if( mnAddLine )
        {
            const SwTableLines &rLines = rTable.GetTabLines();
            const sal_uInt16 nLineCount = rLines.size();
            if( nLineCount < mnAddLine )
                mnAddLine = nLineCount;
            sal_uInt16 nLine = static_cast<sal_uInt16>(maLines.size());
            maLines.resize( nLine + mnAddLine );
            while( mnAddLine )
            {
                SwTableLine *pLine = rLines[ nLineCount - mnAddLine ];
                addLine( nLine, pLine->GetTabBoxes(), nullptr, rTable.IsNewModel() );
                --mnAddLine;
            }
        }
    }

    void TableStructure::incColSpan( sal_uInt16 nLineMax, sal_uInt16 nNewCol )
    {
        for( sal_uInt16 nLine = 0; nLine < nLineMax; ++nLine )
        {
            BoxStructure::iterator pInfo = maLines[nLine].begin();
            BoxStructure::iterator pEnd = maLines[nLine].end();
            long nCol = pInfo->mnColSpan;
            while( nNewCol > nCol && ++pInfo != pEnd )
                nCol += pInfo->mnColSpan;
            if( pInfo != pEnd )
                ++(pInfo->mnColSpan);
        }
    }

    void TableStructure::assignBoxes( const TableStructure &rSource )
    {
        LineStructure::const_iterator pFirstLine = rSource.maLines.begin();
        LineStructure::const_iterator pLastLine = rSource.maLines.end();
        if( pFirstLine == pLastLine )
            return;
        LineStructure::const_iterator pCurrLine = pFirstLine;
        LineStructure::size_type nLineCount = maLines.size();
        sal_uInt16 nFirstStartCol = 0;
        {
            BoxStructure::const_iterator pFirstBox = pFirstLine->begin();
            if( pFirstBox != pFirstLine->end() && pFirstBox->mpBox &&
                pFirstBox->mpBox->getDummyFlag() )
                nFirstStartCol = pFirstBox->mnColSpan;
        }
        for( LineStructure::size_type nLine = 0; nLine < nLineCount; ++nLine )
        {
            BoxStructure::const_iterator pFirstBox = pCurrLine->begin();
            BoxStructure::const_iterator pLastBox = pCurrLine->end();
            sal_uInt16 nCurrStartCol = mnStartCol;
            if( pFirstBox != pLastBox )
            {
                BoxStructure::const_iterator pTmpBox = pLastBox;
                --pTmpBox;
                if( pTmpBox->mpBox && pTmpBox->mpBox->getDummyFlag() )
                    --pLastBox;
                if( pFirstBox != pLastBox && pFirstBox->mpBox &&
                    pFirstBox->mpBox->getDummyFlag() )
                {
                    if( nCurrStartCol < USHRT_MAX )
                    {
                        if( pFirstBox->mnColSpan > nFirstStartCol )
                            nCurrStartCol += pFirstBox->mnColSpan - nFirstStartCol;
                    }
                    ++pFirstBox;
                }
            }
            if( pFirstBox != pLastBox )
            {
                BoxStructure::const_iterator pCurrBox = pFirstBox;
                BoxStructure &rBox = maLines[nLine];
                BoxStructure::size_type nBoxCount = rBox.size();
                sal_uInt16 nCol = 0;
                for( BoxStructure::size_type nBox = 0; nBox < nBoxCount; ++nBox )
                {
                    BoxSpanInfo& rInfo = rBox[nBox];
                    nCol += rInfo.mnColSpan;
                    if( rInfo.mbSelected || nCol > nCurrStartCol )
                    {
                        rInfo.mpCopy = pCurrBox->mpBox;
                        if( rInfo.mbSelected && rInfo.mpCopy->getDummyFlag() )
                        {
                            ++pCurrBox;
                            if( pCurrBox == pLastBox )
                            {
                                pCurrBox = pFirstBox;
                                if( pCurrBox->mpBox->getDummyFlag() )
                                    ++pCurrBox;
                            }
                            rInfo.mpCopy = pCurrBox->mpBox;
                        }
                        ++pCurrBox;
                        if( pCurrBox == pLastBox )
                        {
                            if( rInfo.mbSelected )
                                pCurrBox = pFirstBox;
                            else
                            {
                                rInfo.mbSelected = rInfo.mpCopy == nullptr;
                                break;
                            }
                        }
                        rInfo.mbSelected = rInfo.mpCopy == nullptr;
                    }
                }
            }
            ++pCurrLine;
            if( pCurrLine == pLastLine )
                pCurrLine = pFirstLine;
        }
    }

    void TableStructure::copyBoxes( const SwTable& rSource, SwTable& rDstTable,
                                    SwUndoTableCpyTable* pUndo ) const
    {
        LineStructure::size_type nLineCount = maLines.size();
        for( LineStructure::size_type nLine = 0; nLine < nLineCount; ++nLine )
        {
            const BoxStructure &rBox = maLines[nLine];
            BoxStructure::size_type nBoxCount = rBox.size();
            for( BoxStructure::size_type nBox = 0; nBox < nBoxCount; ++nBox )
            {
                const BoxSpanInfo& rInfo = rBox[nBox];
                if( ( rInfo.mpCopy && !rInfo.mpCopy->getDummyFlag() )
                    || rInfo.mbSelected )
                {
                    SwTableBox *pBox = rInfo.mpBox;
                    if( pBox && pBox->getRowSpan() > 0 )
                        lcl_CpyBox( rSource, rInfo.mpCopy, rDstTable, pBox,
                                    true, pUndo );
                }
            }
        }
    }
}

/** Copy Table into this Box.
    Copy all Boxes of a Line into the corresponding Boxes. The old content
    is deleted by doing this.
    If no Box is left the remaining content goes to the Box of a "BaseLine".
    If there's no Line anymore, put it also into the last Box of a "BaseLine". */
static void lcl_CpyBox( const SwTable& rCpyTable, const SwTableBox* pCpyBox,
                    SwTable& rDstTable, SwTableBox* pDstBox,
                    bool bDelContent, SwUndoTableCpyTable* pUndo )
{
    OSL_ENSURE( ( !pCpyBox || pCpyBox->GetSttNd() ) && pDstBox->GetSttNd(),
            "No content in this Box" );

    SwDoc* pCpyDoc = rCpyTable.GetFrameFormat()->GetDoc();
    SwDoc* pDoc = rDstTable.GetFrameFormat()->GetDoc();

    // First copy the new content and then delete the old one.
    // Do not create empty Sections, otherwise they will be deleted!
    std::unique_ptr< SwNodeRange > pRg( pCpyBox ?
        new SwNodeRange ( *pCpyBox->GetSttNd(), 1,
        *pCpyBox->GetSttNd()->EndOfSectionNode() ) : nullptr );

    SwNodeIndex aInsIdx( *pDstBox->GetSttNd(), bDelContent ? 1 :
                        pDstBox->GetSttNd()->EndOfSectionIndex() -
                        pDstBox->GetSttIdx() );

    if( pUndo )
        pUndo->AddBoxBefore( *pDstBox, bDelContent );

    bool bUndoRedline = pUndo && pDoc->getIDocumentRedlineAccess().IsRedlineOn();
    ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());

    SwNodeIndex aSavePos( aInsIdx, -1 );
    if( pRg.get() )
        pCpyDoc->GetDocumentContentOperationsManager().CopyWithFlyInFly( *pRg, 0, aInsIdx, nullptr, false );
    else
        pDoc->GetNodes().MakeTextNode( aInsIdx, pDoc->GetDfltTextFormatColl() );
    ++aSavePos;

    SwTableLine* pLine = pDstBox->GetUpper();
    while( pLine->GetUpper() )
        pLine = pLine->GetUpper()->GetUpper();

    bool bReplaceColl = true;
    if( bDelContent && !bUndoRedline )
    {
        // Delete the Fly first, then the corresponding Nodes
        SwNodeIndex aEndNdIdx( *aInsIdx.GetNode().EndOfSectionNode() );

        // Move Bookmarks
        {
            SwPosition aMvPos( aInsIdx );
            SwContentNode* pCNd = SwNodes::GoPrevious( &aMvPos.nNode );
            aMvPos.nContent.Assign( pCNd, pCNd->Len() );
            SwDoc::CorrAbs( aInsIdx, aEndNdIdx, aMvPos );
        }

        // If we still have FlyFrames hanging around, delete them too
        for( const auto pFly : *pDoc->GetSpzFrameFormats() )
        {
            SwFormatAnchor const*const pAnchor = &pFly->GetAnchor();
            SwPosition const*const pAPos = pAnchor->GetContentAnchor();
            if (pAPos &&
                ((RndStdIds::FLY_AT_PARA == pAnchor->GetAnchorId()) ||
                 (RndStdIds::FLY_AT_CHAR == pAnchor->GetAnchorId())) &&
                aInsIdx <= pAPos->nNode && pAPos->nNode <= aEndNdIdx )
            {
                pDoc->getIDocumentLayoutAccess().DelLayoutFormat( pFly );
            }
        }

        // If DestBox is a Headline Box and has Table style set, then
        // DO NOT automatically set the TableHeadline style!
        if( 1 < rDstTable.GetTabLines().size() &&
            pLine == rDstTable.GetTabLines().front() )
        {
            SwContentNode* pCNd = aInsIdx.GetNode().GetContentNode();
            if( !pCNd )
            {
                SwNodeIndex aTmp( aInsIdx );
                pCNd = pDoc->GetNodes().GoNext( &aTmp );
            }

            if( pCNd &&
                RES_POOLCOLL_TABLE_HDLN !=
                    pCNd->GetFormatColl()->GetPoolFormatId() )
                bReplaceColl = false;
        }

        pDoc->GetNodes().Delete( aInsIdx, aEndNdIdx.GetIndex() - aInsIdx.GetIndex() );
    }

    //b6341295: Table copy redlining will be managed by AddBoxAfter()
    if( pUndo )
        pUndo->AddBoxAfter( *pDstBox, aInsIdx, bDelContent );

    // heading
    SwTextNode *const pTextNd = aSavePos.GetNode().GetTextNode();
    if( pTextNd )
    {
        const sal_uInt16 nPoolId = pTextNd->GetTextColl()->GetPoolFormatId();
        if( bReplaceColl &&
            (( 1 < rDstTable.GetTabLines().size() &&
                pLine == rDstTable.GetTabLines().front() )
                // Is the Table's content sill valid?
                ? RES_POOLCOLL_TABLE == nPoolId
                : RES_POOLCOLL_TABLE_HDLN == nPoolId ) )
        {
            SwTextFormatColl* pColl = pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(
                static_cast<sal_uInt16>(
                                    RES_POOLCOLL_TABLE == nPoolId
                                        ? RES_POOLCOLL_TABLE_HDLN
                                        : RES_POOLCOLL_TABLE ) );
            if( pColl )         // Apply style
            {
                SwPaM aPam( aSavePos );
                aPam.SetMark();
                aPam.Move( fnMoveForward, GoInSection );
                pDoc->SetTextFormatColl( aPam, pColl );
            }
        }

        // Delete the current Formula/Format/Value values
        if( SfxItemState::SET == pDstBox->GetFrameFormat()->GetItemState( RES_BOXATR_FORMAT ) ||
            SfxItemState::SET == pDstBox->GetFrameFormat()->GetItemState( RES_BOXATR_FORMULA ) ||
            SfxItemState::SET == pDstBox->GetFrameFormat()->GetItemState( RES_BOXATR_VALUE ) )
        {
            pDstBox->ClaimFrameFormat()->ResetFormatAttr( RES_BOXATR_FORMAT,
                                                 RES_BOXATR_VALUE );
        }

        // Copy the TableBoxAttributes - Formula/Format/Value
        if( pCpyBox )
        {
            SfxItemSet aBoxAttrSet( pCpyDoc->GetAttrPool(), svl::Items<RES_BOXATR_FORMAT,
                                                            RES_BOXATR_VALUE>{} );
            aBoxAttrSet.Put( pCpyBox->GetFrameFormat()->GetAttrSet() );
            if( aBoxAttrSet.Count() )
            {
                const SfxPoolItem* pItem;
                SvNumberFormatter* pN = pDoc->GetNumberFormatter( false );
                if( pN && pN->HasMergeFormatTable() && SfxItemState::SET == aBoxAttrSet.
                    GetItemState( RES_BOXATR_FORMAT, false, &pItem ) )
                {
                    sal_uLong nOldIdx = static_cast<const SwTableBoxNumFormat*>(pItem)->GetValue();
                    sal_uLong nNewIdx = pN->GetMergeFormatIndex( nOldIdx );
                    if( nNewIdx != nOldIdx )
                        aBoxAttrSet.Put( SwTableBoxNumFormat( nNewIdx ));
                }
                pDstBox->ClaimFrameFormat()->SetFormatAttr( aBoxAttrSet );
            }
        }
    }
}

bool SwTable::InsNewTable( const SwTable& rCpyTable, const SwSelBoxes& rSelBoxes,
                        SwUndoTableCpyTable* pUndo )
{
    SwDoc* pDoc = GetFrameFormat()->GetDoc();
    SwDoc* pCpyDoc = rCpyTable.GetFrameFormat()->GetDoc();

    SwTableNumFormatMerge aTNFM( *pCpyDoc, *pDoc );

    // Analyze source structure
    TableStructure aCopyStruct( rCpyTable );

    // Analyze target structure (from start box) and selected substructure
    FndBox_ aFndBox( nullptr, nullptr );
    {   // get all boxes/lines
        FndPara aPara( rSelBoxes, &aFndBox );
        ForEach_FndLineCopyCol( GetTabLines(), &aPara );
    }
    TableStructure aTarget( *this, aFndBox, rSelBoxes, aCopyStruct.getLineCount() );

    bool bClear = false;
    if( aTarget.mnAddLine && IsNewModel() )
    {
        SwSelBoxes aBoxes;
        aBoxes.insert( GetTabLines().back()->GetTabBoxes().front().get() );
        if( pUndo )
            pUndo->InsertRow( *this, aBoxes, aTarget.mnAddLine );
        else
            InsertRow( pDoc, aBoxes, aTarget.mnAddLine, /*bBehind*/true );

        aTarget.moreLines( *this );
        bClear = true;
    }

    // Find mapping, if needed extend target table and/or selection
    aTarget.assignBoxes( aCopyStruct );

    {
        // Change table formulas into relative representation
        SwTableFormulaUpdate aMsgHint( &rCpyTable );
        aMsgHint.m_eFlags = TBL_RELBOXNAME;
        pCpyDoc->getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );
    }

    // delete frames
    aFndBox.SetTableLines( *this );
    if( bClear )
        aFndBox.ClearLineBehind();
    aFndBox.DelFrames( *this );

    // copy boxes
    aTarget.copyBoxes( rCpyTable, *this, pUndo );

    // adjust row span attributes accordingly

    // make frames
    aFndBox.MakeFrames( *this );

    return true;
}

/** Copy Table into this Box.
    Copy all Boxes of a Line into the corresponding Boxes. The old content is
    deleted by doing this.
    If no Box is left the remaining content goes to the Box of a "BaseLine".
    If there's no Line anymore, put it also into the last Box of a "BaseLine". */
bool SwTable::InsTable( const SwTable& rCpyTable, const SwNodeIndex& rSttBox,
                        SwUndoTableCpyTable* pUndo )
{
    SetHTMLTableLayout(std::shared_ptr<SwHTMLTableLayout>());    // Delete HTML Layout

    SwDoc* pDoc = GetFrameFormat()->GetDoc();

    SwTableNode* pTableNd = pDoc->IsIdxInTable( rSttBox );

    // Find the Box, to which should be copied:
    SwTableBox* pMyBox = GetTableBox(
            rSttBox.GetNode().FindTableBoxStartNode()->GetIndex() );

    OSL_ENSURE( pMyBox, "Index is not in a Box in this Table" );

    // First delete the Table's Frames
    FndBox_ aFndBox( nullptr, nullptr );
    aFndBox.DelFrames( pTableNd->GetTable() );

    SwDoc* pCpyDoc = rCpyTable.GetFrameFormat()->GetDoc();

    {
        // Convert Table formulas to their relative representation
        SwTableFormulaUpdate aMsgHint( &rCpyTable );
        aMsgHint.m_eFlags = TBL_RELBOXNAME;
        pCpyDoc->getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );
    }

    SwTableNumFormatMerge aTNFM( *pCpyDoc, *pDoc );

    bool bDelContent = true;
    const SwTableBox* pTmp;

    for( auto pLine : rCpyTable.GetTabLines() )
    {
        // Get the first from the CopyLine
        const SwTableBox* pCpyBox = pLine->GetTabBoxes().front().get();
        while( !pCpyBox->GetTabLines().empty() )
            pCpyBox = pCpyBox->GetTabLines().front()->GetTabBoxes().front().get();

        do {
            // First copy the new content and then delete the old one.
            // Do not create empty Sections, otherwise they will be deleted!
            lcl_CpyBox( rCpyTable, pCpyBox, *this, pMyBox, bDelContent, pUndo );

            if( nullptr == (pTmp = pCpyBox->FindNextBox( rCpyTable, pCpyBox, false )))
                break;      // no more Boxes
            pCpyBox = pTmp;

            if( nullptr == ( pTmp = pMyBox->FindNextBox( *this, pMyBox, false )))
                bDelContent = false;  // No space left?
            else
                pMyBox = const_cast<SwTableBox*>(pTmp);

        } while( true );

        // Find the topmost Line
        SwTableLine* pNxtLine = pMyBox->GetUpper();
        while( pNxtLine->GetUpper() )
            pNxtLine = pNxtLine->GetUpper()->GetUpper();
        const SwTableLines::size_type nPos = GetTabLines().GetPos( pNxtLine ) + 1;
        // Is there a next?
        if( nPos >= GetTabLines().size() )
            bDelContent = false;      // there is none, all goes into the last Box
        else
        {
            // Find the next Box with content
            pNxtLine = GetTabLines()[ nPos ];
            pMyBox = pNxtLine->GetTabBoxes().front().get();
            while( !pMyBox->GetTabLines().empty() )
                pMyBox = pMyBox->GetTabLines().front()->GetTabBoxes().front().get();
            bDelContent = true;
        }
    }

    aFndBox.MakeFrames( pTableNd->GetTable() );     // Create the Frames anew
    return true;
}

bool SwTable::InsTable( const SwTable& rCpyTable, const SwSelBoxes& rSelBoxes,
                        SwUndoTableCpyTable* pUndo )
{
    OSL_ENSURE( !rSelBoxes.empty(), "Missing selection" );

    SetHTMLTableLayout(std::shared_ptr<SwHTMLTableLayout>());    // Delete HTML Layout

    if( IsNewModel() || rCpyTable.IsNewModel() )
        return InsNewTable( rCpyTable, rSelBoxes, pUndo );

    OSL_ENSURE( !rCpyTable.IsTableComplex(), "Table too complex" );

    SwDoc* pDoc = GetFrameFormat()->GetDoc();
    SwDoc* pCpyDoc = rCpyTable.GetFrameFormat()->GetDoc();

    SwTableNumFormatMerge aTNFM( *pCpyDoc, *pDoc );

    FndLine_ *pFLine, *pInsFLine = nullptr;
    FndBox_ aFndBox( nullptr, nullptr );
    // Find all Boxes/Lines
    {
        FndPara aPara( rSelBoxes, &aFndBox );
        ForEach_FndLineCopyCol( GetTabLines(), &aPara );
    }

    // Special case: If a Box is located in a Table, copy it to all selected
    // Boxes!
    if( 1 != rCpyTable.GetTabSortBoxes().size() )
    {
        FndBox_* pFndBox;

        const FndLines_t::size_type nFndCnt = aFndBox.GetLines().size();
        if( !nFndCnt )
            return false;

        // Check if we have enough space for all Lines and Boxes
        SwTableLines::size_type nTstLns = 0;
        pFLine = aFndBox.GetLines().front().get();
        sal_uInt16 nSttLine = GetTabLines().GetPos( pFLine->GetLine() );
        // Do we have as many rows, actually?
        if( 1 == nFndCnt )
        {
            // Is there still enough space in the Table?
            if( (GetTabLines().size() - nSttLine ) <
                rCpyTable.GetTabLines().size() )
            {
                // If we don't have enough Lines, then see if we can insert
                // new ones to reach our goal. But only if the SSelection
                // contains a Box!
                if( 1 < rSelBoxes.size() )
                    return false;

                const sal_uInt16 nNewLns = rCpyTable.GetTabLines().size() -
                                (GetTabLines().size() - nSttLine );

                // See if the Box count is high enough for the Lines
                SwTableLine* pLastLn = GetTabLines().back();

                SwTableBox* pSttBox = pFLine->GetBoxes()[0]->GetBox();
                const SwTableBoxes::size_type nSttBox = pFLine->GetLine()->GetBoxPos( pSttBox );
                for( SwTableLines::size_type n = rCpyTable.GetTabLines().size() - nNewLns;
                        n < rCpyTable.GetTabLines().size(); ++n )
                {
                    SwTableLine* pCpyLn = rCpyTable.GetTabLines()[ n ];

                    if( pLastLn->GetTabBoxes().size() < nSttBox ||
                        ( pLastLn->GetTabBoxes().size() - nSttBox ) <
                            pCpyLn->GetTabBoxes().size() )
                        return false;

                    // Test for nesting
                    for( SwTableBoxes::size_type nBx = 0; nBx < pCpyLn->GetTabBoxes().size(); ++nBx )
                        if( !pLastLn->GetTabBoxes()[ nSttBox + nBx ]->GetSttNd() )
                            return false;
                }
                // We have enough space for the to-be-copied, so insert new
                // rows accordingly.
                SwTableBox* pInsBox = pLastLn->GetTabBoxes()[ nSttBox ].get();
                OSL_ENSURE( pInsBox && pInsBox->GetSttNd(),
                    "no ContentBox or it's not in this Table" );
                SwSelBoxes aBoxes;

                if( pUndo
                    ? !pUndo->InsertRow( *this, SelLineFromBox( pInsBox,
                                aBoxes ), nNewLns )
                    : !InsertRow( pDoc, SelLineFromBox( pInsBox,
                                aBoxes ), nNewLns, /*bBehind*/true ) )
                    return false;
            }

            nTstLns = rCpyTable.GetTabLines().size();        // copy this many
        }
        else if( 0 == (nFndCnt % rCpyTable.GetTabLines().size()) )
            nTstLns = nFndCnt;
        else
            return false;       // not enough space for the rows

        for( SwTableLines::size_type nLn = 0; nLn < nTstLns; ++nLn )
        {
            // We have enough rows, so check the Boxes per row
            pFLine = aFndBox.GetLines()[ nLn % nFndCnt ].get();
            SwTableLine* pLine = pFLine->GetLine();
            SwTableBox* pSttBox = pFLine->GetBoxes()[0]->GetBox();
            const SwTableBoxes::size_type nSttBox = pLine->GetBoxPos( pSttBox );
            if( nLn >= nFndCnt )
            {
                // We have more rows in the ClipBoard than we have selected
                pInsFLine = new FndLine_( GetTabLines()[ nSttLine + nLn ],
                                        &aFndBox );
                pLine = pInsFLine->GetLine();
            }
            SwTableLine* pCpyLn = rCpyTable.GetTabLines()[ nLn %
                                        rCpyTable.GetTabLines().size() ];

            // Selected too few rows?
            if( pInsFLine )
            {
                // We insert a new row into the FndBox
                if( pLine->GetTabBoxes().size() < nSttBox ||
                    pLine->GetTabBoxes().size() - nSttBox < pFLine->GetBoxes().size() )
                {
                    delete pInsFLine;
                    return false;
                }

                // Test for nesting
                for (FndBoxes_t::size_type nBx = 0; nBx < pFLine->GetBoxes().size(); ++nBx)
                {
                    SwTableBox *pTmpBox = pLine->GetTabBoxes()[ nSttBox + nBx ].get();
                    if( !pTmpBox->GetSttNd() )
                    {
                        delete pInsFLine;
                        return false;
                    }
                    // if Ok, insert the Box into the FndLine
                    pFndBox = new FndBox_( pTmpBox, pInsFLine );
                    pInsFLine->GetBoxes().insert( pInsFLine->GetBoxes().begin() + nBx,
                            std::unique_ptr<FndBox_>(pFndBox));
                }
                aFndBox.GetLines().insert( aFndBox.GetLines().begin() + nLn, std::unique_ptr<FndLine_>(pInsFLine));
            }
            else if( pFLine->GetBoxes().size() == 1 )
            {
                if( pLine->GetTabBoxes().size() < nSttBox  ||
                    ( pLine->GetTabBoxes().size() - nSttBox ) <
                    pCpyLn->GetTabBoxes().size() )
                    return false;

                // Test for nesting
                for( SwTableBoxes::size_type nBx = 0; nBx < pCpyLn->GetTabBoxes().size(); ++nBx )
                {
                    SwTableBox *pTmpBox = pLine->GetTabBoxes()[ nSttBox + nBx ].get();
                    if( !pTmpBox->GetSttNd() )
                        return false;
                    // if Ok, insert the Box into the FndLine
                    if( nBx == pFLine->GetBoxes().size() )
                    {
                        pFndBox = new FndBox_( pTmpBox, pFLine );
                        pFLine->GetBoxes().insert(pFLine->GetBoxes().begin() + nBx,
                                std::unique_ptr<FndBox_>(pFndBox));
                    }
                }
            }
            else
            {
                // Match the selected Boxes with the ones in the Clipboard
                // (n times)
                if( 0 != ( pFLine->GetBoxes().size() %
                            pCpyLn->GetTabBoxes().size() ))
                    return false;

                // Test for nesting
                for (auto &rpBox : pFLine->GetBoxes())
                {
                    if (!rpBox->GetBox()->GetSttNd())
                        return false;
                }
            }
        }

        if( aFndBox.GetLines().empty() )
            return false;
    }

    {
        // Convert Table formulas to their relative representation
        SwTableFormulaUpdate aMsgHint( &rCpyTable );
        aMsgHint.m_eFlags = TBL_RELBOXNAME;
        pCpyDoc->getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );
    }

    // Delete the Frames
    aFndBox.SetTableLines( *this );
    //Not dispose accessible table
    aFndBox.DelFrames( *this );

    if( 1 == rCpyTable.GetTabSortBoxes().size() )
    {
        SwTableBox *pTmpBx = rCpyTable.GetTabSortBoxes()[0];
        for (size_t n = 0; n < rSelBoxes.size(); ++n)
        {
            lcl_CpyBox( rCpyTable, pTmpBx, *this,
                        rSelBoxes[n], true, pUndo );
        }
    }
    else
        for (FndLines_t::size_type nLn = 0; nLn < aFndBox.GetLines().size(); ++nLn)
        {
            pFLine = aFndBox.GetLines()[ nLn ].get();
            SwTableLine* pCpyLn = rCpyTable.GetTabLines()[
                                nLn % rCpyTable.GetTabLines().size() ];
            for (FndBoxes_t::size_type nBx = 0; nBx < pFLine->GetBoxes().size(); ++nBx)
            {
                // Copy the pCpyBox into pMyBox
                auto pCpyBox = pCpyLn->GetTabBoxes()[
                                nBx % pCpyLn->GetTabBoxes().size() ].get();
                lcl_CpyBox( rCpyTable, pCpyBox,
                            *this, pFLine->GetBoxes()[nBx]->GetBox(), true, pUndo );
            }
        }

    aFndBox.MakeFrames( *this );
    return true;
}

static void FndContentLine( const SwTableLine* pLine, SwSelBoxes* pPara );

static void FndContentBox( const SwTableBox* pBox, SwSelBoxes* pPara )
{
    if( !pBox->GetTabLines().empty() )
    {
        for( const SwTableLine* pLine : pBox->GetTabLines() )
            FndContentLine( pLine, pPara );
    }
    else
        pPara->insert( const_cast<SwTableBox*>(pBox) );
}

static void FndContentLine( const SwTableLine* pLine, SwSelBoxes* pPara )
{
    for( std::unique_ptr<SwTableBox> const & pBox : pLine->GetTabBoxes() )
        FndContentBox(pBox.get(), pPara );
}

// Find all Boxes with content in this Box
SwSelBoxes& SwTable::SelLineFromBox( const SwTableBox* pFindBox,
                                    SwSelBoxes& rBoxes, bool bToTop )
{
    SwTableLine* pLine = const_cast<SwTableLine*>(pFindBox->GetUpper());
    if( bToTop )
        while( pLine->GetUpper() )
            pLine = pLine->GetUpper()->GetUpper();

    // Delete all old ones
    rBoxes.clear();
    for( std::unique_ptr<SwTableBox> const & pBox : pLine->GetTabBoxes() )
        FndContentBox(pBox.get(), &rBoxes );
    return rBoxes;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
