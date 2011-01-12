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

#include <comphelper/stlunosequence.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <tools/diagnose_ex.h>
#include <rtl/ref.hxx>

#include <algorithm>

using ::rtl::OUString;
using ::comphelper::stl_begin;
using ::comphelper::stl_end;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::awt::grid;
using namespace ::com::sun::star::lang;

//......................................................................................................................
namespace toolkit
//......................................................................................................................
{
    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::util::XCloneable;
    /** === end UNO using === **/

    //==================================================================================================================
    //= DefaultGridDataModel
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    DefaultGridDataModel::DefaultGridDataModel()
        :DefaultGridDataModel_Base()
        ,MutexAndBroadcastHelper()
        ,m_aRowHeaders()
        ,m_nColumnCount(0)
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    DefaultGridDataModel::DefaultGridDataModel( DefaultGridDataModel const & i_copySource )
        :DefaultGridDataModel_Base()
        ,MutexAndBroadcastHelper()
        ,m_aData( i_copySource.m_aData )
        ,m_aRowHeaders( i_copySource.m_aRowHeaders )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    DefaultGridDataModel::~DefaultGridDataModel()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    namespace
    {
        Sequence< sal_Int32 > lcl_buildSingleElementSequence( sal_Int32 const i_index )
        {
            Sequence< sal_Int32 > aIndexes(1);
            aIndexes[0] = i_index;
            return aIndexes;
        }
        Sequence< sal_Int32 > lcl_buildIndexSequence( sal_Int32 const i_start, sal_Int32 const i_end )
        {
            Sequence< sal_Int32 > aIndexes;
            ENSURE_OR_RETURN( i_end >= i_start, "lcl_buildIndexSequence: illegal indexes!", aIndexes );

            aIndexes.realloc( i_end - i_start + 1 );
            for ( sal_Int32 i = i_start; i <= i_end; ++i )
                aIndexes[ i - i_start ] = i;

            return aIndexes;
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void DefaultGridDataModel::broadcast( GridDataEvent const & i_event,
        void ( SAL_CALL XGridDataListener::*i_listenerMethod )( GridDataEvent const & ), ::osl::ClearableMutexGuard & i_instanceLock )
    {
        ::cppu::OInterfaceContainerHelper* pListeners = BrdcstHelper.getContainer( XGridDataListener::static_type() );
        if ( !pListeners )
            return;

        i_instanceLock.clear();
        pListeners->notifyEach( i_listenerMethod, i_event );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Int32 SAL_CALL DefaultGridDataModel::getRowCount() throw (::com::sun::star::uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        return m_aData.size();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Int32 SAL_CALL DefaultGridDataModel::getColumnCount() throw (::com::sun::star::uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        return m_nColumnCount;
    }

    //------------------------------------------------------------------------------------------------------------------
    Any SAL_CALL DefaultGridDataModel::getCellData( ::sal_Int32 i_column, ::sal_Int32 i_row ) throw (RuntimeException, IndexOutOfBoundsException)
    {
        ::osl::MutexGuard aGuard( GetMutex() );

        if  (   ( i_row < 0 ) || ( size_t( i_row ) > m_aData.size() )
            ||  ( i_column < 0 ) || ( i_column > m_nColumnCount )
            )
            throw IndexOutOfBoundsException( ::rtl::OUString(), *this );

        ::std::vector< Any > const & rRow( m_aData[ i_row ] );
        if ( size_t( i_column ) < rRow.size() )
            return rRow[ i_column ];

        return Any();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL DefaultGridDataModel::getRowTitle( ::sal_Int32 i_row ) throw (RuntimeException, IndexOutOfBoundsException)
    {
        ::osl::MutexGuard aGuard( GetMutex() );

        if ( ( i_row < 0 ) || ( size_t( i_row ) >= m_aRowHeaders.size() ) )
            throw IndexOutOfBoundsException( ::rtl::OUString(), *this );

        return m_aRowHeaders[ i_row ];
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DefaultGridDataModel::addRow( const ::rtl::OUString& i_title, const Sequence< Any >& i_data ) throw (RuntimeException)
    {
        ::osl::ClearableMutexGuard aGuard( GetMutex() );

        sal_Int32 const columnCount = i_data.getLength();

        // store header name
        m_aRowHeaders.push_back( i_title );

        // store row m_aData
        ::std::vector< Any > newRow( columnCount );
        ::std::copy( i_data.getConstArray(), i_data.getConstArray() + columnCount, newRow.begin() );
        m_aData.push_back( newRow );

        // update column count
        if ( columnCount > m_nColumnCount )
            m_nColumnCount = columnCount;

        broadcast(
            GridDataEvent( *this, Sequence< sal_Int32 >(), lcl_buildSingleElementSequence( m_aData.size() - 1 ) ),
            &XGridDataListener::rowsAdded,
            aGuard
        );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DefaultGridDataModel::addRows( const Sequence< ::rtl::OUString >& i_titles, const Sequence< Sequence< Any > >& i_data ) throw (IllegalArgumentException, RuntimeException)
    {
        if ( i_titles.getLength() != i_data.getLength() )
            throw IllegalArgumentException( ::rtl::OUString(), *this, -1 );

        ::osl::ClearableMutexGuard aGuard( GetMutex() );

        sal_Int32 const rowCount = i_titles.getLength();
        if ( rowCount == 0 )
            return;

        // determine max col count in the new data
        sal_Int32 maxColCount = 0;
        for ( sal_Int32 row=0; row<rowCount; ++row )
            if ( i_data[row].getLength() > maxColCount )
                maxColCount = i_data[row].getLength();

        if ( maxColCount < m_nColumnCount )
            maxColCount = m_nColumnCount;

        for ( sal_Int32 row=0; row<rowCount;  ++row )
        {
            m_aRowHeaders.push_back( i_titles[row] );

            ::std::vector< Any > newRow( maxColCount );
            Sequence< Any > const & rRowData = i_data[row];
            ::std::copy( rRowData.getConstArray(), rRowData.getConstArray() + rRowData.getLength(), newRow.begin() );
            m_aData.push_back( newRow );
        }

        if ( maxColCount > m_nColumnCount )
            m_nColumnCount = maxColCount;

        broadcast(
            GridDataEvent( *this, Sequence< sal_Int32 >(), lcl_buildIndexSequence( m_aData.size() - rowCount, m_aData.size() - 1 ) ),
            &XGridDataListener::rowsAdded,
            aGuard
        );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DefaultGridDataModel::removeRow( ::sal_Int32 i_rowIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::osl::ClearableMutexGuard aGuard( GetMutex() );

        if ( ( i_rowIndex < 0 ) || ( size_t( i_rowIndex ) >= m_aData.size() ) )
            throw IndexOutOfBoundsException( ::rtl::OUString(), *this );

        m_aRowHeaders.erase( m_aRowHeaders.begin() + i_rowIndex );
        m_aData.erase( m_aData.begin() + i_rowIndex );

        broadcast(
            GridDataEvent( *this, Sequence< sal_Int32 >(), lcl_buildSingleElementSequence( i_rowIndex ) ),
            &XGridDataListener::rowsRemoved,
            aGuard
        );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DefaultGridDataModel::removeAllRows(  ) throw (RuntimeException)
    {
        ::osl::ClearableMutexGuard aGuard( GetMutex() );

        m_aRowHeaders.clear();
        m_aData.clear();

        broadcast(
            GridDataEvent( *this, Sequence< sal_Int32 >(), Sequence< sal_Int32 >() ),
            &XGridDataListener::rowsRemoved,
            aGuard
        );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DefaultGridDataModel::updateCell( ::sal_Int32 i_rowIndex, ::sal_Int32 i_columnIndex, const Any& i_value ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::osl::ClearableMutexGuard aGuard( GetMutex() );

        if  (   ( i_rowIndex < 0 ) || ( size_t( i_rowIndex ) >= m_aData.size() )
            ||  ( i_columnIndex < 0 ) || ( i_columnIndex >= m_nColumnCount )
            )
            throw IndexOutOfBoundsException( ::rtl::OUString(), *this );

        ::std::vector< Any >& rRowData( m_aData[ i_rowIndex ] );
        if ( size_t( i_columnIndex ) >= rRowData.size() )
            rRowData.resize( i_columnIndex + 1 );
        rRowData[ i_columnIndex ] = i_value;

        broadcast(
            GridDataEvent( *this, lcl_buildSingleElementSequence( i_columnIndex ), lcl_buildSingleElementSequence( i_rowIndex ) ),
            &XGridDataListener::dataChanged,
            aGuard
        );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DefaultGridDataModel::updateRow( const Sequence< ::sal_Int32 >& i_columnIndexes, ::sal_Int32 i_rowIndex, const Sequence< Any >& i_values ) throw (IndexOutOfBoundsException, IllegalArgumentException, RuntimeException)
    {
        ::osl::ClearableMutexGuard aGuard( GetMutex() );

        if  ( ( i_rowIndex < 0 ) || ( size_t( i_rowIndex ) >= m_aData.size() ) )
            throw IndexOutOfBoundsException( ::rtl::OUString(), *this );

        if ( i_columnIndexes.getLength() != i_values.getLength() )
            throw IllegalArgumentException( ::rtl::OUString(), *this, 1 );

        sal_Int32 const columnCount = i_columnIndexes.getLength();
        if ( columnCount == 0 )
            return;

        for ( sal_Int32 col = 0; col < columnCount; ++col )
        {
            if ( ( i_columnIndexes[col] < 0 ) || ( i_columnIndexes[col] > m_nColumnCount ) )
                throw IndexOutOfBoundsException( ::rtl::OUString(), *this );
        }

        ::std::vector< Any >& rDataRow = m_aData[ i_rowIndex ];
        for ( sal_Int32 col = 0; col < columnCount; ++col )
        {
            sal_Int32 const columnIndex = i_columnIndexes[ col ];
            if ( size_t( columnIndex ) >= rDataRow.size() )
                rDataRow.resize( columnIndex + 1 );

            rDataRow[ columnIndex ] = i_values[ col ];
        }

        // by definition, the indexes in the notified sequences shall be sorted
        Sequence< sal_Int32 > columnIndexes( i_columnIndexes );
        ::std::sort( stl_begin( columnIndexes ), stl_end( columnIndexes ) );

        broadcast(
            GridDataEvent( *this, columnIndexes, lcl_buildSingleElementSequence( i_rowIndex ) ),
            &XGridDataListener::dataChanged,
            aGuard
        );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DefaultGridDataModel::setRowTitle( ::sal_Int32 i_rowIndex, const ::rtl::OUString& i_title ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::osl::ClearableMutexGuard aGuard( GetMutex() );

        if  ( ( i_rowIndex < 0 ) || ( size_t( i_rowIndex ) >= m_aData.size() ) )
            throw IndexOutOfBoundsException( ::rtl::OUString(), *this );

        m_aRowHeaders[ i_rowIndex ] = i_title;

        broadcast(
            GridDataEvent( *this, Sequence< sal_Int32 >(), lcl_buildSingleElementSequence( i_rowIndex ) ),
            &XGridDataListener::rowTitleChanged,
            aGuard
        );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DefaultGridDataModel::addGridDataListener( const Reference< grid::XGridDataListener >& i_listener ) throw (RuntimeException)
    {
        BrdcstHelper.addListener( XGridDataListener::static_type(), i_listener );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DefaultGridDataModel::removeGridDataListener( const Reference< grid::XGridDataListener >& i_listener ) throw (RuntimeException)
    {
        BrdcstHelper.removeListener( XGridDataListener::static_type(), i_listener );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DefaultGridDataModel::dispose() throw (RuntimeException)
    {
        ::com::sun::star::lang::EventObject aEvent;
        aEvent.Source.set( static_cast< ::cppu::OWeakObject* >( this ) );
        BrdcstHelper.aLC.disposeAndClear( aEvent );

    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DefaultGridDataModel::addEventListener( const Reference< XEventListener >& xListener ) throw (RuntimeException)
    {
        BrdcstHelper.addListener( XEventListener::static_type(), xListener );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DefaultGridDataModel::removeEventListener( const Reference< XEventListener >& xListener ) throw (RuntimeException)
    {
        BrdcstHelper.removeListener( XEventListener::static_type(), xListener );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL DefaultGridDataModel::getImplementationName(  ) throw (RuntimeException)
    {
        static const OUString aImplName( RTL_CONSTASCII_USTRINGPARAM( "toolkit.DefaultGridDataModel" ) );
        return aImplName;
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_Bool SAL_CALL DefaultGridDataModel::supportsService( const ::rtl::OUString& ServiceName ) throw (RuntimeException)
    {
        return ServiceName.equalsAscii( szServiceName_DefaultGridDataModel );
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL DefaultGridDataModel::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        static const OUString aServiceName( OUString::createFromAscii( szServiceName_DefaultGridDataModel ) );
        static const Sequence< OUString > aSeq( &aServiceName, 1 );
        return aSeq;
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XCloneable > SAL_CALL DefaultGridDataModel::createClone(  ) throw (RuntimeException)
    {
        return new DefaultGridDataModel( *this );
    }

//......................................................................................................................
}   // namespace toolkit
//......................................................................................................................

Reference< XInterface > SAL_CALL DefaultGridDataModel_CreateInstance( const Reference< XMultiServiceFactory >& )
{
    return Reference < XInterface >( ( ::cppu::OWeakObject* ) new ::toolkit::DefaultGridDataModel() );
}
