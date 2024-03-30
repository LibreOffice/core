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

#include <memory>
#include "propertyhandler.hxx"
#include "formmetadata.hxx"
#include "formbrowsertools.hxx"
#include "handlerhelper.hxx"
#include "formstrings.hxx"

#include <com/sun/star/lang/NullPointerException.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/script/Converter.hpp>

#include <cppuhelper/supportsservice.hxx>
#include <tools/debug.hxx>
#include <unotools/confignode.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/syslocale.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <rtl/ref.hxx>

#include <algorithm>

namespace pcr
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::script;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::inspection;
    using namespace ::comphelper;


    PropertyHandler::PropertyHandler( const Reference< XComponentContext >& _rxContext )
        :PropertyHandler_Base( m_aMutex )
        ,m_bSupportedPropertiesAreKnown( false )
        ,m_aPropertyListeners( m_aMutex )
        ,m_xContext( _rxContext )
        ,m_pInfoService  ( new OPropertyInfoService )
    {

        m_xTypeConverter = Converter::create(_rxContext);
    }

    PropertyHandler::~PropertyHandler()
    {
    }

    void SAL_CALL PropertyHandler::inspect( const Reference< XInterface >& _rxIntrospectee )
    {
        if ( !_rxIntrospectee.is() )
            throw NullPointerException();

        ::osl::MutexGuard aGuard( m_aMutex );

        Reference< XPropertySet > xNewComponent( _rxIntrospectee, UNO_QUERY );
        if ( xNewComponent == m_xComponent )
            return;

        // remove all old property change listeners
        ::comphelper::OInterfaceIteratorHelper3 removeListener(m_aPropertyListeners);
        ::comphelper::OInterfaceIteratorHelper3 readdListener(m_aPropertyListeners);  // will copy the container as needed
        while ( removeListener.hasMoreElements() )
            removePropertyChangeListener( removeListener.next() );
        OSL_ENSURE( m_aPropertyListeners.getLength() == 0, "PropertyHandler::inspect: derived classes are expected to forward the removePropertyChangeListener call to their base class (me)!" );

        // remember the new component, and give derived classes the chance to react on it
        m_xComponent = xNewComponent;
        onNewComponent();

        // add the listeners, again
        while ( readdListener.hasMoreElements() )
            addPropertyChangeListener( readdListener.next() );
    }

    void PropertyHandler::onNewComponent()
    {
        if ( m_xComponent.is() )
            m_xComponentPropertyInfo = m_xComponent->getPropertySetInfo();
        else
            m_xComponentPropertyInfo.clear();

        m_bSupportedPropertiesAreKnown = false;
        m_aSupportedProperties.realloc( 0 );
    }

    Sequence< Property > SAL_CALL PropertyHandler::getSupportedProperties()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !m_bSupportedPropertiesAreKnown )
        {
            m_aSupportedProperties = StlSyntaxSequence<css::beans::Property>(doDescribeSupportedProperties());
            m_bSupportedPropertiesAreKnown = true;
        }
        return m_aSupportedProperties;
    }

    Sequence< OUString > SAL_CALL PropertyHandler::getSupersededProperties( )
    {
        return Sequence< OUString >();
    }

    Sequence< OUString > SAL_CALL PropertyHandler::getActuatingProperties( )
    {
        return Sequence< OUString >();
    }

    Any SAL_CALL PropertyHandler::convertToPropertyValue( const OUString& _rPropertyName, const Any& _rControlValue )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nPropId = m_pInfoService->getPropertyId( _rPropertyName );
        Property aProperty( impl_getPropertyFromName_throw( _rPropertyName ) );

        Any aPropertyValue;
        if ( !_rControlValue.hasValue() )
            // NULL is converted to NULL
            return aPropertyValue;

        if ( ( m_pInfoService->getPropertyUIFlags( nPropId ) & PROP_FLAG_ENUM ) != 0 )
        {
            OUString sControlValue;
            OSL_VERIFY( _rControlValue >>= sControlValue );
            ::rtl::Reference< IPropertyEnumRepresentation > aEnumConversion(
                new DefaultEnumRepresentation( *m_pInfoService, aProperty.Type, nPropId ) );
            // TODO/UNOize: cache those converters?
            aEnumConversion->getValueFromDescription( sControlValue, aPropertyValue );
        }
        else
            aPropertyValue = PropertyHandlerHelper::convertToPropertyValue(
                m_xContext, m_xTypeConverter, aProperty, _rControlValue );
        return aPropertyValue;
    }

    Any SAL_CALL PropertyHandler::convertToControlValue( const OUString& _rPropertyName, const Any& _rPropertyValue, const Type& _rControlValueType )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nPropId = m_pInfoService->getPropertyId( _rPropertyName );

        if ( ( m_pInfoService->getPropertyUIFlags( nPropId ) & PROP_FLAG_ENUM ) != 0 )
        {
            DBG_ASSERT( _rControlValueType.getTypeClass() == TypeClass_STRING, "PropertyHandler::convertToControlValue: ENUM, but not STRING?" );

            ::rtl::Reference< IPropertyEnumRepresentation > aEnumConversion(
                new DefaultEnumRepresentation( *m_pInfoService, _rPropertyValue.getValueType(), nPropId ) );
            // TODO/UNOize: cache those converters?
            return Any( aEnumConversion->getDescriptionForValue( _rPropertyValue ) );
        }

        return PropertyHandlerHelper::convertToControlValue(
            m_xContext, m_xTypeConverter, _rPropertyValue, _rControlValueType );
    }

    PropertyState SAL_CALL PropertyHandler::getPropertyState( const OUString& /*_rPropertyName*/ )
    {
        return PropertyState_DIRECT_VALUE;
    }

    LineDescriptor SAL_CALL PropertyHandler::describePropertyLine( const OUString& _rPropertyName,
        const Reference< XPropertyControlFactory >& _rxControlFactory )
    {
        if ( !_rxControlFactory.is() )
            throw NullPointerException();

        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nPropId( impl_getPropertyId_throwUnknownProperty( _rPropertyName ) );
        const Property& rProperty( impl_getPropertyFromId_throw( nPropId ) );

        LineDescriptor aDescriptor;
        if ( ( m_pInfoService->getPropertyUIFlags( nPropId ) & PROP_FLAG_ENUM ) != 0 )
        {
            aDescriptor.Control = PropertyHandlerHelper::createListBoxControl(
                _rxControlFactory, m_pInfoService->getPropertyEnumRepresentations( nPropId ),
                PropertyHandlerHelper::requiresReadOnlyControl( rProperty.Attributes ), false );
        }
        else
            PropertyHandlerHelper::describePropertyLine( rProperty, aDescriptor, _rxControlFactory );

        aDescriptor.HelpURL = HelpIdUrl::getHelpURL( m_pInfoService->getPropertyHelpId( nPropId ) );
        aDescriptor.DisplayName = m_pInfoService->getPropertyTranslation( nPropId );

        if ( ( m_pInfoService->getPropertyUIFlags( nPropId ) & PROP_FLAG_DATA_PROPERTY ) != 0 )
            aDescriptor.Category = "Data";
        else
            aDescriptor.Category = "General";
        return aDescriptor;
    }

    sal_Bool SAL_CALL PropertyHandler::isComposable( const OUString& _rPropertyName )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return m_pInfoService->isComposeable( _rPropertyName );
    }

    InteractiveSelectionResult SAL_CALL PropertyHandler::onInteractivePropertySelection( const OUString& /*_rPropertyName*/, sal_Bool /*_bPrimary*/, Any& /*_rData*/, const Reference< XObjectInspectorUI >& /*_rxInspectorUI*/ )
    {
        OSL_FAIL( "PropertyHandler::onInteractivePropertySelection: not implemented!" );
        return InteractiveSelectionResult_Cancelled;
    }

    void SAL_CALL PropertyHandler::actuatingPropertyChanged( const OUString& /*_rActuatingPropertyName*/, const Any& /*_rNewValue*/, const Any& /*_rOldValue*/, const Reference< XObjectInspectorUI >& /*_rxInspectorUI*/, sal_Bool /*_bFirstTimeInit*/ )
    {
        OSL_FAIL( "PropertyHandler::actuatingPropertyChanged: not implemented!" );
    }

    void SAL_CALL PropertyHandler::addPropertyChangeListener( const Reference< XPropertyChangeListener >& _rxListener )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !_rxListener.is() )
            throw NullPointerException();
        m_aPropertyListeners.addInterface( _rxListener );
    }

    void SAL_CALL PropertyHandler::removePropertyChangeListener( const Reference< XPropertyChangeListener >& _rxListener )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_aPropertyListeners.removeInterface( _rxListener );
    }

    sal_Bool SAL_CALL PropertyHandler::suspend( sal_Bool /*_bSuspend*/ )
    {
        return true;
    }

    void SAL_CALL PropertyHandler::dispose(  )
    {
        PropertyHandler_Base::WeakComponentImplHelperBase::dispose();
        m_xComponent.clear();
        m_xComponentPropertyInfo.clear();
        m_xTypeConverter.clear();
    }
    void SAL_CALL PropertyHandler::addEventListener( const css::uno::Reference< css::lang::XEventListener >& Listener )
    {
        PropertyHandler_Base::WeakComponentImplHelperBase::addEventListener( Listener );
    }
    void SAL_CALL PropertyHandler::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& Listener )
    {
        PropertyHandler_Base::WeakComponentImplHelperBase::removeEventListener( Listener );
    }


    void SAL_CALL PropertyHandler::disposing()
    {
        m_xComponent.clear();
        m_aPropertyListeners.clear();
        m_xTypeConverter.clear();
        m_aSupportedProperties.realloc( 0 );
    }

    void PropertyHandler::firePropertyChange( const OUString& _rPropName, PropertyId _nPropId, const Any& _rOldValue, const Any& _rNewValue )
    {
        PropertyChangeEvent aEvent;
        aEvent.Source = m_xComponent;
        aEvent.PropertyHandle = _nPropId;
        aEvent.PropertyName = _rPropName;
        aEvent.OldValue = _rOldValue;
        aEvent.NewValue = _rNewValue;
        m_aPropertyListeners.notifyEach( &XPropertyChangeListener::propertyChange, aEvent );
    }

    const Property* PropertyHandler::impl_getPropertyFromId_nothrow( PropertyId _nPropId ) const
    {
        const_cast< PropertyHandler* >( this )->getSupportedProperties();
        const Property* pFound = std::find_if( m_aSupportedProperties.begin(), m_aSupportedProperties.end(),
            FindPropertyByHandle( _nPropId )
        );
        if ( pFound != m_aSupportedProperties.end() )
            return pFound;
        return nullptr;
    }

    const Property& PropertyHandler::impl_getPropertyFromId_throw( PropertyId _nPropId ) const
    {
        const Property* pProperty = impl_getPropertyFromId_nothrow( _nPropId );
        if ( !pProperty )
            throw UnknownPropertyException();

        return *pProperty;
    }

    const Property& PropertyHandler::impl_getPropertyFromName_throw( const OUString& _rPropertyName ) const
    {
        const_cast< PropertyHandler* >( this )->getSupportedProperties();
        StlSyntaxSequence< Property >::const_iterator pFound = std::find_if( m_aSupportedProperties.begin(), m_aSupportedProperties.end(),
            FindPropertyByName( _rPropertyName )
        );
        if ( pFound == m_aSupportedProperties.end() )
            throw UnknownPropertyException(_rPropertyName);

        return *pFound;
    }

    void PropertyHandler::implAddPropertyDescription( std::vector< Property >& _rProperties, const OUString& _rPropertyName, const Type& _rType, sal_Int16 _nAttribs ) const
    {
        _rProperties.push_back( Property(
            _rPropertyName,
            m_pInfoService->getPropertyId( _rPropertyName ),
            _rType,
            _nAttribs
        ) );
    }

    weld::Window* PropertyHandler::impl_getDefaultDialogFrame_nothrow() const
    {
        return PropertyHandlerHelper::getDialogParentFrame(m_xContext);
    }

    PropertyId PropertyHandler::impl_getPropertyId_throwUnknownProperty( const OUString& _rPropertyName ) const
    {
        PropertyId nPropId = m_pInfoService->getPropertyId( _rPropertyName );
        if ( nPropId == -1 )
            throw UnknownPropertyException(_rPropertyName);
        return nPropId;
    }

    PropertyId PropertyHandler::impl_getPropertyId_throwRuntime( const OUString& _rPropertyName ) const
    {
        PropertyId nPropId = m_pInfoService->getPropertyId( _rPropertyName );
        if ( nPropId == -1 )
            throw RuntimeException();
        return nPropId;
    }

    PropertyId PropertyHandler::impl_getPropertyId_nothrow( const OUString& _rPropertyName ) const
    {
        return m_pInfoService->getPropertyId( _rPropertyName );
    }

    void PropertyHandler::impl_setContextDocumentModified_nothrow() const
    {
        Reference< XModifiable > xModifiable( impl_getContextDocument_nothrow(), UNO_QUERY );
        if ( xModifiable.is() )
            xModifiable->setModified( true );
    }

    bool PropertyHandler::impl_componentHasProperty_throw( const OUString& _rPropName ) const
    {
        return m_xComponentPropertyInfo.is() && m_xComponentPropertyInfo->hasPropertyByName( _rPropName );
    }

    sal_Int16 PropertyHandler::impl_getDocumentMeasurementUnit_throw() const
    {
        FieldUnit eUnit = FieldUnit::NONE;

        Reference< XServiceInfo > xDocumentSI( impl_getContextDocument_nothrow(), UNO_QUERY );
        OSL_ENSURE( xDocumentSI.is(), "PropertyHandlerHelper::impl_getDocumentMeasurementUnit_throw: No context document - where do I live?" );
        if ( xDocumentSI.is() )
        {
            // determine the application type we live in
            OUString sConfigurationLocation;
            OUString sConfigurationProperty;
            if ( xDocumentSI->supportsService( SERVICE_WEB_DOCUMENT ) )
            {   // writer
                sConfigurationLocation = "/org.openoffice.Office.WriterWeb/Layout/Other";
                sConfigurationProperty = "MeasureUnit";
            }
            else if ( xDocumentSI->supportsService( SERVICE_TEXT_DOCUMENT ) )
            {   // writer
                sConfigurationLocation = "/org.openoffice.Office.Writer/Layout/Other";
                sConfigurationProperty = "MeasureUnit";
            }
            else if ( xDocumentSI->supportsService( SERVICE_SPREADSHEET_DOCUMENT ) )
            {   // calc
                sConfigurationLocation = "/org.openoffice.Office.Calc/Layout/Other/MeasureUnit";
                sConfigurationProperty = "Metric";
            }
            else if ( xDocumentSI->supportsService( SERVICE_DRAWING_DOCUMENT ) )
            {
                sConfigurationLocation = "/org.openoffice.Office.Draw/Layout/Other/MeasureUnit";
                sConfigurationProperty = "Metric";
            }
            else if ( xDocumentSI->supportsService( SERVICE_PRESENTATION_DOCUMENT ) )
            {
                sConfigurationLocation = "/org.openoffice.Office.Impress/Layout/Other/MeasureUnit";
                sConfigurationProperty = "Metric";
            }

            // read the measurement unit from the configuration
            if ( !(sConfigurationLocation.isEmpty() || sConfigurationProperty.isEmpty()) )
            {
                ::utl::OConfigurationTreeRoot aConfigTree( ::utl::OConfigurationTreeRoot::createWithComponentContext(
                    m_xContext, sConfigurationLocation, -1, ::utl::OConfigurationTreeRoot::CM_READONLY ) );
                sal_Int32 nUnitAsInt = sal_Int32(FieldUnit::NONE);
                aConfigTree.getNodeValue( sConfigurationProperty ) >>= nUnitAsInt;

                // if this denotes a valid (and accepted) unit, then use it
                if  ( ( nUnitAsInt > sal_Int32(FieldUnit::NONE) ) && ( nUnitAsInt <= sal_Int32(FieldUnit::MM_100TH) ) )
                    eUnit = static_cast< FieldUnit >( nUnitAsInt );
            }
        }

        if ( FieldUnit::NONE == eUnit )
        {
            MeasurementSystem eSystem = SvtSysLocale().GetLocaleData().getMeasurementSystemEnum();
            eUnit = MeasurementSystem::Metric == eSystem ? FieldUnit::CM : FieldUnit::INCH;
        }

        return VCLUnoHelper::ConvertToMeasurementUnit( eUnit, 1 );
    }

    PropertyHandlerComponent::PropertyHandlerComponent( const Reference< XComponentContext >& _rxContext )
        :PropertyHandler( _rxContext )
    {
    }

    IMPLEMENT_FORWARD_XINTERFACE2( PropertyHandlerComponent, PropertyHandler, PropertyHandlerComponent_Base )
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( PropertyHandlerComponent, PropertyHandler, PropertyHandlerComponent_Base )

    sal_Bool SAL_CALL PropertyHandlerComponent::supportsService( const OUString& ServiceName )
    {
        return cppu::supportsService(this, ServiceName);
    }

}   // namespace pcr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
