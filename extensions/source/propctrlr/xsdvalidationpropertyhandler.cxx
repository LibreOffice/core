/*************************************************************************
 *
 *  $RCSfile: xsdvalidationpropertyhandler.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 12:14:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_XSDVALIDATIONPROPERTYHANDLER_HXX
#include "xsdvalidationpropertyhandler.hxx"
#endif
#ifndef _EXTENSIONS_FORMSCTRLR_FORMSTRINGS_HXX_
#include "formstrings.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_FORMMETADATA_HXX_
#include "formmetadata.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_LINEDESCRIPTOR_HXX_
#include "linedescriptor.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_XSDDATATYPES_HXX
#include "xsddatatypes.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_MODULEPRC_HXX_
#include "modulepcr.hxx"
#endif
#ifndef _EXTENSIONS_FORMCTRLR_PROPRESID_HRC_
#include "formresid.hrc"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_FORMLOCALID_HRC_
#include "formlocalid.hrc"
#endif
#ifndef EXTENSIONS_INC_EXTENSIO_HRC
#include "extensio.hrc"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_STRINGREPRESENTATION_HXX
#include "stringrepresentation.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_NEWDATATYPE_HXX
#include "newdatatype.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_PROPBROWSERUI_HXX
#include "propbrowserui.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_XSDVALIDATIONHELPER_HXX
#include "xsdvalidationhelper.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_XSD_WHITESPACETREATMENT_HPP_
#include <com/sun/star/xsd/WhiteSpaceTreatment.hpp>
#endif
#ifndef _COM_SUN_STAR_XSD_DATATYPECLASS_HPP_
#include <com/sun/star/xsd/DataTypeClass.hpp>
#endif
/** === end UNO includes === **/

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SVTOOLS_LOCALRESACCESS_HXX_
#include <svtools/localresaccess.hxx>
#endif

#include <algorithm>
#include <functional>
#include <limits>

//........................................................................
namespace pcr
{
//........................................................................

    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::xforms;
    using namespace ::com::sun::star::xsd;
    using namespace ::com::sun::star::script;

    using ::com::sun::star::beans::PropertyAttribute::MAYBEVOID;

    //====================================================================
    //= XSDValidationPropertyHandler
    //====================================================================
    //--------------------------------------------------------------------
    XSDValidationPropertyHandler::XSDValidationPropertyHandler( const Reference< XPropertySet >& _rxIntrospectee,
        const Reference< frame::XModel >& _rxContextDocument, const Reference< XTypeConverter >& _rxTypeConverter )
        :PropertyHandler( _rxIntrospectee, _rxTypeConverter )
    {
        if ( EFormsHelper::isEForm( _rxContextDocument ) )
            m_pHelper.reset( new XSDValidationHelper( m_xIntrospectee, _rxContextDocument ) );
    }

    //--------------------------------------------------------------------
    XSDValidationPropertyHandler::~XSDValidationPropertyHandler()
    {
    }

    //--------------------------------------------------------------------
    bool SAL_CALL XSDValidationPropertyHandler::supportsUIDescriptor(  PropertyId _nPropId ) const
    {
        return true;
    }

    //--------------------------------------------------------------------
    Any SAL_CALL XSDValidationPropertyHandler::getPropertyValue( PropertyId _nPropId, bool _bLazy ) const
    {
        Any aReturn;

        OSL_ENSURE( m_pHelper.get(), "XSDValidationPropertyHandler::getPropertyValue: we don't have any SupportedProperties!" );
        if ( !m_pHelper.get() )
            return aReturn;

        ::rtl::Reference< XSDDataType > pType = m_pHelper->getValidatingDataType();

        switch ( _nPropId )
        {
        // common facets
        case PROPERTY_ID_XSD_DATA_TYPE:  aReturn = pType.is() ? pType->getFacet( PROPERTY_NAME ) : makeAny( ::rtl::OUString() ); break;
        case PROPERTY_ID_XSD_WHITESPACES:aReturn = pType.is() ? pType->getFacet( PROPERTY_XSD_WHITESPACES ) : makeAny( WhiteSpaceTreatment::Preserve ); break;
        case PROPERTY_ID_XSD_PATTERN:    aReturn = pType.is() ? pType->getFacet( PROPERTY_XSD_PATTERN ) : makeAny( ::rtl::OUString() ); break;

        // all other properties are simply forwarded, if they exist at the given type
        default:
        {
            ::rtl::OUString sFacetName = getPropertyNameFromId( _nPropId );
            if ( pType.is() && pType->hasFacet( sFacetName ) )
                aReturn = pType->getFacet( sFacetName );
        }
        break;
        }

        return aReturn;
    }

