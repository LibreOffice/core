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
#include "precompiled_extensions.hxx"
#include "res_services.hxx"

/** === begin UNO using === **/
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::lang::XSingleServiceFactory;
using ::com::sun::star::uno::UNO_QUERY;
/** === end UNO using === **/

#include <vector>

namespace res
{
    ::std::vector< ComponentInfo > getComponentInfos()
    {
        ::std::vector< ::res::ComponentInfo > aComponentInfos;
        aComponentInfos.push_back( getComponentInfo_VclStringResourceLoader() );
        aComponentInfos.push_back( getComponentInfo_OpenOfficeResourceLoader() );
        return aComponentInfos;
    }
}

extern "C" {

void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, XMultiServiceFactory * /*pServiceManager*/, void * /*pRegistryKey*/ )
{
    void * pRet = 0;
    ::std::vector< ::res::ComponentInfo > aComponentInfos( ::res::getComponentInfos() );
    for (   ::std::vector< ::res::ComponentInfo >::const_iterator loop = aComponentInfos.begin();
            loop != aComponentInfos.end();
            ++loop
        )
    {
        if ( 0 == loop->sImplementationName.compareToAscii( pImplName ) )
        {
            // create the factory
            Reference< XSingleServiceFactory > xFactory( ::cppu::createSingleComponentFactory(
                    loop->pFactory,  loop->sImplementationName, loop->aSupportedServices ),
                    UNO_QUERY );
            // acquire, because we return an interface pointer instead of a reference
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }
    return pRet;
}

}   // extern "C"
