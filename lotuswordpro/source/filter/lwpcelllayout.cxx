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
 *  For LWP filter architecture prototype - cell layouts
 */

#include "lwpcelllayout.hxx"
#include "lwpfoundry.hxx"
#include "lwpobjfactory.hxx"
#include "lwptblcell.hxx"
#include "lwptblformula.hxx"
#include "lwpholder.hxx"
#include "lwpnumericfmt.hxx"
#include "lwptable.hxx"
#include "lwpglobalmgr.hxx"

#include "xfilter/xfstylemanager.hxx"
#include "xfilter/xfcell.hxx"
#include "xfilter/xfcellstyle.hxx"
#include "xfilter/xfcolstyle.hxx"

LwpCellLayout::LwpCellLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm)
    : LwpMiddleLayout(objHdr, pStrm)
    , crowid(0)
    , ccolid(0)
    , cType(LDT_NONE)
{
}

LwpCellLayout::~LwpCellLayout()
{}

/**
 * @short   Get table layout pointer, if default cell layout, return NULL
 * @param LwpTableLayout *
 * @return
 */
LwpTableLayout * LwpCellLayout::GetTableLayout()
{
    LwpRowLayout * pRow = dynamic_cast<LwpRowLayout *>(GetParent().obj().get());
    if(!pRow)
    {
        return nullptr;
    }
    LwpTableLayout * pTableLayout = pRow->GetParentTableLayout();
    return pTableLayout;
}
/**
 * @short   Get table pointer, if default cell layout, return NULL
 * @param LwpTable *
 * @return
 */
LwpTable * LwpCellLayout::GetTable()
{
    LwpTableLayout * pTableLayout = GetTableLayout();
    if(!pTableLayout)
    {
        return nullptr;
    }
    LwpTable *pTable = pTableLayout->GetTable();
    return pTable;
}
/**
 * @short   Set current cell layout to cell layout map
 * @param
 * @return
 */
void LwpCellLayout::SetCellMap()
{
    LwpTableLayout * pTableLayout = GetTableLayout();
    if (pTableLayout)
        pTableLayout->SetWordProCellMap(crowid, ccolid, this);
}
/**
 * @short  Get actual width of this cell layout
 * @param
 * @return width (cm)
 */
double LwpCellLayout::GetActualWidth()
{
    //Get table layout
    LwpTableLayout * pTableLayout = GetTableLayout();

    if (pTableLayout == nullptr)
    {
        return GetGeometryWidth();
    }

    OUString strColStyle = pTableLayout->GetColumnWidth(ccolid);

    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    XFColStyle *pStyle = static_cast<XFColStyle *>(pXFStyleManager->FindStyle(strColStyle));
    if(pStyle)
    {
        return pStyle->GetWidth();
    }

    return GetGeometryWidth();
}

/**
 * @short   Apply padding to cell style
 * @param pCellStyle - pointer of XFCellStyle
 * @return
 */
void LwpCellLayout::ApplyPadding(XFCellStyle *pCellStyle)
{
    double fLeft = GetMarginsValue(MARGIN_LEFT);
    double fRight = GetMarginsValue(MARGIN_RIGHT);
    double fTop = GetMarginsValue(MARGIN_TOP);
    double fBottom = GetMarginsValue(MARGIN_BOTTOM);
    pCellStyle->SetPadding((float)fLeft,(float)fRight,(float)fTop,(float)fBottom);
}
/**
 * @short   Apply border to cell style according to cell position, default cell layout won't use this function
 * @param
 * @return pCellStyle - pointer of XFCellStyle
 */
void LwpCellLayout::ApplyBorders(XFCellStyle *pCellStyle)
{
    // judge cell border type
    LwpCellBorderType eType = GetCellBorderType(crowid, ccolid, GetTableLayout());

    // get left cell and judge if neighbour border is different
    XFBorders * pBorders = GetXFBorders();
    if(!pBorders)
    {
        return;
    }

    switch (eType)
    {
    case enumNoBottomBorder:
        pBorders->SetWidth(enumXFBorderBottom, 0);
        break;
    case enumNoLeftBorder:
        pBorders->SetWidth(enumXFBorderLeft, 0);
        break;
    case enumNoLeftNoBottomBorder:
        pBorders->SetWidth(enumXFBorderBottom, 0);
        pBorders->SetWidth(enumXFBorderLeft, 0);
        break;
    case enumWholeBorder:
        break;
    default:
        assert(false);
    }
    pCellStyle->SetBorders(pBorders);
}
/**
 * @short   Apply watermark to cell style
 * @param pCellStyle - pointer of XFCellStyle
 * @return
 */
