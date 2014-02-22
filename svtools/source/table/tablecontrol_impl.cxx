/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "svtools/table/tablecontrol.hxx"
#include "svtools/table/defaultinputhandler.hxx"
#include "svtools/table/tablemodel.hxx"

#include "tabledatawindow.hxx"
#include "tablecontrol_impl.hxx"
#include "tablegeometry.hxx"

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChange.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChangeType.hpp>

#include <comphelper/flagguard.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/seleng.hxx>
#include <vcl/settings.hxx>
#include <rtl/ref.hxx>
#include <vcl/image.hxx>
#include <tools/diagnose_ex.h>

#include <functional>
#include <numeric>

#define MIN_COLUMN_WIDTH_PIXEL  4


namespace svt { namespace table
{


    using ::com::sun::star::accessibility::AccessibleTableModelChange;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::accessibility::XAccessible;
    using ::com::sun::star::uno::Reference;

    namespace AccessibleEventId = ::com::sun::star::accessibility::AccessibleEventId;
    namespace AccessibleTableModelChangeType = ::com::sun::star::accessibility::AccessibleTableModelChangeType;

    
    
    
    class SuppressCursor
    {
    private:
        ITableControl&  m_rTable;

    public:
        SuppressCursor( ITableControl& _rTable )
            :m_rTable( _rTable )
        {
            m_rTable.hideCursor();
        }
        ~SuppressCursor()
        {
            m_rTable.showCursor();
        }
    };

    
    
    
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
            OSL_FAIL( "EmptyTableModel::getColumnModel: invalid call!" );
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
        virtual ::boost::optional< ::Color >    getActiveSelectionBackColor() const
        {
            return ::boost::optional< ::Color >();
        }
        virtual ::boost::optional< ::Color >    getInactiveSelectionBackColor() const
        {
            return ::boost::optional< ::Color >();
        }
        virtual ::boost::optional< ::Color >    getActiveSelectionTextColor() const
        {
            return ::boost::optional< ::Color >();
        }
        virtual ::boost::optional< ::Color >    getInactiveSelectionTextColor() const
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
        virtual ITableDataSort* getSortAdapter()
        {
            return NULL;
        }
        virtual bool isEnabled() const
        {
            return true;
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
        virtual Any getRowHeading( RowPos const i_rowPos ) const
        {
            (void)i_rowPos;
            return Any();
        }
    };


    
    
    
    DBG_NAME( TableControl_Impl )

#ifdef DBG_UTIL
    
    
    
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

#ifdef DBG_UTIL
    
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

