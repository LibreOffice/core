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
class SvxBorderLine;

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

    USHORT nRow;                // Start-Zeile
    USHORT nCol;                // Start-Spalte

    USHORT nRowSpan;            // ueberspannte Zeilen
    USHORT nColSpan;            // ueberspannte Spalten


    BOOL bPrcWidthOpt;

public:

    SwWriteTableCell(const SwTableBox *pB, USHORT nR, USHORT nC, USHORT nRSpan,
        USHORT nCSpan, long nHght, const SvxBrushItem *pBGround)
    : pBox( pB ), pBackground( pBGround ), nHeight( nHght ), nWidthOpt( 0 ),
    nRow( nR ), nCol( nC ), nRowSpan( nRSpan ), nColSpan( nCSpan ),
    bPrcWidthOpt( FALSE )
    {}

    const SwTableBox *GetBox() const { return pBox; }

    USHORT GetRow() const { return nRow; }
    USHORT GetCol() const { return nCol; }

    USHORT GetRowSpan() const { return nRowSpan; }
    USHORT GetColSpan() const { return nColSpan; }

    long GetHeight() const { return nHeight; }
    sal_Int16 GetVertOri() const;

    const SvxBrushItem *GetBackground() const { return pBackground; }

    void SetWidthOpt( USHORT nWidth, BOOL bPrc )
    {
        nWidthOpt = nWidth; bPrcWidthOpt = bPrc;
    }

    sal_uInt32 GetWidthOpt() const { return nWidthOpt; }
    BOOL HasPrcWidthOpt() const { return bPrcWidthOpt; }
};

typedef SwWriteTableCell *SwWriteTableCellPtr;
SV_DECL_PTRARR_DEL( SwWriteTableCells, SwWriteTableCellPtr, 5, 5 )


//-----------------------------------------------------------------------

class SwWriteTableRow
{
    SwWriteTableCells aCells;       // Alle Zellen der Rows
    const SvxBrushItem *pBackground;// Hintergrund

    long nPos;                  // End-Position (twips) der Zeile
    BOOL mbUseLayoutHeights;

    // Forbidden and not implemented.
    SwWriteTableRow();

    SwWriteTableRow & operator= (const SwWriteTableRow &);

protected:
    // GCC >= 3.4 needs accessible T (const T&) to pass T as const T& argument.
    SwWriteTableRow( const SwWriteTableRow & );

public:

    USHORT nTopBorder;              // Dicke der oberen/unteren Umrandugen
    USHORT nBottomBorder;

    BOOL bTopBorder : 1;            // Welche Umrandungen sind da?
    BOOL bBottomBorder : 1;

    SwWriteTableRow( long nPos, BOOL bUseLayoutHeights );

    SwWriteTableCell *AddCell( const SwTableBox *pBox,
                                 USHORT nRow, USHORT nCol,
                                 USHORT nRowSpan, USHORT nColSpan,
                                 long nHeight,
                                 const SvxBrushItem *pBackground );

    void SetBackground( const SvxBrushItem *pBGround )
    {
        pBackground = pBGround;
    }
    const SvxBrushItem *GetBackground() const { return pBackground; }

    BOOL HasTopBorder() const                   { return bTopBorder; }
    BOOL HasBottomBorder() const                { return bBottomBorder; }

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

    BOOL bRelWidthOpt : 1;
    BOOL bOutWidth : 1;                 // Spaltenbreite ausgeben?

public:
    BOOL bLeftBorder : 1;               // Welche Umrandungen sind da?
    BOOL bRightBorder : 1;

    SwWriteTableCol( sal_uInt32 nPosition );

    sal_uInt32 GetPos() const                       { return nPos; }

    void SetLeftBorder( BOOL bBorder )          { bLeftBorder = bBorder; }
    BOOL HasLeftBorder() const                  { return bLeftBorder; }

    void SetRightBorder( BOOL bBorder )         { bRightBorder = bBorder; }
    BOOL HasRightBorder() const                 { return bRightBorder; }

    void SetOutWidth( BOOL bSet )               { bOutWidth = bSet; }
    BOOL GetOutWidth() const                    { return bOutWidth; }

    inline int operator==( const SwWriteTableCol& rCol ) const;
    inline int operator<( const SwWriteTableCol& rCol ) const;

