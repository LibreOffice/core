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

#ifndef SVTOOLS_TABLECONTROL_IMPL_HXX
#define SVTOOLS_TABLECONTROL_IMPL_HXX

#include <svtools/table/tablemodel.hxx>
#include <svtools/table/tablecontrolinterface.hxx>
#include <svtools/table/tablemodel.hxx>

#include <vcl/seleng.hxx>

#include <vector>

class ScrollBar;
class ScrollBarBox;

//........................................................................
namespace svt { namespace table
{
//........................................................................

    struct ColumnWidthInfo
    {
        ColumnWidthInfo()
            :nStartPixel( 0 )
            ,nEndPixel( 0 )
        {
        }

        ColumnWidthInfo( long const i_startPixel, long const i_endPixel )
            :nStartPixel( i_startPixel )
            ,nEndPixel( i_endPixel )
        {
        }

        long getStart() const { return nStartPixel; }
        long getEnd() const { return nEndPixel; }

        void setEnd( long const i_end ) { nEndPixel = i_end; }
        void move( long const i_offset ) { nStartPixel += i_offset; nEndPixel += i_offset; }

        long getWidth() const { return nEndPixel - nStartPixel; }

    private:
        /** the start of the column, in pixels. Might be negative, in case the column is scrolled out of the visible
            area.
        */
        long    nStartPixel;

        /** the end of the column, in pixels, plus 1. Effectively, this is the accumulated width of a all columns
            up to the current one.
        */
        long    nEndPixel;
    };

    struct ColumnInfoPositionLess
    {
        bool operator()( ColumnWidthInfo const& i_colInfo, long const i_position )
        {
            return i_colInfo.getEnd() < i_position;
        }
        bool operator()( long const i_position, ColumnWidthInfo const& i_colInfo )
        {
            return i_position < i_colInfo.getStart();
        }
    };

    typedef ::std::vector< ColumnWidthInfo >    ColumnPositions;

    class TableControl;
    class TableDataWindow;
    class TableFunctionSet;

