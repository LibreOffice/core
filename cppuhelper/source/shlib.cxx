/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#include "osl/diagnose.h"
#include "osl/file.hxx"
#include "osl/mutex.hxx"
#include "osl/module.hxx"
#include "rtl/unload.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/instance.hxx"
#include "uno/environment.h"
#include "uno/mapping.hxx"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/shlib.hxx"

#include "com/sun/star/beans/XPropertySet.hpp"

#include <stdio.h>
#include <vector>

#ifdef ANDROID
#include <osl/detail/android-bootstrap.h>
#endif

#ifdef IOS
#include <osl/detail/ios-bootstrap.h>
#endif

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace cppu
{

#if OSL_DEBUG_LEVEL > 1
//------------------------------------------------------------------------------
static inline void out( const char * p ) SAL_THROW(())
{
    printf( "%s\n", p );
}
static inline void out( const OUString & r ) throw ()
{
    OString s( OUStringToOString( r, RTL_TEXTENCODING_ASCII_US ) );
    out( s.getStr() );
}
#endif

namespace
{
    class buildAccessDPath
    {
    private:
        ::std::vector< OUString > m_aAccessDPath;
        bool m_bCPLD_ACCESSPATHSet;
    public:
        buildAccessDPath() : m_bCPLD_ACCESSPATHSet(false)
        {
            const char * pEnv = ::getenv( "CPLD_ACCESSPATH" );
            if (pEnv)
            {
                m_bCPLD_ACCESSPATHSet = true;

                OString aEnv( pEnv );
                sal_Int32 nIndex = 0;
                do
                {
                    OUString aStr( OStringToOUString(
                        aEnv.getToken( 0, ';', nIndex ),
                        RTL_TEXTENCODING_ASCII_US ) );
                    OUString aFileUrl;
                    if (FileBase::getFileURLFromSystemPath(aStr, aFileUrl)
                        != FileBase::E_None)
                    {
                        OSL_ASSERT(false);
                    }
                    m_aAccessDPath.push_back( aFileUrl );
                } while( nIndex != -1 );
    #if OSL_G_LEVEL > 1
                out( "> cpld: acknowledged following access path(s): \"" );
                ::std::vector< OUString >::const_iterator iPos( m_aAccessDPath.begin() );
                while (iPos != m_aAccessDPath.end())
                {
                    out( *iPos );
                    ++iPos;
                    if (iPos != m_aAccessDPath.end())
                        out( ";" );
                }
                out( "\"\n" );
    #endif
            }
            else
            {
                // no access path env set
    #if OSL_G_LEVEL > 1
                out( "=> no CPLD_ACCESSPATH set.\n" );
    #endif
            }
        }
        ::std::vector< OUString >* getAccessDPath() { return m_bCPLD_ACCESSPATHSet ? &m_aAccessDPath : NULL; }
    };

    class theAccessDPath : public rtl::Static<buildAccessDPath, theAccessDPath> {};
}

#ifndef DISABLE_DYNLOADING

static const ::std::vector< OUString > * getAccessDPath() SAL_THROW(())
{
    return theAccessDPath::get().getAccessDPath();
}

//------------------------------------------------------------------------------
static bool checkAccessPath( OUString * pComp ) throw ()
{
    const ::std::vector< OUString > * pPath = getAccessDPath();

    if (pPath)
    {
        sal_Bool bAbsolute = (pComp->compareToAscii( "file://" , 7 ) == 0);
        for ( ::std::vector< OUString >::const_iterator iPos( pPath->begin() );
              iPos != pPath->end(); ++iPos )
        {
            OUString aBaseDir( *iPos );
            OUString aAbs;

            if ( bAbsolute )
            {
                aAbs = *pComp;
#if OSL_DEBUG_LEVEL > 1
                out( "> taking path: \"" );
                out( aAbs );
#endif
            }
            else
            {
                if (osl_File_E_None !=
                    ::osl_getAbsoluteFileURL(
                        aBaseDir.pData, pComp->pData, &aAbs.pData ))
                {
                    continue;
                }
#if OSL_DEBUG_LEVEL > 1
                out( "> found path: \"" );
                out( aBaseDir );
                out( "\" + \"" );
                out( *pComp );
                out( "\" => \"" );
                out( aAbs );
#endif
            }

            if (0 == aAbs.indexOf( aBaseDir ) && // still part of it?
                aBaseDir.getLength() < aAbs.getLength() &&
                (aBaseDir[ aBaseDir.getLength() -1 ] == (sal_Unicode)'/' ||
                 // dir boundary
                 aAbs[ aBaseDir.getLength() ] == (sal_Unicode)'/'))
            {
#if OSL_DEBUG_LEVEL > 1
                out( ": ok.\n" );
#endif
                // load from absolute path
                *pComp = aAbs;
                return true;
            }
#if OSL_DEBUG_LEVEL > 1
            else
            {
                out( "\" ...does not match given path \"" );
                out( aBaseDir );
                out( "\".\n" );
            }
#endif
        }
        return false;
    }
    else
    {
        // no access path env set
        return true;
    }
}

//------------------------------------------------------------------------------
static OUString makeComponentPath(
    const OUString & rLibName, const OUString & rPath )
{
#if OSL_DEBUG_LEVEL > 0
    // No system path allowed here !
    {
        OUString aComp;
        OSL_ASSERT( FileBase::E_None ==
                    FileBase::getSystemPathFromFileURL( rLibName, aComp ) );
        OSL_ASSERT(
            rPath.isEmpty() ||
            FileBase::E_None ==
              FileBase::getSystemPathFromFileURL( rPath, aComp ) );
    }
#endif

    OUStringBuffer buf( rPath.getLength() + rLibName.getLength() + 12 );

    if (!rPath.isEmpty())
    {
        buf.append( rPath );
        if (rPath[ rPath.getLength() -1 ] != '/')
            buf.append( (sal_Unicode) '/' );
    }
    if (! rLibName.endsWithIgnoreAsciiCase( OUSTR(SAL_DLLEXTENSION) ))
    {
#if defined SAL_DLLPREFIX
        if (! rLibName.endsWithIgnoreAsciiCase( OUSTR(".uno") ))
        {
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(SAL_DLLPREFIX) );
        }
#endif
        buf.append( rLibName );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(SAL_DLLEXTENSION) );
    }
    else // name is completely pre/postfixed
    {
        buf.append( rLibName );
    }

    OUString out( buf.makeStringAndClear() );
