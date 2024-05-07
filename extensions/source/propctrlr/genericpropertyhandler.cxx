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

#include "enumrepresentation.hxx"
#include "genericpropertyhandler.hxx"
#include "handlerhelper.hxx"

#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/lang/NullPointerException.hpp>
#include <com/sun/star/reflection/XEnumTypeDescription.hpp>
#include <com/sun/star/beans/theIntrospection.hpp>
#include <com/sun/star/inspection/PropertyControlType.hpp>
#include <com/sun/star/inspection/XHyperlinkControl.hpp>
#include <com/sun/star/awt/XActionListener.hpp>
#include <com/sun/star/script/Converter.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/Desktop.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>
#include <o3tl/safeint.hxx>
#include <tools/debug.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <algorithm>

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

    namespace {

    class EnumRepresentation : public IPropertyEnumRepresentation
    {
    private:
        Reference< XEnumTypeDescription >   m_xTypeDescription;
        Type                                m_aEnumType;

    public:
        EnumRepresentation( const Reference< XComponentContext >& _rxContext, const Type& _rEnumType );
        EnumRepresentation(const EnumRepresentation&) = delete;
        EnumRepresentation& operator=(const EnumRepresentation&) = delete;

        // IPropertyEnumRepresentation implementqation
        virtual std::vector< OUString >
                                    getDescriptions() const override;
        virtual void                getValueFromDescription( const OUString& _rDescription, css::uno::Any& _out_rValue ) const override;
        virtual OUString            getDescriptionForValue( const css::uno::Any& _rEnumValue ) const override;

    private:
        void            impl_getValues( Sequence< sal_Int32 >& _out_rValues ) const;
    };

    }

    EnumRepresentation::EnumRepresentation( const Reference< XComponentContext >& _rxContext, const Type& _rEnumType )
        :m_aEnumType( _rEnumType )
    {
        try
        {
            if ( _rxContext.is() )
            {
                Reference< XHierarchicalNameAccess > xTypeDescProv(
                    _rxContext->getValueByName(u"/singletons/com.sun.star.reflection.theTypeDescriptionManager"_ustr),
                    UNO_QUERY_THROW );

                m_xTypeDescription.set( xTypeDescProv->getByHierarchicalName( m_aEnumType.getTypeName() ), UNO_QUERY_THROW );
            }
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "extensions.propctrlr", "EnumRepresentation::EnumRepresentation" );
        }
    }

    std::vector< OUString > EnumRepresentation::getDescriptions() const
    {
        Sequence< OUString > aNames;
        try
        {
            if ( m_xTypeDescription.is() )
                aNames = m_xTypeDescription->getEnumNames();
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "extensions.propctrlr", "EnumRepresentation::getDescriptions" );
        }

        return std::vector< OUString >( std::cbegin(aNames), std::cend(aNames) );
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
            TOOLS_WARN_EXCEPTION( "extensions.propctrlr", "EnumRepresentation::impl_getValues" );
        }
    }

    void EnumRepresentation::getValueFromDescription( const OUString& _rDescription, Any& _out_rValue ) const
    {
        std::vector< OUString > aDescriptions( getDescriptions() );

        sal_Int32 index = std::find( aDescriptions.begin(), aDescriptions.end(),
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

        sal_Int32 index = std::find( std::cbegin(aValues), std::cend(aValues), nAsInt ) - std::cbegin(aValues);

        std::vector< OUString > aDescriptions( getDescriptions() );
        if ( ( index >= 0 ) && ( o3tl::make_unsigned(index) < aDescriptions.size() ) )
            sDescription = aDescriptions[ index ];
        else
        {
             OSL_FAIL( "EnumRepresentation::getDescriptionForValue: cannot convert!" );
        }
        return sDescription;
    }

    typedef ::cppu::WeakImplHelper <   XActionListener
                                    >   UrlClickHandler_Base;

    namespace {

    class UrlClickHandler : public UrlClickHandler_Base
    {
        Reference<XComponentContext>    m_xContext;
    public:
        UrlClickHandler( const Reference<XComponentContext>& _rContext, const Reference< XHyperlinkControl >& _rxControl );

    protected:
        virtual ~UrlClickHandler() override;

        // XActionListener
        virtual void SAL_CALL actionPerformed( const ActionEvent& rEvent ) override;

        // XEventListener
        virtual void SAL_CALL disposing( const EventObject& Source ) override;

    protected:
        void impl_dispatch_throw( const OUString& _rURL );
    };

    }

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

    void SAL_CALL UrlClickHandler::actionPerformed( const ActionEvent& rEvent )
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

    void SAL_CALL UrlClickHandler::disposing( const EventObject& /*Source*/ )
    {
        // not interested in
    }

    void UrlClickHandler::impl_dispatch_throw( const OUString& _rURL )
    {
        Reference< XURLTransformer > xTransformer( URLTransformer::create(m_xContext) );
        URL aURL; aURL.Complete = ".uno:OpenHyperlink";
        xTransformer->parseStrict( aURL );

        Reference< XDesktop2 > xDispProv = Desktop::create( m_xContext );
        Reference< XDispatch > xDispatch( xDispProv->queryDispatch( aURL, OUString(), 0 ), UNO_SET_THROW );

        Sequence aDispatchArgs{ comphelper::makePropertyValue(u"URL"_ustr, _rURL) };

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

    OUString SAL_CALL GenericPropertyHandler::getImplementationName(  )
    {
        return u"com.sun.star.comp.extensions.GenericPropertyHandler"_ustr;
    }

    sal_Bool SAL_CALL GenericPropertyHandler::supportsService( const OUString& ServiceName )
    {
        return cppu::supportsService(this, ServiceName);
    }

    Sequence< OUString > SAL_CALL GenericPropertyHandler::getSupportedServiceNames(  )
    {
        return { u"com.sun.star.inspection.GenericPropertyHandler"_ustr };
    }

    void SAL_CALL GenericPropertyHandler::inspect( const Reference< XInterface >& _rxIntrospectee )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( !_rxIntrospectee.is() )
            throw NullPointerException();

        // revoke old property change listeners
        ::comphelper::OInterfaceIteratorHelper2 iterRemove( m_aPropertyListeners );
        ::comphelper::OInterfaceIteratorHelper2 iterReAdd( m_aPropertyListeners ); // this holds a copy of the container ...
        while ( iterRemove.hasMoreElements() )
            m_xComponent->removePropertyChangeListener( OUString(), static_cast< XPropertyChangeListener* >( iterRemove.next() ) );

        m_xComponentIntrospectionAccess.clear();
        m_xComponent.clear();
        m_xPropertyState.clear();

        // create an introspection adapter for the component
        Reference< XIntrospection > xIntrospection = theIntrospection::get( m_xContext );

        Reference< XIntrospectionAccess > xIntrospectionAccess( xIntrospection->inspect( Any( _rxIntrospectee ) ) );
        if ( !xIntrospectionAccess.is() )
            throw RuntimeException(u"The introspection service could not handle the given component."_ustr, *this );

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

    Any SAL_CALL GenericPropertyHandler::getPropertyValue( const OUString& _rPropertyName )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !m_xComponent.is() )
            throw UnknownPropertyException(_rPropertyName);

        return m_xComponent->getPropertyValue( _rPropertyName );
    }

    void SAL_CALL GenericPropertyHandler::setPropertyValue( const OUString& _rPropertyName, const Any& _rValue )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !m_xComponent.is() )
            throw UnknownPropertyException(_rPropertyName);

        m_xComponent->setPropertyValue( _rPropertyName, _rValue );
    }

    ::rtl::Reference< IPropertyEnumRepresentation > GenericPropertyHandler::impl_getEnumConverter( const Type& _rEnumType )
    {
        ::rtl::Reference< IPropertyEnumRepresentation >& rConverter = m_aEnumConverters[ _rEnumType ];
        if ( !rConverter.is() )
            rConverter = new EnumRepresentation( m_xContext, _rEnumType );
        return rConverter;
    }

    Any SAL_CALL GenericPropertyHandler::convertToPropertyValue( const OUString& _rPropertyName, const Any& _rControlValue )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        impl_ensurePropertyMap();

        PropertyMap::const_iterator pos = m_aProperties.find( _rPropertyName );
        if ( pos == m_aProperties.end() )
            throw UnknownPropertyException(_rPropertyName);

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

    Any SAL_CALL GenericPropertyHandler::convertToControlValue( const OUString& _rPropertyName, const Any& _rPropertyValue, const Type& _rControlValueType )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        impl_ensurePropertyMap();

        PropertyMap::const_iterator pos = m_aProperties.find( _rPropertyName );
        if ( pos == m_aProperties.end() )
            throw UnknownPropertyException(_rPropertyName);

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

    PropertyState SAL_CALL GenericPropertyHandler::getPropertyState( const OUString& _rPropertyName )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyState eState = PropertyState_DIRECT_VALUE;
        if ( m_xPropertyState.is() )
            eState = m_xPropertyState->getPropertyState( _rPropertyName );
        return eState;
    }

    void SAL_CALL GenericPropertyHandler::addPropertyChangeListener(const Reference< XPropertyChangeListener >& _rxListener)
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

    void SAL_CALL GenericPropertyHandler::removePropertyChangeListener( const Reference< XPropertyChangeListener >& _rxListener )
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
        if ( m_bPropertyMapInitialized )
            return;

        m_bPropertyMapInitialized = true;
        try
        {
            Reference< XPropertySetInfo > xPSI;
            if ( m_xComponent.is() )
                xPSI = m_xComponent->getPropertySetInfo();
            Sequence< Property > aProperties;
            if ( xPSI.is() )
                aProperties = xPSI->getProperties();
            DBG_ASSERT( aProperties.hasElements(), "GenericPropertyHandler::getSupportedProperties: no properties!" );

            for (auto const& property : aProperties)
            {
                switch ( property.Type.getTypeClass() )
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
                    TypeClass eElementTypeClass = ::comphelper::getSequenceElementType( property.Type ).getTypeClass();
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

                m_aProperties.emplace( property.Name, property );
            }
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "extensions.propctrlr", "GenericPropertyHandler::impl_ensurePropertyMap" );
        }
    }

    Sequence< Property > SAL_CALL GenericPropertyHandler::getSupportedProperties()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        impl_ensurePropertyMap();

        return comphelper::mapValuesToSequence( m_aProperties );
    }

    Sequence< OUString > SAL_CALL GenericPropertyHandler::getSupersededProperties( )
    {
        // no superseded properties at all. This handler offers the very basic PropertyHandler
        // functionality, so it's much more likely that other handlers want to supersede
        // *our* properties...
        return Sequence< OUString >( );
    }

    Sequence< OUString > SAL_CALL GenericPropertyHandler::getActuatingProperties( )
    {
        // This basic PropertyHandler implementation is too dumb^Wgeneric to know
        // anything about property dependencies
        return Sequence< OUString >( );
    }

    LineDescriptor SAL_CALL GenericPropertyHandler::describePropertyLine( const OUString& _rPropertyName,
        const Reference< XPropertyControlFactory >& _rxControlFactory )
    {
        if ( !_rxControlFactory.is() )
            throw NullPointerException();

        ::osl::MutexGuard aGuard( m_aMutex );
        impl_ensurePropertyMap();

        PropertyMap::const_iterator pos = m_aProperties.find( _rPropertyName );
        if ( pos == m_aProperties.end() )
            throw UnknownPropertyException(_rPropertyName);

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
                new UrlClickHandler( m_xContext, xControl );
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

    sal_Bool SAL_CALL GenericPropertyHandler::isComposable( const OUString& /*_rPropertyName*/ )
    {
        return false;
    }

    InteractiveSelectionResult SAL_CALL GenericPropertyHandler::onInteractivePropertySelection( const OUString& /*_rPropertyName*/, sal_Bool /*_bPrimary*/, Any& /*_rData*/, const Reference< XObjectInspectorUI >& /*_rxInspectorUI*/ )
    {
        OSL_FAIL( "GenericPropertyHandler::onInteractivePropertySelection: I'm too dumb to know anything about property browse buttons!" );
        return InteractiveSelectionResult_Cancelled;
    }

    void SAL_CALL GenericPropertyHandler::actuatingPropertyChanged( const OUString& /*_rActuatingPropertyName*/, const Any& /*_rNewValue*/, const Any& /*_rOldValue*/, const Reference< XObjectInspectorUI >& /*_rxInspectorUI*/, sal_Bool /*_bFirstTimeInit*/ )
    {
        OSL_FAIL( "GenericPropertyHandler::actuatingPropertyChanged: no no no, I did not register for any actuating properties!" );
    }

    sal_Bool SAL_CALL GenericPropertyHandler::suspend( sal_Bool /*_bSuspend*/ )
    {
        return true;
    }

    void SAL_CALL GenericPropertyHandler::disposing()
    {
        m_aPropertyListeners.clear();
            // not disposeAndClear: the listeners are (virtually) listeners at our introspectee, not
            // at this handler instance
    }

    void SAL_CALL GenericPropertyHandler::dispose(  )
    {
        GenericPropertyHandler_Base::WeakComponentImplHelperBase::dispose();
        m_xComponentIntrospectionAccess.clear();
        m_xComponent.clear();
        m_xTypeConverter.clear();
        m_xPropertyState.clear();
    }
    void SAL_CALL GenericPropertyHandler::addEventListener( const css::uno::Reference< css::lang::XEventListener >& Listener )
    {
        GenericPropertyHandler_Base::WeakComponentImplHelperBase::addEventListener( Listener );
    }
    void SAL_CALL GenericPropertyHandler::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& Listener )
    {
        GenericPropertyHandler_Base::WeakComponentImplHelperBase::removeEventListener( Listener );
    }

}   // namespace pcr

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
extensions_propctrlr_GenericPropertyHandler_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new pcr::GenericPropertyHandler(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
