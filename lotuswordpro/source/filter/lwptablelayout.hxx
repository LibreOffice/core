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

#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPTABLELAYOUT_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPTABLELAYOUT_HXX

#include "lwplayout.hxx"
#include <xfilter/xftable.hxx>
#include <svl/hint.hxx>
#include <svl/lstner.hxx>

#include <mdds/rtree.hpp>

#include <vector>
#include <map>
#include <memory>

class XFTableStyle;
class XFTable;
class XFCell;
class LwpTable;
class LwpFrame;
class LwpSuperTableLayout;
class LwpRowLayout;
class LwpCellLayout;
class LwpTableHeadingLayout;
class LwpRowHeadingLayout;
class LwpConnectedCellLayout;
class LwpColumnLayout;

struct TableConvertAttempt
{
    sal_uInt16 mnStartRow;
    sal_uInt16 mnEndRow;
    sal_uInt8 mnStartCol;
    sal_uInt8 mnEndCol;

    TableConvertAttempt(sal_uInt16 nStartRow, sal_uInt16 nEndRow, sal_uInt8 nStartCol, sal_uInt8 nEndCol)
        : mnStartRow(nStartRow), mnEndRow(nEndRow), mnStartCol(nStartCol), mnEndCol(nEndCol)
    {
    }
};

class XFCellListener : public SfxListener
{
public:
    XFCellListener(XFCell* pCell)
        : m_pCell(pCell)
    {
        if (m_pCell)
            StartListening(*m_pCell);
    }

    XFCell* GetCell()
    {
        return m_pCell;
    }

private:
    XFCell* m_pCell;

    virtual void Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint) override
    {
        if (rHint.GetId() == SfxHintId::Dying)
            m_pCell = nullptr;
    }
};

/**
 * @brief
 * VO_TABLELAYOUT object and functions for registering styles and converting tables
 */
class LwpTableLayout: public LwpLayout
{
public:
    LwpTableLayout(LwpObjectHeader const &objHdr, LwpSvStream* pStrm);
    virtual LWP_LAYOUT_TYPE GetLayoutType () override { return LWP_TABLE_LAYOUT;}
    LwpObjectID& GetColumnLayoutHead(){return m_ColumnLayout;}
    void RegisterStyle() override;
    LwpTable *  GetTable();
    LwpCellLayout * GetDefaultCellLayout(){return m_pDefaultCellLayout;}
    LwpSuperTableLayout * GetSuperTableLayout();
    LwpObjectID * SearchCellStoryMap(sal_uInt16 nRow, sal_uInt16 nCol);
    LwpCellLayout * GetCellByRowCol(sal_uInt16 nRow, sal_uInt16 nCol);
    OUString GetColumnWidth(sal_uInt16 nCol);
    void SetWordProCellMap(sal_uInt16 nRow, sal_uInt16 nCol, LwpCellLayout * pCell)
    {
        if (nRow >= m_nRows || nCol >= m_nCols)
            return;
        m_WordProCellsMap[static_cast<size_t>(nRow) * m_nCols + nCol] = pCell;
    };
protected:
    void Read() override;
    void TraverseTable();
    void RegisterColumns();
    void RegisterRows();
    void ParseTable();
    void PostProcessParagraph(XFCell *pCell, sal_uInt16 nRowID, sal_uInt16 nColID);

public:
    void XFConvert(XFContentContainer* pCont) override;
    void ConvertTable(rtl::Reference<XFTable> const & pXFTable, sal_uInt16 nStartRow,
                sal_uInt16 nEndRow,sal_uInt8 nStartCol,sal_uInt8 nEndCol);
    const OUString& GetDefaultRowStyleName() const {return m_DefaultRowStyleName;}
    void SetCellsMap(sal_uInt16 nRow1, sal_uInt8 nCol1, sal_uInt16 nRow2, sal_uInt8 nCol2, XFCell* pXFCell);
    XFCell* GetCellsMap(sal_uInt16 nRow,sal_uInt8 nCol);
   const  std::map<sal_uInt16,LwpRowLayout*>& GetRowsMap() const {return m_RowsMap;}
    LwpRowLayout* GetRowLayout(sal_uInt16 nRow);

private:
    void ConvertDefaultRow(rtl::Reference<XFTable> const & pXFTable, sal_uInt8 nStartCol,
                sal_uInt8 nEndCol, sal_uInt16 nRowID);
    void ConvertColumn(rtl::Reference<XFTable> const & pXFTable, sal_uInt8 nStartCol, sal_uInt8 nEndCol);
    sal_uInt16 ConvertHeadingRow(rtl::Reference<XFTable> const & pXFTable,sal_uInt16 nStartHeadRow,sal_uInt16 nEndHeadRow);
    static bool FindSplitColMark(XFTable* pXFTable, std::vector<sal_uInt8>& rCellMark, sal_uInt8& nMaxColSpan);
    void SplitRowToCells(XFTable* pTmpTable, rtl::Reference<XFTable> const & pXFTable,
                sal_uInt8 nFirstColSpann, const sal_uInt8* pCellMark);

    void SplitConflictCells();

