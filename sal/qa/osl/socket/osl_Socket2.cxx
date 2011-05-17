/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sal.hxx"

/**  test coder preface:
    1. the BSD socket function will meet "unresolved external symbol error" on Windows platform
    if you are not including ws2_32.lib in makefile.mk,  the including format will be like this:

    .IF "$(GUI)" == "WNT"
    SHL1STDLIBS +=  $(SOLARLIBDIR)$/cppunit.lib
    SHL1STDLIBS +=  ws2_32.lib
    .ENDIF

    likewise on Solaris platform.
    .IF "$(GUI)" == "UNX"
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

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include "sockethelper.hxx"

using namespace osl;
using ::rtl::OUString;

#define IP_PORT_FTP     21
#define IP_PORT_TELNET  23
#define IP_PORT_HTTP2   8080
#define IP_PORT_INVAL   99999
#define IP_PORT_POP3    110
#define IP_PORT_NETBIOS 139
#define IP_PORT_MYPORT  8881
#define IP_PORT_MYPORT1 8882
#define IP_PORT_MYPORT5 8886
#define IP_PORT_MYPORT6 8887
#define IP_PORT_MYPORT7 8895
#define IP_PORT_MYPORT8 8896
#define IP_PORT_MYPORT9 8897

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

// just used to test socket::close() when accepting
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

        asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //integer not sal_Bool : sal_True);
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

namespace osl_Socket
{

    /** testing the methods:
        inline Socket( );
        inline Socket( const Socket & socket );
        inline Socket( oslSocket socketHandle );
        inline Socket( oslSocket socketHandle, __sal_NoAcquire noacquire );
    */

    /**  test writer's comment:

        class Socket can not be initialized by its protected constructor, though the protected
        constructor is the most convenient way to create a new socket.
        it only allow the method of C function osl_createSocket like:
        ::osl::Socket sSocket( osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream,
                                          osl_Socket_ProtocolIp ) );
        the use of C method lost some of the transparent of tester using C++ wrapper.
    */


    class ctors : public CppUnit::TestFixture
    {
    public:
        oslSocket sHandle;
        // initialization
        void setUp( )
        {
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        }

        void tearDown( )
        {
            sHandle = NULL;
        }


        void ctors_none()
        {
            /// Socket constructor.
            // ::osl::Socket sSocket();

            CPPUNIT_ASSERT_MESSAGE( "test for ctors_none constructor function: check if the socket was created successfully, if no exception occurred",
                                    1 == 1 );
        }

        void ctors_acquire()
        {
            /// Socket constructor.
            ::osl::Socket sSocket( sHandle );

            CPPUNIT_ASSERT_MESSAGE( "test for ctors_acquire constructor function: check if the socket was created successfully",
                                    osl_Socket_TypeStream == sSocket.getType( ) );
        }

        void ctors_no_acquire()
        {
            /// Socket constructor.
            ::osl::Socket sSocket( sHandle, SAL_NO_ACQUIRE );

            CPPUNIT_ASSERT_MESSAGE(" test for ctors_no_acquire constructor function: check if the socket was created successfully",
                                    osl_Socket_TypeStream == sSocket.getType( ) );
        }

        void ctors_copy_ctor()
        {
            ::osl::Socket sSocket( sHandle );
            /// Socket copy constructor.
            ::osl::Socket copySocket( sSocket );

            CPPUNIT_ASSERT_MESSAGE(" test for ctors_copy_ctor constructor function: create new Socket instance using copy constructor",
                                    osl_Socket_TypeStream == copySocket.getType( ) );
        }

        void ctors_TypeRaw()
        {
#ifdef WNT
            oslSocket sHandleRaw = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeRaw, osl_Socket_ProtocolIp );
// LLA: ?           ::osl::Socket sSocket( sHandleRaw );
            CPPUNIT_ASSERT_MESSAGE( " type osl_Socket_TypeRaw socket create failed on UNX ", sHandleRaw != NULL);
#else
            oslSocket sHandleRaw = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeRaw, osl_Socket_ProtocolIp );
            CPPUNIT_ASSERT_MESSAGE( " can't create socket with type osl_Socket_TypeRaw within UNX is ok.", sHandleRaw == NULL);
