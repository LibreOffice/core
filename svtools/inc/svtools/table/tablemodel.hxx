/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
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
************************************************************************/

#ifndef SVTOOLS_INC_TABLE_TABLEMODEL_HXX
#define SVTOOLS_INC_TABLE_TABLEMODEL_HXX

#include <svtools/table/tabletypes.hxx>
#include <svtools/table/tablerenderer.hxx>
#include <svtools/table/tableinputhandler.hxx>

#include <sal/types.h>

#include <boost/shared_ptr.hpp>

//........................................................................
namespace svt { namespace table
{
//........................................................................


    //====================================================================
    //= cell data
    //====================================================================
    struct CellEntryType
    {
         String m_aStr;
         //Image        m_aImage;
        //Control   m_aControl;
        CellEntryType( const String& _rStr ) :
            m_aStr( _rStr )
            {}
     };

    //typedef ::std::vector<CellEntryType*> CellColumnContent;
    //vector, which contains text data for each cell
    typedef ::std::vector<String> CellColumnContent;
    //vector, which contains data for rows
    typedef ::std::vector<CellColumnContent> CellContent;
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
    /** declares an interface to be implemented by components interested in
        changes in an ->ITableModel
    */
    class SAL_NO_VTABLE ITableModelListener
    {
    public:
        //virtual void  onTableModelChanged(PTableModel pTableModel) = 0;
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
                the old index of the first removed row
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
        virtual void    columnsInserted( ColPos first, ColPos last ) = 0;

        /** notifies the listener that one or more columns have been removed from
            the table

            @param first
                the old index of the first removed row
            @param last
                the old index of the last removed row. Must not be smaller
                than ->first
        */
        virtual void    columnsRemoved( ColPos first, ColPos last ) = 0;

        /** notifies the listener that a column in the table has moved

            @param oldIndex
                the old index of the column within the model
            @param newIndex
                the new index of the column within the model
        */
        virtual void    columnMoved( ColPos oldIndex, ColPos newIndex ) = 0;

        /** notifies the listener that a rectangular cell range in the table
            has been updated

            Listeners are required to discard any possibly cached information
            they have about the cells in question, in particular any possibly
            cached cell values.
        */
        virtual void    cellsUpdated( ColPos firstCol, ColPos lastCol, RowPos firstRow, RowPos lastRow ) = 0;

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
        virtual ColumnID    getID() const = 0;

        /** sets a new column ID

            @return
                <TRUE/> if setting the new ID was successfull. A possible error
                conditions is if you try to set an ID which is already used
                by another column within the same table.

            @see getID
        */
        virtual bool        setID( const ColumnID _nID ) = 0;

        /** returns the name of the column

            Column names should be human-readable, but not necessarily unique
            within a given table.

            @see setName
        */
        virtual String      getName() const = 0;

        /** sets a new name for the column

            @see getName
        */
        virtual void        setName( const String& _rName ) = 0;

        /** determines whether the column can be resized

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

        /** returns the width of the column, in 1/100 millimeters

            The returned value must be a positive ->TableMetrics value.

            It can also be COLWIDTH_FIT_TO_VIEW, to indicate that the width of the column
            should automatically be adjusted to completely fit the view. For instance, a
            model's last column could return this value, to indicate that it is to occupy
            all horizontal space remaining in the view, after all other columns have been
            layouted.

            If there is more than one column with width COLWIDTH_FIT_TO_VIEW in a model,
            they're all layouted equal-width.

            If the columns with a read width (i.e. other than COLWIDTH_FIT_TO_VIEW) are,
            in sum, wider than the view, then the view is free to choose a real width for any
            columns which return COLWIDTH_FIT_TO_VIEW here.

            @see setWidth
            @see getMinWidth
            @see getMaxWidth
            @see COLWIDTH_FIT_TO_VIEW
        */
        virtual TableMetrics    getWidth() const = 0;

        /** sets a new width for the column

            @param _nWidth
                the new width, in 1/100 millimeters

            @see getWidth
        */
        virtual void            setWidth( TableMetrics _nWidth ) = 0;

        /** returns the minimum width of the column, in 1/100 millimeters, or 0 if the column
            does not have a minimal width

            @see setMinWidth
            @see getMaxWidth
            @see getWidth
        */
        virtual TableMetrics    getMinWidth() const = 0;

        /** sets the minimum width of the column, in 1/100 millimeters

            @see getMinWidth
            @see setMaxWidth
            @see setWidth
        */
        virtual void            setMinWidth( TableMetrics _nMinWidth ) = 0;

