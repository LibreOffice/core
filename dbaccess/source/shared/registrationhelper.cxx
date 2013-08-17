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

// be included in other cxx files

#ifndef _REGISTRATIONHELPER_CXX_INCLUDED_INDIRECTLY_
#error "don't build this file directly! use dbu_reghelper.cxx instead!"
#endif

using namespace ::com::sun::star;
using namespace ::comphelper;
using namespace ::cppu;

uno::Sequence< OUString >*                   OModuleRegistration::s_pImplementationNames = NULL;
uno::Sequence< uno::Sequence< OUString > >*  OModuleRegistration::s_pSupportedServices = NULL;
uno::Sequence< sal_Int64 >*                 OModuleRegistration::s_pCreationFunctionPointers = NULL;
uno::Sequence< sal_Int64 >*                 OModuleRegistration::s_pFactoryFunctionPointers = NULL;

void OModuleRegistration::registerComponent(
    const OUString& _rImplementationName,
    const uno::Sequence< OUString >& _rServiceNames,
    ComponentInstantiation _pCreateFunction,
    FactoryInstantiation _pFactoryFunction)
{
    if (!s_pImplementationNames)
    {
        OSL_ENSURE(!s_pSupportedServices && !s_pCreationFunctionPointers && !s_pFactoryFunctionPointers,
            "OModuleRegistration::registerComponent : inconsistent state (the pointers (1)) !");
        s_pImplementationNames = new uno::Sequence< OUString >;
        s_pSupportedServices = new uno::Sequence< uno::Sequence< OUString > >;
        s_pCreationFunctionPointers = new uno::Sequence< sal_Int64 >;
        s_pFactoryFunctionPointers = new uno::Sequence< sal_Int64 >;
    }
    OSL_ENSURE(s_pImplementationNames && s_pSupportedServices && s_pCreationFunctionPointers && s_pFactoryFunctionPointers,
        "OModuleRegistration::registerComponent : inconsistent state (the pointers (2)) !");

    OSL_ENSURE( (s_pImplementationNames->getLength() == s_pSupportedServices->getLength())
                &&  (s_pImplementationNames->getLength() == s_pCreationFunctionPointers->getLength())
                &&  (s_pImplementationNames->getLength() == s_pFactoryFunctionPointers->getLength()),
        "OModuleRegistration::registerComponent : inconsistent state !");

    sal_Int32 nOldLen = s_pImplementationNames->getLength();
    s_pImplementationNames->realloc(nOldLen + 1);
    s_pSupportedServices->realloc(nOldLen + 1);
    s_pCreationFunctionPointers->realloc(nOldLen + 1);
    s_pFactoryFunctionPointers->realloc(nOldLen + 1);

    s_pImplementationNames->getArray()[nOldLen] = _rImplementationName;
    s_pSupportedServices->getArray()[nOldLen] = _rServiceNames;
    s_pCreationFunctionPointers->getArray()[nOldLen] = reinterpret_cast<sal_Int64>(_pCreateFunction);
    s_pFactoryFunctionPointers->getArray()[nOldLen] = reinterpret_cast<sal_Int64>(_pFactoryFunction);
}

void OModuleRegistration::revokeComponent(const OUString& _rImplementationName)
{
    if (!s_pImplementationNames)
    {
        OSL_FAIL("OModuleRegistration::revokeComponent : have no class infos ! Are you sure called this method at the right time ?");
        return;
    }
    OSL_ENSURE(s_pImplementationNames && s_pSupportedServices && s_pCreationFunctionPointers && s_pFactoryFunctionPointers,
        "OModuleRegistration::revokeComponent : inconsistent state (the pointers) !");
    OSL_ENSURE( (s_pImplementationNames->getLength() == s_pSupportedServices->getLength())
                &&  (s_pImplementationNames->getLength() == s_pCreationFunctionPointers->getLength())
                &&  (s_pImplementationNames->getLength() == s_pFactoryFunctionPointers->getLength()),
        "OModuleRegistration::revokeComponent : inconsistent state !");

    sal_Int32 nLen = s_pImplementationNames->getLength();
    const OUString* pImplNames = s_pImplementationNames->getConstArray();
    for (sal_Int32 i=0; i<nLen; ++i, ++pImplNames)
    {
        if (pImplNames->equals(_rImplementationName))
        {
            removeElementAt(*s_pImplementationNames, i);
            removeElementAt(*s_pSupportedServices, i);
            removeElementAt(*s_pCreationFunctionPointers, i);
            removeElementAt(*s_pFactoryFunctionPointers, i);
            break;
        }
    }

    if (s_pImplementationNames->getLength() == 0)
    {
        delete s_pImplementationNames; s_pImplementationNames = NULL;
        delete s_pSupportedServices; s_pSupportedServices = NULL;
        delete s_pCreationFunctionPointers; s_pCreationFunctionPointers = NULL;
        delete s_pFactoryFunctionPointers; s_pFactoryFunctionPointers = NULL;
    }
}

uno::Reference< uno::XInterface > OModuleRegistration::getComponentFactory(
    const OUString& _rImplementationName,
    const uno::Reference< lang::XMultiServiceFactory >& _rxServiceManager)
{
    OSL_ENSURE(_rxServiceManager.is(), "OModuleRegistration::getComponentFactory : invalid argument (service manager) !");
    OSL_ENSURE(!_rImplementationName.isEmpty(), "OModuleRegistration::getComponentFactory : invalid argument (implementation name) !");

    if (!s_pImplementationNames)
    {
        OSL_FAIL("OModuleRegistration::getComponentFactory : have no class infos ! Are you sure called this method at the right time ?");
        return NULL;
    }
    OSL_ENSURE(s_pImplementationNames && s_pSupportedServices && s_pCreationFunctionPointers && s_pFactoryFunctionPointers,
        "OModuleRegistration::getComponentFactory : inconsistent state (the pointers) !");
    OSL_ENSURE( (s_pImplementationNames->getLength() == s_pSupportedServices->getLength())
                &&  (s_pImplementationNames->getLength() == s_pCreationFunctionPointers->getLength())
                &&  (s_pImplementationNames->getLength() == s_pFactoryFunctionPointers->getLength()),
        "OModuleRegistration::getComponentFactory : inconsistent state !");

    uno::Reference< uno::XInterface > xReturn;

    sal_Int32 nLen = s_pImplementationNames->getLength();
    const OUString* pImplName = s_pImplementationNames->getConstArray();
    const uno::Sequence< OUString >* pServices = s_pSupportedServices->getConstArray();
    const sal_Int64* pComponentFunction = s_pCreationFunctionPointers->getConstArray();
    const sal_Int64* pFactoryFunction = s_pFactoryFunctionPointers->getConstArray();

    for (sal_Int32 i=0; i<nLen; ++i, ++pImplName, ++pServices, ++pComponentFunction, ++pFactoryFunction)
    {
        if (pImplName->equals(_rImplementationName))
        {
            const FactoryInstantiation FactoryInstantiationFunction = reinterpret_cast<const FactoryInstantiation>(*pFactoryFunction);
            const ComponentInstantiation ComponentInstantiationFunction = reinterpret_cast<const ComponentInstantiation>(*pComponentFunction);

            xReturn = FactoryInstantiationFunction( _rxServiceManager, *pImplName, ComponentInstantiationFunction, *pServices, NULL);
            if (xReturn.is())
            {
                xReturn->acquire();
                return xReturn.get();
            }
        }
    }

    return NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
