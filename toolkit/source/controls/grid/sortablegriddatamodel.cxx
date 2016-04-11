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

#include "initguard.hxx"

#include <com/sun/star/i18n/Collator.hpp>
#include <com/sun/star/i18n/XCollator.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ucb/AlreadyInitializedException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/awt/grid/XGridDataListener.hpp>
#include <com/sun/star/awt/grid/XSortableMutableGridDataModel.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/anycompare.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

using namespace css::awt;
using namespace css::awt::grid;
using namespace css::i18n;
using namespace css::lang;
using namespace css::ucb;
using namespace css::uno;
using namespace toolkit;

namespace {

class SortableGridDataModel;
typedef InitGuard< SortableGridDataModel >  MethodGuard;

typedef ::cppu::WeakComponentImplHelper    <   css::awt::grid::XSortableMutableGridDataModel
                                            ,   css::lang::XServiceInfo
                                            ,   css::lang::XInitialization
                                            >   SortableGridDataModel_Base;
typedef ::cppu::ImplHelper  <   css::awt::grid::XGridDataListener
                            >   SortableGridDataModel_PrivateBase;
class SortableGridDataModel :public ::cppu::BaseMutex
                            ,public SortableGridDataModel_Base
                            ,public SortableGridDataModel_PrivateBase
{
public:
    explicit SortableGridDataModel( const css::uno::Reference< css::uno::XComponentContext > & rxContext );
    SortableGridDataModel( SortableGridDataModel const & i_copySource );

    bool    isInitialized() const { return m_isInitialized; }

protected:
    virtual ~SortableGridDataModel() override;

public:
    // XSortableGridData
    virtual void SAL_CALL sortByColumn( ::sal_Int32 ColumnIndex, sal_Bool SortAscending ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeColumnSort(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::beans::Pair< ::sal_Int32, sal_Bool > SAL_CALL getCurrentSortOrder(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XMutableGridDataModel
    virtual void SAL_CALL addRow( const css::uno::Any& Heading, const css::uno::Sequence< css::uno::Any >& Data ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addRows( const css::uno::Sequence< css::uno::Any >& Headings, const css::uno::Sequence< css::uno::Sequence< css::uno::Any > >& Data ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL insertRow( ::sal_Int32 i_index, const css::uno::Any& i_heading, const css::uno::Sequence< css::uno::Any >& Data ) throw (css::uno::RuntimeException, css::lang::IndexOutOfBoundsException, std::exception) override;
    virtual void SAL_CALL insertRows( ::sal_Int32 i_index, const css::uno::Sequence< css::uno::Any>& Headings, const css::uno::Sequence< css::uno::Sequence< css::uno::Any > >& Data ) throw (css::lang::IllegalArgumentException, css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeRow( ::sal_Int32 RowIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeAllRows(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL updateCellData( ::sal_Int32 ColumnIndex, ::sal_Int32 RowIndex, const css::uno::Any& Value ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL updateRowData( const css::uno::Sequence< ::sal_Int32 >& ColumnIndexes, ::sal_Int32 RowIndex, const css::uno::Sequence< css::uno::Any >& Values ) throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL updateRowHeading( ::sal_Int32 RowIndex, const css::uno::Any& Heading ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL updateCellToolTip( ::sal_Int32 ColumnIndex, ::sal_Int32 RowIndex, const css::uno::Any& Value ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL updateRowToolTip( ::sal_Int32 RowIndex, const css::uno::Any& Value ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addGridDataListener( const css::uno::Reference< css::awt::grid::XGridDataListener >& Listener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeGridDataListener( const css::uno::Reference< css::awt::grid::XGridDataListener >& Listener ) throw (css::uno::RuntimeException, std::exception) override;

    // XGridDataModel
    virtual ::sal_Int32 SAL_CALL getRowCount() throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getColumnCount() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getCellData( ::sal_Int32 Column, ::sal_Int32 RowIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getCellToolTip( ::sal_Int32 Column, ::sal_Int32 RowIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getRowHeading( ::sal_Int32 RowIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL getRowData( ::sal_Int32 RowIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

    // OComponentHelper
    virtual void SAL_CALL disposing() override;

    // XCloneable
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    // XGridDataListener
    virtual void SAL_CALL rowsInserted( const css::awt::grid::GridDataEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL rowsRemoved( const css::awt::grid::GridDataEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL dataChanged( const css::awt::grid::GridDataEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL rowHeadingChanged( const css::awt::grid::GridDataEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& i_event ) throw (css::uno::RuntimeException, std::exception) override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire(  ) throw () final override;
    virtual void SAL_CALL release(  ) throw () override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL getImplementationId(  ) throw (css::uno::RuntimeException, std::exception) override;

private:
    /** translates the given public index into one to be passed to our delegator
        @throws css::lang::IndexOutOfBoundsException
            if the given index does not denote a valid row
    */
    ::sal_Int32 impl_getPrivateRowIndex_throw( ::sal_Int32 const i_publicRowIndex ) const;

    /** translates the given private row index to a public one
    */
    ::sal_Int32 impl_getPublicRowIndex_nothrow( ::sal_Int32 const i_privateRowIndex ) const;

    inline bool impl_isSorted_nothrow() const
    {
        return m_currentSortColumn >= 0;
    }

    /** rebuilds the index translation structure.

        Neither <member>m_currentSortColumn</member> nor <member>m_sortAscending</member> are touched by this method.
        Also, the given column index is not checked, this is the responsibility of the caller.
    */
    bool    impl_reIndex_nothrow( ::sal_Int32 const i_columnIndex, bool const i_sortAscending );

    /** translates the given event, obtained from our delegator, to a version which can be broadcasted to our own
        clients.
    */
    css::awt::grid::GridDataEvent
            impl_createPublicEvent( css::awt::grid::GridDataEvent const & i_originalEvent ) const;

    /** broadcasts the given event to our registered XGridDataListeners
    */
    void    impl_broadcast(
                void ( SAL_CALL css::awt::grid::XGridDataListener::*i_listenerMethod )( const css::awt::grid::GridDataEvent & ),
                css::awt::grid::GridDataEvent const & i_publicEvent,
                MethodGuard& i_instanceLock
            );

    /** rebuilds our indexes, notifying row removal and row addition events

        First, a rowsRemoved event is notified to our registered listeners. Then, the index translation tables are
        rebuilt, and a rowsInserted event is notified.

        Only to be called when we're sorted.
    */
    void    impl_rebuildIndexesAndNotify( MethodGuard& i_instanceLock );

    /** removes the current sorting, and notifies a change of all data
    */
    void    impl_removeColumnSort( MethodGuard& i_instanceLock );

    /** removes the current sorting, without any broadcast
    */
    void    impl_removeColumnSort_noBroadcast();

private:
    css::uno::Reference< css::uno::XComponentContext >            m_xContext;
    bool                                                          m_isInitialized;
    css::uno::Reference< css::awt::grid::XMutableGridDataModel >  m_delegator;
    css::uno::Reference< css::i18n::XCollator >                   m_collator;
    ::sal_Int32                                                   m_currentSortColumn;
    bool                                                    m_sortAscending;
    ::std::vector< ::sal_Int32 >                                  m_publicToPrivateRowIndex;
    ::std::vector< ::sal_Int32 >                                  m_privateToPublicRowIndex;
};

    namespace
    {
        template< class STLCONTAINER >
        void lcl_clear( STLCONTAINER& i_container )
        {
            STLCONTAINER empty;
            empty.swap( i_container );
        }
    }

    SortableGridDataModel::SortableGridDataModel( Reference< XComponentContext > const & rxContext )
        :SortableGridDataModel_Base( m_aMutex )
        ,SortableGridDataModel_PrivateBase()
        ,m_xContext( rxContext )
        ,m_isInitialized( false )
        ,m_delegator()
        ,m_collator()
        ,m_currentSortColumn( -1 )
        ,m_sortAscending( true )
        ,m_publicToPrivateRowIndex()
        ,m_privateToPublicRowIndex()
    {
    }


    SortableGridDataModel::SortableGridDataModel( SortableGridDataModel const & i_copySource )
        :cppu::BaseMutex()
        ,SortableGridDataModel_Base( m_aMutex )
        ,SortableGridDataModel_PrivateBase()
        ,m_xContext( i_copySource.m_xContext )
        ,m_isInitialized( true )
        ,m_delegator()
        ,m_collator( i_copySource.m_collator )
        ,m_currentSortColumn( i_copySource.m_currentSortColumn )
        ,m_sortAscending( i_copySource.m_sortAscending )
        ,m_publicToPrivateRowIndex( i_copySource.m_publicToPrivateRowIndex )
        ,m_privateToPublicRowIndex( i_copySource.m_privateToPublicRowIndex )
    {
        ENSURE_OR_THROW( i_copySource.m_delegator.is(),
            "not expected to be called for a disposed copy source!" );
        m_delegator.set( i_copySource.m_delegator->createClone(), UNO_QUERY_THROW );
    }


    SortableGridDataModel::~SortableGridDataModel()
    {
        if ( !rBHelper.bDisposed )
        {
            acquire();
            dispose();
        }
    }


    Any SAL_CALL SortableGridDataModel::queryInterface( const Type& aType ) throw (RuntimeException, std::exception)
    {
        Any aReturn( SortableGridDataModel_Base::queryInterface( aType ) );
        if ( !aReturn.hasValue() )
            aReturn = SortableGridDataModel_PrivateBase::queryInterface( aType );
        return aReturn;
    }


    void SAL_CALL SortableGridDataModel::acquire(  ) throw ()
    {
        SortableGridDataModel_Base::acquire();
    }


    void SAL_CALL SortableGridDataModel::release(  ) throw ()
    {
        SortableGridDataModel_Base::release();
    }


    Sequence< Type > SAL_CALL SortableGridDataModel::getTypes(  ) throw (RuntimeException, std::exception)
    {
        return SortableGridDataModel_Base::getTypes();
        // don't expose the types got via SortableGridDataModel_PrivateBase - they're private, after all
    }


    Sequence< ::sal_Int8 > SAL_CALL SortableGridDataModel::getImplementationId(  ) throw (RuntimeException, std::exception)
    {
        return css::uno::Sequence<sal_Int8>();
    }


    namespace
    {
        Reference< XCollator > lcl_loadDefaultCollator_throw( const Reference<XComponentContext> & rxContext )
        {
            Reference< XCollator > const xCollator = Collator::create( rxContext );
            xCollator->loadDefaultCollator( Application::GetSettings().GetLanguageTag().getLocale(), 0 );
            return xCollator;
        }
    }


    void SAL_CALL SortableGridDataModel::initialize( const Sequence< Any >& i_arguments ) throw (Exception, RuntimeException, std::exception)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );

        if ( m_delegator.is() )
            throw AlreadyInitializedException( OUString(), *this );

        Reference< XMutableGridDataModel > xDelegator;
        Reference< XCollator > xCollator;
        switch ( i_arguments.getLength() )
        {
        case 1: // SortableGridDataModel.create( XMutableGridDataModel )
            xDelegator.set( i_arguments[0], UNO_QUERY );
            xCollator = lcl_loadDefaultCollator_throw( m_xContext );
            break;

        case 2: // SortableGridDataModel.createWithCollator( XMutableGridDataModel, XCollator )
            xDelegator.set( i_arguments[0], UNO_QUERY );
            xCollator.set( i_arguments[1], UNO_QUERY );
            if ( !xCollator.is() )
                throw IllegalArgumentException( OUString(), *this, 2 );
            break;
        }
        if ( !xDelegator.is() )
            throw IllegalArgumentException( OUString(), *this, 1 );

        m_delegator = xDelegator;
        m_collator = xCollator;

        m_delegator->addGridDataListener( this );

        m_isInitialized = true;
    }


    GridDataEvent SortableGridDataModel::impl_createPublicEvent( GridDataEvent const & i_originalEvent ) const
    {
        GridDataEvent aEvent( i_originalEvent );
        aEvent.Source = *const_cast< SortableGridDataModel* >( this );
        aEvent.FirstRow = impl_getPublicRowIndex_nothrow( aEvent.FirstRow );
        aEvent.LastRow = impl_getPublicRowIndex_nothrow( aEvent.LastRow );
        return aEvent;
    }


    void SortableGridDataModel::impl_broadcast( void ( SAL_CALL XGridDataListener::*i_listenerMethod )( const GridDataEvent & ),
            GridDataEvent const & i_publicEvent, MethodGuard& i_instanceLock )
    {
        ::cppu::OInterfaceContainerHelper* pListeners = rBHelper.getContainer( cppu::UnoType<XGridDataListener>::get() );
        if ( pListeners == nullptr )
            return;

        i_instanceLock.clear();
        pListeners->notifyEach( i_listenerMethod, i_publicEvent );
    }


    void SAL_CALL SortableGridDataModel::rowsInserted( const GridDataEvent& i_event ) throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this, rBHelper );

        if ( impl_isSorted_nothrow() )
        {
            // no infrastructure is in place currently to sort the new row to its proper location,
            // so we remove the sorting here.
            impl_removeColumnSort( aGuard );
            aGuard.reset();
        }

        GridDataEvent const aEvent( impl_createPublicEvent( i_event ) );
        impl_broadcast( &XGridDataListener::rowsInserted, aEvent, aGuard );
    }


    namespace
    {
        void lcl_decrementValuesGreaterThan( ::std::vector< ::sal_Int32 > & io_indexMap, sal_Int32 const i_threshold )
        {
            for (   ::std::vector< ::sal_Int32 >::iterator loop = io_indexMap.begin();
                    loop != io_indexMap.end();
                    ++loop
                )
            {
                if ( *loop >= i_threshold )
                    --*loop;
            }
        }
    }


    void SortableGridDataModel::impl_rebuildIndexesAndNotify( MethodGuard& i_instanceLock )
    {
        OSL_PRECOND( impl_isSorted_nothrow(), "SortableGridDataModel::impl_rebuildIndexesAndNotify: illegal call!" );

        // clear the indexes
        lcl_clear( m_publicToPrivateRowIndex );
        lcl_clear( m_privateToPublicRowIndex );

        // rebuild the index
        if ( !impl_reIndex_nothrow( m_currentSortColumn, m_sortAscending ) )
        {
            impl_removeColumnSort( i_instanceLock );
            return;
        }

        // broadcast an artificial event, saying that all rows have been removed
        GridDataEvent const aRemovalEvent( *this, -1, -1, -1, -1 );
        impl_broadcast( &XGridDataListener::rowsRemoved, aRemovalEvent, i_instanceLock );
        i_instanceLock.reset();

        // broadcast an artificial event, saying that n rows have been added
        GridDataEvent const aAdditionEvent( *this, -1, -1, 0, m_delegator->getRowCount() - 1 );
        impl_broadcast( &XGridDataListener::rowsInserted, aAdditionEvent, i_instanceLock );
    }


    void SAL_CALL SortableGridDataModel::rowsRemoved( const GridDataEvent& i_event ) throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this, rBHelper );

        // if the data is not sorted, broadcast the event unchanged
        if ( !impl_isSorted_nothrow() )
        {
            GridDataEvent const aEvent( impl_createPublicEvent( i_event ) );
            impl_broadcast( &XGridDataListener::rowsRemoved, aEvent, aGuard );
            return;
        }

        // if all rows have been removed, also simply multiplex to own listeners
        if ( i_event.FirstRow < 0 )
        {
            lcl_clear( m_publicToPrivateRowIndex );
            lcl_clear( m_privateToPublicRowIndex );
            GridDataEvent aEvent( i_event );
            aEvent.Source = *this;
            impl_broadcast( &XGridDataListener::rowsRemoved, aEvent, aGuard );
            return;
        }

        bool needReIndex = false;
        if ( i_event.FirstRow != i_event.LastRow )
        {
            OSL_ENSURE( false, "SortableGridDataModel::rowsRemoved: missing implementation - removal of multiple rows!" );
            needReIndex = true;
        }
        else if ( size_t( i_event.FirstRow ) >= m_privateToPublicRowIndex.size() )
        {
            OSL_ENSURE( false, "SortableGridDataModel::rowsRemoved: inconsistent/wrong data!" );
            needReIndex = true;
        }

        if ( needReIndex )
        {
            impl_rebuildIndexesAndNotify( aGuard );
            return;
        }

        // build public event version
        GridDataEvent const aEvent( impl_createPublicEvent( i_event ) );

        // remove the entries from the index maps
        sal_Int32 const privateIndex = i_event.FirstRow;
        sal_Int32 const publicIndex = aEvent.FirstRow;

        m_publicToPrivateRowIndex.erase( m_publicToPrivateRowIndex.begin() + publicIndex );
        m_privateToPublicRowIndex.erase( m_privateToPublicRowIndex.begin() + privateIndex );

        // adjust remaining entries in the index maps
        lcl_decrementValuesGreaterThan( m_publicToPrivateRowIndex, privateIndex );
        lcl_decrementValuesGreaterThan( m_privateToPublicRowIndex, publicIndex );

        // broadcast the event
        impl_broadcast( &XGridDataListener::rowsRemoved, aEvent, aGuard );
    }


    void SAL_CALL SortableGridDataModel::dataChanged( const GridDataEvent& i_event ) throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this, rBHelper );

        GridDataEvent const aEvent( impl_createPublicEvent( i_event ) );
        impl_broadcast( &XGridDataListener::dataChanged, aEvent, aGuard );
    }


    void SAL_CALL SortableGridDataModel::rowHeadingChanged( const GridDataEvent& i_event ) throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this, rBHelper );

        GridDataEvent const aEvent( impl_createPublicEvent( i_event ) );
        impl_broadcast( &XGridDataListener::rowHeadingChanged, aEvent, aGuard );
    }


    void SAL_CALL SortableGridDataModel::disposing( const EventObject& ) throw (RuntimeException, std::exception)
    {
    }


    namespace
    {
        class CellDataLessComparison : public ::std::binary_function< sal_Int32, sal_Int32, bool >
        {
        public:
            CellDataLessComparison(
                ::std::vector< Any > const & i_data,
                ::comphelper::IKeyPredicateLess& i_predicate,
                bool const i_sortAscending
            )
                :m_data( i_data )
                ,m_predicate( i_predicate )
                ,m_sortAscending( i_sortAscending )
            {
            }

            bool operator()( sal_Int32 const i_lhs, sal_Int32 const i_rhs ) const
            {
                Any const & lhs = m_data[ i_lhs ];
                Any const & rhs = m_data[ i_rhs ];
                // <VOID/> is less than everything else
                if ( !lhs.hasValue() )
                    return m_sortAscending;
                if ( !rhs.hasValue() )
                    return !m_sortAscending;

                // actually compare
                if ( m_sortAscending )
                    return m_predicate.isLess( lhs, rhs );
                else
                    return m_predicate.isLess( rhs, lhs );
            }

        private:
            ::std::vector< Any > const &            m_data;
            ::comphelper::IKeyPredicateLess const & m_predicate;
            bool const                          m_sortAscending;
        };
    }


    bool SortableGridDataModel::impl_reIndex_nothrow( ::sal_Int32 const i_columnIndex, bool const i_sortAscending )
    {
        ::sal_Int32 const rowCount( getRowCount() );
        ::std::vector< ::sal_Int32 > aPublicToPrivate( rowCount );

        try
        {
            // build an unsorted translation table, and retrieve the unsorted data
            ::std::vector< Any > aColumnData( rowCount );
            Type dataType;
            for ( ::sal_Int32 rowIndex = 0; rowIndex < rowCount; ++rowIndex )
            {
                aColumnData[ rowIndex ] = m_delegator->getCellData( i_columnIndex, rowIndex );
                aPublicToPrivate[ rowIndex ] = rowIndex;

                // determine the data types we assume for the complete column
                if ( ( dataType.getTypeClass() == TypeClass_VOID ) && aColumnData[ rowIndex ].hasValue() )
                    dataType = aColumnData[ rowIndex ].getValueType();
            }

            // get predicate object
            ::std::unique_ptr< ::comphelper::IKeyPredicateLess > const pPredicate( ::comphelper::getStandardLessPredicate( dataType, m_collator ) );
            ENSURE_OR_RETURN_FALSE( pPredicate.get(), "SortableGridDataModel::impl_reIndex_nothrow: no sortable data found!" );

            // then sort
            CellDataLessComparison const aComparator( aColumnData, *pPredicate, i_sortAscending );
            ::std::sort( aPublicToPrivate.begin(), aPublicToPrivate.end(), aComparator );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
            return false;
        }

        // also build the "private to public" mapping
        ::std::vector< sal_Int32 > aPrivateToPublic( aPublicToPrivate.size() );
        for ( size_t i=0; i<aPublicToPrivate.size(); ++i )
            aPrivateToPublic[ aPublicToPrivate[i] ] = i;

        m_publicToPrivateRowIndex.swap( aPublicToPrivate );
        m_privateToPublicRowIndex.swap( aPrivateToPublic );

        return true;
    }


    void SAL_CALL SortableGridDataModel::sortByColumn( ::sal_Int32 i_columnIndex, sal_Bool i_sortAscending ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this, rBHelper );

        if ( ( i_columnIndex < 0 ) || ( i_columnIndex >= getColumnCount() ) )
            throw IndexOutOfBoundsException( OUString(), *this );

        if ( !impl_reIndex_nothrow( i_columnIndex, i_sortAscending ) )
            return;

        m_currentSortColumn = i_columnIndex;
        m_sortAscending = i_sortAscending;

        impl_broadcast(
            &XGridDataListener::dataChanged,
            GridDataEvent( *this, -1, -1, -1, -1 ),
            aGuard
        );
    }


    void SortableGridDataModel::impl_removeColumnSort_noBroadcast()
    {
        lcl_clear( m_publicToPrivateRowIndex );
        lcl_clear( m_privateToPublicRowIndex );

        m_currentSortColumn = -1;
        m_sortAscending = true;
    }


    void SortableGridDataModel::impl_removeColumnSort( MethodGuard& i_instanceLock )
    {
        impl_removeColumnSort_noBroadcast();
        impl_broadcast(
            &XGridDataListener::dataChanged,
            GridDataEvent( *this, -1, -1, -1, -1 ),
            i_instanceLock
        );
    }


    void SAL_CALL SortableGridDataModel::removeColumnSort(  ) throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this, rBHelper );
        impl_removeColumnSort( aGuard );
    }


    css::beans::Pair< ::sal_Int32, sal_Bool > SAL_CALL SortableGridDataModel::getCurrentSortOrder(  ) throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this, rBHelper );

        return css::beans::Pair< ::sal_Int32, sal_Bool >( m_currentSortColumn, m_sortAscending );
    }


    void SAL_CALL SortableGridDataModel::addRow( const Any& i_heading, const Sequence< Any >& i_data ) throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this, rBHelper );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        delegator->addRow( i_heading, i_data );
    }


    void SAL_CALL SortableGridDataModel::addRows( const Sequence< Any >& i_headings, const Sequence< Sequence< Any > >& i_data ) throw (IllegalArgumentException, RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this, rBHelper );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        delegator->addRows( i_headings, i_data );
    }


    void SAL_CALL SortableGridDataModel::insertRow( ::sal_Int32 i_index, const Any& i_heading, const Sequence< Any >& i_data ) throw (RuntimeException, IndexOutOfBoundsException, std::exception)
    {
        MethodGuard aGuard( *this, rBHelper );

        ::sal_Int32 const rowIndex = i_index == getRowCount() ? i_index : impl_getPrivateRowIndex_throw( i_index );
            // note that |RowCount| is a valid index in this method, but not for impl_getPrivateRowIndex_throw

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        delegator->insertRow( rowIndex, i_heading, i_data );
    }


    void SAL_CALL SortableGridDataModel::insertRows( ::sal_Int32 i_index, const Sequence< Any>& i_headings, const Sequence< Sequence< Any > >& i_data ) throw (IllegalArgumentException, IndexOutOfBoundsException, RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this, rBHelper );

        ::sal_Int32 const rowIndex = i_index == getRowCount() ? i_index : impl_getPrivateRowIndex_throw( i_index );
            // note that |RowCount| is a valid index in this method, but not for impl_getPrivateRowIndex_throw

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        delegator->insertRows( rowIndex, i_headings, i_data );
    }


    void SAL_CALL SortableGridDataModel::removeRow( ::sal_Int32 i_rowIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this, rBHelper );

        ::sal_Int32 const rowIndex = impl_getPrivateRowIndex_throw( i_rowIndex );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        delegator->removeRow( rowIndex );
    }


