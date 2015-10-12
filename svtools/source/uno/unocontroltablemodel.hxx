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

#ifndef INCLUDED_SVTOOLS_SOURCE_UNO_UNOCONTROLTABLEMODEL_HXX
#define INCLUDED_SVTOOLS_SOURCE_UNO_UNOCONTROLTABLEMODEL_HXX

#include <svtools/table/tablemodel.hxx>
#include "table/tablecontrol.hxx"

#include <com/sun/star/awt/grid/XGridControl.hpp>
#include <com/sun/star/awt/grid/GridDataEvent.hpp>
#include <com/sun/star/awt/grid/XGridColumnModel.hpp>
#include <com/sun/star/awt/grid/XGridDataModel.hpp>
#include <com/sun/star/awt/grid/XGridSelectionListener.hpp>
#include <com/sun/star/awt/grid/XGridColumn.hpp>
#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>

#include <rtl/ref.hxx>

#include <boost/noncopyable.hpp>


namespace svt { namespace table
{



    //= UnoControlTableModel

    class UnoGridColumnFacade;
    struct UnoControlTableModel_Impl;
    class UnoControlTableModel : public ITableModel, public ITableDataSort
    {
    private:
        UnoControlTableModel_Impl*     m_pImpl;

    public:
        UnoControlTableModel();
        virtual ~UnoControlTableModel();

    public:
        // ITableModel overridables
        virtual TableSize                       getColumnCount() const override;
        virtual TableSize                       getRowCount() const override;
        virtual bool                            hasColumnHeaders() const override;
        virtual bool                            hasRowHeaders() const override;
        virtual PColumnModel                    getColumnModel( ColPos column ) override;
        virtual PTableRenderer                  getRenderer() const override;
        virtual PTableInputHandler              getInputHandler() const override;
        virtual TableMetrics                    getRowHeight() const override;
        virtual TableMetrics                    getColumnHeaderHeight() const override;
        virtual TableMetrics                    getRowHeaderWidth() const override;
        virtual ScrollbarVisibility             getVerticalScrollbarVisibility() const override;
        virtual ScrollbarVisibility             getHorizontalScrollbarVisibility() const override;
        virtual void                            addTableModelListener( const PTableModelListener& i_listener ) override;
        virtual void                            removeTableModelListener( const PTableModelListener& i_listener ) override;
        virtual void                            getCellContent( ColPos const i_col, RowPos const i_row, ::com::sun::star::uno::Any& o_cellContent ) override;
        virtual void                            getCellToolTip( ColPos const i_col, RowPos const i_row, ::com::sun::star::uno::Any & o_cellToolTip ) override;
        virtual ::com::sun::star::uno::Any      getRowHeading( RowPos const i_rowPos ) const override;
        virtual ::boost::optional< ::Color >    getLineColor() const override;
        virtual ::boost::optional< ::Color >    getHeaderBackgroundColor() const override;
        virtual ::boost::optional< ::Color >    getHeaderTextColor() const override;
        virtual ::boost::optional< ::Color >    getActiveSelectionBackColor() const override;
        virtual ::boost::optional< ::Color >    getInactiveSelectionBackColor() const override;
        virtual ::boost::optional< ::Color >    getActiveSelectionTextColor() const override;
        virtual ::boost::optional< ::Color >    getInactiveSelectionTextColor() const override;
        virtual ::boost::optional< ::Color >    getTextColor() const override;
        virtual ::boost::optional< ::Color >    getTextLineColor() const override;
        virtual ::boost::optional< ::std::vector< ::Color > >
                                                getRowBackgroundColors() const override;
        virtual ::com::sun::star::style::VerticalAlignment
                                                getVerticalAlign() const override;
        virtual ITableDataSort*                 getSortAdapter() override;
        virtual bool                            isEnabled() const override;

        // ITableDataSort overridables
        virtual void        sortByColumn( ColPos const i_column, ColumnSortDirection const i_sortDirection ) override;
        virtual ColumnSort  getCurrentSortOrder() const override;

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
        void    setEnabled( bool _bEnabled );

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

    private:
        void    impl_notifyTableMetricsChanged() const;
    };


} } // svt::table


#endif // INCLUDED_SVTOOLS_SOURCE_UNO_UNOCONTROLTABLEMODEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