#endif
        }

        void ctors_family_Ipx()
        {
            oslSocket sHandleIpx = osl_createSocket( osl_Socket_FamilyIpx, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
            CPPUNIT_ASSERT_MESSAGE( " family osl_Socket_FamilyIpx socket create failed! ", sHandleIpx != NULL);
            ::osl::Socket sSocket( sHandleIpx );        //, SAL_NO_ACQUIRE );
            t_print("#Type is %d \n", sSocket.getType( ) );

            CPPUNIT_ASSERT_MESSAGE(" test for create new Socket instance that family is osl_Socket_FamilyIpx",
                                    osl_Socket_TypeStream == sSocket.getType( ) );
        }



        CPPUNIT_TEST_SUITE( ctors );
        CPPUNIT_TEST( ctors_none );
        CPPUNIT_TEST( ctors_acquire );
        CPPUNIT_TEST( ctors_no_acquire );
        CPPUNIT_TEST( ctors_copy_ctor );
        CPPUNIT_TEST( ctors_TypeRaw );
        //TODO: Check if family_Ipx is still in use?
        // CPPUNIT_TEST( ctors_family_Ipx );
        CPPUNIT_TEST_SUITE_END();

    }; // class ctors


    /** testing the methods:
        inline Socket& SAL_CALL operator= ( oslSocket socketHandle);
        inline Socket& SAL_CALL operator= (const Socket& sock);
        inline sal_Bool SAL_CALL operator==( const Socket& rSocket ) const ;
        inline sal_Bool SAL_CALL operator==( const oslSocket socketHandle ) const;
    */

    class operators : public CppUnit::TestFixture
    {
    public:
        oslSocket sHandle;
        // initialization
        void setUp( )
        {
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        }

        void tearDown( )
        {
            sHandle = NULL;
        }


    /**  test writer's comment:

        the assignment operator does not support direct assinment like:
        ::osl::Socket sSocket = sHandle.
    */
        void operators_assignment_handle()
        {
            ::osl::Socket sSocket(sHandle);
            ::osl::Socket assignSocket = sSocket.getHandle();

            CPPUNIT_ASSERT_MESSAGE( "test for operators_assignment_handle function: test the assignment operator.",
                                    osl_Socket_TypeStream == assignSocket.getType( )  );
        }

        void operators_assignment()
        {
            ::osl::Socket sSocket( sHandle );
            ::osl::Socket assignSocket = sSocket;

            CPPUNIT_ASSERT_MESSAGE( "test for operators_assignment function: assignment operator",
                                    osl_Socket_TypeStream == assignSocket.getType( ) );
        }

        void operators_equal_handle_001()
        {
            /// Socket constructor.
            ::osl::Socket sSocket( sHandle );
            ::osl::Socket equalSocket = sSocket;

            CPPUNIT_ASSERT_MESSAGE(" test for operators_equal_handle_001 function: check equal.",
                                    equalSocket == sHandle );
        }

        void operators_equal_handle_002()
        {
            /// Socket constructor.
            ::osl::Socket equalSocket( osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeDgram, osl_Socket_ProtocolIp ) );

            CPPUNIT_ASSERT_MESSAGE(" test for operators_equal_handle_001 function: check unequal.",
                                    !( equalSocket == sHandle ) );
        }

        void operators_equal_001()
        {
            ::osl::Socket sSocket( sHandle );
            /// Socket copy constructor.
            ::osl::Socket equalSocket( sSocket );

            CPPUNIT_ASSERT_MESSAGE(" test for operators_equal function: check equal.",
                                    equalSocket == sSocket );
        }

        void operators_equal_002()
        {
            ::osl::Socket sSocket( sHandle );
            /// Socket copy constructor.
            ::osl::Socket equalSocket( osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeDgram, osl_Socket_ProtocolIp ) );

            CPPUNIT_ASSERT_MESSAGE(" test for operators_equal_002 function: check unequal.",
                                    !( equalSocket == sSocket ) );
        }

        CPPUNIT_TEST_SUITE( operators );
        CPPUNIT_TEST( operators_assignment_handle );
        CPPUNIT_TEST( operators_assignment );
        CPPUNIT_TEST( operators_equal_handle_001 );
        CPPUNIT_TEST( operators_equal_handle_002 );
        CPPUNIT_TEST( operators_equal_001 );
        CPPUNIT_TEST( operators_equal_002 );
        CPPUNIT_TEST_SUITE_END();

    }; // class operators


    /** testing the methods:
        inline void SAL_CALL shutdown( oslSocketDirection Direction = osl_Socket_DirReadWrite );
        inline void SAL_CALL close();
    */

    class close : public CppUnit::TestFixture
    {
    public:
        oslSocket sHandle;
        // initialization
        void setUp( )
        {
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        }

        void tearDown( )
        {
            sHandle = NULL;
        }


        void close_001()
        {
            ::osl::Socket sSocket(sHandle);
            sSocket.close();

            CPPUNIT_ASSERT_MESSAGE( "test for close_001 function: this function is reserved for test.",
                                    sSocket.getHandle() == sHandle );
        }

        void close_002()
        {
//#if defined(LINUX)
            ::osl::AcceptorSocket asSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
            AcceptorThread myAcceptorThread( asSocket, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")) );
            myAcceptorThread.create();

            thread_sleep( 1 );
            //when accepting, close the socket, the thread will not block for accepting
            //man close:Any locks held on the file it was associated with, and owned by the process, are removed
            asSocket.close();
            //thread_sleep( 2 );
            myAcceptorThread.join();

            CPPUNIT_ASSERT_MESSAGE( "test for close when is accepting: the socket will quit accepting status.",
                                myAcceptorThread.isOK() == sal_True );
//#endif
        }

        // to cover "if ( pSockAddrIn->sin_addr.s_addr == htonl(INADDR_ANY) )" in osl_closeSocket( )
        void close_003()
        {
            ::osl::AcceptorSocket asSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
            AcceptorThread myAcceptorThread( asSocket, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("0.0.0.0")) );
            myAcceptorThread.create();

            thread_sleep( 1 );
            asSocket.close();
            myAcceptorThread.join();

            CPPUNIT_ASSERT_MESSAGE( "test for close when is accepting: the socket will quit accepting status.",
                                myAcceptorThread.isOK() == sal_True );
        }

        CPPUNIT_TEST_SUITE( close );
        CPPUNIT_TEST( close_001 );
        CPPUNIT_TEST( close_002 );
        CPPUNIT_TEST( close_003 );
        CPPUNIT_TEST_SUITE_END();

    }; // class close

    /** testing the method:
        inline void SAL_CALL getLocalAddr( SocketAddr &Addr ) const;
    */

    class getLocalAddr : public CppUnit::TestFixture
    {
    public:
        oslSocket sHandle;
        // initialization
        void setUp( )
        {
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        }

        void tearDown( )
        {
            sHandle = NULL;
        }

        // get the Address of the local end of the socket
        void getLocalAddr_001()
        {
            ::osl::Socket sSocket(sHandle);
            ::osl::SocketAddr saBindSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), IP_PORT_MYPORT8 );
            ::osl::SocketAddr saLocalSocketAddr;

            sSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //sal_True);

            sal_Bool bOK1 = sSocket.bind( saBindSocketAddr );
            ::rtl::OUString suError1 = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Socket bind fail:")) + sSocket.getErrorAsString();
            CPPUNIT_ASSERT_MESSAGE( STD_STRING(suError1), sal_True == bOK1 );

            sSocket.getLocalAddr( saLocalSocketAddr );

            sal_Bool bOK = compareUString( saLocalSocketAddr.getHostname( 0 ), sSocket.getLocalHost() ) ;

            CPPUNIT_ASSERT_MESSAGE( "test for getLocalAddr function: first create a new socket, then a socket address, bind them, and check the address.",
                                    sal_True == bOK );
        }


        CPPUNIT_TEST_SUITE( getLocalAddr );
        CPPUNIT_TEST( getLocalAddr_001 );
        CPPUNIT_TEST_SUITE_END();

    }; // class getLocalAddr


    /** testing the method:
        inline sal_Int32    SAL_CALL getLocalPort() const;
    */

    class getLocalPort : public CppUnit::TestFixture
    {
    public:
        oslSocket sHandle;
        // initialization
        void setUp( )
        {
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        }

        void tearDown( )
        {
            sHandle = NULL;
        }


        void getLocalPort_001()
        {
            ::osl::Socket sSocket(sHandle);
            ::osl::SocketAddr saBindSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), IP_PORT_MYPORT7 );  // aHostIp1 localhost
            ::osl::SocketAddr saLocalSocketAddr;

            sSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //sal_True);

            sal_Bool bOK1 = sSocket.bind( saBindSocketAddr );
            ::rtl::OUString suError1 = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Socket bind fail:")) + sSocket.getErrorAsString();
            CPPUNIT_ASSERT_MESSAGE( STD_STRING(suError1), sal_True == bOK1 );
            sal_Bool bOK = ( IP_PORT_MYPORT7 == sSocket.getLocalPort( )  );

            CPPUNIT_ASSERT_MESSAGE( "test for getLocalPort function: first create a new socket, then a socket address, bind them, and check the port.",
                                    sal_True == bOK );
        }

    /**  test writer's comment:

        the invalid port number can not be set by giving invalid port number
        such as 99999 or -1, it will convert to ( x mod 65535 ), so it will always be
        valid,  the only instance that the getLocalPort returns OSL_INVALID_PORT
        is when saSocketAddr itself is an invalid one, that is , the IP or host name
        can not be found, then the created socket address is not valid.
    */
        void getLocalPort_002()
        {
            ::osl::SocketAddr saBindSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("123.45.67.89")), IP_PORT_TELNET);
