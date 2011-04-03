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
#ifndef COMPHELPER_INC_COMPHELPER_COMPONENTMODULE_HXX
#define COMPHELPER_INC_COMPHELPER_COMPONENTMODULE_HXX

#include <comphelper/comphelperdllapi.h>
#include <comphelper/legacysingletonfactory.hxx>

/** === begin UNO includes === **/
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/uno/Sequence.hxx>
/** === end UNO includes === **/

#include <cppuhelper/factory.hxx>

#include <osl/mutex.hxx>

#include <rtl/string.hxx>
#include <rtl/instance.hxx>

//........................................................................
namespace comphelper
{
//........................................................................

    /** factory factory declaration
    */
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleComponentFactory > (SAL_CALL *FactoryInstantiation)
    (
        ::cppu::ComponentFactoryFunc _pFactoryFunc,
        ::rtl::OUString const& _rComponentName,
        ::com::sun::star::uno::Sequence< ::rtl::OUString > const & _rServiceNames,
        rtl_ModuleCount* _pModuleCounter
    ) SAL_THROW(());

    //=========================================================================
    //= ComponentDescription
    //=========================================================================
    struct COMPHELPER_DLLPUBLIC ComponentDescription
    {
        /// the implementation name of the component
        ::rtl::OUString                                     sImplementationName;
        /// the services supported by the component implementation
        ::com::sun::star::uno::Sequence< ::rtl::OUString >  aSupportedServices;
        /** the name under which the component implementation should be registered as singleton,
            or empty if the component does not implement a singleton.
        */
        ::rtl::OUString                                     sSingletonName;
        /// the function to create an instance of the component
        ::cppu::ComponentFactoryFunc                        pComponentCreationFunc;
        /// the function to create a factory for the component (usually <code>::cppu::createSingleComponentFactory</code>)
        FactoryInstantiation                                pFactoryCreationFunc;

        ComponentDescription()
            :sImplementationName()
            ,aSupportedServices()
            ,sSingletonName()
            ,pComponentCreationFunc( NULL )
            ,pFactoryCreationFunc( NULL )
        {
        }

        ComponentDescription(
                const ::rtl::OUString& _rImplementationName,
                const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rSupportedServices,
                const ::rtl::OUString& _rSingletonName,
                ::cppu::ComponentFactoryFunc _pComponentCreationFunc,
                FactoryInstantiation _pFactoryCreationFunc
            )
            :sImplementationName( _rImplementationName )
            ,aSupportedServices( _rSupportedServices )
            ,sSingletonName( _rSingletonName )
            ,pComponentCreationFunc( _pComponentCreationFunc )
            ,pFactoryCreationFunc( _pFactoryCreationFunc )
        {
        }
    };

    //=========================================================================
    //= OModule
    //=========================================================================
    class OModuleImpl;
    class COMPHELPER_DLLPUBLIC OModule
    {
    private:
        oslInterlockedCount     m_nClients;     /// number of registered clients
        OModuleImpl*            m_pImpl;        /// impl class. lives as long as at least one client for the module is registered

    protected:
        mutable ::osl::Mutex    m_aMutex;       /// access safety

    public:
        OModule();

        virtual ~OModule();

        /** register a component implementing a service with the given data.
            @param _rImplementationName
                the implementation name of the component
            @param _rServiceNames
                the services the component supports
            @param _pCreateFunction
                a function for creating an instance of the component
            @param _pFactoryFunction
                a function for creating a factory for that component
        */
        void registerImplementation(
            const ::rtl::OUString& _rImplementationName,
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rServiceNames,
            ::cppu::ComponentFactoryFunc _pCreateFunction,
            FactoryInstantiation _pFactoryFunction = ::cppu::createSingleComponentFactory );

        /** registers a component given by <type>ComponentDescription</type>
        */
        void registerImplementation( const ComponentDescription& _rComp );

