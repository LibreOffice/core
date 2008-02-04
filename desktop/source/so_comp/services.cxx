/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: services.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-04 13:48:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "evaluation.hxx"

#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif

#ifndef _UNO_ENVIRONMENT_H_
#include <uno/environment.h>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif

#include "oemjob.hxx"
#include "evaluation.hxx"

#include <string.h>

using namespace rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::registry;
using namespace ::desktop;

static const char* pServices[] =
{
    SOEvaluation::serviceName,
    OEMPreloadJob::serviceName,
    NULL
};

static const char* pImplementations[] =
{
    SOEvaluation::implementationName,
    OEMPreloadJob::implementationName,
    NULL
};

typedef Reference<XInterface>(* fProvider)(const Reference<XMultiServiceFactory>&);

static const fProvider pInstanceProviders[] =
{
    SOEvaluation::CreateInstance,
    OEMPreloadJob::CreateInstance,
    NULL
};


static const char** pSupportedServices[] =
{
    SOEvaluation::interfaces,
    OEMPreloadJob::interfaces,
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
