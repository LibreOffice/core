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

#include "cellvaluebinding.hxx"
#include <tools/debug.hxx>
#include <rtl/math.hxx>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/sheet/XCellAddressable.hpp>
#include <com/sun/star/sheet/XCellRangeData.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <cppuhelper/supportsservice.hxx>

namespace calc
{

#define PROP_HANDLE_BOUND_CELL  1

    namespace lang = ::com::sun::star::lang;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::table;
    using namespace ::com::sun::star::text;
    using namespace ::com::sun::star::sheet;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::form::binding;

    OCellValueBinding::OCellValueBinding( const Reference< XSpreadsheetDocument >& _rxDocument, bool _bListPos )
        :OCellValueBinding_Base( m_aMutex )
        ,OCellValueBinding_PBase( OCellValueBinding_Base::rBHelper )
        ,m_xDocument( _rxDocument )
        ,m_aModifyListeners( m_aMutex )
        ,m_bInitialized( false )
        ,m_bListPos( _bListPos )
    {
        // register our property at the base class
        CellAddress aInitialPropValue;
        registerPropertyNoMember(
            "BoundCell",
            PROP_HANDLE_BOUND_CELL,
            PropertyAttribute::BOUND | PropertyAttribute::READONLY,
            cppu::UnoType<decltype(aInitialPropValue)>::get(),
            &aInitialPropValue
        );

        // TODO: implement a ReadOnly property as required by the service,
        // which probably maps to the cell being locked
    }

    OCellValueBinding::~OCellValueBinding( )
    {
        if ( !OCellValueBinding_Base::rBHelper.bDisposed )
        {
            acquire();  // prevent duplicate dtor
            dispose();
        }
    }

    IMPLEMENT_FORWARD_XINTERFACE2( OCellValueBinding, OCellValueBinding_Base, OCellValueBinding_PBase )

    IMPLEMENT_FORWARD_XTYPEPROVIDER2( OCellValueBinding, OCellValueBinding_Base, OCellValueBinding_PBase )

    void SAL_CALL OCellValueBinding::disposing()
    {
        Reference<XModifyBroadcaster> xBroadcaster( m_xCell, UNO_QUERY );
        if ( xBroadcaster.is() )
        {
            xBroadcaster->removeModifyListener( this );
        }

        WeakAggComponentImplHelperBase::disposing();

        // TODO: clean up here whatever you need to clean up (e.g. deregister as XEventListener
        // for the cell)
    }

    Reference< XPropertySetInfo > SAL_CALL OCellValueBinding::getPropertySetInfo(  ) throw(RuntimeException, std::exception)
    {
        return createPropertySetInfo( getInfoHelper() ) ;
    }

    ::cppu::IPropertyArrayHelper& SAL_CALL OCellValueBinding::getInfoHelper()
    {
        return *OCellValueBinding_PABase::getArrayHelper();
    }

