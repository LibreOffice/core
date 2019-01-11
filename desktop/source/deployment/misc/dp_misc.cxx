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
#include <config_features.h>
#include <chrono>

#include <dp_misc.h>
#include <dp_version.hxx>
#include <dp_interact.h>
#include <rtl/uri.hxx>
#include <rtl/digest.h>
#include <rtl/random.h>
#include <rtl/bootstrap.hxx>
#include <sal/log.hxx>
#include <unotools/bootstrap.hxx>
#include <osl/file.hxx>
#include <osl/pipe.hxx>
#include <osl/security.hxx>
#include <osl/thread.hxx>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/bridge/BridgeFactory.hpp>
#include <com/sun/star/bridge/UnoUrlResolver.hpp>
#include <com/sun/star/bridge/XUnoUrlResolver.hpp>
#include <com/sun/star/deployment/ExtensionManager.hpp>
#include <com/sun/star/task/OfficeRestartManager.hpp>
#include <memory>
#include <string_view>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <salhelper/linkhelper.hxx>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

#if defined(_WIN32)
#define SOFFICE1 "soffice.exe"
#define SBASE "sbase.exe"
#define SCALC "scalc.exe"
#define SDRAW "sdraw.exe"
#define SIMPRESS "simpress.exe"
#define SWRITER "swriter.exe"
#endif

#ifdef MACOSX
#define SOFFICE2 "soffice"
#else
#define SOFFICE2 "soffice.bin"
#endif

