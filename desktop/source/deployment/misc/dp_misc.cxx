/*************************************************************************
 *
 *  $RCSfile: dp_misc.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-04-13 12:07:02 $
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
#include "rtl/uri.hxx"
#include "rtl/digest.h"
#include "osl/file.hxx"
#include "osl/pipe.hxx"
#include "com/sun/star/util/XMacroExpander.hpp"


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

namespace dp_misc
{

//==============================================================================
OUString expand_reg_url(
    OUString const & url, Reference< XComponentContext > const & xContext )
{
    Reference< util::XMacroExpander > xMacroExpander(
        xContext->getValueByName(
            OUSTR("/singletons/com.sun.star.util.theMacroExpander") ),
        UNO_QUERY_THROW );
    if (url.matchIgnoreAsciiCaseAsciiL(
            RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.expand:") ))
    {
        // cut protocol:
        OUString macro( url.copy( sizeof ("vnd.sun.star.expand:") -1 ) );
        // decode uric class chars:
        macro = ::rtl::Uri::decode(
            macro, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
        // expand macro string:
        OUString ret( xMacroExpander->expandMacros( macro ) );
// #if OSL_DEBUG_LEVEL > 1
//         {
//         OUStringBuffer buf( 128 );
//         buf.appendAscii(
//             RTL_CONSTASCII_STRINGPARAM(__FILE__" expand_reg_url(): ") );
//         buf.append( url );
//         buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" => ") );
//         buf.append( macro );
//         buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" => ") );
//         buf.append( ret );
//         OString cstr(
//             OUStringToOString(
//                 buf.makeStringAndClear(), osl_getThreadTextEncoding() ) );
//         OSL_TRACE( "%s", cstr.getStr() );
//         }
// #endif
        return ret;
    }
    else
    {
        return url;
    }
}

enum t_status { RUNNING, NOT_RUNNING, INIT_ME };
//==============================================================================
bool office_is_running( Reference< XComponentContext > const & xContext )
{
    static t_status s_status = INIT_ME;
    if (s_status == INIT_ME)
    {
        Reference< util::XMacroExpander > xMacroExpander(
            xContext->getValueByName(
                OUSTR("/singletons/com.sun.star.util.theMacroExpander") ),
            UNO_QUERY_THROW );
        OUString user_path(
            xMacroExpander->expandMacros( OUSTR( "${$SYSBINDIR/"
                                                 SAL_CONFIGFILE("bootstrap")
                                                 ":UserInstallation}") ) );
        // normalize path:
        ::osl::FileStatus status( FileStatusMask_FileURL );
        ::osl::DirectoryItem dirItem;
        if (::osl::DirectoryItem::get( user_path, dirItem )
            != ::osl::DirectoryItem::E_None ||
            dirItem.getFileStatus( status )
            != ::osl::DirectoryItem::E_None ||
            !status.isValid( FileStatusMask_FileURL ) ||
            ::osl::FileBase::getAbsoluteFileURL(
                OUString(), status.getFileURL(), user_path )
            != ::osl::FileBase::E_None)
        {
            throw RuntimeException(
                OUSTR("Cannot normalize path ") + user_path, 0 );
        }

        rtlDigest digest = rtl_digest_create( rtl_Digest_AlgorithmMD5 );
        if (digest <= 0)
        {
            throw RuntimeException(
                OUSTR("cannot get digest rtl_Digest_AlgorithmMD5!"), 0 );
        }

        sal_uInt8 const * data =
            reinterpret_cast< sal_uInt8 const * >(user_path.getStr());
        sal_Size size = (user_path.getLength() * sizeof (sal_Unicode));
        sal_uInt32 md5_key_len = rtl_digest_queryLength( digest );
        sal_uInt8 * md5_buf = new sal_uInt8 [ md5_key_len ];

        rtl_digest_init( digest, data, static_cast< sal_uInt32 >(size) );
        rtl_digest_update( digest, data, static_cast< sal_uInt32 >(size) );
        rtl_digest_get( digest, md5_buf, md5_key_len );
        rtl_digest_destroy( digest );

        // create hex-value string from the MD5 value to keep
        // the string size minimal
        ::rtl::OUStringBuffer buf;
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("SingleOfficeIPC_") );
        for ( sal_uInt32 i = 0; i < md5_key_len; ++i )
            buf.append( static_cast< sal_Int32 >(md5_buf[ i ]), 0x10 );

        delete [] md5_buf;

        OUString pipe_id( buf.makeStringAndClear() );
        ::osl::Security sec;
        ::osl::Pipe pipe( pipe_id, osl_Pipe_OPEN, sec );
        s_status = (pipe.is() ? RUNNING : NOT_RUNNING);
    }
    return (s_status == RUNNING);
}

}
