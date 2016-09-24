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
 *  For LWP filter architecture prototype - table layouts
 */

#include "lwpglobalmgr.hxx"
#include "lwptablelayout.hxx"
#include "lwpfoundry.hxx"
#include "lwpobjfactory.hxx"
#include "lwpholder.hxx"
#include "lwptable.hxx"
#include "lwptblcell.hxx"
#include "lwpnumericfmt.hxx"
#include "lwpdlvlist.hxx"
#include "lwppara.hxx"

#include "xfilter/xfstylemanager.hxx"
#include "xfilter/xftablestyle.hxx"
#include "xfilter/xftable.hxx"
#include "xfilter/xfrow.hxx"
#include "xfilter/xfrowstyle.hxx"
#include "xfilter/xfcell.hxx"
#include "xfilter/xfcellstyle.hxx"
#include "xfilter/xfcolstyle.hxx"
#include "xfilter/xfframestyle.hxx"
#include "xfilter/xfframe.hxx"
#include "xfilter/xffloatframe.hxx"
#include "lwpframelayout.hxx"
#include "xfilter/xfparastyle.hxx"

LwpSuperTableLayout::LwpSuperTableLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm)
    : LwpPlacableLayout(objHdr, pStrm)
{
    m_pFrame = new LwpFrame(this);
}

LwpSuperTableLayout::~LwpSuperTableLayout()
{
    if(m_pFrame)
    {
        delete m_pFrame;
    }
}
/**
 * @short    Read super table layout record
 */
void LwpSuperTableLayout::Read()
{
    LwpPlacableLayout::Read();
    m_pObjStrm->SkipExtra();

}
/**
 * @short   Get child table layout
 * @return pointer to table layout
 */
LwpTableLayout* LwpSuperTableLayout::GetTableLayout()
{
    LwpObjectID& rID = GetChildTail();

    while(!rID.IsNull())
    {
        LwpLayout* pLayout = dynamic_cast<LwpLayout*>(rID.obj().get());
        if (!pLayout)
        {
            break;
        }
        if (pLayout->GetLayoutType() == LWP_TABLE_LAYOUT)
        {
            return dynamic_cast<LwpTableLayout *>(pLayout);
        }
        rID = pLayout->GetPrevious();
    }

    return nullptr;
}
/**
 * @short   Get effective heading table layout, the one just before table layout is the only one which is effective
 * @return LwpTableHeadingLayout* - pointer to table heading layout
 */
LwpTableHeadingLayout* LwpSuperTableLayout::GetTableHeadingLayout()
{
    LwpObjectID& rID = GetChildTail();

    while(!rID.IsNull())
    {
        LwpLayout * pLayout = dynamic_cast<LwpLayout *>(rID.obj().get());
        if (!pLayout)
        {
            break;
        }

        if (pLayout->GetLayoutType() == LWP_TABLE_HEADING_LAYOUT)
        {
            return dynamic_cast<LwpTableHeadingLayout *>(pLayout);
        }
        rID = pLayout->GetPrevious();
    }

    return nullptr;
}
/**
 * @short   Register super table layout style
 */
void LwpSuperTableLayout::RegisterNewStyle()
{
    // if this layout is style of real table entry
    LwpTableLayout* pTableLayout = GetTableLayout();
    if (pTableLayout != nullptr)
    {
        pTableLayout->SetFoundry(m_pFoundry);
        pTableLayout->RegisterStyle();
    }
}
/**
 * @short   Judge whether table size is according to content, borrowed from Word Pro code
 * @param
 * @return sal_Bool
 */
bool LwpSuperTableLayout::IsSizeRightToContent()
{
    /* Only "with paragraph above" tables can size right to content. */
    if (GetRelativeType() == LwpLayoutRelativityGuts::LAY_INLINE_NEWLINE)
        return LwpPlacableLayout::IsSizeRightToContent();

    return false;
}
/**
 * @short   Judge whether table is justifiable, borrowed from Word Pro code
 * @param
 * @return sal_Bool
 */
bool LwpSuperTableLayout::IsJustifiable()
{
    return (GetRelativeType() != LwpLayoutRelativityGuts::LAY_INLINE_NEWLINE || IsSizeRightToContent());
}
/**
 * @short   Get width of frame outside table
 * @param pTableStyle - pointer of XFTableStyle
 * @return double - table width
 */
double LwpSuperTableLayout::GetWidth()
{
    double dWidth = GetTableWidth();
    double dLeft    = GetMarginsValue(MARGIN_LEFT);
    double dRight   = GetMarginsValue(MARGIN_RIGHT);

    return (dWidth + dLeft + dRight);
}
/**
 * @short   Get width of table
 * @param pTableStyle - pointer of XFTableStyle
 * @return double - table width
 */
double LwpSuperTableLayout::GetTableWidth()
{
    sal_Int32 nWidth = 0;
    if(!IsJustifiable() || ((nWidth = LwpMiddleLayout::GetMinimumWidth()) <= 0))
    {
        LwpTableLayout* pTableLayout = GetTableLayout();
        if(!pTableLayout)
        {
            SAL_WARN("lwp", "missing table layout, early return");
            return 0;
        }
        LwpTable *pTable = pTableLayout->GetTable();
        if(!pTable)
        {
            SAL_WARN("lwp", "missing table, early return");
            return 0;
        }
        double dDefaultWidth = pTable->GetWidth();
        sal_uInt16 nCol = pTable->GetColumn();

        double dWidth = 0;

        for(sal_uInt16 i =0; i< nCol; i++)
        {
            LwpObjectID& rColumnID = pTableLayout->GetColumnLayoutHead();
            LwpColumnLayout * pColumnLayout = dynamic_cast<LwpColumnLayout *>(rColumnID.obj().get());
            double dColumnWidth = dDefaultWidth;
            while (pColumnLayout)
            {
                if(pColumnLayout->GetColumnID() == i)
                {
                    dColumnWidth = pColumnLayout->GetWidth();
                    break;
                }
                rColumnID = pColumnLayout->GetNext();
                pColumnLayout = dynamic_cast<LwpColumnLayout *>(rColumnID.obj().get());
            }
            dWidth += dColumnWidth;
        }

        return dWidth;
    }

    double dLeft    = GetMarginsValue(MARGIN_LEFT);
    double dRight   = GetMarginsValue(MARGIN_RIGHT);
    return LwpTools::ConvertFromUnitsToMetric(nWidth)-dLeft-dRight;

}
/**
 * @short   Apply shadow to table
 * @param pTableStyle - pointer of XFTableStyle
 * @return
 */
