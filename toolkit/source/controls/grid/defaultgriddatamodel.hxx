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

#include <com/sun/star/awt/grid/XMutableGridDataModel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase2.hxx>
#include <toolkit/helper/mutexandbroadcasthelper.hxx>

#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::awt::grid;
using namespace ::com::sun::star::lang;

namespace comphelper
{
    class ComponentGuard;
}

namespace toolkit
{

enum broadcast_type { row_added, row_removed, data_changed};

typedef ::cppu::WeakComponentImplHelper2    <   XMutableGridDataModel
                                            ,   XServiceInfo
                                            >   DefaultGridDataModel_Base;

class DefaultGridDataModel  :public ::cppu::BaseMutex
                            ,public DefaultGridDataModel_Base
{
public:
    DefaultGridDataModel();
    DefaultGridDataModel( DefaultGridDataModel const & i_copySource );
    virtual ~DefaultGridDataModel();

    // XMutableGridDataModel
    virtual void SAL_CALL addRow( const Any& i_heading, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Data ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addRows( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>& Headings, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& Data ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insertRow( ::sal_Int32 i_index, const ::com::sun::star::uno::Any& i_heading, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Data ) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException);
    virtual void SAL_CALL insertRows( ::sal_Int32 i_index, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>& Headings, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& Data ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeRow( ::sal_Int32 RowIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeAllRows(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL updateCellData( ::sal_Int32 ColumnIndex, ::sal_Int32 RowIndex, const ::com::sun::star::uno::Any& Value ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL updateRowData( const ::com::sun::star::uno::Sequence< ::sal_Int32 >& ColumnIndexes, ::sal_Int32 RowIndex, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Values ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL updateRowHeading( ::sal_Int32 RowIndex, const ::com::sun::star::uno::Any& Heading ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL updateCellToolTip( ::sal_Int32 ColumnIndex, ::sal_Int32 RowIndex, const ::com::sun::star::uno::Any& Value ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL updateRowToolTip( ::sal_Int32 RowIndex, const ::com::sun::star::uno::Any& Value ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addGridDataListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridDataListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeGridDataListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridDataListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);

    // XGridDataModel
    virtual ::sal_Int32 SAL_CALL getRowCount() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getColumnCount() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getCellData( ::sal_Int32 Column, ::sal_Int32 Row ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getCellToolTip( ::sal_Int32 Column, ::sal_Int32 Row ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getRowHeading( ::sal_Int32 RowIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getRowData( ::sal_Int32 RowIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    // OComponentHelper
    virtual void SAL_CALL disposing();

    // XCloneable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone(  ) throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (RuntimeException);

private:
    typedef ::std::pair< Any, Any >     CellData;
    typedef ::std::vector< CellData >   RowData;
    typedef ::std::vector< RowData >    GridData;

    void broadcast(
        GridDataEvent const & i_event,
        void ( SAL_CALL ::com::sun::star::awt::grid::XGridDataListener::*i_listenerMethod )( ::com::sun::star::awt::grid::GridDataEvent const & ),
        ::comphelper::ComponentGuard & i_instanceLock
    );

    void    impl_insertRow( sal_Int32 const i_position, Any const & i_heading, Sequence< Any > const & i_rowData, sal_Int32 const i_assumedColCount = -1 );

    ::sal_Int32 impl_getRowCount_nolck() const { return sal_Int32( m_aData.size() ); }

    CellData const &    impl_getCellData_throw( sal_Int32 const i_columnIndex, sal_Int32 const i_rowIndex ) const;
    CellData&           impl_getCellDataAccess_throw( sal_Int32 const i_columnIndex, sal_Int32 const i_rowIndex );
    RowData&            impl_getRowDataAccess_throw( sal_Int32 const i_rowIndex, size_t const i_requiredColumnCount );

    GridData                                    m_aData;
    ::std::vector< ::com::sun::star::uno::Any > m_aRowHeaders;
    sal_Int32                                   m_nColumnCount;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