    void SetWidthOpt( sal_uInt32 nWidth, BOOL bRel )
    {
        nWidthOpt = nWidth; bRelWidthOpt = bRel;
    }
    sal_uInt32 GetWidthOpt() const                 { return nWidthOpt; }
    BOOL HasRelWidthOpt() const                 { return bRelWidthOpt; }
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

    UINT32 nBorderColor;        // Umrandungsfarbe

    USHORT nCellSpacing;        // Dicke der inneren Umrandung
    USHORT nCellPadding;        // Absatnd Umrandung-Inhalt

    USHORT nBorder;             // Dicke der ausseren Umrandung
    USHORT nInnerBorder;        // Dicke der inneren Umrandung
    sal_uInt32 nBaseWidth;            // Bezugsgroesse fur Breiten SwFmtFrmSize

    USHORT nHeadEndRow;         // letzte Zeile des Tabellen-Kopfes

    USHORT nLeftSub;
    USHORT nRightSub;

    sal_uInt32 nTabWidth;              // Absolute/Relative Breite der Tabelle

    BOOL bRelWidths : 1;        // Breiten relativ ausgeben?
    BOOL bUseLayoutHeights : 1; // Layout zur Hoehenbestimmung nehmen?
#ifdef DBG_UTIL
    BOOL bGetLineHeightCalled : 1;
#endif

    BOOL bColsOption : 1;
    BOOL bColTags : 1;
    BOOL bLayoutExport : 1;
    BOOL bCollectBorderWidth : 1;

    virtual BOOL ShouldExpandSub( const SwTableBox *pBox,
                                BOOL bExpandedBefore, USHORT nDepth ) const;

    void CollectTableRowsCols( long nStartRPos, sal_uInt32 nStartCPos,
                               long nParentLineHeight,
                               sal_uInt32 nParentLineWidth,
                               const SwTableLines& rLines,
                               USHORT nDepth );

    void FillTableRowsCols( long nStartRPos, USHORT nStartRow,
                            sal_uInt32 nStartCPos, USHORT nStartCol,
                            long nParentLineHeight,
                            sal_uInt32 nParentLineWidth,
                            const SwTableLines& rLines,
                            const SvxBrushItem* pLineBrush,
                            USHORT nDepth,
                            sal_uInt16 nNumOfHeaderRows );

    void MergeBorders( const SvxBorderLine* pBorderLine, BOOL bTable );

    USHORT MergeBoxBorders( const SwTableBox *pBox, USHORT nRow, USHORT nCol,
                            USHORT nRowSpan, USHORT nColSpan,
                            USHORT &rTopBorder, USHORT &rBottomBorder );

    sal_uInt32 GetBaseWidth() const { return nBaseWidth; }

    BOOL HasRelWidths() const { return bRelWidths; }

public:
    static sal_uInt32 GetBoxWidth( const SwTableBox *pBox );

    sal_uInt32 GetRawWidth( USHORT nCol, USHORT nColSpan ) const;
    USHORT GetAbsWidth( USHORT nCol, USHORT nColSpan ) const;
    USHORT GetRelWidth( USHORT nCol, USHORT nColSpan ) const;
    USHORT GetPrcWidth( USHORT nCol, USHORT nColSpan ) const;

    long GetAbsHeight( long nRawWidth, USHORT nRow, USHORT nRowSpan ) const;
protected:

    long GetLineHeight( const SwTableLine *pLine );
    long GetLineHeight( const SwTableBox *pBox ) const;
    const SvxBrushItem *GetLineBrush( const SwTableBox *pBox,
                                      SwWriteTableRow *pRow );

    USHORT GetLeftSpace( USHORT nCol ) const;
    USHORT GetRightSpace( USHORT nCol, USHORT nColSpan ) const;


public:
    SwWriteTable( const SwTableLines& rLines, long nWidth, sal_uInt32 nBWidth,
                    BOOL bRel, USHORT nMaxDepth = USHRT_MAX,
                    USHORT nLeftSub=0, USHORT nRightSub=0, sal_uInt32 nNumOfRowsToRepeat=0 );
    SwWriteTable( const SwHTMLTableLayout *pLayoutInfo );
    virtual ~SwWriteTable();

    const SwWriteTableCols& GetCols() const { return aCols; }
    const SwWriteTableRows& GetRows() const { return aRows; }
};




#endif