void LwpSuperTableLayout::ApplyShadow(XFTableStyle *pTableStyle)
{
    // use shadow property of supertable
    std::unique_ptr<XFShadow> pXFShadow(GetXFShadow());
    if(pXFShadow)
    {
        pTableStyle->SetShadow(pXFShadow->GetPosition(), pXFShadow->GetOffset(), pXFShadow->GetColor());
    }
}
/**
 * @short   Apply pattern fill to table style
 * @param pTableStyle - pointer of XFTableStyle
 * @return
 */
void LwpSuperTableLayout::ApplyPatternFill(XFTableStyle* pTableStyle)
{
    XFBGImage* pXFBGImage = GetFillPattern();
    if (pXFBGImage)
    {
        pTableStyle->SetBackImage(pXFBGImage);
    }
}

/**
 * @short   Apply background to table style
 * @param pTableStyle - pointer of XFTableStyle
 * @return
 */
void LwpSuperTableLayout::ApplyBackGround(XFTableStyle* pTableStyle)
{
    if (IsPatternFill())
    {
        ApplyPatternFill(pTableStyle);
    }
    else
    {
        ApplyBackColor(pTableStyle);
    }
}
/**
 * @short   Apply back color to table
 * @param pTableStyle - pointer of XFTableStyle
 * @return
 */
void LwpSuperTableLayout::ApplyBackColor(XFTableStyle *pTableStyle)
{
    LwpColor* pColor = GetBackColor();
    if(pColor && pColor->IsValidColor())
    {
        XFColor aColor(pColor->To24Color());
        pTableStyle->SetBackColor(aColor);
    }
}
/**
 * @short   Apply watermark to  table
 * @param pTableStyle - pointer of XFTableStyle
 * @return
 */
void LwpSuperTableLayout::ApplyWatermark(XFTableStyle *pTableStyle)
{
    XFBGImage* pBGImage = GetXFBGImage();
    if(pBGImage)
    {
        pTableStyle->SetBackImage(pBGImage);
    }
}
/**
 * @short   Apply alignment  to table
 * @param pTableStyle - pointer of XFTableStyle
 * @return
 */
void LwpSuperTableLayout::ApplyAlignment(XFTableStyle * pTableStyle)
{
    LwpPoint aPoint;
    if (GetGeometry())
        aPoint = GetGeometry()->GetOrigin();
    //LwpPoint aPoint = GetOrigin();
    double dXOffset = LwpTools::ConvertFromUnitsToMetric(aPoint.GetX());

    // add left padding to alignment distance
    double dLeft = GetMarginsValue(MARGIN_LEFT);

    pTableStyle->SetAlign(enumXFAlignStart, dXOffset+ dLeft);
}
/**
 * @short   Add table to container
 * @param pCont - pointer of container
 * @return pCont
 */
void  LwpSuperTableLayout::XFConvert(XFContentContainer* pCont)
{
    if ( LwpLayoutRelativityGuts::LAY_INLINE_NEWLINE == GetRelativeType()
            && (!GetContainerLayout().is() || !GetContainerLayout()->IsCell()) )
    {
        LwpTableLayout * pTableLayout = GetTableLayout();
        if (pTableLayout)
        {
            pTableLayout->XFConvert(pCont);
        }
    }
    else if(IsRelativeAnchored())
    {
        //anchor to paragraph except "with paragraph above"
        XFConvertFrame(pCont);
    }
    else if(m_pFrame)
    {
        //anchor to page, frame, cell
        m_pFrame->XFConvert(pCont);
    }
}
/**
 * @short   convert frame which anchor to page
 * @param
 * @return
 */
void  LwpSuperTableLayout::XFConvertFrame(XFContentContainer* pCont, sal_Int32 nStart, sal_Int32 nEnd, bool bAll)
{
    if(m_pFrame)
    {
        XFFrame* pXFFrame = nullptr;
        if(nEnd < nStart)
        {
            pXFFrame = new XFFrame();
        }
        else
        {
            pXFFrame = new XFFloatFrame(nStart, nEnd, bAll);
        }

        m_pFrame->Parse(pXFFrame, static_cast<sal_uInt16>(nStart));
        //parse table, and add table to frame
        LwpTableLayout * pTableLayout = GetTableLayout();
        if (pTableLayout)
        {
            pTableLayout->XFConvert(pXFFrame);
        }
        //add frame to the container
        pCont ->Add(pXFFrame);
    }

}
/**
 * @short  register frame style
 * @param
 * @return
 */
void  LwpSuperTableLayout::RegisterFrameStyle()
{
    XFFrameStyle* pFrameStyle = new XFFrameStyle();
    m_pFrame->RegisterStyle(pFrameStyle);
}

LwpTableLayout::LwpTableLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm)
    : LwpLayout(objHdr, pStrm)
    , m_nRows(0)
    , m_nCols(0)
    , m_pDefaultCellLayout(nullptr)
    , m_pColumns(nullptr)
{
    m_CellsMap.clear();
}

LwpTableLayout::~LwpTableLayout()
{
    m_CellsMap.clear();

    if (m_pColumns)
    {
        delete [] m_pColumns;
        m_pColumns = nullptr;
    }
}

/**
 * @short   Get neighbour cell by specifying ROW+COL
 * @param   nRow
 * @param   nCol
 * @return   LwpCellLayout *
 */
