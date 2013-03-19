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

#include "celllistsource.hxx"
#include <tools/debug.hxx>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

//.........................................................................
namespace calc
{
//.........................................................................

#define PROP_HANDLE_RANGE_ADDRESS  1

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::table;
    using namespace ::com::sun::star::text;
    using namespace ::com::sun::star::sheet;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::form::binding;

    //=====================================================================
    //= OCellListSource
    //=====================================================================
    DBG_NAME( OCellListSource )
    //---------------------------------------------------------------------
#ifdef DBG_UTIL
    const char* OCellListSource::checkConsistency_static( const void* _pThis )
    {
        return static_cast< const OCellListSource* >( _pThis )->checkConsistency( );
    }

    const char* OCellListSource::checkConsistency( ) const
    {
        const char* pAssertion = NULL;

        // TODO: place any checks here to ensure consistency of this instance

        return pAssertion;
    }
#endif

    //---------------------------------------------------------------------
    OCellListSource::OCellListSource( const Reference< XSpreadsheetDocument >& _rxDocument )
        :OCellListSource_Base( m_aMutex )
        ,OCellListSource_PBase( OCellListSource_Base::rBHelper )
        ,m_xDocument( _rxDocument )
        ,m_aListEntryListeners( m_aMutex )
        ,m_bInitialized( false )
    {
        DBG_CTOR( OCellListSource, checkConsistency_static );

        OSL_PRECOND( m_xDocument.is(), "OCellListSource::OCellListSource: invalid document!" );

        // register our property at the base class
        CellRangeAddress aInitialPropValue;
        registerPropertyNoMember(
            ::rtl::OUString( "CellRange" ),
            PROP_HANDLE_RANGE_ADDRESS,
            PropertyAttribute::BOUND | PropertyAttribute::READONLY,
            ::getCppuType( &aInitialPropValue ),
            &aInitialPropValue
        );
    }

    //---------------------------------------------------------------------
    OCellListSource::~OCellListSource( )
    {
        if ( !OCellListSource_Base::rBHelper.bDisposed )
        {
            acquire();  // prevent duplicate dtor
            dispose();
        }

        DBG_DTOR( OCellListSource, checkConsistency_static );
    }

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XINTERFACE2( OCellListSource, OCellListSource_Base, OCellListSource_PBase )

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( OCellListSource, OCellListSource_Base, OCellListSource_PBase )

    //--------------------------------------------------------------------
    void SAL_CALL OCellListSource::disposing()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        DBG_CHKTHIS( OCellListSource, checkConsistency_static );

        Reference<XModifyBroadcaster> xBroadcaster( m_xRange, UNO_QUERY );
        if ( xBroadcaster.is() )
        {
            xBroadcaster->removeModifyListener( this );
        }

        EventObject aDisposeEvent( *this );
        m_aListEntryListeners.disposeAndClear( aDisposeEvent );

        WeakAggComponentImplHelperBase::disposing();

