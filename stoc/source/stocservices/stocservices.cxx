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

#include "stocservices.hxx"

#include "cppuhelper/factory.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "sal/types.h"
#include "uno/environment.h"
#include "uno/lbnames.h"

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace osl;
using namespace rtl;
using namespace stoc_services;

rtl_StandardModuleCount g_moduleCount = MODULE_COUNT_INIT;

static struct ImplementationEntry g_entries[] =
{
    // typeconv
    {
    TypeConverter_Impl_CreateInstance, tcv_getImplementationName,
    tcv_getSupportedServiceNames, createSingleComponentFactory,
    &g_moduleCount.modCnt , 0
    },
    // uriproc
    {
    ExternalUriReferenceTranslator::create,
    ExternalUriReferenceTranslator::getImplementationName,
    ExternalUriReferenceTranslator::getSupportedServiceNames,
    createSingleComponentFactory, 0, 0
    },
    {
    UriReferenceFactory::create,
    UriReferenceFactory::getImplementationName,
    UriReferenceFactory::getSupportedServiceNames,
    createSingleComponentFactory, 0, 0
    },
    {
    UriSchemeParser_vndDOTsunDOTstarDOTexpand::create,
    UriSchemeParser_vndDOTsunDOTstarDOTexpand::getImplementationName,
    UriSchemeParser_vndDOTsunDOTstarDOTexpand::getSupportedServiceNames,
    createSingleComponentFactory, 0, 0
    },
    {
    UriSchemeParser_vndDOTsunDOTstarDOTscript::create,
    UriSchemeParser_vndDOTsunDOTstarDOTscript::getImplementationName,
    UriSchemeParser_vndDOTsunDOTstarDOTscript::getSupportedServiceNames,
    createSingleComponentFactory, 0, 0
    },
    {
    VndSunStarPkgUrlReferenceFactory::create,
    VndSunStarPkgUrlReferenceFactory::getImplementationName,
    VndSunStarPkgUrlReferenceFactory::getSupportedServiceNames,
    createSingleComponentFactory, 0, 0
    },
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
