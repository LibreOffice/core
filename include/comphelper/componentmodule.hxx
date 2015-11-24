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
#ifndef INCLUDED_COMPHELPER_COMPONENTMODULE_HXX
#define INCLUDED_COMPHELPER_COMPONENTMODULE_HXX

#include <comphelper/comphelperdllapi.h>

#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <cppuhelper/factory.hxx>

#include <osl/mutex.hxx>

#include <rtl/string.hxx>
#include <rtl/instance.hxx>


namespace comphelper
{


    /** factory factory declaration
    */
    typedef css::uno::Reference< css::lang::XSingleComponentFactory > (SAL_CALL *FactoryInstantiation)
    (
        ::cppu::ComponentFactoryFunc          _pFactoryFunc,
        OUString const&                       _rComponentName,
        css::uno::Sequence< OUString > const & _rServiceNames,
        rtl_ModuleCount*
    );


    //= ComponentDescription

    struct COMPHELPER_DLLPUBLIC ComponentDescription
    {
        /// the implementation name of the component
        OUString                                     sImplementationName;
        /// the services supported by the component implementation
        css::uno::Sequence< OUString >               aSupportedServices;
        /// the function to create an instance of the component
        ::cppu::ComponentFactoryFunc                 pComponentCreationFunc;
        /// the function to create a factory for the component (usually <code>::cppu::createSingleComponentFactory</code>)
        FactoryInstantiation                         pFactoryCreationFunc;

        ComponentDescription()
            :sImplementationName()
            ,aSupportedServices()
            ,pComponentCreationFunc( nullptr )
            ,pFactoryCreationFunc( nullptr )
        {
        }

        ComponentDescription(
                const OUString& _rImplementationName,
                const css::uno::Sequence< OUString >& _rSupportedServices,
                ::cppu::ComponentFactoryFunc _pComponentCreationFunc,
                FactoryInstantiation _pFactoryCreationFunc
            )
            :sImplementationName( _rImplementationName )
            ,aSupportedServices( _rSupportedServices )
            ,pComponentCreationFunc( _pComponentCreationFunc )
            ,pFactoryCreationFunc( _pFactoryCreationFunc )
        {
        }
    };


    //= OModule

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
            const OUString& _rImplementationName,
            const css::uno::Sequence< OUString >& _rServiceNames,
            ::cppu::ComponentFactoryFunc _pCreateFunction,
            FactoryInstantiation _pFactoryFunction = ::cppu::createSingleComponentFactory );

        /** registers a component given by ComponentDescription
        */
        void registerImplementation( const ComponentDescription& _rComp );

        /** creates a Factory for the component with the given implementation name.
            <p>Usually used from within component_getFactory.<p/>
            @param _pImplementationName
                the implementation name of the component
            @return
                the XInterface access to a factory for the component
        */
        css::uno::Reference< css::uno::XInterface > getComponentFactory(
            const OUString& _rImplementationName );

        /** version of getComponentFactory which directly takes the char argument you got in your component_getFactory call
        */
        void* getComponentFactory( const sal_Char* _pImplementationName );

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
        OModule( const OModule& ) = delete;
        OModule& operator=( const OModule& ) = delete;
    };


    //= OModuleClient

    /** base class for objects which uses any global module-specific resources
    */
    class COMPHELPER_DLLPUBLIC OModuleClient
    {
    protected:
        OModule&    m_rModule;

    public:
        OModuleClient( OModule& _rModule ) :m_rModule( _rModule )   { m_rModule.registerClient( OModule::ClientAccess() ); }
        ~OModuleClient()                                            { m_rModule.revokeClient( OModule::ClientAccess() ); }
    };


    //= OAutoRegistration

    template <class TYPE>
    class OAutoRegistration
    {
    public:
        /** automatically provides all component information to an OModule instance
            <p>Assumed that the template argument has the three methods
                <ul>
                    <li><code>static OUString getImplementationName_static()</code><li/>
                    <li><code>static css::uno::Sequence< OUString > getSupportedServiceNames_static()</code><li/>
                    <li><code>static css::uno::Reference< css::uno::XInterface >
                        Create(const css::uno::Reference< css::lang::XMultiServiceFactory >&)</code>
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


    //= OSingletonRegistration

    template <class TYPE>
    class OSingletonRegistration
    {
    public:
        /** automatically provides all component information to an OModule instance,
            for a singleton component

            <p>Assumed that the template argument has the three methods
                <ul>
                    <li><code>static OUString getImplementationName_static()</code><li/>
                    <li><code>static css::uno::Sequence< OUString > getSupportedServiceNames_static()</code><li/>
                    <li><code>static OUString getSingletonName_static()</code></li>
                    <li><code>static css::uno::Reference< css::uno::XInterface >
                        Create(const css::uno::Reference< css::lang::XMultiServiceFactory >&)</code>
                        </li>
                <ul/>
            the instantiation of this object will automatically register the class via <member>OModule::registerImplementation</member>.
            </p>
        */
        OSingletonRegistration( OModule& _rModule );
    };

    template <class TYPE>

    OSingletonRegistration<TYPE>::OSingletonRegistration( OModule& _rModule )
    {
        _rModule.registerImplementation( ComponentDescription(
            TYPE::getImplementationName_static(),
            TYPE::getSupportedServiceNames_static(),
            &TYPE::Create,
            &::cppu::createSingleComponentFactory
        ) );
    }


} // namespace comphelper


#endif // INCLUDED_COMPHELPER_COMPONENTMODULE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
