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

#ifndef SVTOOLS_INC_TABLE_TABLEMODEL_HXX
#define SVTOOLS_INC_TABLE_TABLEMODEL_HXX

#include "svtools/svtdllapi.h"
#include "svtools/table/tabletypes.hxx"
#include "svtools/table/tablerenderer.hxx"
#include "svtools/table/tableinputhandler.hxx"
#include "svtools/table/tablesort.hxx"

#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/style/HorizontalAlignment.hpp>

#include <rtl/ref.hxx>
#include <sal/types.h>

#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <boost/enable_shared_from_this.hpp>

//........................................................................
namespace svt { namespace table
{
//........................................................................


    //====================================================================
    //= ScrollbarVisibility
    //====================================================================
    enum ScrollbarVisibility
    {
        /** enumeration value denoting that a scrollbar should never be visible, even
            if needed normally
        */
        ScrollbarShowNever,
        /** enumeration value denoting that a scrollbar should be visible when needed only
        */
        ScrollbarShowSmart,
        /** enumeration value denoting that a scrollbar should always be visible, even
            if not needed normally
        */
        ScrollbarShowAlways
    };

    //====================================================================
    //= ITableModelListener
    //====================================================================
    typedef sal_Int32   ColumnAttributeGroup;
    #define COL_ATTRS_NONE          (0x00000000)
    /// denotes column attributes related to the width of the column
    #define COL_ATTRS_WIDTH         (0x00000001)
    /// denotes column attributes related to the appearance of the column, i.e. those relevant for rendering
    #define COL_ATTRS_APPEARANCE    (0x00000002)
    /// denotes the entirety of column attributes
    #define COL_ATTRS_ALL           (0x7FFFFFFF)

    //====================================================================
    //= ITableModelListener
    //====================================================================
    /** declares an interface to be implemented by components interested in
        changes in an ->ITableModel
    */
    class SAL_NO_VTABLE ITableModelListener : public ::boost::enable_shared_from_this< ITableModelListener >
    {
    public:
        /** notifies the listener that one or more rows have been inserted into
            the table

            @param first
                the index of the first newly inserted row
            @param last
                the index of the last newly inserted row. Must not be smaller
                than ->first
        */
        virtual void    rowsInserted( RowPos first, RowPos last ) = 0;

        /** notifies the listener that one or more rows have been removed from
            the table

            @param first
                the old index of the first removed row. If this is <code>-1</code>, then all
                rows have been removed from the model.
            @param last
                the old index of the last removed row. Must not be smaller
                than ->first
        */
        virtual void    rowsRemoved( RowPos first, RowPos last ) = 0;

        /** notifies the listener that one or more columns have been inserted into
            the table

            @param first
                the index of the first newly inserted row
            @param last
                the index of the last newly inserted row. Must not be smaller
                than ->first
        */
        virtual void    columnInserted( ColPos const i_colIndex ) = 0;

        /** notifies the listener that one or more columns have been removed from
            the table

            @param i_colIndex
                the old index of the removed column
        */
        virtual void    columnRemoved( ColPos const i_colIndex ) = 0;

        /** notifies the listener that all columns have been removed form the model
        */
        virtual void    allColumnsRemoved() = 0;

        /** notifies the listener that a rectangular cell range in the table
            has been updated

            Listeners are required to discard any possibly cached information
            they have about the cells in question, in particular any possibly
            cached cell values.
        */
        virtual void    cellsUpdated( ColPos const i_firstCol, ColPos i_lastCol, RowPos const i_firstRow, RowPos const i_lastRow ) = 0;

        /** notifies the listener that attributes of a given column changed

            @param i_column
                the position of the column whose attributes changed
            @param i_attributeGroup
                a combination of one or more <code>COL_ATTRS_*</code> flags, denoting the attribute group(s)
                in which changes occurred.
        */
        virtual void    columnChanged( ColPos const i_column, ColumnAttributeGroup const i_attributeGroup ) = 0;

        /** notifies the listener that the metrics of the table changed.

            Metrics here include the column header height, the row header width, the row height, and the presence
            of both the row and column header.
        */
        virtual void    tableMetricsChanged() = 0;

        /// deletes the listener instance
        virtual ~ITableModelListener(){};
    };
    typedef ::boost::shared_ptr< ITableModelListener > PTableModelListener;

    //====================================================================
    //= IColumnModel
    //====================================================================
    /** interface to be implemented by table column models
    */
    class SAL_NO_VTABLE IColumnModel
    {
    public:
        /** retrieves the ID of the column

            The semantics of a column id is not defined. It's up to the
            implementor of the ->IColumnModel, respectively the ->ITableModel
            which provides the column models, to define such a semantics.

            @return
                the ID of the column. May be 0 if the table which the column
                belongs to does not need and support column ids.

            @see setID
        */
        virtual ::com::sun::star::uno::Any
                            getID() const = 0;

        /** sets a new column ID

            @return
                <TRUE/> if setting the new ID was successful. A possible error
                conditions is if you try to set an ID which is already used
                by another column within the same table.

            @see getID
        */
        virtual void        setID( const ::com::sun::star::uno::Any& _nID ) = 0;

