/*************************************************************************
 *
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

#include "svtools/table/defaultinputhandler.hxx"
#include "svtools/table/gridtablerenderer.hxx"
#include "svtools/table/tablecontrol.hxx"
#include "unocontroltablemodel.hxx"

#include <com/sun/star/awt/grid/XGridColumn.hpp>
#include <com/sun/star/view/SelectionType.hpp>
#include <comphelper/processfactory.hxx>

#include <comphelper/sequence.hxx>
#include <comphelper/stlunosequence.hxx>
#include <rtl/ref.hxx>
#include <toolkit/helper/property.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>

using ::rtl::OUString;
using namespace ::svt::table;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt::grid;

    //--------------------------------------------------------------------
    namespace
    {
        template< class ATTRIBUTE_TYPE >
        void lcl_set( Reference< XGridColumn > const & i_column, void ( SAL_CALL XGridColumn::*i_setter )( ATTRIBUTE_TYPE ),
            ATTRIBUTE_TYPE i_value )
        {
            try
            {
                (i_column.get()->*i_setter) ( i_value );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }

        template< class ATTRIBUTE_TYPE >
        ATTRIBUTE_TYPE lcl_get( Reference< XGridColumn > const & i_column, ATTRIBUTE_TYPE ( SAL_CALL XGridColumn::*i_getter )() )
        {
            ATTRIBUTE_TYPE value = ATTRIBUTE_TYPE();
            try
            {
                value = (i_column.get()->*i_getter)();
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            return value;
        }
    }

    //--------------------------------------------------------------------
    UnoControlTableColumn::UnoControlTableColumn( const Reference< XGridColumn >& i_gridColumn )
        :m_nID( 0 )
        ,m_xGridColumn( i_gridColumn, UNO_QUERY_THROW )
    {
    }

    //--------------------------------------------------------------------
    ColumnID UnoControlTableColumn::getID() const
    {
        return m_nID;
    }

    //--------------------------------------------------------------------
    bool UnoControlTableColumn::setID( const ColumnID _nID )
    {
        m_nID = _nID;
        return true;
    }

    //--------------------------------------------------------------------
    String UnoControlTableColumn::getName() const
    {
        ::rtl::OUString sName;
        try
        {
            sName = m_xGridColumn->getTitle();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return sName;
    }

    //--------------------------------------------------------------------
    void UnoControlTableColumn::setName( const String& _rName )
    {
        try
        {
            m_xGridColumn->setTitle( _rName );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //--------------------------------------------------------------------
    bool UnoControlTableColumn::isResizable() const
    {
        return lcl_get( m_xGridColumn, &XGridColumn::getResizeable );
    }

    //--------------------------------------------------------------------
    void UnoControlTableColumn::setResizable( bool _bResizable )
    {
        return lcl_set( m_xGridColumn, &XGridColumn::setResizeable, sal_Bool( _bResizable ) );
    }

    //--------------------------------------------------------------------
    TableMetrics UnoControlTableColumn::getWidth() const
    {
        return lcl_get( m_xGridColumn, &XGridColumn::getColumnWidth );
    }

    //--------------------------------------------------------------------
    void UnoControlTableColumn::setWidth( TableMetrics _nWidth )
    {
        lcl_set( m_xGridColumn, &XGridColumn::setColumnWidth, _nWidth );
    }

    //--------------------------------------------------------------------
    TableMetrics UnoControlTableColumn::getMinWidth() const
    {
        return lcl_get( m_xGridColumn, &XGridColumn::getMinWidth );
    }

    //--------------------------------------------------------------------
    void UnoControlTableColumn::setMinWidth( TableMetrics _nMinWidth )
    {
        lcl_set( m_xGridColumn, &XGridColumn::setMinWidth, _nMinWidth );
    }

    //--------------------------------------------------------------------
    TableMetrics UnoControlTableColumn::getMaxWidth() const
    {
        return lcl_get( m_xGridColumn, &XGridColumn::getMaxWidth );
    }

    //--------------------------------------------------------------------
    void UnoControlTableColumn::setMaxWidth( TableMetrics _nMaxWidth )
    {
        lcl_set( m_xGridColumn, &XGridColumn::setMinWidth, _nMaxWidth );
    }

    //--------------------------------------------------------------------
    TableMetrics UnoControlTableColumn::getPreferredWidth() const
    {
        return lcl_get( m_xGridColumn, &XGridColumn::getPreferredWidth );
    }

    //--------------------------------------------------------------------
    void UnoControlTableColumn::setPreferredWidth( TableMetrics _nPrefWidth )
    {
        lcl_set( m_xGridColumn, &XGridColumn::setPreferredWidth, _nPrefWidth );
    }

    //--------------------------------------------------------------------
    ::com::sun::star::style::HorizontalAlignment UnoControlTableColumn::getHorizontalAlign()
    {
        return lcl_get( m_xGridColumn, &XGridColumn::getHorizontalAlign );
    }

    //--------------------------------------------------------------------
    void UnoControlTableColumn::setHorizontalAlign( com::sun::star::style::HorizontalAlignment _align )
    {
        lcl_set( m_xGridColumn, &XGridColumn::setHorizontalAlign, _align );
    }

    //====================================================================
    //= DefaultTableModel_Impl
    //====================================================================
    typedef ::std::vector< PTableModelListener >    ModellListeners;
    struct UnoControlTableModel_Impl
    {
        ::std::vector< PColumnModel >               aColumns;
        TableSize                                   nRowCount;
        bool                                        bHasColumnHeaders;
        bool                                        bHasRowHeaders;
        ScrollbarVisibility                         eVScrollMode;
        ScrollbarVisibility                         eHScrollMode;
        PTableRenderer                              pRenderer;
        PTableInputHandler                          pInputHandler;
        TableMetrics                                nRowHeight;
        TableMetrics                                nColumnHeaderHeight;
        TableMetrics                                nRowHeaderWidth;
        ::std::vector< ::rtl::OUString >            aRowHeadersTitle;
        ::std::vector< ::std::vector< Any > >       aCellContent;
        ::com::sun::star::util::Color               m_nLineColor;
        ::com::sun::star::util::Color               m_nHeaderColor;
        ::com::sun::star::util::Color               m_nTextColor;
        ::com::sun::star::util::Color               m_nRowColor1;
        ::com::sun::star::util::Color               m_nRowColor2;
        ::com::sun::star::style::VerticalAlignment  m_eVerticalAlign;
        ModellListeners                             m_aListeners;

        UnoControlTableModel_Impl()
            :aColumns           ( )
            ,nRowCount          ( 0         )
            ,bHasColumnHeaders  ( true      )
            ,bHasRowHeaders     ( false     )
            ,eVScrollMode       ( ScrollbarShowNever )
            ,eHScrollMode       ( ScrollbarShowNever )
            ,pRenderer          (           )
            ,pInputHandler      (           )
            ,nRowHeight         ( 10 )
            ,nColumnHeaderHeight( 10 )
            ,nRowHeaderWidth    ( 10 )
            ,aRowHeadersTitle   ( )
            ,aCellContent       ( )
            ,m_nLineColor       ( 0xFFFFFF )
            ,m_nHeaderColor     ( 0xFFFFFF )
            ,m_nTextColor       ( 0 )//black as default
            ,m_nRowColor1       ( 0xFFFFFF )
            ,m_nRowColor2       ( 0xFFFFFF )
            ,m_eVerticalAlign   ( com::sun::star::style::VerticalAlignment_TOP )
        {
        }
    };

    //====================================================================
    //= UnoControlTableModel
    //====================================================================
    //--------------------------------------------------------------------
    UnoControlTableModel::UnoControlTableModel()
        :m_pImpl( new UnoControlTableModel_Impl )
    {
        m_pImpl->bHasColumnHeaders = true;
        m_pImpl->bHasRowHeaders = false;
        m_pImpl->pRenderer.reset( new GridTableRenderer( *this ) );
        m_pImpl->pInputHandler.reset( new DefaultInputHandler );
    }

    //--------------------------------------------------------------------
    UnoControlTableModel::~UnoControlTableModel()
    {
        DELETEZ( m_pImpl );
    }

    //--------------------------------------------------------------------
    TableSize UnoControlTableModel::getColumnCount() const
    {
        return (TableSize)m_pImpl->aColumns.size();
    }

    //--------------------------------------------------------------------
    TableSize UnoControlTableModel::getRowCount() const
    {
        return m_pImpl->nRowCount;
    }

    //--------------------------------------------------------------------
    bool UnoControlTableModel::hasColumnHeaders() const
    {
        return m_pImpl->bHasColumnHeaders;
    }

    //--------------------------------------------------------------------
    bool UnoControlTableModel::hasRowHeaders() const
    {
        return m_pImpl->bHasRowHeaders;
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::setRowHeaders(bool _bRowHeaders)
    {
        m_pImpl->bHasRowHeaders = _bRowHeaders;
    }
    //--------------------------------------------------------------------
    void UnoControlTableModel::setColumnHeaders(bool _bColumnHeaders)
    {
        m_pImpl->bHasColumnHeaders = _bColumnHeaders;
    }

    void UnoControlTableModel::setColumnCount(TableSize _nColCount)
    {
       m_pImpl->aColumns.resize( _nColCount);
    }
    //--------------------------------------------------------------------
    bool UnoControlTableModel::isCellEditable( ColPos col, RowPos row ) const
    {
        (void)col;
        (void)row;
        return false;
    }

    //--------------------------------------------------------------------
    PColumnModel UnoControlTableModel::getColumnModel( ColPos column )
    {
        ENSURE_OR_RETURN( ( column >= 0 ) && ( column < getColumnCount() ),
            "DefaultTableModel::getColumnModel: invalid index!", PColumnModel() );
        return m_pImpl->aColumns[ column ];
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::appendColumn( const PColumnModel& i_column )
    {
        insertColumn( m_pImpl->aColumns.size(), i_column );
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::insertColumn( ColPos const i_position, const PColumnModel& i_column )
    {
        ENSURE_OR_RETURN_VOID( ( i_position >= 0 ) && ( i_position <= m_pImpl->aColumns.size() ), "illegal position!" );
        ENSURE_OR_RETURN_VOID( !!i_column, "illegal column" );

        m_pImpl->aColumns.insert( m_pImpl->aColumns.begin() + i_position, i_column );

        // notify listeners
        ModellListeners aListeners( m_pImpl->m_aListeners );
        for (   ModellListeners::const_iterator loop = aListeners.begin();
                loop != aListeners.end();
                ++loop
            )
        {
            (*loop)->columnsInserted( i_position, i_position );
        }
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::removeAllColumns()
    {
        if ( m_pImpl->aColumns.empty() )
            return;

        const ColPos nLastIndex = m_pImpl->aColumns.size() - 1;
        m_pImpl->aColumns.clear();

        // notify listeners
        ModellListeners aListeners( m_pImpl->m_aListeners );
        for (   ModellListeners::const_iterator loop = aListeners.begin();
                loop != aListeners.end();
                ++loop
            )
        {
            (*loop)->columnsRemoved( 0, nLastIndex );
        }
    }

    //--------------------------------------------------------------------
    PTableRenderer UnoControlTableModel::getRenderer() const
    {
        return m_pImpl->pRenderer;
    }

    //--------------------------------------------------------------------
    PTableInputHandler UnoControlTableModel::getInputHandler() const
    {
        return m_pImpl->pInputHandler;
    }

    //--------------------------------------------------------------------
    TableMetrics UnoControlTableModel::getRowHeight() const
    {
        return m_pImpl->nRowHeight;
    }
        //--------------------------------------------------------------------
    void UnoControlTableModel::setRowHeight(TableMetrics _nRowHeight)
    {
        m_pImpl->nRowHeight = _nRowHeight;
    }

    //--------------------------------------------------------------------
    TableMetrics UnoControlTableModel::getColumnHeaderHeight() const
    {
        DBG_ASSERT( hasColumnHeaders(), "DefaultTableModel::getColumnHeaderHeight: invalid call!" );
        return m_pImpl->nColumnHeaderHeight;
    }

    //--------------------------------------------------------------------
    TableMetrics UnoControlTableModel::getRowHeaderWidth() const
    {
        DBG_ASSERT( hasRowHeaders(), "DefaultTableModel::getRowHeaderWidth: invalid call!" );
        return m_pImpl->nRowHeaderWidth;
    }
    //--------------------------------------------------------------------
    void UnoControlTableModel::setColumnHeaderHeight(TableMetrics _nHeight)
    {
        m_pImpl->nColumnHeaderHeight = _nHeight;
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::setRowHeaderWidth(TableMetrics _nWidth)
    {
        m_pImpl->nRowHeaderWidth = _nWidth;
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::SetTitleHeight( TableMetrics _nHeight )
    {
        DBG_ASSERT( _nHeight > 0, "DefaultTableModel::SetTitleHeight: invalid height value!" );
        m_pImpl->nColumnHeaderHeight = _nHeight;
        // TODO: notification
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::SetHandleWidth( TableMetrics _nWidth )
    {
        DBG_ASSERT( _nWidth > 0, "DefaultTableModel::SetHandleWidth: invalid width value!" );
        m_pImpl->nRowHeaderWidth = _nWidth;
        // TODO: notification
    }

    //--------------------------------------------------------------------
    ScrollbarVisibility UnoControlTableModel::getVerticalScrollbarVisibility() const
    {
        return m_pImpl->eVScrollMode;
    }

    //--------------------------------------------------------------------
    ScrollbarVisibility UnoControlTableModel::getHorizontalScrollbarVisibility() const
    {
        return m_pImpl->eHScrollMode;
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::addTableModelListener( const PTableModelListener& i_listener )
    {
        ENSURE_OR_RETURN_VOID( !!i_listener, "illegal NULL listener" );
        m_pImpl->m_aListeners.push_back( i_listener );
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::removeTableModelListener( const PTableModelListener& i_listener )
    {
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

    //--------------------------------------------------------------------
    void UnoControlTableModel::setVerticalScrollbarVisibility( ScrollbarVisibility const i_visibility ) const
    {
        m_pImpl->eVScrollMode = i_visibility;
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::setHorizontalScrollbarVisibility( ScrollbarVisibility const i_visibility ) const
    {
        m_pImpl->eHScrollMode = i_visibility;
    }
    //--------------------------------------------------------------------
    void UnoControlTableModel::setCellContent(const std::vector<std::vector< Any > >& cellContent)
    {
        m_pImpl->aCellContent = cellContent;
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::getCellContent( RowPos const i_row, ColPos const i_col, Any& o_cellContent )
    {
        ENSURE_OR_RETURN_VOID( ( i_row >= 0 ) && ( i_row < m_pImpl->aCellContent.size() ),
            "UnoControlTableModel::getCellContent: illegal row index!" );
        ::std::vector< Any >& rRowContent( m_pImpl->aCellContent[ i_row ] );
        ENSURE_OR_RETURN_VOID( ( i_col >= 0 ) && ( i_col < rRowContent.size() ),
            "UnoControlTableModel::getCellContent: illegal column index" );
        o_cellContent = rRowContent[ i_col ];
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::updateCellContent( RowPos const i_row, ColPos const i_col, Any const & i_cellContent )
    {
        ENSURE_OR_RETURN_VOID( ( i_row >= 0 ) && ( i_row < m_pImpl->aCellContent.size() ),
            "UnoControlTableModel::getCellContent: illegal row index!" );
        ::std::vector< Any >& rRowContent( m_pImpl->aCellContent[ i_row ] );
        ENSURE_OR_RETURN_VOID( ( i_col >= 0 ) && ( i_col < rRowContent.size() ),
            "UnoControlTableModel::getCellContent: illegal column index" );
        rRowContent[ i_col ] = i_cellContent;

        // TODO: listener notification
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::setRowHeaderNames( const Sequence< ::rtl::OUString >& i_rowHeaders )
    {
        ENSURE_OR_RETURN_VOID( size_t( i_rowHeaders.getLength() ) == m_pImpl->aRowHeadersTitle.size(),
            "UnoControlTableModel::setRowHeaderNames: illegal number of row headers!" );
            // this method is not intended to set a new row count, but only to modify the existing row headers

        ::std::copy(
            ::comphelper::stl_begin( i_rowHeaders ),
            ::comphelper::stl_end( i_rowHeaders ),
            m_pImpl->aRowHeadersTitle.begin()
        );

        // TODO: listener notification
    }

    //--------------------------------------------------------------------
    ::rtl::OUString UnoControlTableModel::getRowHeader( RowPos const i_rowPos ) const
    {
        ENSURE_OR_RETURN( ( i_rowPos >= 0 ) && ( i_rowPos < m_pImpl->aRowHeadersTitle.size() ),
            "UnoControlTableModel::getRowHeader: illegal row position!", ::rtl::OUString() );
        return m_pImpl->aRowHeadersTitle[ i_rowPos ];
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::appendRow( Sequence< Any > const & i_rowData, ::rtl::OUString const & i_rowHeader )
    {
        ENSURE_OR_RETURN_VOID( i_rowData.getLength() == getColumnCount(), "UnoControlTableModel::appendRow: invalid row data!" );

        // add row data
        ::std::vector< Any > newRow( i_rowData.getLength() );
        if ( !newRow.empty() )
            ::std::copy(
                ::comphelper::stl_begin( i_rowData ),
                ::comphelper::stl_end( i_rowData ),
                newRow.begin()
            );

        m_pImpl->aCellContent.push_back( newRow );

        if ( hasRowHeaders() )
            m_pImpl->aRowHeadersTitle.push_back( i_rowHeader );

        ++m_pImpl->nRowCount;

        // TODO: listener notification
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::removeRow( RowPos const i_rowPos )
    {
        ENSURE_OR_RETURN_VOID( ( i_rowPos >= 0 ) && ( i_rowPos < getRowCount() ), "UnoControlTableModel::removeRow: illegal row position!" );

        if ( hasRowHeaders() )
            m_pImpl->aRowHeadersTitle.erase( m_pImpl->aRowHeadersTitle.begin() + i_rowPos );

        const ::std::vector< ::std::vector< Any > >::iterator contentPos = m_pImpl->aCellContent.begin() + i_rowPos;
        m_pImpl->aCellContent.erase( contentPos );

        --m_pImpl->nRowCount;

        // TODO: listener notification
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::clearAllRows()
    {
        if ( hasRowHeaders() )
        {
            ::std::vector< ::rtl::OUString > aEmpty;
            m_pImpl->aRowHeadersTitle.swap( aEmpty );
        }

        ::std::vector< ::std::vector< Any > > aEmptyContent;
        m_pImpl->aCellContent.swap( aEmptyContent );

        m_pImpl->nRowCount = 0;

        // TODO: listener notification
    }

    //--------------------------------------------------------------------
    ::com::sun::star::util::Color UnoControlTableModel::getLineColor()
    {
        return  m_pImpl->m_nLineColor;
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::setLineColor( ::com::sun::star::util::Color _rColor )
    {
         m_pImpl->m_nLineColor = _rColor;
    }
    //--------------------------------------------------------------------
    ::com::sun::star::util::Color UnoControlTableModel::getHeaderBackgroundColor()
    {
        return  m_pImpl->m_nHeaderColor;
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::setHeaderBackgroundColor( ::com::sun::star::util::Color _rColor )
    {
        m_pImpl->m_nHeaderColor = _rColor;
    }
    //--------------------------------------------------------------------
    ::com::sun::star::util::Color UnoControlTableModel::getTextColor()
    {
        return  m_pImpl->m_nTextColor;
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::setTextColor( ::com::sun::star::util::Color _rColor )
    {
         m_pImpl->m_nTextColor = _rColor;
    }
    //--------------------------------------------------------------------
    ::com::sun::star::util::Color UnoControlTableModel::getOddRowBackgroundColor()
    {
        return  m_pImpl->m_nRowColor1;
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::setOddRowBackgroundColor( ::com::sun::star::util::Color _rColor )
    {
        m_pImpl->m_nRowColor1 = _rColor;
    }
    //--------------------------------------------------------------------
    ::com::sun::star::util::Color UnoControlTableModel::getEvenRowBackgroundColor()
    {
        return  m_pImpl->m_nRowColor2;
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::setEvenRowBackgroundColor( ::com::sun::star::util::Color _rColor )
    {
        m_pImpl->m_nRowColor2 = _rColor;
    }
    //--------------------------------------------------------------------
    ::com::sun::star::style::VerticalAlignment UnoControlTableModel::getVerticalAlign()
    {
        return  m_pImpl->m_eVerticalAlign;
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::setVerticalAlign( com::sun::star::style::VerticalAlignment _xAlign )
    {
         m_pImpl->m_eVerticalAlign = _xAlign;
    }