        // TODO: clean up here whatever you need to clean up (e.g. revoking listeners etc.)
    }

    //--------------------------------------------------------------------
    Reference< XPropertySetInfo > SAL_CALL OCellListSource::getPropertySetInfo(  ) throw(RuntimeException)
    {
        DBG_CHKTHIS( OCellListSource, checkConsistency_static );
        return createPropertySetInfo( getInfoHelper() ) ;
    }

    //--------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& SAL_CALL OCellListSource::getInfoHelper()
    {
        return *OCellListSource_PABase::getArrayHelper();
    }

    //--------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper* OCellListSource::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties( aProps );
        return new ::cppu::OPropertyArrayHelper(aProps);
    }

    //--------------------------------------------------------------------
    void SAL_CALL OCellListSource::getFastPropertyValue( Any& _rValue, sal_Int32 _nHandle ) const
    {
        DBG_CHKTHIS( OCellListSource, checkConsistency_static );
        OSL_ENSURE( _nHandle == PROP_HANDLE_RANGE_ADDRESS, "OCellListSource::getFastPropertyValue: invalid handle!" );
            // we only have this one property ....
        (void)_nHandle;     // avoid warning in product version

        _rValue <<= getRangeAddress( );
    }

    //--------------------------------------------------------------------
    void OCellListSource::checkDisposed( ) const SAL_THROW( ( DisposedException ) )
    {
        if ( OCellListSource_Base::rBHelper.bInDispose || OCellListSource_Base::rBHelper.bDisposed )
            throw DisposedException();
            // TODO: is it worth having an error message here?
    }

    //--------------------------------------------------------------------
    void OCellListSource::checkInitialized() SAL_THROW( ( RuntimeException ) )
    {
        if ( !m_bInitialized )
            throw RuntimeException();
            // TODO: error message
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OCellListSource::getImplementationName(  ) throw (RuntimeException)
    {
        return ::rtl::OUString( "com.sun.star.comp.sheet.OCellListSource" );
    }

    //--------------------------------------------------------------------
    sal_Bool SAL_CALL OCellListSource::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aSupportedServices( getSupportedServiceNames() );
        const ::rtl::OUString* pLookup = aSupportedServices.getConstArray();
        const ::rtl::OUString* pLookupEnd = aSupportedServices.getConstArray() + aSupportedServices.getLength();
        while ( pLookup != pLookupEnd )
            if ( *pLookup++ == _rServiceName )
                return sal_True;

        return false;
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL OCellListSource::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aServices( 2 );
        aServices[ 0 ] =  ::rtl::OUString( "com.sun.star.table.CellRangeListSource" );
        aServices[ 1 ] =  ::rtl::OUString( "com.sun.star.form.binding.ListEntrySource" );
        return aServices;
    }

    //--------------------------------------------------------------------
    CellRangeAddress OCellListSource::getRangeAddress( ) const
    {
        OSL_PRECOND( m_xRange.is(), "OCellListSource::getRangeAddress: invalid range!" );

        CellRangeAddress aAddress;
        Reference< XCellRangeAddressable > xRangeAddress( m_xRange, UNO_QUERY );
        if ( xRangeAddress.is() )
            aAddress = xRangeAddress->getRangeAddress( );
        return aAddress;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString OCellListSource::getCellTextContent_noCheck( sal_Int32 _nRangeRelativeColumn, sal_Int32 _nRangeRelativeRow )
    {
        OSL_PRECOND( m_xRange.is(), "OCellListSource::getRangeAddress: invalid range!" );
        Reference< XTextRange > xCellText;
        if ( m_xRange.is() )
            xCellText.set(xCellText.query( m_xRange->getCellByPosition( _nRangeRelativeColumn, _nRangeRelativeRow ) ));

        ::rtl::OUString sText;
        if ( xCellText.is() )
            sText = xCellText->getString();
        return sText;
    }

    //--------------------------------------------------------------------
    sal_Int32 SAL_CALL OCellListSource::getListEntryCount(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        DBG_CHKTHIS( OCellListSource, checkConsistency_static );
        checkDisposed();
        checkInitialized();

        CellRangeAddress aAddress( getRangeAddress( ) );
        return aAddress.EndRow - aAddress.StartRow + 1;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OCellListSource::getListEntry( sal_Int32 _nPosition ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        DBG_CHKTHIS( OCellListSource, checkConsistency_static );
        checkDisposed();
        checkInitialized();

        if ( _nPosition >= getListEntryCount() )
            throw IndexOutOfBoundsException();

        return getCellTextContent_noCheck( 0, _nPosition );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL OCellListSource::getAllListEntries(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        DBG_CHKTHIS( OCellListSource, checkConsistency_static );
        checkDisposed();
        checkInitialized();

        Sequence< ::rtl::OUString > aAllEntries( getListEntryCount() );
        ::rtl::OUString* pAllEntries = aAllEntries.getArray();
        for ( sal_Int32 i = 0; i < aAllEntries.getLength(); ++i )
        {
            *pAllEntries++ = getCellTextContent_noCheck( 0, i );
        }

        return aAllEntries;
    }

    //--------------------------------------------------------------------
    void SAL_CALL OCellListSource::addListEntryListener( const Reference< XListEntryListener >& _rxListener ) throw (NullPointerException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        DBG_CHKTHIS( OCellListSource, checkConsistency_static );
        checkDisposed();
        checkInitialized();

        if ( !_rxListener.is() )
            throw NullPointerException();

        m_aListEntryListeners.addInterface( _rxListener );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OCellListSource::removeListEntryListener( const Reference< XListEntryListener >& _rxListener ) throw (NullPointerException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        DBG_CHKTHIS( OCellListSource, checkConsistency_static );
        checkDisposed();
        checkInitialized();

        if ( !_rxListener.is() )
            throw NullPointerException();

        m_aListEntryListeners.removeInterface( _rxListener );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OCellListSource::modified( const EventObject& /* aEvent */ ) throw (RuntimeException)
    {
        DBG_CHKTHIS( OCellListSource, checkConsistency_static );

        notifyModified();
    }

    //--------------------------------------------------------------------
    void OCellListSource::notifyModified()
    {
        EventObject aEvent;
        aEvent.Source.set(*this);

        ::cppu::OInterfaceIteratorHelper aIter( m_aListEntryListeners );
        while ( aIter.hasMoreElements() )
        {
            try
            {
                static_cast< XListEntryListener* >( aIter.next() )->allEntriesChanged( aEvent );
            }
            catch( const RuntimeException& )
            {
                // silent this
            }
            catch( const Exception& )
            {
                OSL_FAIL( "OCellListSource::notifyModified: caught a (non-runtime) exception!" );
            }
        }

    }

    //--------------------------------------------------------------------
    void SAL_CALL OCellListSource::disposing( const EventObject& aEvent ) throw (RuntimeException)
    {
        DBG_CHKTHIS( OCellListSource, checkConsistency_static );

        Reference<XInterface> xRangeInt( m_xRange, UNO_QUERY );
        if ( xRangeInt == aEvent.Source )
        {
            // release references to range object
            m_xRange.clear();
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL OCellListSource::initialize( const Sequence< Any >& _rArguments ) throw (Exception, RuntimeException)
    {
        if ( m_bInitialized )
            throw Exception();
            // TODO: error message

        // get the cell address
        CellRangeAddress aRangeAddress;
        sal_Bool bFoundAddress = false;

        const Any* pLoop = _rArguments.getConstArray();
        const Any* pLoopEnd = _rArguments.getConstArray() + _rArguments.getLength();
        for ( ; ( pLoop != pLoopEnd ) && !bFoundAddress; ++pLoop )
        {
            NamedValue aValue;
            if ( *pLoop >>= aValue )
            {
                if ( aValue.Name == "CellRange" )
                {
                    if ( aValue.Value >>= aRangeAddress )
                        bFoundAddress = sal_True;
                }
            }
        }

        if ( !bFoundAddress )
            // TODO: error message
            throw Exception();

        // determine the range we're bound to
        try
        {
            if ( m_xDocument.is() )
            {
                // first the sheets collection
                Reference< XIndexAccess > xSheets(m_xDocument->getSheets( ), UNO_QUERY);
                OSL_ENSURE( xSheets.is(), "OCellListSource::initialize: could not retrieve the sheets!" );

                if ( xSheets.is() )
                {
                    // the concrete sheet
                    Reference< XCellRange > xSheet(xSheets->getByIndex( aRangeAddress.Sheet ), UNO_QUERY);
                    OSL_ENSURE( xSheet.is(), "OCellListSource::initialize: NULL sheet, but no exception!" );

                    // the concrete cell
                    if ( xSheet.is() )
                    {
                        m_xRange.set(xSheet->getCellRangeByPosition(
                            aRangeAddress.StartColumn, aRangeAddress.StartRow,
                            aRangeAddress.EndColumn, aRangeAddress.EndRow));
                        OSL_ENSURE( Reference< XCellRangeAddressable >( m_xRange, UNO_QUERY ).is(), "OCellListSource::initialize: either NULL range, or cell without address access!" );
                    }
                }
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "OCellListSource::initialize: caught an exception while retrieving the cell object!" );
        }


        if ( !m_xRange.is() )
            throw Exception();
            // TODO error message

        Reference<XModifyBroadcaster> xBroadcaster( m_xRange, UNO_QUERY );
        if ( xBroadcaster.is() )
        {
            xBroadcaster->addModifyListener( this );
        }

        // TODO: add as XEventListener to the cell range, so we get notified when it dies,
        // and can dispose ourself then

        // TODO: somehow add as listener so we get notified when the address of the cell range changes
        // We need to forward this as change in our CellRange property to our property change listeners

        // TODO: somehow add as listener to the cells in the range, so that we get notified
        // when their content changes. We need to forward this to our list entry listeners then

        // TODO: somehow add as listener so that we get notified of insertions and removals of rows in our
        // range. In this case, we need to fire a change in our CellRange property, and additionally
        // notify our XListEntryListeners

        m_bInitialized = sal_True;
    }

//.........................................................................
}   // namespace calc
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
