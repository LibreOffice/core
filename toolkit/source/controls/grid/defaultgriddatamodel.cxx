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
#include "defaultgriddatamodel.hxx"
#include <comphelper/sequence.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <rtl/ref.hxx>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::awt::grid;
using namespace ::com::sun::star::lang;

#define ROWHEIGHT ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "RowHeight" ))
#define ROWHEADERS ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "RowHeaders" ))
#define CELLUPDATED ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "CellUpdated" ))
#define ROWUPDATED ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "RowUpdated" ))
#define ROWHEADERWIDTH ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "RowHeaderWidth" ))

namespace toolkit
{

///////////////////////////////////////////////////////////////////////
// class DefaultGridDataModel
///////////////////////////////////////////////////////////////////////

DefaultGridDataModel::DefaultGridDataModel()
: rowHeight(0),
  rowHeaders(std::vector< ::rtl::OUString >()),
  m_nRowHeaderWidth(10)
{
}

//---------------------------------------------------------------------

DefaultGridDataModel::~DefaultGridDataModel()
{
}

void DefaultGridDataModel::broadcast( broadcast_type eType, const GridDataEvent& aEvent ) throw (::com::sun::star::uno::RuntimeException)
{
    ::cppu::OInterfaceContainerHelper* pIter = BrdcstHelper.getContainer( XGridDataListener::static_type() );
    if( pIter )
    {
        ::cppu::OInterfaceIteratorHelper aListIter(*pIter);
        while(aListIter.hasMoreElements())
        {
            XGridDataListener* pListener = static_cast<XGridDataListener*>(aListIter.next());
            switch( eType )
            {
            case row_added:     pListener->rowAdded(aEvent); break;
            case row_removed:   pListener->rowRemoved(aEvent); break;
            case data_changed:  pListener->dataChanged(aEvent); break;
            }
        }
    }
}

//---------------------------------------------------------------------

void DefaultGridDataModel::broadcast_changed( ::rtl::OUString name, sal_Int32 index, Any oldValue, Any newValue) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
    GridDataEvent aEvent( xSource, name, oldValue, newValue, index, ::rtl::OUString(), Sequence< Any >());
    broadcast( data_changed, aEvent);
}

//---------------------------------------------------------------------

void DefaultGridDataModel::broadcast_add( sal_Int32 index, const ::rtl::OUString & headerName,
                                         ::com::sun::star::uno::Sequence< Any > rowData ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
    GridDataEvent aEvent( xSource, ::rtl::OUString(), Any(), Any(), index, headerName, (const ::com::sun::star::uno::Sequence< Any >&)rowData );
    broadcast( row_added, aEvent);
}

//---------------------------------------------------------------------

void DefaultGridDataModel::broadcast_remove( sal_Int32 index, const ::rtl::OUString & headerName,
                                            ::com::sun::star::uno::Sequence< Any > rowData ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
    GridDataEvent aEvent( xSource, ::rtl::OUString(), Any(), Any(), index, headerName, rowData );
    broadcast( row_removed, aEvent);
}

//---------------------------------------------------------------------

//---------------------------------------------------------------------
// XDefaultGridDataModel
//---------------------------------------------------------------------
::sal_Int32 SAL_CALL DefaultGridDataModel::getRowHeight() throw (::com::sun::star::uno::RuntimeException)
{
    return rowHeight;
}

//---------------------------------------------------------------------

void SAL_CALL DefaultGridDataModel::setRowHeight(::sal_Int32 value) throw (::com::sun::star::uno::RuntimeException)
{
    sal_Int32 oldValue = rowHeight;
    rowHeight = value;

    broadcast_changed( ROWHEIGHT, 0, Any(oldValue), Any(value));
}

//---------------------------------------------------------------------

::sal_Int32 SAL_CALL DefaultGridDataModel::getRowCount() throw (::com::sun::star::uno::RuntimeException)
{
    return data.size();
}

//---------------------------------------------------------------------

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL DefaultGridDataModel::getRowHeaders() throw (::com::sun::star::uno::RuntimeException)
{
    return  comphelper::containerToSequence(rowHeaders);
}

//---------------------------------------------------------------------

void SAL_CALL DefaultGridDataModel::setRowHeaders(const ::com::sun::star::uno::Sequence< ::rtl::OUString > & value) throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString > oldValue( comphelper::containerToSequence(rowHeaders) );

    std::vector< rtl::OUString>::iterator iterator;
    int i = 0;
    int sequenceSize = value.getLength();

    for(iterator = rowHeaders.begin(); iterator != rowHeaders.end(); iterator++)
    {
        if ( sequenceSize > i )
            *iterator = value[i];
        else
            *iterator = ::rtl::OUString();
        i++;
    }

    broadcast_changed( ROWHEADERS, 0, Any(oldValue), Any(comphelper::containerToSequence(rowHeaders)) );
}

//---------------------------------------------------------------------

void SAL_CALL DefaultGridDataModel::addRow(const ::rtl::OUString & headername, const ::com::sun::star::uno::Sequence< Any > & rRowdata) throw (::com::sun::star::uno::RuntimeException)
{
    // store header name
    rowHeaders.push_back(headername);

    // store row data
    std::vector< Any > newRow;
    for ( int i = 0; i < rRowdata.getLength();i++)
    {
        newRow.push_back(rRowdata[i]);
    }

    data.push_back( newRow );

    broadcast_add( data.size()-1, headername, comphelper::containerToSequence(newRow));

}

//---------------------------------------------------------------------

