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

#include "cellbindinghelper.hxx"
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/form/XGridColumnFactory.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/form/XFormsSupplier.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <unotools/transliterationwrapper.hxx>
#include <osl/diagnose.h>
#include <tools/diagnose_ex.h>
#include "formstrings.hxx"

#include <algorithm>


namespace pcr
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::sheet;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::drawing;
    using namespace ::com::sun::star::table;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::i18n;
    using namespace ::com::sun::star::form::binding;

    namespace
    {

        struct StringCompare
        {
        private:
            OUString m_sReference;

        public:
            explicit StringCompare( const OUString& _rReference ) : m_sReference( _rReference ) { }

            bool operator()( std::u16string_view _rCompare )
            {
                return ( _rCompare == m_sReference );
            }
        };
    }


    CellBindingHelper::CellBindingHelper( const Reference< XPropertySet >& _rxControlModel, const Reference< XModel >& _rxContextDocument )
        :m_xControlModel( _rxControlModel )
    {
        OSL_ENSURE( m_xControlModel.is(), "CellBindingHelper::CellBindingHelper: invalid control model!" );

        m_xDocument.set(_rxContextDocument, css::uno::UNO_QUERY);
        OSL_ENSURE( m_xDocument.is(), "CellBindingHelper::CellBindingHelper: This is no spreadsheet document!" );

        OSL_ENSURE( isSpreadsheetDocumentWhichSupplies( SERVICE_ADDRESS_CONVERSION ),
            "CellBindingHelper::CellBindingHelper: the document cannot convert address representations!" );
    }


    bool CellBindingHelper::isSpreadsheetDocument( const Reference< XModel >& _rxContextDocument )
    {
        return Reference< XSpreadsheetDocument >::query( _rxContextDocument ).is();
    }


    sal_Int16 CellBindingHelper::getControlSheetIndex( Reference< XSpreadsheet >& _out_rxSheet ) const
    {
        sal_Int16 nSheetIndex = -1;
        // every sheet has a draw page, and every draw page has a forms collection.
        // Our control, OTOH, belongs to a forms collection. Match these...
        try
        {
            // for determining the draw page, we need the forms collection which
            // the object belongs to. This is the first object up the hierarchy which is
            // *no* XForm (and, well, no XGridColumnFactory)
            Reference< XChild > xCheck( m_xControlModel, UNO_QUERY );
            Reference< XForm > xParentAsForm; if ( xCheck.is() ) xParentAsForm.set(xCheck->getParent(), css::uno::UNO_QUERY);
            Reference< XGridColumnFactory > xParentAsGrid; if ( xCheck.is() ) xParentAsGrid.set(xCheck->getParent(), css::uno::UNO_QUERY);

            while ( ( xParentAsForm.is() || xParentAsGrid.is() ) && xCheck.is() )
            {
                xCheck.set(xCheck->getParent(), css::uno::UNO_QUERY);
                xParentAsForm.set(xCheck.is() ? xCheck->getParent() : Reference< XForm >(), css::uno::UNO_QUERY);
                xParentAsGrid.set(xCheck.is() ? xCheck->getParent() : Reference< XGridColumnFactory >(), css::uno::UNO_QUERY);
            }
            Reference< XInterface > xFormsCollection( xCheck.is() ? xCheck->getParent() : Reference< XInterface >() );

            // now iterate through the sheets
            Reference< XIndexAccess > xSheets( m_xDocument->getSheets(), UNO_QUERY );
            if ( xSheets.is() && xFormsCollection.is() )
            {
                for ( sal_Int32 i = 0; i < xSheets->getCount(); ++i )
                {
                    Reference< XDrawPageSupplier > xSuppPage( xSheets->getByIndex( i ), UNO_QUERY_THROW );
                    Reference< XFormsSupplier > xSuppForms( xSuppPage->getDrawPage(), UNO_QUERY_THROW );

                    if ( xSuppForms->getForms() == xFormsCollection )
                    {   // found it
                        nSheetIndex = static_cast<sal_Int16>(i);
                        _out_rxSheet.set( xSuppPage, UNO_QUERY_THROW );
                        break;
                    }
                }
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("extensions.propctrlr");
        }

        return nSheetIndex;
    }


    bool CellBindingHelper::convertStringAddress( const OUString& _rAddressDescription, CellAddress& /* [out] */ _rAddress ) const
    {
        Any aAddress;
        return doConvertAddressRepresentations(
                    PROPERTY_UI_REPRESENTATION,
                    makeAny( _rAddressDescription ),
                    PROPERTY_ADDRESS,
                    aAddress,
                    false
               )
           &&  ( aAddress >>= _rAddress );
    }


    bool CellBindingHelper::doConvertAddressRepresentations( const OUString& _rInputProperty, const Any& _rInputValue,
        const OUString& _rOutputProperty, Any& _rOutputValue, bool _bIsRange ) const
    {
        bool bSuccess = false;

        Reference< XPropertySet > xConverter(
            createDocumentDependentInstance(
                _bIsRange ? OUString(SERVICE_RANGEADDRESS_CONVERSION) : OUString(SERVICE_ADDRESS_CONVERSION),
                OUString(),
                Any()
            ),
            UNO_QUERY
        );
        OSL_ENSURE( xConverter.is(), "CellBindingHelper::doConvertAddressRepresentations: could not get a converter service!" );
        if ( xConverter.is() )
        {
            try
            {
                Reference< XSpreadsheet > xSheet;
                xConverter->setPropertyValue( PROPERTY_REFERENCE_SHEET, makeAny( static_cast<sal_Int32>(getControlSheetIndex( xSheet )) ) );
                xConverter->setPropertyValue( _rInputProperty, _rInputValue );
                _rOutputValue = xConverter->getPropertyValue( _rOutputProperty );
                bSuccess = true;
            }
            catch( const Exception& )
            {
                TOOLS_WARN_EXCEPTION( "extensions.propctrlr", "CellBindingHelper::doConvertAddressRepresentations" );
            }
        }

        return bSuccess;
    }


    bool CellBindingHelper::convertStringAddress( const OUString& _rAddressDescription,
                            CellRangeAddress& /* [out] */ _rAddress ) const
    {
        Any aAddress;
        return doConvertAddressRepresentations(
                    PROPERTY_UI_REPRESENTATION,
                    makeAny( _rAddressDescription ),
                    PROPERTY_ADDRESS,
                    aAddress,
                    true
               )
           &&  ( aAddress >>= _rAddress );
    }


    Reference< XValueBinding > CellBindingHelper::createCellBindingFromAddress( const CellAddress& _rAddress, bool _bSupportIntegerExchange ) const
    {
        Reference< XValueBinding > xBinding( createDocumentDependentInstance(
            _bSupportIntegerExchange ? OUString(SERVICE_SHEET_CELL_INT_BINDING) : OUString(SERVICE_SHEET_CELL_BINDING),
            PROPERTY_BOUND_CELL,
            makeAny( _rAddress )
        ), UNO_QUERY );

        return xBinding;
    }


    Reference< XValueBinding > CellBindingHelper::createCellBindingFromStringAddress( const OUString& _rAddress, bool _bSupportIntegerExchange ) const
    {
        Reference< XValueBinding > xBinding;
        if ( !m_xDocument.is() )
            // very bad ...
            return xBinding;

        // get the UNO representation of the address
        CellAddress aAddress;
        if ( _rAddress.isEmpty() || !convertStringAddress( _rAddress, aAddress ) )
            return xBinding;

        return createCellBindingFromAddress( aAddress, _bSupportIntegerExchange );
    }


    Reference< XListEntrySource > CellBindingHelper::createCellListSourceFromStringAddress( const OUString& _rAddress ) const
    {
        Reference< XListEntrySource > xSource;

        CellRangeAddress aRangeAddress;
        if ( _rAddress.isEmpty() || !convertStringAddress( _rAddress, aRangeAddress ) )
            return xSource;

        // create a range object for this address
        xSource.set(createDocumentDependentInstance(
            SERVICE_SHEET_CELLRANGE_LISTSOURCE,
            PROPERTY_LIST_CELL_RANGE,
            makeAny( aRangeAddress )
        ), css::uno::UNO_QUERY);

        return xSource;
    }


    Reference< XInterface > CellBindingHelper::createDocumentDependentInstance( const OUString& _rService, const OUString& _rArgumentName,
        const Any& _rArgumentValue ) const
    {
        Reference< XInterface > xReturn;

        Reference< XMultiServiceFactory > xDocumentFactory( m_xDocument, UNO_QUERY );
        OSL_ENSURE( xDocumentFactory.is(), "CellBindingHelper::createDocumentDependentInstance: no document service factory!" );
        if ( xDocumentFactory.is() )
        {
            try
            {
                if ( !_rArgumentName.isEmpty() )
                {
                    NamedValue aArg;
                    aArg.Name = _rArgumentName;
                    aArg.Value = _rArgumentValue;

                    Sequence< Any > aArgs( 1 );
                    aArgs[ 0 ] <<= aArg;

                    xReturn = xDocumentFactory->createInstanceWithArguments( _rService, aArgs );
                }
                else
                {
                    xReturn = xDocumentFactory->createInstance( _rService );
                }
            }
            catch ( const Exception& )
            {
                OSL_FAIL( "CellBindingHelper::createDocumentDependentInstance: could not create the binding at the document!" );
            }
        }
        return xReturn;
    }


    bool CellBindingHelper::getAddressFromCellBinding(
        const Reference< XValueBinding >& _rxBinding, CellAddress& _rAddress ) const
    {
        OSL_PRECOND( !_rxBinding.is() || isCellBinding( _rxBinding ), "CellBindingHelper::getAddressFromCellBinding: this is no cell binding!" );

        bool bReturn = false;
        if ( !m_xDocument.is() )
            // very bad ...
            return bReturn;

        try
        {
            Reference< XPropertySet > xBindingProps( _rxBinding, UNO_QUERY );
            OSL_ENSURE( xBindingProps.is() || !_rxBinding.is(), "CellBindingHelper::getAddressFromCellBinding: no property set for the binding!" );
            if ( xBindingProps.is() )
            {
                bReturn = ( xBindingProps->getPropertyValue( PROPERTY_BOUND_CELL ) >>= _rAddress );
            }
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "extensions.propctrlr", "CellBindingHelper::getAddressFromCellBinding" );
        }

        return bReturn;
    }


    OUString CellBindingHelper::getStringAddressFromCellBinding( const Reference< XValueBinding >& _rxBinding ) const
    {
        CellAddress aAddress;
        OUString sAddress;
        if ( getAddressFromCellBinding( _rxBinding, aAddress ) )
        {
            Any aStringAddress;
            doConvertAddressRepresentations( PROPERTY_ADDRESS, makeAny( aAddress ),
                PROPERTY_UI_REPRESENTATION, aStringAddress, false );

            aStringAddress >>= sAddress;
        }

        return sAddress;
    }


    OUString CellBindingHelper::getStringAddressFromCellListSource( const Reference< XListEntrySource >& _rxSource ) const
    {
        OSL_PRECOND( !_rxSource.is() || isCellRangeListSource( _rxSource ), "CellBindingHelper::getStringAddressFromCellListSource: this is no cell list source!" );

        OUString sAddress;
        if ( !m_xDocument.is() )
            // very bad ...
            return sAddress;

        try
        {
            Reference< XPropertySet > xSourceProps( _rxSource, UNO_QUERY );
            OSL_ENSURE( xSourceProps.is() || !_rxSource.is(), "CellBindingHelper::getStringAddressFromCellListSource: no property set for the list source!" );
            if ( xSourceProps.is() )
            {
                CellRangeAddress aRangeAddress;
                xSourceProps->getPropertyValue( PROPERTY_LIST_CELL_RANGE ) >>= aRangeAddress;

                Any aStringAddress;
                doConvertAddressRepresentations( PROPERTY_ADDRESS, makeAny( aRangeAddress ),
                    PROPERTY_UI_REPRESENTATION, aStringAddress, true );
                aStringAddress >>= sAddress;
            }
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "extensions.propctrlr", "CellBindingHelper::getStringAddressFromCellListSource" );
        }

        return sAddress;
    }


    bool CellBindingHelper::isSpreadsheetDocumentWhichSupplies( const OUString& _rService ) const
    {
        bool bYesItIs = false;

        Reference< XServiceInfo > xSI( m_xDocument, UNO_QUERY );
        if ( xSI.is() && xSI->supportsService( SERVICE_SPREADSHEET_DOCUMENT ) )
        {
            Reference< XMultiServiceFactory > xDocumentFactory( m_xDocument, UNO_QUERY );
            OSL_ENSURE( xDocumentFactory.is(), "CellBindingHelper::isSpreadsheetDocumentWhichSupplies: spreadsheet document, but no factory?" );

            Sequence< OUString > aAvailableServices;
            if ( xDocumentFactory.is() )
                aAvailableServices = xDocumentFactory->getAvailableServiceNames( );

            bYesItIs = std::any_of(
                aAvailableServices.begin(),
                aAvailableServices.end(),
                StringCompare( _rService )
            );
        }

        return bYesItIs;
    }


    bool CellBindingHelper::isListCellRangeAllowed( ) const
    {
        bool bAllow( false );

        Reference< XListEntrySink > xSink( m_xControlModel, UNO_QUERY );
        if ( xSink.is() )
        {
            bAllow = isSpreadsheetDocumentWhichSupplies( SERVICE_SHEET_CELLRANGE_LISTSOURCE );
        }

        return bAllow;
    }


    bool CellBindingHelper::isCellIntegerBindingAllowed( ) const
    {
        bool bAllow( true );

        // first, we only offer this for controls which allow bindings in general
        Reference< XBindableValue > xBindable( m_xControlModel, UNO_QUERY );
        if ( !xBindable.is() )
            bAllow = false;

        // then, we must live in a spreadsheet document which can provide the special
        // service needed for exchanging integer values
        if ( bAllow )
            bAllow = isSpreadsheetDocumentWhichSupplies( SERVICE_SHEET_CELL_INT_BINDING );

        // then, we only offer this for list boxes
        if ( bAllow )
        {
            try
            {
                sal_Int16 nClassId = FormComponentType::CONTROL;
                m_xControlModel->getPropertyValue( PROPERTY_CLASSID ) >>= nClassId;
                if ( FormComponentType::LISTBOX != nClassId )
                    bAllow = false;
            }
            catch( const Exception& )
            {
                TOOLS_WARN_EXCEPTION( "extensions.propctrlr", "CellBindingHelper::isCellIntegerBindingAllowed" );
                    // are there really control models which survive isCellBindingAllowed, but don't have a ClassId
                    // property?
                bAllow = false;
            }
        }

        return bAllow;
    }


    bool CellBindingHelper::isCellBindingAllowed( ) const
    {
        bool bAllow( false );

        Reference< XBindableValue > xBindable( m_xControlModel, UNO_QUERY );
        if ( xBindable.is() )
        {
            // the control can potentially be bound to an external value
            // Does it live within a Calc document, and is able to supply CellBindings?
            bAllow = isSpreadsheetDocumentWhichSupplies( SERVICE_SHEET_CELL_BINDING );
        }

        // disallow for some types
        // TODO: shouldn't the XBindableValue supply a list of supported types, and we can distinguish
        // using this list? The current behavior below is somewhat hackish...
        if ( bAllow )
        {
            try
            {
                sal_Int16 nClassId = FormComponentType::CONTROL;
                m_xControlModel->getPropertyValue( PROPERTY_CLASSID ) >>= nClassId;
                if ( ( FormComponentType::DATEFIELD == nClassId ) || ( FormComponentType::TIMEFIELD == nClassId ) )
                    bAllow = false;
            }
            catch( const Exception& )
            {
                TOOLS_WARN_EXCEPTION( "extensions.propctrlr", "CellBindingHelper::isCellBindingAllowed" );
                bAllow = false;
            }
        }
        return bAllow;
    }


    bool CellBindingHelper::isCellBinding( const Reference< XValueBinding >& _rxBinding )
    {
        return doesComponentSupport( _rxBinding, SERVICE_SHEET_CELL_BINDING );
    }


    bool CellBindingHelper::isCellIntegerBinding( const Reference< XValueBinding >& _rxBinding )
    {
        return doesComponentSupport( _rxBinding, SERVICE_SHEET_CELL_INT_BINDING );
    }


    bool CellBindingHelper::isCellRangeListSource( const Reference< XListEntrySource >& _rxSource )
    {
        return doesComponentSupport( _rxSource, SERVICE_SHEET_CELLRANGE_LISTSOURCE );
    }


    bool CellBindingHelper::doesComponentSupport( const Reference< XInterface >& _rxComponent, const OUString& _rService )
    {
        Reference< XServiceInfo > xSI( _rxComponent, UNO_QUERY );
        bool bDoes = xSI.is() && xSI->supportsService( _rService );
        return bDoes;
    }


    Reference< XValueBinding > CellBindingHelper::getCurrentBinding( ) const
    {
        Reference< XValueBinding > xBinding;
        Reference< XBindableValue > xBindable( m_xControlModel, UNO_QUERY );
        if ( xBindable.is() )
            xBinding = xBindable->getValueBinding();
        return xBinding;
    }


    Reference< XListEntrySource > CellBindingHelper::getCurrentListSource( ) const
    {
        Reference< XListEntrySource > xSource;
        Reference< XListEntrySink > xSink( m_xControlModel, UNO_QUERY );
        if ( xSink.is() )
            xSource = xSink->getListEntrySource();
        return xSource;
    }


    void CellBindingHelper::setBinding( const Reference< XValueBinding >& _rxBinding )
    {
        Reference< XBindableValue > xBindable( m_xControlModel, UNO_QUERY );
        OSL_PRECOND( xBindable.is(), "CellBindingHelper::setBinding: the object is not bindable!" );
        if ( xBindable.is() )
            xBindable->setValueBinding( _rxBinding );
    }


    void CellBindingHelper::setListSource( const Reference< XListEntrySource >& _rxSource )
    {
        Reference< XListEntrySink > xSink( m_xControlModel, UNO_QUERY );
        OSL_PRECOND( xSink.is(), "CellBindingHelper::setListSource: the object is no list entry sink!" );
        if ( xSink.is() )
            xSink->setListEntrySource( _rxSource );
    }


}   // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
