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
using namespace ::com::sun::star::style;

#define COLWIDTH ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "ColWidth" ))
#define MAXWIDTH ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "MaxWidth" ))
#define MINWIDTH ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "MinWidth" ))
#define PREFWIDTH ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "PrefWidth" ))
#define HALIGN ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "HAlign" ))
#define TITLE ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Title" ))
#define COLRESIZE ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "ColumnResize" ))
#define UPDATE ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "UpdateWidth" ))

namespace toolkit
{

///////////////////////////////////////////////////////////////////////
// class GridColumn
///////////////////////////////////////////////////////////////////////

GridColumn::GridColumn()
: identifier(Any())
,index(0)
,columnWidth(4)
,preferredWidth(0)
,maxWidth(0)
,minWidth(0)
,bResizeable(true)
,horizontalAlign(HorizontalAlignment(0))
{
}

//---------------------------------------------------------------------

GridColumn::~GridColumn()
{
}

//---------------------------------------------------------------------

void GridColumn::broadcast( broadcast_column_type eType, const GridColumnEvent& aEvent )
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
                case column_attribute_changed:  pListener->columnChanged(aEvent); break;
            }
        }
    }
}

//---------------------------------------------------------------------

void GridColumn::broadcast_changed(::rtl::OUString name, Any oldValue, Any newValue)
{
    Reference< XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
    GridColumnEvent aEvent( xSource, name, oldValue, newValue, index);
    broadcast( column_attribute_changed, aEvent);
}

void SAL_CALL GridColumn::updateColumn(const ::rtl::OUString& name, sal_Int32 width) throw (::com::sun::star::uno::RuntimeException)
{
    if(PREFWIDTH == name)
        preferredWidth = width;
    else if (COLWIDTH == name)
        columnWidth = width;
}
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
    broadcast_changed(UPDATE, Any(columnWidth), Any());
    return columnWidth;
}

//--------------------------------------------------------------------

void SAL_CALL GridColumn::setColumnWidth(::sal_Int32 value) throw (::com::sun::star::uno::RuntimeException)
{
    columnWidth = value;
    broadcast_changed(COLWIDTH, Any(columnWidth),Any(value));
}
//--------------------------------------------------------------------

::sal_Int32 SAL_CALL GridColumn::getPreferredWidth() throw (::com::sun::star::uno::RuntimeException)
{
    broadcast_changed(UPDATE, Any(preferredWidth), Any());
    return preferredWidth;
}

//--------------------------------------------------------------------

void SAL_CALL GridColumn::setPreferredWidth(::sal_Int32 value) throw (::com::sun::star::uno::RuntimeException)
{
    preferredWidth = value;
    broadcast_changed(PREFWIDTH, Any(preferredWidth),Any(value));
}
//--------------------------------------------------------------------

::sal_Int32 SAL_CALL GridColumn::getMaxWidth() throw (::com::sun::star::uno::RuntimeException)
{
    return maxWidth;
}

//--------------------------------------------------------------------

void SAL_CALL GridColumn::setMaxWidth(::sal_Int32 value) throw (::com::sun::star::uno::RuntimeException)
{
    maxWidth = value;
    broadcast_changed(MAXWIDTH, Any(maxWidth),Any(value));
}
//--------------------------------------------------------------------

::sal_Int32 SAL_CALL GridColumn::getMinWidth() throw (::com::sun::star::uno::RuntimeException)
{
    return minWidth;
}

//--------------------------------------------------------------------

void SAL_CALL GridColumn::setMinWidth(::sal_Int32 value) throw (::com::sun::star::uno::RuntimeException)
{
    minWidth = value;
    broadcast_changed(MINWIDTH, Any(minWidth),Any(value));
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
    broadcast_changed(TITLE, Any(title),Any(value));
}
//--------------------------------------------------------------------

sal_Bool SAL_CALL GridColumn::getResizeable() throw (::com::sun::star::uno::RuntimeException)
{
    return bResizeable;
}

//--------------------------------------------------------------------

void SAL_CALL GridColumn::setResizeable(sal_Bool value) throw (::com::sun::star::uno::RuntimeException)
{
    bResizeable = value;
    broadcast_changed(COLRESIZE, Any(bResizeable),Any(value));
}
//---------------------------------------------------------------------
HorizontalAlignment SAL_CALL GridColumn::getHorizontalAlign() throw (::com::sun::star::uno::RuntimeException)
{
    return horizontalAlign;
}
//---------------------------------------------------------------------

void SAL_CALL GridColumn::setHorizontalAlign(HorizontalAlignment align) throw (::com::sun::star::uno::RuntimeException)
{
    horizontalAlign = align;
    broadcast_changed(HALIGN, Any(horizontalAlign),Any(align));
}
//---------------------------------------------------------------------
void SAL_CALL GridColumn::addColumnListener( const Reference< XGridColumnListener >& xListener ) throw (RuntimeException)
{
    BrdcstHelper.addListener( XGridColumnListener::static_type(), xListener );
}

//---------------------------------------------------------------------

void SAL_CALL GridColumn::removeColumnListener( const Reference< XGridColumnListener >& xListener ) throw (RuntimeException)
{
    BrdcstHelper.removeListener( XGridColumnListener::static_type(), xListener );
}

//---------------------------------------------------------------------
// XComponent
//---------------------------------------------------------------------

void SAL_CALL GridColumn::dispose() throw (RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::lang::EventObject aEvent;
    aEvent.Source.set( static_cast< ::cppu::OWeakObject* >( this ) );
    BrdcstHelper.aLC.disposeAndClear( aEvent );
}

//---------------------------------------------------------------------

void SAL_CALL GridColumn::addEventListener( const Reference< XEventListener >& xListener ) throw (RuntimeException)
{
    BrdcstHelper.addListener( XEventListener::static_type(), xListener );
}

//---------------------------------------------------------------------

void SAL_CALL GridColumn::removeEventListener( const Reference< XEventListener >& xListener ) throw (RuntimeException)
{
    BrdcstHelper.removeListener( XEventListener::static_type(), xListener );
}
void SAL_CALL GridColumn::setIndex(sal_Int32 _nIndex) throw (::com::sun::star::uno::RuntimeException)
{
    index = _nIndex;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
