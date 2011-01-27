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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_toolkit.hxx"
#include "defaultgridcolumnmodel.hxx"
#include <comphelper/sequence.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <rtl/ref.hxx>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::awt::grid;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::style;

namespace toolkit
{

///////////////////////////////////////////////////////////////////////
// class DefaultGridColumnModel
///////////////////////////////////////////////////////////////////////

DefaultGridColumnModel::DefaultGridColumnModel(const Reference< XMultiServiceFactory >& xFactory)
: columns(std::vector< Reference< XGridColumn > >())
 ,m_nColumnHeaderHeight(0)
 ,m_xFactory(xFactory)
{
}

//---------------------------------------------------------------------

DefaultGridColumnModel::~DefaultGridColumnModel()
{
}

//---------------------------------------------------------------------

::sal_Int32 SAL_CALL DefaultGridColumnModel::getColumnCount() throw (::com::sun::star::uno::RuntimeException)
{
    return columns.size();
}

//---------------------------------------------------------------------

::sal_Int32 SAL_CALL DefaultGridColumnModel::addColumn(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumn > & column) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    Reference<XGridColumn> xColumn(column);
    columns.push_back(xColumn);
    sal_Int32 index = columns.size() - 1;
    xColumn->setIndex(index);
    return index;
}

//---------------------------------------------------------------------

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumn > > SAL_CALL DefaultGridColumnModel::getColumns() throw (::com::sun::star::uno::RuntimeException)
{
    return comphelper::containerToSequence(columns);
}

//---------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumn > SAL_CALL DefaultGridColumnModel::getColumn(::sal_Int32 index) throw (::com::sun::star::uno::RuntimeException)
{
    if ( index >=0 && index < ((sal_Int32)columns.size()))
    {
        return columns[index];
    }
    else
        return Reference< XGridColumn >();
}
//---------------------------------------------------------------------
void SAL_CALL DefaultGridColumnModel::setColumnHeaderHeight(sal_Int32 _value) throw (::com::sun::star::uno::RuntimeException)
{
    m_nColumnHeaderHeight = _value;
}
//---------------------------------------------------------------------
sal_Int32 SAL_CALL DefaultGridColumnModel::getColumnHeaderHeight() throw (::com::sun::star::uno::RuntimeException)
{
    return m_nColumnHeaderHeight;
}

//---------------------------------------------------------------------
void SAL_CALL DefaultGridColumnModel::setDefaultColumns(sal_Int32 rowElements) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    for(sal_Int32 i=0;i<rowElements;i++)
    {
        Reference<XGridColumn> xColumn( m_xFactory->createInstance ( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.grid.GridColumn")) ), UNO_QUERY );
        columns.push_back(xColumn);
        xColumn->setIndex(i);
    }
}
::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumn > SAL_CALL DefaultGridColumnModel::copyColumn(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumn > & column)  throw (::com::sun::star::uno::RuntimeException)
{
    Reference<XGridColumn> xColumn( m_xFactory->createInstance ( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.grid.GridColumn")) ), UNO_QUERY );
    xColumn->setColumnWidth(column->getColumnWidth());
    xColumn->setPreferredWidth(column->getPreferredWidth());
    xColumn->setMaxWidth(column->getMaxWidth());
    xColumn->setMinWidth(column->getMinWidth());
    xColumn->setPreferredWidth(column->getPreferredWidth());
    xColumn->setResizeable(column->getResizeable());
    xColumn->setTitle(column->getTitle());
    xColumn->setHorizontalAlign(column->getHorizontalAlign());
    return xColumn;
}
//---------------------------------------------------------------------
// XComponent
//---------------------------------------------------------------------

void SAL_CALL DefaultGridColumnModel::dispose() throw (RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::lang::EventObject aEvent;
    aEvent.Source.set( static_cast< ::cppu::OWeakObject* >( this ) );
    BrdcstHelper.aLC.disposeAndClear( aEvent );

}

//---------------------------------------------------------------------

void SAL_CALL DefaultGridColumnModel::addEventListener( const Reference< XEventListener >& xListener ) throw (RuntimeException)
{
    BrdcstHelper.addListener( XEventListener::static_type(), xListener );
}

//---------------------------------------------------------------------

void SAL_CALL DefaultGridColumnModel::removeEventListener( const Reference< XEventListener >& xListener ) throw (RuntimeException)
{
    BrdcstHelper.removeListener( XEventListener::static_type(), xListener );
}

//---------------------------------------------------------------------
// XServiceInfo
//---------------------------------------------------------------------

::rtl::OUString SAL_CALL DefaultGridColumnModel::getImplementationName(  ) throw (RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    static const OUString aImplName( RTL_CONSTASCII_USTRINGPARAM( "toolkit.DefaultGridColumnModel" ) );
    return aImplName;
}

//---------------------------------------------------------------------

sal_Bool SAL_CALL DefaultGridColumnModel::supportsService( const ::rtl::OUString& ServiceName ) throw (RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    return ServiceName.equalsAscii( szServiceName_DefaultGridColumnModel );
}

//---------------------------------------------------------------------

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL DefaultGridColumnModel::getSupportedServiceNames(  ) throw (RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    static const OUString aServiceName( OUString::createFromAscii( szServiceName_DefaultGridColumnModel ) );
    static const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

}

Reference< XInterface > SAL_CALL DefaultGridColumnModel_CreateInstance( const Reference< XMultiServiceFactory >& _rFactory)
{
    return Reference < XInterface >( ( ::cppu::OWeakObject* ) new ::toolkit::DefaultGridColumnModel( _rFactory ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
