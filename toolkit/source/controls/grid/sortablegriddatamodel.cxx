/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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


#include "sortablegriddatamodel.hxx"
#include "toolkit/helper/servicenames.hxx"

#include <com/sun/star/i18n/XCollator.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/ucb/AlreadyInitializedException.hpp>

#include <comphelper/anycompare.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <tools/diagnose_ex.h>
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>

#include <set>

//......................................................................................................................
namespace toolkit
{
//......................................................................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::TypeClass;
    using ::com::sun::star::uno::TypeClass_VOID;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::lang::IndexOutOfBoundsException;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::awt::grid::XGridDataListener;
    using ::com::sun::star::beans::Pair;
    using ::com::sun::star::util::XCloneable;
    using ::com::sun::star::i18n::XCollator;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::awt::grid::GridDataEvent;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::ucb::AlreadyInitializedException;
    /** === end UNO using === **/

#ifdef DBG_UTIL
    const char* SortableGridDataModel_checkInvariants( const void* _pInstance )
    {
        return static_cast< const SortableGridDataModel* >( _pInstance )->checkInvariants();
    }

    //------------------------------------------------------------------------------------------------------------------
    const char* SortableGridDataModel::checkInvariants() const
    {
        if ( m_publicToPrivateRowIndex.size() != m_privateToPublicRowIndex.size() )
            return "inconsistent index maps";

        if ( m_delegator.is() )
        {
            if ( m_publicToPrivateRowIndex.size() != size_t( m_delegator->getRowCount() ) )
                return "wrong cached row count";
        }
        else
        {
            if ( !m_publicToPrivateRowIndex.empty() )
                return "disposed or not initialized, but having a non-empty map";
        }

        for ( size_t publicIndex=0; publicIndex<m_publicToPrivateRowIndex.size(); ++publicIndex )
        {
            ::sal_Int32 const privateIndex = m_publicToPrivateRowIndex[ publicIndex ];
            if ( ( privateIndex < 0 ) || ( size_t( privateIndex ) >= m_privateToPublicRowIndex.size() ) )
                return "invalid cached private index";

            if ( m_privateToPublicRowIndex[ privateIndex ] != sal_Int32( publicIndex ) )
                return "index map traversal not commutavive";
        }

        if ( impl_isSorted_nothrow() && m_publicToPrivateRowIndex.empty() )
            return "sorted, but no row index translation tables";

        if ( !impl_isSorted_nothrow() && !m_publicToPrivateRowIndex.empty() )
            return "unsorted, but have index translation tables";

        return NULL;
    }
#endif

#define DBG_CHECK_ME() \
    DBG_CHKTHIS( SortableGridDataModel, SortableGridDataModel_checkInvariants )

    //------------------------------------------------------------------------------------------------------------------
    namespace
    {
        template< class STLCONTAINER >
        static void lcl_clear( STLCONTAINER& i_container )
        {
            STLCONTAINER empty;
            empty.swap( i_container );
        }
    }

    //==================================================================================================================
    //= SortableGridDataModel
    //==================================================================================================================
    DBG_NAME( SortableGridDataModel )
    //------------------------------------------------------------------------------------------------------------------
    SortableGridDataModel::SortableGridDataModel( Reference< XMultiServiceFactory > const & i_factory )
        :SortableGridDataModel_Base( m_aMutex )
        ,SortableGridDataModel_PrivateBase()
        ,m_context( i_factory )
        ,m_isInitialized( false )
        ,m_delegator()
        ,m_collator()
        ,m_currentSortColumn( -1 )
        ,m_sortAscending( true )
        ,m_publicToPrivateRowIndex()
        ,m_privateToPublicRowIndex()
    {
        DBG_CTOR( SortableGridDataModel, SortableGridDataModel_checkInvariants );
    }

