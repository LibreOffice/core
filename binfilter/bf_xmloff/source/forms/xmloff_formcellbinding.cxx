/*************************************************************************
 *
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

#ifndef XMLOFF_FORMS_FORMCELLBINDING
#include "formcellbinding.hxx"
#endif

#ifndef _DRAFTS_COM_SUN_STAR_FORM_XBINDABLEVALUE_HPP_
#include <drafts/com/sun/star/form/XBindableValue.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_FORM_XLISTENTRYSINK_HPP_
#include <drafts/com/sun/star/form/XListEntrySink.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif
#ifndef _XMLOFF_FORMS_STRINGS_HXX_
#include "strings.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#include <functional>
#include <algorithm>
namespace binfilter {

//............................................................................
namespace xmloff
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
    using namespace ::drafts::com::sun::star::form;

    namespace
    {
        //....................................................................
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
                    return NULL;
            }
        }

        //....................................................................
        Reference< XModel > getDocument( const Reference< XInterface >& _rxModelNode )
        {
            return getTypedModelNode< XModel >( _rxModelNode );
        }

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
    }

    //========================================================================
    //= FormCellBindingHelper
    //========================================================================
    //------------------------------------------------------------------------
    FormCellBindingHelper::FormCellBindingHelper( const Reference< XPropertySet >& _rxControlModel, const Reference< XModel >& _rxDocument )
        :m_xControlModel( _rxControlModel )
        ,m_xDocument( _rxDocument, UNO_QUERY )
    {
        OSL_ENSURE( m_xControlModel.is(), "FormCellBindingHelper::FormCellBindingHelper: invalid control model!" );

        if ( !m_xDocument.is() )
            m_xDocument = m_xDocument.query( getDocument( m_xControlModel ) );
        OSL_ENSURE( m_xDocument.is(), "FormCellBindingHelper::FormCellBindingHelper: Did not find the spreadsheet document!" );
    }

    //------------------------------------------------------------------------
    sal_Bool FormCellBindingHelper::livesInSpreadsheetDocument( const Reference< XPropertySet >& _rxControlModel )
    {
        Reference< XSpreadsheetDocument > xDocument( getDocument( _rxControlModel ), UNO_QUERY );
        return xDocument.is();
    }

    //------------------------------------------------------------------------
    bool FormCellBindingHelper::convertStringAddress( const ::rtl::OUString& _rAddressDescription, CellAddress& /* [out] */ _rAddress, sal_Int16 _nAssumeSheet ) const
    {
        Any aAddress;
        return doConvertAddressRepresentations(
                    PROPERTY_FILE_REPRESENTATION,
                    makeAny( _rAddressDescription ),
                    PROPERTY_ADDRESS,
                    aAddress,
                    false
               )
           &&  ( aAddress >>= _rAddress );
    }

    //------------------------------------------------------------------------
    bool FormCellBindingHelper::convertStringAddress( const ::rtl::OUString& _rAddressDescription,
                            CellRangeAddress& /* [out] */ _rAddress ) const
    {
        Any aAddress;
        return doConvertAddressRepresentations(
                    PROPERTY_FILE_REPRESENTATION,
                    makeAny( _rAddressDescription ),
                    PROPERTY_ADDRESS,
                    aAddress,
                    true
               )
           &&  ( aAddress >>= _rAddress );
    }

    //------------------------------------------------------------------------
    Reference< XValueBinding > FormCellBindingHelper::createCellBindingFromStringAddress( const ::rtl::OUString& _rAddress, bool _bUseIntegerBinding ) const
    {
        Reference< XValueBinding > xBinding;
        if ( !m_xDocument.is() )
            // very bad ...
            return xBinding;

        // get the UNO representation of the address
        CellAddress aAddress;
        if ( !_rAddress.getLength() || !convertStringAddress( _rAddress, aAddress ) )
            return xBinding;

        xBinding = xBinding.query( createDocumentDependentInstance(
            _bUseIntegerBinding ? SERVICE_LISTINDEXCELLBINDING : SERVICE_CELLVALUEBINDING,
            PROPERTY_BOUND_CELL,
            makeAny( aAddress )
        ) );

        return xBinding;
    }

    //------------------------------------------------------------------------
    Reference< XListEntrySource > FormCellBindingHelper::createCellListSourceFromStringAddress( const ::rtl::OUString& _rAddress ) const
    {
        Reference< XListEntrySource > xSource;

        CellRangeAddress aRangeAddress;
        if ( !convertStringAddress( _rAddress, aRangeAddress ) )
            return xSource;

        // create a range object for this address
        xSource = xSource.query( createDocumentDependentInstance(
            SERVICE_CELLRANGELISTSOURCE,
            PROPERTY_LIST_CELL_RANGE,
            makeAny( aRangeAddress )
        ) );

        return xSource;
    }

    //------------------------------------------------------------------------
    ::rtl::OUString FormCellBindingHelper::getStringAddressFromCellBinding( const Reference< XValueBinding >& _rxBinding ) const
    {
        OSL_PRECOND( !_rxBinding.is() || isCellBinding( _rxBinding ), "FormCellBindingHelper::getStringAddressFromCellBinding: this is no cell binding!" );

        ::rtl::OUString sAddress;
        try
        {
            Reference< XPropertySet > xBindingProps( _rxBinding, UNO_QUERY );
            OSL_ENSURE( xBindingProps.is() || !_rxBinding.is(), "FormCellBindingHelper::getStringAddressFromCellBinding: no property set for the binding!" );
            if ( xBindingProps.is() )
            {
                CellAddress aAddress;
                xBindingProps->getPropertyValue( PROPERTY_BOUND_CELL ) >>= aAddress;

                Any aStringAddress;
                doConvertAddressRepresentations( PROPERTY_ADDRESS, makeAny( aAddress ),
                    PROPERTY_FILE_REPRESENTATION, aStringAddress, false );

                aStringAddress >>= sAddress;
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "FormCellBindingHelper::getStringAddressFromCellBinding: caught an exception!" );
        }

        return sAddress;
    }

    //------------------------------------------------------------------------
    ::rtl::OUString FormCellBindingHelper::getStringAddressFromCellListSource( const Reference< XListEntrySource >& _rxSource ) const
    {
        OSL_PRECOND( !_rxSource.is() || isCellRangeListSource( _rxSource ), "FormCellBindingHelper::getStringAddressFromCellListSource: this is no cell list source!" );

        ::rtl::OUString sAddress;
        try
        {
            Reference< XPropertySet > xSourceProps( _rxSource, UNO_QUERY );
            OSL_ENSURE( xSourceProps.is() || !_rxSource.is(), "FormCellBindingHelper::getStringAddressFromCellListSource: no property set for the list source!" );
            if ( xSourceProps.is() )
            {
                CellRangeAddress aRangeAddress;
                xSourceProps->getPropertyValue( PROPERTY_LIST_CELL_RANGE ) >>= aRangeAddress;

                Any aStringAddress;
                doConvertAddressRepresentations( PROPERTY_ADDRESS, makeAny( aRangeAddress ),
                    PROPERTY_FILE_REPRESENTATION, aStringAddress, true );
                aStringAddress >>= sAddress;
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "FormCellBindingHelper::getStringAddressFromCellListSource: caught an exception!" );
        }

        return sAddress;
    }

    //------------------------------------------------------------------------
    bool FormCellBindingHelper::isSpreadsheetDocumentWhichSupplies( const Reference< XSpreadsheetDocument >& _rxDocument, const ::rtl::OUString& _rService ) SAL_THROW(())
    {
        bool bYesItIs = false;

        try
        {
            Reference< XServiceInfo > xSI( _rxDocument, UNO_QUERY );
            if ( xSI.is() && xSI->supportsService( SERVICE_SPREADSHEET_DOCUMENT ) )
            {
                Reference< XMultiServiceFactory > xDocumentFactory( _rxDocument, UNO_QUERY );
                OSL_ENSURE( xDocumentFactory.is(), "FormCellBindingHelper::isSpreadsheetDocumentWhichSupplies: spreadsheet document, but no factory?" );

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
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "FormCellBindingHelper::isSpreadsheetDocumentWhichSupplies: caught an exception!" );
        }

        return bYesItIs;
    }

    //------------------------------------------------------------------------
    bool FormCellBindingHelper::isSpreadsheetDocumentWhichSupplies( const ::rtl::OUString& _rService ) const SAL_THROW(())
    {
        return isSpreadsheetDocumentWhichSupplies( m_xDocument, _rService );
    }

    //------------------------------------------------------------------------
    bool FormCellBindingHelper::isListCellRangeAllowed( const Reference< XModel >& _rxDocument )
    {
        return isSpreadsheetDocumentWhichSupplies(
            Reference< XSpreadsheetDocument >( _rxDocument, UNO_QUERY ),
            SERVICE_CELLRANGELISTSOURCE
        );
    }

    //------------------------------------------------------------------------
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

    //------------------------------------------------------------------------
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

    //------------------------------------------------------------------------
    bool FormCellBindingHelper::isCellBindingAllowed( const Reference< XModel >& _rxDocument )
    {
        return isSpreadsheetDocumentWhichSupplies(
            Reference< XSpreadsheetDocument >( _rxDocument, UNO_QUERY ),
            SERVICE_CELLVALUEBINDING
        );
    }

    //------------------------------------------------------------------------
    bool FormCellBindingHelper::isCellBinding( const Reference< XValueBinding >& _rxBinding ) const
    {
        return doesComponentSupport( _rxBinding.get(), SERVICE_CELLVALUEBINDING );
    }

    //------------------------------------------------------------------------
    bool FormCellBindingHelper::isCellIntegerBinding( const Reference< XValueBinding >& _rxBinding ) const
    {
        return doesComponentSupport( _rxBinding.get(), SERVICE_LISTINDEXCELLBINDING );
    }

    //------------------------------------------------------------------------
    bool FormCellBindingHelper::isCellRangeListSource( const Reference< XListEntrySource >& _rxSource ) const
    {
        return doesComponentSupport( _rxSource.get(), SERVICE_CELLRANGELISTSOURCE );
    }

    //------------------------------------------------------------------------
    bool FormCellBindingHelper::doesComponentSupport( const Reference< XInterface >& _rxComponent, const ::rtl::OUString& _rService ) const
    {
        bool bDoes = false;
        Reference< XServiceInfo > xSI( _rxComponent, UNO_QUERY );
        bDoes = xSI.is() && xSI->supportsService( _rService );
        return bDoes;
    }

    //------------------------------------------------------------------------
    Reference< XValueBinding > FormCellBindingHelper::getCurrentBinding( ) const
    {
        Reference< XValueBinding > xBinding;
        Reference< XBindableValue > xBindable( m_xControlModel, UNO_QUERY );
        if ( xBindable.is() )
            xBinding = xBindable->getValueBinding();
        return xBinding;
    }

    //------------------------------------------------------------------------
    Reference< XListEntrySource > FormCellBindingHelper::getCurrentListSource( ) const
    {
        Reference< XListEntrySource > xSource;
        Reference< XListEntrySink > xSink( m_xControlModel, UNO_QUERY );
        if ( xSink.is() )
            xSource = xSink->getListEntrySource();
        return xSource;
    }

    //------------------------------------------------------------------------
    void FormCellBindingHelper::setBinding( const Reference< XValueBinding >& _rxBinding )
    {
        Reference< XBindableValue > xBindable( m_xControlModel, UNO_QUERY );
        OSL_PRECOND( xBindable.is(), "FormCellBindingHelper::setBinding: the object is not bindable!" );
        if ( xBindable.is() )
            xBindable->setValueBinding( _rxBinding );
    }

    //------------------------------------------------------------------------
    void FormCellBindingHelper::setListSource( const Reference< XListEntrySource >& _rxSource )
    {
        Reference< XListEntrySink > xSink( m_xControlModel, UNO_QUERY );
        OSL_PRECOND( xSink.is(), "FormCellBindingHelper::setListSource: the object is no list entry sink!" );
        if ( xSink.is() )
            xSink->setListEntrySource( _rxSource );
    }

    //------------------------------------------------------------------------
    Reference< XInterface > FormCellBindingHelper::createDocumentDependentInstance( const ::rtl::OUString& _rService, const ::rtl::OUString& _rArgumentName,
        const Any& _rArgumentValue ) const
    {
        Reference< XInterface > xReturn;

        Reference< XMultiServiceFactory > xDocumentFactory( m_xDocument, UNO_QUERY );
        OSL_ENSURE( xDocumentFactory.is(), "FormCellBindingHelper::createDocumentDependentInstance: no document service factory!" );
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
                OSL_ENSURE( sal_False, "FormCellBindingHelper::createDocumentDependentInstance: could not create the binding at the document!" );
            }
        }
        return xReturn;
    }

    //------------------------------------------------------------------------
    bool FormCellBindingHelper::doConvertAddressRepresentations( const ::rtl::OUString& _rInputProperty, const Any& _rInputValue,
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
                OSL_ENSURE( sal_False, "FormCellBindingHelper::doConvertAddressRepresentations: caught an exception!" );
            }
        }

        return bSuccess;
    }

//............................................................................
}   // namespace xmloff
//............................................................................
}//end of namespace binfilter
