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
#include "precompiled_xmlhelp.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

#include "provider.hxx"

using namespace com::sun::star;

//=========================================================================
extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//=========================================================================
extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    const sal_Char * pImplName,
    void * pServiceManager,
    void * /*pRegistryKey*/ )
{
    void * pRet = 0;

    uno::Reference< lang::XMultiServiceFactory > xSMgr(
        reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ) );
    uno::Reference< lang::XSingleServiceFactory > xFactory;

    //////////////////////////////////////////////////////////////////////
    // Create factory, if implementation name matches.
    //////////////////////////////////////////////////////////////////////

    if ( ::chelp::ContentProvider::getImplementationName_Static().
             compareToAscii( pImplName ) == 0 )
    {
        xFactory = ::chelp::ContentProvider::createServiceFactory( xSMgr );
    }

    //////////////////////////////////////////////////////////////////////

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

