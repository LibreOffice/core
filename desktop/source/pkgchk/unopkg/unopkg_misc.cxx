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


#include "deployment.hrc"
#include "unopkg_shared.h"
#include "dp_identifier.hxx"
#include "../../deployment/gui/dp_gui.hrc"
#include "lockfile.hxx"
#include "vcl/svapp.hxx"
#include "vcl/msgbox.hxx"
#include "rtl/bootstrap.hxx"
#include "rtl/strbuf.hxx"
#include "rtl/ustrbuf.hxx"
#include "osl/process.h"
#include "osl/file.hxx"
#include "osl/thread.hxx"
#include "tools/getprocessworkingdir.hxx"
#include "comphelper/processfactory.hxx"
#include "unotools/configmgr.hxx"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/ucb/UniversalContentBroker.hpp"
#include "cppuhelper/bootstrap.hxx"
#include "comphelper/sequence.hxx"
#include <stdio.h>

using ::rtl::OUString;
using ::rtl::OString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;

namespace unopkg {

bool getLockFilePath(OUString & out);

::rtl::OUString toString( OptionInfo const * info )
{
    OSL_ASSERT( info != 0 );
    ::rtl::OUStringBuffer buf;
    buf.appendAscii("--");
    buf.appendAscii(info->m_name);
    if (info->m_short_option != '\0')
    {
        buf.appendAscii(" (short -" );
        buf.append(info->m_short_option );
        buf.appendAscii(")");
    }
    if (info->m_has_argument)
        buf.appendAscii(" <argument>" );
    return buf.makeStringAndClear();
}

//==============================================================================
OptionInfo const * getOptionInfo(
    OptionInfo const * list,
    OUString const & opt, sal_Unicode copt )
{
    for ( ; list->m_name != 0; ++list )
    {
        OptionInfo const & option_info = *list;
        if (!opt.isEmpty())
        {
            if (opt.equalsAsciiL(
                    option_info.m_name, option_info.m_name_length ) &&
                (copt == '\0' || copt == option_info.m_short_option))
            {
                return &option_info;
            }
        }
        else
        {
            OSL_ASSERT( copt != '\0' );
            if (copt == option_info.m_short_option)
            {
                return &option_info;
            }
        }
    }
    OSL_FAIL( ::rtl::OUStringToOString(
                    opt, osl_getThreadTextEncoding() ).getStr() );
    return 0;
}

//==============================================================================
bool isOption( OptionInfo const * option_info, sal_uInt32 * pIndex )
{
    OSL_ASSERT( option_info != 0 );
    if (osl_getCommandArgCount() <= *pIndex)
        return false;

    OUString arg;
    osl_getCommandArg( *pIndex, &arg.pData );
    sal_Int32 len = arg.getLength();

    if (len < 2 || arg[ 0 ] != '-')
        return false;

    if (len == 2 && arg[ 1 ] == option_info->m_short_option)
    {
        ++(*pIndex);
        dp_misc::TRACE(OUSTR(__FILE__": identified option \'")
            + OUSTR("\'") + OUString( option_info->m_short_option ) + OUSTR("\n"));
        return true;
    }
    if (arg[ 1 ] == '-' && rtl_ustr_ascii_compare(
            arg.pData->buffer + 2, option_info->m_name ) == 0)
    {
        ++(*pIndex);
        dp_misc::TRACE(OUSTR( __FILE__": identified option \'")
            + OUString::createFromAscii(option_info->m_name) + OUSTR("\'\n"));
        return true;
    }
    return false;
}
//==============================================================================

bool isBootstrapVariable(sal_uInt32 * pIndex)
{
    OSL_ASSERT(osl_getCommandArgCount() >=  *pIndex);

    OUString arg;
    osl_getCommandArg(*pIndex, &arg.pData);
    if (arg.matchAsciiL("-env:", 5))
    {
        ++(*pIndex);
        return true;
    }
    return false;
}

//==============================================================================
bool readArgument(
    OUString * pValue, OptionInfo const * option_info, sal_uInt32 * pIndex )
{
    if (isOption( option_info, pIndex ))
    {
        if (*pIndex < osl_getCommandArgCount())
        {
            OSL_ASSERT( pValue != 0 );
            osl_getCommandArg( *pIndex, &pValue->pData );
            dp_misc::TRACE(OUSTR( __FILE__": argument value: ")
                + *pValue + OUSTR("\n"));
            ++(*pIndex);
            return true;
        }
        --(*pIndex);
    }
    return false;
}


namespace {
struct ExecutableDir : public rtl::StaticWithInit<
    OUString, ExecutableDir> {
    const OUString operator () () {
        OUString path;
        if (osl_getExecutableFile( &path.pData ) != osl_Process_E_None) {
            throw RuntimeException(
                OUSTR("cannot locate executable directory!"),0  );
        }
        return path.copy( 0, path.lastIndexOf( '/' ) );
    }
};
struct ProcessWorkingDir : public rtl::StaticWithInit<
    OUString, ProcessWorkingDir> {
    const OUString operator () () {
        OUString workingDir;
        tools::getProcessWorkingDir(workingDir);
        return workingDir;
    }
};
} // anon namespace

//==============================================================================
OUString const & getExecutableDir()
{
    return ExecutableDir::get();
}

//==============================================================================
OUString const & getProcessWorkingDir()
{
    return ProcessWorkingDir::get();
}

//==============================================================================
OUString makeAbsoluteFileUrl(
    OUString const & sys_path, OUString const & base_url, bool throw_exc )
{
    // system path to file url
    OUString file_url;
    oslFileError rc = osl_getFileURLFromSystemPath( sys_path.pData, &file_url.pData );
    if ( rc != osl_File_E_None) {
        OUString tempPath;
        if ( osl_getSystemPathFromFileURL( sys_path.pData, &tempPath.pData) == osl_File_E_None )
        {
            file_url = sys_path;
        }
        else if (throw_exc)
        {
            throw RuntimeException(
                OUSTR("cannot get file url from system path: ") +
                sys_path, Reference< XInterface >() );
        }
    }

    OUString abs;
    if (osl_getAbsoluteFileURL(
            base_url.pData, file_url.pData, &abs.pData ) != osl_File_E_None)
    {
        if (throw_exc) {
            ::rtl::OUStringBuffer buf;
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(
                                 "making absolute file url failed: \"") );
            buf.append( base_url );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(
                                 "\" (base-url) and \"") );
            buf.append( file_url );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\" (file-url)!") );
            throw RuntimeException(
                buf.makeStringAndClear(), Reference< XInterface >() );
        }
        return OUString();
    }
    return abs[ abs.getLength() -1 ] == '/'
        ? abs.copy( 0, abs.getLength() -1 ) : abs;
}