    ::cppu::IPropertyArrayHelper* OCellValueBinding::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties( aProps );
        return new ::cppu::OPropertyArrayHelper(aProps);
    }

    void SAL_CALL OCellValueBinding::getFastPropertyValue( Any& _rValue, sal_Int32 _nHandle ) const
    {
        OSL_ENSURE( _nHandle == PROP_HANDLE_BOUND_CELL, "OCellValueBinding::getFastPropertyValue: invalid handle!" );
            // we only have this one property ....
        (void)_nHandle;     // avoid warning in product version

        _rValue.clear();
        Reference< XCellAddressable > xCellAddress( m_xCell, UNO_QUERY );
        if ( xCellAddress.is() )
            _rValue <<= xCellAddress->getCellAddress( );
    }

    Sequence< Type > SAL_CALL OCellValueBinding::getSupportedValueTypes(  ) throw (RuntimeException, std::exception)
    {
        checkDisposed( );
        checkInitialized( );

        sal_Int32 nCount = m_xCellText.is() ? 3 : m_xCell.is() ? 1 : 0;
        if ( m_bListPos )
            ++nCount;

        Sequence< Type > aTypes( nCount );
        if ( m_xCell.is() )
        {
            // an XCell can be used to set/get "double" values
            aTypes[0] = ::cppu::UnoType<double>::get();
            if ( m_xCellText.is() )
            {
                // an XTextRange can be used to set/get "string" values
                aTypes[1] = ::cppu::UnoType<OUString>::get();
                // and additionally, we use it to handle booleans
                aTypes[2] = ::cppu::UnoType<sal_Bool>::get();
            }

            // add sal_Int32 only if constructed as ListPositionCellBinding
            if ( m_bListPos )
                aTypes[nCount-1] = cppu::UnoType<sal_Int32>::get();
        }

        return aTypes;
    }

    sal_Bool SAL_CALL OCellValueBinding::supportsType( const Type& aType ) throw (RuntimeException, std::exception)
    {
        checkDisposed( );
        checkInitialized( );

        // look up in our sequence
        Sequence< Type > aSupportedTypes( getSupportedValueTypes() );
        const Type* pTypes = aSupportedTypes.getConstArray();
        const Type* pTypesEnd = aSupportedTypes.getConstArray() + aSupportedTypes.getLength();
        while ( pTypes != pTypesEnd )
            if ( aType.equals( *pTypes++ ) )
                return true;

        return false;
    }

    Any SAL_CALL OCellValueBinding::getValue( const Type& aType ) throw (IncompatibleTypesException, RuntimeException, std::exception)
    {
        checkDisposed( );
        checkInitialized( );
        checkValueType( aType );

        Any aReturn;
        switch ( aType.getTypeClass() )
        {
        case TypeClass_STRING:
            OSL_ENSURE( m_xCellText.is(), "OCellValueBinding::getValue: don't have a text!" );
            if ( m_xCellText.is() )
                aReturn <<= m_xCellText->getString();
            else
                aReturn <<= OUString();
            break;

        case TypeClass_BOOLEAN:
            OSL_ENSURE( m_xCell.is(), "OCellValueBinding::getValue: don't have a double value supplier!" );
            if ( m_xCell.is() )
            {
                // check if the cell has a numeric value (this might go into a helper function):

                bool bHasValue = false;
                CellContentType eCellType = m_xCell->getType();
                if ( eCellType == CellContentType_VALUE )
                    bHasValue = true;
                else if ( eCellType == CellContentType_FORMULA )
                {
                    // check if the formula result is a value
                    if ( m_xCell->getError() == 0 )
                    {
                        Reference<XPropertySet> xProp( m_xCell, UNO_QUERY );
                        if ( xProp.is() )
                        {
                            CellContentType eResultType;
                            if ( (xProp->getPropertyValue("FormulaResultType") >>= eResultType) && eResultType == CellContentType_VALUE )
                                bHasValue = true;
                        }
                    }
                }

                if ( bHasValue )
                {
                    // 0 is "unchecked", any other value is "checked", regardless of number format
                    double nCellValue = m_xCell->getValue();
                    bool bBoolValue = ( nCellValue != 0.0 );
                    aReturn <<= bBoolValue;
                }
                // empty cells, text cells and text or error formula results: leave return value empty
            }
            break;

        case TypeClass_DOUBLE:
            OSL_ENSURE( m_xCell.is(), "OCellValueBinding::getValue: don't have a double value supplier!" );
            if ( m_xCell.is() )
                aReturn <<= m_xCell->getValue();
            else
                aReturn <<= (double)0;
            break;

        case TypeClass_LONG:
            OSL_ENSURE( m_xCell.is(), "OCellValueBinding::getValue: don't have a double value supplier!" );
            if ( m_xCell.is() )
            {
                // The list position value in the cell is 1-based.
                // We subtract 1 from any cell value (no special handling for 0 or negative values).

                sal_Int32 nValue = (sal_Int32) rtl::math::approxFloor( m_xCell->getValue() );
                --nValue;

                aReturn <<= nValue;
            }
            else
                aReturn <<= (sal_Int32)0;
            break;

        default:
            OSL_FAIL( "OCellValueBinding::getValue: unreachable code!" );
                // a type other than double and string should never have survived the checkValueType
                // above
        }
        return aReturn;
    }

    void SAL_CALL OCellValueBinding::setValue( const Any& aValue ) throw (IncompatibleTypesException, NoSupportException, RuntimeException, std::exception)
    {
        checkDisposed( );
        checkInitialized( );
        if ( aValue.hasValue() )
            checkValueType( aValue.getValueType() );

        switch ( aValue.getValueType().getTypeClass() )
        {
        case TypeClass_STRING:
            {
                OSL_ENSURE( m_xCellText.is(), "OCellValueBinding::setValue: don't have a text!" );

                OUString sText;
                aValue >>= sText;
                if ( m_xCellText.is() )
                    m_xCellText->setString( sText );
            }
            break;

        case TypeClass_BOOLEAN:
            {
                OSL_ENSURE( m_xCell.is(), "OCellValueBinding::setValue: don't have a double value supplier!" );

                // boolean is stored as values 0 or 1
                // TODO: set the number format to boolean if no format is set?

                bool bValue( false );
                aValue >>= bValue;
                double nCellValue = bValue ? 1.0 : 0.0;

                if ( m_xCell.is() )
                    m_xCell->setValue( nCellValue );

                setBooleanFormat();
            }
            break;

        case TypeClass_DOUBLE:
            {
                OSL_ENSURE( m_xCell.is(), "OCellValueBinding::setValue: don't have a double value supplier!" );

                double nValue = 0;
                aValue >>= nValue;
                if ( m_xCell.is() )
                    m_xCell->setValue( nValue );
            }
            break;

        case TypeClass_LONG:
            {
                OSL_ENSURE( m_xCell.is(), "OCellValueBinding::setValue: don't have a double value supplier!" );

                sal_Int32 nValue = 0;
                aValue >>= nValue;      // list index from control layer (0-based)
                ++nValue;               // the list position value in the cell is 1-based
                if ( m_xCell.is() )
                    m_xCell->setValue( nValue );
            }
            break;

        case TypeClass_VOID:
            {
                // #N/A error value can only be set using XCellRangeData

                Reference<XCellRangeData> xData( m_xCell, UNO_QUERY );
                OSL_ENSURE( xData.is(), "OCellValueBinding::setValue: don't have XCellRangeData!" );
                if ( xData.is() )
                {
                    Sequence<Any> aInner(1);                            // one empty element
                    Sequence< Sequence<Any> > aOuter( &aInner, 1 );     // one row
                    xData->setDataArray( aOuter );
                }
            }
            break;

        default:
            OSL_FAIL( "OCellValueBinding::setValue: unreachable code!" );
                // a type other than double and string should never have survived the checkValueType
                // above
        }
    }

    void OCellValueBinding::setBooleanFormat()
    {
        // set boolean number format if not already set

        OUString sPropName( "NumberFormat" );
        Reference<XPropertySet> xCellProp( m_xCell, UNO_QUERY );
        Reference<XNumberFormatsSupplier> xSupplier( m_xDocument, UNO_QUERY );
        if ( xSupplier.is() && xCellProp.is() )
        {
            Reference<XNumberFormats> xFormats(xSupplier->getNumberFormats());
            Reference<XNumberFormatTypes> xTypes( xFormats, UNO_QUERY );
            if ( xTypes.is() )
            {
                lang::Locale aLocale;
                bool bWasBoolean = false;

                sal_Int32 nOldIndex = ::comphelper::getINT32( xCellProp->getPropertyValue( sPropName ) );
                Reference<XPropertySet> xOldFormat;
                try
                {
                    xOldFormat.set(xFormats->getByKey( nOldIndex ));
                }
                catch ( Exception& )
                {
                    // non-existing format - can happen, use defaults
                }
                if ( xOldFormat.is() )
                {
                    // use the locale of the existing format
                    xOldFormat->getPropertyValue("Locale") >>= aLocale;

                    sal_Int16 nOldType = ::comphelper::getINT16(
                        xOldFormat->getPropertyValue("Type") );
                    if ( nOldType & NumberFormat::LOGICAL )
                        bWasBoolean = true;
                }

                if ( !bWasBoolean )
                {
                    sal_Int32 nNewIndex = xTypes->getStandardFormat( NumberFormat::LOGICAL, aLocale );
                    xCellProp->setPropertyValue( sPropName, makeAny( nNewIndex ) );
                }
            }
        }
    }

    void OCellValueBinding::checkDisposed( ) const
    {
        if ( OCellValueBinding_Base::rBHelper.bInDispose || OCellValueBinding_Base::rBHelper.bDisposed )
            throw DisposedException();
            // TODO: is it worth having an error message here?
    }

    void OCellValueBinding::checkInitialized()
    {
        if ( !m_bInitialized )
            throw RuntimeException();
            // TODO: error message
    }

    void OCellValueBinding::checkValueType( const Type& _rType ) const
    {
        OCellValueBinding* pNonConstThis = const_cast< OCellValueBinding* >( this );
        if ( !pNonConstThis->supportsType( _rType ) )
        {
            OUString sMessage( "The given type (" );
            sMessage += _rType.getTypeName();
            sMessage += ") is not supported by this binding.";
                // TODO: localize this error message

            throw IncompatibleTypesException( sMessage, *pNonConstThis );
                // TODO: alternatively use a type converter service for this?
        }
    }

    OUString SAL_CALL OCellValueBinding::getImplementationName(  ) throw (RuntimeException, std::exception)
    {
        return OUString( "com.sun.star.comp.sheet.OCellValueBinding" );
    }

    sal_Bool SAL_CALL OCellValueBinding::supportsService( const OUString& _rServiceName ) throw (RuntimeException, std::exception)
    {
        return cppu::supportsService(this, _rServiceName);
    }

    Sequence< OUString > SAL_CALL OCellValueBinding::getSupportedServiceNames(  ) throw (RuntimeException, std::exception)
    {
        Sequence< OUString > aServices( m_bListPos ? 3 : 2 );
        aServices[ 0 ] = "com.sun.star.table.CellValueBinding";
        aServices[ 1 ] = "com.sun.star.form.binding.ValueBinding";
        if ( m_bListPos )
            aServices[ 2 ] = "com.sun.star.table.ListPositionCellBinding";
        return aServices;
    }

    void SAL_CALL OCellValueBinding::addModifyListener( const Reference< XModifyListener >& _rxListener ) throw (RuntimeException, std::exception)
    {
       if ( _rxListener.is() )
           m_aModifyListeners.addInterface( _rxListener );
    }

    void SAL_CALL OCellValueBinding::removeModifyListener( const Reference< XModifyListener >& _rxListener ) throw (RuntimeException, std::exception)
    {
       if ( _rxListener.is() )
           m_aModifyListeners.removeInterface( _rxListener );
    }

    void OCellValueBinding::notifyModified()
    {
        EventObject aEvent;
        aEvent.Source.set(*this);

        ::cppu::OInterfaceIteratorHelper aIter( m_aModifyListeners );
        while ( aIter.hasMoreElements() )
        {
            try
            {
                static_cast< XModifyListener* >( aIter.next() )->modified( aEvent );
            }
            catch( const RuntimeException& )
            {
                // silent this
            }
            catch( const Exception& )
            {
                OSL_FAIL( "OCellValueBinding::notifyModified: caught a (non-runtime) exception!" );
            }
        }
    }

    void SAL_CALL OCellValueBinding::modified( const EventObject& /* aEvent */ ) throw (RuntimeException, std::exception)
    {
        notifyModified();
    }

    void SAL_CALL OCellValueBinding::disposing( const EventObject& aEvent ) throw (RuntimeException, std::exception)
    {
        Reference<XInterface> xCellInt( m_xCell, UNO_QUERY );
        if ( xCellInt == aEvent.Source )
        {
            // release references to cell object
            m_xCell.clear();
            m_xCellText.clear();
        }
    }

    void SAL_CALL OCellValueBinding::initialize( const Sequence< Any >& _rArguments ) throw (Exception, RuntimeException, std::exception)
    {
        if ( m_bInitialized )
            throw Exception();
            // TODO: error message

        // get the cell address
        CellAddress aAddress;
        bool bFoundAddress = false;

        const Any* pLoop = _rArguments.getConstArray();
        const Any* pLoopEnd = _rArguments.getConstArray() + _rArguments.getLength();
        for ( ; ( pLoop != pLoopEnd ) && !bFoundAddress; ++pLoop )
        {
            NamedValue aValue;
            if ( *pLoop >>= aValue )
            {
                if ( aValue.Name == "BoundCell" )
                {
                    if ( aValue.Value >>= aAddress )
                        bFoundAddress = true;
                }
            }
        }

        if ( !bFoundAddress )
            // TODO: error message
            throw Exception();

        // get the cell object
        try
        {
            // first the sheets collection
            Reference< XIndexAccess > xSheets;
            if ( m_xDocument.is() )
                xSheets.set(m_xDocument->getSheets( ), css::uno::UNO_QUERY);
            OSL_ENSURE( xSheets.is(), "OCellValueBinding::initialize: could not retrieve the sheets!" );

            if ( xSheets.is() )
            {
                // the concrete sheet
                Reference< XCellRange > xSheet(xSheets->getByIndex( aAddress.Sheet ), UNO_QUERY);
                OSL_ENSURE( xSheet.is(), "OCellValueBinding::initialize: NULL sheet, but no exception!" );

                // the concrete cell
                if ( xSheet.is() )
                {
                    m_xCell.set(xSheet->getCellByPosition( aAddress.Column, aAddress.Row ));
                    Reference< XCellAddressable > xAddressAccess( m_xCell, UNO_QUERY );
                    OSL_ENSURE( xAddressAccess.is(), "OCellValueBinding::initialize: either NULL cell, or cell without address access!" );
                }
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "OCellValueBinding::initialize: caught an exception while retrieving the cell object!" );
        }

        if ( !m_xCell.is() )
            throw Exception();
            // TODO error message

        m_xCellText.set(m_xCell, css::uno::UNO_QUERY);

        Reference<XModifyBroadcaster> xBroadcaster( m_xCell, UNO_QUERY );
        if ( xBroadcaster.is() )
        {
            xBroadcaster->addModifyListener( this );
        }

        // TODO: add as XEventListener to the cell, so we get notified when it dies,
        // and can dispose ourself then

        // TODO: somehow add as listener so we get notified when the address of the cell changes
        // We need to forward this as change in our BoundCell property to our property change listeners

        // TODO: be an XModifyBroadcaster, so that changes in our cell can be notified
        // to the BindableValue which is/will be bound to this instance.

        m_bInitialized = true;
        // TODO: place your code here
    }

}   // namespace calc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
