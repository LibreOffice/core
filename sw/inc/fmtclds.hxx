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
#ifndef _FMTCLDS_HXX
#define _FMTCLDS_HXX

#include <tools/color.hxx>
#include <svl/poolitem.hxx>
#include "swdllapi.h"
#include <hintids.hxx>
#include <format.hxx>

// ColumnDescriptor
class SwColumn
{
    USHORT nWish;   // Desired width, borders included.
                    // It is inversely proportional to the ratio of
                    // desired width environment / current width column.
    USHORT nUpper;  // Top border.
    USHORT nLower;  // Bottom border.
    USHORT nLeft;   // Left border.
    USHORT nRight;  // Right border.

public:
    SwColumn();

    BOOL operator==( const SwColumn & );


    void SetWishWidth( USHORT nNew ) { nWish  = nNew; }
    void SetUpper( USHORT  nNew ) { nUpper = nNew; }
    void SetLower( USHORT  nNew ) { nLower = nNew; }
    void SetLeft ( USHORT  nNew ) { nLeft  = nNew; }
    void SetRight( USHORT  nNew ) { nRight = nNew; }

    USHORT GetWishWidth() const { return nWish;  }
    USHORT GetUpper() const { return nUpper; }
    USHORT GetLower() const { return nLower; }
    USHORT GetLeft () const { return nLeft; }
    USHORT GetRight() const { return nRight; }
};

typedef SwColumn* SwColumnPtr;
SV_DECL_PTRARR_DEL( SwColumns, SwColumnPtr, 0, 2 )

enum SwColLineAdj
{
    COLADJ_NONE,
    COLADJ_TOP,
    COLADJ_CENTER,
    COLADJ_BOTTOM
};

class SW_DLLPUBLIC SwFmtCol : public SfxPoolItem
{
    ULONG   nLineWidth;     // Width of the separator line.
    Color   aLineColor;     // Color of the separator line.

    BYTE     nLineHeight;   // Percentile height of lines.
                            // (Based on height of columns including UL).

    SwColLineAdj eAdj;      // Line will be adjusted top, centered or bottom.

    SwColumns   aColumns;   // Information concerning the columns.
    USHORT      nWidth;     // Total desired width of all columns.

    BOOL bOrtho;            // Only if this flag is set, the setting of GutterWidth will
                            // be accompanied by a "visual rearrangement".
                            // The flag must be reset if widths of columns or borders are changed.
                            // When it is set (again) the visual arrangement is recalculated.
                            // The flag is initially set.

    SW_DLLPRIVATE void Calc( USHORT nGutterWidth, USHORT nAct );

public:
    SwFmtCol();
    SwFmtCol( const SwFmtCol& );
    ~SwFmtCol();

    SwFmtCol& operator=( const SwFmtCol& );

    // "pure virtual methods" of SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper* pIntl = 0 ) const;

    virtual bool QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    const SwColumns &GetColumns() const { return aColumns; }
          SwColumns &GetColumns()       { return aColumns; }
    USHORT           GetNumCols() const { return aColumns.Count(); }

    ULONG           GetLineWidth() const  { return nLineWidth;}
    const Color&    GetLineColor() const { return aLineColor;}


    SwColLineAdj     GetLineAdj() const { return eAdj; }
    BOOL             IsOrtho()    const { return bOrtho; }
    USHORT           GetWishWidth() const { return nWidth; }
    BYTE             GetLineHeight()const { return nLineHeight; }

    // Return USHRT_MAX if ambiguous.
    // Return smallest width if bMin is true.
    USHORT GetGutterWidth( BOOL bMin = FALSE ) const;

    void SetLineWidth(ULONG nLWidth)        { nLineWidth = nLWidth;}
    void SetLineColor(const Color& rCol )   { aLineColor = rCol;}
    void SetLineHeight( BYTE nNew )     { nLineHeight = nNew; }
    void SetLineAdj( SwColLineAdj eNew ){ eAdj = eNew; }
    void SetWishWidth( USHORT nNew )    { nWidth = nNew; }

    // This function allows to (repeatedly) initialize the columns.
    // The Ortho flag is set automatically.
    void Init( USHORT nNumCols, USHORT nGutterWidth, USHORT nAct );

    // Adjusts borders for columns in aColumns.
    // If flag bOrtho is set, columns are visually re-arranged.
    // If the flag is not set, columns widths are not changed and
    // borders are adjusted.
    void SetGutterWidth( USHORT nNew, USHORT nAct );

    // This too re-arranges columns automatically if flag is set.
    // Only in this case the second parameter is needed and evaluated.
    void SetOrtho( BOOL bNew, USHORT nGutterWidth, USHORT nAct );

    //For the reader
    void _SetOrtho( BOOL bNew ) { bOrtho = bNew; }

    // Calculates current width of column nCol.
    // The ratio of desired width of this column to return value is
    // proportional to ratio of total desired value to nAct.
    USHORT CalcColWidth( USHORT nCol, USHORT nAct ) const;

    // As above except that it returns the width of PrtArea -
    // that corresponds to what constitutes the column for the user.
    USHORT CalcPrtColWidth( USHORT nCol, USHORT nAct ) const;
};

inline const SwFmtCol &SwAttrSet::GetCol(BOOL bInP) const
    { return (const SwFmtCol&)Get( RES_COL,bInP); }

inline const SwFmtCol &SwFmt::GetCol(BOOL bInP) const
    { return aSet.GetCol(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
