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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYCOMPOSER_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYCOMPOSER_HXX

#include "pcrcommon.hxx"
#include "composeduiupdate.hxx"
#include "formbrowsertools.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/inspection/XPropertyHandler.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/beans/PropertyVetoException.hpp>
#include <cppuhelper/compbase2.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/listenernotification.hxx>

#include <vector>
#include <set>

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= PropertyComposer
    //====================================================================
    typedef ::cppu::WeakComponentImplHelper2    <   ::com::sun::star::inspection::XPropertyHandler
                                                ,   ::com::sun::star::beans::XPropertyChangeListener
                                                >   PropertyComposer_Base;
    /** implements an <type>XPropertyHandler</type> which composes it's information
        from a set of other property handlers
    */
    class PropertyComposer  :public PropertyComposer_Base
                            ,public ::comphelper::OBaseMutex
                            ,public IPropertyExistenceCheck
    {
    public:
        typedef ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyHandler > >
                                                            HandlerArray;

    private:
        HandlerArray                                    m_aSlaveHandlers;
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr< ComposedPropertyUIUpdate >     m_pUIRequestComposer;
        SAL_WNODEPRECATED_DECLARATIONS_POP
        PropertyChangeListeners                         m_aPropertyListeners;
        bool                                            m_bSupportedPropertiesAreKnown;
        PropertyBag                                     m_aSupportedProperties;

    public:
        /** constructs an <type>XPropertyHandler</type> which composes it's information from a set
            of other property handlers

            @param _rSlaveHandlers
                the set of slave handlers to invoke. Must not be <NULL/>
        */
        PropertyComposer( const ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyHandler > >& _rSlaveHandlers );

    public:
        // XPropertyHandler overridables
        virtual void                                SAL_CALL inspect( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxIntrospectee ) throw (::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Any          SAL_CALL getPropertyValue( const OUString& _rPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual void                                SAL_CALL setPropertyValue( const OUString& _rPropertyName, const ::com::sun::star::uno::Any& _rValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::PropertyVetoException, std::exception);
        virtual ::com::sun::star::uno::Any SAL_CALL convertToPropertyValue( const OUString& _rPropertyName, const ::com::sun::star::uno::Any& _rControlValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Any SAL_CALL convertToControlValue( const OUString& _rPropertyName, const ::com::sun::star::uno::Any& _rPropertyValue, const ::com::sun::star::uno::Type& _rControlValueType ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::beans::PropertyState
                                                    SAL_CALL getPropertyState( const OUString& _rPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual void                                SAL_CALL addPropertyChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& _rxListener ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void                                SAL_CALL removePropertyChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& _rxListener ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >
                                                    SAL_CALL getSupportedProperties() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Sequence< OUString >
                                                    SAL_CALL getSupersededProperties( ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Sequence< OUString >
                                                    SAL_CALL getActuatingProperties( ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::inspection::LineDescriptor
                                                    SAL_CALL describePropertyLine( const OUString& _rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControlFactory >& _rxControlFactory ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::sal_Bool                          SAL_CALL isComposable( const OUString& _rPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::inspection::InteractiveSelectionResult
                                                    SAL_CALL onInteractivePropertySelection( const OUString& _rPropertyName, sal_Bool _bPrimary, ::com::sun::star::uno::Any& _rData, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI >& _rxInspectorUI ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual void                                SAL_CALL actuatingPropertyChanged( const OUString& _rActuatingPropertyName, const ::com::sun::star::uno::Any& _rNewValue, const ::com::sun::star::uno::Any& _rOldValue, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI >& _rxInspectorUI, sal_Bool _bFirstTimeInit ) throw (::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual sal_Bool                            SAL_CALL suspend( sal_Bool _bSuspend ) throw (::com::sun::star::uno::RuntimeException, std::exception);

        // XComponent
        DECLARE_XCOMPONENT()
        virtual void                                SAL_CALL disposing();

        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw (::com::sun::star::uno::RuntimeException, std::exception);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception);

        // IPropertyExistenceCheck
        virtual ::sal_Bool SAL_CALL hasPropertyByName( const OUString& _rName ) throw (::com::sun::star::uno::RuntimeException);

    private:
        /** ensures that m_pUIRequestComposer exists
        */
        void    impl_ensureUIRequestComposer( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI >& _rxInspectorUI );

        /** checks whether a given property exists in <member>m_aSupportedProperties</member>
        */
        bool    impl_isSupportedProperty_nothrow( const OUString& _rPropertyName )
        {
            ::com::sun::star::beans::Property aDummy; aDummy.Name = _rPropertyName;
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
                    throw ::com::sun::star::lang::DisposedException( OUString(), *(&_rInstance) );
            }
        };
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYCOMPOSER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
