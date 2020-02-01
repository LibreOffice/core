/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/**
 * @file
 *  For LWP filter architecture prototype - row layouts
 */

#include "lwprowlayout.hxx"
#include "lwptable.hxx"
#include <lwpglobalmgr.hxx>
#include <xfilter/xfstylemanager.hxx>
#include <xfilter/xfrow.hxx>
#include <xfilter/xfrowstyle.hxx>
#include <xfilter/xftablestyle.hxx>
#include <xfilter/xftable.hxx>
#include <xfilter/xfcell.hxx>
#include <xfilter/xfcellstyle.hxx>
#include <set>

LwpRowLayout::LwpRowLayout(LwpObjectHeader const &objHdr, LwpSvStream* pStrm)
    : LwpVirtualLayout(objHdr, pStrm)
    , crowid(0)
    , cheight(0)
    , cLeaderDotCount(0)
    , cLeaderDotY(0)
    , cRowFlags(0)
{
    m_ConnCellList.clear();
}

LwpRowLayout::~LwpRowLayout()
{}

/**
 * @short   register row style
 * @param
 * @param
 * @param
 * @return
 */
void LwpRowLayout::SetRowMap()
{
    LwpObjectID *pCellID= &GetChildHead();
    LwpCellLayout * pCellLayout = dynamic_cast<LwpCellLayout *>(pCellID->obj().get());

    std::set<LwpCellLayout*> aSeen;
    while(pCellLayout)
    {
        aSeen.insert(pCellLayout);
        pCellLayout->SetCellMap();

        pCellID = &pCellLayout->GetNext();
        pCellLayout = dynamic_cast<LwpCellLayout *>(pCellID->obj().get());
        if (aSeen.find(pCellLayout) != aSeen.end())
            throw std::runtime_error("loop in conversion");
    }
}
/**
 * @short   register row style
 * @param
 * @param
 * @param
 * @return
 */
void LwpRowLayout::RegisterStyle()
{
    // register row style
    std::unique_ptr<XFRowStyle> pRowStyle(new XFRowStyle());

    if (m_nDirection & 0x0030)
    {
        pRowStyle->SetMinRowHeight(static_cast<float>(LwpTools::ConvertFromUnitsToMetric(cheight)));
    }
    else
    {
        pRowStyle->SetRowHeight(static_cast<float>(LwpTools::ConvertFromUnitsToMetric(cheight)));
    }
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    m_StyleName = pXFStyleManager->AddStyle(std::move(pRowStyle)).m_pStyle->GetStyleName();

    LwpTableLayout* pTableLayout = GetParentTableLayout();
    if (pTableLayout)
    {
        pTableLayout->GetTable();
    }
    // register cells' style
    LwpObjectID *pCellID= &GetChildHead();
    LwpCellLayout * pCellLayout = dynamic_cast<LwpCellLayout *>(pCellID->obj().get());

    std::set<LwpCellLayout*> aSeen;
    while (pCellLayout)
    {
        aSeen.insert(pCellLayout);

        pCellLayout->SetFoundry(m_pFoundry);
        pCellLayout->RegisterStyle();
        pCellID = &pCellLayout->GetNext();
        pCellLayout = dynamic_cast<LwpCellLayout *>(pCellID->obj().get());

        if (aSeen.find(pCellLayout) != aSeen.end())
            throw std::runtime_error("loop in conversion");
    }

}
/**
 * @short   register row style
 * @param
 * @param
 * @param
 * @return
 */
void LwpRowLayout::Read()
{
    #define MAXUNIT (0x7fffffffL)               // Highest positive UNIT value
    LwpObjectStream* pStrm = m_pObjStrm.get();

    LwpVirtualLayout::Read();

    //skip CLiteLayout data;
    LwpAtomHolder ContentClass;
    ContentClass.Read(pStrm);
    pStrm->SkipExtra();

    // Row layout content
    crowid = pStrm->QuickReaduInt16();
    cheight = pStrm->QuickReadInt32();
    cLeaderDotCount = static_cast<sal_uInt8>(pStrm->QuickReaduInt16());  // was written as lushort.
    cLeaderDotY = MAXUNIT;  // Sentinel meaning "not calculated yet"
    cRowFlags = static_cast<sal_uInt8>(pStrm->QuickReaduInt16());    // was written as lushort.

    pStrm->SkipExtra();
}