void LwpCellLayout::ApplyWatermark(XFCellStyle *pCellStyle)
{
    XFBGImage* pBGImage = GetXFBGImage();
    if(pBGImage)
    {
        pCellStyle->SetBackImage(pBGImage);
    }
}

/**
 * @short   Apply pattern fill to cell style
 * @param pCellStyle - pointer of XFCellStyle
 * @return
 */
void LwpCellLayout::ApplyPatternFill(XFCellStyle* pCellStyle)
{
    XFBGImage* pXFBGImage = GetFillPattern();
    if (pXFBGImage)
    {
        pCellStyle->SetBackImage(pXFBGImage);
    }
}

/**
 * @short   Apply background to cell style
 * @param pCellStyle - pointer of XFCellStyle
 * @return
 */
void LwpCellLayout::ApplyBackGround(XFCellStyle* pCellStyle)
{
    if (IsPatternFill())
    {
        ApplyPatternFill(pCellStyle);
    }
    else
    {
        ApplyBackColor(pCellStyle);
    }
}
/**
 * @short   Apply back color to cell style
 * @param pCellStyle - pointer of XFCellStyle
 * @return
 */
void LwpCellLayout::ApplyBackColor(XFCellStyle *pCellStyle)
{
    LwpColor* pColor = GetBackColor();
    if(pColor && pColor->IsValidColor())
    {
        XFColor aXFColor(pColor->To24Color());
        pCellStyle->SetBackColor(aXFColor);
    }
}
/**
 * @short   register style of cell layout
 * @param pCellStyle The style of the cell, which would be applied to the cell.
 * @return
 */
void LwpCellLayout::ApplyFmtStyle(XFCellStyle *pCellStyle)
{
    LwpLayoutNumerics* pLayoutNumerics = dynamic_cast<LwpLayoutNumerics*>(cLayNumerics.obj().get());
    if (!pLayoutNumerics)
    {
        // if current layout doesn't have format, go to based on layout
        LwpCellLayout* pCellLayout = dynamic_cast<LwpCellLayout*>(GetBasedOnStyle().get());
        if (pCellLayout)
        {
            pLayoutNumerics = dynamic_cast<LwpLayoutNumerics*>(pCellLayout->GetNumericsObject().obj().get());
        }
    }

    // apply format style
    if (pLayoutNumerics)
    {
        XFStyle* pStyle = pLayoutNumerics->Convert();
        if (pStyle)
        {
            XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
            m_NumfmtName = pXFStyleManager->AddStyle(pStyle).m_pStyle->GetStyleName();
            pCellStyle->SetDataStyle(m_NumfmtName);
        }
    }

    return;
}
/**
 * @short   get style name according to cell position, only table default cells use this function
 * @param   nRow - default cell position row number
 * @param   nCol - default cell position col number
 * @return  OUString - registered cell style name
 */
OUString const & LwpCellLayout::GetCellStyleName(sal_uInt16 nRow, sal_uInt16 nCol, LwpTableLayout * pTableLayout)
{
    // judge cell border type
    LwpCellBorderType eType = GetCellBorderType(nRow, nCol, pTableLayout);
    return m_CellStyleNames[eType];
}
/**
*   Make the XFCell
*   @param  aTableID - ID of the table which this cell belongs to
*   @param  bIsTopRow - whether current cell is top row
*   @param  bIsRightCol - whether current cell is the rightest column
*   @return XFCell*
*/
XFCell* LwpCellLayout::ConvertCell(LwpObjectID aTableID, sal_uInt16 nRow, sal_uInt16 nCol)
{
    // if cell layout is aTableID's default cell layout
    // it can't have any content, bypass these code
    LwpTable * pTable = dynamic_cast<LwpTable *>(aTableID.obj().get());
    if (!pTable)
    {
        assert(false);
        return nullptr;
    }
    XFCell * pXFCell = new XFCell();
    OUString aStyleName = m_StyleName;

    // if cell layout is aTableID's default cell layout
    // we should adjust its style by current position
    if (pTable->GetDefaultCellStyle() == GetObjectID())
    {
        aStyleName = GetCellStyleName(nRow, nCol, pTable->GetTableLayout().get());
    }

    // content of cell
    LwpStory* pStory = dynamic_cast<LwpStory*>(m_Content.obj().get());
    if (pStory)
    {
        pStory->XFConvert(pXFCell);
    }

    ApplyProtect(pXFCell, aTableID);
    pXFCell->SetStyleName(aStyleName);
    return pXFCell;
}

