/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"
#include "xsdvalidationpropertyhandler.hxx"
#include "formstrings.hxx"
#include "formmetadata.hxx"
#include "xsddatatypes.hxx"
#include "modulepcr.hxx"
#include "formresid.hrc"
#include "formlocalid.hrc"
#include "propctrlr.hrc"
#include "newdatatype.hxx"
#include "xsdvalidationhelper.hxx"
#include "pcrcommon.hxx"
#include "handlerhelper.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/xsd/WhiteSpaceTreatment.hpp>
#include <com/sun/star/xsd/DataTypeClass.hpp>
#include <com/sun/star/inspection/PropertyControlType.hpp>
#include <com/sun/star/beans/Optional.hpp>
#include <com/sun/star/inspection/XObjectInspectorUI.hpp>
#include <com/sun/star/inspection/PropertyLineElement.hpp>
/** === end UNO includes === **/
#include <vcl/msgbox.hxx>
#include <tools/debug.hxx>
#include <svtools/localresaccess.hxx>
#include <sal/macros.h>

#include <algorithm>
#include <functional>
#include <limits>

//------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_XSDValidationPropertyHandler()
{
    ::pcr::XSDValidationPropertyHandler::registerImplementation();
}

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
    using namespace ::com::sun::star::inspection;

    using ::com::sun::star::beans::PropertyAttribute::MAYBEVOID;

    //====================================================================
    //= XSDValidationPropertyHandler
    //====================================================================
    DBG_NAME( XSDValidationPropertyHandler )
    //--------------------------------------------------------------------
    XSDValidationPropertyHandler::XSDValidationPropertyHandler( const Reference< XComponentContext >& _rxContext )
        :XSDValidationPropertyHandler_Base( _rxContext )
    {
        DBG_CTOR( XSDValidationPropertyHandler, NULL );
    }

    //--------------------------------------------------------------------
    XSDValidationPropertyHandler::~XSDValidationPropertyHandler()
    {
        DBG_DTOR( XSDValidationPropertyHandler, NULL );
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL XSDValidationPropertyHandler::getImplementationName_static(  ) throw (RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.extensions.XSDValidationPropertyHandler" ) );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL XSDValidationPropertyHandler::getSupportedServiceNames_static(  ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aSupported( 1 );
        aSupported[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.inspection.XSDValidationPropertyHandler" ) );
        return aSupported;
    }

    //--------------------------------------------------------------------
    Any SAL_CALL XSDValidationPropertyHandler::getPropertyValue( const ::rtl::OUString& _rPropertyName ) throw (UnknownPropertyException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nPropId( impl_getPropertyId_throw( _rPropertyName ) );

        OSL_ENSURE( m_pHelper.get(), "XSDValidationPropertyHandler::getPropertyValue: inconsistency!" );
            // if we survived impl_getPropertyId_throw, we should have a helper, since no helper implies no properties

        Any aReturn;
        ::rtl::Reference< XSDDataType > pType = m_pHelper->getValidatingDataType();
        switch ( nPropId )
        {
        // common facets
        case PROPERTY_ID_XSD_DATA_TYPE:  aReturn = pType.is() ? pType->getFacet( PROPERTY_NAME ) : makeAny( ::rtl::OUString() ); break;
        case PROPERTY_ID_XSD_WHITESPACES:aReturn = pType.is() ? pType->getFacet( PROPERTY_XSD_WHITESPACES ) : makeAny( WhiteSpaceTreatment::Preserve ); break;
        case PROPERTY_ID_XSD_PATTERN:    aReturn = pType.is() ? pType->getFacet( PROPERTY_XSD_PATTERN ) : makeAny( ::rtl::OUString() ); break;

        // all other properties are simply forwarded, if they exist at the given type
        default:
        {
            if ( pType.is() && pType->hasFacet( _rPropertyName ) )
                aReturn = pType->getFacet( _rPropertyName );
        }
        break;
        }

        return aReturn;
    }

    //--------------------------------------------------------------------
    void SAL_CALL XSDValidationPropertyHandler::setPropertyValue( const ::rtl::OUString& _rPropertyName, const Any& _rValue ) throw (UnknownPropertyException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nPropId( impl_getPropertyId_throw( _rPropertyName ) );

        OSL_ENSURE( m_pHelper.get(), "XSDValidationPropertyHandler::getPropertyValue: inconsistency!" );
            // if we survived impl_getPropertyId_throw, we should have a helper, since no helper implies no properties

        if ( PROPERTY_ID_XSD_DATA_TYPE == nPropId )
        {
            ::rtl::OUString sTypeName;
            OSL_VERIFY( _rValue >>= sTypeName );
            m_pHelper->setValidatingDataTypeByName( sTypeName );
            impl_setContextDocumentModified_nothrow();
            return;
        }

        ::rtl::Reference< XSDDataType > pType = m_pHelper->getValidatingDataType();
        if ( !pType.is() )
        {
            OSL_FAIL( "XSDValidationPropertyHandler::setPropertyValue: you're trying to set a type facet, without a current type!" );
            return;
        }

        pType->setFacet( _rPropertyName, _rValue );
        impl_setContextDocumentModified_nothrow();
    }

    //--------------------------------------------------------------------
    void XSDValidationPropertyHandler::onNewComponent()
    {
        XSDValidationPropertyHandler_Base::onNewComponent();

        Reference< frame::XModel > xDocument( impl_getContextDocument_nothrow() );
        DBG_ASSERT( xDocument.is(), "XSDValidationPropertyHandler::onNewComponent: no document!" );
        if ( EFormsHelper::isEForm( xDocument ) )
            m_pHelper.reset( new XSDValidationHelper( m_aMutex, m_xComponent, xDocument ) );
        else
            m_pHelper.reset( NULL );
    }

    //--------------------------------------------------------------------
    Sequence< Property > XSDValidationPropertyHandler::doDescribeSupportedProperties() const
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

                // facets for different types
                addInt16PropertyDescription( aProperties, PROPERTY_XSD_MAX_INCLUSIVE_INT, MAYBEVOID );
                addInt16PropertyDescription( aProperties, PROPERTY_XSD_MAX_EXCLUSIVE_INT, MAYBEVOID );
                addInt16PropertyDescription( aProperties, PROPERTY_XSD_MIN_INCLUSIVE_INT, MAYBEVOID );
                addInt16PropertyDescription( aProperties, PROPERTY_XSD_MIN_EXCLUSIVE_INT, MAYBEVOID );
                addDoublePropertyDescription( aProperties, PROPERTY_XSD_MAX_INCLUSIVE_DOUBLE, MAYBEVOID );
                addDoublePropertyDescription( aProperties, PROPERTY_XSD_MAX_EXCLUSIVE_DOUBLE, MAYBEVOID );
                addDoublePropertyDescription( aProperties, PROPERTY_XSD_MIN_INCLUSIVE_DOUBLE, MAYBEVOID );
                addDoublePropertyDescription( aProperties, PROPERTY_XSD_MIN_EXCLUSIVE_DOUBLE, MAYBEVOID );
                addDatePropertyDescription( aProperties, PROPERTY_XSD_MAX_INCLUSIVE_DATE, MAYBEVOID );
                addDatePropertyDescription( aProperties, PROPERTY_XSD_MAX_EXCLUSIVE_DATE, MAYBEVOID );
                addDatePropertyDescription( aProperties, PROPERTY_XSD_MIN_INCLUSIVE_DATE, MAYBEVOID );
                addDatePropertyDescription( aProperties, PROPERTY_XSD_MIN_EXCLUSIVE_DATE, MAYBEVOID );
                addTimePropertyDescription( aProperties, PROPERTY_XSD_MAX_INCLUSIVE_TIME, MAYBEVOID );
                addTimePropertyDescription( aProperties, PROPERTY_XSD_MAX_EXCLUSIVE_TIME, MAYBEVOID );
                addTimePropertyDescription( aProperties, PROPERTY_XSD_MIN_INCLUSIVE_TIME, MAYBEVOID );
                addTimePropertyDescription( aProperties, PROPERTY_XSD_MIN_EXCLUSIVE_TIME, MAYBEVOID );
                addDateTimePropertyDescription( aProperties, PROPERTY_XSD_MAX_INCLUSIVE_DATE_TIME, MAYBEVOID );
                addDateTimePropertyDescription( aProperties, PROPERTY_XSD_MAX_EXCLUSIVE_DATE_TIME, MAYBEVOID );
                addDateTimePropertyDescription( aProperties, PROPERTY_XSD_MIN_INCLUSIVE_DATE_TIME, MAYBEVOID );
                addDateTimePropertyDescription( aProperties, PROPERTY_XSD_MIN_EXCLUSIVE_DATE_TIME, MAYBEVOID );
            }
        }

        if ( aProperties.empty() )
            return Sequence< Property >();
        return Sequence< Property >( &(*aProperties.begin()), aProperties.size() );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL XSDValidationPropertyHandler::getSupersededProperties( ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

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

        if ( aSuperfluous.empty() )
            return Sequence< ::rtl::OUString >();
        return Sequence< ::rtl::OUString >( &(*aSuperfluous.begin()), aSuperfluous.size() );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL XSDValidationPropertyHandler::getActuatingProperties( ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        ::std::vector< ::rtl::OUString > aInterestedInActuations( 2 );
        if ( m_pHelper.get() )
        {
            aInterestedInActuations.push_back( PROPERTY_XSD_DATA_TYPE );
            aInterestedInActuations.push_back( PROPERTY_XML_DATA_MODEL );
        }
        if ( aInterestedInActuations.empty() )
            return Sequence< ::rtl::OUString >();
        return Sequence< ::rtl::OUString >( &(*aInterestedInActuations.begin()), aInterestedInActuations.size() );
    }

    //--------------------------------------------------------------------
    namespace
    {
        void showPropertyUI( const Reference< XObjectInspectorUI >& _rxInspectorUI, const ::rtl::OUString& _rPropertyName, bool _bShow )
        {
            if ( _bShow )
                _rxInspectorUI->showPropertyUI( _rPropertyName );
            else
                _rxInspectorUI->hidePropertyUI( _rPropertyName );
        }
    }

    //--------------------------------------------------------------------
    LineDescriptor SAL_CALL XSDValidationPropertyHandler::describePropertyLine( const ::rtl::OUString& _rPropertyName,
        const Reference< XPropertyControlFactory >& _rxControlFactory )
        throw (UnknownPropertyException, NullPointerException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !_rxControlFactory.is() )
            throw NullPointerException();
        if ( !m_pHelper.get() )
            throw RuntimeException();

        PropertyId nPropId( impl_getPropertyId_throw( _rPropertyName ) );

        LineDescriptor aDescriptor;
        if ( nPropId != PROPERTY_ID_XSD_DATA_TYPE )
            aDescriptor.IndentLevel = 1;

        // collect some information about the to-be-created control
        sal_Int16 nControlType = PropertyControlType::TextField;
        ::std::vector< ::rtl::OUString > aListEntries;
        Optional< double > aMinValue( sal_False, 0 );
        Optional< double > aMaxValue( sal_False, 0 );

        switch ( nPropId )
        {
        case PROPERTY_ID_XSD_DATA_TYPE:
            nControlType = PropertyControlType::ListBox;

            implGetAvailableDataTypeNames( aListEntries );

            aDescriptor.PrimaryButtonId = rtl::OUString::createFromAscii(UID_PROP_ADD_DATA_TYPE);
            aDescriptor.SecondaryButtonId = rtl::OUString::createFromAscii(UID_PROP_REMOVE_DATA_TYPE);;
            aDescriptor.HasPrimaryButton = aDescriptor.HasSecondaryButton = sal_True;
            aDescriptor.PrimaryButtonImageURL = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:graphicrepository/extensions/res/buttonplus.png" ) );
            aDescriptor.SecondaryButtonImageURL = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:graphicrepository/extensions/res/buttonminus.png" ) );
            break;

        case PROPERTY_ID_XSD_WHITESPACES:
        {
            nControlType = PropertyControlType::ListBox;
            aListEntries = m_pInfoService->getPropertyEnumRepresentations( PROPERTY_ID_XSD_WHITESPACES );
        }
        break;

        case PROPERTY_ID_XSD_PATTERN:
            nControlType = PropertyControlType::TextField;
            break;

        case PROPERTY_ID_XSD_LENGTH:
        case PROPERTY_ID_XSD_MIN_LENGTH:
        case PROPERTY_ID_XSD_MAX_LENGTH:
            nControlType = PropertyControlType::NumericField;
            break;

        case PROPERTY_ID_XSD_TOTAL_DIGITS:
        case PROPERTY_ID_XSD_FRACTION_DIGITS:
            nControlType = PropertyControlType::NumericField;
            break;

        case PROPERTY_ID_XSD_MAX_INCLUSIVE_INT:
        case PROPERTY_ID_XSD_MAX_EXCLUSIVE_INT:
        case PROPERTY_ID_XSD_MIN_INCLUSIVE_INT:
        case PROPERTY_ID_XSD_MIN_EXCLUSIVE_INT:
        {
            nControlType = PropertyControlType::NumericField;

            // handle limits for various 'INT' types according to
            // their actual semantics (year, month, day)

            ::rtl::Reference< XSDDataType > xDataType( m_pHelper->getValidatingDataType() );
            sal_Int16 nTypeClass = xDataType.is() ? xDataType->classify() : DataTypeClass::STRING;

            aMinValue.IsPresent = aMaxValue.IsPresent = sal_True;
            aMinValue.Value = DataTypeClass::gYear == nTypeClass ? 0 : 1;
            aMaxValue.Value = ::std::numeric_limits< sal_Int32 >::max();
            if ( DataTypeClass::gMonth == nTypeClass )
                aMaxValue.Value = 12;
            else if ( DataTypeClass::gDay == nTypeClass )
                aMaxValue.Value = 31;
        }
        break;

        case PROPERTY_ID_XSD_MAX_INCLUSIVE_DOUBLE:
        case PROPERTY_ID_XSD_MAX_EXCLUSIVE_DOUBLE:
        case PROPERTY_ID_XSD_MIN_INCLUSIVE_DOUBLE:
        case PROPERTY_ID_XSD_MIN_EXCLUSIVE_DOUBLE:
            nControlType = PropertyControlType::NumericField;
            // TODO/eForms: do we have "auto-digits"?
            break;

        case PROPERTY_ID_XSD_MAX_INCLUSIVE_DATE:
        case PROPERTY_ID_XSD_MAX_EXCLUSIVE_DATE:
        case PROPERTY_ID_XSD_MIN_INCLUSIVE_DATE:
        case PROPERTY_ID_XSD_MIN_EXCLUSIVE_DATE:
            nControlType = PropertyControlType::DateField;
            break;

        case PROPERTY_ID_XSD_MAX_INCLUSIVE_TIME:
        case PROPERTY_ID_XSD_MAX_EXCLUSIVE_TIME:
        case PROPERTY_ID_XSD_MIN_INCLUSIVE_TIME:
        case PROPERTY_ID_XSD_MIN_EXCLUSIVE_TIME:
            nControlType = PropertyControlType::TimeField;
            break;

        case PROPERTY_ID_XSD_MAX_INCLUSIVE_DATE_TIME:
        case PROPERTY_ID_XSD_MAX_EXCLUSIVE_DATE_TIME:
        case PROPERTY_ID_XSD_MIN_INCLUSIVE_DATE_TIME:
        case PROPERTY_ID_XSD_MIN_EXCLUSIVE_DATE_TIME:
            nControlType = PropertyControlType::DateTimeField;
            break;

        default:
            OSL_FAIL( "XSDValidationPropertyHandler::describePropertyLine: cannot handle this property!" );
            break;
        }

        switch ( nControlType )
        {
        case PropertyControlType::ListBox:
            aDescriptor.Control = PropertyHandlerHelper::createListBoxControl( _rxControlFactory, aListEntries, sal_False, sal_False );
            break;
        case PropertyControlType::NumericField:
            aDescriptor.Control = PropertyHandlerHelper::createNumericControl( _rxControlFactory, 0, aMinValue, aMaxValue, sal_False );
            break;
        default:
            aDescriptor.Control = _rxControlFactory->createPropertyControl( nControlType, sal_False );
            break;
        }

        aDescriptor.Category = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Data" ) );
        aDescriptor.DisplayName = m_pInfoService->getPropertyTranslation( nPropId );
        aDescriptor.HelpURL = HelpIdUrl::getHelpURL( m_pInfoService->getPropertyHelpId( nPropId ) );

        return aDescriptor;
    }

    //--------------------------------------------------------------------
    InteractiveSelectionResult SAL_CALL XSDValidationPropertyHandler::onInteractivePropertySelection( const ::rtl::OUString& _rPropertyName, sal_Bool _bPrimary, Any& /*_rData*/, const Reference< XObjectInspectorUI >& _rxInspectorUI ) throw (UnknownPropertyException, NullPointerException, RuntimeException)
    {
        if ( !_rxInspectorUI.is() )
            throw NullPointerException();

        ::osl::MutexGuard aGuard( m_aMutex );
        OSL_ENSURE( m_pHelper.get(), "XSDValidationPropertyHandler::onInteractivePropertySelection: we don't have any SupportedProperties!" );
        if ( !m_pHelper.get() )
            return InteractiveSelectionResult_Cancelled;

        PropertyId nPropId( impl_getPropertyId_throw( _rPropertyName ) );

        switch ( nPropId )
        {
        case PROPERTY_ID_XSD_DATA_TYPE:
        {
            if ( _bPrimary )
            {
                ::rtl::OUString sNewDataTypeName;
                if ( implPrepareCloneDataCurrentType( sNewDataTypeName ) )
                {
                    implDoCloneCurrentDataType( sNewDataTypeName );
                    return InteractiveSelectionResult_Success;
                }
            }
            else
                return implPrepareRemoveCurrentDataType() && implDoRemoveCurrentDataType() ? InteractiveSelectionResult_Success : InteractiveSelectionResult_Cancelled;
        }
        break;

        default:
            OSL_FAIL( "XSDValidationPropertyHandler::onInteractivePropertySelection: unexpected property to build a dedicated UI!" );
            break;
        }
        return InteractiveSelectionResult_Cancelled;
    }

    //--------------------------------------------------------------------
    void SAL_CALL XSDValidationPropertyHandler::addPropertyChangeListener( const Reference< XPropertyChangeListener >& _rxListener ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        XSDValidationPropertyHandler_Base::addPropertyChangeListener( _rxListener );
        if ( m_pHelper.get() )
            m_pHelper->registerBindingListener( _rxListener );
    }

    //--------------------------------------------------------------------
    void SAL_CALL XSDValidationPropertyHandler::removePropertyChangeListener( const Reference< XPropertyChangeListener >& _rxListener ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( m_pHelper.get() )
            m_pHelper->revokeBindingListener( _rxListener );
        XSDValidationPropertyHandler_Base::removePropertyChangeListener( _rxListener );
    }

    //--------------------------------------------------------------------
    bool XSDValidationPropertyHandler::implPrepareCloneDataCurrentType( ::rtl::OUString& _rNewName ) SAL_THROW(())
    {
        OSL_PRECOND( m_pHelper.get(), "XSDValidationPropertyHandler::implPrepareCloneDataCurrentType: this will crash!" );

        ::rtl::Reference< XSDDataType > pType = m_pHelper->getValidatingDataType();
        if ( !pType.is() )
        {
            OSL_FAIL( "XSDValidationPropertyHandler::implPrepareCloneDataCurrentType: invalid current data type!" );
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
            OSL_FAIL( "XSDValidationPropertyHandler::implPrepareRemoveCurrentDataType: invalid current data type!" );
            return false;
        }

        // confirmation message
        String sConfirmation( PcrRes( RID_STR_CONFIRM_DELETE_DATA_TYPE ) );
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
    void SAL_CALL XSDValidationPropertyHandler::actuatingPropertyChanged( const ::rtl::OUString& _rActuatingPropertyName, const Any& _rNewValue, const Any& _rOldValue, const Reference< XObjectInspectorUI >& _rxInspectorUI, sal_Bool _bFirstTimeInit ) throw (NullPointerException, RuntimeException)
    {
        if ( !_rxInspectorUI.is() )
            throw NullPointerException();

        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nActuatingPropId( impl_getPropertyId_throw( _rActuatingPropertyName ) );
        if ( !m_pHelper.get() )
            throw RuntimeException();
            // if we survived impl_getPropertyId_throw, we should have a helper, since no helper implies no properties

        switch ( nActuatingPropId )
        {
        case PROPERTY_ID_XSD_DATA_TYPE:
        {
            ::rtl::Reference< XSDDataType > xDataType( m_pHelper->getValidatingDataType() );

            // is removal of this type possible?
            sal_Bool bIsBasicType = xDataType.is() && xDataType->isBasicType();
            _rxInspectorUI->enablePropertyUIElements( PROPERTY_XSD_DATA_TYPE, PropertyLineElement::PrimaryButton, xDataType.is() );
            _rxInspectorUI->enablePropertyUIElements( PROPERTY_XSD_DATA_TYPE, PropertyLineElement::SecondaryButton, xDataType.is() && !bIsBasicType );

            //------------------------------------------------------------
            // show the facets which are available at the data type
            ::rtl::OUString aFacets[] = {
                PROPERTY_XSD_WHITESPACES, PROPERTY_XSD_PATTERN,
                PROPERTY_XSD_LENGTH, PROPERTY_XSD_MIN_LENGTH, PROPERTY_XSD_MAX_LENGTH, PROPERTY_XSD_TOTAL_DIGITS,
                PROPERTY_XSD_FRACTION_DIGITS,
                PROPERTY_XSD_MAX_INCLUSIVE_INT,
                PROPERTY_XSD_MAX_EXCLUSIVE_INT,
                PROPERTY_XSD_MIN_INCLUSIVE_INT,
                PROPERTY_XSD_MIN_EXCLUSIVE_INT,
                PROPERTY_XSD_MAX_INCLUSIVE_DOUBLE,
                PROPERTY_XSD_MAX_EXCLUSIVE_DOUBLE,
                PROPERTY_XSD_MIN_INCLUSIVE_DOUBLE,
                PROPERTY_XSD_MIN_EXCLUSIVE_DOUBLE,
                PROPERTY_XSD_MAX_INCLUSIVE_DATE,
                PROPERTY_XSD_MAX_EXCLUSIVE_DATE,
                PROPERTY_XSD_MIN_INCLUSIVE_DATE,
                PROPERTY_XSD_MIN_EXCLUSIVE_DATE,
                PROPERTY_XSD_MAX_INCLUSIVE_TIME,
                PROPERTY_XSD_MAX_EXCLUSIVE_TIME,
                PROPERTY_XSD_MIN_INCLUSIVE_TIME,
                PROPERTY_XSD_MIN_EXCLUSIVE_TIME,
                PROPERTY_XSD_MAX_INCLUSIVE_DATE_TIME,
                PROPERTY_XSD_MAX_EXCLUSIVE_DATE_TIME,
                PROPERTY_XSD_MIN_INCLUSIVE_DATE_TIME,
                PROPERTY_XSD_MIN_EXCLUSIVE_DATE_TIME
            };

            size_t i=0;
            const ::rtl::OUString* pLoop = NULL;
            for ( i = 0, pLoop = aFacets;
                  i < SAL_N_ELEMENTS( aFacets );
                  ++i, ++pLoop
                )
            {
                showPropertyUI( _rxInspectorUI, *pLoop, xDataType.is() && xDataType->hasFacet( *pLoop ) );
                _rxInspectorUI->enablePropertyUI( *pLoop, !bIsBasicType );
            }
        }
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
            if ( !_bFirstTimeInit )
                _rxInspectorUI->rebuildPropertyUI( PROPERTY_XSD_DATA_TYPE );
        }
        break;

        default:
            OSL_FAIL( "XSDValidationPropertyHandler::actuatingPropertyChanged: cannot handle this property!" );
            return;
        }

        // in both cases, we need to care for the current value of the XSD_DATA_TYPE property,
        // and update the FormatKey of the formatted field we're inspecting (if any)
        if ( !_bFirstTimeInit && m_pHelper->isInspectingFormattedField() )
            m_pHelper->findDefaultFormatForIntrospectee();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
