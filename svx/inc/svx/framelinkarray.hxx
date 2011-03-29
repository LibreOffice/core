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

#ifndef SVX_FRAMELINKARRAY_HXX
#define SVX_FRAMELINKARRAY_HXX

#include <svx/framelink.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>

#include <memory>

#include <vector>
#include "svx/svxdllapi.h"

namespace svx {
namespace frame {

// ============================================================================


struct Cell
{
    Style               maLeft;
    Style               maRight;
    Style               maTop;
    Style               maBottom;
    Style               maTLBR;
    Style               maBLTR;
    long                mnAddLeft;
    long                mnAddRight;
    long                mnAddTop;
    long                mnAddBottom;
    bool                mbMergeOrig;
    bool                mbOverlapX;
    bool                mbOverlapY;

    explicit            Cell();

    inline bool         IsMerged() const { return mbMergeOrig || mbOverlapX || mbOverlapY; }
    inline bool         IsOverlapped() const { return mbOverlapX || mbOverlapY; }

    void                MirrorSelfX( bool bMirrorStyles, bool bSwapDiag );
    void                MirrorSelfY( bool bMirrorStyles, bool bSwapDiag );
};

typedef std::vector< long >     LongVec;
typedef std::vector< Cell >     CellVec;

struct ArrayImpl
{
    CellVec             maCells;
    LongVec             maWidths;
    LongVec             maHeights;
    mutable LongVec     maXCoords;
    mutable LongVec     maYCoords;
    size_t              mnWidth;
    size_t              mnHeight;
    size_t              mnFirstClipCol;
    size_t              mnFirstClipRow;
    size_t              mnLastClipCol;
    size_t              mnLastClipRow;
    mutable bool        mbXCoordsDirty;
    mutable bool        mbYCoordsDirty;
    bool                mbDiagDblClip;

    explicit            ArrayImpl( size_t nWidth, size_t nHeight, bool bDiagDblClip );

    inline bool         IsValidPos( size_t nCol, size_t nRow ) const
                            { return (nCol < mnWidth) && (nRow < mnHeight); }
    inline size_t       GetIndex( size_t nCol, size_t nRow ) const
                            { return nRow * mnWidth + nCol; }

    const Cell&         GetCell( size_t nCol, size_t nRow ) const;
    Cell&               GetCellAcc( size_t nCol, size_t nRow );

    size_t              GetMergedFirstCol( size_t nCol, size_t nRow ) const;
    size_t              GetMergedFirstRow( size_t nCol, size_t nRow ) const;
    size_t              GetMergedLastCol( size_t nCol, size_t nRow ) const;
    size_t              GetMergedLastRow( size_t nCol, size_t nRow ) const;

    const Cell&         GetMergedOriginCell( size_t nCol, size_t nRow ) const;
    Cell&               GetMergedOriginCellAcc( size_t nCol, size_t nRow );

    bool                IsMergedOverlappedLeft( size_t nCol, size_t nRow ) const;
    bool                IsMergedOverlappedRight( size_t nCol, size_t nRow ) const;
    bool                IsMergedOverlappedTop( size_t nCol, size_t nRow ) const;
    bool                IsMergedOverlappedBottom( size_t nCol, size_t nRow ) const;

    bool                IsInClipRange( size_t nCol, size_t nRow ) const;
    bool                IsColInClipRange( size_t nCol ) const;
    bool                IsRowInClipRange( size_t nRow ) const;

    inline size_t       GetMirrorCol( size_t nCol ) const { return mnWidth - nCol - 1; }
    inline size_t       GetMirrorRow( size_t nRow ) const { return mnHeight - nRow - 1; }

    long                GetColPosition( size_t nCol ) const;
    long                GetRowPosition( size_t nRow ) const;

    long                GetColWidth( size_t nFirstCol, size_t nLastCol ) const;
    long                GetRowHeight( size_t nFirstRow, size_t nLastRow ) const;

