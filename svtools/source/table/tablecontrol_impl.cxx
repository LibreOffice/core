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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include "svtools/table/tablecontrol.hxx"
#include "svtools/table/defaultinputhandler.hxx"
#include "svtools/table/tablemodel.hxx"

#include "tabledatawindow.hxx"
#include "tablecontrol_impl.hxx"
#include "tablegeometry.hxx"
#include "cellvalueconversion.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChange.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChangeType.hpp>
/** === end UNO includes === **/

#include <comphelper/scopeguard.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/seleng.hxx>
#include <rtl/ref.hxx>
#include <vcl/image.hxx>
#include <tools/diagnose_ex.h>

#include <functional>

//......................................................................................................................
namespace svt { namespace table
{
//......................................................................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::accessibility::AccessibleTableModelChange;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::accessibility::XAccessible;
    using ::com::sun::star::uno::Reference;
    /** === end UNO using === **/
    namespace AccessibleEventId = ::com::sun::star::accessibility::AccessibleEventId;
    namespace AccessibleTableModelChangeType = ::com::sun::star::accessibility::AccessibleTableModelChangeType;

    //====================================================================
    //= TempHideCursor
    //====================================================================
    class TempHideCursor
    {
    private:
        ITableControl&  m_rTable;

    public:
        TempHideCursor( ITableControl& _rTable )
            :m_rTable( _rTable )
        {
            m_rTable.hideCursor();
        }
        ~TempHideCursor()
        {
            m_rTable.showCursor();
        }
    };

    //====================================================================
    //= EmptyTableModel
    //====================================================================
    /** default implementation of an ->ITableModel, used as fallback when no
        real model is present

        Instances of this class are static in any way, and provide the least
        necessary default functionality for a table model.
    */
    class EmptyTableModel : public ITableModel
    {
    public:
        EmptyTableModel()
        {
        }

        // ITableModel overridables
        virtual TableSize           getColumnCount() const
        {
            return 0;
        }
        virtual TableSize           getRowCount() const
        {
            return 0;
        }
        virtual bool                hasColumnHeaders() const
        {
            return false;
        }
        virtual bool                hasRowHeaders() const
        {
            return false;
        }
        virtual bool                isCellEditable( ColPos col, RowPos row ) const
        {
            (void)col;
            (void)row;
            return false;
        }
        virtual PColumnModel        getColumnModel( ColPos column )
        {
            DBG_ERROR( "EmptyTableModel::getColumnModel: invalid call!" );
            (void)column;
            return PColumnModel();
        }
        virtual PTableRenderer      getRenderer() const
        {
            return PTableRenderer();
        }
        virtual PTableInputHandler  getInputHandler() const
        {
            return PTableInputHandler();
        }
        virtual TableMetrics        getRowHeight() const
        {
            return 5 * 100;
        }
        virtual void setRowHeight(TableMetrics _nRowHeight)
        {
            (void)_nRowHeight;
        }
        virtual TableMetrics        getColumnHeaderHeight() const
        {
            return 0;
        }
        virtual TableMetrics        getRowHeaderWidth() const
        {
            return 0;
        }
        virtual ScrollbarVisibility getVerticalScrollbarVisibility() const
        {
            return ScrollbarShowNever;
        }
        virtual ScrollbarVisibility getHorizontalScrollbarVisibility() const
        {
            return ScrollbarShowNever;
        }
        virtual void addTableModelListener( const PTableModelListener& i_listener )
        {
            (void)i_listener;
        }
        virtual void removeTableModelListener( const PTableModelListener& i_listener )
        {
            (void)i_listener;
        }
        virtual ::boost::optional< ::Color > getLineColor() const
        {
            return ::boost::optional< ::Color >();
        }
        virtual ::boost::optional< ::Color > getHeaderBackgroundColor() const
        {
            return ::boost::optional< ::Color >();
        }
        virtual ::boost::optional< ::Color > getHeaderTextColor() const
        {
            return ::boost::optional< ::Color >();
        }
        virtual ::boost::optional< ::Color > getTextColor() const
        {
            return ::boost::optional< ::Color >();
        }
        virtual ::boost::optional< ::Color > getTextLineColor() const
        {
            return ::boost::optional< ::Color >();
        }
        virtual ::boost::optional< ::std::vector< ::Color > > getRowBackgroundColors() const
        {
            return ::boost::optional< ::std::vector< ::Color > >();
        }
        virtual ::com::sun::star::style::VerticalAlignment getVerticalAlign() const
        {
            return com::sun::star::style::VerticalAlignment(0);
        }
        virtual void getCellContent( ColPos const i_col, RowPos const i_row, ::com::sun::star::uno::Any& o_cellContent )
        {
            (void)i_row;
            (void)i_col;
            o_cellContent.clear();
        }
        virtual void getCellToolTip( ColPos const, RowPos const, ::com::sun::star::uno::Any& )
        {
        }
        virtual ::rtl::OUString getRowHeader( RowPos const i_rowPos ) const
        {
            (void)i_rowPos;
            return ::rtl::OUString();
        }
    };


    //====================================================================
    //= TableControl_Impl
    //====================================================================
    DBG_NAME( TableControl_Impl )

#if DBG_UTIL
    //====================================================================
    //= SuspendInvariants
    //====================================================================
    class SuspendInvariants
    {
    private:
        const TableControl_Impl&    m_rTable;
        sal_Int32                   m_nSuspendFlags;

    public:
        SuspendInvariants( const TableControl_Impl& _rTable, sal_Int32 _nSuspendFlags )
            :m_rTable( _rTable )
            ,m_nSuspendFlags( _nSuspendFlags )
        {
            //DBG_ASSERT( ( m_rTable.m_nRequiredInvariants & m_nSuspendFlags ) == m_nSuspendFlags,
            //    "SuspendInvariants: cannot suspend what is already suspended!" );
            const_cast< TableControl_Impl& >( m_rTable ).m_nRequiredInvariants &= ~m_nSuspendFlags;
        }
        ~SuspendInvariants()
        {
            const_cast< TableControl_Impl& >( m_rTable ).m_nRequiredInvariants |= m_nSuspendFlags;
        }
    };
    #define DBG_SUSPEND_INV( flags ) \
        SuspendInvariants aSuspendInv( *this, flags );
#else
    #define DBG_SUSPEND_INV( flags )
#endif

#if DBG_UTIL
    //====================================================================
    const char* TableControl_Impl_checkInvariants( const void* _pInstance )
    {
        return static_cast< const TableControl_Impl* >( _pInstance )->impl_checkInvariants();
    }

    namespace
    {
        template< typename SCALAR_TYPE >
        bool lcl_checkLimitsExclusive( SCALAR_TYPE _nValue, SCALAR_TYPE _nMin, SCALAR_TYPE _nMax )
        {
            return ( _nValue > _nMin ) && ( _nValue < _nMax );
        }

