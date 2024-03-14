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

#include "formcellbinding.hxx"
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include "strings.hxx"
#include <osl/diagnose.h>
#include <comphelper/diagnose_ex.hxx>

#include <algorithm>

namespace xmloff
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::sheet;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::table;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::form::binding;

namespace
{
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::container::XChild;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::uno::UNO_QUERY;

    template< class TYPE >
    Reference< TYPE > getTypedModelNode( const Reference< XInterface >& _rxModelNode )
    {
        Reference< TYPE > xTypedNode( _rxModelNode, UNO_QUERY );
        if ( xTypedNode.is() )
            return xTypedNode;
        else
        {
            Reference< XChild > xChild( _rxModelNode, UNO_QUERY );
            if ( xChild.is() )
                return getTypedModelNode< TYPE >( xChild->getParent() );
            else
                return nullptr;
        }
    }

    Reference< XModel > getDocument( const Reference< XInterface >& _rxModelNode )
    {
        return getTypedModelNode< XModel >( _rxModelNode );
    }

    struct StringCompare
    {
    private:
        const OUString & m_sReference;

    public:
        explicit StringCompare( const OUString& _rReference ) : m_sReference( _rReference ) { }

        bool operator()( std::u16string_view _rCompare )
        {
            return ( _rCompare == m_sReference );
        }
    };
}

//= FormCellBindingHelper
FormCellBindingHelper::FormCellBindingHelper( const Reference< XPropertySet >& _rxControlModel, const Reference< XModel >& _rxDocument )
    :m_xControlModel( _rxControlModel )
    ,m_xDocument( _rxDocument, UNO_QUERY )
{
    OSL_ENSURE( m_xControlModel.is(), "FormCellBindingHelper::FormCellBindingHelper: invalid control model!" );

    if ( !m_xDocument.is() )
        m_xDocument.set(getDocument( m_xControlModel ), css::uno::UNO_QUERY);
    OSL_ENSURE( m_xDocument.is(), "FormCellBindingHelper::FormCellBindingHelper: Did not find the spreadsheet document!" );
}

bool FormCellBindingHelper::livesInSpreadsheetDocument( const Reference< XPropertySet >& _rxControlModel )
{
    Reference< XSpreadsheetDocument > xDocument( getDocument( _rxControlModel ), UNO_QUERY );
    return xDocument.is();
}

bool FormCellBindingHelper::convertStringAddress( const OUString& _rAddressDescription, CellAddress& /* [out] */ _rAddress ) const
{
    Any aAddress;
    return doConvertAddressRepresentations(
                PROPERTY_FILE_REPRESENTATION,
                Any( _rAddressDescription ),
                PROPERTY_ADDRESS,
                aAddress,
                false
           )
       &&  ( aAddress >>= _rAddress );
}

bool FormCellBindingHelper::convertStringAddress( const OUString& _rAddressDescription,
                        CellRangeAddress& /* [out] */ _rAddress ) const
{
    Any aAddress;
    return doConvertAddressRepresentations(
                PROPERTY_FILE_REPRESENTATION,
                Any( _rAddressDescription ),
                PROPERTY_ADDRESS,
                aAddress,
                true
           )
       &&  ( aAddress >>= _rAddress );
}

Reference< XValueBinding > FormCellBindingHelper::createCellBindingFromStringAddress( const OUString& _rAddress, bool _bUseIntegerBinding ) const
{
    Reference< XValueBinding > xBinding;
    if ( !m_xDocument.is() )
        // very bad ...
        return xBinding;

    // get the UNO representation of the address
    CellAddress aAddress;
    if ( _rAddress.isEmpty() || !convertStringAddress( _rAddress, aAddress ) )
        return xBinding;

    xBinding.set(createDocumentDependentInstance(
        _bUseIntegerBinding ? SERVICE_LISTINDEXCELLBINDING : SERVICE_CELLVALUEBINDING,
        PROPERTY_BOUND_CELL,
        Any( aAddress )
    ), css::uno::UNO_QUERY);

    return xBinding;
}

Reference< XListEntrySource > FormCellBindingHelper::createCellListSourceFromStringAddress( const OUString& _rAddress ) const
{
    Reference< XListEntrySource > xSource;

    CellRangeAddress aRangeAddress;
    if ( !convertStringAddress( _rAddress, aRangeAddress ) )
        return xSource;

    // create a range object for this address
    xSource.set(createDocumentDependentInstance(
        SERVICE_CELLRANGELISTSOURCE,
        PROPERTY_LIST_CELL_RANGE,
        Any( aRangeAddress )
    ), css::uno::UNO_QUERY);

    return xSource;
}

