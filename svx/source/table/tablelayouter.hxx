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

#ifndef _SVX_TABLE_TABLELAYOUTER_HXX_
#define _SVX_TABLE_TABLELAYOUTER_HXX_

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/table/XTable.hpp>
#include <basegfx/range/b2irectangle.hxx>
#include <basegfx/tuple/b2ituple.hxx>
#include <tools/gen.hxx>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <map>

#include "svx/svdotable.hxx"

// -----------------------------------------------------------------------------

class SvxBorderLine;

namespace sdr { namespace table {

/** returns true if the cell(nMergedCol,nMergedRow) is merged with other cells.
    the returned cell( rOriginCol, rOriginRow ) is the origin( top left cell ) of the merge.
*/
bool findMergeOrigin( const TableModelRef& xTable, sal_Int32 nMergedCol, sal_Int32 nMergedRow, sal_Int32& rOriginCol, sal_Int32& rOriginRow );

typedef std::vector< SvxBorderLine* > BorderLineVector;
typedef std::vector< BorderLineVector > BorderLineMap;

// -----------------------------------------------------------------------------
// TableModel
// -----------------------------------------------------------------------------

class TableLayouter
{
public:
    TableLayouter( const TableModelRef& xTableModel );
    virtual ~TableLayouter();

    /** this checks if new rows are inserted or old rows where removed.
        This can be used to grow or shrink the table shape in this case.
        @returns
            the height difference
    sal_Int32 detectInsertedOrRemovedRows();
    */

    /** try to fit the table into the given rectangle.
        If the rectangle is to small, it will be grown to fit the table.

        if bFitWidth or bFitHeight is set, the layouter tries to scale
        the rows and/or columns to the given area. The result my be bigger
        to fullfill constrains.

        if bFitWidth or bFitHeight is set, the model is changed.
    */
    void LayoutTable( ::Rectangle& rRectangle, bool bFitWidth, bool bFitHeight );

    /** after a call to LayoutTable, this method can be used to set the new
        column and row sizes back to the model. */
//  void SetLayoutToModel();

    void UpdateBorderLayout();

    basegfx::B2ITuple getCellSize( const CellPos& rPos ) const;
    bool getCellArea( const CellRef& xCell, basegfx::B2IRectangle& rArea ) const;
    bool getCellArea( const CellPos& rPos, basegfx::B2IRectangle& rArea ) const;

    ::sal_Int32 getRowCount() const { return static_cast< ::sal_Int32 >( maRows.size() ); }
    ::sal_Int32 getColumnCount() const { return static_cast< ::sal_Int32 >( maColumns.size() ); }
    sal_Int32 getRowHeight( sal_Int32 nRow ) const;

    // sets the layout height of the given row hard, LayoutTable must be called directly after calling this method! */
    void setRowHeight( sal_Int32 nRow, sal_Int32 nHeight );

    sal_Int32 getColumnWidth( sal_Int32 nColumn ) const;

    // sets the layout width of the given column hard, LayoutTable must be called directly after calling this method! */
    void setColumnWidth( sal_Int32 nColumn, sal_Int32 nWidth );

    sal_Int32 getMinimumColumnWidth( sal_Int32 nColumn );

    sal_Int32 getColumnStart( sal_Int32 nColumn ) const;
    sal_Int32 getRowStart( sal_Int32 nRow ) const;

    /** checks if the given edge is visible.
        Edges between merged cells are not visible.
    */
    bool isEdgeVisible( sal_Int32 nEdgeX, sal_Int32 nEdgeY, bool bHorizontal ) const;

    /** returns the requested borderline in rpBorderLine or a null pointer if there is no border at this edge */
    SvxBorderLine* getBorderLine( sal_Int32 nEdgeX, sal_Int32 nEdgeY, bool bHorizontal )const;

    void updateCells( ::Rectangle& rRectangle );

    sal_Int32 getHorizontalEdge( int nEdgeY, sal_Int32* pnMin = 0, sal_Int32* pnMax = 0 );
    sal_Int32 getVerticalEdge( int nEdgeX , sal_Int32* pnMin = 0, sal_Int32* pnMax = 0);

    void DistributeColumns( ::Rectangle& rArea, sal_Int32 nFirstCol, sal_Int32 nLastCol );
    void DistributeRows( ::Rectangle& rArea, sal_Int32 nFirstRow, sal_Int32 nLastRow );

    com::sun::star::text::WritingMode GetWritingMode() const { return meWritingMode; }
    void SetWritingMode( com::sun::star::text::WritingMode eWritingMode );

private:
    CellRef getCell( const CellPos& rPos ) const;

    void LayoutTableWidth( ::Rectangle& rArea, bool bFit );
    void LayoutTableHeight( ::Rectangle& rArea, bool bFit );

    inline bool isValidColumn( sal_Int32 nColumn ) const { return (nColumn >= 0) && (nColumn < static_cast<sal_Int32>( maColumns.size())); }
    inline bool isValidRow( sal_Int32 nRow ) const { return (nRow >= 0) && (nRow < static_cast<sal_Int32>( maRows.size())); }
    inline bool isValid( const CellPos& rPos ) const { return isValidColumn( rPos.mnCol ) && isValidRow( rPos.mnRow ); }

    void ClearBorderLayout();
    void ClearBorderLayout(BorderLineMap& rMap);
    void ResizeBorderLayout();
    void ResizeBorderLayout( BorderLineMap& rMap );

    void SetBorder( sal_Int32 nCol, sal_Int32 nRow, bool bHorizontal, const SvxBorderLine* pLine );

    static bool HasPriority( const SvxBorderLine* pThis, const SvxBorderLine* pOther );

    struct Layout
    {
        sal_Int32 mnPos;
        sal_Int32 mnSize;
        sal_Int32 mnMinSize;

        Layout() : mnPos( 0 ), mnSize( 0 ), mnMinSize( 0 ) {}
        void clear() { mnPos = 0; mnSize = 0; mnMinSize = 0; }
    };
    typedef std::vector< Layout > LayoutVector;

    sal_Int32 distribute( LayoutVector& rLayouts, sal_Int32 nDistribute );

    TableModelRef mxTable;
    LayoutVector maRows;
    LayoutVector maColumns;

    BorderLineMap maHorizontalBorders;
    BorderLineMap maVerticalBorders;

    com::sun::star::text::WritingMode   meWritingMode;

    const rtl::OUString msSize;
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
