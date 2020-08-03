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

#pragma once

#include "pcrcommontypes.hxx"
#include "pcrcommon.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#include <com/sun/star/inspection/XPropertyHandler.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/compbase.hxx>
#include <rtl/ref.hxx>

#include <map>


namespace pcr
{


    struct TypeLess
    {
        bool operator()( const css::uno::Type& _rLHS, const css::uno::Type& _rRHS ) const
        {
            return _rLHS.getTypeName() < _rRHS.getTypeName();
        }
    };

    class IPropertyEnumRepresentation;

    //= GenericPropertyHandler

    typedef ::cppu::WeakComponentImplHelper    <   css::inspection::XPropertyHandler
                                                ,  css::lang::XServiceInfo
                                                >   GenericPropertyHandler_Base;
    class GenericPropertyHandler final : public GenericPropertyHandler_Base
    {
        mutable ::osl::Mutex    m_aMutex;

        /// the service factory for creating services
        css::uno::Reference< css::uno::XComponentContext >        m_xContext;
        /// need this to keep alive as long as m_xComponent lives
        css::uno::Reference< css::beans::XIntrospectionAccess >   m_xComponentIntrospectionAccess;
        /// the properties of the object we're handling
        css::uno::Reference< css::beans::XPropertySet >           m_xComponent;
        /// cached interface of ->m_xComponent
        css::uno::Reference< css::beans::XPropertyState >         m_xPropertyState;
        /// type converter, needed on various occasions
        css::uno::Reference< css::script::XTypeConverter >        m_xTypeConverter;
        /// cache of our supported properties
        PropertyMap                                                                         m_aProperties;
        /// property change listeners
        ::comphelper::OInterfaceContainerHelper2                                                   m_aPropertyListeners;
        std::map< css::uno::Type, ::rtl::Reference< IPropertyEnumRepresentation >, TypeLess >
                                                                                            m_aEnumConverters;

        /// has ->m_aProperties been initialized?
        bool    m_bPropertyMapInitialized : 1;

    public:
        explicit GenericPropertyHandler(
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext
        );

        virtual ~GenericPropertyHandler() override;

    private:
        // XPropertyHandler overridables
        virtual void                                                SAL_CALL inspect( const css::uno::Reference< css::uno::XInterface >& _rxIntrospectee ) override;
        virtual css::uno::Any                                       SAL_CALL getPropertyValue( const OUString& _rPropertyName ) override;
        virtual void                                                SAL_CALL setPropertyValue( const OUString& _rPropertyName, const css::uno::Any& _rValue ) override;
        virtual css::uno::Any                                       SAL_CALL convertToPropertyValue( const OUString& _rPropertyName, const css::uno::Any& _rControlValue ) override;
        virtual css::uno::Any                                       SAL_CALL convertToControlValue( const OUString& _rPropertyName, const css::uno::Any& _rPropertyValue, const css::uno::Type& _rControlValueType ) override;
        virtual css::beans::PropertyState                           SAL_CALL getPropertyState( const OUString& _rPropertyName ) override;
        virtual void                                                SAL_CALL addPropertyChangeListener( const css::uno::Reference< css::beans::XPropertyChangeListener >& _rxListener ) override;
        virtual void                                                SAL_CALL removePropertyChangeListener( const css::uno::Reference< css::beans::XPropertyChangeListener >& _rxListener ) override;
        virtual css::uno::Sequence< css::beans::Property >
                                                                    SAL_CALL getSupportedProperties() override;
        virtual css::uno::Sequence< OUString >
                                                                    SAL_CALL getSupersededProperties() override;
        virtual css::uno::Sequence< OUString >                      SAL_CALL getActuatingProperties() override;
        virtual css::inspection::LineDescriptor                     SAL_CALL describePropertyLine( const OUString& _rPropertyName, const css::uno::Reference< css::inspection::XPropertyControlFactory >& _rxControlFactory ) override;
        virtual sal_Bool                                            SAL_CALL isComposable( const OUString& _rPropertyName ) override;
        virtual css::inspection::InteractiveSelectionResult
                                                                    SAL_CALL onInteractivePropertySelection( const OUString& _rPropertyName, sal_Bool _bPrimary, css::uno::Any& _rData, const css::uno::Reference< css::inspection::XObjectInspectorUI >& _rxInspectorUI ) override;
        virtual void                                                SAL_CALL actuatingPropertyChanged( const OUString& _rActuatingPropertyName, const css::uno::Any& _rNewValue, const css::uno::Any& _rOldValue, const css::uno::Reference< css::inspection::XObjectInspectorUI >& _rxInspectorUI, sal_Bool _bFirstTimeInit ) override;
        virtual sal_Bool                                            SAL_CALL suspend( sal_Bool _bSuspend ) override;

        // XComponent
        DECLARE_XCOMPONENT()
        virtual void                                                SAL_CALL disposing() override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

        /** ensures that ->m_aProperties is initialized
            @precond
                our mutex is locked
        */
        void    impl_ensurePropertyMap();

        /** retrieves the enum converter for the given ENUM type
        */
        ::rtl::Reference< IPropertyEnumRepresentation >
                impl_getEnumConverter( const css::uno::Type& _rEnumType );

        GenericPropertyHandler( const GenericPropertyHandler& ) = delete;
        GenericPropertyHandler& operator=( const GenericPropertyHandler& ) = delete;
    };


} // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
