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
#include "WW8TableInfo.hxx"
#include "swtable.hxx"
#include "pam.hxx"
#include "ndtxt.hxx"
#include "dbgoutsw.hxx"

namespace ww8
{
// WW8TableTextNodeInfo

WW8TableNodeInfo::WW8TableNodeInfo(const SwNode * pNode,
                                   const SwTableBox * pTableBox,
                                   const SwTable * pTable)
    :mnDepth(0), mbEndOfLine(false), mbEndOfCell(false),
    mpNode(pNode), mpTableBox(pTableBox), mpTable(pTable)
{
}

WW8TableNodeInfo::~WW8TableNodeInfo()
{
}

::std::string WW8TableNodeInfo::toString() const
{
    ::std::string sResult = "<tableNodeInfo depth=\"";
    static char buffer[256];
    snprintf(buffer, sizeof(buffer), "%" SAL_PRIxUINT32, mnDepth);
    sResult += buffer;
    sResult += "\" endOfLine=\"";
    sResult += mbEndOfLine ? "yes" : "no";
    sResult += "\" endOfCell=\"";
    sResult += mbEndOfCell ? "yes" : "no";
    sResult += "\">";

#ifdef DEBUG
    sResult += dbg_out(*mpNode);
#endif
    sResult +="</tableNodeInfo>";

    return sResult;
}

void WW8TableNodeInfo::setDepth(sal_uInt32 nDepth)
{
    mnDepth = nDepth;
}

void WW8TableNodeInfo::setEndOfLine(bool bEndOfLine)
{
    mbEndOfLine = bEndOfLine;
}

void WW8TableNodeInfo::setEndOfCell(bool bEndOfCell)
{
    mbEndOfCell = bEndOfCell;
}

void WW8TableNodeInfo::setTableBox(const SwTableBox * pTableBox)
{
    mpTableBox = pTableBox;
}

void WW8TableNodeInfo::setTable(const SwTable * pTable)
{
    mpTable = pTable;
}

void WW8TableNodeInfo::setCell(sal_uInt32 nCell)
{
    mnCell = nCell;
}

void WW8TableNodeInfo::setRow(sal_uInt32 nRow)
{
    mnRow = nRow;
}

sal_uInt32 WW8TableNodeInfo::getDepth() const
{
    return mnDepth;
}

const SwNode * WW8TableNodeInfo::getNode() const
{
    return mpNode;
}

const SwTableBox * WW8TableNodeInfo::getTableBox() const
{
    return mpTableBox;
}

const SwTable * WW8TableNodeInfo::getTable() const
{
    return mpTable;
}

bool WW8TableNodeInfo::isEndOfLine() const
{
    return mbEndOfLine;
}

bool WW8TableNodeInfo::isEndOfCell() const
{
    return mbEndOfCell;
}

sal_uInt32 WW8TableNodeInfo::getCell() const
{
    return mnCell;
}

sal_uInt32 WW8TableNodeInfo::getRow() const
{
    return mnRow;
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

    for (USHORT n = 0; n < rLines.Count(); n++)
    {
        const SwTableLine * pLine = rLines[n];

        processTableLine(pTable, pLine, n, 1);
    }

#ifdef DEBUG
    ::std::clog << "</processSwTable>" << ::std::endl;
#endif
}

void WW8TableInfo::processTableLine(const SwTable * pTable,
                                    const SwTableLine * pTableLine,
                                    sal_uInt32 nRow,
                                    sal_uInt32 nDepth)
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

        pTextNodeInfo = processTableBox(pTable, pBox, nRow, n, nDepth);
    }

    if (pTextNodeInfo.get() != NULL)
        pTextNodeInfo->setEndOfLine(true);

#ifdef DEBUG
    ::std::clog << "</processTableLine>" << ::std::endl;
#endif
}

WW8TableNodeInfo::Pointer_t
WW8TableInfo::processTableBox(const SwTable * pTable,
                              const SwTableBox * pBox,
                              sal_uInt32 nRow,
                              sal_uInt32 nCell,
                              sal_uInt32 nDepth)
{
#ifdef DEBUG
    ::std::clog << "<processTableBox row=\"" << nRow << "\" cell=\"" << nCell
    << "\" depth=\"" << nDepth << "\">" << ::std::endl;
#endif

    WW8TableNodeInfo::Pointer_t pNodeInfo;
    const SwTableLines & rLines = pBox->GetTabLines();
    WW8TableNodeInfo::Pointer_t pEndOfCellInfo;

    if (rLines.Count() > 0)
    {

        for (sal_uInt32 n = 0; n < rLines.Count(); n++)
        {
            const SwTableLine * pLine = rLines[n];

            processTableLine(pTable, pLine, n, nDepth + 1);
        }
    }
    else
    {
        const SwStartNode * pSttNd = pBox->GetSttNd();
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

            if (nDepthInsideCell == 1 && rNode.IsTxtNode())
                pEndOfCellInfo = pNodeInfo;

            if (rNode.IsEndNode())
            {
                nDepthInsideCell--;

                if (nDepthInsideCell == 1 && pEndOfCellInfo.get() == NULL)
                    pEndOfCellInfo = pNodeInfo;

                SwEndNode * pEndNode = dynamic_cast<SwEndNode *> (&rNode);
                SwStartNode * pTmpSttNd = pEndNode->StartOfSectionNode();
                if (pTmpSttNd == pSttNd)
                    bDone = true;
            }

            aPaM.GetPoint()->nNode++;
        }
        while (!bDone);

        if (pEndOfCellInfo.get() != NULL)
            pEndOfCellInfo->setEndOfCell(true);
    }
#ifdef DEBUG
    ::std::clog << "</processTableBox>" << ::std::endl;
#endif

    return pEndOfCellInfo;
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
        pNodeInfo = WW8TableNodeInfo::Pointer_t
        (new WW8TableNodeInfo(pNode, pTableBox, pTable));
        mMap.insert(Map_t::value_type(pNode, pNodeInfo));
    }
    else
    {
        pNodeInfo->setTable(pTable);
        pNodeInfo->setTableBox(pTableBox);
    }

    pNodeInfo->setDepth(nDepth + pNodeInfo->getDepth());
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

}
