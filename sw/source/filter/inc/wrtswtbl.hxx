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
#ifndef  _WRTSWTBL_HXX
#define  _WRTSWTBL_HXX

#include <tools/solar.h>
#include <tools/color.hxx>
#include <svl/svarray.hxx>

#include <swdllapi.h>

class Color;
class SwTableBox;
class SwTableLine;
class SwTableLines;
class SwTable;
class SwHTMLTableLayout;
class SvxBrushItem;

namespace editeng { class SvxBorderLine; }

//---------------------------------------------------------------------------
//       Code aus dem HTML-Filter fuers schreiben von Tabellen
//---------------------------------------------------------------------------

#define COLFUZZY 20
#define ROWFUZZY 20
#define COL_DFLT_WIDTH ((2*COLFUZZY)+1)
#define ROW_DFLT_HEIGHT (2*ROWFUZZY)+1


//-----------------------------------------------------------------------

class SwWriteTableCell
{
    const SwTableBox *pBox;     // SwTableBox der Zelle
    const SvxBrushItem *pBackground;    // geerbter Hintergrund einer Zeile

    long nHeight;               // fixe/Mindest-Hoehe der Zeile

    sal_uInt32 nWidthOpt;          // Breite aus Option;

    sal_uInt16 nRow;                // Start-Zeile
    sal_uInt16 nCol;                // Start-Spalte

    sal_uInt16 nRowSpan;            // ueberspannte Zeilen
    sal_uInt16 nColSpan;            // ueberspannte Spalten


    sal_Bool bPrcWidthOpt;

public:

    SwWriteTableCell(const SwTableBox *pB, sal_uInt16 nR, sal_uInt16 nC, sal_uInt16 nRSpan,
        sal_uInt16 nCSpan, long nHght, const SvxBrushItem *pBGround)
    : pBox( pB ), pBackground( pBGround ), nHeight( nHght ), nWidthOpt( 0 ),
    nRow( nR ), nCol( nC ), nRowSpan( nRSpan ), nColSpan( nCSpan ),
    bPrcWidthOpt( sal_False )
    {}

    const SwTableBox *GetBox() const { return pBox; }

    sal_uInt16 GetRow() const { return nRow; }
    sal_uInt16 GetCol() const { return nCol; }

    sal_uInt16 GetRowSpan() const { return nRowSpan; }
    sal_uInt16 GetColSpan() const { return nColSpan; }

    long GetHeight() const { return nHeight; }
    sal_Int16 GetVertOri() const;

    const SvxBrushItem *GetBackground() const { return pBackground; }

    void SetWidthOpt( sal_uInt16 nWidth, sal_Bool bPrc )
    {
        nWidthOpt = nWidth; bPrcWidthOpt = bPrc;
    }

    sal_uInt32 GetWidthOpt() const { return nWidthOpt; }
    sal_Bool HasPrcWidthOpt() const { return bPrcWidthOpt; }
};

typedef SwWriteTableCell *SwWriteTableCellPtr;
SV_DECL_PTRARR_DEL( SwWriteTableCells, SwWriteTableCellPtr, 5, 5 )


//-----------------------------------------------------------------------

class SwWriteTableRow
{
    SwWriteTableCells aCells;       // Alle Zellen der Rows
    const SvxBrushItem *pBackground;// Hintergrund

    long nPos;                  // End-Position (twips) der Zeile
    sal_Bool mbUseLayoutHeights;

    // Forbidden and not implemented.
    SwWriteTableRow();

    SwWriteTableRow & operator= (const SwWriteTableRow &);

protected:
    // GCC >= 3.4 needs accessible T (const T&) to pass T as const T& argument.
    SwWriteTableRow( const SwWriteTableRow & );

public:

    sal_uInt16 nTopBorder;              // Dicke der oberen/unteren Umrandugen
    sal_uInt16 nBottomBorder;

    sal_Bool bTopBorder : 1;            // Welche Umrandungen sind da?
    sal_Bool bBottomBorder : 1;

