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
#include <com/sun/star/awt/grid/XGridDataModel.hpp>
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

class DefaultGridDataModel : public ::cppu::WeakImplHelper2< XGridDataModel, XServiceInfo >,
                             public MutexAndBroadcastHelper
{
public:
    DefaultGridDataModel();
    virtual ~DefaultGridDataModel();

    // XGridDataModel
    virtual ::sal_Int32 SAL_CALL getRowHeight() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setRowHeight(::sal_Int32 the_value) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getRowCount() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getRowHeaders() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setRowHeaders(const ::com::sun::star::uno::Sequence< ::rtl::OUString > & value) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< Any > > SAL_CALL getData() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addRow(const ::rtl::OUString & headername, const ::com::sun::star::uno::Sequence< Any > & data) throw (::com::sun::star::uno::RuntimeException);
       virtual void SAL_CALL removeRow(::sal_Int32 index) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addDataListener( const Reference< XGridDataListener >& xListener ) throw (RuntimeException);
    virtual void SAL_CALL removeDataListener( const Reference< XGridDataListener >& xListener ) throw (RuntimeException);
    virtual void SAL_CALL removeAll() throw (RuntimeException);
    virtual void SAL_CALL setRowHeaderWidth(sal_Int32 _value) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getRowHeaderWidth() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL updateCell( ::sal_Int32 row, ::sal_Int32 column, const ::com::sun::star::uno::Any& value ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL updateRow( ::sal_Int32 row, const ::com::sun::star::uno::Sequence< ::sal_Int32 >& columns, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& values ) throw (::com::sun::star::uno::RuntimeException);
    // XComponent
    virtual void SAL_CALL dispose(  ) throw (RuntimeException);
    virtual void SAL_CALL addEventListener( const Reference< XEventListener >& xListener ) throw (RuntimeException);
    virtual void SAL_CALL removeEventListener( const Reference< XEventListener >& aListener ) throw (RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (RuntimeException);

private:

    void broadcast( broadcast_type eType, const GridDataEvent& aEvent ) throw (::com::sun::star::uno::RuntimeException);
    void broadcast_changed( ::rtl::OUString name, sal_Int32 index, Any oldValue, Any newValue ) throw (::com::sun::star::uno::RuntimeException);
    void broadcast_add( sal_Int32 index, const ::rtl::OUString & headerName,  const ::com::sun::star::uno::Sequence< Any  > rowData ) throw (::com::sun::star::uno::RuntimeException);
    void broadcast_remove( sal_Int32 index, const ::rtl::OUString & headerName, const ::com::sun::star::uno::Sequence< Any  > rowData ) throw (::com::sun::star::uno::RuntimeException);

    sal_Int32 rowHeight;
    std::vector< std::vector < Any > > data;
    std::vector< ::rtl::OUString > rowHeaders;
    sal_Int32 m_nRowHeaderWidth;
};

}
