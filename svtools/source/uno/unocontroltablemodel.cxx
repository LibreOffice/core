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

#include "unocontroltablemodel.hxx"
#include "unogridcolumnfacade.hxx"

#include "table/defaultinputhandler.hxx"
#include "table/gridtablerenderer.hxx"
#include "table/tablecontrol.hxx"

#include <com/sun/star/awt/grid/XGridColumn.hpp>
#include <com/sun/star/view/SelectionType.hpp>
#include <com/sun/star/awt/grid/XGridColumnListener.hpp>
#include <com/sun/star/awt/grid/XSortableGridData.hpp>

#include <cppuhelper/weakref.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>


namespace svt { namespace table
{


    using css::uno::Reference;
    using css::uno::RuntimeException;
    using css::uno::Sequence;
    using css::uno::UNO_QUERY_THROW;
    using css::uno::UNO_QUERY;
    using css::awt::grid::XGridColumn;
    using css::uno::XInterface;
    using css::uno::Exception;
    using css::awt::grid::XGridColumnListener;
    using css::lang::EventObject;
    using css::awt::grid::GridColumnEvent;
    using css::awt::grid::XGridDataModel;
    using css::awt::grid::XGridColumnModel;
    using css::uno::Any;
    using css::style::HorizontalAlignment_LEFT;
    using css::style::HorizontalAlignment;
    using css::style::VerticalAlignment_TOP;
    using css::style::VerticalAlignment;
    using css::uno::WeakReference;
    using css::awt::grid::GridDataEvent;
    using css::awt::grid::XSortableGridData;
    using css::beans::Pair;


    //= UnoControlTableModel_Impl

    typedef ::std::vector< PTableModelListener >    ModellListeners;
    typedef ::std::vector< PColumnModel >           ColumnModels;
    struct UnoControlTableModel_Impl
    {
        ColumnModels                                    aColumns;
        bool                                            bHasColumnHeaders;
        bool                                            bHasRowHeaders;
        ScrollbarVisibility                             eVScrollMode;
        ScrollbarVisibility                             eHScrollMode;
        PTableRenderer                                  pRenderer;
        PTableInputHandler                              pInputHandler;
        TableMetrics                                    nRowHeight;
        TableMetrics                                    nColumnHeaderHeight;
        TableMetrics                                    nRowHeaderWidth;
        ::boost::optional< ::Color >                    m_aGridLineColor;
        ::boost::optional< ::Color >                    m_aHeaderBackgroundColor;
        ::boost::optional< ::Color >                    m_aHeaderTextColor;
        ::boost::optional< ::Color >                    m_aActiveSelectionBackColor;
        ::boost::optional< ::Color >                    m_aInactiveSelectionBackColor;
        ::boost::optional< ::Color >                    m_aActiveSelectionTextColor;
        ::boost::optional< ::Color >                    m_aInactiveSelectionTextColor;
        ::boost::optional< ::Color >                    m_aTextColor;
        ::boost::optional< ::Color >                    m_aTextLineColor;
        ::boost::optional< ::std::vector< ::Color > >   m_aRowColors;
        VerticalAlignment                               m_eVerticalAlign;
        bool                                            bEnabled;
        ModellListeners                                 m_aListeners;
        WeakReference< XGridDataModel >                 m_aDataModel;
        WeakReference< XGridColumnModel >               m_aColumnModel;

        UnoControlTableModel_Impl()
            :aColumns                       ( )
            ,bHasColumnHeaders              ( true      )
            ,bHasRowHeaders                 ( false     )
            ,eVScrollMode                   ( ScrollbarShowNever )
            ,eHScrollMode                   ( ScrollbarShowNever )
            ,pRenderer                      ( )
            ,pInputHandler                  ( )
            ,nRowHeight                     ( 10 )
            ,nColumnHeaderHeight            ( 10 )
            ,nRowHeaderWidth                ( 10 )
            ,m_aGridLineColor               ( )
            ,m_aHeaderBackgroundColor       ( )
            ,m_aHeaderTextColor             ( )
            ,m_aActiveSelectionBackColor    ( )
            ,m_aInactiveSelectionBackColor  ( )
            ,m_aActiveSelectionTextColor    ( )
            ,m_aInactiveSelectionTextColor  ( )
            ,m_aTextColor                   ( )
            ,m_aTextLineColor               ( )
            ,m_aRowColors                   ( )
            ,m_eVerticalAlign               ( VerticalAlignment_TOP )
            ,bEnabled                       ( true )
        {
        }
    };