    //====================================================================
    //= TableControl_Impl
    //====================================================================
    class TableControl_Impl :public ITableControl
                            ,public ITableModelListener
    {
        friend class TableGeometry;
        friend class TableRowGeometry;
        friend class TableColumnGeometry;
        friend class SuspendInvariants;
        friend class TableFunctionSet;

    private:
        /// the control whose impl-instance we implemnt
        TableControl&           m_rAntiImpl;
        /// the model of the table control
        PTableModel             m_pModel;
        /// the input handler to use, usually the input handler as provided by ->m_pModel
        PTableInputHandler      m_pInputHandler;
        /// info about the widths of our columns
        ColumnPositions         m_aColumnWidths;

        /// the height of a single row in the table, measured in pixels
        long                    m_nRowHeightPixel;
        /// the height of the column header row in the table, measured in pixels
        long                    m_nColHeaderHeightPixel;
        /// the width of the row header column in the table, measured in pixels
        long                    m_nRowHeaderWidthPixel;

        /// the number of columns in the table control. Cached model value.
        TableSize               m_nColumnCount;
        /// the number of rows in the table control. Cached model value.
        TableSize               m_nRowCount;

        ColPos                  m_nCurColumn;
        RowPos                  m_nCurRow;
        ColPos                  m_nLeftColumn;
        RowPos                  m_nTopRow;

        sal_Int32               m_nCursorHidden;

        /** the window to contain all data content, including header bars

            The window's upper left corner is at position (0,0), relative to the
            table control, which is the direct parent of the data window.
        */
        TableDataWindow*        m_pDataWindow;
        /// the vertical scrollbar, if any
        ScrollBar*              m_pVScroll;
        /// the horizontal scrollbar, if any
        ScrollBar*              m_pHScroll;
        ScrollBarBox*           m_pScrollCorner;
        //selection engine - for determining selection range, e.g. single, multiple
        SelectionEngine*        m_pSelEngine;
        //vector which contains the selected rows
        std::vector<RowPos>     m_aSelectedRows;
        //part of selection engine
        TableFunctionSet*       m_pTableFunctionSet;
        //part of selection engine
        RowPos                  m_nAnchor;
        bool                    m_bResizingColumn;
        ColPos                  m_nResizingColumn;
        bool                    m_bResizingGrid;
        bool                    m_bUpdatingColWidths;

#if DBG_UTIL
    #define INV_SCROLL_POSITION     1
        /** represents a bitmask of invariants to check

            Actually, impl_checkInvariants checks more invariants than denoted in this
            bit mask, but only those present here can be disabled temporarily.
        */
        sal_Int32           m_nRequiredInvariants;
#endif

    public:


        void        setModel( PTableModel _pModel );

        inline  const PTableInputHandler&   getInputHandler() const { return m_pInputHandler; }

        inline  void    setCurRow(RowPos curRow){ m_nCurRow = curRow; }
        inline  RowPos  getTopRow() const       { return m_nTopRow; }

        inline  const TableControl&   getAntiImpl() const { return m_rAntiImpl; }
        inline        TableControl&   getAntiImpl()       { return m_rAntiImpl; }

    public:
        TableControl_Impl( TableControl& _rAntiImpl );
        ~TableControl_Impl();

#if DBG_UTIL
        const sal_Char* impl_checkInvariants() const;
#endif
        /** to be called when the anti-impl instance has been resized
        */
        void    onResize();

        /** paints the table control content which intersects with the given rectangle
        */
        void    doPaintContent( const Rectangle& _rUpdateRect );

        /** moves the cursor to the cell with the given coordinates

            To ease the caller's code, the coordinates must not necessarily denote a
            valid position. If they don't, <FALSE/> will be returned.
        */
        bool    goTo( ColPos _nColumn, RowPos _nRow );

        /** ensures that the given coordinate is visible
            @param _nColumn
                the column position which should be visible. Must be non-negative, and smaller
                than the column count.
            @param _nRow
                the row position which should be visibleMust be non-negative, and smaller
                than the row count.
            @param _bAcceptPartialVisibility
                <TRUE/> if it's okay that the given cooordinate is only partially visible
        */
        void    ensureVisible( ColPos _nColumn, RowPos _nRow, bool _bAcceptPartialVisibility );

        /** retrieves the content of the given cell, converted to a string
        */
        ::rtl::OUString getCellContentAsString( RowPos const i_row, ColPos const i_col );

        /** returns the position of the current row in the selection vector */
        int getRowSelectedNumber(const ::std::vector<RowPos>& selectedRows, RowPos current);

        /** _rCellRect contains the region, which should be invalidate after some action e.g. selecting row*/
        void    invalidateSelectedRegion(RowPos _nPrevRow, RowPos _nCurRow, Rectangle& _rCellRect );

        /** to be called when a new row is added to the control*/
        void    invalidateRow(RowPos _nRowPos, Rectangle& _rCellRect );

        /** returns the vector, which contains the selected rows*/
        std::vector<RowPos>& getSelectedRows();

        /** updates the vector, which contains the selected rows after removing the row nRowPos*/
        void    removeSelectedRow(RowPos _nRowPos);
        void    invalidateRows();

        bool    hasRowSelection() const { return !m_aSelectedRows.empty(); }
        size_t  getSelectedRowCount() const { return m_aSelectedRows.size(); }

        /** removes the given row index from m_aSelectedRows

            @return
                <TRUE/> if and only if the row was previously marked as selected
        */
        bool        markRowAsDeselected( RowPos const i_rowIndex );

        /** marks the given row as selectged, by putting it into m_aSelectedRows
            @return
                <TRUE/> if and only if the row was previously <em>not</em> marked as selected
        */
        bool        markRowAsSelected( RowPos const i_rowIndex );

        /** marks all rows as deselected
            @return
                <TRUE/> if and only if the selection actually changed by this operation
        */
        bool        markAllRowsAsDeselected();

        /** marks all rows as selected
            @return
                <FALSE/> if and only if all rows were selected already.
        */
        bool        markAllRowsAsSelected();


        // ITableControl
        virtual void                hideCursor();
        virtual void                showCursor();
        virtual bool                dispatchAction( TableControlAction _eAction );
        virtual SelectionEngine*    getSelEngine();
        virtual PTableModel         getModel() const;
        virtual ColPos              getCurrentColumn() const;
        virtual RowPos              getCurrentRow() const;
        virtual void                activateCellAt( const Point& rPoint );
        virtual RowPos              getRowAtPoint( const Point& rPoint ) const;
        virtual ColPos              getColAtPoint( const Point& rPoint ) const;
        virtual void                resizeColumn(const Point& rPoint);
        virtual bool                checkResizeColumn(const Point& rPoint);
        virtual bool                endResizeColumn(const Point& rPoint);
        virtual bool                isRowSelected( RowPos i_row ) const;


        TableDataWindow* getDataWindow();
        ScrollBar* getHorzScrollbar();
        ScrollBar* getVertScrollbar();

        Rectangle calcHeaderRect(bool bColHeader);
        Rectangle calcTableRect();

        // ITableModelListener
        virtual void    rowsInserted( RowPos first, RowPos last );
        virtual void    rowsRemoved( RowPos first, RowPos last );
        virtual void    columnInserted( ColPos const i_colIndex );
        virtual void    columnRemoved( ColPos const i_colIndex );
        virtual void    allColumnsRemoved();
        virtual void    cellsUpdated( ColPos firstCol, ColPos lastCol, RowPos firstRow, RowPos lastRow );
        virtual void    columnChanged( ColPos const i_column, ColumnAttributeGroup const i_attributeGroup );

    private:
        /** toggles the cursor visibility

            The method is not bound to the classes public invariants, as it's used in
            situations where the they must not necessarily be fullfilled.
        */
        void        impl_ni_doSwitchCursor( bool _bOn );

        /** returns the number of visible rows.

            @param _bAcceptPartialRow
                specifies whether a possible only partially visible last row is
                counted, too.
        */
        TableSize   impl_getVisibleRows( bool _bAcceptPartialRow ) const;

        /** returns the number of visible columns

            The value may change with different horizontal scroll positions, as
            different columns have different widths. For instance, if your control is
            100 pixels wide, and has three columns of width 50, 50, 100, respectively,
            then this method will return either "2" or "1", depending on which column
            is the first visible one.

            @param _bAcceptPartialRow
                specifies whether a possible only partially visible last row is
                counted, too.
        */
        TableSize   impl_getVisibleColumns( bool _bAcceptPartialCol ) const;

        /** determines the rectangle occupied by the given cell
        */
        void        impl_getCellRect( ColPos _nColumn, RowPos _nRow, Rectangle& _rCellRect ) const;

        /** updates all cached model values

            The method is not bound to the classes public invariants, as it's used in
            situations where the they must not necessarily be fullfilled.
        */
        void        impl_ni_updateCachedModelValues();

        /** updates ->m_aColumnWidthsPixel with the current pixel widths of all model columns

            The method takes into account the current zoom factor and map mode of the table
            control, plus any possible COLWIDTH_FIT_TO_VIEW widths in the model columns.

            The method is not bound to the classes public invariants, as it's used in
            situations where the they must not necessarily be fullfilled.
        */
        void        impl_ni_updateColumnWidths();

        /** updates the scrollbars of the control

            The method is not bound to the classes public invariants, as it's used in
            situations where the they must not necessarily be fullfilled.

            This includes both the existence of the scrollbars, and their
            state.
        */
        void        impl_ni_updateScrollbars();

        /** scrolls the view by the given number of rows

            The method is not bound to the classes public invariants, as it's used in
            situations where the they must not necessarily be fullfilled.

            @return
                the number of rows by which the viewport was scrolled. This may differ
                from the given numbers to scroll in case the begin or the end of the
                row range were reached.
        */
        TableSize   impl_ni_ScrollRows( TableSize _nRowDelta );

        /** equivalent to impl_ni_ScrollRows, but checks the instances invariants beforehand (in a non-product build only)
        */
        TableSize   impl_scrollRows( TableSize const i_rowDelta );

        /** scrolls the view by the given number of columns

            The method is not bound to the classes public invariants, as it's used in
            situations where the they must not necessarily be fullfilled.

            @return
                the number of columns by which the viewport was scrolled. This may differ
                from the given numbers to scroll in case the begin or the end of the
                column range were reached.
        */
        TableSize   impl_ni_ScrollColumns( TableSize _nColumnDelta );

        /** equivalent to impl_ni_ScrollColumns, but checks the instances invariants beforehand (in a non-product build only)
        */
        TableSize   impl_scrollColumns( TableSize const i_columnDelta );

        /** retrieves the area occupied by the totality of (at least partially) visible cells

            The returned area includes row and column headers. Also, it takes into
            account the the fact that there might be less columns than would normally
            find room in the control.

            As a result of respecting the partial visibility of rows and columns,
            the returned area might be larger than the data window's output size.
        */
        void        impl_getAllVisibleCellsArea( Rectangle& _rCellArea ) const;

        /** retrieves the area occupied by all (at least partially) visible data cells.

            Effectively, the returned area is the same as returned by ->impl_getAllVisibleCellsArea,
            minus the row and column header areas.
        */
        void        impl_getAllVisibleDataCellArea( Rectangle& _rCellArea ) const;

        /** retrieves the column which covers the given ordinate
        */
        ColPos      impl_getColumnForOrdinate( long const i_ordinate ) const;

        /// invalidates the window area occupied by the given column
        void        impl_invalidateColumn( ColPos const i_column );

        DECL_LINK( OnScroll, ScrollBar* );
        DECL_LINK( OnUpdateScrollbars, void* );
    };

    //see seleng.hxx, seleng.cxx, FunctionSet overridables, part of selection engine
    class TableFunctionSet : public FunctionSet
    {
        friend class TableDataWindow;
    private:
        TableControl_Impl* m_pTableControl;
        RowPos m_nCurrentRow;
    public:
        TableFunctionSet(TableControl_Impl* _pTableControl);
        virtual ~TableFunctionSet();

       virtual void BeginDrag();
       virtual void CreateAnchor();
       virtual void DestroyAnchor();
       virtual BOOL SetCursorAtPoint(const Point& rPoint, BOOL bDontSelectAtCursor);
       virtual BOOL IsSelectionAtPoint( const Point& rPoint );
       virtual void DeselectAtPoint( const Point& rPoint );
       virtual void DeselectAll();
    };


//........................................................................
} } // namespace svt::table
//........................................................................

#endif // SVTOOLS_TABLECONTROL_IMPL_HXX
