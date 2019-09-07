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
#include <com/sun/star/awt/grid/XGridSelectionListener.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <toolkit/awt/vclxwindow.hxx>
#include <cppuhelper/implbase.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>


namespace svt { namespace table {
    class TableControl;
} }

typedef ::cppu::ImplInheritanceHelper  <   VCLXWindow
                                        ,   css::awt::grid::XGridControl
                                        ,   css::awt::grid::XGridRowSelection
                                        ,   css::awt::grid::XGridDataListener
                                        ,   css::container::XContainerListener
                                        >   SVTXGridControl_Base;
class SVTXGridControl final : public SVTXGridControl_Base
{
public:
    SVTXGridControl();
    virtual ~SVTXGridControl() override;

    // XGridDataListener
    virtual void SAL_CALL rowsInserted( const css::awt::grid::GridDataEvent& Event ) override;
    virtual void SAL_CALL rowsRemoved( const css::awt::grid::GridDataEvent& Event ) override;
    virtual void SAL_CALL dataChanged( const css::awt::grid::GridDataEvent& Event ) override;
    virtual void SAL_CALL rowHeadingChanged( const css::awt::grid::GridDataEvent& Event ) override;

    // XContainerListener
    virtual void SAL_CALL elementInserted( const css::container::ContainerEvent& Event ) override;
    virtual void SAL_CALL elementRemoved( const css::container::ContainerEvent& Event ) override;
    virtual void SAL_CALL elementReplaced( const css::container::ContainerEvent& Event ) override;

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    // XGridControl
    virtual ::sal_Int32 SAL_CALL getRowAtPoint(::sal_Int32 x, ::sal_Int32 y) override;
    virtual ::sal_Int32 SAL_CALL getColumnAtPoint(::sal_Int32 x, ::sal_Int32 y) override;
    virtual ::sal_Int32 SAL_CALL getCurrentColumn(  ) override;
    virtual ::sal_Int32 SAL_CALL getCurrentRow(  ) override;
    virtual void SAL_CALL goToCell( ::sal_Int32 i_columnIndex, ::sal_Int32 i_rowIndex ) override;

    // XGridRowSelection
    virtual void SAL_CALL selectRow( ::sal_Int32 i_rowIndex ) override;
    virtual void SAL_CALL selectAllRows() override;
    virtual void SAL_CALL deselectRow( ::sal_Int32 i_rowIndex ) override;
    virtual void SAL_CALL deselectAllRows() override;
    virtual css::uno::Sequence< ::sal_Int32 > SAL_CALL getSelectedRows() override;
    virtual sal_Bool SAL_CALL hasSelectedRows() override;
    virtual sal_Bool SAL_CALL isRowSelected(::sal_Int32 index) override;
    virtual void SAL_CALL addSelectionListener(const css::uno::Reference< css::awt::grid::XGridSelectionListener > & listener) override;
    virtual void SAL_CALL removeSelectionListener(const css::uno::Reference< css::awt::grid::XGridSelectionListener > & listener) override;

    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) override;

    // css::lang::XComponent
    void SAL_CALL dispose(  ) override;

    // XWindow
    void SAL_CALL setEnable( sal_Bool bEnable ) override;

private:
    // VCLXWindow
    virtual void    SetWindow( const VclPtr< vcl::Window > &pWindow ) override;

    void    impl_updateColumnsFromModel_nothrow();
    void    impl_checkTableModelInit();

    void    impl_checkColumnIndex_throw( ::svt::table::TableControl const & i_table, sal_Int32 const i_columnIndex ) const;
    void    impl_checkRowIndex_throw( ::svt::table::TableControl const & i_table, sal_Int32 const i_rowIndex ) const;

    virtual void    ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
    void            ImplCallItemListeners();

    std::shared_ptr< ::svt::table::UnoControlTableModel >   m_xTableModel;
    bool                                                    m_bTableModelInitCompleted;
    SelectionListenerMultiplexer                            m_aSelectionListeners;
};
#endif // INCLUDED_SVTOOLS_SOURCE_UNO_SVTXGRIDCONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