    double              GetHorDiagAngle( size_t nCol, size_t nRow, bool bSimple = false ) const;
    double              GetVerDiagAngle( size_t nCol, size_t nRow, bool bSimple = false ) const;
};

/** Stores frame styles of an array of cells, supports merged ranges.

    This class is able to store the frame styles of an array of cells and to
    draw the entire array or parts of it to any output device.

    Every cell contains the style of the left, right, top, bottom, top-left to
    bottom-right, and bottom-left to top-right frame border.

    On drawing, the thicker frame border of neighbored cells is selected
    automatically. All borders are drawn "connected", that means, the correct
    start and end coordinates of all lines of the borders are calculated,
    especaially, if they are drawn together with diagonal frame borders.

    This array fully supports merged cell ranges. In a merged range, the frame
    borders of the top-left cell is extended to the entire range, and all other
    cells in that range are overlapped. Again, all connected frame borders,
    also diagonals and frame borders from adjacent merged ranges, are handled
    automatically.

    Additionally, a clipping range can be set. If such a range is used, all
    frame borders outside this range are completely ignored, and are not used
    in the connected border calculation anymore.

    The array can be mirrored in both directions. It is possible to specify,
    whether to mirror the double frame styles, and whether to swap diagonal
    frame borders.
 */
class SVX_DLLPUBLIC Array
{
public:
    /** Constructs an empty array. */
    explicit            Array();

    /** Constructs an array with the specified width and height. */
    explicit            Array( size_t nWidth, size_t nHeight );

    /** Destructs the array. */
                        ~Array();

    // array size and column/row indexes --------------------------------------

    /** Reinitializes the array with the specified size. Clears all styles. */
    void                Initialize( size_t nWidth, size_t nHeight );

    /** Clears all line styles, column widths, row heights, merge data, and the clip range. */
    void                Clear();

    /** Returns the number of columns in the array. */
    size_t              GetColCount() const;

    /** Returns the number of rows in the array. */
    size_t              GetRowCount() const;

    /** Returns the number of cells in the array. */
    size_t              GetCellCount() const;

    /** Returns the column index of the specified cell index. */
    size_t              GetColFromIndex( size_t nCellIndex ) const;

    /** Returns the row index of the specified cell index. */
    size_t              GetRowFromIndex( size_t nCellIndex ) const;

    /** Returns the cell index from the cell address (nCol,nRow). */
    size_t              GetCellIndex( size_t nCol, size_t nRow, bool bRTL = false) const;

    // cell border styles -----------------------------------------------------

    /** Sets the left frame style of the cell (nCol,nRow). Ignores merged ranges. */
    void                SetCellStyleLeft( size_t nCol, size_t nRow, const Style& rStyle );

    /** Sets the right frame style of the cell (nCol,nRow). Ignores merged ranges. */
    void                SetCellStyleRight( size_t nCol, size_t nRow, const Style& rStyle );

    /** Sets the top frame style of the cell (nCol,nRow). Ignores merged ranges. */
    void                SetCellStyleTop( size_t nCol, size_t nRow, const Style& rStyle );

    /** Sets the bottom frame style of the specified cell (nCol,nRow). Ignores merged ranges. */
    void                SetCellStyleBottom( size_t nCol, size_t nRow, const Style& rStyle );

    /** Sets the top-left to bottom-right frame style of the cell (nCol,nRow). Ignores merged ranges. */
    void                SetCellStyleTLBR( size_t nCol, size_t nRow, const Style& rStyle );

    /** Sets the bottom-left to top-right frame style of the cell (nCol,nRow). Ignores merged ranges. */
    void                SetCellStyleBLTR( size_t nCol, size_t nRow, const Style& rStyle );

    /** Sets both diagonal frame styles of the specified cell (nCol,nRow). Ignores merged ranges. */
    void                SetCellStyleDiag( size_t nCol, size_t nRow, const Style& rTLBR, const Style& rBLTR );

    /** Sets the left frame style of the specified column. Ignores merged ranges. */
    void                SetColumnStyleLeft( size_t nCol, const Style& rStyle );

    /** Sets the right frame style of the specified column. Ignores merged ranges. */
    void                SetColumnStyleRight( size_t nCol, const Style& rStyle );

    /** Sets the top frame style of the specified row. Ignores merged ranges. */
    void                SetRowStyleTop( size_t nRow, const Style& rStyle );

    /** Sets the bottom frame style of the specified row. Ignores merged ranges. */
    void                SetRowStyleBottom( size_t nRow, const Style& rStyle );

    /** Returns the left frame style of the cell (nCol,nRow).
        @param bSimple
            true = Ignores merged ranges and neighbor cells;
            false = Returns thicker of own left style or right style of the cell to the left.
                Returns the style only if visible (i.e. at left border of a merged range).
        @return
            The left frame style or an invisible style for invalid cell addresses. */
    const Style&        GetCellStyleLeft( size_t nCol, size_t nRow, bool bSimple = false ) const;

    /** Returns the right frame style of the cell (nCol,nRow).
        @param bSimple
            true = Ignores merged ranges and neighbor cells;
            false = Returns thicker of own right style or left style of the cell to the right.
                Returns the style only if visible (i.e. at right border of a merged range).
        @return
            The left frame style or an invisible style for invalid cell addresses. */
    const Style&        GetCellStyleRight( size_t nCol, size_t nRow, bool bSimple = false ) const;