LwpPara* LwpCellLayout::GetLastParaOfPreviousStory()
{
    LwpObjectID* pPreStoryID = GetPreviousCellStory();
    if (pPreStoryID && !(pPreStoryID->IsNull()))
    {
        LwpStory* pPreStory = dynamic_cast<LwpStory*>(pPreStoryID->obj(VO_STORY).get());
        return dynamic_cast<LwpPara*>(pPreStory->GetLastPara().obj(VO_PARA).get());
    }
    else
    {
        return nullptr;
    }
}

/**
 * @short    Get previous cell which used for bullet inside cell
 * @param
 * @return   LwpObjectID * - object ID of cell content story
 */
LwpObjectID * LwpCellLayout::GetPreviousCellStory()
{
    LwpTable *pTable = GetTable();
    if (!pTable)
    {
        assert(false);
        return nullptr;
    }
    sal_uInt16 nRow = crowid;
    sal_uInt16 nCol = ccolid;

    // if table is reset paragraph in columns, get cell on the top side of current cell
    if (pTable->IsNumberDown())
    {
        if (nRow == 0)
        {
            return nullptr;
        }
        nRow -=1;
    }
    else
    {
        // if not, get cell on the left side of current cell
        if (nCol == 0)
        {
            if (nRow == 0)
            {
                return nullptr;
            }
            else
            {
                nRow--;
                nCol = pTable->GetColumn() - 1;
            }
        }
        else
        {
            nCol -=1;
        }
    }

    // get the object id pointer of previous cell story
    LwpTableLayout * pTableLayout = GetTableLayout();
    if (!pTableLayout)
    {
        assert(false);
        return nullptr;
    }
    return pTableLayout->SearchCellStoryMap(nRow, nCol);
}

/**
 * @short   judge border type by cell neighbour
 * @param nRow
 * @param nCol
 * @param pTableLayout
 * @return   LwpCellBorderType
 */
LwpCellBorderType LwpCellLayout::GetCellBorderType(sal_uInt16 nRow, sal_uInt16 nCol, LwpTableLayout * pTableLayout)
{
    if (!pTableLayout)
        return enumWholeBorder;

    // get left cell and judge if neighbour border is different
    XFBorders * pBorders = GetXFBorders();
    if(!pBorders)
    {
        return enumWholeBorder;
    }
    XFBorder& rLeftBorder = pBorders->GetLeft();
    XFBorder& rBottomBorder = pBorders->GetBottom();
    bool bNoLeftBorder = false;
    bool bNoBottomBorder = false;

    LwpCellLayout * pLeftNeighbour = GetCellByRowCol(nRow, GetLeftColID(nCol), pTableLayout);
    if (pLeftNeighbour)
    {
        XFBorders * pNeighbourBorders = pLeftNeighbour->GetXFBorders();
        if (pNeighbourBorders)
        {
            XFBorder& rRightBorder = pNeighbourBorders->GetRight();
            if (rLeftBorder == rRightBorder)
            {
                // for these 2 types cell, left border should be ignored for sake of avoiding duplication border
                // but if left border is different with right border of left cell
                // we should not ignored it
                bNoLeftBorder = true;
            }
            delete pNeighbourBorders;
        }

    }

    LwpCellLayout * pBelowNeighbour = GetCellByRowCol(GetBelowRowID(nRow), nCol, pTableLayout);
    if (pBelowNeighbour) //&& (eType == enumRightNotLastCellBorder || eType ==  enumLeftNotLastCellBorder) )
    {
        XFBorders * pBelowBorders = pBelowNeighbour->GetXFBorders();
        if (pBelowBorders)
        {
            XFBorder& rTopBorder = pBelowBorders->GetTop();
            if (rTopBorder == rBottomBorder)
            {
                // for these 2 types cell, bottom border should be ignored for sake of avoiding duplication border
                // but if bottom border is different with right border of left cell
                // we should not ignored it
                bNoBottomBorder = true;
            }
            delete pBelowBorders;
        }
    }

    delete pBorders;

    if (bNoBottomBorder)
    {
        if (bNoLeftBorder)
        {
            return enumNoLeftNoBottomBorder;
        }
        return enumNoBottomBorder;
    }
    if (bNoLeftBorder)
    {
        return enumNoLeftBorder;
    }
    return enumWholeBorder;
}