LwpCellLayout * LwpTableLayout::GetCellByRowCol(sal_uInt16 nRow, sal_uInt16 nCol)
{
    if (nRow >= m_nRows || nCol >= m_nCols)
        return nullptr;

    return m_WordProCellsMap[static_cast<size_t>(nRow)*m_nCols + nCol];
}

/**
 * @short   traverse all table cells
 * @param
 * @param
 * @param
 */
void LwpTableLayout::TraverseTable()
{
    sal_uInt32 nCount = m_nRows*m_nCols;

    // new cell map nRow*nCOl and initialize
    for (sal_uInt32 iLoop = 0; iLoop < nCount; ++iLoop)
    {
        m_WordProCellsMap.push_back(GetDefaultCellLayout());
    }

    // set value
    LwpObjectID& rRowID = GetChildHead();
    LwpRowLayout * pRowLayout = dynamic_cast<LwpRowLayout *>(rRowID.obj().get());
    while (pRowLayout)
    {
        pRowLayout->SetRowMap();

        // for 's analysis job
        m_RowsMap[pRowLayout->GetRowID()] = pRowLayout;
        pRowLayout->CollectMergeInfo();
        // end for 's analysis

        rRowID = pRowLayout->GetNext();
        pRowLayout = dynamic_cast<LwpRowLayout *>(rRowID.obj().get());
    }
}

/**
 * @short   search the cell map
 * @param   nRow - row id (0 based)
 * @param   nRow - row id (0 based)
 * @return   LwpObjectID * - pointer to cell story object ID
 */
LwpObjectID * LwpTableLayout::SearchCellStoryMap(sal_uInt16 nRow, sal_uInt16 nCol)
{
    if (nRow >= m_nRows || nCol >= m_nCols )
    {
        return nullptr;
    }

    LwpCellLayout * pCell = GetCellByRowCol(nRow, nCol);
    if (pCell)
    {
        // maybe connected cell layout
        // maybe default cell layout
        if (nRow != pCell->GetRowID() || nCol != pCell->GetColID())
        {
            return nullptr;
        }
        return &pCell->GetContent();
    }

    return nullptr;
}

/**
 * @short   Get parent super table layout of table layout
 * @return  LwpSuperTableLayout * - pointer of parent super table layout
 */
LwpSuperTableLayout * LwpTableLayout::GetSuperTableLayout()
{
    return dynamic_cast<LwpSuperTableLayout *>(GetParent().obj().get());
}
/**
 * @short    Get table pointer
 * @return   LwpTable * - content table pointer
 */
LwpTable *  LwpTableLayout::GetTable()
{
    LwpTable *pTable = dynamic_cast<LwpTable *>(m_Content.obj().get());
    return pTable;
}
/**
 * @short   Get column style name by column ID
 * @param   sal_uInt16 -- col id(0 based)
 * @return OUString - name of column style
 */
OUString LwpTableLayout::GetColumnWidth(sal_uInt16 nCol)
{
    if (nCol >= m_nCols)
    {
        assert(false);
        return m_DefaultColumnStyleName;
    }

    LwpColumnLayout * pCol = m_pColumns[nCol];
    if (pCol)
    {
        return pCol->GetStyleName();
    }

    return m_DefaultColumnStyleName;
}
/**
 * @short   analyze all columns to get whole table width and width of all columns
 * @short   and register all column styles
 * @param   none
 */
void LwpTableLayout::RegisterColumns()
{
    LwpTable * pTable = GetTable();
    LwpSuperTableLayout * pSuper = GetSuperTableLayout();

    sal_uInt16 nCols = m_nCols;

    m_pColumns = new LwpColumnLayout *[nCols];
    sal_Bool * pWidthCalculated = new sal_Bool[nCols];
    for(sal_uInt16 i=0;i<nCols; i++)
    {
        pWidthCalculated[i] = false;
        m_pColumns[i] = nullptr;
    }

    double dDefaultColumn = pTable->GetWidth();
    sal_uInt16 nJustifiableColumn = nCols;

    double dTableWidth = pSuper->GetTableWidth();

    // Get total width of justifiable columns
    // NOTICE: all default columns are regarded as justifiable columns
    LwpObjectID& rColumnID = GetColumnLayoutHead();
    LwpColumnLayout * pColumnLayout = dynamic_cast<LwpColumnLayout *>(rColumnID.obj().get());
    while (pColumnLayout)
    {
        auto nColId = pColumnLayout->GetColumnID();
        if (nColId >= nCols)
        {
            delete [] pWidthCalculated;
            throw std::range_error("corrupt LwpTableLayout");
        }
        m_pColumns[nColId] = pColumnLayout;
        if (!pColumnLayout->IsJustifiable())
        {
            pWidthCalculated[nColId] = true;
            dTableWidth -= pColumnLayout->GetWidth();
            nJustifiableColumn --;
        }

        rColumnID = pColumnLayout->GetNext();
        pColumnLayout = dynamic_cast<LwpColumnLayout *>(rColumnID.obj().get());
    }

    // if all columns are not justifiable, the rightmost column will be changed to justifiable
    if (nJustifiableColumn == 0 && nCols != 0)
    {
        nJustifiableColumn ++;
        if (m_pColumns[nCols - 1])
        {
            pWidthCalculated[nCols-1] = false;
            dTableWidth += m_pColumns[nCols-1]->GetWidth();
        }
        else
        {
            // this can't happen
            dTableWidth = dDefaultColumn;
            assert(false);
        }
    }

    // justifiable columns will share the remain width averagely
    dDefaultColumn = nJustifiableColumn ? dTableWidth/nJustifiableColumn : 0;

    // register default column style
    XFColStyle *pColStyle = new XFColStyle();
    pColStyle->SetWidth(static_cast<float>(dDefaultColumn));

    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    m_DefaultColumnStyleName =  pXFStyleManager->AddStyle(pColStyle).m_pStyle->GetStyleName();

    // register existed column style
    sal_uInt16 i=0;
    for( i=0;i<nCols; i++)
    {
        if(m_pColumns[i])
        {
            m_pColumns[i]->SetFoundry(m_pFoundry);
            if(!pWidthCalculated[i])
            {
                // justifiable ----register style with calculated value
                m_pColumns[i]->SetStyleName(m_DefaultColumnStyleName);
            }
            else
            {
                // not justifiable ---- register style with original value
                m_pColumns[i]->RegisterStyle(m_pColumns[i]->GetWidth());
            }
        }
    }
    delete [] pWidthCalculated;
}
/**
 * @short    register all row styles
 * @param   none
 */
