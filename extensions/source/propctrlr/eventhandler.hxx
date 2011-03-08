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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_EVENTHANDLER_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_EVENTHANDLER_HXX

#include "pcrcomponentcontext.hxx"
#include "pcrcommontypes.hxx"
#include "pcrcommon.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/inspection/XPropertyHandler.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XFrame.hpp>
/** === end UNO includes === **/
#include <cppuhelper/compbase2.hxx>
#include <comphelper/listenernotification.hxx>

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= EventDescription
    //====================================================================
    typedef sal_Int32   EventId;
    struct EventDescription
    {
    public:
        ::rtl::OUString sDisplayName;
        ::rtl::OUString sListenerClassName;
        ::rtl::OUString sListenerMethodName;
        sal_Int32       nHelpId;
        sal_Int32       nUniqueBrowseId;
        EventId         nId;

        EventDescription()
            :nHelpId( 0 )
            ,nUniqueBrowseId( 0 )
            ,nId( 0 )
            {
            }

        EventDescription(
            EventId _nId,
            const sal_Char* _pListenerNamespaceAscii,
            const sal_Char* _pListenerClassAsciiName,
            const sal_Char* _pListenerMethodAsciiName,
            sal_uInt16 _nDisplayNameResId,
            sal_Int32 _nHelpId,
            sal_Int32 _nUniqueBrowseId );
    };

    typedef ::boost::unordered_map< ::rtl::OUString, EventDescription, ::rtl::OUStringHash >   EventMap;

    //====================================================================
    //= EventHandler
    //====================================================================
    typedef ::cppu::WeakComponentImplHelper2    <   ::com::sun::star::inspection::XPropertyHandler
                                                ,   ::com::sun::star::lang::XServiceInfo
                                                >   EventHandler_Base;
    class EventHandler : public EventHandler_Base
    {
    private:
        mutable ::osl::Mutex    m_aMutex;

        /// the context in which the instance was created
        ComponentContext                                                            m_aContext;
        /// the properties of the object we're handling
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   m_xComponent;
        /// our XPropertyChangeListener(s)
        PropertyChangeListeners                                                     m_aPropertyListeners;
        /// cache of the events we found at our introspectee
        EventMap                                                                    m_aEvents;
        /// has m_aEvents been initialized?
        bool                                                                        m_bEventsMapInitialized;
        /// is our introspectee a dialog element?
        bool                                                                        m_bIsDialogElement;
            // TODO: move different handling into different derived classes?
        /// (FormComponent) type of the grid column being inspected, or -1 if we're not inspecting a grid column
        sal_Int16                                                                   m_nGridColumnType;

    public:
        // XServiceInfo - static versions
        static ::rtl::OUString SAL_CALL getImplementationName_static(  ) throw (::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_static(  ) throw (::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > Create( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext );

    protected:
        EventHandler(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext
       );

        ~EventHandler();

    protected:
        // XPropertyHandler overridables
        virtual void                                                SAL_CALL inspect( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxIntrospectee ) throw (::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any                          SAL_CALL getPropertyValue( const ::rtl::OUString& _rPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual void                                                SAL_CALL setPropertyValue( const ::rtl::OUString& _rPropertyName, const ::com::sun::star::uno::Any& _rValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any                          SAL_CALL convertToPropertyValue( const ::rtl::OUString& _rPropertyName, const ::com::sun::star::uno::Any& _rControlValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any                          SAL_CALL convertToControlValue( const ::rtl::OUString& _rPropertyName, const ::com::sun::star::uno::Any& _rPropertyValue, const ::com::sun::star::uno::Type& _rControlValueType ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::beans::PropertyState              SAL_CALL getPropertyState( const ::rtl::OUString& _rPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual void                                                SAL_CALL addPropertyChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& _rxListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void                                                SAL_CALL removePropertyChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& _rxListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >
                                                                    SAL_CALL getSupportedProperties() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >  SAL_CALL getSupersededProperties( ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >  SAL_CALL getActuatingProperties( ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::inspection::LineDescriptor        SAL_CALL describePropertyLine( const ::rtl::OUString& _rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControlFactory >& _rxControlFactory ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool                                          SAL_CALL isComposable( const ::rtl::OUString& _rPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::inspection::InteractiveSelectionResult
                                                                    SAL_CALL onInteractivePropertySelection( const ::rtl::OUString& _rPropertyName, sal_Bool _bPrimary, ::com::sun::star::uno::Any& _rData, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI >& _rxInspectorUI ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException);
        virtual void                                                SAL_CALL actuatingPropertyChanged( const ::rtl::OUString& _rActuatingPropertyName, const ::com::sun::star::uno::Any& _rNewValue, const ::com::sun::star::uno::Any& _rOldValue, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI >& _rxInspectorUI, sal_Bool _bFirstTimeInit ) throw (::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool                                            SAL_CALL suspend( sal_Bool _bSuspend ) throw (::com::sun::star::uno::RuntimeException);

        // XComponent
        DECLARE_XCOMPONENT()
        virtual void                                                SAL_CALL disposing();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

    private:
        /** returns the script events associated with our introspectee
            @param  _out_rEvents
                Takes, upon successfull return, the events currently associated with the introspectee
            @precond
                Our introspectee is a form component
        */
        void    impl_getFormComponentScriptEvents_nothrow(
                    ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >& _out_rEvents
                ) const;

        /** returns the script events associated with our introspectee
            @param  _out_rEvents
                Takes, upon successfull return, the events currently associated with the introspectee
            @precond
                Our introspectee is a dialog element
        */
        void    impl_getDialogElementScriptEvents_nothrow(
                    ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >& _out_rEvents
                ) const;

        /** returns the script events associated with our introspectee
            @param  _out_rEvents
                Takes, the events currently associated with the introspectee
        */
        inline void impl_getComponentScriptEvents_nothrow(
                    ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >& _out_rEvents
                ) const
        {
            if ( m_bIsDialogElement )
                impl_getDialogElementScriptEvents_nothrow( _out_rEvents );
            else
                impl_getFormComponentScriptEvents_nothrow( _out_rEvents );
        }

        /** returns the types of the listeners which can be registered at our introspectee
            @param  _out_rTypes
                Takes, upon successfull return, the types of possible listeners at the introspectee
        */
        void    impl_getCopmonentListenerTypes_nothrow(
                    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >& _out_rTypes
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
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                impl_getSecondaryComponentForEventInspection_throw( ) const;

        /** returns the event description for the given (programmatic) property name
            @param  _rPropertyName
                the name whose event description should be looked up
            @return
                the event description for the property name
            @throws ::com::sun::star::beans::UnknownPropertyException
                if our introspectee does not have an event with the given logical name (see ->getSupportedProperties)
        */
        const EventDescription&
                impl_getEventForName_throw( const ::rtl::OUString& _rPropertyName ) const;

        /** returns the index of our component within its parent, if this parent can be
            obtained  (XChild::getParent) and supports an ->XIndexAccess interface
        */
        sal_Int32   impl_getComponentIndexInParent_throw() const;

        /** sets a given script event as event handler at a form component

            @param  _rScriptEvent
                the script event to set
        */
        void    impl_setFormComponentScriptEvent_nothrow( const ::com::sun::star::script::ScriptEventDescriptor& _rScriptEvent );

        /** sets a given script event as event handler at a dialog component

            @param  _rScriptEvent
                the script event to set
        */
        void    impl_setDialogElementScriptEvent_nothrow( const ::com::sun::star::script::ScriptEventDescriptor& _rScriptEvent );

        /** returns the frame associated with our context document
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >
                impl_getContextFrame_nothrow() const;

        /** approves or denies a certain method to be included in the UI
            @return
                <TRUE/> if and only if the given method is allowed.
        */
        bool    impl_filterMethod_nothrow( const EventDescription& _rEvent ) const;

    private:
        EventHandler();                                 // never implemented
        EventHandler( const EventHandler& );            // never implemented
        EventHandler& operator=( const EventHandler& ); // never implemented
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_EVENTHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