    //= UnoControlTableModel
#define DBG_CHECK_ME() \
    DBG_TESTSOLARMUTEX(); \

    UnoControlTableModel::UnoControlTableModel()
        :m_pImpl( new UnoControlTableModel_Impl )
    {
        m_pImpl->bHasColumnHeaders = true;
        m_pImpl->bHasRowHeaders = false;
        m_pImpl->bEnabled = true;
        m_pImpl->pRenderer.reset( new GridTableRenderer( *this ) );
        m_pImpl->pInputHandler.reset( new DefaultInputHandler );
    }


    UnoControlTableModel::~UnoControlTableModel()
    {
        DELETEZ( m_pImpl );
    }


    TableSize UnoControlTableModel::getColumnCount() const
    {
        DBG_CHECK_ME();
        return (TableSize)m_pImpl->aColumns.size();
    }


    TableSize UnoControlTableModel::getRowCount() const
    {
        DBG_CHECK_ME();

        TableSize nRowCount = 0;
        try
        {
            Reference< XGridDataModel > const xDataModel( m_pImpl->m_aDataModel );
            ENSURE_OR_THROW( xDataModel.is(), "no data model anymore!" );
            nRowCount = xDataModel->getRowCount();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return nRowCount;
    }


    bool UnoControlTableModel::hasColumnHeaders() const
    {
        DBG_CHECK_ME();
        return m_pImpl->bHasColumnHeaders;
    }


    bool UnoControlTableModel::hasRowHeaders() const
    {
        DBG_CHECK_ME();
        return m_pImpl->bHasRowHeaders;
    }


    void UnoControlTableModel::setRowHeaders(bool _bRowHeaders)
    {
        DBG_CHECK_ME();
        if ( m_pImpl->bHasRowHeaders == _bRowHeaders )
            return;

        m_pImpl->bHasRowHeaders = _bRowHeaders;
        impl_notifyTableMetricsChanged();
    }


    void UnoControlTableModel::setColumnHeaders(bool _bColumnHeaders)
    {
        DBG_CHECK_ME();
        if ( m_pImpl->bHasColumnHeaders == _bColumnHeaders )
            return;

        m_pImpl->bHasColumnHeaders = _bColumnHeaders;
        impl_notifyTableMetricsChanged();
    }


    PColumnModel UnoControlTableModel::getColumnModel( ColPos column )
    {
        DBG_CHECK_ME();
        ENSURE_OR_RETURN( ( column >= 0 ) && ( column < getColumnCount() ),
            "DefaultTableModel::getColumnModel: invalid index!", PColumnModel() );
        return m_pImpl->aColumns[ column ];
    }


    void UnoControlTableModel::appendColumn( Reference< XGridColumn > const & i_column )
    {
        DBG_CHECK_ME();
        insertColumn( m_pImpl->aColumns.size(), i_column );
    }


    void UnoControlTableModel::insertColumn( ColPos const i_position, Reference< XGridColumn > const & i_column )
    {
        DBG_CHECK_ME();
        ENSURE_OR_RETURN_VOID( ( i_position >= 0 ) && ( size_t( i_position ) <= m_pImpl->aColumns.size() ),
            "UnoControlTableModel::insertColumn: illegal position!" );

        const PColumnModel pColumn( new UnoGridColumnFacade( *this, i_column ) );
        m_pImpl->aColumns.insert( m_pImpl->aColumns.begin() + i_position, pColumn );

        // notify listeners
        ModellListeners aListeners( m_pImpl->m_aListeners );
        for (   ModellListeners::const_iterator loop = aListeners.begin();
                loop != aListeners.end();
                ++loop
            )
        {
            (*loop)->columnInserted();
        }
    }


    void UnoControlTableModel::removeColumn( ColPos const i_position )
    {
        DBG_CHECK_ME();
        ENSURE_OR_RETURN_VOID( ( i_position >= 0 ) && ( size_t( i_position ) <= m_pImpl->aColumns.size() ),
            "UnoControlTableModel::removeColumn: illegal position!" );

        // remove the column
        ColumnModels::iterator pos = m_pImpl->aColumns.begin() + i_position;
        const PColumnModel pColumn = *pos;
        m_pImpl->aColumns.erase( pos );

        // notify listeners
        ModellListeners aListeners( m_pImpl->m_aListeners );
        for (   ModellListeners::const_iterator loop = aListeners.begin();
                loop != aListeners.end();
                ++loop
            )
        {
            (*loop)->columnRemoved();
        }

        // dispose the column
        UnoGridColumnFacade* pColumnImpl = dynamic_cast< UnoGridColumnFacade* >( pColumn.get() );
        OSL_ENSURE( pColumnImpl != NULL, "UnoControlTableModel::removeColumn: illegal column implementation!" );
        if ( pColumnImpl )
            pColumnImpl->dispose();
    }


    void UnoControlTableModel::removeAllColumns()
    {
        DBG_CHECK_ME();
        if ( m_pImpl->aColumns.empty() )
            return;

        // dispose the column instances
        for (   ColumnModels::const_iterator col = m_pImpl->aColumns.begin();
                col != m_pImpl->aColumns.end();
                ++col
            )
        {
            UnoGridColumnFacade* pColumn = dynamic_cast< UnoGridColumnFacade* >( col->get() );
            if ( !pColumn )
            {
                SAL_WARN( "svtools.uno", "UnoControlTableModel::removeAllColumns: illegal column implementation!" );
                continue;
            }

            pColumn->dispose();
        }
        m_pImpl->aColumns.clear();

        // notify listeners
        ModellListeners aListeners( m_pImpl->m_aListeners );
        for (   ModellListeners::const_iterator loop = aListeners.begin();
                loop != aListeners.end();
                ++loop
            )
        {
            (*loop)->allColumnsRemoved();
        }
    }


    void UnoControlTableModel::impl_notifyTableMetricsChanged() const
    {
        ModellListeners aListeners( m_pImpl->m_aListeners );
        for (   ModellListeners::const_iterator loop = aListeners.begin();
                loop != aListeners.end();
                ++loop
            )
        {
            (*loop)->tableMetricsChanged();
        }
    }


    PTableRenderer UnoControlTableModel::getRenderer() const
    {
        DBG_CHECK_ME();
        return m_pImpl->pRenderer;
    }


    PTableInputHandler UnoControlTableModel::getInputHandler() const
    {
        DBG_CHECK_ME();
        return m_pImpl->pInputHandler;
    }


    TableMetrics UnoControlTableModel::getRowHeight() const
    {
        DBG_CHECK_ME();
        return m_pImpl->nRowHeight;
    }


    void UnoControlTableModel::setRowHeight(TableMetrics _nRowHeight)
    {
        DBG_CHECK_ME();
        if ( m_pImpl->nRowHeight == _nRowHeight )
            return;

        m_pImpl->nRowHeight = _nRowHeight;
        impl_notifyTableMetricsChanged();
    }


    TableMetrics UnoControlTableModel::getColumnHeaderHeight() const
    {
        DBG_CHECK_ME();
        DBG_ASSERT( hasColumnHeaders(), "DefaultTableModel::getColumnHeaderHeight: invalid call!" );
        return m_pImpl->nColumnHeaderHeight;
    }


    TableMetrics UnoControlTableModel::getRowHeaderWidth() const
    {
        DBG_CHECK_ME();
        DBG_ASSERT( hasRowHeaders(), "DefaultTableModel::getRowHeaderWidth: invalid call!" );
        return m_pImpl->nRowHeaderWidth;
    }

    void UnoControlTableModel::setColumnHeaderHeight(TableMetrics _nHeight)
    {
        DBG_CHECK_ME();
        if ( m_pImpl->nColumnHeaderHeight == _nHeight )
            return;

        m_pImpl->nColumnHeaderHeight = _nHeight;
        impl_notifyTableMetricsChanged();
    }


    void UnoControlTableModel::setRowHeaderWidth(TableMetrics _nWidth)
    {
        DBG_CHECK_ME();
        if ( m_pImpl->nRowHeaderWidth == _nWidth )
            return;

        m_pImpl->nRowHeaderWidth = _nWidth;
        impl_notifyTableMetricsChanged();
    }


    ScrollbarVisibility UnoControlTableModel::getVerticalScrollbarVisibility() const
    {
        DBG_CHECK_ME();
        return m_pImpl->eVScrollMode;
    }


    ScrollbarVisibility UnoControlTableModel::getHorizontalScrollbarVisibility() const
    {
        DBG_CHECK_ME();
        return m_pImpl->eHScrollMode;
    }


    void UnoControlTableModel::addTableModelListener( const PTableModelListener& i_listener )
    {
        DBG_CHECK_ME();
        ENSURE_OR_RETURN_VOID( !!i_listener, "illegal NULL listener" );
        m_pImpl->m_aListeners.push_back( i_listener );
    }


    void UnoControlTableModel::removeTableModelListener( const PTableModelListener& i_listener )
    {
        DBG_CHECK_ME();
        for (   ModellListeners::iterator lookup = m_pImpl->m_aListeners.begin();
                lookup != m_pImpl->m_aListeners.end();
                ++lookup
            )
        {
            if ( *lookup == i_listener )
            {
                m_pImpl->m_aListeners.erase( lookup );
                return;
            }
        }
        OSL_ENSURE( false, "UnoControlTableModel::removeTableModelListener: listener is not registered - sure you're doing the right thing here?" );
    }


    void UnoControlTableModel::setVerticalScrollbarVisibility( ScrollbarVisibility const i_visibility ) const
    {
        DBG_CHECK_ME();
        m_pImpl->eVScrollMode = i_visibility;
    }


    void UnoControlTableModel::setHorizontalScrollbarVisibility( ScrollbarVisibility const i_visibility ) const
    {
        DBG_CHECK_ME();
        m_pImpl->eHScrollMode = i_visibility;
    }


    void UnoControlTableModel::setDataModel( Reference< XGridDataModel > const & i_gridDataModel )
    {
        DBG_CHECK_ME();
        m_pImpl->m_aDataModel = i_gridDataModel;
        // TODO: register as listener, so we're notified of row/data changes, and can multiplex them to our
        // own listeners
    }


    Reference< XGridDataModel > UnoControlTableModel::getDataModel() const
    {
        Reference< XGridDataModel > const xDataModel( m_pImpl->m_aDataModel );
        return xDataModel;
    }


    bool UnoControlTableModel::hasDataModel() const
    {
        return getDataModel().is();
    }


    void UnoControlTableModel::setColumnModel( Reference< XGridColumnModel > const & i_gridColumnModel )
    {
        DBG_CHECK_ME();
        m_pImpl->m_aColumnModel = i_gridColumnModel;
    }


    Reference< XGridColumnModel > UnoControlTableModel::getColumnModel() const
    {
        Reference< XGridColumnModel > const xColumnModel( m_pImpl->m_aColumnModel );
        return xColumnModel;
    }


    bool UnoControlTableModel::hasColumnModel() const
    {
        return getColumnModel().is();
    }


    void UnoControlTableModel::getCellContent( ColPos const i_col, RowPos const i_row, Any& o_cellContent )
    {
        DBG_CHECK_ME();

        o_cellContent.clear();
        try
        {
            Reference< XGridDataModel > const xDataModel( m_pImpl->m_aDataModel );
            ENSURE_OR_RETURN_VOID( xDataModel.is(), "UnoControlTableModel::getCellContent: no data model anymore!" );

            PColumnModel const pColumn = getColumnModel( i_col );
            UnoGridColumnFacade* pColumnImpl = dynamic_cast< UnoGridColumnFacade* >( pColumn.get() );
            ENSURE_OR_RETURN_VOID( pColumnImpl != NULL, "UnoControlTableModel::getCellContent: no (valid) column at this position!" );
            sal_Int32 const nDataColumnIndex = pColumnImpl->getDataColumnIndex() >= 0 ? pColumnImpl->getDataColumnIndex() : i_col;

            if ( nDataColumnIndex >= xDataModel->getColumnCount() )
            {
                // this is allowed, in case the column model has been dynamically extended, but the data model does
                // not (yet?) know about it.
                // So, handle it gracefully.
            #if OSL_DEBUG_LEVEL > 0
                {
                    Reference< XGridColumnModel > const xColumnModel( m_pImpl->m_aColumnModel );
                    OSL_ENSURE( xColumnModel.is() && i_col < xColumnModel->getColumnCount(),
                        "UnoControlTableModel::getCellContent: request a column's value which the ColumnModel doesn't know about!" );
                }
            #endif
            }
            else
            {
                o_cellContent = xDataModel->getCellData( nDataColumnIndex, i_row );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }


    void UnoControlTableModel::getCellToolTip( ColPos const i_col, RowPos const i_row, Any& o_cellToolTip )
    {
        DBG_CHECK_ME();
        try
        {
            Reference< XGridDataModel > const xDataModel( m_pImpl->m_aDataModel );
            ENSURE_OR_THROW( xDataModel.is(), "no data model anymore!" );

            o_cellToolTip = xDataModel->getCellToolTip( i_col, i_row );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }


    Any UnoControlTableModel::getRowHeading( RowPos const i_rowPos ) const
    {
        DBG_CHECK_ME();

        Any aRowHeading;

        Reference< XGridDataModel > const xDataModel( m_pImpl->m_aDataModel );
        ENSURE_OR_RETURN( xDataModel.is(), "UnoControlTableModel::getRowHeading: no data model anymore!", aRowHeading );

        try
        {
            aRowHeading = xDataModel->getRowHeading( i_rowPos );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return aRowHeading;
    }


    namespace
    {
        void lcl_setColor( Any const & i_color, ::boost::optional< ::Color > & o_convertedColor )
        {
            if ( !i_color.hasValue() )
                o_convertedColor.reset();
            else
            {
                sal_Int32 nColor = COL_TRANSPARENT;
                if ( i_color >>= nColor )
                {
                    o_convertedColor.reset( ::Color( nColor ) );
                }
                else
                {
                    OSL_ENSURE( false, "lcl_setColor: could not extract color value!" );
                }
            }
        }
    }


    ::boost::optional< ::Color > UnoControlTableModel::getLineColor() const
    {
        DBG_CHECK_ME();
        return m_pImpl->m_aGridLineColor;
    }


    void UnoControlTableModel::setLineColor( Any const & i_color )
    {
        DBG_CHECK_ME();
        lcl_setColor( i_color, m_pImpl->m_aGridLineColor );
    }


    ::boost::optional< ::Color > UnoControlTableModel::getHeaderBackgroundColor() const
    {
        DBG_CHECK_ME();
        return m_pImpl->m_aHeaderBackgroundColor;
    }


    void UnoControlTableModel::setHeaderBackgroundColor( Any const & i_color )
    {
        DBG_CHECK_ME();
        lcl_setColor( i_color, m_pImpl->m_aHeaderBackgroundColor );
    }


    ::boost::optional< ::Color > UnoControlTableModel::getHeaderTextColor() const
    {
        DBG_CHECK_ME();
        return m_pImpl->m_aHeaderTextColor;
    }


    ::boost::optional< ::Color > UnoControlTableModel::getActiveSelectionBackColor() const
    {
        DBG_CHECK_ME();
        return m_pImpl->m_aActiveSelectionBackColor;
    }


    ::boost::optional< ::Color > UnoControlTableModel::getInactiveSelectionBackColor() const
    {
        DBG_CHECK_ME();
        return m_pImpl->m_aInactiveSelectionBackColor;
    }


    ::boost::optional< ::Color > UnoControlTableModel::getActiveSelectionTextColor() const
    {
        DBG_CHECK_ME();
        return m_pImpl->m_aActiveSelectionTextColor;
    }


    ::boost::optional< ::Color > UnoControlTableModel::getInactiveSelectionTextColor() const
    {
        DBG_CHECK_ME();
        return m_pImpl->m_aInactiveSelectionTextColor;
    }


    void UnoControlTableModel::setHeaderTextColor( Any const & i_color )
    {
        DBG_CHECK_ME();
        lcl_setColor( i_color, m_pImpl->m_aHeaderTextColor );
    }


    void UnoControlTableModel::setActiveSelectionBackColor( Any const & i_color )
    {
        DBG_CHECK_ME();
        lcl_setColor( i_color, m_pImpl->m_aActiveSelectionBackColor );
    }


    void UnoControlTableModel::setInactiveSelectionBackColor( Any const & i_color )
    {
        DBG_CHECK_ME();
        lcl_setColor( i_color, m_pImpl->m_aInactiveSelectionBackColor );
    }


    void UnoControlTableModel::setActiveSelectionTextColor( Any const & i_color )
    {
        DBG_CHECK_ME();
        lcl_setColor( i_color, m_pImpl->m_aActiveSelectionTextColor );
    }


    void UnoControlTableModel::setInactiveSelectionTextColor( Any const & i_color )
    {
        DBG_CHECK_ME();
        lcl_setColor( i_color, m_pImpl->m_aInactiveSelectionTextColor );
    }


    ::boost::optional< ::Color > UnoControlTableModel::getTextColor() const
    {
        DBG_CHECK_ME();
        return m_pImpl->m_aTextColor;
    }


    void UnoControlTableModel::setTextColor( Any const & i_color )
    {
        DBG_CHECK_ME();
        lcl_setColor( i_color, m_pImpl->m_aTextColor );
    }


    ::boost::optional< ::Color > UnoControlTableModel::getTextLineColor() const
    {
        DBG_CHECK_ME();
        return m_pImpl->m_aTextColor;
    }


    void UnoControlTableModel::setTextLineColor( Any const & i_color )
    {
        DBG_CHECK_ME();
        lcl_setColor( i_color, m_pImpl->m_aTextLineColor );
    }


    ::boost::optional< ::std::vector< ::Color > > UnoControlTableModel::getRowBackgroundColors() const
    {
        DBG_CHECK_ME();
        return m_pImpl->m_aRowColors;
    }


    void UnoControlTableModel::setRowBackgroundColors( css::uno::Any const & i_APIValue )
    {
        DBG_CHECK_ME();
        Sequence< css::util::Color > aAPIColors;
        if ( !( i_APIValue >>= aAPIColors ) )
            m_pImpl->m_aRowColors.reset();
        else
        {
            ::std::vector< ::Color > aColors( aAPIColors.getLength() );
            for ( sal_Int32 i=0; i<aAPIColors.getLength(); ++i )
            {
                aColors[i] = ::Color( aAPIColors[i] );
            }
            m_pImpl->m_aRowColors.reset( aColors );
        }
    }


    VerticalAlignment UnoControlTableModel::getVerticalAlign() const
    {
        DBG_CHECK_ME();
        return  m_pImpl->m_eVerticalAlign;
    }


    void UnoControlTableModel::setVerticalAlign( VerticalAlignment _xAlign )
    {
        DBG_CHECK_ME();
        m_pImpl->m_eVerticalAlign = _xAlign;
    }


    ColPos UnoControlTableModel::getColumnPos( UnoGridColumnFacade const & i_column ) const
    {
        DBG_CHECK_ME();
        for (   ColumnModels::const_iterator col = m_pImpl->aColumns.begin();
                col != m_pImpl->aColumns.end();
                ++col
            )
        {
            if ( &i_column == col->get() )
                return col - m_pImpl->aColumns.begin();
        }
        OSL_ENSURE( false, "UnoControlTableModel::getColumnPos: column not found!" );
        return COL_INVALID;
    }


    ITableDataSort* UnoControlTableModel::getSortAdapter()
    {
        DBG_CHECK_ME();

        Reference< XSortableGridData > const xSortAccess( getDataModel(), UNO_QUERY );
        if ( xSortAccess.is() )
            return this;
        return NULL;
    }


    bool UnoControlTableModel::isEnabled() const
    {
        DBG_CHECK_ME();
        return m_pImpl->bEnabled;
    }


    void UnoControlTableModel::setEnabled( bool _bEnabled )
    {
        DBG_CHECK_ME();
        m_pImpl->bEnabled = _bEnabled;
    }


    void UnoControlTableModel::sortByColumn( ColPos const i_column, ColumnSortDirection const i_sortDirection )
    {
        DBG_CHECK_ME();

        try
        {
            Reference< XSortableGridData > const xSortAccess( getDataModel(), UNO_QUERY_THROW );
            xSortAccess->sortByColumn( i_column, i_sortDirection == ColumnSortAscending );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }


    ColumnSort UnoControlTableModel::getCurrentSortOrder() const
    {
        DBG_CHECK_ME();

        ColumnSort currentSort;
        try
        {
            Reference< XSortableGridData > const xSortAccess( getDataModel(), UNO_QUERY_THROW );
            Pair< ::sal_Int32, sal_Bool > const aCurrentSortOrder( xSortAccess->getCurrentSortOrder() );
            currentSort.nColumnPos = aCurrentSortOrder.First;
            currentSort.eSortDirection = aCurrentSortOrder.Second ? ColumnSortAscending : ColumnSortDescending;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return currentSort;
    }


    void UnoControlTableModel::notifyColumnChange( ColPos const i_columnPos, ColumnAttributeGroup const i_attributeGroup ) const
    {
        DBG_CHECK_ME();
        ENSURE_OR_RETURN_VOID( ( i_columnPos >= 0 ) && ( i_columnPos < getColumnCount() ),
            "UnoControlTableModel::notifyColumnChange: invalid column index!" );

        ModellListeners aListeners( m_pImpl->m_aListeners );
        for (   ModellListeners::const_iterator loop = aListeners.begin();
                loop != aListeners.end();
                ++loop
            )
        {
            (*loop)->columnChanged( i_columnPos, i_attributeGroup );
        }
    }


    void UnoControlTableModel::notifyRowsInserted( GridDataEvent const & i_event ) const
    {
        // check sanity of the event
        ENSURE_OR_RETURN_VOID( i_event.FirstRow >= 0, "UnoControlTableModel::notifyRowsInserted: invalid first row!" );
        ENSURE_OR_RETURN_VOID( i_event.LastRow >= i_event.FirstRow, "UnoControlTableModel::notifyRowsInserted: invalid row indexes!" );

        // check own sanity
        Reference< XGridColumnModel > const xColumnModel( m_pImpl->m_aColumnModel );
        ENSURE_OR_RETURN_VOID( xColumnModel.is(), "UnoControlTableModel::notifyRowsInserted: no column model anymore!" );

        Reference< XGridDataModel > const xDataModel( m_pImpl->m_aDataModel );
        ENSURE_OR_RETURN_VOID( xDataModel.is(), "UnoControlTableModel::notifyRowsInserted: no data model anymore!" );

        // implicitly add columns to the column model
        // TODO: is this really a good idea?
        sal_Int32 const dataColumnCount = xDataModel->getColumnCount();
        OSL_ENSURE( dataColumnCount > 0, "UnoControlTableModel::notifyRowsInserted: no columns at all?" );

        sal_Int32 const modelColumnCount = xColumnModel->getColumnCount();
        if ( ( modelColumnCount == 0 ) && ( dataColumnCount > 0 ) )
        {
            // TODO: shouldn't we clear the mutexes guard for this call?
            xColumnModel->setDefaultColumns( dataColumnCount );
        }

        // multiplex the event to our own listeners
        ModellListeners aListeners( m_pImpl->m_aListeners );
        for (   ModellListeners::const_iterator loop = aListeners.begin();
                loop != aListeners.end();
                ++loop
            )
        {
            (*loop)->rowsInserted( i_event.FirstRow, i_event.LastRow );
        }
    }


    void UnoControlTableModel::notifyRowsRemoved( GridDataEvent const & i_event ) const
    {
        ModellListeners aListeners( m_pImpl->m_aListeners );
        for (   ModellListeners::const_iterator loop = aListeners.begin();
                loop != aListeners.end();
                ++loop
            )
        {
            (*loop)->rowsRemoved( i_event.FirstRow, i_event.LastRow );
        }
    }


    void UnoControlTableModel::notifyDataChanged( css::awt::grid::GridDataEvent const & i_event ) const
    {
        RowPos const firstRow = i_event.FirstRow == -1 ? 0 : i_event.FirstRow;
        RowPos const lastRow = i_event.FirstRow == -1 ? getRowCount() - 1 : i_event.LastRow;

        ModellListeners aListeners( m_pImpl->m_aListeners );
        for (   ModellListeners::const_iterator loop = aListeners.begin();
                loop != aListeners.end();
                ++loop
            )
        {
            (*loop)->cellsUpdated( firstRow, lastRow );
        }
    }


    void UnoControlTableModel::notifyAllDataChanged() const
    {
        ModellListeners aListeners( m_pImpl->m_aListeners );
        for (   ModellListeners::const_iterator loop = aListeners.begin();
                loop != aListeners.end();
                ++loop
            )
        {
            (*loop)->cellsUpdated( 0, getRowCount() - 1 );
        }
    }


} } // svt::table

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
