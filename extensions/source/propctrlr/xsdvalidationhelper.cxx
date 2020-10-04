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

#include "xsdvalidationhelper.hxx"
#include "xsddatatypes.hxx"
#include "formstrings.hxx"

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/xsd/DataTypeClass.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/xforms/XDataTypeRepository.hpp>
#include <unotools/syslocale.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <tools/diagnose_ex.h>


namespace pcr
{


    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::xsd;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::xforms;

    namespace NumberFormat = ::com::sun::star::util::NumberFormat;


    //= XSDValidationHelper


    XSDValidationHelper::XSDValidationHelper( ::osl::Mutex& _rMutex, const Reference< XPropertySet >& _rxIntrospectee, const Reference< frame::XModel >& _rxContextDocument )
        :EFormsHelper( _rMutex, _rxIntrospectee, _rxContextDocument )
        ,m_bInspectingFormattedField( false )
    {
        try
        {
            Reference< XPropertySetInfo > xPSI;
            Reference< XServiceInfo >     xSI( _rxIntrospectee, UNO_QUERY );
            if ( m_xControlModel.is() )
                xPSI = m_xControlModel->getPropertySetInfo();
            if  (   xPSI.is()
                &&  xPSI->hasPropertyByName( PROPERTY_FORMATKEY )
                &&  xPSI->hasPropertyByName( PROPERTY_FORMATSSUPPLIER )
                &&  xSI.is()
                &&  xSI->supportsService( SERVICE_COMPONENT_FORMATTEDFIELD )
                )
                m_bInspectingFormattedField = true;
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION("extensions.propctrlr",
                                 "caught an exception while examining the introspectee!");
        }
    }


