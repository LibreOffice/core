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

#include "xmxcom.hxx"
#include "xmxtrct.hxx"
#include <osl/diagnose.h>
#include <cppuhelper/factory.hxx>
#include <uno/mapping.hxx>

// -------------------
// - factory methods -
// -------------------

static REF( NMSP_UNO::XInterface ) SAL_CALL create_XMLExtractor( const REF( NMSP_LANG::XMultiServiceFactory )& rxFact )
{
    return REF( NMSP_UNO::XInterface )( *new XMLExtractor( rxFact ) );
}

extern "C"
{
//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void * pRet = 0;

    if (rtl_str_compare( pImplName, "com.sun.star.comp.io.XMLExtractor" ) == 0)
    {
        NMSP_RTL::OUString aServiceName( B2UCONST( "com.sum.star.io.XMLExtractor" ) );

        REF( NMSP_LANG::XSingleServiceFactory ) xFactory( NMSP_CPPU::createSingleFactory(
            reinterpret_cast< NMSP_LANG::XMultiServiceFactory * >( pServiceManager ),
            NMSP_RTL::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.io.XMLExtractor") ),
            create_XMLExtractor,
            SEQ( NMSP_RTL::OUString )( &aServiceName, 1 ) ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}