    void PutCellVals(LwpFoundry* pFoundry, LwpObjectID aTableID);

    sal_uInt16 m_nRows;
    sal_uInt16 m_nCols;

    std::vector<TableConvertAttempt> m_aConvertingStack;

    //CColumnLayoutHead cColumnLayout;
    LwpObjectID m_ColumnLayout;
    LwpCellLayout * m_pDefaultCellLayout;
    OUString m_DefaultColumnStyleName;
    OUString m_DefaultRowStyleName;

    // wordpro cell  map
    std::vector<LwpCellLayout*> m_WordProCellsMap;
    // column vector
    std::vector<LwpColumnLayout*> m_aColumns;
    std::map<sal_uInt16,LwpRowLayout*> m_RowsMap;
    rtl::Reference<XFTable> m_pXFTable;
    bool m_bConverted;
    using rt_type = mdds::rtree<int, XFCellListener>;
    rt_type m_CellsMap;
};

/**
 * @brief
 * VO_SUPERTABLELAYOUT object
 */
class LwpSuperTableLayout: public LwpPlacableLayout
{
public:
    LwpSuperTableLayout(LwpObjectHeader const &objHdr, LwpSvStream* pStrm);
    virtual ~LwpSuperTableLayout() override;
    virtual LWP_LAYOUT_TYPE GetLayoutType () override { return LWP_SUPERTABLE_LAYOUT;}
    void RegisterNewStyle();
    // for table style
    void ApplyPatternFill(XFTableStyle* pTableStyle);
    void ApplyBackGround(XFTableStyle* pTableStyle);

    void ApplyBackColor(XFTableStyle *pTableStyle);
    void ApplyShadow(XFTableStyle *pTableStyle);
    double GetWidth() override;
    double GetTableWidth();
    void ApplyWatermark(XFTableStyle *pTableStyle);
    void ApplyAlignment(XFTableStyle * pTableStyle);
    void XFConvert(XFContentContainer* pCont) override;
    // for frame style
    virtual void XFConvertFrame(XFContentContainer* pCont, sal_Int32 nStart = 0, sal_Int32 nEnd = 0, bool bAll = false) override;
    void RegisterFrameStyle();
    LwpTableHeadingLayout* GetTableHeadingLayout();
protected:
    void Read() override;
    LwpTableLayout* GetTableLayout();
    bool IsSizeRightToContent();
    bool IsJustifiable();
    std::unique_ptr<LwpFrame> m_pFrame;
};

/**
 * @brief
 * VO_COLUMNLAYOUT object
 */
class LwpColumnLayout : public LwpVirtualLayout
{
public:
    LwpColumnLayout(LwpObjectHeader const &objHdr, LwpSvStream* pStrm);
    virtual ~LwpColumnLayout() override;
    virtual LWP_LAYOUT_TYPE GetLayoutType () override { return LWP_COLUMN_LAYOUT;}
    sal_uInt32 GetColumnID() const {return ccolid;}
    double GetWidth() override { return LwpTools::ConvertFromUnits(cwidth); }
    using LwpVirtualLayout::RegisterStyle;
    void RegisterStyle(double dCalculatedWidth);
    bool IsJustifiable() const {return (( m_nAttributes2 & STYLE2_JUSTIFIABLE) != 0);}
protected:
    void Read() override;
private:
    sal_uInt8 ccolid;
    sal_Int32 cwidth;

};
/**
 * @brief
 * VO_TABLEHEADINGLAYOUT object
 */
class LwpTableHeadingLayout : public LwpTableLayout
{
public:
    LwpTableHeadingLayout(LwpObjectHeader const &objHdr, LwpSvStream* pStrm);
    virtual ~LwpTableHeadingLayout() override;
    virtual LWP_LAYOUT_TYPE GetLayoutType () override { return LWP_TABLE_HEADING_LAYOUT;}
    void GetStartEndRow(sal_uInt16& nStartRow, sal_uInt16& nEndRow);
protected:
    void Read() override;
private:
    sal_uInt16 cStartRow;
    sal_uInt16 cEndRow;
};
/**
 * @brief
 * VO_SUPERPARALLELCOLUMNLAYOUT object
 */
class LwpSuperParallelColumnLayout : public LwpSuperTableLayout
{
public:
    LwpSuperParallelColumnLayout(LwpObjectHeader const &objHdr, LwpSvStream* pStrm);
    virtual ~LwpSuperParallelColumnLayout() override;
protected:
    void Read() override;
};

/**
 * @brief
 * VO_PCOLLAYOUT object
 */
class LwpParallelColumnsLayout : public LwpTableLayout
{
public:
    LwpParallelColumnsLayout(LwpObjectHeader const &objHdr, LwpSvStream* pStrm);
    virtual ~LwpParallelColumnsLayout() override;
protected:
    void Read() override;
};

class LwpSuperGlossaryLayout : public LwpSuperTableLayout
{
public:
    LwpSuperGlossaryLayout(LwpObjectHeader const &objHdr, LwpSvStream* pStrm);
    virtual ~LwpSuperGlossaryLayout() override;
protected:
    void Read() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