    //--------------------------------------------------------------------
    void SAL_CALL XSDValidationPropertyHandler::setPropertyValue( PropertyId _nPropId, const Any& _rValue )
    {
        OSL_ENSURE( m_pHelper.get(), "XSDValidationPropertyHandler::setPropertyValue: we don't have any SupportedProperties!" );
        if ( !m_pHelper.get() )
            return;

        if ( PROPERTY_ID_XSD_DATA_TYPE == _nPropId )
        {
            ::rtl::OUString sTypeName;
            OSL_VERIFY( _rValue >>= sTypeName );
            m_pHelper->setValidatingDataTypeByName( sTypeName );
            return;
        }

        ::rtl::Reference< XSDDataType > pType = m_pHelper->getValidatingDataType();
        if ( !pType.is() )
        {
            DBG_ERROR( "XSDValidationPropertyHandler::setPropertyValue: you're trying to set a type facet, without a current type!" );
            return;
        }

        pType->setFacet( getPropertyNameFromId( _nPropId ), _rValue );
    }

    //--------------------------------------------------------------------
    ::std::vector< Property > XSDValidationPropertyHandler::implDescribeSupportedProperties() const
    {
        ::std::vector< Property > aProperties;

        if ( m_pHelper.get() )
        {
            bool bAllowBinding = m_pHelper->canBindToAnyDataType();

            if ( bAllowBinding )
            {
                aProperties.reserve( 12 );

                addStringPropertyDescription( aProperties, PROPERTY_XSD_DATA_TYPE   );
                addInt16PropertyDescription ( aProperties, PROPERTY_XSD_WHITESPACES );
                addStringPropertyDescription( aProperties, PROPERTY_XSD_PATTERN     );

                // string facets
                addInt32PropertyDescription( aProperties, PROPERTY_XSD_LENGTH,          MAYBEVOID );
                addInt32PropertyDescription( aProperties, PROPERTY_XSD_MIN_LENGTH,      MAYBEVOID );
                addInt32PropertyDescription( aProperties, PROPERTY_XSD_MAX_LENGTH,      MAYBEVOID );

                // decimal facets
                addInt32PropertyDescription( aProperties, PROPERTY_XSD_TOTAL_DIGITS,    MAYBEVOID );
                addInt32PropertyDescription( aProperties, PROPERTY_XSD_FRACTION_DIGITS, MAYBEVOID );

                // facets common for different types
                addDoublePropertyDescription( aProperties, PROPERTY_XSD_MAX_INCLUSIVE, MAYBEVOID );
                addDoublePropertyDescription( aProperties, PROPERTY_XSD_MAX_EXCLUSIVE, MAYBEVOID );
                addDoublePropertyDescription( aProperties, PROPERTY_XSD_MIN_INCLUSIVE, MAYBEVOID );
                addDoublePropertyDescription( aProperties, PROPERTY_XSD_MIN_EXCLUSIVE, MAYBEVOID );
            }
        }

        return aProperties;
    }

