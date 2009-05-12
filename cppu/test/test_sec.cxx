/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: test_sec.cxx,v $
 * $Revision: 1.5 $
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
#include "precompiled_cppu.hxx"

#include <stdio.h>


#include <uno/current_context.hxx>

#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/container/XNameContainer.hpp>

using namespace rtl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::security;
using namespace com::sun::star::lang;
using namespace com::sun::star::loader;
using namespace com::sun::star::registry;
using namespace com::sun::star::container;


static inline void out( const sal_Char * p )
{
    ::fprintf( stderr, p );
}
static inline void out( const OUString & r )
{
    OString aStr( OUStringToOString( r, RTL_TEXTENCODING_ASCII_US ) );
    out( aStr.getStr() );
}


static Reference< XInterface > load(
    const Reference< XMultiServiceFactory > & xMgr,
    const char * service,
    const char * implName, const char * activator, const char * loc ) throw ()
{
    Reference< XInterface > xRet(
        xMgr->createInstance( OUString::createFromAscii( service ) ) );
    if (xRet.is())
        return xRet;

    try
    {
        Reference< XImplementationLoader > xLoader(
            xMgr->createInstance( OUString::createFromAscii( activator ) ), UNO_QUERY );
        if (xLoader.is())
        {
            Reference< XSingleServiceFactory > xFac( xLoader->activate(
                OUString::createFromAscii( implName ), OUString(),
                OUString::createFromAscii( loc ),
                Reference< XRegistryKey >() ), UNO_QUERY );
            if (xFac.is())
                return xFac->createInstance();
        }
        else
        {
            out( "### cannot activate loader \"" );
            out( activator );
            out( "\"!\n" );
        }
    }
    catch (Exception &)
    {
        out( "### cannot activate service \"" );
        out( service );
        out( "\"!\n" );
    }
    return Reference< XInterface >();
}

static void setEnv( const Reference< XMultiServiceFactory > & xMgr ) throw ()
{
    OSL_ASSERT( xMgr.is() );
    Reference< XNameContainer > xContext( getCurrentContext(), UNO_QUERY );
    OSL_ASSERT( xContext.is() );

    try
    {
        xContext->insertByName(
            OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.lang.ServiceManager") ),
            makeAny( xMgr ) );
        xContext->insertByName(
            OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.security.AccessController") ),
            makeAny( load( xMgr,
                           "com.sun.star.security.AccessController",
                           "com.sun.star.comp.security.AccessController",
                           "com.sun.star.loader.Java2",
                           "com.sun.star.comp.security.AccessController" ) ) );
        xContext->insertByName(
            OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.security.auth.login.LoginContext") ),
            makeAny( load( xMgr,
                           "com.sun.star.security.auth.login.LoginContext",
                           "com.sun.star.comp.security.auth.login.LoginContext",
                           "com.sun.star.loader.Java2",
                           "com.sun.star.comp.security.auth.login.LoginContext" ) ) );
    }
    catch (Exception & rExc)
    {
        out( "### exception occured: " );
        out( rExc.Message );
        out( "\n" );
    }
}

//##################################################################################################
//##################################################################################################
//##################################################################################################

struct PrivAction : WeakImplHelper1< XPrivilegedAction >
{
    virtual Any SAL_CALL run()
        throw (Exception)
    {
        throw IllegalArgumentException(
            OUString::createFromAscii("testtest"), (OWeakObject *)this, (sal_Int16)5 );
    }
};

struct Thread1 : public OThread
{
    Reference< XMultiServiceFactory > _xMgr;

    Thread1( const Reference< XMultiServiceFactory > & xMgr )
        : _xMgr( xMgr )
        {}

    void f()
    {
        Reference< XNameAccess > xAccess( getCurrentContext(), UNO_QUERY );
        OSL_ASSERT( xAccess->getByName( OUString::createFromAscii("a") ) == (sal_Int16)5 );
        Reference< XNameContainer > xCont( getCurrentContext(), UNO_QUERY );
        xCont->insertByName( OUString::createFromAscii("b"), makeAny( (sal_Int32)6 ) );
    }
    virtual void SAL_CALL run()
    {
        try
        {
            setEnv( _xMgr );
            Reference< XNameContainer > xCont( getCurrentContext(), UNO_QUERY );
            xCont->insertByName( OUString::createFromAscii("a"), makeAny( (sal_Int32)5 ) );
            f();
            Reference< XNameAccess > xAccess( getCurrentContext(), UNO_QUERY );
            OSL_ASSERT( xAccess->getByName( OUString::createFromAscii("b") ) == (sal_Int16)6 );

//              checkPermission(
//                  Permission( OUString::createFromAscii("java.io.FilePermission"),
//                              OUString::createFromAscii("f:\\userprofiles.dat"),
//                              OUString::createFromAscii("read") ) );

//              try
//              {
//                  Reference< XCurrentContext > xContext( getCurrentContext() );
//                  Reference< XAccessController > xACC( xContext->getAccessController() );
//                  xACC->doPrivileged( new PrivAction(), Reference< XAccessControlContext >() );
//              }
//              catch (IllegalArgumentException & r)
//              {
//                  OSL_ASSERT( r.ArgumentPosition == 5 &&
//                              r.Message.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("testtest") ));
//              }
        }
        catch (RuntimeException & rExc)
        {
            out( rExc.Message );
        }
    }
};


void test_security( const Reference< XMultiServiceFactory > & xMgr ) throw ()
{
    setEnv( xMgr );
    OSL_ASSERT( getCurrentContext()->getServiceManager().is() );

    Thread1 thread1( xMgr );
    thread1.create();
    thread1.join();
    out( "> exiting...\n" );
}