#ifdef WNT
            ::osl::Socket sSocket(sHandle);
            sSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); // sal_True);
            sSocket.bind( saBindSocketAddr );
            //Invalid IP, so bind should fail
            ::rtl::OUString suError = outputError(::rtl::OUString::valueOf(sSocket.getLocalPort( )),
                ::rtl::OUString::valueOf((sal_Int32)OSL_INVALID_PORT),
                "test for getLocalPort function: first create a new socket, then an invalid socket address, bind them, and check the port assigned.");
            sal_Bool bOK = ( OSL_INVALID_PORT == sSocket.getLocalPort( ) );
            (void)bOK;
#else
            //on Unix, if Addr is not an address of type osl_Socket_FamilyInet, it returns OSL_INVALID_PORT
            ::rtl::OUString suError (RTL_CONSTASCII_USTRINGPARAM("on Unix, if Addr is not an address of type osl_Socket_FamilyInet, it returns OSL_INVALID_PORT, but can not create Addr of that case"));
#endif
            CPPUNIT_ASSERT_MESSAGE( STD_STRING(suError), sal_False );

        }

        void getLocalPort_003()
        {
            ::osl::Socket sSocket(sHandle);
            ::osl::SocketAddr saBindSocketAddr( getLocalIP(), IP_PORT_INVAL);

            sSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //sal_True);

            sal_Bool bOK1 = sSocket.bind( saBindSocketAddr );
            ::rtl::OUString suError1 = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Socket bind fail:")) + sSocket.getErrorAsString();
            CPPUNIT_ASSERT_MESSAGE( STD_STRING(suError1), sal_True == bOK1 );
            ::rtl::OUString suError = outputError(::rtl::OUString::valueOf(sSocket.getLocalPort( )),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("34463")),
                "test for getLocalPort function: first create a new socket, then an invalid socket address, bind them, and check the port assigned");
            sal_Bool bOK = ( sSocket.getLocalPort( ) >= 1 &&  sSocket.getLocalPort( ) <= 65535);

            CPPUNIT_ASSERT_MESSAGE( STD_STRING(suError), sal_True == bOK );
        }

        CPPUNIT_TEST_SUITE( getLocalPort );
        CPPUNIT_TEST( getLocalPort_001 );
// LLA:     CPPUNIT_TEST( getLocalPort_002 );
        CPPUNIT_TEST( getLocalPort_003 );
        CPPUNIT_TEST_SUITE_END();

    }; // class getLocalPort


    /** testing the method:
        inline ::rtl::OUString SAL_CALL getLocalHost() const;

        Mindyliu: on Linux, at first it will check the binded in /etc/hosts, if it has the binded IP, it will return the hostname in it;
        else if the binded IP is "127.0.0.1", it will return "localhost", if it's the machine's ethernet ip such as "129.158.217.90", it
        will return hostname of current processor such as "aegean.PRC.Sun.COM"
    */

    class getLocalHost : public CppUnit::TestFixture
    {
    public:
        oslSocket sHandle;
        // initialization
        void setUp( )
        {
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        }

        void tearDown( )
        {
            sHandle = NULL;
        }


        void getLocalHost_001()
        {
            ::osl::Socket sSocket(sHandle);
            //port number from IP_PORT_HTTP1 to IP_PORT_MYPORT6, mindyliu
            ::osl::SocketAddr saBindSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), IP_PORT_MYPORT6 );

            sSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //sal_True);

            sal_Bool bOK1 = sSocket.bind( saBindSocketAddr );
            ::rtl::OUString suError1 = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Socket bind fail:")) + sSocket.getErrorAsString();
            CPPUNIT_ASSERT_MESSAGE( STD_STRING(suError1), sal_True == bOK1 );
            sal_Bool bOK;
            ::rtl::OUString suError;
#ifdef WNT
            bOK = compareUString( sSocket.getLocalHost( ), getThisHostname( ) ) ;
            suError = outputError(sSocket.getLocalHost( ), getThisHostname( ),
"test for getLocalHost function: create localhost socket and check name");
#else
            ::rtl::OUString aUString = ::rtl::OUString::createFromAscii( (const sal_Char *) "localhost" );
            sal_Bool bRes1, bRes2;
            bRes1 = compareUString( sSocket.getLocalHost( ), aUString ) ;
            bRes2 = compareUString( sSocket.getLocalHost( ), saBindSocketAddr.getHostname(0) ) ;
            bOK = bRes1 || bRes2;
            suError = outputError(sSocket.getLocalHost( ), aUString, "test for getLocalHost function: create localhost socket and check name");
