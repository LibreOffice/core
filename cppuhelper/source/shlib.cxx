/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "sal/config.h"

#include "osl/module.hxx"
#include "uno/environment.h"
#include "uno/mapping.hxx"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/shlib.hxx"

#include "com/sun/star/beans/XPropertySet.hpp"

#include <stdio.h>

#ifdef ANDROID
#include <osl/detail/android-bootstrap.h>
#endif

#ifdef IOS
#include <osl/detail/component-mapping.h>
#endif

using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

using rtl::OString;
using rtl::OUString;

namespace cppu
{

#ifndef DISABLE_DYNLOADING

static void getLibEnv(oslModule                lib,
                      uno::Environment       * pEnv,
                      OUString               * pSourceEnv_name,
                      uno::Environment const & cTargetEnv,
                      OUString         const & cImplName = OUString(),
                      OUString         const & rPrefix = OUString())
{
    sal_Char const * pEnvTypeName = NULL;

    OUString aGetEnvNameExt = rPrefix + COMPONENT_GETENVEXT;
    component_getImplementationEnvironmentExtFunc pGetImplEnvExt =
        (component_getImplementationEnvironmentExtFunc)osl_getFunctionSymbol(lib, aGetEnvNameExt.pData);

    if (pGetImplEnvExt)
    {
        OString implName(OUStringToOString(cImplName, RTL_TEXTENCODING_ASCII_US));
        pGetImplEnvExt(&pEnvTypeName, (uno_Environment **)pEnv, implName.getStr(), cTargetEnv.get());
    }
    else
    {
        OUString aGetEnvName = rPrefix + COMPONENT_GETENV;
        component_getImplementationEnvironmentFunc pGetImplEnv =
            (component_getImplementationEnvironmentFunc)osl_getFunctionSymbol(
                lib, aGetEnvName.pData );
        if (pGetImplEnv)
            pGetImplEnv(&pEnvTypeName, (uno_Environment **)pEnv);

        else // this symbol used to be mandatory, but is no longer
            pEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    if (!pEnv->is() && pEnvTypeName)
    {
        *pSourceEnv_name = OUString::createFromAscii(pEnvTypeName);
        static const char * pUNO_ENV_LOG = ::getenv( "UNO_ENV_LOG" );
        if (pUNO_ENV_LOG && rtl_str_getLength(pUNO_ENV_LOG) )
        {
            OString implName(OUStringToOString(cImplName, RTL_TEXTENCODING_ASCII_US));
            OString aEnv( pUNO_ENV_LOG );
            sal_Int32 nIndex = 0;
            do
            {
                const OString aStr( aEnv.getToken( 0, ';', nIndex ) );
                if ( aStr.equals(implName) )
                {
                    *pSourceEnv_name += ::rtl::OUString(":log");
                    break;
                }
            } while( nIndex != -1 );
        }
    }
}

#endif

extern "C" {static void s_getFactory(va_list * pParam)
{
    component_getFactoryFunc         pSym      = va_arg(*pParam, component_getFactoryFunc);
    OString                  const * pImplName = va_arg(*pParam, OString const *);
    void                           * pSMgr     = va_arg(*pParam, void *);
    void                           * pKey      = va_arg(*pParam, void *);
    void                          ** ppSSF     = va_arg(*pParam, void **);

    *ppSSF = pSym(pImplName->getStr(), pSMgr, pKey);
}}

/* For backwards compatibility */
Reference< XInterface > SAL_CALL loadSharedLibComponentFactory(
    OUString const & uri, OUString const & rPath, OUString const & rImplName,
    Reference< lang::XMultiServiceFactory > const & xMgr,
    Reference< registry::XRegistryKey > const & xKey )
    SAL_THROW( (loader::CannotActivateFactoryException) )
{
    return loadSharedLibComponentFactory( uri, rPath, rImplName, xMgr, xKey, rtl::OUString() );
}

namespace
{

Reference< XInterface > invokeComponentFactory(
    oslGenericFunction pGetter,
    oslModule lib,
    OUString const & rModulePath,
    OUString const & rImplName,
    Reference< ::com::sun::star::lang::XMultiServiceFactory > const & xMgr,
    OUString const & rPrefix,
    OUString &rExcMsg )
{
    Reference< XInterface > xRet;
    uno::Environment currentEnv(Environment::getCurrent());
    uno::Environment env;
    OUString aEnvTypeName;

#ifdef DISABLE_DYNLOADING
    (void) lib;
    (void) rPrefix;
    // It seems that the only UNO components that have
    // component_getImplementationEnvironment functions are the JDBC
    // and ADO (whatever that is) database connectivity thingies
    // neither of which make sense on iOS and Android (which are the
    // only platforms for which DISABLE_DYNLOADING is intended,
    // really). So we can simply bypass the getLibEnv() stuff and
    // don't need to wonder how to find out what function to call at
    // this point if statically linked.
    aEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
#else
    getLibEnv(lib, &env, &aEnvTypeName, currentEnv, rImplName, rPrefix);
#endif

    OString aImplName(
        OUStringToOString( rImplName, RTL_TEXTENCODING_ASCII_US ) );

    if (!env.is())
        env = uno::Environment(aEnvTypeName);

    if (env.is() && currentEnv.is())
    {
#if OSL_DEBUG_LEVEL > 1
        {
            rtl::OString modPath(rtl::OUStringToOString(rModulePath, RTL_TEXTENCODING_ASCII_US));
            rtl::OString implName(rtl::OUStringToOString(rImplName, RTL_TEXTENCODING_ASCII_US));
            rtl::OString envDcp(rtl::OUStringToOString(env.getTypeName(), RTL_TEXTENCODING_ASCII_US));

            fprintf(stderr, "invokeComponentFactory envDcp:%s implName:%s modPath:%s\n", envDcp.getStr(), implName.getStr(), modPath.getStr());
        }
#endif

        Mapping aCurrent2Env( currentEnv, env );
        Mapping aEnv2Current( env, currentEnv );

        if (aCurrent2Env.is() && aEnv2Current.is())
        {
            void * pSMgr = aCurrent2Env.mapInterface(
                xMgr.get(), ::getCppuType( &xMgr ) );

            void * pSSF = NULL;

            env.invoke(s_getFactory, pGetter, &aImplName, pSMgr, 0, &pSSF);

            if (pSMgr)
            {
                (*env.get()->pExtEnv->releaseInterface)(
                    env.get()->pExtEnv, pSMgr );
            }

            if (pSSF)
            {
                aEnv2Current.mapInterface(
                    reinterpret_cast< void ** >( &xRet ),
                    pSSF, ::getCppuType( &xRet ) );
                (env.get()->pExtEnv->releaseInterface)(
                    env.get()->pExtEnv, pSSF );
            }
            else
            {
                rExcMsg = rModulePath +
                          ": cannot get factory of " +
                          "demanded implementation: " +
                          OStringToOUString(
                        aImplName, RTL_TEXTENCODING_ASCII_US );
            }
        }
        else
        {
            rExcMsg =
                "cannot get uno mappings: C++ <=> UNO!";
        }
    }
    else
    {
        rExcMsg = "cannot get uno environments!";
    }

    return xRet;
}

} // namespace

#ifdef DISABLE_DYNLOADING
extern "C"
{
    extern void * bootstrap_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * configmgr_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * comphelp_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * deployment_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * expwrap_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * fastsax_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * filterconfig1_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * fwk_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * introspection_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * localebe1_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * package2_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * reflection_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * sfx_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * svl_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * tk_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * stocservices_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * i18npool_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * ucb_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * ucpexpand1_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * ucpfile_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * utl_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * vcl_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * xstor_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
}
#endif

Reference< XInterface > SAL_CALL loadSharedLibComponentFactory(
    OUString const & uri, OUString const & rPath, OUString const & rImplName,
    Reference< lang::XMultiServiceFactory > const & xMgr,
    Reference< registry::XRegistryKey > const & xKey,
    OUString const & rPrefix )
    SAL_THROW( (loader::CannotActivateFactoryException) )
{
    assert(rPath.isEmpty());
    assert(!xKey.is());
    (void) rPath;
    (void) xKey;
#ifndef DISABLE_DYNLOADING
    OUString moduleUri(uri);

#ifdef ANDROID
    if ( uri == "bootstrap.uno" SAL_DLLEXTENSION )
        moduleUri = "libbootstrap.uno" SAL_DLLEXTENSION;
#endif

    oslModule lib = osl_loadModule(
        moduleUri.pData, SAL_LOADMODULE_LAZY | SAL_LOADMODULE_GLOBAL );
    if (! lib)
    {
        OUString const msg("loading component library failed: " + moduleUri);
        SAL_WARN("cppuhelper", msg);
        throw loader::CannotActivateFactoryException(msg,
            Reference< XInterface >() );
    }
#else
    oslModule lib;
    OUString moduleUri("MAIN");
    if (! osl_getModuleHandle( NULL, &lib))
    {
        throw loader::CannotActivateFactoryException(
            "osl_getModuleHandle of the executable: ",
            Reference< XInterface >() );
    }
#endif

    Reference< XInterface > xRet;

    OUString aExcMsg;

    OUString aGetFactoryName = rPrefix + COMPONENT_GETFACTORY;

    oslGenericFunction pSym = NULL;

#ifdef DISABLE_DYNLOADING

    // First test library names that aren't app-specific.
    static lib_to_component_mapping non_app_specific_map[] = {
        // Sigh, the name under which the bootstrap component is looked for
        // varies a lot? Or then I just have been confused by some mixed-up
        // incremental build.
        { "bootstrap.uno" SAL_DLLEXTENSION, bootstrap_component_getFactory },
        { "bootstrap.uno.a", bootstrap_component_getFactory },
        { "libbootstraplo.a", bootstrap_component_getFactory },

        { "libintrospectionlo.a", introspection_component_getFactory },
        { "libreflectionlo.a", reflection_component_getFactory },
        { "libstocserviceslo.a", stocservices_component_getFactory },
        { "libcomphelper.a", comphelp_component_getFactory },
        { "libconfigmgrlo.a", configmgr_component_getFactory },
        { "libdeployment.a", deployment_component_getFactory },
        { "libexpwraplo.a", expwrap_component_getFactory },
        { "libfastsaxlo.a", fastsax_component_getFactory },
        { "libfilterconfiglo.a", filterconfig1_component_getFactory },
        { "libfwklo.a", fwk_component_getFactory },
        { "libi18npoollo.a", i18npool_component_getFactory },
        { "liblocalebe1lo.a", localebe1_component_getFactory },
        { "libpackage2.a", package2_component_getFactory },
        { "libsfxlo.a", sfx_component_getFactory },
        { "libsvllo.a", svl_component_getFactory },
        { "libtklo.a", tk_component_getFactory },
        { "libucb1.a", ucb_component_getFactory },
        { "libucpexpand1lo.a", ucpexpand1_component_getFactory },
        { "libucpfile1.a", ucpfile_component_getFactory },
        { "libutllo.a", utl_component_getFactory },
        { "libvcllo.a", vcl_component_getFactory },
        { "libxstor.a", xstor_component_getFactory },
        { NULL, NULL }
    };
    for (int i = 0; pSym == NULL && non_app_specific_map[i].lib != NULL; ++i)
    {
        if ( uri.equalsAscii( non_app_specific_map[i].lib ) )
            pSym = (oslGenericFunction) non_app_specific_map[i].component_getFactory_function;
    }

    if ( pSym == NULL)
    {
        // The call the app-specific lo_get_libmap() to get a mapping for the rest
        const lib_to_component_mapping *map = lo_get_libmap();
        for (int i = 0; pSym == NULL && map[i].lib != NULL; ++i)
        {
            if ( uri.equalsAscii( map[i].lib ) )
                pSym = (oslGenericFunction) map[i].component_getFactory_function;
        }
        if ( pSym == NULL )
        {
            fprintf( stderr, "attempting to load unknown library %s\n", OUStringToOString( uri, RTL_TEXTENCODING_ASCII_US ).getStr() );
            assert( !"Attempt to load unknown library" );
        }
    }
#else

    if ( pSym == NULL )
        pSym = osl_getFunctionSymbol( lib, aGetFactoryName.pData );
#endif

    if (pSym != 0)
    {
        xRet = invokeComponentFactory( pSym, lib, moduleUri, rImplName, xMgr, rPrefix, aExcMsg );
    }
    else
    {
        aExcMsg = moduleUri;
        aExcMsg += ": cannot get symbol: ";
        aExcMsg += aGetFactoryName;
    }

    if (! xRet.is())
    {
#ifndef DISABLE_DYNLOADING
        osl_unloadModule( lib );
#endif
        SAL_WARN("cppuhelper", "### cannot activate factory: " << aExcMsg);
        throw loader::CannotActivateFactoryException(
            aExcMsg,
            Reference< XInterface >() );
    }
    return xRet;
}

#ifndef DISABLE_DYNLOADING

//==============================================================================
extern "C" { static void s_writeInfo(va_list * pParam)
{
    component_writeInfoFunc         pSym      = va_arg(*pParam, component_writeInfoFunc);
    void                          * pSMgr     = va_arg(*pParam, void *);
    void                          * pKey      = va_arg(*pParam, void *);
    sal_Bool                      * pbRet     = va_arg(*pParam, sal_Bool *);

    *pbRet = pSym(pSMgr, pKey);

}}

void SAL_CALL writeSharedLibComponentInfo(
    OUString const & uri, OUString const & rPath,
    Reference< lang::XMultiServiceFactory > const & xMgr,
    Reference< registry::XRegistryKey > const & xKey )
    SAL_THROW( (registry::CannotRegisterImplementationException) )
{
    (void) rPath;
    assert(rPath.isEmpty());
    oslModule lib = osl_loadModule(
        uri.pData, SAL_LOADMODULE_LAZY | SAL_LOADMODULE_GLOBAL );
    if (! lib)
    {
        OUString const msg("loading component library failed: " + uri);
        SAL_WARN("cppuhelper", msg);
        throw registry::CannotRegisterImplementationException(msg,
            Reference< XInterface >() );
    }

    sal_Bool bRet = sal_False;

    uno::Environment currentEnv(Environment::getCurrent());
    uno::Environment env;

    OUString aEnvTypeName;
    OUString aExcMsg;

    getLibEnv(lib, &env, &aEnvTypeName, currentEnv);

    OUString aWriteInfoName = COMPONENT_WRITEINFO;
    oslGenericFunction pSym = osl_getFunctionSymbol( lib, aWriteInfoName.pData );
    if (pSym != 0)
    {
        if (!env.is())
            env = uno::Environment(aEnvTypeName);

        if (env.is() && currentEnv.is())
        {
            Mapping aCurrent2Env( currentEnv, env );
            if (aCurrent2Env.is())
            {
                void * pSMgr = aCurrent2Env.mapInterface(
                    xMgr.get(), ::getCppuType( &xMgr ) );
                void * pKey = aCurrent2Env.mapInterface(
                    xKey.get(), ::getCppuType( &xKey ) );
                if (pKey)
                {
                    env.invoke(s_writeInfo, pSym, pSMgr, pKey, &bRet);


                    (*env.get()->pExtEnv->releaseInterface)(
                        env.get()->pExtEnv, pKey );
                    if (! bRet)
                    {
                        aExcMsg = uri;
                        aExcMsg += ": component_writeInfo() "
                                   "returned false!";
                    }
                }
                else
                {
                    // key is mandatory
                    aExcMsg = uri;
                    aExcMsg += ": registry is mandatory to invoke"
                               " component_writeInfo()!";
                }

                if (pSMgr)
                {
                    (*env.get()->pExtEnv->releaseInterface)(
                        env.get()->pExtEnv, pSMgr );
                }
            }
            else
            {
                aExcMsg = "cannot get uno mapping: C++ <=> UNO!";
            }
        }
        else
        {
            aExcMsg = "cannot get uno environments!";
        }
    }
    else
    {
        aExcMsg = uri;
        aExcMsg += ": cannot get symbol: ";
        aExcMsg += aWriteInfoName;
    }

//!
//! OK: please look at #88219#
//!
//! ::osl_unloadModule( lib);
    if (! bRet)
    {
        SAL_WARN("cppuhelper", "### cannot write component info: " << aExcMsg);
        throw registry::CannotRegisterImplementationException(
            aExcMsg, Reference< XInterface >() );
    }
}

#endif // DISABLE_DYNLOADING

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
