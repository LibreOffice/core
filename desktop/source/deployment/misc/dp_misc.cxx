/*************************************************************************
 *
 *  $RCSfile: dp_misc.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 12:08:13 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "dp_misc.h"
#include "dp_interact.h"
#include "rtl/uri.hxx"
#include "rtl/digest.h"
#include "rtl/random.h"
#include "rtl/bootstrap.hxx"
#include "osl/file.hxx"
#include "osl/pipe.hxx"
#include "osl/security.hxx"
#include "osl/thread.hxx"
#include "osl/mutex.hxx"
#include "com/sun/star/ucb/CommandAbortedException.hpp"
#include "com/sun/star/bridge/UnoUrlResolver.hpp"
#include "com/sun/star/bridge/XUnoUrlResolver.hpp"


using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace dp_misc {

static OUString s_platform, s_os = OUSTR("$_OS"), s_arch = OUSTR("$_ARCH");

//==============================================================================
OUString const & getPlatformString()
{
    if (s_platform.getLength() == 0) {
        ::osl::MutexGuard guard( ::osl::Mutex::getGlobalMutex() );
        ::rtl::Bootstrap::expandMacros( s_os );
        ::rtl::Bootstrap::expandMacros( s_arch );
        ::rtl::OUStringBuffer buf;
        buf.append( s_os );
        buf.append( static_cast<sal_Unicode>('_') );
        buf.append( s_arch );
        s_platform = buf.makeStringAndClear();
    }
    return s_platform;
}
//==============================================================================
bool platform_fits( OUString const & platform_string )
{
    OUString const & thisPlatform = getPlatformString();
    sal_Int32 index = 0;
    for (;;) {
        OUString token( platform_string.getToken( 0, ',', index ).trim() );
        // check if this platform:
        if (token.equalsIgnoreAsciiCase( thisPlatform ) ||
            (token.indexOf( '_' ) < 0 && /* check OS part only */
             token.equalsIgnoreAsciiCase( s_os ))) {
            return true;
        }
        if (index < 0)
            break;
    }
    return false;
}

//==============================================================================
OUString make_url( OUString const & base_url, OUString const & url )
{
    // xxx todo: wait for SB's api
    ::rtl::OUStringBuffer buf;
    buf.append( base_url );
    if (base_url.getLength() > 0 &&
        base_url[ base_url.getLength() - 1 ] != '/') {
        buf.append( static_cast< sal_Unicode >('/') );
    }
    if (url.getLength() > 0 && url[ 0 ] == '/')
        buf.append( url.copy( 1 ) );
    else
        buf.append( url );
    return buf.makeStringAndClear();
}

//==============================================================================
OUString expand_url( OUString const & url )
{
    if (url.matchIgnoreAsciiCaseAsciiL(
            RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.expand:") )) {
        static ::rtl::Bootstrap * s_punorc = 0;
        if (s_punorc == 0) {
            OUString unorc = OUSTR("$ORIGIN/" SAL_CONFIGFILE("uno"));
            ::rtl::Bootstrap::expandMacros( unorc );
            static ::rtl::Bootstrap s_unorc( unorc );
            OSL_ASSERT( s_unorc.getHandle() != 0 );
            s_punorc = &s_unorc;
        }

        // cut protocol:
        OUString macro( url.copy( sizeof ("vnd.sun.star.expand:") - 1 ) );
        // decode uric class chars:
        macro = ::rtl::Uri::decode(
            macro, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
        // expand macro string:
        s_punorc->expandMacrosFrom( macro );
        return macro;
    } else {
        return url;
    }
}

//==============================================================================
bool office_is_running()
{
    static OUString s_pipeId;
    if (s_pipeId.getLength() == 0) {
        OUString userPath = OUSTR("${$SYSBINDIR/" SAL_CONFIGFILE("bootstrap")
                                  ":UserInstallation}");
        ::rtl::Bootstrap::expandMacros( userPath );
        OSL_ASSERT( userPath.getLength() > 0 );

        // normalize path:
        ::osl::FileStatus status( FileStatusMask_FileURL );
        ::osl::DirectoryItem dirItem;
        if (::osl::DirectoryItem::get( userPath, dirItem )
            != ::osl::DirectoryItem::E_None ||
            dirItem.getFileStatus( status )
            != ::osl::DirectoryItem::E_None ||
            !status.isValid( FileStatusMask_FileURL ) ||
            ::osl::FileBase::getAbsoluteFileURL(
                OUString(), status.getFileURL(), userPath )
            != ::osl::FileBase::E_None) {
            return false;
        }

        rtlDigest digest = rtl_digest_create( rtl_Digest_AlgorithmMD5 );
        if (digest <= 0)
            throw RuntimeException(
                OUSTR("cannot get digest rtl_Digest_AlgorithmMD5!"), 0 );

        sal_uInt8 const * data =
            reinterpret_cast< sal_uInt8 const * >(userPath.getStr());
        sal_Size size = (userPath.getLength() * sizeof (sal_Unicode));
        sal_uInt32 md5_key_len = rtl_digest_queryLength( digest );
        sal_uInt8 * md5_buf = new sal_uInt8 [ md5_key_len ];

        rtl_digest_init( digest, data, static_cast<sal_uInt32>(size) );
        rtl_digest_update( digest, data, static_cast<sal_uInt32>(size) );
        rtl_digest_get( digest, md5_buf, md5_key_len );
        rtl_digest_destroy( digest );

        // create hex-value string from the MD5 value to keep
        // the string size minimal
        ::rtl::OUStringBuffer buf;
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("SingleOfficeIPC_") );
        for ( sal_uInt32 i = 0; i < md5_key_len; ++i )
            buf.append( static_cast<sal_Int32>(md5_buf[ i ]), 0x10 );

        delete [] md5_buf;
        s_pipeId = buf.makeStringAndClear();
    }

    ::osl::Security sec;
    ::osl::Pipe pipe( s_pipeId, osl_Pipe_OPEN, sec );
    return pipe.is();
}

//==============================================================================
oslProcess raiseProcess(
    OUString const & appURL, Sequence<OUString> const & args )
{
    ::osl::Security sec;
    sal_Int32 pos = args.getLength();
    OUString const * pargs = args.getConstArray();
    rtl_uString ** parArgs = new rtl_uString * [ pos ];
    for ( ; pos--; )
        parArgs[ pos ] = pargs[ pos ].pData;

    oslProcess hProcess = 0;
    oslProcessError rc = osl_executeProcess(
        appURL.pData, parArgs, args.getLength(),
        osl_Process_DETACHED,
        sec.getHandle(),
        0, // => current working dir
        0, 0, // => no env vars
        &hProcess );
    delete [] parArgs;

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
            s_hPool, bytes, ARLEN(bytes) ) != rtl_Random_E_None)
        throw RuntimeException( OUSTR("random pool error!?"), 0 );

    ::rtl::OUStringBuffer buf;
    for ( sal_uInt32 i = 0; i < ARLEN(bytes); ++i )
        buf.append( static_cast<sal_Int32>(bytes[ i ]), 0x10 );

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

    for (;;) {
        if (abortChannel != 0 && abortChannel->isAborted())
            throw ucb::CommandAbortedException(
                OUSTR("abort!"), Reference<XInterface>() );

        try {
            return xUnoUrlResolver->resolve( connectString );
        }
        catch (connection::NoConnectException &) {
            TimeValue tv = { 0 /* secs */, 500000000 /* nanosecs */ };
            ::osl::Thread::wait( tv );
        }
    }
}

}