        /** creates a Factory for the component with the given implementation name.
            <p>Usually used from within component_getFactory.<p/>
            @param _rxServiceManager
                a pointer to an XMultiServiceFactory interface as got in component_getFactory
            @param _pImplementationName
                the implementation name of the component
            @return
                the XInterface access to a factory for the component
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getComponentFactory(
            const ::rtl::OUString& _rImplementationName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxServiceManager
        );

        /** version of getComponentFactory which directly takes the arguments you got in your component_getFactory call
        */
        void* getComponentFactory(
            const sal_Char* _pImplementationName, void* _pServiceManager, void* _pRegistryKey
        );

    public:
        class ClientAccess { friend class OModuleClient; private: ClientAccess() { } };
        /// register a client for the module
        void registerClient( ClientAccess );
        /// revoke a client for the module
        void revokeClient( ClientAccess );

    protected:
        /** called when the first client has been registered
            @precond
                <member>m_aMutex</member> is locked
        */
        virtual void onFirstClient();

        /** called when the last client has been revoked
            @precond
                <member>m_aMutex</member> is locked
        */
        virtual void onLastClient();

    private:
        OModule( const OModule& );              // never implemented
        OModule& operator=( const OModule& );   // never implemented
    };

    //=========================================================================
    //= OModuleClient
    //=========================================================================
    /** base class for objects which uses any global module-specific ressources
    */
    class COMPHELPER_DLLPUBLIC OModuleClient
    {
    protected:
        OModule&    m_rModule;

    public:
        OModuleClient( OModule& _rModule ) :m_rModule( _rModule )   { m_rModule.registerClient( OModule::ClientAccess() ); }
        ~OModuleClient()                                            { m_rModule.revokeClient( OModule::ClientAccess() ); }
    };

    //==========================================================================
    //= OAutoRegistration
    //==========================================================================
    template <class TYPE>
    class OAutoRegistration
    {
    public:
        /** automatically provides all component information to an OModule instance
            <p>Assumed that the template argument has the three methods
                <ul>
                    <li><code>static ::rtl::OUString getImplementationName_static()</code><li/>
                    <li><code>static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static()</code><li/>
                    <li><code>static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                        Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&)</code>
                        </li>
                <ul/>
            the instantiation of this object will automatically register the class via <member>OModule::registerImplementation</member>.
            <p/>
            The factory creation function used is <code>::cppu::createSingleComponentFactory</code>.
        */
        OAutoRegistration( OModule& _rModule );
    };

    template <class TYPE>
    OAutoRegistration<TYPE>::OAutoRegistration( OModule& _rModule )
    {
        _rModule.registerImplementation(
            TYPE::getImplementationName_static(),
            TYPE::getSupportedServiceNames_static(),
            TYPE::Create
        );
    }

    //==========================================================================
    //= OSingletonRegistration
    //==========================================================================
    template <class TYPE>
    class OSingletonRegistration
    {
    public:
        /** automatically provides all component information to an OModule instance,
            for a singleton component

            <p>Assumed that the template argument has the three methods
                <ul>
                    <li><code>static ::rtl::OUString getImplementationName_static()</code><li/>
                    <li><code>static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static()</code><li/>
                    <li><code>static ::rtl::OUString getSingletonName_static()</code></li>
                    <li><code>static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                        Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&)</code>
                        </li>
                <ul/>
            the instantiation of this object will automatically register the class via <member>OModule::registerImplementation</member>.
            </p>
        */
        OSingletonRegistration( OModule& _rModule );
    };

    template <class TYPE>
    //--------------------------------------------------------------------------
    OSingletonRegistration<TYPE>::OSingletonRegistration( OModule& _rModule )
    {
        _rModule.registerImplementation( ComponentDescription(
            TYPE::getImplementationName_static(),
            TYPE::getSupportedServiceNames_static(),
            TYPE::getSingletonName_static(),
            &TYPE::Create,
            &::cppu::createSingleComponentFactory
        ) );
    }

    //==========================================================================
    //= OLegacySingletonRegistration
    //==========================================================================
    template <class TYPE>
    class OLegacySingletonRegistration
    {
    public:
        OLegacySingletonRegistration( OModule& _rModule );
    };

