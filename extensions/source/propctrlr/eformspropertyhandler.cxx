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

#include "eformspropertyhandler.hxx"
#include "formstrings.hxx"
#include "formmetadata.hxx"
#include "pcrservices.hxx"
#include "propctrlr.hrc"
#include "formbrowsertools.hxx"
#include "eformshelper.hxx"
#include "handlerhelper.hxx"

#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/inspection/PropertyControlType.hpp>
#include <com/sun/star/inspection/XObjectInspectorUI.hpp>
#include <tools/debug.hxx>

#include <functional>


extern "C" void SAL_CALL createRegistryInfo_EFormsPropertyHandler()
{
    ::pcr::EFormsPropertyHandler::registerImplementation();
}


namespace pcr
{


    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::xforms;
    using namespace ::com::sun::star::script;
    using namespace ::com::sun::star::ui::dialogs;
    using namespace ::com::sun::star::form::binding;
    using namespace ::com::sun::star::inspection;


    //= EFormsPropertyHandler


    EFormsPropertyHandler::EFormsPropertyHandler( const Reference< XComponentContext >& _rxContext )
        :EFormsPropertyHandler_Base( _rxContext )
        ,m_bSimulatingModelChange( false )
    {
    }


    EFormsPropertyHandler::~EFormsPropertyHandler( )
    {
    }


    OUString SAL_CALL EFormsPropertyHandler::getImplementationName_static(  ) throw (RuntimeException)
    {
        return OUString( "com.sun.star.comp.extensions.EFormsPropertyHandler" );
    }


    Sequence< OUString > SAL_CALL EFormsPropertyHandler::getSupportedServiceNames_static(  ) throw (RuntimeException)
    {
        Sequence<OUString> aSupported { "com.sun.star.form.inspection.XMLFormsPropertyHandler" };
        return aSupported;
    }


    OUString EFormsPropertyHandler::getModelNamePropertyValue() const
    {
        OUString sModelName = m_pHelper->getCurrentFormModelName();
        if ( sModelName.isEmpty() )
            sModelName = m_sBindingLessModelName;
        return sModelName;
    }


    Any SAL_CALL EFormsPropertyHandler::getPropertyValue( const OUString& _rPropertyName ) throw (UnknownPropertyException, RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nPropId( impl_getPropertyId_throwUnknownProperty( _rPropertyName ) );

        OSL_ENSURE( m_pHelper.get(), "EFormsPropertyHandler::getPropertyValue: we don't have any SupportedProperties!" );
            // if we survived impl_getPropertyId_throwUnknownProperty, we should have a helper, since no helper implies no properties

        Any aReturn;
        try
        {
            switch ( nPropId )
            {
            case PROPERTY_ID_LIST_BINDING:
                aReturn <<= m_pHelper->getCurrentListSourceBinding();
                break;

            case PROPERTY_ID_XML_DATA_MODEL:
                aReturn <<= getModelNamePropertyValue();
                break;

            case PROPERTY_ID_BINDING_NAME:
                aReturn <<= m_pHelper->getCurrentBindingName();
                break;

            case PROPERTY_ID_BIND_EXPRESSION:
            case PROPERTY_ID_XSD_CONSTRAINT:
            case PROPERTY_ID_XSD_CALCULATION:
            case PROPERTY_ID_XSD_REQUIRED:
            case PROPERTY_ID_XSD_RELEVANT:
            case PROPERTY_ID_XSD_READONLY:
            {
                Reference< XPropertySet > xBindingProps( m_pHelper->getCurrentBinding() );
                if ( xBindingProps.is() )
                {
                    aReturn = xBindingProps->getPropertyValue( _rPropertyName );
                    DBG_ASSERT( aReturn.getValueType().equals( ::cppu::UnoType<OUString>::get() ),
                        "EFormsPropertyHandler::getPropertyValue: invalid BindingExpression value type!" );
                }
                else
                    aReturn <<= OUString();
            }
            break;

            default:
                OSL_FAIL( "EFormsPropertyHandler::getPropertyValue: cannot handle this property!" );
                break;
            }
        }
        catch( const Exception& )
        {
#if OSL_DEBUG_LEVEL > 0
            OString sMessage( "EFormsPropertyHandler::getPropertyValue: caught an exception!" );
            sMessage += "\n(have been asked for the \"";
            sMessage += OString( _rPropertyName.getStr(), _rPropertyName.getLength(), RTL_TEXTENCODING_ASCII_US );
            sMessage += "\" property.)";
            OSL_FAIL( sMessage.getStr() );
#endif
        }
        return aReturn;
    }


