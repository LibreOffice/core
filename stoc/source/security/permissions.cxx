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


#include <vector>

#include <osl/process.h>
#include <osl/socket.hxx>
#include <osl/mutex.hxx>

#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>

#include <com/sun/star/security/RuntimePermission.hpp>
#include <com/sun/star/security/AllPermission.hpp>
#include <com/sun/star/io/FilePermission.hpp>
#include <com/sun/star/connection/SocketPermission.hpp>
#include <com/sun/star/security/AccessControlException.hpp>

#include "permissions.h"


using namespace ::std;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace css::uno;

namespace stoc_sec
{


static sal_Int32 makeMask(
    OUString const & items, char const * const * strings )
{
    sal_Int32 mask = 0;

    sal_Int32 n = 0;
    do
    {
        OUString item( items.getToken( 0, ',', n ).trim() );
        if ( item.isEmpty())
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
            SAL_WARN("stoc", "ignoring unknown socket action: " << item );
        }
#endif
    }
    while (n >= 0); // all items
    return mask;
}

static OUString makeStrings(
    sal_Int32 mask, char const * const * strings )
{
    OUStringBuffer buf( 48 );
    while (mask)
    {
        if (0x80000000 & mask)
        {
            buf.appendAscii( *strings );
            if ((mask << 1) != 0) // more items following
                buf.append( ',' );
        }
        mask = (mask << 1);
        ++strings;
    }
    return buf.makeStringAndClear();
}


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
    bool const m_wildCardHost;

    inline bool resolveHost() const;

public:
    SocketPermission(
        connection::SocketPermission const & perm,
        ::rtl::Reference< Permission > const & next = ::rtl::Reference< Permission >() );
    virtual bool implies( Permission const & perm ) const override;
    virtual OUString toString() const override;
};

char const * SocketPermission::s_actions [] = { "accept", "connect", "listen", "resolve", nullptr };

SocketPermission::SocketPermission(
    connection::SocketPermission const & perm,
    ::rtl::Reference< Permission > const & next )
    : Permission( SOCKET, next )
    , m_actions( makeMask( perm.Actions, s_actions ) )
    , m_host( perm.Host )
    , m_lowerPort( 0 )
    , m_upperPort( 65535 )
    , m_resolveErr( false )
    , m_resolvedHost( false )
    , m_wildCardHost( !perm.Host.isEmpty() && '*' == perm.Host.pData->buffer[ 0 ] )
{
    if (0xe0000000 & m_actions) // if any (except resolve) is given => resolve implied
        m_actions |= 0x10000000;

    // separate host from portrange
    sal_Int32 colon = m_host.indexOf( ':' );
    if (colon < 0) // port [range] not given
        return;

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
        m_upperPort = m_host.copy( minus +1 ).toInt32();
    }
    m_host = m_host.copy( 0, colon );
}