    //--------------------------------------------------------------------------
    template <class TYPE>
    OLegacySingletonRegistration<TYPE>::OLegacySingletonRegistration( OModule& _rModule )
    {
        _rModule.registerImplementation( ComponentDescription(
            TYPE::getImplementationName_static(),
            TYPE::getSupportedServiceNames_static(),
            ::rtl::OUString(),
            &TYPE::Create,
            &::comphelper::createLegacySingletonFactory
        ) );
    }

    //==========================================================================
    //= helpers
    //==========================================================================

    //==========================================================================
    // declaring a OModule for a component library

#define DECLARE_COMPONENT_MODULE( ModuleClass, ClientClass ) \
    /* -------------------------------------------------------------------- */ \
    class ModuleClass : public ::comphelper::OModule \
    { \
        friend struct ModuleClass##Creator; \
        typedef ::comphelper::OModule BaseClass; \
    \
    public: \
        static ModuleClass& getInstance(); \
    \
    private: \
        ModuleClass(); \
    }; \
    \
    /* -------------------------------------------------------------------- */ \
    class ClientClass : public ::comphelper::OModuleClient \
    { \
    private: \
        typedef ::comphelper::OModuleClient BaseClass; \
    \
    public: \
        ClientClass() : BaseClass( ModuleClass::getInstance() ) \
        { \
        } \
    }; \
    \
    /* -------------------------------------------------------------------- */ \
    template < class TYPE > \
    class OAutoRegistration : public ::comphelper::OAutoRegistration< TYPE > \
    { \
    private: \
        typedef ::comphelper::OAutoRegistration< TYPE >    BaseClass; \
    \
    public: \
        OAutoRegistration() : BaseClass( ModuleClass::getInstance() ) \
        { \
        } \
    }; \
    /* -------------------------------------------------------------------- */ \
    template < class TYPE > \
    class OSingletonRegistration : public ::comphelper::OSingletonRegistration< TYPE > \
    { \
    private: \
        typedef ::comphelper::OSingletonRegistration< TYPE >    BaseClass; \
    \
    public: \
        OSingletonRegistration() : BaseClass( ModuleClass::getInstance() ) \
        { \
        } \
    }; \
    /* -------------------------------------------------------------------- */ \
    template < class TYPE > \
    class OLegacySingletonRegistration : public ::comphelper::OLegacySingletonRegistration< TYPE > \
    { \
    private: \
        typedef ::comphelper::OLegacySingletonRegistration< TYPE >  BaseClass; \
    \
    public: \
        OLegacySingletonRegistration() : BaseClass( ModuleClass::getInstance() ) \
        { \
        } \
    }; \

    //==========================================================================
    //= implementing a OModule for a component library

#define IMPLEMENT_COMPONENT_MODULE( ModuleClass ) \
    struct ModuleClass##Creator \
    { \
        ModuleClass m_aModuleClass; \
    }; \
    namespace \
    { \
        class the##ModuleClass##Instance : public rtl::Static<ModuleClass##Creator, the##ModuleClass##Instance> {}; \
    } \
    \
    ModuleClass::ModuleClass() \
        :BaseClass() \
    { \
    } \
    \
    ModuleClass& ModuleClass::getInstance() \
    { \
        return the##ModuleClass##Instance::get().m_aModuleClass; \
    } \

    //==========================================================================
    //= implementing the API of a component library (component_*)

#define IMPLEMENT_COMPONENT_LIBRARY_API( module_class, initializer_function )   \
    extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL \
    component_getImplementationEnvironment(    \
        const sal_Char **ppEnvTypeName, uno_Environment ** /*ppEnv*/ )  \
    {   \
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;    \
    }   \
    extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL component_getFactory( \
        const sal_Char* pImplementationName, void* pServiceManager, void* pRegistryKey ) \
    { \
        initializer_function(); \
        return module_class::getInstance().getComponentFactory( pImplementationName, pServiceManager, pRegistryKey );   \
    }

//........................................................................
} // namespace comphelper
//........................................................................

#endif // COMPHELPER_INC_COMPHELPER_COMPONENTMODULE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