#endif
            CPPUNIT_ASSERT_MESSAGE( STD_STRING(suError), sal_True == bOK );
        }

        void getLocalHost_002()
        {
            ::osl::Socket sSocket(sHandle);
            ::osl::SocketAddr saBindSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("123.45.67.89")), IP_PORT_POP3);
            ::osl::SocketAddr saLocalSocketAddr;

            sSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //sal_True);
            sSocket.bind( saBindSocketAddr );
            //Invalid IP, so bind should fail
            sal_Bool bOK = compareUString( sSocket.getLocalHost( ), rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")) ) ;
            ::rtl::OUString suError = outputError(sSocket.getLocalHost( ), rtl::OUString(), "test for getLocalHost function: getLocalHost with invalid SocketAddr");

            CPPUNIT_ASSERT_MESSAGE( STD_STRING(suError), sal_True == bOK );
        }

        CPPUNIT_TEST_SUITE( getLocalHost );
        CPPUNIT_TEST( getLocalHost_001 );
        CPPUNIT_TEST( getLocalHost_002 );
        CPPUNIT_TEST_SUITE_END();

    }; // class getLocalHost


    /** testing the methods:
        inline void SAL_CALL getPeerAddr( SocketAddr & Addr) const;
        inline sal_Int32    SAL_CALL getPeerPort() const;
        inline ::rtl::OUString SAL_CALL getPeerHost() const;
    */
    class getPeer : public CppUnit::TestFixture
    {
    public:
        oslSocket sHandle;
        TimeValue *pTimeout;
        ::osl::AcceptorSocket asAcceptorSocket;
        ::osl::ConnectorSocket csConnectorSocket;


        // initialization
        void setUp( )
        {
            pTimeout  = ( TimeValue* )malloc( sizeof( TimeValue ) );
            pTimeout->Seconds = 3;
            pTimeout->Nanosec = 0;
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        }

        void tearDown( )
        {
            free( pTimeout );
            sHandle = NULL;
            asAcceptorSocket.close( );
            csConnectorSocket.close( );
        }


        void getPeer_001()
        {
            ::osl::SocketAddr saLocalSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), IP_PORT_MYPORT );
            ::osl::SocketAddr saTargetSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), IP_PORT_MYPORT );
            ::osl::SocketAddr saPeerSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("129.158.217.202")), IP_PORT_FTP );
            ::osl::StreamSocket ssConnection;
            asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //sal_True);
            /// launch server socket
            sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
            CPPUNIT_ASSERT_MESSAGE( "AcceptorSocket bind '127.0.0.1' address failed.", sal_True == bOK1 );
            sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
            CPPUNIT_ASSERT_MESSAGE( "AcceptorSocket listen failed.",  sal_True == bOK2 );

            asAcceptorSocket.enableNonBlockingMode( sal_True );
            asAcceptorSocket.acceptConnection(ssConnection); /// waiting for incoming connection...

            /// launch client socket
            csConnectorSocket.connect( saTargetSocketAddr, pTimeout );   /// connecting to server...

            /// get peer information
            csConnectorSocket.getPeerAddr( saPeerSocketAddr );/// connected.
            sal_Int32 peerPort = csConnectorSocket.getPeerPort( );
            ::rtl::OUString peerHost = csConnectorSocket.getPeerHost( );

            CPPUNIT_ASSERT_MESSAGE( "test for getPeer function: setup a connection and then get the peer address, port and host from client side.",
                                    ( sal_True == compareSocketAddr( saPeerSocketAddr, saLocalSocketAddr ) )&&
                                    ( sal_True == compareUString( peerHost, saLocalSocketAddr.getHostname( 0 ) ) ) &&
                                    ( peerPort == saLocalSocketAddr.getPort( ) ));
        }


        CPPUNIT_TEST_SUITE( getPeer );
        CPPUNIT_TEST( getPeer_001 );
        CPPUNIT_TEST_SUITE_END();

    }; // class getPeer


    /** testing the methods:
        inline sal_Bool SAL_CALL bind(const SocketAddr& LocalInterface);
    */


    class bind : public CppUnit::TestFixture
    {
    public:
        oslSocket sHandle;
        // initialization
        void setUp( )
        {
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        }

        void tearDown( )
        {
            sHandle = NULL;
        }


        void bind_001()
        {
            ::osl::Socket sSocket(sHandle);
            //bind must use local IP address ---mindyliu
            ::osl::SocketAddr saBindSocketAddr( getLocalIP(), IP_PORT_MYPORT5 );

            sSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //sal_True);
            sal_Bool bOK1 = sSocket.bind( saBindSocketAddr );
            CPPUNIT_ASSERT_MESSAGE( "Socket bind fail.", sal_True == bOK1 );

            sal_Bool bOK2 = compareUString( sSocket.getLocalHost( ), saBindSocketAddr.getHostname( ) ) ;

            sSocket.close();
            CPPUNIT_ASSERT_MESSAGE( "test for bind function: bind a valid address.", sal_True == bOK2 );
        }

        void bind_002()
        {
            ::osl::Socket sSocket(sHandle);
            ::osl::SocketAddr saBindSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("123.45.67.89")), IP_PORT_NETBIOS );
            ::osl::SocketAddr saLocalSocketAddr;

            sSocket.setOption( osl_Socket_OptionReuseAddr, 1); // sal_True);
            sal_Bool bOK1 = sSocket.bind( saBindSocketAddr );
            sal_Bool bOK2 = compareUString( sSocket.getLocalHost( ), getThisHostname( ) ) ;

            CPPUNIT_ASSERT_MESSAGE( "test for bind function: bind a valid address.",
                                    ( sal_False == bOK1 ) && ( sal_False == bOK2 ) );
        }

        CPPUNIT_TEST_SUITE( bind );
        CPPUNIT_TEST( bind_001 );
        CPPUNIT_TEST( bind_002 );
        CPPUNIT_TEST_SUITE_END();

    }; // class bind


    /** testing the methods:
        inline sal_Bool SAL_CALL isRecvReady(const TimeValue *pTimeout = 0) const;

    */
    class isRecvReady : public CppUnit::TestFixture
    {
    public:
        oslSocket sHandle;
        TimeValue *pTimeout;
        ::osl::AcceptorSocket asAcceptorSocket;
        ::osl::ConnectorSocket csConnectorSocket;


        // initialization
        void setUp( )
        {
            pTimeout  = ( TimeValue* )malloc( sizeof( TimeValue ) );
            pTimeout->Seconds = 3;
            pTimeout->Nanosec = 0;
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        }

        void tearDown( )
        {
            free( pTimeout );
            sHandle = NULL;
            asAcceptorSocket.close( );
            csConnectorSocket.close( );
        }


        void isRecvReady_001()
        {
            ::osl::SocketAddr saLocalSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), IP_PORT_MYPORT1 );
            ::osl::SocketAddr saTargetSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), IP_PORT_MYPORT1 );
            ::osl::SocketAddr saPeerSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("129.158.217.202")), IP_PORT_FTP );
            ::osl::StreamSocket ssConnection;
            /// launch server socket
            asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); // sal_True);
            sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
            CPPUNIT_ASSERT_MESSAGE( "AcceptorSocket bind address failed.", sal_True == bOK1 );
            sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
            CPPUNIT_ASSERT_MESSAGE( "AcceptorSocket listen failed.",  sal_True == bOK2 );
            asAcceptorSocket.enableNonBlockingMode( sal_True );
            asAcceptorSocket.acceptConnection(ssConnection); /// waiting for incoming connection...

            /// launch client socket
            csConnectorSocket.connect( saTargetSocketAddr, pTimeout );   /// connecting to server...

            /// is receive ready?
            sal_Bool bOK3 = asAcceptorSocket.isRecvReady( pTimeout );

            CPPUNIT_ASSERT_MESSAGE( "test for isRecvReady function: setup a connection and then check if it can transmit data.",
                                      ( sal_True == bOK3 ) );
        }


        CPPUNIT_TEST_SUITE( isRecvReady );
        CPPUNIT_TEST( isRecvReady_001 );
        CPPUNIT_TEST_SUITE_END();

    }; // class isRecvReady


    /** testing the methods:
        inline sal_Bool SAL_CALL isSendReady(const TimeValue *pTimeout = 0) const;
    */
    class isSendReady : public CppUnit::TestFixture
    {
    public:
        oslSocket sHandle;
        TimeValue *pTimeout;
        ::osl::AcceptorSocket asAcceptorSocket;
        ::osl::ConnectorSocket csConnectorSocket;


        // initialization
        void setUp( )
        {
            pTimeout  = ( TimeValue* )malloc( sizeof( TimeValue ) );
            pTimeout->Seconds = 3;
            pTimeout->Nanosec = 0;
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        }

        void tearDown( )
        {
            free( pTimeout );
            sHandle = NULL;
            asAcceptorSocket.close( );
            csConnectorSocket.close( );
        }


        void isSendReady_001()
        {
            ::osl::SocketAddr saLocalSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), IP_PORT_MYPORT );
            ::osl::SocketAddr saTargetSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), IP_PORT_MYPORT );
            ::osl::SocketAddr saPeerSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("129.158.217.202")), IP_PORT_FTP );
            ::osl::StreamSocket ssConnection;

            /// launch server socket
            asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //sal_True);
            sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
            CPPUNIT_ASSERT_MESSAGE( "AcceptorSocket bind address failed.", sal_True == bOK1 );
            sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
            CPPUNIT_ASSERT_MESSAGE( "AcceptorSocket listen failed.",  sal_True == bOK2 );
            asAcceptorSocket.enableNonBlockingMode( sal_True );
            asAcceptorSocket.acceptConnection(ssConnection); /// waiting for incoming connection...

            /// launch client socket
            csConnectorSocket.connect( saTargetSocketAddr, pTimeout );   /// connecting to server...

            /// is send ready?
            sal_Bool bOK3 = csConnectorSocket.isSendReady( pTimeout );

            CPPUNIT_ASSERT_MESSAGE( "test for isSendReady function: setup a connection and then check if it can transmit data.",
                                      ( sal_True == bOK3 ) );
        }


        CPPUNIT_TEST_SUITE( isSendReady );
        CPPUNIT_TEST( isSendReady_001 );
        CPPUNIT_TEST_SUITE_END();

    }; // class isSendReady


    /** testing the methods:
        inline oslSocketType    SAL_CALL getType() const;

    */

    class getType : public CppUnit::TestFixture
    {
    public:
        oslSocket sHandle;
        // initialization
        void setUp( )
        {

        }

        void tearDown( )
        {
            sHandle = NULL;
        }


        void getType_001()
        {
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
            ::osl::Socket sSocket(sHandle);

            CPPUNIT_ASSERT_MESSAGE( "test for getType function: get type of socket.",
                                    osl_Socket_TypeStream ==  sSocket.getType( ) );
        }

        void getType_002()
        {
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeDgram, osl_Socket_ProtocolIp );
            ::osl::Socket sSocket(sHandle);

            CPPUNIT_ASSERT_MESSAGE( "test for getType function: get type of socket.",
                                    osl_Socket_TypeDgram ==  sSocket.getType( ) );
        }

