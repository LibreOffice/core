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
#ifndef INCLUDED_SW_SOURCE_FILTER_INC_WRTSWTBL_HXX
#define INCLUDED_SW_SOURCE_FILTER_INC_WRTSWTBL_HXX

#include <tools/solar.h>
#include <vcl/color.hxx>
#include <o3tl/sorted_vector.hxx>

#include <swdllapi.h>

#include <memory>
#include <vector>

class SwTableBox;
class SwTableLine;
class SwTableLines;
class SwHTMLTableLayout;
class SvxBrushItem;

namespace editeng { class SvxBorderLine; }

//       Code from the HTML filter for writing of tables

#define COLFUZZY 20
#define ROWFUZZY 20
#define COL_DFLT_WIDTH ((2*COLFUZZY)+1)
#define ROW_DFLT_HEIGHT (2*ROWFUZZY)+1

class SW_DLLPUBLIC SwWriteTableCell
{
    const SwTableBox *pBox;             // SwTableBox of the cell
    const SvxBrushItem *pBackground;    // inherited background of a row

    long nHeight;                   // fix/minimum height of a row

    sal_uInt32 nWidthOpt;           // width from option;

    sal_uInt16 nRow;                // start row
    sal_uInt16 nCol;                // start column

    sal_uInt16 nRowSpan;            // spanned rows
    sal_uInt16 nColSpan;            // spanned columns

    bool bPrcWidthOpt;

public:

    SwWriteTableCell(const SwTableBox *pB, sal_uInt16 nR, sal_uInt16 nC, sal_uInt16 nRSpan,
        sal_uInt16 nCSpan, long nHght, const SvxBrushItem *pBGround)
    : pBox( pB ), pBackground( pBGround ), nHeight( nHght ), nWidthOpt( 0 ),
    nRow( nR ), nCol( nC ), nRowSpan( nRSpan ), nColSpan( nCSpan ),
    bPrcWidthOpt( false )
    {}

    const SwTableBox *GetBox() const { return pBox; }

    sal_uInt16 GetRow() const { return nRow; }
    sal_uInt16 GetCol() const { return nCol; }

    sal_uInt16 GetRowSpan() const { return nRowSpan; }
    sal_uInt16 GetColSpan() const { return nColSpan; }

    long GetHeight() const { return nHeight; }
    sal_Int16 GetVertOri() const;

    const SvxBrushItem *GetBackground() const { return pBackground; }

    void SetWidthOpt( sal_uInt16 nWidth, bool bPrc )
    {
        nWidthOpt = nWidth; bPrcWidthOpt = bPrc;
    }

    sal_uInt32 GetWidthOpt() const { return nWidthOpt; }
    bool HasPrcWidthOpt() const { return bPrcWidthOpt; }
};

typedef std::vector<std::unique_ptr<SwWriteTableCell>> SwWriteTableCells;

class SW_DLLPUBLIC SwWriteTableRow final
{
    SwWriteTableCells m_Cells;       ///< all cells of the rows
    const SvxBrushItem *pBackground; // background

    long nPos;                       // end position (twips) of the row
    bool mbUseLayoutHeights;

    SwWriteTableRow & operator= (const SwWriteTableRow &) = delete;

    // GCC >= 3.4 needs accessible T (const T&) to pass T as const T& argument.
    SwWriteTableRow( const SwWriteTableRow & );

public:

    sal_uInt16 nTopBorder;          // thickness of upper/lower border
    sal_uInt16 nBottomBorder;

    bool bTopBorder : 1;            // which borders are there?
    bool bBottomBorder : 1;

    SwWriteTableRow( long nPos, bool bUseLayoutHeights );

    SwWriteTableCell *AddCell( const SwTableBox *pBox,
                                 sal_uInt16 nRow, sal_uInt16 nCol,
                                 sal_uInt16 nRowSpan, sal_uInt16 nColSpan,
                                 long nHeight,
                                 const SvxBrushItem *pBackground );

