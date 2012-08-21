/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

#ifndef SVTOOLS_INC_TABLE_TABLERENDERER_HXX
#define SVTOOLS_INC_TABLE_TABLERENDERER_HXX

#include <svtools/table/tabletypes.hxx>

#include <vcl/outdev.hxx>

#include <boost/shared_ptr.hpp>

//........................................................................
namespace svt { namespace table
{
//........................................................................

    //====================================================================
    //= ITableRenderer
    //====================================================================
    /** interface to implement by components rendering a ->TableControl
    */
    class SAL_NO_VTABLE ITableRenderer
    {
        public:

        /** paints a (part of) header area

            There are two header areas in a table control:
            <ul><li>The row containing all column headers, i.e. <em>above</em> all rows containing the data</li>
                <li>The column containing all row headers. i.e. <em>left of</em> all columns containing the data</li>
            </ul>

            A header area is more than the union of the single column/row headers.

            First, there might be less columns than fit into the view - in this case, right
            beside the right-most column, there's still room which belongs to the column header
            area, but is not occupied by any particular column header.<br/>
            An equivalent statement holds for the row header area, if there are less rows than
            fit into the view.

            Second, if the table control has both a row header and a column header,
            the intersection between those both belongs to both the column header area and the
            row header area, but not to any particular column or row header.

            There are two flags specifying whether the to-be-painted area is part of the column
            and/or row header area.
            <ul><li>If both are <TRUE/>, the intersection of both areas is to be painted.</li>
                <li>If ->_bIsColHeaderArea is <TRUE/> and ->_bIsRowHeaderArea is <FALSE/>,
                    then ->_rArea denotes the column header area <em>excluding</em> the
                    intersection between row and column header area.</li>
                <li>Equivalently for ->_bIsColHeaderArea being <FALSE/> and ->_bIsRowHeaderArea
                    being <TRUE/></li>
            </ul>
            Note that it's not possible for both ->_bIsColHeaderArea and ->_bIsRowHeaderArea
            to be <FALSE/> at the same time.

            @param _rDevice
                the device to paint onto
            @param _rArea
                the area to paint into
            @param _bIsColHeaderArea
                <TRUE/> if and only if ->_rArea is part of the column header area.
            @param _bIsRowHeaderArea
                <TRUE/> if and only if ->_rArea is part of the row header area.
            @param _rStyle
                the style to be used for drawing
        */
        virtual void    PaintHeaderArea(
                            OutputDevice& _rDevice, const Rectangle& _rArea,
                            bool _bIsColHeaderArea, bool _bIsRowHeaderArea,
                            const StyleSettings& _rStyle ) = 0;

        /** paints the header for a given column

            @param _nCol
                the index of the column to paint
            @param _bActive
                <TRUE/> if and only if the column whose column is to be painted
                contains the active cell.
            @param _bSelected
                <TRUE/> if and only if the column whose column is to be painted
                is selected currently.
            @param _rDevice
                denotes the device to paint onto
            @param _rArea
                the are into which the column header should be painted
            @param _rStyle
                the style to be used for drawing
        */
        virtual void    PaintColumnHeader( ColPos _nCol, bool _bActive, bool _bSelected,
                            OutputDevice& _rDevice, const Rectangle& _rArea,
                            const StyleSettings& _rStyle ) = 0;

        /** prepares a row for painting

            Painting a table means painting rows as necessary, in an increasing
            order. The assumption is that retrieving data for two different rows
            is (potentially) more expensive than retrieving data for two different
            columns. Thus, the renderer will get the chance to "seek" to a certain
            row, and then has to render all cells in this row, before another
            row is going to be painted.

            @param _nRow
                the row which is going to be painted. The renderer should
                at least remember this row, since subsequent calls to
                ->PaintRowHeader(), ->PaintCell(), and ->FinishRow() will
                not pass this parameter again.

                However, the renderer is also allowed to render any
                cell-independent content of this row.

            @param _bActive
                <TRUE/> if and only if the row to be painted contains the
                currently active cell.
            @param _bSelected
                <TRUE/> if and only if the row to be prepared is
                selected currently.
            @param _rDevice
                denotes the device to paint onto
            @param _rRowArea
                the are into which the row should be painted. This excludes
                the row header area, if applicable.
            @param _rStyle
                the style to be used for drawing
        */
        virtual void    PrepareRow( RowPos _nRow, bool _bActive, bool _bSelected,
                            OutputDevice& _rDevice, const Rectangle& _rRowArea,
                            const StyleSettings& _rStyle ) = 0;

