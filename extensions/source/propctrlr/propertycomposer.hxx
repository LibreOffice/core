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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYCOMPOSER_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYCOMPOSER_HXX

#include "pcrcommon.hxx"
#include "composeduiupdate.hxx"
#include "formbrowsertools.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/inspection/XPropertyHandler.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/beans/PropertyVetoException.hpp>
#include <cppuhelper/compbase.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/listenernotification.hxx>

#include <vector>
#include <set>


namespace pcr
{



    //= PropertyComposer

    typedef ::cppu::WeakComponentImplHelper    <   css::inspection::XPropertyHandler
                                                ,   css::beans::XPropertyChangeListener
                                                >   PropertyComposer_Base;
    /** implements an <type>XPropertyHandler</type> which composes it's information
        from a set of other property handlers
    */
    class PropertyComposer  :public ::comphelper::OBaseMutex
                            ,public PropertyComposer_Base
                            ,public IPropertyExistenceCheck
    {
    public:
        typedef ::std::vector< css::uno::Reference< css::inspection::XPropertyHandler > >
                                                            HandlerArray;

    private:
        HandlerArray                                    m_aSlaveHandlers;
        ::std::unique_ptr< ComposedPropertyUIUpdate >   m_pUIRequestComposer;
        PropertyChangeListeners                         m_aPropertyListeners;
        bool                                            m_bSupportedPropertiesAreKnown;
        PropertyBag                                     m_aSupportedProperties;

    public:
        /** constructs an <type>XPropertyHandler</type> which composes it's information from a set
            of other property handlers

            @param _rSlaveHandlers
                the set of slave handlers to invoke. Must not be <NULL/>
        */
        PropertyComposer( const ::std::vector< css::uno::Reference< css::inspection::XPropertyHandler > >& _rSlaveHandlers );

    public:
        // XPropertyHandler overridables
        virtual void                                SAL_CALL inspect( const css::uno::Reference< css::uno::XInterface >& _rxIntrospectee ) throw (css::lang::NullPointerException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Any                       SAL_CALL getPropertyValue( const OUString& _rPropertyName ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void                                SAL_CALL setPropertyValue( const OUString& _rPropertyName, const css::uno::Any& _rValue ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, css::beans::PropertyVetoException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Any SAL_CALL convertToPropertyValue( const OUString& _rPropertyName, const css::uno::Any& _rControlValue ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Any SAL_CALL convertToControlValue( const OUString& _rPropertyName, const css::uno::Any& _rPropertyValue, const css::uno::Type& _rControlValueType ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::beans::PropertyState
                                                    SAL_CALL getPropertyState( const OUString& _rPropertyName ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void                                SAL_CALL addPropertyChangeListener( const css::uno::Reference< css::beans::XPropertyChangeListener >& _rxListener ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void                                SAL_CALL removePropertyChangeListener( const css::uno::Reference< css::beans::XPropertyChangeListener >& _rxListener ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Sequence< css::beans::Property >
                                                    SAL_CALL getSupportedProperties() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Sequence< OUString >
                                                    SAL_CALL getSupersededProperties( ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Sequence< OUString >
                                                    SAL_CALL getActuatingProperties( ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::inspection::LineDescriptor
                                                    SAL_CALL describePropertyLine( const OUString& _rPropertyName, const css::uno::Reference< css::inspection::XPropertyControlFactory >& _rxControlFactory ) throw (css::beans::UnknownPropertyException, css::lang::NullPointerException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool                            SAL_CALL isComposable( const OUString& _rPropertyName ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::inspection::InteractiveSelectionResult
                                                    SAL_CALL onInteractivePropertySelection( const OUString& _rPropertyName, sal_Bool _bPrimary, css::uno::Any& _rData, const css::uno::Reference< css::inspection::XObjectInspectorUI >& _rxInspectorUI ) throw (css::beans::UnknownPropertyException, css::lang::NullPointerException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void                                SAL_CALL actuatingPropertyChanged( const OUString& _rActuatingPropertyName, const css::uno::Any& _rNewValue, const css::uno::Any& _rOldValue, const css::uno::Reference< css::inspection::XObjectInspectorUI >& _rxInspectorUI, sal_Bool _bFirstTimeInit ) throw (css::lang::NullPointerException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool                            SAL_CALL suspend( sal_Bool _bSuspend ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XComponent
        DECLARE_XCOMPONENT()
        virtual void                                SAL_CALL disposing() SAL_OVERRIDE;

        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const css::beans::PropertyChangeEvent& evt ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // IPropertyExistenceCheck
        virtual bool SAL_CALL hasPropertyByName( const OUString& _rName ) throw (css::uno::RuntimeException) SAL_OVERRIDE;

    private:
        /** ensures that m_pUIRequestComposer exists
        */
        void    impl_ensureUIRequestComposer( const css::uno::Reference< css::inspection::XObjectInspectorUI >& _rxInspectorUI );

        /** checks whether a given property exists in <member>m_aSupportedProperties</member>
        */
        bool    impl_isSupportedProperty_nothrow( const OUString& _rPropertyName )
        {
            css::beans::Property aDummy; aDummy.Name = _rPropertyName;
            return m_aSupportedProperties.find( aDummy ) != m_aSupportedProperties.end();
        }

    private:
        class MethodGuard;
        friend class MethodGuard;
        class MethodGuard : public ::osl::MutexGuard
        {
        public:
            MethodGuard( PropertyComposer& _rInstance )
                : ::osl::MutexGuard( _rInstance.m_aMutex )
            {
                if ( _rInstance.m_aSlaveHandlers.empty() )
                    throw css::lang::DisposedException( OUString(), *(&_rInstance) );
            }
        };
    };


} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYCOMPOSER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