    void XSDValidationHelper::getAvailableDataTypeNames( std::vector< OUString >& /* [out] */ _rNames ) const
    {
        _rNames.resize( 0 );

        try
        {
            Reference< XDataTypeRepository > xRepository = getDataTypeRepository();
            Sequence< OUString > aElements;
            if ( xRepository.is() )
                aElements = xRepository->getElementNames();

            _rNames.resize( aElements.getLength() );
            std::copy( aElements.begin(), aElements.end(), _rNames.begin() );
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "extensions.propctrlr", "XSDValidationHelper::getAvailableDataTypeNames" );
        }
    }


    Reference< XDataTypeRepository > XSDValidationHelper::getDataTypeRepository() const
    {
        Reference< XDataTypeRepository > xRepository;

        Reference< xforms::XModel > xModel( getCurrentFormModel( ) );
        if ( xModel.is() )
            xRepository = xModel->getDataTypeRepository();

        return xRepository;
    }


    Reference< XDataTypeRepository > XSDValidationHelper::getDataTypeRepository( const OUString& _rModelName ) const
    {
        Reference< XDataTypeRepository > xRepository;

        Reference< xforms::XModel > xModel( getFormModelByName( _rModelName ) );
        if ( xModel.is() )
            xRepository = xModel->getDataTypeRepository();

        return xRepository;
    }


    Reference< XDataType > XSDValidationHelper::getDataType( const OUString& _rName ) const
    {
        Reference< XDataType > xDataType;

        if ( !_rName.isEmpty() )
        {
            Reference< XDataTypeRepository > xRepository = getDataTypeRepository();
            if ( xRepository.is() )
                xDataType = xRepository->getDataType( _rName );
        }
        return xDataType;
    }


    OUString XSDValidationHelper::getValidatingDataTypeName( ) const
    {
        OUString sDataTypeName;
        try
        {
            Reference< XPropertySet > xBinding( getCurrentBinding() );
            // it's allowed here to not (yet) have a binding
            if ( xBinding.is() )
            {
                OSL_VERIFY( xBinding->getPropertyValue( PROPERTY_XSD_DATA_TYPE ) >>= sDataTypeName );
            }
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "extensions.propctrlr", "XSDValidationHelper::getValidatingDataTypeName" );
        }
        return sDataTypeName;
    }


    ::rtl::Reference< XSDDataType > XSDValidationHelper::getDataTypeByName( const OUString& _rName ) const
    {
        ::rtl::Reference< XSDDataType > pReturn;

        try
        {
            Reference< XDataType > xValidatedAgainst;

            if ( !_rName.isEmpty() )
                xValidatedAgainst = getDataType( _rName );

            if ( xValidatedAgainst.is() )
                pReturn = new XSDDataType( xValidatedAgainst );
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "extensions.propctrlr", "XSDValidationHelper::getDataTypeByName" );
        }

        return pReturn;
    }


    ::rtl::Reference< XSDDataType > XSDValidationHelper::getValidatingDataType( ) const
    {
        return getDataTypeByName( getValidatingDataTypeName() );
    }


    bool XSDValidationHelper::cloneDataType( const ::rtl::Reference< XSDDataType >& _pDataType, const OUString& _rNewName ) const
    {
        OSL_ENSURE( _pDataType.is(), "XSDValidationHelper::removeDataTypeFromRepository: invalid data type!" );
        if ( !_pDataType.is() )
            return false;

        try
        {
            Reference< XDataTypeRepository > xRepository( getDataTypeRepository() );
            OSL_ENSURE( xRepository.is(), "XSDValidationHelper::removeDataTypeFromRepository: invalid data type repository!" );
            if ( !xRepository.is() )
                return false;

            Reference< XDataType > xDataType( _pDataType->getUnoDataType() );
            OSL_ENSURE( xDataType.is(), "XSDValidationHelper::removeDataTypeFromRepository: invalid data type (II)!" );
            if ( !xDataType.is() )
                return false;

            xRepository->cloneDataType( xDataType->getName(), _rNewName );
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "extensions.propctrlr", "XSDValidationHelper::cloneDataType" );
        }
        return true;
    }


    bool XSDValidationHelper::removeDataTypeFromRepository( const OUString& _rName ) const
    {
        try
        {
            Reference< XDataTypeRepository > xRepository( getDataTypeRepository() );
            OSL_ENSURE( xRepository.is(), "XSDValidationHelper::removeDataTypeFromRepository: invalid data type repository!" );
            if ( !xRepository.is() )
                return false;

            if ( !xRepository->hasByName( _rName ) )
            {
                OSL_FAIL( "XSDValidationHelper::removeDataTypeFromRepository: invalid repository and/or data type!" );
                return false;
            }

            xRepository->revokeDataType( _rName );
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "extensions.propctrlr", "XSDValidationHelper::removeDataTypeFromRepository" );
            return false;
        }
        return true;
    }


    void XSDValidationHelper::setValidatingDataTypeByName( const OUString& _rName ) const
    {
        try
        {
            Reference< XPropertySet > xBinding( getCurrentBinding() );
            OSL_ENSURE( xBinding.is(), "XSDValidationHelper::setValidatingDataTypeByName: no active binding - how this?" );

            if ( xBinding.is() )
            {
                // get the old data type - this is necessary for notifying property changes
                OUString sOldDataTypeName;
                OSL_VERIFY( xBinding->getPropertyValue( PROPERTY_XSD_DATA_TYPE ) >>= sOldDataTypeName );
                Reference< XPropertySet > xOldType;
                try {
                    xOldType = getDataType( sOldDataTypeName );
                } catch( const Exception& ) { }

                // set the new data type name
                xBinding->setPropertyValue( PROPERTY_XSD_DATA_TYPE, makeAny( _rName ) );

                // retrieve the new data type object
                Reference< XPropertySet > xNewType = getDataType( _rName );

                // fire any changes in the properties which result from this new type
                std::set< OUString > aFilter; aFilter.insert( PROPERTY_NAME );
                firePropertyChanges( xOldType, xNewType, aFilter );

                // fire the change in the Data Type property
                OUString sNewDataTypeName;
                OSL_VERIFY( xBinding->getPropertyValue( PROPERTY_XSD_DATA_TYPE ) >>= sNewDataTypeName );
                firePropertyChange( PROPERTY_XSD_DATA_TYPE, makeAny( sOldDataTypeName ), makeAny( sNewDataTypeName ) );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("extensions.propctrlr");
        }
    }


    void XSDValidationHelper::copyDataType( const OUString& _rFromModel, const OUString& _rToModel,
                const OUString& _rDataTypeName ) const
    {
        if ( _rFromModel == _rToModel )
            // nothing to do (me thinks)
            return;

        try
        {
            Reference< XDataTypeRepository > xFromRepository, xToRepository;
            if ( !_rFromModel.isEmpty() )
                xFromRepository = getDataTypeRepository( _rFromModel );
            if ( !_rToModel.isEmpty() )
                xToRepository = getDataTypeRepository( _rToModel );

            if ( !xFromRepository.is() || !xToRepository.is() )
                return;

            if ( !xFromRepository->hasByName( _rDataTypeName ) || xToRepository->hasByName( _rDataTypeName ) )
                // not existent in the source, or already existent (by name) in the destination
                return;

            // determine the built-in type belonging to the source type
            ::rtl::Reference< XSDDataType > pSourceType = new XSDDataType( xFromRepository->getDataType( _rDataTypeName ) );
            OUString sTargetBaseType = getBasicTypeNameForClass( pSourceType->classify(), xToRepository );

            // create the target type
            Reference< XDataType > xTargetType = xToRepository->cloneDataType( sTargetBaseType, _rDataTypeName );
            ::rtl::Reference< XSDDataType > pTargetType = new XSDDataType( xTargetType );

            // copy the facets
            pTargetType->copyFacetsFrom( pSourceType );
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "extensions.propctrlr", "XSDValidationHelper::copyDataType" );
        }
    }


    void XSDValidationHelper::findDefaultFormatForIntrospectee()
    {
        try
        {
            ::rtl::Reference< XSDDataType > xDataType = getValidatingDataType();
            if ( xDataType.is() )
            {
                // find a NumberFormat type corresponding to the DataTypeClass
                sal_Int16 nNumberFormatType = NumberFormat::NUMBER;
                switch ( xDataType->classify() )
                {
                case DataTypeClass::DATETIME:
                    nNumberFormatType = NumberFormat::DATETIME;
                    break;
                case DataTypeClass::DATE:
                    nNumberFormatType = NumberFormat::DATE;
                    break;
                case DataTypeClass::TIME:
                    nNumberFormatType = NumberFormat::TIME;
                    break;
                case DataTypeClass::STRING:
                case DataTypeClass::anyURI:
                case DataTypeClass::QName:
                case DataTypeClass::NOTATION:
                    nNumberFormatType = NumberFormat::TEXT;
                    break;
                }

                // get the number formatter from the introspectee
                Reference< XNumberFormatsSupplier > xSupplier;
                Reference< XNumberFormatTypes > xFormatTypes;
                OSL_VERIFY( m_xControlModel->getPropertyValue( PROPERTY_FORMATSSUPPLIER ) >>= xSupplier );
                if ( xSupplier.is() )
                    xFormatTypes.set(xSupplier->getNumberFormats(), css::uno::UNO_QUERY);
                OSL_ENSURE( xFormatTypes.is(), "XSDValidationHelper::findDefaultFormatForIntrospectee: no number formats for the introspectee!" );
                if ( !xFormatTypes.is() )
                    return;

                // and the standard format for the given NumberFormat type
                sal_Int32 nDesiredFormat = xFormatTypes->getStandardFormat( nNumberFormatType, SvtSysLocale().GetLanguageTag().getLocale() );

                // set this at the introspectee
                m_xControlModel->setPropertyValue( PROPERTY_FORMATKEY, makeAny( nDesiredFormat ) );
            }
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "extensions.propctrlr", "XSDValidationHelper::findDefaultFormatForIntrospectee" );
        }
    }


    OUString XSDValidationHelper::getBasicTypeNameForClass( sal_Int16 _nClass ) const
    {
        return getBasicTypeNameForClass( _nClass, getDataTypeRepository() );
    }


    OUString XSDValidationHelper::getBasicTypeNameForClass( sal_Int16 _nClass, const Reference< XDataTypeRepository >& _rxRepository )
    {
        OUString sReturn;
        OSL_ENSURE( _rxRepository.is(), "XSDValidationHelper::getBasicTypeNameForClass: invalid repository!" );
        if ( !_rxRepository.is() )
            return sReturn;

        try
        {
            Reference< XDataType > xDataType = _rxRepository->getBasicDataType( _nClass );
            OSL_ENSURE( xDataType.is(), "XSDValidationHelper::getBasicTypeNameForClass: invalid data type returned!" );
            if ( xDataType.is() )
                sReturn = xDataType->getName();
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "extensions.propctrlr", "XSDValidationHelper::getBasicTypeNameForClass" );
        }

        return sReturn;
    }


} // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
