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



#include "precompiled_configmgr.hxx"
#include "sal/config.h"

#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "osl/diagnose.h"
#include "uno/lbnames.h"
#include "sal/types.h"

#include "configurationprovider.hxx"
#include "configurationregistry.hxx"
#include "defaultprovider.hxx"
#include "update.hxx"

namespace {

namespace css = com::sun::star;

css::uno::Reference< css::uno::XInterface > SAL_CALL dummy(
    css::uno::Reference< css::uno::XComponentContext > const &)
    SAL_THROW((css::uno::Exception))
{
    OSL_ASSERT(false);
    return css::uno::Reference< css::uno::XInterface >();
}

static cppu::ImplementationEntry const services[] = {
    { &dummy, &configmgr::configuration_provider::getImplementationName,
      &configmgr::configuration_provider::getSupportedServiceNames,
      &configmgr::configuration_provider::createFactory, 0, 0 },
    { &configmgr::default_provider::create,
      &configmgr::default_provider::getImplementationName,
      &configmgr::default_provider::getSupportedServiceNames,
      &cppu::createSingleComponentFactory, 0, 0 },
    { &configmgr::configuration_registry::create,
      &configmgr::configuration_registry::getImplementationName,
      &configmgr::configuration_registry::getSupportedServiceNames,
      &cppu::createSingleComponentFactory, 0, 0 },
    { &configmgr::update::create, &configmgr::update::getImplementationName,
      &configmgr::update::getSupportedServiceNames,
      &cppu::createSingleComponentFactory, 0, 0 },
    { 0, 0, 0, 0, 0, 0 }
};

}

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    char const * pImplName, void * pServiceManager, void * pRegistryKey)
{
    return cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey, services);
}

extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL
component_getImplementationEnvironment(
    char const ** ppEnvTypeName, uno_Environment **)
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