        template< typename SCALAR_TYPE >
        bool lcl_checkLimitsExclusive_OrDefault_OrFallback( SCALAR_TYPE _nValue, SCALAR_TYPE _nMin, SCALAR_TYPE _nMax,
            PTableModel _pModel, SCALAR_TYPE _nDefaultOrFallback )
        {
            if ( !_pModel )
                return _nValue == _nDefaultOrFallback;
            if ( _nMax <= _nMin )
                return _nDefaultOrFallback == _nValue;
            return lcl_checkLimitsExclusive( _nValue, _nMin, _nMax );
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    const sal_Char* TableControl_Impl::impl_checkInvariants() const
    {
        if ( !m_pModel )
            return "no model, not even an EmptyTableModel";

        if ( !m_pDataWindow )
            return "invalid data window!";

        if ( m_pModel->getColumnCount() != m_nColumnCount )
            return "column counts are inconsistent!";

        if ( m_pModel->getRowCount() != m_nRowCount )
            return "row counts are inconsistent!";

        if ( ( m_nCurColumn != COL_INVALID ) && !m_aColumnWidths.empty() && ( m_nCurColumn < 0 ) || ( m_nCurColumn >= (ColPos)m_aColumnWidths.size() ) )
            return "current column is invalid!";

        if ( !lcl_checkLimitsExclusive_OrDefault_OrFallback( m_nTopRow, (RowPos)-1, m_nRowCount, getModel(), (RowPos)0 ) )
            return "invalid top row value!";

        if ( !lcl_checkLimitsExclusive_OrDefault_OrFallback( m_nCurRow, (RowPos)-1, m_nRowCount, getModel(), ROW_INVALID ) )
            return "invalid current row value!";

        if ( !lcl_checkLimitsExclusive_OrDefault_OrFallback( m_nLeftColumn, (ColPos)-1, m_nColumnCount, getModel(), (ColPos)0 ) )
            return "invalid current column value!";

        if ( !lcl_checkLimitsExclusive_OrDefault_OrFallback( m_nCurColumn, (ColPos)-1, m_nColumnCount, getModel(), COL_INVALID ) )
            return "invalid current column value!";

        if  ( m_pInputHandler != m_pModel->getInputHandler() )
            return "input handler is not the model-provided one!";

        // m_aSelectedRows should have reasonable content
        {
            if ( m_aSelectedRows.size() > m_pModel->getRowCount() )
                return "there are more rows selected than actually exist";
            for (   ::std::vector< RowPos >::const_iterator selRow = m_aSelectedRows.begin();
                    selRow != m_aSelectedRows.end();
                    ++selRow
                )
            {
                if ( ( *selRow < 0 ) || ( *selRow >= m_pModel->getRowCount() ) )
                    return "a non-existent row is selected";
            }
        }

        // m_nColHeaderHeightPixel consistent with the model's value?
        {
            TableMetrics nHeaderHeight = m_pModel->hasColumnHeaders() ? m_pModel->getColumnHeaderHeight() : 0;
            nHeaderHeight = m_rAntiImpl.LogicToPixel( Size( 0, nHeaderHeight ), MAP_APPFONT ).Height();
            if ( nHeaderHeight != m_nColHeaderHeightPixel )
                return "column header heights are inconsistent!";
        }

        bool isDummyModel = dynamic_cast< const EmptyTableModel* >( m_pModel.get() ) != NULL;
        if ( !isDummyModel )
        {
            TableMetrics nRowHeight = m_pModel->getRowHeight();
            nRowHeight = m_rAntiImpl.LogicToPixel( Size( 0, nRowHeight ), MAP_APPFONT).Height();
            if ( nRowHeight != m_nRowHeightPixel )
                return "row heights are inconsistent!";
        }

        // m_nRowHeaderWidthPixel consistent with the model's value?
        {
            TableMetrics nHeaderWidth = m_pModel->hasRowHeaders() ? m_pModel->getRowHeaderWidth() : 0;
            nHeaderWidth = m_rAntiImpl.LogicToPixel( Size( nHeaderWidth, 0 ), MAP_APPFONT ).Width();
            if ( nHeaderWidth != m_nRowHeaderWidthPixel )
                return "row header widths are inconsistent!";
        }

        // m_aColumnWidths consistency
        if ( size_t( m_nColumnCount ) != m_aColumnWidths.size() )
            return "wrong number of cached column widths";

        for (   ColumnPositions::const_iterator col = m_aColumnWidths.begin();
                col != m_aColumnWidths.end();
            )
        {
            if ( col->getEnd() < col->getStart() )
                return "column widths: 'end' is expected to not be smaller than start";

            ColumnPositions::const_iterator nextCol = col + 1;
            if ( nextCol != m_aColumnWidths.end() )
                if ( col->getEnd() != nextCol->getStart() )
                    return "column widths: one column's end should be the next column's start";
            col = nextCol;
        }

        if ( m_nLeftColumn < m_nColumnCount )
            if ( m_aColumnWidths[ m_nLeftColumn ].getStart() != m_nRowHeaderWidthPixel )
                return "the left-most column should start immediately after the row header";

        if ( m_nCursorHidden < 0 )
            return "invalid hidden count for the cursor!";

        if ( ( m_nRequiredInvariants & INV_SCROLL_POSITION ) && m_pVScroll )
        {
            DBG_SUSPEND_INV( INV_SCROLL_POSITION );
                // prevent infinite recursion

            if ( m_nLeftColumn < 0 )
                return "invalid left-most column index";
            if ( m_pVScroll->GetThumbPos() != m_nTopRow )
                return "vertical scroll bar |position| is incorrect!";
            if ( m_pVScroll->GetRange().Max() != m_nRowCount )
                return "vertical scroll bar |range| is incorrect!";
            if ( m_pVScroll->GetVisibleSize() != impl_getVisibleRows( false ) )
                return "vertical scroll bar |visible size| is incorrect!";
        }

        if ( ( m_nRequiredInvariants & INV_SCROLL_POSITION ) && m_pHScroll )
        {
            DBG_SUSPEND_INV( INV_SCROLL_POSITION );
                // prevent infinite recursion

            if ( m_pHScroll->GetThumbPos() != m_nLeftColumn )
                return "horizontal scroll bar |position| is incorrect!";
            if ( m_pHScroll->GetRange().Max() != m_nColumnCount )
                return "horizontal scroll bar |range| is incorrect!";
            if ( m_pHScroll->GetVisibleSize() != impl_getVisibleColumns( false ) )
                return "horizontal scroll bar |visible size| is incorrect!";
        }

        return NULL;
    }
#endif

#define DBG_CHECK_ME() \
    DBG_CHKTHIS( TableControl_Impl, TableControl_Impl_checkInvariants )

    //------------------------------------------------------------------------------------------------------------------
    TableControl_Impl::TableControl_Impl( TableControl& _rAntiImpl )
        :m_rAntiImpl            ( _rAntiImpl                    )
        ,m_pModel               ( new EmptyTableModel           )
        ,m_pInputHandler        (                               )
        ,m_nRowHeightPixel      ( 15                            )
        ,m_nColHeaderHeightPixel( 0                             )
        ,m_nRowHeaderWidthPixel ( 0                             )
        ,m_nColumnCount         ( 0                             )
        ,m_nRowCount            ( 0                             )
        ,m_nCurColumn           ( COL_INVALID                   )
        ,m_nCurRow              ( ROW_INVALID                   )
        ,m_nLeftColumn          ( 0                             )
        ,m_nTopRow              ( 0                             )
        ,m_nCursorHidden        ( 1                             )
        ,m_pDataWindow          ( new TableDataWindow( *this )  )
        ,m_pVScroll             ( NULL                          )
        ,m_pHScroll             ( NULL                          )
        ,m_pScrollCorner        ( NULL                          )
        ,m_pSelEngine           (                               )
        ,m_aSelectedRows        (                               )
        ,m_pTableFunctionSet    ( new TableFunctionSet( this  ) )
        ,m_nAnchor              ( -1                            )
        ,m_bResizingColumn      ( false                         )
        ,m_nResizingColumn      ( 0                             )
        ,m_bResizingGrid        ( false                         )
        ,m_bUpdatingColWidths   ( false                         )
        ,m_pAccessibleTable     ( NULL                          )
#if DBG_UTIL
        ,m_nRequiredInvariants ( INV_SCROLL_POSITION )
#endif
    {
        DBG_CTOR( TableControl_Impl, TableControl_Impl_checkInvariants );
        m_pSelEngine = new SelectionEngine( m_pDataWindow.get(), m_pTableFunctionSet );
        m_pSelEngine->SetSelectionMode(SINGLE_SELECTION);
        m_pDataWindow->SetPosPixel( Point( 0, 0 ) );
        m_pDataWindow->Show();
    }

    //------------------------------------------------------------------------------------------------------------------
    TableControl_Impl::~TableControl_Impl()
    {
        DBG_DTOR( TableControl_Impl, TableControl_Impl_checkInvariants );

        DELETEZ( m_pVScroll );
        DELETEZ( m_pHScroll );
        DELETEZ( m_pScrollCorner );
        DELETEZ( m_pTableFunctionSet );
        DELETEZ( m_pSelEngine );
    }

    //------------------------------------------------------------------------------------------------------------------
    void TableControl_Impl::setModel( PTableModel _pModel )
    {
        DBG_CHECK_ME();

        TempHideCursor aHideCursor( *this );

        if ( !!m_pModel )
            m_pModel->removeTableModelListener( shared_from_this() );

        m_pModel = _pModel;
        if ( !m_pModel)
            m_pModel.reset( new EmptyTableModel );

        m_pModel->addTableModelListener( shared_from_this() );

        m_nCurRow = ROW_INVALID;
        m_nCurColumn = COL_INVALID;

        // recalc some model-dependent cached info
        impl_ni_updateCachedModelValues();
        impl_ni_updateScrollbars();

        // completely invalidate
        m_rAntiImpl.Invalidate();

        // reset cursor to (0,0)
        if ( m_nRowCount ) m_nCurRow = 0;
        if ( m_nColumnCount ) m_nCurColumn = 0;
    }

    //------------------------------------------------------------------------------------------------------------------
    namespace
    {
        bool lcl_adjustSelectedRows( ::std::vector< RowPos >& io_selectionIndexes, RowPos const i_firstAffectedRowIndex, TableSize const i_offset )
        {
            bool didChanges = false;
            for (   ::std::vector< RowPos >::iterator selPos = io_selectionIndexes.begin();
                    selPos != io_selectionIndexes.end();
                    ++selPos
                )
            {
                if ( *selPos < i_firstAffectedRowIndex )
                    continue;
                *selPos += i_offset;
                didChanges = true;
            }
            return didChanges;
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void TableControl_Impl::rowsInserted( RowPos i_first, RowPos i_last )
    {
        DBG_CHECK_ME();
        OSL_PRECOND( i_last >= i_first, "TableControl_Impl::rowsInserted: invalid row indexes!" );

        TableSize const insertedRows = i_last - i_first + 1;

        // adjust selection, if necessary
        bool const selectionChanged = lcl_adjustSelectedRows( m_aSelectedRows, i_first, insertedRows );

        // adjust our cached row count
        m_nRowCount = m_pModel->getRowCount();

        // if the rows have been inserted before the current row, adjust this
        if ( i_first <= m_nCurRow )
            goTo( m_nCurColumn, m_nCurRow + insertedRows );

        // adjust scrollbars
        impl_ni_updateScrollbars();

        // notify A1YY events
        if ( impl_isAccessibleAlive() )
        {
            impl_commitAccessibleEvent( AccessibleEventId::TABLE_MODEL_CHANGED,
                makeAny( AccessibleTableModelChange( AccessibleTableModelChangeType::INSERT, i_first, i_last, 0, m_pModel->getColumnCount() ) ),
                Any()
            );
            impl_commitAccessibleEvent( AccessibleEventId::CHILD,
                 makeAny( m_pAccessibleTable->getTableHeader( TCTYPE_ROWHEADERBAR ) ),
                Any()
            );

//          for ( sal_Int32 i = 0 ; i <= m_pModel->getColumnCount(); ++i )
//          {
//              impl_commitAccessibleEvent(
//                    CHILD,
//                  makeAny( m_pAccessibleTable->getTable() ),
//                  Any());
//          }
            // Huh? What's that? We're notifying |columnCount| CHILD events here, claiming the *table* itself
            // has been inserted. Doesn't make much sense, does it?
        }

        // schedule repaint
        invalidateRowRange( i_first, ROW_INVALID );

        // call selection handlers, if necessary
        if ( selectionChanged )
            m_rAntiImpl.Select();
    }

    //------------------------------------------------------------------------------------------------------------------
    void TableControl_Impl::rowsRemoved( RowPos i_first, RowPos i_last )
    {
        sal_Int32 firstRemovedRow = i_first;
        sal_Int32 lastRemovedRow = i_last;

        // adjust selection, if necessary
        bool selectionChanged = false;
        if ( i_first == -1 )
        {
            selectionChanged = markAllRowsAsDeselected();

            firstRemovedRow = 0;
            lastRemovedRow = m_nRowCount - 1;
        }
        else
        {
            ENSURE_OR_RETURN_VOID( i_last >= i_first, "TableControl_Impl::rowsRemoved: illegal indexes!" );

            for ( sal_Int32 row = i_first; row <= i_last; ++row )
            {
                if ( markRowAsDeselected( row ) )
                    selectionChanged = true;
            }

            if ( lcl_adjustSelectedRows( m_aSelectedRows, i_last + 1, i_first - i_last - 1 ) )
                selectionChanged = true;
        }

        // adjust cached row count
        m_nRowCount = m_pModel->getRowCount();

        // adjust the current row, if it is larger than the row count now
        if ( m_nCurRow >= m_nRowCount )
        {
            if ( m_nRowCount > 0 )
                goTo( m_nCurColumn, m_nRowCount - 1 );
            else
                m_nCurRow = ROW_INVALID;
        }

        // adjust scrollbars
        impl_ni_updateScrollbars();

        // notify A11Y events
        if ( impl_isAccessibleAlive() )
        {
            impl_commitAccessibleEvent(
                AccessibleEventId::TABLE_MODEL_CHANGED,
                makeAny( AccessibleTableModelChange(
                    AccessibleTableModelChangeType::DELETE,
                    firstRemovedRow,
                    lastRemovedRow,
                    0,
                    m_pModel->getColumnCount()
                ) ),
                Any()
            );
        }

        // schedule a repaint
        invalidateRowRange( firstRemovedRow, ROW_INVALID );

        // call selection handlers, if necessary
        if ( selectionChanged )
            m_rAntiImpl.Select();
    }

    //------------------------------------------------------------------------------------------------------------------
    void TableControl_Impl::columnInserted( ColPos const i_colIndex )
    {
        m_nColumnCount = m_pModel->getColumnCount();
        impl_ni_updateColumnWidths();
        impl_ni_updateScrollbars();

        m_rAntiImpl.Invalidate();

        OSL_UNUSED( i_colIndex );
   }

    //------------------------------------------------------------------------------------------------------------------
    void TableControl_Impl::columnRemoved( ColPos const i_colIndex )
    {
        m_nColumnCount = m_pModel->getColumnCount();
        impl_ni_updateColumnWidths();
        impl_ni_updateScrollbars();

        m_rAntiImpl.Invalidate();

        OSL_UNUSED( i_colIndex );
    }

    //------------------------------------------------------------------------------------------------------------------
    void TableControl_Impl::allColumnsRemoved()
    {
        m_nColumnCount = m_pModel->getColumnCount();
        impl_ni_updateColumnWidths();
        impl_ni_updateScrollbars();

        m_rAntiImpl.Invalidate();
    }

    //------------------------------------------------------------------------------------------------------------------
    void TableControl_Impl::cellsUpdated( ColPos firstCol, ColPos lastCol, RowPos firstRow, RowPos lastRow )
    {
        OSL_ENSURE( false, "TableControl_Impl::cellsUpdated: not implemented!" );
            // there's no known implementation (yet) which calls this method
        OSL_UNUSED( firstCol );
        OSL_UNUSED( lastCol );
        OSL_UNUSED( firstRow );
        OSL_UNUSED( lastRow );
    }

    //------------------------------------------------------------------------------------------------------------------
    void TableControl_Impl::tableMetricsChanged()
    {
        impl_ni_updateCachedTableMetrics();
        impl_ni_updateScrollbars();
        m_rAntiImpl.Invalidate();
    }

    //------------------------------------------------------------------------------------------------------------------
    void TableControl_Impl::impl_invalidateColumn( ColPos const i_column )
    {
        DBG_CHECK_ME();

        Rectangle aAllCellsArea;
        impl_getAllVisibleCellsArea( aAllCellsArea );

        const TableColumnGeometry aColumn( *this, aAllCellsArea, i_column );
        if ( aColumn.isValid() )
            m_rAntiImpl.Invalidate( aColumn.getRect() );
    }

    //------------------------------------------------------------------------------------------------------------------
    void TableControl_Impl::columnChanged( ColPos const i_column, ColumnAttributeGroup const i_attributeGroup )
    {
        ColumnAttributeGroup nGroup( i_attributeGroup );
        if ( nGroup & COL_ATTRS_APPEARANCE )
        {
            impl_invalidateColumn( i_column );
            nGroup &= ~COL_ATTRS_APPEARANCE;
        }

        if ( nGroup & COL_ATTRS_WIDTH )
        {
            if ( !m_bUpdatingColWidths )
                impl_ni_updateColumnWidths();

            nGroup &= ~COL_ATTRS_WIDTH;
        }

        OSL_ENSURE( ( nGroup == COL_ATTRS_NONE ) || ( i_attributeGroup == COL_ATTRS_ALL ),
            "TableControl_Impl::columnChanged: don't know how to handle this change!" );
    }

    //------------------------------------------------------------------------------------------------------------------
    void TableControl_Impl::impl_getAllVisibleCellsArea( Rectangle& _rCellArea ) const
    {
        DBG_CHECK_ME();

        _rCellArea.Left() = 0;
        _rCellArea.Top() = 0;

        // determine the right-most border of the last column which is
        // at least partially visible
        _rCellArea.Right() = m_nRowHeaderWidthPixel;
        if ( !m_aColumnWidths.empty() )
        {
            // the number of pixels which are scrolled out of the left hand
            // side of the window
            const long nScrolledOutLeft = m_nLeftColumn == 0 ? 0 : m_aColumnWidths[ m_nLeftColumn - 1 ].getEnd();

            ColumnPositions::const_reverse_iterator loop = m_aColumnWidths.rbegin();
            do
            {
                _rCellArea.Right() = loop->getEnd() - nScrolledOutLeft + m_nRowHeaderWidthPixel;
                ++loop;
            }
            while ( (   loop != m_aColumnWidths.rend() )
                 && (   loop->getEnd() - nScrolledOutLeft >= _rCellArea.Right() )
                 );
        }
        // so far, _rCellArea.Right() denotes the first pixel *after* the cell area
        --_rCellArea.Right();

        // determine the last row which is at least partially visible
        _rCellArea.Bottom() =
                m_nColHeaderHeightPixel
            +   impl_getVisibleRows( true ) * m_nRowHeightPixel
            -   1;
    }

    //------------------------------------------------------------------------------------------------------------------
    void TableControl_Impl::impl_getAllVisibleDataCellArea( Rectangle& _rCellArea ) const
    {
        DBG_CHECK_ME();

        impl_getAllVisibleCellsArea( _rCellArea );
        _rCellArea.Left() = m_nRowHeaderWidthPixel;
        _rCellArea.Top() = m_nColHeaderHeightPixel;
    }

    //------------------------------------------------------------------------------------------------------------------
    void TableControl_Impl::impl_ni_updateCachedTableMetrics()
    {
        m_nRowHeightPixel = m_rAntiImpl.LogicToPixel( Size( 0, m_pModel->getRowHeight() ), MAP_APPFONT ).Height();

        m_nColHeaderHeightPixel = 0;
        if ( m_pModel->hasColumnHeaders() )
           m_nColHeaderHeightPixel = m_rAntiImpl.LogicToPixel( Size( 0, m_pModel->getColumnHeaderHeight() ), MAP_APPFONT ).Height();

        m_nRowHeaderWidthPixel = 0;
        if ( m_pModel->hasRowHeaders() )
            m_nRowHeaderWidthPixel = m_rAntiImpl.LogicToPixel( Size( m_pModel->getRowHeaderWidth(), 0 ), MAP_APPFONT).Width();
    }

    //------------------------------------------------------------------------------------------------------------------
    void TableControl_Impl::impl_ni_updateCachedModelValues()
    {
        m_pInputHandler.reset();
        m_nColumnCount = m_nRowCount = 0;

        impl_ni_updateCachedTableMetrics();
        impl_ni_updateColumnWidths();

        m_pInputHandler = m_pModel->getInputHandler();
        if ( !m_pInputHandler )
            m_pInputHandler.reset( new DefaultInputHandler );

        m_nColumnCount = m_pModel->getColumnCount();
        m_nRowCount = m_pModel->getRowCount();
    }

    //------------------------------------------------------------------------------------------------------------------
    void TableControl_Impl::impl_ni_updateColumnWidths()
    {
        ENSURE_OR_RETURN_VOID( !m_bUpdatingColWidths, "TableControl_Impl::impl_ni_updateColumnWidths: recursive call detected!" );

        m_aColumnWidths.resize( 0 );
        if ( !m_pModel )
            return;

        const TableSize colCount = m_pModel->getColumnCount();
        if ( colCount == 0 )
            return;

        m_bUpdatingColWidths = true;
        const ::comphelper::FlagGuard aWidthUpdateFlag( m_bUpdatingColWidths );

        m_aColumnWidths.reserve( colCount );

        std::vector<sal_Int32> aPrePixelWidths(0);
        long accumulatedPixelWidth = 0;
        int lastResizableCol = COL_INVALID;
        double gridWidth = m_rAntiImpl.GetOutputSizePixel().Width();
        if ( m_pModel->hasRowHeaders() && ( gridWidth != 0 ) )
        {
#if OSL_DEBUG_LEVEL > 0
            const TableMetrics rowHeaderWidth = m_pModel->getRowHeaderWidth();
            const long rowHeaderWidthPixel = m_rAntiImpl.LogicToPixel( Size( rowHeaderWidth, 0 ), MAP_APPFONT ).Width();
            OSL_ENSURE( rowHeaderWidthPixel == m_nRowHeaderWidthPixel,
                "TableControl_Impl::impl_ni_updateColumnWidths: cached row header width is not correct anymore!" );
#endif
            accumulatedPixelWidth += m_nRowHeaderWidthPixel;
            gridWidth -= m_nRowHeaderWidthPixel;
        }

        double colWidthsSum = 0.0;
        double colWithoutFixedWidthsSum = 0.0;
        double minColWithoutFixedSum = 0.0;

        for ( ColPos col = 0; col < colCount; ++col )
        {
            const PColumnModel pColumn = m_pModel->getColumnModel( col );
            ENSURE_OR_CONTINUE( !!pColumn, "TableControl_Impl::impl_ni_updateColumnWidths: invalid column returned by the model!" );

            TableMetrics colWidth = 0;
            const TableMetrics colPrefWidth = pColumn->getPreferredWidth();
            const bool bResizable = pColumn->isResizable();

            if ( colPrefWidth != 0)
            {
                if ( m_bResizingGrid )
                {
                    colWidth = pColumn->getWidth();
                    pColumn->setPreferredWidth(0);
                }
                else
                {
                    colWidth = colPrefWidth;
                    pColumn->setWidth( colPrefWidth );
                }
            }
            else
                colWidth = pColumn->getWidth();

            const long pixelWidth = m_rAntiImpl.LogicToPixel( Size( colWidth, 0 ), MAP_APPFONT ).Width();
            if ( bResizable && colPrefWidth == 0 )
            {
                colWithoutFixedWidthsSum += pixelWidth;
                lastResizableCol = col;
            }
            colWidthsSum += pixelWidth;
            aPrePixelWidths.push_back( pixelWidth );
        }

        double gridWidthWithoutFixed = gridWidth - colWidthsSum + colWithoutFixedWidthsSum;
        double scalingFactor = 1.0;
        if ( m_bResizingGrid )
        {
            if ( gridWidthWithoutFixed > ( minColWithoutFixedSum + colWidthsSum - colWithoutFixedWidthsSum ) )
                scalingFactor = gridWidthWithoutFixed / colWithoutFixedWidthsSum;
        }
        else
        {
            if ( colWidthsSum < gridWidthWithoutFixed )
            {
                if ( colWithoutFixedWidthsSum > 0 )
                    scalingFactor = gridWidthWithoutFixed / colWithoutFixedWidthsSum;
            }
        }
        for ( ColPos i = 0; i < colCount; ++i )
        {
            const PColumnModel pColumn = m_pModel->getColumnModel( i );
            ENSURE_OR_CONTINUE( !!pColumn, "TableControl_Impl::impl_ni_updateColumnWidths: invalid column returned by the model!" );

            if ( pColumn->isResizable() && ( pColumn->getPreferredWidth() == 0 ) )
            {
                aPrePixelWidths[i] *= scalingFactor;
                const TableMetrics logicColWidth = m_rAntiImpl.PixelToLogic( Size( aPrePixelWidths[i], 0 ), MAP_APPFONT ).Width();
                pColumn->setWidth( logicColWidth );
            }

            const long columnStart = accumulatedPixelWidth;
            const long columnEnd = columnStart + aPrePixelWidths[i];
            m_aColumnWidths.push_back( ColumnWidthInfo( columnStart, columnEnd ) );
            accumulatedPixelWidth = columnEnd;
        }

        // care for the horizontal scroll position (as indicated by m_nLeftColumn)
        if ( m_nLeftColumn > 0 )
        {
            const long offsetPixel = m_aColumnWidths[ 0 ].getStart() - m_aColumnWidths[ m_nLeftColumn ].getStart();
            for (   ColumnPositions::iterator colPos = m_aColumnWidths.begin();
                    colPos != m_aColumnWidths.end();
                    ++colPos
                 )
            {
                colPos->move( offsetPixel );
            }
        }

        const long freeSpaceRight = gridWidth - m_aColumnWidths[ colCount-1 ].getEnd();
        if  (   ( freeSpaceRight > 0 )
            &&  ( lastResizableCol != COL_INVALID )
            &&  ( lastResizableCol >= m_nLeftColumn )
            )
        {
            // make the last resizable column wider
            ColumnWidthInfo& rResizeColInfo( m_aColumnWidths[ lastResizableCol ] );
            rResizeColInfo.setEnd( rResizeColInfo.getEnd() + freeSpaceRight );

            // update the column model
            const TableMetrics logicColWidth = m_rAntiImpl.PixelToLogic( Size( rResizeColInfo.getWidth(), 0 ), MAP_APPFONT ).Width();
            const PColumnModel pColumn = m_pModel->getColumnModel( lastResizableCol );
            pColumn->setWidth( logicColWidth );

            // update all other columns after the resized one
            ColPos adjustColumn = lastResizableCol;
            while ( ++adjustColumn < colCount )
            {
                m_aColumnWidths[ adjustColumn ].move( freeSpaceRight );
            }
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    namespace
    {
        //..............................................................................................................
        /// determines whether a scrollbar is needed for the given values
        bool lcl_determineScrollbarNeed( long const i_position, ScrollbarVisibility const i_visibility,
            long const i_availableSpace, long const i_neededSpace )
        {
            if ( i_visibility == ScrollbarShowNever )
                return false;
            if ( i_visibility == ScrollbarShowAlways )
                return true;
            if ( i_position > 0 )
                return true;
            if ( i_availableSpace >= i_neededSpace )
                return false;
            return true;
        }

        //..............................................................................................................
        void lcl_setButtonRepeat( Window& _rWindow, ULONG _nDelay )
        {
            AllSettings aSettings = _rWindow.GetSettings();
            MouseSettings aMouseSettings = aSettings.GetMouseSettings();

            aMouseSettings.SetButtonRepeat( _nDelay );
            aSettings.SetMouseSettings( aMouseSettings );

            _rWindow.SetSettings( aSettings, TRUE );
        }

        //..............................................................................................................
        void lcl_updateScrollbar( Window& _rParent, ScrollBar*& _rpBar,
            bool const i_needBar, long _nVisibleUnits,
            long _nPosition, long _nLineSize, long _nRange,
            bool _bHorizontal, const Link& _rScrollHandler )
        {
            // do we currently have the scrollbar?
            bool bHaveBar = _rpBar != NULL;

            // do we need to correct the scrollbar visibility?
            if ( bHaveBar && !i_needBar )
            {
                if ( _rpBar->IsTracking() )
                    _rpBar->EndTracking();
                DELETEZ( _rpBar );
            }
            else if ( !bHaveBar && i_needBar )
            {
                _rpBar = new ScrollBar(
                    &_rParent,
                    WB_DRAG | ( _bHorizontal ? WB_HSCROLL : WB_VSCROLL )
                );
                _rpBar->SetScrollHdl( _rScrollHandler );
                // get some speed into the scrolling ....
                lcl_setButtonRepeat( *_rpBar, 0 );
            }

            if ( _rpBar )
            {
                _rpBar->SetRange( Range( 0, _nRange ) );
                _rpBar->SetVisibleSize( _nVisibleUnits );
                _rpBar->SetPageSize( _nVisibleUnits );
                _rpBar->SetLineSize( _nLineSize );
                _rpBar->SetThumbPos( _nPosition );
                _rpBar->Show();
            }
        }

        //..............................................................................................................
        /** returns the number of rows fitting into the given range,
            for the given row height. Partially fitting rows are counted, too, if the
            respective parameter says so.
        */
        TableSize lcl_getRowsFittingInto( long _nOverallHeight, long _nRowHeightPixel, bool _bAcceptPartialRow = false )
        {
            return  _bAcceptPartialRow
                ?   ( _nOverallHeight + ( _nRowHeightPixel - 1 ) ) / _nRowHeightPixel
                :   _nOverallHeight / _nRowHeightPixel;
        }

        //..............................................................................................................
        /** returns the number of columns fitting into the given area,
            with the first visible column as given. Partially fitting columns are counted, too,
            if the respective parameter says so.
        */
        TableSize lcl_getColumnsVisibleWithin( const Rectangle& _rArea, ColPos _nFirstVisibleColumn,
            const TableControl_Impl& _rControl, bool _bAcceptPartialRow )
        {
            TableSize visibleColumns = 0;
            TableColumnGeometry aColumn( _rControl, _rArea, _nFirstVisibleColumn );
            while ( aColumn.isValid() )
            {
                if ( !_bAcceptPartialRow )
                    if ( aColumn.getRect().Right() > _rArea.Right() )
                        // this column is only partially visible, and this is not allowed
                        break;

                aColumn.moveRight();
                ++visibleColumns;
            }
            return visibleColumns;
        }

    }

    //------------------------------------------------------------------------------------------------------------------
    void TableControl_Impl::impl_ni_updateScrollbars()
    {
        TempHideCursor aHideCursor( *this );

        // the width/height of a scrollbar, needed several times below
        long nScrollbarMetrics = m_rAntiImpl.GetSettings().GetStyleSettings().GetScrollBarSize();
        if ( m_rAntiImpl.IsZoom() )
            nScrollbarMetrics = (long)( nScrollbarMetrics * (double)m_rAntiImpl.GetZoom() );

        // determine the playground for the data cells (excluding headers)
        // TODO: what if the control is smaller than needed for the headers/scrollbars?
        Rectangle aDataCellPlayground( Point( 0, 0 ), m_rAntiImpl.GetOutputSizePixel() );
        aDataCellPlayground.Left() = m_nRowHeaderWidthPixel;
        aDataCellPlayground.Top() = m_nColHeaderHeightPixel;
        m_nRowCount = m_pModel->getRowCount();
        m_nColumnCount = m_pModel->getColumnCount();

        if ( m_aColumnWidths.empty() )
            impl_ni_updateColumnWidths();
        OSL_ENSURE( m_aColumnWidths.size() == size_t( m_nColumnCount ), "TableControl_Impl::impl_ni_updateScrollbars: inconsistency!" );
        const long nAllColumnsWidth =   m_aColumnWidths.empty()
                                    ?   0
                                    :   m_aColumnWidths[ m_nColumnCount - 1 ].getEnd() - m_aColumnWidths[ 0 ].getStart();

        const ScrollbarVisibility eVertScrollbar = m_pModel->getVerticalScrollbarVisibility();
        const ScrollbarVisibility eHorzScrollbar = m_pModel->getHorizontalScrollbarVisibility();

        // do we need a vertical scrollbar?
        bool bNeedVerticalScrollbar = lcl_determineScrollbarNeed(
            m_nTopRow, eVertScrollbar, aDataCellPlayground.GetHeight(), m_nRowHeightPixel * m_nRowCount );
        bool bFirstRoundVScrollNeed = false;
        if ( bNeedVerticalScrollbar )
        {
            aDataCellPlayground.Right() -= nScrollbarMetrics;
            bFirstRoundVScrollNeed = true;
        }
        // do we need a horizontal scrollbar?
        const bool bNeedHorizontalScrollbar = lcl_determineScrollbarNeed( m_nLeftColumn, eHorzScrollbar, aDataCellPlayground.GetWidth(), nAllColumnsWidth );
        if ( bNeedHorizontalScrollbar )
        {
            aDataCellPlayground.Bottom() -= nScrollbarMetrics;

            // now that we just found that we need a horizontal scrollbar,
            // the need for a vertical one may have changed, since the horizontal
            // SB might just occupy enough space so that not all rows do fit
            // anymore
            if  ( !bFirstRoundVScrollNeed )
            {
                bNeedVerticalScrollbar = lcl_determineScrollbarNeed(
                    m_nTopRow, eVertScrollbar, aDataCellPlayground.GetHeight(), m_nRowHeightPixel * m_nRowCount );
                if ( bNeedVerticalScrollbar )
                {
                    aDataCellPlayground.Right() -= nScrollbarMetrics;
                }
            }
        }
        // create or destroy the vertical scrollbar, as needed
        lcl_updateScrollbar(
            m_rAntiImpl,
            m_pVScroll,
            bNeedVerticalScrollbar,
            lcl_getRowsFittingInto( aDataCellPlayground.GetHeight(), m_nRowHeightPixel ),
                                                                    // visible units
            m_nTopRow,                                              // current position
            1,                                                      // line size
            m_nRowCount,                                            // range
            false,                                                  // vertical
            LINK( this, TableControl_Impl, OnScroll )               // scroll handler
        );
        // position it
        if ( m_pVScroll )
        {
            Rectangle aScrollbarArea(
                Point( aDataCellPlayground.Right() + 1, 0 ),
                Size( nScrollbarMetrics, aDataCellPlayground.Bottom() + 1 )
            );
            m_pVScroll->SetPosSizePixel(
                aScrollbarArea.TopLeft(), aScrollbarArea.GetSize() );
        }

        // create or destroy the horizontal scrollbar, as needed
        lcl_updateScrollbar(
            m_rAntiImpl,
            m_pHScroll,
            bNeedHorizontalScrollbar,
            lcl_getColumnsVisibleWithin( aDataCellPlayground, m_nLeftColumn, *this, false ),
                                                                    // visible units
            m_nLeftColumn,                                          // current position
            1,                                                      // line size
            m_nColumnCount,                                         // range
            true,                                                   // horizontal
            LINK( this, TableControl_Impl, OnScroll )               // scroll handler
        );
        // position it
        if ( m_pHScroll )
        {
            TableSize const nVisibleUnits = lcl_getColumnsVisibleWithin( aDataCellPlayground, m_nLeftColumn, *this, false );
            TableMetrics const nRange = m_nColumnCount;
            if( m_nLeftColumn + nVisibleUnits == nRange - 1 )
            {
                if ( m_aColumnWidths[ nRange - 1 ].getStart() - m_aColumnWidths[ m_nLeftColumn ].getEnd() + m_aColumnWidths[ nRange-1 ].getWidth() > aDataCellPlayground.GetWidth() )
                {
                    m_pHScroll->SetVisibleSize( nVisibleUnits -1 );
                    m_pHScroll->SetPageSize( nVisibleUnits - 1 );
                }
            }
            Rectangle aScrollbarArea(
                Point( 0, aDataCellPlayground.Bottom() + 1 ),
                Size( aDataCellPlayground.Right() + 1, nScrollbarMetrics )
            );
            m_pHScroll->SetPosSizePixel(
                aScrollbarArea.TopLeft(), aScrollbarArea.GetSize() );
        }

        // the corner window connecting the two scrollbars in the lower right corner
        bool bHaveScrollCorner = NULL != m_pScrollCorner;
        bool bNeedScrollCorner = ( NULL != m_pHScroll ) && ( NULL != m_pVScroll );
        if ( bHaveScrollCorner && !bNeedScrollCorner )
        {
            DELETEZ( m_pScrollCorner );
        }
        else if ( !bHaveScrollCorner && bNeedScrollCorner )
        {
            m_pScrollCorner = new ScrollBarBox( &m_rAntiImpl );
            m_pScrollCorner->SetSizePixel( Size( nScrollbarMetrics, nScrollbarMetrics ) );
            m_pScrollCorner->SetPosPixel( Point( aDataCellPlayground.Right() + 1, aDataCellPlayground.Bottom() + 1 ) );
            m_pScrollCorner->Show();
        }
        else if(bHaveScrollCorner && bNeedScrollCorner)
        {
            m_pScrollCorner->SetPosPixel( Point( aDataCellPlayground.Right() + 1, aDataCellPlayground.Bottom() + 1 ) );
            m_pScrollCorner->Show();
        }

        // resize the data window
        m_pDataWindow->SetSizePixel( Size(
            aDataCellPlayground.GetWidth() + m_nRowHeaderWidthPixel,
            aDataCellPlayground.GetHeight() + m_nColHeaderHeightPixel
        ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    void TableControl_Impl::onResize()
    {
        DBG_CHECK_ME();
        if ( m_nRowCount != 0 )
        {
            if ( m_nColumnCount != 0 )
            {
                if ( m_bResizingGrid )
                    impl_ni_updateColumnWidths();
                impl_ni_updateScrollbars();
                checkCursorPosition();
                m_bResizingGrid = true;
            }
        }
        else
        {
            //In the case that column headers are defined but data hasn't yet been set,
            //only column headers will be shown
            if ( m_pModel->hasColumnHeaders() )
            {
                if ( m_nColHeaderHeightPixel > 1 )
                {
                    m_pDataWindow->SetSizePixel( m_rAntiImpl.GetOutputSizePixel() );
                    if ( m_bResizingGrid )
                        //update column widths to fit in grid
                        impl_ni_updateColumnWidths();
                    m_bResizingGrid = true;
                }
            }
            impl_ni_updateScrollbars();
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void TableControl_Impl::doPaintContent( const Rectangle& _rUpdateRect )
    {
        DBG_CHECK_ME();

        if ( !getModel() )
            return;
        PTableRenderer pRenderer = getModel()->getRenderer();
        DBG_ASSERT( !!pRenderer, "TableDataWindow::doPaintContent: invalid renderer!" );
        if ( !pRenderer )
            return;

        // our current style settings, to be passed to the renderer
        const StyleSettings& rStyle = m_rAntiImpl.GetSettings().GetStyleSettings();
        m_nRowCount = m_pModel->getRowCount();
        TableSize nVisibleRows = impl_getVisibleRows(true);
        TableSize nActualRows = m_nRowCount;
        if(m_nRowCount>nVisibleRows)
            nActualRows = nVisibleRows;
        // the area occupied by all (at least partially) visible cells, including
        // headers
        Rectangle aAllCellsWithHeaders;
        impl_getAllVisibleCellsArea( aAllCellsWithHeaders );

        // ............................
        // draw the header column area
        if ( m_pModel->hasColumnHeaders() )
        {
            TableRowGeometry aHeaderRow( *this, Rectangle( Point( 0, 0 ),
                aAllCellsWithHeaders.BottomRight() ), ROW_COL_HEADERS );
            Rectangle aColRect(aHeaderRow.getRect());
            pRenderer->PaintHeaderArea(
                *m_pDataWindow, aColRect, true, false, rStyle
            );
            // Note that strictly, aHeaderRow.getRect() also contains the intersection between column
            // and row header area. However, below we go to paint this intersection, again,
            // so this hopefully doesn't hurt if we already paint it here.

            for ( TableCellGeometry aCell( aHeaderRow, m_nLeftColumn );
                  aCell.isValid();
                  aCell.moveRight()
                )
            {
                if ( _rUpdateRect.GetIntersection( aCell.getRect() ).IsEmpty() )
                    continue;

                bool isActiveColumn = ( aCell.getColumn() == getCurrentColumn() );
                bool isSelectedColumn = false;
                pRenderer->PaintColumnHeader( aCell.getColumn(), isActiveColumn, isSelectedColumn,
                    *m_pDataWindow, aCell.getRect(), rStyle );
            }
        }
        // the area occupied by the row header, if any
        Rectangle aRowHeaderArea;
        if ( m_pModel->hasRowHeaders() )
        {
            aRowHeaderArea = aAllCellsWithHeaders;
            aRowHeaderArea.Right() = m_nRowHeaderWidthPixel - 1;
            if ( m_nTopRow + nActualRows > m_nRowCount )
                aRowHeaderArea.Bottom() = m_nRowHeightPixel * (nActualRows - 1 ) + m_nColHeaderHeightPixel - 1;
            else
                aRowHeaderArea.Bottom() = m_nRowHeightPixel * nActualRows + m_nColHeaderHeightPixel - 1;
            pRenderer->PaintHeaderArea( *m_pDataWindow, aRowHeaderArea, false, true, rStyle );
            // Note that strictly, aRowHeaderArea also contains the intersection between column
            // and row header area. However, below we go to paint this intersection, again,
            // so this hopefully doesn't hurt if we already paint it here.

            if ( m_pModel->hasColumnHeaders() )
            {
                TableCellGeometry const aIntersection( *this, Rectangle( Point( 0, 0 ),
                    aAllCellsWithHeaders.BottomRight() ), COL_ROW_HEADERS, ROW_COL_HEADERS );
                Rectangle const aInters( aIntersection.getRect() );
                pRenderer->PaintHeaderArea(
                    *m_pDataWindow, aInters, true, true, rStyle
                );
            }
        }

        // ............................
        // draw the table content row by row

        TableSize colCount = getModel()->getColumnCount();

        // paint all rows
        Rectangle aAllDataCellsArea;
        impl_getAllVisibleDataCellArea( aAllDataCellsArea );
        for ( TableRowGeometry aRowIterator( *this, aAllCellsWithHeaders, getTopRow() );
              aRowIterator.isValid();
              aRowIterator.moveDown() )
        {
            if ( _rUpdateRect.GetIntersection( aRowIterator.getRect() ).IsEmpty() )
                continue;

            bool const isActiveRow = ( aRowIterator.getRow() == getCurrentRow() );
            bool const isSelectedRow = isRowSelected( aRowIterator.getRow() );

            Rectangle const aRect = aRowIterator.getRect().GetIntersection( aAllDataCellsArea );

            // give the redenderer a chance to prepare the row
            pRenderer->PrepareRow(
                aRowIterator.getRow(), isActiveRow, isSelectedRow,
                *m_pDataWindow, aRect, rStyle
            );

            // paint the row header
            if ( m_pModel->hasRowHeaders() )
            {
                const Rectangle aCurrentRowHeader( aRowHeaderArea.GetIntersection( aRowIterator.getRect() ) );
                pRenderer->PaintRowHeader( isActiveRow, isSelectedRow, *m_pDataWindow, aCurrentRowHeader,
                    rStyle );
            }

            if ( !colCount )
                continue;

            // paint all cells in this row
            for ( TableCellGeometry aCell( aRowIterator, m_nLeftColumn );
                  aCell.isValid();
                  aCell.moveRight()
                )
            {
                bool isSelectedColumn = false;
                pRenderer->PaintCell( aCell.getColumn(), isSelectedRow || isSelectedColumn, isActiveRow,
                                *m_pDataWindow, aCell.getRect(), rStyle );
            }
        }
    }
    //------------------------------------------------------------------------------------------------------------------
    void TableControl_Impl::hideCursor()
    {
        DBG_CHECK_ME();

        if ( ++m_nCursorHidden == 1 )
            impl_ni_doSwitchCursor( false );
    }

    //------------------------------------------------------------------------------------------------------------------
    void TableControl_Impl::showCursor()
    {
        DBG_CHECK_ME();

        DBG_ASSERT( m_nCursorHidden > 0, "TableControl_Impl::showCursor: cursor not hidden!" );
        if ( --m_nCursorHidden == 0 )
            impl_ni_doSwitchCursor( true );
    }

    //------------------------------------------------------------------------------------------------------------------
    bool TableControl_Impl::dispatchAction( TableControlAction _eAction )
    {
        DBG_CHECK_ME();

        bool bSuccess = false;
        bool selectionChanged = false;

        Rectangle rCells;
        switch ( _eAction )
        {
        case cursorDown:
        if(m_pSelEngine->GetSelectionMode() == SINGLE_SELECTION)
        {
            //if other rows already selected, deselect them
            if(m_aSelectedRows.size()>0)
            {
                for(std::vector<RowPos>::iterator it=m_aSelectedRows.begin();
                        it!=m_aSelectedRows.end();++it)
                {
                    invalidateSelectedRegion(*it, *it, rCells);
                }
                m_aSelectedRows.clear();
            }
            if(m_nCurRow < m_nRowCount-1)
            {
                ++m_nCurRow;
                m_aSelectedRows.push_back(m_nCurRow);
            }
            else
                m_aSelectedRows.push_back(m_nCurRow);
            invalidateSelectedRegion(m_nCurRow, m_nCurRow, rCells);
            ensureVisible(m_nCurColumn,m_nCurRow,false);
            selectionChanged = true;
            bSuccess = true;
        }
        else
        {
            if ( m_nCurRow < m_nRowCount - 1 )
                bSuccess = goTo( m_nCurColumn, m_nCurRow + 1 );
        }
            break;

        case cursorUp:
        if(m_pSelEngine->GetSelectionMode() == SINGLE_SELECTION)
        {
            if(m_aSelectedRows.size()>0)
            {
                for(std::vector<RowPos>::iterator it=m_aSelectedRows.begin();
                    it!=m_aSelectedRows.end();++it)
                {
                    invalidateSelectedRegion(*it, *it, rCells);
                }
                m_aSelectedRows.clear();
            }
            if(m_nCurRow>0)
            {
                --m_nCurRow;
                m_aSelectedRows.push_back(m_nCurRow);
                invalidateSelectedRegion(m_nCurRow, m_nCurRow, rCells);
            }
            else
            {
                m_aSelectedRows.push_back(m_nCurRow);
                invalidateSelectedRegion(m_nCurRow, m_nCurRow, rCells);
            }
            ensureVisible(m_nCurColumn,m_nCurRow,false);
            selectionChanged = true;
            bSuccess = true;
        }
        else
        {
            if ( m_nCurRow > 0 )
                bSuccess = goTo( m_nCurColumn, m_nCurRow - 1 );
        }
        break;
        case cursorLeft:
            if ( m_nCurColumn > 0 )
                bSuccess = goTo( m_nCurColumn - 1, m_nCurRow );
            else
                if ( ( m_nCurColumn == 0) && ( m_nCurRow > 0 ) )
                    bSuccess = goTo( m_nColumnCount - 1, m_nCurRow - 1 );
            break;

        case cursorRight:
            if ( m_nCurColumn < m_nColumnCount - 1 )
                bSuccess = goTo( m_nCurColumn + 1, m_nCurRow );
            else
                if ( ( m_nCurColumn == m_nColumnCount - 1 ) && ( m_nCurRow < m_nRowCount - 1 ) )
                    bSuccess = goTo( 0, m_nCurRow + 1 );
            break;

        case cursorToLineStart:
            bSuccess = goTo( 0, m_nCurRow );
            break;

        case cursorToLineEnd:
            bSuccess = goTo( m_nColumnCount - 1, m_nCurRow );
            break;

        case cursorToFirstLine:
            bSuccess = goTo( m_nCurColumn, 0 );
            break;

        case cursorToLastLine:
            bSuccess = goTo( m_nCurColumn, m_nRowCount - 1 );
            break;

        case cursorPageUp:
        {
            RowPos nNewRow = ::std::max( (RowPos)0, m_nCurRow - impl_getVisibleRows( false ) );
            bSuccess = goTo( m_nCurColumn, nNewRow );
        }
        break;

        case cursorPageDown:
        {
            RowPos nNewRow = ::std::min( m_nRowCount - 1, m_nCurRow + impl_getVisibleRows( false ) );
            bSuccess = goTo( m_nCurColumn, nNewRow );
        }
        break;

        case cursorTopLeft:
            bSuccess = goTo( 0, 0 );
            break;

        case cursorBottomRight:
            bSuccess = goTo( m_nColumnCount - 1, m_nRowCount - 1 );
            break;

        case cursorSelectRow:
        {
            if(m_pSelEngine->GetSelectionMode() == NO_SELECTION)
                return bSuccess = false;
            //pos is the position of the current row in the vector of selected rows, if current row is selected
            int pos = getRowSelectedNumber(m_aSelectedRows, m_nCurRow);
            //if current row is selected, it should be deselected, when ALT+SPACE are pressed
            if(pos>-1)
            {
                m_aSelectedRows.erase(m_aSelectedRows.begin()+pos);
                if(m_aSelectedRows.empty() && m_nAnchor != -1)
                    m_nAnchor = -1;
            }
            //else select the row->put it in the vector
            else
                m_aSelectedRows.push_back(m_nCurRow);
            invalidateSelectedRegion(m_nCurRow, m_nCurRow, rCells);
            selectionChanged = true;
            bSuccess = true;
        }
            break;
        case cursorSelectRowUp:
        {
            if(m_pSelEngine->GetSelectionMode() == NO_SELECTION)
                return bSuccess = false;
            else if(m_pSelEngine->GetSelectionMode() == SINGLE_SELECTION)
            {
                //if there are other selected rows, deselect them
                return false;
            }
            else
            {
                //there are other selected rows
                if(m_aSelectedRows.size()>0)
                {
                    //the anchor wasn't set -> a region is not selected, that's why clear all selection
                    //and select the current row
                    if(m_nAnchor==-1)
                    {
                        for(std::vector<RowPos>::iterator it=m_aSelectedRows.begin();
                            it!=m_aSelectedRows.end();++it)
                        {
                            invalidateSelectedRegion(*it, *it, rCells);
                        }
                        m_aSelectedRows.clear();
                        m_aSelectedRows.push_back(m_nCurRow);
                        invalidateSelectedRegion(m_nCurRow, m_nCurRow, rCells);
                    }
                    else
                    {
                        //a region is already selected, prevRow is last selected row and the row above - nextRow - should be selected
                        int prevRow = getRowSelectedNumber(m_aSelectedRows, m_nCurRow);
                        int nextRow = getRowSelectedNumber(m_aSelectedRows, m_nCurRow-1);
                        if(prevRow>-1)
                         {
                             //if m_nCurRow isn't the upper one, can move up, otherwise not
                            if(m_nCurRow>0)
                                 m_nCurRow--;
                             else
                                 return bSuccess = true;
                             //if nextRow already selected, deselect it, otherwise select it
                             if(nextRow>-1 && m_aSelectedRows[nextRow] == m_nCurRow)
                             {
                                 m_aSelectedRows.erase(m_aSelectedRows.begin()+prevRow);
                                 invalidateSelectedRegion(m_nCurRow+1, m_nCurRow+1, rCells);
                             }
                             else
                            {
                                 m_aSelectedRows.push_back(m_nCurRow);
                                 invalidateSelectedRegion(m_nCurRow, m_nCurRow, rCells);
                             }
                         }
                        else
                        {
                            if(m_nCurRow>0)
                            {
                                m_aSelectedRows.push_back(m_nCurRow);
                                m_nCurRow--;
                                m_aSelectedRows.push_back(m_nCurRow);
                                invalidateSelectedRegion(m_nCurRow+1, m_nCurRow, rCells);
                            }
                        }
                    }
                }
                else
                {
                    //if nothing is selected and the current row isn't the upper one
                    //select the current and one row above
                    //otherwise select only the upper row
                    if(m_nCurRow>0)
                    {
                        m_aSelectedRows.push_back(m_nCurRow);
                        m_nCurRow--;
                        m_aSelectedRows.push_back(m_nCurRow);
                        invalidateSelectedRegion(m_nCurRow+1, m_nCurRow, rCells);
                    }
                    else
                    {
                        m_aSelectedRows.push_back(m_nCurRow);
                        invalidateSelectedRegion(m_nCurRow, m_nCurRow, rCells);
                    }
                }
                m_pSelEngine->SetAnchor(TRUE);
                m_nAnchor = m_nCurRow;
                ensureVisible(m_nCurColumn, m_nCurRow, false);
                selectionChanged = true;
                bSuccess = true;
            }
        }
        break;
        case cursorSelectRowDown:
        {
            if(m_pSelEngine->GetSelectionMode() == NO_SELECTION)
                bSuccess = false;
            else if(m_pSelEngine->GetSelectionMode() == SINGLE_SELECTION)
            {
                bSuccess = false;
            }
            else
            {
                if(m_aSelectedRows.size()>0)
                {
                    //the anchor wasn't set -> a region is not selected, that's why clear all selection
                    //and select the current row
                    if(m_nAnchor==-1)
                    {
                        for(std::vector<RowPos>::iterator it=m_aSelectedRows.begin();
                            it!=m_aSelectedRows.end();++it)
                        {
                            invalidateSelectedRegion(*it, *it, rCells);
                        }
                        m_aSelectedRows.clear();
                        m_aSelectedRows.push_back(m_nCurRow);
                        invalidateSelectedRegion(m_nCurRow, m_nCurRow, rCells);
                        }
                    else
                    {
                        //a region is already selected, prevRow is last selected row and the row beneath - nextRow - should be selected
                        int prevRow = getRowSelectedNumber(m_aSelectedRows, m_nCurRow);
                        int nextRow = getRowSelectedNumber(m_aSelectedRows, m_nCurRow+1);
                        if(prevRow>-1)
                         {
                             //if m_nCurRow isn't the last one, can move down, otherwise not
                             if(m_nCurRow<m_nRowCount-1)
                                 m_nCurRow++;
                             else
                                return bSuccess = true;
                             //if next row already selected, deselect it, otherwise select it
                             if(nextRow>-1 && m_aSelectedRows[nextRow] == m_nCurRow)
                             {
                                 m_aSelectedRows.erase(m_aSelectedRows.begin()+prevRow);
                                 invalidateSelectedRegion(m_nCurRow-1, m_nCurRow-1, rCells);
                             }
                             else
                             {
                                 m_aSelectedRows.push_back(m_nCurRow);
                                 invalidateSelectedRegion(m_nCurRow, m_nCurRow, rCells);
                             }
                        }
                        else
                        {
                            if(m_nCurRow<m_nRowCount-1)
                            {
                                m_aSelectedRows.push_back(m_nCurRow);
                                m_nCurRow++;
                                m_aSelectedRows.push_back(m_nCurRow);
                                invalidateSelectedRegion(m_nCurRow-1, m_nCurRow, rCells);
                            }
                        }
                    }
                }
                else
                {
                    //there wasn't any selection, select current and row beneath, otherwise only row beneath
                    if(m_nCurRow<m_nRowCount-1)
                    {
                        m_aSelectedRows.push_back(m_nCurRow);
                        m_nCurRow++;
                        m_aSelectedRows.push_back(m_nCurRow);
                        invalidateSelectedRegion(m_nCurRow-1, m_nCurRow, rCells);
                    }
                    else
                    {
                        m_aSelectedRows.push_back(m_nCurRow);
                        invalidateSelectedRegion(m_nCurRow, m_nCurRow, rCells);
                    }
                }
                m_pSelEngine->SetAnchor(TRUE);
                m_nAnchor = m_nCurRow;
                ensureVisible(m_nCurColumn, m_nCurRow, false);
                selectionChanged = true;
                bSuccess = true;
            }
        }
        break;

        case cursorSelectRowAreaTop:
        {
            if(m_pSelEngine->GetSelectionMode() == NO_SELECTION)
                bSuccess = false;
            else if(m_pSelEngine->GetSelectionMode() == SINGLE_SELECTION)
                bSuccess = false;
            else
            {
                //select the region between the current and the upper row
                RowPos iter = m_nCurRow;
                invalidateSelectedRegion(m_nCurRow, 0, rCells);
                //put the rows in vector
                while(iter>=0)
                {
                    if ( !isRowSelected( iter ) )
                        m_aSelectedRows.push_back(iter);
                    --iter;
                }
                m_nCurRow = 0;
                m_nAnchor = m_nCurRow;
                m_pSelEngine->SetAnchor(TRUE);
                ensureVisible(m_nCurColumn, 0, false);
                selectionChanged = true;
                bSuccess = true;
            }
        }
        break;

        case cursorSelectRowAreaBottom:
        {
            if(m_pSelEngine->GetSelectionMode() == NO_SELECTION)
                return bSuccess = false;
            else if(m_pSelEngine->GetSelectionMode() == SINGLE_SELECTION)
                return bSuccess = false;
            //select the region between the current and the last row
            RowPos iter = m_nCurRow;
            invalidateSelectedRegion(m_nCurRow, m_nRowCount-1, rCells);
            //put the rows in the vector
            while(iter<=m_nRowCount)
            {
                if ( !isRowSelected( iter ) )
                    m_aSelectedRows.push_back(iter);
                ++iter;
            }
            m_nCurRow = m_nRowCount-1;
            m_nAnchor = m_nCurRow;
            m_pSelEngine->SetAnchor(TRUE);
            ensureVisible(m_nCurColumn, m_nRowCount-1, false);
            selectionChanged = true;
            bSuccess = true;
        }
        break;
        default:
            DBG_ERROR( "TableControl_Impl::dispatchAction: unsupported action!" );
            break;
        }

        if ( bSuccess && selectionChanged )
        {
            m_rAntiImpl.Select();
        }

        return bSuccess;
    }

    //------------------------------------------------------------------------------------------------------------------
    void TableControl_Impl::impl_ni_doSwitchCursor( bool _bShow )
    {
        PTableRenderer pRenderer = !!m_pModel ? m_pModel->getRenderer() : PTableRenderer();
        if ( !!pRenderer )
        {
            Rectangle aCellRect;
            impl_getCellRect( m_nCurColumn, m_nCurRow, aCellRect );
            if ( _bShow )
                pRenderer->ShowCellCursor( *m_pDataWindow, aCellRect );
            else
                pRenderer->HideCellCursor( *m_pDataWindow, aCellRect );
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void TableControl_Impl::impl_getCellRect( ColPos _nColumn, RowPos _nRow, Rectangle& _rCellRect ) const
    {
        DBG_CHECK_ME();

        if  (   !m_pModel
            ||  ( COL_INVALID == _nColumn )
            ||  ( ROW_INVALID == _nRow )
            )
        {
            _rCellRect.SetEmpty();
            return;
        }

        Rectangle aAllCells;
        impl_getAllVisibleCellsArea( aAllCells );

        TableCellGeometry aCell( *this, aAllCells, _nColumn, _nRow );
        _rCellRect = aCell.getRect();
    }

    //------------------------------------------------------------------------------------------------------------------
    RowPos TableControl_Impl::getRowAtPoint( const Point& rPoint ) const
    {
        DBG_CHECK_ME();

        if ( ( rPoint.Y() >= 0 ) && ( rPoint.Y() < m_nColHeaderHeightPixel ) )
            return ROW_COL_HEADERS;

        Rectangle aAllCellsArea;
        impl_getAllVisibleCellsArea( aAllCellsArea );

        TableRowGeometry aHitTest( *this, aAllCellsArea, ROW_COL_HEADERS );
        while ( aHitTest.isValid() )
        {
            if ( aHitTest.getRect().IsInside( rPoint ) )
                return aHitTest.getRow();
            aHitTest.moveDown();
        }
        return ROW_INVALID;
    }

    //------------------------------------------------------------------------------------------------------------------
    ColPos TableControl_Impl::getColAtPoint( const Point& rPoint ) const
    {
        DBG_CHECK_ME();

        if ( ( rPoint.X() >= 0 ) && ( rPoint.X() < m_nRowHeaderWidthPixel ) )
            return COL_ROW_HEADERS;

        Rectangle aAllCellsArea;
        impl_getAllVisibleCellsArea( aAllCellsArea );

        TableColumnGeometry aHitTest( *this, aAllCellsArea, COL_ROW_HEADERS );
        while ( aHitTest.isValid() )
        {
            if ( aHitTest.getRect().IsInside( rPoint ) )
                return aHitTest.getCol();
            aHitTest.moveRight();
        }

        return COL_INVALID;
    }

    //------------------------------------------------------------------------------------------------------------------
    PTableModel TableControl_Impl::getModel() const
    {
        return m_pModel;
    }

    //------------------------------------------------------------------------------------------------------------------
    RowPos TableControl_Impl::getCurrentColumn() const
    {
        return m_nCurColumn;
    }

    //------------------------------------------------------------------------------------------------------------------
    RowPos TableControl_Impl::getCurrentRow() const
    {
        return m_nCurRow;
    }

    //------------------------------------------------------------------------------------------------------------------
    void TableControl_Impl::activateCellAt(const Point& rPoint)
    {
        DBG_CHECK_ME();

        TempHideCursor aHideCursor( *this );

        const RowPos newRowPos = getRowAtPoint( rPoint );
        const ColPos newColPos = getColAtPoint( rPoint );

        if ( ( newRowPos != ROW_INVALID ) && ( newColPos != COL_INVALID ) )
        {
            m_nCurColumn = newColPos;
            m_nCurRow = newRowPos;
            ensureVisible( m_nCurColumn, m_nCurRow, true );
        }
    }
    //------------------------------------------------------------------------------------------------------------------
    void TableControl_Impl::invalidateSelectedRegion(RowPos _nPrevRow, RowPos _nCurRow, Rectangle& _rCellRect)
    {
        DBG_CHECK_ME();
        Rectangle aAllCells;
        //get the visible area of the table control and set the Left and right border of the region to be repainted
        impl_getAllVisibleCellsArea( aAllCells );
        _rCellRect.Left() = aAllCells.Left();
        _rCellRect.Right() = aAllCells.Right();
        //if only one row is selected
        if(_nPrevRow == _nCurRow)
        {
            Rectangle aCellRect;
            impl_getCellRect( m_nCurColumn, _nCurRow, aCellRect );
            _rCellRect.Top() = aCellRect.Top();
            _rCellRect.Bottom() = aCellRect.Bottom();
        }
        //if the region is above the current row
        else if(_nPrevRow < _nCurRow )
        {
            Rectangle aCellRect;
            impl_getCellRect( m_nCurColumn, _nPrevRow, aCellRect );
            _rCellRect.Top() = aCellRect.Top();
            impl_getCellRect( m_nCurColumn, _nCurRow, aCellRect );
            _rCellRect.Bottom() = aCellRect.Bottom();
        }
        //if the region is beneath the current row
        else
        {
            Rectangle aCellRect;
            impl_getCellRect( m_nCurColumn, _nCurRow, aCellRect );
            _rCellRect.Top() = aCellRect.Top();
            impl_getCellRect( m_nCurColumn, _nPrevRow, aCellRect );
            _rCellRect.Bottom() = aCellRect.Bottom();
        }
        m_pDataWindow->Invalidate(_rCellRect);
    }
    //------------------------------------------------------------------------------------------------------------------
    //this method is to be called, when a new row is added
    void TableControl_Impl::invalidateRowRange( RowPos const i_firstRow, RowPos const i_lastRow )
    {
        if ( m_nCursorHidden == 2 )
            // WTF? what kind of hack is this?
            --m_nCursorHidden;

        RowPos const firstRow = i_firstRow < m_nTopRow ? m_nTopRow : i_firstRow;
        RowPos const lastVisibleRow = m_nTopRow + impl_getVisibleRows( true ) - 1;
        RowPos const lastRow = ( ( i_lastRow == ROW_INVALID ) || ( i_lastRow > lastVisibleRow ) ) ? lastVisibleRow : i_lastRow;

        Rectangle aInvalidateRect;

        Rectangle aVisibleCellsArea;
        impl_getAllVisibleCellsArea( aVisibleCellsArea );

        TableRowGeometry aRow( *this, aVisibleCellsArea, firstRow, true );
        while ( aRow.isValid() && ( aRow.getRow() <= lastRow ) )
        {
            aInvalidateRect.Union( aRow.getRect() );
            aRow.moveDown();
        }

        if ( i_lastRow == ROW_INVALID )
            aInvalidateRect.Bottom() = m_pDataWindow->GetOutputSizePixel().Height();

        m_pDataWindow->Invalidate( aInvalidateRect );
    }

    //------------------------------------------------------------------------------
    void TableControl_Impl::checkCursorPosition()
    {
        DBG_CHECK_ME();

        TableSize nVisibleRows = impl_getVisibleRows(true);
        TableSize nVisibleCols = impl_getVisibleColumns(true);
        if  (   ( m_nTopRow + nVisibleRows > m_nRowCount )
            &&  ( m_nRowCount >= nVisibleRows )
            )
        {
            --m_nTopRow;
        }
        else
        {
            m_nTopRow = 0;
        }

        if  (   ( m_nLeftColumn + nVisibleCols > m_nColumnCount )
            &&  ( m_nColumnCount >= nVisibleCols )
            )
        {
            --m_nLeftColumn;
        }
        else
        {
            m_nLeftColumn = 0;
        }

        m_pDataWindow->Invalidate();
    }

    //--------------------------------------------------------------------
    TableSize TableControl_Impl::impl_getVisibleRows( bool _bAcceptPartialRow ) const
    {
        DBG_CHECK_ME();

        DBG_ASSERT( m_pDataWindow, "TableControl_Impl::impl_getVisibleRows: no data window!" );

        return lcl_getRowsFittingInto(
            m_pDataWindow->GetOutputSizePixel().Height() - m_nColHeaderHeightPixel,
            m_nRowHeightPixel,
            _bAcceptPartialRow
        );
    }

    //--------------------------------------------------------------------
    TableSize TableControl_Impl::impl_getVisibleColumns( bool _bAcceptPartialCol ) const
    {
        DBG_CHECK_ME();

        DBG_ASSERT( m_pDataWindow, "TableControl_Impl::impl_getVisibleColumns: no data window!" );

        return lcl_getColumnsVisibleWithin(
            Rectangle( Point( 0, 0 ), m_pDataWindow->GetOutputSizePixel() ),
            m_nLeftColumn,
            *this,
            _bAcceptPartialCol
        );
    }

    //--------------------------------------------------------------------
    bool TableControl_Impl::goTo( ColPos _nColumn, RowPos _nRow )
    {
        DBG_CHECK_ME();

        // TODO: give veto listeners a chance

        if  (  ( _nColumn < 0 ) || ( _nColumn >= m_nColumnCount )
            || ( _nRow < 0 ) || ( _nRow >= m_nRowCount )
            )
        {
            OSL_ENSURE( false, "TableControl_Impl::goTo: invalid row or column index!" );
            return false;
        }

        TempHideCursor aHideCursor( *this );
        m_nCurColumn = _nColumn;
        m_nCurRow = _nRow;

        // ensure that the new cell is visible
        ensureVisible( m_nCurColumn, m_nCurRow, false );
        return true;
    }

    //--------------------------------------------------------------------
    void TableControl_Impl::ensureVisible( ColPos _nColumn, RowPos _nRow, bool _bAcceptPartialVisibility )
    {
        DBG_CHECK_ME();
        DBG_ASSERT( ( _nColumn >= 0 ) && ( _nColumn < m_nColumnCount )
                 && ( _nRow >= 0 ) && ( _nRow < m_nRowCount ),
                 "TableControl_Impl::ensureVisible: invalid coordinates!" );

        TempHideCursor aHideCursor( *this );

        if ( _nColumn < m_nLeftColumn )
            impl_scrollColumns( _nColumn - m_nLeftColumn );
        else
        {
            TableSize nVisibleColumns = impl_getVisibleColumns( _bAcceptPartialVisibility );
            if ( _nColumn > m_nLeftColumn + nVisibleColumns - 1 )
            {
                impl_scrollColumns( _nColumn - ( m_nLeftColumn + nVisibleColumns - 1 ) );
                // TODO: since not all columns have the same width, this might in theory result
                // in the column still not being visible.
            }
        }

        if ( _nRow < m_nTopRow )
            impl_scrollRows( _nRow - m_nTopRow );
        else
        {
            TableSize nVisibleRows = impl_getVisibleRows( _bAcceptPartialVisibility );
            if ( _nRow > m_nTopRow + nVisibleRows - 1 )
                impl_scrollRows( _nRow - ( m_nTopRow + nVisibleRows - 1 ) );
        }
    }

    //--------------------------------------------------------------------
    ::rtl::OUString TableControl_Impl::getCellContentAsString( RowPos const i_row, ColPos const i_col )
    {
        ::com::sun::star::uno::Any content;
        m_pModel->getCellContent( i_col, i_row, content );
        return CellValueConversion::convertToString( content );
    }

    //--------------------------------------------------------------------
    TableSize TableControl_Impl::impl_ni_ScrollRows( TableSize _nRowDelta )
    {
        // compute new top row
        RowPos nNewTopRow =
            ::std::max(
                ::std::min( (RowPos)( m_nTopRow + _nRowDelta ), (RowPos)( m_nRowCount - 1 ) ),
                (RowPos)0
            );

        RowPos nOldTopRow = m_nTopRow;
        m_nTopRow = nNewTopRow;

        // if updates are enabled currently, scroll the viewport
        if ( m_nTopRow != nOldTopRow )
        {
            DBG_SUSPEND_INV( INV_SCROLL_POSITION );
            TempHideCursor aHideCursor( *this );
            // TODO: call a onStartScroll at our listener (or better an own onStartScroll,
            // which hides the cursor and then calls the listener)
            // Same for onEndScroll

            // scroll the view port, if possible
            long nPixelDelta = m_nRowHeightPixel * ( m_nTopRow - nOldTopRow );

            Rectangle aDataArea( Point( 0, m_nColHeaderHeightPixel ), m_pDataWindow->GetOutputSizePixel() );

            if  (   m_pDataWindow->GetBackground().IsScrollable()
                &&  abs( nPixelDelta ) < aDataArea.GetHeight()
                )
            {
                m_pDataWindow->Scroll( 0, (long)-nPixelDelta, aDataArea, SCROLL_CLIP | SCROLL_UPDATE | SCROLL_CHILDREN);
            }
            else
                m_pDataWindow->Invalidate( INVALIDATE_UPDATE );

            // update the position at the vertical scrollbar
            m_pVScroll->SetThumbPos( m_nTopRow );
        }

        // The scroll bar availaility might change when we scrolled. This is because we do not hide
        // the scrollbar when it is, in theory, unnecessary, but currently at a position > 0. In this case, it will
        // be auto-hidden when it's scrolled back to pos 0.
        if ( m_nTopRow == 0 )
            m_rAntiImpl.PostUserEvent( LINK( this, TableControl_Impl, OnUpdateScrollbars ) );

        return (TableSize)( m_nTopRow - nOldTopRow );
    }

    //--------------------------------------------------------------------
    TableSize TableControl_Impl::impl_scrollRows( TableSize const i_rowDelta )
    {
        DBG_CHECK_ME();
        return impl_ni_ScrollRows( i_rowDelta );
    }

    //--------------------------------------------------------------------
    TableSize TableControl_Impl::impl_ni_ScrollColumns( TableSize _nColumnDelta )
    {
        // compute new left column
        const ColPos nNewLeftColumn =
            ::std::max(
                ::std::min( (ColPos)( m_nLeftColumn + _nColumnDelta ), (ColPos)( m_nColumnCount - 1 ) ),
                (ColPos)0
            );

        const ColPos nOldLeftColumn = m_nLeftColumn;
        m_nLeftColumn = nNewLeftColumn;

        // if updates are enabled currently, scroll the viewport
        if ( m_nLeftColumn != nOldLeftColumn )
        {
            DBG_SUSPEND_INV( INV_SCROLL_POSITION );
            TempHideCursor aHideCursor( *this );
            // TODO: call a onStartScroll at our listener (or better an own onStartScroll,
            // which hides the cursor and then calls the listener)
            // Same for onEndScroll

            // scroll the view port, if possible
            const Rectangle aDataArea( Point( m_nRowHeaderWidthPixel, 0 ), m_pDataWindow->GetOutputSizePixel() );

            long nPixelDelta =
                    m_aColumnWidths[ nOldLeftColumn ].getStart()
                -   m_aColumnWidths[ m_nLeftColumn ].getStart();

            // update our column positions
            // Do this *before* scrolling, as SCROLL_UPDATE will trigger a paint, which already needs the correct
            // information in m_aColumnWidths
            for (   ColumnPositions::iterator colPos = m_aColumnWidths.begin();
                    colPos != m_aColumnWidths.end();
                    ++colPos
                 )
            {
                colPos->move( nPixelDelta );
            }

            // scroll the window content (if supported and possible), or invalidate the complete window
            if  (   m_pDataWindow->GetBackground().IsScrollable()
                &&  abs( nPixelDelta ) < aDataArea.GetWidth()
                )
            {
                m_pDataWindow->Scroll( nPixelDelta, 0, aDataArea, SCROLL_CLIP | SCROLL_UPDATE );
            }
            else
                m_pDataWindow->Invalidate( INVALIDATE_UPDATE );

            // update the position at the horizontal scrollbar
            m_pHScroll->SetThumbPos( m_nLeftColumn );
        }

        // The scroll bar availaility might change when we scrolled. This is because we do not hide
        // the scrollbar when it is, in theory, unnecessary, but currently at a position > 0. In this case, it will
        // be auto-hidden when it's scrolled back to pos 0.
        if ( m_nLeftColumn == 0 )
            m_rAntiImpl.PostUserEvent( LINK( this, TableControl_Impl, OnUpdateScrollbars ) );

        return (TableSize)( m_nLeftColumn - nOldLeftColumn );
    }

    //------------------------------------------------------------------------------------------------------------------
    TableSize TableControl_Impl::impl_scrollColumns( TableSize const i_columnDelta )
    {
        DBG_CHECK_ME();
        return impl_ni_ScrollColumns( i_columnDelta );
    }

    //------------------------------------------------------------------------------------------------------------------
    SelectionEngine* TableControl_Impl::getSelEngine()
    {
        return m_pSelEngine;
    }

    //------------------------------------------------------------------------------------------------------------------
    ScrollBar* TableControl_Impl::getHorzScrollbar()
    {
        return m_pHScroll;
    }

    //------------------------------------------------------------------------------------------------------------------
    ScrollBar* TableControl_Impl::getVertScrollbar()
    {
        return m_pVScroll;
    }

    //------------------------------------------------------------------------------------------------------------------
    bool TableControl_Impl::isRowSelected( RowPos i_row ) const
    {
        return ::std::find( m_aSelectedRows.begin(), m_aSelectedRows.end(), i_row ) != m_aSelectedRows.end();
    }

    //------------------------------------------------------------------------------------------------------------------
    RowPos TableControl_Impl::getSelectedRowIndex( size_t const i_selectionIndex ) const
    {
        if ( i_selectionIndex < m_aSelectedRows.size() )
            return m_aSelectedRows[ i_selectionIndex ];
        return ROW_INVALID;
    }

    //------------------------------------------------------------------------------------------------------------------
    int TableControl_Impl::getRowSelectedNumber(const ::std::vector<RowPos>& selectedRows, RowPos current)
    {
        std::vector<RowPos>::const_iterator it = ::std::find(selectedRows.begin(),selectedRows.end(),current);
        if ( it != selectedRows.end() )
        {
            return it - selectedRows.begin();
        }
        return -1;
    }

    //--------------------------------------------------------------------
    ColPos TableControl_Impl::impl_getColumnForOrdinate( long const i_ordinate ) const
    {
        DBG_CHECK_ME();

        if ( m_aColumnWidths.empty() )
            return COL_INVALID;

        ColumnPositions::const_iterator lowerBound = ::std::lower_bound(
            m_aColumnWidths.begin(),
            m_aColumnWidths.end(),
            i_ordinate + 1,
            ColumnInfoPositionLess()
        );
        if ( lowerBound == m_aColumnWidths.end() )
        {
            // point is *behind* the start of the last column ...
            if ( i_ordinate < m_aColumnWidths.rbegin()->getEnd() )
                // ... but still before its end
                return m_nColumnCount - 1;
            return COL_INVALID;
        }
        return lowerBound - m_aColumnWidths.begin();
    }

    //--------------------------------------------------------------------
    bool TableControl_Impl::markRowAsDeselected( RowPos const i_rowIndex )
    {
        DBG_CHECK_ME();

        ::std::vector< RowPos >::iterator selPos = ::std::find( m_aSelectedRows.begin(), m_aSelectedRows.end(), i_rowIndex );
        if ( selPos == m_aSelectedRows.end() )
            return false;

        m_aSelectedRows.erase( selPos );
        return true;
    }

    //--------------------------------------------------------------------
    bool TableControl_Impl::markRowAsSelected( RowPos const i_rowIndex )
    {
        DBG_CHECK_ME();

        if ( isRowSelected( i_rowIndex ) )
            return false;

        SelectionMode const eSelMode = getSelEngine()->GetSelectionMode();
        switch ( eSelMode )
        {
        case SINGLE_SELECTION:
            if ( !m_aSelectedRows.empty() )
            {
                OSL_ENSURE( m_aSelectedRows.size() == 1, "TableControl::markRowAsSelected: SingleSelection with more than one selected element?" );
                m_aSelectedRows[0] = i_rowIndex;
                break;
            }
            // fall through

        case MULTIPLE_SELECTION:
            m_aSelectedRows.push_back( i_rowIndex );
            break;

        default:
            OSL_ENSURE( false, "TableControl_Impl::markRowAsSelected: unsupported selection mode!" );
            return false;
        }

        return true;
    }

    //--------------------------------------------------------------------
    bool TableControl_Impl::markAllRowsAsDeselected()
    {
        if ( m_aSelectedRows.empty() )
            return false;

        m_aSelectedRows.clear();
        return true;
    }

    //--------------------------------------------------------------------
    bool TableControl_Impl::markAllRowsAsSelected()
    {
        DBG_CHECK_ME();

        SelectionMode const eSelMode = getSelEngine()->GetSelectionMode();
        ENSURE_OR_RETURN_FALSE( eSelMode == MULTIPLE_SELECTION, "TableControl_Impl::markAllRowsAsSelected: unsupported selection mode!" );

        if ( m_aSelectedRows.size() == size_t( m_pModel->getRowCount() ) )
        {
        #if OSL_DEBUG_LEVEL > 0
            for ( TableSize row = 0; row < m_pModel->getRowCount(); ++row )
            {
                OSL_ENSURE( isRowSelected( row ), "TableControl_Impl::markAllRowsAsSelected: inconsistency in the selected rows!" );
            }
        #endif
            // already all rows marked as selected
            return false;
        }

        m_aSelectedRows.clear();
        for ( RowPos i=0; i < m_pModel->getRowCount(); ++i )
            m_aSelectedRows.push_back(i);

        return true;
    }

    //--------------------------------------------------------------------
    void TableControl_Impl::resizeColumn( const Point& rPoint )
    {
        Pointer aNewPointer( POINTER_ARROW );
        const ColPos hitColumn = impl_getColumnForOrdinate( rPoint.X() );
        if ( m_bResizingColumn )
        {
            const ColumnWidthInfo& rColInfo( m_aColumnWidths[ m_nResizingColumn ] );
            if  (   ( rPoint.X() > m_pDataWindow->GetOutputSizePixel().Width() )
                ||  ( rPoint.X() < rColInfo.getStart() )
                )
            {
                aNewPointer = Pointer( POINTER_NOTALLOWED );
            }
            else
            {
                aNewPointer = Pointer( POINTER_HSPLIT );
            }
            m_pDataWindow->HideTracking();

            int lineHeight = m_nColHeaderHeightPixel;
            lineHeight += m_nRowHeightPixel * m_nRowCount;
            const int gridHeight = m_pDataWindow->GetOutputSizePixel().Height();
            if ( lineHeight >= gridHeight )
                lineHeight = gridHeight;

            m_pDataWindow->ShowTracking(
                Rectangle(
                    Point( rPoint.X(), 0 ),
                    Size( 1, lineHeight )
                ),
                SHOWTRACK_SPLIT | SHOWTRACK_WINDOW
            );
        }
        else if ( hitColumn != COL_INVALID )
        {
            // hit test for the column separator
            const PColumnModel pColumn = m_pModel->getColumnModel( hitColumn );
            const ColumnWidthInfo& rColInfo( m_aColumnWidths[ hitColumn ] );
            if  (   ( rColInfo.getEnd() - 2 <= rPoint.X() )
                &&  ( rColInfo.getEnd() + 2 > rPoint.X() )
                &&  pColumn->isResizable()
                )
                aNewPointer = Pointer( POINTER_HSPLIT );
        }

        m_pDataWindow->SetPointer( aNewPointer );
    }

    //--------------------------------------------------------------------
    bool TableControl_Impl::checkResizeColumn( const Point& rPoint )
    {
        m_bResizingGrid = false;

        if ( m_bResizingColumn )
            return true;

        const ColPos hitColumn = impl_getColumnForOrdinate( rPoint.X() );
        if ( hitColumn == COL_INVALID )
            return false;

        const PColumnModel pColumn = m_pModel->getColumnModel( hitColumn );
        const ColumnWidthInfo& rColInfo( m_aColumnWidths[ hitColumn ] );

        // hit test for the column separator
        if  (   ( rColInfo.getEnd() - 2 <= rPoint.X() )
            &&  ( rColInfo.getEnd() + 2 > rPoint.X() )
            &&  pColumn->isResizable()
            )
        {
            m_nResizingColumn = hitColumn;
            m_pDataWindow->CaptureMouse();
            m_bResizingColumn = true;
        }
        return m_bResizingColumn;
    }
    //--------------------------------------------------------------------
    bool TableControl_Impl::endResizeColumn( const Point& rPoint )
    {
        DBG_CHECK_ME();
        ENSURE_OR_RETURN_FALSE( m_bResizingColumn, "TableControl_Impl::endResizeColumn: not resizing currently!" );

        m_pDataWindow->HideTracking();
        const PColumnModel pColumn = m_pModel->getColumnModel( m_nResizingColumn );
        const long maxWidthLogical = pColumn->getMaxWidth();
        const long minWidthLogical = pColumn->getMinWidth();

        // new position of mouse
        const long requestedEnd = rPoint.X();

        // old position of right border
        const long oldEnd = m_aColumnWidths[ m_nResizingColumn ].getEnd();

        // position of left border if cursor in the to-be-resized column
        const long columnStart = m_aColumnWidths[ m_nResizingColumn ].getStart();
        const long requestedWidth = requestedEnd - columnStart;
            // TODO: this is not correct, strictly: It assumes that the mouse was pressed exactly on the "end" pos,
            // but for a while now, we have relaxed this, and allow clicking a few pixels aside, too

        if ( requestedEnd >= columnStart )
        {
            long requestedWidthLogical = m_rAntiImpl.PixelToLogic( Size( requestedWidth, 0 ), MAP_APPFONT ).Width();
            // respect column width limits
            if ( oldEnd > requestedEnd )
            {
                // column has become smaller, check against minimum width
                if ( ( minWidthLogical != 0 ) && ( requestedWidthLogical < minWidthLogical ) )
                    requestedWidthLogical = minWidthLogical;
            }
            else if ( oldEnd < requestedEnd )
            {
                // column has become larger, check against max width
                if ( ( maxWidthLogical != 0 ) && ( requestedWidthLogical >= maxWidthLogical ) )
                    requestedWidthLogical = maxWidthLogical;
            }
            pColumn->setPreferredWidth( requestedWidthLogical );
        }

        impl_ni_updateColumnWidths();
        impl_ni_updateScrollbars();
        m_pDataWindow->Invalidate( INVALIDATE_UPDATE );
        m_pDataWindow->SetPointer( Pointer() );
        m_bResizingColumn = false;
        m_bResizingGrid = true;

        m_pDataWindow->ReleaseMouse();
        return m_bResizingColumn;
    }

    //--------------------------------------------------------------------
    Rectangle TableControl_Impl::calcHeaderRect(bool bColHeader)
    {
        Rectangle aRectTable, aRectTableWithHeaders;
        impl_getAllVisibleDataCellArea(aRectTable);
        impl_getAllVisibleCellsArea(aRectTableWithHeaders);
        Size aSizeTable(aRectTable.GetSize());
        Size aSizeTableWithHeaders(aRectTableWithHeaders.GetSize());
        if(bColHeader)
            return Rectangle(aRectTableWithHeaders.TopLeft(),Size(aSizeTableWithHeaders.Width()-aSizeTable.Width(), aSizeTableWithHeaders.Height()));
        else
            return Rectangle(aRectTableWithHeaders.TopLeft(),Size(aSizeTableWithHeaders.Width(), aSizeTableWithHeaders.Height()-aSizeTable.Height()));
    }

    //--------------------------------------------------------------------
    Rectangle TableControl_Impl::calcTableRect()
    {
        Rectangle aRect;
        impl_getAllVisibleDataCellArea(aRect);
        return aRect;
    }

    //--------------------------------------------------------------------
    IMPL_LINK( TableControl_Impl, OnUpdateScrollbars, void*, /**/ )
    {
        DBG_CHECK_ME();
        impl_ni_updateScrollbars();
        return 1L;
    }

    //--------------------------------------------------------------------
    IMPL_LINK( TableControl_Impl, OnScroll, ScrollBar*, _pScrollbar )
    {
        DBG_ASSERT( ( _pScrollbar == m_pVScroll ) || ( _pScrollbar == m_pHScroll ),
            "TableControl_Impl::OnScroll: where did this come from?" );

        if ( _pScrollbar == m_pVScroll )
            impl_ni_ScrollRows( _pScrollbar->GetDelta() );
        else
            impl_ni_ScrollColumns( _pScrollbar->GetDelta() );

        return 0L;
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XAccessible > TableControl_Impl::getAccessible( Window& i_parentWindow )
    {
        DBG_TESTSOLARMUTEX();
        if ( m_pAccessibleTable == NULL )
        {
            Reference< XAccessible > const xAccParent = i_parentWindow.GetAccessible();
            if ( xAccParent.is() )
            {
                m_pAccessibleTable = m_aFactoryAccess.getFactory().createAccessibleTableControl(
                    xAccParent, m_rAntiImpl
                );
            }
        }

        Reference< XAccessible > xAccessible;
        if ( m_pAccessibleTable )
            xAccessible = m_pAccessibleTable->getMyself();
        return xAccessible;
    }

    //------------------------------------------------------------------------------------------------------------------
    void TableControl_Impl::disposeAccessible()
    {
        if ( m_pAccessibleTable )
            m_pAccessibleTable->dispose();
        m_pAccessibleTable = NULL;
    }

    //------------------------------------------------------------------------------------------------------------------
    bool TableControl_Impl::impl_isAccessibleAlive() const
    {
        DBG_CHECK_ME();
        return ( NULL != m_pAccessibleTable ) && m_pAccessibleTable->isAlive();
    }

    //------------------------------------------------------------------------------------------------------------------
    void TableControl_Impl::impl_commitAccessibleEvent( sal_Int16 const i_eventID, Any const & i_newValue, Any const & i_oldValue )
    {
        DBG_CHECK_ME();
        if ( impl_isAccessibleAlive() )
             m_pAccessibleTable->commitEvent( i_eventID, i_newValue, i_oldValue );
    }

    //==================================================================================================================
    //= TableFunctionSet
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    TableFunctionSet::TableFunctionSet(TableControl_Impl* _pTableControl)
        :m_pTableControl( _pTableControl)
        ,m_nCurrentRow (-2)
    {
    }
    //------------------------------------------------------------------------------------------------------------------
    TableFunctionSet::~TableFunctionSet()
    {
    }
    //------------------------------------------------------------------------------------------------------------------
    void TableFunctionSet::BeginDrag()
    {
    }
    //------------------------------------------------------------------------------------------------------------------
    void TableFunctionSet::CreateAnchor()
    {
        m_pTableControl->m_nAnchor = m_pTableControl->m_nCurRow;
    }
    //------------------------------------------------------------------------------------------------------------------
    void TableFunctionSet::DestroyAnchor()
    {
        m_pTableControl->m_nAnchor = -1;
    }
    //------------------------------------------------------------------------------------------------------------------
    BOOL TableFunctionSet::SetCursorAtPoint(const Point& rPoint, BOOL bDontSelectAtCursor)
    {
        BOOL bHandled = FALSE;
        // newRow is the row which includes the point, m_nCurRow is the last selected row, before the mouse click
        const RowPos newRow = m_pTableControl->getRowAtPoint( rPoint );
        const ColPos newCol = m_pTableControl->getColAtPoint( rPoint );
        if ( ( newRow == ROW_INVALID ) || ( newCol == COL_INVALID ) )
            return FALSE;

        if ( bDontSelectAtCursor )
        {
            if ( m_pTableControl->m_aSelectedRows.size()>1 )
                m_pTableControl->m_pSelEngine->AddAlways(TRUE);
            bHandled = TRUE;
        }
        else if ( m_pTableControl->m_nAnchor == m_pTableControl->m_nCurRow )
        {
            //selecting region,
            int diff = m_pTableControl->m_nCurRow - newRow;
            //selected region lies above the last selection
            if( diff >= 0)
            {
                //put selected rows in vector
                while ( m_pTableControl->m_nAnchor >= newRow )
                {
                    bool isAlreadySelected = m_pTableControl->isRowSelected( m_pTableControl->m_nAnchor );
                    //if row isn't selected, put it in vector, otherwise don't put it there, because it will be twice there
                    if(!isAlreadySelected)
                        m_pTableControl->m_aSelectedRows.push_back(m_pTableControl->m_nAnchor);
                    m_pTableControl->m_nAnchor--;
                    diff--;
                }
                m_pTableControl->m_nAnchor++;
            }
            //selected region lies beneath the last selected row
            else
            {
                while ( m_pTableControl->m_nAnchor <= newRow )
                {
                    bool isAlreadySelected = m_pTableControl->isRowSelected( m_pTableControl->m_nAnchor );
                    if(!isAlreadySelected)
                        m_pTableControl->m_aSelectedRows.push_back(m_pTableControl->m_nAnchor);
                    m_pTableControl->m_nAnchor++;
                    diff++;
                }
                m_pTableControl->m_nAnchor--;
            }
            Rectangle aCellRect;
            m_pTableControl->invalidateSelectedRegion( m_pTableControl->m_nCurRow, newRow, aCellRect );
            bHandled = TRUE;
        }
        //no region selected
        else
        {
            if(m_pTableControl->m_aSelectedRows.empty())
                m_pTableControl->m_aSelectedRows.push_back( newRow );
            else
            {
                if(m_pTableControl->m_pSelEngine->GetSelectionMode()==SINGLE_SELECTION)
                {
                    DeselectAll();
                    m_pTableControl->m_aSelectedRows.push_back( newRow );
                }
                else
                {
                    bool isAlreadySelected = m_pTableControl->isRowSelected( newRow );
                    if ( !isAlreadySelected )
                        m_pTableControl->m_aSelectedRows.push_back( newRow );
                }
            }
            if(m_pTableControl->m_aSelectedRows.size()>1 && m_pTableControl->m_pSelEngine->GetSelectionMode()!=SINGLE_SELECTION)
                m_pTableControl->m_pSelEngine->AddAlways(TRUE);

            Rectangle aCellRect;
            m_pTableControl->invalidateSelectedRegion( newRow, newRow, aCellRect );
            bHandled = TRUE;
        }
        m_pTableControl->m_nCurRow = newRow;
        m_pTableControl->m_nCurColumn = newCol;
        m_pTableControl->ensureVisible( newCol, newRow, true );
        return bHandled;
    }
    //------------------------------------------------------------------------------------------------------------------
    BOOL TableFunctionSet::IsSelectionAtPoint( const Point& rPoint )
    {
        m_pTableControl->m_pSelEngine->AddAlways(FALSE);
        if(m_pTableControl->m_aSelectedRows.empty())
            return FALSE;
        else
        {
            RowPos curRow = m_pTableControl->getRowAtPoint( rPoint );
            m_pTableControl->m_nAnchor = -1;
            bool selected = m_pTableControl->isRowSelected( curRow );
            m_nCurrentRow = curRow;
            return selected;
        }
    }
    //------------------------------------------------------------------------------------------------------------------
    void TableFunctionSet::DeselectAtPoint( const Point& rPoint )
    {
        (void)rPoint;
        long pos = 0;
        long i = 0;
        Rectangle rCells;
        for(std::vector<RowPos>::iterator it=m_pTableControl->m_aSelectedRows.begin();
            it!=m_pTableControl->m_aSelectedRows.end();++it)
        {
            if(*it == m_nCurrentRow)
            {
                pos = i;
                m_pTableControl->invalidateSelectedRegion(*it, *it, rCells);
            }
            ++i;
        }
        m_pTableControl->m_aSelectedRows.erase(m_pTableControl->m_aSelectedRows.begin()+pos);
    }

    //------------------------------------------------------------------------------------------------------------------
    void TableFunctionSet::DeselectAll()
    {
        if(!m_pTableControl->m_aSelectedRows.empty())
        {
            Rectangle rCells;
            for(std::vector<RowPos>::iterator it=m_pTableControl->m_aSelectedRows.begin();
                it!=m_pTableControl->m_aSelectedRows.end();++it)
            {
                m_pTableControl->invalidateSelectedRegion(*it, *it, rCells);
            }
            m_pTableControl->m_aSelectedRows.clear();
        }
    }

//......................................................................................................................
} } // namespace svt::table
//......................................................................................................................
