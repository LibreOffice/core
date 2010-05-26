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
#include <com/sun/star/awt/grid/XGridColumnModel.hpp>
#include <com/sun/star/awt/grid/XGridColumn.hpp>
//#include <com/sun/star/awt/grid/GridColumnEvent.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>
#include <rtl/ref.hxx>
#include <vector>
#include <toolkit/helper/mutexandbroadcasthelper.hxx>
#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/awt/grid/XGridColumnListener.hpp>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::awt::grid;
using namespace ::com::sun::star::lang;

namespace toolkit
{

//enum broadcast_type { column_added, column_removed, column_changed};

class DefaultGridColumnModel : public ::cppu::WeakImplHelper2< XGridColumnModel, XServiceInfo >,
                             public MutexAndBroadcastHelper
{
public:
    DefaultGridColumnModel(const Reference< XMultiServiceFactory >& xFactory);
    virtual ~DefaultGridColumnModel();

    // XGridColumnModel

    //virtual ::sal_Bool SAL_CALL getColumnSelectionAllowed() throw (::com::sun::star::uno::RuntimeException);
    //virtual void SAL_CALL setColumnSelectionAllowed(::sal_Bool the_value) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getColumnCount() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL addColumn(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumn > & column) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumn > > SAL_CALL getColumns() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumn > SAL_CALL getColumn(::sal_Int32 index) throw (::com::sun::star::uno::RuntimeException);
    //virtual void SAL_CALL addColumnListener( const Reference< XGridColumnListener >& xListener ) throw (RuntimeException);
    //virtual void SAL_CALL removeColumnListener( const Reference< XGridColumnListener >& xListener ) throw (RuntimeException);
    virtual void SAL_CALL setColumnHeaderHeight( sal_Int32 _value) throw (com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getColumnHeaderHeight() throw (com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDefaultColumns(sal_Int32 rowElements) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumn > SAL_CALL copyColumn(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumn > & column)  throw (::com::sun::star::uno::RuntimeException);
    // XComponent
    virtual void SAL_CALL dispose(  ) throw (RuntimeException);
    virtual void SAL_CALL addEventListener( const Reference< XEventListener >& xListener ) throw (RuntimeException);
    virtual void SAL_CALL removeEventListener( const Reference< XEventListener >& aListener ) throw (RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);
private:

    /*void broadcast( broadcast_type eType, const GridColumnEvent& aEvent );
    void broadcast_changed( ::rtl::OUString name, Any oldValue, Any newValue, sal_Int32 index,const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumn > & rColumn );
    void broadcast_add( sal_Int32 index,const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumn > & rColumn );
    void broadcast_remove( sal_Int32 index, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumn > & rColumn );*/

    std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumn > > columns;
    sal_Bool selectionAllowed;
    sal_Int32 m_nColumnHeaderHeight;
    Reference< XMultiServiceFactory > m_xFactory;
};

}
