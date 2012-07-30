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

#ifndef _EXTENSIONS_COMPONENT_MODULE_HXX_
#define _EXTENSIONS_COMPONENT_MODULE_HXX_

/** you may find this file helpfull if you implement a component (in it's own library) which can't use
    the usual infrastructure.<br/>
    More precise, you find helper classes to ease the use of resources and the registration of services.
    <p>
    You need to define a preprocessor variable COMPMOD_NAMESPACE in order to use this file. Set it to a string
    which should be used as namespace for the classes defined herein.</p>
*/

#include <osl/mutex.hxx>
#include <tools/resid.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <cppuhelper/factory.hxx>
#include <rtl/string.hxx>

class ResMgr;

//.........................................................................
namespace COMPMOD_NAMESPACE
{
//.........................................................................

typedef ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > (SAL_CALL *FactoryInstantiation)
        (
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rServiceManager,
            const ::rtl::OUString & _rComponentName,
            ::cppu::ComponentInstantiation _pCreateFunction,
            const ::com::sun::star::uno::Sequence< ::rtl::OUString > & _rServiceNames,
            rtl_ModuleCount* _pModuleCounter
        );

    //=========================================================================
    //= OModule
    //=========================================================================
    class OModuleImpl;
    class OModule
    {
        friend class OModuleResourceClient;

    private:
        OModule();
            // not implemented. OModule is a static class

    protected:
        // resource administration
        static ::osl::Mutex     s_aMutex;       /// access safety
        static sal_Int32        s_nClients;     /// number of registered clients
        static OModuleImpl*     s_pImpl;        /// impl class. lives as long as at least one client for the module is registered
        static ::rtl::OString   s_sResPrefix;

        // auto registration administration
        static  ::com::sun::star::uno::Sequence< ::rtl::OUString >*
            s_pImplementationNames;
        static  ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::rtl::OUString > >*
            s_pSupportedServices;
        static  ::com::sun::star::uno::Sequence< sal_Int64 >*
            s_pCreationFunctionPointers;
        static  ::com::sun::star::uno::Sequence< sal_Int64 >*
            s_pFactoryFunctionPointers;

    public:
        // cna be set as long as no resource has been accessed ...
        static void     setResourceFilePrefix(const ::rtl::OString& _rPrefix);

        /// get the vcl res manager of the module
        static ResMgr*  getResManager();

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
            const ::rtl::OUString& _rImplementationName,
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rServiceNames,
            ::cppu::ComponentInstantiation _pCreateFunction,
            FactoryInstantiation _pFactoryFunction);

        /** revoke the registration for the specified component
            @param  _rImplementationName
                the implementation name of the component
        */
        static void revokeComponent(
            const ::rtl::OUString& _rImplementationName);

        /** creates a Factory for the component with the given implementation name.
            <p>Usually used from within component_getFactory.<p/>
            @param  _rxServiceManager
                        a pointer to an XMultiServiceFactory interface as got in component_getFactory
            @param  _pImplementationName
                        the implementation name of the component
            @return
                        the XInterface access to a factory for the component
        */
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getComponentFactory(
            const ::rtl::OUString& _rImplementationName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxServiceManager
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

    //=========================================================================
    //= OModuleResourceClient
    //=========================================================================
    /** base class for objects which uses any global module-specific resources
    */
    class OModuleResourceClient
    {
    public:
        OModuleResourceClient()     { OModule::registerClient(); }
        ~OModuleResourceClient()    { OModule::revokeClient(); }
    };

    //=========================================================================
    //= ModuleRes
    //=========================================================================
    /** specialized ResId, using the resource manager provided by the global module
    */
    class ModuleRes : public ::ResId
    {
    public:
        ModuleRes(sal_uInt16 _nId) : ResId(_nId, *OModule::getResManager()) { }
    };

    //==========================================================================
    //= OMultiInstanceAutoRegistration
    //==========================================================================
    template <class TYPE>
    class OMultiInstanceAutoRegistration
    {
    public:
        /** automatically registeres a multi instance component
            <p>Assumed that the template argument has the three methods
                <ul>
                    <li><code>static ::rtl::OUString getImplementationName_Static()</code><li/>
                    <li><code>static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static()</code><li/>
                    <li><code>static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                        Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&)</code>
                        </li>
                <ul/>
            the instantiation of this object will automatically register the class via <method>OModule::registerComponent</method>.
            <p/>
            The factory creation function used is <code>::cppu::createSingleFactory</code>.
            @see OOneInstanceAutoRegistration
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

    //==========================================================================
    //= OOneInstanceAutoRegistration
    //==========================================================================
    template <class TYPE>
    class OOneInstanceAutoRegistration
    {
    public:
        /** automatically registeres a single instance component
            <p>Assumed that the template argument has the three methods
                <ul>
                    <li><code>static ::rtl::OUString getImplementationName_Static()</code><li/>
                    <li><code>static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static()</code><li/>
                    <li><code>static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                        Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&)</code>
                        </li>
                <ul/>
            the instantiation of this object will automatically register the class via <method>OModule::registerComponent</method>.
            <p/>
            The factory creation function used is <code>::cppu::createOneInstanceFactory</code>.
            @see OOneInstanceAutoRegistration
        */
        OOneInstanceAutoRegistration();
        ~OOneInstanceAutoRegistration();
    };

    template <class TYPE>
    OOneInstanceAutoRegistration<TYPE>::OOneInstanceAutoRegistration()
    {
        OModule::registerComponent(
            TYPE::getImplementationName_Static(),
            TYPE::getSupportedServiceNames_Static(),
            TYPE::Create,
            ::cppu::createOneInstanceFactory
            );
    }

    template <class TYPE>
    OOneInstanceAutoRegistration<TYPE>::~OOneInstanceAutoRegistration()
    {
        OModule::revokeComponent(TYPE::getImplementationName_Static());
    }

//.........................................................................
}   // namespace COMPMOD_NAMESPACE
//.........................................................................

#endif // _EXTENSIONS_COMPONENT_MODULE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
