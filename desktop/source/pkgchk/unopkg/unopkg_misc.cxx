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

#include <config_folders.h>

#include "deployment.hrc"
#include "unopkg_shared.h"
#include "dp_identifier.hxx"
#include "dp_gui.hrc"
#include "lockfile.hxx"
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/process.h>
#include <osl/file.hxx>
#include <osl/thread.hxx>
#include <tools/getprocessworkingdir.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/configmgr.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <cppuhelper/bootstrap.hxx>
#include <comphelper/sequence.hxx>
#include <stdio.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;

namespace unopkg {

OUString toString( OptionInfo const * info )
{
    assert(info != nullptr);
    OUStringBuffer buf;
    buf.append("--");
    buf.appendAscii(info->m_name);
    if (info->m_short_option != '\0')
    {
        buf.append(" (short -" );
        buf.append(info->m_short_option );
        buf.append(")");
    }
    if (info->m_has_argument)
        buf.append(" <argument>" );
    return buf.makeStringAndClear();
}


OptionInfo const * getOptionInfo(
    OptionInfo const * list,
    OUString const & opt )
{
    for ( ; list->m_name != nullptr; ++list )
    {
        OptionInfo const & option_info = *list;
        if (!opt.isEmpty())
        {
            if (opt.equalsAsciiL(
                    option_info.m_name, option_info.m_name_length ))
            {
                return &option_info;
            }
        }
    }
    OSL_FAIL( OUStringToOString(
                    opt, osl_getThreadTextEncoding() ).getStr() );
    return nullptr;
}


bool isOption( OptionInfo const * option_info, sal_uInt32 * pIndex )
{
    assert(option_info != nullptr);
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
        dp_misc::TRACE(__FILE__ ": identified option \'\'"
            + OUStringLiteral1( option_info->m_short_option ) + "\n");
        return true;
    }
    if (arg[ 1 ] == '-' && rtl_ustr_ascii_compare(
            arg.pData->buffer + 2, option_info->m_name ) == 0)
    {
        ++(*pIndex);
        dp_misc::TRACE(__FILE__ ": identified option \'"
            + OUString::createFromAscii(option_info->m_name) + "\'\n");
        return true;
    }
    return false;
}


bool isBootstrapVariable(sal_uInt32 * pIndex)
{
    OSL_ASSERT(osl_getCommandArgCount() >=  *pIndex);

    OUString arg;
    osl_getCommandArg(*pIndex, &arg.pData);
    if (arg.match("-env:"))
    {
        ++(*pIndex);
        return true;
    }
    return false;
}


bool readArgument(
    OUString * pValue, OptionInfo const * option_info, sal_uInt32 * pIndex )
{
    if (isOption( option_info, pIndex ))
    {
        if (*pIndex < osl_getCommandArgCount())
        {
            OSL_ASSERT( pValue != nullptr );
            osl_getCommandArg( *pIndex, &pValue->pData );
            dp_misc::TRACE(__FILE__ ": argument value: "
                + *pValue + "\n");
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
            throw RuntimeException("cannot locate executable directory!",nullptr);
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


OUString const & getExecutableDir()
{
    return ExecutableDir::get();
}


OUString const & getProcessWorkingDir()
{
    return ProcessWorkingDir::get();
}


OUString makeAbsoluteFileUrl(
    OUString const & sys_path, OUString const & base_url )
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
        else
        {
            throw RuntimeException("cannot get file url from system path: " +
                sys_path );
        }
    }

    OUString abs;
    if (osl_getAbsoluteFileURL(
            base_url.pData, file_url.pData, &abs.pData ) != osl_File_E_None)
    {
        throw RuntimeException(
            "making absolute file url failed: \"" + base_url
            + "\" (base-url) and \"" + file_url + "\" (file-url)!" );
    }
    return abs[ abs.getLength() -1 ] == '/'
        ? abs.copy( 0, abs.getLength() -1 ) : abs;
}


namespace {


inline void printf_space( sal_Int32 space )
{
    while (space--)
        dp_misc::writeConsole("  ");
}


void printf_line(
    OUString const & name, OUString const & value, sal_Int32 level )
{
   printf_space( level );
    dp_misc::writeConsole(name + ": " + value + "\n");
}


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
        printf_line( "Identifier", id.Value, level );
    OUString version(xPackage->getVersion());
    if (!version.isEmpty())
        printf_line( "Version", version, level + 1 );
    printf_line( "URL", xPackage->getURL(), level + 1 );

    beans::Optional< beans::Ambiguous<sal_Bool> > option(
        xPackage->isRegistered( Reference<task::XAbortChannel>(), xCmdEnv ) );
    OUString value;
    if (option.IsPresent) {
        beans::Ambiguous<sal_Bool> const & reg = option.Value;
        if (reg.IsAmbiguous)
            value = "unknown";
        else
            value = reg.Value ? OUString("yes") : OUString("no");
    }
    else
        value = "n/a";
    printf_line( "is registered", value, level + 1 );

    const Reference<deployment::XPackageTypeInfo> xPackageType(
        xPackage->getPackageType() );
    OSL_ASSERT( xPackageType.is() );
    if (xPackageType.is()) {
        printf_line( "Media-Type", xPackageType->getMediaType(), level + 1 );
    }
    printf_line( "Description", xPackage->getDescription(), level + 1 );
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
        printf_line( "Identifier", id, 0 );
        printf_space(1);
        dp_misc::writeConsole("License not accepted\n\n");
}


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
            dp_misc::writeConsole("\n");
        }
    }
}