/**
 * @short   Parse rows with connect cell
 * @param  pXFTable - pointer to created XFTable
 */
void LwpRowLayout::ConvertRow(rtl::Reference<XFTable> const & pXFTable,sal_uInt8 nStartCol,sal_uInt8 nEndCol)
{
    LwpTableLayout* pTableLayout = GetParentTableLayout();
    if (!pTableLayout)
        throw std::runtime_error("missing TableLayout");
    LwpTable* pTable = pTableLayout->GetTable();

    //calculate the connected cell position
    sal_Int32 nMarkConnCell = FindMarkConnCell(nStartCol,nEndCol);

    //if there is no connected cell
    if (nMarkConnCell == -1)
    {
        ConvertCommonRow(pXFTable,nStartCol,nEndCol);
        return;
    }

    //register connect row style
    sal_uInt16 nRowMark = crowid + GetCurMaxSpannedRows(nStartCol,nEndCol);
    rtl::Reference<XFRow> xXFRow(new XFRow);
    RegisterCurRowStyle(xXFRow.get(), nRowMark);

    //if there is connected cell
    for (sal_uInt8 i=nStartCol; i<nEndCol; )
    {
        rtl::Reference<XFCell> xXFCell;
        sal_uInt8 nColMark;

        if (nMarkConnCell == -1)
            nColMark = nEndCol;
        else
            nColMark = m_ConnCellList[nMarkConnCell]->GetColID();

        if (nColMark > i)//create subtable
        {
            xXFCell.set(new XFCell);
            xXFCell->SetColumnSpaned(nColMark-i);
            XFTable* pSubTable = new XFTable;
            pTableLayout->ConvertTable(pSubTable,crowid,nRowMark,i,nColMark);
            xXFCell->Add(pSubTable);
            i = nColMark;
        }
        else
        {
            sal_uInt8 nColID = m_ConnCellList[nMarkConnCell]->GetColID()
                    +m_ConnCellList[nMarkConnCell]->GetNumcols()-1;
            xXFCell = m_ConnCellList[nMarkConnCell]->DoConvertCell(
                pTable->GetObjectID(),
                crowid+m_ConnCellList[nMarkConnCell]->GetNumrows()-1,
                m_ConnCellList[nMarkConnCell]->GetColID());

            //set all cell in this merge cell to cellsmap
            for (sal_uInt16 nRowLoop = crowid;nRowLoop<nRowMark ;nRowLoop++)
                for (sal_uInt8 nColLoop = i;nColLoop<nColID+1;nColLoop++)
                    pTableLayout->SetCellsMap(nRowLoop,nColLoop, xXFCell.get());

            i += m_ConnCellList[nMarkConnCell]->GetNumcols();
            nMarkConnCell = FindNextMarkConnCell(static_cast<sal_uInt16>(nMarkConnCell),nEndCol);
        }

        if (xXFCell)
            xXFRow->AddCell(xXFCell);
    }
    pXFTable->AddRow(xXFRow);
}

/**
 * @short   register row style in SODC table
 * @param   pXFRow - pointer of row
 * @param   nRowMark - spanned row number
 */
