/*************************************************************************
 *
 *  $RCSfile: servicefactory.cxx,v $
 *
 *  $Revision: 1.36 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-23 16:29:21 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#if OSL_DEBUG_LEVEL > 0
#include <stdio.h>
#endif
#include <vector>

#include "rtl/string.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/bootstrap.hxx"
#include "osl/diagnose.h"
#include "osl/file.h"
#include "osl/module.h"
#include "osl/process.h"
#include "cppuhelper/shlib.hxx"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/component_context.hxx"
#include "cppuhelper/servicefactory.hxx"
#include "cppuhelper/bootstrap.hxx"

#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/lang/XSingleServiceFactory.hpp"
#include "com/sun/star/lang/XSingleComponentFactory.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/container/XSet.hpp"
#include "com/sun/star/container/XHierarchicalNameAccess.hpp"
#include "com/sun/star/registry/XSimpleRegistry.hpp"
#include "com/sun/star/registry/XImplementationRegistration.hpp"
#include "com/sun/star/security/XAccessController.hpp"

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace cppu
{

// private forward decl
void addFactories(
    char const * const * ppNames /* lib, implname, ..., 0 */,
    OUString const & bootstrapPath,
    Reference< lang::XMultiComponentFactory > const & xMgr,
    Reference< registry::XRegistryKey > const & xKey )
    SAL_THROW( (Exception) );

Reference< security::XAccessController >
createDefaultAccessController() SAL_THROW( () );

Reference< lang::XSingleComponentFactory >
create_boostrap_macro_expander_factory() SAL_THROW( () );

OUString const & get_this_libpath();


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
        Reference< lang::XSingleServiceFactory > xFac( xFactory, UNO_QUERY );
        if (xFac.is())
        {
            OSL_ENSURE( !xContext.is(), "### ignoring context!" );
            return xFac->createInstance();
        }
    }
    throw RuntimeException(
        OUSTR("no factory object given!"),
        Reference< XInterface >() );
}

