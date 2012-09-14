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


#include "defaultgriddatamodel.hxx"

#include <comphelper/stlunosequence.hxx>
#include <comphelper/componentguard.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <tools/diagnose_ex.h>
#include <rtl/ref.hxx>

#include <algorithm>
#include <o3tl/compat_functional.hxx>

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

    using ::comphelper::stl_begin;
    using ::comphelper::stl_end;

    //==================================================================================================================
    //= DefaultGridDataModel
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    DefaultGridDataModel::DefaultGridDataModel()
        :DefaultGridDataModel_Base( m_aMutex )
        ,m_aRowHeaders()
        ,m_nColumnCount(0)
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    DefaultGridDataModel::DefaultGridDataModel( DefaultGridDataModel const & i_copySource )
        :cppu::BaseMutex()
        ,DefaultGridDataModel_Base( m_aMutex )
        ,m_aData( i_copySource.m_aData )
        ,m_aRowHeaders( i_copySource.m_aRowHeaders )
        ,m_nColumnCount( i_copySource.m_nColumnCount )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    DefaultGridDataModel::~DefaultGridDataModel()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    void DefaultGridDataModel::broadcast( GridDataEvent const & i_event,
        void ( SAL_CALL XGridDataListener::*i_listenerMethod )( GridDataEvent const & ), ::comphelper::ComponentGuard & i_instanceLock )
    {
        ::cppu::OInterfaceContainerHelper* pListeners = rBHelper.getContainer( XGridDataListener::static_type() );
        if ( !pListeners )
            return;

        i_instanceLock.clear();
        pListeners->notifyEach( i_listenerMethod, i_event );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Int32 SAL_CALL DefaultGridDataModel::getRowCount() throw (::com::sun::star::uno::RuntimeException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );
        return m_aData.size();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Int32 SAL_CALL DefaultGridDataModel::getColumnCount() throw (::com::sun::star::uno::RuntimeException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );
        return m_nColumnCount;
    }

    //------------------------------------------------------------------------------------------------------------------
    DefaultGridDataModel::CellData const & DefaultGridDataModel::impl_getCellData_throw( sal_Int32 const i_column, sal_Int32 const i_row ) const
    {
        if  (   ( i_row < 0 ) || ( size_t( i_row ) > m_aData.size() )
            ||  ( i_column < 0 ) || ( i_column > m_nColumnCount )
            )
            throw IndexOutOfBoundsException( ::rtl::OUString(), *const_cast< DefaultGridDataModel* >( this ) );

        RowData const & rRow( m_aData[ i_row ] );
        if ( size_t( i_column ) < rRow.size() )
            return rRow[ i_column ];

        static CellData s_aEmpty;
        return s_aEmpty;
    }

    //------------------------------------------------------------------------------------------------------------------
    DefaultGridDataModel::RowData& DefaultGridDataModel::impl_getRowDataAccess_throw( sal_Int32 const i_rowIndex, size_t const i_requiredColumnCount )
    {
        OSL_ENSURE( i_requiredColumnCount <= size_t( m_nColumnCount ), "DefaultGridDataModel::impl_getRowDataAccess_throw: invalid column count!" );
        if  ( ( i_rowIndex < 0 ) || ( size_t( i_rowIndex ) >= m_aData.size() ) )
            throw IndexOutOfBoundsException( ::rtl::OUString(), *this );

        RowData& rRowData( m_aData[ i_rowIndex ] );
        if ( rRowData.size() < i_requiredColumnCount )
            rRowData.resize( i_requiredColumnCount );
        return rRowData;
    }

    //------------------------------------------------------------------------------------------------------------------
    DefaultGridDataModel::CellData& DefaultGridDataModel::impl_getCellDataAccess_throw( sal_Int32 const i_columnIndex, sal_Int32 const i_rowIndex )
    {
        if  ( ( i_columnIndex < 0 ) || ( i_columnIndex >= m_nColumnCount ) )
            throw IndexOutOfBoundsException( ::rtl::OUString(), *this );

        RowData& rRowData( impl_getRowDataAccess_throw( i_rowIndex, size_t( i_columnIndex + 1 ) ) );
        return rRowData[ i_columnIndex ];
    }

    //------------------------------------------------------------------------------------------------------------------
    Any SAL_CALL DefaultGridDataModel::getCellData( ::sal_Int32 i_column, ::sal_Int32 i_row ) throw (RuntimeException, IndexOutOfBoundsException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );
        return impl_getCellData_throw( i_column, i_row ).first;
    }

    //------------------------------------------------------------------------------------------------------------------
    Any SAL_CALL DefaultGridDataModel::getCellToolTip( ::sal_Int32 i_column, ::sal_Int32 i_row ) throw (RuntimeException, IndexOutOfBoundsException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );
        return impl_getCellData_throw( i_column, i_row ).second;
    }

    //------------------------------------------------------------------------------------------------------------------
    Any SAL_CALL DefaultGridDataModel::getRowHeading( ::sal_Int32 i_row ) throw (RuntimeException, IndexOutOfBoundsException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );

        if ( ( i_row < 0 ) || ( size_t( i_row ) >= m_aRowHeaders.size() ) )
            throw IndexOutOfBoundsException( ::rtl::OUString(), *this );

        return m_aRowHeaders[ i_row ];
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< Any > SAL_CALL DefaultGridDataModel::getRowData( ::sal_Int32 i_rowIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );

        Sequence< Any > resultData( m_nColumnCount );
        RowData& rRowData = impl_getRowDataAccess_throw( i_rowIndex, m_nColumnCount );

        ::std::transform( rRowData.begin(), rRowData.end(), resultData.getArray(), ::o3tl::select1st< CellData >() );
        return resultData;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DefaultGridDataModel::addRow( const Any& i_heading, const Sequence< Any >& i_data ) throw (RuntimeException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );

        sal_Int32 const columnCount = i_data.getLength();

        // store header name
        m_aRowHeaders.push_back( i_heading );

        // store row m_aData
        impl_addRow( i_data );

        // update column count
        if ( columnCount > m_nColumnCount )
            m_nColumnCount = columnCount;

        sal_Int32 const rowIndex = sal_Int32( m_aData.size() - 1 );
        broadcast(
            GridDataEvent( *this, -1, -1, rowIndex, rowIndex ),
            &XGridDataListener::rowsInserted,
            aGuard
        );
    }

    //------------------------------------------------------------------------------------------------------------------
    void DefaultGridDataModel::impl_addRow( Sequence< Any > const & i_rowData, sal_Int32 const i_assumedColCount )
    {
        OSL_PRECOND( ( i_assumedColCount <= 0 ) || ( i_assumedColCount >= i_rowData.getLength() ),
            "DefaultGridDataModel::impl_addRow: invalid column count!" );

        RowData newRow( i_assumedColCount > 0 ? i_assumedColCount : i_rowData.getLength() );
        RowData::iterator cellData = newRow.begin();
        for ( const Any* pData = stl_begin( i_rowData ); pData != stl_end( i_rowData ); ++pData, ++cellData )
            cellData->first = *pData;

        m_aData.push_back( newRow );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DefaultGridDataModel::addRows( const Sequence< Any >& i_headings, const Sequence< Sequence< Any > >& i_data ) throw (IllegalArgumentException, RuntimeException)
    {
        if ( i_headings.getLength() != i_data.getLength() )
            throw IllegalArgumentException( ::rtl::OUString(), *this, -1 );

        ::comphelper::ComponentGuard aGuard( *this, rBHelper );

        sal_Int32 const rowCount = i_headings.getLength();
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
            m_aRowHeaders.push_back( i_headings[row] );
            impl_addRow( i_data[row], maxColCount );
        }

        if ( maxColCount > m_nColumnCount )
            m_nColumnCount = maxColCount;

        sal_Int32 const firstRow = sal_Int32( m_aData.size() - rowCount );
        sal_Int32 const lastRow = sal_Int32( m_aData.size() - 1 );
        broadcast(
            GridDataEvent( *this, -1, -1, firstRow, lastRow ),
            &XGridDataListener::rowsInserted,
            aGuard
        );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DefaultGridDataModel::removeRow( ::sal_Int32 i_rowIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );

        if ( ( i_rowIndex < 0 ) || ( size_t( i_rowIndex ) >= m_aData.size() ) )
            throw IndexOutOfBoundsException( ::rtl::OUString(), *this );

        m_aRowHeaders.erase( m_aRowHeaders.begin() + i_rowIndex );
        m_aData.erase( m_aData.begin() + i_rowIndex );

        broadcast(
            GridDataEvent( *this, -1, -1, i_rowIndex, i_rowIndex ),
            &XGridDataListener::rowsRemoved,
            aGuard
        );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DefaultGridDataModel::removeAllRows(  ) throw (RuntimeException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );

        m_aRowHeaders.clear();
        m_aData.clear();

        broadcast(
            GridDataEvent( *this, -1, -1, -1, -1 ),
            &XGridDataListener::rowsRemoved,
            aGuard
        );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DefaultGridDataModel::updateCellData( ::sal_Int32 i_columnIndex, ::sal_Int32 i_rowIndex, const Any& i_value ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );

        impl_getCellDataAccess_throw( i_columnIndex, i_rowIndex ).first = i_value;

        broadcast(
            GridDataEvent( *this, i_columnIndex, i_columnIndex, i_rowIndex, i_rowIndex ),
            &XGridDataListener::dataChanged,
            aGuard
        );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DefaultGridDataModel::updateRowData( const Sequence< ::sal_Int32 >& i_columnIndexes, ::sal_Int32 i_rowIndex, const Sequence< Any >& i_values ) throw (IndexOutOfBoundsException, IllegalArgumentException, RuntimeException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );

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

        RowData& rDataRow = m_aData[ i_rowIndex ];
        for ( sal_Int32 col = 0; col < columnCount; ++col )
        {
            sal_Int32 const columnIndex = i_columnIndexes[ col ];
            if ( size_t( columnIndex ) >= rDataRow.size() )
                rDataRow.resize( columnIndex + 1 );

            rDataRow[ columnIndex ].first = i_values[ col ];
        }

        sal_Int32 const firstAffectedColumn = *::std::min_element( stl_begin( i_columnIndexes ), stl_end( i_columnIndexes ) );
        sal_Int32 const lastAffectedColumn = *::std::max_element( stl_begin( i_columnIndexes ), stl_end( i_columnIndexes ) );
        broadcast(
            GridDataEvent( *this, firstAffectedColumn, lastAffectedColumn, i_rowIndex, i_rowIndex ),
            &XGridDataListener::dataChanged,
            aGuard
        );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DefaultGridDataModel::updateRowHeading( ::sal_Int32 i_rowIndex, const Any& i_heading ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );

        if  ( ( i_rowIndex < 0 ) || ( size_t( i_rowIndex ) >= m_aRowHeaders.size() ) )
            throw IndexOutOfBoundsException( ::rtl::OUString(), *this );

        m_aRowHeaders[ i_rowIndex ] = i_heading;

        broadcast(
            GridDataEvent( *this, -1, -1, i_rowIndex, i_rowIndex ),
            &XGridDataListener::rowHeadingChanged,
            aGuard
        );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DefaultGridDataModel::updateCellToolTip( ::sal_Int32 i_columnIndex, ::sal_Int32 i_rowIndex, const Any& i_value ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );
        impl_getCellDataAccess_throw( i_columnIndex, i_rowIndex ).second = i_value;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DefaultGridDataModel::updateRowToolTip( ::sal_Int32 i_rowIndex, const Any& i_value ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );

        RowData& rRowData = impl_getRowDataAccess_throw( i_rowIndex, m_nColumnCount );
        for ( RowData::iterator cell = rRowData.begin(); cell != rRowData.end(); ++cell )
            cell->second = i_value;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DefaultGridDataModel::addGridDataListener( const Reference< grid::XGridDataListener >& i_listener ) throw (RuntimeException)
    {
        rBHelper.addListener( XGridDataListener::static_type(), i_listener );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DefaultGridDataModel::removeGridDataListener( const Reference< grid::XGridDataListener >& i_listener ) throw (RuntimeException)
    {
        rBHelper.removeListener( XGridDataListener::static_type(), i_listener );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DefaultGridDataModel::disposing()
    {
        ::com::sun::star::lang::EventObject aEvent;
        aEvent.Source.set( *this );
        rBHelper.aLC.disposeAndClear( aEvent );

        ::osl::MutexGuard aGuard( m_aMutex );
        GridData aEmptyData;
        m_aData.swap( aEmptyData );

        ::std::vector< Any > aEmptyRowHeaders;
        m_aRowHeaders.swap( aEmptyRowHeaders );

        m_nColumnCount = 0;
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL DefaultGridDataModel::getImplementationName(  ) throw (RuntimeException)
    {
        static const ::rtl::OUString aImplName( RTL_CONSTASCII_USTRINGPARAM( "toolkit.DefaultGridDataModel" ) );
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
        static const ::rtl::OUString aServiceName( ::rtl::OUString::createFromAscii( szServiceName_DefaultGridDataModel ) );
        static const Sequence< ::rtl::OUString > aSeq( &aServiceName, 1 );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