    //------------------------------------------------------------------------------------------------------------------
    SortableGridDataModel::SortableGridDataModel( SortableGridDataModel const & i_copySource )
        :cppu::BaseMutex()
        ,SortableGridDataModel_Base( m_aMutex )
        ,SortableGridDataModel_PrivateBase()
        ,m_context( i_copySource.m_context )
        ,m_isInitialized( true )
        ,m_delegator()
        ,m_collator( i_copySource.m_collator )
        ,m_currentSortColumn( i_copySource.m_currentSortColumn )
        ,m_sortAscending( i_copySource.m_sortAscending )
        ,m_publicToPrivateRowIndex( i_copySource.m_publicToPrivateRowIndex )
        ,m_privateToPublicRowIndex( i_copySource.m_privateToPublicRowIndex )
    {
        DBG_CTOR( SortableGridDataModel, SortableGridDataModel_checkInvariants );

        ENSURE_OR_THROW( i_copySource.m_delegator.is(),
            "not expected to be called for a disposed copy source!" );
        m_delegator.set( i_copySource.m_delegator->createClone(), UNO_QUERY_THROW );
    }

    //------------------------------------------------------------------------------------------------------------------
    SortableGridDataModel::~SortableGridDataModel()
    {
        if ( !rBHelper.bDisposed )
        {
            acquire();
            dispose();
        }

        DBG_DTOR( SortableGridDataModel, SortableGridDataModel_checkInvariants );
    }

