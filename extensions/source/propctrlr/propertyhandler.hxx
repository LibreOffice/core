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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYHANDLER_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYHANDLER_HXX

#include "pcrcommon.hxx"
#include "modulepcr.hxx"

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/inspection/XPropertyHandler.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <osl/interlck.h>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>

#include <memory>
#include <vector>

namespace com { namespace sun { namespace star {
    namespace inspection {
        struct LineDescriptor;
        class XPropertyControlFactory;
    }
} } }

namespace vcl { class Window; }

namespace pcr
{


    typedef sal_Int32   PropertyId;


    //= PropertyHandler

    class OPropertyInfoService;
    typedef ::cppu::WeakComponentImplHelper    <   css::inspection::XPropertyHandler
                                                >   PropertyHandler_Base;
    /** the base class for property handlers
    */
    class PropertyHandler : public PropertyHandler_Base
    {
    private:
        /// cache for getSupportedProperties
        mutable StlSyntaxSequence< css::beans::Property >
                                    m_aSupportedProperties;
        mutable bool                m_bSupportedPropertiesAreKnown;

        /// helper which ensures that we can access resources as long as the instance lives
        PcrClient       m_aEnsureResAccess;

    private:
        /// the property listener which has been registered
        PropertyChangeListeners                               m_aPropertyListeners;

    protected:
        mutable ::osl::Mutex                                  m_aMutex;
        /// the context in which the instance was created
        css::uno::Reference< css::uno::XComponentContext >    m_xContext;
        /// the component we're inspecting
        css::uno::Reference< css::beans::XPropertySet >       m_xComponent;
        /// info about our component's properties
        css::uno::Reference< css::beans::XPropertySetInfo >   m_xComponentPropertyInfo;
        /// type converter, needed on various occasions
        css::uno::Reference< css::script::XTypeConverter >    m_xTypeConverter;
        /// access to property meta data
        ::std::unique_ptr< OPropertyInfoService >             m_pInfoService;

    protected:
        explicit PropertyHandler(
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext
        );
        virtual ~PropertyHandler() override;

