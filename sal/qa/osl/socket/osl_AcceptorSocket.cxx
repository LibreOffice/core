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

/**  test coder preface:
    1. the BSD socket function will meet "unresolved external symbol error" on Windows platform
    if you are not including ws2_32.lib in makefile.mk,  the including format will be like this:

    .IF "$(OS)" == "WNT"
    SHL1STDLIBS +=  $(SOLARLIBDIR)$/cppunit.lib
    SHL1STDLIBS +=  ws2_32.lib
    .ENDIF

    likewise on Solaris platform.
    .IF "$(OS)" != "WNT"
    SHL1STDLIBS+=$(SOLARLIBDIR)$/libcppunit$(DLLPOSTFIX).a
    SHL1STDLIBS += -lsocket -ldl -lnsl
    .ENDIF

    2. since the Socket implementation of osl is only IPv4 oriented, our test are mainly focus on IPv4
    category.

    3. some fragment of Socket source implementation are lack of comment so it is hard for testers
    guess what the exact functionality or usage of a member.  Hope the Socket section's comment
    will be added.

    4. following functions are declared but not implemented:
    inline sal_Bool SAL_CALL operator== (const SocketAddr & Addr) const;
 */





#include <sal/types.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include "osl_Socket_Const.h"
#include "sockethelper.hxx"

using namespace osl;
using ::rtl::OUString;

#define IP_PORT_FTP     21
#define IP_PORT_MYPORT9 8897
#define IP_PORT_MYPORT4 8885
#define IP_PORT_MYPORT3 8884






class AcceptorThread : public Thread
{
    ::osl::AcceptorSocket asAcceptorSocket;
    ::rtl::OUString aHostIP;
    sal_Bool bOK;
protected:
    void SAL_CALL run( )
    {
        ::osl::SocketAddr saLocalSocketAddr( aHostIP, IP_PORT_MYPORT9 );
        ::osl::StreamSocket ssStreamConnection;

        asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); 
        sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
        if  ( sal_True != bOK1 )
        {
            t_print("# AcceptorSocket bind address failed.\n" ) ;
            return;
        }
        sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
        if  ( sal_True != bOK2 )
        {
            t_print("# AcceptorSocket listen address failed.\n" ) ;
            return;
        }

        asAcceptorSocket.enableNonBlockingMode( sal_False );

        oslSocketResult eResult = asAcceptorSocket.acceptConnection( ssStreamConnection );
        if (eResult != osl_Socket_Ok )
        {
            bOK = sal_True;
            t_print("AcceptorThread: acceptConnection failed! \n");
        }
    }
public:
    AcceptorThread(::osl::AcceptorSocket & asSocket, ::rtl::OUString const& aBindIP )
        : asAcceptorSocket( asSocket ), aHostIP( aBindIP )
    {
        bOK = sal_False;
    }

    sal_Bool isOK() { return bOK; }

    ~AcceptorThread( )
    {
        if ( isRunning( ) )
        {
            asAcceptorSocket.shutdown();
            t_print("# error: Acceptor thread not terminated.\n" );
        }
    }
};

namespace osl_AcceptorSocket
{

    /** testing the methods:
        inline AcceptorSocket(oslAddrFamily Family = osl_Socket_FamilyInet,
                              oslProtocol   Protocol = osl_Socket_ProtocolIp,
                              oslSocketType Type = osl_Socket_TypeStream);
    */

    class ctors : public CppUnit::TestFixture
    {
    public:

        void ctors_001()
        {
            
            ::osl::AcceptorSocket asSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );

            CPPUNIT_ASSERT_MESSAGE( "test for ctors_001 constructor function: check if the acceptor socket was created successfully.",
                                    osl_Socket_TypeStream ==  asSocket.getType( ) );
        }

