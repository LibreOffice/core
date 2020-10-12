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

#include <drawinglayer/primitive2d/Primitive2DContainer.hxx>
#include <svx/svxdllapi.h>
#include <svx/rotmodit.hxx>
#include <memory>

class Color;

namespace svx::frame {

struct ArrayImpl;
class Style;

/** Stores frame styles of an array of cells, supports merged ranges.

    This class is able to store the frame styles of an array of cells and to
    draw the entire array or parts of it to any output device.

    Every cell contains the style of the left, right, top, bottom, top-left to
    bottom-right, and bottom-left to top-right frame border.

    On drawing, the thicker frame border of neighbored cells is selected
    automatically. All borders are drawn "connected", that means, the correct
    start and end coordinates of all lines of the borders are calculated,
    especially, if they are drawn together with diagonal frame borders.

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
class SAL_WARN_UNUSED SVXCORE_DLLPUBLIC Array
{
public:
    /** Constructs an empty array. */
    explicit            Array();

    /** Destructs the array. */
                        ~Array();

    // array size and column/row indexes

    /** Reinitializes the array with the specified size. Clears all styles. */
    void                Initialize( size_t nWidth, size_t nHeight );

    /** Returns the number of columns in the array. */
    size_t              GetColCount() const;

    /** Returns the number of rows in the array. */
    size_t              GetRowCount() const;

    /** Returns the number of cells in the array. */
    size_t              GetCellCount() const;

    /** Returns the cell index from the cell address (nCol,nRow). */
    size_t              GetCellIndex( size_t nCol, size_t nRow, bool bRTL) const;

    // cell border styles

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

    /** Sets the rotation parameters of the cell (nCol,nRow). Ignores merged ranges. */
    void                SetCellRotation(size_t nCol, size_t nRow, SvxRotateMode eRotMode, double fOrientation);

    /** Check if at least one cell is rotated */
    bool                HasCellRotation() const;

    /** Returns the left frame style of the cell (nCol,nRow).
        Returns thicker of own left style or right style of the cell to the left.
        Returns the style only if visible (i.e. at left border of a merged range).
        @return
            The left frame style or an invisible style for invalid cell addresses. */
    const Style&        GetCellStyleLeft( size_t nCol, size_t nRow ) const;

    /** Returns the right frame style of the cell (nCol,nRow).
        Returns thicker of own right style or left style of the cell to the right.
        Returns the style only if visible (i.e. at right border of a merged range).
        @return
            The left frame style or an invisible style for invalid cell addresses. */
    const Style&        GetCellStyleRight( size_t nCol, size_t nRow ) const;

    /** Returns the top frame style of the cell (nCol,nRow).
        Returns thicker of own top style or bottom style of the cell above.
        Returns the style only if visible (i.e. at top border of a merged range).
        @return
            The top frame style or an invisible style for invalid cell addresses. */
    const Style&        GetCellStyleTop( size_t nCol, size_t nRow ) const;

    /** Returns the top frame style of the cell (nCol,nRow).
        Returns thicker of own top style or bottom style of the cell above.
        Returns the style only if visible (i.e. at top border of a merged range).
        @return
            The top frame style or an invisible style for invalid cell addresses. */
    const Style&        GetCellStyleBottom( size_t nCol, size_t nRow ) const;

    /** Returns the top-left to bottom-right frame style of the cell (nCol,nRow).
        Ignores merged ranges;
        @return
            The top-left to bottom-right frame style or an invisible style for invalid cell addresses. */
    const Style&        GetCellStyleTLBR( size_t nCol, size_t nRow ) const;

    /** Returns the bottom-left to top-right frame style of the cell (nCol,nRow).
        Ignores merged ranges;
        @return
            The bottom-left to top-right frame style or an invisible style for invalid cell addresses. */
    const Style&        GetCellStyleBLTR( size_t nCol, size_t nRow ) const;

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

    // cell merging

    /** Inserts a new merged cell range.
        @precond  The range must not intersect other merged ranges. */
    void                SetMergedRange( size_t nFirstCol, size_t nFirstRow, size_t nLastCol, size_t nLastRow );

    /** Sets an additional left width for the merged range that contains (nCol,nRow).
        @descr  Useful to handle merged ranges that are not completely part of the array.
        @precond  The merged range must be at the left border of the array. */
    void                SetAddMergedLeftSize( size_t nCol, size_t nRow, tools::Long nAddSize );

