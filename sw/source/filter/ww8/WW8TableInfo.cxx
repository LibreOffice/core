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

#include <iostream>
#include <set>
#include <stdio.h>
#include "WW8TableInfo.hxx"
#include <fmtfsize.hxx>
#include "attributeoutputbase.hxx"
#include <swtable.hxx>
#include <frmfmt.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <dbgoutsw.hxx>

namespace ww8
{

WW8TableNodeInfoInner::WW8TableNodeInfoInner(WW8TableNodeInfo * pParent)
: mpParent(pParent)
, mnDepth(0)
, mnCell(0)
, mnRow(0)
, mnShadowsBefore(0)
, mnShadowsAfter(0)
, mbEndOfLine(false)
, mbFinalEndOfLine(false)
, mbEndOfCell(false)
, mbFirstInTable(false)
, mbVertMerge(false)
, mpTableBox(nullptr)
, mpTable(nullptr)
{
}

WW8TableNodeInfoInner::~WW8TableNodeInfoInner()
{
}

void WW8TableNodeInfoInner::setDepth(sal_uInt32 nDepth)
{
    mnDepth = nDepth;
}

void WW8TableNodeInfoInner::setCell(sal_uInt32 nCell)
{
    mnCell = nCell;
}

void WW8TableNodeInfoInner::setRow(sal_uInt32 nRow)
{
    mnRow = nRow;
}

void WW8TableNodeInfoInner::setShadowsBefore(sal_uInt32 nShadowsBefore)
{
    mnShadowsBefore = nShadowsBefore;
}

void WW8TableNodeInfoInner::setShadowsAfter(sal_uInt32 nShadowsAfter)
{
    mnShadowsAfter = nShadowsAfter;
}

void WW8TableNodeInfoInner::setEndOfLine(bool bEndOfLine)
{
    mbEndOfLine = bEndOfLine;
}

void WW8TableNodeInfoInner::setFinalEndOfLine(bool bFinalEndOfLine)
{
    mbFinalEndOfLine = bFinalEndOfLine;
}

void WW8TableNodeInfoInner::setEndOfCell(bool bEndOfCell)
{
    mbEndOfCell = bEndOfCell;
}

void WW8TableNodeInfoInner::setFirstInTable(bool bFirstInTable)
{
    mbFirstInTable = bFirstInTable;
}

void WW8TableNodeInfoInner::setVertMerge(bool bVertMerge)

{
    mbVertMerge = bVertMerge;
}

void WW8TableNodeInfoInner::setTableBox(const SwTableBox * pTableBox)
{
    mpTableBox = pTableBox;
}

void WW8TableNodeInfoInner::setTable(const SwTable * pTable)
{
    mpTable = pTable;
}

void WW8TableNodeInfoInner::setRect(const SwRect & rRect)
{
    maRect = rRect;
}


const SwNode * WW8TableNodeInfoInner::getNode() const
{
    const SwNode * pResult = nullptr;

    if (mpParent != nullptr)
        pResult = mpParent->getNode();

    return pResult;
}

TableBoxVectorPtr WW8TableNodeInfoInner::getTableBoxesOfRow()
{
    TableBoxVectorPtr pResult(new TableBoxVector);

    WW8TableCellGrid::Pointer_t pCellGrid =
        mpParent->getParent()->getCellGridForTable(getTable(), false);

    if (pCellGrid.get() == nullptr)
    {
        const SwTableLine * pTabLine = getTableBox()->GetUpper();
        const SwTableBoxes & rTableBoxes = pTabLine->GetTabBoxes();

        sal_uInt8 nBoxes = rTableBoxes.size();
        if (nBoxes > MAXTABLECELLS)
            nBoxes = MAXTABLECELLS;
        for ( sal_uInt8 n = 0; n < nBoxes; n++ )
        {
            pResult->push_back(rTableBoxes[n].get());
        }
    }
    else
        pResult = pCellGrid->getTableBoxesOfRow(this);

    return pResult;
}

GridColsPtr WW8TableNodeInfoInner::getGridColsOfRow(AttributeOutputBase & rBase, bool calculateColumnsFromAllRows)
{
    GridColsPtr pResult(new GridCols);
    WidthsPtr pWidths;

    // Check which columns should be checked - only the current row,
    // or all the rows together
    if (calculateColumnsFromAllRows)
    {
                // Calculate the width of all the columns based on ALL the rows.
        // The difference is that this kind of draws vertical lines,
        // so that if the rows look like this:
        //
        //  ------------------------
        //  |                   |  |
        //  ------------------------
        //  |   |                  |
        //  ------------------------
        //  |       |              |
        //  ------------------------

        // then the actual column widths will be broken down like this:
        //
        //  ------------------------
        //  |   |   |           |  |
        //  ------------------------

        // See the example at
        // http://officeopenxml.com/WPtableGrid.php
        // Under "Word 2007 Example"
        pWidths = getColumnWidthsBasedOnAllRows();
    }
    else
    {
        // Calculate the width of all the columns based on the current row
        pWidths = getWidthsOfRow();
    }

    const SwFrameFormat *pFormat = getTable()->GetFrameFormat();
    OSL_ENSURE(pFormat,"Impossible");
    if (!pFormat)
        return pResult;

    const SwFormatFrameSize &rSize = pFormat->GetFrameSize();
    unsigned long nTableSz = static_cast<unsigned long>(rSize.GetWidth());

    long nPageSize = 0;
    bool bRelBoxSize = false;

    rBase.GetTablePageSize( this, nPageSize, bRelBoxSize );

    SwTwips nSz = 0;
    Widths::const_iterator aWidthsEnd = pWidths->end();
    for ( Widths::const_iterator aIt = pWidths->begin();
          aIt != aWidthsEnd;
          ++aIt)
    {
        nSz += *aIt;
        SwTwips nCalc = nSz;
        if ( bRelBoxSize )
            nCalc = ( nCalc * nPageSize ) / nTableSz;

        pResult->push_back( nCalc );
    }

    return pResult;
}

WidthsPtr WW8TableNodeInfoInner::getColumnWidthsBasedOnAllRows()
{
    WidthsPtr pWidths;

    WW8TableCellGrid::Pointer_t pCellGrid =
        mpParent->getParent()->getCellGridForTable(getTable(), false);

    if (pCellGrid.get() == nullptr)
    {
        const SwTable * pTable = getTable();
        const SwTableLines& rTableLines = pTable->GetTabLines();
        const size_t nNumOfLines = rTableLines.size();

        // Go over all the rows - and for each row - calculate where
        // there is a separator between columns
        WidthsPtr pSeparators(new Widths);
        for ( size_t nLineIndex = 0; nLineIndex < nNumOfLines; ++nLineIndex )
        {
            const SwTableLine *pCurrentLine = rTableLines[nLineIndex];
            const SwTableBoxes & rTabBoxes = pCurrentLine->GetTabBoxes();
            size_t nBoxes = rTabBoxes.size();
            if (nBoxes > MAXTABLECELLS)
                nBoxes = MAXTABLECELLS;

            sal_uInt32 nSeparatorPosition = 0;
            for (size_t nBoxIndex = 0; nBoxIndex < nBoxes; ++nBoxIndex)
            {
                const SwFrameFormat* pBoxFormat = rTabBoxes[ nBoxIndex ]->GetFrameFormat();
                const SwFormatFrameSize& rLSz = pBoxFormat->GetFrameSize();
                nSeparatorPosition += rLSz.GetWidth();
                pSeparators->push_back(nSeparatorPosition);
            }
        }

        // Sort the separator positions and remove any duplicates
        std::sort(pSeparators->begin(), pSeparators->end());
        std::vector<sal_uInt32>::iterator it = std::unique(pSeparators->begin(), pSeparators->end());
        pSeparators->erase(it, pSeparators->end());

        // Calculate the widths based on the position of the unique & sorted
        // column separators
        pWidths = std::make_shared<Widths>();
        sal_uInt32 nPreviousWidth = 0;
        Widths::const_iterator aItEnd2 = pSeparators->end();
        for (Widths::const_iterator aIt2 = pSeparators->begin(); aIt2 != aItEnd2; ++aIt2)
        {
            sal_uInt32 nCurrentWidth = *aIt2;
            pWidths->push_back(nCurrentWidth - nPreviousWidth);
            nPreviousWidth = nCurrentWidth;
        }
    }
    else
    {
        pWidths = pCellGrid->getWidthsOfRow(this);
    }

    return pWidths;
}

WidthsPtr WW8TableNodeInfoInner::getWidthsOfRow()
{
    WidthsPtr pWidths;

    WW8TableCellGrid::Pointer_t pCellGrid =
        mpParent->getParent()->getCellGridForTable(getTable(), false);

    if (pCellGrid.get() == nullptr)
    {
        const SwTableBox * pTabBox = getTableBox();
        const SwTableLine * pTabLine = pTabBox->GetUpper();
        const SwTableBoxes & rTabBoxes = pTabLine->GetTabBoxes();

        pWidths = std::make_shared<Widths>();
        // number of cell written
        sal_uInt32 nBoxes = rTabBoxes.size();
        if (nBoxes > MAXTABLECELLS)
            nBoxes = MAXTABLECELLS;

        for (sal_uInt32 n = 0; n < nBoxes; n++)
        {
            const SwFrameFormat* pBoxFormat = rTabBoxes[ n ]->GetFrameFormat();
            const SwFormatFrameSize& rLSz = pBoxFormat->GetFrameSize();

            pWidths->push_back(rLSz.GetWidth());
        }
    }
    else
        pWidths = pCellGrid->getWidthsOfRow(this);

    return pWidths;
}

RowSpansPtr WW8TableNodeInfoInner::getRowSpansOfRow()
{
    RowSpansPtr pResult(new RowSpans);

    WW8TableCellGrid::Pointer_t pCellGrid =
        mpParent->getParent()->getCellGridForTable(getTable(), false);

    if (pCellGrid.get() == nullptr)
    {
        const SwTableBox * pTabBox = getTableBox();
        const SwTableLine * pTabLine = pTabBox->GetUpper();
        const SwTableBoxes & rTabBoxes = pTabLine->GetTabBoxes();

        sal_uInt32 nBoxes = rTabBoxes.size();
        if (nBoxes > MAXTABLECELLS)
            nBoxes = MAXTABLECELLS;

        for (sal_uInt32 n = 0; n < nBoxes; ++n)
        {
            pResult->push_back(rTabBoxes[n]->getRowSpan());
        }
    }
    else
        pResult = pCellGrid->getRowSpansOfRow(this);

    return pResult;
 }


#ifdef DBG_UTIL
std::string WW8TableNodeInfoInner::toString() const
{
    static char buffer[256];
    snprintf(buffer, sizeof(buffer),
             "<tableinner depth=\"%" SAL_PRIuUINT32 "\""
             " cell=\"%" SAL_PRIuUINT32 "\""
             " row=\"%" SAL_PRIuUINT32 "\""
             " endOfCell=\"%s\""
             " endOfLine=\"%s\""
             " shadowsBefore=\"%" SAL_PRIuUINT32 "\""
             " shadowsAfter=\"%" SAL_PRIuUINT32 "\""
             " vertMerge=\"%s\"/>",
             mnDepth, mnCell, mnRow,
             mbEndOfCell ? "yes" : "no",
             mbEndOfLine ? "yes" : "no",
             mnShadowsBefore,
             mnShadowsAfter,
             mbVertMerge ? "yes" : "no");

    return std::string(buffer);
}
#endif

WW8TableNodeInfo::WW8TableNodeInfo(WW8TableInfo * pParent,
                                   const SwNode * pNode)
: mpParent(pParent),
  mnDepth(0),
  mpNode(pNode),
  mpNext(nullptr),
  mpNextNode(nullptr)
{
}

WW8TableNodeInfo::~WW8TableNodeInfo()
{
}

#ifdef DBG_UTIL
std::string WW8TableNodeInfo::toString() const
{
    static char buffer[1024];
    snprintf(buffer, sizeof(buffer),
             "<tableNodeInfo p=\"%p\" depth=\"%" SAL_PRIuUINT32 "\">"
             ,this, getDepth());

    std::string sResult(buffer);

    Inners_t::const_iterator aIt(mInners.begin());
    Inners_t::const_iterator aEnd(mInners.end());

    while (aIt != aEnd)
    {
        WW8TableNodeInfoInner::Pointer_t pInner = aIt->second;
        sResult += pInner->toString();

        ++aIt;
    }
    sResult += dbg_out(*mpNode);
    sResult += "</tableNodeInfo>";

    return sResult;
}
#endif

void WW8TableNodeInfo::setDepth(sal_uInt32 nDepth)
{
    mnDepth = nDepth;

    Inners_t::iterator aIt = mInners.find(mnDepth);

    if (aIt == mInners.end())
        mInners[mnDepth] = std::make_shared<ww8::WW8TableNodeInfoInner>(this);

    mInners[mnDepth]->setDepth(mnDepth);
}

void WW8TableNodeInfo::setEndOfLine(bool bEndOfLine)
{
    WW8TableNodeInfoInner::Pointer_t pInner = getInnerForDepth(mnDepth);
    pInner->setEndOfLine(bEndOfLine);

#ifdef DBG_UTIL
    SAL_INFO( "sw.ww8", "<endOfLine depth=\"" << mnDepth << "\">" << toString() << "</endOfLine>" );
#endif
}

void WW8TableNodeInfo::setEndOfCell(bool bEndOfCell)
{
    WW8TableNodeInfoInner::Pointer_t pInner = getInnerForDepth(mnDepth);
    pInner->setEndOfCell(bEndOfCell);

#ifdef DBG_UTIL
    SAL_INFO( "sw.ww8", "<endOfCell depth=\"" << mnDepth << "\">" << toString() << "</endOfCell>" );
#endif
}

void WW8TableNodeInfo::setFirstInTable(bool bFirstInTable)
{
    WW8TableNodeInfoInner::Pointer_t pInner = getInnerForDepth(mnDepth);

    pInner->setFirstInTable(bFirstInTable);

#ifdef DBG_UTIL
    SAL_INFO( "sw.ww8", "<firstInTable depth=\"" << mnDepth << "\">" << toString() << "</firstInTable>" );
#endif
}

void WW8TableNodeInfo::setVertMerge(bool bVertMerge)
{
    WW8TableNodeInfoInner::Pointer_t pInner = getInnerForDepth(mnDepth);

    pInner->setVertMerge(bVertMerge);

#ifdef DBG_UTIL
    SAL_INFO( "sw.ww8", "<vertMerge depth=\"" << mnDepth << "\">" << toString() << "</vertMerge>" );
#endif
}

void WW8TableNodeInfo::setTableBox(const SwTableBox * pTableBox)
{
    getInnerForDepth(mnDepth)->setTableBox(pTableBox);
}

void WW8TableNodeInfo::setTable(const SwTable * pTable)
{
    getInnerForDepth(mnDepth)->setTable(pTable);
}

void WW8TableNodeInfo::setNext(WW8TableNodeInfo * pNext)
{
    mpNext = pNext;

#ifdef DBG_UTIL
    SAL_INFO( "sw.ww8", "<setnext><from>" << toString() << "</from><to>" << pNext->toString() << "</to></setnext>" );
#endif
}

void WW8TableNodeInfo::setNextNode(const SwNode * pNode)
{
    mpNextNode = pNode;
}

void WW8TableNodeInfo::setRect(const SwRect & rRect)
{
    getInnerForDepth(mnDepth)->setRect(rRect);
}

void WW8TableNodeInfo::setCell(sal_uInt32 nCell)
{
    getInnerForDepth(mnDepth)->setCell(nCell);
}

void WW8TableNodeInfo::setRow(sal_uInt32 nRow)
{
    getInnerForDepth(mnDepth)->setRow(nRow);
}

void WW8TableNodeInfo::setShadowsBefore(sal_uInt32 nShadowsBefore)
{
    getInnerForDepth(mnDepth)->setShadowsBefore(nShadowsBefore);
}

void WW8TableNodeInfo::setShadowsAfter(sal_uInt32 nShadowsAfter)
{
    getInnerForDepth(mnDepth)->setShadowsAfter(nShadowsAfter);
}


sal_uInt32 WW8TableNodeInfo::getDepth() const
{
    if (!mInners.empty())
        return mInners.begin()->second->getDepth();

    return mnDepth;
}


const SwTableBox * WW8TableNodeInfo::getTableBox() const
{
    return getInnerForDepth(mnDepth)->getTableBox();
}

sal_uInt32 WW8TableNodeInfo::getCell() const
{
    return getInnerForDepth(mnDepth)->getCell();
}

sal_uInt32 WW8TableNodeInfo::getRow() const
{
    return getInnerForDepth(mnDepth)->getRow();
}

const WW8TableNodeInfoInner::Pointer_t WW8TableNodeInfo::getFirstInner() const
{
    WW8TableNodeInfoInner::Pointer_t pResult;

    if (!mInners.empty())
        pResult = mInners.begin()->second;

    return pResult;
}

const WW8TableNodeInfoInner::Pointer_t WW8TableNodeInfo::getInnerForDepth(sal_uInt32 nDepth) const
{
    WW8TableNodeInfoInner::Pointer_t pResult;

    Inners_t::const_iterator aIt = mInners.find(nDepth);
    if (aIt != mInners.end())
    {
        pResult = aIt->second;
    }

    return pResult;
}

WW8TableInfo::WW8TableInfo()
{
}

WW8TableInfo::~WW8TableInfo()
{
}

WW8TableNodeInfo *
WW8TableInfo::processSwTableByLayout(const SwTable * pTable, RowEndInners_t &rLastRowEnds)
{
    SwTableCellInfo aTableCellInfo(pTable);

    while (aTableCellInfo.getNext())
    {
        SwRect aRect = aTableCellInfo.getRect();

        SAL_INFO( "sw.ww8", "<CellFrame>" );
        SAL_INFO( "sw.ww8", "<rect top=\"" << aRect.Top() << "\" bottom=\"" << aRect.Bottom()
            << "\" left=\"" << aRect.Left() << "\" right=\"" << aRect.Right() << "\"/>" );
        const SwTableBox * pTableBox = aTableCellInfo.getTableBox();
        const SwStartNode * pSttNd = pTableBox->GetSttNd();

        if (pSttNd != nullptr)
        {
            SwPaM aPam(*pSttNd, 0);

            bool bDone = false;
            do
            {
                SwNode & rNode = aPam.GetPoint()->nNode.GetNode();

                insertTableNodeInfo(&rNode, pTable, pTableBox, 0, 0, 1, & aRect);

                if (rNode.IsEndNode())
                {
                    SwEndNode * pEndNode = rNode.GetEndNode();
                    SwStartNode * pTmpSttNd = pEndNode->StartOfSectionNode();

                    if (pTmpSttNd == pSttNd)
                        bDone = true;
                }

                aPam.GetPoint()->nNode++;
            }
            while (!bDone);
        }

        SAL_INFO( "sw.ww8", "</CellFrame>" );
    }

    return reorderByLayout(pTable, rLastRowEnds);
}

void WW8TableInfo::processSwTable(const SwTable * pTable)
{
    SAL_INFO( "sw.ww8", "<processSwTable>" );

    WW8TableNodeInfo * pPrev = nullptr;
    RowEndInners_t aLastRowEnds;

    if (pTable->IsTableComplex() && pTable->HasLayout())
    {
        pPrev = processSwTableByLayout(pTable, aLastRowEnds);
#ifdef DBG_UTIL
        SAL_INFO( "sw.ww8", getCellGridForTable(pTable)->toString());
#endif
    }
    else
    {
        const SwTableLines & rLines = pTable->GetTabLines();

        for (size_t n = 0; n < rLines.size(); ++n)
        {
            const SwTableLine * pLine = rLines[n];

            pPrev = processTableLine(pTable, pLine, static_cast<sal_uInt32>(n), 1, pPrev, aLastRowEnds);
        }

    }

    if (pPrev)
    {
        SwTableNode * pTableNode = pTable->GetTableNode();
        SwEndNode * pEndNode = pTableNode->EndOfSectionNode();
        pPrev->setNextNode(pEndNode);
        assert(!aLastRowEnds.empty());
        for (auto &a : aLastRowEnds)
        {
            assert(a.second->isEndOfLine());
            a.second->setFinalEndOfLine(true);
        }
    }
    SAL_INFO( "sw.ww8", "</processSwTable>" );
}

WW8TableNodeInfo *
WW8TableInfo::processTableLine(const SwTable * pTable,
                               const SwTableLine * pTableLine,
                               sal_uInt32 nRow,
                               sal_uInt32 nDepth,
                               WW8TableNodeInfo * pPrev,
                               RowEndInners_t &rLastRowEnds)
{
    SAL_INFO( "sw.ww8", "<processTableLine row=\"" << nRow << "\" depth=\"" << nDepth << "\">" );

    const SwTableBoxes & rBoxes = pTableLine->GetTabBoxes();

    WW8TableNodeInfo::Pointer_t pTextNodeInfo;

    for (size_t n = 0; n < rBoxes.size(); ++n)
    {
        const SwTableBox * pBox = rBoxes[n].get();

        pPrev = processTableBox(pTable, pBox, nRow, static_cast<sal_uInt32>(n), nDepth, n == rBoxes.size() - 1, pPrev, rLastRowEnds);
    }

    SAL_INFO( "sw.ww8", "</processTableLine>" );

    return pPrev;
}

WW8TableNodeInfo::Pointer_t
WW8TableInfo::processTableBoxLines(const SwTableBox * pBox,
                                   const SwTable * pTable,
                                   const SwTableBox * pBoxToSet,
                                   sal_uInt32 nRow,
                                   sal_uInt32 nCell,
                                   sal_uInt32 nDepth)
{
    SAL_INFO( "sw.ww8", "<processTableBoxLines depth=\"" << nDepth << "\" row=\"" << nRow
        << "\" cell=\"" << nCell << "\">" );

    const SwTableLines & rLines = pBox->GetTabLines();
    WW8TableNodeInfo::Pointer_t pNodeInfo;

    if (!rLines.empty())
    {
        for (size_t n = 0; n < rLines.size(); ++n)
        {
            const SwTableLine * pLine = rLines[n];
            const SwTableBoxes & rBoxes = pLine->GetTabBoxes();

            for (size_t nBox = 0; nBox < rBoxes.size(); ++nBox)
                pNodeInfo = processTableBoxLines(rBoxes[nBox].get(), pTable, pBoxToSet, nRow, nCell, nDepth);
        }
    }
    else
    {
        const SwStartNode * pSttNd = pBox->GetSttNd();
        const SwEndNode * pEndNd = pSttNd->EndOfSectionNode();
        SwPaM aPaM(*pSttNd, 0);
        SwPaM aEndPaM(*pEndNd, 0);

        bool bDone = false;
        while (!bDone)
        {
            SwNode & rNode = aPaM.GetPoint()->nNode.GetNode();

            pNodeInfo = insertTableNodeInfo(&rNode, pTable, pBoxToSet, nRow, nCell, nDepth);

            if (aPaM.GetPoint()->nNode == aEndPaM.GetPoint()->nNode)
                bDone = true;
            else
                aPaM.GetPoint()->nNode++;
        }
    }

    SAL_INFO( "sw.ww8", "</processTableBoxLines>" );

    return pNodeInfo;
}

void updateFinalEndOfLine(RowEndInners_t &rLastRowEnds, WW8TableNodeInfo const * pEndOfCellInfo)
{
    sal_Int32 nDepth = pEndOfCellInfo->getDepth();
    WW8TableNodeInfoInner::Pointer_t pInner = pEndOfCellInfo->getInnerForDepth(nDepth);

    auto aIt = rLastRowEnds.find(nDepth);
    if (aIt == rLastRowEnds.end() || (pInner->getRow() > aIt->second->getRow()))
        rLastRowEnds[nDepth] = pInner.get();
}

WW8TableNodeInfo *
WW8TableInfo::processTableBox(const SwTable * pTable,
                              const SwTableBox * pBox,
                              sal_uInt32 nRow,
                              sal_uInt32 nCell,
                              sal_uInt32 nDepth,
                              bool bEndOfLine,
                              WW8TableNodeInfo * pPrev,
                              RowEndInners_t &rLastRowEnds)
{
    SAL_INFO( "sw.ww8", "<processTableBox row=\"" << nRow << "\" cell=\"" << nCell
        << "\" depth=\"" << nDepth << "\">" );

    WW8TableNodeInfo::Pointer_t pNodeInfo;
    const SwTableLines & rLines = pBox->GetTabLines();
    const SwStartNode * pSttNd = pBox->GetSttNd();
    WW8TableNodeInfo::Pointer_t pEndOfCellInfo;

    if (!rLines.empty())
    {
        pNodeInfo = processTableBoxLines(pBox, pTable, pBox, nRow, nCell, nDepth);
        pNodeInfo->setEndOfCell(true);
        if (bEndOfLine)
        {
            pNodeInfo->setEndOfLine(true);
            updateFinalEndOfLine(rLastRowEnds, pNodeInfo.get());
        }

        for (size_t n = 0; n < rLines.size(); n++)
        {
            const SwTableLine * pLine = rLines[n];

            pPrev = processTableLine(pTable, pLine, n, 1, pPrev, rLastRowEnds);
        }
    }
    else
    {
        SwPaM aPaM(*pSttNd, 0);

        bool bDone = false;
        sal_uInt32 nDepthInsideCell = 0;

        do
        {
            SwNode & rNode = aPaM.GetPoint()->nNode.GetNode();

            if (rNode.IsStartNode())
            {
                if (nDepthInsideCell > 0)
                    pEndOfCellInfo.reset();

                nDepthInsideCell++;
            }

            pNodeInfo = insertTableNodeInfo(&rNode, pTable, pBox, nRow, nCell, nDepth);

            if (pPrev)
                pPrev->setNext(pNodeInfo.get());

            pPrev = pNodeInfo.get();

            if (nDepthInsideCell == 1 && rNode.IsTextNode())
                pEndOfCellInfo = pNodeInfo;

            if (rNode.IsEndNode())
            {
                nDepthInsideCell--;

                if (nDepthInsideCell == 0 && !pEndOfCellInfo)
                    pEndOfCellInfo = pNodeInfo;

                SwEndNode * pEndNode = rNode.GetEndNode( );
                SwStartNode * pTmpSttNd = pEndNode->StartOfSectionNode();
                if (pTmpSttNd == pSttNd)
                    bDone = true;
            }

            aPaM.GetPoint()->nNode++;
        }
        while (!bDone);

        if (pEndOfCellInfo.get() != nullptr)
        {
            pEndOfCellInfo->setEndOfCell(true);

            if (bEndOfLine)
            {
                pEndOfCellInfo->setEndOfLine(true);
                updateFinalEndOfLine(rLastRowEnds, pEndOfCellInfo.get());
            }
        }
    }

    SAL_INFO( "sw.ww8", "</processTableBox>" );

    return pPrev;
}

WW8TableNodeInfo::Pointer_t WW8TableInfo::insertTableNodeInfo
(const SwNode * pNode,
 const SwTable * pTable,
 const SwTableBox * pTableBox,
 sal_uInt32 nRow,
 sal_uInt32 nCell,
 sal_uInt32 nDepth,
 SwRect const * pRect)
{
    WW8TableNodeInfo::Pointer_t pNodeInfo = getTableNodeInfo(pNode);

    if (pNodeInfo.get() == nullptr)
    {
        pNodeInfo =
            std::make_shared<ww8::WW8TableNodeInfo>(this, pNode);
        mMap.emplace(pNode, pNodeInfo);
    }

    pNodeInfo->setDepth(nDepth + pNodeInfo->getDepth());

    pNodeInfo->setTable(pTable);
    pNodeInfo->setTableBox(pTableBox);

    pNodeInfo->setCell(nCell);
    pNodeInfo->setRow(nRow);

    if (pNode->IsTextNode())
    {
        FirstInTableMap_t::const_iterator aIt = mFirstInTableMap.find(pTable);
        if (aIt == mFirstInTableMap.end())
        {
            mFirstInTableMap[pTable] = pNode;
            pNodeInfo->setFirstInTable(true);
        }
    }

    if (pRect)
    {
        WW8TableCellGrid::Pointer_t pCellGrid = getCellGridForTable(pTable);

        pCellGrid->insert(*pRect, pNodeInfo.get());
        pNodeInfo->setRect(*pRect);
    }

#ifdef DBG_UTIL
    SAL_INFO( "sw.ww8", pNodeInfo->toString());
#endif
    return pNodeInfo;
}

WW8TableCellGrid::Pointer_t WW8TableInfo::getCellGridForTable
(const SwTable * pTable, bool bCreate)
{
    WW8TableCellGrid::Pointer_t pResult;
    CellGridMap_t::iterator aIt = mCellGridMap.find(pTable);

    if (aIt == mCellGridMap.end())
    {
        if (bCreate)
        {
            pResult = std::make_shared<ww8::WW8TableCellGrid>();
            mCellGridMap[pTable] = pResult;
        }
    }
    else
        pResult = mCellGridMap[pTable];

    return pResult;
}

WW8TableNodeInfo::Pointer_t WW8TableInfo::getTableNodeInfo
(const SwNode * pNode)
{
    WW8TableNodeInfo::Pointer_t pResult;
    Map_t::iterator aIt = mMap.find(pNode);

    if (aIt != mMap.end())
        pResult = (*aIt).second;

    return pResult;
}

const SwNode * WW8TableInfo::getNextNode(const SwNode * pNode)
{
    const SwNode * pResult = nullptr;

    WW8TableNodeInfo::Pointer_t pNodeInfo = getTableNodeInfo(pNode);

    if (pNodeInfo.get() != nullptr)
    {
        WW8TableNodeInfo * pNextInfo = pNodeInfo->getNext();

        if (pNextInfo != nullptr)
            pResult = pNextInfo->getNode();
        else
        {
            const SwNode * pNextNode = pNodeInfo->getNextNode();

            if (pNextNode != nullptr)
                pResult = pNextNode;
        }
    }

    return pResult;
}

bool WW8TableNodeInfo::operator < (const WW8TableNodeInfo & rInfo) const
{
    bool bRet = false;

    if (rInfo.mpNode != nullptr)
    {
        if (mpNode == nullptr)
        {
            bRet = true;
        }
        else
        {
            if (mpNode->GetIndex() < rInfo.mpNode->GetIndex())
                bRet = true;
        }
    }

    return bRet;
}

bool CellInfo::operator < (const CellInfo & aCellInfo) const
{
    bool aRet = false;

    if (top() < aCellInfo.top())
        aRet = true;
    else if (top() == aCellInfo.top())
    {
        if (left() < aCellInfo.left())
            aRet = true;
        else if (left() == aCellInfo.left())
        {
            if (width() < aCellInfo.width())
                aRet = true;
            else if (width() == aCellInfo.width())
            {
                if (height() < aCellInfo.height())
                    aRet = true;
                else if (height() == aCellInfo.height())
                {
                    if (aCellInfo.getTableNodeInfo())
                    {
                        if (m_pNodeInfo == nullptr)
                            aRet = true;
                        else
                        {
                            aRet = *m_pNodeInfo < *aCellInfo.getTableNodeInfo();
                        }
                    }
                }
            }
        }
    }

    return aRet;
}

#ifdef DBG_UTIL
std::string CellInfo::toString() const
{
    static char sBuffer[256];

    snprintf(sBuffer, sizeof(sBuffer),
             "<cellinfo left=\"%ld\""
             " right=\"%ld\""
             " top=\"%ld\""
             " bottom=\"%ld\""
             " node=\"%p\"/>",
             left(),
             right(),
             top(),
             bottom(),
             m_pNodeInfo);

    return sBuffer;
}
#endif

WW8TableNodeInfo * WW8TableInfo::reorderByLayout(const SwTable * pTable, RowEndInners_t &rLastRowEnds)
{
    WW8TableCellGrid::Pointer_t pCellGrid = getCellGridForTable(pTable);

#ifdef DBG_UTIL
    SAL_INFO( "sw.ww8", pCellGrid->toString());
#endif

    pCellGrid->addShadowCells();
    return pCellGrid->connectCells(rLastRowEnds);
}

WW8TableCellGrid::WW8TableCellGrid()
{
}

WW8TableCellGrid::~WW8TableCellGrid()
{
}

WW8TableCellGridRow::Pointer_t WW8TableCellGrid::getRow(long nTop, bool bCreate)
{
    WW8TableCellGridRow::Pointer_t pResult;

    RowTops_t::iterator aIt = m_aRowTops.find(nTop);

    if (aIt == m_aRowTops.end())
    {
        if (bCreate)
        {
            pResult = std::make_shared<ww8::WW8TableCellGridRow>();
            m_aRows[nTop] = pResult;
            m_aRowTops.insert(nTop);
        }
    }
    else
        pResult = m_aRows[nTop];

    return pResult;
}

WW8TableCellGrid::RowTops_t::const_iterator WW8TableCellGrid::getRowTopsBegin() const
{
    return m_aRowTops.begin();
}

WW8TableCellGrid::RowTops_t::const_iterator WW8TableCellGrid::getRowTopsEnd() const
{
    return m_aRowTops.end();
}

CellInfoMultiSet::const_iterator WW8TableCellGrid::getCellsBegin(long nTop)
{
    return getRow(nTop)->begin();
}

CellInfoMultiSet::const_iterator WW8TableCellGrid::getCellsEnd(long nTop)
{
    return getRow(nTop)->end();
}

void WW8TableCellGrid::insert(const SwRect & rRect,
                              WW8TableNodeInfo * pNodeInfo,
                              const unsigned long * pFormatFrameWidth)
{
    CellInfo aCellInfo(rRect, pNodeInfo);

    if (pFormatFrameWidth != nullptr)
        aCellInfo.setFormatFrameWidth(*pFormatFrameWidth);

    WW8TableCellGridRow::Pointer_t pRow = getRow(rRect.Top());
    pRow->insert(aCellInfo);
}

void WW8TableCellGrid::addShadowCells()
{
    SAL_INFO( "sw.ww8", "<addShadowCells>" );

    RowTops_t::const_iterator aTopsIt = getRowTopsBegin();

    while (aTopsIt != getRowTopsEnd())
    {
        CellInfoMultiSet::const_iterator aCellIt = getCellsBegin(*aTopsIt);
        CellInfoMultiSet::const_iterator aCellEndIt = getCellsEnd(*aTopsIt);

        RowSpansPtr pRowSpans(new RowSpans);

        bool bBeginningOfCell = true;
        bool bVertMerge = false;
        SwRect aRect = aCellIt->getRect();
        long nRowSpan = 1;
        while (aCellIt != aCellEndIt)
        {
            WW8TableNodeInfo * pNodeInfo = aCellIt->getTableNodeInfo();

            if (bBeginningOfCell)
            {
                RowTops_t::const_iterator aRowSpanIt(aTopsIt);
                ++aRowSpanIt;

                if (aRowSpanIt != getRowTopsEnd() &&
                    *aRowSpanIt < aCellIt->bottom())
                {
                    aRect.Top(*aRowSpanIt);
                    unsigned long nFormatFrameWidth = aCellIt->getFormatFrameWidth();
                    insert(aRect, nullptr, &nFormatFrameWidth);

                    bVertMerge = true;
                }
                else
                    bVertMerge = false;

                nRowSpan = 1;
                while (aRowSpanIt != getRowTopsEnd() &&
                       *aRowSpanIt < aCellIt->bottom())
                {
                    ++aRowSpanIt;
                    nRowSpan++;
                }

                if (pNodeInfo)
                    pRowSpans->push_back(nRowSpan);
                else
                    pRowSpans->push_back(-nRowSpan);
            }

            if (pNodeInfo)
            {
                pNodeInfo->setVertMerge(bVertMerge);
            }

            ++aCellIt;
            if (aCellIt != aCellEndIt)
            {
                bBeginningOfCell = (aRect.Left() != aCellIt->left());
                aRect = aCellIt->getRect();
            }
        }

        WW8TableCellGridRow::Pointer_t pRow = getRow(*aTopsIt);
        if (pRow.get() != nullptr)
            pRow->setRowSpans(pRowSpans);

        ++aTopsIt;
    }
    SAL_INFO( "sw.ww8", "</addShadowCells>" );
}

WW8TableNodeInfo * WW8TableCellGrid::connectCells(RowEndInners_t &rLastRowEnds)
{
    RowTops_t::const_iterator aTopsIt = getRowTopsBegin();
    sal_uInt32 nRow = 0;
    WW8TableNodeInfo * pLastNodeInfo = nullptr;

    while (aTopsIt != getRowTopsEnd())
    {
        CellInfoMultiSet::const_iterator aCellIt = getCellsBegin(*aTopsIt);
        CellInfoMultiSet::const_iterator aCellEndIt = getCellsEnd(*aTopsIt);
        GridColsPtr pWidths(new Widths);
        TableBoxVectorPtr pTableBoxes(new TableBoxVector);

        sal_uInt32 nShadows = 0;
        sal_uInt32 nCell = 0;
        bool bBeginningOfCell = true;
        WW8TableNodeInfo * pEndOfCellInfo = nullptr;
        sal_uInt32 nDepthInCell = 0;
        while (aCellIt != aCellEndIt)
        {
            long nCellX = aCellIt->left();
            WW8TableNodeInfo * pNodeInfo = aCellIt->getTableNodeInfo();
            if (pNodeInfo)
            {
                const SwNode * pNode = pNodeInfo->getNode();

                if (pNode->IsStartNode())
                {
                    nDepthInCell++;
                    pEndOfCellInfo = nullptr;
                }

                if (nDepthInCell == 1 && pNode->IsTextNode())
                    pEndOfCellInfo = pNodeInfo;

                pNodeInfo->setShadowsBefore(nShadows);
                pNodeInfo->setCell(nCell);
                pNodeInfo->setRow(nRow);
                if (pLastNodeInfo)
                {
                    pLastNodeInfo->setNext(pNodeInfo);
                    pLastNodeInfo->setNextNode(pNode);
                }
                pLastNodeInfo = pNodeInfo;
                nShadows = 0;

                if (pNode->IsEndNode())
                {
                    nDepthInCell--;

                    if (nDepthInCell == 0 && !pEndOfCellInfo)
                        pEndOfCellInfo = pNodeInfo;
                }
            }
            else
            {
                nShadows++;
            }

            if (bBeginningOfCell)
            {
                pWidths->push_back(aCellIt->getFormatFrameWidth());

                if (pNodeInfo)
                    pTableBoxes->push_back(pNodeInfo->getTableBox());
                else
                    pTableBoxes->push_back(nullptr);
            }

            ++aCellIt;
            bBeginningOfCell = false;

            if (aCellIt != aCellEndIt && aCellIt->left() != nCellX)
            {
                nCell++;
                bBeginningOfCell = true;

                if (pEndOfCellInfo)
                {
                    pEndOfCellInfo->setEndOfCell(true);
                }

                pEndOfCellInfo = nullptr;
            }
        }

        pLastNodeInfo->setShadowsAfter(nShadows);

        if (!pEndOfCellInfo)
        {
            pEndOfCellInfo = pLastNodeInfo;
        }

        pEndOfCellInfo->setEndOfCell(true);
        pLastNodeInfo->setEndOfLine(true);
        updateFinalEndOfLine(rLastRowEnds, pLastNodeInfo);

        WW8TableCellGridRow::Pointer_t pRow(getRow(*aTopsIt));
        pRow->setTableBoxVector(pTableBoxes);
        pRow->setWidths(pWidths);

        ++aTopsIt;
        nRow++;
    }

    return pLastNodeInfo;
}

#ifdef DBG_UTIL
std::string WW8TableCellGrid::toString()
{
    std::string sResult = "<WW8TableCellGrid>";

    RowTops_t::const_iterator aTopsIt = getRowTopsBegin();
    static char sBuffer[1024];
    while (aTopsIt != getRowTopsEnd())
    {
        sprintf(sBuffer, "<row y=\"%ld\">", *aTopsIt);
        sResult += sBuffer;

        CellInfoMultiSet::const_iterator aCellIt = getCellsBegin(*aTopsIt);
        CellInfoMultiSet::const_iterator aCellsEnd = getCellsEnd(*aTopsIt);

        while (aCellIt != aCellsEnd)
        {
            snprintf(sBuffer, sizeof(sBuffer), "<cellInfo top=\"%ld\" bottom=\"%ld\" left=\"%ld\" right=\"%ld\">",
                     aCellIt->top(), aCellIt->bottom(), aCellIt->left(), aCellIt->right());
            sResult += sBuffer;

            WW8TableNodeInfo * pInfo = aCellIt->getTableNodeInfo();
            if (pInfo)
                sResult += pInfo->toString();
            else
                sResult += "<shadow/>\n";

            sResult += "</cellInfo>\n";
            ++aCellIt;
        }

        WW8TableCellGridRow::Pointer_t pRow = getRow(*aTopsIt);
        WidthsPtr pWidths = pRow->getWidths();
        if (pWidths != nullptr)
        {
            sResult += "<widths>";

            Widths::const_iterator aItEnd = pWidths->end();
            for (Widths::const_iterator aIt = pWidths->begin();
                 aIt != aItEnd;
                 ++aIt)
            {
                if (aIt != pWidths->begin())
                    sResult += ", ";

                snprintf(sBuffer, sizeof(sBuffer), "%" SAL_PRIxUINT32 "", *aIt);
                sResult += sBuffer;
            }

            sResult += "</widths>";
        }

        RowSpansPtr pRowSpans = pRow->getRowSpans();
        if (pRowSpans.get() != nullptr)
        {
            sResult += "<rowspans>";

            RowSpans::const_iterator aItEnd = pRowSpans->end();
            for (RowSpans::const_iterator aIt = pRowSpans->begin();
                 aIt != aItEnd;
                 ++aIt)
            {
                if (aIt != pRowSpans->begin())
                    sResult += ", ";

                snprintf(sBuffer, sizeof(sBuffer), "%" SAL_PRIxUINT32 "", *aIt);
                sResult += sBuffer;
            }

            sResult += "</rowspans>";
        }

        sResult += "</row>\n";
        ++aTopsIt;
    }

    sResult += "</WW8TableCellGrid>\n";

    return sResult;
}
#endif

TableBoxVectorPtr WW8TableCellGrid::getTableBoxesOfRow
(WW8TableNodeInfoInner const * pNodeInfoInner)
{
    TableBoxVectorPtr pResult;
    WW8TableCellGridRow::Pointer_t pRow =
        getRow(pNodeInfoInner->getRect().Top(), false);

    if (pRow.get() != nullptr)
    {
        pResult = pRow->getTableBoxVector();
    }

    return pResult;
}

WidthsPtr WW8TableCellGrid::getWidthsOfRow
(WW8TableNodeInfoInner const * pNodeInfoInner)
{
    GridColsPtr pResult;

    WW8TableCellGridRow::Pointer_t pRow =
        getRow(pNodeInfoInner->getRect().Top(), false);

    if (pRow.get() != nullptr)
    {
        pResult = pRow->getWidths();
    }

    return pResult;
}

RowSpansPtr WW8TableCellGrid::getRowSpansOfRow
(WW8TableNodeInfoInner const * pNodeInfoInner)
{
    RowSpansPtr pResult;

    WW8TableCellGridRow::Pointer_t pRow =
        getRow(pNodeInfoInner->getRect().Top(), false);

    if (pRow.get() != nullptr)
    {
        pResult = pRow->getRowSpans();
    }

    return pResult;
}

WW8TableCellGridRow::WW8TableCellGridRow()
: m_pCellInfos(new CellInfoMultiSet)
{
}

WW8TableCellGridRow::~WW8TableCellGridRow()
{
}

void WW8TableCellGridRow::insert(const CellInfo & rCellInfo)
{
    m_pCellInfos->insert(rCellInfo);

#ifdef DBG_UTIL
    SAL_INFO( "sw.ww8", "<gridRowInsert>" << rCellInfo.toString() << "</gridRowInsert>" );
#endif
}

CellInfoMultiSet::const_iterator WW8TableCellGridRow::begin() const
{
    return m_pCellInfos->begin();
}

CellInfoMultiSet::const_iterator WW8TableCellGridRow::end() const
{
    return m_pCellInfos->end();
}

void WW8TableCellGridRow::setTableBoxVector(TableBoxVectorPtr const & pTableBoxVector)
{
    if (pTableBoxVector->size() > MAXTABLECELLS)
        pTableBoxVector->resize(MAXTABLECELLS);
    m_pTableBoxVector = pTableBoxVector;
}

void WW8TableCellGridRow::setWidths(WidthsPtr const & pWidths)
{
    m_pWidths = pWidths;
}

void WW8TableCellGridRow::setRowSpans(RowSpansPtr const & pRowSpans)
{
    m_pRowSpans = pRowSpans;
}


CellInfo::CellInfo(const SwRect & aRect, WW8TableNodeInfo * pNodeInfo)
: m_aRect(aRect), m_pNodeInfo(pNodeInfo), m_nFormatFrameWidth(0)
{
    if (pNodeInfo != nullptr)
    {
        const SwTableBox * pBox = pNodeInfo->getTableBox();
        const SwFrameFormat * pFrameFormat = pBox->GetFrameFormat();
        const SwFormatFrameSize & rSize = pFrameFormat->GetFrameSize();

        m_nFormatFrameWidth = rSize.GetWidth();
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
