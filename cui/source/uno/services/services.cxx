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
#include "precompiled_cui.hxx"

#include <com/sun/star/registry/XRegistryKey.hpp>
#include "sal/types.h"
#include "cppuhelper/factory.hxx"
#include <cppuhelper/implementationentry.hxx>

using rtl::OUString;
using namespace com::sun::star;

namespace cui {
extern rtl::OUString SAL_CALL ColorPicker_getImplementationName();
extern uno::Reference< uno::XInterface > SAL_CALL ColorPicker_createInstance( uno::Reference< uno::XComponentContext > const & ) SAL_THROW( (uno::Exception) );
extern uno::Sequence< rtl::OUString > SAL_CALL ColorPicker_getSupportedServiceNames() throw( uno::RuntimeException );
}

namespace
{

    cppu::ImplementationEntry entries[] = {
        { &::cui::ColorPicker_createInstance, &::cui::ColorPicker_getImplementationName, &::cui::ColorPicker_getSupportedServiceNames, &cppu::createSingleComponentFactory, 0, 0 },
        { 0, 0, 0, 0, 0, 0 }
    };
}

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory( char const * implName, void * serviceManager, void * registryKey)
{
    return cppu::component_getFactoryHelper(implName, serviceManager, registryKey, entries);
}


extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment ( const sal_Char ** ppEnvTypeName, uno_Environment ** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

// eof