Reference< registry::XSimpleRegistry > SAL_CALL createSimpleRegistry(
    OUString const & rBootstrapPath )
    SAL_THROW( () )
{
    try
    {
        return Reference< registry::XSimpleRegistry >(
            createInstance(
                loadSharedLibComponentFactory(
                    OUSTR("simplereg.uno" SAL_DLLEXTENSION),
                    0 == rBootstrapPath.getLength()
                    ? get_this_libpath() : rBootstrapPath,
                    OUSTR("com.sun.star.comp.stoc.SimpleRegistry"),
                    Reference< lang::XMultiServiceFactory >(),
                    Reference< registry::XRegistryKey >() ) ),
            UNO_QUERY );
    }
    catch (Exception & exc)
    {
#if OSL_DEBUG_LEVEL > 0
        OString cstr_msg(
            OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
        OSL_ENSURE( !"### exception occured:", cstr_msg.getStr() );
#endif
    }

    return Reference< registry::XSimpleRegistry >();
}

Reference< registry::XSimpleRegistry > SAL_CALL createNestedRegistry(
    OUString const & rBootstrapPath )
    SAL_THROW( () )
{
    try
    {
        return Reference< registry::XSimpleRegistry >(
            createInstance(
                loadSharedLibComponentFactory(
                    OUSTR("nestedreg.uno" SAL_DLLEXTENSION),
                    0 == rBootstrapPath.getLength()
                    ? get_this_libpath() : rBootstrapPath,
                    OUSTR("com.sun.star.comp.stoc.NestedRegistry"),
                    Reference< lang::XMultiServiceFactory >(),
                    Reference< registry::XRegistryKey >() ) ),
            UNO_QUERY );
    }
    catch (Exception & exc)
    {
#if OSL_DEBUG_LEVEL > 0
        OString cstr_msg(
            OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
        OSL_ENSURE( !"### exception occured:", cstr_msg.getStr() );
#endif
    }

    return Reference< registry::XSimpleRegistry >();
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
static void add_access_control_entries(
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
            oslProcessError prc = ::osl_getProcessWorkingDir(
                &baseDir.pData );
            OSL_ASSERT( osl_Process_E_None == prc );
            OUString fileURL;
            oslFileError frc = ::osl_getAbsoluteFileURL(
                baseDir.pData, ac_policy.pData, &fileURL.pData );
            OSL_ASSERT( osl_File_E_None == frc );
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
        if (ac_user.getLength())
        {
            // - ac prop: single-user-id
            entry.bLateInitService = false;
            entry.name =
                OUSTR("/services/com.sun.star.security.AccessController/"
                      "single-user-id");
            entry.value <<= ac_user;
            context_values.push_back( entry );
            if (! ac_mode.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM("single-user") ))
            {
                throw SecurityException(
                    OUSTR("set UNO_AC=single-user "
                          "if you set UNO_AC_SINGLEUSER=<user-id>!"),
                    Reference< XInterface >() );
            }
        }
        else
        {
            if (! ac_mode.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM("single-default-user") ))
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

Reference< lang::XMultiComponentFactory > bootstrapInitialSF(
    OUString const & rBootstrapPath )
    SAL_THROW( (Exception) )
{
    OUString const & bootstrap_path =
        0 == rBootstrapPath.getLength() ? get_this_libpath() : rBootstrapPath;

    Reference< lang::XMultiComponentFactory > xMgr(
        createInstance(
            loadSharedLibComponentFactory(
                OUSTR("servicemgr.uno" SAL_DLLEXTENSION), bootstrap_path,
                OUSTR("com.sun.star.comp.stoc.ORegistryServiceManager"),
                Reference< lang::XMultiServiceFactory >(),
                Reference< registry::XRegistryKey >() ) ),
        UNO_QUERY );

    // add initial bootstrap services
    static char const * ar[] = {
        "servicemgr.uno" SAL_DLLEXTENSION,
        "com.sun.star.comp.stoc.OServiceManagerWrapper",
        "shlibloader.uno" SAL_DLLEXTENSION,
        "com.sun.star.comp.stoc.DLLComponentLoader",
        "simplereg.uno" SAL_DLLEXTENSION,
        "com.sun.star.comp.stoc.SimpleRegistry",
        "nestedreg.uno" SAL_DLLEXTENSION,
        "com.sun.star.comp.stoc.NestedRegistry",
        "typemgr.uno" SAL_DLLEXTENSION,
        "com.sun.star.comp.stoc.TypeDescriptionManager",
        "implreg.uno" SAL_DLLEXTENSION,
        "com.sun.star.comp.stoc.ImplementationRegistration",
        "security.uno" SAL_DLLEXTENSION,
        "com.sun.star.security.comp.stoc.AccessController",
        "security.uno" SAL_DLLEXTENSION,
        "com.sun.star.security.comp.stoc.FilePolicy",
        0
    };
    addFactories(
        ar, bootstrap_path,
        xMgr, Reference< registry::XRegistryKey >() );

    return xMgr;
}

// returns context with UNinitialized smgr
Reference< XComponentContext > bootstrapInitialContext(
    Reference< lang::XMultiComponentFactory > const & xSF,
    Reference< registry::XSimpleRegistry > const & types_xRegistry,
    Reference< registry::XSimpleRegistry > const & services_xRegistry,
    OUString const & rBootstrapPath, Bootstrap const & bootstrap )
    SAL_THROW( (Exception) )
{
    Reference< lang::XInitialization > xSFInit( xSF, UNO_QUERY );
    if (! xSFInit.is())
    {
        throw RuntimeException(
            OUSTR("servicemanager does not support XInitialization!"),
            Reference< XInterface >() );
    }

    // basic context values
    ContextEntry_Init entry;
    ::std::vector< ContextEntry_Init > context_values;
    context_values.reserve( 14 );

    // macro expander singleton for loader
    entry.bLateInitService = true;
    entry.name = OUSTR("/singletons/com.sun.star.util.theMacroExpander");
    entry.value <<= create_boostrap_macro_expander_factory();
    context_values.push_back( entry );

    // tdmgr singleton
    entry.bLateInitService = true;
    entry.name =
        OUSTR("/singletons/com.sun.star.reflection.theTypeDescriptionManager");
    entry.value <<= OUSTR("com.sun.star.comp.stoc.TypeDescriptionManager");
    context_values.push_back( entry );

    // read out singleton infos from registry
    if (services_xRegistry.is())
    {
        Reference< registry::XRegistryKey > xKey(
            services_xRegistry->getRootKey() );
        if (xKey.is())
        {
            xKey = xKey->openKey( OUSTR("/SINGLETONS") );
            if (xKey.is())
            {
                entry.bLateInitService = true;

                Sequence< Reference< registry::XRegistryKey > > keys(
                    xKey->openKeys() );
                Reference< registry::XRegistryKey > const * pKeys =
                    keys.getConstArray();
                for ( sal_Int32 nPos = keys.getLength(); nPos--; )
                {
                    try
                    {
                        Reference< registry::XRegistryKey > const & xKey =
                            pKeys[ nPos ];

                        OUStringBuffer buf( 32 );
                        buf.appendAscii(
                            RTL_CONSTASCII_STRINGPARAM("/singletons/") );
                        buf.append(
                            xKey->getKeyName().copy(
                                sizeof("/SINGLETONS") /* -\0 +'/' */ ) );
                        entry.name = buf.makeStringAndClear();
                        entry.value <<= xKey->getStringValue();
                        context_values.push_back( entry );
                    }
                    catch (Exception & rExc)
                    {
#if OSL_DEBUG_LEVEL > 0
                        OString aStr(
                            OUStringToOString(
                                xKey->getKeyName().copy( 11 ),
                                RTL_TEXTENCODING_ASCII_US ) );
                        OString aStr2(
                            OUStringToOString(
                                rExc.Message, RTL_TEXTENCODING_ASCII_US ) );
                        fprintf(
                            stderr,
                            "### failed reading singleton [%s]"
                            " service name from registry: %s\n",
                            aStr.getStr(), aStr2.getStr() );
#endif
                    }
                }
            }
        }
    }

    // ac, policy:
    add_access_control_entries( &context_values, bootstrap );

    // smgr singleton
    entry.bLateInitService = false;
    entry.name = OUSTR("/singletons/com.sun.star.lang.theServiceManager");
    entry.value <<= xSF;
    context_values.push_back( entry );

    Reference< XComponentContext > xContext(
        createComponentContext(
            &context_values[ 0 ], context_values.size(),
            Reference< XComponentContext >() ) );
    // set default context
    Reference< beans::XPropertySet > xProps( xSF, UNO_QUERY );
    OSL_ASSERT( xProps.is() );
    if (xProps.is())
    {
        xProps->setPropertyValue(
            OUSTR("DefaultContext"), makeAny( xContext ) );
    }

    Reference< container::XHierarchicalNameAccess > xTDMgr;

    // get tdmgr singleton
    if (xContext->getValueByName(
            OUSTR("/singletons/"
                  "com.sun.star.reflection.theTypeDescriptionManager") )
        >>= xTDMgr)
    {
        if (types_xRegistry.is()) // insert rdb provider?
        {
            // add registry td provider factory to smgr and instance to tdmgr
            Reference< lang::XSingleComponentFactory > xFac(
                loadSharedLibComponentFactory(
                    OUSTR("regtypeprov.uno" SAL_DLLEXTENSION),
                    0 == rBootstrapPath.getLength()
                    ? get_this_libpath() : rBootstrapPath,
                OUSTR("com.sun.star.comp.stoc.RegistryTypeDescriptionProvider"),
                Reference< lang::XMultiServiceFactory >( xSF, UNO_QUERY ),
                Reference< registry::XRegistryKey >() ), UNO_QUERY );
            OSL_ASSERT( xFac.is() );

            // smgr
            Reference< container::XSet > xSet( xSF, UNO_QUERY );
            xSet->insert( makeAny( xFac ) );
            OSL_ENSURE(
                xSet->has( makeAny( xFac ) ),
                "### failed registering registry td provider at smgr!" );
            // tdmgr
            xSet.set( xTDMgr, UNO_QUERY );
            OSL_ASSERT( xSet.is() );
            Any types_RDB( makeAny( types_xRegistry ) );
            Any rdbtdp( makeAny( xFac->createInstanceWithArgumentsAndContext(
                Sequence< Any >( &types_RDB, 1 ), xContext ) ) );
            xSet->insert( rdbtdp );
            OSL_ENSURE(
                xSet->has( rdbtdp ),
                "### failed inserting registry td provider to tdmgr!" );
        }
        // install callback
        installTypeDescriptionManager( xTDMgr );
    }

    return xContext;
}

static Reference< lang::XMultiComponentFactory > createImplServiceFactory(
    const OUString & rWriteRegistry,
    const OUString & rReadRegistry,
    sal_Bool bReadOnly,
    const OUString & rBootstrapPath )
    SAL_THROW( (Exception) )
{
    Reference< lang::XMultiComponentFactory > xSF(
        bootstrapInitialSF( rBootstrapPath ) );

    Reference< registry::XSimpleRegistry > xRegistry;

    // open a registry
    sal_Bool bRegistryShouldBeValid = sal_False;
    if (rWriteRegistry.getLength() && !rReadRegistry.getLength())
    {
        bRegistryShouldBeValid = sal_True;
        xRegistry.set( createSimpleRegistry( rBootstrapPath ) );
        if (xRegistry.is())
        {
            if (bReadOnly)
            {
                xRegistry->open( rWriteRegistry, sal_True, sal_False );
            }
            else
            {
                xRegistry->open( rWriteRegistry, sal_False, sal_True );
            }
        }
    }
    else if (rWriteRegistry.getLength() && rReadRegistry.getLength())
    {
        // default registry
        bRegistryShouldBeValid = sal_True;
        xRegistry.set( createNestedRegistry( rBootstrapPath ) );

        Reference< registry::XSimpleRegistry > xWriteReg(
            createSimpleRegistry( rBootstrapPath ) );
        if (xWriteReg.is())
        {
            if (bReadOnly)
            {
                try
                {
                    xWriteReg->open( rWriteRegistry, sal_True, sal_False );
                }
                catch (registry::InvalidRegistryException &)
                {
                }

                if (! xWriteReg->isValid())
                {
                    throw RuntimeException(
                        OUSTR("specified first registry "
                              "could not be open readonly!"),
                        Reference< XInterface >() );
                }
            }
            else
            {
                xWriteReg->open( rWriteRegistry, sal_False, sal_True );
            }
        }

        Reference< registry::XSimpleRegistry > xReadReg(
            createSimpleRegistry( rBootstrapPath ) );
        if (xReadReg.is())
        {
            xReadReg->open( rReadRegistry, sal_True, sal_False );
        }

        Reference< lang::XInitialization > xInit( xRegistry, UNO_QUERY );
        Sequence< Any > aInitSeq( 2 );
        aInitSeq[ 0 ] <<= xWriteReg;
        aInitSeq[ 1 ] <<= xReadReg;
        xInit->initialize( aInitSeq );
    }

    if (bRegistryShouldBeValid && (!xRegistry.is() || !xRegistry->isValid()))
    {
        throw RuntimeException(
            OUSTR("specified registry could not be initialized"),
            Reference< XInterface >() );
    }

    Bootstrap bootstrap;
    Reference< XComponentContext > xContext(
        bootstrapInitialContext(
            xSF, xRegistry, xRegistry, rBootstrapPath, bootstrap ) );

    // initialize sf
    Reference< lang::XInitialization > xInit( xSF, UNO_QUERY );
    OSL_ASSERT( xInit.is() );
    Sequence< Any > aSFInit( 1 );
    aSFInit[ 0 ] <<= xRegistry;
    xInit->initialize( aSFInit );

    return xSF;
}

Reference< lang::XMultiServiceFactory > SAL_CALL createRegistryServiceFactory(
    const OUString & rWriteRegistry,
    const OUString & rReadRegistry,
    sal_Bool bReadOnly,
    const OUString & rBootstrapPath )
    SAL_THROW( (Exception) )
{
    return Reference< lang::XMultiServiceFactory >( createImplServiceFactory(
        rWriteRegistry, rReadRegistry, bReadOnly, rBootstrapPath ), UNO_QUERY );
}

Reference< XComponentContext > SAL_CALL bootstrap_InitialComponentContext(
    Reference< registry::XSimpleRegistry > const & xRegistry,
    OUString const & rBootstrapPath )
    SAL_THROW( (Exception) )
{
    Bootstrap bootstrap;

    Reference< lang::XMultiComponentFactory > xSF(
        bootstrapInitialSF( rBootstrapPath ) );
    Reference< XComponentContext > xContext(
        bootstrapInitialContext(
            xSF, xRegistry, xRegistry, rBootstrapPath, bootstrap ) );

    // initialize sf
    Reference< lang::XInitialization > xInit( xSF, UNO_QUERY );
    OSL_ASSERT( xInit.is() );
    Sequence< Any > aSFInit( 2 );
    aSFInit[ 0 ] <<= xRegistry;
    aSFInit[ 1 ] <<= xContext; // default context
    xInit->initialize( aSFInit );

    return xContext;
}

}
