/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "defaultgridcolumnmodel.hxx"
#include "gridcolumn.hxx"

#include <com/sun/star/awt/XVclWindowPeer.hpp>

#include <comphelper/sequence.hxx>
#include <comphelper/componentguard.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/diagnose_ex.h>

//......................................................................................................................
namespace toolkit
//......................................................................................................................
{
    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::awt::grid::XGridColumn;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::container::XContainerListener;
    using ::com::sun::star::container::ContainerEvent;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::lang::IndexOutOfBoundsException;
    using ::com::sun::star::util::XCloneable;
    using ::com::sun::star::lang::IllegalArgumentException;
    /** === end UNO using === **/

    //==================================================================================================================
    //= DefaultGridColumnModel
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    DefaultGridColumnModel::DefaultGridColumnModel( const Reference< XMultiServiceFactory >& i_factory )
        :DefaultGridColumnModel_Base( m_aMutex )
        ,m_aContext( i_factory )
        ,m_aContainerListeners( m_aMutex )
        ,m_aColumns()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    DefaultGridColumnModel::DefaultGridColumnModel( DefaultGridColumnModel const & i_copySource )
        :cppu::BaseMutex()
        ,DefaultGridColumnModel_Base( m_aMutex )
        ,m_aContext( i_copySource.m_aContext )
        ,m_aContainerListeners( m_aMutex )
        ,m_aColumns()
    {
        Columns aColumns;
        aColumns.reserve( i_copySource.m_aColumns.size() );
        try
        {
            for (   Columns::const_iterator col = i_copySource.m_aColumns.begin();
                    col != i_copySource.m_aColumns.end();
                    ++col
                )
            {
                Reference< XCloneable > const xCloneable( *col, UNO_QUERY_THROW );
                Reference< XGridColumn > const xClone( xCloneable->createClone(), UNO_QUERY_THROW );

                GridColumn* const pGridColumn = GridColumn::getImplementation( xClone );
                if ( pGridColumn == NULL )
                    throw RuntimeException( "invalid clone source implementation", *this );
                    // that's indeed a RuntimeException, not an IllegalArgumentException or some such:
                    // a DefaultGridColumnModel implementation whose columns are not GridColumn implementations
                    // is borked.
                pGridColumn->setIndex( col - i_copySource.m_aColumns.begin() );

                aColumns.push_back( xClone );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        if ( aColumns.size() == i_copySource.m_aColumns.size() )
            m_aColumns.swap( aColumns );
    }

    //------------------------------------------------------------------------------------------------------------------
    DefaultGridColumnModel::~DefaultGridColumnModel()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Int32 SAL_CALL DefaultGridColumnModel::getColumnCount() throw (RuntimeException)
    {
        return m_aColumns.size();
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XGridColumn > SAL_CALL DefaultGridColumnModel::createColumn(  ) throw (RuntimeException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );
        return new GridColumn();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Int32 SAL_CALL DefaultGridColumnModel::addColumn( const Reference< XGridColumn > & i_column ) throw (RuntimeException, IllegalArgumentException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );

        GridColumn* const pGridColumn = GridColumn::getImplementation( i_column );
        if ( pGridColumn == NULL )
            throw IllegalArgumentException( "invalid column implementation", *this, 1 );

        m_aColumns.push_back( i_column );
        sal_Int32 index = m_aColumns.size() - 1;
        pGridColumn->setIndex( index );

        // fire insertion notifications
        ContainerEvent aEvent;
        aEvent.Source = *this;
        aEvent.Accessor <<= index;
        aEvent.Element <<= i_column;

        aGuard.clear();
        m_aContainerListeners.notifyEach( &XContainerListener::elementInserted, aEvent );

        return index;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DefaultGridColumnModel::removeColumn( ::sal_Int32 i_columnIndex )  throw (RuntimeException, IndexOutOfBoundsException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );

        if ( ( i_columnIndex < 0 ) || ( size_t( i_columnIndex ) >= m_aColumns.size() ) )
            throw IndexOutOfBoundsException( ::rtl::OUString(), *this );

        Columns::iterator const pos = m_aColumns.begin() + i_columnIndex;
        Reference< XGridColumn > const xColumn( *pos );
        m_aColumns.erase( pos );

        // update indexes of all subsequent columns
        sal_Int32 columnIndex( i_columnIndex );
        for (   Columns::iterator updatePos = m_aColumns.begin() + columnIndex;
                updatePos != m_aColumns.end();
                ++updatePos, ++columnIndex
            )
        {
            GridColumn* pColumnImpl = GridColumn::getImplementation( *updatePos );
            if ( !pColumnImpl )
            {
                SAL_WARN( "toolkit.controls", "DefaultGridColumnModel::removeColumn: invalid column implementation!" );
                continue;
            }

            pColumnImpl->setIndex( columnIndex );
        }

        // fire removal notifications
        ContainerEvent aEvent;
        aEvent.Source = *this;
        aEvent.Accessor <<= i_columnIndex;
        aEvent.Element <<= xColumn;

        aGuard.clear();
        m_aContainerListeners.notifyEach( &XContainerListener::elementRemoved, aEvent );

        // dispose the removed column
        try
        {
            Reference< XComponent > const xColComp( xColumn, UNO_QUERY_THROW );
            xColComp->dispose();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< Reference< XGridColumn > > SAL_CALL DefaultGridColumnModel::getColumns() throw (RuntimeException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );
        return ::comphelper::containerToSequence( m_aColumns );
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XGridColumn > SAL_CALL DefaultGridColumnModel::getColumn(::sal_Int32 index) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );

        if ( index >=0 && index < ((sal_Int32)m_aColumns.size()))
            return m_aColumns[index];

        throw IndexOutOfBoundsException();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DefaultGridColumnModel::setDefaultColumns(sal_Int32 rowElements) throw (RuntimeException)
    {
        ::std::vector< ContainerEvent > aRemovedColumns;
        ::std::vector< ContainerEvent > aInsertedColumns;

        {
            ::comphelper::ComponentGuard aGuard( *this, rBHelper );

            // remove existing columns
            while ( !m_aColumns.empty() )
            {
                const size_t lastColIndex = m_aColumns.size() - 1;

                ContainerEvent aEvent;
                aEvent.Source = *this;
                aEvent.Accessor <<= sal_Int32( lastColIndex );
                aEvent.Element <<= m_aColumns[ lastColIndex ];
                aRemovedColumns.push_back( aEvent );

                m_aColumns.erase( m_aColumns.begin() + lastColIndex );
            }

            // add new columns
            for ( sal_Int32 i=0; i<rowElements; ++i )
            {
                ::rtl::Reference< GridColumn > const pGridColumn = new GridColumn();
                Reference< XGridColumn > const xColumn( pGridColumn.get() );
                ::rtl::OUStringBuffer colTitle;
                colTitle.appendAscii( "Column " );
                colTitle.append( i + 1 );
                pGridColumn->setTitle( colTitle.makeStringAndClear() );
                pGridColumn->setColumnWidth( 80 /* APPFONT */ );
                pGridColumn->setFlexibility( 1 );
                pGridColumn->setResizeable( sal_True );
                pGridColumn->setDataColumnIndex( i );

                ContainerEvent aEvent;
                aEvent.Source = *this;
                aEvent.Accessor <<= i;
                aEvent.Element <<= xColumn;
                aInsertedColumns.push_back( aEvent );

                m_aColumns.push_back( xColumn );
                pGridColumn->setIndex( i );
            }
        }

        // fire removal notifications
        for (   ::std::vector< ContainerEvent >::const_iterator event = aRemovedColumns.begin();
                event != aRemovedColumns.end();
                ++event
            )
        {
            m_aContainerListeners.notifyEach( &XContainerListener::elementRemoved, *event );
        }

        // fire insertion notifications
        for (   ::std::vector< ContainerEvent >::const_iterator event = aInsertedColumns.begin();
                event != aInsertedColumns.end();
                ++event
            )
        {
            m_aContainerListeners.notifyEach( &XContainerListener::elementInserted, *event );
        }

        // dispose removed columns
        for (   ::std::vector< ContainerEvent >::const_iterator event = aRemovedColumns.begin();
                event != aRemovedColumns.end();
                ++event
            )
        {
            try
            {
                const Reference< XComponent > xColComp( event->Element, UNO_QUERY_THROW );
                xColComp->dispose();
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL DefaultGridColumnModel::getImplementationName(  ) throw (RuntimeException)
    {
        return ::rtl::OUString( "org.openoffice.comp.toolkit.DefaultGridColumnModel" );
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_Bool SAL_CALL DefaultGridColumnModel::supportsService( const ::rtl::OUString& i_serviceName ) throw (RuntimeException)
    {
        const Sequence< ::rtl::OUString > aServiceNames( getSupportedServiceNames() );
        for ( sal_Int32 i=0; i<aServiceNames.getLength(); ++i )
            if ( aServiceNames[i] == i_serviceName )
                return sal_True;
        return sal_False;
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL DefaultGridColumnModel::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        const ::rtl::OUString aServiceName( ::rtl::OUString::createFromAscii( szServiceName_DefaultGridColumnModel ) );
        const Sequence< ::rtl::OUString > aSeq( &aServiceName, 1 );
        return aSeq;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DefaultGridColumnModel::addContainerListener( const Reference< XContainerListener >& i_listener ) throw (RuntimeException)
    {
        if ( i_listener.is() )
            m_aContainerListeners.addInterface( i_listener );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DefaultGridColumnModel::removeContainerListener( const Reference< XContainerListener >& i_listener ) throw (RuntimeException)
    {
        if ( i_listener.is() )
            m_aContainerListeners.removeInterface( i_listener );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DefaultGridColumnModel::disposing()
    {
        DefaultGridColumnModel_Base::disposing();

        EventObject aEvent( *this );
        m_aContainerListeners.disposeAndClear( aEvent );

        ::osl::MutexGuard aGuard( m_aMutex );

        // remove, dispose and clear columns
        while ( !m_aColumns.empty() )
        {
            try
            {
                const Reference< XComponent > xColComponent( m_aColumns[ 0 ], UNO_QUERY_THROW );
                xColComponent->dispose();
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }

            m_aColumns.erase( m_aColumns.begin() );
        }

        Columns aEmpty;
        m_aColumns.swap( aEmpty );
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XCloneable > SAL_CALL DefaultGridColumnModel::createClone(  ) throw (RuntimeException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );
        return new DefaultGridColumnModel( *this );
    }

//......................................................................................................................
}   // namespace toolkit
//......................................................................................................................

//----------------------------------------------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL DefaultGridColumnModel_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rFactory)
{
    return ::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface >( ( ::cppu::OWeakObject* ) new ::toolkit::DefaultGridColumnModel( _rFactory ) );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