void LwpTableLayout::RegisterRows()
{
    LwpTable * pTable = GetTable();
    if (pTable == nullptr)
    {
        assert(false);
        return;
    }

    // register default row style
    XFRowStyle * pRowStyle = new  XFRowStyle();
    if (m_nDirection & 0x0030)
    {
        pRowStyle->SetMinRowHeight((float)pTable->GetHeight());
    }
    else
    {
        pRowStyle->SetRowHeight((float)pTable->GetHeight());
    }
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    m_DefaultRowStyleName =  pXFStyleManager->AddStyle(pRowStyle).m_pStyle->GetStyleName();

    // register style of rows
    LwpObjectID& rRowID = GetChildHead();
    LwpRowLayout * pRowLayout = dynamic_cast<LwpRowLayout *>(rRowID.obj().get());
    while (pRowLayout)
    {
        pRowLayout->SetFoundry(m_pFoundry);
        pRowLayout->RegisterStyle();

        rRowID = pRowLayout->GetNext();
        pRowLayout = dynamic_cast<LwpRowLayout *>(rRowID.obj().get());
    }
}
/**
 * @short   register table style, if needed, including frame style
 * @param   none
 */
void LwpTableLayout::RegisterStyle()
{
    // get super table layout
    LwpSuperTableLayout * pSuper = GetSuperTableLayout();
    if (!pSuper)
        return;

    // get table
    LwpTable * pTable = GetTable();
    if (pTable == nullptr)
    {
        SAL_WARN("lwp", "missing table, early return");
        return;
    }

    // get row/column number of this table
    m_nRows = pTable->GetRow();
    m_nCols = pTable->GetColumn();

    // get default cell layout of current table
    LwpObjectID& rID= pTable->GetDefaultCellStyle();
    m_pDefaultCellLayout = dynamic_cast<LwpCellLayout *>(rID.obj().get());

    // register columns styles
    RegisterColumns();

    // register style of whole table
    XFTableStyle * pTableStyle = new XFTableStyle();

    sal_uInt8 nType = pSuper->GetRelativeType();
    // If the table is not "with paragraph above" placement, create an frame style
    // by supertable layout
    if ( LwpLayoutRelativityGuts::LAY_INLINE_NEWLINE == nType
        && (!pSuper->GetContainerLayout().is() || !pSuper->GetContainerLayout()->IsCell()) )
    {
        //with para above
//      pSuper->ApplyBackColor(pTableStyle);
        pSuper->ApplyBackGround(pTableStyle);
        pSuper->ApplyWatermark(pTableStyle);
        pSuper->ApplyShadow(pTableStyle);
        pSuper->ApplyAlignment(pTableStyle);
        pTableStyle->SetWidth(pSuper->GetTableWidth());
    }
    else
    {
        pSuper->RegisterFrameStyle();
        pTableStyle->SetAlign(enumXFAlignCenter);
        pTableStyle->SetWidth(pSuper->GetTableWidth());
    }
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    m_StyleName = pXFStyleManager->AddStyle(pTableStyle).m_pStyle->GetStyleName();

    //convert to OO table now and register row style
    // traverse
    TraverseTable();

    SplitConflictCells();

    // Register rows layouts, it must be after SplitConflictCells
    RegisterRows();

    // Parse table
    ParseTable();

    //Comment: the old code doesn't check if the LwpFoundry pointer is NULL,
    //         so the NULL pointer cause sodc freeze. Add code to check the
    //         the pointer.
    //New Code
    if (GetFoundry() && GetTable())
        PutCellVals(GetFoundry(), GetTable()->GetObjectID());
}
/**
 * @short   read table layout
 * @param   none
 */
void LwpTableLayout::ParseTable()
{
    // get super table layout
    LwpSuperTableLayout* pSuper = GetSuperTableLayout();
    if (!pSuper)
    {
        throw std::runtime_error("missing super table");
    }

    if (m_pXFTable.get())
    {
        throw std::runtime_error("this table is already parsed");
    }

    // set name of object
    m_pXFTable = new XFTable;
    m_pXFTable->SetTableName(pSuper->GetName().str());
    // set table style
    m_pXFTable->SetStyleName(m_StyleName);

    sal_uInt16 nRow = m_nRows;
    sal_uInt8 nCol = (sal_uInt8)m_nCols;

    //process header rows
    LwpTableHeadingLayout* pTableHeading;
    pTableHeading = pSuper->GetTableHeadingLayout();
    sal_uInt16 nStartHeadRow;
    sal_uInt16 nEndHeadRow;
    sal_uInt16 nContentRow;
    if (pTableHeading)
    {
        pTableHeading->GetStartEndRow(nStartHeadRow,nEndHeadRow);
        if (nStartHeadRow != 0)
            ConvertTable(m_pXFTable,0,nRow,0,nCol);
        else
        {
            nContentRow = ConvertHeadingRow(m_pXFTable,nStartHeadRow,nEndHeadRow+1);
            ConvertTable(m_pXFTable,nContentRow,nRow,0,nCol);
        }
    }
    else
        ConvertTable(m_pXFTable,0,nRow,0,nCol);
}

/**
 * @short   read table layout
 * @param   none
 */
void LwpTableLayout::Read()
{
    LwpLayout::Read();

    // before layout hierarchy rework!
    if(LwpFileHeader::m_nFileRevision < 0x000b)
    {
        assert(false);
    }
    m_ColumnLayout.ReadIndexed(m_pObjStrm);

    m_pObjStrm->SkipExtra();
}