namespace {


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


Reference<XComponentContext> connectToOffice(
    Reference<XComponentContext> const & xLocalComponentContext,
    bool verbose )
{
    Sequence<OUString> args( 3 );
    args[ 0 ] = "--nologo";
    args[ 1 ] = "--nodefault";

    OUString pipeId( ::dp_misc::generateRandomPipeId() );
    OUStringBuffer buf;
    buf.append( "--accept=pipe,name=" );
    buf.append( pipeId );
    buf.append( ";urp;" );
    args[ 2 ] = buf.makeStringAndClear();
    OUString appURL( getExecutableDir() + "/soffice" );

    if (verbose)
    {
        dp_misc::writeConsole(
            "Raising process: " + appURL +
            "\nArguments: --nologo --nodefault " + args[2] +
            "\n");
    }

    ::dp_misc::raiseProcess( appURL, args );

    if (verbose)
        dp_misc::writeConsole("OK.  Connecting...");

    OSL_ASSERT( buf.isEmpty() );
    buf.append( "uno:pipe,name=" );
    buf.append( pipeId );
    buf.append( ";urp;StarOffice.ComponentContext" );
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
    OUString sBootstrap("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}");
    rtl::Bootstrap::expandMacros(sBootstrap);
    OUString sAbs;
    if (::osl::File::E_None ==  ::osl::File::getAbsoluteFileURL(
        sBootstrap, ".lock", sAbs))
    {
        if (::osl::File::E_None ==
            ::osl::File::getSystemPathFromFileURL(sAbs, sBootstrap))
        {
            ret = sBootstrap;
        }
    }

    return ret;
}

Reference<XComponentContext> getUNO(
    bool verbose, bool shared, bool bGui,
    Reference<XComponentContext> & out_localContext)
{
    // do not create any user data (for the root user) in --shared mode:
    if (shared) {
        rtl::Bootstrap::set("CFG_CacheUrl", OUString());
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
        if (! s_lockfile.check( nullptr ))
        {
            OUString sMsg(ResId(RID_STR_CONCURRENTINSTANCE, *DeploymentResMgr::get()));
            //Create this string before we call DeInitVCL, because this will kill
            //the ResMgr
            OUString sError(ResId(RID_STR_UNOPKG_ERROR, *DeploymentResMgr::get()));

            sMsg += "\n" + getLockFilePath();

            if (bGui)
            {
                //We show a message box or print to the console that there
                //is another instance already running
                if ( ! InitVCL() )
                    throw RuntimeException( "Cannot initialize VCL!" );
                {
                    ScopedVclPtrInstance< WarningBox > warn(nullptr, WB_OK | WB_DEF_OK, sMsg);
                    warn->SetText(utl::ConfigManager::getProductName());
                    warn->SetIcon(0);
                    warn->Execute();
                }
                DeInitVCL();
            }

            throw LockFileException(sError + sMsg);
        }
    }

    return xComponentContext;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