inline bool SocketPermission::resolveHost() const
{
    if (m_resolveErr)
        return false;

    if (! m_resolvedHost)
    {
        // dns lookup
        SocketAddr addr;
        SocketAddr::resolveHostname( m_host, addr );
        OUString ip;
        m_resolveErr = (::osl_Socket_Ok != ::osl_getDottedInetAddrOfSocketAddr(
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

bool SocketPermission::implies( Permission const & perm ) const
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
    return m_ip == demanded.m_ip;
}

OUString SocketPermission::toString() const
{
    OUStringBuffer buf( 48 );
    // host
    buf.append( "com.sun.star.connection.SocketPermission (host=\"" );
    buf.append( m_host );
    if (m_resolvedHost)
    {
        buf.append( '[' );
        buf.append( m_ip );
        buf.append( ']' );
    }
    // port
    if (0 != m_lowerPort || 65535 != m_upperPort)
    {
        buf.append( ':' );
        if (m_lowerPort > 0)
            buf.append( m_lowerPort );
        if (m_upperPort > m_lowerPort)
        {
            buf.append( '-' );
            if (m_upperPort < 65535)
                buf.append( m_upperPort );
        }
    }
    // actions
    buf.append( "\", actions=\"" );
    buf.append( makeStrings( m_actions, s_actions ) );
    buf.append( "\")" );
    return buf.makeStringAndClear();
}


class FilePermission : public Permission
{
    static char const * s_actions [];
    sal_Int32 const m_actions;

    OUString m_url;
    bool const m_allFiles;

public:
    FilePermission(
        io::FilePermission const & perm,
        ::rtl::Reference< Permission > const & next = ::rtl::Reference< Permission >() );
    virtual bool implies( Permission const & perm ) const override;
    virtual OUString toString() const override;
};

char const * FilePermission::s_actions [] = { "read", "write", "execute", "delete", nullptr };

static OUString const & getWorkingDir()
{
    static OUString s_workingDir = []() {
        OUString workingDir;
        ::osl_getProcessWorkingDir(&workingDir.pData);
        return workingDir;
    }();
    return s_workingDir;
}

FilePermission::FilePermission(
    io::FilePermission const & perm,
    ::rtl::Reference< Permission > const & next )
    : Permission( FILE, next )
    , m_actions( makeMask( perm.Actions, s_actions ) )
    , m_url( perm.URL )
    , m_allFiles( perm.URL == "<<ALL FILES>>" )
{
    if ( m_allFiles)
        return;

    if ( m_url == "*" )
    {
        OUStringBuffer buf( 64 );
        buf.append( getWorkingDir() );
        buf.append( "/*" );
        m_url = buf.makeStringAndClear();
    }
    else if ( m_url == "-" )
    {
        OUStringBuffer buf( 64 );
        buf.append( getWorkingDir() );
        buf.append( "/-" );
        m_url = buf.makeStringAndClear();
    }
    else if (!m_url.startsWith("file:///"))
    {
        // relative path
        OUString out;
        oslFileError rc = ::osl_getAbsoluteFileURL(
            getWorkingDir().pData, perm.URL.pData, &out.pData );
        m_url = (osl_File_E_None == rc ? out : perm.URL); // fallback
    }
#ifdef _WIN32
    // correct win drive letters
    if (9 < m_url.getLength() && '|' == m_url[ 9 ]) // file:///X|
    {
        static OUString s_colon = ":";
        // common case in API is a ':' (sal), so convert '|' to ':'
        m_url = m_url.replaceAt( 9, 1, s_colon );
    }
#endif
}

bool FilePermission::implies( Permission const & perm ) const
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

#ifdef _WIN32
    if (m_url.equalsIgnoreAsciiCase( demanded.m_url ))
        return true;
#else
    if (m_url == demanded.m_url )
        return true;
#endif
    if (m_url.getLength() > demanded.m_url.getLength())
        return false;
    // check /- wildcard: all files and recursive in that path
    if (m_url.endsWith("/-"))
    {
        // demanded url must start with granted path (including path trailing path sep)
        sal_Int32 len = m_url.getLength() -1;
#ifdef _WIN32
        return (0 == ::rtl_ustr_compareIgnoreAsciiCase_WithLength(
                    demanded.m_url.pData->buffer, len, m_url.pData->buffer, len ));
#else
        return (0 == ::rtl_ustr_reverseCompare_WithLength(
                    demanded.m_url.pData->buffer, len, m_url.pData->buffer, len ));
#endif
    }
    // check /* wildcard: all files in that path (not recursive!)
    if (m_url.endsWith("/*"))
    {
        // demanded url must start with granted path (including path trailing path sep)
        sal_Int32 len = m_url.getLength() -1;
#ifdef _WIN32
        return ((0 == ::rtl_ustr_compareIgnoreAsciiCase_WithLength(
                     demanded.m_url.pData->buffer, len, m_url.pData->buffer, len )) &&
                (0 > demanded.m_url.indexOf( '/', len ))); // in addition, no deeper paths
#else
        return ((0 == ::rtl_ustr_reverseCompare_WithLength(
                     demanded.m_url.pData->buffer, len, m_url.pData->buffer, len )) &&
                (0 > demanded.m_url.indexOf( '/', len ))); // in addition, no deeper paths
#endif
    }
    return false;
}

OUString FilePermission::toString() const
{
    OUStringBuffer buf( 48 );
    // url
    buf.append( "com.sun.star.io.FilePermission (url=\"" );
    buf.append( m_url );
    // actions
    buf.append( "\", actions=\"" );
    buf.append( makeStrings( m_actions, s_actions ) );
    buf.append( "\")" );
    return buf.makeStringAndClear();
}


class RuntimePermission : public Permission
{
    OUString const m_name;

public:
    RuntimePermission(
        security::RuntimePermission const & perm,
        ::rtl::Reference< Permission > const & next = ::rtl::Reference< Permission >() )
        : Permission( RUNTIME, next )
        , m_name( perm.Name )
        {}
    virtual bool implies( Permission const & perm ) const override;
    virtual OUString toString() const override;
};

bool RuntimePermission::implies( Permission const & perm ) const
{
    // check type
    if (RUNTIME != perm.m_type)
        return false;
    RuntimePermission const & demanded = static_cast< RuntimePermission const & >( perm );

    // check name
    return m_name == demanded.m_name;
}

OUString RuntimePermission::toString() const
{
    OUStringBuffer buf( 48 );
    buf.append( "com.sun.star.security.RuntimePermission (name=\"" );
    buf.append( m_name );
    buf.append( "\")" );
    return buf.makeStringAndClear();
}


bool AllPermission::implies( Permission const & ) const
{
    return true;
}

OUString AllPermission::toString() const
{
    return OUString("com.sun.star.security.AllPermission");
}


PermissionCollection::PermissionCollection(
    Sequence< Any > const & permissions, PermissionCollection const & addition )
    : m_head( addition.m_head )
{
    Any const * perms = permissions.getConstArray();
    for ( sal_Int32 nPos = permissions.getLength(); nPos--; )
    {
        Any const & perm = perms[ nPos ];
        Type const & perm_type = perm.getValueType();

        // supported permission types
        if (perm_type.equals( cppu::UnoType<io::FilePermission>::get()))
        {
            m_head = new FilePermission(
                *static_cast< io::FilePermission const * >( perm.pData ), m_head );
        }
        else if (perm_type.equals( cppu::UnoType<connection::SocketPermission>::get()))
        {
            m_head = new SocketPermission(
                *static_cast< connection::SocketPermission const * >( perm.pData ), m_head );
        }
        else if (perm_type.equals( cppu::UnoType<security::RuntimePermission>::get()))
        {
            m_head = new RuntimePermission(
                *static_cast< security::RuntimePermission const * >( perm.pData ), m_head );
        }
        else if (perm_type.equals( cppu::UnoType<security::AllPermission>::get()))
        {
            m_head = new AllPermission( m_head );
        }
        else
        {
            throw RuntimeException( "checking for unsupported permission type: " + perm_type.getTypeName() );
        }
    }
}
#ifdef __DIAGNOSE

Sequence< OUString > PermissionCollection::toStrings() const
{
    vector< OUString > strings;
    strings.reserve( 8 );
    for ( Permission * perm = m_head.get(); perm; perm = perm->m_next.get() )
    {
        strings.push_back( perm->toString() );
    }
    return Sequence< OUString >( strings.data(), strings.size() );
}
#endif

static bool implies(
    ::rtl::Reference< Permission > const & head, Permission const & demanded )
{
    for ( Permission * perm = head.get(); perm; perm = perm->m_next.get() )
    {
        if (perm->implies( demanded ))
            return true;
    }
    return false;
}

#ifdef __DIAGNOSE

static void demanded_diag(
    Permission const & perm )
{
    OUStringBuffer buf( 48 );
    buf.append( "demanding " );
    buf.append( perm.toString() );
    buf.append( " => ok." );
    OString str(
        OUStringToOString( buf.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US ) );
    SAL_INFO("stoc",( "%s", str.getStr() );
}
#endif

static void throwAccessControlException(
    Permission const & perm, Any const & demanded_perm )
{
    throw security::AccessControlException(
        "access denied: " + perm.toString(),
        Reference< XInterface >(), demanded_perm );
}

void PermissionCollection::checkPermission( Any const & perm ) const
{
    Type const & demanded_type = perm.getValueType();

    // supported permission types
    // stack object of SimpleReferenceObject are ok, as long as they are not
    // assigned to a ::rtl::Reference<> (=> delete this)
    if (demanded_type.equals( cppu::UnoType<io::FilePermission>::get()))
    {
        FilePermission demanded(
            *static_cast< io::FilePermission const * >( perm.pData ) );
        if (implies( m_head, demanded ))
        {
#ifdef __DIAGNOSE
            demanded_diag( demanded );
#endif
            return;
        }
        throwAccessControlException( demanded, perm );
    }
    else if (demanded_type.equals( cppu::UnoType<connection::SocketPermission>::get()))
    {
        SocketPermission demanded(
            *static_cast< connection::SocketPermission const * >( perm.pData ) );
        if (implies( m_head, demanded ))
        {
#ifdef __DIAGNOSE
            demanded_diag( demanded );
#endif
            return;
        }
        throwAccessControlException( demanded, perm );
    }
    else if (demanded_type.equals( cppu::UnoType<security::RuntimePermission>::get()))
    {
        RuntimePermission demanded(
            *static_cast< security::RuntimePermission const * >( perm.pData ) );
        if (implies( m_head, demanded ))
        {
#ifdef __DIAGNOSE
            demanded_diag( demanded );
#endif
            return;
        }
        throwAccessControlException( demanded, perm );
    }
    else if (demanded_type.equals( cppu::UnoType<security::AllPermission>::get()))
    {
        AllPermission demanded;
        if (implies( m_head, demanded ))
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
        throw RuntimeException( "checking for unsupported permission type: " + demanded_type.getTypeName() );
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
