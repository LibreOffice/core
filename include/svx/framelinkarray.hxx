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

#ifndef INCLUDED_SVX_FRAMELINKARRAY_HXX
#define INCLUDED_SVX_FRAMELINKARRAY_HXX

#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <svx/framelink.hxx>
#include <svx/svxdllapi.h>
#include <memory>
#include <vector>

namespace svx {
namespace frame {

struct ArrayImpl;

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
class SVX_DLLPUBLIC SAL_WARN_UNUSED Array
{
public:
    /** Constructs an empty array. */
    explicit            Array();

    /** Destructs the array. */
                        ~Array();

    // array size and column/row indexes --------------------------------------

    /** Reinitializes the array with the specified size. Clears all styles. */
    void                Initialize( size_t nWidth, size_t nHeight );

    /** Returns the number of columns in the array. */
    size_t              GetColCount() const;

    /** Returns the number of rows in the array. */
    size_t              GetRowCount() const;

    /** Returns the number of cells in the array. */
    size_t              GetCellCount() const;

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

    /** Returns true, if the left border of the cell (nCol,nRow) is overlapped by a merged range. */
    bool                IsMergedOverlappedLeft( size_t nCol, size_t nRow ) const;

    /** Returns true, if the right border of the cell (nCol,nRow) is overlapped by a merged range. */
    bool                IsMergedOverlappedRight( size_t nCol, size_t nRow ) const;

    /** Returns the address of the top-left cell of the merged range that contains (nCol,nRow). */
    void                GetMergedOrigin( size_t& rnFirstCol, size_t& rnFirstRow, size_t nCol, size_t nRow ) const;

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

    /** Returns the output width of the specified range of columns. */
    long                GetColWidth( size_t nFirstCol, size_t nLastCol ) const;

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

    // mirroring --------------------------------------------------------------

    /** Mirrors the entire array horizontally.
        @param bMirrorStyles
            true = Swap primary and secondary line of all vertical double frame borders.
        @param bSwapDiag
            true = Swap top-left to bottom-right and bottom-left to top-right frame borders. */
    void                MirrorSelfX( bool bMirrorStyles, bool bSwapDiag );

    // drawing ----------------------------------------------------------------

    /** Draws the part of the specified range, that is inside the clipping range.
        @param pForceColor
            If not NULL, only this color will be used to draw all frame borders. */
    void                DrawRange( drawinglayer::processor2d::BaseProcessor2D* rDev,
                            size_t nFirstCol, size_t nFirstRow,
                            size_t nLastCol, size_t nLastRow,
                            const Color* pForceColor = nullptr ) const;

    /** Draws the part of the specified range, that is inside the clipping range.
        @param pForceColor
            If not NULL, only this color will be used to draw all frame borders. */
    void                DrawRange( OutputDevice& rDev,
                            size_t nFirstCol, size_t nFirstRow,
                            size_t nLastCol, size_t nLastRow,
                            const Color* pForceColor = nullptr ) const;

    /** Draws the part of the array, that is inside the clipping range.
        @param pForceColor
            If not NULL, only this color will be used to draw all frame borders. */
    void                DrawArray( OutputDevice& rDev, const Color* pForceColor = nullptr ) const;



private:
    typedef std::unique_ptr<ArrayImpl> ArrayImplPtr;

    ArrayImplPtr        mxImpl;
};

}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