namespace {

//------------------------------------------------------------------------------
inline void printf_space( sal_Int32 space )
{
    while (space--)
        dp_misc::writeConsole("  ");
}

//------------------------------------------------------------------------------
void printf_line(
    OUString const & name, OUString const & value, sal_Int32 level )
{
   printf_space( level );
    dp_misc::writeConsole(name + OUSTR(": ") + value + OUSTR("\n"));
}

//------------------------------------------------------------------------------
void printf_package(
    Reference<deployment::XPackage> const & xPackage,
    Reference<XCommandEnvironment> const & xCmdEnv, sal_Int32 level )
{
    beans::Optional< OUString > id(
        level == 0
        ? beans::Optional< OUString >(
            true, dp_misc::getIdentifier( xPackage ) )
        : xPackage->getIdentifier() );
    if (id.IsPresent)
        printf_line( OUSTR("Identifier"), id.Value, level );
    OUString version(xPackage->getVersion());
    if (!version.isEmpty())
        printf_line( OUSTR("Version"), version, level + 1 );
    printf_line( OUSTR("URL"), xPackage->getURL(), level + 1 );

    beans::Optional< beans::Ambiguous<sal_Bool> > option(
        xPackage->isRegistered( Reference<task::XAbortChannel>(), xCmdEnv ) );
    OUString value;
    if (option.IsPresent) {
        beans::Ambiguous<sal_Bool> const & reg = option.Value;
        if (reg.IsAmbiguous)
            value = OUSTR("unknown");
        else
            value = reg.Value ? OUSTR("yes") : OUSTR("no");
    }
    else
        value = OUSTR("n/a");
    printf_line( OUSTR("is registered"), value, level + 1 );

    const Reference<deployment::XPackageTypeInfo> xPackageType(
        xPackage->getPackageType() );
    OSL_ASSERT( xPackageType.is() );
    if (xPackageType.is()) {
        printf_line( OUSTR("Media-Type"),
                     xPackageType->getMediaType(), level + 1 );
    }
    printf_line( OUSTR("Description"), xPackage->getDescription(), level + 1 );
    if (xPackage->isBundle()) {
        Sequence< Reference<deployment::XPackage> > seq(
            xPackage->getBundle( Reference<task::XAbortChannel>(), xCmdEnv ) );
        printf_space( level + 1 );
        dp_misc::writeConsole("bundled Packages: {\n");
        ::std::vector<Reference<deployment::XPackage> >vec_bundle;
        ::comphelper::sequenceToContainer(vec_bundle, seq);
        printf_packages( vec_bundle, ::std::vector<bool>(vec_bundle.size()),
                         xCmdEnv, level + 2 );
        printf_space( level + 1 );
        dp_misc::writeConsole("}\n");
    }
}

} // anon namespace

