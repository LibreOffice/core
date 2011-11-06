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



#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>

#include "CNodes.hxx"
#include "librdf_repository.hxx"


using namespace ::com::sun::star;


extern "C"
{

SAL_DLLPUBLIC_EXPORT void SAL_CALL
component_getImplementationEnvironment(const sal_Char **o_ppEnvironmentTypeName,
    uno_Environment ** /* ppEnvironment */)
{
    *o_ppEnvironmentTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

static ::cppu::ImplementationEntry const entries[] = {
    { &comp_CBlankNode::_create,
      &comp_CBlankNode::_getImplementationName,
      &comp_CBlankNode::_getSupportedServiceNames,
      &::cppu::createSingleComponentFactory, 0, 0 },
    { &comp_CURI::_create,
      &comp_CURI::_getImplementationName,
      &comp_CURI::_getSupportedServiceNames,
      &::cppu::createSingleComponentFactory, 0, 0 },
    { &comp_CLiteral::_create,
      &comp_CLiteral::_getImplementationName,
      &comp_CLiteral::_getSupportedServiceNames,
      &::cppu::createSingleComponentFactory, 0, 0 },
    { &comp_librdf_Repository::_create,
      &comp_librdf_Repository::_getImplementationName,
      &comp_librdf_Repository::_getSupportedServiceNames,
      &::cppu::createSingleComponentFactory, 0, 0 },
    { 0, 0, 0, 0, 0, 0 }
};

SAL_DLLPUBLIC_EXPORT void * SAL_CALL
component_getFactory(
    const char * implName, void * serviceManager, void * registryKey)
{
    return ::cppu::component_getFactoryHelper(
        implName, serviceManager, registryKey, entries);
}

} // extern "C"

