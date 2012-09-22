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


#include <stdio.h>

#include <sal/main.h>
#include <osl/diagnose.h>
#include <osl/socket.hxx>
#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <uno/current_context.hxx>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/access_control.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/uno/XCurrentContext.hpp>

#include <com/sun/star/io/FilePermission.hpp>

#define USER_CREDS "access-control.user-credentials"
#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


using namespace ::osl;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

//--------------------------------------------------------------------------------------------------
static OUString localhost( OUString const & addition ) SAL_THROW(())
{
    static OUString ip;
    if (! ip.getLength())
    {
        // dns lookup
        SocketAddr addr;
        SocketAddr::resolveHostname( OUSTR("localhost"), addr );
        ::oslSocketResult rc = ::osl_getDottedInetAddrOfSocketAddr( addr.getHandle(), &ip.pData );
        if (::osl_Socket_Ok != rc)
            fprintf(stdout, "### cannot resolve localhost!" );
    }
    OUStringBuffer buf( 48 );
    buf.append( ip );
    buf.append( addition );
    return buf.makeStringAndClear();
}

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
//==================================================================================================
class user_CurrentContext
    : public ImplHelper1< XCurrentContext >
{
    oslInterlockedCount m_refcount;

    Reference< XCurrentContext > m_xDelegate;
    Any m_userId;

public:
    inline user_CurrentContext(
        Reference< XCurrentContext > const & xDelegate,
        OUString const & userId )
        SAL_THROW(())
        : m_refcount( 0 )
        , m_xDelegate( xDelegate )
        , m_userId( makeAny( userId ) )
        {}

    // XInterface impl
    virtual void SAL_CALL acquire()
        throw ();
    virtual void SAL_CALL release()
        throw ();

    // XCurrentContext impl
    virtual Any SAL_CALL getValueByName( OUString const & name )
        throw (RuntimeException);
};
//__________________________________________________________________________________________________
void user_CurrentContext::acquire()
    throw ()
{
    ::osl_atomic_increment( &m_refcount );
}
//__________________________________________________________________________________________________
void user_CurrentContext::release()
    throw ()
{
    if (! ::osl_atomic_decrement( &m_refcount ))
    {
        delete this;
    }
}
//__________________________________________________________________________________________________
Any user_CurrentContext::getValueByName( OUString const & name )
    throw (RuntimeException)
{
    if ( name == USER_CREDS ".id" )
    {
        return m_userId;
    }
    else if (m_xDelegate.is())
    {
        return m_xDelegate->getValueByName( name );
    }
    else
    {
        return Any();
    }
}

// prepends line number
#define CHECK( check, negative_test ) \
{ \
    try \
    { \
        if (negative_test) \
        { \
            bool thrown = true; \
            try \
            { \
                check; \
                thrown = false; \
            } \
            catch (RuntimeException &) \
            { \
            } \
            if (! thrown) \
            { \
                throw RuntimeException( \
                    OUSTR("expected RuntimeException upon check!"), Reference< XInterface >() ); \
            } \
        } \
        else \
        { \
            check; \
        } \
    } \
    catch (const RuntimeException & exc) \
    { \
        OUStringBuffer buf( 64 ); \
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("[line ") ); \
        buf.append( (sal_Int32)__LINE__ ); \
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("] ") ); \
        buf.append( exc.Message ); \
        throw RuntimeException( buf.makeStringAndClear(), Reference< XInterface >() ); \
    } \
}