void printf_unaccepted_licenses(
    Reference<deployment::XPackage> const & ext)
{
        OUString id(
            dp_misc::getIdentifier(ext) );
        printf_line( OUSTR("Identifier"), id, 0 );
        printf_space(1);
        dp_misc::writeConsole(OUSTR("License not accepted\n\n"));
}

//==============================================================================
void printf_packages(
    ::std::vector< Reference<deployment::XPackage> > const & allExtensions,
    ::std::vector<bool> const & vecUnaccepted,
    Reference<XCommandEnvironment> const & xCmdEnv, sal_Int32 level )
{
    OSL_ASSERT(allExtensions.size() == vecUnaccepted.size());

    if (allExtensions.empty())
    {
        printf_space( level );
        dp_misc::writeConsole("<none>\n");
    }
    else
    {
        typedef ::std::vector< Reference<deployment::XPackage> >::const_iterator I_EXT;
        int index = 0;
        for (I_EXT i = allExtensions.begin(); i != allExtensions.end(); ++i, ++index)
        {
            if (vecUnaccepted[index])
                printf_unaccepted_licenses(*i);
            else
                printf_package( *i, xCmdEnv, level );
            dp_misc::writeConsole(OUSTR("\n"));
        }
    }
}



namespace {

//------------------------------------------------------------------------------
Reference<XComponentContext> bootstrapStandAlone()
{
    Reference<XComponentContext> xContext =
        ::cppu::defaultBootstrap_InitialComponentContext();

    Reference<lang::XMultiServiceFactory> xServiceManager(
        xContext->getServiceManager(), UNO_QUERY_THROW );
    // set global process service factory used by unotools config helpers
    ::comphelper::setProcessServiceFactory( xServiceManager );

    // Initialize the UCB (for backwards compatibility, in case some code still
    // uses plain createInstance w/o args directly to obtain an instance):
    UniversalContentBroker::create( xContext );

    return xContext;
}

//------------------------------------------------------------------------------
Reference<XComponentContext> connectToOffice(
    Reference<XComponentContext> const & xLocalComponentContext,
    bool verbose )
{
    Sequence<OUString> args( 3 );
    args[ 0 ] = OUSTR("--nologo");
    args[ 1 ] = OUSTR("--nodefault");

    OUString pipeId( ::dp_misc::generateRandomPipeId() );
    ::rtl::OUStringBuffer buf;
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("--accept=pipe,name=") );
    buf.append( pipeId );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(";urp;") );
    args[ 2 ] = buf.makeStringAndClear();
    OUString appURL( getExecutableDir() + OUSTR("/soffice") );

    if (verbose)
    {
        dp_misc::writeConsole(
            OUSTR("Raising process: ") +
            appURL +
            OUSTR("\nArguments: --nologo --nodefault ") +
            args[2] +
            OUSTR("\n"));
    }

    ::dp_misc::raiseProcess( appURL, args );

    if (verbose)
        dp_misc::writeConsole("OK.  Connecting...");

    OSL_ASSERT( buf.getLength() == 0 );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("uno:pipe,name=") );
    buf.append( pipeId );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(
                         ";urp;StarOffice.ComponentContext") );
    Reference<XComponentContext> xRet(
        ::dp_misc::resolveUnoURL(
            buf.makeStringAndClear(), xLocalComponentContext ),
        UNO_QUERY_THROW );
    if (verbose)
        dp_misc::writeConsole("OK.\n");

    return xRet;
}

} // anon namespace

