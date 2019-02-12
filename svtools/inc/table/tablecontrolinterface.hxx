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

#ifndef INCLUDED_SVTOOLS_INC_TABLE_TABLECONTROLINTERFACE_HXX
#define INCLUDED_SVTOOLS_INC_TABLE_TABLECONTROLINTERFACE_HXX

#include <sal/types.h>
#include <vcl/event.hxx>
#include <vcl/seleng.hxx>

#include <svtools/table/tabletypes.hxx>
#include <svtools/table/tablemodel.hxx>


namespace svt { namespace table
{


    //= TableControlAction

    enum TableControlAction
    {
        /// moves the cursor in the table control one row up, if possible, by keeping the current column
        cursorUp,
        /// moves the cursor in the table control one row down, if possible, by keeping the current column
        cursorDown,
        /// moves the cursor in the table control one column to the left, if possible, by keeping the current row
        cursorLeft,
        /// moves the cursor in the table control one column to the right, if possible, by keeping the current row
        cursorRight,
        /// moves the cursor to the beginning of the current line
        cursorToLineStart,
        /// moves the cursor to the end of the current line
        cursorToLineEnd,
        /// moves the cursor to the first row, keeping the current column
        cursorToFirstLine,
        /// moves the cursor to the last row, keeping the current column
        cursorToLastLine,
        /// moves the cursor one page up, keeping the current column
        cursorPageUp,
        /// moves the cursor one page down, keeping the current column
        cursorPageDown,
        /// moves the cursor to the top-most, left-most cell
        cursorTopLeft,
        /// moves the cursor to the bottom-most, right-most cell
        cursorBottomRight,
        /// selects the row, where the actual cursor is
        cursorSelectRow,
        /// selects the rows, above the actual cursor is
        cursorSelectRowUp,
        /// selects the row, beneath the actual cursor is
        cursorSelectRowDown,
        /// selects the row, from the actual cursor till top
        cursorSelectRowAreaTop,
        /// selects the row, from the actual cursor till bottom
        cursorSelectRowAreaBottom,

        /// invalid and final enumeration value, not to be actually used
        invalidTableControlAction
    };


    //= TableCellArea

    enum TableCellArea
    {
        CellContent,
        ColumnDivider
    };


    //= TableCell

    struct TableCell
    {
        ColPos const    nColumn;
        RowPos const    nRow;
        TableCellArea   eArea;

        TableCell( ColPos const i_column, RowPos const i_row )
            :nColumn( i_column )
            ,nRow( i_row )
            ,eArea( CellContent )
        {
        }
    };


    //= ColumnMetrics

    struct ColumnMetrics
    {
        /** the start of the column, in pixels. Might be negative, in case the column is scrolled out of the visible
            area. Note: see below.
        */
        long    nStartPixel;

        /** the end of the column, in pixels, plus 1. Effectively, this is the accumulated width of a all columns
            up to the current one.

            Huh? Earlier you said that the nStartPixel of columns
            scrolled out (to the left) of the visible area is
            negative. Also, where is the promise that there is no gap
            between columns? The above claim would be true only if the
            first column always started at zero, and there is never a
            gap. So these doc comments are inconsistent. How
            surprising.
        */
        long    nEndPixel;

        ColumnMetrics()
            :nStartPixel(0)
            ,nEndPixel(0)
        {
        }

        ColumnMetrics( long const i_start, long const i_end )
            :nStartPixel( i_start )
            ,nEndPixel( i_end )
        {
        }
    };


    //= TableArea

    enum class TableArea
    {
        ColumnHeaders,
        RowHeaders,
        All
    };


    //= ITableControl

    /** defines a callback interface to be implemented by a concrete table control
    */
    class SAL_NO_VTABLE ITableControl
    {
    public:
        /** hides the cell cursor

            The method cares for successive calls, that is, for every call to
            ->hideCursor(), you need one call to ->showCursor. Only if the number
            of both calls matches, the cursor is really shown.

            @see showCursor
        */
        virtual void    hideCursor() = 0;

        /** shows the cell cursor

            @see hideCursor
        */
        virtual void    showCursor() = 0;

        /** dispatches an action to the table control

            @return
                <TRUE/> if the action could be dispatched successfully, <FALSE/> otherwise. Usual
                failure conditions include some other instance vetoing the action, or impossibility
                to execute the action at all (for instance moving up one row when already positioned
                on the very first row).

            @see TableControlAction
        */
        virtual bool    dispatchAction( TableControlAction _eAction ) = 0;

        /** returns selection engine*/
        virtual SelectionEngine* getSelEngine() = 0;

        /** returns the table model

            The returned model is guaranteed to not be <NULL/>.
        */
        virtual PTableModel getModel() const = 0;

        /// returns the index of the currently active column
        virtual ColPos  getCurrentColumn() const = 0;

        /// returns the index of the currently active row
        virtual RowPos  getCurrentRow() const = 0;

        /// activates the given cell
        virtual void    activateCell( ColPos const i_col, RowPos const i_row ) = 0;

        /// retrieves the size of the table window, in pixels
        virtual ::Size  getTableSizePixel() const = 0;

        /// sets a new mouse pointer for the table window
        virtual void    setPointer( PointerStyle i_pointer ) = 0;

        /// captures the mouse to the table window
        virtual void    captureMouse() = 0;

        /// releases the mouse, after it had previously been captured
        virtual void    releaseMouse() = 0;

        /// invalidates the table window
        virtual void    invalidate( TableArea const i_what ) = 0;

        /// calculates a width, given in pixels, into a AppFont-based width
        virtual long    pixelWidthToAppFont( long const i_pixels ) const = 0;

        /// shows a tracking rectangle
        virtual void    showTracking( tools::Rectangle const & i_location, ShowTrackFlags const i_flags ) = 0;

        /// hides a prviously shown tracking rectangle
        virtual void    hideTracking() = 0;

        /// does a hit test for the given pixel coordinates
        virtual TableCell       hitTest( const Point& rPoint ) const = 0;

        /// retrieves the metrics for a given column
        virtual ColumnMetrics   getColumnMetrics( ColPos const i_column ) const = 0;

        /// determines whether a given row is selected
        virtual bool isRowSelected( RowPos _nRow ) const = 0;

        virtual ~ITableControl() {};
    };


} } // namespace svt::table


#endif // INCLUDED_SVTOOLS_INC_TABLE_TABLECONTROLINTERFACE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