#if OSL_DEBUG_LEVEL > 1
    OString str( OUStringToOString( out, RTL_TEXTENCODING_ASCII_US ) );
    OSL_TRACE(OSL_LOG_PREFIX "component path=%s", str.getStr());
#endif

    return out;
}

//==============================================================================
static void getLibEnv(oslModule                lib,
                      uno::Environment       * pEnv,
                      OUString               * pSourceEnv_name,
                      uno::Environment const & cTargetEnv,
                      OUString         const & cImplName = OUString(),
                      OUString         const & rPrefix = OUString())
{
    sal_Char const * pEnvTypeName = NULL;

    OUString aGetEnvNameExt = rPrefix + OUSTR(COMPONENT_GETENVEXT);
    component_getImplementationEnvironmentExtFunc pGetImplEnvExt =
        (component_getImplementationEnvironmentExtFunc)osl_getFunctionSymbol(lib, aGetEnvNameExt.pData);

    if (pGetImplEnvExt)
    {
        OString implName(OUStringToOString(cImplName, RTL_TEXTENCODING_ASCII_US));
        pGetImplEnvExt(&pEnvTypeName, (uno_Environment **)pEnv, implName.getStr(), cTargetEnv.get());
    }
    else
    {
        OUString aGetEnvName = rPrefix + OUSTR(COMPONENT_GETENV);
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
        const char * pUNO_ENV_LOG = ::getenv( "UNO_ENV_LOG" );
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
                    *pSourceEnv_name += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(":log"));
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
    OUString const & rLibName, OUString const & rPath,
    OUString const & rImplName,
    Reference< lang::XMultiServiceFactory > const & xMgr,
    Reference< registry::XRegistryKey > const & xKey )
    SAL_THROW( (loader::CannotActivateFactoryException) )
{
    return loadSharedLibComponentFactory( rLibName, rPath, rImplName, xMgr, xKey, rtl::OUString() );
}

namespace
{

Reference< XInterface > invokeComponentFactory(
    oslGenericFunction pGetter,
    oslModule lib,
    OUString const & rModulePath,
    OUString const & rImplName,
    Reference< ::com::sun::star::lang::XMultiServiceFactory > const & xMgr,
    Reference< ::com::sun::star::registry::XRegistryKey > const & xKey,
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
    // neither of which make sense on iOS (which is the only platform
    // for which DISABLE_DYNLOADING is intended, really). So we can
    // simoly bypass the getLibEnv() stuff and don't need to wonder
    // how to find out what function to call at this point if
    // statically linked.
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
            void * pKey = aCurrent2Env.mapInterface(
                xKey.get(), ::getCppuType( &xKey ) );

            void * pSSF = NULL;

            env.invoke(s_getFactory, pGetter, &aImplName, pSMgr, pKey, &pSSF);

            if (pKey)
            {
                (env.get()->pExtEnv->releaseInterface)(
                    env.get()->pExtEnv, pKey );
            }
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
                rExcMsg = rModulePath;
                rExcMsg += OUSTR(": cannot get factory of "
                                 "demanded implementation: ");
                rExcMsg += OStringToOUString(
                        aImplName, RTL_TEXTENCODING_ASCII_US );
            }
        }
        else
        {
            rExcMsg =
                OUSTR("cannot get uno mappings: C++ <=> UNO!");
        }
    }
    else
    {
        rExcMsg = OUSTR("cannot get uno environments!");
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
    extern void * expwrap_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * fastsax_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * filterconfig1_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * fwk_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * introspection_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * package2_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * reflection_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * sfx_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * svl_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * tk_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * stocservices_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * i18npool_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * ucb_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * ucpfile_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * utl_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * vcl_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * xstor_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );
}
#endif