/**
 * @short    Convert table
 * @param
 * @return   pCont - container which will contain table
 */
void LwpTableLayout::XFConvert(XFContentContainer* pCont)
{

    pCont->Add(m_pXFTable.get());
}
/**
 * @short   convert heading row
 * @param  pXFTable - pointer of table
 * @param  nStartRow - start heading row ID
 * @param  nEndRow - end heading row ID
 */
sal_uInt16 LwpTableLayout::ConvertHeadingRow(
        rtl::Reference<XFTable> const & pXFTable, sal_uInt16 nStartHeadRow, sal_uInt16 nEndHeadRow)
{
    sal_uInt16 nContentRow;
    sal_uInt8 nCol = static_cast<sal_uInt8>(GetTable()->GetColumn());
    XFTable* pTmpTable = new XFTable;
    XFRow* pXFRow;

    ConvertTable(pTmpTable,nStartHeadRow,nEndHeadRow,0,nCol);

    sal_uInt16 nRowNum = pTmpTable->GetRowCount();
    sal_uInt8* CellMark = new sal_uInt8[nRowNum];

    if (nRowNum == 1)
    {
        pXFRow = pTmpTable->GetRow(1);
        pXFTable->AddHeaderRow(pXFRow);
        pTmpTable->RemoveRow(1);
        nContentRow = nEndHeadRow;
    }
    else
    {
        sal_uInt8 nFirstColSpann = 1;
        const bool bFindFlag = FindSplitColMark(pTmpTable,CellMark,nFirstColSpann);

        if (bFindFlag)//split to 2 cells
        {
            SplitRowToCells(pTmpTable,pXFTable,nFirstColSpann,CellMark);
            nContentRow = nEndHeadRow;
        }
        else//can not split,the first row will be the heading row,the rest will be content row
        {
            pXFRow = pTmpTable->GetRow(1);
            pXFTable->AddHeaderRow(pXFRow);
            pTmpTable->RemoveRow(1);
            nContentRow = m_RowsMap[0]->GetCurMaxSpannedRows(0,nCol);
        }
    }
    delete pTmpTable;
    delete [] CellMark;
    return nContentRow;
}

void LwpTableLayout::SplitRowToCells(XFTable* pTmpTable, rtl::Reference<XFTable> const & pXFTable,
        sal_uInt8 nFirstColSpann,sal_uInt8* pCellMark)
{
    sal_uInt16 i;
    sal_uInt8 j;
    sal_uInt16 nRowNum = pTmpTable->GetRowCount();
    sal_uInt8 nCol = static_cast<sal_uInt8>(GetTable()->GetColumn());

    XFRow* pXFRow = new XFRow;

    //register style for heading row
    double fHeight = 0;
    OUString styleName;
    XFRowStyle* pRowStyle = new XFRowStyle;
    styleName = pTmpTable->GetRow(1)->GetStyleName();

    // get settings of the row and assign them to new row style
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    XFRowStyle *pTempRowStyle = static_cast<XFRowStyle*>(pXFStyleManager->FindStyle(styleName));
    if (pTempRowStyle)
        *pRowStyle = *pTempRowStyle;

    for (i=1;i<=nRowNum;i++)
    {
        styleName = pTmpTable->GetRow(i)->GetStyleName();
        fHeight+=static_cast<XFRowStyle*>(pXFStyleManager->FindStyle(styleName))->GetRowHeight();
    }
    if (m_nDirection & 0x0030)
    {
        pRowStyle->SetMinRowHeight((float)fHeight);
    }
    else
    {
        pRowStyle->SetRowHeight((float)fHeight);
    }
    pXFRow->SetStyleName(pXFStyleManager->AddStyle(pRowStyle).m_pStyle->GetStyleName());

    //construct heading row
    XFCell* pXFCell1 = new XFCell;
    XFCell* pXFCell2 = new XFCell;
    XFTable* pSubTable1 = new XFTable;
    XFTable* pSubTable2 = new XFTable;
    XFRow* pNewRow;
    XFRow* pOldRow;
    XFCell* pNewCell;

    for (i=1;i<=nRowNum;i++)
    {
        pOldRow = pTmpTable->GetRow(i);
        pNewRow = new XFRow;
        pNewRow->SetStyleName(pOldRow->GetStyleName());
        for (j=1;j<=pCellMark[i];j++)
        {
            pNewCell = pOldRow->GetCell(j);
            pNewRow->AddCell(pNewCell);
        }
        pSubTable1->AddRow(pNewRow);
    }
    ConvertColumn(pSubTable1,0,nFirstColSpann);//add column info

    pXFCell1->Add(pSubTable1);
    pXFCell1->SetColumnSpaned(nFirstColSpann);
    pXFRow->AddCell(pXFCell1);

    for (i=1;i<=nRowNum;i++)
    {
        pOldRow = pTmpTable->GetRow(i);
        pNewRow = new XFRow;
        pNewRow->SetStyleName(pOldRow->GetStyleName());
        for(j=pCellMark[i]+1;j<=pOldRow->GetCellCount();j++)
        {
            pNewCell = pOldRow->GetCell(j);
            pNewRow->AddCell(pNewCell);
        }
        pSubTable2->AddRow(pNewRow);

    }
    ConvertColumn(pSubTable2,nFirstColSpann,nCol);//add column info
    pXFCell2->Add(pSubTable2);
    pXFCell2->SetColumnSpaned(nCol-nFirstColSpann);
    pXFRow->AddCell(pXFCell2);

    pXFTable->AddHeaderRow(pXFRow);

    //remove tmp table
    for (i=1;i<=nRowNum;i++)
    {
        pOldRow = pTmpTable->GetRow(i);
        for(j=1;j<=pOldRow->GetCellCount();j++)
            pOldRow->RemoveCell(j);
        pTmpTable->RemoveRow(i);
    }
}

/**
 * @short   find if the heading rows can be split to 2 cells
 * @param  pXFTable - pointer of tmp XFtable
 * @param  CellMark - pointer of cell mark array
 */
