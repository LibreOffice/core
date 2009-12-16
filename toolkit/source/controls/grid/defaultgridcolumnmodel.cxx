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
#include "defaultgridcolumnmodel.hxx"
#include <comphelper/sequence.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <rtl/ref.hxx>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::awt::grid;
using namespace ::com::sun::star::lang;

#define COLUMNSELECTIONALLOWED ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "ColumnSelectionAllowed" ))

namespace toolkit
{

///////////////////////////////////////////////////////////////////////
// class DefaultGridColumnModel
///////////////////////////////////////////////////////////////////////

DefaultGridColumnModel::DefaultGridColumnModel()
: columns(std::vector< Reference< XGridColumn > >())
{
}

//---------------------------------------------------------------------

DefaultGridColumnModel::~DefaultGridColumnModel()
{
}

//---------------------------------------------------------------------

void DefaultGridColumnModel::broadcast( broadcast_type eType, const GridColumnEvent& aEvent )
{
    ::cppu::OInterfaceContainerHelper* pIter = BrdcstHelper.getContainer( XGridColumnListener::static_type() );
    if( pIter )
    {
        ::cppu::OInterfaceIteratorHelper aListIter(*pIter);
        while(aListIter.hasMoreElements())
        {
            XGridColumnListener* pListener = static_cast<XGridColumnListener*>(aListIter.next());
            switch( eType )
            {
            case column_added:      pListener->columnAdded(aEvent); break;
            case column_removed:    pListener->columnRemoved(aEvent); break;
            case column_changed:    pListener->columnChanged(aEvent); break;
            }
        }
    }
}

//---------------------------------------------------------------------

void DefaultGridColumnModel::broadcast_changed( ::rtl::OUString name, Any oldValue, Any newValue )
{
    Reference< XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
    GridColumnEvent aEvent( xSource, name, oldValue, newValue, 0, NULL );
    broadcast( column_changed, aEvent);
}

//---------------------------------------------------------------------

void DefaultGridColumnModel::broadcast_add( sal_Int32 index, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumn > & rColumn )
{
    Reference< XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
    GridColumnEvent aEvent( xSource, ::rtl::OUString(), Any(), Any(), index, rColumn );
    broadcast( column_added, aEvent);
}

//---------------------------------------------------------------------

void DefaultGridColumnModel::broadcast_remove( sal_Int32 index, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumn > & rColumn )
{
    Reference< XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
    GridColumnEvent aEvent( xSource, ::rtl::OUString(), Any(), Any(), index, rColumn );
    broadcast( column_changed, aEvent);
}

//---------------------------------------------------------------------
// XDefaultGridColumnModel
//---------------------------------------------------------------------
::sal_Bool SAL_CALL DefaultGridColumnModel::getColumnSelectionAllowed() throw (::com::sun::star::uno::RuntimeException)
{
    return selectionAllowed;
}

//---------------------------------------------------------------------

void SAL_CALL DefaultGridColumnModel::setColumnSelectionAllowed(::sal_Bool value) throw (::com::sun::star::uno::RuntimeException)
{
    sal_Bool oldValue = selectionAllowed;
    selectionAllowed = value;
    broadcast_changed( COLUMNSELECTIONALLOWED, Any(oldValue) , Any(selectionAllowed));
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

    columns.push_back(column);

    sal_Int32 index = columns.size() - 1;
    broadcast_add(index, column );
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
        return columns[index];
    else
        return Reference< XGridColumn >();
}

void SAL_CALL DefaultGridColumnModel::addColumnListener( const Reference< XGridColumnListener >& xListener ) throw (RuntimeException)
{
    BrdcstHelper.addListener( XGridColumnListener::static_type(), xListener );
}

//---------------------------------------------------------------------

void SAL_CALL DefaultGridColumnModel::removeColumnListener( const Reference< XGridColumnListener >& xListener ) throw (RuntimeException)
{
    BrdcstHelper.removeListener( XGridColumnListener::static_type(), xListener );
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

Reference< XInterface > SAL_CALL DefaultGridColumnModel_CreateInstance( const Reference< XMultiServiceFactory >& )
{
    return Reference < XInterface >( ( ::cppu::OWeakObject* ) new ::toolkit::DefaultGridColumnModel );
}

