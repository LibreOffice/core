/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
using namespace rtl;

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
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}

}
