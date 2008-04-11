/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cellbindinghelper.cxx,v $
 * $Revision: 1.12 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"
#include "cellbindinghelper.hxx"
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/form/XGridColumnFactory.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/form/XFormsSupplier.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <unotools/transliterationwrapper.hxx>
#include <osl/diagnose.h>
#include "formstrings.hxx"

#include <functional>
#include <algorithm>

//............................................................................
namespace pcr
{
//............................................................................

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
        //....................................................................
        struct StringCompare : public ::std::unary_function< ::rtl::OUString, bool >
        {
        private:
            ::rtl::OUString m_sReference;

        public:
            StringCompare( const ::rtl::OUString& _rReference ) : m_sReference( _rReference ) { }

            inline bool operator()( const ::rtl::OUString& _rCompare )
            {
                return ( _rCompare == m_sReference ) ? true : false;
            }
        };
        //....................................................................
        bool isAsciiLetter( sal_Unicode _c )
        {
            return  ( _c >= 'A' && _c <= 'Z' )
                ||  ( _c >= 'a' && _c <= 'z' );
        }

        //....................................................................
        sal_Unicode toUpperAscii( sal_Unicode _c )
        {
            sal_Unicode nUpper( _c );
            if ( _c >= 'a' && _c <= 'z' )
                nUpper += sal_Unicode( 'A' - 'a' );
            return nUpper;
        }
    }

    //========================================================================
    //= CellBindingHelper
    //========================================================================
    //------------------------------------------------------------------------
    CellBindingHelper::CellBindingHelper( const Reference< XPropertySet >& _rxControlModel, const Reference< XModel >& _rxContextDocument )
        :m_xControlModel( _rxControlModel )
    {
        OSL_ENSURE( m_xControlModel.is(), "CellBindingHelper::CellBindingHelper: invalid control model!" );

        m_xDocument = m_xDocument.query( _rxContextDocument );
        OSL_ENSURE( m_xDocument.is(), "CellBindingHelper::CellBindingHelper: This is no spreadsheet document!" );

        OSL_ENSURE( isSpreadsheetDocumentWhichSupplies( SERVICE_ADDRESS_CONVERSION ),
            "CellBindingHelper::CellBindingHelper: the document cannot convert address representations!" );
    }

    //------------------------------------------------------------------------
    sal_Bool CellBindingHelper::isSpreadsheetDocument( const Reference< XModel >& _rxContextDocument )
    {
        return Reference< XSpreadsheetDocument >::query( _rxContextDocument ).is();
    }

    //------------------------------------------------------------------------
    sal_Int16 CellBindingHelper::getControlSheetIndex( ) const
    {
        sal_Int16 nSheetIndex = -1;
        // every sheet has a draw page, and every draw page has a forms collection.
        // Our control, OTOH, belongs to a forms collection. Match these ...
        try
        {
            // for determining the draw page, we need the forms collection which
            // the object belongs to. This is the first object up the hierarchy which is
            // *no* XForm (and, well, no XGridColumnFactory)
            Reference< XChild > xCheck( m_xControlModel, UNO_QUERY );
            Reference< XForm > xParentAsForm; if ( xCheck.is() ) xParentAsForm = xParentAsForm.query( xCheck->getParent() );
            Reference< XGridColumnFactory > xParentAsGrid; if ( xCheck.is() ) xParentAsGrid = xParentAsGrid.query( xCheck->getParent() );

            while ( ( xParentAsForm.is() || xParentAsGrid.is() ) && xCheck.is() )
            {
                xCheck = xCheck.query( xCheck->getParent() );
                xParentAsForm = xParentAsForm.query( xCheck.is() ? xCheck->getParent() : (Reference< XInterface >) Reference< XForm >() );
                xParentAsGrid = xParentAsGrid.query( xCheck.is() ? xCheck->getParent() : (Reference< XInterface >) Reference< XGridColumnFactory >() );
            }
            Reference< XInterface > xFormsCollection( xCheck.is() ? xCheck->getParent() : Reference< XInterface >() );

            // now iterate through the sheets
            Reference< XIndexAccess > xSheets( m_xDocument->getSheets(), UNO_QUERY );
            if ( xSheets.is() && xFormsCollection.is() )
            {
                for ( sal_Int32 i = 0; i < xSheets->getCount(); ++i )
                {
                    Reference< XDrawPageSupplier > xSuppPage;
                    xSheets->getByIndex( i ) >>= xSuppPage;

                    Reference< XFormsSupplier > xSuppForms;
                    if ( xSuppPage.is() )
                        xSuppForms = xSuppForms.query( xSuppPage->getDrawPage() );

                    OSL_ENSURE( xSuppForms.is(), "CellBindingHelper::getControlSheetIndex: could not determine the forms supplier!" );

                    if ( xSuppForms.is() && ( xSuppForms->getForms() == xFormsCollection ) )
                    {   // found it
                        nSheetIndex = (sal_Int16)i;
                        break;
                    }
                }
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "CellBindingHelper::getControlSheetIndex: caught an exception!" );
        }

        return nSheetIndex;
    }

    //------------------------------------------------------------------------
    bool CellBindingHelper::convertStringAddress( const ::rtl::OUString& _rAddressDescription, CellAddress& /* [out] */ _rAddress ) const
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

    //------------------------------------------------------------------------
    bool CellBindingHelper::doConvertAddressRepresentations( const ::rtl::OUString& _rInputProperty, const Any& _rInputValue,
        const ::rtl::OUString& _rOutputProperty, Any& _rOutputValue, bool _bIsRange ) const SAL_THROW(())
    {
        bool bSuccess = false;

        Reference< XPropertySet > xConverter(
            createDocumentDependentInstance(
                _bIsRange ? SERVICE_RANGEADDRESS_CONVERSION : SERVICE_ADDRESS_CONVERSION,
                ::rtl::OUString(),
                Any()
            ),
            UNO_QUERY
        );
        OSL_ENSURE( xConverter.is(), "CellBindingHelper::doConvertAddressRepresentations: could not get a converter service!" );
        if ( xConverter.is() )
        {
            try
            {
                xConverter->setPropertyValue( PROPERTY_REFERENCE_SHEET, makeAny( (sal_Int32)getControlSheetIndex() ) );
                xConverter->setPropertyValue( _rInputProperty, _rInputValue );
                _rOutputValue = xConverter->getPropertyValue( _rOutputProperty );
                bSuccess = true;
            }
            catch( const Exception& )
            {
                OSL_ENSURE( sal_False, "CellBindingHelper::doConvertAddressRepresentations: caught an exception!" );
            }
        }

        return bSuccess;
    }

    //------------------------------------------------------------------------
    bool CellBindingHelper::convertStringAddress( const ::rtl::OUString& _rAddressDescription,
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

    //------------------------------------------------------------------------
    Reference< XValueBinding > CellBindingHelper::createCellBindingFromAddress( const CellAddress& _rAddress, bool _bSupportIntegerExchange ) const
    {
        Reference< XValueBinding > xBinding( createDocumentDependentInstance(
            _bSupportIntegerExchange ? SERVICE_SHEET_CELL_INT_BINDING : SERVICE_SHEET_CELL_BINDING,
            PROPERTY_BOUND_CELL,
            makeAny( _rAddress )
        ), UNO_QUERY );

        return xBinding;
    }

    //------------------------------------------------------------------------
    Reference< XValueBinding > CellBindingHelper::createCellBindingFromStringAddress( const ::rtl::OUString& _rAddress, bool _bSupportIntegerExchange ) const
    {
        Reference< XValueBinding > xBinding;
        if ( !m_xDocument.is() )
            // very bad ...
            return xBinding;

        // get the UNO representation of the address
        CellAddress aAddress;
        if ( !_rAddress.getLength() || !convertStringAddress( _rAddress, aAddress ) )
            return xBinding;

        return createCellBindingFromAddress( aAddress, _bSupportIntegerExchange );
    }

    //------------------------------------------------------------------------
    Reference< XListEntrySource > CellBindingHelper::createCellListSourceFromStringAddress( const ::rtl::OUString& _rAddress ) const
    {
        Reference< XListEntrySource > xSource;

        CellRangeAddress aRangeAddress;
        if ( !_rAddress.getLength() || !convertStringAddress( _rAddress, aRangeAddress ) )
            return xSource;

        // create a range object for this address
        xSource = xSource.query( createDocumentDependentInstance(
            SERVICE_SHEET_CELLRANGE_LISTSOURCE,
            PROPERTY_LIST_CELL_RANGE,
            makeAny( aRangeAddress )
        ) );

        return xSource;
    }

    //------------------------------------------------------------------------
    Reference< XInterface > CellBindingHelper::createDocumentDependentInstance( const ::rtl::OUString& _rService, const ::rtl::OUString& _rArgumentName,
        const Any& _rArgumentValue ) const
    {
        Reference< XInterface > xReturn;

        Reference< XMultiServiceFactory > xDocumentFactory( m_xDocument, UNO_QUERY );
        OSL_ENSURE( xDocumentFactory.is(), "CellBindingHelper::createDocumentDependentInstance: no document service factory!" );
        if ( xDocumentFactory.is() )
        {
            try
            {
                if ( _rArgumentName.getLength() )
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
                OSL_ENSURE( sal_False, "CellBindingHelper::createDocumentDependentInstance: could not create the binding at the document!" );
            }
        }
        return xReturn;
    }

    //------------------------------------------------------------------------
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
                CellAddress aAddress;
                bReturn = (bool)( xBindingProps->getPropertyValue( PROPERTY_BOUND_CELL ) >>= _rAddress );
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "CellBindingHelper::getAddressFromCellBinding: caught an exception!" );
        }

        return bReturn;
    }

    //------------------------------------------------------------------------
    ::rtl::OUString CellBindingHelper::getStringAddressFromCellBinding( const Reference< XValueBinding >& _rxBinding ) const
    {
        CellAddress aAddress;
        ::rtl::OUString sAddress;
        if ( getAddressFromCellBinding( _rxBinding, aAddress ) )
        {
            Any aStringAddress;
            doConvertAddressRepresentations( PROPERTY_ADDRESS, makeAny( aAddress ),
                PROPERTY_UI_REPRESENTATION, aStringAddress, false );

            aStringAddress >>= sAddress;
        }

        return sAddress;
    }

    //------------------------------------------------------------------------
    ::rtl::OUString CellBindingHelper::getStringAddressFromCellListSource( const Reference< XListEntrySource >& _rxSource ) const
    {
        OSL_PRECOND( !_rxSource.is() || isCellRangeListSource( _rxSource ), "CellBindingHelper::getStringAddressFromCellListSource: this is no cell list source!" );

        ::rtl::OUString sAddress;
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
            OSL_ENSURE( sal_False, "CellBindingHelper::getStringAddressFromCellListSource: caught an exception!" );
        }

        return sAddress;
    }

    //------------------------------------------------------------------------
    bool CellBindingHelper::isSpreadsheetDocumentWhichSupplies( const ::rtl::OUString& _rService ) const
    {
        bool bYesItIs = false;

        Reference< XServiceInfo > xSI( m_xDocument, UNO_QUERY );
        if ( xSI.is() && xSI->supportsService( SERVICE_SPREADSHEET_DOCUMENT ) )
        {
            Reference< XMultiServiceFactory > xDocumentFactory( m_xDocument, UNO_QUERY );
            OSL_ENSURE( xDocumentFactory.is(), "CellBindingHelper::isSpreadsheetDocumentWhichSupplies: spreadsheet document, but no factory?" );

            Sequence< ::rtl::OUString > aAvailableServices;
            if ( xDocumentFactory.is() )
                aAvailableServices = xDocumentFactory->getAvailableServiceNames( );

            const ::rtl::OUString* pFound = ::std::find_if(
                aAvailableServices.getConstArray(),
                aAvailableServices.getConstArray() + aAvailableServices.getLength(),
                StringCompare( _rService )
            );
            if ( pFound - aAvailableServices.getConstArray() < aAvailableServices.getLength() )
            {
                bYesItIs = true;
            }
        }

        return bYesItIs;
    }

    //------------------------------------------------------------------------
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

    //------------------------------------------------------------------------
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
                OSL_ENSURE( sal_False, "CellBindingHelper::isCellIntegerBindingAllowed: caught an exception!" );
                    // are there really control models which survive isCellBindingAllowed, but don't have a ClassId
                    // property?
                bAllow = false;
            }
        }

        return bAllow;
    }

    //------------------------------------------------------------------------
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
        // TODO: shouldn't the XBindableValue supply a list of supported types, and we can distingusih
        // using this list? The current behavior below is somewhat hackish ...
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
                OSL_ENSURE( sal_False, "CellBindingHelper::isCellBindingAllowed: caught an exception!" );
                bAllow = false;
            }
        }
        return bAllow;
    }

    //------------------------------------------------------------------------
    bool CellBindingHelper::isCellBinding( const Reference< XValueBinding >& _rxBinding ) const
    {
        return doesComponentSupport( _rxBinding.get(), SERVICE_SHEET_CELL_BINDING );
    }

    //------------------------------------------------------------------------
    bool CellBindingHelper::isCellIntegerBinding( const Reference< XValueBinding >& _rxBinding ) const
    {
        return doesComponentSupport( _rxBinding.get(), SERVICE_SHEET_CELL_INT_BINDING );
    }

    //------------------------------------------------------------------------
    bool CellBindingHelper::isCellRangeListSource( const Reference< XListEntrySource >& _rxSource ) const
    {
        return doesComponentSupport( _rxSource.get(), SERVICE_SHEET_CELLRANGE_LISTSOURCE );
    }

    //------------------------------------------------------------------------
    bool CellBindingHelper::doesComponentSupport( const Reference< XInterface >& _rxComponent, const ::rtl::OUString& _rService ) const
    {
        bool bDoes = false;
        Reference< XServiceInfo > xSI( _rxComponent, UNO_QUERY );
        bDoes = xSI.is() && xSI->supportsService( _rService );
        return bDoes;
    }

    //------------------------------------------------------------------------
    Reference< XValueBinding > CellBindingHelper::getCurrentBinding( ) const
    {
        Reference< XValueBinding > xBinding;
        Reference< XBindableValue > xBindable( m_xControlModel, UNO_QUERY );
        if ( xBindable.is() )
            xBinding = xBindable->getValueBinding();
        return xBinding;
    }

    //------------------------------------------------------------------------
    Reference< XListEntrySource > CellBindingHelper::getCurrentListSource( ) const
    {
        Reference< XListEntrySource > xSource;
        Reference< XListEntrySink > xSink( m_xControlModel, UNO_QUERY );
        if ( xSink.is() )
            xSource = xSink->getListEntrySource();
        return xSource;
    }

    //------------------------------------------------------------------------
    void CellBindingHelper::setBinding( const Reference< XValueBinding >& _rxBinding )
    {
        Reference< XBindableValue > xBindable( m_xControlModel, UNO_QUERY );
        OSL_PRECOND( xBindable.is(), "CellBindingHelper::setBinding: the object is not bindable!" );
        if ( xBindable.is() )
            xBindable->setValueBinding( _rxBinding );
    }

    //------------------------------------------------------------------------
    void CellBindingHelper::setListSource( const Reference< XListEntrySource >& _rxSource )
    {
        Reference< XListEntrySink > xSink( m_xControlModel, UNO_QUERY );
        OSL_PRECOND( xSink.is(), "CellBindingHelper::setListSource: the object is no list entry sink!" );
        if ( xSink.is() )
            xSink->setListEntrySource( _rxSource );
    }

//............................................................................
}   // namespace pcr
//............................................................................