    //--------------------------------------------------------------------
    ::std::vector< ::rtl::OUString > XSDValidationPropertyHandler::getSupersededProperties( ) const
    {
        ::std::vector< ::rtl::OUString > aSuperfluous;

        if ( m_pHelper.get() )
        {
            aSuperfluous.push_back( PROPERTY_CONTROLSOURCE );
            aSuperfluous.push_back( PROPERTY_EMPTY_IS_NULL );
            aSuperfluous.push_back( PROPERTY_FILTERPROPOSAL );
            aSuperfluous.push_back( PROPERTY_LISTSOURCETYPE );
            aSuperfluous.push_back( PROPERTY_LISTSOURCE );
            aSuperfluous.push_back( PROPERTY_BOUNDCOLUMN );

            bool bAllowBinding = m_pHelper->canBindToAnyDataType();

            if ( bAllowBinding )
            {
                aSuperfluous.push_back( PROPERTY_MAXTEXTLEN );
                aSuperfluous.push_back( PROPERTY_VALUEMIN );
                aSuperfluous.push_back( PROPERTY_VALUEMAX );
                aSuperfluous.push_back( PROPERTY_DECIMAL_ACCURACY );
                aSuperfluous.push_back( PROPERTY_TIMEMIN );
                aSuperfluous.push_back( PROPERTY_TIMEMAX );
                aSuperfluous.push_back( PROPERTY_DATEMIN );
                aSuperfluous.push_back( PROPERTY_DATEMAX );
                aSuperfluous.push_back( PROPERTY_EFFECTIVE_MIN );
                aSuperfluous.push_back( PROPERTY_EFFECTIVE_MAX );
            }
        }

        return aSuperfluous;
    }

    //--------------------------------------------------------------------
    ::std::vector< ::rtl::OUString > SAL_CALL XSDValidationPropertyHandler::getActuatingProperties( ) const
    {
        ::std::vector< ::rtl::OUString > aInterestedInActuations( 2 );
        if ( m_pHelper.get() )
        {
            aInterestedInActuations.push_back( PROPERTY_XSD_DATA_TYPE );
            aInterestedInActuations.push_back( PROPERTY_XML_DATA_MODEL );
        }
        return aInterestedInActuations;
    }

