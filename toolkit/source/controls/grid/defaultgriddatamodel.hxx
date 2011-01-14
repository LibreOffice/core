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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_toolkit.hxx"
#include <com/sun/star/awt/grid/XMutableGridDataModel.hpp>
#include <com/sun/star/awt/grid/GridDataEvent.hpp>
#include <com/sun/star/awt/grid/XGridDataListener.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>
#include <rtl/ref.hxx>
#include <vector>
#include <toolkit/helper/mutexandbroadcasthelper.hxx>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::awt::grid;
using namespace ::com::sun::star::lang;

namespace toolkit
{

enum broadcast_type { row_added, row_removed, data_changed};

typedef ::cppu::WeakImplHelper2< XMutableGridDataModel, XServiceInfo > DefaultGridDataModel_Base;
class DefaultGridDataModel : public DefaultGridDataModel_Base,
                             public MutexAndBroadcastHelper
{
public:
    DefaultGridDataModel();
    DefaultGridDataModel( DefaultGridDataModel const & i_copySource );
    virtual ~DefaultGridDataModel();

    // XMutableGridDataModel
    virtual void SAL_CALL addRow( const ::rtl::OUString& Title, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Data ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addRows( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& Titles, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& Data ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeRow( ::sal_Int32 RowIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeAllRows(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL updateCellData( ::sal_Int32 ColumnIndex, ::sal_Int32 RowIndex, const ::com::sun::star::uno::Any& Value ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL updateRowData( const ::com::sun::star::uno::Sequence< ::sal_Int32 >& ColumnIndexes, ::sal_Int32 RowIndex, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Values ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setRowTitle( ::sal_Int32 RowIndex, const ::rtl::OUString& Title ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL updateCellToolTip( ::sal_Int32 ColumnIndex, ::sal_Int32 RowIndex, const ::com::sun::star::uno::Any& Value ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL updateRowToolTip( ::sal_Int32 RowIndex, const ::com::sun::star::uno::Any& Value ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addGridDataListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridDataListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeGridDataListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridDataListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);

    // XGridDataModel
    virtual ::sal_Int32 SAL_CALL getRowCount() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getColumnCount() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getCellData( ::sal_Int32 Column, ::sal_Int32 Row ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getCellToolTip( ::sal_Int32 Column, ::sal_Int32 Row ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getRowTitle( ::sal_Int32 RowIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (RuntimeException);
    virtual void SAL_CALL addEventListener( const Reference< XEventListener >& xListener ) throw (RuntimeException);
    virtual void SAL_CALL removeEventListener( const Reference< XEventListener >& aListener ) throw (RuntimeException);

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
        ::osl::ClearableMutexGuard & i_instanceLock
    );

    void    impl_addRow( Sequence< Any > const & i_rowData, sal_Int32 const i_assumedColCount = -1 );

    CellData const &    impl_getCellData_throw( sal_Int32 const i_columnIndex, sal_Int32 const i_rowIndex ) const;
    CellData&           impl_getCellDataAccess_throw( sal_Int32 const i_columnIndex, sal_Int32 const i_rowIndex );
    RowData&            impl_getRowDataAccess_throw( sal_Int32 const i_rowIndex, size_t const i_requiredColumnCount );

    GridData                            m_aData;
    ::std::vector< ::rtl::OUString >    m_aRowHeaders;
    sal_Int32                           m_nColumnCount;
};

}
