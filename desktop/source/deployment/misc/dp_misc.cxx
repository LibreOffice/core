/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_misc.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 13:47:17 $
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

#include "dp_misc.h"
#include "dp_interact.h"
#include "rtl/uri.hxx"
#include "rtl/digest.h"
#include "rtl/random.h"
#include "rtl/bootstrap.hxx"
#include "unotools/bootstrap.hxx"
#include "osl/file.hxx"
#include "osl/pipe.hxx"
#include "osl/security.hxx"
#include "osl/thread.hxx"
#include "osl/mutex.hxx"
#include "com/sun/star/ucb/CommandAbortedException.hpp"
#include "com/sun/star/bridge/UnoUrlResolver.hpp"
#include "com/sun/star/bridge/XUnoUrlResolver.hpp"
#include "boost/scoped_array.hpp"
#include "boost/shared_ptr.hpp"


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;


#define SOFFICE1 "soffice.exe"
#define SOFFICE2 "soffice.bin"
#define SBASE "sbase.exe"
#define SCALC "scalc.exe"
#define SDRAW "sdraw.exe"
#define SIMPRESS "simpress.exe"
#define SWRITER "swriter.exe"

namespace dp_misc {
namespace {

struct StrOperatingSystem :
        public rtl::StaticWithInit<const OUString, StrOperatingSystem> {
    const OUString operator () () {
        OUString os( RTL_CONSTASCII_USTRINGPARAM("$_OS") );
        ::rtl::Bootstrap::expandMacros( os );
        return os;
    }
};

struct StrPlatform : public rtl::StaticWithInit<
    const OUString, StrPlatform> {
    const OUString operator () () {
        ::rtl::OUStringBuffer buf;
        buf.append( StrOperatingSystem::get() );
        buf.append( static_cast<sal_Unicode>('_') );
        OUString arch( RTL_CONSTASCII_USTRINGPARAM("$_ARCH") );
        ::rtl::Bootstrap::expandMacros( arch );
        buf.append( arch );
        return buf.makeStringAndClear();
    }
};

struct UnoRc : public rtl::StaticWithInit<
    const boost::shared_ptr<rtl::Bootstrap>, UnoRc> {
    const boost::shared_ptr<rtl::Bootstrap> operator () () {
        OUString unorc( RTL_CONSTASCII_USTRINGPARAM(
                            "$OOO_BASE_DIR/program/" SAL_CONFIGFILE("uno")) );
        ::rtl::Bootstrap::expandMacros( unorc );
        ::boost::shared_ptr< ::rtl::Bootstrap > ret(
            new ::rtl::Bootstrap( unorc ) );
        OSL_ASSERT( ret->getHandle() != 0 );
        return ret;
    }
};

struct OfficePipeId : public rtl::StaticWithInit<const OUString, OfficePipeId> {
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
        throw Exception(OUSTR("Extension Manager: Could not obtain path for UserInstallation."), 0);
    }

    rtlDigest digest = rtl_digest_create( rtl_Digest_AlgorithmMD5 );
    if (digest <= 0) {
        throw RuntimeException(
            OUSTR("cannot get digest rtl_Digest_AlgorithmMD5!"), 0 );
    }

    sal_uInt8 const * data =
        reinterpret_cast<sal_uInt8 const *>(userPath.getStr());
    sal_Size size = (userPath.getLength() * sizeof (sal_Unicode));
    sal_uInt32 md5_key_len = rtl_digest_queryLength( digest );
    ::boost::scoped_array<sal_uInt8> md5_buf( new sal_uInt8 [ md5_key_len ] );

    rtl_digest_init( digest, data, static_cast<sal_uInt32>(size) );
    rtl_digest_update( digest, data, static_cast<sal_uInt32>(size) );
    rtl_digest_get( digest, md5_buf.get(), md5_key_len );
    rtl_digest_destroy( digest );

    // create hex-value string from the MD5 value to keep
    // the string size minimal
    ::rtl::OUStringBuffer buf;
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("SingleOfficeIPC_") );
    for ( sal_uInt32 i = 0; i < md5_key_len; ++i ) {
        buf.append( static_cast<sal_Int32>(md5_buf[ i ]), 0x10 );
    }
    return buf.makeStringAndClear();
}

bool existsOfficePipe()
{
    OUString const & pipeId = OfficePipeId::get();
    if (pipeId.getLength() == 0)
        return false;
    ::osl::Security sec;
    ::osl::Pipe pipe( pipeId, osl_Pipe_OPEN, sec );
    return pipe.is();
}

} // anon namespace

//==============================================================================
OUString const & getPlatformString()
{
    return StrPlatform::get();
}

//==============================================================================
bool platform_fits( OUString const & platform_string )
{
    sal_Int32 index = 0;
    for (;;)
    {
        const OUString token(
            platform_string.getToken( 0, ',', index ).trim() );
        // check if this platform:
        if (token.equalsIgnoreAsciiCase( StrPlatform::get() ) ||
            (token.indexOf( '_' ) < 0 && /* check OS part only */
             token.equalsIgnoreAsciiCase( StrOperatingSystem::get() )))
        {
            return true;
        }
        if (index < 0)
            break;
    }
    return false;
}

