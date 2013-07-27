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

#include "MyProtocolHandler.h"

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>

namespace framework
{
    namespace complex_toolbar_controls
    {
        static ::cppu::ImplementationEntry const s_impl_entries[] =
        {
            {
                MyProtocolHandler_createInstance,
                MyProtocolHandler_getImplementationName,
                MyProtocolHandler_getSupportedServiceNames,
                ::cppu::createSingleComponentFactory,
                0,
                0
            },
            { 0, 0, 0, 0, 0, 0 }
        };
    }
}

extern "C"
{
    SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(
        const sal_Char **ppEnvTypeName, uno_Environment ** )
    {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    SAL_DLLPUBLIC_EXPORT void *SAL_CALL component_getFactory(
        const sal_Char *pImplName, void *pServiceManager, void *pRegistryKey )
    {
        return ::cppu::component_getFactoryHelper( pImplName,
                pServiceManager,
                pRegistryKey ,
                framework::complex_toolbar_controls::s_impl_entries );
    }
}