bool  LwpTableLayout::FindSplitColMark(XFTable* pXFTable, sal_uInt8* pCellMark,
            sal_uInt8& nMaxColSpan)
{
    sal_uInt16 nRowNum = pXFTable->GetRowCount();
    sal_uInt8 nColNum = static_cast<sal_uInt8>(pXFTable->GetColumnCount());
    sal_uInt8 nCellMark=0;
    sal_uInt8 nCount;
    sal_uInt8 nColSpan;
    bool bFindFlag = false;
    XFRow* pTmpRow;

    for(sal_uInt8 i=1;i<=nColNum;i++)
    {
        sal_uInt16 nRowLoop;
        sal_uInt8 nCellLoop;

        //find current max column span
        nMaxColSpan = 1;
        for (nRowLoop=1;nRowLoop<=nRowNum;nRowLoop++)
        {
            nColSpan = 0;
            for(nCellLoop=1; nCellLoop<i+1; nCellLoop++)
            {
                pTmpRow = pXFTable->GetRow(nRowLoop);
                XFCell* pCell = pTmpRow->GetCell(nCellLoop);
                if (pCell)
                    nColSpan += static_cast<sal_uInt8>(pCell->GetColSpaned());
                else
                    return false;
            }
            if (nColSpan > nMaxColSpan)
                nMaxColSpan = nColSpan;
            pCellMark[nRowLoop] = 0;//reset all cell mark to zero
        }

        //find if other row has the same column
        for (nRowLoop=1;nRowLoop<=nRowNum;nRowLoop++)
        {
            pTmpRow = pXFTable->GetRow(nRowLoop);
            nCount = 0;
            nCellMark = 0;
            for (nCellLoop=1; nCellLoop<=pTmpRow->GetCellCount(); nCellLoop++)
            {
                if (nCount>nMaxColSpan)
                    break;
                nCount+= static_cast<sal_uInt8>(pTmpRow->GetCell(nCellLoop)->GetColSpaned());
                if (nCount == nMaxColSpan)
                {
                    nCellMark = nCellLoop;
                    break;
                }
            }
            if (nCellMark == 0)
                break;
            else
                pCellMark[nRowLoop] = nCellMark;
        }
        for(nRowLoop=1;nRowLoop<=nRowNum;nRowLoop++)//check if all ==0,break
        {
            if (pCellMark[nRowLoop] == 0)
                break;
        }
        if (nRowLoop == nRowNum+1)
        {
            bFindFlag = true;
            return bFindFlag;
        }

    }
    return bFindFlag;
}

/**
 * @short   convert word pro table to SODC table
 * @param  pXFTable - pointer of table
 * @param  nStartRow - start row ID
 * @param  nEndRow - end row ID
 * @param  nStartCol - start column ID
 * @param  nEndCol - end column ID
 */
void LwpTableLayout::ConvertTable(rtl::Reference<XFTable> const & pXFTable, sal_uInt16 nStartRow,
                sal_uInt16 nEndRow,sal_uInt8 nStartCol,sal_uInt8 nEndCol)
{
    //out put column info TO BE CHANGED
    ConvertColumn(pXFTable,nStartCol,nEndCol);

    std::map<sal_uInt16,LwpRowLayout*>::iterator iter;

    for (sal_uInt16 i=nStartRow; i<nEndRow;)
    {
        iter = m_RowsMap.find(i);
        if (iter == m_RowsMap.end())
        {
            ConvertDefaultRow(pXFTable,nStartCol,nEndCol,i);
            i++;
        }
        else
        {
            LwpRowLayout* pRow = iter->second;
            if (pRow->GetCurMaxSpannedRows(nStartCol,nEndCol) == 1)
            {
                pRow->ConvertCommonRow(pXFTable,nStartCol,nEndCol);
                i++;
            }
            else
            {
                pRow->ConvertRow(pXFTable,nStartCol,nEndCol);
                i += pRow->GetCurMaxSpannedRows(nStartCol,nEndCol);
            }
        }
    }
}

/**
 * @short   apply numeric value and formula to cell
 * @param  pFoundry - pointer of foundry
 * @param  aTableID - table ID
 */
void LwpTableLayout::PutCellVals(LwpFoundry* pFoundry, LwpObjectID aTableID)
{

    //Comment:The old code doesn't check if the LwpFoundry pointer is NULL,
    //        So the NULL pointer cause sodc frozen. Add code to check the
    //        the pointer.
    //New Code
    if( !pFoundry ) return;

    try{

        LwpDLVListHeadHolder* pHolder = dynamic_cast<LwpDLVListHeadHolder*>(pFoundry->GetNumberManager().GetTableRangeID().obj().get());

        LwpTableRange* pTableRange = pHolder ? dynamic_cast<LwpTableRange*>(pHolder->GetHeadID().obj().get()) : nullptr;

        //Look up the table
        while (nullptr!=pTableRange)
        {
            LwpObjectID aID = pTableRange->GetTableID();
            if (aID == aTableID)
            {
                break;
            }
            pTableRange = pTableRange->GetNext();
        }

        if (!pTableRange)
            return;

        LwpCellRange* pRange = dynamic_cast<LwpCellRange*>(pTableRange->GetCellRangeID().obj().get());
        if (!pRange)
            return;

        LwpFolder* pFolder = dynamic_cast<LwpFolder*>(pRange->GetFolderID().obj().get());
        if (!pFolder)
            return;

        LwpObjectID aRowListID = pFolder->GetChildHeadID();
        LwpRowList* pRowList = dynamic_cast<LwpRowList*>(aRowListID.obj().get());

        //loop the rowlist
        while( nullptr!=pRowList)
        {
            sal_uInt16 nRowID =  pRowList->GetRowID();
            {
                LwpCellList* pCellList = dynamic_cast<LwpCellList*>(pRowList->GetChildHeadID().obj().get());
                //loop the cellList
                while( nullptr!=pCellList)
                {
                    {//put cell
                        sal_uInt16 nColID = pCellList->GetColumnID();

                        XFCell* pCell = GetCellsMap(nRowID,static_cast<sal_uInt8>(nColID));
                        if (pCell)
                        {
                            pCellList->Convert(pCell, this);

                            //process paragraph
                            PostProcessParagraph(pCell, nRowID, nColID);
                        }
                        else
                        {
                            //Hidden cell would not be in cellsmap
                            assert(false);
                        }
                    }
                    pCellList = dynamic_cast<LwpCellList*>(pCellList->GetNextID().obj().get());
                }
            }
            pRowList = dynamic_cast<LwpRowList*>(pRowList->GetNextID().obj().get());
        }

    }catch (...) {
        assert(false);
    }
}