    SwWriteTableRow( long nPos, sal_Bool bUseLayoutHeights );

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

    sal_Bool HasTopBorder() const                   { return bTopBorder; }
    sal_Bool HasBottomBorder() const                { return bBottomBorder; }

    long GetPos() const                         { return nPos; }
    const SwWriteTableCells& GetCells() const   { return aCells; }

    inline int operator==( const SwWriteTableRow& rRow ) const;
    inline int operator<( const SwWriteTableRow& rRow2 ) const;
};

inline int SwWriteTableRow::operator==( const SwWriteTableRow& rRow ) const
{
    // etwas Unschaerfe zulassen
    return (nPos >= rRow.nPos ?  nPos - rRow.nPos : rRow.nPos - nPos ) <=
        (mbUseLayoutHeights ? 0 : ROWFUZZY);
}

inline int SwWriteTableRow::operator<( const SwWriteTableRow& rRow ) const
{
    // Da wir hier nur die Wahrheits-Grade 0 und 1 kennen, lassen wir lieber
    // auch nicht zu, dass x==y und x<y gleichzeitig gilt ;-)
    return nPos < rRow.nPos - (mbUseLayoutHeights ? 0 : ROWFUZZY);
}

typedef SwWriteTableRow *SwWriteTableRowPtr;
SV_DECL_PTRARR_SORT_DEL( SwWriteTableRows, SwWriteTableRowPtr, 5, 5 )


//-----------------------------------------------------------------------

class SwWriteTableCol
{
    sal_uInt32 nPos;                        // End Position der Spalte

    sal_uInt32 nWidthOpt;

    sal_Bool bRelWidthOpt : 1;
    sal_Bool bOutWidth : 1;                 // Spaltenbreite ausgeben?

public:
    sal_Bool bLeftBorder : 1;               // Welche Umrandungen sind da?
    sal_Bool bRightBorder : 1;

    SwWriteTableCol( sal_uInt32 nPosition );

    sal_uInt32 GetPos() const                       { return nPos; }

    void SetLeftBorder( sal_Bool bBorder )          { bLeftBorder = bBorder; }
    sal_Bool HasLeftBorder() const                  { return bLeftBorder; }

    void SetRightBorder( sal_Bool bBorder )         { bRightBorder = bBorder; }
    sal_Bool HasRightBorder() const                 { return bRightBorder; }

    void SetOutWidth( sal_Bool bSet )               { bOutWidth = bSet; }
    sal_Bool GetOutWidth() const                    { return bOutWidth; }

    inline int operator==( const SwWriteTableCol& rCol ) const;
    inline int operator<( const SwWriteTableCol& rCol ) const;

    void SetWidthOpt( sal_uInt32 nWidth, sal_Bool bRel )
    {
        nWidthOpt = nWidth; bRelWidthOpt = bRel;
    }
    sal_uInt32 GetWidthOpt() const                 { return nWidthOpt; }
    sal_Bool HasRelWidthOpt() const                 { return bRelWidthOpt; }
};

inline int SwWriteTableCol::operator==( const SwWriteTableCol& rCol ) const
{
    // etwas Unschaerfe zulassen
    return (nPos >= rCol.nPos ? nPos - rCol.nPos
                                     : rCol.nPos - nPos ) <= COLFUZZY;
}

inline int SwWriteTableCol::operator<( const SwWriteTableCol& rCol ) const
{
    // Da wir hier nur die Wahrheits-Grade 0 und 1 kennen, lassen wir lieber
    // auch nicht zu, dass x==y und x<y gleichzeitig gilt ;-)
    return nPos < rCol.nPos - COLFUZZY;
}


typedef SwWriteTableCol *SwWriteTableColPtr;
SV_DECL_PTRARR_SORT_DEL( SwWriteTableCols, SwWriteTableColPtr, 5, 5 )

//-----------------------------------------------------------------------

