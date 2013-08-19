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


#include "bootstrapservices.hxx"

#include "cppuhelper/factory.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "sal/types.h"
#include "uno/environment.h"
#include "uno/lbnames.h"

#include <stdio.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace osl;

using namespace stoc_bootstrap;

static struct ImplementationEntry g_entries[] =
{
    //servicemanager
    {
    OServiceManager_CreateInstance, smgr_getImplementationName,
    smgr_getSupportedServiceNames, createSingleComponentFactory,
    0, 0
    },
    {
    ORegistryServiceManager_CreateInstance, regsmgr_getImplementationName,
    regsmgr_getSupportedServiceNames, createSingleComponentFactory,
    0, 0
    },
    {
    OServiceManagerWrapper_CreateInstance, smgr_wrapper_getImplementationName,
    smgr_wrapper_getSupportedServiceNames, createSingleComponentFactory,
    0, 0
    },
    //security
    {
    ac_create, ac_getImplementationName,
    ac_getSupportedServiceNames, createSingleComponentFactory,
    0, 0
    },
    {
    filepolicy_create, filepolicy_getImplementationName,
    filepolicy_getSupportedServiceNames, createSingleComponentFactory,
    0, 0
    },
    //simpleregistry
    {
    SimpleRegistry_CreateInstance, simreg_getImplementationName,
    simreg_getSupportedServiceNames, createSingleComponentFactory,
    0, 0
    },
    //defaultregistry
    {
    NestedRegistry_CreateInstance, defreg_getImplementationName,
    defreg_getSupportedServiceNames, createSingleComponentFactory,
    0, 0
    },
    //implementationregistry
    {
    ImplementationRegistration_CreateInstance, impreg_getImplementationName,
    impreg_getSupportedServiceNames, createSingleComponentFactory,
    0, 0
    },
    //loader
    {
    DllComponentLoader_CreateInstance, loader_getImplementationName,
    loader_getSupportedServiceNames, createSingleComponentFactory,
    0, 0
    },
    //end
    { 0, 0, 0, 0, 0, 0 }
};

#define component_getFactory bootstrap_component_getFactory

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
