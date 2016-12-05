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

namespace animcore
{
    rtl_StandardModuleCount g_moduleCount = MODULE_COUNT_INIT;
}

using namespace animcore;

#define IMPLEMENTATION_ENTRY(N)\
{\
    createInstance_##N, getImplementationName_##N ,\
    getSupportedServiceNames_##N, createSingleComponentFactory ,\
    &g_moduleCount.modCnt , 0\
}\

static struct ImplementationEntry g_entries[] =
{
    IMPLEMENTATION_ENTRY( PAR ),
    IMPLEMENTATION_ENTRY( SEQ ),
    IMPLEMENTATION_ENTRY( ITERATE ),
    IMPLEMENTATION_ENTRY( ANIMATE ),
    IMPLEMENTATION_ENTRY( SET ),
    IMPLEMENTATION_ENTRY( ANIMATECOLOR ),
    IMPLEMENTATION_ENTRY( ANIMATEMOTION ),
    IMPLEMENTATION_ENTRY( ANIMATETRANSFORM ),
    IMPLEMENTATION_ENTRY( TRANSITIONFILTER ),
    IMPLEMENTATION_ENTRY( AUDIO ),
    IMPLEMENTATION_ENTRY( COMMAND ),
    IMPLEMENTATION_ENTRY( TargetPropertiesCreator ),
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
