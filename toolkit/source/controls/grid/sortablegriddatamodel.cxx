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

#include "precompiled_toolkit.hxx"

#include "sortablegriddatamodel.hxx"
#include "initguard.hxx"
#include "toolkit/helper/servicenames.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/i18n/XCollator.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
/** === end UNO includes === **/

#include <comphelper/anycompare.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/svapp.hxx>

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
    /** === end UNO using === **/

    typedef InitGuard< SortableGridDataModel >  MethodGuard;

    //==================================================================================================================
    //= SortableGridDataModel
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    SortableGridDataModel::SortableGridDataModel( Reference< XMultiServiceFactory > const & i_factory )
        :SortableGridDataModel_Base( m_aMutex )
        ,m_context( i_factory )
        ,m_isInitialized( false )
        ,m_delegator()
        ,m_collator()
        ,m_currentSortColumn( -1 )
        ,m_sortAscending( true )
        ,m_rowIndexTranslation()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    SortableGridDataModel::SortableGridDataModel( SortableGridDataModel const & i_copySource )
        :cppu::BaseMutex()
        ,SortableGridDataModel_Base( m_aMutex )
        ,m_context( i_copySource.m_context )
        ,m_collator( i_copySource.m_collator )
        ,m_isInitialized( true )
        ,m_delegator()
        ,m_currentSortColumn( i_copySource.m_currentSortColumn )
        ,m_sortAscending( i_copySource.m_sortAscending )
        ,m_rowIndexTranslation( i_copySource.m_rowIndexTranslation )
    {
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

        // TODO: add as listener to the delegator model, so we're able to multiplex the events it will notify

        m_delegator = xDelegator;
        m_isInitialized = true;
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
    void SortableGridDataModel::impl_reIndex_nothrow( ::sal_Int32 const i_columnIndex, sal_Bool const i_sortAscending )
    {
        ::sal_Int32 const rowCount( getRowCount() );
        ::std::vector< ::sal_Int32 > aIndexTranslation( rowCount );

        try
        {
            // build an unsorted translation table, and retrieve the unsorted data
            ::std::vector< Any > aColumnData( rowCount );
            Type dataType;
            for ( ::sal_Int32 rowIndex = 0; rowIndex < rowCount; ++rowIndex )
            {
                aColumnData[ rowIndex ] = m_delegator->getCellData( i_columnIndex, rowIndex );
                aIndexTranslation[ rowIndex ] = rowIndex;

                // determine the data types we assume for the complete column
                if ( ( dataType.getTypeClass() == TypeClass_VOID ) && aColumnData[ rowIndex ].hasValue() )
                    dataType = aColumnData[ rowIndex ].getValueType();
            }

            // get predicate object
            ::std::auto_ptr< ::comphelper::IKeyPredicateLess > const pPredicate( ::comphelper::getStandardLessPredicate( dataType ) );
            ENSURE_OR_RETURN_VOID( pPredicate.get(), "SortableGridDataModel::impl_reIndex_nothrow: no sortable data found!" );

            // then sort
            CellDataLessComparison const aComparator( aColumnData, *pPredicate, i_sortAscending );
            ::std::sort( aIndexTranslation.begin(), aIndexTranslation.end(), aComparator );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
            return;
        }

        m_rowIndexTranslation.swap( aIndexTranslation );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::sortByColumn( ::sal_Int32 i_columnIndex, ::sal_Bool i_sortAscending ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );

        if ( ( i_columnIndex < 0 ) || ( i_columnIndex >= getColumnCount() ) )
            throw IndexOutOfBoundsException( ::rtl::OUString(), *this );

        impl_reIndex_nothrow( i_columnIndex, i_sortAscending );

        m_currentSortColumn = i_columnIndex;
        m_sortAscending = i_sortAscending;
    }

    //------------------------------------------------------------------------------------------------------------------
    Pair< ::sal_Int32, ::sal_Bool > SAL_CALL SortableGridDataModel::getCurrentSortOrder(  ) throw (RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        return Pair< ::sal_Int32, ::sal_Bool >( m_currentSortColumn, m_sortAscending );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::addRow( const Any& i_heading, const Sequence< Any >& i_data ) throw (RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        m_delegator->addRow( i_heading, i_data );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::addRows( const Sequence< Any >& i_headings, const Sequence< Sequence< Any > >& i_data ) throw (IllegalArgumentException, RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        m_delegator->addRows( i_headings, i_data );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::removeRow( ::sal_Int32 i_rowIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        ::sal_Int32 const rowIndex = impl_translateRowIndex_throw( i_rowIndex );
        m_delegator->removeRow( rowIndex );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::removeAllRows(  ) throw (RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        m_delegator->removeAllRows();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::updateCellData( ::sal_Int32 i_columnIndex, ::sal_Int32 i_rowIndex, const Any& i_value ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        ::sal_Int32 const rowIndex = impl_translateRowIndex_throw( i_rowIndex );
        m_delegator->updateCellData( i_columnIndex, rowIndex, i_value );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::updateRowData( const Sequence< ::sal_Int32 >& i_columnIndexes, ::sal_Int32 i_rowIndex, const Sequence< Any >& i_values ) throw (IndexOutOfBoundsException, IllegalArgumentException, RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        ::sal_Int32 const rowIndex = impl_translateRowIndex_throw( i_rowIndex );
        m_delegator->updateRowData( i_columnIndexes, rowIndex, i_values );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::setRowHeading( ::sal_Int32 i_rowIndex, const Any& i_heading ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        ::sal_Int32 const rowIndex = impl_translateRowIndex_throw( i_rowIndex );
        m_delegator->setRowHeading( rowIndex, i_heading );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::updateCellToolTip( ::sal_Int32 i_columnIndex, ::sal_Int32 i_rowIndex, const Any& i_value ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        ::sal_Int32 const rowIndex = impl_translateRowIndex_throw( i_rowIndex );
        m_delegator->updateCellToolTip( i_columnIndex, rowIndex, i_value );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::updateRowToolTip( ::sal_Int32 i_rowIndex, const Any& i_value ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        ::sal_Int32 const rowIndex = impl_translateRowIndex_throw( i_rowIndex );
        m_delegator->updateRowToolTip( rowIndex, i_value );
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
        return m_delegator->getRowCount();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Int32 SAL_CALL SortableGridDataModel::getColumnCount() throw (RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        return m_delegator->getColumnCount();
    }

    //------------------------------------------------------------------------------------------------------------------
    Any SAL_CALL SortableGridDataModel::getCellData( ::sal_Int32 i_columnIndex, ::sal_Int32 i_rowIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        ::sal_Int32 const rowIndex = impl_translateRowIndex_throw( i_rowIndex );
        return m_delegator->getCellData( i_columnIndex, rowIndex );
    }

    //------------------------------------------------------------------------------------------------------------------
    Any SAL_CALL SortableGridDataModel::getCellToolTip( ::sal_Int32 i_columnIndex, ::sal_Int32 i_rowIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        ::sal_Int32 const rowIndex = impl_translateRowIndex_throw( i_rowIndex );
        return m_delegator->getCellToolTip( i_columnIndex, rowIndex );
    }

    //------------------------------------------------------------------------------------------------------------------
    Any SAL_CALL SortableGridDataModel::getRowHeading( ::sal_Int32 i_rowIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
        ::sal_Int32 const rowIndex = impl_translateRowIndex_throw( i_rowIndex );
        return m_delegator->getRowHeading( rowIndex );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL SortableGridDataModel::disposing()
    {
        m_currentSortColumn = -1;

        Reference< XComponent > const delegatorComponent( m_delegator.get() );
        m_delegator.clear();
        delegatorComponent->dispose();

        Reference< XComponent > const collatorComponent( m_collator, UNO_QUERY );
        m_collator.clear();
        if ( collatorComponent.is() )
            collatorComponent->dispose();

        ::std::vector< ::sal_Int32 > aEmpty;
        m_rowIndexTranslation.swap( aEmpty );
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XCloneable > SAL_CALL SortableGridDataModel::createClone(  ) throw (RuntimeException)
    {
        MethodGuard aGuard( *this, rBHelper );
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
    ::sal_Int32 SortableGridDataModel::impl_translateRowIndex_throw( ::sal_Int32 const i_publicRowIndex ) const
    {
        if ( ( i_publicRowIndex < 0 ) || ( i_publicRowIndex >= m_delegator->getRowCount() ) )
            throw IndexOutOfBoundsException( ::rtl::OUString(), *const_cast< SortableGridDataModel* >( this ) );

        if ( !impl_isSorted_nothrow() )
            // no need to translate anything
            return i_publicRowIndex;

        ENSURE_OR_RETURN( size_t( i_publicRowIndex ) < m_rowIndexTranslation.size(),
            "SortableGridDataModel::impl_translateRowIndex_throw: inconsistency!", i_publicRowIndex );
                // obviously the translation table contains too few elements - it should have exactly |getRowCount()|
                // elements

        return m_rowIndexTranslation[ i_publicRowIndex ];
    }

//......................................................................................................................
} // namespace toolkit
//......................................................................................................................

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL SortableGridDataModel_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory )
{
    return *( new ::toolkit::SortableGridDataModel( i_factory ) );
}
