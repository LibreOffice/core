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

#include "eformshelper.hxx"
#include "formstrings.hxx"
#include "formresid.hrc"
#include "modulepcr.hxx"
#include "propeventtranslation.hxx"
#include "formbrowsertools.hxx"

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/xforms/XFormsUIHelper1.hpp>
#include <com/sun/star/xsd/DataTypeClass.hpp>
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <tools/diagnose_ex.h>
#include <rtl/ustrbuf.hxx>

#include <functional>
#include <algorithm>
#include <o3tl/compat_functional.hxx>

//........................................................................
namespace pcr
{
//........................................................................

    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::form::binding;
    using namespace ::com::sun::star::xsd;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::form;

    //====================================================================
    //= file-local helpers
    //====================================================================
    namespace
    {
        //--------------------------------------------------------------------
        OUString composeModelElementUIName( const OUString& _rModelName, const OUString& _rElementName )
        {
            OUStringBuffer aBuffer;
            aBuffer.appendAscii( "[" );
            aBuffer.append( _rModelName );
            aBuffer.appendAscii( "] " );
            aBuffer.append( _rElementName );
            return aBuffer.makeStringAndClear();
        }
    }

    //====================================================================
    //= EFormsHelper
    //====================================================================
    //--------------------------------------------------------------------
    EFormsHelper::EFormsHelper( ::osl::Mutex& _rMutex, const Reference< XPropertySet >& _rxControlModel, const Reference< frame::XModel >& _rxContextDocument )
        :m_xControlModel( _rxControlModel )
        ,m_aPropertyListeners( _rMutex )
    {
        OSL_ENSURE( _rxControlModel.is(), "EFormsHelper::EFormsHelper: invalid control model!" );
        m_xBindableControl = m_xBindableControl.query( _rxControlModel );

        m_xDocument = m_xDocument.query( _rxContextDocument );
        OSL_ENSURE( m_xDocument.is(), "EFormsHelper::EFormsHelper: invalid document!" );

    }

    //--------------------------------------------------------------------
    bool EFormsHelper::isEForm( const Reference< frame::XModel >& _rxContextDocument )
    {
        try
        {
            Reference< xforms::XFormsSupplier > xDocument( _rxContextDocument, UNO_QUERY );
            if ( !xDocument.is() )
                return false;

            return xDocument->getXForms().is();
        }
        catch( const Exception& )
        {
            OSL_FAIL( "EFormsHelper::isEForm: caught an exception!" );
        }
        return false;
    }

