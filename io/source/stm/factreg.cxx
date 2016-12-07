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
#include "precompiled_io.hxx"

#include <osl/diagnose.h>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>

#include <com/sun/star/registry/XRegistryKey.hpp>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

#include "factreg.hxx"

namespace io_stm
{
    rtl_StandardModuleCount g_moduleCount = MODULE_COUNT_INIT;
}

using namespace io_stm;

static struct ImplementationEntry g_entries[] =
{
    {
        OPipeImpl_CreateInstance, OPipeImpl_getImplementationName ,
        OPipeImpl_getSupportedServiceNames, createSingleComponentFactory ,
        &g_moduleCount.modCnt , 0
    },
    {
        OPumpImpl_CreateInstance, OPumpImpl_getImplementationName ,
        OPumpImpl_getSupportedServiceNames, createSingleComponentFactory ,
        &g_moduleCount.modCnt , 0
    },
    {
        ODataInputStream_CreateInstance, ODataInputStream_getImplementationName,
        ODataInputStream_getSupportedServiceNames, createSingleComponentFactory,
        &g_moduleCount.modCnt, 0
    },
    {
        ODataOutputStream_CreateInstance, ODataOutputStream_getImplementationName,
        ODataOutputStream_getSupportedServiceNames, createSingleComponentFactory,
        &g_moduleCount.modCnt, 0
    },
    {
        OObjectInputStream_CreateInstance, OObjectInputStream_getImplementationName,
        OObjectInputStream_getSupportedServiceNames, createSingleComponentFactory,
        &g_moduleCount.modCnt, 0
    },
    {
        OObjectOutputStream_CreateInstance, OObjectOutputStream_getImplementationName,
        OObjectOutputStream_getSupportedServiceNames, createSingleComponentFactory,
        &g_moduleCount.modCnt, 0
    },
    {
        OMarkableInputStream_CreateInstance, OMarkableInputStream_getImplementationName,
        OMarkableInputStream_getSupportedServiceNames, createSingleComponentFactory,
        &g_moduleCount.modCnt, 0
    },
    {
        OMarkableOutputStream_CreateInstance, OMarkableOutputStream_getImplementationName,
        OMarkableOutputStream_getSupportedServiceNames, createSingleComponentFactory,
        &g_moduleCount.modCnt, 0
    },
    { 0, 0, 0, 0, 0, 0 }

};

extern "C"
{

SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_canUnload( TimeValue *pTime )
{
    return g_moduleCount.canUnload( &g_moduleCount , pTime );
}

//==================================================================================================
SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}

}
