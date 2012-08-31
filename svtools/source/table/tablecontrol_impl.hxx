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

#ifndef SVTOOLS_TABLECONTROL_IMPL_HXX
#define SVTOOLS_TABLECONTROL_IMPL_HXX

#include "svtools/table/tablemodel.hxx"
#include "svtools/table/tablecontrolinterface.hxx"

#include "svtaccessiblefactory.hxx"

#include <vcl/seleng.hxx>

#include <vector>

#include <boost/scoped_ptr.hpp>

class ScrollBar;
class ScrollBarBox;

//........................................................................
namespace svt { namespace table
{
//........................................................................

    struct MutableColumnMetrics : protected ColumnMetrics
    {
        MutableColumnMetrics()
            :ColumnMetrics()
        {
        }

        MutableColumnMetrics( long const i_startPixel, long const i_endPixel )
            :ColumnMetrics( i_startPixel, i_endPixel )
        {
        }

        long getStart() const { return nStartPixel; }
        long getEnd() const { return nEndPixel; }

        void setEnd( long const i_end ) { nEndPixel = i_end; }
        void move( long const i_offset ) { nStartPixel += i_offset; nEndPixel += i_offset; }

        long getWidth() const { return nEndPixel - nStartPixel; }

        ColumnMetrics const & operator()() { return *this; }
    };

    struct ColumnInfoPositionLess
    {
        bool operator()( MutableColumnMetrics const& i_lhs, MutableColumnMetrics const& i_rhs )
        {
            return i_lhs.getEnd() < i_rhs.getStart();
        }
    };

    typedef ::std::vector< MutableColumnMetrics >    ColumnPositions;

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

        /// denotes whether or not the columns fitted into the available width, last time we checked
        long                    m_bColumnsFit;

        ColPos                  m_nCurColumn;
        RowPos                  m_nCurRow;
        ColPos                  m_nLeftColumn;
        RowPos                  m_nTopRow;

        sal_Int32               m_nCursorHidden;

        /** the window to contain all data content, including header bars

            The window's upper left corner is at position (0,0), relative to the
            table control, which is the direct parent of the data window.
        */
        ::boost::scoped_ptr< TableDataWindow >
                                m_pDataWindow;
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
        bool                    m_bUpdatingColWidths;

        Link                    m_aSelectHdl;

        AccessibleFactoryAccess     m_aFactoryAccess;
        IAccessibleTableControl*    m_pAccessibleTable;

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

        inline  RowPos  getCurRow() const           { return m_nCurRow; }
        inline  void    setCurRow( RowPos i_curRow ){ m_nCurRow = i_curRow; }

        RowPos  getAnchor() const { return m_nAnchor; }
        void    setAnchor( RowPos const i_anchor ) { m_nAnchor = i_anchor; }

        inline  RowPos  getTopRow() const       { return m_nTopRow; }
        inline  ColPos  getLeftColumn() const { return m_nLeftColumn; }

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

        /** ??? */
        void    invalidateSelectedRegion( RowPos _nPrevRow, RowPos _nCurRow );

        /** invalidates the part of the data window which is covered by the given rows
            @param i_firstRow
                the index of the first row to include in the invalidation
            @param i_lastRow
                the index of the last row to include in the invalidation, or ROW_INVALID if the invalidation
                should happen down to the bottom of the data window.
        */
        void    invalidateRowRange( RowPos const i_firstRow, RowPos const i_lastRow );

        /** invalidates the part of the data window which is covered by the given row
        */
        void    invalidateRow( RowPos const i_row ) { invalidateRowRange( i_row, i_row ); }

        /** invalidates all selected rows
        */
        void    invalidateSelectedRows();

        void    checkCursorPosition();

        bool    hasRowSelection() const { return !m_aSelectedRows.empty(); }
        size_t  getSelectedRowCount() const { return m_aSelectedRows.size(); }
        RowPos  getSelectedRowIndex( size_t const i_selectionIndex ) const;

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

        void        setSelectHandler( Link const & i_selectHandler ) { m_aSelectHdl = i_selectHandler; }
        Link const& getSelectHandler() const { return m_aSelectHdl; }

        void commitAccessibleEvent( sal_Int16 const i_eventID, const com::sun::star::uno::Any& i_newValue, const com::sun::star::uno::Any& i_oldValue );
        void commitCellEvent( sal_Int16 const i_eventID, const com::sun::star::uno::Any& i_newValue, const com::sun::star::uno::Any& i_oldValue );
        void commitTableEvent( sal_Int16 const i_eventID, const com::sun::star::uno::Any& i_newValue, const com::sun::star::uno::Any& i_oldValue );

