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

#ifndef INCLUDED_SVTOOLS_SOURCE_UNO_SVTXGRIDCONTROL_HXX
#define INCLUDED_SVTOOLS_SOURCE_UNO_SVTXGRIDCONTROL_HXX

#include "unocontroltablemodel.hxx"
#include <table/tablecontrol.hxx>
#include <com/sun/star/awt/grid/XGridControl.hpp>
#include <com/sun/star/awt/grid/XGridRowSelection.hpp>
#include <com/sun/star/awt/grid/XGridDataListener.hpp>
#include <com/sun/star/awt/grid/GridDataEvent.hpp>
#include <com/sun/star/awt/grid/GridColumnEvent.hpp>
#include <com/sun/star/awt/grid/XGridColumnModel.hpp>
#include <com/sun/star/awt/grid/XGridDataModel.hpp>
#include <com/sun/star/awt/grid/XGridSelectionListener.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/awt/vclxwindows.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/implbase.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>


namespace svt { namespace table {
    class TableControl;
} }

typedef ::cppu::ImplInheritanceHelper  <   VCLXWindow
                                        ,   ::com::sun::star::awt::grid::XGridControl
                                        ,   ::com::sun::star::awt::grid::XGridRowSelection
                                        ,   ::com::sun::star::awt::grid::XGridDataListener
                                        ,   ::com::sun::star::container::XContainerListener
                                        >   SVTXGridControl_Base;
class SVTXGridControl : public SVTXGridControl_Base
{
private:
    std::shared_ptr< ::svt::table::UnoControlTableModel >   m_xTableModel;
    bool                                                        m_bTableModelInitCompleted;
    SelectionListenerMultiplexer                                m_aSelectionListeners;

protected:
    virtual void    ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
    void            ImplCallItemListeners();

public:
    SVTXGridControl();
    virtual ~SVTXGridControl();

    // XGridDataListener
    virtual void SAL_CALL rowsInserted( const ::com::sun::star::awt::grid::GridDataEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL rowsRemoved( const ::com::sun::star::awt::grid::GridDataEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL dataChanged( const ::com::sun::star::awt::grid::GridDataEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL rowHeadingChanged( const ::com::sun::star::awt::grid::GridDataEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XContainerListener
    virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XGridControl
    virtual ::sal_Int32 SAL_CALL getRowAtPoint(::sal_Int32 x, ::sal_Int32 y) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getColumnAtPoint(::sal_Int32 x, ::sal_Int32 y) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getCurrentColumn(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getCurrentRow(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL goToCell( ::sal_Int32 i_columnIndex, ::sal_Int32 i_rowIndex ) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::util::VetoException, std::exception) override;

    // XGridRowSelection
    virtual void SAL_CALL selectRow( ::sal_Int32 i_rowIndex ) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException, std::exception ) override;
    virtual void SAL_CALL selectAllRows() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL deselectRow( ::sal_Int32 i_rowIndex ) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException, std::exception ) override;
    virtual void SAL_CALL deselectAllRows() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::sal_Int32 > SAL_CALL getSelectedRows() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasSelectedRows() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isRowSelected(::sal_Int32 index) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addSelectionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridSelectionListener > & listener) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeSelectionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridSelectionListener > & listener) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    void SAL_CALL setProperty( const OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::lang::XComponent
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XWindow
    void SAL_CALL setEnable( sal_Bool bEnable ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

protected:
    // VCLXWindow
    virtual void    SetWindow( const VclPtr< vcl::Window > &pWindow ) override;

private:
    void    impl_updateColumnsFromModel_nothrow();
    void    impl_checkTableModelInit();

    void    impl_checkColumnIndex_throw( ::svt::table::TableControl const & i_table, sal_Int32 const i_columnIndex ) const;
    void    impl_checkRowIndex_throw( ::svt::table::TableControl const & i_table, sal_Int32 const i_rowIndex ) const;
};
#endif // INCLUDED_SVTOOLS_SOURCE_UNO_SVTXGRIDCONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