        /** returns the name of the column

            Column names should be human-readable, but not necessarily unique
            within a given table.

            @see setName
        */
        virtual OUString      getName() const = 0;

        /** sets a new name for the column

            @see getName
        */
        virtual void        setName( const OUString& _rName ) = 0;

        /** retrieves the help text to be displayed for the column.
        */
        virtual OUString      getHelpText() const = 0;

        /** sets a new the help text to be displayed for the column.
        */
        virtual void        setHelpText( const OUString& i_helpText ) = 0;

        /** determines whether the column can be interactively resized

            @see getMinWidth
            @see getMaxWidth
            @see getWidth
        */
        virtual bool        isResizable() const = 0;

        /** declares the column as resizable or fixed in width

            @see getMinWidth
            @see getMaxWidth
            @see getWidth
        */
        virtual void        setResizable( bool _bResizable ) = 0;

        /** denotes the relative flexibility of the column

            This flexibility is taken into account when a table control auto-resizes its columns, because the available
            space changed. In this case, the columns grow or shrink according to their flexibility.

            A value of 0 means the column is not auto-resized at all.
        */
        virtual sal_Int32   getFlexibility() const = 0;

        /** sets a new flexibility value for the column

            @see getFlexibility
        */
        virtual void        setFlexibility( sal_Int32 const i_flexibility ) = 0;

        /** returns the width of the column, in app-font unitss

            The returned value must be a positive ->TableMetrics value.

            @see setWidth
            @see getMinWidth
            @see getMaxWidth
        */
        virtual TableMetrics    getWidth() const = 0;

        /** sets a new width for the column

            @param _nWidth
                the new width, app-font units

            @see getWidth
        */
        virtual void            setWidth( TableMetrics _nWidth ) = 0;

        /** returns the minimum width of the column, in app-font units, or 0 if the column
            does not have a minimal width

            @see setMinWidth
            @see getMaxWidth
            @see getWidth
        */
        virtual TableMetrics    getMinWidth() const = 0;

        /** sets the minimum width of the column, in app-font units

            @see getMinWidth
            @see setMaxWidth
            @see setWidth
        */
        virtual void            setMinWidth( TableMetrics _nMinWidth ) = 0;

        /** returns the maximum width of the column, in app-font units, or 0 if the column
            does not have a minimal width

            @see setMaxWidth
            @see getMinWidth
            @see getWidth
        */
        virtual TableMetrics    getMaxWidth() const = 0;

        /** sets the maximum width of the column, in app-font units

            @see getMaxWidth
            @see setMinWidth
            @see setWidth
        */
        virtual void            setMaxWidth( TableMetrics _nMaxWidth ) = 0;

        /** retrieves the horizontal alignment to be used for content in this cell
        */
        virtual ::com::sun::star::style::HorizontalAlignment getHorizontalAlign() = 0;

        /** sets a new the horizontal alignment to be used for content in this cell
        */
        virtual void setHorizontalAlign(::com::sun::star::style::HorizontalAlignment _xAlign) = 0;

        /// deletes the column model instance
        virtual ~IColumnModel() { }
    };
    typedef ::boost::shared_ptr< IColumnModel > PColumnModel;

    //====================================================================
    //= ITableModel
    //====================================================================
    /** declares the interface to implement by an abtract table model
    */
    class SAL_NO_VTABLE SVT_DLLPUBLIC ITableModel
    {
    public:
        /** returns the number of columns in the table
        */
        virtual TableSize   getColumnCount() const = 0;

        /** returns the number of rows in the table
        */
        virtual TableSize   getRowCount() const = 0;

        /** determines whether the table has column headers

            If this method returns <TRUE/>, the renderer returned by
            ->getRenderer must be able to render column headers.

            @see IColumnRenderer
        */
        virtual bool        hasColumnHeaders() const = 0;

        /** determines whether the table has row headers

            If this method returns <TRUE/>, the renderer returned by
            ->getRenderer must be able to render row headers.

            @see IColumnRenderer
        */
        virtual bool        hasRowHeaders() const = 0;

        /** determines whether the given cell is editable

            @see ICellEditor
            @todo
        */
        virtual bool        isCellEditable( ColPos col, RowPos row ) const = 0;

        /** returns a model for a certain column

            @param column
                the index of the column in question. Must be greater than or
                equal 0, and smaller than the return value of ->getColumnCount()

            @return
                the model of the column in question. Must not be <NULL/>
        */
        virtual PColumnModel    getColumnModel( ColPos column ) = 0;

        /** returns a renderer which is able to paint the table represented
            by this table model

            @return the renderer to use. Must not be <NULL/>
        */
        virtual PTableRenderer  getRenderer() const = 0;

        /** returns the component handling input in a view associated with the model
        */
        virtual PTableInputHandler  getInputHandler() const = 0;

        /** determines the height of rows in the table.

            @return
                the logical height of rows in the table, in app-font units. The height must be
                greater 0.
        */
        virtual TableMetrics    getRowHeight() const = 0;

