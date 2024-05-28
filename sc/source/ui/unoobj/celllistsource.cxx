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
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/NotInitializedException.hpp>
#include <com/sun/star/lang/NullPointerException.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/FormulaResult.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/diagnose_ex.hxx>

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
        :m_xDocument( _rxDocument )
        ,m_bInitialized( false )
    {
        OSL_PRECOND( m_xDocument.is(), "OCellListSource::OCellListSource: invalid document!" );

        // register our property at the base class
        registerPropertyNoMember(
            u"CellRange"_ustr,
            PROP_HANDLE_RANGE_ADDRESS,
            PropertyAttribute::BOUND | PropertyAttribute::READONLY,
            cppu::UnoType<CellRangeAddress>::get(),
            css::uno::Any(CellRangeAddress())
        );
    }

    OCellListSource::~OCellListSource( )
    {
        if ( !m_bDisposed )
        {
            acquire();  // prevent duplicate dtor
            dispose();
        }
    }

    IMPLEMENT_FORWARD_XINTERFACE2( OCellListSource, OCellListSource_Base, OCellListSource_PBase )

    IMPLEMENT_FORWARD_XTYPEPROVIDER2( OCellListSource, OCellListSource_Base, OCellListSource_PBase )

    void OCellListSource::disposing(std::unique_lock<std::mutex>& rGuard)
    {
        Reference<XModifyBroadcaster> xBroadcaster( m_xRange, UNO_QUERY );
        if ( xBroadcaster.is() )
        {
            xBroadcaster->removeModifyListener( this );
        }

        EventObject aDisposeEvent( *this );
        m_aListEntryListeners.disposeAndClear( rGuard, aDisposeEvent );

        WeakComponentImplHelperBase::disposing(rGuard);

        // TODO: clean up here whatever you need to clean up (e.g. revoking listeners etc.)
    }

    Reference< XPropertySetInfo > SAL_CALL OCellListSource::getPropertySetInfo(  )
    {
        return createPropertySetInfo( getInfoHelper() ) ;
    }

    ::cppu::IPropertyArrayHelper& OCellListSource::getInfoHelper()
    {
        return *OCellListSource_PABase::getArrayHelper();
    }

    ::cppu::IPropertyArrayHelper* OCellListSource::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties( aProps );
        return new ::cppu::OPropertyArrayHelper(aProps);
    }

    void OCellListSource::getFastPropertyValue( std::unique_lock<std::mutex>& /*rGuard*/, Any& _rValue, sal_Int32 _nHandle ) const
    {
        OSL_ENSURE( _nHandle == PROP_HANDLE_RANGE_ADDRESS, "OCellListSource::getFastPropertyValue: invalid handle!" );
            // we only have this one property...

        _rValue <<= getRangeAddress( );
    }

    void OCellListSource::checkInitialized()
    {
        if ( !m_bInitialized )
            throw NotInitializedException(u"CellListSource is not initialized"_ustr, getXWeak());
    }

    OUString SAL_CALL OCellListSource::getImplementationName(  )
    {
        return u"com.sun.star.comp.sheet.OCellListSource"_ustr;
    }

    sal_Bool SAL_CALL OCellListSource::supportsService( const OUString& _rServiceName )
    {
        return cppu::supportsService(this, _rServiceName);
    }

    Sequence< OUString > SAL_CALL OCellListSource::getSupportedServiceNames(  )
    {
        return {u"com.sun.star.table.CellRangeListSource"_ustr,
                u"com.sun.star.form.binding.ListEntrySource"_ustr};
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

    OUString OCellListSource::getCellTextContent_noCheck( std::unique_lock<std::mutex>& /*rGuard*/, sal_Int32 _nRangeRelativeRow, css::uno::Any* pAny )
    {
        OUString sText;

        OSL_PRECOND( m_xRange.is(), "OCellListSource::getRangeAddress: invalid range!" );

        if (!m_xRange.is())
            return sText;

        Reference< XCell > xCell( m_xRange->getCellByPosition( 0, _nRangeRelativeRow ));
        if (!xCell.is())
        {
            if (pAny)
                *pAny <<= sText;
            return sText;
        }

        Reference< XTextRange > xCellText;
        xCellText.set( xCell, UNO_QUERY);

        if (xCellText.is())
            sText = xCellText->getString();     // formatted output string

        if (pAny)
        {
            switch (xCell->getType())
            {
                case CellContentType_VALUE:
                    *pAny <<= xCell->getValue();
                break;
                case CellContentType_TEXT:
                    *pAny <<= sText;
                break;
                case CellContentType_FORMULA:
                    if (xCell->getError())
                        *pAny <<= sText;    // Err:... or #...!
                    else
                    {
                        Reference< XPropertySet > xProp( xCell, UNO_QUERY);
                        if (xProp.is())
                        {
                            sal_Int32 nResultType;
                            if ((xProp->getPropertyValue(u"FormulaResultType2"_ustr) >>= nResultType) &&
                                    nResultType == FormulaResult::VALUE)
                                *pAny <<= xCell->getValue();
                            else
                                *pAny <<= sText;
                        }
                    }
                break;
                case CellContentType_EMPTY:
                    *pAny <<= OUString();
                break;
                default:
                    ;   // nothing, if actually occurred it would result in #N/A being displayed if selected
            }
        }

        return sText;
    }

    sal_Int32 SAL_CALL OCellListSource::getListEntryCount(  )
    {
        std::unique_lock<std::mutex> aGuard( m_aMutex );
        throwIfDisposed(aGuard);
        checkInitialized();
        return getListEntryCount(aGuard);
    }

    sal_Int32 OCellListSource::getListEntryCount( std::unique_lock<std::mutex>& /*rGuard*/ )
    {
        CellRangeAddress aAddress( getRangeAddress( ) );
        return aAddress.EndRow - aAddress.StartRow + 1;
    }

    OUString SAL_CALL OCellListSource::getListEntry( sal_Int32 _nPosition )
    {
        std::unique_lock<std::mutex> aGuard( m_aMutex );
        throwIfDisposed(aGuard);
        checkInitialized();

        if ( _nPosition >= getListEntryCount() )
            throw IndexOutOfBoundsException();

        return getCellTextContent_noCheck( aGuard, _nPosition, nullptr );
    }

    Sequence< OUString > SAL_CALL OCellListSource::getAllListEntries(  )
    {
        std::unique_lock<std::mutex> aGuard( m_aMutex );
        throwIfDisposed(aGuard);
        checkInitialized();

        Sequence< OUString > aAllEntries( getListEntryCount(aGuard) );
        OUString* pAllEntries = aAllEntries.getArray();
        for ( sal_Int32 i = 0; i < aAllEntries.getLength(); ++i )
        {
            *pAllEntries++ = getCellTextContent_noCheck( aGuard, i, nullptr );
        }

        return aAllEntries;
    }

    Sequence< OUString > SAL_CALL OCellListSource::getAllListEntriesTyped( Sequence< Any >& rDataValues )
    {
        std::unique_lock<std::mutex> aGuard( m_aMutex );
        throwIfDisposed(aGuard);
        checkInitialized();

        const sal_Int32 nCount = getListEntryCount(aGuard);
        Sequence< OUString > aAllEntries( nCount );
        rDataValues = Sequence< Any >( nCount );
        OUString* pAllEntries = aAllEntries.getArray();
        Any* pDataValues = rDataValues.getArray();
        for ( sal_Int32 i = 0; i < nCount; ++i )
        {
            *pAllEntries++ = getCellTextContent_noCheck( aGuard, i, pDataValues++ );
        }

        return aAllEntries;
    }

    void SAL_CALL OCellListSource::addListEntryListener( const Reference< XListEntryListener >& _rxListener )
    {
        std::unique_lock<std::mutex> aGuard( m_aMutex );
        throwIfDisposed(aGuard);
        checkInitialized();

        if ( !_rxListener.is() )
            throw NullPointerException();

        m_aListEntryListeners.addInterface( aGuard, _rxListener );
    }

    void SAL_CALL OCellListSource::removeListEntryListener( const Reference< XListEntryListener >& _rxListener )
    {
        std::unique_lock<std::mutex> aGuard( m_aMutex );
        throwIfDisposed(aGuard);
        checkInitialized();

        if ( !_rxListener.is() )
            throw NullPointerException();

        m_aListEntryListeners.removeInterface( aGuard, _rxListener );
    }

    void SAL_CALL OCellListSource::modified( const EventObject& /* aEvent */ )
    {
        notifyModified();
    }

    void OCellListSource::notifyModified()
    {
        std::unique_lock<std::mutex> aGuard( m_aMutex );
        EventObject aEvent;
        aEvent.Source.set(*this);

        m_aListEntryListeners.forEach(aGuard,
            [&aEvent] (const css::uno::Reference<css::form::binding::XListEntryListener>& l)
            {
                try
                {
                    l->allEntriesChanged( aEvent );
                }
                catch( const RuntimeException& )
                {
                    // silent this
                }
                catch( const Exception& )
                {
                    TOOLS_WARN_EXCEPTION( "sc", "OCellListSource::notifyModified: caught a (non-runtime) exception!" );
                }
            });
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
            throw RuntimeException(u"CellListSource is already initialized"_ustr, getXWeak());

        // get the cell address
        CellRangeAddress aRangeAddress;
        bool bFoundAddress = false;

        for ( const Any& rArg : _rArguments )
        {
            NamedValue aValue;
            if ( rArg >>= aValue )
            {
                if ( aValue.Name == "CellRange" )
                {
                    if ( aValue.Value >>= aRangeAddress )
                    {
                        bFoundAddress = true;
                        break;
                    }
                }
            }
        }

        if ( !bFoundAddress )
            throw RuntimeException(u"Cell not found"_ustr, getXWeak());

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
            TOOLS_WARN_EXCEPTION( "sc", "OCellListSource::initialize: caught an exception while retrieving the cell object!" );
        }

        if ( !m_xRange.is() )
            throw RuntimeException(u"Failed to retrieve cell range"_ustr, getXWeak());

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
