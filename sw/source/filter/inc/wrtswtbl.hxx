/*************************************************************************
 *
 *  $RCSfile: wrtswtbl.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:54 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef  _WRTSWTBL_HXX
#define  _WRTSWTBL_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif

#ifndef _ORNTENUM_HXX
#include <orntenum.hxx>
#endif
#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

class Color;
class SwTableBox;
class SwTableBoxes;
class SwTableLine;
class SwTableLines;
class SwTable;
class SwFrmFmt;
class SwHTMLTableLayout;
class SvxBrushItem;
class SvxBoxItem;
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

    USHORT nWidthOpt;           // Breite aus Option;

    USHORT nRow;                // Start-Zeile
    USHORT nCol;                // Start-Spalte

    USHORT nRowSpan;            // ueberspannte Zeilen
    USHORT nColSpan;            // ueberspannte Spalten


    BOOL bPrcWidthOpt;

public:

    SwWriteTableCell( const SwTableBox *pB, USHORT nR, USHORT nC,
                        USHORT nRSpan, USHORT nCSpan, long nHght,
                        const SvxBrushItem *pBGround ) :
        pBox( pB ),
        nRow( nR ), nCol( nC ),
        nRowSpan( nRSpan ), nColSpan( nCSpan ),
        nHeight( nHght ), pBackground( pBGround ),
        nWidthOpt( 0 ), bPrcWidthOpt( FALSE )
    {}

    const SwTableBox *GetBox() const { return pBox; }

    USHORT GetRow() const { return nRow; }
    USHORT GetCol() const { return nCol; }

    USHORT GetRowSpan() const { return nRowSpan; }
    USHORT GetColSpan() const { return nColSpan; }

    long GetHeight() const { return nHeight; }
    SwVertOrient GetVertOri() const;

    const SvxBrushItem *GetBackground() const { return pBackground; }

    void SetWidthOpt( USHORT nWidth, BOOL bPrc )
    {
        nWidthOpt = nWidth; bPrcWidthOpt = bPrc;
    }

    USHORT GetWidthOpt() const { return nWidthOpt; }
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

public:

    USHORT nTopBorder;              // Dicke der oberen/unteren Umrandugen
    USHORT nBottomBorder;

    BOOL bTopBorder : 1;            // Welche Umrandungen sind da?
    BOOL bBottomBorder : 1;

    SwWriteTableRow( long nPos );

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
    return (nPos >= rRow.nPos ? nPos - rRow.nPos
                               : rRow.nPos - nPos ) <= ROWFUZZY;
}

inline int SwWriteTableRow::operator<( const SwWriteTableRow& rRow ) const
{
    // Da wir hier nur die Wahrheits-Grade 0 und 1 kennen, lassen wir lieber
    // auch nicht zu, dass x==y und x<y gleichzeitig gilt ;-)
    return nPos < rRow.nPos - ROWFUZZY;
}

typedef SwWriteTableRow *SwWriteTableRowPtr;
SV_DECL_PTRARR_SORT_DEL( SwWriteTableRows, SwWriteTableRowPtr, 5, 5 )


//-----------------------------------------------------------------------

class SwWriteTableCol
{
    USHORT nPos;                        // End Position der Spalte

    USHORT nWidthOpt;

    BOOL bRelWidthOpt : 1;
    BOOL bOutWidth : 1;                 // Spaltenbreite ausgeben?

public:
    BOOL bLeftBorder : 1;               // Welche Umrandungen sind da?
    BOOL bRightBorder : 1;

    SwWriteTableCol( USHORT nPosition );

    USHORT GetPos() const                       { return nPos; }

    void SetLeftBorder( BOOL bBorder )          { bLeftBorder = bBorder; }
    BOOL HasLeftBorder() const                  { return bLeftBorder; }

    void SetRightBorder( BOOL bBorder )         { bRightBorder = bBorder; }
    BOOL HasRightBorder() const                 { return bRightBorder; }

    void SetOutWidth( BOOL bSet )               { bOutWidth = bSet; }
    BOOL GetOutWidth() const                    { return bOutWidth; }

    inline int operator==( const SwWriteTableCol& rCol ) const;
    inline int operator<( const SwWriteTableCol& rCol ) const;

    void SetWidthOpt( USHORT nWidth, BOOL bRel )
    {
        nWidthOpt = nWidth; bRelWidthOpt = bRel;
    }
    USHORT GetWidthOpt() const                  { return nWidthOpt; }
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

class SwWriteTable
{
protected:
    SwWriteTableCols aCols; // alle Spalten
    SwWriteTableRows aRows; // alle Zellen

    UINT32 nBorderColor;        // Umrandungsfarbe

    USHORT nCellSpacing;        // Dicke der inneren Umrandung
    USHORT nCellPadding;        // Absatnd Umrandung-Inhalt

    USHORT nBorder;             // Dicke der ausseren Umrandung
    USHORT nInnerBorder;        // Dicke der inneren Umrandung
    USHORT nBaseWidth;          // Bezugsgroesse fur Breiten SwFmtFrmSize

    USHORT nHeadEndRow;         // letzte Zeile des Tabellen-Kopfes

    USHORT nLeftSub;
    USHORT nRightSub;

    long nTabWidth;             // Absolute/Relative Breite der Tabelle

    BOOL bRelWidths : 1;        // Breiten relativ ausgeben?
    BOOL bUseLayoutHeights : 1; // Layout zur Hoehenbestimmung nehmen?
#ifndef PRODUCT
    BOOL bGetLineHeightCalled : 1;
#endif

    BOOL bColsOption : 1;
    BOOL bColTags : 1;
    BOOL bLayoutExport : 1;
    BOOL bCollectBorderWidth : 1;

    virtual BOOL ShouldExpandSub( const SwTableBox *pBox,
                                BOOL bExpandedBefore, USHORT nDepth ) const;

    void CollectTableRowsCols( long nStartRPos, USHORT nStartCPos,
                               long nParentLineHeight,
                               USHORT nParentLineWidth,
                               const SwTableLines& rLines,
                               USHORT nDepth );

    void FillTableRowsCols( long nStartRPos, USHORT nStartRow,
                            USHORT nStartCPos, USHORT nStartCol,
                            long nParentLineHeight,
                            USHORT nParentLineWidth,
                            const SwTableLines& rLines,
                            const SvxBrushItem* pLineBrush,
                            USHORT nDepth );

    void MergeBorders( const SvxBorderLine* pBorderLine, BOOL bTable );

    USHORT MergeBoxBorders( const SwTableBox *pBox, USHORT nRow, USHORT nCol,
                            USHORT nRowSpan, USHORT nColSpan,
                            USHORT &rTopBorder, USHORT &rBottomBorder );

    USHORT GetBaseWidth() const { return nBaseWidth; }

    BOOL HasRelWidths() const { return bRelWidths; }

public:
    static long GetBoxWidth( const SwTableBox *pBox );
protected:

    long GetLineHeight( const SwTableLine *pLine );
    long GetLineHeight( const SwTableBox *pBox ) const;
    const SvxBrushItem *GetLineBrush( const SwTableBox *pBox,
                                      SwWriteTableRow *pRow );

    USHORT GetLeftSpace( USHORT nCol ) const;
    USHORT GetRightSpace( USHORT nCol, USHORT nColSpan ) const;

    USHORT GetRawWidth( USHORT nCol, USHORT nColSpan ) const;
    USHORT GetAbsWidth( USHORT nCol, USHORT nColSpan ) const;
    USHORT GetRelWidth( USHORT nCol, USHORT nColSpan ) const;
    USHORT GetPrcWidth( USHORT nCol, USHORT nColSpan ) const;

    long GetAbsHeight( long nRawWidth, USHORT nRow, USHORT nRowSpan ) const;

public:
    SwWriteTable( const SwTableLines& rLines, long nWidth, USHORT nBWidth,
                    BOOL bRel, USHORT nMaxDepth = USHRT_MAX,
                    USHORT nLeftSub=0, USHORT nRightSub=0 );
    SwWriteTable( const SwHTMLTableLayout *pLayoutInfo );

    const SwWriteTableCols& GetCols() const { return aCols; }
    const SwWriteTableRows& GetRows() const { return aRows; }
};




#endif

