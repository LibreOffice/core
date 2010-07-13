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

#ifndef _UNOCONTROL_TABLEMODEL_HXX_
#define _UNOCONTROL_TABLEMODEL_HXX_

#include <svtools/table/tablemodel.hxx>
#include <svtools/table/tablecontrol.hxx>
#include <com/sun/star/awt/grid/XGridControl.hpp>
#include <com/sun/star/awt/grid/XGridDataListener.hpp>
#include <com/sun/star/awt/grid/GridDataEvent.hpp>
#include <com/sun/star/awt/grid/XGridColumnModel.hpp>
#include <com/sun/star/awt/grid/XGridDataModel.hpp>
#include <com/sun/star/awt/grid/XGridSelectionListener.hpp>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/awt/vclxwindows.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/awt/grid/XGridColumn.hpp>
#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/style/HorizontalAlignment.hpp>


using namespace ::svt::table;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt::grid;

class UnoControlTableColumn : public IColumnModel
{
    private:
        ColumnID        m_nID;
        String          m_sName;
        bool            m_bIsResizable;
        TableMetrics    m_nWidth;
        TableMetrics    m_nMinWidth;
        TableMetrics    m_nMaxWidth;
        TableMetrics    m_nPrefWidth;
        ::com::sun::star::style::HorizontalAlignment m_xHorizontalAlign;

    public:
        UnoControlTableColumn(Reference<XGridColumn>);
    UnoControlTableColumn();

        // IColumnModel overridables
        virtual ColumnID        getID() const;
        virtual bool            setID( const ColumnID _nID );
        virtual String          getName() const;
        virtual void            setName( const String& _rName );
        virtual bool            isResizable() const;
        virtual void            setResizable( bool _bResizable );
        virtual TableMetrics    getWidth() const;
        virtual void            setWidth( TableMetrics _nWidth );
        virtual TableMetrics    getMinWidth() const;
        virtual void            setMinWidth( TableMetrics _nMinWidth );
        virtual TableMetrics    getMaxWidth() const;
        virtual void            setMaxWidth( TableMetrics _nMaxWidth );
    virtual TableMetrics    getPreferredWidth() const;
        virtual void            setPreferredWidth( TableMetrics _nPrefWidth );
    virtual ::com::sun::star::style::HorizontalAlignment getHorizontalAlign();
    virtual void setHorizontalAlign(::com::sun::star::style::HorizontalAlignment _xAlign);
};

struct UnoControlTableModel_Impl;

class UnoControlTableModel : public ITableModel
{
    private:
        UnoControlTableModel_Impl*     m_pImpl;

    public:
        UnoControlTableModel();
        ~UnoControlTableModel();

        /// returns the current row height, in 1/100 millimeters
        inline  TableMetrics    GetRowHeight() const { return getRowHeight(); }
        /// sets a new row height.
        void                    setRowHeight( TableMetrics _nHeight );
         /// sets a new row header width.
        void                    setRowHeaderWidth( TableMetrics _nWidth );
         /// sets a new column header height.
        void                    setColumnHeaderHeight( TableMetrics _nHeight );

        /// returns the height of the title row (containing the column headers)
        inline  TableMetrics    GetTitleHeight() const { return getColumnHeaderHeight(); }
        /// sets a new height for the title row (containing the column headers)
        void                    SetTitleHeight( TableMetrics _nHeight );

        /// returns the width of the handle column (containing the row headers)
        inline  TableMetrics    GetHandleWidth() const { return getRowHeaderWidth(); }
        /// sets a new width for the handle column (containing the row headers)
        void                    SetHandleWidth( TableMetrics _nWidth );

        /// sets the width of a column
        inline void            SetColumnWidth( ColPos _nColumn, TableMetrics _nWidth100thMM );
        /// retrieves the width of a column, in 1/100th millimeters
        inline TableMetrics    GetColumnWidth( ColPos _nColumn );

    public:
        // ITableModel overridables
        virtual TableSize           getColumnCount() const;
        virtual TableSize           getRowCount() const;
    virtual void                setColumnCount(TableSize _nColCount);
        virtual void                setRowCount(TableSize _nRowCount);
        virtual bool                hasColumnHeaders() const;
        virtual bool                hasRowHeaders() const;
    virtual void                setRowHeaders(bool _bRowHeaders);
    virtual void                setColumnHeaders(bool _bColumnHeaders);
        virtual bool                isCellEditable( ColPos col, RowPos row ) const;
        virtual void                addTableModelListener( const PTableModelListener& listener );
        virtual void                removeTableModelListener( const PTableModelListener& listener );
        virtual PColumnModel        getColumnModel( ColPos column );
    virtual std::vector<PColumnModel>& getColumnModel();
        virtual PColumnModel        getColumnModelByID( ColumnID id );
        virtual PTableRenderer      getRenderer() const;
        virtual PTableInputHandler  getInputHandler() const;
        virtual TableMetrics        getRowHeight() const;
        virtual TableMetrics        getColumnHeaderHeight() const;
        virtual TableMetrics        getRowHeaderWidth() const;
        virtual ScrollbarVisibility getVerticalScrollbarVisibility(int overAllHeight, int actHeight) const;
        virtual ScrollbarVisibility getHorizontalScrollbarVisibility(int overAllWidth, int actWidth) const;
    virtual void                setVerticalScrollbarVisibility(bool _bVScroll) const;
        virtual void                setHorizontalScrollbarVisibility(bool _bHScroll) const;
    virtual void                setCellContent(const std::vector<std::vector< Any > >& cellContent);
    virtual std::vector<std::vector< Any > >&   getCellContent();
    virtual void                setRowHeaderName(const std::vector<rtl::OUString>& cellColumnContent);
    virtual std::vector<rtl::OUString>& getRowHeaderName();
    virtual ::com::sun::star::util::Color getLineColor();
    virtual void                setLineColor(::com::sun::star::util::Color _rColor);
    virtual ::com::sun::star::util::Color getHeaderBackgroundColor();
    virtual void                setHeaderBackgroundColor(::com::sun::star::util::Color _rColor);
    virtual ::com::sun::star::util::Color getTextColor();
    virtual void                setTextColor(::com::sun::star::util::Color _rColor);
    virtual ::com::sun::star::util::Color getOddRowBackgroundColor();
    virtual void                setOddRowBackgroundColor(::com::sun::star::util::Color _rColor);
    virtual ::com::sun::star::util::Color getEvenRowBackgroundColor();
    virtual void                setEvenRowBackgroundColor(::com::sun::star::util::Color _rColor);
    virtual ::com::sun::star::style::VerticalAlignment getVerticalAlign();
    virtual void                setVerticalAlign(::com::sun::star::style::VerticalAlignment _rAlign);
    virtual bool                hasVerticalScrollbar();
    virtual bool                hasHorizontalScrollbar();
};

inline void UnoControlTableModel::SetColumnWidth( ColPos _nColumn, TableMetrics _nWidth100thMM )
{
        getColumnModel( _nColumn )->setWidth( _nWidth100thMM );
}

inline TableMetrics UnoControlTableModel::GetColumnWidth( ColPos _nColumn )
{
    return getColumnModel( _nColumn )->getWidth();
}
 #endif // _UNOCONTROL_TABLEMODEL_HXX_