void SAL_CALL DefaultGridDataModel::removeRow(::sal_Int32 index) throw (::com::sun::star::uno::RuntimeException)
{
    if ( index >= 0 && index <= getRowCount()-1)
    {
        ::rtl::OUString headerName( (::rtl::OUString) rowHeaders[index] );
        rowHeaders.erase(rowHeaders.begin() + index);

        Sequence< Any > rowData ( (Sequence< Any >&)data[index] );
        data.erase(data.begin() + index);
        broadcast_remove( index, headerName, rowData);
    }
    else
        return;
}
//---------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< Any > > SAL_CALL DefaultGridDataModel::getData() throw (::com::sun::star::uno::RuntimeException)
{

    std::vector< std::vector< Any > >::iterator iterator;
    std::vector< Sequence< Any  > > dummyContainer(0);


    for(iterator = data.begin(); iterator != data.end(); iterator++)
    {
        Sequence< Any > cols(comphelper::containerToSequence(*iterator));
        dummyContainer.push_back( cols );
    }
    Sequence< Sequence< Any  > > dataSequence(comphelper::containerToSequence(dummyContainer));

    return dataSequence;
}

//---------------------------------------------------------------------

void SAL_CALL DefaultGridDataModel::addDataListener( const Reference< XGridDataListener >& xListener ) throw (RuntimeException)
{
    BrdcstHelper.addListener( XGridDataListener::static_type(), xListener );
}

//---------------------------------------------------------------------

void SAL_CALL DefaultGridDataModel::removeDataListener( const Reference< XGridDataListener >& xListener ) throw (RuntimeException)
{
    BrdcstHelper.removeListener( XGridDataListener::static_type(), xListener );
}
//---------------------------------------------------------------------
void SAL_CALL DefaultGridDataModel::removeAll() throw (RuntimeException)
{
    rowHeaders.clear();
    data.clear();
    broadcast_remove( -1, ::rtl::OUString(), Sequence< Any >());
}
//---------------------------------------------------------------------
void SAL_CALL DefaultGridDataModel::setRowHeaderWidth(sal_Int32 _value) throw (::com::sun::star::uno::RuntimeException)
{
    sal_Int32 oldValue = m_nRowHeaderWidth;
    m_nRowHeaderWidth = _value;
    broadcast_changed( ROWHEADERWIDTH, 0, Any(oldValue), Any(_value) );
}
//---------------------------------------------------------------------
sal_Int32 SAL_CALL DefaultGridDataModel::getRowHeaderWidth() throw (::com::sun::star::uno::RuntimeException)
{
    return m_nRowHeaderWidth;
}
//---------------------------------------------------------------------
void SAL_CALL DefaultGridDataModel::updateCell(::sal_Int32 row, ::sal_Int32 column, const Any& value) throw (::com::sun::star::uno::RuntimeException)
{
    if(row >= 0 && row < (signed)data.size())
    {
        if(column >= 0 && column < (signed)data[0].size())
        {
            data[row][column] = value;
            Sequence< Any >dataSeq(comphelper::containerToSequence(data[row]));
            broadcast_changed( CELLUPDATED, row, Any(column), value );
        }
    }
}
//---------------------------------------------------------------------
void SAL_CALL DefaultGridDataModel::updateRow(::sal_Int32 row, const ::com::sun::star::uno::Sequence< ::sal_Int32 > & columns, const ::com::sun::star::uno::Sequence< Any > & values) throw (::com::sun::star::uno::RuntimeException)
{
    if(row >= 0 && row < (signed)data.size())
    {
        if(columns.getLength() == values.getLength())
        {
            for(int i = 0; i < columns.getLength(); i++)
                data[row][i] = values[i];
            Sequence< Any >dataSeq(comphelper::containerToSequence(data[row]));
            broadcast_changed( ROWUPDATED, row, Any(columns), Any(values) );
        }
    }
}
//---------------------------------------------------------------------
// XComponent
//---------------------------------------------------------------------

void SAL_CALL DefaultGridDataModel::dispose() throw (RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::lang::EventObject aEvent;
    aEvent.Source.set( static_cast< ::cppu::OWeakObject* >( this ) );
    BrdcstHelper.aLC.disposeAndClear( aEvent );

}

//---------------------------------------------------------------------

void SAL_CALL DefaultGridDataModel::addEventListener( const Reference< XEventListener >& xListener ) throw (RuntimeException)
{
    BrdcstHelper.addListener( XEventListener::static_type(), xListener );
}

//---------------------------------------------------------------------

void SAL_CALL DefaultGridDataModel::removeEventListener( const Reference< XEventListener >& xListener ) throw (RuntimeException)
{
    BrdcstHelper.removeListener( XEventListener::static_type(), xListener );
}
//---------------------------------------------------------------------
// XServiceInfo
//---------------------------------------------------------------------

::rtl::OUString SAL_CALL DefaultGridDataModel::getImplementationName(  ) throw (RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    static const OUString aImplName( RTL_CONSTASCII_USTRINGPARAM( "toolkit.DefaultGridDataModel" ) );
    return aImplName;
}

//---------------------------------------------------------------------

sal_Bool SAL_CALL DefaultGridDataModel::supportsService( const ::rtl::OUString& ServiceName ) throw (RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    return ServiceName.equalsAscii( szServiceName_DefaultGridDataModel );
}

//---------------------------------------------------------------------

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL DefaultGridDataModel::getSupportedServiceNames(  ) throw (RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    static const OUString aServiceName( OUString::createFromAscii( szServiceName_DefaultGridDataModel ) );
    static const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

}

Reference< XInterface > SAL_CALL DefaultGridDataModel_CreateInstance( const Reference< XMultiServiceFactory >& )
{
    return Reference < XInterface >( ( ::cppu::OWeakObject* ) new ::toolkit::DefaultGridDataModel() );
}