        /** determines the height of the column header row

            This method is not to be called if ->hasColumnHeaders()
            returned <FALSE/>.

            @return
                the logical height of the column header row, in app-font units.
                Must be greater than 0.
        */
        virtual TableMetrics    getColumnHeaderHeight() const = 0;

        /** determines the width of the row header column

            This method is not to be called if ->hasRowHeaders()
            returned <FALSE/>.

            @return
                the logical width of the row header column, in app-font units.
                Must be greater than 0.
        */
        virtual TableMetrics    getRowHeaderWidth() const = 0;

        /** returns the visibilit mode of the vertical scrollbar
        */
        virtual ScrollbarVisibility getVerticalScrollbarVisibility() const = 0;

        /** returns the visibilit mode of the horizontal scrollbar
        */
        virtual ScrollbarVisibility getHorizontalScrollbarVisibility() const = 0;

        /** adds a listener to be notified of changes in the table model
        */
        virtual void addTableModelListener( const PTableModelListener& i_listener ) = 0;

        /** remove a listener to be notified of changes in the table model
        */
        virtual void removeTableModelListener( const PTableModelListener& i_listener ) = 0;

        /** retrieves the content of the given cell
        */
        virtual void getCellContent( ColPos const i_col, RowPos const i_row, ::com::sun::star::uno::Any& o_cellContent ) = 0;

        /** returns an object which should be displayed as tooltip for the given cell

            At the moment, only string-convertible values are supported here. In the future, one might imagine displaying
            scaled-down versions of a graphic in a cell, and a larger version of that very graphic as tooltip.

            If no tooltip object is provided, then the cell content is used, and displayed as tooltip for the cell
            if and only if it doesn't fit into the cell's space itself.

            @param i_col
                The column index of the cell in question. COL_ROW_HEADERS is a valid argument here.
            @param i_row
                The row index of the cell in question.
            @param o_cellToolTip
                takes the tooltip object upon return.
        */
        virtual void getCellToolTip( ColPos const i_col, RowPos const i_row, ::com::sun::star::uno::Any & o_cellToolTip ) = 0;

        /** retrieves title of a given row
        */
        virtual ::com::sun::star::uno::Any      getRowHeading( RowPos const i_rowPos ) const = 0;

        /** returns the color to be used for rendering the grid lines.

            If this value is not set, a default color from the style settings will be used.
        */
        virtual ::boost::optional< ::Color >    getLineColor() const = 0;

        /** returns the color to be used for rendering the header background.

            If this value is not set, a default color from the style settings will be used.
        */
        virtual ::boost::optional< ::Color >    getHeaderBackgroundColor() const = 0;

        /** returns the color to be used for rendering the header text.

            If this value is not set, a default color from the style settings will be used.
        */
        virtual ::boost::optional< ::Color >    getHeaderTextColor() const = 0;

        /** returns the color to be used for the background of selected cells, when the control has the focus

            If this value is not set, a default color from the style settings will be used.
        */
        virtual ::boost::optional< ::Color >    getActiveSelectionBackColor() const = 0;

        /** returns the color to be used for the background of selected cells, when the control does not have the focus

            If this value is not set, a default color from the style settings will be used.
        */
        virtual ::boost::optional< ::Color >    getInactiveSelectionBackColor() const = 0;

        /** returns the color to be used for the text of selected cells, when the control has the focus

            If this value is not set, a default color from the style settings will be used.
        */
        virtual ::boost::optional< ::Color >    getActiveSelectionTextColor() const = 0;

        /** returns the color to be used for the text of selected cells, when the control does not have the focus

            If this value is not set, a default color from the style settings will be used.
        */
        virtual ::boost::optional< ::Color >    getInactiveSelectionTextColor() const = 0;

        /** returns the color to be used for rendering cell texts.

            If this value is not set, a default color from the style settings will be used.
        */
        virtual ::boost::optional< ::Color >    getTextColor() const = 0;

        /** returns the color to be used for text lines (underline, strikethrough) when rendering cell text.

            If this value is not set, a default color from the style settings will be used.
        */
        virtual ::boost::optional< ::Color >    getTextLineColor() const = 0;

        /** returns the colors to be used for the row backgrounds.

            If this value is not set, every second row will have a background color derived from the style settings's
            selection color, the other rows will not have a special background at all.

            If this value is an empty sequence, the rows will not have a special background at all, instead the
            normal background of the complete control will be used.

            If value is a non-empty sequence, then rows will have the background colors as specified in the sequence,
            in alternating order.
        */
        virtual ::boost::optional< ::std::vector< ::Color > >
                                                getRowBackgroundColors() const = 0;

        /** determines the vertical alignment of content within a cell
        */
        virtual ::com::sun::star::style::VerticalAlignment getVerticalAlign() const = 0;

        /** returns an adapter to the sorting functionality of the model

            It is legitimate to return <NULL/> here, in this case, the table model does not support sorting.
        */
        virtual ITableDataSort* getSortAdapter() = 0;

        /// destroys the table model instance
        virtual ~ITableModel() { }
    };
    typedef ::boost::shared_ptr< ITableModel > PTableModel;

//........................................................................
} } // namespace svt::table
//........................................................................

#endif // SVTOOLS_INC_TABLE_TABLEMODEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
