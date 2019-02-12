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

#ifndef INCLUDED_SVTOOLS_SOURCE_TABLE_TABLECONTROL_IMPL_HXX
#define INCLUDED_SVTOOLS_SOURCE_TABLE_TABLECONTROL_IMPL_HXX

#include <svtools/table/tablemodel.hxx>
#include <table/tablecontrolinterface.hxx>

#include <vcl/svtaccessiblefactory.hxx>
#include <vcl/accessiblefactory.hxx>

#include <vcl/seleng.hxx>

#include <vector>

class ScrollBar;
class ScrollBarBox;

namespace svt { namespace table
{
    struct MutableColumnMetrics : public ColumnMetrics
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

        void move( long const i_offset ) { nStartPixel += i_offset; nEndPixel += i_offset; }

        long getWidth() const { return nEndPixel - nStartPixel; }
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


    //= TableControl_Impl

    class TableControl_Impl :public ITableControl
                            ,public ITableModelListener
    {
        friend class TableGeometry;
        friend class TableRowGeometry;
        friend class TableColumnGeometry;
        friend class SuspendInvariants;

    private:
        /// the control whose impl-instance we implement
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
        VclPtr<TableDataWindow> m_pDataWindow;
        /// the vertical scrollbar, if any
        VclPtr<ScrollBar>       m_pVScroll;
        /// the horizontal scrollbar, if any
        VclPtr<ScrollBar>       m_pHScroll;
        VclPtr<ScrollBarBox>    m_pScrollCorner;
        //selection engine - for determining selection range, e.g. single, multiple
        std::unique_ptr<SelectionEngine> m_pSelEngine;
        //vector which contains the selected rows
        std::vector<RowPos>     m_aSelectedRows;
        //part of selection engine
        std::unique_ptr<TableFunctionSet> m_pTableFunctionSet;
        //part of selection engine
        RowPos                  m_nAnchor;
        bool                    m_bUpdatingColWidths;

        vcl::AccessibleFactoryAccess     m_aFactoryAccess;
        vcl::table::IAccessibleTableControl*    m_pAccessibleTable;

    public:
        void        setModel( const PTableModel& _pModel );

        const PTableInputHandler&   getInputHandler() const { return m_pInputHandler; }

        RowPos  getCurRow() const           { return m_nCurRow; }

        RowPos  getAnchor() const { return m_nAnchor; }
        void    setAnchor( RowPos const i_anchor ) { m_nAnchor = i_anchor; }

        RowPos  getTopRow() const       { return m_nTopRow; }
        ColPos  getLeftColumn() const { return m_nLeftColumn; }

        const TableControl&   getAntiImpl() const { return m_rAntiImpl; }
        TableControl&   getAntiImpl()       { return m_rAntiImpl; }

    public:
        explicit TableControl_Impl( TableControl& _rAntiImpl );
        virtual ~TableControl_Impl() override;

        /** to be called when the anti-impl instance has been resized
        */
        void    onResize();

        /** paints the table control content which intersects with the given rectangle
        */
        void    doPaintContent(vcl::RenderContext& rRenderContext, const tools::Rectangle& _rUpdateRect);

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
        */
        void    ensureVisible( ColPos _nColumn, RowPos _nRow );

        /** retrieves the content of the given cell, converted to a string
        */
        OUString getCellContentAsString( RowPos const i_row, ColPos const i_col );

        /** returns the position of the current row in the selection vector */
        static int getRowSelectedNumber(const ::std::vector<RowPos>& selectedRows, RowPos current);

        void invalidateRect(const tools::Rectangle &rInvalidateRect);

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

        void commitAccessibleEvent( sal_Int16 const i_eventID );
        void commitCellEvent( sal_Int16 const i_eventID, const css::uno::Any& i_newValue, const css::uno::Any& i_oldValue );
        void commitTableEvent( sal_Int16 const i_eventID, const css::uno::Any& i_newValue, const css::uno::Any& i_oldValue );

        // ITableControl
        virtual void                hideCursor() override;
        virtual void                showCursor() override;
        virtual bool                dispatchAction( TableControlAction _eAction ) override;
        virtual SelectionEngine*    getSelEngine() override;
        virtual PTableModel         getModel() const override;
        virtual ColPos              getCurrentColumn() const override;
        virtual RowPos              getCurrentRow() const override;
        virtual void                activateCell( ColPos const i_col, RowPos const i_row ) override;
        virtual ::Size              getTableSizePixel() const override;
        virtual void                setPointer( PointerStyle i_pointer ) override;
        virtual void                captureMouse() override;
        virtual void                releaseMouse() override;
        virtual void                invalidate( TableArea const i_what ) override;
        virtual long                pixelWidthToAppFont( long const i_pixels ) const override;
        virtual void                hideTracking() override;
        virtual void                showTracking( tools::Rectangle const & i_location, ShowTrackFlags const i_flags ) override;
        RowPos                      getRowAtPoint( const Point& rPoint ) const;
        ColPos                      getColAtPoint( const Point& rPoint ) const;
        virtual TableCell           hitTest( const Point& rPoint ) const override;
        virtual ColumnMetrics       getColumnMetrics( ColPos const i_column ) const override;
        virtual bool                isRowSelected( RowPos i_row ) const override;


        long                        appFontWidthToPixel( long const i_appFontUnits ) const;

        TableDataWindow&        getDataWindow()       { return *m_pDataWindow; }
        const TableDataWindow&  getDataWindow() const { return *m_pDataWindow; }
        ScrollBar* getHorzScrollbar() { return m_pHScroll; }
        ScrollBar* getVertScrollbar() { return m_pVScroll; }

        tools::Rectangle calcHeaderRect( bool bColHeader );
        tools::Rectangle calcHeaderCellRect( bool bColHeader, sal_Int32 nPos );
        tools::Rectangle calcTableRect();
        tools::Rectangle calcCellRect( sal_Int32 nRow, sal_Int32 nCol );

        // A11Y
        css::uno::Reference< css::accessibility::XAccessible >
                        getAccessible( vcl::Window& i_parentWindow );
        void            disposeAccessible();

        bool     isAccessibleAlive() const { return impl_isAccessibleAlive(); }

        // ITableModelListener
        virtual void    rowsInserted( RowPos first, RowPos last ) override;
        virtual void    rowsRemoved( RowPos first, RowPos last ) override;
        virtual void    columnInserted() override;
        virtual void    columnRemoved() override;
        virtual void    allColumnsRemoved() override;
        virtual void    cellsUpdated( RowPos const i_firstRow, RowPos const i_lastRow ) override;
        virtual void    columnChanged( ColPos const i_column, ColumnAttributeGroup const i_attributeGroup ) override;
        virtual void    tableMetricsChanged() override;

    private:
        bool            impl_isAccessibleAlive() const;
        void            impl_commitAccessibleEvent(
                            sal_Int16 const i_eventID,
                            css::uno::Any const & i_newValue
                        );

        /** toggles the cursor visibility

            The method is not bound to the classes public invariants, as it's used in
            situations where the they must not necessarily be fulfilled.
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
        void        impl_getCellRect( ColPos _nColumn, RowPos _nRow, tools::Rectangle& _rCellRect ) const;

        /** updates all cached model values

            The method is not bound to the classes public invariants, as it's used in
            situations where the they must not necessarily be fulfilled.
        */
        void        impl_ni_updateCachedModelValues();

        /** updates the cached table metrics (row height etc.)
        */
        void        impl_ni_updateCachedTableMetrics();

        /** does a relayout of the table control

            Column widths, and consequently the availability of the vertical and horizontal scrollbar, are updated
            with a call to this method.

            @param i_assumeInflexibleColumnsUpToIncluding
                the index of a column up to which all columns should be considered as inflexible, or
                <code>COL_INVALID</code>.
        */
        void        impl_ni_relayout( ColPos const i_assumeInflexibleColumnsUpToIncluding = COL_INVALID );

        /** calculates the new width of our columns, taking into account their min and max widths, and their relative
            flexibility.

            @param i_assumeInflexibleColumnsUpToIncluding
                the index of a column up to which all columns should be considered as inflexible, or
                <code>COL_INVALID</code>.

            @param i_assumeVerticalScrollbar
                controls whether or not we should assume the presence of a vertical scrollbar. If <true/>, and
                if the model has a VerticalScrollbarVisibility != ScrollbarShowNever, the method will leave
                space for a vertical scrollbar.

            @return
                the overall width of the grid, which is available for columns
        */
        long        impl_ni_calculateColumnWidths(
                        ColPos const i_assumeInflexibleColumnsUpToIncluding,
                        bool const i_assumeVerticalScrollbar,
                        ::std::vector< long >& o_newColWidthsPixel
                    ) const;

        /** positions all child windows, e.g. the both scrollbars, the corner window, and the data window
        */
        void        impl_ni_positionChildWindows(
                        tools::Rectangle const & i_dataCellPlayground,
                        bool const i_verticalScrollbar,
                        bool const i_horizontalScrollbar
                    );

        /** scrolls the view by the given number of rows

            The method is not bound to the classes public invariants, as it's used in
            situations where the they must not necessarily be fulfilled.

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
            situations where the they must not necessarily be fulfilled.

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
            account the fact that there might be less columns than would normally
            find room in the control.

            As a result of respecting the partial visibility of rows and columns,
            the returned area might be larger than the data window's output size.
        */
        tools::Rectangle   impl_getAllVisibleCellsArea() const;

        /** retrieves the area occupied by all (at least partially) visible data cells.

            Effectively, the returned area is the same as returned by ->impl_getAllVisibleCellsArea,
            minus the row and column header areas.
        */
        tools::Rectangle   impl_getAllVisibleDataCellArea() const;

        /** retrieves the column which covers the given ordinate
        */
        ColPos      impl_getColumnForOrdinate( long const i_ordinate ) const;

        /** retrieves the row which covers the given abscissa
        */
        RowPos      impl_getRowForAbscissa( long const i_abscissa ) const;

        /// invalidates the window area occupied by the given column
        void        impl_invalidateColumn( ColPos const i_column );

        DECL_LINK( OnScroll, ScrollBar*, void );
        DECL_LINK( OnUpdateScrollbars, void*, void );
    };

    //see seleng.hxx, seleng.cxx, FunctionSet overridables, part of selection engine
    class TableFunctionSet : public FunctionSet
    {
    private:
        TableControl_Impl*  m_pTableControl;
        RowPos              m_nCurrentRow;

    public:
        explicit TableFunctionSet(TableControl_Impl* _pTableControl);
        virtual ~TableFunctionSet() override;

        virtual void BeginDrag() override;
        virtual void CreateAnchor() override;
        virtual void DestroyAnchor() override;
        virtual void SetCursorAtPoint(const Point& rPoint, bool bDontSelectAtCursor = false) override;
        virtual bool IsSelectionAtPoint( const Point& rPoint ) override;
        virtual void DeselectAtPoint( const Point& rPoint ) override;
        virtual void DeselectAll() override;
    };


} } // namespace svt::table


#endif // INCLUDED_SVTOOLS_SOURCE_TABLE_TABLECONTROL_IMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