    //--------------------------------------------------------------------
    bool EFormsHelper::canBindToDataType( sal_Int32 _nDataType ) const SAL_THROW(())
    {
        if ( !m_xBindableControl.is() )
            // cannot bind at all
            return false;

        // some types cannot be bound, independent from the control type
        if (  ( DataTypeClass::hexBinary == _nDataType )
           || ( DataTypeClass::base64Binary == _nDataType )
           || ( DataTypeClass::QName == _nDataType )
           || ( DataTypeClass::NOTATION == _nDataType )
           )
           return false;

        bool bCan = false;
        try
        {
            // classify the control model
            sal_Int16 nControlType = FormComponentType::CONTROL;
            OSL_VERIFY( m_xControlModel->getPropertyValue( PROPERTY_CLASSID ) >>= nControlType );

            // some lists
            sal_Int16 nNumericCompatibleTypes[] = { DataTypeClass::DECIMAL, DataTypeClass::FLOAT, DataTypeClass::DOUBLE, 0 };
            sal_Int16 nDateCompatibleTypes[] = { DataTypeClass::DATE, 0 };
            sal_Int16 nTimeCompatibleTypes[] = { DataTypeClass::TIME, 0 };
            sal_Int16 nCheckboxCompatibleTypes[] = { DataTypeClass::BOOLEAN, DataTypeClass::STRING, DataTypeClass::anyURI, 0 };
            sal_Int16 nRadiobuttonCompatibleTypes[] = { DataTypeClass::STRING, DataTypeClass::anyURI, 0 };
            sal_Int16 nFormattedCompatibleTypes[] = { DataTypeClass::DECIMAL, DataTypeClass::FLOAT, DataTypeClass::DOUBLE, DataTypeClass::DATETIME, DataTypeClass::DATE, DataTypeClass::TIME, 0 };

            sal_Int16* pCompatibleTypes = NULL;
            switch ( nControlType )
            {
            case FormComponentType::SPINBUTTON:
            case FormComponentType::NUMERICFIELD:
                pCompatibleTypes = nNumericCompatibleTypes;
                break;
            case FormComponentType::DATEFIELD:
                pCompatibleTypes = nDateCompatibleTypes;
                break;
            case FormComponentType::TIMEFIELD:
                pCompatibleTypes = nTimeCompatibleTypes;
                break;
            case FormComponentType::CHECKBOX:
                pCompatibleTypes = nCheckboxCompatibleTypes;
                break;
            case FormComponentType::RADIOBUTTON:
                pCompatibleTypes = nRadiobuttonCompatibleTypes;
                break;

            case FormComponentType::TEXTFIELD:
            {
                // both the normal text field, and the formatted field, claim to be a TEXTFIELD
                // need to distinguish by service name
                Reference< XServiceInfo > xSI( m_xControlModel, UNO_QUERY );
                OSL_ENSURE( xSI.is(), "EFormsHelper::canBindToDataType: a control model which has no service info?" );
                if ( xSI.is() )
                {
                    if ( xSI->supportsService( SERVICE_COMPONENT_FORMATTEDFIELD ) )
                    {
                        pCompatibleTypes = nFormattedCompatibleTypes;
                        break;
                    }
                }
                // NO break here!
            }
            case FormComponentType::LISTBOX:
            case FormComponentType::COMBOBOX:
                // edit fields and list/combo boxes can be bound to anything
                bCan = true;
            }

            if ( !bCan && pCompatibleTypes )
            {
                if ( _nDataType == -1 )
                {
                    // the control can be bound to at least one type, and exactly this is being asked for
                    bCan = true;
                }
                else
                {
                    while ( *pCompatibleTypes && !bCan )
                        bCan = ( *pCompatibleTypes++ == _nDataType );
                }
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "EFormsHelper::canBindToDataType: caught an exception!" );
        }

        return bCan;
    }

    //--------------------------------------------------------------------
    bool EFormsHelper::isListEntrySink() const SAL_THROW(())
    {
        bool bIs = false;
        try
        {
            Reference< XListEntrySink > xAsSink( m_xControlModel, UNO_QUERY );
            bIs = xAsSink.is();
        }
        catch( const Exception& )
        {
            OSL_FAIL( "EFormsHelper::isListEntrySink: caught an exception!" );
        }
        return bIs;
    }

    //--------------------------------------------------------------------
    void EFormsHelper::impl_switchBindingListening_throw( bool _bDoListening, const Reference< XPropertyChangeListener >& _rxListener )
    {
        Reference< XPropertySet > xBindingProps;
        if ( m_xBindableControl.is() )
            xBindingProps = xBindingProps.query( m_xBindableControl->getValueBinding() );
        if ( !xBindingProps.is() )
            return;

        if ( _bDoListening )
        {
            xBindingProps->addPropertyChangeListener( OUString(), _rxListener );
        }
        else
        {
            xBindingProps->removePropertyChangeListener( OUString(), _rxListener );
        }
    }

    //--------------------------------------------------------------------
    void EFormsHelper::registerBindingListener( const Reference< XPropertyChangeListener >& _rxBindingListener )
    {
        if ( !_rxBindingListener.is() )
            return;
        impl_toggleBindingPropertyListening_throw( true, _rxBindingListener );
    }