    void SetBackground( const SvxBrushItem *pBGround )
    {
        pBackground = pBGround;
    }
    const SvxBrushItem *GetBackground() const { return pBackground; }

    bool HasTopBorder() const                   { return bTopBorder; }
    bool HasBottomBorder() const                { return bBottomBorder; }

    const SwWriteTableCells& GetCells() const   { return m_Cells; }

    inline bool operator==( const SwWriteTableRow& rRow ) const;
    inline bool operator<( const SwWriteTableRow& rRow2 ) const;
};

inline bool SwWriteTableRow::operator==( const SwWriteTableRow& rRow ) const
{
    // allow for some fuzzyness
    return (nPos >= rRow.nPos ?  nPos - rRow.nPos : rRow.nPos - nPos ) <=
        (mbUseLayoutHeights ? 0 : ROWFUZZY);
}

inline bool SwWriteTableRow::operator<( const SwWriteTableRow& rRow ) const
{
    // Since we only know the degrees of truth of 0 and 1 here, we also prefer to
    // not let x==y and x<y at the same time ;-)
    return nPos < rRow.nPos - (mbUseLayoutHeights ? 0 : ROWFUZZY);
}

class SwWriteTableRows : public o3tl::sorted_vector<SwWriteTableRow*, o3tl::less_ptr_to<SwWriteTableRow> > {
public:
    ~SwWriteTableRows() { DeleteAndDestroyAll(); }
};

class SW_DLLPUBLIC SwWriteTableCol
{
    sal_uInt32 nPos;                    // end position of the column

    sal_uInt32 nWidthOpt;

    bool bRelWidthOpt : 1;

public:
    bool bLeftBorder : 1;               // which borders are there?
    bool bRightBorder : 1;

    SwWriteTableCol( sal_uInt32 nPosition );

    sal_uInt32 GetPos() const                       { return nPos; }

    bool HasLeftBorder() const                  { return bLeftBorder; }

    bool HasRightBorder() const                 { return bRightBorder; }

    inline bool operator==( const SwWriteTableCol& rCol ) const;
    inline bool operator<( const SwWriteTableCol& rCol ) const;

    void SetWidthOpt( sal_uInt32 nWidth, bool bRel )
    {
        nWidthOpt = nWidth; bRelWidthOpt = bRel;
    }
    sal_uInt32 GetWidthOpt() const                 { return nWidthOpt; }
    bool HasRelWidthOpt() const                 { return bRelWidthOpt; }
};

inline bool SwWriteTableCol::operator==( const SwWriteTableCol& rCol ) const
{
    // allow for some fuzzyness
    return (nPos >= rCol.nPos ? nPos - rCol.nPos
                                     : rCol.nPos - nPos ) <= COLFUZZY;
}

inline bool SwWriteTableCol::operator<( const SwWriteTableCol& rCol ) const
{
    // Since we only know the degrees of truth of 0 and 1 here, we also prefer to
    // not let x==y and x<y at the same time ;-)
    return nPos + COLFUZZY < rCol.nPos;
}

struct SwWriteTableColLess {
    bool operator()(SwWriteTableCol const * lhs, SwWriteTableCol const * rhs) {
        return lhs->GetPos() < rhs->GetPos();
    }
};

class SwWriteTableCols : public o3tl::sorted_vector<SwWriteTableCol*, SwWriteTableColLess> {
public:
    ~SwWriteTableCols() { DeleteAndDestroyAll(); }
};

class SwTable;

class SW_DLLPUBLIC SwWriteTable
{
private:
    const SwTable* m_pTable;
protected:
    SwWriteTableCols m_aCols; // all columns
    SwWriteTableRows m_aRows; // all rows

    sal_uInt32 m_nBorderColor;        // border color

    sal_uInt16 m_nCellSpacing;        // thickness of the inner border
    sal_uInt16 m_nCellPadding;        // distance of border to content

    sal_uInt16 m_nBorder;             // thickness of the outer border
    sal_uInt16 m_nInnerBorder;        // thickness of the inner border
    sal_uInt32 m_nBaseWidth;          // reference value for SwFormatFrameSize width