    void SAL_CALL EFormsPropertyHandler::setPropertyValue( const OUString& _rPropertyName, const Any& _rValue ) throw (UnknownPropertyException, RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nPropId( impl_getPropertyId_throwUnknownProperty( _rPropertyName ) );

        OSL_ENSURE( m_pHelper.get(), "EFormsPropertyHandler::setPropertyValue: we don't have any SupportedProperties!" );
            // if we survived impl_getPropertyId_throwUnknownProperty, we should have a helper, since no helper implies no properties

        try
        {
            Any aOldValue = getPropertyValue( _rPropertyName );

            switch ( nPropId )
            {
            case PROPERTY_ID_LIST_BINDING:
            {
                Reference< XListEntrySource > xSource;
                OSL_VERIFY( _rValue >>= xSource );
                m_pHelper->setListSourceBinding( xSource );
            }
            break;

            case PROPERTY_ID_XML_DATA_MODEL:
            {
                OSL_VERIFY( _rValue >>= m_sBindingLessModelName );

                // if the model changed, reset the binding to NULL
                if ( m_pHelper->getCurrentFormModelName() != m_sBindingLessModelName )
                {
                    OUString sOldBindingName = m_pHelper->getCurrentBindingName();
                    m_pHelper->setBinding( nullptr );
                    firePropertyChange( PROPERTY_BINDING_NAME, PROPERTY_ID_BINDING_NAME,
                        makeAny( sOldBindingName ), makeAny( OUString() ) );
                }
            }
            break;

            case PROPERTY_ID_BINDING_NAME:
            {
                OUString sNewBindingName;
                OSL_VERIFY( _rValue >>= sNewBindingName );

                bool bPreviouslyEmptyModel = !m_pHelper->getCurrentFormModel().is();

                Reference< XPropertySet > xNewBinding;
                if ( !sNewBindingName.isEmpty() )
                    // obtain the binding with this name, for the current model
                    xNewBinding = m_pHelper->getOrCreateBindingForModel( getModelNamePropertyValue(), sNewBindingName );

                m_pHelper->setBinding( xNewBinding );

                if ( bPreviouslyEmptyModel )
                {   // simulate a property change for the model property
                    // This is because we "simulate" the Model property by remembering the
                    // value ourself. Other instances might, however, not know this value,
                    // but prefer to retrieve it somewhere else - e.g. from the EFormsHelper

                    // The really correct solution would be if *all* property handlers
                    // obtain a "current property value" for *all* properties from a central
                    // instance. Then, handler A could ask it for the value of property
                    // X, and this request would be re-routed to handler B, which ultimately
                    // knows the current value.
                    // However, there's no such mechanism in place currently.
                    m_bSimulatingModelChange = true;
                    firePropertyChange( PROPERTY_XML_DATA_MODEL, PROPERTY_ID_XML_DATA_MODEL,
                        makeAny( OUString() ), makeAny( getModelNamePropertyValue() ) );
                    m_bSimulatingModelChange = false;
                }
            }
            break;

            case PROPERTY_ID_BIND_EXPRESSION:
            {
                Reference< XPropertySet > xBinding( m_pHelper->getCurrentBinding() );
                OSL_ENSURE( xBinding.is(), "You should not reach this without an active binding!" );
                if ( xBinding.is() )
                    xBinding->setPropertyValue( PROPERTY_BIND_EXPRESSION, _rValue );
            }
            break;

            case PROPERTY_ID_XSD_REQUIRED:
            case PROPERTY_ID_XSD_RELEVANT:
            case PROPERTY_ID_XSD_READONLY:
            case PROPERTY_ID_XSD_CONSTRAINT:
            case PROPERTY_ID_XSD_CALCULATION:
            {
                Reference< XPropertySet > xBindingProps( m_pHelper->getCurrentBinding() );
                DBG_ASSERT( xBindingProps.is(), "EFormsPropertyHandler::setPropertyValue: how can I set a property if there's no binding?" );
                if ( xBindingProps.is() )
                {
                    DBG_ASSERT( _rValue.getValueType().equals( ::cppu::UnoType<OUString>::get() ),
                        "EFormsPropertyHandler::setPropertyValue: invalid value type!" );
                    xBindingProps->setPropertyValue( _rPropertyName, _rValue );
                }
            }
            break;

            default:
                OSL_FAIL( "EFormsPropertyHandler::setPropertyValue: cannot handle this property!" );
                break;
            }

            impl_setContextDocumentModified_nothrow();

            Any aNewValue( getPropertyValue( _rPropertyName ) );
            firePropertyChange( _rPropertyName, nPropId, aOldValue, aNewValue );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "EFormsPropertyHandler::setPropertyValue: caught an exception!" );
        }
    }


    void EFormsPropertyHandler::onNewComponent()
    {
        EFormsPropertyHandler_Base::onNewComponent();

        Reference< frame::XModel > xDocument( impl_getContextDocument_nothrow() );
        DBG_ASSERT( xDocument.is(), "EFormsPropertyHandler::onNewComponent: no document!" );
        if ( EFormsHelper::isEForm( xDocument ) )
            m_pHelper.reset( new EFormsHelper( m_aMutex, m_xComponent, xDocument ) );
        else
            m_pHelper.reset();
    }


    Sequence< Property > SAL_CALL EFormsPropertyHandler::doDescribeSupportedProperties() const
    {
        ::std::vector< Property > aProperties;

        if ( m_pHelper.get() )
        {
            if ( m_pHelper->canBindToAnyDataType() )
            {
                aProperties.reserve( 7 );
                addStringPropertyDescription( aProperties, PROPERTY_XML_DATA_MODEL );
                addStringPropertyDescription( aProperties, PROPERTY_BINDING_NAME );
                addStringPropertyDescription( aProperties, PROPERTY_BIND_EXPRESSION );
                addStringPropertyDescription( aProperties, PROPERTY_XSD_REQUIRED );
                addStringPropertyDescription( aProperties, PROPERTY_XSD_RELEVANT );
                addStringPropertyDescription( aProperties, PROPERTY_XSD_READONLY );
                addStringPropertyDescription( aProperties, PROPERTY_XSD_CONSTRAINT );
                addStringPropertyDescription( aProperties, PROPERTY_XSD_CALCULATION );
            }
            if ( m_pHelper->isListEntrySink() )
            {
                implAddPropertyDescription( aProperties, PROPERTY_LIST_BINDING,
                    cppu::UnoType<XListEntrySource>::get() );
            }
        }

        if ( aProperties.empty() )
            return Sequence< Property >();
        return Sequence< Property >( &(*aProperties.begin()), aProperties.size() );
    }


    Any SAL_CALL EFormsPropertyHandler::convertToPropertyValue( const OUString& _rPropertyName, const Any& _rControlValue ) throw (UnknownPropertyException, RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        Any aReturn;

        OSL_ENSURE( m_pHelper.get(), "EFormsPropertyHandler::convertToPropertyValue: we have no SupportedProperties!" );
        if ( !m_pHelper.get() )
            return aReturn;

        PropertyId nPropId( m_pInfoService->getPropertyId( _rPropertyName ) );

        OUString sControlValue;
        switch ( nPropId )
        {
        case PROPERTY_ID_LIST_BINDING:
        {
            OSL_VERIFY( _rControlValue >>= sControlValue );
            Reference< XListEntrySource > xListSource( m_pHelper->getModelElementFromUIName( EFormsHelper::Binding, sControlValue ), UNO_QUERY );
            OSL_ENSURE( xListSource.is() || !m_pHelper->getModelElementFromUIName( EFormsHelper::Binding, sControlValue ).is(),
                "EFormsPropertyHandler::convertToPropertyValue: there's a binding which is no ListEntrySource!" );
            aReturn <<= xListSource;
        }
        break;

        default:
            aReturn = EFormsPropertyHandler_Base::convertToPropertyValue( _rPropertyName, _rControlValue );
            break;
        }

        return aReturn;
    }


    Any SAL_CALL EFormsPropertyHandler::convertToControlValue( const OUString& _rPropertyName, const Any& _rPropertyValue, const Type& _rControlValueType ) throw (UnknownPropertyException, RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        Any aReturn;

        OSL_ENSURE( m_pHelper.get(), "EFormsPropertyHandler::convertToControlValue: we have no SupportedProperties!" );
        if ( !m_pHelper.get() )
            return aReturn;

        PropertyId nPropId( m_pInfoService->getPropertyId( _rPropertyName ) );

        OSL_ENSURE( _rControlValueType.getTypeClass() == TypeClass_STRING,
            "EFormsPropertyHandler::convertToControlValue: all our controls should use strings for value exchange!" );

        switch ( nPropId )
        {
        case PROPERTY_ID_LIST_BINDING:
        {
            Reference< XPropertySet > xListSourceBinding( _rPropertyValue, UNO_QUERY );
            if ( xListSourceBinding.is() )
                aReturn <<= EFormsHelper::getModelElementUIName( EFormsHelper::Binding, xListSourceBinding );
        }
        break;

        default:
            aReturn = EFormsPropertyHandler_Base::convertToControlValue( _rPropertyName, _rPropertyValue, _rControlValueType );
            break;
        }

        return aReturn;
    }


    Sequence< OUString > SAL_CALL EFormsPropertyHandler::getActuatingProperties( ) throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !m_pHelper.get() )
            return Sequence< OUString >();

        ::std::vector< OUString > aInterestedInActuations( 2 );
        aInterestedInActuations[ 0 ] = PROPERTY_XML_DATA_MODEL;
        aInterestedInActuations[ 1 ] = PROPERTY_BINDING_NAME;
        return Sequence< OUString >( &(*aInterestedInActuations.begin()), aInterestedInActuations.size() );
    }


    Sequence< OUString > SAL_CALL EFormsPropertyHandler::getSupersededProperties( ) throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !m_pHelper.get() )
            return Sequence< OUString >();

        Sequence<OUString> aReturn { PROPERTY_INPUT_REQUIRED };
        return aReturn;
    }


    LineDescriptor SAL_CALL EFormsPropertyHandler::describePropertyLine( const OUString& _rPropertyName,
        const Reference< XPropertyControlFactory >& _rxControlFactory )
        throw (UnknownPropertyException, NullPointerException, RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !_rxControlFactory.is() )
            throw NullPointerException();
        if ( !m_pHelper.get() )
            throw RuntimeException();

        LineDescriptor aDescriptor;
        sal_Int16 nControlType = PropertyControlType::TextField;
        ::std::vector< OUString > aListEntries;
        PropertyId nPropId( impl_getPropertyId_throwUnknownProperty( _rPropertyName ) );
        switch ( nPropId )
        {
        case PROPERTY_ID_LIST_BINDING:
            nControlType = PropertyControlType::ListBox;
            m_pHelper.get()->getAllElementUINames( EFormsHelper::Binding, aListEntries, true );
            break;

        case PROPERTY_ID_XML_DATA_MODEL:
            nControlType = PropertyControlType::ListBox;
            m_pHelper->getFormModelNames( aListEntries );
            break;

        case PROPERTY_ID_BINDING_NAME:
        {
            nControlType = PropertyControlType::ComboBox;
            OUString sCurrentModel( getModelNamePropertyValue() );
            if ( !sCurrentModel.isEmpty() )
                m_pHelper->getBindingNames( sCurrentModel, aListEntries );
        }
        break;

        case PROPERTY_ID_BIND_EXPRESSION:   aDescriptor.PrimaryButtonId = UID_PROP_DLG_BIND_EXPRESSION; break;
        case PROPERTY_ID_XSD_REQUIRED:      aDescriptor.PrimaryButtonId = UID_PROP_DLG_XSD_REQUIRED;    break;
        case PROPERTY_ID_XSD_RELEVANT:      aDescriptor.PrimaryButtonId = UID_PROP_DLG_XSD_RELEVANT;    break;
        case PROPERTY_ID_XSD_READONLY:      aDescriptor.PrimaryButtonId = UID_PROP_DLG_XSD_READONLY;    break;
        case PROPERTY_ID_XSD_CONSTRAINT:    aDescriptor.PrimaryButtonId = UID_PROP_DLG_XSD_CONSTRAINT;  break;
        case PROPERTY_ID_XSD_CALCULATION:   aDescriptor.PrimaryButtonId = UID_PROP_DLG_XSD_CALCULATION; break;

        default:
            OSL_FAIL( "EFormsPropertyHandler::describePropertyLine: cannot handle this property!" );
            break;
        }

        switch ( nControlType )
        {
        case PropertyControlType::ListBox:
            aDescriptor.Control = PropertyHandlerHelper::createListBoxControl( _rxControlFactory, aListEntries, false, true );
            break;
        case PropertyControlType::ComboBox:
            aDescriptor.Control = PropertyHandlerHelper::createComboBoxControl( _rxControlFactory, aListEntries, false, true );
            break;
        default:
            aDescriptor.Control = _rxControlFactory->createPropertyControl( nControlType, sal_False );
            break;
        }

        aDescriptor.DisplayName = m_pInfoService->getPropertyTranslation( nPropId );
        aDescriptor.Category = "Data";
        aDescriptor.HelpURL = HelpIdUrl::getHelpURL( m_pInfoService->getPropertyHelpId( nPropId ) );
        return aDescriptor;
    }


    InteractiveSelectionResult SAL_CALL EFormsPropertyHandler::onInteractivePropertySelection( const OUString& _rPropertyName, sal_Bool /*_bPrimary*/, Any& _rData, const Reference< XObjectInspectorUI >& _rxInspectorUI ) throw (UnknownPropertyException, NullPointerException, RuntimeException, std::exception)
    {
        if ( !_rxInspectorUI.is() )
            throw NullPointerException();

        ::osl::MutexGuard aGuard( m_aMutex );
        OSL_ENSURE( m_pHelper.get(), "EFormsPropertyHandler::onInteractivePropertySelection: we do not have any SupportedProperties!" );
        if ( !m_pHelper.get() )
            return InteractiveSelectionResult_Cancelled;

        PropertyId nPropId( impl_getPropertyId_throwUnknownProperty( _rPropertyName ) );
        (void)nPropId;
        OSL_ENSURE( ( PROPERTY_ID_BINDING_NAME == nPropId )
                 || ( PROPERTY_ID_BIND_EXPRESSION == nPropId )
                 || ( PROPERTY_ID_XSD_REQUIRED == nPropId )
                 || ( PROPERTY_ID_XSD_RELEVANT == nPropId )
                 || ( PROPERTY_ID_XSD_READONLY == nPropId )
                 || ( PROPERTY_ID_XSD_CONSTRAINT == nPropId )
                 || ( PROPERTY_ID_XSD_CALCULATION == nPropId ), "EFormsPropertyHandler::onInteractivePropertySelection: unexpected!" );

        try
        {
            Reference< XExecutableDialog > xDialog;
            xDialog.set( m_xContext->getServiceManager()->createInstanceWithContext( "com.sun.star.xforms.ui.dialogs.AddCondition", m_xContext ), UNO_QUERY );
            Reference< XPropertySet > xDialogProps( xDialog, UNO_QUERY_THROW );

            // the model for the dialog to work with
            Reference< xforms::XModel > xModel( m_pHelper->getCurrentFormModel() );
            // the binding for the dialog to work with
            Reference< XPropertySet > xBinding( m_pHelper->getCurrentBinding() );
            // the aspect of the binding which the dialog should modify
            OUString sFacetName( _rPropertyName );

            OSL_ENSURE( xModel.is() && xBinding.is() && !sFacetName.isEmpty(),
                "EFormsPropertyHandler::onInteractivePropertySelection: something is missing for the dialog initialization!" );
            if ( !( xModel.is() && xBinding.is() && !sFacetName.isEmpty() ) )
                return InteractiveSelectionResult_Cancelled;

            xDialogProps->setPropertyValue("FormModel", makeAny( xModel ) );
            xDialogProps->setPropertyValue("Binding", makeAny( xBinding ) );
            xDialogProps->setPropertyValue("FacetName", makeAny( sFacetName ) );

            if ( !xDialog->execute() )
                // cancelled
                return InteractiveSelectionResult_Cancelled;

            _rData = xDialogProps->getPropertyValue("ConditionValue");
            return InteractiveSelectionResult_ObtainedValue;
        }
        catch( const Exception& )
        {
            OSL_FAIL( "EFormsPropertyHandler::onInteractivePropertySelection: caught an exception!" );
        }

        // something went wrong here ...(but has been asserted already)
        return InteractiveSelectionResult_Cancelled;
    }


    void SAL_CALL EFormsPropertyHandler::addPropertyChangeListener( const Reference< XPropertyChangeListener >& _rxListener ) throw (NullPointerException, RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        EFormsPropertyHandler_Base::addPropertyChangeListener( _rxListener );
        if ( m_pHelper.get() )
            m_pHelper->registerBindingListener( _rxListener );
    }


    void SAL_CALL EFormsPropertyHandler::removePropertyChangeListener( const Reference< XPropertyChangeListener >& _rxListener ) throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( m_pHelper.get() )
            m_pHelper->revokeBindingListener( _rxListener );
        EFormsPropertyHandler_Base::removePropertyChangeListener( _rxListener );
    }


    void SAL_CALL EFormsPropertyHandler::actuatingPropertyChanged( const OUString& _rActuatingPropertyName, const Any& _rNewValue, const Any& /*_rOldValue*/, const Reference< XObjectInspectorUI >& _rxInspectorUI, sal_Bool ) throw (NullPointerException, RuntimeException, std::exception)
    {
        if ( !_rxInspectorUI.is() )
            throw NullPointerException();

        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nActuatingPropId( impl_getPropertyId_throwRuntime( _rActuatingPropertyName ) );
        OSL_PRECOND( m_pHelper.get(), "EFormsPropertyHandler::actuatingPropertyChanged: inconsistentcy!" );
            // if we survived impl_getPropertyId_throwRuntime, we should have a helper, since no helper implies no properties

        DBG_ASSERT( _rxInspectorUI.is(), "EFormsPropertyHandler::actuatingPropertyChanged: invalid callback!" );
        if ( !_rxInspectorUI.is() )
            return;

        switch ( nActuatingPropId )
        {
        case PROPERTY_ID_XML_DATA_MODEL:
        {
            if ( m_bSimulatingModelChange )
                break;
            OUString sDataModelName;
            OSL_VERIFY( _rNewValue >>= sDataModelName );
            bool bBoundToSomeModel = !sDataModelName.isEmpty();
            _rxInspectorUI->rebuildPropertyUI( PROPERTY_BINDING_NAME );
            _rxInspectorUI->enablePropertyUI( PROPERTY_BINDING_NAME, bBoundToSomeModel );
            SAL_FALLTHROUGH;
        }

        case PROPERTY_ID_BINDING_NAME:
        {
            bool bHaveABinding = !m_pHelper->getCurrentBindingName().isEmpty();
            _rxInspectorUI->enablePropertyUI( PROPERTY_BIND_EXPRESSION, bHaveABinding );
            _rxInspectorUI->enablePropertyUI( PROPERTY_XSD_REQUIRED, bHaveABinding );
            _rxInspectorUI->enablePropertyUI( PROPERTY_XSD_RELEVANT, bHaveABinding );
            _rxInspectorUI->enablePropertyUI( PROPERTY_XSD_READONLY, bHaveABinding );
            _rxInspectorUI->enablePropertyUI( PROPERTY_XSD_CONSTRAINT, bHaveABinding );
            _rxInspectorUI->enablePropertyUI( PROPERTY_XSD_CALCULATION, bHaveABinding );
            _rxInspectorUI->enablePropertyUI( PROPERTY_XSD_DATA_TYPE, bHaveABinding );
        }
        break;

        default:
            OSL_FAIL( "EFormsPropertyHandler::actuatingPropertyChanged: cannot handle this property!" );
            break;
        }
    }


} // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