void LwpRowLayout::RegisterCurRowStyle(XFRow* pXFRow,sal_uInt16 nRowMark)
{
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    XFRowStyle* pRowStyle = static_cast<XFRowStyle*>(pXFStyleManager->FindStyle(m_StyleName));
    if (!pRowStyle)
        return;
    double fHeight = pRowStyle->GetRowHeight();

    std::unique_ptr<XFRowStyle> pNewStyle(new XFRowStyle);
    *pNewStyle = *pRowStyle;
    LwpTableLayout* pTableLayout = GetParentTableLayout();
    if (!pTableLayout)
    {
        return;
    }
    std::map<sal_uInt16,LwpRowLayout*> RowsMap = pTableLayout->GetRowsMap();

    for (sal_uInt16 i=crowid+1; i<nRowMark;i++)
    {
        std::map<sal_uInt16,LwpRowLayout*>::iterator iter = RowsMap.find(i);
        if (iter == RowsMap.end())
        {
            pRowStyle = static_cast<XFRowStyle*>(
                pXFStyleManager->FindStyle(pTableLayout->GetDefaultRowStyleName()));
        }
        else
        {
            pRowStyle = static_cast<XFRowStyle*>(
                pXFStyleManager->FindStyle(iter->second->GetStyleName()));
        }
        if (!pRowStyle)
            throw std::runtime_error("missing RowStyle");
        fHeight += pRowStyle->GetRowHeight();
    }

    if (m_nDirection & 0x0030)
    {
        pNewStyle->SetMinRowHeight(static_cast<float>(fHeight));
    }
    else
    {
        pNewStyle->SetRowHeight(static_cast<float>(fHeight));
    }

    pXFRow->SetStyleName(pXFStyleManager->AddStyle(std::move(pNewStyle)).m_pStyle->GetStyleName());
}

/**
 * @short   find max merge cell in a given column range
 * @param   nStartCol - start column ID
 * @param   nEndCol - end column ID
 */
sal_Int32 LwpRowLayout::FindMarkConnCell(sal_uInt8 nStartCol,sal_uInt8 nEndCol)
{
    if (m_ConnCellList.empty())
        return -1;

    sal_uInt16 nSpannRows = 1;
    sal_Int32 nMarkConnCell = -1;

    for (size_t i=0;i<m_ConnCellList.size();i++)
    {
        if (m_ConnCellList[i]->GetColID()>=nEndCol)
            break;
        if (m_ConnCellList[i]->GetColID()>=nStartCol)
        {
            if (m_ConnCellList[i]->GetNumrows()>nSpannRows)
            {
                nSpannRows = m_ConnCellList[i]->GetNumrows();
                nMarkConnCell = i;
            }
        }
    }
    return nMarkConnCell;
}

/**
 * @short   find next merge cell with the same spanned row number with current merge cell
 * @param   nStartCol - start column ID
 * @param   nEndCol - end column ID
 */
sal_Int32 LwpRowLayout::FindNextMarkConnCell(sal_uInt16 nMarkConnCell,sal_uInt8 nEndCol)
{
    sal_uInt16 nMaxRows = m_ConnCellList[nMarkConnCell]->GetNumrows();

    for (size_t i=nMarkConnCell+1;i<m_ConnCellList.size();i++)
    {
        if (m_ConnCellList[i]->GetColID()>=nEndCol)
            break;
        if (m_ConnCellList[i]->GetNumrows() == nMaxRows)
        {
            return i;
        }
    }
    return -1;
}
/**
 * @short   get max spanned row numbers in a given column range
 * @param   nStartCol - start column ID
 * @param   nEndCol - end column ID
 */
sal_uInt16 LwpRowLayout::GetCurMaxSpannedRows(sal_uInt8 nStartCol,sal_uInt8 nEndCol)
{
    sal_Int32 nMarkConnCell = FindMarkConnCell(nStartCol,nEndCol);
    if (nMarkConnCell == -1)
        return 1;
    else
        return m_ConnCellList[nMarkConnCell]->GetNumrows();
}
/**
 * @short   convert row with rowlayout,but no merge cells
 * @param   pXFTable - pointer of table
 * @param   nStartCol - start column ID
 * @param   nEndCol - end column ID
 */