    //--------------------------------------------------------------------
    void EFormsHelper::impl_toggleBindingPropertyListening_throw( bool _bDoListen, const Reference< XPropertyChangeListener >& _rxConcreteListenerOrNull )
    {
        if ( !_bDoListen )
        {
            SAL_WNODEPRECATED_DECLARATIONS_PUSH
            ::std::auto_ptr< ::cppu::OInterfaceIteratorHelper > pListenerIterator = m_aPropertyListeners.createIterator();
            SAL_WNODEPRECATED_DECLARATIONS_POP
            while ( pListenerIterator->hasMoreElements() )
            {
                PropertyEventTranslation* pTranslator = dynamic_cast< PropertyEventTranslation* >( pListenerIterator->next() );
                OSL_ENSURE( pTranslator, "EFormsHelper::impl_toggleBindingPropertyListening_throw: invalid listener element in my container!" );
                if ( !pTranslator )
                    continue;

                Reference< XPropertyChangeListener > xEventSourceTranslator( pTranslator );
                if ( _rxConcreteListenerOrNull.is() )
                {
                    if ( pTranslator->getDelegator() == _rxConcreteListenerOrNull )
                    {
                        impl_switchBindingListening_throw( false, xEventSourceTranslator );
                        m_aPropertyListeners.removeListener( xEventSourceTranslator );
                        break;
                    }
                }
                else
                {
                    impl_switchBindingListening_throw( false, xEventSourceTranslator );
                }
            }
        }
        else
        {
            if ( _rxConcreteListenerOrNull.is() )
            {
                Reference< XPropertyChangeListener > xEventSourceTranslator( new PropertyEventTranslation( _rxConcreteListenerOrNull, m_xBindableControl ) );
                m_aPropertyListeners.addListener( xEventSourceTranslator );
                impl_switchBindingListening_throw( true, xEventSourceTranslator );
            }
            else
            {
                SAL_WNODEPRECATED_DECLARATIONS_PUSH
                ::std::auto_ptr< ::cppu::OInterfaceIteratorHelper > pListenerIterator = m_aPropertyListeners.createIterator();
                SAL_WNODEPRECATED_DECLARATIONS_POP
                while ( pListenerIterator->hasMoreElements() )
                {
                    Reference< XPropertyChangeListener > xListener( pListenerIterator->next(), UNO_QUERY );
                    impl_switchBindingListening_throw( true, xListener );
                }
            }
        }
    }

    //--------------------------------------------------------------------
    void EFormsHelper::revokeBindingListener( const Reference< XPropertyChangeListener >& _rxBindingListener )
    {
        impl_toggleBindingPropertyListening_throw( false, _rxBindingListener );
    }

    //--------------------------------------------------------------------
    void EFormsHelper::getFormModelNames( ::std::vector< OUString >& /* [out] */ _rModelNames ) const SAL_THROW(())
    {
        if ( m_xDocument.is() )
        {
            try
            {
                _rModelNames.resize( 0 );

                Reference< XNameContainer > xForms( m_xDocument->getXForms() );
                OSL_ENSURE( xForms.is(), "EFormsHelper::getFormModelNames: invalid forms container!" );
                if ( xForms.is() )
                {
                    Sequence< OUString > aModelNames = xForms->getElementNames();
                    _rModelNames.resize( aModelNames.getLength() );
                    ::std::copy( aModelNames.getConstArray(), aModelNames.getConstArray() + aModelNames.getLength(),
                        _rModelNames.begin()
                    );
                }
            }
            catch( const Exception& )
            {
                OSL_FAIL( "EFormsHelper::getFormModelNames: caught an exception!" );
            }
        }
    }