class SW_DLLPUBLIC SwWriteTable
{
protected:
    SwWriteTableCols aCols; // alle Spalten
    SwWriteTableRows aRows; // alle Zellen

    sal_uInt32 nBorderColor;        // Umrandungsfarbe

    sal_uInt16 nCellSpacing;        // Dicke der inneren Umrandung
    sal_uInt16 nCellPadding;        // Absatnd Umrandung-Inhalt

    sal_uInt16 nBorder;             // Dicke der ausseren Umrandung
    sal_uInt16 nInnerBorder;        // Dicke der inneren Umrandung
    sal_uInt32 nBaseWidth;            // Bezugsgroesse fur Breiten SwFmtFrmSize

    sal_uInt16 nHeadEndRow;         // letzte Zeile des Tabellen-Kopfes

    sal_uInt16 nLeftSub;
    sal_uInt16 nRightSub;

    sal_uInt32 nTabWidth;              // Absolute/Relative Breite der Tabelle

    sal_Bool bRelWidths : 1;        // Breiten relativ ausgeben?
    sal_Bool bUseLayoutHeights : 1; // Layout zur Hoehenbestimmung nehmen?
#if OSL_DEBUG_LEVEL > 1
    sal_Bool bGetLineHeightCalled : 1;
#endif

    sal_Bool bColsOption : 1;
    sal_Bool bColTags : 1;
    sal_Bool bLayoutExport : 1;
    sal_Bool bCollectBorderWidth : 1;

    virtual sal_Bool ShouldExpandSub( const SwTableBox *pBox,
                                sal_Bool bExpandedBefore, sal_uInt16 nDepth ) const;

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

    void MergeBorders( const editeng::SvxBorderLine* pBorderLine, sal_Bool bTable );

    sal_uInt16 MergeBoxBorders( const SwTableBox *pBox, sal_uInt16 nRow, sal_uInt16 nCol,
                            sal_uInt16 nRowSpan, sal_uInt16 nColSpan,
                            sal_uInt16 &rTopBorder, sal_uInt16 &rBottomBorder );

    sal_uInt32 GetBaseWidth() const { return nBaseWidth; }

    sal_Bool HasRelWidths() const { return bRelWidths; }

public:
    static sal_uInt32 GetBoxWidth( const SwTableBox *pBox );

    sal_uInt32 GetRawWidth( sal_uInt16 nCol, sal_uInt16 nColSpan ) const;
    sal_uInt16 GetAbsWidth( sal_uInt16 nCol, sal_uInt16 nColSpan ) const;
    sal_uInt16 GetRelWidth( sal_uInt16 nCol, sal_uInt16 nColSpan ) const;
    sal_uInt16 GetPrcWidth( sal_uInt16 nCol, sal_uInt16 nColSpan ) const;

    long GetAbsHeight( long nRawWidth, sal_uInt16 nRow, sal_uInt16 nRowSpan ) const;
protected:

    long GetLineHeight( const SwTableLine *pLine );
    long GetLineHeight( const SwTableBox *pBox ) const;
    const SvxBrushItem *GetLineBrush( const SwTableBox *pBox,
                                      SwWriteTableRow *pRow );

    sal_uInt16 GetLeftSpace( sal_uInt16 nCol ) const;
    sal_uInt16 GetRightSpace( sal_uInt16 nCol, sal_uInt16 nColSpan ) const;


public:
    SwWriteTable( const SwTableLines& rLines, long nWidth, sal_uInt32 nBWidth,
                    sal_Bool bRel, sal_uInt16 nMaxDepth = USHRT_MAX,
                    sal_uInt16 nLeftSub=0, sal_uInt16 nRightSub=0, sal_uInt32 nNumOfRowsToRepeat=0 );
    SwWriteTable( const SwHTMLTableLayout *pLayoutInfo );
    virtual ~SwWriteTable();

    const SwWriteTableCols& GetCols() const { return aCols; }
    const SwWriteTableRows& GetRows() const { return aRows; }
};




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
