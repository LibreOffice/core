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


#include <osl/diagnose.h>
#include <osl/file.h>
#include <rtl/byteseq.hxx>
#include <rtl/ustrbuf.hxx>

#include <cppuhelper/access_control.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/security/XPolicy.hpp>
#include <com/sun/star/security/AllPermission.hpp>
#include <com/sun/star/security/RuntimePermission.hpp>
#include <com/sun/star/io/FilePermission.hpp>
#include <com/sun/star/connection/SocketPermission.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <string_view>
#include <unordered_map>
#include <utility>

constexpr OUString IMPL_NAME = u"com.sun.star.security.comp.stoc.FilePolicy"_ustr;

using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace css::uno;

namespace {

typedef WeakComponentImplHelper< security::XPolicy, lang::XServiceInfo > t_helper;


class FilePolicy
    : public cppu::BaseMutex
    , public t_helper
{
    Reference< XComponentContext > m_xComponentContext;
    AccessControl m_ac;

    Sequence< Any > m_defaultPermissions;
    typedef std::unordered_map< OUString, Sequence< Any > > t_permissions;
    t_permissions m_userPermissions;
    bool m_init;

protected:
    virtual void SAL_CALL disposing() override;

public:
    explicit FilePolicy( Reference< XComponentContext > const & xComponentContext );

    // XPolicy impl
    virtual Sequence< Any > SAL_CALL getPermissions(
        OUString const & userId ) override;
    virtual Sequence< Any > SAL_CALL getDefaultPermissions() override;
    virtual void SAL_CALL refresh() override;

    // XServiceInfo impl
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( OUString const & serviceName ) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

FilePolicy::FilePolicy( Reference< XComponentContext > const & xComponentContext )
    : t_helper( m_aMutex )
    , m_xComponentContext( xComponentContext )
    , m_ac( xComponentContext )
    , m_init( false )
{}

void FilePolicy::disposing()
{
    m_userPermissions.clear();
    m_defaultPermissions = Sequence< Any >();
    m_xComponentContext.clear();
}


Sequence< Any > FilePolicy::getPermissions(
    OUString const & userId )
{
    if (! m_init)
    {
        refresh();
        m_init = true;
    }

    MutexGuard guard( m_aMutex );
    t_permissions::iterator iFind( m_userPermissions.find( userId ) );
    if (m_userPermissions.end() == iFind)
    {
        return Sequence< Any >();
    }
    else
    {
        return iFind->second;
    }
}

Sequence< Any > FilePolicy::getDefaultPermissions()
{
    if (! m_init)
    {
        refresh();
        m_init = true;
    }

    MutexGuard guard( m_aMutex );
    return m_defaultPermissions;
}


class PolicyReader
{
    OUString m_fileName;
    oslFileHandle m_file;

    sal_Int32 m_linepos;
    rtl::ByteSequence m_line;
    sal_Int32 m_pos;
    sal_Unicode m_back;

    sal_Unicode get();
    void back( sal_Unicode c )
        { m_back = c; }

    static bool isWhiteSpace( sal_Unicode c )
        { return (' ' == c || '\t' == c || '\n' == c || '\r' == c); }
    void skipWhiteSpace();

    static bool isCharToken( sal_Unicode c )
        { return (';' == c || ',' == c || '{' == c || '}' == c); }

public:
    PolicyReader( OUString file, AccessControl & ac );
    ~PolicyReader();

    void error( std::u16string_view msg );

    OUString getToken();
    OUString assureToken();
    OUString getQuotedToken();
    OUString assureQuotedToken();
    void assureToken( sal_Unicode token );
};

void PolicyReader::assureToken( sal_Unicode token )
{
    skipWhiteSpace();
    sal_Unicode c = get();
    if (c == token)
        return;
    OUString msg = "expected >" + OUStringChar(c) + "<!";
    error( msg );
}

OUString PolicyReader::assureQuotedToken()
{
    OUString token( getQuotedToken() );
    if (token.isEmpty())
        error( u"unexpected end of file!" );
    return token;
}

OUString PolicyReader::getQuotedToken()
{
    skipWhiteSpace();
    OUStringBuffer buf( 32 );
    sal_Unicode c = get();
    if ('\"' != c)
        error( u"expected quoting >\"< character!" );
    c = get();
    while ('\0' != c && '\"' != c)
    {
        buf.append( c );
        c = get();
    }
    return buf.makeStringAndClear();
}

OUString PolicyReader::assureToken()
{
    OUString token( getToken() );
    if ( token.isEmpty())
        error( u"unexpected end of file!" );
    return token;
}

OUString PolicyReader::getToken()
{
    skipWhiteSpace();
    sal_Unicode c = get();
    if (isCharToken( c ))
        return OUString( &c, 1 );
    OUStringBuffer buf( 32 );
    while ('\0' != c && !isCharToken( c ) && !isWhiteSpace( c ))
    {
        buf.append( c );
        c = get();
    }
    back( c );
    return buf.makeStringAndClear();
}

void PolicyReader::skipWhiteSpace()
{
    sal_Unicode c;
    do
    {
        c = get();
    }
    while (isWhiteSpace( c )); // seeking next non-whitespace char

    if ('/' == c) // C/C++ like comment
    {
        c = get();
        if ('/' == c) // C++ like comment
        {
            do
            {
                c = get();
            }
            while ('\n' != c && '\0' != c); // seek eol/eof
            skipWhiteSpace(); // cont skip on next line
        }
        else if ('*' == c) // C like comment
        {
            bool fini = true;
            do
            {
                c = get();
                if ('*' == c)
                {
                    c = get();
                    fini = ('/' == c || '\0' == c);
                }
                else
                {
                    fini = ('\0' == c);
                }
            }
            while (! fini);
            skipWhiteSpace(); // cont skip on next line
        }
        else
        {
            error( u"expected C/C++ like comment!" );
        }
    }
    else if ('#' == c) // script like comment
    {
        do
        {
            c = get();
        }
        while ('\n' != c && '\0' != c); // seek eol/eof
        skipWhiteSpace(); // cont skip on next line
    }

    else // is token char
    {
        back( c );
    }
}

sal_Unicode PolicyReader::get()
{
    if ('\0' != m_back) // one char push back possible
    {
        sal_Unicode c = m_back;
        m_back = '\0';
        return c;
    }
    else if (m_pos == m_line.getLength()) // provide newline as whitespace
    {
        ++m_pos;
        return '\n';
    }
    else if (m_pos > m_line.getLength()) // read new line
    {
        sal_Bool eof;
        oslFileError rc = ::osl_isEndOfFile( m_file, &eof );
        if (osl_File_E_None != rc)
            error( u"checking eof failed!" );
        if (eof)
            return '\0';

        rc = ::osl_readLine( m_file, reinterpret_cast< sal_Sequence ** >( &m_line ) );
        if (osl_File_E_None != rc)
            error( u"read line failed!" );
        ++m_linepos;
        if (! m_line.getLength()) // empty line read
        {
            m_pos = 1; // read new line next time
            return '\n';
        }
        m_pos = 0;
    }
    return (m_line.getConstArray()[ m_pos++ ]);
}

void PolicyReader::error( std::u16string_view msg )
{
    throw RuntimeException(
        "error processing file \"" + m_fileName +
        "\" [line " + OUString::number(m_linepos) +
        ", column " + OUString::number(m_pos) +
        "] " + msg);
}

PolicyReader::PolicyReader( OUString fileName, AccessControl & ac )
    : m_fileName(std::move( fileName ))
    , m_linepos( 0 )
    , m_pos( 1 ) // force readline
    , m_back( '\0' )
{
    ac.checkFilePermission( m_fileName, u"read"_ustr );
    if (osl_File_E_None != ::osl_openFile( m_fileName.pData, &m_file, osl_File_OpenFlag_Read ))
    {
        throw RuntimeException( "cannot open file \"" + m_fileName + "\"!" );
    }
}

PolicyReader::~PolicyReader()
{
    if ( ::osl_closeFile( m_file ) != osl_File_E_None ) {
        OSL_ASSERT( false );
    }
}

constexpr OUStringLiteral s_grant = u"grant";
constexpr OUStringLiteral s_user = u"user";
constexpr OUStringLiteral s_permission = u"permission";
constexpr OUStringLiteral s_openBrace = u"{";
constexpr OUStringLiteral s_closingBrace = u"}";

constexpr OUStringLiteral s_filePermission = u"com.sun.star.io.FilePermission";
constexpr OUStringLiteral s_socketPermission = u"com.sun.star.connection.SocketPermission";
constexpr OUStringLiteral s_runtimePermission = u"com.sun.star.security.RuntimePermission";
constexpr OUStringLiteral s_allPermission = u"com.sun.star.security.AllPermission";


void FilePolicy::refresh()
{
    // read out file (the .../file-name value had originally been set in
    // cppu::add_access_control_entries (cppuhelper/source/servicefactory.cxx)
    // depending on various UNO_AC* bootstrap variables that are no longer
    // supported, so this is effectively dead code):
    OUString fileName;
    m_xComponentContext->getValueByName(
        "/implementations/" + IMPL_NAME + "/file-name" ) >>= fileName;
    if ( fileName.isEmpty() )
    {
        throw RuntimeException(
            u"name of policy file unknown!"_ustr,
            getXWeak() );
    }

    PolicyReader reader( fileName, m_ac );

    // fill these two
    Sequence< Any > defaultPermissions;
    t_permissions userPermissions;

    OUString token( reader.getToken() );
    while (!token.isEmpty())
    {
        if ( token != s_grant )
            reader.error( u"expected >grant< token!" );
        OUString userId;
        token = reader.assureToken();
        if ( token == s_user ) // next token is user-id
        {
            userId = reader.assureQuotedToken();
            token = reader.assureToken();
        }
        if ( token != s_openBrace )
            reader.error( u"expected opening brace >{<!" );
        token = reader.assureToken();
        // permissions list
        while ( token != s_closingBrace )
        {
            if ( token != s_permission )
                reader.error( u"expected >permission< or closing brace >}<!" );

            token = reader.assureToken(); // permission type
            Any perm;
            if ( token == s_filePermission ) // FilePermission
            {
                OUString url( reader.assureQuotedToken() );
                reader.assureToken( ',' );
                OUString actions( reader.assureQuotedToken() );
                perm <<= io::FilePermission( url, actions );
            }
            else if ( token == s_socketPermission ) // SocketPermission
            {
                OUString host( reader.assureQuotedToken() );
                reader.assureToken( ',' );
                OUString actions( reader.assureQuotedToken() );
                perm <<= connection::SocketPermission( host, actions );
            }
            else if ( token == s_runtimePermission ) // RuntimePermission
            {
                OUString name( reader.assureQuotedToken() );
                perm <<= security::RuntimePermission( name );
            }
            else if ( token == s_allPermission ) // AllPermission
            {
                perm <<= security::AllPermission();
            }
            else
            {
                reader.error( u"expected permission type!" );
            }

            reader.assureToken( ';' );

            // insert
            if (!userId.isEmpty())
            {
                Sequence< Any > perms( userPermissions[ userId ] );
                sal_Int32 len = perms.getLength();
                perms.realloc( len +1 );
                perms.getArray()[ len ] = perm;
                userPermissions[ userId ] = perms;
            }
            else
            {
                sal_Int32 len = defaultPermissions.getLength();
                defaultPermissions.realloc( len +1 );
                defaultPermissions.getArray()[ len ] = perm;
            }

            token = reader.assureToken(); // next permissions token
        }

        reader.assureToken( ';' ); // semi
        token = reader.getToken(); // next grant token
    }

    // assign new ones
    MutexGuard guard( m_aMutex );
    m_defaultPermissions = defaultPermissions;
    m_userPermissions = userPermissions;
}


OUString FilePolicy::getImplementationName()
{
    return IMPL_NAME;
}

sal_Bool FilePolicy::supportsService( OUString const & serviceName )
{
    return cppu::supportsService(this, serviceName);
}

Sequence< OUString > FilePolicy::getSupportedServiceNames()
{
    return { u"com.sun.star.security.Policy"_ustr };
}

} // namespace

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_security_comp_stoc_FilePolicy_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new FilePolicy(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
