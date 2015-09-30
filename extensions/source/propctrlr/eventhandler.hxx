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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_EVENTHANDLER_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_EVENTHANDLER_HXX

#include "pcrcommontypes.hxx"
#include "pcrcommon.hxx"

#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/inspection/XPropertyHandler.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/compbase.hxx>
#include <comphelper/listenernotification.hxx>


namespace pcr
{



    //= EventDescription

    typedef sal_Int32   EventId;
    struct EventDescription
    {
    public:
        OUString sDisplayName;
        OUString sListenerClassName;
        OUString sListenerMethodName;
        OString  sHelpId;
        OString  sUniqueBrowseId;
        EventId         nId;

        EventDescription()
            :nId( 0 )
            {
            }

        EventDescription(
            EventId _nId,
            const sal_Char* _pListenerNamespaceAscii,
            const sal_Char* _pListenerClassAsciiName,
            const sal_Char* _pListenerMethodAsciiName,
            sal_uInt16 _nDisplayNameResId,
            const OString& _sHelpId,
            const OString& _sUniqueBrowseId );
    };

    typedef std::unordered_map< OUString, EventDescription, OUStringHash >   EventMap;


    //= EventHandler

    typedef ::cppu::WeakComponentImplHelper    <   css::inspection::XPropertyHandler
                                                ,   css::lang::XServiceInfo
                                                >   EventHandler_Base;
    class EventHandler : public EventHandler_Base
    {
    private:
        mutable ::osl::Mutex    m_aMutex;

        /// the context in which the instance was created
        css::uno::Reference< css::uno::XComponentContext >  m_xContext;
        /// the properties of the object we're handling
        css::uno::Reference< css::beans::XPropertySet >     m_xComponent;
        /// our XPropertyChangeListener(s)
        PropertyChangeListeners                             m_aPropertyListeners;
        /// cache of the events we found at our introspectee
        EventMap                                            m_aEvents;
        /// has m_aEvents been initialized?
        bool                                                m_bEventsMapInitialized;
        /// is our introspectee a dialog element?
        bool                                                m_bIsDialogElement;
            // TODO: move different handling into different derived classes?
        /// (FormComponent) type of the grid column being inspected, or -1 if we're not inspecting a grid column
        sal_Int16                                           m_nGridColumnType;

    public:
        // XServiceInfo - static versions
        static OUString SAL_CALL getImplementationName_static(  ) throw (css::uno::RuntimeException);
        static css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_static(  ) throw (css::uno::RuntimeException);
        static css::uno::Reference< css::uno::XInterface > Create( const css::uno::Reference< css::uno::XComponentContext >& _rxContext );

    protected:
        EventHandler(
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext
       );

        virtual ~EventHandler();

