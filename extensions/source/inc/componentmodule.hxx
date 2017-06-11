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

#ifndef INCLUDED_EXTENSIONS_SOURCE_INC_COMPONENTMODULE_HXX
#define INCLUDED_EXTENSIONS_SOURCE_INC_COMPONENTMODULE_HXX

/** you may find this file helpful if you implement a component (in it's own library) which can't use
    the usual infrastructure.<br/>
    More precise, you find helper classes to ease the use of resources and the registration of services.
*/

#include <osl/mutex.hxx>
#include <tools/simplerm.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <cppuhelper/factory.hxx>
#include <rtl/string.hxx>
#include <vector>

namespace compmodule
{

typedef css::uno::Reference< css::lang::XSingleServiceFactory > (SAL_CALL *FactoryInstantiation)
        (
            const css::uno::Reference< css::lang::XMultiServiceFactory >& _rServiceManager,
            const OUString & _rComponentName,
            ::cppu::ComponentInstantiation _pCreateFunction,
            const css::uno::Sequence< OUString > & _rServiceNames,
            rtl_ModuleCount*
        );

    class OModuleImpl;
    class OModule
    {
        friend class OModuleResourceClient;

    private:
        OModule() = delete; //TODO: get rid of this class

    protected:
        // resource administration
        static ::osl::Mutex     s_aMutex;       /// access safety
        static sal_Int32        s_nClients;     /// number of registered clients
        static OModuleImpl*     s_pImpl;        /// impl class. lives as long as at least one client for the module is registered

        // auto registration administration
        static  std::vector< OUString >*
            s_pImplementationNames;
        static  std::vector< css::uno::Sequence< OUString > >*
            s_pSupportedServices;
        static  std::vector< cppu::ComponentInstantiation >*
            s_pCreationFunctionPointers;
        static  std::vector< FactoryInstantiation >*
            s_pFactoryFunctionPointers;

    public:
        /// get the resource locale of the module
        static const std::locale& getResLocale();

        /** register a component implementing a service with the given data.
            @param  _rImplementationName
                        the implementation name of the component
            @param  _rServiceNames
                        the services the component supports
            @param  _pCreateFunction
                        a function for creating an instance of the component
            @param  _pFactoryFunction
                        a function for creating a factory for that component
            @see revokeComponent
        */
        static void registerComponent(
            const OUString& _rImplementationName,
            const css::uno::Sequence< OUString >& _rServiceNames,
            ::cppu::ComponentInstantiation _pCreateFunction,
            FactoryInstantiation _pFactoryFunction);

        /** revoke the registration for the specified component
            @param  _rImplementationName
                the implementation name of the component
        */
        static void revokeComponent(
            const OUString& _rImplementationName);

        /** creates a Factory for the component with the given implementation name.
            <p>Usually used from within component_getFactory.<p/>
            @param  _rxServiceManager
                        a pointer to an XMultiServiceFactory interface as got in component_getFactory
            @param  _pImplementationName
                        the implementation name of the component
            @return
                        the XInterface access to a factory for the component
        */
        static css::uno::Reference< css::uno::XInterface > getComponentFactory(
            const OUString& _rImplementationName,
            const css::uno::Reference< css::lang::XMultiServiceFactory >& _rxServiceManager
            );

    protected:
        /// register a client for the module
        static void registerClient();
        /// revoke a client for the module
        static void revokeClient();

    private:
        /** ensure that the impl class exists
            @precond m_aMutex is guarded when this method gets called
        */
        static void ensureImpl();
    };


    // base class for objects which uses any global module-specific resources
    class OModuleResourceClient
    {
    public:
        OModuleResourceClient()     { OModule::registerClient(); }
        ~OModuleResourceClient()    { OModule::revokeClient(); }
    };


    // specialized ResId, using the resource locale provided by the global module
    static inline OUString ModuleRes(const char* pId)
    {
        return Translate::get(pId, OModule::getResLocale());
    }

    template <class TYPE>
    class OMultiInstanceAutoRegistration
    {
    public:
        /** automatically registeres a multi instance component
            <p>Assumed that the template argument has the three methods
                <ul>
                    <li><code>static OUString getImplementationName_Static()</code><li/>
                    <li><code>static css::uno::Sequence< OUString > getSupportedServiceNames_Static()</code><li/>
                    <li><code>static css::uno::Reference< css::uno::XInterface >
                        Create(const css::uno::Reference< css::lang::XMultiServiceFactory >&)</code>
                        </li>
                <ul/>
            the instantiation of this object will automatically register the class via <method>OModule::registerComponent</method>.
            <p/>
            The factory creation function used is <code>::cppu::createSingleFactory</code>.
        */
        OMultiInstanceAutoRegistration();
        ~OMultiInstanceAutoRegistration();
    };

    template <class TYPE>
    OMultiInstanceAutoRegistration<TYPE>::OMultiInstanceAutoRegistration()
    {
        OModule::registerComponent(
            TYPE::getImplementationName_Static(),
            TYPE::getSupportedServiceNames_Static(),
            TYPE::Create,
            ::cppu::createSingleFactory
            );
    }

    template <class TYPE>
    OMultiInstanceAutoRegistration<TYPE>::~OMultiInstanceAutoRegistration()
    {
        OModule::revokeComponent(TYPE::getImplementationName_Static());
    }

}   // namespace compmodule


#endif // INCLUDED_EXTENSIONS_SOURCE_INC_COMPONENTMODULE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
