/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unopkg_misc.cxx,v $
 * $Revision: 1.14 $
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

#include "deployment.hrc"
#include "unopkg_shared.h"
#include "dp_identifier.hxx"
#include "../../deployment/gui/dp_gui.hrc"
#include "../../app/lockfile.hxx"
#include "vcl/svapp.hxx"
#include "vcl/msgbox.hxx"
#include "rtl/bootstrap.hxx"
#include "rtl/strbuf.hxx"
#include "rtl/ustrbuf.hxx"
#include "osl/process.h"
#include "osl/file.h"
#include "osl/thread.hxx"
#include "ucbhelper/contentbroker.hxx"
#include "ucbhelper/configurationkeys.hxx"
#include "unotools/processfactory.hxx"
#include "unotools/configmgr.hxx"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "cppuhelper/bootstrap.hxx"
#include <stdio.h>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;

namespace unopkg {


::rtl::OString toString( OptionInfo const * info )
{
    OSL_ASSERT( info != 0 );
    ::rtl::OStringBuffer buf;
    buf.append( RTL_CONSTASCII_STRINGPARAM("--") );
    buf.append( info->m_name, info->m_name_length );
    if (info->m_short_option != '\0')
    {
        buf.append( RTL_CONSTASCII_STRINGPARAM(" (short -") );
        buf.append( static_cast< char >(info->m_short_option) );
        buf.append( ')' );
    }
    if (info->m_has_argument)
        buf.append( RTL_CONSTASCII_STRINGPARAM(" <argument>") );
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
        if (opt.getLength() > 0)
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
    OSL_ENSURE( 0, ::rtl::OUStringToOString(
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
#if OSL_DEBUG_LEVEL > 1
        OSL_TRACE(
            __FILE__": identified option \'%c\'", option_info->m_short_option );
#endif
        return true;
    }
    if (arg[ 1 ] == '-' && rtl_ustr_ascii_compare(
            arg.pData->buffer + 2, option_info->m_name ) == 0)
    {
        ++(*pIndex);
#if OSL_DEBUG_LEVEL > 1
        OSL_TRACE( __FILE__": identified option \'%s\'", option_info->m_name );
#endif
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
#if OSL_DEBUG_LEVEL > 1
            OSL_TRACE(
                __FILE__": argument value: %s\n",
                ::rtl::OUStringToOString(
                    *pValue, osl_getThreadTextEncoding() ).getStr() );
#endif
            ++(*pIndex);
            return true;
        }
        --(*pIndex);
    }
    return false;
}

//##############################################################################

namespace {
struct ExecutableDir : public rtl::StaticWithInit<
    const OUString, ExecutableDir> {
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
    const OUString, ProcessWorkingDir> {
    const OUString operator () () {
        OUString workingDir;
        osl_getProcessWorkingDir( &workingDir.pData );
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

//##############################################################################

namespace {

//------------------------------------------------------------------------------
inline void printf_space( sal_Int32 space )
{
    while (space--)
        printf( "  " );
}

//------------------------------------------------------------------------------
void printf_line(
    OUString const & name, OUString const & value, sal_Int32 level )
{
    rtl_TextEncoding textenc = osl_getThreadTextEncoding();
    printf_space( level );
    printf( "%s: %s\n",
            ::rtl::OUStringToOString( name, textenc ).getStr(),
            ::rtl::OUStringToOString( value, textenc ).getStr() );
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
    if (version.getLength() != 0)
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
        printf( "bundled Packages: {\n" );
        printf_packages( seq, xCmdEnv, level + 2 );
        printf_space( level + 1 );
        printf( "}\n" );
    }
}

} // anon namespace

//==============================================================================
void printf_packages(
    Sequence< Reference<deployment::XPackage> > const & seq,
    Reference<XCommandEnvironment> const & xCmdEnv, sal_Int32 level )
{
    sal_Int32 len = seq.getLength();
    Reference< deployment::XPackage > const * p = seq.getConstArray();
    if (len == 0) {
        printf_space( level );
        printf( "<none>\n" );
    }
    else {
        for ( sal_Int32 pos = 0; pos < len; ++pos )
            printf_package( p[ pos ], xCmdEnv, level );
    }
}

//##############################################################################

namespace {

//------------------------------------------------------------------------------
Reference<XComponentContext> bootstrapStandAlone(
    DisposeGuard & disposeGuard, bool /*verbose */)
{
    Reference<XComponentContext> xContext =
        ::cppu::defaultBootstrap_InitialComponentContext();

    // assure disposing of local component context:
    disposeGuard.reset(
        Reference<lang::XComponent>( xContext, UNO_QUERY ) );

    Reference<lang::XMultiServiceFactory> xServiceManager(
        xContext->getServiceManager(), UNO_QUERY_THROW );
    // set global process service factory used by unotools config helpers
    ::utl::setProcessServiceFactory( xServiceManager );

    // initialize the ucbhelper ucb,
    // because the package implementation uses it
    Sequence<Any> ucb_args( 2 );
    ucb_args[ 0 ] <<= OUSTR(UCB_CONFIGURATION_KEY1_LOCAL);
    ucb_args[ 1 ] <<= OUSTR(UCB_CONFIGURATION_KEY2_OFFICE);
    if (! ::ucbhelper::ContentBroker::initialize( xServiceManager, ucb_args ))
        throw RuntimeException( OUSTR("cannot initialize UCB!"), 0 );

    return xContext;
}

//------------------------------------------------------------------------------
Reference<XComponentContext> connectToOffice(
    Reference<XComponentContext> const & xLocalComponentContext,
    bool verbose )
{
    Sequence<OUString> args( 3 );
    args[ 0 ] = OUSTR("-nologo");
    args[ 1 ] = OUSTR("-nodefault");

    OUString pipeId( ::dp_misc::generateRandomPipeId() );
    ::rtl::OUStringBuffer buf;
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("-accept=pipe,name=") );
    buf.append( pipeId );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(";urp;") );
    args[ 2 ] = buf.makeStringAndClear();
    OUString appURL( getExecutableDir() + OUSTR("/soffice") );

    if (verbose)
    {
        rtl_TextEncoding textenc = osl_getThreadTextEncoding();
        printf( "Raising process: %s\nArguments: -nologo -nodefault %s\n",
                ::rtl::OUStringToOString( appURL, textenc ).getStr(),
                ::rtl::OUStringToOString( args[ 2 ], textenc ).getStr() );
    }

    ::dp_misc::raiseProcess( appURL, args );

    if (verbose)
        printf( "Ok.  Connecting..." );

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
        printf( "Ok.\n" );

    return xRet;
}

} // anon namespace

//==============================================================================
Reference<XComponentContext> getUNO(
    DisposeGuard & disposeGuard, bool verbose, bool shared, bool bGui,
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
    Reference<XComponentContext> xComponentContext(
        bootstrapStandAlone( disposeGuard, verbose ) );
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
                    warn.SetText(::utl::ConfigManager::GetDirectConfigProperty(
                                     ::utl::ConfigManager::PRODUCTNAME).get<OUString>());
                    warn.SetIcon(0);
                    warn.Execute();
                }
                DeInitVCL();
            }
            else
            {
                ::rtl::OString soMsg = ::rtl::OUStringToOString(sMsg, osl_getThreadTextEncoding());
                fprintf(stdout,"%s\n", soMsg.getStr());

            }
            throw RuntimeException(
                OUSTR("Lock file indicates that a concurrent Office process "
                    "is running!"), Reference<XInterface>() );
        }
    }

    return xComponentContext;
}

}

