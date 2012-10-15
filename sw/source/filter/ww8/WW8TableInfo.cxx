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


#include <iostream>
#include <set>
#include <stdio.h>
#include "WW8TableInfo.hxx"
#include "fmtfsize.hxx"
#include "attributeoutputbase.hxx"
#include "swtable.hxx"
#include "frmfmt.hxx"
#include "pam.hxx"
#include "ndtxt.hxx"
#include "dbgoutsw.hxx"

namespace ww8
{

// WW8TableNodeInfoInner

WW8TableNodeInfoInner::WW8TableNodeInfoInner(WW8TableNodeInfo * pParent)
: mpParent(pParent)
, mnCell(0)
, mnRow(0)
, mnShadowsBefore(0)
, mnShadowsAfter(0)
, mbEndOfLine(false)
, mbEndOfCell(false)
, mbFirstInTable(false)
, mbVertMerge(false)
, mpTableBox(NULL)
, mpTable(NULL)
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

sal_uInt32 WW8TableNodeInfoInner::getDepth() const
{
    return mnDepth;
}

sal_uInt32 WW8TableNodeInfoInner::getCell() const
{
    return mnCell;
}

sal_uInt32 WW8TableNodeInfoInner::getRow() const
{
    return mnRow;
}

sal_uInt32 WW8TableNodeInfoInner::getShadowsBefore() const
{
    return mnShadowsBefore;
}

sal_uInt32 WW8TableNodeInfoInner::getShadowsAfter() const
{
    return mnShadowsAfter;
}

bool WW8TableNodeInfoInner::isEndOfCell() const
{
    return mbEndOfCell;
}

bool WW8TableNodeInfoInner::isEndOfLine() const
{
    return mbEndOfLine;
}

bool WW8TableNodeInfoInner::isFirstInTable() const
{
    return mbFirstInTable;
}

const SwNode * WW8TableNodeInfoInner::getNode() const
{
    const SwNode * pResult = NULL;

    if (mpParent != NULL)
        pResult = mpParent->getNode();

    return pResult;
}

TableBoxVectorPtr WW8TableNodeInfoInner::getTableBoxesOfRow()
{
    TableBoxVectorPtr pResult(new TableBoxVector);

    WW8TableCellGrid::Pointer_t pCellGrid =
        mpParent->getParent()->getCellGridForTable(getTable(), false);

    if (pCellGrid.get() == NULL)
    {
        const SwTableLine * pTabLine = getTableBox()->GetUpper();
        const SwTableBoxes & rTblBoxes = pTabLine->GetTabBoxes();

        sal_uInt8 nBoxes = rTblBoxes.size();
        for ( sal_uInt8 n = 0; n < nBoxes; n++ )
        {
            pResult->push_back(rTblBoxes[n]);
        }
    }
    else
        pResult = pCellGrid->getTableBoxesOfRow(this);

    return pResult;
}

GridColsPtr WW8TableNodeInfoInner::getGridColsOfRow(AttributeOutputBase & rBase)
{
    GridColsPtr pResult(new GridCols);
    WidthsPtr pWidths(getWidthsOfRow());

    const SwFrmFmt *pFmt = getTable()->GetFrmFmt();
    OSL_ENSURE(pFmt,"Impossible");
    if (!pFmt)
        return pResult;

    const SwFmtFrmSize &rSize = pFmt->GetFrmSize();
    unsigned long nTblSz = static_cast<unsigned long>(rSize.GetWidth());

    sal_uInt32 nPageSize = 0;
    bool bRelBoxSize = false;

    rBase.GetTablePageSize
        ( this, nPageSize, bRelBoxSize );

    SwTwips nSz = 0;
    Widths::const_iterator aWidthsEnd = pWidths->end();
    for ( Widths::const_iterator aIt = pWidths->begin();
          aIt != aWidthsEnd;
          ++aIt)
    {
        nSz += *aIt;
        SwTwips nCalc = nSz;
        if ( bRelBoxSize )
            nCalc = ( nCalc * nPageSize ) / nTblSz;

        pResult->push_back( nCalc );
    }

    return pResult;
}

WidthsPtr WW8TableNodeInfoInner::getWidthsOfRow()
{
    WidthsPtr pWidths;

    WW8TableCellGrid::Pointer_t pCellGrid =
        mpParent->getParent()->getCellGridForTable(getTable(), false);

    if (pCellGrid.get() == NULL)
    {
        const SwTableBox * pTabBox = getTableBox();
        const SwTableLine * pTabLine = pTabBox->GetUpper();
        const SwTableBoxes & rTabBoxes = pTabLine->GetTabBoxes();

        pWidths = WidthsPtr(new Widths);
        // number of cell written
        sal_uInt32 nBoxes = rTabBoxes.size();
        if ( nBoxes > MAXTABLECELLS )
            nBoxes = MAXTABLECELLS;

        for (sal_uInt32 n = 0; n < nBoxes; n++)
        {
            const SwFrmFmt* pBoxFmt = rTabBoxes[ n ]->GetFrmFmt();
            const SwFmtFrmSize& rLSz = pBoxFmt->GetFrmSize();

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

    if (pCellGrid.get() == NULL)
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

const SwTableBox * WW8TableNodeInfoInner::getTableBox() const
{
    return mpTableBox;
}

const SwTable * WW8TableNodeInfoInner::getTable() const
{
    return mpTable;
}

const SwRect & WW8TableNodeInfoInner::getRect() const
{
    return maRect;
}

#ifdef DBG_UTIL
::std::string WW8TableNodeInfoInner::toString() const
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

    return string(buffer);
}
#endif

WW8TableNodeInfo::WW8TableNodeInfo(WW8TableInfo * pParent,
                                   const SwNode * pNode)
: mpParent(pParent),
  mnDepth(0),
  mpNode(pNode),
  mpNext(NULL),
  mpNextNode(NULL)
{
}

WW8TableNodeInfo::~WW8TableNodeInfo()
{
}

#ifdef DBG_UTIL
::std::string WW8TableNodeInfo::toString() const
{
    static char buffer[1024];
    snprintf(buffer, sizeof(buffer),
             "<tableNodeInfo p=\"%p\" depth=\"%" SAL_PRIuUINT32 "\">"
             ,this, getDepth());

    ::std::string sResult(buffer);

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
        mInners[mnDepth] = WW8TableNodeInfoInner::Pointer_t(new WW8TableNodeInfoInner(this));

    mInners[mnDepth]->setDepth(mnDepth);
}

void WW8TableNodeInfo::setEndOfLine(bool bEndOfLine)
{
    WW8TableNodeInfoInner::Pointer_t pInner = getInnerForDepth(mnDepth);
    pInner->setEndOfLine(bEndOfLine);

#ifdef DBG_UTIL
    ::std::clog << "<endOfLine depth=\"" << mnDepth << "\">"
    << toString() << "</endOfLine>" << ::std::endl;
#endif
}

void WW8TableNodeInfo::setEndOfCell(bool bEndOfCell)
{
    WW8TableNodeInfoInner::Pointer_t pInner = getInnerForDepth(mnDepth);
    pInner->setEndOfCell(bEndOfCell);

#ifdef DBG_UTIL
    ::std::clog << "<endOfCell depth=\"" << mnDepth << "\">"
    << toString() << "</endOfCell>" << ::std::endl;
#endif
}

void WW8TableNodeInfo::setFirstInTable(bool bFirstInTable)
{
    WW8TableNodeInfoInner::Pointer_t pInner = getInnerForDepth(mnDepth);

    pInner->setFirstInTable(bFirstInTable);

#ifdef DBG_UTIL
    ::std::clog << "<firstInTable depth=\"" << mnDepth << "\">"
    << toString() << "</firstInTable>" << ::std::endl;
#endif
}

void WW8TableNodeInfo::setVertMerge(bool bVertMerge)
{
    WW8TableNodeInfoInner::Pointer_t pInner = getInnerForDepth(mnDepth);

    pInner->setVertMerge(bVertMerge);


#ifdef DBG_UTIL
    ::std::clog << "<vertMerge depth=\"" << mnDepth << "\">"
    << toString() << "</vertMerge>" << ::std::endl;
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
    ::std::clog << "<setnext><from>" << toString() << "</from><to>"
                << pNext->toString() << "</to></setnext>"
                << ::std::endl;
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

WW8TableInfo * WW8TableNodeInfo::getParent() const
{
    return mpParent;
}

sal_uInt32 WW8TableNodeInfo::getDepth() const
{
    if (!mInners.empty())
        return mInners.begin()->second->getDepth();

    return mnDepth;
}

const SwNode * WW8TableNodeInfo::getNode() const
{
    return mpNode;
}

const SwTableBox * WW8TableNodeInfo::getTableBox() const
{
    return getInnerForDepth(mnDepth)->getTableBox();
}

WW8TableNodeInfo * WW8TableNodeInfo::getNext() const
{
    return mpNext;
}

const SwNode * WW8TableNodeInfo::getNextNode() const
{
    return mpNextNode;
}

sal_uInt32 WW8TableNodeInfo::getCell() const
{
    return getInnerForDepth(mnDepth)->getCell();
}

sal_uInt32 WW8TableNodeInfo::getRow() const
{
    return getInnerForDepth(mnDepth)->getRow();
}

const ww8::WW8TableNodeInfo::Inners_t & WW8TableNodeInfo::getInners() const
{
    return mInners;
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

// WW8TableInfo

WW8TableInfo::WW8TableInfo()
{
}

WW8TableInfo::~WW8TableInfo()
{
}

WW8TableNodeInfo *
WW8TableInfo::processSwTableByLayout(const SwTable * pTable)
{
    SwTableCellInfo aTableCellInfo(pTable);
    WW8TableNodeInfo * pPrev = NULL;

    while (aTableCellInfo.getNext())
    {
        SwRect aRect = aTableCellInfo.getRect();

#ifdef DBG_UTIL
        static char sBuffer[1024];
        ::std::clog << "<CellFrm>" << ::std::endl;

        snprintf(sBuffer, sizeof(sBuffer),
                 "<rect top=\"%ld\" bottom=\"%ld\" left=\"%ld\" right=\"%ld\"/>",
                 aRect.Top(), aRect.Bottom(), aRect.Left(), aRect.Right());
        ::std::clog << sBuffer << ::std::endl;
#endif
        const SwTableBox * pTableBox = aTableCellInfo.getTableBox();
        const SwStartNode * pSttNd = pTableBox->GetSttNd();

        if (pSttNd != NULL)
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

#ifdef DBG_UTIL
        ::std::clog << "</CellFrm>" << ::std::endl;
#endif
    }

    pPrev = reorderByLayout(pTable);

    return pPrev;
}

void WW8TableInfo::processSwTable(const SwTable * pTable)
{
#ifdef DBG_UTIL
    ::std::clog << "<processSwTable>" << ::std::endl;
#endif

    WW8TableNodeInfo * pPrev = NULL;

    if (pTable->IsTblComplex() && pTable->HasLayout())
    {
        pPrev = processSwTableByLayout(pTable);

#ifdef DBG_UTIL
        WW8TableCellGrid::Pointer_t pCellGrid(getCellGridForTable(pTable));
        ::std::clog << pCellGrid->toString() << ::std::endl;
#endif
    }
    else
    {
        const SwTableLines & rLines = pTable->GetTabLines();

        for (sal_uInt16 n = 0; n < rLines.size(); n++)
        {
            const SwTableLine * pLine = rLines[n];

            pPrev = processTableLine(pTable, pLine, n, 1, pPrev);
        }

    }

    if (pPrev != NULL)
    {
        SwTableNode * pTableNode = pTable->GetTableNode();
        SwEndNode * pEndNode = pTableNode->EndOfSectionNode();

        pPrev->setNextNode(pEndNode);
    }
#ifdef DBG_UTIL
    ::std::clog << "</processSwTable>" << ::std::endl;
#endif
}

WW8TableNodeInfo *
WW8TableInfo::processTableLine(const SwTable * pTable,
                               const SwTableLine * pTableLine,
                               sal_uInt32 nRow,
                               sal_uInt32 nDepth, WW8TableNodeInfo * pPrev)
{
#ifdef DBG_UTIL
    ::std::clog << "<processTableLine row=\"" << nRow << "\" depth=\""
    << nDepth << "\">" << ::std::endl;
#endif

    const SwTableBoxes & rBoxes = pTableLine->GetTabBoxes();

    WW8TableNodeInfo::Pointer_t pTextNodeInfo;

    for (sal_uInt16 n = 0; n < rBoxes.size(); n++)
    {
        const SwTableBox * pBox = rBoxes[n];

        pPrev = processTableBox(pTable, pBox, nRow, n, nDepth, n == rBoxes.size() - 1, pPrev);
    }

#ifdef DBG_UTIL
    ::std::clog << "</processTableLine>" << ::std::endl;
#endif

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
#ifdef DBG_UTIL
    ::std::clog << "<processTableBoxLines depth=\"" << nDepth
    << "\" row=\"" << nRow << "\" cell=\"" << nCell << "\">" << ::std::endl;
#endif

    const SwTableLines & rLines = pBox->GetTabLines();
    WW8TableNodeInfo::Pointer_t pNodeInfo;

    if (!rLines.empty())
    {
        for (sal_uInt32 n = 0; n < rLines.size(); n++)
        {
            const SwTableLine * pLine = rLines[n];
            const SwTableBoxes & rBoxes = pLine->GetTabBoxes();

            for (sal_uInt16 nBox = 0; nBox < rBoxes.size(); nBox++)
                pNodeInfo = processTableBoxLines(rBoxes[nBox], pTable, pBoxToSet, nRow, nCell, nDepth);
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

#ifdef DBG_UTIL
    ::std::clog << "</processTableBoxLines>" << ::std::endl;
#endif

    return pNodeInfo;
}

WW8TableNodeInfo *
WW8TableInfo::processTableBox(const SwTable * pTable,
                              const SwTableBox * pBox,
                              sal_uInt32 nRow,
                              sal_uInt32 nCell,
                              sal_uInt32 nDepth,
                              bool bEndOfLine,
                              WW8TableNodeInfo * pPrev)
{
#ifdef DBG_UTIL
    ::std::clog << "<processTableBox row=\"" << nRow << "\" cell=\"" << nCell
    << "\" depth=\"" << nDepth << "\">" << ::std::endl;
#endif

    WW8TableNodeInfo::Pointer_t pNodeInfo;
    const SwTableLines & rLines = pBox->GetTabLines();
    const SwStartNode * pSttNd = pBox->GetSttNd();
    WW8TableNodeInfo::Pointer_t pEndOfCellInfo;

    if (!rLines.empty())
    {
        pNodeInfo = processTableBoxLines(pBox, pTable, pBox, nRow, nCell, nDepth);
        pNodeInfo->setEndOfCell(true);
        if (bEndOfLine)
            pNodeInfo->setEndOfLine(true);

        for (sal_uInt32 n = 0; n < rLines.size(); n++)
        {
            const SwTableLine * pLine = rLines[n];

            pPrev = processTableLine(pTable, pLine, n, 1, pPrev);
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

            if (pPrev != NULL)
                pPrev->setNext(pNodeInfo.get());

            pPrev = pNodeInfo.get();

            if (nDepthInsideCell == 1 && rNode.IsTxtNode())
                pEndOfCellInfo = pNodeInfo;

            if (rNode.IsEndNode())
            {
                nDepthInsideCell--;

                if (nDepthInsideCell == 0 && pEndOfCellInfo.get() == NULL)
                    pEndOfCellInfo = pNodeInfo;

                SwEndNode * pEndNode = rNode.GetEndNode( );
                SwStartNode * pTmpSttNd = pEndNode->StartOfSectionNode();
                if (pTmpSttNd == pSttNd)
                    bDone = true;
            }

            aPaM.GetPoint()->nNode++;
        }
        while (!bDone);

        if (pEndOfCellInfo.get() != NULL)
        {
            pEndOfCellInfo->setEndOfCell(true);

            if (bEndOfLine)
                pEndOfCellInfo->setEndOfLine(true);
        }
    }

#ifdef DBG_UTIL
    ::std::clog << "</processTableBox>" << ::std::endl;
#endif

    return pPrev;
}

WW8TableNodeInfo::Pointer_t WW8TableInfo::insertTableNodeInfo
(const SwNode * pNode,
 const SwTable * pTable,
 const SwTableBox * pTableBox,
 sal_uInt32 nRow,
 sal_uInt32 nCell,
 sal_uInt32 nDepth,
 SwRect * pRect)
{
    WW8TableNodeInfo::Pointer_t pNodeInfo = getTableNodeInfo(pNode);

    if (pNodeInfo.get() == NULL)
    {
        pNodeInfo =
            WW8TableNodeInfo::Pointer_t(new WW8TableNodeInfo(this, pNode));
        mMap.insert(Map_t::value_type(pNode, pNodeInfo));
    }

    pNodeInfo->setDepth(nDepth + pNodeInfo->getDepth());

    pNodeInfo->setTable(pTable);
    pNodeInfo->setTableBox(pTableBox);

    pNodeInfo->setCell(nCell);
    pNodeInfo->setRow(nRow);

    if (pNode->IsTxtNode())
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
    ::std::clog << pNodeInfo->toString() << ::std::endl;
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
            pResult = WW8TableCellGrid::Pointer_t(new WW8TableCellGrid);
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
    const SwNode * pResult = NULL;

    WW8TableNodeInfo::Pointer_t pNodeInfo = getTableNodeInfo(pNode);

    if (pNodeInfo.get() != NULL)
    {
        WW8TableNodeInfo * pNextInfo = pNodeInfo->getNext();

        if (pNextInfo != NULL)
            pResult = pNextInfo->getNode();
        else
        {
            const SwNode * pNextNode = pNodeInfo->getNextNode();

            if (pNextNode != NULL)
                pResult = pNextNode;
        }
    }

    return pResult;
}

bool WW8TableNodeInfo::operator < (const WW8TableNodeInfo & rInfo) const
{
    bool bRet = false;

    if (rInfo.mpNode != NULL)
    {
        if (mpNode == NULL)
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
                    if (aCellInfo.getTableNodeInfo() != NULL)
                    {
                        if (m_pNodeInfo == NULL)
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
::std::string CellInfo::toString() const
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

WW8TableNodeInfo * WW8TableInfo::reorderByLayout(const SwTable * pTable)
{
    WW8TableNodeInfo * pPrev = NULL;
    WW8TableCellGrid::Pointer_t pCellGrid = getCellGridForTable(pTable);

#ifdef DBG_UTIL
    ::std::clog << pCellGrid->toString() << ::std::endl;
#endif

    pCellGrid->addShadowCells();
    pPrev = pCellGrid->connectCells();

    return pPrev;
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
            pResult = WW8TableCellGridRow::Pointer_t(new WW8TableCellGridRow);
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
                              unsigned long * pFmtFrmWidth)
{
    CellInfo aCellInfo(rRect, pNodeInfo);

    if (pFmtFrmWidth != NULL)
        aCellInfo.setFmtFrmWidth(*pFmtFrmWidth);

    WW8TableCellGridRow::Pointer_t pRow = getRow(rRect.Top());
    pRow->insert(aCellInfo);
}

void WW8TableCellGrid::addShadowCells()
{
#ifdef DBG_UTIL
    ::std::clog << "<addShadowCells>" << ::std::endl;
#endif

    RowTops_t::const_iterator aTopsIt = getRowTopsBegin();

    while (aTopsIt != getRowTopsEnd())
    {
#ifdef DBG_UTIL
        long nTop = *aTopsIt;
        (void) nTop;
#endif
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
                    unsigned long nFmtFrmWidth = aCellIt->getFmtFrmWidth();
                    insert(aRect, NULL, &nFmtFrmWidth);

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

                if (pNodeInfo != NULL)
                    pRowSpans->push_back(nRowSpan);
                else
                    pRowSpans->push_back(-nRowSpan);
            }

            if (pNodeInfo != NULL)
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
        if (pRow.get() != NULL)
            pRow->setRowSpans(pRowSpans);

        ++aTopsIt;
    }
#ifdef DBG_UTIL
    ::std::clog << "</addShadowCells>" << ::std::endl;
#endif
}

WW8TableNodeInfo * WW8TableCellGrid::connectCells()
{
    RowTops_t::const_iterator aTopsIt = getRowTopsBegin();
    sal_uInt32 nRow = 0;
    WW8TableNodeInfo * pLastNodeInfo = NULL;

    while (aTopsIt != getRowTopsEnd())
    {
        CellInfoMultiSet::const_iterator aCellIt = getCellsBegin(*aTopsIt);
        CellInfoMultiSet::const_iterator aCellEndIt = getCellsEnd(*aTopsIt);
        GridColsPtr pWidths(new Widths);
        TableBoxVectorPtr pTableBoxes(new TableBoxVector);

        sal_uInt32 nShadows = 0;
        sal_uInt32 nCell = 0;
        bool bBeginningOfCell = true;
        WW8TableNodeInfo * pEndOfCellInfo = NULL;
        sal_uInt32 nDepthInCell = 0;
        while (aCellIt != aCellEndIt)
        {
            long nCellX = aCellIt->left();
            WW8TableNodeInfo * pNodeInfo = aCellIt->getTableNodeInfo();
            if (pNodeInfo != NULL)
            {
                const SwNode * pNode = pNodeInfo->getNode();

                if (pNode->IsStartNode())
                {
                    nDepthInCell++;
                    pEndOfCellInfo = NULL;
                }

                if (nDepthInCell == 1 && pNode->IsTxtNode())
                    pEndOfCellInfo = pNodeInfo;

                pNodeInfo->setShadowsBefore(nShadows);
                pNodeInfo->setCell(nCell);
                pNodeInfo->setRow(nRow);
                if (pLastNodeInfo != NULL)
                {
                    pLastNodeInfo->setNext(pNodeInfo);
                    pLastNodeInfo->setNextNode(pNode);
                }
                pLastNodeInfo = pNodeInfo;
                nShadows = 0;

                if (pNode->IsEndNode())
                {
                    nDepthInCell--;

                    if (nDepthInCell == 0 && pEndOfCellInfo == NULL)
                        pEndOfCellInfo = pNodeInfo;
                }
            }
            else
            {
                nShadows++;
            }

            if (bBeginningOfCell)
            {
                pWidths->push_back(aCellIt->getFmtFrmWidth());

                if (pNodeInfo != NULL)
                    pTableBoxes->push_back(pNodeInfo->getTableBox());
                else
                    pTableBoxes->push_back(NULL);
            }

            ++aCellIt;
            bBeginningOfCell = false;

            if (aCellIt != aCellEndIt && aCellIt->left() != nCellX)
            {
                nCell++;
                bBeginningOfCell = true;

                if (pEndOfCellInfo != NULL)
                {
                    pEndOfCellInfo->setEndOfCell(true);
                }

                pEndOfCellInfo = NULL;
            }
        }

        pLastNodeInfo->setShadowsAfter(nShadows);

        if (pEndOfCellInfo == NULL)
        {
            pEndOfCellInfo = pLastNodeInfo;
        }

        pEndOfCellInfo->setEndOfCell(true);
        pLastNodeInfo->setEndOfLine(true);

        WW8TableCellGridRow::Pointer_t pRow(getRow(*aTopsIt));
        pRow->setTableBoxVector(pTableBoxes);
        pRow->setWidths(pWidths);

        nShadows = 0;

        ++aTopsIt;
        nRow++;
    }

    return pLastNodeInfo;
}

#ifdef DBG_UTIL
::std::string WW8TableCellGrid::toString()
{
    string sResult = "<WW8TableCellGrid>";

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
            if (pInfo != NULL)
                sResult += pInfo->toString();
            else
                sResult += "<shadow/>\n";

            sResult += "</cellInfo>\n";
            ++aCellIt;
        }

        WW8TableCellGridRow::Pointer_t pRow = getRow(*aTopsIt);
        WidthsPtr pWidths = pRow->getWidths();
        if (pWidths != NULL)
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
        if (pRowSpans.get() != NULL)
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
(WW8TableNodeInfoInner * pNodeInfoInner)
{
    TableBoxVectorPtr pResult;
    WW8TableCellGridRow::Pointer_t pRow =
        getRow(pNodeInfoInner->getRect().Top(), false);

    if (pRow.get() != NULL)
    {
        pResult = pRow->getTableBoxVector();
    }

    return pResult;
}

WidthsPtr WW8TableCellGrid::getWidthsOfRow
(WW8TableNodeInfoInner * pNodeInfoInner)
{
    GridColsPtr pResult;

    WW8TableCellGridRow::Pointer_t pRow =
        getRow(pNodeInfoInner->getRect().Top(), false);

    if (pRow.get() != NULL)
    {
        pResult = pRow->getWidths();
    }

    return pResult;
}

RowSpansPtr WW8TableCellGrid::getRowSpansOfRow
(WW8TableNodeInfoInner * pNodeInfoInner)
{
    RowSpansPtr pResult;

    WW8TableCellGridRow::Pointer_t pRow =
        getRow(pNodeInfoInner->getRect().Top(), false);

    if (pRow.get() != NULL)
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
    ::std::clog << "<gridRowInsert>"
                << rCellInfo.toString()
                << "</gridRowInsert>"
                << ::std::endl;
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

void WW8TableCellGridRow::setTableBoxVector(TableBoxVectorPtr pTableBoxVector)
{
    m_pTableBoxVector = pTableBoxVector;
}

void WW8TableCellGridRow::setWidths(WidthsPtr pWidths)
{
    m_pWidths = pWidths;
}

void WW8TableCellGridRow::setRowSpans(RowSpansPtr pRowSpans)
{
    m_pRowSpans = pRowSpans;
}

TableBoxVectorPtr WW8TableCellGridRow::getTableBoxVector() const
{
    return m_pTableBoxVector;
}

WidthsPtr WW8TableCellGridRow::getWidths() const
{
    return m_pWidths;
}

RowSpansPtr WW8TableCellGridRow::getRowSpans() const
{
    return m_pRowSpans;
}

CellInfo::CellInfo(const SwRect & aRect, WW8TableNodeInfo * pNodeInfo)
: m_aRect(aRect), m_pNodeInfo(pNodeInfo), m_nFmtFrmWidth(0)
{
    if (pNodeInfo != NULL)
    {
        const SwTableBox * pBox = pNodeInfo->getTableBox();
        const SwFrmFmt * pFrmFmt = pBox->GetFrmFmt();
        const SwFmtFrmSize & rSize = pFrmFmt->GetFrmSize();

        m_nFmtFrmWidth = rSize.GetWidth();
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
