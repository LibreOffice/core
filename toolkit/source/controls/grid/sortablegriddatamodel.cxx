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

#include <memory>
#include <com/sun/star/i18n/Collator.hpp>
#include <com/sun/star/i18n/XCollator.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/NotInitializedException.hpp>
#include <com/sun/star/ucb/AlreadyInitializedException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/awt/grid/XGridDataListener.hpp>
#include <com/sun/star/awt/grid/XSortableMutableGridDataModel.hpp>

#include <comphelper/compbase.hxx>
#include <comphelper/interfacecontainer4.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/anycompare.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <o3tl/safeint.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

using namespace css::awt;
using namespace css::awt::grid;
using namespace css::i18n;
using namespace css::lang;
using namespace css::ucb;
using namespace css::uno;

namespace {

class SortableGridDataModel;

typedef ::comphelper::WeakComponentImplHelper    <   css::awt::grid::XSortableMutableGridDataModel
                                            ,   css::awt::grid::XGridDataListener
                                            ,   css::lang::XServiceInfo
                                            ,   css::lang::XInitialization
                                            >   SortableGridDataModel_Base;
class SortableGridDataModel :public SortableGridDataModel_Base
{
public:
    explicit SortableGridDataModel( const css::uno::Reference< css::uno::XComponentContext > & rxContext );
    SortableGridDataModel( SortableGridDataModel const & i_copySource );

    bool    isInitialized() const { return m_isInitialized; }

protected:
    virtual ~SortableGridDataModel() override;

public:
    // XSortableGridData
    virtual void SAL_CALL sortByColumn( ::sal_Int32 ColumnIndex, sal_Bool SortAscending ) override;
    virtual void SAL_CALL removeColumnSort(  ) override;
    virtual css::beans::Pair< ::sal_Int32, sal_Bool > SAL_CALL getCurrentSortOrder(  ) override;

    // XMutableGridDataModel
    virtual void SAL_CALL addRow( const css::uno::Any& Heading, const css::uno::Sequence< css::uno::Any >& Data ) override;
    virtual void SAL_CALL addRows( const css::uno::Sequence< css::uno::Any >& Headings, const css::uno::Sequence< css::uno::Sequence< css::uno::Any > >& Data ) override;
    virtual void SAL_CALL insertRow( ::sal_Int32 i_index, const css::uno::Any& i_heading, const css::uno::Sequence< css::uno::Any >& Data ) override;
    virtual void SAL_CALL insertRows( ::sal_Int32 i_index, const css::uno::Sequence< css::uno::Any>& Headings, const css::uno::Sequence< css::uno::Sequence< css::uno::Any > >& Data ) override;
    virtual void SAL_CALL removeRow( ::sal_Int32 RowIndex ) override;
    virtual void SAL_CALL removeAllRows(  ) override;
    virtual void SAL_CALL updateCellData( ::sal_Int32 ColumnIndex, ::sal_Int32 RowIndex, const css::uno::Any& Value ) override;
    virtual void SAL_CALL updateRowData( const css::uno::Sequence< ::sal_Int32 >& ColumnIndexes, ::sal_Int32 RowIndex, const css::uno::Sequence< css::uno::Any >& Values ) override;
    virtual void SAL_CALL updateRowHeading( ::sal_Int32 RowIndex, const css::uno::Any& Heading ) override;
    virtual void SAL_CALL updateCellToolTip( ::sal_Int32 ColumnIndex, ::sal_Int32 RowIndex, const css::uno::Any& Value ) override;
    virtual void SAL_CALL updateRowToolTip( ::sal_Int32 RowIndex, const css::uno::Any& Value ) override;
    virtual void SAL_CALL addGridDataListener( const css::uno::Reference< css::awt::grid::XGridDataListener >& Listener ) override;
    virtual void SAL_CALL removeGridDataListener( const css::uno::Reference< css::awt::grid::XGridDataListener >& Listener ) override;

