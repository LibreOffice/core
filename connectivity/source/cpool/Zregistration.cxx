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
#include "precompiled_connectivity.hxx"

#include <stdio.h>
#include <cppuhelper/factory.hxx>
#include "ZPoolCollection.hxx"


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace connectivity;
//==========================================================================
//= registration
//==========================================================================
extern "C"
{

//---------------------------------------------------------------------------------------
    SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(const sal_Char** _ppEnvTypeName, uno_Environment** /*_ppEnv*/)
{
    *_ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//---------------------------------------------------------------------------------------
SAL_DLLPUBLIC_EXPORT void* SAL_CALL component_getFactory(const sal_Char* _pImplName, ::com::sun::star::lang::XMultiServiceFactory* _pServiceManager, void* /*_pRegistryKey*/)
{
    void* pRet = NULL;

    if (OPoolCollection::getImplementationName_Static().compareToAscii(_pImplName) == 0)
    {
        Reference< XSingleServiceFactory > xFactory(
            ::cppu::createOneInstanceFactory(
                _pServiceManager,
                OPoolCollection::getImplementationName_Static(),
                OPoolCollection::CreateInstance,
                OPoolCollection::getSupportedServiceNames_Static()
            )
        );
        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}

}   // extern "C"


