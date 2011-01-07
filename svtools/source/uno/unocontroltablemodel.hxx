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

/** === begin UNO includes === **/
#include <com/sun/star/awt/grid/XGridControl.hpp>
#include <com/sun/star/awt/grid/XGridDataListener.hpp>
#include <com/sun/star/awt/grid/GridDataEvent.hpp>
#include <com/sun/star/awt/grid/XGridColumnModel.hpp>
#include <com/sun/star/awt/grid/XGridDataModel.hpp>
#include <com/sun/star/awt/grid/XGridSelectionListener.hpp>
#include <com/sun/star/awt/grid/XGridColumn.hpp>
#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
/** === end UNO includes === **/

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
        virtual ScrollbarVisibility getVerticalScrollbarVisibility() const;
        virtual ScrollbarVisibility getHorizontalScrollbarVisibility() const;
        virtual void addTableModelListener( const PTableModelListener& i_listener );
        virtual void removeTableModelListener( const PTableModelListener& i_listener );
        virtual void getCellContent( ColPos const i_col, RowPos const i_row, ::com::sun::star::uno::Any& o_cellContent );
        virtual ::rtl::OUString getRowHeader( RowPos const i_rowPos ) const;
        virtual ::com::sun::star::util::Color getLineColor();
        virtual ::com::sun::star::util::Color getHeaderBackgroundColor();
        virtual ::com::sun::star::util::Color getTextColor();
        virtual ::com::sun::star::util::Color getOddRowBackgroundColor();
        virtual ::com::sun::star::util::Color getEvenRowBackgroundColor();
        virtual ::com::sun::star::style::VerticalAlignment getVerticalAlign();

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
        void    setColumnModel( ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumnModel > const & i_gridColumnModel );
        bool    hasColumnModel() const;
        void    setRowHeaders(bool _bRowHeaders);
        void    setColumnHeaders(bool _bColumnHeaders);
        void    setLineColor(::com::sun::star::util::Color _rColor);
        void    setHeaderBackgroundColor(::com::sun::star::util::Color _rColor);
        void    setTextColor(::com::sun::star::util::Color _rColor);
        void    setOddRowBackgroundColor(::com::sun::star::util::Color _rColor);
        void    setEvenRowBackgroundColor(::com::sun::star::util::Color _rColor);
        void    setVerticalAlign(::com::sun::star::style::VerticalAlignment _rAlign);

        /// retrieves the index of a column within the model
        ColPos getColumnPos( UnoGridColumnFacade const & i_column ) const;

        /// notifies a change in a column belonging to the model
        void    notifyColumnChange( ColPos const i_columnPos, ColumnAttributeGroup const i_attributeGroup ) const;

#ifdef DBG_UTIL
        const char* checkInvariants() const;
#endif
    };

// .....................................................................................................................
} } // svt::table
// .....................................................................................................................

#endif // _UNOCONTROL_TABLEMODEL_HXX_