    // XGridDataModel
    virtual ::sal_Int32 SAL_CALL getRowCount() override;
    virtual ::sal_Int32 SAL_CALL getColumnCount() override;
    virtual css::uno::Any SAL_CALL getCellData( ::sal_Int32 Column, ::sal_Int32 RowIndex ) override;
    virtual css::uno::Any SAL_CALL getCellToolTip( ::sal_Int32 Column, ::sal_Int32 RowIndex ) override;
    virtual css::uno::Any SAL_CALL getRowHeading( ::sal_Int32 RowIndex ) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL getRowData( ::sal_Int32 RowIndex ) override;

    // OComponentHelper
    virtual void disposing(std::unique_lock<std::mutex>& rGuard) override;

    // XCloneable
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XGridDataListener
    virtual void SAL_CALL rowsInserted( const css::awt::grid::GridDataEvent& Event ) override;
    virtual void SAL_CALL rowsRemoved( const css::awt::grid::GridDataEvent& Event ) override;
    virtual void SAL_CALL dataChanged( const css::awt::grid::GridDataEvent& Event ) override;
    virtual void SAL_CALL rowHeadingChanged( const css::awt::grid::GridDataEvent& Event ) override;

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& i_event ) override;

private:
    /** translates the given public index into one to be passed to our delegator
        @throws css::lang::IndexOutOfBoundsException
            if the given index does not denote a valid row
    */
    ::sal_Int32 impl_getPrivateRowIndex_throw( std::unique_lock<std::mutex>& rGuard, ::sal_Int32 const i_publicRowIndex ) const;

    /** translates the given private row index to a public one
    */
    ::sal_Int32 impl_getPublicRowIndex_nothrow( ::sal_Int32 const i_privateRowIndex ) const;

    bool impl_isSorted_nothrow() const
    {
        return m_currentSortColumn >= 0;
    }

    /** rebuilds the index translation structure.

        Neither <member>m_currentSortColumn</member> nor <member>m_sortAscending</member> are touched by this method.
        Also, the given column index is not checked, this is the responsibility of the caller.
    */
    bool    impl_reIndex_nothrow( std::unique_lock<std::mutex>& rGuard, ::sal_Int32 const i_columnIndex, bool const i_sortAscending );

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
                std::unique_lock<std::mutex>& i_instanceLock
            );

    /** rebuilds our indexes, notifying row removal and row addition events

        First, a rowsRemoved event is notified to our registered listeners. Then, the index translation tables are
        rebuilt, and a rowsInserted event is notified.

        Only to be called when we're sorted.
    */
    void    impl_rebuildIndexesAndNotify( std::unique_lock<std::mutex>& i_instanceLock );

    /** removes the current sorting, and notifies a change of all data
    */
    void    impl_removeColumnSort( std::unique_lock<std::mutex>& i_instanceLock );

    /** removes the current sorting, without any broadcast
    */
    void    impl_removeColumnSort_noBroadcast();

    void throwIfNotInitialized()
    {
        if (!isInitialized())
            throw css::lang::NotInitializedException( OUString(), *this );
    }

private:
    css::uno::Reference< css::uno::XComponentContext >            m_xContext;
    bool                                                          m_isInitialized;
    css::uno::Reference< css::awt::grid::XMutableGridDataModel >  m_delegator;
    css::uno::Reference< css::i18n::XCollator >                   m_collator;
    ::sal_Int32                                                   m_currentSortColumn;
    bool                                                    m_sortAscending;
    ::std::vector< ::sal_Int32 >                                  m_publicToPrivateRowIndex;
    ::std::vector< ::sal_Int32 >                                  m_privateToPublicRowIndex;
    comphelper::OInterfaceContainerHelper4<XGridDataListener>            m_GridListeners;
};