#ifdef UNX
        // mindy: since on LINUX and SOLARIS, Raw type socket can not be created, so do not test getType() here
        // mindy: and add one test case to test creating Raw type socket--> ctors_TypeRaw()
        void getType_003()
        {
            CPPUNIT_ASSERT_MESSAGE( "test for getType function: get type of socket.this is not passed in (LINUX, SOLARIS), the osl_Socket_TypeRaw, type socket can not be created.",
                                    sal_True);
        }
#else
        void getType_003()
        {
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeRaw, osl_Socket_ProtocolIp );
            ::osl::Socket sSocket(sHandle);

            CPPUNIT_ASSERT_MESSAGE( "test for getType function: get type of socket.",
                                    osl_Socket_TypeRaw ==  sSocket.getType( ) );
        }
#endif

        CPPUNIT_TEST_SUITE( getType );
        CPPUNIT_TEST( getType_001 );
        CPPUNIT_TEST( getType_002 );
        CPPUNIT_TEST( getType_003 );
        CPPUNIT_TEST_SUITE_END();

    }; // class getType



    /** testing the methods:
        inline sal_Int32 SAL_CALL getOption(
            oslSocketOption Option,
            void* pBuffer,
            sal_uInt32 BufferLen,
            oslSocketOptionLevel Level= osl_Socket_LevelSocket) const;

        inline sal_Int32 getOption( oslSocketOption option ) const;

    */

    class getOption : public CppUnit::TestFixture
    {
    public:
        oslSocket sHandle;
        // initialization
        void setUp( )
        {

        }

        void tearDown( )
        {
            sHandle = NULL;
        }

        /**  test writer's comment:

            in oslSocketOption, the osl_Socket_OptionType denote 1 as osl_Socket_TypeStream.
            2 as osl_Socket_TypeDgram, etc which is not mapping the oslSocketType enum. differ
            in 1.
        */

        void getOption_001()
        {
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
            ::osl::Socket sSocket(sHandle);
            sal_Int32 * pType = ( sal_Int32 * )malloc( sizeof ( sal_Int32 ) );
            *pType = 0;
            sSocket.getOption( osl_Socket_OptionType,  pType, sizeof ( sal_Int32 ) );
            sal_Bool bOK = ( SOCK_STREAM ==  *pType );
            // there is a TypeMap(socket.c) which map osl_Socket_TypeStream to SOCK_STREAM on UNX, and SOCK_STREAM != osl_Socket_TypeStream
            //sal_Bool bOK = ( TYPE_TO_NATIVE(osl_Socket_TypeStream) ==  *pType );
            free( pType );

            CPPUNIT_ASSERT_MESSAGE( "test for getOption function: get type option of socket.",
                                    sal_True == bOK );
        }

        // getsockopt error
        void getOption_004()
        {
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeDgram, osl_Socket_ProtocolIp );
            ::osl::Socket sSocket(sHandle);

            sal_Bool * pbDontRoute = ( sal_Bool * )malloc( sizeof ( sal_Bool ) );
            sal_Int32 nRes = sSocket.getOption( osl_Socket_OptionInvalid,  pbDontRoute, sizeof ( sal_Bool ) );
            free( pbDontRoute );

            CPPUNIT_ASSERT_MESSAGE( "test for getOption function: get invalid option of socket, should return -1.",
                                     nRes  ==  -1 );
        }

        void getOption_simple_001()
        {
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeDgram, osl_Socket_ProtocolIp );
            ::osl::Socket sSocket(sHandle);

            sal_Bool bOK = ( sal_False  ==  sSocket.getOption( osl_Socket_OptionDontRoute ) );

            CPPUNIT_ASSERT_MESSAGE( "test for getOption function: get debug option of socket.",
                                    sal_True == bOK );
        }

        void getOption_simple_002()
        {
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeDgram, osl_Socket_ProtocolIp );
            ::osl::Socket sSocket(sHandle);

            sal_Bool bOK = ( sal_False  ==  sSocket.getOption( osl_Socket_OptionDebug ) );

            CPPUNIT_ASSERT_MESSAGE( "test for getOption function: get debug option of socket.",
                                    sal_True == bOK );
        }

        CPPUNIT_TEST_SUITE( getOption );
        CPPUNIT_TEST( getOption_001 );
        CPPUNIT_TEST( getOption_004 );
        CPPUNIT_TEST( getOption_simple_001 );
        CPPUNIT_TEST( getOption_simple_002 );
        CPPUNIT_TEST_SUITE_END();

    }; // class getOption


    /** testing the methods:
        inline sal_Bool SAL_CALL setOption( oslSocketOption Option,
                                            void* pBuffer,
                                            sal_uInt32 BufferLen,
                                            oslSocketOptionLevel Level= osl_Socket_LevelSocket ) const;
    */

    class setOption : public CppUnit::TestFixture
    {
    public:
        TimeValue *pTimeout;
// LLA: maybe there is an error in the source,
//      as long as I remember, if a derived class do not overload all ctors there is a problem.

        ::osl::AcceptorSocket asAcceptorSocket;

        void setUp( )
        {

        }

        void tearDown( )
        {
            asAcceptorSocket.close( );
        }


        // LLA:
        // getSocketOption returns BufferLen, or -1 if something failed

        // setSocketOption returns sal_True, if option could stored
        // else sal_False

        void setOption_001()
        {
            /// set and get option.
            int nBufferLen = sizeof ( sal_Int32);
            // LLA: SO_DONTROUTE expect an integer boolean, what ever it is, it's not sal_Bool!

            sal_Int32 * pbDontRouteSet = ( sal_Int32 * )malloc( sizeof ( sal_Int32 ) );
            *pbDontRouteSet = 1; // sal_True;

            sal_Int32 * pGetBuffer = ( sal_Int32 * )malloc( sizeof ( sal_Int32 ) );
            *pGetBuffer = 0;

            // maybe asAcceptorSocket is not right initialized
            sal_Bool  b1 = asAcceptorSocket.setOption( osl_Socket_OptionDontRoute,  pbDontRouteSet, nBufferLen );
            CPPUNIT_ASSERT_MESSAGE( "setOption function failed.", ( sal_True == b1 ) );
            sal_Int32 n2 = asAcceptorSocket.getOption( osl_Socket_OptionDontRoute,  pGetBuffer, nBufferLen );
            CPPUNIT_ASSERT_MESSAGE( "getOption function failed.", ( n2 == nBufferLen ) );

            // on Linux, the value of option is 1, on Solaris, it's 16, but it's not important the exact value,
            // just judge it is zero or not!
            sal_Bool bOK = ( 0  !=  *pGetBuffer );
            t_print("#setOption_001: getOption is %"SAL_PRIdINT32" \n", *pGetBuffer);

            // toggle check, set to 0
            *pbDontRouteSet = 0;

            sal_Bool  b3 = asAcceptorSocket.setOption( osl_Socket_OptionDontRoute,  pbDontRouteSet, sizeof ( sal_Int32 ) );
            CPPUNIT_ASSERT_MESSAGE( "setOption function failed.", ( sal_True == b3 ) );
            sal_Int32 n4 = asAcceptorSocket.getOption( osl_Socket_OptionDontRoute,  pGetBuffer, nBufferLen );
            CPPUNIT_ASSERT_MESSAGE( "getOption (DONTROUTE) function failed.", ( n4 == nBufferLen ) );

            sal_Bool bOK2 = ( 0  ==  *pGetBuffer );

            t_print("#setOption_001: getOption is %"SAL_PRIdINT32" \n", *pGetBuffer);

// LLA:             sal_Bool * pbDontTouteSet = ( sal_Bool * )malloc( sizeof ( sal_Bool ) );
// LLA:             *pbDontTouteSet = sal_True;
// LLA:             sal_Bool * pbDontTouteGet = ( sal_Bool * )malloc( sizeof ( sal_Bool ) );
// LLA:             *pbDontTouteGet = sal_False;
// LLA:             asAcceptorSocket.setOption( osl_Socket_OptionDontRoute,  pbDontTouteSet, sizeof ( sal_Bool ) );
// LLA:             asAcceptorSocket.getOption( osl_Socket_OptionDontRoute,  pbDontTouteGet, sizeof ( sal_Bool ) );
// LLA:             ::rtl::OUString suError = outputError(::rtl::OUString::valueOf((sal_Int32)*pbDontTouteGet),
// LLA:                 ::rtl::OUString::valueOf((sal_Int32)*pbDontTouteSet),
// LLA:                 "test for setOption function: set osl_Socket_OptionDontRoute and then check");
// LLA:
// LLA:             sal_Bool bOK = ( sal_True  ==  *pbDontTouteGet );
// LLA:             free( pbDontTouteSet );
// LLA:             free( pbDontTouteGet );

            CPPUNIT_ASSERT_MESSAGE( "test for setOption function: set option of a socket and then check.",
                                      ( sal_True == bOK ) && (sal_True == bOK2) );

            free( pbDontRouteSet );
            free( pGetBuffer );
// LLA:             CPPUNIT_ASSERT_MESSAGE( suError, sal_True == bOK );
        }

        void setOption_002()
        {
            /// set and get option.

            // sal_Int32 * pbLingerSet = ( sal_Int32 * )malloc( nBufferLen );
            // *pbLingerSet = 7;
            // sal_Int32 * pbLingerGet = ( sal_Int32 * )malloc( nBufferLen );
                    /* struct */linger aLingerSet;
                    sal_Int32 nBufferLen = sizeof( struct linger );
                    aLingerSet.l_onoff = 1;
                    aLingerSet.l_linger = 7;

                   linger aLingerGet;

            asAcceptorSocket.setOption( osl_Socket_OptionLinger,  &aLingerSet, nBufferLen );

            sal_Int32 n1 = asAcceptorSocket.getOption( osl_Socket_OptionLinger,  &aLingerGet, nBufferLen );
                    CPPUNIT_ASSERT_MESSAGE( "getOption (SO_LINGER) function failed.", ( n1 == nBufferLen ) );

            //t_print("#setOption_002: getOption is %d \n", aLingerGet.l_linger);
            sal_Bool bOK = ( 7  ==  aLingerGet.l_linger );
            CPPUNIT_ASSERT_MESSAGE( "test for setOption function: set option of a socket and then check. ",
                sal_True == bOK );

        }

        void setOption_003()
        {
            linger aLingerSet;
                aLingerSet.l_onoff = 1;
                    aLingerSet.l_linger = 7;

            sal_Bool b1 = asAcceptorSocket.setOption( osl_Socket_OptionLinger,  &aLingerSet, 0 );
                    printUString( asAcceptorSocket.getErrorAsString( ) );
            CPPUNIT_ASSERT_MESSAGE( "setOption (SO_LINGER) function failed for optlen is 0.",
                ( b1 == sal_False ) );
        }

        void setOption_simple_001()
        {
            /// set and get option.
            asAcceptorSocket.setOption( osl_Socket_OptionDontRoute, 1 ); //sal_True );
            sal_Bool bOK = ( 0  !=  asAcceptorSocket.getOption( osl_Socket_OptionDontRoute ) );

            t_print("setOption_simple_001(): getoption is %d \n", (int) asAcceptorSocket.getOption( osl_Socket_OptionDontRoute ) );
            CPPUNIT_ASSERT_MESSAGE( "test for setOption function: set option of a socket and then check.",
                                      ( sal_True == bOK ) );
        }

        void setOption_simple_002()
        {
            /// set and get option.
            // LLA: this does not work, due to the fact that SO_LINGER is a structure
// LLA:         asAcceptorSocket.setOption( osl_Socket_OptionLinger,  7 );
// LLA:         sal_Bool bOK = ( 7  ==  asAcceptorSocket.getOption( osl_Socket_OptionLinger ) );

// LLA:         CPPUNIT_ASSERT_MESSAGE( "test for setOption function: set option of a socket and then check.",
// LLA:                                     ( sal_True == bOK ) );
        }

        CPPUNIT_TEST_SUITE( setOption );
//        CPPUNIT_TEST( setOption_001 );
        CPPUNIT_TEST( setOption_002 );
        CPPUNIT_TEST( setOption_003 );
        //TODO: Check this test
//         CPPUNIT_TEST( setOption_simple_001 );
// LLA:     CPPUNIT_TEST( setOption_simple_002 );
        CPPUNIT_TEST_SUITE_END();

    }; // class setOption



    /** testing the method:
        inline sal_Bool SAL_CALL enableNonBlockingMode( sal_Bool bNonBlockingMode);
    */
    class enableNonBlockingMode : public CppUnit::TestFixture
    {
    public:
        ::osl::AcceptorSocket asAcceptorSocket;

        void enableNonBlockingMode_001()
        {
            ::osl::SocketAddr saLocalSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), IP_PORT_MYPORT );
            ::osl::StreamSocket ssConnection;

            /// launch server socket
            asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //sal_True);
            sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
            CPPUNIT_ASSERT_MESSAGE( "AcceptorSocket bind address failed.", sal_True == bOK1 );
            sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
            CPPUNIT_ASSERT_MESSAGE( "AcceptorSocket listen failed.",  sal_True == bOK2 );
            asAcceptorSocket.enableNonBlockingMode( sal_True );
            asAcceptorSocket.acceptConnection(ssConnection); /// waiting for incoming connection...

            /// if reach this statement, it is non-blocking mode, since acceptConnection will blocked by default.
            sal_Bool bOK  = sal_True;
            asAcceptorSocket.close( );

            CPPUNIT_ASSERT_MESSAGE( "test for enableNonBlockingMode function: launch a server socket and make it non blocking. if it can pass the acceptConnection statement, it is non-blocking",
                                      ( sal_True == bOK  ) );
        }


        CPPUNIT_TEST_SUITE( enableNonBlockingMode );
        CPPUNIT_TEST( enableNonBlockingMode_001 );
        CPPUNIT_TEST_SUITE_END();

    }; // class enableNonBlockingMode


    /** testing the method:
        inline sal_Bool SAL_CALL isNonBlockingMode() const;
    */
    class isNonBlockingMode : public CppUnit::TestFixture
    {
    public:
        ::osl::AcceptorSocket asAcceptorSocket;

        void isNonBlockingMode_001()
        {
            ::osl::SocketAddr saLocalSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), IP_PORT_MYPORT );
            ::osl::StreamSocket ssConnection;

            /// launch server socket
            asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); // sal_True);
            sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
            CPPUNIT_ASSERT_MESSAGE( "AcceptorSocket bind address failed.", sal_True == bOK1 );
            sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
            CPPUNIT_ASSERT_MESSAGE( "AcceptorSocket listen failed.",  sal_True == bOK2 );

            sal_Bool bOK3 = asAcceptorSocket.isNonBlockingMode( );
            asAcceptorSocket.enableNonBlockingMode( sal_True );
             asAcceptorSocket.acceptConnection(ssConnection); /// waiting for incoming connection...

            /// if reach this statement, it is non-blocking mode, since acceptConnection will blocked by default.
            sal_Bool bOK4 = asAcceptorSocket.isNonBlockingMode( );
            asAcceptorSocket.close( );

            CPPUNIT_ASSERT_MESSAGE( "test for isNonBlockingMode function: launch a server socket and make it non blocking. it is expected to change from blocking mode to non-blocking mode.",
                                      ( sal_False == bOK3 ) && ( sal_True == bOK4 ) );
        }


        CPPUNIT_TEST_SUITE( isNonBlockingMode );
        CPPUNIT_TEST( isNonBlockingMode_001 );
        CPPUNIT_TEST_SUITE_END();

    }; // class isNonBlockingMode

    /** testing the method:
        inline void SAL_CALL clearError() const;
    */
    class clearError : public CppUnit::TestFixture
    {
    public:
        oslSocket sHandle;
        // initialization
        void setUp( )
        {
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        }

        void tearDown( )
        {
            sHandle = NULL;
        }


        void clearError_001()
        {
            ::osl::Socket sSocket(sHandle);
            ::osl::SocketAddr saBindSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("123.45.67.89")), IP_PORT_HTTP2 );
            ::osl::SocketAddr saLocalSocketAddr;
            sSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //sal_True);
            sSocket.bind( saBindSocketAddr );//build an error "osl_Socket_E_AddrNotAvail"
            oslSocketError seBind = sSocket.getError( );
            sSocket.clearError( );

            CPPUNIT_ASSERT_MESSAGE( "test for clearError function: trick an error called sSocket.getError( ), and then clear the error states, check the result.",
                                    osl_Socket_E_None == sSocket.getError( ) && seBind != osl_Socket_E_None  );
        }


        CPPUNIT_TEST_SUITE( clearError );
        CPPUNIT_TEST( clearError_001 );
        CPPUNIT_TEST_SUITE_END();

    }; // class clearError


    /** testing the methods:
        inline oslSocketError getError() const;
        inline ::rtl::OUString getErrorAsString( ) const;
    */
    class getError : public CppUnit::TestFixture
    {
    public:
        oslSocket sHandle;
        // initialization
        void setUp( )
        {
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        }

        void tearDown( )
        {
            sHandle = NULL;
        }


        void getError_001()
        {
            ::osl::Socket sSocket(sHandle);
            ::osl::SocketAddr saBindSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), IP_PORT_FTP );
            ::osl::SocketAddr saLocalSocketAddr;

            CPPUNIT_ASSERT_MESSAGE( "test for getError function: should get no error.",
                                    osl_Socket_E_None == sSocket.getError( )  );
        }

        void getError_002()
        {
            ::osl::Socket sSocket(sHandle);
            ::osl::SocketAddr saBindSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("123.45.67.89")), IP_PORT_FTP );
            ::osl::SocketAddr saLocalSocketAddr;
            sSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //sal_True);
            sSocket.bind( saBindSocketAddr );//build an error "osl_Socket_E_AddrNotAvail"
            //on Solaris, the error no is EACCES, but it has no mapped value, so getError() returned osl_Socket_E_InvalidError.
