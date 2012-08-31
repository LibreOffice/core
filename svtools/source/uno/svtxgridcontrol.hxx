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

#ifndef _SVT_GRIDCONTROL_HXX_
#define _SVT_GRIDCONTROL_HXX_

#include <unocontroltablemodel.hxx>
#include <svtools/table/tablecontrol.hxx>
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
#include <cppuhelper/implbase4.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>


using namespace ::svt::table;

typedef ::cppu::ImplInheritanceHelper4  <   VCLXWindow
                                        ,   ::com::sun::star::awt::grid::XGridControl
                                        ,   ::com::sun::star::awt::grid::XGridRowSelection
                                        ,   ::com::sun::star::awt::grid::XGridDataListener
                                        ,   ::com::sun::star::container::XContainerListener
                                        >   SVTXGridControl_Base;
class SVTXGridControl : public SVTXGridControl_Base
{
private:
    ::boost::shared_ptr< UnoControlTableModel >     m_pTableModel;
    bool                                            m_bTableModelInitCompleted;
    SelectionListenerMultiplexer                    m_aSelectionListeners;

protected:
    virtual void    ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );
    void            ImplCallItemListeners();

public:
    SVTXGridControl();
    ~SVTXGridControl();

    // XGridDataListener
    virtual void SAL_CALL rowsInserted( const ::com::sun::star::awt::grid::GridDataEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL rowsRemoved( const ::com::sun::star::awt::grid::GridDataEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL dataChanged( const ::com::sun::star::awt::grid::GridDataEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL rowHeadingChanged( const ::com::sun::star::awt::grid::GridDataEvent& Event ) throw (::com::sun::star::uno::RuntimeException);

    // XContainerListener
    virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    // XGridControl
    virtual ::sal_Int32 SAL_CALL getRowAtPoint(::sal_Int32 x, ::sal_Int32 y) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getColumnAtPoint(::sal_Int32 x, ::sal_Int32 y) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getCurrentColumn(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getCurrentRow(  ) throw (::com::sun::star::uno::RuntimeException);

    // XGridRowSelection
    virtual void SAL_CALL selectRow( ::sal_Int32 i_rowIndex ) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException );
    virtual void SAL_CALL selectAllRows() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL deselectRow( ::sal_Int32 i_rowIndex ) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException );
    virtual void SAL_CALL deselectAllRows() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::sal_Int32 > SAL_CALL getSelectedRows() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasSelectedRows() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isRowSelected(::sal_Int32 index) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addSelectionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridSelectionListener > & listener) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeSelectionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridSelectionListener > & listener) throw (::com::sun::star::uno::RuntimeException);

    void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XComponent
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);

protected:
    // VCLXWindow
    virtual void    SetWindow( Window* pWindow );

private:
    void    impl_updateColumnsFromModel_nothrow();
    void    impl_checkTableModelInit();
};
#endif // _SVT_GRIDCONTROL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
