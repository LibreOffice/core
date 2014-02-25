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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYHANDLER_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYHANDLER_HXX

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
#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/uno3.hxx>

#include <memory>
#include <vector>

namespace com { namespace sun { namespace star {
    namespace inspection {
        struct LineDescriptor;
        class XPropertyControlFactory;
    }
} } }

class Window;
//........................................................................
namespace pcr
{
//........................................................................

    typedef sal_Int32   PropertyId;

    //====================================================================
    //= PropertyHandler
    //====================================================================
    class OPropertyInfoService;
    typedef ::cppu::WeakComponentImplHelper1    <   ::com::sun::star::inspection::XPropertyHandler
                                                >   PropertyHandler_Base;
    /** the base class for property handlers
    */
    class PropertyHandler : public PropertyHandler_Base
    {
    private:
        /// cache for getSupportedProperties
        mutable StlSyntaxSequence< ::com::sun::star::beans::Property >
                                    m_aSupportedProperties;
        mutable bool                m_bSupportedPropertiesAreKnown;

        /// helper which ensures that we can access resources as long as the instance lives
        PcrClient       m_aEnsureResAccess;

    private:
        /// the property listener which has been registered
        PropertyChangeListeners                                                         m_aPropertyListeners;

    protected:
        mutable ::osl::Mutex                                                            m_aMutex;
        /// the context in which the instance was created
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >    m_xContext;
        /// the component we're inspecting
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xComponent;
        /// info about our component's properties
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >   m_xComponentPropertyInfo;
        /// type converter, needed on various occasions
        ::com::sun::star::uno::Reference< ::com::sun::star::script::XTypeConverter >    m_xTypeConverter;
        /// access to property meta data
        ::std::auto_ptr< OPropertyInfoService >                                         m_pInfoService;

    protected:
        PropertyHandler(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext
        );
        ~PropertyHandler();

