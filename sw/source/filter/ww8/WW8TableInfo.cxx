/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: WW8TableInfo.cxx,v $
 * $Revision: 1.1.2.6 $
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

#include <iostream>
#include <stdio.h>
#include "WW8TableInfo.hxx"
#include "swtable.hxx"
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
, mbEndOfLine(false)
, mbEndOfCell(false)
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

void WW8TableNodeInfoInner::setEndOfLine(bool bEndOfLine)
{
    mbEndOfLine = bEndOfLine;
}

void WW8TableNodeInfoInner::setEndOfCell(bool bEndOfCell)
{
    mbEndOfCell = bEndOfCell;
}

void WW8TableNodeInfoInner::setTableBox(const SwTableBox * pTableBox)
{
    mpTableBox = pTableBox;
}

void WW8TableNodeInfoInner::setTable(const SwTable * pTable)
{
    mpTable = pTable;
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

bool WW8TableNodeInfoInner::isEndOfCell() const
{
    return mbEndOfCell;
}

bool WW8TableNodeInfoInner::isEndOfLine() const
{
    return mbEndOfLine;
}

const SwNode * WW8TableNodeInfoInner::getNode() const
{
    const SwNode * pResult = NULL;

    if (mpParent != NULL)
        pResult = mpParent->getNode();

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

string WW8TableNodeInfoInner::toString() const
{
    static char buffer[256];
    snprintf(buffer, sizeof(buffer),
             "<tableinner depth=\"%" SAL_PRIxUINT32 "\""
             " cell=\"%" SAL_PRIxUINT32 "\""
             " row=\"%" SAL_PRIxUINT32 "\""
             " endOfCell=\"%s\""
             " endOfLine=\"%s\"/>",
             mnDepth, mnCell, mnRow,
             mbEndOfCell ? "yes" : "no",
             mbEndOfLine ? "yes" : "no");

    return string(buffer);
}

// WW8TableTextNodeInfo

WW8TableNodeInfo::WW8TableNodeInfo(const SwNode * pNode)
:
    mnDepth(0),
    mpNode(pNode),
    mpNext(NULL)
{
}

WW8TableNodeInfo::~WW8TableNodeInfo()
{
}

::std::string WW8TableNodeInfo::toString() const
{
    static char buffer[1024];
    snprintf(buffer, sizeof(buffer),
             "<tableNodeInfo depth=\"%" SAL_PRIxUINT32 "\">"
             , getDepth());

    ::std::string sResult(buffer);

    Inners_t::const_iterator aIt(mInners.begin());
    Inners_t::const_iterator aEnd(mInners.end());

    while (aIt != aEnd)
    {
        WW8TableNodeInfoInner::Pointer_t pInner = aIt->second;
        sResult += pInner->toString();

        aIt++;
    }

#ifdef DEBUG
    sResult += dbg_out(*mpNode);
#endif

    sResult +="</tableNodeInfo>";

    return sResult;
}

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

#ifdef DEBUG
    ::std::clog << "<endOfLine depth=\"" << mnDepth << "\">"
    << toString() << "</endOfLine>" << ::std::endl;
#endif
}

void WW8TableNodeInfo::setEndOfCell(bool bEndOfCell)
{
    WW8TableNodeInfoInner::Pointer_t pInner = getInnerForDepth(mnDepth);
    pInner->setEndOfCell(bEndOfCell);

#ifdef DEBUG
    ::std::clog << "<endOfCell depth=\"" << mnDepth << "\">"
    << toString() << "</endOfCell>" << ::std::endl;
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
}

void WW8TableNodeInfo::setCell(sal_uInt32 nCell)
{
    getInnerForDepth(mnDepth)->setCell(nCell);
}

void WW8TableNodeInfo::setRow(sal_uInt32 nRow)
{
    getInnerForDepth(mnDepth)->setRow(nRow);
}

sal_uInt32 WW8TableNodeInfo::getDepth() const
{
    if (mInners.size() > 0)
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

const SwTable * WW8TableNodeInfo::getTable() const
{
    return getInnerForDepth(mnDepth)->getTable();
}

WW8TableNodeInfo * WW8TableNodeInfo::getNext() const
{
    return mpNext;
}

bool WW8TableNodeInfo::isEndOfLine() const
{
    return getInnerForDepth(mnDepth)->isEndOfLine();
}

bool WW8TableNodeInfo::isEndOfCell() const
{
    return getInnerForDepth(mnDepth)->isEndOfCell();
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

    if (mInners.size() > 0)
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

void WW8TableInfo::processSwTable(const SwTable * pTable)
{
#ifdef DEBUG
    ::std::clog << "<processSwTable>" << ::std::endl;
#endif

    const SwTableLines & rLines = pTable->GetTabLines();

    WW8TableNodeInfo * pPrev = NULL;

    for (USHORT n = 0; n < rLines.Count(); n++)
    {
        const SwTableLine * pLine = rLines[n];

        pPrev = processTableLine(pTable, pLine, n, 1, pPrev);
    }

#ifdef DEBUG
    ::std::clog << "</processSwTable>" << ::std::endl;
#endif
}

WW8TableNodeInfo *
WW8TableInfo::processTableLine(const SwTable * pTable,
                               const SwTableLine * pTableLine,
                               sal_uInt32 nRow,
                               sal_uInt32 nDepth, WW8TableNodeInfo * pPrev)
{
#ifdef DEBUG
    ::std::clog << "<processTableLine row=\"" << nRow << "\" depth=\""
    << nDepth << "\">" << ::std::endl;
#endif

    const SwTableBoxes & rBoxes = pTableLine->GetTabBoxes();

    WW8TableNodeInfo::Pointer_t pTextNodeInfo;

    for (USHORT n = 0; n < rBoxes.Count(); n++)
    {
        const SwTableBox * pBox = rBoxes[n];

        pPrev = processTableBox(pTable, pBox, nRow, n, nDepth, n == rBoxes.Count() - 1, pPrev);
    }

#ifdef DEBUG
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
#ifdef DEBUG
    ::std::clog << "<processTableBoxLines depth=\"" << nDepth
    << "\" row=\"" << nRow << "\" cell=\"" << nCell << "\">" << ::std::endl;
#endif

    const SwTableLines & rLines = pBox->GetTabLines();
    WW8TableNodeInfo::Pointer_t pNodeInfo;

    if (rLines.Count() > 0)
    {
        for (sal_uInt32 n = 0; n < rLines.Count(); n++)
        {
            const SwTableLine * pLine = rLines[n];
            const SwTableBoxes & rBoxes = pLine->GetTabBoxes();

            for (USHORT nBox = 0; nBox < rBoxes.Count(); nBox++)
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

#ifdef DEBUG
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
#ifdef DEBUG
    ::std::clog << "<processTableBox row=\"" << nRow << "\" cell=\"" << nCell
    << "\" depth=\"" << nDepth << "\">" << ::std::endl;
#endif

    WW8TableNodeInfo::Pointer_t pNodeInfo;
    const SwTableLines & rLines = pBox->GetTabLines();
    const SwStartNode * pSttNd = pBox->GetSttNd();
    WW8TableNodeInfo::Pointer_t pEndOfCellInfo;

    if (rLines.Count() > 0)
    {
        pNodeInfo = processTableBoxLines(pBox, pTable, pBox, nRow, nCell, nDepth);
        pNodeInfo->setEndOfCell(true);
        if (bEndOfLine)
            pNodeInfo->setEndOfLine(true);

        for (sal_uInt32 n = 0; n < rLines.Count(); n++)
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

#ifdef DEBUG
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
 sal_uInt32 nDepth)
{
    WW8TableNodeInfo::Pointer_t pNodeInfo = getTableNodeInfo(pNode);

    if (pNodeInfo.get() == NULL)
    {
        pNodeInfo = WW8TableNodeInfo::Pointer_t(new WW8TableNodeInfo(pNode));
        mMap.insert(Map_t::value_type(pNode, pNodeInfo));
    }

    pNodeInfo->setDepth(nDepth + pNodeInfo->getDepth());

    pNodeInfo->setTable(pTable);
    pNodeInfo->setTableBox(pTableBox);

    pNodeInfo->setCell(nCell);
    pNodeInfo->setRow(nRow);

#ifdef DEBUG
    ::std::clog << pNodeInfo->toString() << ::std::endl;
#endif

    return pNodeInfo;
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
    }

    return pResult;
}

}
