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
#include "ucb.hxx"
#include "ucbstore.hxx"
#include "ucbprops.hxx"
#include "provprox.hxx"
#include "cmdenv.hxx"

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

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
    // Universal Content Broker.
    //////////////////////////////////////////////////////////////////////

    if ( UniversalContentBroker::getImplementationName_Static().
                compareToAscii( pImplName ) == 0 )
    {
        xFactory = UniversalContentBroker::createServiceFactory( xSMgr );
    }

    //////////////////////////////////////////////////////////////////////
    // UCB Store.
    //////////////////////////////////////////////////////////////////////

    else if ( UcbStore::getImplementationName_Static().
                compareToAscii( pImplName ) == 0 )
    {
        xFactory = UcbStore::createServiceFactory( xSMgr );
    }

    //////////////////////////////////////////////////////////////////////
    // UCB PropertiesManager.
    //////////////////////////////////////////////////////////////////////

    else if ( UcbPropertiesManager::getImplementationName_Static().
                compareToAscii( pImplName ) == 0 )
    {
        xFactory = UcbPropertiesManager::createServiceFactory( xSMgr );
    }

    //////////////////////////////////////////////////////////////////////
    // UCP Proxy Factory.
    //////////////////////////////////////////////////////////////////////

    else if ( UcbContentProviderProxyFactory::getImplementationName_Static().
                compareToAscii( pImplName ) == 0 )
    {
        xFactory
            = UcbContentProviderProxyFactory::createServiceFactory( xSMgr );
    }

    //////////////////////////////////////////////////////////////////////
    // Command Environment.
    //////////////////////////////////////////////////////////////////////

    else if ( ucb_cmdenv::UcbCommandEnvironment::getImplementationName_Static().
                compareToAscii( pImplName ) == 0 )
    {
        xFactory
            = ucb_cmdenv::UcbCommandEnvironment::createServiceFactory( xSMgr );
    }

    //////////////////////////////////////////////////////////////////////

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

