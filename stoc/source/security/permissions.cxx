/*************************************************************************
 *
 *  $RCSfile: permissions.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 17:13:40 $
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

#include <vector>

#include <osl/process.h>
#include <osl/socket.hxx>
#include <osl/mutex.hxx>

#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/security/RuntimePermission.hpp>
#include <com/sun/star/security/AllPermission.hpp>
#include <com/sun/star/io/FilePermission.hpp>
#include <com/sun/star/connection/SocketPermission.hpp>
#include <com/sun/star/security/AccessControlException.hpp>

#include "permissions.h"

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


using namespace ::std;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

namespace stoc_sec
{

//--------------------------------------------------------------------------------------------------
static inline sal_Int32 makeMask(
    OUString const & items, char const * const * strings ) SAL_THROW( () )
{
    sal_Int32 mask = 0;

    sal_Int32 n = 0;
    do
    {
        OUString item( items.getToken( 0, ',', n ).trim() );
        if (! item.getLength())
            continue;
        sal_Int32 nPos = 0;
        while (strings[ nPos ])
        {
            if (item.equalsAscii( strings[ nPos ] ))
            {
                mask |= (0x80000000 >> nPos);
                break;
            }
            ++nPos;
        }
#if OSL_DEBUG_LEVEL > 0
        if (! strings[ nPos ])
        {
            OUStringBuffer buf( 48 );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("### ignoring unknown socket action: ") );
            buf.append( item );
            ::rtl::OString str( ::rtl::OUStringToOString(
                buf.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US ) );
            OSL_TRACE( str.getStr() );
        }
#endif
    }
    while (n >= 0); // all items
    return mask;
}
//--------------------------------------------------------------------------------------------------
static inline OUString makeStrings(
    sal_Int32 mask, char const * const * strings ) SAL_THROW( () )
{
    OUStringBuffer buf( 48 );
    while (mask)
    {
        if (0x80000000 & mask)
        {
            buf.appendAscii( *strings );
            if (mask << 1) // more items following
                buf.append( (sal_Unicode)',' );
        }
        mask = (mask << 1);
        ++strings;
    }
    return buf.makeStringAndClear();
}

//##################################################################################################

//==================================================================================================
class SocketPermission : public Permission
{
    static char const * s_actions [];
    sal_Int32 m_actions;

    OUString m_host;
    sal_Int32 m_lowerPort;
    sal_Int32 m_upperPort;
    mutable OUString m_ip;
    mutable bool m_resolveErr;
    mutable bool m_resolvedHost;
    bool m_wildCardHost;

    inline bool resolveHost() const SAL_THROW( () );

public:
    SocketPermission(
        connection::SocketPermission const & perm,
        ::rtl::Reference< Permission > const & next = ::rtl::Reference< Permission >() )
        SAL_THROW( () );
    virtual bool implies( Permission const & perm ) const SAL_THROW( () );
    virtual OUString toString() const SAL_THROW( () );
};
//__________________________________________________________________________________________________
char const * SocketPermission::s_actions [] = { "accept", "connect", "listen", "resolve", 0 };
//__________________________________________________________________________________________________
SocketPermission::SocketPermission(
    connection::SocketPermission const & perm,
    ::rtl::Reference< Permission > const & next )
    SAL_THROW( () )
    : Permission( SOCKET, next )
    , m_actions( makeMask( perm.Actions, s_actions ) )
    , m_host( perm.Host )
    , m_lowerPort( 0 )
    , m_upperPort( 65535 )
    , m_resolveErr( false )
    , m_resolvedHost( false )
    , m_wildCardHost( perm.Host.getLength() && '*' == perm.Host.pData->buffer[ 0 ] )
{
    if (0xe0000000 & m_actions) // if any (except resolve) is given => resolve implied
        m_actions |= 0x10000000;

    // separate host from portrange
    sal_Int32 colon = m_host.indexOf( ':' );
    if (colon >= 0) // port [range] given
    {
        sal_Int32 minus = m_host.indexOf( '-', colon +1 );
        if (minus < 0)
        {
            m_lowerPort = m_upperPort = m_host.copy( colon +1 ).toInt32();
        }
        else if (minus == (colon +1)) // -N
        {
            m_upperPort = m_host.copy( minus +1 ).toInt32();
        }
        else if (minus == (m_host.getLength() -1)) // N-
        {
            m_lowerPort = m_host.copy( colon +1, m_host.getLength() -1 -colon -1 ).toInt32();
        }
        else // A-B
        {
            m_lowerPort = m_host.copy( colon +1, minus - colon -1 ).toInt32();
            m_upperPort = m_host.copy( minus +1, m_host.getLength() -minus -1 ).toInt32();
        }
        m_host = m_host.copy( 0, colon );
    }
}
//__________________________________________________________________________________________________
inline bool SocketPermission::resolveHost() const SAL_THROW( () )
{
    if (m_resolveErr)
        return false;

    if (! m_resolvedHost)
    {
        // dns lookup
        SocketAddr addr;
        SocketAddr::resolveHostname( m_host, addr );
        OUString ip;
        m_resolveErr = (::osl_Socket_E_None != ::osl_getDottedInetAddrOfSocketAddr(
            addr.getHandle(), &ip.pData ));
        if (m_resolveErr)
            return false;

        MutexGuard guard( Mutex::getGlobalMutex() );
        if (! m_resolvedHost)
        {
            m_ip = ip;
            m_resolvedHost = true;
        }
    }
    return m_resolvedHost;
}
//__________________________________________________________________________________________________
bool SocketPermission::implies( Permission const & perm ) const SAL_THROW( () )
{
    // check type
    if (SOCKET != perm.m_type)
        return false;
    SocketPermission const & demanded = static_cast< SocketPermission const & >( perm );

    // check actions
    if ((m_actions & demanded.m_actions) != demanded.m_actions)
        return false;

    // check ports
    if (demanded.m_lowerPort < m_lowerPort)
        return false;
    if (demanded.m_upperPort > m_upperPort)
        return false;

    // quick check host (DNS names: RFC 1034/1035)
    if (m_host.equalsIgnoreAsciiCase( demanded.m_host ))
        return true;
    // check for host wildcards
    if (m_wildCardHost)
    {
        OUString const & demanded_host = demanded.m_host;
        if (demanded_host.getLength() <= m_host.getLength())
            return false;
        sal_Int32 len = m_host.getLength() -1; // skip star
        return (0 == ::rtl_ustr_compareIgnoreAsciiCase_WithLength(
            demanded_host.getStr() + demanded_host.getLength() - len, len,
            m_host.pData->buffer + 1, len ));
    }
    if (demanded.m_wildCardHost)
        return false;

    // compare IP addresses
    if (! resolveHost())
        return false;
    if (! demanded.resolveHost())
        return false;
    return (sal_False != m_ip.equals( demanded.m_ip ));
}
//__________________________________________________________________________________________________
OUString SocketPermission::toString() const SAL_THROW( () )
{
    OUStringBuffer buf( 48 );
    // host
    buf.appendAscii(
        RTL_CONSTASCII_STRINGPARAM("com.sun.star.connection.SocketPermission (host=\"") );
    buf.append( m_host );
    if (m_resolvedHost)
    {
        buf.append( (sal_Unicode)'[' );
        buf.append( m_ip );
        buf.append( (sal_Unicode)']' );
    }
    // port
    if (0 != m_lowerPort || 65535 != m_upperPort)
    {
        buf.append( (sal_Unicode)':' );
        if (m_lowerPort > 0)
            buf.append( m_lowerPort );
        if (m_upperPort > m_lowerPort)
        {
            buf.append( (sal_Unicode)'-' );
            if (m_upperPort < 65535)
                buf.append( m_upperPort );
        }
    }
    // actions
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\", actions=\"") );
    buf.append( makeStrings( m_actions, s_actions ) );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\")") );
    return buf.makeStringAndClear();
}

//##################################################################################################

//==================================================================================================
class FilePermission : public Permission
{
    static char const * s_actions [];
    sal_Int32 m_actions;

    OUString m_url;
    bool m_allFiles;

public:
    FilePermission(
        io::FilePermission const & perm,
        ::rtl::Reference< Permission > const & next = ::rtl::Reference< Permission >() )
        SAL_THROW( () );
    virtual bool implies( Permission const & perm ) const SAL_THROW( () );
    virtual OUString toString() const SAL_THROW( () );
};
//__________________________________________________________________________________________________
char const * FilePermission::s_actions [] = { "read", "write", "execute", "delete", 0 };
//--------------------------------------------------------------------------------------------------
static OUString const & getWorkingDir() SAL_THROW( () )
{
    static OUString * s_workingDir = 0;
    if (! s_workingDir)
    {
        OUString workingDir;
        ::osl_getProcessWorkingDir( &workingDir.pData );

        MutexGuard guard( Mutex::getGlobalMutex() );
        if (! s_workingDir)
        {
            static OUString s_dir( workingDir );
            s_workingDir = &s_dir;
        }
    }
    return *s_workingDir;
}
//__________________________________________________________________________________________________
FilePermission::FilePermission(
    io::FilePermission const & perm,
    ::rtl::Reference< Permission > const & next )
    SAL_THROW( () )
    : Permission( FILE, next )
    , m_actions( makeMask( perm.Actions, s_actions ) )
    , m_url( perm.URL )
    , m_allFiles( sal_False != perm.URL.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("<<ALL FILES>>")) )
{
    if (! m_allFiles)
    {
        if (m_url.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("*") ))
        {
            OUStringBuffer buf( 64 );
            buf.append( getWorkingDir() );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("/*") );
            m_url = buf.makeStringAndClear();
        }
        else if (m_url.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("-") ))
        {
            OUStringBuffer buf( 64 );
            buf.append( getWorkingDir() );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("/-") );
            m_url = buf.makeStringAndClear();
        }
        else if (0 != m_url.compareToAscii( RTL_CONSTASCII_STRINGPARAM("file:///") ))
        {
            // relative path
            OUString out;
            oslFileError rc = ::osl_getAbsoluteFileURL(
                getWorkingDir().pData, perm.URL.pData, &out.pData );
            m_url = (osl_File_E_None == rc ? out : perm.URL); // fallback
        }
#ifdef SAL_W32
        // correct win drive letters
        if (9 < m_url.getLength() && '|' == m_url[ 9 ]) // file:///X|
        {
            static OUString s_colon = OUSTR(":");
            // common case in API is a ':' (sal), so convert '|' to ':'
            m_url = m_url.replaceAt( 9, 1, s_colon );
        }
#endif
    }
}
//__________________________________________________________________________________________________
bool FilePermission::implies( Permission const & perm ) const SAL_THROW( () )
{
    // check type
    if (FILE != perm.m_type)
        return false;
    FilePermission const & demanded = static_cast< FilePermission const & >( perm );

    // check actions
    if ((m_actions & demanded.m_actions) != demanded.m_actions)
        return false;

    // check url
    if (m_allFiles)
        return true;
    if (demanded.m_allFiles)
        return false;

#ifdef SAL_W32
    if (m_url.equalsIgnoreAsciiCase( demanded.m_url ))
        return true;
#else
    if (m_url.equals( demanded.m_url ))
        return true;
#endif
    if (m_url.getLength() > demanded.m_url.getLength())
        return false;
    // check /- wildcard: all files and recursive in that path
    if (1 < m_url.getLength() &&
        0 == ::rtl_ustr_ascii_compare_WithLength( m_url.getStr() + m_url.getLength() - 2, 2, "/-" ))
    {
        // demanded url must start with granted path (including path trailing path sep)
        sal_Int32 len = m_url.getLength() -1;
#ifdef SAL_W32
        return (0 == ::rtl_ustr_compareIgnoreAsciiCase_WithLength(
                    demanded.m_url.pData->buffer, len, m_url.pData->buffer, len ));
#else
        return (0 == ::rtl_ustr_reverseCompare_WithLength(
                    demanded.m_url.pData->buffer, len, m_url.pData->buffer, len ));
#endif
    }
    // check /* wildcard: all files in that path (not recursive!)
    if (1 < m_url.getLength() &&
        0 == ::rtl_ustr_ascii_compare_WithLength( m_url.getStr() + m_url.getLength() - 2, 2, "/*" ))
    {
        // demanded url must start with granted path (including path trailing path sep)
        sal_Int32 len = m_url.getLength() -1;
#ifdef SAL_W32
        return ((0 == ::rtl_ustr_compareIgnoreAsciiCase_WithLength(
                     demanded.m_url.pData->buffer, len, m_url.pData->buffer, len )) &&
                (0 > demanded.m_url.indexOf( '/', len ))); // in addition, no deeper pathes
#else
        return ((0 == ::rtl_ustr_reverseCompare_WithLength(
                     demanded.m_url.pData->buffer, len, m_url.pData->buffer, len )) &&
                (0 > demanded.m_url.indexOf( '/', len ))); // in addition, no deeper pathes
#endif
    }
    return false;
}
//__________________________________________________________________________________________________
OUString FilePermission::toString() const SAL_THROW( () )
{
    OUStringBuffer buf( 48 );
    // url
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("com.sun.star.io.FilePermission (url=\"") );
    buf.append( m_url );
    // actions
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\", actions=\"") );
    buf.append( makeStrings( m_actions, s_actions ) );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\")") );
    return buf.makeStringAndClear();
}

//##################################################################################################

//==================================================================================================
class RuntimePermission : public Permission
{
    OUString m_name;

public:
    inline RuntimePermission(
        security::RuntimePermission const & perm,
        ::rtl::Reference< Permission > const & next = ::rtl::Reference< Permission >() )
        SAL_THROW( () )
        : Permission( RUNTIME, next )
        , m_name( perm.Name )
        {}
    virtual bool implies( Permission const & perm ) const SAL_THROW( () );
    virtual OUString toString() const SAL_THROW( () );
};
//__________________________________________________________________________________________________
bool RuntimePermission::implies( Permission const & perm ) const SAL_THROW( () )
{
    // check type
    if (RUNTIME != perm.m_type)
        return false;
    RuntimePermission const & demanded = static_cast< RuntimePermission const & >( perm );

    // check name
    return (sal_False != m_name.equals( demanded.m_name ));
}
//__________________________________________________________________________________________________
OUString RuntimePermission::toString() const SAL_THROW( () )
{
    OUStringBuffer buf( 48 );
    buf.appendAscii(
        RTL_CONSTASCII_STRINGPARAM("com.sun.star.security.RuntimePermission (name=\"") );
    buf.append( m_name );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\")") );
    return buf.makeStringAndClear();
}

//##################################################################################################

//__________________________________________________________________________________________________
bool AllPermission::implies( Permission const & ) const SAL_THROW( () )
{
    return true;
}
//__________________________________________________________________________________________________
OUString AllPermission::toString() const SAL_THROW( () )
{
    return OUSTR("com.sun.star.security.AllPermission");
}

//##################################################################################################

//__________________________________________________________________________________________________
PermissionCollection::PermissionCollection(
    Sequence< Any > const & permissions, PermissionCollection const & addition )
    SAL_THROW( (RuntimeException) )
    : m_head( addition.m_head )
{
    Any const * perms = permissions.getConstArray();
    for ( sal_Int32 nPos = permissions.getLength(); nPos--; )
    {
        Any const & perm = perms[ nPos ];
        Type const & perm_type = perm.getValueType();

        // supported permission types
        if (perm_type.equals( ::getCppuType( (io::FilePermission const *)0 ) ))
        {
            m_head = new FilePermission(
                *reinterpret_cast< io::FilePermission const * >( perm.pData ), m_head );
        }
        else if (perm_type.equals( ::getCppuType( (connection::SocketPermission const *)0 ) ))
        {
            m_head = new SocketPermission(
                *reinterpret_cast< connection::SocketPermission const * >( perm.pData ), m_head );
        }
        else if (perm_type.equals( ::getCppuType( (security::RuntimePermission const *)0 ) ))
        {
            m_head = new RuntimePermission(
                *reinterpret_cast< security::RuntimePermission const * >( perm.pData ), m_head );
        }
        else if (perm_type.equals( ::getCppuType( (security::AllPermission const *)0 ) ))
        {
            m_head = new AllPermission( m_head );
        }
        else
        {
            OUStringBuffer buf( 48 );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(
                "checking for unsupported permission type: ") );
            buf.append( perm_type.getTypeName() );
            throw RuntimeException(
                buf.makeStringAndClear(), Reference< XInterface >() );
            OSL_ASSERT( 0 );
        }
    }
}
//__________________________________________________________________________________________________
Sequence< OUString > PermissionCollection::toStrings() const SAL_THROW( () )
{
    vector< OUString > strings;
    strings.reserve( 8 );
    for ( Permission * perm = m_head.get(); perm; perm = perm->m_next.get() )
    {
        strings.push_back( perm->toString() );
    }
    return Sequence< OUString >( &strings[ 0 ], strings.size() );
}
//__________________________________________________________________________________________________
inline static bool __implies(
    ::rtl::Reference< Permission > const & head, Permission const & demanded ) SAL_THROW( () )
{
    for ( Permission * perm = head.get(); perm; perm = perm->m_next.get() )
    {
        if (perm->implies( demanded ))
            return true;
    }
    return false;
}
//__________________________________________________________________________________________________
bool PermissionCollection::implies( Permission const & perm ) const SAL_THROW( () )
{
    return __implies( m_head, perm );
}

#ifdef __DIAGNOSE
//--------------------------------------------------------------------------------------------------
static void demanded_diag(
    Permission const & perm )
    SAL_THROW( () )
{
    OUStringBuffer buf( 48 );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("demanding ") );
    buf.append( perm.toString() );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" => ok.") );
    ::rtl::OString str(
        ::rtl::OUStringToOString( buf.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US ) );
    OSL_TRACE( str.getStr() );
}
#endif
//--------------------------------------------------------------------------------------------------
static void throwAccessControlException(
    Permission const & perm, Any const & demanded_perm )
    SAL_THROW( (security::AccessControlException) )
{
    OUStringBuffer buf( 48 );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("access denied: ") );
    buf.append( perm.toString() );
    throw security::AccessControlException(
        buf.makeStringAndClear(), Reference< XInterface >(), demanded_perm );
    OSL_ASSERT( 0 );
}
//==================================================================================================
void PermissionCollection::checkPermission( Any const & perm ) const
    SAL_THROW( (RuntimeException) )
{
    Type const & demanded_type = perm.getValueType();

    // supported permission types
    // stack object of SimpleReferenceObject are ok, as long as they are not
    // assigned to a ::rtl::Reference<> (=> delete this)
    if (demanded_type.equals( ::getCppuType( (io::FilePermission const *)0 ) ))
    {
        FilePermission demanded(
            *reinterpret_cast< io::FilePermission const * >( perm.pData ) );
        if (__implies( m_head, demanded ))
        {
#ifdef __DIAGNOSE
            demanded_diag( demanded );
#endif
            return;
        }
        throwAccessControlException( demanded, perm );
    }
    else if (demanded_type.equals( ::getCppuType( (connection::SocketPermission const *)0 ) ))
    {
        SocketPermission demanded(
            *reinterpret_cast< connection::SocketPermission const * >( perm.pData ) );
        if (__implies( m_head, demanded ))
        {
#ifdef __DIAGNOSE
            demanded_diag( demanded );
#endif
            return;
        }
        throwAccessControlException( demanded, perm );
    }
    else if (demanded_type.equals( ::getCppuType( (security::RuntimePermission const *)0 ) ))
    {
        RuntimePermission demanded(
            *reinterpret_cast< security::RuntimePermission const * >( perm.pData ) );
        if (__implies( m_head, demanded ))
        {
#ifdef __DIAGNOSE
            demanded_diag( demanded );
#endif
            return;
        }
        throwAccessControlException( demanded, perm );
    }
    else if (demanded_type.equals( ::getCppuType( (security::AllPermission const *)0 ) ))
    {
        AllPermission demanded;
        if (__implies( m_head, demanded ))
        {
#ifdef __DIAGNOSE
            demanded_diag( demanded );
#endif
            return;
        }
        throwAccessControlException( demanded, perm );
    }
    else
    {
        OUStringBuffer buf( 48 );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("checking for unsupported permission type: ") );
        buf.append( demanded_type.getTypeName() );
        throw RuntimeException(
            buf.makeStringAndClear(), Reference< XInterface >() );
        OSL_ASSERT( 0 );
    }
}

}
