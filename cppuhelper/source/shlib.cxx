/*************************************************************************
 *
 *  $RCSfile: shlib.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 10:54:36 $
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

#include "osl/diagnose.h"
#include "osl/file.hxx"
#include "osl/mutex.hxx"
#include "osl/module.hxx"
#include "rtl/unload.h"
#include "rtl/ustrbuf.hxx"
#include "uno/environment.h"
#include "uno/mapping.hxx"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/shlib.hxx"

#include "com/sun/star/beans/XPropertySet.hpp"

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif
#include <vector>

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace cppu
{

#if OSL_DEBUG_LEVEL > 1
//------------------------------------------------------------------------------
static inline void out( const char * p ) SAL_THROW( () )
{
    printf( p );
}
static inline void out( const OUString & r ) throw ()
{
    OString s( OUStringToOString( r, RTL_TEXTENCODING_ASCII_US ) );
    out( s.getStr() );
}
#endif

//------------------------------------------------------------------------------
static const ::std::vector< OUString > * getAccessDPath() SAL_THROW( () )
{
    static ::std::vector< OUString > * s_p = 0;
    static bool s_bInit = false;

    if (! s_bInit)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! s_bInit)
        {
            const char * pEnv = ::getenv( "CPLD_ACCESSPATH" );
            if (pEnv)
            {
                static ::std::vector< OUString > s_v;

                OString aEnv( pEnv );
                sal_Int32 nIndex = 0;
                do
                {
                    OUString aStr( OStringToOUString(
                        aEnv.getToken( 0, ';', nIndex ),
                        RTL_TEXTENCODING_ASCII_US ) );
                    OUString aFileUrl;
                    OSL_VERIFY(
                        osl_File_E_None ==
                        FileBase::getFileURLFromSystemPath(aStr, aFileUrl) );
                    s_v.push_back( aFileUrl );
                } while( nIndex != -1 );
#if OSL_DEBUG_LEVEL > 1
                out( "> cpld: acknowledged following access path(s): \"" );
                ::std::vector< OUString >::const_iterator iPos( s_v.begin() );
                while (iPos != s_v.end())
                {
                    out( *iPos );
                    ++iPos;
                    if (iPos != s_v.end())
                        out( ";" );
                }
                out( "\"\n" );
#endif
                s_p = & s_v;
            }
            else
            {
                // no access path env set
#if OSL_DEBUG_LEVEL > 1
                out( "=> no CPLD_ACCESSPATH set.\n" );
#endif
            }
            s_bInit = true;
        }
    }

    return s_p;
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
static inline sal_Int32 endsWith(
    const OUString & rText, const OUString & rEnd ) SAL_THROW( () )
{
    if (rText.getLength() >= rEnd.getLength() &&
        rEnd.equalsIgnoreAsciiCase(
            rText.copy( rText.getLength() - rEnd.getLength() ) ))
    {
        return rText.getLength() - rEnd.getLength();
    }
    return -1;
}

//------------------------------------------------------------------------------
static OUString makeComponentPath(
    const OUString & rLibName, const OUString & rPath )
{
#if OSL_DEBUG_LEVEL > 0
    // No system path allowed here !
    {
        OUString aComp;
        OSL_ASSERT( osl_File_E_None ==
                    FileBase::getSystemPathFromFileURL( rLibName, aComp ) );
        OSL_ASSERT(
            ! rPath.getLength() ||
            osl_File_E_None ==
              FileBase::getSystemPathFromFileURL( rPath, aComp ) );
    }
#endif

    OUStringBuffer buf( rPath.getLength() + rLibName.getLength() + 12 );

    if (0 != rPath.getLength())
    {
        buf.append( rPath );
        if (rPath[ rPath.getLength() -1 ] != '/')
            buf.append( (sal_Unicode) '/' );
    }

    sal_Int32 nEnd = endsWith( rLibName, OUSTR(SAL_DLLEXTENSION) );
    if (nEnd < 0) // !endsWith
    {
#if (OSL_DEBUG_LEVEL >= 2)
        OSL_ENSURE(
            !"### library name has no proper extension!",
            OUStringToOString( rLibName, RTL_TEXTENCODING_ASCII_US ).getStr() );
#endif
#if defined SAL_DLLPREFIX
        nEnd = endsWith( rLibName, OUSTR(".uno") );
        if (nEnd < 0) // !endsWith
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(SAL_DLLPREFIX) );
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
    OSL_TRACE( "component path=%s\n", str.getStr() );
#endif

    return out;
}

//==============================================================================
Reference< XInterface > SAL_CALL loadSharedLibComponentFactory(
    OUString const & rLibName, OUString const & rPath,
    OUString const & rImplName,
    Reference< lang::XMultiServiceFactory > const & xMgr,
    Reference< registry::XRegistryKey > const & xKey )
    SAL_THROW( (loader::CannotActivateFactoryException) )
{
    OUString aModulePath( makeComponentPath( rLibName, rPath ) );
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

    OUString aExcMsg;
    Reference< XInterface > xRet;

    void * pSym;
    OUString aGetEnvName = OUSTR(COMPONENT_GETENV);
    pSym = osl_getSymbol( lib, aGetEnvName.pData );
    if (pSym != 0)
    {
        uno_Environment * pCurrentEnv = 0;
        uno_Environment * pEnv = 0;

        const sal_Char * pEnvTypeName = 0;
        (*((component_getImplementationEnvironmentFunc) pSym))(
            &pEnvTypeName, &pEnv );
        OUString aEnvTypeName( OUString::createFromAscii( pEnvTypeName ) );

        bool bNeedsMapping =
            ((pEnv != 0) ||
             !aEnvTypeName.equalsAsciiL(
                 RTL_CONSTASCII_STRINGPARAM(
                     CPPU_CURRENT_LANGUAGE_BINDING_NAME ) ));
        if (bNeedsMapping)
        {
            if (! pEnv)
            {
                uno_getEnvironment( &pEnv, aEnvTypeName.pData, 0 );
            }
            if (pEnv)
            {
                OUString aCppEnvTypeName =
                    OUSTR(CPPU_CURRENT_LANGUAGE_BINDING_NAME);
                uno_getEnvironment( &pCurrentEnv, aCppEnvTypeName.pData, 0 );
                if (pCurrentEnv)
                {
                    bNeedsMapping = (pEnv != pCurrentEnv);
                }
            }
        }

        OUString aGetFactoryName = OUSTR(COMPONENT_GETFACTORY);
        pSym = osl_getSymbol( lib, aGetFactoryName.pData );
        if (pSym != 0)
        {
            OString aImplName(
                OUStringToOString( rImplName, RTL_TEXTENCODING_ASCII_US ) );
            if (bNeedsMapping)
            {
                if (pEnv && pCurrentEnv)
                {
                    Mapping aCurrent2Env( pCurrentEnv, pEnv );
                    Mapping aEnv2Current( pEnv, pCurrentEnv );

                    if (aCurrent2Env.is() && aEnv2Current.is())
                    {
                        void * pSMgr = aCurrent2Env.mapInterface(
                            xMgr.get(), ::getCppuType( &xMgr ) );
                        void * pKey = aCurrent2Env.mapInterface(
                            xKey.get(), ::getCppuType( &xKey ) );

                        void * pSSF =
                            (*((component_getFactoryFunc) pSym))(
                                aImplName.getStr(), pSMgr, pKey );

                        if (pKey)
                        {
                            (*pEnv->pExtEnv->releaseInterface)(
                                pEnv->pExtEnv, pKey );
                        }
                        if (pSMgr)
                        {
                            (*pEnv->pExtEnv->releaseInterface)(
                                pEnv->pExtEnv, pSMgr );
                        }

                        if (pSSF)
                        {
                            aEnv2Current.mapInterface(
                                reinterpret_cast< void ** >( &xRet ),
                                pSSF, ::getCppuType( &xRet ) );
                            (*pEnv->pExtEnv->releaseInterface)(
                                pEnv->pExtEnv, pSSF );
                        }
                        else
                        {
                            aExcMsg = aModulePath;
                            aExcMsg += OUSTR(": cannot get factory of "
                                             "demanded implementation: ");
                            aExcMsg += OStringToOUString(
                                aImplName, RTL_TEXTENCODING_ASCII_US );
                        }
                    }
                    else
                    {
                        aExcMsg =
                            OUSTR("cannot get uno mappings: C++ <=> UNO!");
                    }
                }
                else
                {
                    aExcMsg = OUSTR("cannot get uno environments!");
                }
            }
            else
            {
                XInterface * pRet =
                    (XInterface *) (*((component_getFactoryFunc) pSym))(
                        aImplName.getStr(), xMgr.get(), xKey.get() );
                if (pRet)
                {
                    xRet = pRet;
                    pRet->release();
                }
                else
                {
                    aExcMsg = aModulePath;
                    aExcMsg += OUSTR(": cannot get factory of demanded "
                                     "implementation: ");
                    aExcMsg += OStringToOUString(
                        aImplName, RTL_TEXTENCODING_ASCII_US );
                }
            }
        }
        else
        {
            aExcMsg = aModulePath;
            aExcMsg += OUSTR(": cannot get symbol: ");
            aExcMsg += aGetFactoryName;
        }

        if (pEnv)
            (*pEnv->release)( pEnv );
        if (pCurrentEnv)
            (*pCurrentEnv->release)( pCurrentEnv );
    }

    // ========================= DEPRECATED =========================
    else
    {
        OUString aGetFactoryName = OUSTR(CREATE_COMPONENT_FACTORY_FUNCTION);
        pSym = ::osl_getSymbol( lib, aGetFactoryName.pData );
        if (pSym != 0)
        {
            OUString aCppEnvTypeName = OUSTR(CPPU_CURRENT_LANGUAGE_BINDING_NAME);
            OUString aUnoEnvTypeName = OUSTR(UNO_LB_UNO);
            Mapping aUno2Cpp( aUnoEnvTypeName, aCppEnvTypeName );
            Mapping aCpp2Uno( aCppEnvTypeName, aUnoEnvTypeName );
            OSL_ENSURE(
                aUno2Cpp.is() && aCpp2Uno.is(),
                "### cannot get uno mappings!" );

            if (aUno2Cpp.is() && aCpp2Uno.is())
            {
                uno_Interface * pUComponentFactory = 0;

                uno_Interface * pUSFactory =
                    (uno_Interface *) aCpp2Uno.mapInterface(
                        xMgr.get(),
                        ::getCppuType(
                            (const Reference< lang::XMultiServiceFactory > *)0 )
                        );
                uno_Interface * pUKey =
                    (uno_Interface *) aCpp2Uno.mapInterface(
                        xKey.get(),
                        ::getCppuType(
                            (const Reference< registry::XRegistryKey > *)0 ) );

                pUComponentFactory =
                    (*((CreateComponentFactoryFunc) pSym))(
                        rImplName.getStr(), pUSFactory, pUKey );

                if (pUKey)
                    (*pUKey->release)( pUKey );
                if (pUSFactory)
                    (*pUSFactory->release)( pUSFactory );

                if (pUComponentFactory)
                {
                    aUno2Cpp.mapInterface(
                        reinterpret_cast< void ** >( &xRet ),
                        pUComponentFactory, ::getCppuType( &xRet ) );
                    (*pUComponentFactory->release)( pUComponentFactory );
                }
            }
        }
        else
        {
            aExcMsg = aModulePath;
            aExcMsg += OUSTR(": cannot get symbol: ");
            aExcMsg += aGetEnvName;
        }
    }

    if (! xRet.is())
    {
        osl_unloadModule( lib );
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

//==============================================================================
void SAL_CALL writeSharedLibComponentInfo(
    OUString const & rLibName, OUString const & rPath,
    Reference< lang::XMultiServiceFactory > const & xMgr,
    Reference< registry::XRegistryKey > const & xKey )
    SAL_THROW( (registry::CannotRegisterImplementationException) )
{
    OUString aModulePath( makeComponentPath( rLibName, rPath ) );

    OUString aExcMsg;

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

    void * pSym;
    OUString aGetEnvName = OUSTR(COMPONENT_GETENV);
    pSym = osl_getSymbol( lib, aGetEnvName.pData );
    if (pSym != 0)
    {
        uno_Environment * pCurrentEnv = 0;
        uno_Environment * pEnv = 0;

        const sal_Char * pEnvTypeName = 0;
        (*((component_getImplementationEnvironmentFunc) pSym))(
            &pEnvTypeName, &pEnv );
        OUString aEnvTypeName( OUString::createFromAscii( pEnvTypeName ) );

        bool bNeedsMapping =
            ((pEnv != 0) ||
             !aEnvTypeName.equalsAsciiL(
                 RTL_CONSTASCII_STRINGPARAM(
                     CPPU_CURRENT_LANGUAGE_BINDING_NAME ) ));
        if (bNeedsMapping)
        {
            if (! pEnv)
            {
                uno_getEnvironment( &pEnv, aEnvTypeName.pData, 0 );
            }
            if (pEnv)
            {
                OUString aCppEnvTypeName =
                    OUSTR(CPPU_CURRENT_LANGUAGE_BINDING_NAME);
                uno_getEnvironment( &pCurrentEnv, aCppEnvTypeName.pData, 0 );
                if (pCurrentEnv)
                {
                    bNeedsMapping = (pEnv != pCurrentEnv);
                }
            }
        }

        OUString aWriteInfoName = OUSTR(COMPONENT_WRITEINFO);
        pSym = osl_getSymbol( lib, aWriteInfoName.pData );
        if (pSym != 0)
        {
            if (bNeedsMapping)
            {
                if (pEnv && pCurrentEnv)
                {
                    Mapping aCurrent2Env( pCurrentEnv, pEnv );
                    if (aCurrent2Env.is())
                    {
                        void * pSMgr = aCurrent2Env.mapInterface(
                            xMgr.get(), ::getCppuType( &xMgr ) );
                        void * pKey = aCurrent2Env.mapInterface(
                            xKey.get(), ::getCppuType( &xKey ) );
                        if (pKey)
                        {
                            bRet = (*((component_writeInfoFunc) pSym))(
                                pSMgr, pKey );
                            (*pEnv->pExtEnv->releaseInterface)(
                                pEnv->pExtEnv, pKey );
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
                            (*pEnv->pExtEnv->releaseInterface)(
                                pEnv->pExtEnv, pSMgr );
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
                if (xKey.is())
                {
                    bRet = (*((component_writeInfoFunc) pSym))(
                        xMgr.get(), xKey.get() );
                    if (! bRet)
                    {
                        aExcMsg = aModulePath;
                        aExcMsg += OUSTR(": component_writeInfo() returned "
                                         "false!");
                    }
                }
                else
                {
                    // key is mandatory
                    aExcMsg = aModulePath;
                    aExcMsg += OUSTR(": registry is mandatory to invoke "
                                     "component_writeInfo()!");
                }
            }
        }
        else
        {
            aExcMsg = aModulePath;
            aExcMsg += OUSTR(": cannot get symbol: ");
            aExcMsg += aWriteInfoName;
        }

        if (pEnv)
            (*pEnv->release)( pEnv );
        if (pCurrentEnv)
            (*pCurrentEnv->release)( pCurrentEnv );
    }

    // ========================= DEPRECATED =========================
    else
    {
        OUString aWriteInfoName = OUSTR(WRITE_COMPONENT_INFO_FUNCTION);
        pSym = osl_getSymbol( lib, aWriteInfoName.pData );
        if (pSym != 0)
        {
            OUString aCppEnvTypeName =
                OUSTR(CPPU_CURRENT_LANGUAGE_BINDING_NAME);
            OUString aUnoEnvTypeName = OUSTR(UNO_LB_UNO);
            Mapping aCpp2Uno( aCppEnvTypeName, aUnoEnvTypeName );

            if (aCpp2Uno.is())
            {
                uno_Interface * pUKey =
                    (uno_Interface *) aCpp2Uno.mapInterface(
                        xKey.get(), ::getCppuType( &xKey ) );

                bRet = (*((WriteComponentInfoFunc) pSym))( pUKey );

                if (pUKey)
                    (*pUKey->release)( pUKey );
            }
        }
        else
        {
            aExcMsg = aModulePath;
            aExcMsg += OUSTR(": cannot get symbol: ");
            aExcMsg += aGetEnvName;
        }
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

}
