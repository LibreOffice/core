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
#include "precompiled_linguistic.hxx"


#include <cppuhelper/factory.hxx>   // helper for factories
#include <rtl/string.hxx>

#include <com/sun/star/registry/XRegistryKey.hpp>

using namespace com::sun::star::lang;

using namespace com::sun::star::registry;

extern void * SAL_CALL LngSvcMgr_getFactory
(
    const sal_Char * pImplName,
    XMultiServiceFactory * pServiceManager,
    void * /*pRegistryKey*/
);

extern void * SAL_CALL DicList_getFactory
(
    const sal_Char * pImplName,
    XMultiServiceFactory * pServiceManager,
    void *
);

void * SAL_CALL LinguProps_getFactory
(
    const sal_Char * pImplName,
    XMultiServiceFactory * pServiceManager,
    void *
);

extern void * SAL_CALL ConvDicList_getFactory
(
    const sal_Char * pImplName,
    XMultiServiceFactory * pServiceManager,
    void *
);

extern void * SAL_CALL GrammarCheckingIterator_getFactory
(
    const sal_Char * pImplName,
    XMultiServiceFactory * pServiceManager,
    void *
);

//extern void * SAL_CALL GrammarChecker_getFactory
//(
//    const sal_Char * pImplName,
//    XMultiServiceFactory * pServiceManager,
//    void *
//);

////////////////////////////////////////
// definition of the two functions that are used to provide the services
//

extern "C"
{

SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet =
        LngSvcMgr_getFactory(
            pImplName,
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            pRegistryKey );

    if(!pRet)
        pRet = LinguProps_getFactory(
            pImplName,
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            pRegistryKey );

    if(!pRet)
        pRet =  DicList_getFactory(
            pImplName,
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            pRegistryKey );

    if(!pRet)
        pRet =  ConvDicList_getFactory(
            pImplName,
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            pRegistryKey );

    if(!pRet)
        pRet =  GrammarCheckingIterator_getFactory(
            pImplName,
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            pRegistryKey );
/*
    if(!pRet)
        pRet =  GrammarChecker_getFactory(
            pImplName,
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            pRegistryKey );
*/
    return pRet;
}
}

///////////////////////////////////////////////////////////////////////////