/**
 * @short   1. set number right alignment to right if number 2. remove tab added before if number
 * @param  pCell - cell which to be process
 * @param  nRowID - row number in Word Pro file
 * @param  nColID - column number in Word Pro file
 */
void LwpTableLayout::PostProcessParagraph(XFCell *pCell, sal_uInt16 nRowID, sal_uInt16 nColID)
{
    // if number right, set alignment to right
    LwpCellLayout * pCellLayout = GetCellByRowCol(nRowID, nColID);
    if(pCellLayout)
    {
        rtl::Reference<XFContent> first(
            pCell->FindFirstContent(enumXFContentPara));
        XFParagraph * pXFPara = static_cast<XFParagraph*>(first.get());
        if (!pXFPara)
            return;
        XFColor aNullColor = XFColor();

        if ( pXFPara)
        {
            OUString sNumfmt = pCellLayout->GetNumfmtName();
            bool bColorMod = false;
            XFNumberStyle* pNumStyle = nullptr;
            XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
            if (!sNumfmt.isEmpty())
            {
                pNumStyle = static_cast<XFNumberStyle*>(pXFStyleManager->FindStyle( sNumfmt));
                XFColor aColor = pNumStyle->GetColor();
                if ( aColor != aNullColor )
                    bColorMod = true;//end
            }

            XFParaStyle * pStyle = pXFStyleManager->FindParaStyle(pXFPara->GetStyleName());
            if ((pStyle && pStyle->GetNumberRight()) || bColorMod)
            {
                XFParaStyle* pOverStyle = new XFParaStyle;

                if (pStyle)
                {
                    *pOverStyle = *pStyle;

                    if (pStyle->GetNumberRight())
                        pOverStyle->SetAlignType(enumXFAlignEnd);
                }

                if (bColorMod)
                {
                    rtl::Reference<XFFont> xFont = pOverStyle->GetFont();
                    if (xFont.is())
                    {
                        XFColor aColor = xFont->GetColor();
                        if ( aColor == aNullColor )
                        {
                            rtl::Reference<XFFont> pNewFont = new XFFont;
                            aColor = pNumStyle->GetColor();
                            pNewFont->SetColor(aColor);
                            pOverStyle->SetFont(pNewFont);
                        }
                    }
                }

                pOverStyle->SetStyleName("");
                OUString StyleName = pXFStyleManager->AddStyle(pOverStyle).m_pStyle->GetStyleName();

                pXFPara->SetStyleName(StyleName);
            }
        }
    }
}

/**
 * @short   Parse all cols of table
 * @param  pXFTable - pointer to created XFTable
 */
void LwpTableLayout::ConvertColumn(rtl::Reference<XFTable> const & pXFTable, sal_uInt8 nStartCol, sal_uInt8 nEndCol)
{
    LwpTable * pTable = GetTable();
    if (!pTable)
    {
        assert(false);
        return;
    }

    for (sal_uInt32 iLoop = 0; iLoop < static_cast<sal_uInt32>(nEndCol)-nStartCol; ++iLoop)
    {
        // add row to table
        LwpObjectID& rColID = GetColumnLayoutHead();
        LwpColumnLayout * pColumnLayout = dynamic_cast<LwpColumnLayout *>(rColID.obj().get());
        while (pColumnLayout)
        {
            if (pColumnLayout->GetColumnID() == (iLoop+nStartCol))
            {
                pXFTable->SetColumnStyle(iLoop+1,  pColumnLayout->GetStyleName());
                break;
            }
            rColID = pColumnLayout->GetNext();
            pColumnLayout = dynamic_cast<LwpColumnLayout *>(rColID.obj().get());
        }
        if (!pColumnLayout)
        {
            pXFTable->SetColumnStyle(iLoop+1, m_DefaultColumnStyleName);
        }
    }
}
/**
 * @short   split conflict merged cells
 */
void LwpTableLayout::SplitConflictCells()
{
    LwpTable * pTable = GetTable();
    if (!pTable)
        return;
    sal_uInt16 nCol = pTable->GetColumn();
    sal_uInt16 nRow = pTable->GetRow();

    sal_uInt16 nEffectRows;
    std::map<sal_uInt16,LwpRowLayout*>::iterator iter1;
    std::map<sal_uInt16,LwpRowLayout*>::iterator iter2;
    LwpRowLayout* pRowLayout;
    LwpRowLayout* pEffectRow;

    for (sal_uInt16 i=0; i<nRow; )
    {
        iter1 = m_RowsMap.find(i);
        if (iter1 == m_RowsMap.end())//default rows
        {
                i++;
            continue;
        }
        pRowLayout= iter1->second;
        if (!pRowLayout->GetMergeCellFlag())
        {
            i++;
            continue;
        }
        else
        {
            nEffectRows = i + pRowLayout->GetCurMaxSpannedRows(0,(sal_uInt8)nCol);

            for (sal_uInt16 j = i+1; j<nEffectRows; j++)
            {
                iter2 = m_RowsMap.find(j);
                    if (iter2 == m_RowsMap.end())
                        continue;
                    pEffectRow = iter2->second;
                if (!pEffectRow->GetMergeCellFlag())
                    continue;
                else
                    pEffectRow->SetCellSplit(nEffectRows);
            }
            i = nEffectRows;
        }
    }//end for

}
/**
 * @short   add default row which are missing in the file
 * @param   pXFTable - pointer to new created table
 * @param   nStartCol - starting column
 * @param   nEndCol  - end column
 * @return   pXFTable
 */