Reference< XInterface > SAL_CALL loadSharedLibComponentFactory(
    OUString const & rLibName, OUString const & rPath,
    OUString const & rImplName,
    Reference< lang::XMultiServiceFactory > const & xMgr,
    Reference< registry::XRegistryKey > const & xKey,
    OUString const & rPrefix )
    SAL_THROW( (loader::CannotActivateFactoryException) )
{
#ifndef DISABLE_DYNLOADING
    OUString sLibName(rLibName);

#ifdef ANDROID
    if ( rLibName.equals( OUSTR("bootstrap.uno" SAL_DLLEXTENSION) ) )
        sLibName = OUSTR("libbootstrap.uno" SAL_DLLEXTENSION);
#endif

    OUString aModulePath( makeComponentPath( sLibName, rPath ) );
    if (! checkAccessPath( &aModulePath ))
    {
        throw loader::CannotActivateFactoryException(
            OUSTR("permission denied to load component library: ") +
            aModulePath,
            Reference< XInterface >() );
    }

    oslModule lib = osl_loadModule(
        aModulePath.pData, SAL_LOADMODULE_LAZY | SAL_LOADMODULE_GLOBAL );
    if (! lib)
    {
        throw loader::CannotActivateFactoryException(
            OUSTR("loading component library failed: ") + aModulePath,
            Reference< XInterface >() );
    }
#else
    (void) rPath;
    oslModule lib;
    OUString aModulePath(OUSTR("MAIN"));
    if (! osl_getModuleHandle( NULL, &lib))
    {
        throw loader::CannotActivateFactoryException(
            OUSTR("osl_getModuleHandle of the executable: "),
            Reference< XInterface >() );
    }
#endif

    Reference< XInterface > xRet;

    OUString aExcMsg;

    OUString aGetFactoryName = rPrefix + OUSTR(COMPONENT_GETFACTORY);

    oslGenericFunction pSym = NULL;

#ifdef DISABLE_DYNLOADING

    // First test library names that aren't app-specific.
    static lib_to_component_mapping non_app_specific_map[] = {
        { "bootstrap.uno" SAL_DLLEXTENSION, bootstrap_component_getFactory },
        { "bootstrap.uno.a", bootstrap_component_getFactory },
        { "configmgr.uno.a", configmgr_component_getFactory },
        { "i18npool.uno.a", i18npool_component_getFactory },
        { "introspection.uno.a", introspection_component_getFactory },
        { "libcomphelp" CPPU_STRINGIFY(CPPU_ENV) ".a", comphelp_component_getFactory },
        { "libexpwrap.uno.a", expwrap_component_getFactory },
        { "libfastsax.uno.a", fastsax_component_getFactory },
        { "libfilterconfiglo.a", filterconfig1_component_getFactory },
        { "libfwklo.a", fwk_component_getFactory },
        { "libpackage2.a", package2_component_getFactory },
        { "libsfxlo.a", sfx_component_getFactory },
        { "libsvllo.a", svl_component_getFactory },
        { "libtklo.a", tk_component_getFactory },
        { "libucb1.a", ucb_component_getFactory },
        { "libucpfile1.a", ucpfile_component_getFactory },
        { "libutllo.a", utl_component_getFactory },
        { "libvcllo.a", vcl_component_getFactory },
        { "libxstor.a", xstor_component_getFactory },
        { "reflection.uno.a", reflection_component_getFactory },
        { "stocservices.uno.a", stocservices_component_getFactory },
        { NULL, NULL }
    };
    for (int i = 0; pSym == NULL && non_app_specific_map[i].lib != NULL; ++i)
    {
        if ( rLibName.equalsAscii( non_app_specific_map[i].lib ) )
            pSym = (oslGenericFunction) non_app_specific_map[i].component_getFactory_function;
    }

    if ( pSym == NULL)
    {
        // The call the app-specific lo_get_libmap() to get a mapping for the rest
        const lib_to_component_mapping *map = lo_get_libmap();
        for (int i = 0; pSym == NULL && map[i].lib != NULL; ++i)
        {
            if ( rLibName.equalsAscii( map[i].lib ) )
                pSym = (oslGenericFunction) map[i].component_getFactory_function;
        }
        if ( pSym == NULL )
        {
            fprintf( stderr, "attempting to load unknown library %s\n", OUStringToOString( rLibName, RTL_TEXTENCODING_ASCII_US ).getStr() );
            assert( !"Attempt to load unknown library" );
        }
    }
#else

    if ( pSym == NULL )
        pSym = osl_getFunctionSymbol( lib, aGetFactoryName.pData );
#endif

    if (pSym != 0)
    {
        xRet = invokeComponentFactory( pSym, lib, aModulePath, rImplName, xMgr, xKey, rPrefix, aExcMsg );
    }
    else
    {
        aExcMsg = aModulePath;
        aExcMsg += OUSTR(": cannot get symbol: ");
        aExcMsg += aGetFactoryName;
    }

    if (! xRet.is())
    {
#ifndef DISABLE_DYNLOADING
        osl_unloadModule( lib );
#endif
#if OSL_DEBUG_LEVEL > 1
        out( "### cannot activate factory: " );
        out( aExcMsg );
        out( "\n" );
#endif
        throw loader::CannotActivateFactoryException(
            aExcMsg,
            Reference< XInterface >() );
    }

    rtl_registerModuleForUnloading( lib);
    return xRet;
}