    //------------------------------------------------------------------------------------------------------------------
    Any SAL_CALL SortableGridDataModel::queryInterface( const Type& aType ) throw (RuntimeException)
    {
        Any aReturn( SortableGridDataModel_Base::queryInterface( aType ) );
        if ( !aReturn.hasValue() )
            aReturn = SortableGridDataModel_PrivateBase::queryInterface( aType );
        return aReturn;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::acquire(  ) throw ()
    {
        SortableGridDataModel_Base::acquire();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::release(  ) throw ()
    {
        SortableGridDataModel_Base::release();
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< Type > SAL_CALL SortableGridDataModel::getTypes(  ) throw (RuntimeException)
    {
        return SortableGridDataModel_Base::getTypes();
        // don't expose the types got via SortableGridDataModel_PrivateBase - they're private, after all
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< ::sal_Int8 > SAL_CALL SortableGridDataModel::getImplementationId(  ) throw (RuntimeException)
    {
        static ::cppu::OImplementationId aId;
        return aId.getImplementationId();
    }

    //------------------------------------------------------------------------------------------------------------------
    namespace
    {
        Reference< XCollator > lcl_loadDefaultCollator_throw( ::comphelper::ComponentContext const & i_context )
        {
            Reference< XCollator > const xCollator( i_context.createComponent( "com.sun.star.i18n.Collator" ), UNO_QUERY_THROW );
            xCollator->loadDefaultCollator( Application::GetSettings().GetLocale(), 0 );
            return xCollator;
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::initialize( const Sequence< Any >& i_arguments ) throw (Exception, RuntimeException)
    {
        ::comphelper::ComponentGuard aGuard( *this, rBHelper );
        DBG_CHECK_ME();

        if ( m_delegator.is() )
            throw AlreadyInitializedException( ::rtl::OUString(), *this );

        Reference< XMutableGridDataModel > xDelegator;
        Reference< XCollator > xCollator;
        switch ( i_arguments.getLength() )
        {
        case 1: // SortableGridDataModel.create( XMutableGridDataModel )
            xDelegator.set( i_arguments[0], UNO_QUERY );
            xCollator = lcl_loadDefaultCollator_throw( m_context );
            break;

        case 2: // SortableGridDataModel.createWithCollator( XMutableGridDataModel, XCollator )
            xDelegator.set( i_arguments[0], UNO_QUERY );
            xCollator.set( i_arguments[1], UNO_QUERY );
            if ( !xCollator.is() )
                throw IllegalArgumentException( ::rtl::OUString(), *this, 2 );
            break;
        }
        if ( !xDelegator.is() )
            throw IllegalArgumentException( ::rtl::OUString(), *this, 1 );

        m_delegator = xDelegator;
        m_collator = xCollator;

        m_delegator->addGridDataListener( this );

        m_isInitialized = true;
    }

    //------------------------------------------------------------------------------------------------------------------
    GridDataEvent SortableGridDataModel::impl_createPublicEvent( GridDataEvent const & i_originalEvent ) const
    {
        GridDataEvent aEvent( i_originalEvent );
        aEvent.Source = *const_cast< SortableGridDataModel* >( this );
        aEvent.FirstRow = impl_getPublicRowIndex_nothrow( aEvent.FirstRow );
        aEvent.LastRow = impl_getPublicRowIndex_nothrow( aEvent.LastRow );
        return aEvent;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SortableGridDataModel::impl_broadcast( void ( SAL_CALL XGridDataListener::*i_listenerMethod )( const GridDataEvent & ),
            GridDataEvent const & i_publicEvent, MethodGuard& i_instanceLock )
    {
        ::cppu::OInterfaceContainerHelper* pListeners = rBHelper.getContainer( XGridDataListener::static_type() );
        if ( pListeners == NULL )
            return;

        i_instanceLock.clear();
        pListeners->notifyEach( i_listenerMethod, i_publicEvent );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::rowsInserted( const GridDataEvent& i_event ) throw (RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        DBG_CHECK_ME();

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

    //------------------------------------------------------------------------------------------------------------------
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

    //------------------------------------------------------------------------------------------------------------------
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

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::rowsRemoved( const GridDataEvent& i_event ) throw (RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        DBG_CHECK_ME();

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

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::dataChanged( const GridDataEvent& i_event ) throw (RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        DBG_CHECK_ME();

        GridDataEvent const aEvent( impl_createPublicEvent( i_event ) );
        impl_broadcast( &XGridDataListener::dataChanged, aEvent, aGuard );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::rowHeadingChanged( const GridDataEvent& i_event ) throw (RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        DBG_CHECK_ME();

        GridDataEvent const aEvent( impl_createPublicEvent( i_event ) );
        impl_broadcast( &XGridDataListener::rowHeadingChanged, aEvent, aGuard );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::disposing( const EventObject& i_event ) throw (RuntimeException)
    {
        // not interested in
        OSL_UNUSED( i_event );
    }

    //------------------------------------------------------------------------------------------------------------------
    namespace
    {
        class CellDataLessComparison : public ::std::binary_function< sal_Int32, sal_Int32, bool >
        {
        public:
            CellDataLessComparison(
                ::std::vector< Any > const & i_data,
                ::comphelper::IKeyPredicateLess& i_predicate,
                sal_Bool const i_sortAscending
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
            sal_Bool const                          m_sortAscending;
        };
    }

    //------------------------------------------------------------------------------------------------------------------
    bool SortableGridDataModel::impl_reIndex_nothrow( ::sal_Int32 const i_columnIndex, sal_Bool const i_sortAscending )
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
            ::std::auto_ptr< ::comphelper::IKeyPredicateLess > const pPredicate( ::comphelper::getStandardLessPredicate( dataType, m_collator ) );
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

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::sortByColumn( ::sal_Int32 i_columnIndex, ::sal_Bool i_sortAscending ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        DBG_CHECK_ME();

        if ( ( i_columnIndex < 0 ) || ( i_columnIndex >= getColumnCount() ) )
            throw IndexOutOfBoundsException( ::rtl::OUString(), *this );

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

    //------------------------------------------------------------------------------------------------------------------
    void SortableGridDataModel::impl_removeColumnSort_noBroadcast()
    {
        lcl_clear( m_publicToPrivateRowIndex );
        lcl_clear( m_privateToPublicRowIndex );

        m_currentSortColumn = -1;
        m_sortAscending = sal_True;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SortableGridDataModel::impl_removeColumnSort( MethodGuard& i_instanceLock )
    {
        impl_removeColumnSort_noBroadcast();
        impl_broadcast(
            &XGridDataListener::dataChanged,
            GridDataEvent( *this, -1, -1, -1, -1 ),
            i_instanceLock
        );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::removeColumnSort(  ) throw (RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        DBG_CHECK_ME();
        impl_removeColumnSort( aGuard );
    }

    //------------------------------------------------------------------------------------------------------------------
    Pair< ::sal_Int32, ::sal_Bool > SAL_CALL SortableGridDataModel::getCurrentSortOrder(  ) throw (RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        DBG_CHECK_ME();

        return Pair< ::sal_Int32, ::sal_Bool >( m_currentSortColumn, m_sortAscending );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::addRow( const Any& i_heading, const Sequence< Any >& i_data ) throw (RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        DBG_CHECK_ME();

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        delegator->addRow( i_heading, i_data );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::addRows( const Sequence< Any >& i_headings, const Sequence< Sequence< Any > >& i_data ) throw (IllegalArgumentException, RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        DBG_CHECK_ME();

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        delegator->addRows( i_headings, i_data );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::removeRow( ::sal_Int32 i_rowIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        DBG_CHECK_ME();

        ::sal_Int32 const rowIndex = impl_getPrivateRowIndex_throw( i_rowIndex );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        delegator->removeRow( rowIndex );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::removeAllRows(  ) throw (RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        DBG_CHECK_ME();

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        delegator->removeAllRows();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::updateCellData( ::sal_Int32 i_columnIndex, ::sal_Int32 i_rowIndex, const Any& i_value ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        DBG_CHECK_ME();

        ::sal_Int32 const rowIndex = impl_getPrivateRowIndex_throw( i_rowIndex );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        delegator->updateCellData( i_columnIndex, rowIndex, i_value );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::updateRowData( const Sequence< ::sal_Int32 >& i_columnIndexes, ::sal_Int32 i_rowIndex, const Sequence< Any >& i_values ) throw (IndexOutOfBoundsException, IllegalArgumentException, RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        DBG_CHECK_ME();

        ::sal_Int32 const rowIndex = impl_getPrivateRowIndex_throw( i_rowIndex );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        delegator->updateRowData( i_columnIndexes, rowIndex, i_values );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::updateRowHeading( ::sal_Int32 i_rowIndex, const Any& i_heading ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        DBG_CHECK_ME();

        ::sal_Int32 const rowIndex = impl_getPrivateRowIndex_throw( i_rowIndex );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        delegator->updateRowHeading( rowIndex, i_heading );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::updateCellToolTip( ::sal_Int32 i_columnIndex, ::sal_Int32 i_rowIndex, const Any& i_value ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        DBG_CHECK_ME();

        ::sal_Int32 const rowIndex = impl_getPrivateRowIndex_throw( i_rowIndex );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        delegator->updateCellToolTip( i_columnIndex, rowIndex, i_value );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::updateRowToolTip( ::sal_Int32 i_rowIndex, const Any& i_value ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        DBG_CHECK_ME();

        ::sal_Int32 const rowIndex = impl_getPrivateRowIndex_throw( i_rowIndex );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        delegator->updateRowToolTip( rowIndex, i_value );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::addGridDataListener( const Reference< XGridDataListener >& i_listener ) throw (RuntimeException)
    {
        rBHelper.addListener( XGridDataListener::static_type(), i_listener );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::removeGridDataListener( const Reference< XGridDataListener >& i_listener ) throw (RuntimeException)
    {
        rBHelper.removeListener( XGridDataListener::static_type(), i_listener );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Int32 SAL_CALL SortableGridDataModel::getRowCount() throw (RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        DBG_CHECK_ME();

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        return delegator->getRowCount();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Int32 SAL_CALL SortableGridDataModel::getColumnCount() throw (RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        DBG_CHECK_ME();

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        return delegator->getColumnCount();
    }

    //------------------------------------------------------------------------------------------------------------------
    Any SAL_CALL SortableGridDataModel::getCellData( ::sal_Int32 i_columnIndex, ::sal_Int32 i_rowIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        DBG_CHECK_ME();

        ::sal_Int32 const rowIndex = impl_getPrivateRowIndex_throw( i_rowIndex );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        return delegator->getCellData( i_columnIndex, rowIndex );
    }

    //------------------------------------------------------------------------------------------------------------------
    Any SAL_CALL SortableGridDataModel::getCellToolTip( ::sal_Int32 i_columnIndex, ::sal_Int32 i_rowIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        DBG_CHECK_ME();

        ::sal_Int32 const rowIndex = impl_getPrivateRowIndex_throw( i_rowIndex );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        return delegator->getCellToolTip( i_columnIndex, rowIndex );
    }

    //------------------------------------------------------------------------------------------------------------------
    Any SAL_CALL SortableGridDataModel::getRowHeading( ::sal_Int32 i_rowIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        DBG_CHECK_ME();

        ::sal_Int32 const rowIndex = impl_getPrivateRowIndex_throw( i_rowIndex );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        return delegator->getRowHeading( rowIndex );
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< Any > SAL_CALL SortableGridDataModel::getRowData( ::sal_Int32 i_rowIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        DBG_CHECK_ME();

        ::sal_Int32 const rowIndex = impl_getPrivateRowIndex_throw( i_rowIndex );

        Reference< XMutableGridDataModel > const delegator( m_delegator );
        aGuard.clear();
        return delegator->getRowData( rowIndex );
    }

    //------------------------------------------------------------------------------------------------------------------
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

    //------------------------------------------------------------------------------------------------------------------
    Reference< XCloneable > SAL_CALL SortableGridDataModel::createClone(  ) throw (RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        DBG_CHECK_ME();

        return new SortableGridDataModel( *this );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL SortableGridDataModel::getImplementationName(  ) throw (RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.comp.toolkit.SortableGridDataModel" ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Bool SAL_CALL SortableGridDataModel::supportsService( const ::rtl::OUString& i_serviceName ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > const aServiceNames( getSupportedServiceNames() );
        for ( sal_Int32 i=0; i<aServiceNames.getLength(); ++i )
            if ( aServiceNames[i] == i_serviceName )
                return sal_True;
        return sal_False;
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL SortableGridDataModel::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aServiceNames(1);
        aServiceNames[0] = ::rtl::OUString::createFromAscii( szServiceName_SortableGridDataModel );
        return aServiceNames;
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Int32 SortableGridDataModel::impl_getPrivateRowIndex_throw( ::sal_Int32 const i_publicRowIndex ) const
    {
        if ( ( i_publicRowIndex < 0 ) || ( i_publicRowIndex >= m_delegator->getRowCount() ) )
            throw IndexOutOfBoundsException( ::rtl::OUString(), *const_cast< SortableGridDataModel* >( this ) );

        if ( !impl_isSorted_nothrow() )
            // no need to translate anything
            return i_publicRowIndex;

        ENSURE_OR_RETURN( size_t( i_publicRowIndex ) < m_publicToPrivateRowIndex.size(),
            "SortableGridDataModel::impl_getPrivateRowIndex_throw: inconsistency!", i_publicRowIndex );
                // obviously the translation table contains too few elements - it should have exactly |getRowCount()|
                // elements

        return m_publicToPrivateRowIndex[ i_publicRowIndex ];
    }

    //------------------------------------------------------------------------------------------------------------------
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

//......................................................................................................................
} // namespace toolkit
//......................................................................................................................

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL SortableGridDataModel_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory )
{
    return *( new ::toolkit::SortableGridDataModel( i_factory ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
