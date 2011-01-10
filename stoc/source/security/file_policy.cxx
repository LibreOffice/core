/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_stoc.hxx"

#include <hash_map>

#include <osl/diagnose.h>
#include <osl/file.h>
#include <rtl/byteseq.hxx>
#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>

#include <cppuhelper/access_control.hxx>
#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/implementationentry.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/security/XAccessController.hpp>
#include <com/sun/star/security/XPolicy.hpp>
#include <com/sun/star/security/AllPermission.hpp>
#include <com/sun/star/security/RuntimePermission.hpp>
#include <com/sun/star/io/FilePermission.hpp>
#include <com/sun/star/connection/SocketPermission.hpp>

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )
#define SERVICE_NAME "com.sun.star.security.Policy"
#define IMPL_NAME "com.sun.star.security.comp.stoc.FilePolicy"


using namespace ::osl;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

extern ::rtl_StandardModuleCount g_moduleCount;

namespace stoc_sec
{
// static stuff initialized when loading lib
static OUString s_implName = OUSTR(IMPL_NAME);
static OUString s_serviceName = OUSTR(SERVICE_NAME);

static Sequence< OUString > s_serviceNames = Sequence< OUString >( &s_serviceName, 1 );
//##################################################################################################

//--------------------------------------------------------------------------------------------------
static inline void dispose( Reference< XInterface > const & x )
    SAL_THROW( (RuntimeException) )
{
    Reference< lang::XComponent > xComp( x, UNO_QUERY );
    if (xComp.is())
    {
        xComp->dispose();
    }
}

//##################################################################################################

struct MutexHolder
{
    Mutex m_mutex;
};
typedef WeakComponentImplHelper2< security::XPolicy, lang::XServiceInfo > t_helper;

//==================================================================================================
class FilePolicy
    : public MutexHolder
    , public t_helper
{
    Reference< XComponentContext > m_xComponentContext;
    AccessControl m_ac;

    Sequence< Any > m_defaultPermissions;
    typedef std::hash_map< OUString, Sequence< Any >, OUStringHash > t_permissions;
    t_permissions m_userPermissions;
    bool m_init;

protected:
    virtual void SAL_CALL disposing();

public:
    FilePolicy( Reference< XComponentContext > const & xComponentContext )
        SAL_THROW( () );
    virtual ~FilePolicy()
        SAL_THROW( () );

    // XPolicy impl
    virtual Sequence< Any > SAL_CALL getPermissions(
        OUString const & userId )
        throw (RuntimeException);
    virtual Sequence< Any > SAL_CALL getDefaultPermissions()
        throw (RuntimeException);
    virtual void SAL_CALL refresh()
        throw (RuntimeException);

    // XServiceInfo impl
    virtual OUString SAL_CALL getImplementationName()
        throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( OUString const & serviceName )
        throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw (RuntimeException);
};
//__________________________________________________________________________________________________
FilePolicy::FilePolicy( Reference< XComponentContext > const & xComponentContext )
    SAL_THROW( () )
    : t_helper( m_mutex )
    , m_xComponentContext( xComponentContext )
    , m_ac( xComponentContext )
    , m_init( false )
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
}
//__________________________________________________________________________________________________
FilePolicy::~FilePolicy()
    SAL_THROW( () )
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}
//__________________________________________________________________________________________________
void FilePolicy::disposing()
{
    m_userPermissions.clear();
    m_defaultPermissions = Sequence< Any >();
    m_xComponentContext.clear();
}

//__________________________________________________________________________________________________
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
//__________________________________________________________________________________________________
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

//==================================================================================================
class PolicyReader
{
    OUString m_fileName;
    oslFileHandle m_file;

    sal_Int32 m_linepos;
    ByteSequence m_line;
    sal_Int32 m_pos;
    sal_Unicode m_back;

    sal_Unicode get()
        SAL_THROW( (RuntimeException) );
    inline void back( sal_Unicode c ) SAL_THROW( () )
        { m_back = c; }

    inline bool isWhiteSpace( sal_Unicode c ) SAL_THROW( () )
        { return (' ' == c || '\t' == c || '\n' == c || '\r' == c); }
    void skipWhiteSpace()
        SAL_THROW( (RuntimeException) );