/** returns the path to the lock file used by unopkg.
    @return the path. An empty string signifies an error.
*/
OUString getLockFilePath()
{
    OUString ret;
    OUString sBootstrap(RTL_CONSTASCII_USTRINGPARAM("${$BRAND_BASE_DIR/program/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}"));
    rtl::Bootstrap::expandMacros(sBootstrap);
    OUString sAbs;
    if (::osl::File::E_None ==  ::osl::File::getAbsoluteFileURL(
        sBootstrap, OUSTR(".lock"), sAbs))
    {
        if (::osl::File::E_None ==
            ::osl::File::getSystemPathFromFileURL(sAbs, sBootstrap))
        {
            ret = sBootstrap;
        }
    }

    return ret;
}
//==============================================================================
Reference<XComponentContext> getUNO(
    bool verbose, bool shared, bool bGui,
    Reference<XComponentContext> & out_localContext)
{
    // do not create any user data (for the root user) in --shared mode:
    if (shared) {
        rtl::Bootstrap::set(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CFG_CacheUrl")),
            rtl::OUString());
    }

    // hold lock during process runtime:
    static ::desktop::Lockfile s_lockfile( false /* no IPC server */ );
    Reference<XComponentContext> xComponentContext( bootstrapStandAlone() );
    out_localContext = xComponentContext;
    if (::dp_misc::office_is_running()) {
        xComponentContext.set(
            connectToOffice( xComponentContext, verbose ) );
    }
    else
    {
        if (! s_lockfile.check( 0 ))
        {
            String sMsg(ResId(RID_STR_CONCURRENTINSTANCE, *DeploymentResMgr::get()));
            //Create this string before we call DeInitVCL, because this will kill
            //the ResMgr
            String sError(ResId(RID_STR_UNOPKG_ERROR, *DeploymentResMgr::get()));

            sMsg = sMsg + OUSTR("\n") + getLockFilePath();

            if (bGui)
            {
                //We show a message box or print to the console that there
                //is another instance already running
                if ( ! InitVCL( Reference<lang::XMultiServiceFactory>(
                                    xComponentContext->getServiceManager(),
                                    UNO_QUERY_THROW ) ))
                    throw RuntimeException( OUSTR("Cannot initialize VCL!"),
                                            NULL );
                {
                    WarningBox warn(NULL, WB_OK | WB_DEF_OK, sMsg);
                    warn.SetText(utl::ConfigManager::getProductName());
                    warn.SetIcon(0);
                    warn.Execute();
                }
                DeInitVCL();
            }

            throw LockFileException(
                OUSTR("\n") + sError + sMsg + OUSTR("\n"));
        }
    }

    return xComponentContext;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
