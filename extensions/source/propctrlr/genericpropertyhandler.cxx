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

#include "genericpropertyhandler.hxx"
#include "formmetadata.hxx"
#include "handlerhelper.hxx"

#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/reflection/XEnumTypeDescription.hpp>
#include <com/sun/star/beans/Introspection.hpp>
#include <com/sun/star/inspection/PropertyControlType.hpp>
#include <com/sun/star/inspection/XHyperlinkControl.hpp>
#include <com/sun/star/awt/XActionListener.hpp>
#include <com/sun/star/script/Converter.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <tools/debug.hxx>
#include <comphelper/extract.hxx>

#include <algorithm>
#include <o3tl/compat_functional.hxx>

//------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_GenericPropertyHandler()
{
    ::pcr::OAutoRegistration< ::pcr::GenericPropertyHandler > aAutoRegistration;
}

//........................................................................
namespace pcr
{
//........................................................................

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

    //====================================================================
    //= EnumRepresentation
    //====================================================================
    class EnumRepresentation : public IPropertyEnumRepresentation
    {
    private:
        oslInterlockedCount                 m_refCount;
        Reference< XEnumTypeDescription >   m_xTypeDescription;
        Type                                m_aEnumType;

    public:
        EnumRepresentation( const Reference< XComponentContext >& _rxContext, const Type& _rEnumType );

        // IPropertyEnumRepresentation implementqation
        virtual ::std::vector< ::rtl::OUString >
                                    SAL_CALL getDescriptions() const;
        virtual void                SAL_CALL getValueFromDescription( const ::rtl::OUString& _rDescription, ::com::sun::star::uno::Any& _out_rValue ) const;
        virtual ::rtl::OUString     SAL_CALL getDescriptionForValue( const ::com::sun::star::uno::Any& _rEnumValue ) const;

        // IReference implementqation
        virtual oslInterlockedCount SAL_CALL acquire();
        virtual oslInterlockedCount SAL_CALL release();

    private:
        void            impl_getValues( Sequence< sal_Int32 >& _out_rValues ) const;

    private:
        EnumRepresentation();                                       // never implemented
        EnumRepresentation( const EnumRepresentation& );            // never implemented
        EnumRepresentation& operator=( const EnumRepresentation& ); // never implemented
    };