    inline bool isCharToken( sal_Unicode c ) SAL_THROW( () )
        { return (';' == c || ',' == c || '{' == c || '}' == c); }

public:
    PolicyReader( OUString const & file, AccessControl & ac )
        SAL_THROW( (RuntimeException) );
    ~PolicyReader()
        SAL_THROW( () );

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
//__________________________________________________________________________________________________
void PolicyReader::assureToken( sal_Unicode token )
    SAL_THROW( (RuntimeException) )
{
    skipWhiteSpace();
    sal_Unicode c = get();
    if (c == token)
        return;
    OUStringBuffer buf( 16 );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("expected >") );
    buf.append( c );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("<!") );
    error( buf.makeStringAndClear() );
}
//__________________________________________________________________________________________________
OUString PolicyReader::assureQuotedToken()
    SAL_THROW( (RuntimeException) )
{
    OUString token( getQuotedToken() );
    if (! token.getLength())
        error( OUSTR("unexpected end of file!") );
    return token;
}
//__________________________________________________________________________________________________
OUString PolicyReader::getQuotedToken()
    SAL_THROW( (RuntimeException) )
{
    skipWhiteSpace();
    OUStringBuffer buf( 32 );
    sal_Unicode c = get();
    if ('\"' != c)
        error( OUSTR("expected quoting >\"< character!") );
    c = get();
    while ('\0' != c && '\"' != c)
    {
        buf.append( c );
        c = get();
    }
    return buf.makeStringAndClear();
}
//__________________________________________________________________________________________________
OUString PolicyReader::assureToken()
    SAL_THROW( (RuntimeException) )
{
    OUString token( getToken() );
    if (! token.getLength())
        error( OUSTR("unexpected end of file!") );
    return token;
}
//__________________________________________________________________________________________________
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
//__________________________________________________________________________________________________
void PolicyReader::skipWhiteSpace()
    SAL_THROW( (RuntimeException) )
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
            error( OUSTR("expected C/C++ like comment!") );
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
//__________________________________________________________________________________________________
sal_Unicode PolicyReader::get()
    SAL_THROW( (RuntimeException) )
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
            error( OUSTR("checking eof failed!") );
        if (eof)
            return '\0';

        rc = ::osl_readLine( m_file, reinterpret_cast< sal_Sequence ** >( &m_line ) );
        if (osl_File_E_None != rc)
            error( OUSTR("read line failed!") );
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
//__________________________________________________________________________________________________
void PolicyReader::error( OUString const & msg )
    SAL_THROW( (RuntimeException) )
{
    OUStringBuffer buf( 32 );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("error processing file \"") );
    buf.append( m_fileName );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\" [line ") );
    buf.append( m_linepos );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(", column ") );
    buf.append( m_pos );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("] ") );
    buf.append( msg );
    throw RuntimeException( buf.makeStringAndClear(), Reference< XInterface >() );
}
//__________________________________________________________________________________________________
PolicyReader::PolicyReader( OUString const & fileName, AccessControl & ac )
    SAL_THROW( (RuntimeException) )
    : m_fileName( fileName )
    , m_linepos( 0 )
    , m_pos( 1 ) // force readline
    , m_back( '\0' )
{
    ac.checkFilePermission( m_fileName, OUSTR("read") );
    if (osl_File_E_None != ::osl_openFile( m_fileName.pData, &m_file, osl_File_OpenFlag_Read ))
    {
        OUStringBuffer buf( 32 );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("cannot open file \"") );
        buf.append( m_fileName );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\"!") );
        throw RuntimeException( buf.makeStringAndClear(), Reference< XInterface >() );
    }
}
//__________________________________________________________________________________________________
PolicyReader::~PolicyReader()
    SAL_THROW( () )
{
    if ( ::osl_closeFile( m_file ) != osl_File_E_None ) {
        OSL_ASSERT( false );
    }
}

static OUString s_grant = OUSTR("grant");
static OUString s_user = OUSTR("user");
static OUString s_permission = OUSTR("permission");
static OUString s_openBrace = OUSTR("{");
static OUString s_closingBrace = OUSTR("}");

static OUString s_filePermission = OUSTR("com.sun.star.io.FilePermission");
static OUString s_socketPermission = OUSTR("com.sun.star.connection.SocketPermission");
static OUString s_runtimePermission = OUSTR("com.sun.star.security.RuntimePermission");
static OUString s_allPermission = OUSTR("com.sun.star.security.AllPermission");