    void SAL_CALL SortableGridDataModel::removeAllRows(  ) throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this, rBHelper );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        delegator->removeAllRows();
    }


    void SAL_CALL SortableGridDataModel::updateCellData( ::sal_Int32 i_columnIndex, ::sal_Int32 i_rowIndex, const Any& i_value ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this, rBHelper );

        ::sal_Int32 const rowIndex = impl_getPrivateRowIndex_throw( i_rowIndex );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        delegator->updateCellData( i_columnIndex, rowIndex, i_value );
    }


    void SAL_CALL SortableGridDataModel::updateRowData( const Sequence< ::sal_Int32 >& i_columnIndexes, ::sal_Int32 i_rowIndex, const Sequence< Any >& i_values ) throw (IndexOutOfBoundsException, IllegalArgumentException, RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this, rBHelper );

        ::sal_Int32 const rowIndex = impl_getPrivateRowIndex_throw( i_rowIndex );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        delegator->updateRowData( i_columnIndexes, rowIndex, i_values );
    }


    void SAL_CALL SortableGridDataModel::updateRowHeading( ::sal_Int32 i_rowIndex, const Any& i_heading ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this, rBHelper );

        ::sal_Int32 const rowIndex = impl_getPrivateRowIndex_throw( i_rowIndex );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        delegator->updateRowHeading( rowIndex, i_heading );
    }


    void SAL_CALL SortableGridDataModel::updateCellToolTip( ::sal_Int32 i_columnIndex, ::sal_Int32 i_rowIndex, const Any& i_value ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this, rBHelper );

        ::sal_Int32 const rowIndex = impl_getPrivateRowIndex_throw( i_rowIndex );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        delegator->updateCellToolTip( i_columnIndex, rowIndex, i_value );
    }


    void SAL_CALL SortableGridDataModel::updateRowToolTip( ::sal_Int32 i_rowIndex, const Any& i_value ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this, rBHelper );

        ::sal_Int32 const rowIndex = impl_getPrivateRowIndex_throw( i_rowIndex );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        delegator->updateRowToolTip( rowIndex, i_value );
    }


    void SAL_CALL SortableGridDataModel::addGridDataListener( const Reference< XGridDataListener >& i_listener ) throw (RuntimeException, std::exception)
    {
        rBHelper.addListener( cppu::UnoType<XGridDataListener>::get(), i_listener );
    }


    void SAL_CALL SortableGridDataModel::removeGridDataListener( const Reference< XGridDataListener >& i_listener ) throw (RuntimeException, std::exception)
    {
        rBHelper.removeListener( cppu::UnoType<XGridDataListener>::get(), i_listener );
    }


    ::sal_Int32 SAL_CALL SortableGridDataModel::getRowCount() throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this, rBHelper );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        return delegator->getRowCount();
    }


    ::sal_Int32 SAL_CALL SortableGridDataModel::getColumnCount() throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this, rBHelper );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        return delegator->getColumnCount();
    }


    Any SAL_CALL SortableGridDataModel::getCellData( ::sal_Int32 i_columnIndex, ::sal_Int32 i_rowIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this, rBHelper );

        ::sal_Int32 const rowIndex = impl_getPrivateRowIndex_throw( i_rowIndex );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        return delegator->getCellData( i_columnIndex, rowIndex );
    }


    Any SAL_CALL SortableGridDataModel::getCellToolTip( ::sal_Int32 i_columnIndex, ::sal_Int32 i_rowIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this, rBHelper );

        ::sal_Int32 const rowIndex = impl_getPrivateRowIndex_throw( i_rowIndex );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        return delegator->getCellToolTip( i_columnIndex, rowIndex );
    }


    Any SAL_CALL SortableGridDataModel::getRowHeading( ::sal_Int32 i_rowIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this, rBHelper );

        ::sal_Int32 const rowIndex = impl_getPrivateRowIndex_throw( i_rowIndex );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        return delegator->getRowHeading( rowIndex );
    }


    Sequence< Any > SAL_CALL SortableGridDataModel::getRowData( ::sal_Int32 i_rowIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this, rBHelper );

        ::sal_Int32 const rowIndex = impl_getPrivateRowIndex_throw( i_rowIndex );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        return delegator->getRowData( rowIndex );
    }


    void SAL_CALL SortableGridDataModel::disposing()
    {
        m_currentSortColumn = -1;

        Reference< XComponent > const delegatorComponent( m_delegator.get() );
        m_delegator->removeGridDataListener( this );
        m_delegator.clear();
        delegatorComponent->dispose();

        Reference< XComponent > const collatorComponent( m_collator, UNO_QUERY );
        m_collator.clear();
        if ( collatorComponent.is() )
            collatorComponent->dispose();

        lcl_clear( m_publicToPrivateRowIndex );
        lcl_clear( m_privateToPublicRowIndex );
    }


    Reference< css::util::XCloneable > SAL_CALL SortableGridDataModel::createClone(  ) throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *this, rBHelper );

        return new SortableGridDataModel( *this );
    }


    OUString SAL_CALL SortableGridDataModel::getImplementationName(  ) throw (RuntimeException, std::exception)
    {
        return OUString( "org.openoffice.comp.toolkit.SortableGridDataModel" );
    }

    sal_Bool SAL_CALL SortableGridDataModel::supportsService( const OUString& i_serviceName ) throw (RuntimeException, std::exception)
    {
        return cppu::supportsService(this, i_serviceName);
    }

    Sequence< OUString > SAL_CALL SortableGridDataModel::getSupportedServiceNames(  ) throw (RuntimeException, std::exception)
    {
        Sequence< OUString > aServiceNames { "com.sun.star.awt.grid.SortableGridDataModel" };
        return aServiceNames;
    }


    ::sal_Int32 SortableGridDataModel::impl_getPrivateRowIndex_throw( ::sal_Int32 const i_publicRowIndex ) const
    {
        if ( ( i_publicRowIndex < 0 ) || ( i_publicRowIndex >= m_delegator->getRowCount() ) )
            throw IndexOutOfBoundsException( OUString(), *const_cast< SortableGridDataModel* >( this ) );

        if ( !impl_isSorted_nothrow() )
            // no need to translate anything
            return i_publicRowIndex;

        ENSURE_OR_RETURN( size_t( i_publicRowIndex ) < m_publicToPrivateRowIndex.size(),
            "SortableGridDataModel::impl_getPrivateRowIndex_throw: inconsistency!", i_publicRowIndex );
                // obviously the translation table contains too few elements - it should have exactly |getRowCount()|
                // elements

        return m_publicToPrivateRowIndex[ i_publicRowIndex ];
    }


    ::sal_Int32 SortableGridDataModel::impl_getPublicRowIndex_nothrow( ::sal_Int32 const i_privateRowIndex ) const
    {
        if ( !impl_isSorted_nothrow() )
            // no need to translate anything
            return i_privateRowIndex;

        if ( i_privateRowIndex < 0 )
            return i_privateRowIndex;

        ENSURE_OR_RETURN( size_t( i_privateRowIndex ) < m_privateToPublicRowIndex.size(),
            "SortableGridDataModel::impl_getPublicRowIndex_nothrow: invalid index!", i_privateRowIndex );

        return m_privateToPublicRowIndex[ i_privateRowIndex ];
    }

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
org_openoffice_comp_toolkit_SortableGridDataModel_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SortableGridDataModel(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