namespace dp_misc {
namespace {

struct UnoRc : public rtl::StaticWithInit<
    std::shared_ptr<rtl::Bootstrap>, UnoRc> {
    const std::shared_ptr<rtl::Bootstrap> operator () () {
        OUString unorc( "$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("louno") );
        ::rtl::Bootstrap::expandMacros( unorc );
        std::shared_ptr< ::rtl::Bootstrap > ret(
            new ::rtl::Bootstrap( unorc ) );
        OSL_ASSERT( ret->getHandle() != nullptr );
        return ret;
    }
};

struct OfficePipeId : public rtl::StaticWithInit<OUString, OfficePipeId> {
    const OUString operator () ();
};

const OUString OfficePipeId::operator () ()
{
    OUString userPath;
    ::utl::Bootstrap::PathStatus aLocateResult =
    ::utl::Bootstrap::locateUserInstallation( userPath );
    if (!(aLocateResult == ::utl::Bootstrap::PATH_EXISTS ||
        aLocateResult == ::utl::Bootstrap::PATH_VALID))
    {
        throw Exception("Extension Manager: Could not obtain path for UserInstallation.", nullptr);
    }

    rtlDigest digest = rtl_digest_create( rtl_Digest_AlgorithmMD5 );
    if (!digest) {
        throw RuntimeException("cannot get digest rtl_Digest_AlgorithmMD5!", nullptr );
    }

    sal_uInt8 const * data =
        reinterpret_cast<sal_uInt8 const *>(userPath.getStr());
    std::size_t size = userPath.getLength() * sizeof (sal_Unicode);
    sal_uInt32 md5_key_len = rtl_digest_queryLength( digest );
    std::unique_ptr<sal_uInt8[]> md5_buf( new sal_uInt8 [ md5_key_len ] );

    rtl_digest_init( digest, data, static_cast<sal_uInt32>(size) );
    rtl_digest_update( digest, data, static_cast<sal_uInt32>(size) );
    rtl_digest_get( digest, md5_buf.get(), md5_key_len );
    rtl_digest_destroy( digest );

    // create hex-value string from the MD5 value to keep
    // the string size minimal
    OUStringBuffer buf;
    buf.append( "SingleOfficeIPC_" );
    for ( sal_uInt32 i = 0; i < md5_key_len; ++i ) {
        buf.append( static_cast<sal_Int32>(md5_buf[ i ]), 0x10 );
    }
    return buf.makeStringAndClear();
}

bool existsOfficePipe()
{
    OUString const & pipeId = OfficePipeId::get();
    if (pipeId.isEmpty())
        return false;
    ::osl::Security sec;
    ::osl::Pipe pipe( pipeId, osl_Pipe_OPEN, sec );
    return pipe.is();
}

//get modification time
bool getModifyTimeTargetFile(const OUString &rFileURL, TimeValue &rTime)
{
    salhelper::LinkResolver aResolver(osl_FileStatus_Mask_ModifyTime);

    if (aResolver.fetchFileStatus(rFileURL) != osl::FileBase::E_None)
        return false;

    rTime = aResolver.m_aStatus.getModifyTime();

    return true;
}

//Returns true if the Folder was more recently modified then
//the lastsynchronized file. That is the repository needs to
//be synchronized.
bool compareExtensionFolderWithLastSynchronizedFile(
    OUString const & folderURL, OUString const & fileURL)
{
    bool bNeedsSync = false;
    ::osl::DirectoryItem itemExtFolder;
    ::osl::File::RC err1 =
          ::osl::DirectoryItem::get(folderURL, itemExtFolder);
    //If it does not exist, then there is nothing to be done
    if (err1 == ::osl::File::E_NOENT)
    {
        return false;
    }
    else if (err1 != ::osl::File::E_None)
    {
        OSL_FAIL("Cannot access extension folder");
        return true; //sync just in case
    }

    //If last synchronized does not exist, then OOo is started for the first time
    ::osl::DirectoryItem itemFile;
    ::osl::File::RC err2 = ::osl::DirectoryItem::get(fileURL, itemFile);
    if (err2 == ::osl::File::E_NOENT)
    {
        return true;

    }
    else if (err2 != ::osl::File::E_None)
    {
        OSL_FAIL("Cannot access file lastsynchronized");
        return true; //sync just in case
    }

    //compare the modification time of the extension folder and the last
    //modified file
    TimeValue timeFolder;
    if (getModifyTimeTargetFile(folderURL, timeFolder))
    {
        TimeValue timeFile;
        if (getModifyTimeTargetFile(fileURL, timeFile))
        {
            if (timeFile.Seconds < timeFolder.Seconds)
                bNeedsSync = true;
        }
        else
        {
            OSL_ASSERT(false);
            bNeedsSync = true;
        }
    }
    else
    {
        OSL_ASSERT(false);
        bNeedsSync = true;
    }

    return bNeedsSync;
}

bool needToSyncRepository(OUString const & name)
{
    OUString folder;
    OUString file;
    if ( name == "bundled" )
    {
        folder = "$BUNDLED_EXTENSIONS";
        file = "$BUNDLED_EXTENSIONS_USER/lastsynchronized";
    }
    else if ( name == "shared" )
    {
        folder = "$UNO_SHARED_PACKAGES_CACHE/uno_packages";
        file = "$SHARED_EXTENSIONS_USER/lastsynchronized";
    }
    else
    {
        OSL_ASSERT(false);
        return true;
    }
    ::rtl::Bootstrap::expandMacros(folder);
    ::rtl::Bootstrap::expandMacros(file);
    return compareExtensionFolderWithLastSynchronizedFile(
        folder, file);
}


} // anon namespace


namespace {
OUString encodeForRcFile( OUString const & str )
{
    // escape $\{} (=> rtl bootstrap files)
    OUStringBuffer buf;
    sal_Int32 pos = 0;
    const sal_Int32 len = str.getLength();
    for ( ; pos < len; ++pos ) {
        sal_Unicode c = str[ pos ];
        switch (c) {
        case '$':
        case '\\':
        case '{':
        case '}':
            buf.append( '\\' );
            break;
        }
        buf.append( c );
    }
    return buf.makeStringAndClear();
}
}


OUString makeURL( OUString const & baseURL, OUString const & relPath_ )
{
    OUStringBuffer buf;
    if (baseURL.getLength() > 1 && baseURL[ baseURL.getLength() - 1 ] == '/')
        buf.append( std::u16string_view(baseURL).substr(0, baseURL.getLength() - 1) );
    else
        buf.append( baseURL );
    OUString relPath(relPath_);
    if( relPath.startsWith("/") )
        relPath = relPath.copy( 1 );
    if (!relPath.isEmpty())
    {
        buf.append( '/' );
        if (baseURL.match( "vnd.sun.star.expand:" )) {
            // encode for macro expansion: relPath is supposed to have no
            // macros, so encode $, {} \ (bootstrap mimic)
            relPath = encodeForRcFile(relPath);

            // encode once more for vnd.sun.star.expand schema:
            // vnd.sun.star.expand:$UNO_...
            // will expand to file-url
            relPath = ::rtl::Uri::encode( relPath, rtl_UriCharClassUric,
                                          rtl_UriEncodeIgnoreEscapes,
                                          RTL_TEXTENCODING_UTF8 );
        }
        buf.append( relPath );
    }
    return buf.makeStringAndClear();
}

OUString makeURLAppendSysPathSegment( OUString const & baseURL, OUString const & segment )
{
    OSL_ASSERT(segment.indexOf(u'/') == -1);

    ::rtl::Uri::encode(
        segment, rtl_UriCharClassPchar, rtl_UriEncodeIgnoreEscapes,
        RTL_TEXTENCODING_UTF8);
    return makeURL(baseURL, segment);
}


OUString expandUnoRcTerm( OUString const & term_ )
{
    OUString term(term_);
    UnoRc::get()->expandMacrosFrom( term );
    return term;
}

OUString makeRcTerm( OUString const & url )
{
    OSL_ASSERT( url.match( "vnd.sun.star.expand:" ));
    if (url.match( "vnd.sun.star.expand:" )) {
        // cut protocol:
        OUString rcterm( url.copy( sizeof ("vnd.sun.star.expand:") - 1 ) );
        // decode uric class chars:
        rcterm = ::rtl::Uri::decode(
            rcterm, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
        return rcterm;
    }
    else
        return url;
}


OUString expandUnoRcUrl( OUString const & url )
{
    if (url.match( "vnd.sun.star.expand:" )) {
        // cut protocol:
        OUString rcurl( url.copy( sizeof ("vnd.sun.star.expand:") - 1 ) );
        // decode uric class chars:
        rcurl = ::rtl::Uri::decode(
            rcurl, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
        // expand macro string:
        UnoRc::get()->expandMacrosFrom( rcurl );
        return rcurl;
    }
    else {
        return url;
    }
}


bool office_is_running()
{
    //We need to check if we run within the office process. Then we must not use the pipe, because
    //this could cause a deadlock. This is actually a workaround for i82778
    OUString sFile;
    oslProcessError err = osl_getExecutableFile(& sFile.pData);
    bool ret = false;
    if (osl_Process_E_None == err)
    {
        sFile = sFile.copy(sFile.lastIndexOf('/') + 1);
        if (
#if defined UNIX
            sFile == SOFFICE2
#elif defined WNT
            //osl_getExecutableFile should deliver "soffice.bin" on windows
            //even if swriter.exe, scalc.exe etc. was started. This is a bug
            //in osl_getExecutableFile
            sFile == SOFFICE1 || sFile == SOFFICE2 || sFile == SBASE || sFile == SCALC
            || sFile == SDRAW || sFile == SIMPRESS || sFile == SWRITER
#else
#error "Unsupported platform"
#endif

            )
            ret = true;
        else
            ret = existsOfficePipe();
    }
    else
    {
        OSL_FAIL("NOT osl_Process_E_None ");
        //if osl_getExecutable file then we take the risk of creating a pipe
        ret =  existsOfficePipe();
    }
    return ret;
}


oslProcess raiseProcess(
    OUString const & appURL, Sequence<OUString> const & args )
{
    ::osl::Security sec;
    oslProcess hProcess = nullptr;
    oslProcessError rc = osl_executeProcess(
        appURL.pData,
        reinterpret_cast<rtl_uString **>(
            const_cast<OUString *>(args.getConstArray()) ),
        args.getLength(),
        osl_Process_DETACHED,
        sec.getHandle(),
        nullptr, // => current working dir
        nullptr, 0, // => no env vars
        &hProcess );

    switch (rc) {
    case osl_Process_E_None:
        break;
    case osl_Process_E_NotFound:
        throw RuntimeException( "image not found!", nullptr );
    case osl_Process_E_TimedOut:
        throw RuntimeException( "timeout occurred!", nullptr );
    case osl_Process_E_NoPermission:
        throw RuntimeException( "permission denied!", nullptr );
    case osl_Process_E_Unknown:
        throw RuntimeException( "unknown error!", nullptr );
    case osl_Process_E_InvalidError:
    default:
        throw RuntimeException( "unmapped error!", nullptr );
    }

    return hProcess;
}


OUString generateRandomPipeId()
{
    // compute some good pipe id:
    static rtlRandomPool s_hPool = rtl_random_createPool();
    if (s_hPool == nullptr)
        throw RuntimeException( "cannot create random pool!?", nullptr );
    sal_uInt8 bytes[ 32 ];
    if (rtl_random_getBytes(
            s_hPool, bytes, SAL_N_ELEMENTS(bytes) ) != rtl_Random_E_None) {
        throw RuntimeException( "random pool error!?", nullptr );
    }
    OUStringBuffer buf;
    for (unsigned char byte : bytes) {
        buf.append( static_cast<sal_Int32>(byte), 0x10 );
    }
    return buf.makeStringAndClear();
}


Reference<XInterface> resolveUnoURL(
    OUString const & connectString,
    Reference<XComponentContext> const & xLocalContext,
    AbortChannel const * abortChannel )
{
    Reference<bridge::XUnoUrlResolver> xUnoUrlResolver(
        bridge::UnoUrlResolver::create( xLocalContext ) );

    for (int i = 0; i <= 40; ++i) // 20 seconds
    {
        if (abortChannel != nullptr && abortChannel->isAborted()) {
            throw ucb::CommandAbortedException( "abort!" );
        }
        try {
            return xUnoUrlResolver->resolve( connectString );
        }
        catch (const connection::NoConnectException &) {
            if (i < 40)
            {
                ::osl::Thread::wait( std::chrono::milliseconds(500) );
            }
            else throw;
        }
    }
    return nullptr; // warning C4715
}

#ifdef _WIN32
static void writeConsoleWithStream(OUString const & sText, HANDLE stream)
{
    DWORD nWrittenChars = 0;
    WriteFile(stream, sText.getStr(),
        sText.getLength() * 2, &nWrittenChars, nullptr);
}
#else
static void writeConsoleWithStream(OUString const & sText, FILE * stream)
{
    OString s = OUStringToOString(sText, osl_getThreadTextEncoding());
    fprintf(stream, "%s", s.getStr());
    fflush(stream);
}
#endif

void writeConsole(OUString const & sText)
{
#ifdef _WIN32
    writeConsoleWithStream(sText, GetStdHandle(STD_OUTPUT_HANDLE));
#else
    writeConsoleWithStream(sText, stdout);
#endif
}

void writeConsoleError(OUString const & sText)
{
#ifdef _WIN32
    writeConsoleWithStream(sText, GetStdHandle(STD_ERROR_HANDLE));
#else
    writeConsoleWithStream(sText, stderr);
#endif
}

OUString readConsole()
{
#ifdef _WIN32
    sal_Unicode aBuffer[1024];
    DWORD   dwRead = 0;
    //unopkg.com feeds unopkg.exe with wchar_t|s
    if (ReadFile( GetStdHandle(STD_INPUT_HANDLE), &aBuffer, sizeof(aBuffer), &dwRead, nullptr ) )
    {
        OSL_ASSERT((dwRead % 2) == 0);
        OUString value( aBuffer, dwRead / 2);
        return value.trim();
    }
#else
    char buf[1024];
    memset(buf, 0, 1024);
    // read one char less so that the last char in buf is always zero
    if (fgets(buf, 1024, stdin) != nullptr)
    {
        OUString value = OStringToOUString(OString(buf), osl_getThreadTextEncoding());
        return value.trim();
    }
#endif
    throw css::uno::RuntimeException("reading from stdin failed");
}

void TRACE(OUString const & sText)
{
    SAL_INFO("desktop.deployment", sText);
}

void syncRepositories(
    bool force, Reference<ucb::XCommandEnvironment> const & xCmdEnv)
{
    OUString sDisable;
    ::rtl::Bootstrap::get( "DISABLE_EXTENSION_SYNCHRONIZATION", sDisable, OUString() );
    if (!sDisable.isEmpty())
        return;

    Reference<deployment::XExtensionManager> xExtensionManager;
    //synchronize shared before bundled otherwise there are
    //more revoke and registration calls.
    bool bModified = false;
    if (force || needToSyncRepository("shared") || needToSyncRepository("bundled"))
    {
        xExtensionManager =
            deployment::ExtensionManager::get(
                comphelper::getProcessComponentContext());

        if (xExtensionManager.is())
        {
            bModified = xExtensionManager->synchronize(
                Reference<task::XAbortChannel>(), xCmdEnv);
        }
    }
#if !HAVE_FEATURE_MACOSX_SANDBOX
    if (bModified && !comphelper::LibreOfficeKit::isActive())
    {
        Reference<task::XRestartManager> restarter(task::OfficeRestartManager::get(comphelper::getProcessComponentContext()));
        if (restarter.is())
        {
            restarter->requestRestart(xCmdEnv.is() ? xCmdEnv->getInteractionHandler() :
                                      Reference<task::XInteractionHandler>());
        }
     }
#endif
}

void disposeBridges(Reference<css::uno::XComponentContext> const & ctx)
{
    if (!ctx.is())
        return;

    Reference<css::bridge::XBridgeFactory2> bridgeFac( css::bridge::BridgeFactory::create(ctx) );

    const Sequence< Reference<css::bridge::XBridge> >seqBridges = bridgeFac->getExistingBridges();
    for (sal_Int32 i = 0; i < seqBridges.getLength(); i++)
    {
        Reference<css::lang::XComponent> comp(seqBridges[i], UNO_QUERY);
        if (comp.is())
        {
            try {
                comp->dispose();
            }
            catch ( const css::lang::DisposedException& )
            {
            }
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