    /** Returns the top frame style of the cell (nCol,nRow).
        @param bSimple
            true = Ignores merged ranges and neighbor cells;
            false = Returns thicker of own top style or bottom style of the cell above.
                Returns the style only if visible (i.e. at top border of a merged range).
        @return
            The top frame style or an invisible style for invalid cell addresses. */
    const Style&        GetCellStyleTop( size_t nCol, size_t nRow, bool bSimple = false ) const;

    /** Returns the top frame style of the cell (nCol,nRow).
        @param bSimple
            true = Ignores merged ranges and neighbor cells;
            false = Returns thicker of own top style or bottom style of the cell above.
                Returns the style only if visible (i.e. at top border of a merged range).
        @return
            The top frame style or an invisible style for invalid cell addresses. */
    const Style&        GetCellStyleBottom( size_t nCol, size_t nRow, bool bSimple = false ) const;

    /** Returns the top-left to bottom-right frame style of the cell (nCol,nRow).
        @param bSimple
            true = Ignores merged ranges;
            false = Returns the visible style (i.e. from top-left corner of a merged range).
        @return
            The top-left to bottom-right frame style or an invisible style for invalid cell addresses. */
    const Style&        GetCellStyleTLBR( size_t nCol, size_t nRow, bool bSimple = false ) const;

    /** Returns the bottom-left to top-right frame style of the cell (nCol,nRow).
        @param bSimple
            true = Ignores merged ranges;
            false = Returns the visible style (i.e. from top-left corner of a merged range).
        @return
            The bottom-left to top-right frame style or an invisible style for invalid cell addresses. */
    const Style&        GetCellStyleBLTR( size_t nCol, size_t nRow, bool bSimple = false ) const;

    /** Returns the top-left to bottom-right frame style of the cell (nCol,nRow).
        @return
            The top-left to bottom-right frame style, if the cell is not part of
            a merged range, or if (nCol,nRow) is the top-left corner of a merged
            range (useful to find connected frame styles).
            An invisible style for invalid cell addresses. */
    const Style&        GetCellStyleTL( size_t nCol, size_t nRow ) const;

    /** Returns the top-left to bottom-right frame style of the cell (nCol,nRow).
        @return
            The top-left to bottom-right frame style, if the cell is not part of
            a merged range, or if (nCol,nRow) is the bottom-right corner of a
            merged range (useful to find connected frame styles).
            An invisible style for invalid cell addresses. */
    const Style&        GetCellStyleBR( size_t nCol, size_t nRow ) const;

    /** Returns the bottom-left to top-right frame style of the cell (nCol,nRow).
        @return
            The bottom-left to top-right frame style, if the cell is not part of
            a merged range, or if (nCol,nRow) is the bottom-left corner of a
            merged range (useful to find connected frame styles).
            An invisible style for invalid cell addresses. */
    const Style&        GetCellStyleBL( size_t nCol, size_t nRow ) const;

    /** Returns the bottom-left to top-right frame style of the cell (nCol,nRow).
        @return
            The bottom-left to top-right frame style, if the cell is not part of
            a merged range, or if (nCol,nRow) is the top-right corner of a
            merged range (useful to find connected frame styles).
            An invisible style for invalid cell addresses. */
    const Style&        GetCellStyleTR( size_t nCol, size_t nRow ) const;

    // cell merging -----------------------------------------------------------

    /** Inserts a new merged cell range.
        @precond  The range must not intersect other merged ranges. */
    void                SetMergedRange( size_t nFirstCol, size_t nFirstRow, size_t nLastCol, size_t nLastRow );

    /** Removes the merged cell range that contains (nCol,nRow). */
    void                RemoveMergedRange( size_t nCol, size_t nRow );

    /** Sets an additional left width for the merged range that contains (nCol,nRow).
        @descr  Useful to handle merged ranges that are not completely part of the array.
        @precond  The merged range must be at the left border of the array. */
    void                SetAddMergedLeftSize( size_t nCol, size_t nRow, long nAddSize );

    /** Sets an additional right width for the merged range that contains (nCol,nRow).
        @descr  Useful to handle merged ranges that are not completely part of the array.
        @precond  The merged range must be at the right border of the array. */
    void                SetAddMergedRightSize( size_t nCol, size_t nRow, long nAddSize );

    /** Sets an additional top height for the merged range that contains (nCol,nRow).
        @descr  Useful to handle merged ranges that are not completely part of the array.
        @precond  The merged range must be at the top border of the array. */
    void                SetAddMergedTopSize( size_t nCol, size_t nRow, long nAddSize );

