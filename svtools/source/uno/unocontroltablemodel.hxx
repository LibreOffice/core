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

class UnoControlTableColumn : public IColumnModel
{
    private:
        ColumnID                                        m_nID;

        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumn >
                                                        m_xGridColumn;

    public:
        UnoControlTableColumn( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumn >& i_gridColumn );

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

    private:
        UnoControlTableColumn();    // not implemented
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
        virtual TableSize   getColumnCount() const;
        virtual TableSize   getRowCount() const;
        virtual bool        hasColumnHeaders() const;
        virtual bool        hasRowHeaders() const;
        virtual bool        isCellEditable( ColPos col, RowPos row ) const;
        virtual PColumnModel    getColumnModel( ColPos column );
        virtual PTableRenderer  getRenderer() const;
        virtual PTableInputHandler  getInputHandler() const;
        virtual TableMetrics    getRowHeight() const;
        virtual TableMetrics    getColumnHeaderHeight() const;
        virtual TableMetrics    getRowHeaderWidth() const;
        virtual ScrollbarVisibility getVerticalScrollbarVisibility(int overAllHeight,int actHeight) const;
        virtual ScrollbarVisibility getHorizontalScrollbarVisibility(int overAllWidth, int actWidth) const;
        virtual void addTableModelListener( const PTableModelListener& i_listener );
        virtual void removeTableModelListener( const PTableModelListener& i_listener );
        virtual bool hasVerticalScrollbar();
        virtual bool hasHorizontalScrollbar();
        virtual std::vector< std::vector< ::com::sun::star::uno::Any > >&   getCellContent();
        virtual std::vector<rtl::OUString>&   getRowHeaderName();
        virtual ::com::sun::star::util::Color getLineColor();
        virtual ::com::sun::star::util::Color getHeaderBackgroundColor();
        virtual ::com::sun::star::util::Color getTextColor();
        virtual ::com::sun::star::util::Color getOddRowBackgroundColor();
        virtual ::com::sun::star::util::Color getEvenRowBackgroundColor();
        virtual ::com::sun::star::style::VerticalAlignment getVerticalAlign();


        // other operations
        void    setRowCount(TableSize _nRowCount);
        void    setRowHeaderName(const std::vector<rtl::OUString>& cellColumnContent);
        void    setVerticalScrollbarVisibility(bool _bVScroll) const;
        void    setHorizontalScrollbarVisibility(bool _bHScroll) const;
        void    setCellContent(const std::vector<std::vector< ::com::sun::star::uno::Any > >& cellContent);
        void    setColumnCount(TableSize _nColCount);
        void    setRowHeaders(bool _bRowHeaders);
        void    setColumnHeaders(bool _bColumnHeaders);
        void    setLineColor(::com::sun::star::util::Color _rColor);
        void    setHeaderBackgroundColor(::com::sun::star::util::Color _rColor);
        void    setTextColor(::com::sun::star::util::Color _rColor);
        void    setOddRowBackgroundColor(::com::sun::star::util::Color _rColor);
        void    setEvenRowBackgroundColor(::com::sun::star::util::Color _rColor);
        void    setVerticalAlign(::com::sun::star::style::VerticalAlignment _rAlign);
        void    appendColumn( const PColumnModel& i_column );
        void    insertColumn( ColPos const i_position, const PColumnModel& i_column );
        void    removeAllColumns();
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