/*
grant
{
permission com.sun.star.io.FilePermission "file:///usr/bin/ *", "read";
permission com.sun.star.io.FilePermission "file:///tmp/-", "read,write";
permission com.sun.star.io.FilePermission "file:///etc/profile", "read";

permission com.sun.star.security.RuntimePermission "DEF";

permission com.sun.star.connection.SocketPermission "127.0.0.1:-1023", "resolve, connect, listen";
permission com.sun.star.connection.SocketPermission "localhost:1024-", "accept, connect, listen, resolve,";
permission com.sun.star.connection.SocketPermission "*.sun.com:1024-", "resolve";
};
*/
static void check_defaults_pos( AccessControl & ac, bool invert = false )
{
    // positive tests
    CHECK( ac.checkFilePermission( OUSTR("file:///usr/bin/bla"), OUSTR("read") ), invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///tmp/bla"), OUSTR("read,write") ), invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///tmp/path/path/bla"), OUSTR("write") ), invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///etc/profile"), OUSTR("read") ), invert );
    CHECK( ac.checkRuntimePermission( OUSTR("DEF") ), invert );
    CHECK( ac.checkSocketPermission( OUSTR("localhost:1024"), OUSTR("connect") ), invert );
    CHECK( ac.checkSocketPermission( OUSTR("localhost:65535"), OUSTR("resolve") ), invert );
    CHECK( ac.checkSocketPermission( localhost(OUSTR(":2048")), OUSTR("accept,listen") ), invert );
    CHECK( ac.checkSocketPermission( localhost(OUSTR(":1024-")), OUSTR("accept,connect,listen,resolve") ), invert );
    CHECK( ac.checkSocketPermission( OUSTR("localhost:-1023"), OUSTR("resolve,listen,connect") ), invert );
    CHECK( ac.checkSocketPermission( OUSTR("jl-1036.germany.sun.com:1024-"), OUSTR("resolve") ), invert );
}
static void check_defaults_neg( AccessControl & ac, bool invert = false )
{
    // negative tests
    CHECK( ac.checkFilePermission( OUSTR("file:///usr/tmp"), OUSTR("read") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///"), OUSTR("read") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///usr/bin"), OUSTR("read") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///usr/bin/bla"), OUSTR("write") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///usr/bin/bla"), OUSTR("execute") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///usr/bin/path/bla"), OUSTR("read") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///tmp"), OUSTR("read") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///tmp/"), OUSTR("read") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///tm"), OUSTR("read") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///etc/profile"), OUSTR("write") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///etc/profile/bla"), OUSTR("read") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///etc/blabla"), OUSTR("read,write,execute") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///home/root"), OUSTR("read,write,execute") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///root"), OUSTR("read,write,execute") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///root"), OUSTR("delete") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///root"), OUString() ), !invert );
    CHECK( ac.checkRuntimePermission( OUSTR("ROOT") ), !invert );
    CHECK( ac.checkSocketPermission( OUSTR("localhost:1023"), OUSTR("accept") ), !invert );
    CHECK( ac.checkSocketPermission( OUSTR("localhost:123-"), OUSTR("accept") ), !invert );
    CHECK( ac.checkSocketPermission( localhost(OUSTR(":-1023")), OUSTR("accept") ), !invert );
    CHECK( ac.checkSocketPermission( OUSTR("localhost:-1023"), OUSTR("accept,resolve") ), !invert );
    CHECK( ac.checkSocketPermission( OUSTR("sun.com:1024-"), OUSTR("resolve") ), !invert );
}

/*
grant user "dbo"
{
permission com.sun.star.io.FilePermission "file:///home/dbo/-", "read,write";
permission com.sun.star.io.FilePermission "-", "read,write";
permission com.sun.star.io.FilePermission "file:///usr/local/dbo/ *", "read";

permission com.sun.star.security.RuntimePermission "DBO";

permission com.sun.star.connection.SocketPermission "dbo-1:1024-", "listen";
permission com.sun.star.connection.SocketPermission "dbo-11081:-1023", "resolve";
permission com.sun.star.connection.SocketPermission "dbo-11081:18", "listen";
permission com.sun.star.connection.SocketPermission "dbo-11081:20-24", "listen";
permission com.sun.star.connection.SocketPermission "dbo-11081", "connect";
};
*/
static void check_dbo_pos( AccessControl & ac, bool invert = false )
{
    check_defaults_pos( ac, invert );
    // positive tests
    CHECK( ac.checkFilePermission( OUSTR("file:///home/dbo/bla"), OUSTR("read") ), invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///home/dbo/bla"), OUSTR("write") ), invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///home/dbo/bla"), OUSTR("read,write") ), invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///home/dbo/path/bla"), OUSTR("read,write") ), invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///home/dbo/path/path/bla"), OUSTR("read,write") ), invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///usr/local/dbo/*"), OUSTR("read") ), invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///usr/local/dbo/bla"), OUSTR("read") ), invert );
    CHECK( ac.checkRuntimePermission( OUSTR("DBO") ), invert );
    CHECK( ac.checkSocketPermission( OUSTR("dbo-1:1024-"), OUSTR("listen") ), invert );
    CHECK( ac.checkSocketPermission( OUSTR("dbo-1:2048-3122"), OUSTR("listen") ), invert );
    CHECK( ac.checkSocketPermission( OUSTR("dbo-1:2048-"), OUSTR("listen") ), invert );
    CHECK( ac.checkSocketPermission( OUSTR("dbo-11081:-1023"), OUSTR("resolve") ), invert );
    CHECK( ac.checkSocketPermission( OUSTR("dbo-11081:20-1023"), OUSTR("resolve") ), invert );
    CHECK( ac.checkSocketPermission( OUSTR("dbo-11081:18"), OUSTR("listen") ), invert );
    CHECK( ac.checkSocketPermission( OUSTR("dbo-11081:20-24"), OUSTR("listen") ), invert );
    CHECK( ac.checkSocketPermission( OUSTR("dbo-11081:22"), OUSTR("listen") ), invert );
    CHECK( ac.checkSocketPermission( OUSTR("dbo-11081"), OUSTR("connect") ), invert );
    CHECK( ac.checkSocketPermission( OUSTR("dbo-11081:22"), OUSTR("connect") ), invert );
}
static void check_dbo_neg( AccessControl & ac, bool invert = false )
{
    check_defaults_neg( ac, invert );
    // negative tests
    CHECK( ac.checkFilePermission( OUSTR("file:///home/-"), OUSTR("read") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///home/jbu/bla"), OUSTR("read") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///home/jbu/bla"), OUSTR("write") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///home/jbu/bla"), OUSTR("read,write") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///home/jbu/path/bla"), OUSTR("read") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///home/dbo/path/path/bla"), OUSTR("read,execute") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///usr/local/-"), OUSTR("read") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///usr/local/dbo/path/bla"), OUSTR("read") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///usr/local/dbo/path/path/bla"), OUSTR("read") ), !invert );
    CHECK( ac.checkRuntimePermission( OUSTR("JBU") ), !invert );
    CHECK( ac.checkSocketPermission( OUSTR("dbo-11081"), OUSTR("listen") ), !invert );
    CHECK( ac.checkSocketPermission( OUSTR("dbo-11081:22"), OUSTR("accept") ), !invert );
    CHECK( ac.checkSocketPermission( OUSTR("jbu-11096:22"), OUSTR("resolve") ), !invert );
}

/*
grant user "jbu"
{
permission com.sun.star.io.FilePermission  "file:///home/jbu/-", "read,write";
permission com.sun.star.io.FilePermission "*", "read,write";

permission com.sun.star.security.RuntimePermission "JBU";

permission com.sun.star.connection.SocketPermission "jbu-11096","resolve";
};
*/
static void check_jbu_pos( AccessControl & ac, bool invert = false )
{
    check_defaults_pos( ac, invert );
    // positive tests
    CHECK( ac.checkFilePermission( OUSTR("file:///home/jbu/bla"), OUSTR("read") ), invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///home/jbu/bla"), OUSTR("write") ), invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///home/jbu/bla"), OUSTR("read,write") ), invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///home/jbu/path/bla"), OUSTR("read,write") ), invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///home/jbu/path/path/bla"), OUSTR("read,write") ), invert );
    CHECK( ac.checkRuntimePermission( OUSTR("JBU") ), invert );
    CHECK( ac.checkSocketPermission( OUSTR("jbu-11096"), OUSTR("resolve") ), invert );
    CHECK( ac.checkSocketPermission( OUSTR("jbu-11096:20-24"), OUSTR("resolve") ), invert );
    CHECK( ac.checkSocketPermission( OUSTR("dbo-11081.germany.sun.com:2048"), OUSTR("resolve") ), invert );
}
static void check_jbu_neg( AccessControl & ac, bool invert = false )
{
    check_defaults_neg( ac, invert );
    // negative tests
    CHECK( ac.checkFilePermission( OUSTR("file:///home/-"), OUSTR("read") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///home/dbo/path/bla"), OUSTR("read") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///home/dbo/path/path/bla"), OUSTR("read") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///home/dbo/bla"), OUSTR("read") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///home/dbo/bla"), OUSTR("write") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///home/dbo/bla"), OUSTR("read,write") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///usr/local/-"), OUSTR("read") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///usr/local/dbo/bla"), OUSTR("read") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///usr/local/dbo/path/path/bla"), OUSTR("read") ), !invert );
    CHECK( ac.checkRuntimePermission( OUSTR("DBO") ), !invert );
    CHECK( ac.checkSocketPermission( OUSTR("jbu-11096:20-24"), OUSTR("accept") ), !invert );
    CHECK( ac.checkSocketPermission( OUSTR("dbo-11081"), OUSTR("connect") ), !invert );
    CHECK( ac.checkSocketPermission( OUSTR("dbo-11081.germany.sun.com"), OUSTR("connect") ), !invert );
}

/*
grant principal "root"
{
permission com.sun.star.security.AllPermission;
};
*/
//==================================================================================================
static void check_root_pos( AccessControl & ac, bool invert = false )
{
    check_defaults_pos( ac, invert );
    check_defaults_neg( ac, !invert );
    check_dbo_pos( ac, invert );
    check_dbo_neg( ac, !invert );
    check_jbu_pos( ac, invert );
    check_jbu_neg( ac, !invert );
    // some more root positive
    CHECK( ac.checkFilePermission( OUSTR("file:///etc/blabla"), OUSTR("read,write,execute") ), invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///home/root"), OUSTR("read,write,execute") ), invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///root"), OUSTR("read,write,execute") ), invert );
    CHECK( ac.checkRuntimePermission( OUSTR("ROOT") ), invert );
}

//==================================================================================================
class acc_Restr
    : public WeakImplHelper1< security::XAccessControlContext >
{
    Any m_perm;

public:
    inline acc_Restr( Any const & perm = Any() ) SAL_THROW(())
        : m_perm( perm )
        {}

    // XAccessControlContext impl
    virtual void SAL_CALL checkPermission( Any const & perm )
        throw (RuntimeException);
};
//__________________________________________________________________________________________________
void acc_Restr::checkPermission( Any const & perm )
    throw (RuntimeException)
{
    if (perm != m_perm)
    {
        throw security::AccessControlException(
            OUSTR("dyn violation!"), Reference< XInterface >(), perm );
    }
}

typedef void (* t_action)( AccessControl &, Any const & arg );

//==================================================================================================
class Action
    : public WeakImplHelper1< security::XAction >
{
    t_action m_action;
    AccessControl & m_ac;
    Any m_arg;

public:
    inline Action( t_action action, AccessControl & ac, Any const & arg = Any() ) SAL_THROW(())
        : m_action( action )
        , m_ac( ac )
        , m_arg( arg )
        {}

    // XAction impl
    virtual Any SAL_CALL run()
        throw (Exception);
};
//__________________________________________________________________________________________________
Any Action::run()
    throw (Exception)
{
    (*m_action)( m_ac, m_arg );
    return Any();
}

//==================================================================================================
// static void restr_file_permissions( AccessControl & ac )
// {
//     // running in dbo's domain
//     /* permission com.sun.star.io.FilePermission "file:///home/dbo/-", ",,read , write "; */
//     CHECK( ac.checkFilePermission( OUSTR("file:///home/dbo/bla"), OUSTR("read,write,execute") ), true );
//     CHECK( ac.checkFilePermission( OUSTR("file:///home/dbo/bla"), OUSTR("read,write") ), false );
// }
//==================================================================================================
static void all_dbo_permissions( AccessControl & ac, Any const & )
{
    check_dbo_pos( ac );
    check_dbo_neg( ac );
}
//==================================================================================================
static void no_permissions( AccessControl & ac, Any const & arg )
{
    check_dbo_pos( ac, true );
    check_dbo_neg( ac );
    // set privs to old dbo restr
    Reference< security::XAccessControlContext > xContext;
    OSL_VERIFY( arg >>= xContext );
    ac->doPrivileged(
        new Action( all_dbo_permissions, ac ),
        xContext );
}
//==================================================================================================
static void check_dbo_dynamic( AccessControl & ac )
{
    Any arg( makeAny( ac->getContext() ) );
    ac->doRestricted(
        new Action( no_permissions, ac, arg ),
        new acc_Restr() );
}

SAL_IMPLEMENT_MAIN()
{
    try
    {
        // single-user test
        Reference< XComponentContext > xContext( defaultBootstrap_InitialComponentContext(
            OUSTR("../../test/security/test_security_singleuser.ini") ) );
        {
        ::fprintf( stderr, "[security test] single-user checking dbo..." );
        AccessControl ac( xContext );
        check_dbo_pos( ac );
        check_dbo_neg( ac );
        check_dbo_dynamic( ac );
        ::fprintf( stderr, "dbo checked.\n" );
        }

        // multi-user test
        dispose( xContext );
        xContext = defaultBootstrap_InitialComponentContext(
            OUSTR("../../test/security/test_security.ini") ); // UNO_AC=on
        AccessControl ac( xContext );

        {
        // set up dbo current context
        ContextLayer layer( new user_CurrentContext( getCurrentContext(), OUSTR("dbo") ) );
        ::fprintf( stderr, "[security test] multi-user checking dbo..." );
        check_dbo_pos( ac );
        check_dbo_neg( ac );
        check_dbo_dynamic( ac );
        ::fprintf( stderr, "dbo checked.\n" );
        }
        {
        // set up jbu current context
        ContextLayer layer( new user_CurrentContext( getCurrentContext(), OUSTR("jbu") ) );
        ::fprintf( stderr, "[security test] multi-user checking jbu..." );
        check_jbu_pos( ac );
        check_jbu_neg( ac );
        ::fprintf( stderr, "jbu checked.\n" );
        }
        {
        // set up root current context
        ContextLayer layer( new user_CurrentContext( getCurrentContext(), OUSTR("root") ) );
        ::fprintf( stderr, "[security test] multi-user checking root..." );
        check_root_pos( ac );
        ::fprintf( stderr, "root checked.\n" );
        }
        {
        // set up unknown guest user current context => default permissions
        ContextLayer layer( new user_CurrentContext( getCurrentContext(), OUSTR("guest") ) );
        ::fprintf( stderr, "[security test] multi-user checking guest..." );
        check_defaults_pos( ac );
        check_defaults_neg( ac );
        ::fprintf( stderr, "guest checked.\n" );
        }

        dispose( xContext );
        ::fprintf( stderr, "security test succeeded.\n" );
        return 0;
    }
    catch (const Exception & exc)
    {
        OString str( OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
        ::fprintf( stderr, "[security test] error: %s!\n", str.getStr() );
        return 1;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