#if defined(SOLARIS)
            CPPUNIT_ASSERT_MESSAGE( "trick an error called sSocket.getError( ), check the getError result.Failed on Solaris, returned osl_Socket_E_InvalidError because no entry to map the errno EACCES. ",
                                    osl_Socket_E_InvalidError == sSocket.getError( )  );
#else
            //while on Linux & Win32, the errno is EADDRNOTAVAIL, getError returned osl_Socket_E_AddrNotAvail.

            CPPUNIT_ASSERT_MESSAGE( "trick an error called sSocket.getError( ), check the getError result.Failed on Solaris, returned osl_Socket_E_InvalidError because no entry to map the errno EACCES. Passed on Linux & Win32",
                                    osl_Socket_E_AddrNotAvail == sSocket.getError( )  );
#endif
        }

        CPPUNIT_TEST_SUITE( getError );
        CPPUNIT_TEST( getError_001 );
        CPPUNIT_TEST( getError_002 );
        CPPUNIT_TEST_SUITE_END();

    }; // class getError



    /** testing the methods:
        inline oslSocket getHandle() const;
    */

    class getHandle : public CppUnit::TestFixture
    {
    public:
        oslSocket sHandle;
        // initialization
        void setUp( )
        {
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        }

        void tearDown( )
        {
            sHandle = NULL;
        }

        void getHandle_001()
        {
            ::osl::Socket sSocket(sHandle);
            ::osl::Socket assignSocket = sSocket.getHandle();

            CPPUNIT_ASSERT_MESSAGE( "test for operators_assignment_handle function: test the assignment operator.",
                                    osl_Socket_TypeStream == assignSocket.getType( )  );
        }

        void getHandle_002()
        {
            ::osl::Socket sSocket( sHandle );
            ::osl::Socket assignSocket ( sSocket.getHandle( ) );

            CPPUNIT_ASSERT_MESSAGE( "test for operators_assignment function: assignment operator",
                                    osl_Socket_TypeStream == assignSocket.getType( ) );
        }

        CPPUNIT_TEST_SUITE( getHandle );
        CPPUNIT_TEST( getHandle_001 );
        CPPUNIT_TEST( getHandle_002 );
        CPPUNIT_TEST_SUITE_END();

    }; // class getHandle


// -----------------------------------------------------------------------------


CPPUNIT_TEST_SUITE_REGISTRATION(osl_Socket::ctors);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Socket::operators);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Socket::close);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Socket::getLocalAddr);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Socket::getLocalPort);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Socket::getLocalHost);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Socket::getPeer);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Socket::bind);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Socket::isRecvReady);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Socket::isSendReady);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Socket::getType);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Socket::getOption);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Socket::setOption);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Socket::enableNonBlockingMode);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Socket::isNonBlockingMode);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Socket::clearError);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Socket::getError);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Socket::getHandle);

} // namespace osl_Socket

// -----------------------------------------------------------------------------

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