    protected:
        // XPropertyHandler overridables
        virtual void                                         SAL_CALL inspect( const css::uno::Reference< css::uno::XInterface >& _rxIntrospectee ) throw (css::lang::NullPointerException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Any                                SAL_CALL getPropertyValue( const OUString& _rPropertyName ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void                                         SAL_CALL setPropertyValue( const OUString& _rPropertyName, const css::uno::Any& _rValue ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Any                                SAL_CALL convertToPropertyValue( const OUString& _rPropertyName, const css::uno::Any& _rControlValue ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Any                                SAL_CALL convertToControlValue( const OUString& _rPropertyName, const css::uno::Any& _rPropertyValue, const css::uno::Type& _rControlValueType ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::beans::PropertyState                    SAL_CALL getPropertyState( const OUString& _rPropertyName ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void                                         SAL_CALL addPropertyChangeListener( const css::uno::Reference< css::beans::XPropertyChangeListener >& _rxListener ) throw (css::lang::NullPointerException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void                                         SAL_CALL removePropertyChangeListener( const css::uno::Reference< css::beans::XPropertyChangeListener >& _rxListener ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Sequence< css::beans::Property >   SAL_CALL getSupportedProperties() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Sequence< OUString >               SAL_CALL getSupersededProperties( ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Sequence< OUString >               SAL_CALL getActuatingProperties( ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::inspection::LineDescriptor              SAL_CALL describePropertyLine( const OUString& _rPropertyName, const css::uno::Reference< css::inspection::XPropertyControlFactory >& _rxControlFactory ) throw (css::beans::UnknownPropertyException, css::lang::NullPointerException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool                                     SAL_CALL isComposable( const OUString& _rPropertyName ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::inspection::InteractiveSelectionResult  SAL_CALL onInteractivePropertySelection( const OUString& _rPropertyName, sal_Bool _bPrimary, css::uno::Any& _rData, const css::uno::Reference< css::inspection::XObjectInspectorUI >& _rxInspectorUI ) throw (css::beans::UnknownPropertyException, css::lang::NullPointerException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void                                         SAL_CALL actuatingPropertyChanged( const OUString& _rActuatingPropertyName, const css::uno::Any& _rNewValue, const css::uno::Any& _rOldValue, const css::uno::Reference< css::inspection::XObjectInspectorUI >& _rxInspectorUI, sal_Bool _bFirstTimeInit ) throw (css::lang::NullPointerException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool                                     SAL_CALL suspend( sal_Bool _bSuspend ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XComponent
        DECLARE_XCOMPONENT()
        virtual void                                         SAL_CALL disposing() SAL_OVERRIDE;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    private:
        /** returns the script events associated with our introspectee
            @param  _out_rEvents
                Takes, upon successful return, the events currently associated with the introspectee
            @precond
                Our introspectee is a form component
        */
        void    impl_getFormComponentScriptEvents_nothrow(
                    css::uno::Sequence< css::script::ScriptEventDescriptor >& _out_rEvents
                ) const;

        /** returns the script events associated with our introspectee
            @param  _out_rEvents
                Takes, upon successful return, the events currently associated with the introspectee
            @precond
                Our introspectee is a dialog element
        */
        void    impl_getDialogElementScriptEvents_nothrow(
                    css::uno::Sequence< css::script::ScriptEventDescriptor >& _out_rEvents
                ) const;

        /** returns the script events associated with our introspectee
            @param  _out_rEvents
                Takes, the events currently associated with the introspectee
        */
        inline void impl_getComponentScriptEvents_nothrow(
                    css::uno::Sequence< css::script::ScriptEventDescriptor >& _out_rEvents
                ) const
        {
            if ( m_bIsDialogElement )
                impl_getDialogElementScriptEvents_nothrow( _out_rEvents );
            else
                impl_getFormComponentScriptEvents_nothrow( _out_rEvents );
        }

        /** returns the types of the listeners which can be registered at our introspectee
            @param  _out_rTypes
                Takes, upon successful return, the types of possible listeners at the introspectee
        */
        void    impl_getCopmonentListenerTypes_nothrow(
                    css::uno::Sequence< css::uno::Type >& _out_rTypes
                ) const;

        /** returns a secondary component to be used for event inspection

            In the UI, we want to mix events for the control model with events for the control.
            Since our introspectee is a model, this method creates a control for it (if possible).

            @return
                the secondary component whose events should be mixed with the introspectee's events
                The caller takes the ownership of the component (if not <NULL/>).

            @throws
                if an unexpected error occurs during creation of the secondary component.
                A <NULL/> component to be returned is not unexpected, but allowed

            @precond
                ->m_xComponent is not <NULL/>
        */
        css::uno::Reference< css::uno::XInterface >
                impl_getSecondaryComponentForEventInspection_throw( ) const;

        /** returns the event description for the given (programmatic) property name
            @param  _rPropertyName
                the name whose event description should be looked up
            @return
                the event description for the property name
            @throws css::beans::UnknownPropertyException
                if our introspectee does not have an event with the given logical name (see ->getSupportedProperties)
        */
        const EventDescription&
                impl_getEventForName_throw( const OUString& _rPropertyName ) const;

        /** returns the index of our component within its parent, if this parent can be
            obtained  (XChild::getParent) and supports an ->XIndexAccess interface
        */
        sal_Int32   impl_getComponentIndexInParent_throw() const;

        /** sets a given script event as event handler at a form component

            @param  _rScriptEvent
                the script event to set
        */
        void    impl_setFormComponentScriptEvent_nothrow( const css::script::ScriptEventDescriptor& _rScriptEvent );

        /** sets a given script event as event handler at a dialog component

            @param  _rScriptEvent
                the script event to set
        */
        void    impl_setDialogElementScriptEvent_nothrow( const css::script::ScriptEventDescriptor& _rScriptEvent );

        /** returns the frame associated with our context document
        */
        css::uno::Reference< css::frame::XFrame >
                impl_getContextFrame_nothrow() const;

        /** approves or denies a certain method to be included in the UI
            @return
                <TRUE/> if and only if the given method is allowed.
        */
        bool    impl_filterMethod_nothrow( const EventDescription& _rEvent ) const;

    private:
        EventHandler( const EventHandler& ) SAL_DELETED_FUNCTION;
        EventHandler& operator=( const EventHandler& ) SAL_DELETED_FUNCTION;
    };


} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_EVENTHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