OUString FormCellBindingHelper::getStringAddressFromCellBinding( const Reference< XValueBinding >& _rxBinding ) const
{
    OSL_PRECOND( !_rxBinding.is() || isCellBinding( _rxBinding ), "FormCellBindingHelper::getStringAddressFromCellBinding: this is no cell binding!" );

    OUString sAddress;
    try
    {
        Reference< XPropertySet > xBindingProps( _rxBinding, UNO_QUERY );
        OSL_ENSURE( xBindingProps.is() || !_rxBinding.is(), "FormCellBindingHelper::getStringAddressFromCellBinding: no property set for the binding!" );
        if ( xBindingProps.is() )
        {
            CellAddress aAddress;
            xBindingProps->getPropertyValue( PROPERTY_BOUND_CELL ) >>= aAddress;

            Any aStringAddress;
            doConvertAddressRepresentations( PROPERTY_ADDRESS, Any( aAddress ),
                PROPERTY_FILE_REPRESENTATION, aStringAddress, false );

            aStringAddress >>= sAddress;
        }
    }
    catch( const Exception& )
    {
        TOOLS_WARN_EXCEPTION( "xmloff", "FormCellBindingHelper::getStringAddressFromCellBinding" );
    }

    return sAddress;
}

OUString FormCellBindingHelper::getStringAddressFromCellListSource( const Reference< XListEntrySource >& _rxSource ) const
{
    OSL_PRECOND( !_rxSource.is() || isCellRangeListSource( _rxSource ), "FormCellBindingHelper::getStringAddressFromCellListSource: this is no cell list source!" );

    OUString sAddress;
    try
    {
        Reference< XPropertySet > xSourceProps( _rxSource, UNO_QUERY );
        OSL_ENSURE( xSourceProps.is() || !_rxSource.is(), "FormCellBindingHelper::getStringAddressFromCellListSource: no property set for the list source!" );
        if ( xSourceProps.is() )
        {
            CellRangeAddress aRangeAddress;
            xSourceProps->getPropertyValue( PROPERTY_LIST_CELL_RANGE ) >>= aRangeAddress;

            Any aStringAddress;
            doConvertAddressRepresentations( PROPERTY_ADDRESS, Any( aRangeAddress ),
                PROPERTY_FILE_REPRESENTATION, aStringAddress, true );
            aStringAddress >>= sAddress;
        }
    }
    catch( const Exception& )
    {
        TOOLS_WARN_EXCEPTION( "xmloff", "FormCellBindingHelper::getStringAddressFromCellListSource" );
    }

    return sAddress;
}

bool FormCellBindingHelper::isSpreadsheetDocumentWhichSupplies( const Reference< XSpreadsheetDocument >& _rxDocument, const OUString& _rService )
{
    bool bYesItIs = false;

    try
    {
        Reference< XServiceInfo > xSI( _rxDocument, UNO_QUERY );
        if ( xSI.is() && xSI->supportsService( SERVICE_SPREADSHEET_DOCUMENT ) )
        {
            Reference< XMultiServiceFactory > xDocumentFactory( _rxDocument, UNO_QUERY );
            OSL_ENSURE( xDocumentFactory.is(), "FormCellBindingHelper::isSpreadsheetDocumentWhichSupplies: spreadsheet document, but no factory?" );

            if ( xDocumentFactory.is() )
            {
                const Sequence<OUString> aAvailableServices = xDocumentFactory->getAvailableServiceNames( );

                bYesItIs = std::any_of( aAvailableServices.begin(), aAvailableServices.end(), StringCompare( _rService ) );
            }
        }
    }
    catch( const Exception& )
    {
        TOOLS_WARN_EXCEPTION( "xmloff", "FormCellBindingHelper::isSpreadsheetDocumentWhichSupplies" );
    }

    return bYesItIs;
}

bool FormCellBindingHelper::isSpreadsheetDocumentWhichSupplies( const OUString& _rService ) const
{
    return isSpreadsheetDocumentWhichSupplies( m_xDocument, _rService );
}

bool FormCellBindingHelper::isListCellRangeAllowed( const Reference< XModel >& _rxDocument )
{
    return isSpreadsheetDocumentWhichSupplies(
        Reference< XSpreadsheetDocument >( _rxDocument, UNO_QUERY ),
        SERVICE_CELLRANGELISTSOURCE
    );
}

bool FormCellBindingHelper::isListCellRangeAllowed( ) const
{
    bool bAllow( false );

    Reference< XListEntrySink > xSink( m_xControlModel, UNO_QUERY );
    if ( xSink.is() )
    {
        bAllow = isSpreadsheetDocumentWhichSupplies( SERVICE_CELLRANGELISTSOURCE );
    }

    return bAllow;
}

bool FormCellBindingHelper::isCellBindingAllowed( ) const
{
    bool bAllow( false );

    Reference< XBindableValue > xBindable( m_xControlModel, UNO_QUERY );
    if ( xBindable.is() )
    {
        // the control can potentially be bound to an external value
        // Does it live within a Calc document, and is able to supply CellBindings?
        bAllow = isSpreadsheetDocumentWhichSupplies( SERVICE_CELLVALUEBINDING );
    }

    return bAllow;
}

