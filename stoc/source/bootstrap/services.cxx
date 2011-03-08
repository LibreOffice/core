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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_stoc.hxx"

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

extern rtl_StandardModuleCount g_moduleCount;

using namespace stoc_bootstrap;

static struct ImplementationEntry g_entries[] =
{
    //servicemanager
    {
    OServiceManager_CreateInstance, smgr_getImplementationName,
    smgr_getSupportedServiceNames, createSingleComponentFactory,
    &g_moduleCount.modCnt , 0
    },
    {
    ORegistryServiceManager_CreateInstance, regsmgr_getImplementationName,
    regsmgr_getSupportedServiceNames, createSingleComponentFactory,
    &g_moduleCount.modCnt , 0
    },
    {
    OServiceManagerWrapper_CreateInstance, smgr_wrapper_getImplementationName,
    smgr_wrapper_getSupportedServiceNames, createSingleComponentFactory,
    &g_moduleCount.modCnt , 0
    },
    //security
    {
    ac_create, ac_getImplementationName,
    ac_getSupportedServiceNames, createSingleComponentFactory,
    &g_moduleCount.modCnt, 0
    },
    {
    filepolicy_create, filepolicy_getImplementationName,
    filepolicy_getSupportedServiceNames, createSingleComponentFactory,
    &g_moduleCount.modCnt, 0
    },
    //simpleregistry
    {
    SimpleRegistry_CreateInstance, simreg_getImplementationName,
    simreg_getSupportedServiceNames, createSingleComponentFactory,
    &g_moduleCount.modCnt , 0
    },
    //defaultregistry
    {
    NestedRegistry_CreateInstance, defreg_getImplementationName,
    defreg_getSupportedServiceNames, createSingleComponentFactory,
    &g_moduleCount.modCnt , 0
    },
    //implementationregistry
    {
    ImplementationRegistration_CreateInstance, impreg_getImplementationName,
    impreg_getSupportedServiceNames, createSingleComponentFactory,
    &g_moduleCount.modCnt , 0
    },
    //loader
    {
    DllComponentLoader_CreateInstance, loader_getImplementationName,
    loader_getSupportedServiceNames, createSingleComponentFactory,
    &g_moduleCount.modCnt , 0
    },
    //registry_tdprovider
    {
    ProviderImpl_create, rdbtdp_getImplementationName,
    rdbtdp_getSupportedServiceNames, createSingleComponentFactory,
    &g_moduleCount.modCnt , 0
    },
    //tdmanager
    {
    ManagerImpl_create, tdmgr_getImplementationName,
    tdmgr_getSupportedServiceNames, createSingleComponentFactory,
    &g_moduleCount.modCnt , 0
    },
    //end
    { 0, 0, 0, 0, 0, 0 }
};

extern "C"
{

sal_Bool SAL_CALL component_canUnload( TimeValue *pTime )
{
    return g_moduleCount.canUnload( &g_moduleCount , pTime );
}

//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    return component_writeInfoHelper( pServiceManager, pRegistryKey, g_entries );
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
