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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_GENERICPROPERTYHANDLER_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_GENERICPROPERTYHANDLER_HXX

#include "propertyhandler.hxx"
#include "pcrcommontypes.hxx"
#include "pcrcommon.hxx"

#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <rtl/ref.hxx>

#include <map>


namespace pcr
{


    struct TypeLess : ::std::binary_function< ::com::sun::star::uno::Type, ::com::sun::star::uno::Type, bool >
    {
        bool operator()( const ::com::sun::star::uno::Type& _rLHS, const ::com::sun::star::uno::Type& _rRHS ) const
        {
            return _rLHS.getTypeName() < _rRHS.getTypeName();
        }
    };

    class IPropertyEnumRepresentation;

    //= GenericPropertyHandler

    typedef ::cppu::WeakComponentImplHelper2    <   ::com::sun::star::inspection::XPropertyHandler
                                                ,   ::com::sun::star::lang::XServiceInfo
                                                >   GenericPropertyHandler_Base;
    class GenericPropertyHandler : public GenericPropertyHandler_Base
    {
    private:
        mutable ::osl::Mutex    m_aMutex;

    private:
        /// the service factory for creating services
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >        m_xContext;
        /// need this to keep alive as long as m_xComponent lives
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XIntrospectionAccess >   m_xComponentIntrospectionAccess;
        /// the properties of the object we're handling
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >           m_xComponent;
        /// cached interface of ->m_xComponent
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >         m_xPropertyState;
        /// type converter, needed on various occasions
        ::com::sun::star::uno::Reference< ::com::sun::star::script::XTypeConverter >        m_xTypeConverter;
        /// cache of our supported properties
        PropertyMap                                                                         m_aProperties;
        /// property change listeners
        ::cppu::OInterfaceContainerHelper                                                   m_aPropertyListeners;
        ::std::map< ::com::sun::star::uno::Type, ::rtl::Reference< IPropertyEnumRepresentation >, TypeLess >
                                                                                            m_aEnumConverters;

        /// has ->m_aProperties been initialized?
        bool    m_bPropertyMapInitialized : 1;

    public:
        // XServiceInfo - static versions
        static OUString SAL_CALL getImplementationName_static(  ) throw (::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_static(  ) throw (::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > Create( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext );

    protected:
        GenericPropertyHandler(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext
        );

        virtual ~GenericPropertyHandler();

    protected:
        // XPropertyHandler overridables
        virtual void                                                SAL_CALL inspect( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxIntrospectee ) throw (::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Any                          SAL_CALL getPropertyValue( const OUString& _rPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void                                                SAL_CALL setPropertyValue( const OUString& _rPropertyName, const ::com::sun::star::uno::Any& _rValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Any                          SAL_CALL convertToPropertyValue( const OUString& _rPropertyName, const ::com::sun::star::uno::Any& _rControlValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Any                          SAL_CALL convertToControlValue( const OUString& _rPropertyName, const ::com::sun::star::uno::Any& _rPropertyValue, const ::com::sun::star::uno::Type& _rControlValueType ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::beans::PropertyState              SAL_CALL getPropertyState( const OUString& _rPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void                                                SAL_CALL addPropertyChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& _rxListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void                                                SAL_CALL removePropertyChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& _rxListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >
                                                                    SAL_CALL getSupportedProperties() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< OUString >
                                                                    SAL_CALL getSupersededProperties() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< OUString >  SAL_CALL getActuatingProperties() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::inspection::LineDescriptor        SAL_CALL describePropertyLine( const OUString& _rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControlFactory >& _rxControlFactory ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool                                          SAL_CALL isComposable( const OUString& _rPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::inspection::InteractiveSelectionResult
                                                                    SAL_CALL onInteractivePropertySelection( const OUString& _rPropertyName, sal_Bool _bPrimary, ::com::sun::star::uno::Any& _rData, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI >& _rxInspectorUI ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void                                                SAL_CALL actuatingPropertyChanged( const OUString& _rActuatingPropertyName, const ::com::sun::star::uno::Any& _rNewValue, const ::com::sun::star::uno::Any& _rOldValue, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI >& _rxInspectorUI, sal_Bool _bFirstTimeInit ) throw (::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool                                            SAL_CALL suspend( sal_Bool _bSuspend ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XComponent
        DECLARE_XCOMPONENT()
        virtual void                                                SAL_CALL disposing() SAL_OVERRIDE;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    private:
        /** ensures that ->m_aProperties is initialized
            @precond
                our mutex is locked
        */
        void    impl_ensurePropertyMap();

        /** retrieves the enum converter for the given ENUM type
        */
        ::rtl::Reference< IPropertyEnumRepresentation >
                impl_getEnumConverter( const ::com::sun::star::uno::Type& _rEnumType );

    private:
        GenericPropertyHandler();                                           // never implemented
        GenericPropertyHandler( const GenericPropertyHandler& );            // never implemented
        GenericPropertyHandler& operator=( const GenericPropertyHandler& ); // never implemented
    };


} // namespace pcr


#endif // EXTENSIONS_SOURCE_PROPCTRLR_GENERICPROPERTYHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
