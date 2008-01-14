/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xsdvalidationhelper.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 15:02:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_XSDVALIDATIONHELPER_HXX
#include "xsdvalidationhelper.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_XSDDATATYPES_HXX
#include "xsddatatypes.hxx"
#endif
#ifndef _EXTENSIONS_FORMSCTRLR_FORMSTRINGS_HXX_
#include "formstrings.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_XSD_DATATYPECLASS_HPP_
#include <com/sun/star/xsd/DataTypeClass.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_NUMBERFORMAT_HPP_
#include <com/sun/star/util/NumberFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTYPES_HPP_
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_XFORMS_XDATATYPEREPOSITORY_HPP_
#include <com/sun/star/xforms/XDataTypeRepository.hpp>
#endif
/** === end UNO includes === **/

#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <svtools/syslocale.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif

//........................................................................
namespace pcr
{
//........................................................................

    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::xsd;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::xforms;

    namespace NumberFormat = ::com::sun::star::util::NumberFormat;

    //====================================================================
    //= XSDValidationHelper
    //====================================================================
    //--------------------------------------------------------------------
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
            OSL_ENSURE( sal_False, "XSDValidationHelper::XSDValidationHelper: caught an exception while examining the introspectee!" );
        }
    }

    //--------------------------------------------------------------------
    void XSDValidationHelper::getAvailableDataTypeNames( ::std::vector< ::rtl::OUString >& /* [out] */ _rNames ) const SAL_THROW(())
    {
        _rNames.resize( 0 );

        try
        {
            Reference< XDataTypeRepository > xRepository = getDataTypeRepository();
            Sequence< ::rtl::OUString > aElements;
            if ( xRepository.is() )
                aElements = xRepository->getElementNames();

            _rNames.resize( aElements.getLength() );
            ::std::copy( aElements.getConstArray(), aElements.getConstArray() + aElements.getLength(), _rNames.begin() );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "XSDValidationHelper::getAvailableDataTypeNames: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    Reference< XDataTypeRepository > XSDValidationHelper::getDataTypeRepository() const SAL_THROW((Exception))
    {
        Reference< XDataTypeRepository > xRepository;

        Reference< xforms::XModel > xModel( getCurrentFormModel( ) );
        if ( xModel.is() )
            xRepository = xModel->getDataTypeRepository();

        return xRepository;
    }

    //--------------------------------------------------------------------
    Reference< XDataTypeRepository > XSDValidationHelper::getDataTypeRepository( const ::rtl::OUString& _rModelName ) const SAL_THROW((Exception))
    {
        Reference< XDataTypeRepository > xRepository;

        Reference< xforms::XModel > xModel( getFormModelByName( _rModelName ) );
        if ( xModel.is() )
            xRepository = xModel->getDataTypeRepository();

        return xRepository;
    }

    //--------------------------------------------------------------------
    Reference< XDataType > XSDValidationHelper::getDataType( const ::rtl::OUString& _rName ) const SAL_THROW((Exception))
    {
        Reference< XDataType > xDataType;

        if ( _rName.getLength() )
        {
            Reference< XDataTypeRepository > xRepository = getDataTypeRepository();
            if ( xRepository.is() )
                xDataType = xRepository->getDataType( _rName );
        }
        return xDataType;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString XSDValidationHelper::getValidatingDataTypeName( ) const SAL_THROW(())
    {
        ::rtl::OUString sDataTypeName;
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
            OSL_ENSURE( sal_False, "XSDValidationHelper::getValidatingDataTypeName: caught an exception!" );
        }
        return sDataTypeName;
    }

    //--------------------------------------------------------------------
    ::rtl::Reference< XSDDataType > XSDValidationHelper::getDataTypeByName( const ::rtl::OUString& _rName ) const SAL_THROW(())
    {
        ::rtl::Reference< XSDDataType > pReturn;

        try
        {
            Reference< XDataType > xValidatedAgainst;

            if ( _rName.getLength() )
                xValidatedAgainst = getDataType( _rName );

            if ( xValidatedAgainst.is() )
                pReturn = new XSDDataType( xValidatedAgainst );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "XSDValidationHelper::getDataTypeByName: caught an exception!" );
        }

        return pReturn;
    }

    //--------------------------------------------------------------------
    ::rtl::Reference< XSDDataType > XSDValidationHelper::getValidatingDataType( ) const SAL_THROW(())
    {
        return getDataTypeByName( getValidatingDataTypeName() );
    }

    //--------------------------------------------------------------------
    bool XSDValidationHelper::cloneDataType( const ::rtl::Reference< XSDDataType >& _pDataType, const ::rtl::OUString& _rNewName ) const SAL_THROW(())
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
            OSL_ENSURE( sal_False, "XSDValidationHelper::cloneDataType: caught an exception!" );
        }
        return true;
    }

    //--------------------------------------------------------------------
    bool XSDValidationHelper::removeDataTypeFromRepository( const ::rtl::OUString& _rName ) const SAL_THROW(())
    {
        try
        {
            Reference< XDataTypeRepository > xRepository( getDataTypeRepository() );
            OSL_ENSURE( xRepository.is(), "XSDValidationHelper::removeDataTypeFromRepository: invalid data type repository!" );
            if ( !xRepository.is() )
                return false;

            if ( !xRepository->hasByName( _rName ) )
            {
                OSL_ENSURE( sal_False, "XSDValidationHelper::removeDataTypeFromRepository: invalid repository and/or data type!" );
                return false;
            }

            xRepository->revokeDataType( _rName );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "XSDValidationHelper::removeDataTypeFromRepository: caught an exception!" );
            return false;
        }
        return true;
    }

    //--------------------------------------------------------------------
    void XSDValidationHelper::setValidatingDataTypeByName( const ::rtl::OUString& _rName ) const SAL_THROW(())
    {
        try
        {
            Reference< XPropertySet > xBinding( getCurrentBinding() );
            OSL_ENSURE( xBinding.is(), "XSDValidationHelper::setValidatingDataTypeByName: no active binding - how this?" );

            if ( xBinding.is() )
            {
                // get the old data type - this is necessary for notifying property changes
                ::rtl::OUString sOldDataTypeName;
                OSL_VERIFY( xBinding->getPropertyValue( PROPERTY_XSD_DATA_TYPE ) >>= sOldDataTypeName );
                Reference< XPropertySet > xOldType;
                try { xOldType = xOldType.query( getDataType( sOldDataTypeName ) ); } catch( const Exception& ) { }

                // set the new data type name
                xBinding->setPropertyValue( PROPERTY_XSD_DATA_TYPE, makeAny( _rName ) );

                // retrieve the new data type object
                Reference< XPropertySet > xNewType( getDataType( _rName ), UNO_QUERY );

                // fire any changes in the properties which result from this new type
                std::set< ::rtl::OUString > aFilter; aFilter.insert( PROPERTY_NAME );
                firePropertyChanges( xOldType, xNewType, aFilter );

                // fire the change in the Data Type property
                ::rtl::OUString sNewDataTypeName;
                OSL_VERIFY( xBinding->getPropertyValue( PROPERTY_XSD_DATA_TYPE ) >>= sNewDataTypeName );
                firePropertyChange( PROPERTY_XSD_DATA_TYPE, makeAny( sOldDataTypeName ), makeAny( sNewDataTypeName ) );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //--------------------------------------------------------------------
    void XSDValidationHelper::copyDataType( const ::rtl::OUString& _rFromModel, const ::rtl::OUString& _rToModel,
                const ::rtl::OUString& _rDataTypeName ) const SAL_THROW(())
    {
        if ( _rFromModel == _rToModel )
            // nothing to do (me thinks)
            return;

        try
        {
            Reference< XDataTypeRepository > xFromRepository, xToRepository;
            if ( _rFromModel.getLength() )
                xFromRepository = getDataTypeRepository( _rFromModel );
            if ( _rToModel.getLength() )
                xToRepository = getDataTypeRepository( _rToModel );

            if ( !xFromRepository.is() || !xToRepository.is() )
                return;

            if ( !xFromRepository->hasByName( _rDataTypeName ) || xToRepository->hasByName( _rDataTypeName ) )
                // not existent in the source, or already existent (by name) in the destination
                return;

            // determine the built-in type belonging to the source type
            ::rtl::Reference< XSDDataType > pSourceType = new XSDDataType( xFromRepository->getDataType( _rDataTypeName ) );
            ::rtl::OUString sTargetBaseType = getBasicTypeNameForClass( pSourceType->classify(), xToRepository );

            // create the target type
            Reference< XDataType > xTargetType = xToRepository->cloneDataType( sTargetBaseType, _rDataTypeName );
            ::rtl::Reference< XSDDataType > pTargetType = new XSDDataType( xTargetType );

            // copy the facets
            pTargetType->copyFacetsFrom( pSourceType );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "XSDValidationHelper::copyDataType: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    void XSDValidationHelper::findDefaultFormatForIntrospectee() SAL_THROW(())
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
                    xFormatTypes = xFormatTypes.query( xSupplier->getNumberFormats() );
                OSL_ENSURE( xFormatTypes.is(), "XSDValidationHelper::findDefaultFormatForIntrospectee: no number formats for the introspectee!" );
                if ( !xFormatTypes.is() )
                    return;

                // and the standard format for the given NumberFormat type
                sal_Int32 nDesiredFormat = xFormatTypes->getStandardFormat( nNumberFormatType, SvtSysLocale().GetLocaleData().getLocale() );

                // set this at the introspectee
                m_xControlModel->setPropertyValue( PROPERTY_FORMATKEY, makeAny( nDesiredFormat ) );
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "XSDValidationHelper::findDefaultFormatForIntrospectee: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    ::rtl::OUString XSDValidationHelper::getBasicTypeNameForClass( sal_Int16 _nClass ) const SAL_THROW(())
    {
        return getBasicTypeNameForClass( _nClass, getDataTypeRepository() );
    }

    //--------------------------------------------------------------------
    ::rtl::OUString XSDValidationHelper::getBasicTypeNameForClass( sal_Int16 _nClass, Reference< XDataTypeRepository > _rxRepository ) const SAL_THROW(())
    {
        ::rtl::OUString sReturn;
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
            OSL_ENSURE( sal_False, "XSDValidationHelper::getBasicTypeNameForClass: caught an exception!" );
        }

        return sReturn;
    }

//........................................................................
} // namespace pcr
//........................................................................