    //--------------------------------------------------------------------
    EnumRepresentation::EnumRepresentation( const Reference< XComponentContext >& _rxContext, const Type& _rEnumType )
        :m_refCount( 0 )
        ,m_aEnumType( _rEnumType )
    {
        try
        {
            if ( _rxContext.is() )
            {
                Reference< XHierarchicalNameAccess > xTypeDescProv(
                    _rxContext->getValueByName( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/singletons/com.sun.star.reflection.theTypeDescriptionManager" ) ) ),
                    UNO_QUERY_THROW );

                m_xTypeDescription = Reference< XEnumTypeDescription >( xTypeDescProv->getByHierarchicalName( m_aEnumType.getTypeName() ), UNO_QUERY_THROW );
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "EnumRepresentation::EnumRepresentation: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    ::std::vector< ::rtl::OUString > EnumRepresentation::getDescriptions() const
    {
        Sequence< ::rtl::OUString > aNames;
        try
        {
            if ( m_xTypeDescription.is() )
                aNames = m_xTypeDescription->getEnumNames();
        }
        catch( const Exception& )
        {
            OSL_FAIL( "EnumRepresentation::getDescriptions: caught an exception!" );
        }

        return ::std::vector< ::rtl::OUString >( aNames.getConstArray(), aNames.getConstArray() + aNames.getLength() );
    }

    //--------------------------------------------------------------------
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

    //--------------------------------------------------------------------
    void EnumRepresentation::getValueFromDescription( const ::rtl::OUString& _rDescription, Any& _out_rValue ) const
    {
        ::std::vector< ::rtl::OUString > aDescriptions( getDescriptions() );

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

    //--------------------------------------------------------------------
    ::rtl::OUString EnumRepresentation::getDescriptionForValue( const Any& _rEnumValue ) const
    {
        ::rtl::OUString sDescription;

        sal_Int32 nAsInt = 0;
        OSL_VERIFY( ::cppu::enum2int( nAsInt, _rEnumValue ) );

        Sequence< sal_Int32 > aValues;
        impl_getValues( aValues );

        sal_Int32 index = ::std::find( aValues.getConstArray(), aValues.getConstArray() + aValues.getLength(),
            nAsInt ) - aValues.getConstArray();

        ::std::vector< ::rtl::OUString > aDescriptions( getDescriptions() );
        if ( ( index >= 0 ) && ( index < (sal_Int32)aDescriptions.size() ) )
            sDescription = aDescriptions[ index ];
        else
        {
             OSL_FAIL( "EnumRepresentation::getDescriptionForValue: cannot convert!" );
        }
        return sDescription;
    }

    //--------------------------------------------------------------------
    oslInterlockedCount SAL_CALL EnumRepresentation::acquire()
    {
        return osl_atomic_increment( &m_refCount );
    }

    //--------------------------------------------------------------------
    oslInterlockedCount SAL_CALL EnumRepresentation::release()
    {
        if ( 0 == osl_atomic_decrement( &m_refCount ) )
        {
           delete this;
           return 0;
        }
        return m_refCount;
    }

    //====================================================================
    //= UrlClickHandler
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   XActionListener
                                    >   UrlClickHandler_Base;
    class UrlClickHandler : public UrlClickHandler_Base
    {
        ComponentContext    m_aContext;
    public:
        UrlClickHandler( const ComponentContext& _rContext, const Reference< XHyperlinkControl >& _rxControl );

    protected:
        ~UrlClickHandler();

        // XActionListener
        virtual void SAL_CALL actionPerformed( const ActionEvent& rEvent ) throw (RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const EventObject& Source ) throw (RuntimeException);

    protected:
        void impl_dispatch_throw( const ::rtl::OUString& _rURL );
    };

    //--------------------------------------------------------------------
    DBG_NAME( UrlClickHandler )
    //--------------------------------------------------------------------
    UrlClickHandler::UrlClickHandler( const ComponentContext& _rContext, const Reference< XHyperlinkControl >& _rxControl )
        :m_aContext( _rContext )
    {
        if ( !_rxControl.is() )
            throw NullPointerException();

        osl_atomic_increment( &m_refCount );
        {
            _rxControl->addActionListener( this );
        }
        osl_atomic_decrement( &m_refCount );
        OSL_ENSURE( m_refCount > 0, "UrlClickHandler::UrlClickHandler: leaking!" );

        DBG_CTOR( UrlClickHandler, NULL );
    }

    //--------------------------------------------------------------------
    UrlClickHandler::~UrlClickHandler()
    {
        DBG_DTOR( UrlClickHandler, NULL );
    }

    //--------------------------------------------------------------------
    void SAL_CALL UrlClickHandler::actionPerformed( const ActionEvent& rEvent ) throw (RuntimeException)
    {
        Reference< XPropertyControl > xControl( rEvent.Source, UNO_QUERY_THROW );
        Any aControlValue( xControl->getValue() );

        ::rtl::OUString sURL;
        if ( aControlValue.hasValue() && !( aControlValue >>= sURL ) )
            throw RuntimeException( ::rtl::OUString(), *this );

        if ( sURL.isEmpty() )
            return;

        impl_dispatch_throw( sURL );
    }

    //--------------------------------------------------------------------
    void SAL_CALL UrlClickHandler::disposing( const EventObject& /*Source*/ ) throw (RuntimeException)
    {
        // not interested in
    }

    //--------------------------------------------------------------------
    void UrlClickHandler::impl_dispatch_throw( const ::rtl::OUString& _rURL )
    {
        Reference< XURLTransformer > xTransformer( URLTransformer::create(m_aContext.getUNOContext()) );
        URL aURL; aURL.Complete = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:OpenHyperlink" ) );
        xTransformer->parseStrict( aURL );

        Reference< XDispatchProvider > xDispProv( m_aContext.createComponent( "com.sun.star.frame.Desktop" ), UNO_QUERY_THROW );
        Reference< XDispatch > xDispatch( xDispProv->queryDispatch( aURL, ::rtl::OUString(), 0 ), UNO_QUERY_THROW );

        Sequence< PropertyValue > aDispatchArgs(1);
        aDispatchArgs[0].Name   = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("URL"));
        aDispatchArgs[0].Value  <<= _rURL;

        xDispatch->dispatch( aURL, aDispatchArgs );
    }

    //====================================================================
    //= GenericPropertyHandler
    //====================================================================
    DBG_NAME( GenericPropertyHandler )
    //--------------------------------------------------------------------
    GenericPropertyHandler::GenericPropertyHandler( const Reference< XComponentContext >& _rxContext )
        :GenericPropertyHandler_Base( m_aMutex )
        ,m_aContext( _rxContext )
        ,m_aPropertyListeners( m_aMutex )
        ,m_bPropertyMapInitialized( false )
    {
        DBG_CTOR( GenericPropertyHandler, NULL );

        m_xTypeConverter = Converter::create(_rxContext);
    }

    //--------------------------------------------------------------------
    GenericPropertyHandler::~GenericPropertyHandler()
    {
        DBG_DTOR( GenericPropertyHandler, NULL );
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL GenericPropertyHandler::getImplementationName(  ) throw (RuntimeException)
    {
        return getImplementationName_static();
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL GenericPropertyHandler::supportsService( const ::rtl::OUString& ServiceName ) throw (RuntimeException)
    {
        StlSyntaxSequence< ::rtl::OUString > aAllServices( getSupportedServiceNames() );
        return ::std::find( aAllServices.begin(), aAllServices.end(), ServiceName ) != aAllServices.end();
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL GenericPropertyHandler::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        return getSupportedServiceNames_static();
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL GenericPropertyHandler::getImplementationName_static(  ) throw (RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.extensions.GenericPropertyHandler" ) );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL GenericPropertyHandler::getSupportedServiceNames_static(  ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aSupported( 1 );
        aSupported[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.inspection.GenericPropertyHandler" ) );
        return aSupported;
    }

    //--------------------------------------------------------------------
    Reference< XInterface > SAL_CALL GenericPropertyHandler::Create( const Reference< XComponentContext >& _rxContext )
    {
        return *( new GenericPropertyHandler( _rxContext ) );
    }

    //--------------------------------------------------------------------
    void SAL_CALL GenericPropertyHandler::inspect( const Reference< XInterface >& _rxIntrospectee ) throw (RuntimeException, NullPointerException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( !_rxIntrospectee.is() )
            throw NullPointerException();

        // revoke old property change listeners
        ::cppu::OInterfaceIteratorHelper iterRemove( m_aPropertyListeners );
        ::cppu::OInterfaceIteratorHelper iterReAdd( m_aPropertyListeners ); // this holds a copy of the container ...
        while ( iterRemove.hasMoreElements() )
            m_xComponent->removePropertyChangeListener( ::rtl::OUString(), static_cast< XPropertyChangeListener* >( iterRemove.next() ) );

        m_xComponentIntrospectionAccess.clear();
        m_xComponent.clear();
        m_xPropertyState.clear();

        // create an introspection adapter for the component
        Reference< XIntrospection > xIntrospection = Introspection::create( m_aContext.getUNOContext() );

        Reference< XIntrospectionAccess > xIntrospectionAccess( xIntrospection->inspect( makeAny( _rxIntrospectee ) ) );
        if ( !xIntrospectionAccess.is() )
            throw RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "The introspection service could not handle the given component." ) ), *this );

        m_xComponent = Reference< XPropertySet >( xIntrospectionAccess->queryAdapter( XPropertySet::static_type() ), UNO_QUERY_THROW );
        // now that we survived so far, remember m_xComponentIntrospectionAccess
        m_xComponentIntrospectionAccess = xIntrospectionAccess;
        m_xPropertyState = m_xPropertyState.query( m_xComponent );

        m_bPropertyMapInitialized = false;
        m_aProperties.clear();

        // re-add the property change listeners
        while ( iterReAdd.hasMoreElements() )
            m_xComponent->addPropertyChangeListener( ::rtl::OUString(), static_cast< XPropertyChangeListener* >( iterReAdd.next() ) );
    }

    //--------------------------------------------------------------------
    Any SAL_CALL GenericPropertyHandler::getPropertyValue( const ::rtl::OUString& _rPropertyName ) throw (UnknownPropertyException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !m_xComponent.is() )
            throw UnknownPropertyException();

        return m_xComponent->getPropertyValue( _rPropertyName );
    }

    //--------------------------------------------------------------------
    void SAL_CALL GenericPropertyHandler::setPropertyValue( const ::rtl::OUString& _rPropertyName, const Any& _rValue ) throw (UnknownPropertyException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !m_xComponent.is() )
            throw UnknownPropertyException();

        m_xComponent->setPropertyValue( _rPropertyName, _rValue );
    }

    //--------------------------------------------------------------------
    ::rtl::Reference< IPropertyEnumRepresentation > GenericPropertyHandler::impl_getEnumConverter( const Type& _rEnumType )
    {
        ::rtl::Reference< IPropertyEnumRepresentation >& rConverter = m_aEnumConverters[ _rEnumType ];
        if ( !rConverter.is() )
            rConverter = new EnumRepresentation( m_aContext.getUNOContext(), _rEnumType );
        return rConverter;
    }

    //--------------------------------------------------------------------
    Any SAL_CALL GenericPropertyHandler::convertToPropertyValue( const ::rtl::OUString& _rPropertyName, const Any& _rControlValue ) throw (UnknownPropertyException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        const_cast< GenericPropertyHandler* >( this )->impl_ensurePropertyMap();

        PropertyMap::const_iterator pos = m_aProperties.find( _rPropertyName );
        if ( pos == m_aProperties.end() )
            throw UnknownPropertyException();

        Any aPropertyValue;
        if ( !_rControlValue.hasValue() )
            // NULL is converted to NULL
            return aPropertyValue;

        if ( pos->second.Type.getTypeClass() == TypeClass_ENUM )
        {
            ::rtl::OUString sControlValue;
            OSL_VERIFY( _rControlValue >>= sControlValue );
            impl_getEnumConverter( pos->second.Type )->getValueFromDescription( sControlValue, aPropertyValue );
        }
        else
            aPropertyValue = PropertyHandlerHelper::convertToPropertyValue( m_aContext.getContext(),m_xTypeConverter, pos->second, _rControlValue );

        return aPropertyValue;
    }

    //--------------------------------------------------------------------
    Any SAL_CALL GenericPropertyHandler::convertToControlValue( const ::rtl::OUString& _rPropertyName, const Any& _rPropertyValue, const Type& _rControlValueType ) throw (UnknownPropertyException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        const_cast< GenericPropertyHandler* >( this )->impl_ensurePropertyMap();

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
            aControlValue = PropertyHandlerHelper::convertToControlValue( m_aContext.getContext(),m_xTypeConverter, _rPropertyValue, _rControlValueType );
        return aControlValue;
    }

    //--------------------------------------------------------------------
    PropertyState SAL_CALL GenericPropertyHandler::getPropertyState( const ::rtl::OUString& _rPropertyName ) throw (UnknownPropertyException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyState eState = PropertyState_DIRECT_VALUE;
        if ( m_xPropertyState.is() )
            eState = m_xPropertyState->getPropertyState( _rPropertyName );
        return eState;
    }

    //--------------------------------------------------------------------
    void SAL_CALL GenericPropertyHandler::addPropertyChangeListener( const Reference< XPropertyChangeListener >& _rxListener ) throw (RuntimeException)
    {
        if ( !_rxListener.is() )
            throw NullPointerException();

        ::osl::MutexGuard aGuard( m_aMutex );
        m_aPropertyListeners.addInterface( _rxListener );
        if ( m_xComponent.is() )
        {
            try
            {
                m_xComponent->addPropertyChangeListener( ::rtl::OUString(), _rxListener );
            }
            catch( const UnknownPropertyException& )
            {
                OSL_FAIL( "GenericPropertyHandler::addPropertyChangeListener:\nThe inspected component does not allow registering for all properties at once! This violates the interface contract!" );
            }
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL GenericPropertyHandler::removePropertyChangeListener( const Reference< XPropertyChangeListener >& _rxListener ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( m_xComponent.is() )
        {
            try
            {
                m_xComponent->removePropertyChangeListener( ::rtl::OUString(), _rxListener );
            }
            catch( const UnknownPropertyException& )
            {
                OSL_FAIL( "GenericPropertyHandler::removePropertyChangeListener:\nThe inspected component does not allow de-registering for all properties at once! This violates the interface contract!" );
            }
        }
        m_aPropertyListeners.removeInterface( _rxListener );
    }

    //--------------------------------------------------------------------
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

    //--------------------------------------------------------------------
    Sequence< Property > SAL_CALL GenericPropertyHandler::getSupportedProperties() throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        const_cast< GenericPropertyHandler* >( this )->impl_ensurePropertyMap();

        Sequence< Property > aReturn( m_aProperties.size() );
        ::std::transform( m_aProperties.begin(), m_aProperties.end(),
            aReturn.getArray(), ::o3tl::select2nd< PropertyMap::value_type >() );
        return aReturn;
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL GenericPropertyHandler::getSupersededProperties( ) throw (RuntimeException)
    {
        // no superseded properties at all. This handler offers the very basic PropertyHandler
        // functionality, so it's much more likely that other handlers want to supersede
        // *our* properties ....
        return Sequence< ::rtl::OUString >( );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL GenericPropertyHandler::getActuatingProperties( ) throw (RuntimeException)
    {
        // This basic PropertyHandler implementation is too dumb^Wgeneric to know
        // anything about property dependencies
        return Sequence< ::rtl::OUString >( );
    }

    //--------------------------------------------------------------------
    LineDescriptor SAL_CALL GenericPropertyHandler::describePropertyLine( const ::rtl::OUString& _rPropertyName,
        const Reference< XPropertyControlFactory >& _rxControlFactory )
        throw (UnknownPropertyException, NullPointerException, RuntimeException)
    {
        if ( !_rxControlFactory.is() )
            throw NullPointerException();

        ::osl::MutexGuard aGuard( m_aMutex );
        const_cast< GenericPropertyHandler* >( this )->impl_ensurePropertyMap();

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
                sal_False );
            break;
        case TypeClass_STRING:
        {
            // some special handling for URL properties
            bool bIsURLProperty = ( _rPropertyName.getLength() >= 3 ) && _rPropertyName.matchAsciiL( "URL", 3, _rPropertyName.getLength() - 3 );
            if ( bIsURLProperty )
            {
                aDescriptor.Control = _rxControlFactory->createPropertyControl(
                    PropertyControlType::HyperlinkField, PropertyHandlerHelper::requiresReadOnlyControl( pos->second.Attributes ) );

                Reference< XHyperlinkControl > xControl( aDescriptor.Control, UNO_QUERY_THROW );
                Reference< XActionListener > xEnsureDelete( new UrlClickHandler( m_aContext, xControl ) );
            }
        }
        break;
        default:
            break;
        }
        // fallback
        if ( !aDescriptor.Control.is() )
            PropertyHandlerHelper::describePropertyLine( pos->second, aDescriptor, _rxControlFactory );

        aDescriptor.Category = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "General" ) );
        return aDescriptor;
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL GenericPropertyHandler::isComposable( const ::rtl::OUString& /*_rPropertyName*/ ) throw (UnknownPropertyException, RuntimeException)
    {
        return sal_False;
    }

    //--------------------------------------------------------------------
    InteractiveSelectionResult SAL_CALL GenericPropertyHandler::onInteractivePropertySelection( const ::rtl::OUString& /*_rPropertyName*/, sal_Bool /*_bPrimary*/, Any& /*_rData*/, const Reference< XObjectInspectorUI >& /*_rxInspectorUI*/ ) throw (UnknownPropertyException, NullPointerException, RuntimeException)
    {
        OSL_FAIL( "GenericPropertyHandler::onInteractivePropertySelection: I'm too dumb to know anything about property browse buttons!" );
        return InteractiveSelectionResult_Cancelled;
    }

    //--------------------------------------------------------------------
    void SAL_CALL GenericPropertyHandler::actuatingPropertyChanged( const ::rtl::OUString& /*_rActuatingPropertyName*/, const Any& /*_rNewValue*/, const Any& /*_rOldValue*/, const Reference< XObjectInspectorUI >& /*_rxInspectorUI*/, sal_Bool /*_bFirstTimeInit*/ ) throw (NullPointerException, RuntimeException)
    {
        OSL_FAIL( "GenericPropertyHandler::actuatingPropertyChanged: no no no, I did not register for any actuating properties!" );
    }

    //--------------------------------------------------------------------
    sal_Bool SAL_CALL GenericPropertyHandler::suspend( sal_Bool /*_bSuspend*/ ) throw (RuntimeException)
    {
        return sal_True;
    }

    //--------------------------------------------------------------------
    void SAL_CALL GenericPropertyHandler::disposing()
    {
        m_aPropertyListeners.clear();
            // not disposeAndClear: the listeners are (virtually) listeners at our introspectee, not
            // at this handler instance
    }

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XCOMPONENT( GenericPropertyHandler, GenericPropertyHandler_Base );

//........................................................................
}   // namespace pcr
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