Reference< XInterface > SAL_CALL invokeStaticComponentFactory(
    oslGenericFunction pGetter,
    OUString const & rImplName,
    Reference< ::com::sun::star::lang::XMultiServiceFactory > const & xMgr,
    Reference< ::com::sun::star::registry::XRegistryKey > const & xKey,
    OUString const & rPrefix )
    SAL_THROW( (::com::sun::star::loader::CannotActivateFactoryException) )
{
    Reference< XInterface > xRet;
    oslModule pExe;
    OUString aExePath(OUSTR("MAIN"));
    osl_getModuleHandle( NULL, &pExe );
    OUString aExcMsg;

    xRet = invokeComponentFactory( pGetter, pExe, aExePath, rImplName, xMgr, xKey, rPrefix, aExcMsg );

    if (! xRet.is())
    {
#if OSL_DEBUG_LEVEL > 1
        out( "### cannot activate factory: " );
        out( aExcMsg );
        out( "\n" );
#endif
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
    OUString const & rLibName, OUString const & rPath,
    Reference< lang::XMultiServiceFactory > const & xMgr,
    Reference< registry::XRegistryKey > const & xKey )
    SAL_THROW( (registry::CannotRegisterImplementationException) )
{
    OUString aModulePath( makeComponentPath( rLibName, rPath ) );

    if (! checkAccessPath( &aModulePath ))
    {
        throw registry::CannotRegisterImplementationException(
            OUSTR("permission denied to load component library: ") +
            aModulePath,
            Reference< XInterface >() );
    }

    oslModule lib = osl_loadModule(
        aModulePath.pData, SAL_LOADMODULE_LAZY | SAL_LOADMODULE_GLOBAL );
    if (! lib)
    {
        throw registry::CannotRegisterImplementationException(
            OUSTR("loading component library failed: ") + aModulePath,
            Reference< XInterface >() );
    }

    sal_Bool bRet = sal_False;

    uno::Environment currentEnv(Environment::getCurrent());
    uno::Environment env;

    OUString aEnvTypeName;
    OUString aExcMsg;

    getLibEnv(lib, &env, &aEnvTypeName, currentEnv);

    OUString aWriteInfoName = OUSTR(COMPONENT_WRITEINFO);
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
                        aExcMsg = aModulePath;
                        aExcMsg += OUSTR(": component_writeInfo() "
                                         "returned false!");
                    }
                }
                else
                {
                    // key is mandatory
                    aExcMsg = aModulePath;
                    aExcMsg += OUSTR(": registry is mandatory to invoke"
                                     " component_writeInfo()!");
                }

                if (pSMgr)
                {
                    (*env.get()->pExtEnv->releaseInterface)(
                        env.get()->pExtEnv, pSMgr );
                }
            }
            else
            {
                aExcMsg = OUSTR("cannot get uno mapping: C++ <=> UNO!");
            }
        }
        else
        {
            aExcMsg = OUSTR("cannot get uno environments!");
        }
    }
    else
    {
        aExcMsg = aModulePath;
        aExcMsg += OUSTR(": cannot get symbol: ");
        aExcMsg += aWriteInfoName;
    }

//!
//! OK: please look at #88219#
//!
//! ::osl_unloadModule( lib);
    if (! bRet)
    {
#if OSL_DEBUG_LEVEL > 1
        out( "### cannot write component info: " );
        out( aExcMsg );
        out( "\n" );
#endif
        throw registry::CannotRegisterImplementationException(
            aExcMsg, Reference< XInterface >() );
    }
}

#endif // DISABLE_DYNLOADING

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
