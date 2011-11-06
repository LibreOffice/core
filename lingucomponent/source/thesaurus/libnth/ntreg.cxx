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
#include "precompiled_lingucomponent.hxx"


#include <cppuhelper/factory.hxx>   // helper for factories
#include <rtl/string.hxx>

#include <com/sun/star/registry/XRegistryKey.hpp>

using namespace com::sun::star::lang;
using namespace com::sun::star::registry;

////////////////////////////////////////
// declaration of external RegEntry-functions defined by the service objects
//

extern void * SAL_CALL Thesaurus_getFactory(
    const sal_Char * pImplName,
    XMultiServiceFactory * pServiceManager,
    void * /*pRegistryKey*/ );

////////////////////////////////////////
// definition of the two functions that are used to provide the services
//

extern "C"
{

void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = Thesaurus_getFactory(
            pImplName,
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            pRegistryKey );

    return pRet;
}

}

///////////////////////////////////////////////////////////////////////////

