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
#include "precompiled_desktop.hxx"
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/util/Date.hpp>
#include <uno/environment.h>
#include <cppuhelper/factory.hxx>
#include <unotools/configmgr.hxx>
#include <string.h>

#include "splash.hxx"
#include "firststart.hxx"


using namespace rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::registry;
using namespace ::desktop;

static const char* pServices[] =
{
    SplashScreen::serviceName,
    FirstStart::serviceName,
    NULL
};

static const char* pImplementations[] =
{
    SplashScreen::implementationName,
    FirstStart::implementationName,
    NULL
};

typedef Reference<XInterface>(* fProvider)(const Reference<XMultiServiceFactory>&);

static const fProvider pInstanceProviders[] =
{
    SplashScreen::getInstance,
    FirstStart::CreateInstance,
    NULL
};


static const char** pSupportedServices[] =
{
    SplashScreen::interfaces,
    FirstStart::interfaces,
    NULL
};

static Sequence<OUString>
getSupportedServiceNames(int p) {
    const char **names = pSupportedServices[p];
    Sequence<OUString> aSeq;
    for(int i = 0; names[i] != NULL; i++) {
        aSeq.realloc(i+1);
        aSeq[i] = OUString::createFromAscii(names[i]);
    }
    return aSeq;
}

extern "C"
{
void SAL_CALL
component_getImplementationEnvironment(
    const sal_Char** ppEnvironmentTypeName,
    uno_Environment**)
{
    *ppEnvironmentTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME ;
}

void* SAL_CALL
component_getFactory(
    const sal_Char* pImplementationName,
    void* pServiceManager,
    void*)
{
    // Set default return value for this operation - if it failed.
    if  ( pImplementationName && pServiceManager )
    {
        Reference< XSingleServiceFactory > xFactory;
        Reference< XMultiServiceFactory > xServiceManager(
            reinterpret_cast< XMultiServiceFactory* >( pServiceManager ) ) ;

        // search implementation
        for (int i = 0; (pImplementations[i]!=NULL); i++) {
            if ( strcmp(pImplementations[i], pImplementationName ) == 0 ) {
                // found implementation
                xFactory = Reference<XSingleServiceFactory>(cppu::createSingleFactory(
                    xServiceManager, OUString::createFromAscii(pImplementationName),
                    pInstanceProviders[i], getSupportedServiceNames(i)));
                if ( xFactory.is() ) {
                    // Factory is valid - service was found.
                    xFactory->acquire();
                    return xFactory.get();
                }
            }
        } // for()
    }
    // Return with result of this operation.
    return NULL;
}
} // extern "C"