bool FormCellBindingHelper::isCellBindingAllowed( const Reference< XModel >& _rxDocument )
{
    return isSpreadsheetDocumentWhichSupplies(
        Reference< XSpreadsheetDocument >( _rxDocument, UNO_QUERY ),
        SERVICE_CELLVALUEBINDING
    );
}

bool FormCellBindingHelper::isCellBinding( const Reference< XValueBinding >& _rxBinding )
{
    return doesComponentSupport( _rxBinding, SERVICE_CELLVALUEBINDING );
}

bool FormCellBindingHelper::isCellIntegerBinding( const Reference< XValueBinding >& _rxBinding )
{
    return doesComponentSupport( _rxBinding, SERVICE_LISTINDEXCELLBINDING );
}

bool FormCellBindingHelper::isCellRangeListSource( const Reference< XListEntrySource >& _rxSource )
{
    return doesComponentSupport( _rxSource, SERVICE_CELLRANGELISTSOURCE );
}

bool FormCellBindingHelper::doesComponentSupport( const Reference< XInterface >& _rxComponent, const OUString& _rService )
{
    Reference< XServiceInfo > xSI( _rxComponent, UNO_QUERY );
    bool bDoes = xSI.is() && xSI->supportsService( _rService );
    return bDoes;
}

Reference< XValueBinding > FormCellBindingHelper::getCurrentBinding( ) const
{
    Reference< XBindableValue > xBindable( m_xControlModel, UNO_QUERY );
    if ( xBindable.is() )
        return xBindable->getValueBinding();
    return Reference<XValueBinding>();
}

Reference< XListEntrySource > FormCellBindingHelper::getCurrentListSource( ) const
{
    Reference< XListEntrySink > xSink( m_xControlModel, UNO_QUERY );
    if (xSink.is())
        return xSink->getListEntrySource();
    return Reference<XListEntrySource>();
}

void FormCellBindingHelper::setBinding( const Reference< XValueBinding >& _rxBinding )
{
    Reference< XBindableValue > xBindable( m_xControlModel, UNO_QUERY );
    OSL_PRECOND( xBindable.is(), "FormCellBindingHelper::setBinding: the object is not bindable!" );
    if ( xBindable.is() )
        xBindable->setValueBinding( _rxBinding );
}

void FormCellBindingHelper::setListSource( const Reference< XListEntrySource >& _rxSource )
{
    Reference< XListEntrySink > xSink( m_xControlModel, UNO_QUERY );
    OSL_PRECOND( xSink.is(), "FormCellBindingHelper::setListSource: the object is no list entry sink!" );
    if ( xSink.is() )
        xSink->setListEntrySource( _rxSource );
}

Reference< XInterface > FormCellBindingHelper::createDocumentDependentInstance( const OUString& _rService, const OUString& _rArgumentName,
    const Any& _rArgumentValue ) const
{
    Reference< XInterface > xReturn;

    Reference< XMultiServiceFactory > xDocumentFactory( m_xDocument, UNO_QUERY );
    OSL_ENSURE( xDocumentFactory.is(), "FormCellBindingHelper::createDocumentDependentInstance: no document service factory!" );
    if ( xDocumentFactory.is() )
    {
        try
        {
            if ( !_rArgumentName.isEmpty() )
            {
                NamedValue aArg;
                aArg.Name = _rArgumentName;
                aArg.Value = _rArgumentValue;

                Sequence< Any > aArgs{ Any(aArg) };
                xReturn = xDocumentFactory->createInstanceWithArguments( _rService, aArgs );
            }
            else
            {
                xReturn = xDocumentFactory->createInstance( _rService );
            }
        }
        catch ( const Exception& )
        {
            OSL_FAIL( "FormCellBindingHelper::createDocumentDependentInstance: could not create the binding at the document!" );
        }
    }
    return xReturn;
}

bool FormCellBindingHelper::doConvertAddressRepresentations( const OUString& _rInputProperty, const Any& _rInputValue,
    const OUString& _rOutputProperty, Any& _rOutputValue, bool _bIsRange ) const
{
    bool bSuccess = false;

    Reference< XPropertySet > xConverter(
        createDocumentDependentInstance(
            _bIsRange ? SERVICE_RANGEADDRESS_CONVERSION : SERVICE_ADDRESS_CONVERSION,
            OUString(),
            Any()
        ),
        UNO_QUERY
    );
    OSL_ENSURE( xConverter.is(), "FormCellBindingHelper::doConvertAddressRepresentations: could not get a converter service!" );
    if ( xConverter.is() )
    {
        try
        {
            xConverter->setPropertyValue( _rInputProperty, _rInputValue );
            _rOutputValue = xConverter->getPropertyValue( _rOutputProperty );
            bSuccess = true;
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "xmloff", "FormCellBindingHelper::doConvertAddressRepresentations" );
        }
    }

    return bSuccess;
}

}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