        /** paints the header of a row

            The row to be painted is denoted by the most recent call to
            ->PrepareRow.

            @param _bActive
                <TRUE/> if and only if the row to be painted contains the
                currently active cell.
                <br/>
                Note that this flag is equal to the respective flag in the
                previous ->PrepareRow call, it's passed here for convinience
                only.
            @param _bSelected
                <TRUE/> if and only if the row whose header cell is to be
                painted is selected currently.
                <br/>
                Note that this flag is equal to the respective flag in the
                previous ->PrepareRow call, it's passed here for convinience
                only.
            @param _rDevice
                denotes the device to paint onto
            @param _rArea
                the are into which the row header should be painted
            @param _rStyle
                the style to be used for drawing
        */
        virtual void    PaintRowHeader( bool _bActive, bool _bSelected,
                            OutputDevice& _rDevice, const Rectangle& _rArea,
                const StyleSettings& _rStyle ) = 0;

        /** paints a certain cell

            The row to be painted is denoted by the most recent call to
            ->PrepareRow.

            @param _bSelected
                <TRUE/> if and only if the cell to be painted is
                selected currently. This is the case if either
                the row or the column of the cell is currently selected.
                <br/>
                Note that this flag is equal to the respective flag in the
                previous ->PrepareRow call, it's passed here for convinience
                only.
            @param _bActive
                <TRUE/> if the cell is currently active.
                <br/>
                Note that this flag is equal to the respective flag in the
                previous ->PrepareRow call, it's passed here for convinience
                only.
            @param _rDevice
                denotes the device to paint onto
            @param _rArea
                the are into which the cell should be painted
            @param _rStyle
                the style to be used for drawing
        */
        virtual void    PaintCell( ColPos const i_col,
                            bool _bActive, bool _bSelected,
                            OutputDevice& _rDevice, const Rectangle& _rArea,
                            const StyleSettings& _rStyle ) = 0;

        /** draws a cell cursor in the given rectangle

            The cell cursor is used to indicate the active/current cell
            of a table control.
        */
        virtual void    ShowCellCursor( Window& _rView, const Rectangle& _rCursorRect) = 0;

        /** hides the cell cursor previously drawn into the given rectangle

            The cell cursor is used to indicate the active/current cell
            of a table control.
        */
        virtual void    HideCellCursor( Window& _rView, const Rectangle& _rCursorRect) = 0;

        /** checks whether a given cell content fits into a given target area on a given device.

            @param i_colPos
                denotes the column which the cell content would be painted into. Your renderer implementation
                would only need this parameter if rendering is done differently for different columns.

            @param i_rowPos
                denotes the row which the cell content would be painted into. Your renderer implementation
                would only need this parameter if rendering is done differently for different rows.

            @param i_active
                is <TRUE/> if and only if the renderer should assume the cell content would be painted for the active
                cell.

            @param i_selected
                is <TRUE/> if and only if the renderer should assume the cell content would be painted for a selected
                cell.

            @param i_targetDevice
                denotes the target device for the assumed rendering operation

            @param i_targetArea
                denotes the area within the target device for the assumed rendering operation.

            @return
                <TRUE/> if and only if the given cell content could be rendered into the given device and the
                given area.
        */
        virtual bool    FitsIntoCell(
                            ::com::sun::star::uno::Any const & i_cellContent,
                            ColPos const i_colPos, RowPos const i_rowPos,
                            bool const i_active, bool const i_selected,
                            OutputDevice& i_targetDevice, Rectangle const & i_targetArea
                        ) const = 0;

        /** attempts to format the content of the given cell as string

            @param i_cellValue
                the value for which an attempt for a string conversion should be made
            @param  i_colPos
                the column position of the cell in question
            @param  i_rowPos
                the row position of the cell in question
            @param  o_cellString
                the cell content, formatted as string
            @return
                <TRUE/> if and only if the content could be formatted as string
        */
        virtual bool    GetFormattedCellString(
                            ::com::sun::star::uno::Any const & i_cellValue,
                            ColPos const i_colPos, RowPos const i_rowPos,
                            ::rtl::OUString & o_cellString
                        ) const = 0;

        /// deletes the renderer instance
        virtual ~ITableRenderer() { }
    };
    typedef ::boost::shared_ptr< ITableRenderer > PTableRenderer;

//........................................................................
} } // namespace svt::table
//........................................................................

#endif // SVTOOLS_INC_TABLE_TABLERENDERER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
