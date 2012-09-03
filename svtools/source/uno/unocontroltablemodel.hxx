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

#ifndef _UNOCONTROL_TABLEMODEL_HXX_
#define _UNOCONTROL_TABLEMODEL_HXX_

#include "svtools/table/tablemodel.hxx"
#include "svtools/table/tablecontrol.hxx"

#include <com/sun/star/awt/grid/XGridControl.hpp>
#include <com/sun/star/awt/grid/GridDataEvent.hpp>
#include <com/sun/star/awt/grid/XGridColumnModel.hpp>
#include <com/sun/star/awt/grid/XGridDataModel.hpp>
#include <com/sun/star/awt/grid/XGridSelectionListener.hpp>
#include <com/sun/star/awt/grid/XGridColumn.hpp>
#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>

#include <cppuhelper/implbase2.hxx>
#include <rtl/ref.hxx>

#include <boost/noncopyable.hpp>

// .....................................................................................................................
namespace svt { namespace table
{
// .....................................................................................................................

    //==================================================================================================================
    //= UnoControlTableModel
    //==================================================================================================================
    class UnoGridColumnFacade;
    struct UnoControlTableModel_Impl;
    class UnoControlTableModel : public ITableModel, public ITableDataSort
    {
    private:
        UnoControlTableModel_Impl*     m_pImpl;

    public:
        UnoControlTableModel();
        ~UnoControlTableModel();

    public:
        // ITableModel overridables
        virtual TableSize                       getColumnCount() const;
        virtual TableSize                       getRowCount() const;
        virtual bool                            hasColumnHeaders() const;
        virtual bool                            hasRowHeaders() const;
        virtual bool                            isCellEditable( ColPos col, RowPos row ) const;
        virtual PColumnModel                    getColumnModel( ColPos column );
        virtual PTableRenderer                  getRenderer() const;
        virtual PTableInputHandler              getInputHandler() const;
        virtual TableMetrics                    getRowHeight() const;
        virtual TableMetrics                    getColumnHeaderHeight() const;
        virtual TableMetrics                    getRowHeaderWidth() const;
        virtual ScrollbarVisibility             getVerticalScrollbarVisibility() const;
        virtual ScrollbarVisibility             getHorizontalScrollbarVisibility() const;
        virtual void                            addTableModelListener( const PTableModelListener& i_listener );
        virtual void                            removeTableModelListener( const PTableModelListener& i_listener );
        virtual void                            getCellContent( ColPos const i_col, RowPos const i_row, ::com::sun::star::uno::Any& o_cellContent );
        virtual void                            getCellToolTip( ColPos const i_col, RowPos const i_row, ::com::sun::star::uno::Any & o_cellToolTip );
        virtual ::com::sun::star::uno::Any      getRowHeading( RowPos const i_rowPos ) const;
        virtual ::boost::optional< ::Color >    getLineColor() const;
        virtual ::boost::optional< ::Color >    getHeaderBackgroundColor() const;
        virtual ::boost::optional< ::Color >    getHeaderTextColor() const;
        virtual ::boost::optional< ::Color >    getActiveSelectionBackColor() const;
        virtual ::boost::optional< ::Color >    getInactiveSelectionBackColor() const;
        virtual ::boost::optional< ::Color >    getActiveSelectionTextColor() const;
        virtual ::boost::optional< ::Color >    getInactiveSelectionTextColor() const;
        virtual ::boost::optional< ::Color >    getTextColor() const;
        virtual ::boost::optional< ::Color >    getTextLineColor() const;
        virtual ::boost::optional< ::std::vector< ::Color > >
                                                getRowBackgroundColors() const;
        virtual ::com::sun::star::style::VerticalAlignment
                                                getVerticalAlign() const;
        virtual ITableDataSort*                 getSortAdapter();

        // ITableDataSort overridables
        virtual void        sortByColumn( ColPos const i_column, ColumnSortDirection const i_sortDirection );
        virtual ColumnSort  getCurrentSortOrder() const;

        // column write access
        void    appendColumn( ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumn > const & i_column );
        void    insertColumn( ColPos const i_position, ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumn > const & i_column );
        void    removeColumn( ColPos const i_position );
        void    removeAllColumns();

        // other operations
        void    setVerticalScrollbarVisibility( ScrollbarVisibility const i_visibility ) const;
        void    setHorizontalScrollbarVisibility( ScrollbarVisibility const i_visibility ) const;

        void    setDataModel( ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridDataModel > const & i_gridDataModel );
        bool    hasDataModel() const;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridDataModel >
                getDataModel() const;
        void    setColumnModel( ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumnModel > const & i_gridColumnModel );
        bool    hasColumnModel() const;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumnModel >
                getColumnModel() const;

        void    setRowHeaders(bool _bRowHeaders);
        void    setColumnHeaders(bool _bColumnHeaders);

        void    setRowHeight( TableMetrics _nHeight );
        void    setRowHeaderWidth( TableMetrics _nWidth );
        void    setColumnHeaderHeight( TableMetrics _nHeight );

        void    setLineColor( ::com::sun::star::uno::Any const & i_color );
        void    setHeaderBackgroundColor( ::com::sun::star::uno::Any const & i_color );
        void    setHeaderTextColor( ::com::sun::star::uno::Any const & i_color );
        void    setActiveSelectionBackColor( ::com::sun::star::uno::Any const & i_color );
        void    setInactiveSelectionBackColor( ::com::sun::star::uno::Any const & i_color );
        void    setActiveSelectionTextColor( ::com::sun::star::uno::Any const & i_color );
        void    setInactiveSelectionTextColor( ::com::sun::star::uno::Any const & i_color );
        void    setTextColor( ::com::sun::star::uno::Any const & i_color );
        void    setTextLineColor( ::com::sun::star::uno::Any const & i_color );
        void    setRowBackgroundColors( ::com::sun::star::uno::Any const & i_APIValue );

        void    setVerticalAlign(::com::sun::star::style::VerticalAlignment _rAlign);

        // multiplexing of XGridDataListener events
        void    notifyRowsInserted( ::com::sun::star::awt::grid::GridDataEvent const & i_event ) const;
        void    notifyRowsRemoved( ::com::sun::star::awt::grid::GridDataEvent const & i_event ) const;
        void    notifyDataChanged( ::com::sun::star::awt::grid::GridDataEvent const & i_event ) const;

        /// retrieves the index of a column within the model
        ColPos getColumnPos( UnoGridColumnFacade const & i_column ) const;

        /// notifies a change in a column belonging to the model
        void    notifyColumnChange( ColPos const i_columnPos, ColumnAttributeGroup const i_attributeGroup ) const;

        /** notifies a change in all data represented by the model. To be used if you cannot specified the changed data
            in more detail.
        */
        void    notifyAllDataChanged() const;

#ifdef DBG_UTIL
        const char* checkInvariants() const;
#endif

    private:
        void    impl_notifyTableMetricsChanged() const;
    };

// .....................................................................................................................
} } // svt::table
// .....................................................................................................................

#endif // _UNOCONTROL_TABLEMODEL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
