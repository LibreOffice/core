/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <com/sun/star/view/SelectionType.hpp>
#include "svtools/table/gridtablerenderer.hxx"
#include "svtools/table/defaultinputhandler.hxx"
#include "svtools/table/tablecontrol.hxx"
#include <comphelper/sequence.hxx>
#include <rtl/ref.hxx>
#include <tools/debug.hxx>
#include <toolkit/helper/property.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/awt/grid/XGridColumn.hpp>

using ::rtl::OUString;
using namespace ::svt::table;
using namespace ::com::sun::star::uno;


    //--------------------------------------------------------------------
    UnoControlTableColumn::UnoControlTableColumn(Reference<XGridColumn> m_xGridColumn)
        :m_nID( 0 )
        ,m_sName()
        ,m_bIsResizable( true )
        ,m_nWidth( 4  )
        ,m_nMinWidth( 0 )
        ,m_nMaxWidth( 0 )
    ,m_nPrefWidth ( 0 )
    ,m_xHorizontalAlign(com::sun::star::style::HorizontalAlignment(0))
    {
    m_sName = m_xGridColumn->getTitle();
    }
    //--------------------------------------------------------------------
    UnoControlTableColumn::UnoControlTableColumn()
        :m_nID( 0 )
        ,m_sName()
        ,m_bIsResizable( true )
        ,m_nWidth( 4  )
        ,m_nMinWidth( 0 )
        ,m_nMaxWidth( 0 )
    ,m_nPrefWidth ( 0 )
    ,m_xHorizontalAlign(com::sun::star::style::HorizontalAlignment(0))
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
        return m_sName;
    }

    //--------------------------------------------------------------------
    void UnoControlTableColumn::setName( const String& _rName )
    {
        m_sName = _rName;
    }
    //--------------------------------------------------------------------
    bool UnoControlTableColumn::isResizable() const
    {
        return m_bIsResizable;
    }

    //--------------------------------------------------------------------
    void UnoControlTableColumn::setResizable( bool _bResizable )
    {
        m_bIsResizable = _bResizable;
    }

    //--------------------------------------------------------------------
    TableMetrics UnoControlTableColumn::getWidth() const
    {
        return m_nWidth;
    }

    //--------------------------------------------------------------------
    void UnoControlTableColumn::setWidth( TableMetrics _nWidth )
    {
        m_nWidth = _nWidth;
    }

    //--------------------------------------------------------------------
    TableMetrics UnoControlTableColumn::getMinWidth() const
    {
        return m_nMinWidth;
    }

    //--------------------------------------------------------------------
    void UnoControlTableColumn::setMinWidth( TableMetrics _nMinWidth )
    {
        m_nMinWidth = _nMinWidth;
    }

    //--------------------------------------------------------------------
    TableMetrics UnoControlTableColumn::getMaxWidth() const
    {
        return m_nMaxWidth;
    }

    //--------------------------------------------------------------------
    void UnoControlTableColumn::setMaxWidth( TableMetrics _nMaxWidth )
    {
        m_nMaxWidth = _nMaxWidth;
    }
    //--------------------------------------------------------------------
    TableMetrics UnoControlTableColumn::getPreferredWidth() const
    {
        return m_nPrefWidth;
    }

    //--------------------------------------------------------------------
    void UnoControlTableColumn::setPreferredWidth( TableMetrics _nPrefWidth )
    {
        m_nPrefWidth = _nPrefWidth;
    }
    //--------------------------------------------------------------------
    ::com::sun::star::style::HorizontalAlignment UnoControlTableColumn::getHorizontalAlign()
    {
        return m_xHorizontalAlign;
    }

    //--------------------------------------------------------------------
    void UnoControlTableColumn::setHorizontalAlign( com::sun::star::style::HorizontalAlignment _align )
    {
        m_xHorizontalAlign = _align;
    }

    //====================================================================
    //= DefaultTableModel_Impl
    //====================================================================
    struct UnoControlTableModel_Impl
    {
        ::std::vector< PColumnModel >  aColumns;
        TableSize                       nRowCount;
        bool                            bHasColumnHeaders;
        bool                            bHasRowHeaders;
    bool                            bVScroll;
    bool                            bHScroll;
        PTableRenderer                  pRenderer;
        PTableInputHandler              pInputHandler;
        TableMetrics                    nRowHeight;
        TableMetrics                    nColumnHeaderHeight;
        TableMetrics                    nRowHeaderWidth;
    std::vector<rtl::OUString>      aRowHeadersTitle;
    std::vector<std::vector< Any > >    aCellContent;
    ::com::sun::star::util::Color m_xLineColor;
    ::com::sun::star::util::Color m_xHeaderColor;
    ::com::sun::star::util::Color m_xTextColor;
    ::com::sun::star::util::Color m_xRowColor1;
    ::com::sun::star::util::Color m_xRowColor2;
    ::com::sun::star::style::VerticalAlignment m_xVerticalAlign;

        UnoControlTableModel_Impl()
            :aColumns       ( )
        ,nRowCount          ( 0         )
            ,bHasColumnHeaders  ( false     )
            ,bHasRowHeaders     ( false     )
        ,bVScroll       ( false     )
            ,bHScroll       ( false     )
            ,pRenderer          (           )
            ,pInputHandler      (           )
            ,nRowHeight         ( 0 )
            ,nColumnHeaderHeight( 0 )
            ,nRowHeaderWidth    ( 10 )
        ,aRowHeadersTitle   ( )
        ,aCellContent   ( )
        ,m_xLineColor   ( 0xFFFFFF )
        ,m_xHeaderColor ( 0xFFFFFF )
        ,m_xTextColor   ( 0 )//black as default
        ,m_xRowColor1   ( 0xFFFFFF )
        ,m_xRowColor2   ( 0xFFFFFF )
        ,m_xVerticalAlign   (com::sun::star::style::VerticalAlignment(0))
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
        m_pImpl->bHasColumnHeaders = false;
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
    void UnoControlTableModel::setRowCount(TableSize _nRowCount)
    {
       m_pImpl->nRowCount = _nRowCount;
    }
    //--------------------------------------------------------------------
    bool UnoControlTableModel::isCellEditable( ColPos col, RowPos row ) const
    {
        (void)col;
        (void)row;
        return false;
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::addTableModelListener( const PTableModelListener& listener )
    {
        (void) listener;
        // TODO
        OSL_FAIL( "DefaultTableModel::addTableModelListener: not yet implemented!" );
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::removeTableModelListener( const PTableModelListener& listener )
    {
        (void)listener;
        // TODO
        OSL_FAIL( "DefaultTableModel::removeTableModelListener: not yet implemented!" );
    }

    //--------------------------------------------------------------------
    PColumnModel UnoControlTableModel::getColumnModel( ColPos column )
    {
        DBG_ASSERT( ( column >= 0 ) && ( column < getColumnCount() ),
            "DefaultTableModel::getColumnModel: invalid index!" );
        return m_pImpl->aColumns[ column ];
    }

    //--------------------------------------------------------------------
    std::vector<PColumnModel>& UnoControlTableModel::getColumnModel()
    {
        return m_pImpl->aColumns;
    }
    //--------------------------------------------------------------------
    PColumnModel UnoControlTableModel::getColumnModelByID( ColumnID id )
    {
        (void)id;
        // TODO
        OSL_FAIL( "DefaultTableModel::getColumnModelByID: not yet implemented!" );
        return PColumnModel();
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
    ScrollbarVisibility UnoControlTableModel::getVerticalScrollbarVisibility(int overAllHeight, int actHeight) const
    {
        if(overAllHeight>=actHeight && !m_pImpl->bVScroll)
            return ScrollbarShowNever;
        else
            return ScrollbarShowAlways;
    }

    //--------------------------------------------------------------------
    ScrollbarVisibility UnoControlTableModel::getHorizontalScrollbarVisibility(int overAllWidth, int actWidth) const
    {
        if(overAllWidth>=actWidth && !m_pImpl->bHScroll)
            return ScrollbarShowNever;
        else
            return ScrollbarShowAlways;
    }
    //--------------------------------------------------------------------
    void UnoControlTableModel::setVerticalScrollbarVisibility(bool _bVScroll) const
    {
        m_pImpl->bVScroll = _bVScroll;
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::setHorizontalScrollbarVisibility(bool _bHScroll) const
    {
        m_pImpl->bHScroll = _bHScroll;
    }
    //--------------------------------------------------------------------
    bool UnoControlTableModel::hasVerticalScrollbar()
    {
        return m_pImpl->bVScroll;
    }
    //--------------------------------------------------------------------
    bool UnoControlTableModel::hasHorizontalScrollbar()
    {
        return m_pImpl->bHScroll;
    }
    //--------------------------------------------------------------------
    void UnoControlTableModel::setCellContent(const std::vector<std::vector< Any > >& cellContent)
    {
        m_pImpl->aCellContent = cellContent;
    }

    std::vector<std::vector< Any > >& UnoControlTableModel::getCellContent()
    {
        return m_pImpl->aCellContent;
    }
    //--------------------------------------------------------------------
    void UnoControlTableModel::setRowHeaderName(const std::vector<rtl::OUString>& cellColumnContent)
    {
        m_pImpl->aRowHeadersTitle = cellColumnContent;
    }

    std::vector<rtl::OUString>& UnoControlTableModel::getRowHeaderName()
    {
        return m_pImpl->aRowHeadersTitle;
    }
    //--------------------------------------------------------------------
    ::com::sun::star::util::Color UnoControlTableModel::getLineColor()
    {
        return  m_pImpl->m_xLineColor;
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::setLineColor( ::com::sun::star::util::Color _rColor )
    {
         m_pImpl->m_xLineColor = _rColor;
    }
    //--------------------------------------------------------------------
    ::com::sun::star::util::Color UnoControlTableModel::getHeaderBackgroundColor()
    {
        return  m_pImpl->m_xHeaderColor;
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::setHeaderBackgroundColor( ::com::sun::star::util::Color _rColor )
    {
        m_pImpl->m_xHeaderColor = _rColor;
    }
    //--------------------------------------------------------------------
    ::com::sun::star::util::Color UnoControlTableModel::getTextColor()
    {
        return  m_pImpl->m_xTextColor;
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::setTextColor( ::com::sun::star::util::Color _rColor )
    {
         m_pImpl->m_xTextColor = _rColor;
    }
    //--------------------------------------------------------------------
    ::com::sun::star::util::Color UnoControlTableModel::getOddRowBackgroundColor()
    {
        return  m_pImpl->m_xRowColor1;
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::setOddRowBackgroundColor( ::com::sun::star::util::Color _rColor )
    {
        m_pImpl->m_xRowColor1 = _rColor;
    }
    //--------------------------------------------------------------------
    ::com::sun::star::util::Color UnoControlTableModel::getEvenRowBackgroundColor()
    {
        return  m_pImpl->m_xRowColor2;
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::setEvenRowBackgroundColor( ::com::sun::star::util::Color _rColor )
    {
        m_pImpl->m_xRowColor2 = _rColor;
    }
    //--------------------------------------------------------------------
    ::com::sun::star::style::VerticalAlignment UnoControlTableModel::getVerticalAlign()
    {
        return  m_pImpl->m_xVerticalAlign;
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::setVerticalAlign( com::sun::star::style::VerticalAlignment _xAlign )
    {
         m_pImpl->m_xVerticalAlign = _xAlign;
    }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