        // default implementations for XPropertyHandler
        virtual void SAL_CALL inspect( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxIntrospectee ) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::NullPointerException, std::exception);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL getSupportedProperties() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupersededProperties( ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getActuatingProperties( ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Any SAL_CALL convertToPropertyValue( const OUString& _rPropertyName, const ::com::sun::star::uno::Any& _rControlValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Any SAL_CALL convertToControlValue( const OUString& _rPropertyName, const ::com::sun::star::uno::Any& _rPropertyValue, const ::com::sun::star::uno::Type& _rControlValueType ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::beans::PropertyState  SAL_CALL getPropertyState( const OUString& _rPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::inspection::LineDescriptor SAL_CALL describePropertyLine( const OUString& _rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControlFactory >& _rxControlFactory ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::sal_Bool SAL_CALL isComposable( const OUString& _rPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::inspection::InteractiveSelectionResult SAL_CALL onInteractivePropertySelection( const OUString& _rPropertyName, sal_Bool _bPrimary, ::com::sun::star::uno::Any& _rData, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI >& _rxInspectorUI ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL actuatingPropertyChanged( const OUString& _rActuatingPropertyName, const ::com::sun::star::uno::Any& _rNewValue, const ::com::sun::star::uno::Any& _rOldValue, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI >& _rxInspectorUI, sal_Bool _bFirstTimeInit ) throw (::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL addPropertyChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& _rxListener ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL removePropertyChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& _rxListener ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual sal_Bool SAL_CALL suspend( sal_Bool _bSuspend ) throw (::com::sun::star::uno::RuntimeException, std::exception);

        // XComponent
        DECLARE_XCOMPONENT()
        virtual void SAL_CALL disposing();

        // own overridables
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >
                    SAL_CALL doDescribeSupportedProperties() const = 0;

        /// called when XPropertyHandler::inspect has been called, and we thus have a new component to inspect
        virtual void onNewComponent();

    protected:
        /** fires the change in a property value to our listener (if any)
            @see addPropertyChangeListener
        */
        void    firePropertyChange( const OUString& _rPropName, PropertyId _nPropId,
                    const ::com::sun::star::uno::Any& _rOldValue, const ::com::sun::star::uno::Any& _rNewValue ) SAL_THROW(());

        /** retrieves a window which can be used as parent for dialogs
        */
        Window* impl_getDefaultDialogParent_nothrow() const;

        /** retrieves the property id for a given property name
            @throw com::sun::star::beans::UnknownPropertyException
                if the property name is not known to our ->m_pInfoService
        */
        PropertyId impl_getPropertyId_throw( const OUString& _rPropertyName ) const;

        //-------------------------------------------------------------------------------
        // helper for implementing doDescribeSupportedProperties
        /** adds a description for the given string property to the given property vector
            Most probably to be called from within getSupportedProperties
        */
        inline void addStringPropertyDescription(
                    ::std::vector< ::com::sun::star::beans::Property >& _rProperties,
                    const OUString& _rPropertyName,
                    sal_Int16 _nAttribs = 0
                ) const;

        /** adds a description for the given int32 property to the given property vector
        */
        inline void addInt32PropertyDescription(
                    ::std::vector< ::com::sun::star::beans::Property >& _rProperties,
                    const OUString& _rPropertyName,
                    sal_Int16 _nAttribs = 0
                ) const;

        /** adds a description for the given int16 property to the given property vector
        */
        inline void addInt16PropertyDescription(
                    ::std::vector< ::com::sun::star::beans::Property >& _rProperties,
                    const OUString& _rPropertyName,
                    sal_Int16 _nAttribs = 0
                ) const;

        /** adds a description for the given double property to the given property vector
        */
        inline void addDoublePropertyDescription(
                    ::std::vector< ::com::sun::star::beans::Property >& _rProperties,
                    const OUString& _rPropertyName,
                    sal_Int16 _nAttribs = 0
                ) const;

        /** adds a description for the given date property to the given property vector
        */
        inline void addDatePropertyDescription(
                    ::std::vector< ::com::sun::star::beans::Property >& _rProperties,
                    const OUString& _rPropertyName,
                    sal_Int16 _nAttribs = 0
                ) const;

        /** adds a description for the given time property to the given property vector
        */
        inline void addTimePropertyDescription(
                    ::std::vector< ::com::sun::star::beans::Property >& _rProperties,
                    const OUString& _rPropertyName,
                    sal_Int16 _nAttribs = 0
                ) const;

        /** adds a description for the given DateTime property to the given property vector
        */
        inline void addDateTimePropertyDescription(
                    ::std::vector< ::com::sun::star::beans::Property >& _rProperties,
                    const OUString& _rPropertyName,
                    sal_Int16 _nAttribs = 0
                ) const;

        /// adds a Property, given by name only, to a given vector of Properties
        void implAddPropertyDescription(
                    ::std::vector< ::com::sun::star::beans::Property >& _rProperties,
                    const OUString& _rPropertyName,
                    const ::com::sun::star::uno::Type& _rType,
                    sal_Int16 _nAttribs = 0
                ) const;

        //-------------------------------------------------------------------------------
        // helper for accessing and maintaining meta data about our supported properties

        /** retrieves a property given by handle

            @return
                a pointer to the descriptor for the given properties, if it is one of our
                supported properties, <NULL/> else.

            @see doDescribeSupportedProperties
            @see impl_getPropertyFromId_throw
        */
        const ::com::sun::star::beans::Property*
                    impl_getPropertyFromId_nothrow( PropertyId _nPropId ) const;

        /** retrieves a property given by handle

            @throws UnknownPropertyException
                if the handler does not support a property with the given handle

            @seealso doDescribeSupportedProperties
            @see impl_getPropertyFromId_nothrow
        */
        const ::com::sun::star::beans::Property&
                    impl_getPropertyFromId_throw( PropertyId _nPropId ) const;

        /** determines whether a given property id is part of our supported properties
            @see getSupportedProperties
            @see doDescribeSupportedProperties
        */
        inline bool impl_isSupportedProperty_nothrow( PropertyId _nPropId ) const
        {
            return impl_getPropertyFromId_nothrow( _nPropId ) != NULL;
        }

        /** retrieves a property given by name

            @throws UnknownPropertyException
                if the handler does not support a property with the given name

            @seealso doDescribeSupportedProperties
        */
        const ::com::sun::star::beans::Property&
                    impl_getPropertyFromName_throw( const OUString& _rPropertyName ) const;

        /** get the name of a property given by handle
        */
        inline OUString
                    impl_getPropertyNameFromId_nothrow( PropertyId _nPropId ) const;

        /** returns the value of the ContextDocument property in the ComponentContext which was used to create
            this handler.
        */
        inline ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
                    impl_getContextDocument_nothrow() const
        {
            return ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >(
                m_xContext->getValueByName( "ContextDocument" ), ::com::sun::star::uno::UNO_QUERY );
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
        PropertyHandler();                                    // never implemented
        PropertyHandler( const PropertyHandler& );            // never implemented
        PropertyHandler& operator=( const PropertyHandler& ); // never implemented
    };

    //--------------------------------------------------------------------
    inline void PropertyHandler::addStringPropertyDescription( ::std::vector< ::com::sun::star::beans::Property >& _rProperties, const OUString& _rPropertyName, sal_Int16 _nAttribs ) const
    {
        implAddPropertyDescription( _rProperties, _rPropertyName, ::getCppuType( static_cast< OUString* >( NULL ) ), _nAttribs );
    }

    inline void PropertyHandler::addInt32PropertyDescription( ::std::vector< ::com::sun::star::beans::Property >& _rProperties, const OUString& _rPropertyName, sal_Int16 _nAttribs ) const
    {
        implAddPropertyDescription( _rProperties, _rPropertyName, ::getCppuType( static_cast< sal_Int32* >( NULL ) ), _nAttribs );
    }

    inline void PropertyHandler::addInt16PropertyDescription( ::std::vector< ::com::sun::star::beans::Property >& _rProperties, const OUString& _rPropertyName, sal_Int16 _nAttribs ) const
    {
        implAddPropertyDescription( _rProperties, _rPropertyName, ::getCppuType( static_cast< sal_Int16* >( NULL ) ), _nAttribs );
    }

    inline void PropertyHandler::addDoublePropertyDescription( ::std::vector< ::com::sun::star::beans::Property >& _rProperties, const OUString& _rPropertyName, sal_Int16 _nAttribs ) const
    {
        implAddPropertyDescription( _rProperties, _rPropertyName, ::getCppuType( static_cast< double* >( NULL ) ), _nAttribs );
    }

    inline void PropertyHandler::addDatePropertyDescription( ::std::vector< ::com::sun::star::beans::Property >& _rProperties, const OUString& _rPropertyName, sal_Int16 _nAttribs ) const
    {
        implAddPropertyDescription( _rProperties, _rPropertyName, ::getCppuType( static_cast< com::sun::star::util::Date* >( NULL ) ), _nAttribs );
    }

    inline void PropertyHandler::addTimePropertyDescription( ::std::vector< ::com::sun::star::beans::Property >& _rProperties, const OUString& _rPropertyName, sal_Int16 _nAttribs ) const
    {
        implAddPropertyDescription( _rProperties, _rPropertyName, ::getCppuType( static_cast< com::sun::star::util::Time* >( NULL ) ), _nAttribs );
    }

    inline void PropertyHandler::addDateTimePropertyDescription( ::std::vector< ::com::sun::star::beans::Property >& _rProperties, const OUString& _rPropertyName, sal_Int16 _nAttribs ) const
    {
        implAddPropertyDescription( _rProperties, _rPropertyName, ::getCppuType( static_cast< com::sun::star::util::DateTime* >( NULL ) ), _nAttribs );
    }

    inline OUString PropertyHandler::impl_getPropertyNameFromId_nothrow( PropertyId _nPropId ) const
    {
        const ::com::sun::star::beans::Property* pProp = impl_getPropertyFromId_nothrow( _nPropId );
        return pProp ? pProp->Name : OUString();
    }

    //====================================================================
    //= PropertyHandlerComponent
    //====================================================================
    typedef ::cppu::ImplHelper1 <   ::com::sun::star::lang::XServiceInfo
                                >   PropertyHandlerComponent_Base;
    /** PropertyHandler implementation which additionally supports XServiceInfo
    */
    class PropertyHandlerComponent  :public PropertyHandler
                                    ,public PropertyHandlerComponent_Base
    {
    protected:
        PropertyHandlerComponent(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext
        );

        DECLARE_XINTERFACE()
        DECLARE_XTYPEPROVIDER()

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) = 0;
        virtual ::sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) = 0;
    };

    //====================================================================
    //= HandlerComponentBase
    //====================================================================
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
            static OUString SAL_CALL getImplementationName_static(  ) throw (::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_static(  ) throw (::com::sun::star::uno::RuntimeException);
        };
        </pre>
    */
    template < class HANDLER >
    class HandlerComponentBase : public PropertyHandlerComponent
    {
    protected:
        HandlerComponentBase( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext )
            :PropertyHandlerComponent( _rxContext )
        {
        }

    protected:
        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL Create( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext );

    public:
        /** registers the implementation of HANDLER at the <type>PcrModule</type>
        */
        static void registerImplementation();
    };

    //--------------------------------------------------------------------
    template < class HANDLER >
    OUString SAL_CALL HandlerComponentBase< HANDLER >::getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception)
    {
        return HANDLER::getImplementationName_static();
    }

    //--------------------------------------------------------------------
    template < class HANDLER >
    ::com::sun::star::uno::Sequence< OUString > SAL_CALL HandlerComponentBase< HANDLER >::getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception)
    {
        return HANDLER::getSupportedServiceNames_static();
    }

    //--------------------------------------------------------------------
    template < class HANDLER >
    void HandlerComponentBase< HANDLER >::registerImplementation()
    {
        PcrModule::getInstance().registerImplementation(
            HANDLER::getImplementationName_static(),
            HANDLER::getSupportedServiceNames_static(),
            HANDLER::Create
        );
    }

    //--------------------------------------------------------------------
    template < class HANDLER >
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL HandlerComponentBase< HANDLER >::Create( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext )
    {
        return *( new HANDLER( _rxContext ) );
    }

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
