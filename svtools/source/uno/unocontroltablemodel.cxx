/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unocontroltablemodel.cxx,v $
 * $Revision: 1.32 $
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
using namespace ::com::sun::star::awt::grid;


    //--------------------------------------------------------------------
    UnoControlTableColumn::UnoControlTableColumn(Reference<XGridColumn> m_xGridColumn)
        :m_nID( 0 )
        ,m_sName()
        ,m_bIsResizable( false )
        ,m_nWidth( 10 * 100 )    // 1 cm
        ,m_nMinWidth( 0 )   // no min width
        ,m_nMaxWidth( 0 )   // no max width
    {
        //m_nID = m_xGridColumn->getIdentifier();
        //m_nWidth = m_xGridColumn->getColumnWidth();
        m_sName = m_xGridColumn->getTitle();
    }

    //--------------------------------------------------------------------
    ColumnID UnoControlTableColumn::getID() const
    {
        return m_nID;
    }

    //--------------------------------------------------------------------
    bool UnoControlTableColumn::setID( const ColumnID _nID )
    {
        // TODO: conflict check

        m_nID = _nID;
        // TODO: notifications?

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
        // TODO: notifications?
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
        // TODO: notifications?
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
        // TODO: notifications?
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
        // TODO: notifications?
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
        // TODO: notifications?
    }

    //====================================================================
    //= DefaultTableModel_Impl
    //====================================================================
    struct UnoControlTableModel_Impl
    {
        ::std::vector< PColumnModel >&  aColumns;
        TableSize                       nRowCount;
        bool                            bHasColumnHeaders;
        bool                            bHasRowHeaders;
        PTableRenderer                  pRenderer;
        PTableInputHandler              pInputHandler;
        TableMetrics                    nRowHeight;
        TableMetrics                    nColumnHeaderHeight;
        TableMetrics                    nRowHeaderWidth;
        std::vector<rtl::OUString>&     aRowHeadersTitle;
        std::vector<std::vector<rtl::OUString> >&   aCellContent;

        UnoControlTableModel_Impl()
            :aColumns           ( *(new std::vector< PColumnModel> (0)))
            ,nRowCount          ( 0         )
            ,bHasColumnHeaders  ( false     )
            ,bHasRowHeaders     ( false     )
            ,pRenderer          (           )
            ,pInputHandler      (           )
            ,nRowHeight         ( 4 * 100   )   // 40 mm
            ,nColumnHeaderHeight( 5 * 100   )   // 50 mm
            ,nRowHeaderWidth    ( 10 * 100  )    // 50 mm
            ,aRowHeadersTitle   ( *(new std::vector<rtl::OUString>(0)))
            ,aCellContent       ( *(new std::vector<std::vector<OUString> >(0)))
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
        //m_pImpl->aColumns.resize( m_xColumnModel->getColumnCount());
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
        //listener->onTableModelChanged(PTableModel(this));
        // TODO
        DBG_ERROR( "DefaultTableModel::addTableModelListener: not yet implemented!" );
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::removeTableModelListener( const PTableModelListener& listener )
    {
        (void)listener;
        // TODO
        DBG_ERROR( "DefaultTableModel::removeTableModelListener: not yet implemented!" );
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
        DBG_ERROR( "DefaultTableModel::getColumnModelByID: not yet implemented!" );
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
        if(overAllHeight>=actHeight)// && !m_bVScroll)
            return ScrollbarShowNever;
        else
            return ScrollbarShowAlways;
    }

    //--------------------------------------------------------------------
    ScrollbarVisibility UnoControlTableModel::getHorizontalScrollbarVisibility(int overAllWidth, int actWidth) const
    {
        if(overAllWidth>=actWidth)// && !m_bHScroll)
            return ScrollbarShowNever;
        else
            return ScrollbarShowAlways;
    }
    //--------------------------------------------------------------------
    void UnoControlTableModel::setCellContent(std::vector<std::vector<rtl::OUString> > cellContent)
    {
        //if(cellContent.empty())
        //{
        //  unsigned int i = m_pImpl->aColumns.size();
        //  std::vector<rtl::OUString>& emptyCells;
        //  while(i!=0)
        //  {
        //      cellContent.push_back(emptyCells);
        //      --i;
        //  }
        //}
        //std::vector<rtl::OUString> cCC;
        //for(::std::vector<std::vector<rtl::OUString> >::iterator iter = cellContent.begin(); iter!= cellContent.end();++iter)
        //{
        //  cCC = *iter;
        //  m_pImpl->aCellContent.push_back(cCC);
        //}
        m_pImpl->aCellContent.swap( cellContent );
    }

    std::vector<std::vector<rtl::OUString> >& UnoControlTableModel::getCellContent()
    {
        return m_pImpl->aCellContent;
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::setRowHeaderName(std::vector<rtl::OUString> cellColumnContent)
    {
        if(cellColumnContent.empty())
        {
            unsigned int i = m_pImpl->aColumns.size();
            while(i!=0)
            {
                cellColumnContent.push_back(rtl::OUString::createFromAscii(""));
                --i;
            }
        }
        for(::std::vector<rtl::OUString>::iterator iter = cellColumnContent.begin(); iter!= cellColumnContent.end();++iter)
        {
            rtl::OUString s = *iter;
            m_pImpl->aRowHeadersTitle.push_back(*iter);
        }
    }

    std::vector<rtl::OUString>& UnoControlTableModel::getRowHeaderName()
    {
        return m_pImpl->aRowHeadersTitle;
    }