    /** Sets an additional bottom height for the merged range that contains (nCol,nRow).
        @descr  Useful to handle merged ranges that are not completely part of the array.
        @precond  The merged range must be at the bottom border of the array. */
    void                SetAddMergedBottomSize( size_t nCol, size_t nRow, long nAddSize );

    /** Returns true, if the cell (nCol,nRow) is part of a merged range. */
    bool                IsMerged( size_t nCol, size_t nRow ) const;

    /** Returns true, if the cell (nCol,nRow) is the top-left corner of a merged range. */
    bool                IsMergedOrigin( size_t nCol, size_t nRow ) const;

    /** Returns true, if the cell (nCol,nRow) is overlapped by a merged range. */
    bool                IsMergedOverlapped( size_t nCol, size_t nRow ) const;

    /** Returns true, if the left border of the cell (nCol,nRow) is overlapped by a merged range. */
    bool                IsMergedOverlappedLeft( size_t nCol, size_t nRow ) const;

    /** Returns true, if the right border of the cell (nCol,nRow) is overlapped by a merged range. */
    bool                IsMergedOverlappedRight( size_t nCol, size_t nRow ) const;

    /** Returns true, if the top border of the cell (nCol,nRow) is overlapped by a merged range. */
    bool                IsMergedOverlappedTop( size_t nCol, size_t nRow ) const;

    /** Returns true, if the bottom border of the cell (nCol,nRow) is overlapped by a merged range. */
    bool                IsMergedOverlappedBottom( size_t nCol, size_t nRow ) const;

    /** Returns the address of the top-left cell of the merged range that contains (nCol,nRow). */
    void                GetMergedOrigin( size_t& rnFirstCol, size_t& rnFirstRow, size_t nCol, size_t nRow ) const;

    /** Returns the range size of the merged range thst contains (nCol,nRow). */
    void                GetMergedSize( size_t& rnWidth, size_t& rnHeight, size_t nCol, size_t nRow ) const;

    /** Returns the top-left and bottom-right address of the merged range that contains (nCol,nRow). */
    void                GetMergedRange( size_t& rnFirstCol, size_t& rnFirstRow,
                            size_t& rnLastCol, size_t& rnLastRow, size_t nCol, size_t nRow ) const;

    // clipping ---------------------------------------------------------------

    /** Sets a clipping range.
        @descr
            No cell borders outside of this clipping range will be drawn. In
            difference to simply using the DrawRange() function with the same
            range, a clipping range causes the drawing functions to completely
            ignore the frame styles connected from outside. This is used i.e.
            in Calc to print single pages and to draw the print preview.
            Partly visible diagonal frame borders in merged ranges are correctly
            clipped too. This array can handle only one clip range at a time. */
    void                SetClipRange( size_t nFirstCol, size_t nFirstRow, size_t nLastCol, size_t nLastRow );

    /** Removes the clipping range set with the SetClipRange() function. */
    void                RemoveClipRange();

    /** Returns true, if the cell (bCol,nRow) is inside the current clip range. */
    bool                IsInClipRange( size_t nCol, size_t nRow ) const;

    /** Returns the rectangle (output coordinates) of the current clipping range. */
    Rectangle           GetClipRangeRectangle() const;

    // cell coordinates -------------------------------------------------------

    /** Sets the X output coordinate of the left column. */
    void                SetXOffset( long nXOffset );

    /** Sets the Y output coordinate of the top row. */
    void                SetYOffset( long nYOffset );

    /** Sets the output width of the specified column. */
    void                SetColWidth( size_t nCol, long nWidth );

    /** Sets the output height of the specified row. */
    void                SetRowHeight( size_t nRow, long nHeight );

    /** Sets the same output width for all columns. */
    void                SetAllColWidths( long nWidth );

    /** Sets the same output height for all rows. */
    void                SetAllRowHeights( long nHeight );

    /** Returns the X output coordinate of the left border of the specified column.
        @descr  The column index <array-width> returns the X output coordinate
                of the right array border. */
    long                GetColPosition( size_t nCol ) const;

    /** Returns the Y output coordinate of the top border of the specified row.
        @descr  The row index <array-height> returns the Y output coordinate
                of the bottom array border. */
    long                GetRowPosition( size_t nRow ) const;

    /** Returns the output width of the specified column. */
    long                GetColWidth( size_t nCol ) const;

    /** Returns the output width of the specified range of columns. */
    long                GetColWidth( size_t nFirstCol, size_t nLastCol ) const;

