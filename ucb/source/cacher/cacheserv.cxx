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
#include "precompiled_ucb.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <cachedcontentresultset.hxx>
#include <cachedcontentresultsetstub.hxx>
#include <cacheddynamicresultset.hxx>
#include <cacheddynamicresultsetstub.hxx>

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;

//=========================================================================
extern "C" void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//=========================================================================
extern "C" void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * )
{
    void * pRet = 0;

    Reference< XMultiServiceFactory > xSMgr(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ) );
    Reference< XSingleServiceFactory > xFactory;

    //////////////////////////////////////////////////////////////////////
    // CachedContentResultSetFactory.
    //////////////////////////////////////////////////////////////////////

    if ( CachedContentResultSetFactory::getImplementationName_Static().
                compareToAscii( pImplName ) == 0 )
    {
        xFactory = CachedContentResultSetFactory::createServiceFactory( xSMgr );
    }

    //////////////////////////////////////////////////////////////////////
    // CachedContentResultSetStubFactory.
    //////////////////////////////////////////////////////////////////////

    else if ( CachedContentResultSetStubFactory::getImplementationName_Static().
                compareToAscii( pImplName ) == 0 )
    {
        xFactory = CachedContentResultSetStubFactory::createServiceFactory( xSMgr );
    }

    //////////////////////////////////////////////////////////////////////
    // CachedDynamicResultSetFactory.
    //////////////////////////////////////////////////////////////////////

    else if ( CachedDynamicResultSetFactory::getImplementationName_Static().
                compareToAscii( pImplName ) == 0 )
    {
        xFactory = CachedDynamicResultSetFactory::createServiceFactory( xSMgr );
    }

    //////////////////////////////////////////////////////////////////////
    // CachedDynamicResultSetStubFactory.
    //////////////////////////////////////////////////////////////////////

    else if ( CachedDynamicResultSetStubFactory::getImplementationName_Static().
                compareToAscii( pImplName ) == 0 )
    {
        xFactory = CachedDynamicResultSetStubFactory::createServiceFactory( xSMgr );
    }

    //////////////////////////////////////////////////////////////////////

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

