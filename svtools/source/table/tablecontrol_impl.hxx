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

#ifndef SVTOOLS_INC_TABLE_TABLEMODEL_HXX
#include <svtools/table/tablemodel.hxx>
#endif

#ifndef SVTOOLS_INC_TABLE_ABSTRACTTABLECONTROL_HXX
#include <svtools/table/abstracttablecontrol.hxx>
#endif

#include <svtools/table/tablemodel.hxx>
#include <vector>
#include <vcl/seleng.hxx>


class ScrollBar;
class ScrollBarBox;

//........................................................................
namespace svt { namespace table
{
//........................................................................

    typedef ::std::vector< long >   ArrayOfLong;

    class TableControl;
    class TableDataWindow;
    class TableFunctionSet;

    //====================================================================
    //= TableControl_Impl
    //====================================================================
    class TableControl_Impl : public IAbstractTableControl
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
        /// the widths of the single columns, measured in pixel
        ArrayOfLong             m_aColumnWidthsPixel;
        /** the accumulated widths of the single columns, i.e. their exclusive right borders,
            <strong<not</strong> counting the space for a possible row header column
        */
        ArrayOfLong             m_aAccColumnWidthsPixel;

    ArrayOfLong     m_aVisibleColumnWidthsPixel;
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
        ScrollBar*          m_pHScroll;
        ScrollBarBox*           m_pScrollCorner;
    //selection engine - for determining selection range, e.g. single, multiple
    SelectionEngine*        m_pSelEngine;
    //vector which contains the selected rows
    std::vector<RowPos>     m_nRowSelected;
    //part of selection engine
    TableFunctionSet*       m_pTableFunctionSet;
    //part of selection engine
    RowPos              m_nAnchor;
    bool            m_bResizing;
    ColPos          m_nResizingColumn;
    bool            m_bResizingGrid;
    rtl::OUString   m_aTooltipText;

#if DBG_UTIL
    #define INV_SCROLL_POSITION     1
        /** represents a bitmask of invariants to check

            Actually, impl_checkInvariants checks more invariants than denoted in this
            bit mask, but only those present here can be disabled temporarily.
        */
        sal_Int32           m_nRequiredInvariants;
#endif

    public:


        PTableModel getModel() const;
        void        setModel( PTableModel _pModel );

        inline  const PTableInputHandler&   getInputHandler() const { return m_pInputHandler; }

        inline  ColPos  getCurColumn() const    { return m_nCurColumn; }
        inline  RowPos  getCurRow() const       { return m_nCurRow; }
        inline  void    setCurRow(RowPos curRow){ m_nCurRow = curRow; }
        inline  RowPos  getTopRow() const       { return m_nTopRow; }
        inline  long    getRowCount() const     { return m_nRowCount; }
        inline  long    getColumnCount() const  { return m_nColumnCount; }

        inline  long    getColHeaderHightPixel() const  { return m_nColHeaderHeightPixel; }

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
    /** returns the row, which contains the input point*/
    virtual RowPos  getCurrentRow (const Point& rPoint);

    void setCursorAtCurrentCell(const Point& rPoint);
    /** checks whether the vector with the selected rows contains the current row*/
    sal_Bool    isRowSelected(const ::std::vector<RowPos>& selectedRows, RowPos current);

    bool    isRowSelected(RowPos current);
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
    void    clearSelection();
        // IAbstractTableControl
        virtual void    hideCursor();
        virtual void    showCursor();
        virtual bool    dispatchAction( TableControlAction _eAction );
    virtual SelectionEngine* getSelEngine();
    virtual bool isTooltipActive();
    virtual rtl::OUString& setTooltip(const Point& rPoint );
    virtual void resizeColumn(const Point& rPoint);
    virtual bool startResizeColumn(const Point& rPoint);
    virtual bool endResizeColumn(const Point& rPoint);

    TableDataWindow* getDataWindow();
    ScrollBar* getHorzScrollbar();
    ScrollBar* getVertScrollbar();

    ::rtl::OUString convertToString(const ::com::sun::star::uno::Any& _value);
    Rectangle calcHeaderRect(bool bColHeader);
    Rectangle calcTableRect();
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
        TableSize   impl_getVisibleColumns( bool _bAcceptPartialRow ) const;

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

        /** scrolls the view by the given number of columns

            The method is not bound to the classes public invariants, as it's used in
            situations where the they must not necessarily be fullfilled.

            @return
                the number of columns by which the viewport was scrolled. This may differ
                from the given numbers to scroll in case the begin or the end of the
                column range were reached.
        */
        TableSize   impl_ni_ScrollColumns( TableSize _nRowDelta );
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

        void impl_ni_getAccVisibleColWidths();
        void impl_updateLeftColumn();

        DECL_LINK( OnScroll, ScrollBar* );
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
       virtual sal_Bool SetCursorAtPoint(const Point& rPoint, sal_Bool bDontSelectAtCursor);
       virtual sal_Bool IsSelectionAtPoint( const Point& rPoint );
       virtual void DeselectAtPoint( const Point& rPoint );
       virtual void DeselectAll();
    };


//........................................................................
} } // namespace svt::table
//........................................................................

#endif // SVTOOLS_TABLECONTROL_IMPL_HXX