    /** Returns the output height of the specified row. */
    long                GetRowHeight( size_t nRow ) const;

    /** Returns the output height of the specified range of rows. */
    long                GetRowHeight( size_t nFirstRow, size_t nLastRow ) const;

    /** Returns the output width of the entire array. */
    long                GetWidth() const;

    /** Returns the output height of the entire array. */
    long                GetHeight() const;

    /** Returns the top-left output position of the cell (nCol,nRow).
        @param bSimple
            true = Ignores merged ranges;
            false = Returns output position of top-left corner of merged ranges. */
    Point               GetCellPosition( size_t nCol, size_t nRow, bool bSimple = false ) const;

    /** Returns the output size of the cell (nCol,nRow).
        @param bSimple
            true = Ignores merged ranges;
            false = Returns total output size of merged ranges. */
    Size                GetCellSize( size_t nCol, size_t nRow, bool bSimple = false ) const;

    /** Returns the output rectangle of the cell (nCol,nRow).
        @param bSimple
            true = Ignores merged ranges;
            false = Returns total output rectangle of merged ranges. */
    Rectangle           GetCellRect( size_t nCol, size_t nRow, bool bSimple = false ) const;

    // diagonal frame borders -------------------------------------------------

    /** Returns the angle between horizontal and diagonal border of the cell (nCol,nRow).
        @param bSimple
            true = Ignores merged ranges;
            false = Returns the horizontal angle of merged ranges. */
    double              GetHorDiagAngle( size_t nCol, size_t nRow, bool bSimple = false ) const;

    /** Returns the angle between vertical and diagonal border of the cell (nCol,nRow).
        @param bSimple
            true = Ignores merged ranges;
            false = Returns the vertical angle of merged ranges. */
    double              GetVerDiagAngle( size_t nCol, size_t nRow, bool bSimple = false ) const;

    /** Specifies whether to use polygon clipping to draw diagonal frame borders.
        @descr
            If enabled, diagonal frame borders are drawn interrupted, if they are
            crossed by a double frame border. Polygon clipping is very expensive
            and should only be used for very small output devices (i.e. in the
            Border tab page). Default after construction is OFF. */
    void                SetUseDiagDoubleClipping( bool bSet );

    /** Returns true, if polygon clipping is used to draw diagonal frame borders. */
    bool                GetUseDiagDoubleClipping() const;

    // mirroring --------------------------------------------------------------

    /** Mirrors the entire array horizontally.
        @param bMirrorStyles
            true = Swap primary and secondary line of all vertical double frame borders.
        @param bSwapDiag
            true = Swap top-left to bottom-right and bottom-left to top-right frame borders. */
    void                MirrorSelfX( bool bMirrorStyles, bool bSwapDiag );

    /** Mirrors the entire array vertically.
        @param bMirrorStyles
            true = Swap primary and secondary line of all horizontal double frame borders.
        @param bSwapDiag
            true = Swap top-left to bottom-right and bottom-left to top-right frame borders. */
    void                MirrorSelfY( bool bMirrorStyles, bool bSwapDiag );

    // drawing ----------------------------------------------------------------

    /** Draws the cell (nCol,nRow), if it is inside the clipping range.
        @param pForceColor
            If not NULL, only this color will be used to draw all frame borders. */
    void                DrawCell( OutputDevice& rDev, size_t nCol, size_t nRow,
                            const Color* pForceColor = 0 ) const;
    /** Draws the part of the specified range, that is inside the clipping range.
        @param pForceColor
            If not NULL, only this color will be used to draw all frame borders. */
    void                DrawRange( drawinglayer::processor2d::BaseProcessor2D* rDev,
                            size_t nFirstCol, size_t nFirstRow,
                            size_t nLastCol, size_t nLastRow,
                            const Color* pForceColor = 0 ) const;

    /** Draws the part of the specified range, that is inside the clipping range.
        @param pForceColor
            If not NULL, only this color will be used to draw all frame borders. */
    void                DrawRange( OutputDevice& rDev,
                            size_t nFirstCol, size_t nFirstRow,
                            size_t nLastCol, size_t nLastRow,
                            const Color* pForceColor = 0 ) const;

    /** Draws the part of the array, that is inside the clipping range.
        @param pForceColor
            If not NULL, only this color will be used to draw all frame borders. */
    void                DrawArray( OutputDevice& rDev, const Color* pForceColor = 0 ) const;

    // ------------------------------------------------------------------------

private:
    typedef std::auto_ptr< ArrayImpl > ArrayImplPtr;

    ArrayImplPtr        mxImpl;
};

// ============================================================================

} // namespace frame
} // namespace svx

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