template< class STLCONTAINER >
void lcl_clear( STLCONTAINER& i_container )
{
    STLCONTAINER().swap(i_container);
}

    SortableGridDataModel::SortableGridDataModel( Reference< XComponentContext > const & rxContext )
        :m_xContext( rxContext )
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
        :m_xContext( i_copySource.m_xContext )
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
        acquire();
        dispose();
    }

    Reference< XCollator > lcl_loadDefaultCollator_throw( const Reference<XComponentContext> & rxContext )
    {
        Reference< XCollator > const xCollator = Collator::create( rxContext );
        xCollator->loadDefaultCollator( Application::GetSettings().GetLanguageTag().getLocale(), 0 );
        return xCollator;
    }

    void SAL_CALL SortableGridDataModel::initialize( const Sequence< Any >& i_arguments )
    {
        std::unique_lock aGuard( m_aMutex );
        throwIfDisposed(aGuard);

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

        m_delegator = std::move(xDelegator);
        m_collator = std::move(xCollator);

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
            GridDataEvent const & i_publicEvent, std::unique_lock<std::mutex>& i_instanceLock )
    {
        m_GridListeners.notifyEach( i_instanceLock, i_listenerMethod, i_publicEvent );
    }


    void SAL_CALL SortableGridDataModel::rowsInserted( const GridDataEvent& i_event )
    {
        std::unique_lock aGuard(m_aMutex);
        throwIfNotInitialized();

        if ( impl_isSorted_nothrow() )
        {
            // no infrastructure is in place currently to sort the new row to its proper location,
            // so we remove the sorting here.
            impl_removeColumnSort( aGuard );
        }

        GridDataEvent const aEvent( impl_createPublicEvent( i_event ) );
        impl_broadcast( &XGridDataListener::rowsInserted, aEvent, aGuard );
    }

    void lcl_decrementValuesGreaterThan( ::std::vector< ::sal_Int32 > & io_indexMap, sal_Int32 const i_threshold )
    {
        for ( auto& rIndex : io_indexMap )
        {
            if ( rIndex >= i_threshold )
                --rIndex;
        }
    }

    void SortableGridDataModel::impl_rebuildIndexesAndNotify( std::unique_lock<std::mutex>& i_instanceLock )
    {
        OSL_PRECOND( impl_isSorted_nothrow(), "SortableGridDataModel::impl_rebuildIndexesAndNotify: illegal call!" );

        // clear the indexes
        lcl_clear( m_publicToPrivateRowIndex );
        lcl_clear( m_privateToPublicRowIndex );

        // rebuild the index
        if ( !impl_reIndex_nothrow( i_instanceLock, m_currentSortColumn, m_sortAscending ) )
        {
            impl_removeColumnSort( i_instanceLock );
            return;
        }

        // broadcast an artificial event, saying that all rows have been removed
        GridDataEvent const aRemovalEvent( *this, -1, -1, -1, -1 );
        impl_broadcast( &XGridDataListener::rowsRemoved, aRemovalEvent, i_instanceLock );

        // broadcast an artificial event, saying that n rows have been added
        GridDataEvent const aAdditionEvent( *this, -1, -1, 0, m_delegator->getRowCount() - 1 );
        impl_broadcast( &XGridDataListener::rowsInserted, aAdditionEvent, i_instanceLock );
    }


    void SAL_CALL SortableGridDataModel::rowsRemoved( const GridDataEvent& i_event )
    {
        std::unique_lock aGuard(m_aMutex);
        throwIfNotInitialized();

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
        else if ( o3tl::make_unsigned( i_event.FirstRow ) >= m_privateToPublicRowIndex.size() )
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


    void SAL_CALL SortableGridDataModel::dataChanged( const GridDataEvent& i_event )
    {
        std::unique_lock aGuard(m_aMutex);
        throwIfNotInitialized();

        GridDataEvent const aEvent( impl_createPublicEvent( i_event ) );
        impl_broadcast( &XGridDataListener::dataChanged, aEvent, aGuard );
    }


    void SAL_CALL SortableGridDataModel::rowHeadingChanged( const GridDataEvent& i_event )
    {
        std::unique_lock aGuard(m_aMutex);
        throwIfNotInitialized();

        GridDataEvent const aEvent( impl_createPublicEvent( i_event ) );
        impl_broadcast( &XGridDataListener::rowHeadingChanged, aEvent, aGuard );
    }


    void SAL_CALL SortableGridDataModel::disposing( const EventObject& )
    {
    }

    class CellDataLessComparison
    {
    public:
        CellDataLessComparison(
            ::std::vector< Any > const & i_data,
            ::comphelper::IKeyPredicateLess const & i_predicate,
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

    bool SortableGridDataModel::impl_reIndex_nothrow( std::unique_lock<std::mutex>& rGuard, ::sal_Int32 const i_columnIndex, bool const i_sortAscending )
    {
        Reference< XMutableGridDataModel > const delegator( m_delegator );
        rGuard.unlock();
        ::sal_Int32 const rowCount = delegator->getRowCount();
        rGuard.lock();
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
            ENSURE_OR_RETURN_FALSE(
                pPredicate, "SortableGridDataModel::impl_reIndex_nothrow: no sortable data found!");

            // then sort
            CellDataLessComparison const aComparator( aColumnData, *pPredicate, i_sortAscending );
            ::std::sort( aPublicToPrivate.begin(), aPublicToPrivate.end(), aComparator );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("toolkit.controls");
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


    void SAL_CALL SortableGridDataModel::sortByColumn( ::sal_Int32 i_columnIndex, sal_Bool i_sortAscending )
    {
        std::unique_lock aGuard(m_aMutex);
        throwIfNotInitialized();

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.unlock();
        sal_Int32 nColumnCount = delegator->getColumnCount();
        aGuard.lock();
        if ( ( i_columnIndex < 0 ) || ( i_columnIndex >= nColumnCount ) )
            throw IndexOutOfBoundsException( OUString(), *this );

        if ( !impl_reIndex_nothrow( aGuard, i_columnIndex, i_sortAscending ) )
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


    void SortableGridDataModel::impl_removeColumnSort( std::unique_lock<std::mutex>& i_instanceLock )
    {
        impl_removeColumnSort_noBroadcast();
        impl_broadcast(
            &XGridDataListener::dataChanged,
            GridDataEvent( *this, -1, -1, -1, -1 ),
            i_instanceLock
        );
    }


    void SAL_CALL SortableGridDataModel::removeColumnSort(  )
    {
        std::unique_lock aGuard(m_aMutex);
        throwIfNotInitialized();
        impl_removeColumnSort( aGuard );
    }


    css::beans::Pair< ::sal_Int32, sal_Bool > SAL_CALL SortableGridDataModel::getCurrentSortOrder(  )
    {
        std::unique_lock aGuard(m_aMutex);
        throwIfNotInitialized();

        return css::beans::Pair< ::sal_Int32, sal_Bool >( m_currentSortColumn, m_sortAscending );
    }


    void SAL_CALL SortableGridDataModel::addRow( const Any& i_heading, const Sequence< Any >& i_data )
    {
        std::unique_lock aGuard(m_aMutex);
        throwIfNotInitialized();

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.unlock();
        delegator->addRow( i_heading, i_data );
    }


    void SAL_CALL SortableGridDataModel::addRows( const Sequence< Any >& i_headings, const Sequence< Sequence< Any > >& i_data )
    {
        std::unique_lock aGuard(m_aMutex);
        throwIfNotInitialized();

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.unlock();
        delegator->addRows( i_headings, i_data );
    }


    void SAL_CALL SortableGridDataModel::insertRow( ::sal_Int32 i_index, const Any& i_heading, const Sequence< Any >& i_data )
    {
        std::unique_lock aGuard(m_aMutex);
        throwIfNotInitialized();

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.unlock();
        sal_Int32 nRowCount = delegator->getRowCount();
        aGuard.lock();
        
        ::sal_Int32 const rowIndex = i_index == nRowCount ? i_index : impl_getPrivateRowIndex_throw( aGuard, i_index );
            // note that |RowCount| is a valid index in this method, but not for impl_getPrivateRowIndex_throw

        aGuard.unlock();
        delegator->insertRow( rowIndex, i_heading, i_data );
    }


    void SAL_CALL SortableGridDataModel::insertRows( ::sal_Int32 i_index, const Sequence< Any>& i_headings, const Sequence< Sequence< Any > >& i_data )
    {
        std::unique_lock aGuard(m_aMutex);
        throwIfNotInitialized();

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.unlock();
        sal_Int32 nRowCount = delegator->getRowCount();
        aGuard.lock();

        ::sal_Int32 const rowIndex = i_index == nRowCount ? i_index : impl_getPrivateRowIndex_throw( aGuard, i_index );
            // note that |RowCount| is a valid index in this method, but not for impl_getPrivateRowIndex_throw

        aGuard.unlock();
        delegator->insertRows( rowIndex, i_headings, i_data );
    }


    void SAL_CALL SortableGridDataModel::removeRow( ::sal_Int32 i_rowIndex )
    {
        std::unique_lock aGuard(m_aMutex);
        throwIfNotInitialized();

        ::sal_Int32 const rowIndex = impl_getPrivateRowIndex_throw( aGuard, i_rowIndex );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.unlock();
        delegator->removeRow( rowIndex );
    }


    void SAL_CALL SortableGridDataModel::removeAllRows(  )
    {
        std::unique_lock aGuard(m_aMutex);
        throwIfNotInitialized();

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.unlock();
        delegator->removeAllRows();
    }


    void SAL_CALL SortableGridDataModel::updateCellData( ::sal_Int32 i_columnIndex, ::sal_Int32 i_rowIndex, const Any& i_value )
    {
        std::unique_lock aGuard(m_aMutex);
        throwIfNotInitialized();

        ::sal_Int32 const rowIndex = impl_getPrivateRowIndex_throw( aGuard, i_rowIndex );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.unlock();
        delegator->updateCellData( i_columnIndex, rowIndex, i_value );
    }


    void SAL_CALL SortableGridDataModel::updateRowData( const Sequence< ::sal_Int32 >& i_columnIndexes, ::sal_Int32 i_rowIndex, const Sequence< Any >& i_values )
    {
        std::unique_lock aGuard(m_aMutex);
        throwIfNotInitialized();

        ::sal_Int32 const rowIndex = impl_getPrivateRowIndex_throw( aGuard, i_rowIndex );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.unlock();
        delegator->updateRowData( i_columnIndexes, rowIndex, i_values );
    }


    void SAL_CALL SortableGridDataModel::updateRowHeading( ::sal_Int32 i_rowIndex, const Any& i_heading )
    {
        std::unique_lock aGuard(m_aMutex);
        throwIfNotInitialized();

        ::sal_Int32 const rowIndex = impl_getPrivateRowIndex_throw( aGuard, i_rowIndex );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.unlock();
        delegator->updateRowHeading( rowIndex, i_heading );
    }


    void SAL_CALL SortableGridDataModel::updateCellToolTip( ::sal_Int32 i_columnIndex, ::sal_Int32 i_rowIndex, const Any& i_value )
    {
        std::unique_lock aGuard(m_aMutex);
        throwIfNotInitialized();

        ::sal_Int32 const rowIndex = impl_getPrivateRowIndex_throw( aGuard, i_rowIndex );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.unlock();
        delegator->updateCellToolTip( i_columnIndex, rowIndex, i_value );
    }


    void SAL_CALL SortableGridDataModel::updateRowToolTip( ::sal_Int32 i_rowIndex, const Any& i_value )
    {
        std::unique_lock aGuard(m_aMutex);
        throwIfNotInitialized();

        ::sal_Int32 const rowIndex = impl_getPrivateRowIndex_throw( aGuard, i_rowIndex );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.unlock();
        delegator->updateRowToolTip( rowIndex, i_value );
    }


    void SAL_CALL SortableGridDataModel::addGridDataListener( const Reference< XGridDataListener >& i_listener )
    {
        std::unique_lock aGuard(m_aMutex);
        m_GridListeners.addInterface( aGuard, i_listener );
    }


    void SAL_CALL SortableGridDataModel::removeGridDataListener( const Reference< XGridDataListener >& i_listener )
    {
        std::unique_lock aGuard(m_aMutex);
        m_GridListeners.removeInterface( aGuard, i_listener );
    }


    ::sal_Int32 SAL_CALL SortableGridDataModel::getRowCount()
    {
        std::unique_lock aGuard(m_aMutex);
        throwIfNotInitialized();

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.unlock();
        return delegator->getRowCount();
    }


    ::sal_Int32 SAL_CALL SortableGridDataModel::getColumnCount()
    {
        std::unique_lock aGuard(m_aMutex);
        throwIfNotInitialized();

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.unlock();
        return delegator->getColumnCount();
    }


    Any SAL_CALL SortableGridDataModel::getCellData( ::sal_Int32 i_columnIndex, ::sal_Int32 i_rowIndex )
    {
        std::unique_lock aGuard(m_aMutex);
        throwIfNotInitialized();

        ::sal_Int32 const rowIndex = impl_getPrivateRowIndex_throw( aGuard, i_rowIndex );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.unlock();
        return delegator->getCellData( i_columnIndex, rowIndex );
    }


    Any SAL_CALL SortableGridDataModel::getCellToolTip( ::sal_Int32 i_columnIndex, ::sal_Int32 i_rowIndex )
    {
        std::unique_lock aGuard(m_aMutex);
        throwIfNotInitialized();

        ::sal_Int32 const rowIndex = impl_getPrivateRowIndex_throw( aGuard, i_rowIndex );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.unlock();
        return delegator->getCellToolTip( i_columnIndex, rowIndex );
    }


    Any SAL_CALL SortableGridDataModel::getRowHeading( ::sal_Int32 i_rowIndex )
    {
        std::unique_lock aGuard(m_aMutex);
        throwIfNotInitialized();

        ::sal_Int32 const rowIndex = impl_getPrivateRowIndex_throw( aGuard, i_rowIndex );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.unlock();
        return delegator->getRowHeading( rowIndex );
    }


    Sequence< Any > SAL_CALL SortableGridDataModel::getRowData( ::sal_Int32 i_rowIndex )
    {
        std::unique_lock aGuard(m_aMutex);
        throwIfNotInitialized();

        ::sal_Int32 const rowIndex = impl_getPrivateRowIndex_throw( aGuard, i_rowIndex );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.unlock();
        return delegator->getRowData( rowIndex );
    }


    void SortableGridDataModel::disposing(std::unique_lock<std::mutex>& /*rGuard*/)
    {
        m_currentSortColumn = -1;

        Reference< XComponent > const delegatorComponent( m_delegator );
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


    Reference< css::util::XCloneable > SAL_CALL SortableGridDataModel::createClone(  )
    {
        std::unique_lock aGuard(m_aMutex);
        throwIfNotInitialized();

        return new SortableGridDataModel( *this );
    }


    OUString SAL_CALL SortableGridDataModel::getImplementationName(  )
    {
        return u"org.openoffice.comp.toolkit.SortableGridDataModel"_ustr;
    }

    sal_Bool SAL_CALL SortableGridDataModel::supportsService( const OUString& i_serviceName )
    {
        return cppu::supportsService(this, i_serviceName);
    }

    Sequence< OUString > SAL_CALL SortableGridDataModel::getSupportedServiceNames(  )
    {
        return { u"com.sun.star.awt.grid.SortableGridDataModel"_ustr };
    }


    ::sal_Int32 SortableGridDataModel::impl_getPrivateRowIndex_throw( std::unique_lock<std::mutex>& rGuard, ::sal_Int32 const i_publicRowIndex ) const
    {
        rGuard.unlock();
        sal_Int32 nRowCount = m_delegator->getRowCount();
        rGuard.lock();
        if ( ( i_publicRowIndex < 0 ) || ( i_publicRowIndex >= nRowCount ) )
            throw IndexOutOfBoundsException( OUString(), *const_cast< SortableGridDataModel* >( this ) );

        if ( !impl_isSorted_nothrow() )
            // no need to translate anything
            return i_publicRowIndex;

        ENSURE_OR_RETURN( o3tl::make_unsigned( i_publicRowIndex ) < m_publicToPrivateRowIndex.size(),
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

        ENSURE_OR_RETURN( o3tl::make_unsigned( i_privateRowIndex ) < m_privateToPublicRowIndex.size(),
            "SortableGridDataModel::impl_getPublicRowIndex_nothrow: invalid index!", i_privateRowIndex );

        return m_privateToPublicRowIndex[ i_privateRowIndex ];
    }

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
org_openoffice_comp_toolkit_SortableGridDataModel_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SortableGridDataModel(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
