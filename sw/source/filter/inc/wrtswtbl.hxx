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
#include <tools/color.hxx>
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

//       Code aus dem HTML-Filter fuers schreiben von Tabellen

#define COLFUZZY 20
#define ROWFUZZY 20
#define COL_DFLT_WIDTH ((2*COLFUZZY)+1)
#define ROW_DFLT_HEIGHT (2*ROWFUZZY)+1

class SW_DLLPUBLIC SwWriteTableCell
{
    const SwTableBox *pBox;     // SwTableBox der Zelle
    const SvxBrushItem *pBackground;    // geerbter Hintergrund einer Zeile

    long nHeight;               // fixe/Mindest-Hoehe der Zeile

    sal_uInt32 nWidthOpt;          // Breite aus Option;

    sal_uInt16 nRow;                // Start-Zeile
    sal_uInt16 nCol;                // Start-Spalte

    sal_uInt16 nRowSpan;            // ueberspannte Zeilen
    sal_uInt16 nColSpan;            // ueberspannte Spalten

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

class SW_DLLPUBLIC SwWriteTableRow
{
    SwWriteTableCells m_Cells; ///< All cells of the Rows
    const SvxBrushItem *pBackground;// Hintergrund

    long nPos;                  // End-Position (twips) der Zeile
    bool mbUseLayoutHeights;

    // Forbidden and not implemented.
    SwWriteTableRow();

    SwWriteTableRow & operator= (const SwWriteTableRow &) = delete;

protected:
    // GCC >= 3.4 needs accessible T (const T&) to pass T as const T& argument.
    SwWriteTableRow( const SwWriteTableRow & );

public:

    sal_uInt16 nTopBorder;              // Dicke der oberen/unteren Umrandugen
    sal_uInt16 nBottomBorder;

    bool bTopBorder : 1;            // Welche Umrandungen sind da?
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
    // etwas Unschaerfe zulassen
    return (nPos >= rRow.nPos ?  nPos - rRow.nPos : rRow.nPos - nPos ) <=
        (mbUseLayoutHeights ? 0 : ROWFUZZY);
}

inline bool SwWriteTableRow::operator<( const SwWriteTableRow& rRow ) const
{
    // Da wir hier nur die Wahrheits-Grade 0 und 1 kennen, lassen wir lieber
    // auch nicht zu, dass x==y und x<y gleichzeitig gilt ;-)
    return nPos < rRow.nPos - (mbUseLayoutHeights ? 0 : ROWFUZZY);
}

class SwWriteTableRows : public o3tl::sorted_vector<SwWriteTableRow*, o3tl::less_ptr_to<SwWriteTableRow> > {
public:
    ~SwWriteTableRows() { DeleteAndDestroyAll(); }
};

class SW_DLLPUBLIC SwWriteTableCol
{
    sal_uInt32 nPos;                        // End Position der Spalte

    sal_uInt32 nWidthOpt;

    bool bRelWidthOpt : 1;
    bool bOutWidth : 1;                 // Spaltenbreite ausgeben?

public:
    bool bLeftBorder : 1;               // Welche Umrandungen sind da?
    bool bRightBorder : 1;

    SwWriteTableCol( sal_uInt32 nPosition );

    sal_uInt32 GetPos() const                       { return nPos; }

    bool HasLeftBorder() const                  { return bLeftBorder; }

    bool HasRightBorder() const                 { return bRightBorder; }

    void SetOutWidth( bool bSet )               { bOutWidth = bSet; }

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
    // etwas Unschaerfe zulassen
    return (nPos >= rCol.nPos ? nPos - rCol.nPos
                                     : rCol.nPos - nPos ) <= COLFUZZY;
}

inline bool SwWriteTableCol::operator<( const SwWriteTableCol& rCol ) const
{
    // Da wir hier nur die Wahrheits-Grade 0 und 1 kennen, lassen wir lieber
    // auch nicht zu, dass x==y und x<y gleichzeitig gilt ;-)
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
    SwWriteTableCols m_aCols; // alle Spalten
    SwWriteTableRows m_aRows; // alle Zellen

    sal_uInt32 m_nBorderColor;        // Umrandungsfarbe

    sal_uInt16 m_nCellSpacing;        // Dicke der inneren Umrandung
    sal_uInt16 m_nCellPadding;        // Absatnd Umrandung-Inhalt

    sal_uInt16 m_nBorder;             // Dicke der ausseren Umrandung
    sal_uInt16 m_nInnerBorder;        // Dicke der inneren Umrandung
    sal_uInt32 m_nBaseWidth;            // Bezugsgroesse fur Breiten SwFormatFrameSize

    sal_uInt16 m_nHeadEndRow;         // letzte Zeile des Tabellen-Kopfes

    sal_uInt16 m_nLeftSub;
    sal_uInt16 m_nRightSub;

    sal_uInt32 m_nTabWidth;              // Absolute/Relative Breite der Tabelle

    bool m_bRelWidths : 1;        // Breiten relativ ausgeben?
    bool m_bUseLayoutHeights : 1; // Layout zur Hoehenbestimmung nehmen?
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
