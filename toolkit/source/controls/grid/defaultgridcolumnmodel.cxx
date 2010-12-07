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

/** === begin UNO includes === **/
#include <com/sun/star/awt/XVclWindowPeer.hpp>
/** === end UNO includes === **/

#include <comphelper/sequence.hxx>
#include <toolkit/helper/servicenames.hxx>
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
        ,m_nColumnHeaderHeight(0)
    {
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
    ::sal_Int32 SAL_CALL DefaultGridColumnModel::addColumn( const Reference< XGridColumn > & i_column ) throw (RuntimeException)
    {
        ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

        m_aColumns.push_back( i_column );
        sal_Int32 index = m_aColumns.size() - 1;
        i_column->setIndex( index );

        return index;
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< Reference< XGridColumn > > SAL_CALL DefaultGridColumnModel::getColumns() throw (RuntimeException)
    {
        ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );
        return ::comphelper::containerToSequence( m_aColumns );
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XGridColumn > SAL_CALL DefaultGridColumnModel::getColumn(::sal_Int32 index) throw (RuntimeException)
    {
        ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );
        if ( index >=0 && index < ((sal_Int32)m_aColumns.size()))
        {
            return m_aColumns[index];
        }
        else
            // TODO: exception
            return Reference< XGridColumn >();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DefaultGridColumnModel::setColumnHeaderHeight(sal_Int32 _value) throw (RuntimeException)
    {
        ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );
        m_nColumnHeaderHeight = _value;
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_Int32 SAL_CALL DefaultGridColumnModel::getColumnHeaderHeight() throw (RuntimeException)
    {
        ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );
        return m_nColumnHeaderHeight;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL DefaultGridColumnModel::setDefaultColumns(sal_Int32 rowElements) throw (RuntimeException)
    {
        ::std::vector< ContainerEvent > aRemovedColumns;
        ::std::vector< ContainerEvent > aInsertedColumns;

        {
            ::osl::MutexGuard aGuard( m_aMutex );

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
                const Reference< XGridColumn > xColumn( m_aContext.createComponent( "com.sun.star.awt.grid.GridColumn" ), UNO_QUERY_THROW );

                ContainerEvent aEvent;
                aEvent.Source = *this;
                aEvent.Accessor <<= i;
                aEvent.Element <<= xColumn;
                aInsertedColumns.push_back( aEvent );

                m_aColumns.push_back( xColumn );
                xColumn->setIndex( i );
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
    Reference< XGridColumn > SAL_CALL DefaultGridColumnModel::copyColumn(const Reference< XGridColumn > & column)  throw (RuntimeException)
    {
        Reference< XGridColumn > xColumn( m_aContext.createComponent( "com.sun.star.awt.grid.GridColumn" ), UNO_QUERY_THROW );
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

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL DefaultGridColumnModel::getImplementationName(  ) throw (RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.comp.toolkit.DefaultGridColumnModel" ) );
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
        {
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
    }

//......................................................................................................................
}   // namespace toolkit
//......................................................................................................................

//----------------------------------------------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL DefaultGridColumnModel_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rFactory)
{
    return ::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface >( ( ::cppu::OWeakObject* ) new ::toolkit::DefaultGridColumnModel( _rFactory ) );
}