namespace {
inline OUString encodeForRcFile( OUString const & str )
{
    // escape $\{} (=> rtl bootstrap files)
    ::rtl::OUStringBuffer buf;
    sal_Int32 pos = 0;
    const sal_Int32 len = str.getLength();
    for ( ; pos < len; ++pos ) {
        sal_Unicode c = str[ pos ];
        switch (c) {
        case '$':
        case '\\':
        case '{':
        case '}':
            buf.append( static_cast<sal_Unicode>('\\') );
            break;
        }
        buf.append( c );
    }
    return buf.makeStringAndClear();
}
}

//==============================================================================
OUString makeURL( OUString const & baseURL, OUString const & relPath_ )
{
    ::rtl::OUStringBuffer buf;
    if (baseURL.getLength() > 1 && baseURL[ baseURL.getLength() - 1 ] == '/')
        buf.append( baseURL.copy( 0, baseURL.getLength() - 1 ) );
    else
        buf.append( baseURL );
    OUString relPath(relPath_);
    if (relPath.getLength() > 0 && relPath[ 0 ] == '/')
        relPath = relPath.copy( 1 );
    if (relPath.getLength() > 0)
    {
        buf.append( static_cast<sal_Unicode>('/') );
        if (baseURL.matchAsciiL(
                RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.expand:") )) {
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

//==============================================================================
OUString expandUnoRcTerm( OUString const & term_ )
{
    OUString term(term_);
    UnoRc::get()->expandMacrosFrom( term );
    return term;
}

//==============================================================================
OUString expandUnoRcUrl( OUString const & url )
{
    if (url.matchAsciiL( RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.expand:") )) {
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

//==============================================================================
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
            sFile.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(SOFFICE2)))
#elif defined WNT
            //osl_getExecutableFile should deliver "soffice.bin" on windows
            //even if swriter.exe, scalc.exe etc. was started. This is a bug
            //in osl_getExecutableFile
            sFile.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(SOFFICE1)))
            || sFile.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(SOFFICE2)))
            || sFile.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(SBASE)))
            || sFile.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(SCALC)))
            || sFile.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(SDRAW)))
            || sFile.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(SIMPRESS)))
            || sFile.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(SWRITER)))
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
        OSL_ENSURE(0, "NOT osl_Process_E_None ");
        //if osl_getExecutable file than we take the risk of creating a pipe
        ret =  existsOfficePipe();
    }
    return ret;
}

//==============================================================================
oslProcess raiseProcess(
    OUString const & appURL, Sequence<OUString> const & args )
{
    ::osl::Security sec;
    oslProcess hProcess = 0;
    oslProcessError rc = osl_executeProcess(
        appURL.pData,
        reinterpret_cast<rtl_uString **>(
            const_cast<OUString *>(args.getConstArray()) ),
        args.getLength(),
        osl_Process_DETACHED,
        sec.getHandle(),
        0, // => current working dir
        0, 0, // => no env vars
        &hProcess );

    switch (rc) {
    case osl_Process_E_None:
        break;
    case osl_Process_E_NotFound:
        throw RuntimeException( OUSTR("image not found!"), 0 );
    case osl_Process_E_TimedOut:
        throw RuntimeException( OUSTR("timout occured!"), 0 );
    case osl_Process_E_NoPermission:
        throw RuntimeException( OUSTR("permission denied!"), 0 );
    case osl_Process_E_Unknown:
        throw RuntimeException( OUSTR("unknown error!"), 0 );
    case osl_Process_E_InvalidError:
    default:
        throw RuntimeException( OUSTR("unmapped error!"), 0 );
    }

    return hProcess;
}

//==============================================================================
OUString generateRandomPipeId()
{
    // compute some good pipe id:
    static rtlRandomPool s_hPool = rtl_random_createPool();
    if (s_hPool == 0)
        throw RuntimeException( OUSTR("cannot create random pool!?"), 0 );
    sal_uInt8 bytes[ 32 ];
    if (rtl_random_getBytes(
            s_hPool, bytes, ARLEN(bytes) ) != rtl_Random_E_None) {
        throw RuntimeException( OUSTR("random pool error!?"), 0 );
    }
    ::rtl::OUStringBuffer buf;
    for ( sal_uInt32 i = 0; i < ARLEN(bytes); ++i ) {
        buf.append( static_cast<sal_Int32>(bytes[ i ]), 0x10 );
    }
    return buf.makeStringAndClear();
}

//==============================================================================
Reference<XInterface> resolveUnoURL(
    OUString const & connectString,
    Reference<XComponentContext> const & xLocalContext,
    AbortChannel * abortChannel )
{
    Reference<bridge::XUnoUrlResolver> xUnoUrlResolver(
        bridge::UnoUrlResolver::create( xLocalContext ) );

    for (;;)
    {
        if (abortChannel != 0 && abortChannel->isAborted()) {
            throw ucb::CommandAbortedException(
                OUSTR("abort!"), Reference<XInterface>() );
        }
        try {
            return xUnoUrlResolver->resolve( connectString );
        }
        catch (connection::NoConnectException &) {
            TimeValue tv = { 0 /* secs */, 500000000 /* nanosecs */ };
            ::osl::Thread::wait( tv );
        }
    }
}

OUString getExtensionDefaultUpdateURL()
{
    ::rtl::OUString sUrl(
        RTL_CONSTASCII_USTRINGPARAM(
        "${$BRAND_BASE_DIR/program/" SAL_CONFIGFILE("version")
        ":Version:ExtensionUpdateURL"));
    ::rtl::Bootstrap::expandMacros(sUrl);
    return sUrl;
}


}
