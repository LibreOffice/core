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
#include "precompiled_shell.hxx"

#include <wininetbackend.hxx>
#include <com/sun/star/registry/XRegistryKey.hpp>

#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_
#include <cppuhelper/implementationentry.hxx>
#endif
#include <com/sun/star/uno/XComponentContext.hpp>

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;

//------------------------------------------------------------------------------

static uno::Reference<uno::XInterface> SAL_CALL createWinInetBackend(
    const uno::Reference<uno::XComponentContext>&){

    return * WinInetBackend::createInstance();
}

//------------------------------------------------------------------------------

static const cppu::ImplementationEntry kImplementations_entries[] =
{
    {
        createWinInetBackend,
        WinInetBackend::getBackendName,
        WinInetBackend::getBackendServiceNames,
        cppu::createSingleComponentFactory,
        NULL,
        0
    },
    { NULL }
} ;

//------------------------------------------------------------------------------

extern "C" void SAL_CALL component_getImplementationEnvironment(
    const sal_Char **aEnvTypeName, uno_Environment ** /*aEnvironment*/) {

    *aEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME ;
}

//------------------------------------------------------------------------------

extern "C" void *component_getFactory( const sal_Char *aImplementationName,
    void *aServiceManager,
    void *aRegistryKey) {

    return cppu::component_getFactoryHelper(
        aImplementationName,
        aServiceManager,
        aRegistryKey,
        kImplementations_entries) ;
}

//------------------------------------------------------------------------------