        // default implementations for XPropertyHandler
        virtual void SAL_CALL inspect( const css::uno::Reference< css::uno::XInterface >& _rxIntrospectee ) throw (css::uno::RuntimeException, css::lang::NullPointerException, std::exception) override;
        virtual css::uno::Sequence< css::beans::Property > SAL_CALL getSupportedProperties() throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupersededProperties( ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getActuatingProperties( ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any SAL_CALL convertToPropertyValue( const OUString& _rPropertyName, const css::uno::Any& _rControlValue ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any SAL_CALL convertToControlValue( const OUString& _rPropertyName, const css::uno::Any& _rPropertyValue, const css::uno::Type& _rControlValueType ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
        virtual css::beans::PropertyState  SAL_CALL getPropertyState( const OUString& _rPropertyName ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
        virtual css::inspection::LineDescriptor SAL_CALL describePropertyLine( const OUString& _rPropertyName, const css::uno::Reference< css::inspection::XPropertyControlFactory >& _rxControlFactory ) throw (css::beans::UnknownPropertyException, css::lang::NullPointerException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL isComposable( const OUString& _rPropertyName ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
        virtual css::inspection::InteractiveSelectionResult SAL_CALL onInteractivePropertySelection( const OUString& _rPropertyName, sal_Bool _bPrimary, css::uno::Any& _rData, const css::uno::Reference< css::inspection::XObjectInspectorUI >& _rxInspectorUI ) throw (css::beans::UnknownPropertyException, css::lang::NullPointerException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL actuatingPropertyChanged( const OUString& _rActuatingPropertyName, const css::uno::Any& _rNewValue, const css::uno::Any& _rOldValue, const css::uno::Reference< css::inspection::XObjectInspectorUI >& _rxInspectorUI, sal_Bool _bFirstTimeInit ) throw (css::lang::NullPointerException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addPropertyChangeListener( const css::uno::Reference< css::beans::XPropertyChangeListener >& _rxListener ) throw (css::lang::NullPointerException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removePropertyChangeListener( const css::uno::Reference< css::beans::XPropertyChangeListener >& _rxListener ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL suspend( sal_Bool _bSuspend ) throw (css::uno::RuntimeException, std::exception) override;

        // XComponent
        DECLARE_XCOMPONENT()
        virtual void SAL_CALL disposing() override;

        // own overridables
        virtual css::uno::Sequence< css::beans::Property >
                    SAL_CALL doDescribeSupportedProperties() const = 0;

        /// called when XPropertyHandler::inspect has been called, and we thus have a new component to inspect
        virtual void onNewComponent();

    protected:
        /** fires the change in a property value to our listener (if any)
            @see addPropertyChangeListener
        */
        void    firePropertyChange( const OUString& _rPropName, PropertyId _nPropId,
                    const css::uno::Any& _rOldValue, const css::uno::Any& _rNewValue );

        /** retrieves a window which can be used as parent for dialogs
        */
        vcl::Window* impl_getDefaultDialogParent_nothrow() const;

        /** retrieves the property id for a given property name
            @throw css::beans::UnknownPropertyException
                if the property name is not known to our ->m_pInfoService
        */
        PropertyId impl_getPropertyId_throwUnknownProperty( const OUString& _rPropertyName ) const;

        /** retrieves the property id for a given property name
            @throw css::uno::RuntimeException
                if the property name is not known to our ->m_pInfoService
        */
        PropertyId impl_getPropertyId_throwRuntime( const OUString& _rPropertyName ) const;


        /** retrieves the property id for a given property name
            @returns -1
                if the property name is not known to our ->m_pInfoService
        */
        PropertyId impl_getPropertyId_nothrow( const OUString& _rPropertyName ) const;

        // helper for implementing doDescribeSupportedProperties
        /** adds a description for the given string property to the given property vector
            Most probably to be called from within getSupportedProperties
        */
        inline void addStringPropertyDescription(
                    ::std::vector< css::beans::Property >& _rProperties,
                    const OUString& _rPropertyName
                ) const;

        /** adds a description for the given int32 property to the given property vector
        */
        inline void addInt32PropertyDescription(
                    ::std::vector< css::beans::Property >& _rProperties,
                    const OUString& _rPropertyName,
                    sal_Int16 _nAttribs = 0
                ) const;

        /** adds a description for the given int16 property to the given property vector
        */
        inline void addInt16PropertyDescription(
                    ::std::vector< css::beans::Property >& _rProperties,
                    const OUString& _rPropertyName,
                    sal_Int16 _nAttribs = 0
                ) const;

        /** adds a description for the given double property to the given property vector
        */
        inline void addDoublePropertyDescription(
                    ::std::vector< css::beans::Property >& _rProperties,
                    const OUString& _rPropertyName,
                    sal_Int16 _nAttribs
                ) const;

        /** adds a description for the given date property to the given property vector
        */
        inline void addDatePropertyDescription(
                    ::std::vector< css::beans::Property >& _rProperties,
                    const OUString& _rPropertyName,
                    sal_Int16 _nAttribs
                ) const;

        /** adds a description for the given time property to the given property vector
        */
        inline void addTimePropertyDescription(
                    ::std::vector< css::beans::Property >& _rProperties,
                    const OUString& _rPropertyName,
                    sal_Int16 _nAttribs
                ) const;

        /** adds a description for the given DateTime property to the given property vector
        */
        inline void addDateTimePropertyDescription(
                    ::std::vector< css::beans::Property >& _rProperties,
                    const OUString& _rPropertyName,
                    sal_Int16 _nAttribs
                ) const;

        /// adds a Property, given by name only, to a given vector of Properties
        void implAddPropertyDescription(
                    ::std::vector< css::beans::Property >& _rProperties,
                    const OUString& _rPropertyName,
                    const css::uno::Type& _rType,
                    sal_Int16 _nAttribs = 0
                ) const;


        // helper for accessing and maintaining meta data about our supported properties

        /** retrieves a property given by handle

            @return
                a pointer to the descriptor for the given properties, if it is one of our
                supported properties, <NULL/> else.

            @see doDescribeSupportedProperties
            @see impl_getPropertyFromId_throw
        */
        const css::beans::Property*
                    impl_getPropertyFromId_nothrow( PropertyId _nPropId ) const;

        /** retrieves a property given by handle

            @throws UnknownPropertyException
                if the handler does not support a property with the given handle

            @seealso doDescribeSupportedProperties
            @see impl_getPropertyFromId_nothrow
        */
        const css::beans::Property&
                    impl_getPropertyFromId_throw( PropertyId _nPropId ) const;

        /** determines whether a given property id is part of our supported properties
            @see getSupportedProperties
            @see doDescribeSupportedProperties
        */
        inline bool impl_isSupportedProperty_nothrow( PropertyId _nPropId ) const
        {
            return impl_getPropertyFromId_nothrow( _nPropId ) != nullptr;
        }

        /** retrieves a property given by name

            @throws UnknownPropertyException
                if the handler does not support a property with the given name

            @seealso doDescribeSupportedProperties
        */
        const css::beans::Property&
                    impl_getPropertyFromName_throw( const OUString& _rPropertyName ) const;

        /** get the name of a property given by handle
        */
        inline OUString
                    impl_getPropertyNameFromId_nothrow( PropertyId _nPropId ) const;

        /** returns the value of the ContextDocument property in the ComponentContext which was used to create
            this handler.
        */
        inline css::uno::Reference< css::frame::XModel >
                    impl_getContextDocument_nothrow() const
        {
            return css::uno::Reference< css::frame::XModel >(
                m_xContext->getValueByName( "ContextDocument" ), css::uno::UNO_QUERY );
        }

        /** marks the context document as modified

            @see impl_getContextDocument_nothrow
        */
        void impl_setContextDocumentModified_nothrow() const;

        /// determines whether our component has a given property
        bool impl_componentHasProperty_throw( const OUString& _rPropName ) const;

        /** determines the default measure unit for the document in which our component lives
        */
        sal_Int16 impl_getDocumentMeasurementUnit_throw() const;

    private:
        PropertyHandler( const PropertyHandler& ) = delete;
        PropertyHandler& operator=( const PropertyHandler& ) = delete;
    };


    inline void PropertyHandler::addStringPropertyDescription( ::std::vector< css::beans::Property >& _rProperties, const OUString& _rPropertyName ) const
    {
        implAddPropertyDescription( _rProperties, _rPropertyName, ::cppu::UnoType<OUString>::get() );
    }

    inline void PropertyHandler::addInt32PropertyDescription( ::std::vector< css::beans::Property >& _rProperties, const OUString& _rPropertyName, sal_Int16 _nAttribs ) const
    {
        implAddPropertyDescription( _rProperties, _rPropertyName, ::cppu::UnoType<sal_Int32>::get(), _nAttribs );
    }

    inline void PropertyHandler::addInt16PropertyDescription( ::std::vector< css::beans::Property >& _rProperties, const OUString& _rPropertyName, sal_Int16 _nAttribs ) const
    {
        implAddPropertyDescription( _rProperties, _rPropertyName, ::cppu::UnoType<sal_Int16>::get(), _nAttribs );
    }

    inline void PropertyHandler::addDoublePropertyDescription( ::std::vector< css::beans::Property >& _rProperties, const OUString& _rPropertyName, sal_Int16 _nAttribs ) const
    {
        implAddPropertyDescription( _rProperties, _rPropertyName, ::cppu::UnoType<double>::get(), _nAttribs );
    }

    inline void PropertyHandler::addDatePropertyDescription( ::std::vector< css::beans::Property >& _rProperties, const OUString& _rPropertyName, sal_Int16 _nAttribs ) const
    {
        implAddPropertyDescription( _rProperties, _rPropertyName, ::cppu::UnoType<css::util::Date>::get(), _nAttribs );
    }

    inline void PropertyHandler::addTimePropertyDescription( ::std::vector< css::beans::Property >& _rProperties, const OUString& _rPropertyName, sal_Int16 _nAttribs ) const
    {
        implAddPropertyDescription( _rProperties, _rPropertyName, ::cppu::UnoType<css::util::Time>::get(), _nAttribs );
    }

    inline void PropertyHandler::addDateTimePropertyDescription( ::std::vector< css::beans::Property >& _rProperties, const OUString& _rPropertyName, sal_Int16 _nAttribs ) const
    {
        implAddPropertyDescription( _rProperties, _rPropertyName, ::cppu::UnoType<css::util::DateTime>::get(), _nAttribs );
    }

    inline OUString PropertyHandler::impl_getPropertyNameFromId_nothrow( PropertyId _nPropId ) const
    {
        const css::beans::Property* pProp = impl_getPropertyFromId_nothrow( _nPropId );
        return pProp ? pProp->Name : OUString();
    }


    //= PropertyHandlerComponent

    typedef ::cppu::ImplHelper  <   css::lang::XServiceInfo
                                >   PropertyHandlerComponent_Base;
    /** PropertyHandler implementation which additionally supports XServiceInfo
    */
    class PropertyHandlerComponent  :public PropertyHandler
                                    ,public PropertyHandlerComponent_Base
    {
    protected:
        explicit PropertyHandlerComponent(
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext
        );

        DECLARE_XINTERFACE()
        DECLARE_XTYPEPROVIDER()

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException, std::exception) override = 0;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception) override = 0;
    };


    //= HandlerComponentBase

    /** a PropertyHandlerComponent implementation which routes XServiceInfo::getImplementationName and
        XServiceInfo::getSupportedServiceNames to static versions of those methods, which are part of
        the derived class.

        Additionally, a method <member>Create</member> is provided which takes a component context, and returns a new
        instance of the derived class. This <member>Create</member> is used to register the implementation
        of the derived class at the <type>PcrModule</type>.

        Well, every time we're talking about derived class, we in fact mean the template argument of
        <type>HandlerComponentBase</type>. But usually this equals your derived class:
        <pre>
        class MyHandler;
        typedef HandlerComponentBase< MyHandler > MyHandler_Base;
        class MyHandler : MyHandler_Base
        {
            ...
        public:
            static OUString SAL_CALL getImplementationName_static(  ) throw (css::uno::RuntimeException);
            static css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_static(  ) throw (css::uno::RuntimeException);
        };
        </pre>
    */
    template < class HANDLER >
    class HandlerComponentBase : public PropertyHandlerComponent
    {
    protected:
        explicit HandlerComponentBase( const css::uno::Reference< css::uno::XComponentContext >& _rxContext )
            :PropertyHandlerComponent( _rxContext )
        {
        }

    protected:
        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception) override;
        static css::uno::Reference< css::uno::XInterface > SAL_CALL Create( const css::uno::Reference< css::uno::XComponentContext >& _rxContext );

    public:
        /** registers the implementation of HANDLER at the <type>PcrModule</type>
        */
        static void registerImplementation();
    };


    template < class HANDLER >
    OUString SAL_CALL HandlerComponentBase< HANDLER >::getImplementationName(  ) throw (css::uno::RuntimeException, std::exception)
    {
        return HANDLER::getImplementationName_static();
    }


    template < class HANDLER >
    css::uno::Sequence< OUString > SAL_CALL HandlerComponentBase< HANDLER >::getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception)
    {
        return HANDLER::getSupportedServiceNames_static();
    }


    template < class HANDLER >
    void HandlerComponentBase< HANDLER >::registerImplementation()
    {
        PcrModule::getInstance().registerImplementation(
            HANDLER::getImplementationName_static(),
            HANDLER::getSupportedServiceNames_static(),
            HANDLER::Create
        );
    }


    template < class HANDLER >
    css::uno::Reference< css::uno::XInterface > SAL_CALL HandlerComponentBase< HANDLER >::Create( const css::uno::Reference< css::uno::XComponentContext >& _rxContext )
    {
        return *( new HANDLER( _rxContext ) );
    }


} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
