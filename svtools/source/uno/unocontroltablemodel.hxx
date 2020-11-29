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

#pragma once

#include <table/tablemodel.hxx>
#include <table/tablesort.hxx>
#include <tools/color.hxx>

#include <com/sun/star/awt/grid/GridDataEvent.hpp>
#include <com/sun/star/awt/grid/XGridColumnModel.hpp>
#include <com/sun/star/awt/grid/XGridDataModel.hpp>
#include <com/sun/star/awt/grid/XGridColumn.hpp>
#include <com/sun/star/awt/grid/XGridColumn.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <cppuhelper/weakref.hxx>


namespace svt::table
{


    //= UnoControlTableModel

    class UnoGridColumnFacade;
    class UnoControlTableModel : public ITableModel, public ITableDataSort
    {
    public:
        UnoControlTableModel();
        virtual ~UnoControlTableModel() override;

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
        virtual void                            getCellContent( ColPos const i_col, RowPos const i_row, css::uno::Any& o_cellContent ) override;
        virtual void                            getCellToolTip( ColPos const i_col, RowPos const i_row, css::uno::Any & o_cellToolTip ) override;
        virtual css::uno::Any      getRowHeading( RowPos const i_rowPos ) const override;
        virtual ::std::optional< ::Color >    getLineColor() const override;
        virtual ::std::optional< ::Color >    getHeaderBackgroundColor() const override;
        virtual ::std::optional< ::Color >    getHeaderTextColor() const override;
        virtual ::std::optional< ::Color >    getActiveSelectionBackColor() const override;
        virtual ::std::optional< ::Color >    getInactiveSelectionBackColor() const override;
        virtual ::std::optional< ::Color >    getActiveSelectionTextColor() const override;
        virtual ::std::optional< ::Color >    getInactiveSelectionTextColor() const override;
        virtual ::std::optional< ::Color >    getTextColor() const override;
        virtual ::std::optional< ::Color >    getTextLineColor() const override;
        virtual ::std::optional< ::std::vector< ::Color > >
                                                getRowBackgroundColors() const override;
        virtual css::style::VerticalAlignment
                                                getVerticalAlign() const override;
        virtual ITableDataSort*                 getSortAdapter() override;
        virtual bool                            isEnabled() const override;

        // ITableDataSort overridables
        virtual void        sortByColumn( ColPos const i_column, ColumnSortDirection const i_sortDirection ) override;
        virtual ColumnSort  getCurrentSortOrder() const override;

        // column write access
        void    appendColumn( css::uno::Reference< css::awt::grid::XGridColumn > const & i_column );
        void    insertColumn( ColPos const i_position, css::uno::Reference< css::awt::grid::XGridColumn > const & i_column );
        void    removeColumn( ColPos const i_position );
        void    removeAllColumns();

        // other operations
        void    setVerticalScrollbarVisibility( ScrollbarVisibility const i_visibility );
        void    setHorizontalScrollbarVisibility( ScrollbarVisibility const i_visibility );

        void    setDataModel( css::uno::Reference< css::awt::grid::XGridDataModel > const & i_gridDataModel );
        bool    hasDataModel() const;
        css::uno::Reference< css::awt::grid::XGridDataModel >
                getDataModel() const;
        void    setColumnModel( css::uno::Reference< css::awt::grid::XGridColumnModel > const & i_gridColumnModel );
        bool    hasColumnModel() const;
        css::uno::Reference< css::awt::grid::XGridColumnModel >
                getColumnModel() const;

        void    setRowHeaders(bool _bRowHeaders);
        void    setColumnHeaders(bool _bColumnHeaders);

        void    setRowHeight( TableMetrics _nHeight );
        void    setRowHeaderWidth( TableMetrics _nWidth );
        void    setColumnHeaderHeight( TableMetrics _nHeight );

        void    setLineColor( css::uno::Any const & i_color );
        void    setHeaderBackgroundColor( css::uno::Any const & i_color );
        void    setHeaderTextColor( css::uno::Any const & i_color );
        void    setActiveSelectionBackColor( css::uno::Any const & i_color );
        void    setInactiveSelectionBackColor( css::uno::Any const & i_color );
        void    setActiveSelectionTextColor( css::uno::Any const & i_color );
        void    setInactiveSelectionTextColor( css::uno::Any const & i_color );
        void    setTextColor( css::uno::Any const & i_color );
        void    setTextLineColor( css::uno::Any const & i_color );
        void    setRowBackgroundColors( css::uno::Any const & i_APIValue );

        void    setVerticalAlign(css::style::VerticalAlignment _rAlign);
        void    setEnabled( bool _bEnabled );

        // multiplexing of XGridDataListener events
        void    notifyRowsInserted( css::awt::grid::GridDataEvent const & i_event ) const;
        void    notifyRowsRemoved( css::awt::grid::GridDataEvent const & i_event ) const;
        void    notifyDataChanged( css::awt::grid::GridDataEvent const & i_event ) const;

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

        typedef ::std::vector< PTableModelListener >    ModellListeners;
        typedef ::std::vector< PColumnModel >           ColumnModels;

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
        ::std::optional< ::Color >                    m_aGridLineColor;
        ::std::optional< ::Color >                    m_aHeaderBackgroundColor;
        ::std::optional< ::Color >                    m_aHeaderTextColor;
        ::std::optional< ::Color >                    m_aActiveSelectionBackColor;
        ::std::optional< ::Color >                    m_aInactiveSelectionBackColor;
        ::std::optional< ::Color >                    m_aActiveSelectionTextColor;
        ::std::optional< ::Color >                    m_aInactiveSelectionTextColor;
        ::std::optional< ::Color >                    m_aTextColor;
        ::std::optional< ::Color >                    m_aTextLineColor;
        ::std::optional< ::std::vector< ::Color > >   m_aRowColors;
        css::style::VerticalAlignment                   m_eVerticalAlign;
        bool                                            bEnabled;
        ModellListeners                                 m_aListeners;
        css::uno::WeakReference< css::awt::grid::XGridDataModel >                 m_aDataModel;
        css::uno::WeakReference< css::awt::grid::XGridColumnModel >               m_aColumnModel;
    };


} // svt::table



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