void LwpTableLayout::ConvertDefaultRow(rtl::Reference<XFTable> const & pXFTable, sal_uInt8 nStartCol,
         sal_uInt8 nEndCol, sal_uInt16 nRowID)
{
    // current row doesn't exist in the file
    XFRow * pRow = new XFRow();
    pRow->SetStyleName(m_DefaultRowStyleName);

    for (sal_uInt16 j =0;j < nEndCol-nStartCol; j++)
    {
        // if table has default cell layout, use it to ConvertCell
        // otherwise use blank cell
        XFCell * pCell = nullptr;
        if (m_pDefaultCellLayout)
        {
            pCell = m_pDefaultCellLayout->ConvertCell(
                GetTable()->GetObjectID(),nRowID,j+nStartCol);
        }
        else
        {
            pCell = new XFCell();
        }
        pRow->AddCell(pCell);
    }

    pXFTable->AddRow(pRow);
}
/**
 * @short   set cell map info
 * @param   pXFCell - pointer to xfcell
 * @param   nRow - row id
 * @param   nCol - column id
 */
void LwpTableLayout::SetCellsMap(sal_uInt16 nRow,sal_uInt8 nCol,XFCell* pXFCell)
{
    std::pair<std::pair<sal_uInt16,sal_uInt8>,XFCell*> cell;
    std::pair<sal_uInt16,sal_uInt8> pos;
    pos.first = nRow;
    pos.second = nCol;
    cell.first = pos;
    cell.second = pXFCell;
    m_CellsMap.insert(cell);
}

/**
 * @short   get cell map info
 * @param   nRow - row id
 * @param   nCol  - column id
 * @return  pXFCell
 */
XFCell* LwpTableLayout::GetCellsMap(sal_uInt16 nRow,sal_uInt8 nCol)
{
    std::pair<sal_uInt16,sal_uInt8> pos;
    pos.first = nRow;
    pos.second = nCol;
    std::map<std::pair<sal_uInt16,sal_uInt8>,XFCell*>::iterator iter;
    iter =  m_CellsMap.find(pos);
    if (iter == m_CellsMap.end())
        return nullptr;
    return iter->second;
}
/**
 * @descr   Get row layout by row id
 * @param   nRow - row id
 */
 LwpRowLayout* LwpTableLayout::GetRowLayout(sal_uInt16 nRow)
{
    LwpObjectID& rRowID = GetChildHead();
    LwpRowLayout * pRowLayout = dynamic_cast<LwpRowLayout *>(rRowID.obj().get());
    while (pRowLayout)
    {
        if(pRowLayout->GetRowID() == nRow)
            return pRowLayout;

        rRowID = pRowLayout->GetNext();
        pRowLayout = dynamic_cast<LwpRowLayout *>(rRowID.obj().get());
    }
    return nullptr;
}

//add end by
LwpColumnLayout::LwpColumnLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm)
    : LwpVirtualLayout(objHdr, pStrm)
    , ccolid(0)
    , cwidth(0)
{}

LwpColumnLayout::~LwpColumnLayout()
{}
void LwpColumnLayout::Read()
{
    LwpObjectStream* pStrm = m_pObjStrm;

    LwpVirtualLayout::Read();

    sal_uInt16 colid;

    colid = pStrm->QuickReaduInt16();   // forced to lushort
    ccolid = (sal_uInt8)colid;  // Phillip
    cwidth = pStrm->QuickReadInt32();

    pStrm->SkipExtra();
}

void LwpColumnLayout::RegisterStyle(double dCalculatedWidth)
{
    XFColStyle * pColStyle = new XFColStyle();
    pColStyle->SetWidth(static_cast<float>(dCalculatedWidth));
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    m_StyleName = pXFStyleManager->AddStyle(pColStyle).m_pStyle->GetStyleName();
}

LwpTableHeadingLayout::LwpTableHeadingLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm)
    : LwpTableLayout(objHdr, pStrm)
    , cStartRow(0)
    , cEndRow(0)
{}

LwpTableHeadingLayout::~LwpTableHeadingLayout()
{}
/**
 * @short   read table heading layout
 * @param
 * @return
 */
void LwpTableHeadingLayout::Read()
{
    LwpTableLayout::Read();

    cStartRow = m_pObjStrm->QuickReaduInt16();
    cEndRow = m_pObjStrm->QuickReaduInt16();

    m_pObjStrm->SkipExtra();

}
/**
 * @short   get start and end row number of table heading
 * @param
 * @return *pStartRow - starting row number
 * @return *pEndRow -   end row number
 */
void LwpTableHeadingLayout::GetStartEndRow(sal_uInt16& nStartRow, sal_uInt16& nEndRow)
{
    nStartRow = cStartRow;
    nEndRow = cEndRow;
}

LwpSuperParallelColumnLayout::LwpSuperParallelColumnLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm):LwpSuperTableLayout(objHdr, pStrm)
{
}
LwpSuperParallelColumnLayout::~LwpSuperParallelColumnLayout()
{}

void LwpSuperParallelColumnLayout::Read()
{
    LwpSuperTableLayout::Read();
    m_pObjStrm->SkipExtra();

}

LwpSuperGlossaryLayout::LwpSuperGlossaryLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm):LwpSuperTableLayout(objHdr, pStrm)
{
}

LwpSuperGlossaryLayout::~LwpSuperGlossaryLayout()
{
}

void LwpSuperGlossaryLayout::Read()
{
    LwpSuperTableLayout::Read();
    m_pObjStrm->SkipExtra();
}

LwpParallelColumnsLayout::LwpParallelColumnsLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm):LwpTableLayout(objHdr, pStrm)
{
}

LwpParallelColumnsLayout::~LwpParallelColumnsLayout()
{
}

void LwpParallelColumnsLayout::Read()
{
    LwpTableLayout::Read();
    m_pObjStrm->SkipExtra();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