        /** returns the maximum width of the column, in 1/100 millimeters, or 0 if the column
            does not have a minimal width

            @see setMaxWidth
            @see getMinWidth
            @see getWidth
        */
        virtual TableMetrics    getMaxWidth() const = 0;

        /** sets the maximum width of the column, in 1/100 millimeters

            @see getMaxWidth
            @see setMinWidth
            @see setWidth
        */
        virtual void            setMaxWidth( TableMetrics _nMaxWidth ) = 0;


        /// deletes the column model instance
        virtual ~IColumnModel() { }
    };
    typedef ::boost::shared_ptr< IColumnModel > PColumnModel;

    //====================================================================
    //= ITableModel
    //====================================================================
    /** declares the interface to implement by an abtract table model
    */
    class SAL_NO_VTABLE ITableModel
    {
    public:
        /** returns the number of columns in the table
        */
        virtual TableSize   getColumnCount() const = 0;

        /** returns the number of rows in the table
        */
        virtual TableSize   getRowCount() const = 0;

        virtual void        setColumnCount(TableSize _nColCount) = 0;
        virtual void        setRowCount(TableSize _nRowCount) = 0;

        /** determines whether the table has column headers

            If this method returns <TRUE/>, the renderer returned by
            ->getRenderer must be able to render column headers.

            @see IColumnRenderer
        */
        virtual bool        hasColumnHeaders() const = 0;
        /** sets whether the table should have row headers
            @see IColumnRenderer
        */
        virtual void        setRowHeaders( bool rowHeaders) = 0;

        /** sets whether the table should have column headers
            @see IColumnRenderer
        */
        virtual void        setColumnHeaders( bool columnHeaders) = 0;

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

        /** adds the given listener to the list of ->ITableModelListener's
        */
        virtual void        addTableModelListener( const PTableModelListener& listener ) = 0;

        /** revokes the given listener from the list of ->ITableModelListener's
        */
        virtual void        removeTableModelListener( const PTableModelListener& listener ) = 0;

        /** returns a model for a certain column

            @param column
                the index of the column in question. Must be greater than or
                equal 0, and smaller than the return value of ->getColumnCount()

            @return
                the model of the column in question. Must not be <NULL/>

            @see getColumnModelByID
        */
        virtual PColumnModel    getColumnModel( ColPos column ) = 0;

        /** finds a column model by ID

            @param id
                the id of the column which is to be looked up
            @return
                the column model with the given ID, or <NULL/> if there is
                no such column
        */
        virtual PColumnModel    getColumnModelByID( ColumnID id ) = 0;

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
                the logical height of rows in the table, in 1/100 millimeters. The height must be
                greater 0.
        */
        virtual TableMetrics    getRowHeight() const = 0;

        virtual void            setRowHeight(TableMetrics _nRowHeight) = 0;

        /** determines the height of the column header row

            This method is not to be called if ->hasColumnHeaders()
            returned <FALSE/>.

            @return
                the logical height of the column header row, in 1/100 millimeters.
                Must be greater than 0.
        */
        virtual TableMetrics    getColumnHeaderHeight() const = 0;

        /** determines the width of the row header column

            This method is not to be called if ->hasRowHeaders()
            returned <FALSE/>.

            @return
                the logical width of the row header column, in 1/100 millimeters.
                Must be greater than 0.
        */
        virtual TableMetrics    getRowHeaderWidth() const = 0;

        /** determines the visibility of the vertical scrollbar of the table control
            @param overAllHeight the height of the table with all rows
            @param actHeight the given height of the table
        */
        virtual ScrollbarVisibility getVerticalScrollbarVisibility(int overAllHeight,int actHeight) const = 0;

        /** determines the visibility of the horizontal scrollbar of the table control
            @param overAllWidth the width of the table with all columns
            @param actWidth the given width of the table
        */
        virtual ScrollbarVisibility getHorizontalScrollbarVisibility(int overAllWidth, int actWidth) const = 0;

        /** fills cells with content
        */
        virtual void                setCellContent(std::vector<std::vector<rtl::OUString> > cellContent)=0;
        /** gets the content of the cells
        */
        virtual std::vector<std::vector<rtl::OUString> >&           getCellContent() = 0;
        /**sets title of header rows
        */
        virtual void                setRowHeaderName(std::vector<rtl::OUString> cellColumnContent)=0;
        /** gets title of header rows
        */
        virtual std::vector<rtl::OUString>&   getRowHeaderName() = 0;

        /// destroys the table model instance
        virtual ~ITableModel() { }
    };
    typedef ::boost::shared_ptr< ITableModel > PTableModel;

//........................................................................
} } // namespace svt::table
//........................................................................

#endif // SVTOOLS_INC_TABLE_TABLEMODEL_HXX