    //--------------------------------------------------------------------
    namespace
    {
        void showPropertyUI( IPropertyBrowserUI* _pUpdater, const ::rtl::OUString& _rPropertyName, bool _bShow )
        {
            if ( _bShow )
                _pUpdater->showPropertyUI( _rPropertyName, true );
            else
                _pUpdater->hidePropertyUI( _rPropertyName );
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL XSDValidationPropertyHandler::initializePropertyUI( PropertyId _nPropId, IPropertyBrowserUI* _pUpdater )
    {
        OSL_ENSURE( m_pHelper.get(), "XSDValidationPropertyHandler::initializePropertyUI: we don't have any SupportedProperties!" );
        if ( !m_pHelper.get() )
            return;

        switch ( _nPropId )
        {
        case PROPERTY_ID_XSD_DATA_TYPE:
        {
            ::rtl::Reference< XSDDataType > xDataType( m_pHelper->getValidatingDataType() );
            sal_Int16 nTypeClass = xDataType.is() ? xDataType->classify() : DataTypeClass::STRING;

            // is removal of this type possible?
            bool bIsBasicType = xDataType.is() && xDataType->isBasicType();
            _pUpdater->enablePropertyButtons( PROPERTY_XSD_DATA_TYPE, xDataType.is(), xDataType.is() && !bIsBasicType );

            //------------------------------------------------------------
            // show the facets which are available at the data type
            ::rtl::OUString aFacets[] = {
                PROPERTY_XSD_WHITESPACES, PROPERTY_XSD_PATTERN,
                PROPERTY_XSD_LENGTH, PROPERTY_XSD_MIN_LENGTH, PROPERTY_XSD_MAX_LENGTH, PROPERTY_XSD_TOTAL_DIGITS,
                PROPERTY_XSD_FRACTION_DIGITS, PROPERTY_XSD_MAX_INCLUSIVE, PROPERTY_XSD_MAX_EXCLUSIVE,
                PROPERTY_XSD_MIN_INCLUSIVE, PROPERTY_XSD_MIN_EXCLUSIVE
            };

            sal_Int32 i=0;
            const ::rtl::OUString* pLoop = NULL;
            for ( i = 0, pLoop = aFacets;
                  i < sizeof( aFacets ) / sizeof( aFacets[0] );
                  ++i, ++pLoop
                )
            {
                showPropertyUI( _pUpdater, *pLoop, xDataType.is() && xDataType->hasFacet( *pLoop ) );
                _pUpdater->enablePropertyUI( *pLoop, !bIsBasicType );
            }

            //------------------------------------------------------------
            // some properties which we support (namely (Min|Max)(In|Ex)clusive) are of different
            // type, depending on the which data type our introspectee is currently bound to.
            // We need to update this information in our base class' meta data, as it's used
            // for, for instance, string conversion
            ::rtl::OUString sDynamicTypeFacets[] = {
                PROPERTY_XSD_MAX_INCLUSIVE, PROPERTY_XSD_MAX_EXCLUSIVE,
                PROPERTY_XSD_MIN_INCLUSIVE, PROPERTY_XSD_MIN_EXCLUSIVE
            };
            for ( i = 0, pLoop = sDynamicTypeFacets;
                  i < sizeof( sDynamicTypeFacets ) / sizeof( sDynamicTypeFacets[0] );
                  ++i, ++pLoop
                )
            {
                if ( xDataType.is() && xDataType->hasFacet( *pLoop ) )
                    changeTypeOfSupportedProperty(
                        m_pInfoService->getPropertyId( *pLoop ),
                        xDataType->getFacetType( *pLoop )
                    );
            }
        }
        break;
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL XSDValidationPropertyHandler::describePropertyUI( PropertyId _nPropId, PropertyUIDescriptor& /* [out] */ _rDescriptor ) const
    {
        OSL_ENSURE( m_pHelper.get(), "XSDValidationPropertyHandler::describePropertyUI: we don't have any SupportedProperties!" );
        if ( !m_pHelper.get() )
            return;

        if ( _nPropId != PROPERTY_ID_XSD_DATA_TYPE )
            _rDescriptor.bIndent = sal_True;

        switch ( _nPropId )
        {
        case PROPERTY_ID_XSD_DATA_TYPE:
            _rDescriptor.eControlType = BCT_LISTBOX;

            implGetAvailableDataTypeNames( _rDescriptor.aListValues );

            _rDescriptor.nButtonHelpId = UID_PROP_ADD_DATA_TYPE;
            _rDescriptor.nAdditionalButtonHelpId = UID_PROP_REMOVE_DATA_TYPE;
            {
                ModuleRes aResId( RID_RSC_BUTTON_IMAGES );
                ::svt::OLocalResourceAccess aEnumStrings( aResId, RSC_RESOURCE );
                _rDescriptor.aButtonImage = Image( ResId( IMG_PLUS ) );
                _rDescriptor.aAdditionalButtonImage = Image( ResId( IMG_MINUS ) );
            }
            break;

        case PROPERTY_ID_XSD_WHITESPACES:
        {
            _rDescriptor.eControlType = BCT_LISTBOX;
            ::std::vector< String > aListValues = m_pInfoService->getPropertyEnumRepresentations( PROPERTY_ID_XSD_WHITESPACES );
            _rDescriptor.aListValues.resize( aListValues.size() );
            ::std::copy( aListValues.begin(), aListValues.end(), _rDescriptor.aListValues.begin() );
        }
        break;

        case PROPERTY_ID_XSD_PATTERN:
            _rDescriptor.eControlType = BCT_EDIT;
            break;

        case PROPERTY_ID_XSD_LENGTH:
        case PROPERTY_ID_XSD_MIN_LENGTH:
        case PROPERTY_ID_XSD_MAX_LENGTH:
            _rDescriptor.eControlType = BCT_NUMFIELD;
            break;

        case PROPERTY_ID_XSD_TOTAL_DIGITS:
        case PROPERTY_ID_XSD_FRACTION_DIGITS:
            _rDescriptor.eControlType = BCT_NUMFIELD;
            _rDescriptor.nDigits = 0;
            break;

        case PROPERTY_ID_XSD_MAX_INCLUSIVE:
        case PROPERTY_ID_XSD_MAX_EXCLUSIVE:
        case PROPERTY_ID_XSD_MIN_INCLUSIVE:
        case PROPERTY_ID_XSD_MIN_EXCLUSIVE:
        {
            ::rtl::Reference< XSDDataType > xDataType( m_pHelper->getValidatingDataType() );
            sal_Int16 nTypeClass = xDataType.is() ? xDataType->classify() : DataTypeClass::STRING;

            switch ( nTypeClass )
            {
            case DataTypeClass::DECIMAL:
            case DataTypeClass::FLOAT:
            case DataTypeClass::DOUBLE:
                _rDescriptor.eControlType = BCT_NUMFIELD;
                // TODO/eForms: do we have "auto-digits"?
                break;
            case DataTypeClass::DATE:
                _rDescriptor.eControlType = BCT_DATEFIELD;
                break;
            case DataTypeClass::TIME:
                _rDescriptor.eControlType = BCT_TIMEFIELD;
                break;
            case DataTypeClass::DATETIME:
                _rDescriptor.eControlType = BCT_DATETIME;
                break;
            case DataTypeClass::gYear:
            case DataTypeClass::gMonth:
            case DataTypeClass::gDay:
                _rDescriptor.eControlType = BCT_NUMFIELD;
                _rDescriptor.bHaveMinMax = sal_True;
                _rDescriptor.nDigits = 0;
                _rDescriptor.nMinValue = DataTypeClass::gYear == nTypeClass ? 0 : 1;
                _rDescriptor.nMaxValue = ::std::numeric_limits< sal_Int32 >::max();
                if ( DataTypeClass::gMonth == nTypeClass )
                    _rDescriptor.nMaxValue = 12;
                else if ( DataTypeClass::gDay == nTypeClass )
                    _rDescriptor.nMaxValue = 31;
                break;
            default:
//                OSL_ENSURE( !xDataType.is(), "XSDValidationPropertyHandler::describePropertyUI: This type does not have this property!" );
                _rDescriptor.eControlType = BCT_NUMFIELD;
                break;
            }
        }
        break;

        default:
            DBG_ERROR( "XSDValidationPropertyHandler::describePropertyUI: cannot handle this property!" );
        }
    }

    //--------------------------------------------------------------------
    bool SAL_CALL XSDValidationPropertyHandler::requestUserInputOnButtonClick( PropertyId _nPropId, bool _bPrimary, Any& _rData )
    {
        OSL_ENSURE( m_pHelper.get(), "XSDValidationPropertyHandler::requestUserInputOnButtonClick: we don't have any SupportedProperties!" );
        if ( !m_pHelper.get() )
            return false;

        switch ( _nPropId )
        {
        case PROPERTY_ID_XSD_DATA_TYPE:
        {
            if ( _bPrimary )
            {
                ::rtl::OUString sNewDataTypeName;
                if ( implPrepareCloneDataCurrentType( sNewDataTypeName ) )
                {
                    _rData <<= sNewDataTypeName;
                    return true;
                }
            }
            else
                return implPrepareRemoveCurrentDataType();
        }
        break;

        default:
            DBG_ERROR( "XSDValidationPropertyHandler::requestUserInputOnButtonClick: not yet implemented!" );
        }
        return false;
    }

    //--------------------------------------------------------------------
    void SAL_CALL XSDValidationPropertyHandler::executeButtonClick( PropertyId _nPropId, bool _bPrimary, const Any& _rData, IPropertyBrowserUI* _pUpdater )
    {
        OSL_ENSURE( m_pHelper.get(), "XSDValidationPropertyHandler::executeButtonClick: we don't have any SupportedProperties!" );
        if ( !m_pHelper.get() )
            return;

        switch ( _nPropId )
        {
        case PROPERTY_ID_XSD_DATA_TYPE:
        {
            if ( _bPrimary )
            {
                ::rtl::OUString sNewDataTypeName;
                OSL_VERIFY( _rData >>= sNewDataTypeName );
                    // if this asserts, the data does not result from my requestUserInputOnButtonClick ...
                if ( implDoCloneCurrentDataType( sNewDataTypeName ) )
                    // the UI for this property needs to be rebuilt, to reflect the new
                    // type entry
                    _pUpdater->rebuildPropertyUI( PROPERTY_XSD_DATA_TYPE );
            }
            else
            {
                if ( implDoRemoveCurrentDataType() )
                {   // the UI for this property needs to be rebuilt, to reflect the missing
                    // type entry
                    _pUpdater->rebuildPropertyUI( PROPERTY_XSD_DATA_TYPE );
                }
            }
        }
        break;

        default:
            DBG_ERROR( "XSDValidationPropertyHandler::executeButtonClick: not yet implemented!" );
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL XSDValidationPropertyHandler::startAllPropertyChangeListening( const Reference< XPropertyChangeListener >& _rxListener )
    {
        OSL_ENSURE( m_pHelper.get(), "XSDValidationPropertyHandler::startAllPropertyChangeListening: we don't have any SupportedProperties!" );
        if ( m_pHelper.get() )
        {
            PropertyHandler::startAllPropertyChangeListening( _rxListener );
            m_pHelper->registerBindingListener( _rxListener );
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL XSDValidationPropertyHandler::stopAllPropertyChangeListening( )
    {
        OSL_ENSURE( m_pHelper.get(), "XSDValidationPropertyHandler::startAllPropertyChangeListening: we don't have any SupportedProperties!" );
        if ( m_pHelper.get() )
        {
            m_pHelper->revokeBindingListener();
            PropertyHandler::stopAllPropertyChangeListening();
        }
    }

    //--------------------------------------------------------------------
    bool XSDValidationPropertyHandler::implPrepareCloneDataCurrentType( ::rtl::OUString& _rNewName ) SAL_THROW(())
    {
        OSL_PRECOND( m_pHelper.get(), "XSDValidationPropertyHandler::implPrepareCloneDataCurrentType: this will crash!" );

        ::rtl::Reference< XSDDataType > pType = m_pHelper->getValidatingDataType();
        if ( !pType.is() )
        {
            DBG_ERROR( "XSDValidationPropertyHandler::implPrepareCloneDataCurrentType: invalid current data type!" );
            return false;
        }

        ::std::vector< ::rtl::OUString > aExistentNames;
        m_pHelper->getAvailableDataTypeNames( aExistentNames );

        NewDataTypeDialog aDialog( NULL, pType->getName(), aExistentNames );  // TODO/eForms: proper parent
        if ( aDialog.Execute() != RET_OK )
            return false;

        _rNewName = aDialog.GetName();
        return true;
    }

    //--------------------------------------------------------------------
    bool XSDValidationPropertyHandler::implDoCloneCurrentDataType( const ::rtl::OUString& _rNewName ) SAL_THROW(())
    {
        OSL_PRECOND( m_pHelper.get(), "XSDValidationPropertyHandler::implDoCloneCurrentDataType: this will crash!" );

        ::rtl::Reference< XSDDataType > pType = m_pHelper->getValidatingDataType();
        if ( !pType.is() )
            return false;

        if ( !m_pHelper->cloneDataType( pType, _rNewName ) )
            return false;

        m_pHelper->setValidatingDataTypeByName( _rNewName );
        return true;
    }

    //--------------------------------------------------------------------
    bool XSDValidationPropertyHandler::implPrepareRemoveCurrentDataType() SAL_THROW(())
    {
        OSL_PRECOND( m_pHelper.get(), "XSDValidationPropertyHandler::implPrepareRemoveCurrentDataType: this will crash!" );

        ::rtl::Reference< XSDDataType > pType = m_pHelper->getValidatingDataType();
        if ( !pType.is() )
        {
            DBG_ERROR( "XSDValidationPropertyHandler::implPrepareRemoveCurrentDataType: invalid current data type!" );
            return false;
        }

        // confirmation message
        String sConfirmation( ModuleRes( RID_STR_CONFIRM_DELETE_DATA_TYPE ) );
        sConfirmation.SearchAndReplaceAscii( "#type#", pType->getName() );
        QueryBox aQuery( NULL, WB_YES_NO, sConfirmation ); // TODO/eForms: proper parent
        if ( aQuery.Execute() != RET_YES )
            return false;

        return true;
    }

    //--------------------------------------------------------------------
    bool XSDValidationPropertyHandler::implDoRemoveCurrentDataType() SAL_THROW(())
    {
        OSL_PRECOND( m_pHelper.get(), "XSDValidationPropertyHandler::implDoRemoveCurrentDataType: this will crash!" );

        ::rtl::Reference< XSDDataType > pType = m_pHelper->getValidatingDataType();
        if ( !pType.is() )
            return false;

        // set a new data type at the binding, which is the "basic" type for the one
        // we are going to delete
        // (do this before the actual deletion, so the old type is still valid for property change
        // notifications)
        m_pHelper->setValidatingDataTypeByName( m_pHelper->getBasicTypeNameForClass( pType->classify() ) );
        // now remove the type
        m_pHelper->removeDataTypeFromRepository( pType->getName() );

        return true;
    }

    //--------------------------------------------------------------------
    void SAL_CALL XSDValidationPropertyHandler::updateDependentProperties( PropertyId _nActuatingPropId, const Any& _rNewValue, const Any& _rOldValue, IPropertyBrowserUI* _pUpdater )
    {
        OSL_ENSURE( m_pHelper.get(), "XSDValidationPropertyHandler::updateDependentProperties: we don't have any ActuatingProperties!" );
        if ( !m_pHelper.get() )
            return;

        switch ( _nActuatingPropId )
        {
        case PROPERTY_ID_XSD_DATA_TYPE:
            initializePropertyUI( PROPERTY_ID_XSD_DATA_TYPE, _pUpdater );
            break;

        case PROPERTY_ID_XML_DATA_MODEL:
        {
            // The data type which the current binding works with may not be present in the
            // new model. Thus, transfer it.
            ::rtl::OUString sOldModelName; _rOldValue >>= sOldModelName;
            ::rtl::OUString sNewModelName; _rNewValue >>= sNewModelName;
            ::rtl::OUString sDataType = m_pHelper->getValidatingDataTypeName();
            m_pHelper->copyDataType( sOldModelName, sNewModelName, sDataType );

            // the list of available data types depends on the chosen model, so update this
            _pUpdater->rebuildPropertyUI( PROPERTY_XSD_DATA_TYPE );
        }
        break;

        default:
            DBG_ERROR( "XSDValidationPropertyHandler::updateDependentProperties: cannot handle this property!" );
        }
    }

    //--------------------------------------------------------------------
    void XSDValidationPropertyHandler::implGetAvailableDataTypeNames( ::std::vector< ::rtl::OUString >& /* [out] */ _rNames ) const SAL_THROW(())
    {
        OSL_PRECOND( m_pHelper.get(), "XSDValidationPropertyHandler::implGetAvailableDataTypeNames: this will crash!" );
        // start with *all* types which are available at the model
        ::std::vector< ::rtl::OUString > aAllTypes;
        m_pHelper->getAvailableDataTypeNames( aAllTypes );
        _rNames.clear();
        _rNames.reserve( aAllTypes.size() );

        // then allow only those which are "compatible" with our control
        for ( ::std::vector< ::rtl::OUString >::const_iterator dataType = aAllTypes.begin();
              dataType != aAllTypes.end();
              ++dataType
            )
        {
            ::rtl::Reference< XSDDataType > pType = m_pHelper->getDataTypeByName( *dataType );
            if ( pType.is() && m_pHelper->canBindToDataType( pType->classify() ) )
                _rNames.push_back( *dataType );
        }
    }

//........................................................................
} // namespace pcr
//........................................................................

