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
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/NullPointerException.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <cppuhelper/supportsservice.hxx>

namespace calc
{

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

    OCellListSource::OCellListSource( const Reference< XSpreadsheetDocument >& _rxDocument )
        :OCellListSource_Base( m_aMutex )
        ,OCellListSource_PBase( OCellListSource_Base::rBHelper )
        ,m_xDocument( _rxDocument )
        ,m_aListEntryListeners( m_aMutex )
        ,m_bInitialized( false )
    {
        OSL_PRECOND( m_xDocument.is(), "OCellListSource::OCellListSource: invalid document!" );

        // register our property at the base class
        registerPropertyNoMember(
            "CellRange",
            PROP_HANDLE_RANGE_ADDRESS,
            PropertyAttribute::BOUND | PropertyAttribute::READONLY,
            cppu::UnoType<CellRangeAddress>::get(),
            css::uno::Any(CellRangeAddress())
        );
    }

    OCellListSource::~OCellListSource( )
    {
        if ( !OCellListSource_Base::rBHelper.bDisposed )
        {
            acquire();  // prevent duplicate dtor
            dispose();
        }
    }

    IMPLEMENT_FORWARD_XINTERFACE2( OCellListSource, OCellListSource_Base, OCellListSource_PBase )

    IMPLEMENT_FORWARD_XTYPEPROVIDER2( OCellListSource, OCellListSource_Base, OCellListSource_PBase )

    void SAL_CALL OCellListSource::disposing()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

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

    Reference< XPropertySetInfo > SAL_CALL OCellListSource::getPropertySetInfo(  )
    {
        return createPropertySetInfo( getInfoHelper() ) ;
    }

    ::cppu::IPropertyArrayHelper& SAL_CALL OCellListSource::getInfoHelper()
    {
        return *OCellListSource_PABase::getArrayHelper();
    }

    ::cppu::IPropertyArrayHelper* OCellListSource::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties( aProps );
        return new ::cppu::OPropertyArrayHelper(aProps);
    }

    void SAL_CALL OCellListSource::getFastPropertyValue( Any& _rValue, sal_Int32 _nHandle ) const
    {
        OSL_ENSURE( _nHandle == PROP_HANDLE_RANGE_ADDRESS, "OCellListSource::getFastPropertyValue: invalid handle!" );
            // we only have this one property ....
        (void)_nHandle;     // avoid warning in product version

        _rValue <<= getRangeAddress( );
    }

    void OCellListSource::checkDisposed( ) const
    {
        if ( OCellListSource_Base::rBHelper.bInDispose || OCellListSource_Base::rBHelper.bDisposed )
            throw DisposedException();
            // TODO: is it worth having an error message here?
    }

    void OCellListSource::checkInitialized()
    {
        if ( !m_bInitialized )
            throw RuntimeException();
            // TODO: error message
    }

    OUString SAL_CALL OCellListSource::getImplementationName(  )
    {
        return OUString( "com.sun.star.comp.sheet.OCellListSource" );
    }

    sal_Bool SAL_CALL OCellListSource::supportsService( const OUString& _rServiceName )
    {
        return cppu::supportsService(this, _rServiceName);
    }

    Sequence< OUString > SAL_CALL OCellListSource::getSupportedServiceNames(  )
    {
        return {"com.sun.star.table.CellRangeListSource",
                "com.sun.star.form.binding.ListEntrySource"};
    }

    CellRangeAddress OCellListSource::getRangeAddress( ) const
    {
        OSL_PRECOND( m_xRange.is(), "OCellListSource::getRangeAddress: invalid range!" );

        CellRangeAddress aAddress;
        Reference< XCellRangeAddressable > xRangeAddress( m_xRange, UNO_QUERY );
        if ( xRangeAddress.is() )
            aAddress = xRangeAddress->getRangeAddress( );
        return aAddress;
    }

    OUString OCellListSource::getCellTextContent_noCheck( sal_Int32 _nRangeRelativeRow )
    {
        OSL_PRECOND( m_xRange.is(), "OCellListSource::getRangeAddress: invalid range!" );
        Reference< XTextRange > xCellText;
        if ( m_xRange.is() )
            xCellText.set(m_xRange->getCellByPosition( 0, _nRangeRelativeRow ), css::uno::UNO_QUERY);

        OUString sText;
        if ( xCellText.is() )
            sText = xCellText->getString();
        return sText;
    }

    sal_Int32 SAL_CALL OCellListSource::getListEntryCount(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed();
        checkInitialized();

        CellRangeAddress aAddress( getRangeAddress( ) );
        return aAddress.EndRow - aAddress.StartRow + 1;
    }

    OUString SAL_CALL OCellListSource::getListEntry( sal_Int32 _nPosition )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed();
        checkInitialized();

        if ( _nPosition >= getListEntryCount() )
            throw IndexOutOfBoundsException();

        return getCellTextContent_noCheck( _nPosition );
    }

    Sequence< OUString > SAL_CALL OCellListSource::getAllListEntries(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed();
        checkInitialized();

        Sequence< OUString > aAllEntries( getListEntryCount() );
        OUString* pAllEntries = aAllEntries.getArray();
        for ( sal_Int32 i = 0; i < aAllEntries.getLength(); ++i )
        {
            *pAllEntries++ = getCellTextContent_noCheck( i );
        }

        return aAllEntries;
    }

    void SAL_CALL OCellListSource::addListEntryListener( const Reference< XListEntryListener >& _rxListener )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed();
        checkInitialized();

        if ( !_rxListener.is() )
            throw NullPointerException();

        m_aListEntryListeners.addInterface( _rxListener );
    }

    void SAL_CALL OCellListSource::removeListEntryListener( const Reference< XListEntryListener >& _rxListener )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed();
        checkInitialized();

        if ( !_rxListener.is() )
            throw NullPointerException();

        m_aListEntryListeners.removeInterface( _rxListener );
    }

    void SAL_CALL OCellListSource::modified( const EventObject& /* aEvent */ )
    {
        notifyModified();
    }

    void OCellListSource::notifyModified()
    {
        EventObject aEvent;
        aEvent.Source.set(*this);

        ::comphelper::OInterfaceIteratorHelper2 aIter( m_aListEntryListeners );
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

    void SAL_CALL OCellListSource::disposing( const EventObject& aEvent )
    {
        Reference<XInterface> xRangeInt( m_xRange, UNO_QUERY );
        if ( xRangeInt == aEvent.Source )
        {
            // release references to range object
            m_xRange.clear();
        }
    }

    void SAL_CALL OCellListSource::initialize( const Sequence< Any >& _rArguments )
    {
        if ( m_bInitialized )
            throw Exception();
            // TODO: error message

        // get the cell address
        CellRangeAddress aRangeAddress;
        bool bFoundAddress = false;

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
                        bFoundAddress = true;
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

        m_bInitialized = true;
    }

}   // namespace calc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
