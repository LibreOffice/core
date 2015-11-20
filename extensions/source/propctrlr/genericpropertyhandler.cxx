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

#include "genericpropertyhandler.hxx"
#include "formmetadata.hxx"
#include "handlerhelper.hxx"
#include "pcrservices.hxx"

#include <boost/noncopyable.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/reflection/XEnumTypeDescription.hpp>
#include <com/sun/star/beans/theIntrospection.hpp>
#include <com/sun/star/inspection/PropertyControlType.hpp>
#include <com/sun/star/inspection/XHyperlinkControl.hpp>
#include <com/sun/star/awt/XActionListener.hpp>
#include <com/sun/star/script/Converter.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/extract.hxx>
#include <tools/debug.hxx>

#include <algorithm>
#include <o3tl/functional.hxx>

extern "C" void SAL_CALL createRegistryInfo_GenericPropertyHandler()
{
    ::pcr::OAutoRegistration< ::pcr::GenericPropertyHandler > aAutoRegistration;
}

namespace pcr
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::script;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::reflection;
    using namespace ::com::sun::star::inspection;
    using ::com::sun::star::awt::XActionListener;
    using ::com::sun::star::awt::ActionEvent;

    class EnumRepresentation:
        public IPropertyEnumRepresentation, private boost::noncopyable
    {
    private:
        Reference< XEnumTypeDescription >   m_xTypeDescription;
        Type                                m_aEnumType;

    public:
        EnumRepresentation( const Reference< XComponentContext >& _rxContext, const Type& _rEnumType );

        // IPropertyEnumRepresentation implementqation
        virtual ::std::vector< OUString >
                                    SAL_CALL getDescriptions() const override;
        virtual void                SAL_CALL getValueFromDescription( const OUString& _rDescription, css::uno::Any& _out_rValue ) const override;
        virtual OUString            SAL_CALL getDescriptionForValue( const css::uno::Any& _rEnumValue ) const override;

    private:
        void            impl_getValues( Sequence< sal_Int32 >& _out_rValues ) const;
    };

    EnumRepresentation::EnumRepresentation( const Reference< XComponentContext >& _rxContext, const Type& _rEnumType )
        :m_aEnumType( _rEnumType )
    {
        try
        {
            if ( _rxContext.is() )
            {
                Reference< XHierarchicalNameAccess > xTypeDescProv(
                    _rxContext->getValueByName("/singletons/com.sun.star.reflection.theTypeDescriptionManager"),
                    UNO_QUERY_THROW );

                m_xTypeDescription.set( xTypeDescProv->getByHierarchicalName( m_aEnumType.getTypeName() ), UNO_QUERY_THROW );
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "EnumRepresentation::EnumRepresentation: caught an exception!" );
        }
    }

    ::std::vector< OUString > EnumRepresentation::getDescriptions() const
    {
        Sequence< OUString > aNames;
        try
        {
            if ( m_xTypeDescription.is() )
                aNames = m_xTypeDescription->getEnumNames();
        }
        catch( const Exception& )
        {
            OSL_FAIL( "EnumRepresentation::getDescriptions: caught an exception!" );
        }

        return ::std::vector< OUString >( aNames.getConstArray(), aNames.getConstArray() + aNames.getLength() );
    }

    void EnumRepresentation::impl_getValues( Sequence< sal_Int32 >& _out_rValues ) const
    {
        _out_rValues.realloc( 0 );
        try
        {
            if  ( m_xTypeDescription.is() )
                _out_rValues = m_xTypeDescription->getEnumValues();
        }
        catch( const Exception& )
        {
            OSL_FAIL( "EnumRepresentation::impl_getValues: caught an exception!" );
        }
    }

    void EnumRepresentation::getValueFromDescription( const OUString& _rDescription, Any& _out_rValue ) const
    {
        ::std::vector< OUString > aDescriptions( getDescriptions() );

        sal_Int32 index = ::std::find( aDescriptions.begin(), aDescriptions.end(),
            _rDescription ) - aDescriptions.begin();

        Sequence< sal_Int32 > aValues;
        impl_getValues( aValues );

        if ( ( index >= 0 ) && ( index < aValues.getLength() ) )
            _out_rValue = ::cppu::int2enum( aValues[ index ], m_aEnumType );
        else
        {
            OSL_FAIL( "EnumRepresentation::getValueFromDescription: cannot convert!" );
            _out_rValue.clear();
        }
    }

    OUString EnumRepresentation::getDescriptionForValue( const Any& _rEnumValue ) const
    {
        OUString sDescription;

        sal_Int32 nAsInt = 0;
        OSL_VERIFY( ::cppu::enum2int( nAsInt, _rEnumValue ) );

        Sequence< sal_Int32 > aValues;
        impl_getValues( aValues );

        sal_Int32 index = ::std::find( aValues.getConstArray(), aValues.getConstArray() + aValues.getLength(),
            nAsInt ) - aValues.getConstArray();

        ::std::vector< OUString > aDescriptions( getDescriptions() );
        if ( ( index >= 0 ) && ( index < (sal_Int32)aDescriptions.size() ) )
            sDescription = aDescriptions[ index ];
        else
        {
             OSL_FAIL( "EnumRepresentation::getDescriptionForValue: cannot convert!" );
        }
        return sDescription;
    }

    typedef ::cppu::WeakImplHelper <   XActionListener
                                    >   UrlClickHandler_Base;
    class UrlClickHandler : public UrlClickHandler_Base
    {
        Reference<XComponentContext>    m_xContext;
    public:
        UrlClickHandler( const Reference<XComponentContext>& _rContext, const Reference< XHyperlinkControl >& _rxControl );

    protected:
        virtual ~UrlClickHandler();

        // XActionListener
        virtual void SAL_CALL actionPerformed( const ActionEvent& rEvent ) throw (RuntimeException, std::exception) override;

        // XEventListener
        virtual void SAL_CALL disposing( const EventObject& Source ) throw (RuntimeException, std::exception) override;

    protected:
        void impl_dispatch_throw( const OUString& _rURL );
    };


    UrlClickHandler::UrlClickHandler( const Reference<XComponentContext>& _rContext, const Reference< XHyperlinkControl >& _rxControl )
        :m_xContext( _rContext )
    {
        if ( !_rxControl.is() )
            throw NullPointerException();

        osl_atomic_increment( &m_refCount );
        {
            _rxControl->addActionListener( this );
        }
        osl_atomic_decrement( &m_refCount );
        OSL_ENSURE( m_refCount > 0, "UrlClickHandler::UrlClickHandler: leaking!" );

    }

    UrlClickHandler::~UrlClickHandler()
    {
    }

    void SAL_CALL UrlClickHandler::actionPerformed( const ActionEvent& rEvent ) throw (RuntimeException, std::exception)
    {
        Reference< XPropertyControl > xControl( rEvent.Source, UNO_QUERY_THROW );
        Any aControlValue( xControl->getValue() );

        OUString sURL;
        if ( aControlValue.hasValue() && !( aControlValue >>= sURL ) )
            throw RuntimeException( OUString(), *this );

        if ( sURL.isEmpty() )
            return;

        impl_dispatch_throw( sURL );
    }

    void SAL_CALL UrlClickHandler::disposing( const EventObject& /*Source*/ ) throw (RuntimeException, std::exception)
    {
        // not interested in
    }

    void UrlClickHandler::impl_dispatch_throw( const OUString& _rURL )
    {
        Reference< XURLTransformer > xTransformer( URLTransformer::create(m_xContext) );
        URL aURL; aURL.Complete = ".uno:OpenHyperlink";
        xTransformer->parseStrict( aURL );

        Reference< XDesktop2 > xDispProv = Desktop::create( m_xContext );
        Reference< XDispatch > xDispatch( xDispProv->queryDispatch( aURL, OUString(), 0 ), UNO_QUERY_THROW );

        Sequence< PropertyValue > aDispatchArgs(1);
        aDispatchArgs[0].Name   = "URL";
        aDispatchArgs[0].Value  <<= _rURL;

        xDispatch->dispatch( aURL, aDispatchArgs );
    }


    GenericPropertyHandler::GenericPropertyHandler( const Reference< XComponentContext >& _rxContext )
        :GenericPropertyHandler_Base( m_aMutex )
        ,m_xContext( _rxContext )
        ,m_aPropertyListeners( m_aMutex )
        ,m_bPropertyMapInitialized( false )
    {
        m_xTypeConverter = Converter::create(_rxContext);
    }

    GenericPropertyHandler::~GenericPropertyHandler()
    {
    }

    OUString SAL_CALL GenericPropertyHandler::getImplementationName(  ) throw (RuntimeException, std::exception)
    {
        return getImplementationName_static();
    }

    sal_Bool SAL_CALL GenericPropertyHandler::supportsService( const OUString& ServiceName ) throw (RuntimeException, std::exception)
    {
        return cppu::supportsService(this, ServiceName);
    }

    Sequence< OUString > SAL_CALL GenericPropertyHandler::getSupportedServiceNames(  ) throw (RuntimeException, std::exception)
    {
        return getSupportedServiceNames_static();
    }

    OUString SAL_CALL GenericPropertyHandler::getImplementationName_static(  ) throw (RuntimeException)
    {
        return OUString( "com.sun.star.comp.extensions.GenericPropertyHandler" );
    }

    Sequence< OUString > SAL_CALL GenericPropertyHandler::getSupportedServiceNames_static(  ) throw (RuntimeException)
    {
        Sequence<OUString> aSupported { "com.sun.star.inspection.GenericPropertyHandler" };
        return aSupported;
    }

    Reference< XInterface > SAL_CALL GenericPropertyHandler::Create( const Reference< XComponentContext >& _rxContext )
    {
        return *( new GenericPropertyHandler( _rxContext ) );
    }

    void SAL_CALL GenericPropertyHandler::inspect( const Reference< XInterface >& _rxIntrospectee ) throw (RuntimeException, NullPointerException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( !_rxIntrospectee.is() )
            throw NullPointerException();

        // revoke old property change listeners
        ::cppu::OInterfaceIteratorHelper iterRemove( m_aPropertyListeners );
        ::cppu::OInterfaceIteratorHelper iterReAdd( m_aPropertyListeners ); // this holds a copy of the container ...
        while ( iterRemove.hasMoreElements() )
            m_xComponent->removePropertyChangeListener( OUString(), static_cast< XPropertyChangeListener* >( iterRemove.next() ) );

        m_xComponentIntrospectionAccess.clear();
        m_xComponent.clear();
        m_xPropertyState.clear();

        // create an introspection adapter for the component
        Reference< XIntrospection > xIntrospection = theIntrospection::get( m_xContext );

        Reference< XIntrospectionAccess > xIntrospectionAccess( xIntrospection->inspect( makeAny( _rxIntrospectee ) ) );
        if ( !xIntrospectionAccess.is() )
            throw RuntimeException("The introspection service could not handle the given component.", *this );

        m_xComponent.set( xIntrospectionAccess->queryAdapter( cppu::UnoType<XPropertySet>::get() ), UNO_QUERY_THROW );
        // now that we survived so far, remember m_xComponentIntrospectionAccess
        m_xComponentIntrospectionAccess = xIntrospectionAccess;
        m_xPropertyState.set(m_xComponent, css::uno::UNO_QUERY);

        m_bPropertyMapInitialized = false;
        m_aProperties.clear();

        // re-add the property change listeners
        while ( iterReAdd.hasMoreElements() )
            m_xComponent->addPropertyChangeListener( OUString(), static_cast< XPropertyChangeListener* >( iterReAdd.next() ) );
    }

    Any SAL_CALL GenericPropertyHandler::getPropertyValue( const OUString& _rPropertyName ) throw (UnknownPropertyException, RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !m_xComponent.is() )
            throw UnknownPropertyException();

        return m_xComponent->getPropertyValue( _rPropertyName );
    }

    void SAL_CALL GenericPropertyHandler::setPropertyValue( const OUString& _rPropertyName, const Any& _rValue ) throw (UnknownPropertyException, RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !m_xComponent.is() )
            throw UnknownPropertyException();

        m_xComponent->setPropertyValue( _rPropertyName, _rValue );
    }

    ::rtl::Reference< IPropertyEnumRepresentation > GenericPropertyHandler::impl_getEnumConverter( const Type& _rEnumType )
    {
        ::rtl::Reference< IPropertyEnumRepresentation >& rConverter = m_aEnumConverters[ _rEnumType ];
        if ( !rConverter.is() )
            rConverter = new EnumRepresentation( m_xContext, _rEnumType );
        return rConverter;
    }

    Any SAL_CALL GenericPropertyHandler::convertToPropertyValue( const OUString& _rPropertyName, const Any& _rControlValue ) throw (UnknownPropertyException, RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        impl_ensurePropertyMap();

        PropertyMap::const_iterator pos = m_aProperties.find( _rPropertyName );
        if ( pos == m_aProperties.end() )
            throw UnknownPropertyException();

        Any aPropertyValue;
        if ( !_rControlValue.hasValue() )
            // NULL is converted to NULL
            return aPropertyValue;

        if ( pos->second.Type.getTypeClass() == TypeClass_ENUM )
        {
            OUString sControlValue;
            OSL_VERIFY( _rControlValue >>= sControlValue );
            impl_getEnumConverter( pos->second.Type )->getValueFromDescription( sControlValue, aPropertyValue );
        }
        else
            aPropertyValue = PropertyHandlerHelper::convertToPropertyValue( m_xContext, m_xTypeConverter, pos->second, _rControlValue );

        return aPropertyValue;
    }

    Any SAL_CALL GenericPropertyHandler::convertToControlValue( const OUString& _rPropertyName, const Any& _rPropertyValue, const Type& _rControlValueType ) throw (UnknownPropertyException, RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        impl_ensurePropertyMap();

        PropertyMap::const_iterator pos = m_aProperties.find( _rPropertyName );
        if ( pos == m_aProperties.end() )
            throw UnknownPropertyException();

        Any aControlValue;
        if ( !_rPropertyValue.hasValue() )
            // NULL is converted to NULL
            return aControlValue;

        if ( pos->second.Type.getTypeClass() == TypeClass_ENUM )
        {
            aControlValue <<= impl_getEnumConverter( pos->second.Type )->getDescriptionForValue( _rPropertyValue );
        }
        else
            aControlValue = PropertyHandlerHelper::convertToControlValue( m_xContext, m_xTypeConverter, _rPropertyValue, _rControlValueType );
        return aControlValue;
    }

    PropertyState SAL_CALL GenericPropertyHandler::getPropertyState( const OUString& _rPropertyName ) throw (UnknownPropertyException, RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyState eState = PropertyState_DIRECT_VALUE;
        if ( m_xPropertyState.is() )
            eState = m_xPropertyState->getPropertyState( _rPropertyName );
        return eState;
    }

    void SAL_CALL GenericPropertyHandler::addPropertyChangeListener(const Reference< XPropertyChangeListener >& _rxListener)
        throw (NullPointerException, RuntimeException, std::exception)
    {
        if ( !_rxListener.is() )
            throw NullPointerException();

        ::osl::MutexGuard aGuard( m_aMutex );
        m_aPropertyListeners.addInterface( _rxListener );
        if ( m_xComponent.is() )
        {
            try
            {
                m_xComponent->addPropertyChangeListener( OUString(), _rxListener );
            }
            catch( const UnknownPropertyException& )
            {
                OSL_FAIL( "GenericPropertyHandler::addPropertyChangeListener:\nThe inspected component does not allow registering for all properties at once! This violates the interface contract!" );
            }
        }
    }

    void SAL_CALL GenericPropertyHandler::removePropertyChangeListener( const Reference< XPropertyChangeListener >& _rxListener ) throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( m_xComponent.is() )
        {
            try
            {
                m_xComponent->removePropertyChangeListener( OUString(), _rxListener );
            }
            catch( const UnknownPropertyException& )
            {
                OSL_FAIL( "GenericPropertyHandler::removePropertyChangeListener:\nThe inspected component does not allow de-registering for all properties at once! This violates the interface contract!" );
            }
        }
        m_aPropertyListeners.removeInterface( _rxListener );
    }

    void GenericPropertyHandler::impl_ensurePropertyMap()
    {
        if ( !m_bPropertyMapInitialized )
        {
            m_bPropertyMapInitialized = true;
            try
            {
                Reference< XPropertySetInfo > xPSI;
                if ( m_xComponent.is() )
                    xPSI = m_xComponent->getPropertySetInfo();
                Sequence< Property > aProperties;
                if ( xPSI.is() )
                    aProperties = xPSI->getProperties();
                DBG_ASSERT( aProperties.getLength(), "GenericPropertyHandler::getSupportedProperties: no properties!" );

                for ( const Property* pProperties = aProperties.getConstArray();
                      pProperties != aProperties.getConstArray() + aProperties.getLength();
                      ++pProperties
                    )
                {
                    switch ( pProperties->Type.getTypeClass() )
                    {
                    case TypeClass_BOOLEAN:
                    case TypeClass_BYTE:
                    case TypeClass_SHORT:
                    case TypeClass_UNSIGNED_SHORT:
                    case TypeClass_LONG:
                    case TypeClass_UNSIGNED_LONG:
                    case TypeClass_HYPER:
                    case TypeClass_UNSIGNED_HYPER:
                    case TypeClass_FLOAT:
                    case TypeClass_DOUBLE:
                    case TypeClass_ENUM:
                    case TypeClass_STRING:
                        // allowed, we can handle this type
                        break;

                    case TypeClass_SEQUENCE:
                    {
                        TypeClass eElementTypeClass = ::comphelper::getSequenceElementType( pProperties->Type ).getTypeClass();
                        if  (   ( eElementTypeClass != TypeClass_STRING )
                            &&  ( eElementTypeClass != TypeClass_BYTE )
                            &&  ( eElementTypeClass != TypeClass_SHORT )
                            &&  ( eElementTypeClass != TypeClass_UNSIGNED_SHORT )
                            &&  ( eElementTypeClass != TypeClass_LONG )
                            &&  ( eElementTypeClass != TypeClass_UNSIGNED_LONG )
                            )
                            // can only handle the above
                            continue;
                    }
                    break;

                    default:
                        // next property, we don't support this type
                        continue;
                    }

                    m_aProperties.insert( PropertyMap::value_type( pProperties->Name, *pProperties ) );
                }
            }
            catch( const Exception& )
            {
                OSL_FAIL( "GenericPropertyHandler::impl_ensurePropertyMap: caught an exception!" );
            }
        }
    }

    Sequence< Property > SAL_CALL GenericPropertyHandler::getSupportedProperties() throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        impl_ensurePropertyMap();

        return comphelper::mapValuesToSequence( m_aProperties );
    }

    Sequence< OUString > SAL_CALL GenericPropertyHandler::getSupersededProperties( ) throw (RuntimeException, std::exception)
    {
        // no superseded properties at all. This handler offers the very basic PropertyHandler
        // functionality, so it's much more likely that other handlers want to supersede
        // *our* properties ....
        return Sequence< OUString >( );
    }

    Sequence< OUString > SAL_CALL GenericPropertyHandler::getActuatingProperties( ) throw (RuntimeException, std::exception)
    {
        // This basic PropertyHandler implementation is too dumb^Wgeneric to know
        // anything about property dependencies
        return Sequence< OUString >( );
    }

    LineDescriptor SAL_CALL GenericPropertyHandler::describePropertyLine( const OUString& _rPropertyName,
        const Reference< XPropertyControlFactory >& _rxControlFactory )
        throw (UnknownPropertyException, NullPointerException, RuntimeException, std::exception)
    {
        if ( !_rxControlFactory.is() )
            throw NullPointerException();

        ::osl::MutexGuard aGuard( m_aMutex );
        impl_ensurePropertyMap();

        PropertyMap::const_iterator pos = m_aProperties.find( _rPropertyName );
        if ( pos == m_aProperties.end() )
            throw UnknownPropertyException();

        LineDescriptor aDescriptor;
        aDescriptor.DisplayName = _rPropertyName;
        switch ( pos->second.Type.getTypeClass() )
        {
        case TypeClass_ENUM:
            aDescriptor.Control = PropertyHandlerHelper::createListBoxControl( _rxControlFactory,
                impl_getEnumConverter( pos->second.Type )->getDescriptions(),
                PropertyHandlerHelper::requiresReadOnlyControl( pos->second.Attributes ),
                false );
            break;
        case TypeClass_STRING:
        {
            // some special handling for URL properties
            bool bIsURLProperty = _rPropertyName.endsWith( "URL" );
            if ( bIsURLProperty )
            {
                aDescriptor.Control = _rxControlFactory->createPropertyControl(
                    PropertyControlType::HyperlinkField, PropertyHandlerHelper::requiresReadOnlyControl( pos->second.Attributes ) );

                Reference< XHyperlinkControl > xControl( aDescriptor.Control, UNO_QUERY_THROW );
                Reference< XActionListener > xEnsureDelete( new UrlClickHandler( m_xContext, xControl ) );
            }
        }
        break;
        default:
            break;
        }
        // fallback
        if ( !aDescriptor.Control.is() )
            PropertyHandlerHelper::describePropertyLine( pos->second, aDescriptor, _rxControlFactory );

        aDescriptor.Category = "General";
        return aDescriptor;
    }

    sal_Bool SAL_CALL GenericPropertyHandler::isComposable( const OUString& /*_rPropertyName*/ ) throw (UnknownPropertyException, RuntimeException, std::exception)
    {
        return sal_False;
    }

    InteractiveSelectionResult SAL_CALL GenericPropertyHandler::onInteractivePropertySelection( const OUString& /*_rPropertyName*/, sal_Bool /*_bPrimary*/, Any& /*_rData*/, const Reference< XObjectInspectorUI >& /*_rxInspectorUI*/ ) throw (UnknownPropertyException, NullPointerException, RuntimeException, std::exception)
    {
        OSL_FAIL( "GenericPropertyHandler::onInteractivePropertySelection: I'm too dumb to know anything about property browse buttons!" );
        return InteractiveSelectionResult_Cancelled;
    }

    void SAL_CALL GenericPropertyHandler::actuatingPropertyChanged( const OUString& /*_rActuatingPropertyName*/, const Any& /*_rNewValue*/, const Any& /*_rOldValue*/, const Reference< XObjectInspectorUI >& /*_rxInspectorUI*/, sal_Bool /*_bFirstTimeInit*/ ) throw (NullPointerException, RuntimeException, std::exception)
    {
        OSL_FAIL( "GenericPropertyHandler::actuatingPropertyChanged: no no no, I did not register for any actuating properties!" );
    }

    sal_Bool SAL_CALL GenericPropertyHandler::suspend( sal_Bool /*_bSuspend*/ ) throw (RuntimeException, std::exception)
    {
        return sal_True;
    }

    void SAL_CALL GenericPropertyHandler::disposing()
    {
        m_aPropertyListeners.clear();
            // not disposeAndClear: the listeners are (virtually) listeners at our introspectee, not
            // at this handler instance
    }

    IMPLEMENT_FORWARD_XCOMPONENT( GenericPropertyHandler, GenericPropertyHandler_Base );

}   // namespace pcr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
