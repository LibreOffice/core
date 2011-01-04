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

#include "unocontroltablemodel.hxx"
#include "unogridcolumnfacade.hxx"

#include "svtools/table/defaultinputhandler.hxx"
#include "svtools/table/gridtablerenderer.hxx"
#include "svtools/table/tablecontrol.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/awt/grid/XGridColumn.hpp>
#include <com/sun/star/view/SelectionType.hpp>
#include <com/sun/star/awt/grid/XGridColumnListener.hpp>
/** === end UNO includes === **/

#include <comphelper/stlunosequence.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>

// .....................................................................................................................
namespace svt { namespace table
{
// .....................................................................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::awt::grid::XGridColumn;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::awt::grid::XGridColumnListener;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::awt::grid::GridColumnEvent;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::style::HorizontalAlignment_LEFT;
    using ::com::sun::star::style::HorizontalAlignment;
    /** === end UNO using === **/

    //==================================================================================================================
    //= UnoControlTableModel_Impl
    //==================================================================================================================
    typedef ::std::vector< PTableModelListener >    ModellListeners;
    typedef ::std::vector< PColumnModel >           ColumnModels;
    struct UnoControlTableModel_Impl
    {
        ColumnModels                                aColumns;
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
            ,m_nLineColor       ( COL_TRANSPARENT )
            ,m_nHeaderColor     ( COL_TRANSPARENT )
            ,m_nTextColor       ( 0 )//black as default
            ,m_nRowColor1       ( COL_TRANSPARENT )
            ,m_nRowColor2       ( COL_TRANSPARENT )
            ,m_eVerticalAlign   ( com::sun::star::style::VerticalAlignment_TOP )
        {
        }
    };

    //==================================================================================================================
    //= UnoControlTableModel
    //==================================================================================================================
#ifdef DBG_UTIL
    const char* UnoControlTableModel_checkInvariants( const void* _pInstance )
    {
        return static_cast< const UnoControlTableModel* >( _pInstance )->checkInvariants();
    }

    //------------------------------------------------------------------------------------------------------------------
    const char* UnoControlTableModel::checkInvariants() const
    {
        if ( m_pImpl->aRowHeadersTitle.size() != m_pImpl->aCellContent.size() )
            return "inconsistency between data and row header arrays";

        // TODO: more?

        return NULL;
    }
#endif

#define DBG_CHECK_ME() \
    DBG_TESTSOLARMUTEX(); \
    DBG_CHKTHIS( UnoControlTableModel, UnoControlTableModel_checkInvariants )

    //------------------------------------------------------------------------------------------------------------------
    DBG_NAME( UnoControlTableModel )
    UnoControlTableModel::UnoControlTableModel()
        :m_pImpl( new UnoControlTableModel_Impl )
    {
        DBG_CTOR( UnoControlTableModel, UnoControlTableModel_checkInvariants );
        m_pImpl->bHasColumnHeaders = true;
        m_pImpl->bHasRowHeaders = false;
        m_pImpl->pRenderer.reset( new GridTableRenderer( *this ) );
        m_pImpl->pInputHandler.reset( new DefaultInputHandler );
    }

    //------------------------------------------------------------------------------------------------------------------
    UnoControlTableModel::~UnoControlTableModel()
    {
        DBG_DTOR( UnoControlTableModel, UnoControlTableModel_checkInvariants );
        DELETEZ( m_pImpl );
    }

    //------------------------------------------------------------------------------------------------------------------
    TableSize UnoControlTableModel::getColumnCount() const
    {
        DBG_CHECK_ME();
        return (TableSize)m_pImpl->aColumns.size();
    }

    //------------------------------------------------------------------------------------------------------------------
    TableSize UnoControlTableModel::getRowCount() const
    {
        DBG_CHECK_ME();
        return m_pImpl->nRowCount;
    }

    //------------------------------------------------------------------------------------------------------------------
    bool UnoControlTableModel::hasColumnHeaders() const
    {
        DBG_CHECK_ME();
        return m_pImpl->bHasColumnHeaders;
    }