        if ( ( ( m_nCurColumn != COL_INVALID ) && !m_aColumnWidths.empty() && ( m_nCurColumn < 0 ) ) || ( m_nCurColumn >= (ColPos)m_aColumnWidths.size() ) )
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

        
        {
            if ( m_aSelectedRows.size() > size_t( m_pModel->getRowCount() ) )
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

        
        {
            TableMetrics nHeaderWidth = m_pModel->hasRowHeaders() ? m_pModel->getRowHeaderWidth() : 0;
            nHeaderWidth = m_rAntiImpl.LogicToPixel( Size( nHeaderWidth, 0 ), MAP_APPFONT ).Width();
            if ( nHeaderWidth != m_nRowHeaderWidthPixel )
                return "row header widths are inconsistent!";
        }

        
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
        ,m_bUpdatingColWidths   ( false                         )
        ,m_pAccessibleTable     ( NULL                          )
#ifdef DBG_UTIL
        ,m_nRequiredInvariants ( INV_SCROLL_POSITION )
#endif
    {
        DBG_CTOR( TableControl_Impl, TableControl_Impl_checkInvariants );
        m_pSelEngine = new SelectionEngine( m_pDataWindow.get(), m_pTableFunctionSet );
        m_pSelEngine->SetSelectionMode(SINGLE_SELECTION);
        m_pDataWindow->SetPosPixel( Point( 0, 0 ) );
        m_pDataWindow->Show();
    }

    
    TableControl_Impl::~TableControl_Impl()
    {
        DBG_DTOR( TableControl_Impl, TableControl_Impl_checkInvariants );

        DELETEZ( m_pVScroll );
        DELETEZ( m_pHScroll );
        DELETEZ( m_pScrollCorner );
        DELETEZ( m_pTableFunctionSet );
        DELETEZ( m_pSelEngine );
    }

    
    void TableControl_Impl::setModel( PTableModel _pModel )
    {
        DBG_CHECK_ME();

        SuppressCursor aHideCursor( *this );

        if ( !!m_pModel )
            m_pModel->removeTableModelListener( shared_from_this() );

        m_pModel = _pModel;
        if ( !m_pModel)
            m_pModel.reset( new EmptyTableModel );

        m_pModel->addTableModelListener( shared_from_this() );

        m_nCurRow = ROW_INVALID;
        m_nCurColumn = COL_INVALID;

        
        impl_ni_updateCachedModelValues();
        impl_ni_relayout();

        
        m_rAntiImpl.Invalidate();

        
        if ( m_nRowCount ) m_nCurRow = 0;
        if ( m_nColumnCount ) m_nCurColumn = 0;
    }

    
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

    
    void TableControl_Impl::rowsInserted( RowPos i_first, RowPos i_last )
    {
        DBG_CHECK_ME();
        OSL_PRECOND( i_last >= i_first, "TableControl_Impl::rowsInserted: invalid row indexes!" );

        TableSize const insertedRows = i_last - i_first + 1;

        
        bool const selectionChanged = lcl_adjustSelectedRows( m_aSelectedRows, i_first, insertedRows );

        
        m_nRowCount = m_pModel->getRowCount();

        
        if ( i_first <= m_nCurRow )
            goTo( m_nCurColumn, m_nCurRow + insertedRows );

        
        impl_ni_relayout();

        
        if ( impl_isAccessibleAlive() )
        {
            impl_commitAccessibleEvent( AccessibleEventId::TABLE_MODEL_CHANGED,
                makeAny( AccessibleTableModelChange( AccessibleTableModelChangeType::INSERT, i_first, i_last, 0, m_pModel->getColumnCount() ) ),
                Any()
            );
        }

        
        invalidateRowRange( i_first, ROW_INVALID );

        
        if ( selectionChanged )
            m_rAntiImpl.Select();
    }

    
    void TableControl_Impl::rowsRemoved( RowPos i_first, RowPos i_last )
    {
        sal_Int32 firstRemovedRow = i_first;
        sal_Int32 lastRemovedRow = i_last;

        
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

        
        m_nRowCount = m_pModel->getRowCount();

        
        if ( m_nCurRow >= m_nRowCount )
        {
            if ( m_nRowCount > 0 )
                goTo( m_nCurColumn, m_nRowCount - 1 );
            else
            {
                m_nCurRow = ROW_INVALID;
                m_nTopRow = 0;
            }
        }
        else if ( m_nRowCount == 0 )
        {
            m_nTopRow = 0;
        }


        
        impl_ni_relayout();

        
        if ( impl_isAccessibleAlive() )
        {
            commitTableEvent(
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

        
        invalidateRowRange( firstRemovedRow, ROW_INVALID );

        
        if ( selectionChanged )
            m_rAntiImpl.Select();
    }

    
    void TableControl_Impl::columnInserted( ColPos const i_colIndex )
    {
        m_nColumnCount = m_pModel->getColumnCount();
        impl_ni_relayout();

        m_rAntiImpl.Invalidate();

        OSL_UNUSED( i_colIndex );
   }

    
    void TableControl_Impl::columnRemoved( ColPos const i_colIndex )
    {
        m_nColumnCount = m_pModel->getColumnCount();

        
        if ( m_nCurColumn >= m_nColumnCount )
        {
            if ( m_nColumnCount > 0 )
                goTo( m_nCurColumn - 1, m_nCurRow );
            else
                m_nCurColumn = COL_INVALID;
        }

        impl_ni_relayout();

        m_rAntiImpl.Invalidate();

        OSL_UNUSED( i_colIndex );
    }

    
    void TableControl_Impl::allColumnsRemoved()
    {
        m_nColumnCount = m_pModel->getColumnCount();
        impl_ni_relayout();

        m_rAntiImpl.Invalidate();
    }

    
    void TableControl_Impl::cellsUpdated( ColPos const i_firstCol, ColPos i_lastCol, RowPos const i_firstRow, RowPos const i_lastRow )
    {
        invalidateRowRange( i_firstRow, i_lastRow );

        OSL_UNUSED( i_firstCol );
        OSL_UNUSED( i_lastCol );
    }

    
    void TableControl_Impl::tableMetricsChanged()
    {
        impl_ni_updateCachedTableMetrics();
        impl_ni_relayout();
        m_rAntiImpl.Invalidate();
    }

    
    void TableControl_Impl::impl_invalidateColumn( ColPos const i_column )
    {
        DBG_CHECK_ME();

        Rectangle const aAllCellsArea( impl_getAllVisibleCellsArea() );

        const TableColumnGeometry aColumn( *this, aAllCellsArea, i_column );
        if ( aColumn.isValid() )
            m_rAntiImpl.Invalidate( aColumn.getRect() );
    }

    
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
            {
                impl_ni_relayout( i_column );
                invalidate( TableAreaAll );
            }

            nGroup &= ~COL_ATTRS_WIDTH;
        }

        OSL_ENSURE( ( nGroup == COL_ATTRS_NONE ) || ( i_attributeGroup == COL_ATTRS_ALL ),
            "TableControl_Impl::columnChanged: don't know how to handle this change!" );
    }

    
    Rectangle TableControl_Impl::impl_getAllVisibleCellsArea() const
    {
        DBG_CHECK_ME();

        Rectangle aArea( Point( 0, 0 ), Size( 0, 0 ) );

        
        
        aArea.Right() = m_nRowHeaderWidthPixel;
        if ( !m_aColumnWidths.empty() )
        {
            
            
            const long nScrolledOutLeft = m_nLeftColumn == 0 ? 0 : m_aColumnWidths[ m_nLeftColumn - 1 ].getEnd();

            ColumnPositions::const_reverse_iterator loop = m_aColumnWidths.rbegin();
            do
            {
                aArea.Right() = loop->getEnd() - nScrolledOutLeft + m_nRowHeaderWidthPixel;
                ++loop;
            }
            while ( (   loop != m_aColumnWidths.rend() )
                 && (   loop->getEnd() - nScrolledOutLeft >= aArea.Right() )
                 );
        }
        
        --aArea.Right();

        
        aArea.Bottom() =
                m_nColHeaderHeightPixel
            +   impl_getVisibleRows( true ) * m_nRowHeightPixel
            -   1;

        return aArea;
    }

    
    Rectangle TableControl_Impl::impl_getAllVisibleDataCellArea() const
    {
        DBG_CHECK_ME();

        Rectangle aArea( impl_getAllVisibleCellsArea() );
        aArea.Left() = m_nRowHeaderWidthPixel;
        aArea.Top() = m_nColHeaderHeightPixel;
        return aArea;
    }

    
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

    
    void TableControl_Impl::impl_ni_updateCachedModelValues()
    {
        m_pInputHandler = m_pModel->getInputHandler();
        if ( !m_pInputHandler )
            m_pInputHandler.reset( new DefaultInputHandler );

        m_nColumnCount = m_pModel->getColumnCount();
        if ( m_nLeftColumn >= m_nColumnCount )
            m_nLeftColumn = ( m_nColumnCount > 0 ) ? m_nColumnCount - 1 : 0;

        m_nRowCount = m_pModel->getRowCount();
        if ( m_nTopRow >= m_nRowCount )
            m_nTopRow = ( m_nRowCount > 0 ) ? m_nRowCount - 1 : 0;

        impl_ni_updateCachedTableMetrics();
    }

    
    namespace
    {
        
        
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

        
        void lcl_setButtonRepeat( Window& _rWindow, sal_uLong _nDelay )
        {
            AllSettings aSettings = _rWindow.GetSettings();
            MouseSettings aMouseSettings = aSettings.GetMouseSettings();

            aMouseSettings.SetButtonRepeat( _nDelay );
            aSettings.SetMouseSettings( aMouseSettings );

            _rWindow.SetSettings( aSettings, sal_True );
        }

        
        bool lcl_updateScrollbar( Window& _rParent, ScrollBar*& _rpBar,
            bool const i_needBar, long _nVisibleUnits,
            long _nPosition, long _nLineSize, long _nRange,
            bool _bHorizontal, const Link& _rScrollHandler )
        {
            
            bool bHaveBar = _rpBar != NULL;

            
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

            return ( bHaveBar != i_needBar );
        }

        
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
                        
                        break;

                aColumn.moveRight();
                ++visibleColumns;
            }
            return visibleColumns;
        }

    }

    
    long TableControl_Impl::impl_ni_calculateColumnWidths( ColPos const i_assumeInflexibleColumnsUpToIncluding,
        bool const i_assumeVerticalScrollbar, ::std::vector< long >& o_newColWidthsPixel ) const
    {
        
        long gridWidthPixel = m_rAntiImpl.GetOutputSizePixel().Width();
        ENSURE_OR_RETURN( !!m_pModel, "TableControl_Impl::impl_ni_calculateColumnWidths: not allowed without a model!", gridWidthPixel );
        if ( m_pModel->hasRowHeaders() && ( gridWidthPixel != 0 ) )
        {
            gridWidthPixel -= m_nRowHeaderWidthPixel;
        }

        if ( i_assumeVerticalScrollbar && ( m_pModel->getVerticalScrollbarVisibility() != ScrollbarShowNever ) )
        {
            long nScrollbarMetrics = m_rAntiImpl.GetSettings().GetStyleSettings().GetScrollBarSize();
            gridWidthPixel -= nScrollbarMetrics;
        }

        
        TableSize const colCount = m_pModel->getColumnCount();
        if ( colCount == 0 )
            return gridWidthPixel;

        
        
        long accumulatedCurrentWidth = 0;
        ::std::vector< long > currentColWidths;
        currentColWidths.reserve( colCount );
        typedef ::std::vector< ::std::pair< long, long > >   ColumnLimits;
        ColumnLimits effectiveColumnLimits;
        effectiveColumnLimits.reserve( colCount );
        long accumulatedMinWidth = 0;
        long accumulatedMaxWidth = 0;
        
        ::std::vector< ::sal_Int32 > columnFlexibilities;
        columnFlexibilities.reserve( colCount );
        long flexibilityDenominator = 0;
        size_t flexibleColumnCount = 0;
        for ( ColPos col = 0; col < colCount; ++col )
        {
            PColumnModel const pColumn = m_pModel->getColumnModel( col );
            ENSURE_OR_THROW( !!pColumn, "invalid column returned by the model!" );

            
            long const currentWidth = appFontWidthToPixel( pColumn->getWidth() );
            currentColWidths.push_back( currentWidth );

            
            accumulatedCurrentWidth += currentWidth;

            
            ::sal_Int32 flexibility = pColumn->getFlexibility();
            OSL_ENSURE( flexibility >= 0, "TableControl_Impl::impl_ni_calculateColumnWidths: a column's flexibility should be non-negative." );
            if  (   ( flexibility < 0 )                                 
                ||  ( !pColumn->isResizable() )                         
                ||  ( col <= i_assumeInflexibleColumnsUpToIncluding )   
                )
                flexibility = 0;

            
            long effectiveMin = currentWidth, effectiveMax = currentWidth;
            
            if ( flexibility > 0 )
            {
                long const minWidth = appFontWidthToPixel( pColumn->getMinWidth() );
                if ( minWidth > 0 )
                    effectiveMin = minWidth;
                else
                    effectiveMin = MIN_COLUMN_WIDTH_PIXEL;

                long const maxWidth = appFontWidthToPixel( pColumn->getMaxWidth() );
                OSL_ENSURE( minWidth <= maxWidth, "TableControl_Impl::impl_ni_calculateColumnWidths: pretty undecided 'bout its width limits, this column!" );
                if ( ( maxWidth > 0 ) && ( maxWidth >= minWidth ) )
                    effectiveMax = maxWidth;
                else
                    effectiveMax = gridWidthPixel; 

                if ( effectiveMin == effectiveMax )
                    
                    flexibility = 0;
            }

            columnFlexibilities.push_back( flexibility );
            flexibilityDenominator += flexibility;
            if ( flexibility > 0 )
                ++flexibleColumnCount;

            effectiveColumnLimits.push_back( ::std::pair< long, long >( effectiveMin, effectiveMax ) );
            accumulatedMinWidth += effectiveMin;
            accumulatedMaxWidth += effectiveMax;
        }

        o_newColWidthsPixel = currentColWidths;
        if ( flexibilityDenominator == 0 )
        {
            
        }
        else if ( gridWidthPixel > accumulatedCurrentWidth )
        {   
            long distributePixel = gridWidthPixel - accumulatedCurrentWidth;
            if ( gridWidthPixel > accumulatedMaxWidth )
            {
                
                
                for ( size_t i = 0; i < size_t( colCount ); ++i )
                {
                    o_newColWidthsPixel[i] = effectiveColumnLimits[i].second;
                }
            }
            else
            {
                bool startOver = false;
                do
                {
                    startOver = false;
                    
                    for ( size_t i=0; i<o_newColWidthsPixel.size() && !startOver; ++i )
                    {
                        long const columnFlexibility = columnFlexibilities[i];
                        if ( columnFlexibility == 0 )
                            continue;

                        long newColWidth = currentColWidths[i] + columnFlexibility * distributePixel / flexibilityDenominator;

                        if ( newColWidth > effectiveColumnLimits[i].second )
                        {   
                            
                            newColWidth = effectiveColumnLimits[i].second;
                            
                            flexibilityDenominator -= columnFlexibility;
                            columnFlexibilities[i] = 0;
                            --flexibleColumnCount;
                            
                            long const difference = newColWidth - currentColWidths[i];
                            distributePixel -= difference;
                            
                            

                            
                            
                            startOver = true;
                        }

                        o_newColWidthsPixel[i] = newColWidth;
                    }
                }
                while ( startOver );

                
                distributePixel = gridWidthPixel - ::std::accumulate( o_newColWidthsPixel.begin(), o_newColWidthsPixel.end(), 0 );
                while ( ( distributePixel > 0 ) && ( flexibleColumnCount > 0 ) )
                {
                    
                    
                    for ( size_t i=0; ( i < o_newColWidthsPixel.size() ) && ( distributePixel > 0 ); ++i )
                    {
                        if ( columnFlexibilities[i] == 0 )
                            continue;

                        OSL_ENSURE( o_newColWidthsPixel[i] <= effectiveColumnLimits[i].second,
                            "TableControl_Impl::impl_ni_calculateColumnWidths: inconsitency!" );
                        if ( o_newColWidthsPixel[i] >= effectiveColumnLimits[i].first )
                        {
                            columnFlexibilities[i] = 0;
                            --flexibleColumnCount;
                            continue;
                        }

                        ++o_newColWidthsPixel[i];
                        --distributePixel;
                    }
                }
            }
        }
        else if ( gridWidthPixel < accumulatedCurrentWidth )
        {   
            long takeAwayPixel = accumulatedCurrentWidth - gridWidthPixel;
            if ( gridWidthPixel < accumulatedMinWidth )
            {
                
                
                for ( size_t i = 0; i < size_t( colCount ); ++i )
                {
                    o_newColWidthsPixel[i] = effectiveColumnLimits[i].first;
                }
            }
            else
            {
                bool startOver = false;
                do
                {
                    startOver = false;
                    
                    for ( size_t i=0; i<o_newColWidthsPixel.size() && !startOver; ++i )
                    {
                        long const columnFlexibility = columnFlexibilities[i];
                        if ( columnFlexibility == 0 )
                            continue;

                        long newColWidth = currentColWidths[i] - columnFlexibility * takeAwayPixel / flexibilityDenominator;

                        if ( newColWidth < effectiveColumnLimits[i].first )
                        {   
                            
                            newColWidth = effectiveColumnLimits[i].first;
                            
                            flexibilityDenominator -= columnFlexibility;
                            columnFlexibilities[i] = 0;
                            --flexibleColumnCount;
                            
                            long const difference = currentColWidths[i] - newColWidth;
                            takeAwayPixel -= difference;

                            
                            
                            startOver = true;
                        }

                        o_newColWidthsPixel[i] = newColWidth;
                    }
                }
                while ( startOver );

                
                takeAwayPixel = ::std::accumulate( o_newColWidthsPixel.begin(), o_newColWidthsPixel.end(), 0 ) - gridWidthPixel;
                while ( ( takeAwayPixel > 0 ) && ( flexibleColumnCount > 0 ) )
                {
                    
                    
                    for ( size_t i=0; ( i < o_newColWidthsPixel.size() ) && ( takeAwayPixel > 0 ); ++i )
                    {
                        if ( columnFlexibilities[i] == 0 )
                            continue;

                        OSL_ENSURE( o_newColWidthsPixel[i] >= effectiveColumnLimits[i].first,
                            "TableControl_Impl::impl_ni_calculateColumnWidths: inconsitency!" );
                        if ( o_newColWidthsPixel[i] <= effectiveColumnLimits[i].first )
                        {
                            columnFlexibilities[i] = 0;
                            --flexibleColumnCount;
                            continue;
                        }

                        --o_newColWidthsPixel[i];
                        --takeAwayPixel;
                    }
                }
            }
        }

        return gridWidthPixel;
    }

    
    void TableControl_Impl::impl_ni_relayout( ColPos const i_assumeInflexibleColumnsUpToIncluding )
    {
        ENSURE_OR_RETURN_VOID( !m_bUpdatingColWidths, "TableControl_Impl::impl_ni_relayout: recursive call detected!" );

        m_aColumnWidths.resize( 0 );
        if ( !m_pModel )
            return;

        ::comphelper::FlagRestorationGuard const aWidthUpdateFlag( m_bUpdatingColWidths, true );
        SuppressCursor aHideCursor( *this );

        
        //
        
        
        
        
        //
        
        
        
        
        

        ::std::vector< long > newWidthsPixel;
        long gridWidthPixel = impl_ni_calculateColumnWidths( i_assumeInflexibleColumnsUpToIncluding, true, newWidthsPixel );

        
        long const nScrollbarMetrics = m_rAntiImpl.GetSettings().GetStyleSettings().GetScrollBarSize();

        
        
        Rectangle aDataCellPlayground( Point( 0, 0 ), m_rAntiImpl.GetOutputSizePixel() );
        aDataCellPlayground.Left() = m_nRowHeaderWidthPixel;
        aDataCellPlayground.Top() = m_nColHeaderHeightPixel;

        OSL_ENSURE( ( m_nRowCount == m_pModel->getRowCount() ) && ( m_nColumnCount == m_pModel->getColumnCount() ),
            "TableControl_Impl::impl_ni_relayout: how is this expected to work with invalid data?" );
        long const nAllColumnsWidth = ::std::accumulate( newWidthsPixel.begin(), newWidthsPixel.end(), 0 );

        ScrollbarVisibility const eVertScrollbar = m_pModel->getVerticalScrollbarVisibility();
        ScrollbarVisibility const eHorzScrollbar = m_pModel->getHorizontalScrollbarVisibility();

        
        bool bNeedVerticalScrollbar = lcl_determineScrollbarNeed(
            m_nTopRow, eVertScrollbar, aDataCellPlayground.GetHeight(), m_nRowHeightPixel * m_nRowCount );
        bool bFirstRoundVScrollNeed = false;
        if ( bNeedVerticalScrollbar )
        {
            aDataCellPlayground.Right() -= nScrollbarMetrics;
            bFirstRoundVScrollNeed = true;
        }

        
        bool const bNeedHorizontalScrollbar = lcl_determineScrollbarNeed(
            m_nLeftColumn, eHorzScrollbar, aDataCellPlayground.GetWidth(), nAllColumnsWidth );
        if ( bNeedHorizontalScrollbar )
        {
            aDataCellPlayground.Bottom() -= nScrollbarMetrics;

            
            
            
            
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

        
        
        if ( !bNeedVerticalScrollbar )
            gridWidthPixel = impl_ni_calculateColumnWidths( i_assumeInflexibleColumnsUpToIncluding, false, newWidthsPixel );

        
        TableSize const colCount = m_pModel->getColumnCount();
        m_aColumnWidths.reserve( colCount );
        long accumulatedWidthPixel = m_nRowHeaderWidthPixel;
        bool anyColumnWidthChanged = false;
        for ( ColPos col = 0; col < colCount; ++col )
        {
            const long columnStart = accumulatedWidthPixel;
            const long columnEnd = columnStart + newWidthsPixel[col];
            m_aColumnWidths.push_back( MutableColumnMetrics( columnStart, columnEnd ) );
            accumulatedWidthPixel = columnEnd;

            
            PColumnModel const pColumn = m_pModel->getColumnModel( col );
            ENSURE_OR_THROW( !!pColumn, "invalid column returned by the model!" );

            long const oldColumnWidthAppFont = pColumn->getWidth();
            long const newColumnWidthAppFont = pixelWidthToAppFont( newWidthsPixel[col] );
            pColumn->setWidth( newColumnWidthAppFont );

            anyColumnWidthChanged |= ( oldColumnWidthAppFont != newColumnWidthAppFont );
        }

        
        if ( anyColumnWidthChanged )
            invalidate( TableAreaAll );

        
        
        while   (   ( m_nLeftColumn > 0 )
                &&  ( accumulatedWidthPixel - m_aColumnWidths[ m_nLeftColumn - 1 ].getStart() <= gridWidthPixel )
                )
        {
            --m_nLeftColumn;
        }

        
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

        
        impl_ni_positionChildWindows( aDataCellPlayground, bNeedVerticalScrollbar, bNeedHorizontalScrollbar );
    }

    
    void TableControl_Impl::impl_ni_positionChildWindows( Rectangle const & i_dataCellPlayground,
        bool const i_verticalScrollbar, bool const i_horizontalScrollbar )
    {
        long const nScrollbarMetrics = m_rAntiImpl.GetSettings().GetStyleSettings().GetScrollBarSize();

        
        lcl_updateScrollbar(
            m_rAntiImpl,
            m_pVScroll,
            i_verticalScrollbar,
            lcl_getRowsFittingInto( i_dataCellPlayground.GetHeight(), m_nRowHeightPixel ),
                                                                    
            m_nTopRow,                                              
            1,                                                      
            m_nRowCount,                                            
            false,                                                  
            LINK( this, TableControl_Impl, OnScroll )               
        );

        
        if ( m_pVScroll )
        {
            Rectangle aScrollbarArea(
                Point( i_dataCellPlayground.Right() + 1, 0 ),
                Size( nScrollbarMetrics, i_dataCellPlayground.Bottom() + 1 )
            );
            m_pVScroll->SetPosSizePixel(
                aScrollbarArea.TopLeft(), aScrollbarArea.GetSize() );
        }

        
        lcl_updateScrollbar(
            m_rAntiImpl,
            m_pHScroll,
            i_horizontalScrollbar,
            lcl_getColumnsVisibleWithin( i_dataCellPlayground, m_nLeftColumn, *this, false ),
                                                                    
            m_nLeftColumn,                                          
            1,                                                      
            m_nColumnCount,                                         
            true,                                                   
            LINK( this, TableControl_Impl, OnScroll )               
        );

        
        if ( m_pHScroll )
        {
            TableSize const nVisibleUnits = lcl_getColumnsVisibleWithin( i_dataCellPlayground, m_nLeftColumn, *this, false );
            TableMetrics const nRange = m_nColumnCount;
            if( m_nLeftColumn + nVisibleUnits == nRange - 1 )
            {
                if ( m_aColumnWidths[ nRange - 1 ].getStart() - m_aColumnWidths[ m_nLeftColumn ].getEnd() + m_aColumnWidths[ nRange-1 ].getWidth() > i_dataCellPlayground.GetWidth() )
                {
                    m_pHScroll->SetVisibleSize( nVisibleUnits -1 );
                    m_pHScroll->SetPageSize( nVisibleUnits - 1 );
                }
            }
            Rectangle aScrollbarArea(
                Point( 0, i_dataCellPlayground.Bottom() + 1 ),
                Size( i_dataCellPlayground.Right() + 1, nScrollbarMetrics )
            );
            m_pHScroll->SetPosSizePixel(
                aScrollbarArea.TopLeft(), aScrollbarArea.GetSize() );
        }

        
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
            m_pScrollCorner->SetPosPixel( Point( i_dataCellPlayground.Right() + 1, i_dataCellPlayground.Bottom() + 1 ) );
            m_pScrollCorner->Show();
        }
        else if(bHaveScrollCorner && bNeedScrollCorner)
        {
            m_pScrollCorner->SetPosPixel( Point( i_dataCellPlayground.Right() + 1, i_dataCellPlayground.Bottom() + 1 ) );
            m_pScrollCorner->Show();
        }

        
        m_pDataWindow->SetSizePixel( Size(
            i_dataCellPlayground.GetWidth() + m_nRowHeaderWidthPixel,
            i_dataCellPlayground.GetHeight() + m_nColHeaderHeightPixel
        ) );
    }

    
    void TableControl_Impl::onResize()
    {
        DBG_CHECK_ME();

        impl_ni_relayout();
        checkCursorPosition();
    }

    
    void TableControl_Impl::doPaintContent( const Rectangle& _rUpdateRect )
    {
        DBG_CHECK_ME();

        if ( !getModel() )
            return;
        PTableRenderer pRenderer = getModel()->getRenderer();
        DBG_ASSERT( !!pRenderer, "TableDataWindow::doPaintContent: invalid renderer!" );
        if ( !pRenderer )
            return;

        
        const StyleSettings& rStyle = m_rAntiImpl.GetSettings().GetStyleSettings();
        m_nRowCount = m_pModel->getRowCount();
        
        
        Rectangle const aAllCellsWithHeaders( impl_getAllVisibleCellsArea() );

        
        
        if ( m_pModel->hasColumnHeaders() )
        {
            TableRowGeometry const aHeaderRow( *this, Rectangle( Point( 0, 0 ),
                aAllCellsWithHeaders.BottomRight() ), ROW_COL_HEADERS );
            Rectangle const aColRect(aHeaderRow.getRect());
            pRenderer->PaintHeaderArea(
                *m_pDataWindow, aColRect, true, false, rStyle
            );
            
            
            

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
        
        Rectangle aRowHeaderArea;
        if ( m_pModel->hasRowHeaders() )
        {
            aRowHeaderArea = aAllCellsWithHeaders;
            aRowHeaderArea.Right() = m_nRowHeaderWidthPixel - 1;

            TableSize const nVisibleRows = impl_getVisibleRows( true );
            TableSize nActualRows = nVisibleRows;
            if ( m_nTopRow + nActualRows > m_nRowCount )
                nActualRows = m_nRowCount - m_nTopRow;
            aRowHeaderArea.Bottom() = m_nColHeaderHeightPixel + m_nRowHeightPixel * nActualRows - 1;

            pRenderer->PaintHeaderArea( *m_pDataWindow, aRowHeaderArea, false, true, rStyle );
            
            
            

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

        
        

        TableSize colCount = getModel()->getColumnCount();

        
        Rectangle const aAllDataCellsArea( impl_getAllVisibleDataCellArea() );
        for ( TableRowGeometry aRowIterator( *this, aAllCellsWithHeaders, getTopRow() );
              aRowIterator.isValid();
              aRowIterator.moveDown() )
        {
            if ( _rUpdateRect.GetIntersection( aRowIterator.getRect() ).IsEmpty() )
                continue;

            bool const isControlFocused = m_rAntiImpl.HasControlFocus();
            bool const isSelectedRow = isRowSelected( aRowIterator.getRow() );

            Rectangle const aRect = aRowIterator.getRect().GetIntersection( aAllDataCellsArea );

            
            pRenderer->PrepareRow(
                aRowIterator.getRow(), isControlFocused, isSelectedRow,
                *m_pDataWindow, aRect, rStyle
            );

            
            if ( m_pModel->hasRowHeaders() )
            {
                const Rectangle aCurrentRowHeader( aRowHeaderArea.GetIntersection( aRowIterator.getRect() ) );
                pRenderer->PaintRowHeader( isControlFocused, isSelectedRow, *m_pDataWindow, aCurrentRowHeader,
                    rStyle );
            }

            if ( !colCount )
                continue;

            
            for ( TableCellGeometry aCell( aRowIterator, m_nLeftColumn );
                  aCell.isValid();
                  aCell.moveRight()
                )
            {
                bool isSelectedColumn = false;
                pRenderer->PaintCell( aCell.getColumn(), isSelectedRow || isSelectedColumn, isControlFocused,
                                *m_pDataWindow, aCell.getRect(), rStyle );
            }
        }
    }
    
    void TableControl_Impl::hideCursor()
    {
        DBG_CHECK_ME();

        if ( ++m_nCursorHidden == 1 )
            impl_ni_doSwitchCursor( false );
    }

    
    void TableControl_Impl::showCursor()
    {
        DBG_CHECK_ME();

        DBG_ASSERT( m_nCursorHidden > 0, "TableControl_Impl::showCursor: cursor not hidden!" );
        if ( --m_nCursorHidden == 0 )
            impl_ni_doSwitchCursor( true );
    }

    
    bool TableControl_Impl::dispatchAction( TableControlAction _eAction )
    {
        DBG_CHECK_ME();

        bool bSuccess = false;
        bool selectionChanged = false;

        switch ( _eAction )
        {
        case cursorDown:
        if ( m_pSelEngine->GetSelectionMode() == SINGLE_SELECTION )
        {
            
            if(!m_aSelectedRows.empty())
            {
                invalidateSelectedRows();
                m_aSelectedRows.clear();
            }
            if ( m_nCurRow < m_nRowCount-1 )
            {
                ++m_nCurRow;
                m_aSelectedRows.push_back(m_nCurRow);
            }
            else
                m_aSelectedRows.push_back(m_nCurRow);
            invalidateRow( m_nCurRow );
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
            if(!m_aSelectedRows.empty())
            {
                invalidateSelectedRows();
                m_aSelectedRows.clear();
            }
            if(m_nCurRow>0)
            {
                --m_nCurRow;
                m_aSelectedRows.push_back(m_nCurRow);
                invalidateRow( m_nCurRow );
            }
            else
            {
                m_aSelectedRows.push_back(m_nCurRow);
                invalidateRow( m_nCurRow );
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
            
            int pos = getRowSelectedNumber(m_aSelectedRows, m_nCurRow);
            
            if(pos>-1)
            {
                m_aSelectedRows.erase(m_aSelectedRows.begin()+pos);
                if(m_aSelectedRows.empty() && m_nAnchor != -1)
                    m_nAnchor = -1;
            }
            
            else
                m_aSelectedRows.push_back(m_nCurRow);
            invalidateRow( m_nCurRow );
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
                
                return false;
            }
            else
            {
                
                if(!m_aSelectedRows.empty())
                {
                    
                    
                    if(m_nAnchor==-1)
                    {
                        invalidateSelectedRows();
                        m_aSelectedRows.clear();
                        m_aSelectedRows.push_back(m_nCurRow);
                        invalidateRow( m_nCurRow );
                    }
                    else
                    {
                        
                        int prevRow = getRowSelectedNumber(m_aSelectedRows, m_nCurRow);
                        int nextRow = getRowSelectedNumber(m_aSelectedRows, m_nCurRow-1);
                        if(prevRow>-1)
                         {
                             
                            if(m_nCurRow>0)
                                 m_nCurRow--;
                             else
                                 return bSuccess = true;
                             
                             if(nextRow>-1 && m_aSelectedRows[nextRow] == m_nCurRow)
                             {
                                 m_aSelectedRows.erase(m_aSelectedRows.begin()+prevRow);
                                 invalidateRow( m_nCurRow + 1 );
                             }
                             else
                            {
                                 m_aSelectedRows.push_back(m_nCurRow);
                                 invalidateRow( m_nCurRow );
                             }
                         }
                        else
                        {
                            if(m_nCurRow>0)
                            {
                                m_aSelectedRows.push_back(m_nCurRow);
                                m_nCurRow--;
                                m_aSelectedRows.push_back(m_nCurRow);
                                invalidateSelectedRegion( m_nCurRow+1, m_nCurRow );
                            }
                        }
                    }
                }
                else
                {
                    
                    
                    
                    if(m_nCurRow>0)
                    {
                        m_aSelectedRows.push_back(m_nCurRow);
                        m_nCurRow--;
                        m_aSelectedRows.push_back(m_nCurRow);
                        invalidateSelectedRegion( m_nCurRow+1, m_nCurRow );
                    }
                    else
                    {
                        m_aSelectedRows.push_back(m_nCurRow);
                        invalidateRow( m_nCurRow );
                    }
                }
                m_pSelEngine->SetAnchor(true);
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
                if(!m_aSelectedRows.empty())
                {
                    
                    
                    if(m_nAnchor==-1)
                    {
                        invalidateSelectedRows();
                        m_aSelectedRows.clear();
                        m_aSelectedRows.push_back(m_nCurRow);
                        invalidateRow( m_nCurRow );
                        }
                    else
                    {
                        
                        int prevRow = getRowSelectedNumber(m_aSelectedRows, m_nCurRow);
                        int nextRow = getRowSelectedNumber(m_aSelectedRows, m_nCurRow+1);
                        if(prevRow>-1)
                         {
                             
                             if(m_nCurRow<m_nRowCount-1)
                                 m_nCurRow++;
                             else
                                return bSuccess = true;
                             
                             if(nextRow>-1 && m_aSelectedRows[nextRow] == m_nCurRow)
                             {
                                 m_aSelectedRows.erase(m_aSelectedRows.begin()+prevRow);
                                 invalidateRow( m_nCurRow - 1 );
                             }
                             else
                             {
                                 m_aSelectedRows.push_back(m_nCurRow);
                                 invalidateRow( m_nCurRow );
                             }
                        }
                        else
                        {
                            if(m_nCurRow<m_nRowCount-1)
                            {
                                m_aSelectedRows.push_back(m_nCurRow);
                                m_nCurRow++;
                                m_aSelectedRows.push_back(m_nCurRow);
                                invalidateSelectedRegion( m_nCurRow-1, m_nCurRow );
                            }
                        }
                    }
                }
                else
                {
                    
                    if(m_nCurRow<m_nRowCount-1)
                    {
                        m_aSelectedRows.push_back(m_nCurRow);
                        m_nCurRow++;
                        m_aSelectedRows.push_back(m_nCurRow);
                        invalidateSelectedRegion( m_nCurRow-1, m_nCurRow );
                    }
                    else
                    {
                        m_aSelectedRows.push_back(m_nCurRow);
                        invalidateRow( m_nCurRow );
                    }
                }
                m_pSelEngine->SetAnchor(true);
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
                
                RowPos iter = m_nCurRow;
                invalidateSelectedRegion( m_nCurRow, 0 );
                
                while(iter>=0)
                {
                    if ( !isRowSelected( iter ) )
                        m_aSelectedRows.push_back(iter);
                    --iter;
                }
                m_nCurRow = 0;
                m_nAnchor = m_nCurRow;
                m_pSelEngine->SetAnchor(true);
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
            
            RowPos iter = m_nCurRow;
            invalidateSelectedRegion( m_nCurRow, m_nRowCount-1 );
            
            while(iter<=m_nRowCount)
            {
                if ( !isRowSelected( iter ) )
                    m_aSelectedRows.push_back(iter);
                ++iter;
            }
            m_nCurRow = m_nRowCount-1;
            m_nAnchor = m_nCurRow;
            m_pSelEngine->SetAnchor(true);
            ensureVisible(m_nCurColumn, m_nRowCount-1, false);
            selectionChanged = true;
            bSuccess = true;
        }
        break;
        default:
            OSL_FAIL( "TableControl_Impl::dispatchAction: unsupported action!" );
            break;
        }

        if ( bSuccess && selectionChanged )
        {
            m_rAntiImpl.Select();
        }

        return bSuccess;
    }

    
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

        TableCellGeometry aCell( *this, impl_getAllVisibleCellsArea(), _nColumn, _nRow );
        _rCellRect = aCell.getRect();
    }

    
    RowPos TableControl_Impl::getRowAtPoint( const Point& rPoint ) const
    {
        DBG_CHECK_ME();
        return impl_getRowForAbscissa( rPoint.Y() );
    }

    
    ColPos TableControl_Impl::getColAtPoint( const Point& rPoint ) const
    {
        DBG_CHECK_ME();
        return impl_getColumnForOrdinate( rPoint.X() );
    }

    
    TableCell TableControl_Impl::hitTest( Point const & i_point ) const
    {
        TableCell aCell( getColAtPoint( i_point ), getRowAtPoint( i_point ) );
        if ( aCell.nColumn > COL_ROW_HEADERS )
        {
            PColumnModel const pColumn = m_pModel->getColumnModel( aCell.nColumn );
            MutableColumnMetrics const & rColInfo( m_aColumnWidths[ aCell.nColumn ] );
            if  (   ( rColInfo.getEnd() - 3 <= i_point.X() )
                &&  ( rColInfo.getEnd() >= i_point.X() )
                &&  pColumn->isResizable()
                )
            {
                aCell.eArea = ColumnDivider;
            }
        }
        return aCell;
    }

    
    ColumnMetrics TableControl_Impl::getColumnMetrics( ColPos const i_column ) const
    {
        DBG_CHECK_ME();

        ENSURE_OR_RETURN( ( i_column >= 0 ) && ( i_column < m_pModel->getColumnCount() ),
            "TableControl_Impl::getColumnMetrics: illegal column index!", ColumnMetrics() );
        return (ColumnMetrics const &)m_aColumnWidths[ i_column ];
    }

    
    PTableModel TableControl_Impl::getModel() const
    {
        return m_pModel;
    }

    
    RowPos TableControl_Impl::getCurrentColumn() const
    {
        return m_nCurColumn;
    }

    
    RowPos TableControl_Impl::getCurrentRow() const
    {
        return m_nCurRow;
    }

    
    ::Size TableControl_Impl::getTableSizePixel() const
    {
        return m_pDataWindow->GetOutputSizePixel();
    }

    
    void TableControl_Impl::setPointer( Pointer const & i_pointer )
    {
        DBG_CHECK_ME();
        m_pDataWindow->SetPointer( i_pointer );
    }

    
    void TableControl_Impl::captureMouse()
    {
        m_pDataWindow->CaptureMouse();
    }

    
    void TableControl_Impl::releaseMouse()
    {
        m_pDataWindow->ReleaseMouse();
    }

    
    void TableControl_Impl::invalidate( TableArea const i_what )
    {
        switch ( i_what )
        {
        case TableAreaColumnHeaders:
            m_pDataWindow->Invalidate( calcHeaderRect( true ) );
            break;

        case TableAreaRowHeaders:
            m_pDataWindow->Invalidate( calcHeaderRect( false ) );
            break;

        case TableAreaDataArea:
            m_pDataWindow->Invalidate( impl_getAllVisibleDataCellArea() );
            break;

        case TableAreaAll:
            m_pDataWindow->Invalidate();
            m_pDataWindow->GetParent()->Invalidate( INVALIDATE_TRANSPARENT );
            break;
        }
    }

    
    long TableControl_Impl::pixelWidthToAppFont( long const i_pixels ) const
    {
        return m_pDataWindow->PixelToLogic( Size( i_pixels, 0 ), MAP_APPFONT ).Width();
    }

    
    long TableControl_Impl::appFontWidthToPixel( long const i_appFontUnits ) const
    {
        return m_pDataWindow->LogicToPixel( Size( i_appFontUnits, 0 ), MAP_APPFONT ).Width();
    }

    
    void TableControl_Impl::hideTracking()
    {
        m_pDataWindow->HideTracking();
    }

    
    void TableControl_Impl::showTracking( Rectangle const & i_location, sal_uInt16 const i_flags )
    {
        m_pDataWindow->ShowTracking( i_location, i_flags );
    }

    
    bool TableControl_Impl::activateCell( ColPos const i_col, RowPos const i_row )
    {
        DBG_CHECK_ME();
        return goTo( i_col, i_row );
    }

    
    void TableControl_Impl::invalidateSelectedRegion( RowPos _nPrevRow, RowPos _nCurRow )
    {
        DBG_CHECK_ME();
        
        Rectangle const aAllCells( impl_getAllVisibleCellsArea() );

        Rectangle aInvalidateRect;
        aInvalidateRect.Left() = aAllCells.Left();
        aInvalidateRect.Right() = aAllCells.Right();
        
        if ( _nPrevRow == _nCurRow )
        {
            Rectangle aCellRect;
            impl_getCellRect( m_nCurColumn, _nCurRow, aCellRect );
            aInvalidateRect.Top() = aCellRect.Top();
            aInvalidateRect.Bottom() = aCellRect.Bottom();
        }
        
        else if(_nPrevRow < _nCurRow )
        {
            Rectangle aCellRect;
            impl_getCellRect( m_nCurColumn, _nPrevRow, aCellRect );
            aInvalidateRect.Top() = aCellRect.Top();
            impl_getCellRect( m_nCurColumn, _nCurRow, aCellRect );
            aInvalidateRect.Bottom() = aCellRect.Bottom();
        }
        
        else
        {
            Rectangle aCellRect;
            impl_getCellRect( m_nCurColumn, _nCurRow, aCellRect );
            aInvalidateRect.Top() = aCellRect.Top();
            impl_getCellRect( m_nCurColumn, _nPrevRow, aCellRect );
            aInvalidateRect.Bottom() = aCellRect.Bottom();
        }

        invalidateRect(aInvalidateRect);
    }

    void TableControl_Impl::invalidateRect(const Rectangle &rInvalidateRect)
    {
        m_pDataWindow->Invalidate( rInvalidateRect,
            m_pDataWindow->GetControlBackground().GetTransparency() ? INVALIDATE_TRANSPARENT : 0 );
    }

    
    void TableControl_Impl::invalidateSelectedRows()
    {
        for (   ::std::vector< RowPos >::iterator selRow = m_aSelectedRows.begin();
                selRow != m_aSelectedRows.end();
                ++selRow
            )
        {
            invalidateRow( *selRow );
        }
    }

    
    void TableControl_Impl::invalidateRowRange( RowPos const i_firstRow, RowPos const i_lastRow )
    {
        RowPos const firstRow = i_firstRow < m_nTopRow ? m_nTopRow : i_firstRow;
        RowPos const lastVisibleRow = m_nTopRow + impl_getVisibleRows( true ) - 1;
        RowPos const lastRow = ( ( i_lastRow == ROW_INVALID ) || ( i_lastRow > lastVisibleRow ) ) ? lastVisibleRow : i_lastRow;

        Rectangle aInvalidateRect;

        Rectangle const aVisibleCellsArea( impl_getAllVisibleCellsArea() );
        TableRowGeometry aRow( *this, aVisibleCellsArea, firstRow, true );
        while ( aRow.isValid() && ( aRow.getRow() <= lastRow ) )
        {
            aInvalidateRect.Union( aRow.getRect() );
            aRow.moveDown();
        }

        if ( i_lastRow == ROW_INVALID )
            aInvalidateRect.Bottom() = m_pDataWindow->GetOutputSizePixel().Height();

        invalidateRect(aInvalidateRect);
    }

    
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

    
    bool TableControl_Impl::goTo( ColPos _nColumn, RowPos _nRow )
    {
        DBG_CHECK_ME();

        

        if  (  ( _nColumn < 0 ) || ( _nColumn >= m_nColumnCount )
            || ( _nRow < 0 ) || ( _nRow >= m_nRowCount )
            )
        {
            OSL_ENSURE( false, "TableControl_Impl::goTo: invalid row or column index!" );
            return false;
        }

        SuppressCursor aHideCursor( *this );
        m_nCurColumn = _nColumn;
        m_nCurRow = _nRow;

        
        ensureVisible( m_nCurColumn, m_nCurRow, false );
        return true;
    }

    
    void TableControl_Impl::ensureVisible( ColPos _nColumn, RowPos _nRow, bool _bAcceptPartialVisibility )
    {
        DBG_CHECK_ME();
        DBG_ASSERT( ( _nColumn >= 0 ) && ( _nColumn < m_nColumnCount )
                 && ( _nRow >= 0 ) && ( _nRow < m_nRowCount ),
                 "TableControl_Impl::ensureVisible: invalid coordinates!" );

        SuppressCursor aHideCursor( *this );

        if ( _nColumn < m_nLeftColumn )
            impl_scrollColumns( _nColumn - m_nLeftColumn );
        else
        {
            TableSize nVisibleColumns = impl_getVisibleColumns( _bAcceptPartialVisibility );
            if ( _nColumn > m_nLeftColumn + nVisibleColumns - 1 )
            {
                impl_scrollColumns( _nColumn - ( m_nLeftColumn + nVisibleColumns - 1 ) );
                
                
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

    
    OUString TableControl_Impl::getCellContentAsString( RowPos const i_row, ColPos const i_col )
    {
        Any aCellValue;
        m_pModel->getCellContent( i_col, i_row, aCellValue );

        OUString sCellStringContent;
        m_pModel->getRenderer()->GetFormattedCellString( aCellValue, i_col, i_row, sCellStringContent );

        return sCellStringContent;
    }

    
    TableSize TableControl_Impl::impl_ni_ScrollRows( TableSize _nRowDelta )
    {
        
        RowPos nNewTopRow =
            ::std::max(
                ::std::min( (RowPos)( m_nTopRow + _nRowDelta ), (RowPos)( m_nRowCount - 1 ) ),
                (RowPos)0
            );

        RowPos nOldTopRow = m_nTopRow;
        m_nTopRow = nNewTopRow;

        
        if ( m_nTopRow != nOldTopRow )
        {
            DBG_SUSPEND_INV( INV_SCROLL_POSITION );
            SuppressCursor aHideCursor( *this );
            
            
            

            
            long nPixelDelta = m_nRowHeightPixel * ( m_nTopRow - nOldTopRow );

            Rectangle aDataArea( Point( 0, m_nColHeaderHeightPixel ), m_pDataWindow->GetOutputSizePixel() );

            if  (   m_pDataWindow->GetBackground().IsScrollable()
                &&  abs( nPixelDelta ) < aDataArea.GetHeight()
                )
            {
                m_pDataWindow->Scroll( 0, (long)-nPixelDelta, aDataArea, SCROLL_CLIP | SCROLL_UPDATE | SCROLL_CHILDREN);
            }
            else
            {
                m_pDataWindow->Invalidate( INVALIDATE_UPDATE );
                m_pDataWindow->GetParent()->Invalidate( INVALIDATE_TRANSPARENT );
            }

            
            if ( m_pVScroll != NULL )
                m_pVScroll->SetThumbPos( m_nTopRow );
        }

        
        
        
        
        
        
        
        
        
        if ( m_nTopRow == 0 )
            m_rAntiImpl.PostUserEvent( LINK( this, TableControl_Impl, OnUpdateScrollbars ) );

        return (TableSize)( m_nTopRow - nOldTopRow );
    }

    
    TableSize TableControl_Impl::impl_scrollRows( TableSize const i_rowDelta )
    {
        DBG_CHECK_ME();
        return impl_ni_ScrollRows( i_rowDelta );
    }

    
    TableSize TableControl_Impl::impl_ni_ScrollColumns( TableSize _nColumnDelta )
    {
        
        const ColPos nNewLeftColumn =
            ::std::max(
                ::std::min( (ColPos)( m_nLeftColumn + _nColumnDelta ), (ColPos)( m_nColumnCount - 1 ) ),
                (ColPos)0
            );

        const ColPos nOldLeftColumn = m_nLeftColumn;
        m_nLeftColumn = nNewLeftColumn;

        
        if ( m_nLeftColumn != nOldLeftColumn )
        {
            DBG_SUSPEND_INV( INV_SCROLL_POSITION );
            SuppressCursor aHideCursor( *this );
            
            
            

            
            const Rectangle aDataArea( Point( m_nRowHeaderWidthPixel, 0 ), m_pDataWindow->GetOutputSizePixel() );

            long nPixelDelta =
                    m_aColumnWidths[ nOldLeftColumn ].getStart()
                -   m_aColumnWidths[ m_nLeftColumn ].getStart();

            
            
            
            for (   ColumnPositions::iterator colPos = m_aColumnWidths.begin();
                    colPos != m_aColumnWidths.end();
                    ++colPos
                 )
            {
                colPos->move( nPixelDelta );
            }

            
            if  (   m_pDataWindow->GetBackground().IsScrollable()
                &&  abs( nPixelDelta ) < aDataArea.GetWidth()
                )
            {
                m_pDataWindow->Scroll( nPixelDelta, 0, aDataArea, SCROLL_CLIP | SCROLL_UPDATE );
            }
            else
            {
                m_pDataWindow->Invalidate( INVALIDATE_UPDATE );
                m_pDataWindow->GetParent()->Invalidate( INVALIDATE_TRANSPARENT );
            }

            
            if ( m_pHScroll != NULL )
                m_pHScroll->SetThumbPos( m_nLeftColumn );
        }

        
        
        
        if ( m_nLeftColumn == 0 )
            m_rAntiImpl.PostUserEvent( LINK( this, TableControl_Impl, OnUpdateScrollbars ) );

        return (TableSize)( m_nLeftColumn - nOldLeftColumn );
    }

    
    TableSize TableControl_Impl::impl_scrollColumns( TableSize const i_columnDelta )
    {
        DBG_CHECK_ME();
        return impl_ni_ScrollColumns( i_columnDelta );
    }

    
    SelectionEngine* TableControl_Impl::getSelEngine()
    {
        return m_pSelEngine;
    }

    
    ScrollBar* TableControl_Impl::getHorzScrollbar()
    {
        return m_pHScroll;
    }

    
    ScrollBar* TableControl_Impl::getVertScrollbar()
    {
        return m_pVScroll;
    }

    
    bool TableControl_Impl::isRowSelected( RowPos i_row ) const
    {
        return ::std::find( m_aSelectedRows.begin(), m_aSelectedRows.end(), i_row ) != m_aSelectedRows.end();
    }

    
    RowPos TableControl_Impl::getSelectedRowIndex( size_t const i_selectionIndex ) const
    {
        if ( i_selectionIndex < m_aSelectedRows.size() )
            return m_aSelectedRows[ i_selectionIndex ];
        return ROW_INVALID;
    }

    
    int TableControl_Impl::getRowSelectedNumber(const ::std::vector<RowPos>& selectedRows, RowPos current)
    {
        std::vector<RowPos>::const_iterator it = ::std::find(selectedRows.begin(),selectedRows.end(),current);
        if ( it != selectedRows.end() )
        {
            return it - selectedRows.begin();
        }
        return -1;
    }

    
    ColPos TableControl_Impl::impl_getColumnForOrdinate( long const i_ordinate ) const
    {
        DBG_CHECK_ME();

        if ( ( m_aColumnWidths.empty() ) || ( i_ordinate < 0 ) )
            return COL_INVALID;

        if ( i_ordinate < m_nRowHeaderWidthPixel )
            return COL_ROW_HEADERS;

        ColumnPositions::const_iterator lowerBound = ::std::lower_bound(
            m_aColumnWidths.begin(),
            m_aColumnWidths.end(),
            MutableColumnMetrics(i_ordinate+1, i_ordinate+1),
            ColumnInfoPositionLess()
        );
        if ( lowerBound == m_aColumnWidths.end() )
        {
            
            if ( i_ordinate < m_aColumnWidths.rbegin()->getEnd() )
                
                return m_nColumnCount - 1;
            return COL_INVALID;
        }
        return lowerBound - m_aColumnWidths.begin();
    }

    
    RowPos TableControl_Impl::impl_getRowForAbscissa( long const i_abscissa ) const
    {
        DBG_CHECK_ME();

        if ( i_abscissa < 0 )
            return ROW_INVALID;

        if ( i_abscissa < m_nColHeaderHeightPixel )
            return ROW_COL_HEADERS;

        long const abscissa = i_abscissa - m_nColHeaderHeightPixel;
        long const row = m_nTopRow + abscissa / m_nRowHeightPixel;
        return row < m_pModel->getRowCount() ? row : ROW_INVALID;
    }

    
    bool TableControl_Impl::markRowAsDeselected( RowPos const i_rowIndex )
    {
        DBG_CHECK_ME();

        ::std::vector< RowPos >::iterator selPos = ::std::find( m_aSelectedRows.begin(), m_aSelectedRows.end(), i_rowIndex );
        if ( selPos == m_aSelectedRows.end() )
            return false;

        m_aSelectedRows.erase( selPos );
        return true;
    }

    
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
            

        case MULTIPLE_SELECTION:
            m_aSelectedRows.push_back( i_rowIndex );
            break;

        default:
            OSL_ENSURE( false, "TableControl_Impl::markRowAsSelected: unsupported selection mode!" );
            return false;
        }

        return true;
    }

    
    bool TableControl_Impl::markAllRowsAsDeselected()
    {
        if ( m_aSelectedRows.empty() )
            return false;

        m_aSelectedRows.clear();
        return true;
    }

    
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
            
            return false;
        }

        m_aSelectedRows.clear();
        for ( RowPos i=0; i < m_pModel->getRowCount(); ++i )
            m_aSelectedRows.push_back(i);

        return true;
    }

    
    void TableControl_Impl::commitAccessibleEvent( sal_Int16 const i_eventID, const Any& i_newValue, const Any& i_oldValue )
    {
        impl_commitAccessibleEvent( i_eventID, i_newValue, i_oldValue );
    }

    
    void TableControl_Impl::commitCellEvent( sal_Int16 const i_eventID, const Any& i_newValue, const Any& i_oldValue )
    {
        DBG_CHECK_ME();
        if ( impl_isAccessibleAlive() )
             m_pAccessibleTable->commitCellEvent( i_eventID, i_newValue, i_oldValue );
    }

    
    void TableControl_Impl::commitTableEvent( sal_Int16 const i_eventID, const Any& i_newValue, const Any& i_oldValue )
    {
        DBG_CHECK_ME();
        if ( impl_isAccessibleAlive() )
             m_pAccessibleTable->commitTableEvent( i_eventID, i_newValue, i_oldValue );
    }

    
    Rectangle TableControl_Impl::calcHeaderRect(bool bColHeader)
    {
        Rectangle const aRectTableWithHeaders( impl_getAllVisibleCellsArea() );
        Size const aSizeTableWithHeaders( aRectTableWithHeaders.GetSize() );
        if ( bColHeader )
            return Rectangle( aRectTableWithHeaders.TopLeft(), Size( aSizeTableWithHeaders.Width(), m_nColHeaderHeightPixel ) );
        else
            return Rectangle( aRectTableWithHeaders.TopLeft(), Size( m_nRowHeaderWidthPixel, aSizeTableWithHeaders.Height() ) );
    }

    
    Rectangle TableControl_Impl::calcHeaderCellRect( bool bColHeader, sal_Int32 nPos )
    {
        Rectangle const aHeaderRect = calcHeaderRect( bColHeader );
        TableCellGeometry const aGeometry(
            *this, aHeaderRect,
            bColHeader ? nPos : COL_ROW_HEADERS,
            bColHeader ? ROW_COL_HEADERS : nPos
        );
        return aGeometry.getRect();
    }

    
    Rectangle TableControl_Impl::calcTableRect()
    {
        return impl_getAllVisibleDataCellArea();
    }

    
    Rectangle TableControl_Impl::calcCellRect( sal_Int32 nRow, sal_Int32 nCol )
    {
        Rectangle aCellRect;
        impl_getCellRect( nRow, nCol, aCellRect );
        return aCellRect;
    }

    
    IMPL_LINK( TableControl_Impl, OnUpdateScrollbars, void*, /**/ )
    {
        DBG_CHECK_ME();
        
        
        impl_ni_relayout();
        return 1L;
    }

    
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

    
    void TableControl_Impl::disposeAccessible()
    {
        if ( m_pAccessibleTable )
            m_pAccessibleTable->DisposeAccessImpl();
        m_pAccessibleTable = NULL;
    }

    
    bool TableControl_Impl::impl_isAccessibleAlive() const
    {
        DBG_CHECK_ME();
        return ( NULL != m_pAccessibleTable ) && m_pAccessibleTable->isAlive();
    }

    
    void TableControl_Impl::impl_commitAccessibleEvent( sal_Int16 const i_eventID, Any const & i_newValue, Any const & i_oldValue )
    {
        DBG_CHECK_ME();
        if ( impl_isAccessibleAlive() )
             m_pAccessibleTable->commitEvent( i_eventID, i_newValue, i_oldValue );
    }

    
    
    
    
    TableFunctionSet::TableFunctionSet(TableControl_Impl* _pTableControl)
        :m_pTableControl( _pTableControl)
        ,m_nCurrentRow( ROW_INVALID )
    {
    }
    
    TableFunctionSet::~TableFunctionSet()
    {
    }
    
    void TableFunctionSet::BeginDrag()
    {
    }
    
    void TableFunctionSet::CreateAnchor()
    {
        m_pTableControl->setAnchor( m_pTableControl->getCurRow() );
    }

    
    void TableFunctionSet::DestroyAnchor()
    {
        m_pTableControl->setAnchor( ROW_INVALID );
    }

    
    bool TableFunctionSet::SetCursorAtPoint(const Point& rPoint, sal_Bool bDontSelectAtCursor)
    {
        sal_Bool bHandled = sal_False;
        
        RowPos newRow = m_pTableControl->getRowAtPoint( rPoint );
        if ( newRow == ROW_COL_HEADERS )
            newRow = m_pTableControl->getTopRow();

        ColPos newCol = m_pTableControl->getColAtPoint( rPoint );
        if ( newCol == COL_ROW_HEADERS )
            newCol = m_pTableControl->getLeftColumn();

        if ( ( newRow == ROW_INVALID ) || ( newCol == COL_INVALID ) )
            return false;

        if ( bDontSelectAtCursor )
        {
            if ( m_pTableControl->getSelectedRowCount() > 1 )
                m_pTableControl->getSelEngine()->AddAlways(true);
            bHandled = sal_True;
        }
        else if ( m_pTableControl->getAnchor() == m_pTableControl->getCurRow() )
        {
            
            int diff = m_pTableControl->getCurRow() - newRow;
            
            if( diff >= 0)
            {
                
                while ( m_pTableControl->getAnchor() >= newRow )
                {
                    m_pTableControl->markRowAsSelected( m_pTableControl->getAnchor() );
                    m_pTableControl->setAnchor( m_pTableControl->getAnchor() - 1 );
                    diff--;
                }
                m_pTableControl->setAnchor( m_pTableControl->getAnchor() + 1 );
            }
            
            else
            {
                while ( m_pTableControl->getAnchor() <= newRow )
                {
                    m_pTableControl->markRowAsSelected( m_pTableControl->getAnchor() );
                    m_pTableControl->setAnchor( m_pTableControl->getAnchor() + 1 );
                    diff++;
                }
                m_pTableControl->setAnchor( m_pTableControl->getAnchor() - 1 );
            }
            m_pTableControl->invalidateSelectedRegion( m_pTableControl->getCurRow(), newRow );
            bHandled = sal_True;
        }
        
        else
        {
            if ( !m_pTableControl->hasRowSelection() )
                m_pTableControl->markRowAsSelected( newRow );
            else
            {
                if ( m_pTableControl->getSelEngine()->GetSelectionMode() == SINGLE_SELECTION )
                {
                    DeselectAll();
                    m_pTableControl->markRowAsSelected( newRow );
                }
                else
                {
                    m_pTableControl->markRowAsSelected( newRow );
                }
            }
            if ( m_pTableControl->getSelectedRowCount() > 1 && m_pTableControl->getSelEngine()->GetSelectionMode() != SINGLE_SELECTION )
                m_pTableControl->getSelEngine()->AddAlways(true);

            m_pTableControl->invalidateRow( newRow );
            bHandled = sal_True;
        }
        m_pTableControl->goTo( newCol, newRow );
        return bHandled;
    }
    
    bool TableFunctionSet::IsSelectionAtPoint( const Point& rPoint )
    {
        m_pTableControl->getSelEngine()->AddAlways(false);
        if ( !m_pTableControl->hasRowSelection() )
            return false;
        else
        {
            RowPos curRow = m_pTableControl->getRowAtPoint( rPoint );
            m_pTableControl->setAnchor( ROW_INVALID );
            bool selected = m_pTableControl->isRowSelected( curRow );
            m_nCurrentRow = curRow;
            return selected;
        }
    }
    
    void TableFunctionSet::DeselectAtPoint( const Point& rPoint )
    {
        (void)rPoint;
        m_pTableControl->invalidateRow( m_nCurrentRow );
        m_pTableControl->markRowAsDeselected( m_nCurrentRow );
    }

    
    void TableFunctionSet::DeselectAll()
    {
        if ( m_pTableControl->hasRowSelection() )
        {
            for ( size_t i=0; i<m_pTableControl->getSelectedRowCount(); ++i )
            {
                RowPos const rowIndex = m_pTableControl->getSelectedRowIndex(i);
                m_pTableControl->invalidateRow( rowIndex );
            }

            m_pTableControl->markAllRowsAsDeselected();
        }
    }


} } 


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
