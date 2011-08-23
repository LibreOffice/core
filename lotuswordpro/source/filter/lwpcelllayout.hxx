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
/*************************************************************************
 * Change History
 Mar 2005		 	Created
 ************************************************************************/
#ifndef _LWPCELLLAYOUT_HXX
#define _LWPCELLLAYOUT_HXX
#include <vector>
#include <map>
#include "lwplayout.hxx"
#include "lwptablelayout.hxx"

typedef enum
{
    enumWholeBorder = 0,
    enumNoLeftBorder,
    enumNoBottomBorder,
    enumNoLeftNoBottomBorder,
    enumCellBorderTopLimit = 4,
} LwpCellBorderType;


class XFCell;
class XFCellStyle;
class LwpCellList;
class LwpTable;
class LwpTableLayout;
/**
 * @brief
 * VO_CELLLAYOUT object
 */
class LwpCellLayout : public LwpMiddleLayout
{
public:
    LwpCellLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    virtual ~LwpCellLayout();
    virtual LWP_LAYOUT_TYPE GetLayoutType () { return LWP_CELL_LAYOUT;}
    virtual XFCell* ConvertCell(LwpObjectID aTableID, sal_uInt16 nRow, sal_uInt16 nCol);
    sal_uInt16 GetRowID(){return crowid;};
    sal_uInt8 GetColID(){return ccolid;};
    void RegisterStyle();
    LwpObjectID * GetNumericsObject() {return &cLayNumerics;};
    LwpObjectID * GetPreviousCellStory();
    virtual LwpPara* GetLastParaOfPreviousStory();
    LwpTableLayout * GetTableLayout();
    virtual void SetCellMap(void);
    double GetActualWidth();
    sal_uInt8 GetLeaderChar();
    OUString GetNumfmtName(){return m_NumfmtName;}
protected:
    void Read();
//	LwpTableLayout * GetTableLayout();
    LwpTable * GetTable();
    void ApplyPadding(XFCellStyle* pCellStyle);
    void ApplyBorders(XFCellStyle* pCellStyle);
    // add by , 06/03/2005
    void ApplyPatternFill(XFCellStyle* pCellStyle);
    void ApplyBackGround(XFCellStyle* pCellStyle);
    // end add
    void ApplyBackColor(XFCellStyle* pCellStyle);
    void ApplyWatermark(XFCellStyle* pCellStyle);
    void ApplyProtect(XFCell * pCell, LwpObjectID aTableID);
    void ApplyFmtStyle(XFCellStyle *pCellStyle);
    OUString GetCellStyleName(sal_uInt16 nRow, sal_uInt16 nCol, LwpTableLayout * pTableLayout);
    void RegisterDefaultCell();
    virtual LwpCellBorderType GetCellBorderType(sal_uInt16 nRow, sal_uInt16 nCol, LwpTableLayout * pTableLayout);
    LwpCellLayout * GetCellByRowCol(sal_uInt16 nRow, sal_uInt16 nCol, LwpTableLayout * pTableLayout);
    virtual sal_uInt16 GetLeftColID(sal_uInt16 nCol){return nCol - 1; };
    virtual sal_uInt16 GetBelowRowID(sal_uInt16 nRow){return nRow + 1; };

    sal_uInt16 crowid;
    sal_uInt8	ccolid;
    LwpObjectID cLayNumerics;
    LwpObjectID cLayDiagonalLine;

    enum LeaderDotType
    {
        LDT_NONE = 0,
        LDT_DOTS = 1,
        LDT_DASHES = 2,
        LDT_UNDERSCORES = 3
    };
    LeaderDotType cType;
    OUString m_CellStyleNames[enumCellBorderTopLimit];

    OUString m_NumfmtName;//Add by , to support number color,2005/11/30
private:
    LwpCellList* GetCellList(LwpFoundry* pFoundry, LwpObjectID aTableID, sal_uInt16 nRow, sal_uInt8 nCol);
};

/**
 * @brief
 * VO_HIDDENCELLLAYOUT object
 */
class LwpHiddenCellLayout : public LwpCellLayout
{
public:
    LwpHiddenCellLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    virtual ~LwpHiddenCellLayout();
    virtual LWP_LAYOUT_TYPE GetLayoutType () { return LWP_HIDDEN_CELL_LAYOUT;}
    virtual void Parse(IXFStream* pOutputStream);
    virtual XFCell* ConvertCell(LwpObjectID aTableID, sal_uInt16 nRow, sal_uInt16 nCol);
    void RegisterStyle(){};
    virtual void SetCellMap(void);
protected:
    void Read();
    LwpObjectID cconnectedlayout;
};

/**
 * @brief
 * VO_CONNECTEDCELLLAYOUT object
 */
class LwpConnectedCellLayout : public LwpCellLayout
{
public:
    LwpConnectedCellLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    virtual ~LwpConnectedCellLayout();
    virtual LWP_LAYOUT_TYPE GetLayoutType () { return LWP_CONNECTED_CELL_LAYOUT;}
    virtual void Parse(IXFStream* pOutputStream);
    virtual XFCell* ConvertCell(LwpObjectID aTableID, sal_uInt16 nRow, sal_uInt16 nCol);
    sal_uInt16 GetNumrows(){return m_nRealrowspan;}
    sal_uInt8 GetNumcols(){return m_nRealcolspan;}
    virtual void SetCellMap(void);
    void SetNumrows(sal_uInt16 nVal){m_nRealrowspan = nVal;}
    void SetNumcols(sal_uInt8 nVal){m_nRealcolspan = nVal;}
protected:
    void Read();
    virtual sal_uInt16 GetBelowRowID(sal_uInt16 nRow){return nRow + m_nRealrowspan; };
    virtual LwpCellBorderType GetCellBorderType(sal_uInt16 nRow, sal_uInt16 nCol, LwpTableLayout * pTableLayout);
    sal_uInt16	cnumrows;
    sal_uInt8		cnumcols;
//	sal_Bool m_bSplitFlag;
    sal_uInt16	m_nRealrowspan;
    sal_uInt8	m_nRealcolspan;
};
/**
 * @brief
 * VO_PCOLBLOCK object
 */

class LwpParallelColumnsBlock : public LwpCellLayout
{
public:
    LwpParallelColumnsBlock(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    virtual ~LwpParallelColumnsBlock();
protected:
    void Read();

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
