/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: treedatamodel.cxx,v $
 * $Revision: 1.4 $
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
#include "gridcolumn.hxx"
#include <comphelper/sequence.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <rtl/ref.hxx>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::awt::grid;
using namespace ::com::sun::star::lang;

namespace toolkit
{

///////////////////////////////////////////////////////////////////////
// class GridColumn
///////////////////////////////////////////////////////////////////////

GridColumn::GridColumn()
: identifier(Any())
{
}

//---------------------------------------------------------------------

GridColumn::~GridColumn()
{
}

//---------------------------------------------------------------------

//---------------------------------------------------------------------
// XGridColumn
//---------------------------------------------------------------------

::com::sun::star::uno::Any SAL_CALL GridColumn::getIdentifier() throw (::com::sun::star::uno::RuntimeException)
{
    return identifier;
}

//---------------------------------------------------------------------

void SAL_CALL GridColumn::setIdentifier(const ::com::sun::star::uno::Any & value) throw (::com::sun::star::uno::RuntimeException)
{
    value >>= identifier;
}

//--------------------------------------------------------------------

::sal_Int32 SAL_CALL GridColumn::getColumnWidth() throw (::com::sun::star::uno::RuntimeException)
{
    return columnWidth;
}

//--------------------------------------------------------------------

void SAL_CALL GridColumn::setColumnWidth(::sal_Int32 value) throw (::com::sun::star::uno::RuntimeException)
{
    columnWidth = value;
}

//--------------------------------------------------------------------

::rtl::OUString SAL_CALL GridColumn::getTitle() throw (::com::sun::star::uno::RuntimeException)
{
    return title;
}

//--------------------------------------------------------------------

void SAL_CALL GridColumn::setTitle(const ::rtl::OUString & value) throw (::com::sun::star::uno::RuntimeException)
{
    title = value;
}

//---------------------------------------------------------------------
// XComponent
//---------------------------------------------------------------------

void SAL_CALL GridColumn::dispose() throw (RuntimeException)
{
}

//---------------------------------------------------------------------

void SAL_CALL GridColumn::addEventListener( const Reference< XEventListener >& xListener ) throw (RuntimeException)
{
    (void) xListener;
}

//---------------------------------------------------------------------

void SAL_CALL GridColumn::removeEventListener( const Reference< XEventListener >& xListener ) throw (RuntimeException)
{
    (void) xListener;
}

//---------------------------------------------------------------------
// XServiceInfo
//---------------------------------------------------------------------

::rtl::OUString SAL_CALL GridColumn::getImplementationName(  ) throw (RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    static const OUString aImplName( RTL_CONSTASCII_USTRINGPARAM( "toolkit.GridColumn" ) );
    return aImplName;
}

//---------------------------------------------------------------------

sal_Bool SAL_CALL GridColumn::supportsService( const ::rtl::OUString& ServiceName ) throw (RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    return ServiceName.equalsAscii( szServiceName_GridColumn );
}

//---------------------------------------------------------------------

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL GridColumn::getSupportedServiceNames(  ) throw (RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    static const OUString aServiceName( OUString::createFromAscii( szServiceName_GridColumn ) );
    static const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

}

Reference< XInterface > SAL_CALL GridColumn_CreateInstance( const Reference< XMultiServiceFactory >& )
{
    return Reference < XInterface >( ( ::cppu::OWeakObject* ) new ::toolkit::GridColumn );
}

