/*************************************************************************
 *
 *  $RCSfile: testconnection.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:24:19 $
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

#include <osl/diagnose.h>
#include <vos/thread.hxx>

#include <cppuhelper/servicefactory.hxx>

#include <com/sun/star/lang/XComponent.hpp>

#include <com/sun/star/registry/XImplementationRegistration.hpp>

#include <com/sun/star/connection/XConnector.hpp>
#include <com/sun/star/connection/XAcceptor.hpp>

using namespace ::vos;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::connection;


class MyThread :
    public OThread
{
public:
    MyThread( const Reference< XAcceptor > &r , const OUString & sConnectionDescription) :
        m_rAcceptor( r ),
        m_sConnectionDescription( sConnectionDescription )
        {}
    virtual void SAL_CALL run();

private:
    Reference < XAcceptor > m_rAcceptor;
    Reference < XConnection > m_rConnection;
    OUString m_sConnectionDescription;
};

void doWrite( const Reference < XConnection > &r )
{
    Sequence < sal_Int8 > seq(10);
    for( sal_Int32 i = 0 ; i < 10 ; i ++ )
    {
        seq.getArray()[i] = i;
    }

    r->write( seq );
}

void doRead( const Reference < XConnection > &r )
{
    Sequence < sal_Int8 > seq(10);

    OSL_ASSERT( 10 == r->read( seq , 10 ) );

    for( sal_Int32 i = 0 ; i < 10 ; i ++ )
    {
        OSL_ASSERT( seq.getConstArray()[i] == i );
    }
}


void MyThread::run()
{
    try
    {
        m_rConnection = m_rAcceptor->accept( m_sConnectionDescription );
    }
    catch ( ... )
    {
        printf( "Exception was thrown by acceptor thread\n" );
        throw;
    }

    if( m_rConnection.is() )
    {
        Sequence < sal_Int8 > seq(12);
        try
        {
            doWrite( m_rConnection );
            doRead( m_rConnection );
        }
        catch (... )
        {
            printf( "unknown exception was thrown\n" );
            throw;
        }
    }

}





void testConnection( const OUString &sConnectionDescription  ,
                     const Reference < XAcceptor > &rAcceptor,
                     const Reference < XConnector > &rConnector )
{
    {
        MyThread thread( rAcceptor , sConnectionDescription );
        thread.create();

        sal_Bool bGotit = sal_False;
        Reference < XConnection > r;

        while( ! bGotit )
        {
            try
            {
                // Why is this wait necessary ????
                TimeValue value = {1,0};
                osl_waitThread( &value );
                r = rConnector->connect( sConnectionDescription );
                OSL_ASSERT( r.is() );
                doWrite( r );
                doRead( r );
                bGotit = sal_True;
            }
            catch( ... )
            {
                printf( "Couldn't connect, retrying ...\n" );

            }
        }

        r->close();

        try
        {
            Sequence < sal_Int8 > seq(10);
            r->write( seq );
            OSL_ENSHURE( 0 , "expected exception not thrown" );
        }
        catch ( IOException & )
        {
            // everything is ok
        }
        catch ( ... )
        {
            OSL_ENSHURE( 0 , "wrong exception was thrown" );
        }

        thread.join();
    }
}


#ifdef UNX
#define REG_PREFIX      L"lib"
#define DLL_POSTFIX     L".so"
#else
#define REG_PREFIX      L""
#define DLL_POSTFIX     L".dll"
#endif


#if (defined UNX) || (defined OS2)
int main( int argc, char * argv[] )
#else
int __cdecl main( int argc, char * argv[] )
#endif
{
#ifdef SOLARIS
    extern "C" ChangeGlobalInit();
    ChangeGlobalInit();
#endif
    Reference< XMultiServiceFactory > xMgr(
        createRegistryServiceFactory( OUString( RTL_CONSTASCII_USTRINGPARAM("applicat.rdb")) ) );

    Reference< XImplementationRegistration > xImplReg(
        xMgr->createInstance( L"com.sun.star.registry.ImplementationRegistration" ), UNO_QUERY );
    OSL_ENSHURE( xImplReg.is(), "### no impl reg!" );

    OUString aLibName( REG_PREFIX );
    aLibName += L"connectr";
#ifndef OS2
    aLibName += DLL_POSTFIX;
#endif
    xImplReg->registerImplementation(
        L"com.sun.star.loader.SharedLibrary", aLibName, Reference< XSimpleRegistry >() );

    aLibName = OUString( REG_PREFIX );
    aLibName += L"acceptor";
#ifndef OS2
    aLibName += DLL_POSTFIX;
#endif
    xImplReg->registerImplementation(
        L"com.sun.star.loader.SharedLibrary", aLibName, Reference< XSimpleRegistry >() );

    Reference < XAcceptor >  rAcceptor(
        xMgr->createInstance( L"com.sun.star.connection.Acceptor" ) , UNO_QUERY );

    Reference < XAcceptor >  rAcceptorPipe(
        xMgr->createInstance( L"com.sun.star.connection.Acceptor" ) , UNO_QUERY );

    Reference < XConnector >  rConnector(
        xMgr->createInstance( L"com.sun.star.connection.Connector" ) , UNO_QUERY );


    printf( "Testing sockets" );
    fflush( stdout );
    testConnection( L"socket,host=localhost,port=2001", rAcceptor , rConnector );
    printf( " Done\n" );

    printf( "Testing pipe" );
    fflush( stdout );
    testConnection( L"pipe,name=bla" , rAcceptorPipe , rConnector );
    printf( " Done\n" );


    Reference < XComponent > rComp( xMgr , UNO_QUERY );
    if( rComp.is() )
    {
        rComp->dispose();
    }
}
