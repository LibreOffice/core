/*************************************************************************
 *
 *  $RCSfile: test_security.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dbo $ $Date: 2002-01-25 12:47:36 $
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

#include <stdio.h>

#include <osl/diagnose.h>
#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <uno/current_context.hxx>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/access_control.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/uno/XCurrentContext.hpp>

#define USER_CREDS "access-control.user-credentials"
#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


using namespace ::cppu;
using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

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
        SAL_THROW( () )
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
    ::osl_incrementInterlockedCount( &m_refcount );
}
//__________________________________________________________________________________________________
void user_CurrentContext::release()
    throw ()
{
    if (! ::osl_decrementInterlockedCount( &m_refcount ))
    {
        delete this;
    }
}
//__________________________________________________________________________________________________
Any user_CurrentContext::getValueByName( OUString const & name )
    throw (RuntimeException)
{
    if (name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(USER_CREDS ".id") ))
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
            try \
            { \
                check; \
                throw RuntimeException( \
                    OUSTR("expected AccessControlException!"), Reference< XInterface >() ); \
            } \
            catch (security::AccessControlException &) \
            { \
            } \
        } \
        else \
        { \
            check; \
        } \
    } \
    catch (RuntimeException & exc) \
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
permission com.sun.star.io.FilePermission "file:///usr/bin/*", "read";
permission com.sun.star.io.FilePermission "file:///tmp/-", "read,write";
permission com.sun.star.io.FilePermission "file:///etc/profile", "read";
};
*/
static void check_defaults_pos( AccessControl & ac, bool invert = false )
{
    // positive tests
    CHECK( ac.checkFilePermission( OUSTR("file:///usr/bin/bla"), OUSTR("read") ), invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///tmp/bla"), OUSTR("read,write") ), invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///tmp/path/path/bla"), OUSTR("write") ), invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///etc/profile"), OUSTR("read") ), invert );
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
    CHECK( ac.checkFilePermission( OUSTR("file:///root"), OUSTR("bla") ), !invert );
    CHECK( ac.checkFilePermission( OUSTR("file:///root"), OUString() ), !invert );
}

/*
grant user "dbo"
{
permission com.sun.star.io.FilePermission "file:///home/dbo/-", "read,write";
permission com.sun.star.io.FilePermission "-", "read,write";
permission com.sun.star.io.FilePermission "file:///usr/local/dbo/*", "read";
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
}

/*
grant user "jbu"
{
permission com.sun.star.io.FilePermission  "file:///home/jbu/-", "read,write";
permission com.sun.star.io.FilePermission "*", "read,write";
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
}

//==================================================================================================
int SAL_CALL main( int argc, char * argv [] )
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
        ::fprintf( stderr, "checked.\n" );
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
        ::fprintf( stderr, "checked.\n" );
        }
        {
        // set up jbu current context
        ContextLayer layer( new user_CurrentContext( getCurrentContext(), OUSTR("jbu") ) );
        ::fprintf( stderr, "[security test] multi-user checking jbu..." );
        check_jbu_pos( ac );
        check_jbu_neg( ac );
        ::fprintf( stderr, "checked.\n" );
        }
        {
        // set up root current context
        ContextLayer layer( new user_CurrentContext( getCurrentContext(), OUSTR("root") ) );
        ::fprintf( stderr, "[security test] multi-user checking root..." );
        check_root_pos( ac );
        ::fprintf( stderr, "checked.\n" );
        }
        {
        // set up unknown user current context
        ContextLayer layer( new user_CurrentContext( getCurrentContext(), OUSTR("vicious") ) );
        ::fprintf( stderr, "[security test] multi-user checking vicious..." );
        check_root_pos( ac, true );
        ::fprintf( stderr, "checked.\n" );
        }

        dispose( xContext );
        ::fprintf( stderr, "security test succeeded.\n" );
        return 0;
    }
    catch (Exception & exc)
    {
        OString str( OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
        ::fprintf( stderr, "[security test] error: %s!\n", str.getStr() );
        return 1;
    }
}