        CPPUNIT_TEST_SUITE( ctors );
        CPPUNIT_TEST( ctors_001 );
        CPPUNIT_TEST_SUITE_END();

    }; 

    /** testing the method:
        inline sal_Bool SAL_CALL listen(sal_Int32 MaxPendingConnections= -1);
        inline oslSocketResult SAL_CALL acceptConnection( StreamSocket& Connection);
        inline oslSocketResult SAL_CALL acceptConnection( StreamSocket& Connection, SocketAddr & PeerAddr);
    */

    class listen_accept : public CppUnit::TestFixture
    {
    public:
        TimeValue *pTimeout;
        ::osl::AcceptorSocket asAcceptorSocket;
        ::osl::ConnectorSocket csConnectorSocket;


        
        void setUp( )
        {
            pTimeout  = ( TimeValue* )malloc( sizeof( TimeValue ) );
            pTimeout->Seconds = 3;
            pTimeout->Nanosec = 0;
            asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, 1);
        
        }

        void tearDown( )
        {
            free( pTimeout );
        
            asAcceptorSocket.close( );
            csConnectorSocket.close( );
        }


        void listen_accept_001()
        {
            ::osl::SocketAddr saLocalSocketAddr( rtl::OUString("127.0.0.1"), IP_PORT_MYPORT3 );
            ::osl::SocketAddr saTargetSocketAddr( rtl::OUString("127.0.0.1"), IP_PORT_MYPORT3 );
            ::osl::StreamSocket ssConnection;

            
            sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
            CPPUNIT_ASSERT_MESSAGE( "AcceptorSocket bind address failed.", sal_True == bOK1 );
            sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
            CPPUNIT_ASSERT_MESSAGE( "AcceptorSocket listen failed.",  sal_True == bOK2 );
            asAcceptorSocket.enableNonBlockingMode( sal_True );

            
            csConnectorSocket.connect( saTargetSocketAddr, pTimeout );   

            oslSocketResult eResult = asAcceptorSocket.acceptConnection(ssConnection); 

            CPPUNIT_ASSERT_MESSAGE( "test for listen_accept function: try to create a connection with remote host, using listen and accept.",
                ( osl_Socket_Ok == eResult ) );
        }

        void listen_accept_002()
        {
            ::osl::SocketAddr saLocalSocketAddr( rtl::OUString("127.0.0.1"), IP_PORT_MYPORT4 );
            ::osl::SocketAddr saTargetSocketAddr( rtl::OUString("127.0.0.1"), IP_PORT_MYPORT4 );
            ::osl::SocketAddr saPeerSocketAddr( rtl::OUString("129.158.217.202"), IP_PORT_FTP );
            ::osl::StreamSocket ssConnection;

            
            sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
            CPPUNIT_ASSERT_MESSAGE( "AcceptorSocket bind address failed.", sal_True == bOK1 );
            sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
            CPPUNIT_ASSERT_MESSAGE( "AcceptorSocket listen failed.",  sal_True == bOK2 );
            asAcceptorSocket.enableNonBlockingMode( sal_True );

            
            csConnectorSocket.connect( saTargetSocketAddr, pTimeout );   

            oslSocketResult eResult = asAcceptorSocket.acceptConnection(ssConnection, saPeerSocketAddr); 

            CPPUNIT_ASSERT_MESSAGE( "test for listen_accept function: try to create a connection with remote host, using listen and accept, accept with peer address.",
                                    ( sal_True == bOK2 ) &&
                                    ( osl_Socket_Ok == eResult ) &&
                                    ( sal_True == compareSocketAddr( saPeerSocketAddr, saLocalSocketAddr ) ) );
        }


        CPPUNIT_TEST_SUITE( listen_accept );
        CPPUNIT_TEST( listen_accept_001 );
        CPPUNIT_TEST( listen_accept_002 );
        CPPUNIT_TEST_SUITE_END();

    }; 




CPPUNIT_TEST_SUITE_REGISTRATION(osl_AcceptorSocket::ctors);

CPPUNIT_TEST_SUITE_REGISTRATION(osl_AcceptorSocket::listen_accept);

} 





CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