/**
 * @short   Get neighbour cell by specifying ROW+COL
 * @param   nRow
 * @param   nCol
 * @return   LwpCellLayout *
 */
LwpCellLayout * LwpCellLayout::GetCellByRowCol(sal_uInt16 nRow, sal_uInt16 nCol, LwpTableLayout * pTableLayout)
{
    return pTableLayout->GetCellByRowCol(nRow, nCol);
}
/**
 * @short   Register table's default cell layout
 * @param
 * @return
 */
void LwpCellLayout::RegisterDefaultCell()
{
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    for (sal_uInt16 eLoop = enumWholeBorder; eLoop < enumCellBorderTopLimit; eLoop++)
    {
        // register cell style
        XFCellStyle *pCellStyle = new XFCellStyle();

        ApplyPadding(pCellStyle);
        ApplyBackColor(pCellStyle);
        ApplyWatermark(pCellStyle);
        ApplyFmtStyle(pCellStyle);
        pCellStyle->SetAlignType(enumXFAlignNone, GetVerticalAlignmentType());

        XFBorders * pBorders = GetXFBorders();
        if (pBorders)
        {
            switch(eLoop)
            {
            case enumNoBottomBorder:

                //| |

                // remove bottom line
                pBorders->SetWidth(enumXFBorderBottom, 0);
                break;
            case enumNoLeftNoBottomBorder:

                //  |

                // remove left and bottom
                pBorders->SetWidth(enumXFBorderLeft, 0);
                pBorders->SetWidth(enumXFBorderBottom, 0);
                break;
            case enumWholeBorder:

                //||

                // nothing to remove
                break;
            case enumNoLeftBorder:

                //| |

                // remove left line
                pBorders->SetWidth(enumXFBorderLeft, 0);
                break;
            default:
                assert(false);
            }
            pCellStyle->SetBorders(pBorders);
        }
        m_CellStyleNames[eLoop] = (pXFStyleManager->AddStyle(pCellStyle)).m_pStyle->GetStyleName();
    }
}
/**
 * @short    Register 4 types of cell style and register content styles
 * @param
 * @param
 * @param
 * @return
 */
void LwpCellLayout::RegisterStyle()
{
    rtl::Reference<LwpVirtualLayout> xParent(dynamic_cast<LwpVirtualLayout *>(GetParent().obj().get()));
    if (!xParent.is() || xParent->GetLayoutType() != LWP_ROW_LAYOUT)
    {
        // default cell layout, we must register 4 styles for it
        RegisterDefaultCell();
        return;
    }

    // register cell style
    XFCellStyle *pCellStyle = new XFCellStyle();

    ApplyPadding(pCellStyle);
    ApplyBackGround(pCellStyle);
    ApplyWatermark(pCellStyle);
    ApplyFmtStyle(pCellStyle);
    ApplyBorders(pCellStyle);

    pCellStyle->SetAlignType(enumXFAlignNone, GetVerticalAlignmentType());

    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    m_StyleName = (pXFStyleManager->AddStyle(pCellStyle)).m_pStyle->GetStyleName();

    // content object register styles
    rtl::Reference<LwpObject> pObj = m_Content.obj();
    if (pObj.is())
    {
        pObj->SetFoundry(m_pFoundry);
        pObj->DoRegisterStyle();
    }

    //register child layout style
    RegisterChildStyle();
}
/**
 * @short   Read cell layout
 * @param
 * @return
 */
void LwpCellLayout::Read()
{
    LwpObjectStream* pStrm = m_pObjStrm;

    LwpMiddleLayout::Read();

    // before the layout hierarchy rework
    if (LwpFileHeader::m_nFileRevision  < 0x000b)
    {
        assert(false);
    }
    else
    {
        crowid = pStrm->QuickReaduInt16();
        ccolid = (sal_uInt8) pStrm->QuickReaduInt16();  // written as a lushort

        sal_uInt16 type;

        type = pStrm->QuickReaduInt16();
        pStrm->SkipExtra();
        cType = (LeaderDotType)type;

        cLayNumerics.ReadIndexed(pStrm);
        cLayDiagonalLine.ReadIndexed(pStrm);

        pStrm->SkipExtra();
    }
}

