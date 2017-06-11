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

#include <memory>
#include "componentmodule.hxx"
#include <tools/simplerm.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <svl/solar.hrc>
#include <comphelper/sequence.hxx>
#include <tools/debug.hxx>
#include <rtl/strbuf.hxx>

#define ENTER_MOD_METHOD()  \
    ::osl::MutexGuard aGuard(s_aMutex); \
    ensureImpl()


namespace COMPMOD_NAMESPACE
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::registry;
    using namespace ::comphelper;
    using namespace ::cppu;

    // implementation for <type>OModule</type>. not threadsafe, has to be guarded by its owner
    class OModuleImpl
    {
        std::locale              m_aResLocale;
        bool                     m_bInitialized;

    public:
        /// ctor
        OModuleImpl();

        /// get the manager for the resources of the module
        const std::locale& getResLocale();
    };


    OModuleImpl::OModuleImpl()
        : m_bInitialized(false)
    {
    }

    const std::locale& OModuleImpl::getResLocale()
    {
        // note that this method is not threadsafe, which counts for the whole class !
        if (!m_bInitialized)
        {
            // create a manager with a fixed prefix
            m_aResLocale = Translate::Create("pcr", Application::GetSettings().GetUILanguageTag());
            m_bInitialized = true;
        }
        return m_aResLocale;
    }

    ::osl::Mutex    OModule::s_aMutex;
    sal_Int32       OModule::s_nClients = 0;
    OModuleImpl*    OModule::s_pImpl = nullptr;

    const std::locale& OModule::getResLocale()
    {
        ENTER_MOD_METHOD();
        return s_pImpl->getResLocale();
    }

    void OModule::registerClient()
    {
        ::osl::MutexGuard aGuard(s_aMutex);
        ++s_nClients;
    }

    void OModule::revokeClient()
    {
        ::osl::MutexGuard aGuard(s_aMutex);
        if (!--s_nClients && s_pImpl)
        {
            delete s_pImpl;
            s_pImpl = nullptr;
        }
    }

    void OModule::ensureImpl()
    {
        if (s_pImpl)
            return;
        s_pImpl = new OModuleImpl();
    }

    //- registration helper


    std::vector< OUString >*                OModule::s_pImplementationNames = nullptr;
    std::vector< Sequence< OUString > >*    OModule::s_pSupportedServices = nullptr;
    std::vector< ComponentInstantiation >*  OModule::s_pCreationFunctionPointers = nullptr;
    std::vector< FactoryInstantiation >*    OModule::s_pFactoryFunctionPointers = nullptr;


    void OModule::registerComponent(
        const OUString& _rImplementationName,
        const Sequence< OUString >& _rServiceNames,
        ComponentInstantiation _pCreateFunction,
        FactoryInstantiation _pFactoryFunction)
    {
        if (!s_pImplementationNames)
        {
            OSL_ENSURE(!s_pSupportedServices && !s_pCreationFunctionPointers && !s_pFactoryFunctionPointers,
                "OModule::registerComponent : inconsistent state (the pointers (1)) !");
            s_pImplementationNames = new std::vector< OUString >;
            s_pSupportedServices = new std::vector< Sequence< OUString > >;
            s_pCreationFunctionPointers = new std::vector< ComponentInstantiation >;
            s_pFactoryFunctionPointers = new std::vector< FactoryInstantiation >;
        }
        OSL_ENSURE(s_pImplementationNames && s_pSupportedServices && s_pCreationFunctionPointers && s_pFactoryFunctionPointers,
            "OModule::registerComponent : inconsistent state (the pointers (2)) !");

        OSL_ENSURE( (s_pImplementationNames->size() == s_pSupportedServices->size())
                    &&  (s_pImplementationNames->size() == s_pCreationFunctionPointers->size())
                    &&  (s_pImplementationNames->size() == s_pFactoryFunctionPointers->size()),
            "OModule::registerComponent : inconsistent state !");

        s_pImplementationNames->push_back(_rImplementationName);
        s_pSupportedServices->push_back(_rServiceNames);
        s_pCreationFunctionPointers->push_back(_pCreateFunction);
        s_pFactoryFunctionPointers->push_back(_pFactoryFunction);
    }


    void OModule::revokeComponent(const OUString& _rImplementationName)
    {
        if (!s_pImplementationNames)
        {
            OSL_FAIL("OModule::revokeComponent : have no class infos ! Are you sure called this method at the right time ?");
            return;
        }
        OSL_ENSURE(s_pImplementationNames && s_pSupportedServices && s_pCreationFunctionPointers && s_pFactoryFunctionPointers,
            "OModule::revokeComponent : inconsistent state (the pointers) !");
        OSL_ENSURE( (s_pImplementationNames->size() == s_pSupportedServices->size())
                    &&  (s_pImplementationNames->size() == s_pCreationFunctionPointers->size())
                    &&  (s_pImplementationNames->size() == s_pFactoryFunctionPointers->size()),
            "OModule::revokeComponent : inconsistent state !");

        sal_Int32 nLen = s_pImplementationNames->size();
        for (sal_Int32 i=0; i<nLen; ++i)
        {
            if ((*s_pImplementationNames)[i] == _rImplementationName)
            {
                s_pImplementationNames->erase(s_pImplementationNames->begin() + i);
                s_pSupportedServices->erase(s_pSupportedServices->begin() + i);
                s_pCreationFunctionPointers->erase(s_pCreationFunctionPointers->begin() + i);
                s_pFactoryFunctionPointers->erase(s_pFactoryFunctionPointers->begin() + i);
                break;
            }
        }

        if (s_pImplementationNames->empty())
        {
            delete s_pImplementationNames; s_pImplementationNames = nullptr;
            delete s_pSupportedServices; s_pSupportedServices = nullptr;
            delete s_pCreationFunctionPointers; s_pCreationFunctionPointers = nullptr;
            delete s_pFactoryFunctionPointers; s_pFactoryFunctionPointers = nullptr;
        }
    }


    Reference< XInterface > OModule::getComponentFactory(
        const OUString& _rImplementationName,
        const Reference< XMultiServiceFactory >& _rxServiceManager)
    {
        OSL_ENSURE(_rxServiceManager.is(), "OModule::getComponentFactory : invalid argument (service manager) !");
        OSL_ENSURE(!_rImplementationName.isEmpty(), "OModule::getComponentFactory : invalid argument (implementation name) !");

        if (!s_pImplementationNames)
        {
            OSL_FAIL("OModule::getComponentFactory : have no class infos ! Are you sure called this method at the right time ?");
            return nullptr;
        }
        OSL_ENSURE(s_pImplementationNames && s_pSupportedServices && s_pCreationFunctionPointers && s_pFactoryFunctionPointers,
            "OModule::getComponentFactory : inconsistent state (the pointers) !");
        OSL_ENSURE( (s_pImplementationNames->size() == s_pSupportedServices->size())
                    &&  (s_pImplementationNames->size() == s_pCreationFunctionPointers->size())
                    &&  (s_pImplementationNames->size() == s_pFactoryFunctionPointers->size()),
            "OModule::getComponentFactory : inconsistent state !");


        Reference< XInterface > xReturn;


        sal_Int32 nLen = s_pImplementationNames->size();

        for (sal_Int32 i=0; i<nLen; ++i)
        {
            if ((*s_pImplementationNames)[i] == _rImplementationName)
            {
                const FactoryInstantiation FactoryInstantiationFunction = (*s_pFactoryFunctionPointers)[i];

                xReturn = FactoryInstantiationFunction( _rxServiceManager, _rImplementationName,
                    (*s_pCreationFunctionPointers)[i],
                    (*s_pSupportedServices)[i], nullptr);
                if (xReturn.is())
                {
                    xReturn->acquire();
                    return xReturn.get();
                }
            }
        }

        return nullptr;
    }


}   // namespace COMPMOD_NAMESPACE


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