//__________________________________________________________________________________________________
void FilePolicy::refresh()
    throw (RuntimeException)
{
    // read out file
    OUString fileName;
    m_xComponentContext->getValueByName(
        OUSTR("/implementations/" IMPL_NAME "/file-name") ) >>= fileName;
    if (! fileName.getLength())
    {
        throw RuntimeException(
            OUSTR("name of policy file unknown!"),
            (OWeakObject *)this );
    }

    PolicyReader reader( fileName, m_ac );

    // fill these two
    Sequence< Any > defaultPermissions;
    t_permissions userPermissions;

    OUString token( reader.getToken() );
    while (token.getLength())
    {
        if (! token.equals( s_grant ))
            reader.error( OUSTR("expected >grant< token!") );
        OUString userId;
        token = reader.assureToken();
        if (token.equals( s_user )) // next token is user-id
        {
            userId = reader.assureQuotedToken();
            token = reader.assureToken();
        }
        if (! token.equals( s_openBrace ))
            reader.error( OUSTR("expected opening brace >{<!") );
        token = reader.assureToken();
        // permissions list
        while (! token.equals( s_closingBrace ))
        {
            if (! token.equals( s_permission ))
                reader.error( OUSTR("expected >permission< or closing brace >}<!") );

            token = reader.assureToken(); // permission type
            Any perm;
            if (token.equals( s_filePermission )) // FilePermission
            {
                OUString url( reader.assureQuotedToken() );
                reader.assureToken( ',' );
                OUString actions( reader.assureQuotedToken() );
                perm <<= io::FilePermission( url, actions );
            }
            else if (token.equals( s_socketPermission )) // SocketPermission
            {
                OUString host( reader.assureQuotedToken() );
                reader.assureToken( ',' );
                OUString actions( reader.assureQuotedToken() );
                perm <<= connection::SocketPermission( host, actions );
            }
            else if (token.equals( s_runtimePermission )) // RuntimePermission
            {
                OUString name( reader.assureQuotedToken() );
                perm <<= security::RuntimePermission( name );
            }
            else if (token.equals( s_allPermission )) // AllPermission
            {
                perm <<= security::AllPermission();
            }
            else
            {
                reader.error( OUSTR("expected permission type!") );
            }

            reader.assureToken( ';' );

            // insert
            if (userId.getLength())
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

            token = reader.assureToken(); // next permissions token
        }

        reader.assureToken( ';' ); // semi
        token = reader.getToken(); // next grant token
    }

    // assign new ones
    MutexGuard guard( m_mutex );
    m_defaultPermissions = defaultPermissions;
    m_userPermissions = userPermissions;
}

//__________________________________________________________________________________________________
OUString FilePolicy::getImplementationName()
    throw (RuntimeException)
{
    return s_implName;
}
//__________________________________________________________________________________________________
sal_Bool FilePolicy::supportsService( OUString const & serviceName )
    throw (RuntimeException)
{
    OUString const * pNames = s_serviceNames.getConstArray();
    for ( sal_Int32 nPos = s_serviceNames.getLength(); nPos--; )
    {
        if (serviceName.equals( pNames[ nPos ] ))
        {
            return sal_True;
        }
    }
    return sal_False;
}
//__________________________________________________________________________________________________
Sequence< OUString > FilePolicy::getSupportedServiceNames()
    throw (RuntimeException)
{
    return s_serviceNames;
}
}
//##################################################################################################
namespace stoc_bootstrap
{
//--------------------------------------------------------------------------------------------------
Reference< XInterface > SAL_CALL filepolicy_create(
    Reference< XComponentContext > const & xComponentContext )
    SAL_THROW( (Exception) )
{
    return (OWeakObject *)new stoc_sec::FilePolicy( xComponentContext );
}
//--------------------------------------------------------------------------------------------------
Sequence< OUString > filepolicy_getSupportedServiceNames() SAL_THROW( () )
{
    return stoc_sec::s_serviceNames;
}
//--------------------------------------------------------------------------------------------------
OUString filepolicy_getImplementationName() SAL_THROW( () )
{
    return stoc_sec::s_implName;
}
}