void LwpRowLayout::ConvertCommonRow(rtl::Reference<XFTable> const & pXFTable, sal_uInt8 nStartCol, sal_uInt8 nEndCol)
{
    LwpTableLayout* pTableLayout = GetParentTableLayout();
    if (!pTableLayout)
        return;
    LwpTable* pTable = pTableLayout->GetTable();
    if (!pTable)
        return;

    rtl::Reference<XFRow> xRow(new XFRow);
    xRow->SetStyleName(m_StyleName);

    sal_uInt8 nCellStartCol,nCellEndCol;

    for (sal_uInt8 i = nStartCol; i < nEndCol ; i++)
    {
        // add row to table
        LwpObjectID *pCellID= &GetChildHead();
        LwpCellLayout * pCellLayout = dynamic_cast<LwpCellLayout *>(pCellID->obj().get());
        nCellStartCol = i;//mark the begin position of cell
        nCellEndCol = i;//mark the end position of cell
        rtl::Reference<XFCell> xCell;
        while(pCellLayout)
        {
            if (pCellLayout->GetColID() == i)
            {
                if (pCellLayout->GetLayoutType() == LWP_CONNECTED_CELL_LAYOUT)
                {
                    LwpConnectedCellLayout* pConnCell = static_cast<LwpConnectedCellLayout*>(pCellLayout);
                    nCellEndCol = i+pConnCell->GetNumcols()-1;
                    i = nCellEndCol;
                }
                xCell = pCellLayout->DoConvertCell(pTable->GetObjectID(),crowid,i);
                break;
            }
            pCellID = &pCellLayout->GetNext();
            pCellLayout = dynamic_cast<LwpCellLayout *>(pCellID->obj().get());
        }
        if (!pCellLayout)
        {
            // if table has default cell layout, use it to ConvertCell
            // otherwise use blank cell
            LwpCellLayout * pDefaultCell = pTableLayout->GetDefaultCellLayout();
            if (pDefaultCell)
            {
                xCell = pDefaultCell->DoConvertCell(
                    pTable->GetObjectID(),crowid, i);
            }
            else
            {
                xCell.set(new XFCell);
            }
        }
        xRow->AddCell(xCell);

        for (sal_uInt8 j=nCellStartCol;j<=nCellEndCol;j++)
            pTableLayout->SetCellsMap(crowid,j, xCell.get());//set to cellsmap
    }

    pXFTable->AddRow(xRow);
}
/**
 * @short   collect merge cell info when register row styles
 */
void LwpRowLayout::CollectMergeInfo()
{
    LwpObjectID *pCellID= &GetChildHead();
    LwpCellLayout * pCellLayout = dynamic_cast<LwpCellLayout *>(pCellID->obj().get());

    while(pCellLayout)
    {
        if (pCellLayout->GetLayoutType() == LWP_CONNECTED_CELL_LAYOUT)
        {
            LwpConnectedCellLayout* pConnCell = static_cast<LwpConnectedCellLayout*>(pCellLayout);
            m_ConnCellList.push_back(pConnCell);
        }
        pCellID = &pCellLayout->GetNext();
        pCellLayout = dynamic_cast<LwpCellLayout *>(pCellID->obj().get());
    }
}
/**
 * @short   split merge cells in this row
 * @param   nEffectRows - max spanned number of previous row
 */
void LwpRowLayout::SetCellSplit(sal_uInt16 nEffectRows)
{
    for (LwpConnectedCellLayout* pConnCell : m_ConnCellList)
    {
        sal_uInt16 nRowSpan = pConnCell->GetRowID()+pConnCell->GetNumrows();
        if ( nRowSpan > nEffectRows )
        {
            pConnCell->SetNumrows(nEffectRows - pConnCell->GetRowID());
        }
    }
}
/**
 * @short   check if the row has merge cell
 */
bool LwpRowLayout::GetMergeCellFlag() const
{
    return !m_ConnCellList.empty();
}

LwpRowHeadingLayout::LwpRowHeadingLayout(LwpObjectHeader const &objHdr, LwpSvStream* pStrm)
    : LwpRowLayout(objHdr, pStrm)
{}

LwpRowHeadingLayout::~LwpRowHeadingLayout()
{}
void LwpRowHeadingLayout::Read()
{
    LwpRowLayout::Read();

    cRowLayout.ReadIndexed(m_pObjStrm.get());
    m_pObjStrm->SkipExtra();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