/**
*   Apply protect attribute to cell of table
*   @param  aTableID - ID of the table which the cell belongs to
*   @param
*   @return XFCell*
*/
void LwpCellLayout::ApplyProtect(XFCell * pCell, LwpObjectID aTableID)
{
    bool bProtected = false;
    // judge current cell
    if (GetIsProtected())
    {
        bProtected = true;
    }
    else
    {
        // judge base on
        LwpCellLayout * pBase = dynamic_cast<LwpCellLayout *>(GetBasedOnStyle().get());
        if (pBase && pBase->GetIsProtected())
        {
            bProtected = true;
        }
        else
        {
            // judge whole table
            LwpTable * pTable = dynamic_cast<LwpTable *>(aTableID.obj().get());
            rtl::Reference<LwpTableLayout> xTableLayout(pTable ? pTable->GetTableLayout() : nullptr);
            LwpSuperTableLayout * pSuper = xTableLayout.is() ? xTableLayout->GetSuperTableLayout() : nullptr;
            if (pSuper && pSuper->GetIsProtected())
            {
                bProtected = true;
            }
        }
    }

    pCell->SetProtect(bProtected);
}

LwpConnectedCellLayout::LwpConnectedCellLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm)
    : LwpCellLayout(objHdr, pStrm)
    , cnumrows(0)
    , cnumcols(0)
    , m_nRealrowspan(0)
    , m_nRealcolspan(0)
{
}

LwpConnectedCellLayout::~LwpConnectedCellLayout()
{}
/**
 * @short   Set current connected cell layout to cell layout map
 * @param pCellLayoutMap - cell layout map reference
 * @return
 */
void LwpConnectedCellLayout::SetCellMap()
{
    LwpTableLayout * pTableLayout = GetTableLayout();
    if (!pTableLayout)
        return;

    sal_uInt16 nRowSpan = m_nRealrowspan;

    for (sal_uInt16 iLoop = 0; iLoop < nRowSpan; iLoop ++)
    {
        for (sal_uInt16 jLoop = 0; jLoop < cnumcols; jLoop ++)
            pTableLayout->SetWordProCellMap(iLoop + crowid, jLoop + ccolid, this);
    }
}

/**
 * @short   judge border type by cell neighbour
 * @param nRow
 * @param nCol
 * @param pTableLayout
 * @return   LwpCellBorderType
 */
LwpCellBorderType LwpConnectedCellLayout::GetCellBorderType(sal_uInt16 nRow, sal_uInt16 nCol, LwpTableLayout * pTableLayout)
{
    if (!pTableLayout)
    {
        assert(false);
        return enumWholeBorder;
    }

    sal_uInt16 nRowSpan = m_nRealrowspan;

    // get left cell and judge if neighbour border is different
    XFBorders * pBorders = GetXFBorders();
    if(!pBorders)
    {
        return enumWholeBorder;
    }
    XFBorder& rLeftBorder = pBorders->GetLeft();
    XFBorder& rBottomBorder = pBorders->GetBottom();
    bool bNoLeftBorder = true;
    bool bNoBottomBorder = true;

    if (nCol == 0)
    {
        bNoLeftBorder = false;
    }
    else
    {
        for (sal_uInt16 iLoop=0; iLoop < nRowSpan; iLoop++)
        {
            LwpCellLayout * pLeftNeighbour = GetCellByRowCol(nRow+iLoop, GetLeftColID(nCol), pTableLayout);
            if (pLeftNeighbour)
            {
                std::unique_ptr<XFBorders> pNeighbourBorders(pLeftNeighbour->GetXFBorders());
                if (pNeighbourBorders)
                {
                    XFBorder& rRightBorder = pNeighbourBorders->GetRight();
                    if (rLeftBorder != rRightBorder)
                    {
                        // if left border is different with right border of left cell
                        // we should not ignored it
                        bNoLeftBorder = false;
                        break;
                    }
                }
            }
        }
    }

    LwpTable* pTable = pTableLayout->GetTable();
    if (!pTable)
        throw std::runtime_error("missing table");

    if ( (nRow + nRowSpan) == pTable->GetRow())
    {
        bNoBottomBorder = false;
    }
    else
    {
        for (sal_uInt16 iLoop = 0; iLoop < cnumcols; iLoop ++)
        {
            LwpCellLayout * pBelowNeighbour = GetCellByRowCol(nRow + nRowSpan, nCol+iLoop, pTableLayout);
            if (pBelowNeighbour)
            {
                std::unique_ptr<XFBorders> pBelowBorders(pBelowNeighbour->GetXFBorders());
                if (pBelowBorders)
                {
                    XFBorder& rTopBorder = pBelowBorders->GetTop();
                    if (rTopBorder != rBottomBorder)
                    {
                        // if bottom border is different with right border of left cell
                        // we should not ignored it
                        bNoBottomBorder = false;
                        break;
                    }
                }
            }
        }
    }
    delete pBorders;

    if (bNoBottomBorder)
    {
        if (bNoLeftBorder)
        {
            return enumNoLeftNoBottomBorder;
        }
        return enumNoBottomBorder;
    }
    if (bNoLeftBorder)
    {
        return enumNoLeftBorder;
    }
    return enumWholeBorder;
}
/**
 * @short   Read connected cell layout
 * @param
 * @return
 */