    sal_uInt16 m_nHeadEndRow;         // last row of the table heading

    sal_uInt16 m_nLeftSub;
    sal_uInt16 m_nRightSub;

    sal_uInt32 m_nTabWidth;           // absolute/relative width of the table

    bool m_bRelWidths : 1;        // generate relative widths?
    bool m_bUseLayoutHeights : 1; // use layout to determine the height?
#ifdef DBG_UTIL
    bool m_bGetLineHeightCalled : 1;
#endif

    bool m_bColTags : 1;
    bool m_bLayoutExport : 1;
    bool m_bCollectBorderWidth : 1;

    virtual bool ShouldExpandSub( const SwTableBox *pBox,
                                bool bExpandedBefore, sal_uInt16 nDepth ) const;

    void CollectTableRowsCols( long nStartRPos, sal_uInt32 nStartCPos,
                               long nParentLineHeight,
                               sal_uInt32 nParentLineWidth,
                               const SwTableLines& rLines,
                               sal_uInt16 nDepth );

    void FillTableRowsCols( long nStartRPos, sal_uInt16 nStartRow,
                            sal_uInt32 nStartCPos, sal_uInt16 nStartCol,
                            long nParentLineHeight,
                            sal_uInt32 nParentLineWidth,
                            const SwTableLines& rLines,
                            const SvxBrushItem* pLineBrush,
                            sal_uInt16 nDepth,
                            sal_uInt16 nNumOfHeaderRows );

    void MergeBorders( const editeng::SvxBorderLine* pBorderLine, bool bTable );

    sal_uInt16 MergeBoxBorders(const SwTableBox *pBox, size_t nRow, size_t nCol,
                            sal_uInt16 nRowSpan, sal_uInt16 nColSpan,
                            sal_uInt16 &rTopBorder, sal_uInt16 &rBottomBorder );

    sal_uInt32 GetBaseWidth() const { return m_nBaseWidth; }

    bool HasRelWidths() const { return m_bRelWidths; }

public:
    static sal_uInt32 GetBoxWidth( const SwTableBox *pBox );

    sal_uInt32 GetRawWidth( sal_uInt16 nCol, sal_uInt16 nColSpan ) const;
    sal_uInt16 GetAbsWidth( sal_uInt16 nCol, sal_uInt16 nColSpan ) const;
    sal_uInt16 GetRelWidth( sal_uInt16 nCol, sal_uInt16 nColSpan ) const;
    sal_uInt16 GetPrcWidth( sal_uInt16 nCol, sal_uInt16 nColSpan ) const;

    long GetAbsHeight(long nRawWidth, size_t nRow, sal_uInt16 nRowSpan) const;

    double GetAbsWidthRatio() const { return m_nTabWidth == m_nBaseWidth ? 1.0 : double(m_nTabWidth) / m_nBaseWidth; }
protected:
    long GetLineHeight( const SwTableLine *pLine );
    static long GetLineHeight( const SwTableBox *pBox );
    static const SvxBrushItem *GetLineBrush( const SwTableBox *pBox,
                                      SwWriteTableRow *pRow );

    sal_uInt16 GetLeftSpace( sal_uInt16 nCol ) const;
    sal_uInt16 GetRightSpace(size_t nCol, sal_uInt16 nColSpan) const;

public:
    SwWriteTable(const SwTable* pTable, const SwTableLines& rLines, long nWidth, sal_uInt32 nBWidth,
                 bool bRel, sal_uInt16 nMaxDepth = USHRT_MAX,
                 sal_uInt16 nLeftSub=0, sal_uInt16 nRightSub=0, sal_uInt32 nNumOfRowsToRepeat=0);
    SwWriteTable(const SwTable* pTable, const SwHTMLTableLayout *pLayoutInfo);
    virtual ~SwWriteTable();

    const SwWriteTableRows& GetRows() const { return m_aRows; }

    const SwTable* GetTable() const { return m_pTable; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
