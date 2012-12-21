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


#include <vector>

#include "rtl/string.hxx"
#include "rtl/bootstrap.hxx"
#include "rtl/strbuf.hxx"
#include "osl/diagnose.h"
#include "osl/file.h"
#include "osl/module.h"
#include "osl/process.h"
#include "cppuhelper/shlib.hxx"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/component_context.hxx"
#include "cppuhelper/bootstrap.hxx"

#include "com/sun/star/uno/DeploymentException.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/lang/XInitialization.hpp"
#include <com/sun/star/lang/XServiceInfo.hpp>
#include "com/sun/star/lang/XSingleServiceFactory.hpp"
#include "com/sun/star/lang/XSingleComponentFactory.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/container/XSet.hpp"
#include "com/sun/star/container/XHierarchicalNameAccess.hpp"
#include "com/sun/star/registry/XSimpleRegistry.hpp"
#include "com/sun/star/uno/SecurityException.hpp"
#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

#include "macro_expander.hxx"
#include "paths.hxx"
#include "servicefactory_detail.hxx"

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace cppu
{

static Reference< XInterface > SAL_CALL createInstance(
    Reference< XInterface > const & xFactory,
    Reference< XComponentContext > const & xContext =
    Reference< XComponentContext >() )
{
    Reference< lang::XSingleComponentFactory > xFac( xFactory, UNO_QUERY );
    if (xFac.is())
    {
        return xFac->createInstanceWithContext( xContext );
    }
    else
    {
        Reference< lang::XSingleServiceFactory > xFac2( xFactory, UNO_QUERY );
        if (xFac2.is())
        {
            OSL_ENSURE( !xContext.is(), "### ignoring context!" );
            return xFac2->createInstance();
        }
    }
    throw RuntimeException(
        OUSTR("no factory object given!"),
        Reference< XInterface >() );
}

/** bootstrap variables:

    UNO_AC=<mode> [mandatory]
      -- mode := { on, off, dynamic-only, single-user, single-default-user }
    UNO_AC_SERVICE=<service_name> [optional]
      -- override ac singleton service name
    UNO_AC_SINGLEUSER=<user-id|nothing> [optional]
      -- run with this user id or with default user policy (<nothing>)
         set UNO_AC=single-[default-]user
    UNO_AC_USERCACHE_SIZE=<cache_size>
      -- number of user permission sets to be cached

    UNO_AC_POLICYSERVICE=<service_name> [optional]
      -- override policy singleton service name
    UNO_AC_POLICYFILE=<file_url> [optional]
      -- read policy out of simple text file
*/
void add_access_control_entries(
    ::std::vector< ContextEntry_Init > * values,
    Bootstrap const & bootstrap )
    SAL_THROW( (Exception) )
{
    ContextEntry_Init entry;
    ::std::vector< ContextEntry_Init > & context_values = *values;

    OUString ac_policy;
    if (bootstrap.getFrom( OUSTR("UNO_AC_POLICYSERVICE"), ac_policy ))
    {
        // overridden service name
        // - policy singleton
        entry.bLateInitService = true;
        entry.name = OUSTR("/singletons/com.sun.star.security.thePolicy");
        entry.value <<= ac_policy;
        context_values.push_back( entry );
    }
    else if (bootstrap.getFrom( OUSTR("UNO_AC_POLICYFILE"), ac_policy ))
    {
        // check for file policy
        // - file policy prop: file-name
        if (0 != ac_policy.compareToAscii(
                RTL_CONSTASCII_STRINGPARAM("file:///") ))
        {
            // no file url
            OUString baseDir;
            if ( ::osl_getProcessWorkingDir( &baseDir.pData )
                 != osl_Process_E_None )
            {
                OSL_ASSERT( false );
            }
            OUString fileURL;
            if ( ::osl_getAbsoluteFileURL(
                     baseDir.pData, ac_policy.pData, &fileURL.pData )
                 != osl_File_E_None )
            {
                OSL_ASSERT( false );
            }
            ac_policy = fileURL;
        }

        entry.bLateInitService = false;
        entry.name =
            OUSTR("/implementations/com.sun.star.security.comp.stoc.FilePolicy/"
                  "file-name");
        entry.value <<= ac_policy;
        context_values.push_back( entry );
        // - policy singleton
        entry.bLateInitService = true;
        entry.name = OUSTR("/singletons/com.sun.star.security.thePolicy");
        entry.value <<= OUSTR("com.sun.star.security.comp.stoc.FilePolicy");
        context_values.push_back( entry );
    } // else policy singleton comes from storage

    OUString ac_mode;
    if (! bootstrap.getFrom( OUSTR("UNO_AC"), ac_mode ))
    {
        ac_mode = OUSTR("off"); // default
    }
    OUString ac_user;
    if (bootstrap.getFrom( OUSTR("UNO_AC_SINGLEUSER"), ac_user ))
    {
        // ac in single-user mode
        if (!ac_user.isEmpty())
        {
            // - ac prop: single-user-id
            entry.bLateInitService = false;
            entry.name =
                OUSTR("/services/com.sun.star.security.AccessController/"
                      "single-user-id");
            entry.value <<= ac_user;
            context_values.push_back( entry );
            if ( ac_mode != "single-user" )
            {
                throw SecurityException(
                    OUSTR("set UNO_AC=single-user "
                          "if you set UNO_AC_SINGLEUSER=<user-id>!"),
                    Reference< XInterface >() );
            }
        }
        else
        {
            if ( ac_mode != "single-default-user" )
            {
                throw SecurityException(
                    OUSTR("set UNO_AC=single-default-user "
                          "if you set UNO_AC_SINGLEUSER=<nothing>!"),
                    Reference< XInterface >() );
            }
        }
    }

    OUString ac_service;
    if (! bootstrap.getFrom( OUSTR("UNO_AC_SERVICE"), ac_service ))
    {
        // override service name
        ac_service = OUSTR("com.sun.star.security.AccessController"); // default
//          ac = OUSTR("com.sun.star.security.comp.stoc.AccessController");
    }

    // - ac prop: user-cache-size
    OUString ac_cache;
    if (bootstrap.getFrom( OUSTR("UNO_AC_USERCACHE_SIZE"), ac_cache ))
    {
        // ac cache size
        sal_Int32 n = ac_cache.toInt32();
        if (0 < n)
        {
            entry.bLateInitService = false;
            entry.name =
                OUSTR("/services/com.sun.star.security.AccessController/"
                      "user-cache-size");
            entry.value <<= n;
            context_values.push_back( entry );
        }
    }

    // - ac prop: mode
    // { "off", "on", "dynamic-only", "single-user", "single-default-user" }
    entry.bLateInitService = false;
    entry.name = OUSTR("/services/com.sun.star.security.AccessController/mode");
    entry.value <<= ac_mode;
    context_values.push_back( entry );
    // - ac singleton
    entry.bLateInitService = true;
    entry.name = OUSTR("/singletons/com.sun.star.security.theAccessController");
    entry.value <<= ac_service;
    context_values.push_back( entry );
}

namespace {
void addFactories(
    char const * const * ppNames /* implname, ..., 0 */,
    OUString const & bootstrapPath,
    Reference< lang::XMultiComponentFactory > const & xMgr,
    Reference< registry::XRegistryKey > const & xKey )
    SAL_THROW( (Exception) )
{
    Reference< container::XSet > xSet( xMgr, UNO_QUERY );
    OSL_ASSERT( xSet.is() );
    Reference< lang::XMultiServiceFactory > xSF( xMgr, UNO_QUERY );

    while (*ppNames)
    {
        OUString implName( OUString::createFromAscii( *ppNames++ ) );

        Any aFac( makeAny( loadSharedLibComponentFactory(
                               OUSTR("bootstrap.uno" SAL_DLLEXTENSION),
                               bootstrapPath, implName, xSF, xKey ) ) );
        xSet->insert( aFac );
#if OSL_DEBUG_LEVEL > 1
        if (xSet->has( aFac ))
        {
            Reference< lang::XServiceInfo > xInfo;
            if (aFac >>= xInfo)
            {
                ::fprintf(
                    stderr, "> implementation %s supports: ", ppNames[ -1 ] );
                Sequence< OUString > supported(
                    xInfo->getSupportedServiceNames() );
                for ( sal_Int32 nPos = supported.getLength(); nPos--; )
                {
                    OString str( OUStringToOString(
                        supported[ nPos ], RTL_TEXTENCODING_ASCII_US ) );
                    ::fprintf( stderr, nPos ? "%s, " : "%s\n", str.getStr() );
                }
            }
            else
            {
                ::fprintf(
                    stderr,
                    "> implementation %s provides NO lang::XServiceInfo!!!\n",
                    ppNames[ -1 ] );
            }
        }
#endif
#if OSL_DEBUG_LEVEL > 0
        if (! xSet->has( aFac ))
        {
            OStringBuffer buf( 64 );
            buf.append( "### failed inserting shared lib \"" );
            buf.append( "bootstrap.uno" SAL_DLLEXTENSION );
            buf.append( "\"!!!" );
            OString str( buf.makeStringAndClear() );
            OSL_FAIL( str.getStr() );
        }
#endif
    }
}

} // namespace

Reference< lang::XMultiComponentFactory > bootstrapInitialSF(
    OUString const & rBootstrapPath )
    SAL_THROW( (Exception) )
{
    OUString const & bootstrap_path =
        rBootstrapPath.isEmpty() ? get_this_libpath() : rBootstrapPath;

    Reference< lang::XMultiComponentFactory > xMgr(
        createInstance(
            loadSharedLibComponentFactory(
                OUSTR("bootstrap.uno" SAL_DLLEXTENSION), bootstrap_path,
                OUSTR("com.sun.star.comp.stoc.ORegistryServiceManager"),
                Reference< lang::XMultiServiceFactory >(),
                Reference< registry::XRegistryKey >() ) ),
        UNO_QUERY );

    // add initial bootstrap services
    static char const * ar[] = {
        "com.sun.star.comp.stoc.OServiceManagerWrapper",
        "com.sun.star.comp.stoc.DLLComponentLoader",
        "com.sun.star.comp.stoc.SimpleRegistry",
        "com.sun.star.comp.stoc.NestedRegistry",
        "com.sun.star.comp.stoc.TypeDescriptionManager",
        "com.sun.star.comp.stoc.ImplementationRegistration",
        "com.sun.star.security.comp.stoc.AccessController",
        "com.sun.star.security.comp.stoc.FilePolicy",
        0
    };
    addFactories(
        ar, bootstrap_path,
        xMgr, Reference< registry::XRegistryKey >() );

    return xMgr;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