        // ITableControl
        virtual void                hideCursor();
        virtual void                showCursor();
        virtual bool                dispatchAction( TableControlAction _eAction );
        virtual SelectionEngine*    getSelEngine();
        virtual PTableModel         getModel() const;
        virtual ColPos              getCurrentColumn() const;
        virtual RowPos              getCurrentRow() const;
        virtual bool                activateCell( ColPos const i_col, RowPos const i_row );
        virtual ::Size              getTableSizePixel() const;
        virtual void                setPointer( Pointer const & i_pointer );
        virtual void                captureMouse();
        virtual void                releaseMouse();
        virtual void                invalidate( TableArea const i_what );
        virtual long                pixelWidthToAppFont( long const i_pixels ) const;
        virtual void                hideTracking();
        virtual void                showTracking( Rectangle const & i_location, sal_uInt16 const i_flags );
        virtual RowPos              getRowAtPoint( const Point& rPoint ) const;
        virtual ColPos              getColAtPoint( const Point& rPoint ) const;
        virtual TableCell           hitTest( const Point& rPoint ) const;
        virtual ColumnMetrics       getColumnMetrics( ColPos const i_column ) const;
        virtual bool                isRowSelected( RowPos i_row ) const;


        TableDataWindow&        getDataWindow()       { return *m_pDataWindow; }
        const TableDataWindow&  getDataWindow() const { return *m_pDataWindow; }
        ScrollBar* getHorzScrollbar();
        ScrollBar* getVertScrollbar();

        Rectangle calcHeaderRect( bool bColHeader );
        Rectangle calcHeaderCellRect( bool bColHeader, sal_Int32 nPos );
        Rectangle calcTableRect();
        Rectangle calcCellRect( sal_Int32 nRow, sal_Int32 nCol );

        // A11Y
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                        getAccessible( Window& i_parentWindow );
        void            disposeAccessible();

        inline bool     isAccessibleAlive() const { return impl_isAccessibleAlive(); }

        // ITableModelListener
        virtual void    rowsInserted( RowPos first, RowPos last );
        virtual void    rowsRemoved( RowPos first, RowPos last );
        virtual void    columnInserted( ColPos const i_colIndex );
        virtual void    columnRemoved( ColPos const i_colIndex );
        virtual void    allColumnsRemoved();
        virtual void    cellsUpdated( ColPos const i_firstCol, ColPos i_lastCol, RowPos const i_firstRow, RowPos const i_lastRow );
        virtual void    columnChanged( ColPos const i_column, ColumnAttributeGroup const i_attributeGroup );
        virtual void    tableMetricsChanged();

    private:
        bool            impl_isAccessibleAlive() const;
        void            impl_commitAccessibleEvent(
                            sal_Int16 const i_eventID,
                            ::com::sun::star::uno::Any const & i_newValue,
                            ::com::sun::star::uno::Any const & i_oldValue
                        );

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

        /** updates the cached table metrics (row height etc.)
        */
        void        impl_ni_updateCachedTableMetrics();

        /** updates ->m_aColumnWidthsPixel with the current pixel widths of all model columns

            The method is not bound to the classes public invariants, as it's used in
            situations where the they must not necessarily be fullfilled.

            @param i_assumeInflexibleColumnsUpToIncluding
                the index of a column up to which all columns should be considered as inflexible, or
                <code>COL_INVALID</code>.
        */
        void        impl_ni_updateColumnWidths( ColPos const i_assumeInflexibleColumnsUpToIncluding = COL_INVALID );

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
        Rectangle   impl_getAllVisibleCellsArea() const;

        /** retrieves the area occupied by all (at least partially) visible data cells.

            Effectively, the returned area is the same as returned by ->impl_getAllVisibleCellsArea,
            minus the row and column header areas.
        */
        Rectangle   impl_getAllVisibleDataCellArea() const;

        /** retrieves the column which covers the given ordinate
        */
        ColPos      impl_getColumnForOrdinate( long const i_ordinate ) const;

        /** retrieves the row which covers the given abscissa
        */
        RowPos      impl_getRowForAbscissa( long const i_abscissa ) const;

        /// invalidates the window area occupied by the given column
        void        impl_invalidateColumn( ColPos const i_column );

        DECL_LINK( OnScroll, ScrollBar* );
        DECL_LINK( OnUpdateScrollbars, void* );
    };

    //see seleng.hxx, seleng.cxx, FunctionSet overridables, part of selection engine
    class TableFunctionSet : public FunctionSet
    {
    private:
        TableControl_Impl*  m_pTableControl;
        RowPos              m_nCurrentRow;

    public:
        TableFunctionSet(TableControl_Impl* _pTableControl);
        virtual ~TableFunctionSet();

       virtual void BeginDrag();
       virtual void CreateAnchor();
       virtual void DestroyAnchor();
       virtual sal_Bool SetCursorAtPoint(const Point& rPoint, sal_Bool bDontSelectAtCursor);
       virtual sal_Bool IsSelectionAtPoint( const Point& rPoint );
       virtual void DeselectAtPoint( const Point& rPoint );
       virtual void DeselectAll();
    };


//........................................................................
} } // namespace svt::table
//........................................................................

#endif // SVTOOLS_TABLECONTROL_IMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
