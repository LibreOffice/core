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
#include <unotools/resmgr.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <svl/solar.hrc>
#include <tools/debug.hxx>
#include <rtl/strbuf.hxx>
#include <osl/diagnose.h>

namespace compmodule
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::registry;
    using namespace ::cppu;

    OUString ModuleRes(const char* pId)
    {
        return Translate::get(pId, Translate::Create("pcr"));
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

        auto it = std::find(s_pImplementationNames->begin(), s_pImplementationNames->end(), _rImplementationName);
        if (it != s_pImplementationNames->end())
        {
            sal_Int32 i = static_cast<sal_Int32>(std::distance(s_pImplementationNames->begin(), it));
            s_pImplementationNames->erase(it);
            s_pSupportedServices->erase(s_pSupportedServices->begin() + i);
            s_pCreationFunctionPointers->erase(s_pCreationFunctionPointers->begin() + i);
            s_pFactoryFunctionPointers->erase(s_pFactoryFunctionPointers->begin() + i);
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


        sal_Int32 nLen = s_pImplementationNames->size();

        for (sal_Int32 i=0; i<nLen; ++i)
        {
            if ((*s_pImplementationNames)[i] == _rImplementationName)
            {
                const FactoryInstantiation FactoryInstantiationFunction = (*s_pFactoryFunctionPointers)[i];

                Reference< XInterface > xReturn = FactoryInstantiationFunction( _rxServiceManager, _rImplementationName,
                    (*s_pCreationFunctionPointers)[i],
                    (*s_pSupportedServices)[i], nullptr);
                return xReturn;
            }
        }

        return nullptr;
    }


}   // namespace compmodule


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
