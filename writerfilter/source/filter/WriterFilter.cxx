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



#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_
#include <cppuhelper/implementationentry.hxx>
#endif
#include <WriterFilter.hxx>
#include <WriterFilterDetection.hxx>
#include <RtfFilter.hxx>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star;

/*-- 22.02.2007 12:19:23---------------------------------------------------

  -----------------------------------------------------------------------*/
WriterFilter::WriterFilter( const uno::Reference< uno::XComponentContext >& rxContext)  :
    m_xContext( rxContext )
{
}
/*-- 22.02.2007 12:19:23---------------------------------------------------

  -----------------------------------------------------------------------*/
WriterFilter::~WriterFilter()
{
}

extern "C"
{
/* shared lib exports implemented with helpers */
static struct ::cppu::ImplementationEntry s_component_entries [] =
{
    { WriterFilter_createInstance, WriterFilter_getImplementationName, WriterFilter_getSupportedServiceNames, ::cppu::createSingleComponentFactory, 0, 0 },
    { WriterFilterDetection_createInstance, WriterFilterDetection_getImplementationName, WriterFilterDetection_getSupportedServiceNames, ::cppu::createSingleComponentFactory, 0, 0} ,
    { RtfFilter_createInstance, RtfFilter_getImplementationName, RtfFilter_getSupportedServiceNames, ::cppu::createSingleComponentFactory, 0, 0 },
    { 0, 0, 0, 0, 0, 0 } // terminate with NULL
};

SAL_DLLPUBLIC_EXPORT void SAL_CALL
component_getImplementationEnvironment(const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

SAL_DLLPUBLIC_EXPORT void * SAL_CALL
component_getFactory(sal_Char const * implName, ::com::sun::star::lang::XMultiServiceFactory * xMgr, ::com::sun::star::registry::XRegistryKey * xRegistry )
{
    return ::cppu::component_getFactoryHelper(implName, xMgr, xRegistry, s_component_entries );
}

} //extern "C"