void LwpConnectedCellLayout::Read()
{
    LwpCellLayout::Read();
    sal_uInt16 numcols;

    cnumrows = m_pObjStrm->QuickReaduInt16();
    numcols = m_pObjStrm->QuickReaduInt16();        // written as a lushort
    cnumcols = (sal_uInt8)numcols;

    m_nRealrowspan = cnumrows;
    m_nRealcolspan = cnumcols;

    m_pObjStrm->SkipExtra();
}
XFCell* LwpConnectedCellLayout::ConvertCell(LwpObjectID aTableID, sal_uInt16 nRow, sal_uInt16 nCol)
{
    XFCell * pXFCell = LwpCellLayout::ConvertCell(aTableID, nRow, nCol);
    pXFCell->SetColumnSpaned(cnumcols);
//  if(!m_bSplitFlag)
//  {
//  }
    return pXFCell;
}
/**
 * @short   parse connected cell layout
 * @param pOutputStream - output stream
 * @return
 */
 void  LwpConnectedCellLayout::Parse(IXFStream* /*pOutputStream*/)
{
}

LwpHiddenCellLayout::LwpHiddenCellLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm)
    : LwpCellLayout(objHdr, pStrm)
{}

LwpHiddenCellLayout::~LwpHiddenCellLayout()
{}
/**
 * @short   Set current hidden cell layout to cell layout map
 * @param
 * @return
 */
void LwpHiddenCellLayout::SetCellMap()
{
    return;
}
/**
 * @short   Read hidden cell layout
 * @param
 * @return
 */
void LwpHiddenCellLayout::Read()
{
    LwpCellLayout::Read();

    cconnectedlayout.ReadIndexed(m_pObjStrm);
    m_pObjStrm->SkipExtra();

}
/**
 * @short   Convert hidden cell layout
 * @param aTableID - Object ID of table
 * @return XFCell * - pointer to converted cell
 */

XFCell* LwpHiddenCellLayout::ConvertCell(LwpObjectID aTableID, sal_uInt16 nRow, sal_uInt16 nCol)
{
    if (!cconnectedlayout.obj().is())
        return nullptr;
    LwpConnectedCellLayout* pConnCell = dynamic_cast<LwpConnectedCellLayout* >(cconnectedlayout.obj().get());

    if (!pConnCell || nRow < (pConnCell->GetNumrows()+pConnCell->GetRowID()))
        return nullptr;
    // if the hidden cell should be displayed for limit of SODC
    // use the default cell layout
    XFCell* pXFCell = nullptr;
    LwpTable *pTable = dynamic_cast<LwpTable *>(aTableID.obj().get());
    if (pTable)
    {
        LwpCellLayout *pDefault = dynamic_cast<LwpCellLayout *>(pTable->GetDefaultCellStyle().obj().get());
        if (pDefault)
        {
            pXFCell = pDefault->ConvertCell(aTableID, nRow, nCol);
        }
        else
        {
            pXFCell = pConnCell->ConvertCell(aTableID, nRow, nCol);
        }
        pXFCell->SetColumnSpaned(pConnCell->GetNumcols());
    }
    else
    {
        assert(false);
    }
    return pXFCell;
}
/**
 * @short   parse hidden cell layout
 * @param pOutputStream - output stream
 * @return
 */
 void  LwpHiddenCellLayout::Parse(IXFStream* /*pOutputStream*/)
{
}

LwpParallelColumnsBlock::LwpParallelColumnsBlock(LwpObjectHeader &objHdr, LwpSvStream* pStrm):LwpCellLayout(objHdr, pStrm)
{}

LwpParallelColumnsBlock::~LwpParallelColumnsBlock()
{}

void LwpParallelColumnsBlock::Read()
{
    LwpCellLayout::Read();
    m_pObjStrm->SkipExtra();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
