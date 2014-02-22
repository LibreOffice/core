/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <boost/unordered_map.hpp>

#include <osl/diagnose.h>
#include <osl/file.h>
#include <rtl/byteseq.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>

#include <cppuhelper/access_control.hxx>
#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/security/XAccessController.hpp>
#include <com/sun/star/security/XPolicy.hpp>
#include <com/sun/star/security/AllPermission.hpp>
#include <com/sun/star/security/RuntimePermission.hpp>
#include <com/sun/star/io/FilePermission.hpp>
#include <com/sun/star/connection/SocketPermission.hpp>

#define IMPL_NAME "com.sun.star.security.comp.stoc.FilePolicy"

using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace {

struct MutexHolder
{
    Mutex m_mutex;
};
typedef WeakComponentImplHelper2< security::XPolicy, lang::XServiceInfo > t_helper;


class FilePolicy
    : public MutexHolder
    , public t_helper
{
    Reference< XComponentContext > m_xComponentContext;
    AccessControl m_ac;

    Sequence< Any > m_defaultPermissions;
    typedef boost::unordered_map< OUString, Sequence< Any >, OUStringHash > t_permissions;
    t_permissions m_userPermissions;
    bool m_init;

protected:
    virtual void SAL_CALL disposing();

public:
    FilePolicy( Reference< XComponentContext > const & xComponentContext )
        SAL_THROW(());
    virtual ~FilePolicy()
        SAL_THROW(());

    
    virtual Sequence< Any > SAL_CALL getPermissions(
        OUString const & userId )
        throw (RuntimeException);
    virtual Sequence< Any > SAL_CALL getDefaultPermissions()
        throw (RuntimeException);
    virtual void SAL_CALL refresh()
        throw (RuntimeException);

    
    virtual OUString SAL_CALL getImplementationName()
        throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( OUString const & serviceName )
        throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw (RuntimeException);
};

FilePolicy::FilePolicy( Reference< XComponentContext > const & xComponentContext )
    SAL_THROW(())
    : t_helper( m_mutex )
    , m_xComponentContext( xComponentContext )
    , m_ac( xComponentContext )
    , m_init( false )
{}

FilePolicy::~FilePolicy()
    SAL_THROW(())
{}

void FilePolicy::disposing()
{
    m_userPermissions.clear();
    m_defaultPermissions = Sequence< Any >();
    m_xComponentContext.clear();
}


Sequence< Any > FilePolicy::getPermissions(
    OUString const & userId )
    throw (RuntimeException)
{
    if (! m_init)
    {
        refresh();
        m_init = true;
    }

    MutexGuard guard( m_mutex );
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
    throw (RuntimeException)
{
    if (! m_init)
    {
        refresh();
        m_init = true;
    }

    MutexGuard guard( m_mutex );
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

    sal_Unicode get()
        SAL_THROW( (RuntimeException) );
    inline void back( sal_Unicode c ) SAL_THROW(())
        { m_back = c; }

    inline bool isWhiteSpace( sal_Unicode c ) const SAL_THROW(())
        { return (' ' == c || '\t' == c || '\n' == c || '\r' == c); }
    void skipWhiteSpace()
        SAL_THROW( (RuntimeException) );

    inline bool isCharToken( sal_Unicode c ) const SAL_THROW(())
        { return (';' == c || ',' == c || '{' == c || '}' == c); }

public:
    PolicyReader( OUString const & file, AccessControl & ac )
        SAL_THROW( (RuntimeException) );
    ~PolicyReader()
        SAL_THROW(());

    void error( OUString const & msg )
        SAL_THROW( (RuntimeException) );

    OUString getToken()
        SAL_THROW( (RuntimeException) );
    OUString assureToken()
        SAL_THROW( (RuntimeException) );
    OUString getQuotedToken()
        SAL_THROW( (RuntimeException) );
    OUString assureQuotedToken()
        SAL_THROW( (RuntimeException) );
    void assureToken( sal_Unicode token )
        SAL_THROW( (RuntimeException) );
};

void PolicyReader::assureToken( sal_Unicode token )
    SAL_THROW( (RuntimeException) )
{
    skipWhiteSpace();
    sal_Unicode c = get();
    if (c == token)
        return;
    OUStringBuffer buf( 16 );
    buf.append( "expected >" );
    buf.append( c );
    buf.append( "<!" );
    error( buf.makeStringAndClear() );
}

OUString PolicyReader::assureQuotedToken()
    SAL_THROW( (RuntimeException) )
{
    OUString token( getQuotedToken() );
    if (token.isEmpty())
        error( "unexpected end of file!" );
    return token;
}

OUString PolicyReader::getQuotedToken()
    SAL_THROW( (RuntimeException) )
{
    skipWhiteSpace();
    OUStringBuffer buf( 32 );
    sal_Unicode c = get();
    if ('\"' != c)
        error( "expected quoting >\"< character!" );
    c = get();
    while ('\0' != c && '\"' != c)
    {
        buf.append( c );
        c = get();
    }
    return buf.makeStringAndClear();
}

OUString PolicyReader::assureToken()
    SAL_THROW( (RuntimeException) )
{
    OUString token( getToken() );
    if ( token.isEmpty())
        error( "unexpected end of file!" );
    return token;
}

OUString PolicyReader::getToken()
    SAL_THROW( (RuntimeException) )
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
    SAL_THROW( (RuntimeException) )
{
    sal_Unicode c;
    do
    {
        c = get();
    }
    while (isWhiteSpace( c )); 

    if ('/' == c) 
    {
        c = get();
        if ('/' == c) 
        {
            do
            {
                c = get();
            }
            while ('\n' != c && '\0' != c); 
            skipWhiteSpace(); 
        }
        else if ('*' == c) 
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
            skipWhiteSpace(); 
        }
        else
        {
            error( "expected C/C++ like comment!" );
        }
    }
    else if ('#' == c) 
    {
        do
        {
            c = get();
        }
        while ('\n' != c && '\0' != c); 
        skipWhiteSpace(); 
    }

    else 
    {
        back( c );
    }
}

sal_Unicode PolicyReader::get()
    SAL_THROW( (RuntimeException) )
{
    if ('\0' != m_back) 
    {
        sal_Unicode c = m_back;
        m_back = '\0';
        return c;
    }
    else if (m_pos == m_line.getLength()) 
    {
        ++m_pos;
        return '\n';
    }
    else if (m_pos > m_line.getLength()) 
    {
        sal_Bool eof;
        oslFileError rc = ::osl_isEndOfFile( m_file, &eof );
        if (osl_File_E_None != rc)
            error( "checking eof failed!" );
        if (eof)
            return '\0';

        rc = ::osl_readLine( m_file, reinterpret_cast< sal_Sequence ** >( &m_line ) );
        if (osl_File_E_None != rc)
            error( "read line failed!" );
        ++m_linepos;
        if (! m_line.getLength()) 
        {
            m_pos = 1; 
            return '\n';
        }
        m_pos = 0;
    }
    return (m_line.getConstArray()[ m_pos++ ]);
}

void PolicyReader::error( OUString const & msg )
    SAL_THROW( (RuntimeException) )
{
    OUStringBuffer buf( 32 );
    buf.append( "error processing file \"" );
    buf.append( m_fileName );
    buf.append( "\" [line " );
    buf.append( m_linepos );
    buf.append( ", column " );
    buf.append( m_pos );
    buf.append( "] " );
    buf.append( msg );
    throw RuntimeException( buf.makeStringAndClear(), Reference< XInterface >() );
}

PolicyReader::PolicyReader( OUString const & fileName, AccessControl & ac )
    SAL_THROW( (RuntimeException) )
    : m_fileName( fileName )
    , m_linepos( 0 )
    , m_pos( 1 ) 
    , m_back( '\0' )
{
    ac.checkFilePermission( m_fileName, "read" );
    if (osl_File_E_None != ::osl_openFile( m_fileName.pData, &m_file, osl_File_OpenFlag_Read ))
    {
        OUStringBuffer buf( 32 );
        buf.append( "cannot open file \"" );
        buf.append( m_fileName );
        buf.append( "\"!" );
        throw RuntimeException( buf.makeStringAndClear(), Reference< XInterface >() );
    }
}

PolicyReader::~PolicyReader()
    SAL_THROW(())
{
    if ( ::osl_closeFile( m_file ) != osl_File_E_None ) {
        OSL_ASSERT( false );
    }
}

#define s_grant "grant"
#define s_user "user"
#define s_permission "permission"
#define s_openBrace "{"
#define s_closingBrace "}"

#define s_filePermission "com.sun.star.io.FilePermission"
#define s_socketPermission "com.sun.star.connection.SocketPermission"
#define s_runtimePermission "com.sun.star.security.RuntimePermission"
#define s_allPermission "com.sun.star.security.AllPermission"


void FilePolicy::refresh()
    throw (RuntimeException)
{
    
    
    
    
    OUString fileName;
    m_xComponentContext->getValueByName(
        "/implementations/" IMPL_NAME "/file-name" ) >>= fileName;
    if ( fileName.isEmpty() )
    {
        throw RuntimeException(
            "name of policy file unknown!",
            (OWeakObject *)this );
    }

    PolicyReader reader( fileName, m_ac );

    
    Sequence< Any > defaultPermissions;
    t_permissions userPermissions;

    OUString token( reader.getToken() );
    while (!token.isEmpty())
    {
        if ( token != s_grant )
            reader.error( "expected >grant< token!" );
        OUString userId;
        token = reader.assureToken();
        if ( token == s_user ) 
        {
            userId = reader.assureQuotedToken();
            token = reader.assureToken();
        }
        if ( token != s_openBrace )
            reader.error( "expected opening brace >{<!" );
        token = reader.assureToken();
        
        while ( token != s_closingBrace )
        {
            if ( token != s_permission )
                reader.error( "expected >permission< or closing brace >}<!" );

            token = reader.assureToken(); 
            Any perm;
            if ( token == s_filePermission ) 
            {
                OUString url( reader.assureQuotedToken() );
                reader.assureToken( ',' );
                OUString actions( reader.assureQuotedToken() );
                perm <<= io::FilePermission( url, actions );
            }
            else if ( token == s_socketPermission ) 
            {
                OUString host( reader.assureQuotedToken() );
                reader.assureToken( ',' );
                OUString actions( reader.assureQuotedToken() );
                perm <<= connection::SocketPermission( host, actions );
            }
            else if ( token == s_runtimePermission ) 
            {
                OUString name( reader.assureQuotedToken() );
                perm <<= security::RuntimePermission( name );
            }
            else if ( token == s_allPermission ) 
            {
                perm <<= security::AllPermission();
            }
            else
            {
                reader.error( "expected permission type!" );
            }

            reader.assureToken( ';' );

            
            if (!userId.isEmpty())
            {
                Sequence< Any > perms( userPermissions[ userId ] );
                sal_Int32 len = perms.getLength();
                perms.realloc( len +1 );
                perms[ len ] = perm;
                userPermissions[ userId ] = perms;
            }
            else
            {
                sal_Int32 len = defaultPermissions.getLength();
                defaultPermissions.realloc( len +1 );
                defaultPermissions[ len ] = perm;
            }

            token = reader.assureToken(); 
        }

        reader.assureToken( ';' ); 
        token = reader.getToken(); 
    }

    
    MutexGuard guard( m_mutex );
    m_defaultPermissions = defaultPermissions;
    m_userPermissions = userPermissions;
}


OUString FilePolicy::getImplementationName()
    throw (RuntimeException)
{
    return OUString(IMPL_NAME);
}

sal_Bool FilePolicy::supportsService( OUString const & serviceName )
    throw (RuntimeException)
{
    return cppu::supportsService(this, serviceName);
}

Sequence< OUString > FilePolicy::getSupportedServiceNames()
    throw (RuntimeException)
{
    Sequence< OUString > aSNS( 1 );
    aSNS[0] = OUString("com.sun.star.security.Policy");
    return aSNS;
}

} 

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_security_comp_stoc_FilePolicy_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new FilePolicy(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
