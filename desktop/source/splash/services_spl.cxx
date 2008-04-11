/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: services_spl.cxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/util/Date.hpp>
#include <uno/environment.h>
#include <cppuhelper/factory.hxx>
#include <unotools/configmgr.hxx>

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

sal_Bool SAL_CALL
component_writeInfo(
    void* pServiceManager,
    void* pRegistryKey)
{
    Reference<XMultiServiceFactory> xMan(
        reinterpret_cast< XMultiServiceFactory* >( pServiceManager ) ) ;
    Reference<XRegistryKey> xKey(
        reinterpret_cast< XRegistryKey* >( pRegistryKey ) ) ;

    // iterate over service names and register them...
    OUString aImpl;
    const char* pServiceName = NULL;
    const char* pImplName = NULL;
    for (int i = 0; (pServices[i]!=NULL)&&(pImplementations[i]!=NULL); i++) {
        pServiceName= pServices[i];
        pImplName = pImplementations[i];
        aImpl = OUString::createFromAscii("/")
              + OUString::createFromAscii(pImplName)
              + OUString::createFromAscii("/UNO/SERVICES");
        Reference<XRegistryKey> xNewKey = xKey->createKey(aImpl);
        xNewKey->createKey(OUString::createFromAscii(pServiceName));
    }
    return sal_True;
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