    //------------------------------------------------------------------------------------------------------------------
    bool UnoControlTableModel::hasRowHeaders() const
    {
        DBG_CHECK_ME();
        return m_pImpl->bHasRowHeaders;
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoControlTableModel::setRowHeaders(bool _bRowHeaders)
    {
        DBG_CHECK_ME();
        m_pImpl->bHasRowHeaders = _bRowHeaders;
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoControlTableModel::setColumnHeaders(bool _bColumnHeaders)
    {
        DBG_CHECK_ME();
        m_pImpl->bHasColumnHeaders = _bColumnHeaders;
    }

    //------------------------------------------------------------------------------------------------------------------
    bool UnoControlTableModel::isCellEditable( ColPos col, RowPos row ) const
    {
        DBG_CHECK_ME();
        (void)col;
        (void)row;
        return false;
    }

    //------------------------------------------------------------------------------------------------------------------
    PColumnModel UnoControlTableModel::getColumnModel( ColPos column )
    {
        DBG_CHECK_ME();
        ENSURE_OR_RETURN( ( column >= 0 ) && ( column < getColumnCount() ),
            "DefaultTableModel::getColumnModel: invalid index!", PColumnModel() );
        return m_pImpl->aColumns[ column ];
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoControlTableModel::appendColumn( Reference< XGridColumn > const & i_column )
    {
        DBG_CHECK_ME();
        insertColumn( m_pImpl->aColumns.size(), i_column );
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoControlTableModel::insertColumn( ColPos const i_position, Reference< XGridColumn > const & i_column )
    {
        DBG_CHECK_ME();
        ENSURE_OR_RETURN_VOID( ( i_position >= 0 ) && ( size_t( i_position ) <= m_pImpl->aColumns.size() ), "UnoControlTableModel::insertColumn: illegal position!" );

        const PColumnModel pColumn( new UnoGridColumnFacade( *this, i_column ) );
        m_pImpl->aColumns.insert( m_pImpl->aColumns.begin() + i_position, pColumn );

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

    //------------------------------------------------------------------------------------------------------------------
    void UnoControlTableModel::removeAllColumns()
    {
        DBG_CHECK_ME();
        if ( m_pImpl->aColumns.empty() )
            return;

        const ColPos nLastIndex = m_pImpl->aColumns.size() - 1;

        // dispose the column instances
        for (   ColumnModels::const_iterator col = m_pImpl->aColumns.begin();
                col != m_pImpl->aColumns.end();
                ++col
            )
        {
            UnoGridColumnFacade* pColumn = dynamic_cast< UnoGridColumnFacade* >( col->get() );
            ENSURE_OR_CONTINUE( pColumn != NULL, "UnoControlTableModel::removeAllColumns: illegal column implementation!" );
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
            (*loop)->columnsRemoved( 0, nLastIndex );
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    PTableRenderer UnoControlTableModel::getRenderer() const
    {
        DBG_CHECK_ME();
        return m_pImpl->pRenderer;
    }

    //------------------------------------------------------------------------------------------------------------------
    PTableInputHandler UnoControlTableModel::getInputHandler() const
    {
        DBG_CHECK_ME();
        return m_pImpl->pInputHandler;
    }

    //------------------------------------------------------------------------------------------------------------------
    TableMetrics UnoControlTableModel::getRowHeight() const
    {
        DBG_CHECK_ME();
        return m_pImpl->nRowHeight;
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoControlTableModel::setRowHeight(TableMetrics _nRowHeight)
    {
        DBG_CHECK_ME();
        m_pImpl->nRowHeight = _nRowHeight;
    }

    //------------------------------------------------------------------------------------------------------------------
    TableMetrics UnoControlTableModel::getColumnHeaderHeight() const
    {
        DBG_CHECK_ME();
        DBG_ASSERT( hasColumnHeaders(), "DefaultTableModel::getColumnHeaderHeight: invalid call!" );
        return m_pImpl->nColumnHeaderHeight;
    }

    //------------------------------------------------------------------------------------------------------------------
    TableMetrics UnoControlTableModel::getRowHeaderWidth() const
    {
        DBG_CHECK_ME();
        DBG_ASSERT( hasRowHeaders(), "DefaultTableModel::getRowHeaderWidth: invalid call!" );
        return m_pImpl->nRowHeaderWidth;
    }
    //------------------------------------------------------------------------------------------------------------------
    void UnoControlTableModel::setColumnHeaderHeight(TableMetrics _nHeight)
    {
        DBG_CHECK_ME();
        m_pImpl->nColumnHeaderHeight = _nHeight;
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoControlTableModel::setRowHeaderWidth(TableMetrics _nWidth)
    {
        DBG_CHECK_ME();
        m_pImpl->nRowHeaderWidth = _nWidth;
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoControlTableModel::SetTitleHeight( TableMetrics _nHeight )
    {
        DBG_CHECK_ME();
        DBG_ASSERT( _nHeight > 0, "DefaultTableModel::SetTitleHeight: invalid height value!" );
        m_pImpl->nColumnHeaderHeight = _nHeight;
        // TODO: notification
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoControlTableModel::SetHandleWidth( TableMetrics _nWidth )
    {
        DBG_CHECK_ME();
        DBG_ASSERT( _nWidth > 0, "DefaultTableModel::SetHandleWidth: invalid width value!" );
        m_pImpl->nRowHeaderWidth = _nWidth;
        // TODO: notification
    }

    //------------------------------------------------------------------------------------------------------------------
    ScrollbarVisibility UnoControlTableModel::getVerticalScrollbarVisibility() const
    {
        DBG_CHECK_ME();
        return m_pImpl->eVScrollMode;
    }

    //------------------------------------------------------------------------------------------------------------------
    ScrollbarVisibility UnoControlTableModel::getHorizontalScrollbarVisibility() const
    {
        DBG_CHECK_ME();
        return m_pImpl->eHScrollMode;
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoControlTableModel::addTableModelListener( const PTableModelListener& i_listener )
    {
        DBG_CHECK_ME();
        ENSURE_OR_RETURN_VOID( !!i_listener, "illegal NULL listener" );
        m_pImpl->m_aListeners.push_back( i_listener );
    }

    //------------------------------------------------------------------------------------------------------------------
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

    //------------------------------------------------------------------------------------------------------------------
    void UnoControlTableModel::setVerticalScrollbarVisibility( ScrollbarVisibility const i_visibility ) const
    {
        DBG_CHECK_ME();
        m_pImpl->eVScrollMode = i_visibility;
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoControlTableModel::setHorizontalScrollbarVisibility( ScrollbarVisibility const i_visibility ) const
    {
        DBG_CHECK_ME();
        m_pImpl->eHScrollMode = i_visibility;
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoControlTableModel::setCellContent(const std::vector<std::vector< Any > >& cellContent)
    {
        DBG_CHECK_ME();
        m_pImpl->aCellContent = cellContent;
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoControlTableModel::getCellContent( ColPos const i_col, RowPos const i_row, Any& o_cellContent )
    {
        DBG_CHECK_ME();
        ENSURE_OR_RETURN_VOID( ( i_row >= 0 ) && ( size_t( i_row ) < m_pImpl->aCellContent.size() ),
            "UnoControlTableModel::getCellContent: illegal row index!" );
        ::std::vector< Any >& rRowContent( m_pImpl->aCellContent[ i_row ] );
        ENSURE_OR_RETURN_VOID( ( i_col >= 0 ) && ( size_t( i_col ) < rRowContent.size() ),
            "UnoControlTableModel::getCellContent: illegal column index" );
        o_cellContent = rRowContent[ i_col ];
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoControlTableModel::updateCellContent( RowPos const i_row, ColPos const i_col, Any const & i_cellContent )
    {
        DBG_CHECK_ME();
        ENSURE_OR_RETURN_VOID( ( i_row >= 0 ) && ( size_t( i_row ) < m_pImpl->aCellContent.size() ),
            "UnoControlTableModel::updateCellContent: illegal row index!" );
        ::std::vector< Any >& rRowContent( m_pImpl->aCellContent[ i_row ] );
        ENSURE_OR_RETURN_VOID( ( i_col >= 0 ) && ( size_t( i_col ) < rRowContent.size() ),
            "UnoControlTableModel::updateCellContent: illegal column index" );
        rRowContent[ i_col ] = i_cellContent;

        // TODO: listener notification
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoControlTableModel::setRowHeaderNames( const Sequence< ::rtl::OUString >& i_rowHeaders )
    {
        DBG_CHECK_ME();
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

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString UnoControlTableModel::getRowHeader( RowPos const i_rowPos ) const
    {
        DBG_CHECK_ME();
        ENSURE_OR_RETURN( ( i_rowPos >= 0 ) && ( size_t( i_rowPos ) < m_pImpl->aRowHeadersTitle.size() ),
            "UnoControlTableModel::getRowHeader: illegal row position!", ::rtl::OUString() );
        return m_pImpl->aRowHeadersTitle[ i_rowPos ];
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoControlTableModel::appendRow( Sequence< Any > const & i_rowData, ::rtl::OUString const & i_rowHeader )
    {
        DBG_CHECK_ME();
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

    //------------------------------------------------------------------------------------------------------------------
    void UnoControlTableModel::removeRow( RowPos const i_rowPos )
    {
        DBG_CHECK_ME();
        ENSURE_OR_RETURN_VOID( ( i_rowPos >= 0 ) && ( i_rowPos < getRowCount() ), "UnoControlTableModel::removeRow: illegal row position!" );

        if ( hasRowHeaders() )
            m_pImpl->aRowHeadersTitle.erase( m_pImpl->aRowHeadersTitle.begin() + i_rowPos );

        const ::std::vector< ::std::vector< Any > >::iterator contentPos = m_pImpl->aCellContent.begin() + i_rowPos;
        m_pImpl->aCellContent.erase( contentPos );

        --m_pImpl->nRowCount;

        // TODO: listener notification
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoControlTableModel::clearAllRows()
    {
        DBG_CHECK_ME();
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

    //------------------------------------------------------------------------------------------------------------------
    ::com::sun::star::util::Color UnoControlTableModel::getLineColor()
    {
        DBG_CHECK_ME();
        return  m_pImpl->m_nLineColor;
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoControlTableModel::setLineColor( ::com::sun::star::util::Color _rColor )
    {
        DBG_CHECK_ME();
         m_pImpl->m_nLineColor = _rColor;
    }

    //------------------------------------------------------------------------------------------------------------------
    ::com::sun::star::util::Color UnoControlTableModel::getHeaderBackgroundColor()
    {
        DBG_CHECK_ME();
        return  m_pImpl->m_nHeaderColor;
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoControlTableModel::setHeaderBackgroundColor( ::com::sun::star::util::Color _rColor )
    {
        DBG_CHECK_ME();
        m_pImpl->m_nHeaderColor = _rColor;
    }

    //------------------------------------------------------------------------------------------------------------------
    ::com::sun::star::util::Color UnoControlTableModel::getTextColor()
    {
        DBG_CHECK_ME();
        return  m_pImpl->m_nTextColor;
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoControlTableModel::setTextColor( ::com::sun::star::util::Color _rColor )
    {
        DBG_CHECK_ME();
         m_pImpl->m_nTextColor = _rColor;
    }

    //------------------------------------------------------------------------------------------------------------------
    ::com::sun::star::util::Color UnoControlTableModel::getOddRowBackgroundColor()
    {
        DBG_CHECK_ME();
        return  m_pImpl->m_nRowColor1;
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoControlTableModel::setOddRowBackgroundColor( ::com::sun::star::util::Color _rColor )
    {
        DBG_CHECK_ME();
        m_pImpl->m_nRowColor1 = _rColor;
    }

    //------------------------------------------------------------------------------------------------------------------
    ::com::sun::star::util::Color UnoControlTableModel::getEvenRowBackgroundColor()
    {
        DBG_CHECK_ME();
        return  m_pImpl->m_nRowColor2;
    }

    //------------------------------------------------------------------------------------------------------------------
    void UnoControlTableModel::setEvenRowBackgroundColor( ::com::sun::star::util::Color _rColor )
    {
        DBG_CHECK_ME();
        m_pImpl->m_nRowColor2 = _rColor;
    }

    //------------------------------------------------------------------------------------------------------------------
    ::com::sun::star::style::VerticalAlignment UnoControlTableModel::getVerticalAlign()
    {
        DBG_CHECK_ME();
        return  m_pImpl->m_eVerticalAlign;
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::setVerticalAlign( com::sun::star::style::VerticalAlignment _xAlign )
    {
        DBG_CHECK_ME();
        m_pImpl->m_eVerticalAlign = _xAlign;
    }

    //--------------------------------------------------------------------
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

    //--------------------------------------------------------------------
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

// .....................................................................................................................
} } // svt::table
// .....................................................................................................................