    //--------------------------------------------------------------------
    void EFormsHelper::getBindingNames( const OUString& _rModelName, ::std::vector< OUString >& /* [out] */ _rBindingNames ) const SAL_THROW(())
    {
        _rBindingNames.resize( 0 );
        try
        {
            Reference< xforms::XModel > xModel( getFormModelByName( _rModelName ) );
            if ( xModel.is() )
            {
                Reference< XNameAccess > xBindings( xModel->getBindings(), UNO_QUERY );
                OSL_ENSURE( xBindings.is(), "EFormsHelper::getBindingNames: invalid bindings container obtained from the model!" );
                if ( xBindings.is() )
                {
                    Sequence< OUString > aNames = xBindings->getElementNames();
                    _rBindingNames.resize( aNames.getLength() );
                    ::std::copy( aNames.getConstArray(), aNames.getConstArray() + aNames.getLength(), _rBindingNames.begin() );
                }
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "EFormsHelper::getBindingNames: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    Reference< xforms::XModel > EFormsHelper::getFormModelByName( const OUString& _rModelName ) const SAL_THROW(())
    {
        Reference< xforms::XModel > xReturn;
        try
        {
            Reference< XNameContainer > xForms( m_xDocument->getXForms() );
            OSL_ENSURE( xForms.is(), "EFormsHelper::getFormModelByName: invalid forms container!" );
            if ( xForms.is() )
                OSL_VERIFY( xForms->getByName( _rModelName ) >>= xReturn );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "EFormsHelper::getFormModelByName: caught an exception!" );
        }
        return xReturn;
    }

    //--------------------------------------------------------------------
    Reference< xforms::XModel > EFormsHelper::getCurrentFormModel() const SAL_THROW(())
    {
        Reference< xforms::XModel > xModel;
        try
        {
            Reference< XPropertySet > xBinding( getCurrentBinding() );
            if ( xBinding.is() )
            {
                OSL_VERIFY( xBinding->getPropertyValue( PROPERTY_MODEL ) >>= xModel );
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "EFormsHelper::getCurrentFormModel: caught an exception!" );
        }
        return xModel;
    }

    //--------------------------------------------------------------------
    OUString EFormsHelper::getCurrentFormModelName() const SAL_THROW(())
    {
        OUString sModelName;
        try
        {
            Reference< xforms::XModel > xFormsModel( getCurrentFormModel() );
            if ( xFormsModel.is() )
                sModelName = xFormsModel->getID();
        }
        catch( const Exception& )
        {
            OSL_FAIL( "EFormsHelper::getCurrentFormModel: caught an exception!" );
        }
        return sModelName;
    }

    //--------------------------------------------------------------------
    Reference< XPropertySet > EFormsHelper::getCurrentBinding() const SAL_THROW(())
    {
        Reference< XPropertySet > xBinding;

        try
        {
            if ( m_xBindableControl.is() )
                xBinding = xBinding.query( m_xBindableControl->getValueBinding() );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "EFormsHelper::getCurrentBinding: caught an exception!" );
        }

        return xBinding;
    }

    //--------------------------------------------------------------------
    OUString EFormsHelper::getCurrentBindingName() const SAL_THROW(())
    {
        OUString sBindingName;
        try
        {
            Reference< XPropertySet > xBinding( getCurrentBinding() );
            if ( xBinding.is() )
                xBinding->getPropertyValue( PROPERTY_BINDING_ID ) >>= sBindingName;
        }
        catch( const Exception& )
        {
            OSL_FAIL( "EFormsHelper::getCurrentBindingName: caught an exception!" );
        }
        return sBindingName;
    }

    //--------------------------------------------------------------------
    Reference< XListEntrySource > EFormsHelper::getCurrentListSourceBinding() const SAL_THROW(())
    {
        Reference< XListEntrySource > xReturn;
        try
        {
            Reference< XListEntrySink > xAsSink( m_xControlModel, UNO_QUERY );
            OSL_ENSURE( xAsSink.is(), "EFormsHelper::getCurrentListSourceBinding: you should have used isListEntrySink before!" );
            if ( xAsSink.is() )
                xReturn = xAsSink->getListEntrySource();
        }
        catch( const Exception& )
        {
            OSL_FAIL( "EFormsHelper::getCurrentListSourceBinding: caught an exception!" );
        }
        return xReturn;
    }

    //--------------------------------------------------------------------
    void EFormsHelper::setListSourceBinding( const Reference< XListEntrySource >& _rxListSource ) SAL_THROW(())
    {
        try
        {
            Reference< XListEntrySink > xAsSink( m_xControlModel, UNO_QUERY );
            OSL_ENSURE( xAsSink.is(), "EFormsHelper::setListSourceBinding: you should have used isListEntrySink before!" );
            if ( xAsSink.is() )
                xAsSink->setListEntrySource( _rxListSource );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "EFormsHelper::setListSourceBinding: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    void EFormsHelper::setBinding( const Reference< ::com::sun::star::beans::XPropertySet >& _rxBinding ) SAL_THROW(())
    {
        if ( !m_xBindableControl.is() )
            return;

        try
        {
            Reference< XPropertySet > xOldBinding( m_xBindableControl->getValueBinding(), UNO_QUERY );

            Reference< XValueBinding > xBinding( _rxBinding, UNO_QUERY );
            OSL_ENSURE( xBinding.is() || !_rxBinding.is(), "EFormsHelper::setBinding: invalid binding!" );

            impl_toggleBindingPropertyListening_throw( false, NULL );
            m_xBindableControl->setValueBinding( xBinding );
            impl_toggleBindingPropertyListening_throw( true, NULL );

            ::std::set< OUString > aSet;
            firePropertyChanges( xOldBinding, _rxBinding, aSet );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "EFormsHelper::setBinding: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    Reference< XPropertySet > EFormsHelper::getOrCreateBindingForModel( const OUString& _rTargetModel, const OUString& _rBindingName ) const SAL_THROW(())
    {
        OSL_ENSURE( !_rBindingName.isEmpty(), "EFormsHelper::getOrCreateBindingForModel: invalid binding name!" );
        return implGetOrCreateBinding( _rTargetModel, _rBindingName );
    }

    //--------------------------------------------------------------------
    Reference< XPropertySet > EFormsHelper::implGetOrCreateBinding( const OUString& _rTargetModel, const OUString& _rBindingName ) const SAL_THROW(())
    {
        OSL_ENSURE( !( _rTargetModel.isEmpty() && !_rBindingName.isEmpty() ), "EFormsHelper::implGetOrCreateBinding: no model, but a binding name?" );

        Reference< XPropertySet > xBinding;
        try
        {
            OUString sTargetModel( _rTargetModel );
            // determine the model which the binding should belong to
            if ( sTargetModel.isEmpty() )
            {
                ::std::vector< OUString > aModelNames;
                getFormModelNames( aModelNames );
                if ( !aModelNames.empty() )
                    sTargetModel = *aModelNames.begin();
                OSL_ENSURE( !sTargetModel.isEmpty(), "EFormsHelper::implGetOrCreateBinding: unable to obtain a default model!" );
            }
            Reference< xforms::XModel > xModel( getFormModelByName( sTargetModel ) );
            Reference< XNameAccess > xBindingNames( xModel.is() ? xModel->getBindings() : Reference< XSet >(), UNO_QUERY );
            if ( xBindingNames.is() )
            {
                // get or create the binding instance
                if ( !_rBindingName.isEmpty() )
                {
                    if ( xBindingNames->hasByName( _rBindingName ) )
                        OSL_VERIFY( xBindingNames->getByName( _rBindingName ) >>= xBinding );
                    else
                    {
                        xBinding = xModel->createBinding( );
                        if ( xBinding.is() )
                        {
                            xBinding->setPropertyValue( PROPERTY_BINDING_ID, makeAny( _rBindingName ) );
                            xModel->getBindings()->insert( makeAny( xBinding ) );
                        }
                    }
                }
                else
                {
                    xBinding = xModel->createBinding( );
                    if ( xBinding.is() )
                    {
                        // find a nice name for it
                        OUString sBaseName(PcrRes(RID_STR_BINDING_UI_NAME).toString());
                        sBaseName += OUString(" ");
                        OUString sNewName;
                        sal_Int32 nNumber = 1;
                        do
                        {
                            sNewName = sBaseName + OUString::number( nNumber++ );
                        }
                        while ( xBindingNames->hasByName( sNewName ) );
                        Reference< XNamed > xName( xBinding, UNO_QUERY_THROW );
                        xName->setName( sNewName );
                        // and insert into the model
                        xModel->getBindings()->insert( makeAny( xBinding ) );
                    }
                }
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return xBinding;
    }

    //--------------------------------------------------------------------
    namespace
    {
        //................................................................
        struct PropertyBagInserter : public ::std::unary_function< Property, void >
        {
        private:
            PropertyBag& m_rProperties;

        public:
            PropertyBagInserter( PropertyBag& rProperties ) : m_rProperties( rProperties ) { }

            void operator()( const Property& _rProp )
            {
                m_rProperties.insert( _rProp );
            }
        };

        //................................................................
        Reference< XPropertySetInfo > collectPropertiesGetInfo( const Reference< XPropertySet >& _rxProps, PropertyBag& _rBag )
        {
            Reference< XPropertySetInfo > xInfo;
            if ( _rxProps.is() )
                xInfo = _rxProps->getPropertySetInfo();
            if ( xInfo.is() )
            {
                Sequence< Property > aProperties = xInfo->getProperties();
                ::std::for_each( aProperties.getConstArray(), aProperties.getConstArray() + aProperties.getLength(),
                    PropertyBagInserter( _rBag )
                );
            }
            return xInfo;
        }
    }

    //--------------------------------------------------------------------
    OUString EFormsHelper::getModelElementUIName( const EFormsHelper::ModelElementType _eType, const Reference< XPropertySet >& _rxElement ) const SAL_THROW(())
    {
        OUString sUIName;
        try
        {
            // determine the model which the element belongs to
            Reference< xforms::XFormsUIHelper1 > xHelper;
            if ( _rxElement.is() )
                _rxElement->getPropertyValue( PROPERTY_MODEL ) >>= xHelper;
            OSL_ENSURE( xHelper.is(), "EFormsHelper::getModelElementUIName: invalid element or model!" );
            if ( xHelper.is() )
            {
                OUString sElementName = ( _eType == Submission ) ? xHelper->getSubmissionName( _rxElement, sal_True ) : xHelper->getBindingName( _rxElement, sal_True );
                Reference< xforms::XModel > xModel( xHelper, UNO_QUERY_THROW );
                sUIName = composeModelElementUIName( xModel->getID(), sElementName );
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "EFormsHelper::getModelElementUIName: caught an exception!" );
        }

        return sUIName;
    }

    //--------------------------------------------------------------------
    Reference< XPropertySet > EFormsHelper::getModelElementFromUIName( const EFormsHelper::ModelElementType _eType, const OUString& _rUIName ) const SAL_THROW(())
    {
        const MapStringToPropertySet& rMapUINameToElement( ( _eType == Submission ) ? m_aSubmissionUINames : m_aBindingUINames );
        MapStringToPropertySet::const_iterator pos = rMapUINameToElement.find( _rUIName );
        OSL_ENSURE( pos != rMapUINameToElement.end(), "EFormsHelper::getModelElementFromUIName: didn't find it!" );

        return ( pos != rMapUINameToElement.end() ) ? pos->second : Reference< XPropertySet >();
    }

    //--------------------------------------------------------------------
    void EFormsHelper::getAllElementUINames( const ModelElementType _eType, ::std::vector< OUString >& /* [out] */ _rElementNames, bool _bPrepentEmptyEntry )
    {
        MapStringToPropertySet& rMapUINameToElement( ( _eType == Submission ) ? m_aSubmissionUINames : m_aBindingUINames );
        rMapUINameToElement.clear();
        _rElementNames.resize( 0 );

        if ( _bPrepentEmptyEntry )
            rMapUINameToElement[ OUString() ].clear();

        try
        {
            // obtain the model names
            ::std::vector< OUString > aModels;
            getFormModelNames( aModels );
            _rElementNames.reserve( aModels.size() * 2 );    // heuristics

            // for every model, obtain the element
            for ( ::std::vector< OUString >::const_iterator pModelName = aModels.begin();
                  pModelName != aModels.end();
                  ++pModelName
                )
            {
                Reference< xforms::XModel > xModel = getFormModelByName( *pModelName );
                OSL_ENSURE( xModel.is(), "EFormsHelper::getAllElementUINames: inconsistency in the models!" );
                Reference< xforms::XFormsUIHelper1 > xHelper( xModel, UNO_QUERY );

                Reference< XIndexAccess > xElements;
                if ( xModel.is() )
                    xElements = xElements.query( ( _eType == Submission ) ? xModel->getSubmissions() : xModel->getBindings() );
                if ( !xElements.is() )
                    break;

                sal_Int32 nElementCount = xElements->getCount();
                for ( sal_Int32 i = 0; i < nElementCount; ++i )
                {
                    Reference< XPropertySet > xElement( xElements->getByIndex( i ), UNO_QUERY );
                    OSL_ENSURE( xElement.is(), "EFormsHelper::getAllElementUINames: empty element!" );
                    if ( !xElement.is() )
                        continue;
#if OSL_DEBUG_LEVEL > 0
                    {
                        Reference< xforms::XModel > xElementsModel;
                        xElement->getPropertyValue( PROPERTY_MODEL ) >>= xElementsModel;
                        OSL_ENSURE( xElementsModel == xModel, "EFormsHelper::getAllElementUINames: inconsistency in the model-element relationship!" );
                        if ( !( xElementsModel == xModel ) )
                            xElement->setPropertyValue( PROPERTY_MODEL, makeAny( xModel ) );
                    }
#endif
                    OUString sElementName = ( _eType == Submission ) ? xHelper->getSubmissionName( xElement, sal_True ) : xHelper->getBindingName( xElement, sal_True );
                    OUString sUIName = composeModelElementUIName( *pModelName, sElementName );

                    OSL_ENSURE( rMapUINameToElement.find( sUIName ) == rMapUINameToElement.end(), "EFormsHelper::getAllElementUINames: duplicate name!" );
                    rMapUINameToElement.insert( MapStringToPropertySet::value_type( sUIName, xElement ) );
                }
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "EFormsHelper::getAllElementUINames: caught an exception!" );
        }

        _rElementNames.resize( rMapUINameToElement.size() );
        ::std::transform( rMapUINameToElement.begin(), rMapUINameToElement.end(), _rElementNames.begin(), ::o3tl::select1st< MapStringToPropertySet::value_type >() );
    }

    //--------------------------------------------------------------------
    void EFormsHelper::firePropertyChange( const OUString& _rName, const Any& _rOldValue, const Any& _rNewValue ) const
    {
        if ( m_aPropertyListeners.empty() )
            return;

        if ( _rOldValue == _rNewValue )
            return;

        try
        {
            PropertyChangeEvent aEvent;

            aEvent.Source = m_xBindableControl.get();
            aEvent.PropertyName = _rName;
            aEvent.OldValue = _rOldValue;
            aEvent.NewValue = _rNewValue;

            const_cast< EFormsHelper* >( this )->m_aPropertyListeners.notify( aEvent, &XPropertyChangeListener::propertyChange );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "EFormsHelper::firePropertyChange: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    void EFormsHelper::firePropertyChanges( const Reference< XPropertySet >& _rxOldProps, const Reference< XPropertySet >& _rxNewProps, ::std::set< OUString >& _rFilter ) const
    {
        if ( m_aPropertyListeners.empty() )
            return;

        try
        {
            PropertyBag aProperties;
            Reference< XPropertySetInfo > xOldInfo = collectPropertiesGetInfo( _rxOldProps, aProperties );
            Reference< XPropertySetInfo > xNewInfo = collectPropertiesGetInfo( _rxNewProps, aProperties );

            for ( PropertyBag::const_iterator aProp = aProperties.begin();
                  aProp != aProperties.end();
                  ++aProp
                )
            {
                if ( _rFilter.find( aProp->Name ) != _rFilter.end() )
                    continue;

                Any aOldValue( NULL, aProp->Type );
                if ( xOldInfo.is() && xOldInfo->hasPropertyByName( aProp->Name ) )
                    aOldValue = _rxOldProps->getPropertyValue( aProp->Name );

                Any aNewValue( NULL, aProp->Type );
                if ( xNewInfo.is() && xNewInfo->hasPropertyByName( aProp->Name ) )
                    aNewValue = _rxNewProps->getPropertyValue( aProp->Name );

                firePropertyChange( aProp->Name, aOldValue, aNewValue );
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "EFormsHelper::firePropertyChanges: caught an exception!" );
        }
    }

//........................................................................
} // namespace pcr
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