    /** Sets an additional right width for the merged range that contains (nCol,nRow).
        @descr  Useful to handle merged ranges that are not completely part of the array.
        @precond  The merged range must be at the right border of the array. */
    void                SetAddMergedRightSize( size_t nCol, size_t nRow, tools::Long nAddSize );

    /** Sets an additional top height for the merged range that contains (nCol,nRow).
        @descr  Useful to handle merged ranges that are not completely part of the array.
        @precond  The merged range must be at the top border of the array. */
    void                SetAddMergedTopSize( size_t nCol, size_t nRow, tools::Long nAddSize );

    /** Sets an additional bottom height for the merged range that contains (nCol,nRow).
        @descr  Useful to handle merged ranges that are not completely part of the array.
        @precond  The merged range must be at the bottom border of the array. */
    void                SetAddMergedBottomSize( size_t nCol, size_t nRow, tools::Long nAddSize );

    /** Returns true, if the cell (nCol,nRow) is part of a merged range. */
    bool                IsMerged( size_t nCol, size_t nRow ) const;

    /** Returns the address of the top-left cell of the merged range that contains (nCol,nRow). */
    void                GetMergedOrigin( size_t& rnFirstCol, size_t& rnFirstRow, size_t nCol, size_t nRow ) const;

    /** Returns the top-left and bottom-right address of the merged range that contains (nCol,nRow). */
    void                GetMergedRange( size_t& rnFirstCol, size_t& rnFirstRow,
                            size_t& rnLastCol, size_t& rnLastRow, size_t nCol, size_t nRow ) const;

    // clipping

    /** Sets a clipping range.
        @descr
            No cell borders outside of this clipping range will be drawn. In
            difference to simply using the CreateB2DPrimitiveRange() function with the same
            range, a clipping range causes the drawing functions to completely
            ignore the frame styles connected from outside. This is used i.e.
            in Calc to print single pages and to draw the print preview.
            Partly visible diagonal frame borders in merged ranges are correctly
            clipped too. This array can handle only one clip range at a time. */
    void                SetClipRange( size_t nFirstCol, size_t nFirstRow, size_t nLastCol, size_t nLastRow );

    // cell coordinates

    /** Sets the X output coordinate of the left column. */
    void                SetXOffset( tools::Long nXOffset );

    /** Sets the Y output coordinate of the top row. */
    void                SetYOffset( tools::Long nYOffset );

    /** Sets the output width of the specified column. */
    void                SetColWidth( size_t nCol, tools::Long nWidth );

    /** Sets the output height of the specified row. */
    void                SetRowHeight( size_t nRow, tools::Long nHeight );

    /** Sets the same output width for all columns. */
    void                SetAllColWidths( tools::Long nWidth );

    /** Sets the same output height for all rows. */
    void                SetAllRowHeights( tools::Long nHeight );

    /** Returns the X output coordinate of the left border of the specified column.
        @descr  The column index <array-width> returns the X output coordinate
                of the right array border. */
    tools::Long                GetColPosition( size_t nCol ) const;

    /** Returns the Y output coordinate of the top border of the specified row.
        @descr  The row index <array-height> returns the Y output coordinate
                of the bottom array border. */
    tools::Long                GetRowPosition( size_t nRow ) const;

    /** Returns the output width of the specified range of columns. */
    tools::Long                GetColWidth( size_t nFirstCol, size_t nLastCol ) const;

    /** Returns the output height of the specified range of rows. */
    tools::Long                GetRowHeight( size_t nFirstRow, size_t nLastRow ) const;

    /** Returns the output width of the entire array. */
    tools::Long                GetWidth() const;

    /** Returns the output height of the entire array. */
    tools::Long                GetHeight() const;

    /** Returns the output range of the cell (nCol,nRow).
        Returns total output range of merged ranges, if bExpandMerged is true. */
    basegfx::B2DRange GetCellRange( size_t nCol, size_t nRow, bool bExpandMerged ) const;

    // mirroring

    /** Mirrors the entire array horizontally. */
    void                MirrorSelfX();

    // drawing

    /** Draws the part of the specified range, that is inside the clipping range.
        @param pForceColor
            If not NULL, only this color will be used to draw all frame borders. */
    drawinglayer::primitive2d::Primitive2DContainer CreateB2DPrimitiveRange(
        size_t nFirstCol, size_t nFirstRow,
        size_t nLastCol, size_t nLastRow,
        const Color* pForceColor ) const;

    /** Draws the part of the array, that is inside the clipping range. */
    drawinglayer::primitive2d::Primitive2DContainer CreateB2DPrimitiveArray() const;

private:
    std::unique_ptr<ArrayImpl>        mxImpl;
};

}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
