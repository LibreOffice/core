/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

//------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------

#include "gtest/gtest.h"

//#include "osl_Socket_Const.h"
#include "sockethelper.hxx"

using namespace osl;
using namespace rtl;

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
            printf("# AcceptorSocket bind address failed.\n" ) ;
            return;
        }
        sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
        if  ( sal_True != bOK2 )
        {
            printf("# AcceptorSocket listen address failed.\n" ) ;
            return;
        }

        asAcceptorSocket.enableNonBlockingMode( sal_False );

        oslSocketResult eResult = asAcceptorSocket.acceptConnection( ssStreamConnection );
        if (eResult != osl_Socket_Ok )
        {
            bOK = sal_True;
            printf("AcceptorThread: acceptConnection failed! \n");
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
            printf("# error: Acceptor thread not terminated.\n" );
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


    class ctors : public ::testing::Test
    {
    public:
        oslSocket sHandle;
        // initialization
        void SetUp( )
        {
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        }

        void TearDown( )
        {
            sHandle = NULL;
        }
    }; // class ctors

    TEST_F(ctors, ctors_none)
    {
        /// Socket constructor.
        // ::osl::Socket sSocket();

        ASSERT_TRUE(1 == 1) << "test for ctors_none constructor function: check if the socket was created successfully, if no exception occurred";
    }

    TEST_F(ctors, ctors_acquire)
    {
        /// Socket constructor.
        ::osl::Socket sSocket( sHandle );

        ASSERT_TRUE(osl_Socket_TypeStream == sSocket.getType( )) << "test for ctors_acquire constructor function: check if the socket was created successfully";
    }

    TEST_F(ctors, ctors_no_acquire)
    {
        /// Socket constructor.
        ::osl::Socket sSocket( sHandle, SAL_NO_ACQUIRE );

        ASSERT_TRUE(osl_Socket_TypeStream == sSocket.getType( )) << " test for ctors_no_acquire constructor function: check if the socket was created successfully";
    }

    TEST_F(ctors, ctors_copy_ctor)
    {
        ::osl::Socket sSocket( sHandle );
        /// Socket copy constructor.
        ::osl::Socket copySocket( sSocket );

        ASSERT_TRUE(osl_Socket_TypeStream == copySocket.getType( )) << " test for ctors_copy_ctor constructor function: create new Socket instance using copy constructor";
    }

    TEST_F(ctors, ctors_TypeRaw)
    {
#ifdef WNT
        oslSocket sHandleRaw = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeRaw, osl_Socket_ProtocolIp );
// LLA: ?           ::osl::Socket sSocket( sHandleRaw );
        ASSERT_TRUE(sHandleRaw != NULL) << " type osl_Socket_TypeRaw socket create failed on UNX ";
#else
        oslSocket sHandleRaw = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeRaw, osl_Socket_ProtocolIp );
        ASSERT_TRUE(sHandleRaw == NULL) << " can't create socket with type osl_Socket_TypeRaw within UNX is ok.";
#endif
    }

    TEST_F(ctors, ctors_family_Ipx)
    {
        oslSocket sHandleIpx = osl_createSocket( osl_Socket_FamilyIpx, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        ASSERT_TRUE(sHandleIpx != NULL) << " family osl_Socket_FamilyIpx socket create failed! ";
        ::osl::Socket sSocket( sHandleIpx );        //, SAL_NO_ACQUIRE );
        printf("#Type is %d \n", sSocket.getType( ) );

        ASSERT_TRUE(osl_Socket_TypeStream == sSocket.getType( )) << " test for create new Socket instance that family is osl_Socket_FamilyIpx";
    }

    /** testing the methods:
        inline Socket& SAL_CALL operator= ( oslSocket socketHandle);
        inline Socket& SAL_CALL operator= (const Socket& sock);
        inline sal_Bool SAL_CALL operator==( const Socket& rSocket ) const ;
        inline sal_Bool SAL_CALL operator==( const oslSocket socketHandle ) const;
    */

    class operators : public ::testing::Test
    {
    public:
        oslSocket sHandle;
        // initialization
        void SetUp( )
        {
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        }

        void TearDown( )
        {
            sHandle = NULL;
        }
    }; // class operators

    /**  test writer's comment:

        the assignment operator does not support direct assinment like:
        ::osl::Socket sSocket = sHandle.
    */
    TEST_F(operators, operators_assignment_handle)
    {
        ::osl::Socket sSocket(sHandle);
        ::osl::Socket assignSocket = sSocket.getHandle();

        ASSERT_TRUE(osl_Socket_TypeStream == assignSocket.getType( )) << "test for operators_assignment_handle function: test the assignment operator.";
    }

    TEST_F(operators, operators_assignment)
    {
        ::osl::Socket sSocket( sHandle );
        ::osl::Socket assignSocket = sSocket;

        ASSERT_TRUE(osl_Socket_TypeStream == assignSocket.getType( )) << "test for operators_assignment function: assignment operator";
    }

    TEST_F(operators, operators_equal_handle_001)
    {
        /// Socket constructor.
        ::osl::Socket sSocket( sHandle );
        ::osl::Socket equalSocket = sSocket;

        ASSERT_TRUE(equalSocket == sHandle) << " test for operators_equal_handle_001 function: check equal.";
    }

    TEST_F(operators, operators_equal_handle_002)
    {
        /// Socket constructor.
        ::osl::Socket equalSocket( osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeDgram, osl_Socket_ProtocolIp ) );

        ASSERT_TRUE(!( equalSocket == sHandle )) << " test for operators_equal_handle_001 function: check unequal.";
    }

    TEST_F(operators, operators_equal_001)
    {
        ::osl::Socket sSocket( sHandle );
        /// Socket copy constructor.
        ::osl::Socket equalSocket( sSocket );

        ASSERT_TRUE(equalSocket == sSocket) << " test for operators_equal function: check equal.";
    }

    TEST_F(operators, operators_equal_002)
    {
        ::osl::Socket sSocket( sHandle );
        /// Socket copy constructor.
        ::osl::Socket equalSocket( osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeDgram, osl_Socket_ProtocolIp ) );

        ASSERT_TRUE(!( equalSocket == sSocket )) << " test for operators_equal_002 function: check unequal.";
    }

    /** testing the methods:
        inline void SAL_CALL shutdown( oslSocketDirection Direction = osl_Socket_DirReadWrite );
        inline void SAL_CALL close();
    */

    class close : public ::testing::Test
    {
    public:
        oslSocket sHandle;
        // initialization
        void SetUp( )
        {
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        }

        void TearDown( )
        {
            sHandle = NULL;
        }
    }; // class close

    TEST_F(close, close_001)
    {
        ::osl::Socket sSocket(sHandle);
        sSocket.close();

        ASSERT_TRUE(sSocket.getHandle() == sHandle) << "test for close_001 function: this function is reserved for test.";
    }

    TEST_F(close, close_002)
    {
        // This blocks forever on FreeBSD
#if defined(LINUX)
        ::osl::AcceptorSocket asSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
        AcceptorThread myAcceptorThread( asSocket, rtl::OUString::createFromAscii("127.0.0.1") );
        myAcceptorThread.create();

        thread_sleep( 1 );
        //when accepting, close the socket, the thread will not block for accepting
        //man close:Any locks held on the file it was associated with, and owned by the process, are removed
        asSocket.close();
        //thread_sleep( 2 );
        myAcceptorThread.join();

        ASSERT_TRUE(myAcceptorThread.isOK() == sal_True) << "test for close when is accepting: the socket will quit accepting status.";
#endif
    }

    // to cover "if ( pSockAddrIn->sin_addr.s_addr == htonl(INADDR_ANY) )" in osl_closeSocket( )
    TEST_F(close, close_003)
    {
        // This blocks forever on FreeBSD
#if defined(LINUX)
        ::osl::AcceptorSocket asSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
        AcceptorThread myAcceptorThread( asSocket, rtl::OUString::createFromAscii("0.0.0.0") );
        myAcceptorThread.create();

        thread_sleep( 1 );
        asSocket.close();
        myAcceptorThread.join();

        ASSERT_TRUE(myAcceptorThread.isOK() == sal_True) << "test for close when is accepting: the socket will quit accepting status.";
#endif
    }

    /** testing the method:
        inline void SAL_CALL getLocalAddr( SocketAddr &Addr ) const;
    */

    class getLocalAddr : public ::testing::Test
    {
    public:
        oslSocket sHandle;
        // initialization
        void SetUp( )
        {
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        }

        void TearDown( )
        {
            sHandle = NULL;
        }
    }; // class getLocalAddr

    // get the Address of the local end of the socket
    TEST_F(getLocalAddr, getLocalAddr_001)
    {
        ::osl::Socket sSocket(sHandle);
        ::osl::SocketAddr saBindSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT8 );
        ::osl::SocketAddr saLocalSocketAddr;

        sSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //sal_True);

        sal_Bool bOK1 = sSocket.bind( saBindSocketAddr );
        ::rtl::OUString suError1 = ::rtl::OUString::createFromAscii("Socket bind fail:") + sSocket.getErrorAsString();
        ASSERT_TRUE(sal_True == bOK1) << suError1.pData;

        sSocket.getLocalAddr( saLocalSocketAddr );

        sal_Bool bOK = compareUString( saLocalSocketAddr.getHostname( 0 ), sSocket.getLocalHost() ) ;

        ASSERT_TRUE(sal_True == bOK) << "test for getLocalAddr function: first create a new socket, then a socket address, bind them, and check the address.";
    }

    /** testing the method:
        inline sal_Int32    SAL_CALL getLocalPort() const;
    */

    class getLocalPort : public ::testing::Test
    {
    public:
        oslSocket sHandle;
        // initialization
        void SetUp( )
        {
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        }

        void TearDown( )
        {
            sHandle = NULL;
        }
    }; // class getLocalPort

    TEST_F(getLocalPort, getLocalPort_001)
    {
        ::osl::Socket sSocket(sHandle);
        ::osl::SocketAddr saBindSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT7 );  // aHostIp1 localhost
        ::osl::SocketAddr saLocalSocketAddr;

        sSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //sal_True);

        sal_Bool bOK1 = sSocket.bind( saBindSocketAddr );
        ::rtl::OUString suError1 = ::rtl::OUString::createFromAscii("Socket bind fail:") + sSocket.getErrorAsString();
        ASSERT_TRUE(sal_True == bOK1) << suError1.pData;
        sal_Bool bOK = ( IP_PORT_MYPORT7 == sSocket.getLocalPort( )  );

        ASSERT_TRUE(sal_True == bOK) << "test for getLocalPort function: first create a new socket, then a socket address, bind them, and check the port.";
    }

/**  test writer's comment:

    the invalid port number can not be set by giving invalid port number
    such as 99999 or -1, it will convert to ( x mod 65535 ), so it will always be
    valid,  the only instance that the getLocalPort returns OSL_INVALID_PORT
    is when saSocketAddr itself is an invalid one, that is , the IP or host name
    can not be found, then the created socket address is not valid.
*/
    TEST_F(getLocalPort, getLocalPort_002)
    {
        ::osl::SocketAddr saBindSocketAddr( rtl::OUString::createFromAscii("123.45.67.89"), IP_PORT_TELNET);
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
        ::rtl::OUString suError = ::rtl::OUString::createFromAscii( "on Unix, if Addr is not an address of type osl_Socket_FamilyInet, it returns OSL_INVALID_PORT, but can not create Addr of that case");
#endif
        ASSERT_TRUE(sal_False) << suError.pData;

    }

    TEST_F(getLocalPort, getLocalPort_003)
    {
        ::osl::Socket sSocket(sHandle);
        ::osl::SocketAddr saBindSocketAddr( getLocalIP(), IP_PORT_INVAL);

        sSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //sal_True);

        sal_Bool bOK1 = sSocket.bind( saBindSocketAddr );
        ::rtl::OUString suError1 = ::rtl::OUString::createFromAscii("Socket bind fail:") + sSocket.getErrorAsString();
        ASSERT_TRUE(sal_True == bOK1) << suError1.pData;
        ::rtl::OUString suError = outputError(::rtl::OUString::valueOf(sSocket.getLocalPort( )),
            ::rtl::OUString::createFromAscii("34463"),
            "test for getLocalPort function: first create a new socket, then an invalid socket address, bind them, and check the port assigned");
        sal_Bool bOK = ( sSocket.getLocalPort( ) >= 1 &&  sSocket.getLocalPort( ) <= 65535);

        ASSERT_TRUE(sal_True == bOK) << suError.pData;
    }

    /** testing the method:
        inline ::rtl::OUString SAL_CALL getLocalHost() const;

        Mindyliu: on Linux, at first it will check the binded in /etc/hosts, if it has the binded IP, it will return the hostname in it;
        else if the binded IP is "127.0.0.1", it will return "localhost", if it's the machine's ethernet ip such as "129.158.217.90", it
        will return hostname of current processor such as "aegean.PRC.Sun.COM"
    */

    class getLocalHost : public ::testing::Test
    {
    public:
        oslSocket sHandle;
        // initialization
        void SetUp( )
        {
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        }

        void TearDown( )
        {
            sHandle = NULL;
        }
    }; // class getLocalHost

    TEST_F(getLocalHost, getLocalHost_001)
    {
        ::osl::Socket sSocket(sHandle);
        //port number from IP_PORT_HTTP1 to IP_PORT_MYPORT6, mindyliu
        ::osl::SocketAddr saBindSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT6 );

        sSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //sal_True);

        sal_Bool bOK1 = sSocket.bind( saBindSocketAddr );
        ::rtl::OUString suError1 = ::rtl::OUString::createFromAscii("Socket bind fail:") + sSocket.getErrorAsString();
        ASSERT_TRUE(sal_True == bOK1) << suError1.pData;
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
        ASSERT_TRUE(sal_True == bOK) << suError.pData;
    }

    TEST_F(getLocalHost, getLocalHost_002)
    {
        ::osl::Socket sSocket(sHandle);
        ::osl::SocketAddr saBindSocketAddr( rtl::OUString::createFromAscii("123.45.67.89"), IP_PORT_POP3);
        ::osl::SocketAddr saLocalSocketAddr;

        sSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //sal_True);
        sSocket.bind( saBindSocketAddr );
        //Invalid IP, so bind should fail
        sal_Bool bOK = compareUString( sSocket.getLocalHost( ), rtl::OUString::createFromAscii("") ) ;
        ::rtl::OUString suError = outputError(sSocket.getLocalHost( ), rtl::OUString::createFromAscii(""), "test for getLocalHost function: getLocalHost with invalid SocketAddr");

        ASSERT_TRUE(sal_True == bOK) << suError.pData;
    }

    /** testing the methods:
        inline void SAL_CALL getPeerAddr( SocketAddr & Addr) const;
        inline sal_Int32    SAL_CALL getPeerPort() const;
        inline ::rtl::OUString SAL_CALL getPeerHost() const;
    */
    class getPeer : public ::testing::Test
    {
    public:
        oslSocket sHandle;
        TimeValue *pTimeout;
        ::osl::AcceptorSocket asAcceptorSocket;
        ::osl::ConnectorSocket csConnectorSocket;


        // initialization
        void SetUp( )
        {
            pTimeout  = ( TimeValue* )malloc( sizeof( TimeValue ) );
            pTimeout->Seconds = 3;
            pTimeout->Nanosec = 0;
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        }

        void TearDown( )
        {
            free( pTimeout );
            sHandle = NULL;
            asAcceptorSocket.close( );
            csConnectorSocket.close( );
        }
    }; // class getPeer

    TEST_F(getPeer, getPeer_001)
    {
        ::osl::SocketAddr saLocalSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT );
        ::osl::SocketAddr saTargetSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT );
        ::osl::SocketAddr saPeerSocketAddr( rtl::OUString::createFromAscii("129.158.217.202"), IP_PORT_FTP );
        ::osl::StreamSocket ssConnection;
        asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //sal_True);
        /// launch server socket
        sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
        ASSERT_TRUE(sal_True == bOK1) << "AcceptorSocket bind '127.0.0.1' address failed.";
        sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
        ASSERT_TRUE(sal_True == bOK2) << "AcceptorSocket listen failed.";

        asAcceptorSocket.enableNonBlockingMode( sal_True );
        asAcceptorSocket.acceptConnection(ssConnection); /// waiting for incoming connection...

        /// launch client socket
        csConnectorSocket.connect( saTargetSocketAddr, pTimeout );   /// connecting to server...

        /// get peer information
        csConnectorSocket.getPeerAddr( saPeerSocketAddr );/// connected.
        sal_Int32 peerPort = csConnectorSocket.getPeerPort( );
        ::rtl::OUString peerHost = csConnectorSocket.getPeerHost( );

        ASSERT_TRUE(( sal_True == compareSocketAddr( saPeerSocketAddr, saLocalSocketAddr ) )&&
                                ( sal_True == compareUString( peerHost, saLocalSocketAddr.getHostname( 0 ) ) ) &&
                                ( peerPort == saLocalSocketAddr.getPort( ) )) << "test for getPeer function: setup a connection and then get the peer address, port and host from client side.";
    }

    /** testing the methods:
        inline sal_Bool SAL_CALL bind(const SocketAddr& LocalInterface);
    */


    class bind : public ::testing::Test
    {
    public:
        oslSocket sHandle;
        // initialization
        void SetUp( )
        {
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        }

        void TearDown( )
        {
            sHandle = NULL;
        }
    }; // class bind

    TEST_F(bind, bind_001)
    {
        ::osl::Socket sSocket(sHandle);
        //bind must use local IP address ---mindyliu
        ::osl::SocketAddr saBindSocketAddr( getLocalIP(), IP_PORT_MYPORT5 );

        sSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //sal_True);
        sal_Bool bOK1 = sSocket.bind( saBindSocketAddr );
        ASSERT_TRUE(sal_True == bOK1) << "Socket bind fail.";

        sal_Bool bOK2 = compareUString( sSocket.getLocalHost( ), saBindSocketAddr.getHostname( ) ) ;

        sSocket.close();
        ASSERT_TRUE(sal_True == bOK2) << "test for bind function: bind a valid address.";
    }

    TEST_F(bind, bind_002)
    {
        ::osl::Socket sSocket(sHandle);
        ::osl::SocketAddr saBindSocketAddr( rtl::OUString::createFromAscii("123.45.67.89"), IP_PORT_NETBIOS );
        ::osl::SocketAddr saLocalSocketAddr;

        sSocket.setOption( osl_Socket_OptionReuseAddr, 1); // sal_True);
        sal_Bool bOK1 = sSocket.bind( saBindSocketAddr );
        sal_Bool bOK2 = compareUString( sSocket.getLocalHost( ), getThisHostname( ) ) ;

        ASSERT_TRUE(( sal_False == bOK1 ) && ( sal_False == bOK2 )) << "test for bind function: bind a valid address.";
    }

    /** testing the methods:
        inline sal_Bool SAL_CALL isRecvReady(const TimeValue *pTimeout = 0) const;

    */
    class isRecvReady : public ::testing::Test
    {
    public:
        oslSocket sHandle;
        TimeValue *pTimeout;
        ::osl::AcceptorSocket asAcceptorSocket;
        ::osl::ConnectorSocket csConnectorSocket;


        // initialization
        void SetUp( )
        {
            pTimeout  = ( TimeValue* )malloc( sizeof( TimeValue ) );
            pTimeout->Seconds = 3;
            pTimeout->Nanosec = 0;
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        }

        void TearDown( )
        {
            free( pTimeout );
            sHandle = NULL;
            asAcceptorSocket.close( );
            csConnectorSocket.close( );
        }
    }; // class isRecvReady

    TEST_F(isRecvReady, isRecvReady_001)
    {
        ::osl::SocketAddr saLocalSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT1 );
        ::osl::SocketAddr saTargetSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT1 );
        ::osl::SocketAddr saPeerSocketAddr( rtl::OUString::createFromAscii("129.158.217.202"), IP_PORT_FTP );
        ::osl::StreamSocket ssConnection;
        /// launch server socket
        asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); // sal_True);
        sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
        ASSERT_TRUE(sal_True == bOK1) << "AcceptorSocket bind address failed.";
        sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
        ASSERT_TRUE(sal_True == bOK2) << "AcceptorSocket listen failed.";
        asAcceptorSocket.enableNonBlockingMode( sal_True );
        asAcceptorSocket.acceptConnection(ssConnection); /// waiting for incoming connection...

        /// launch client socket
        csConnectorSocket.connect( saTargetSocketAddr, pTimeout );   /// connecting to server...

        /// is receive ready?
        sal_Bool bOK3 = asAcceptorSocket.isRecvReady( pTimeout );

        ASSERT_TRUE(( sal_True == bOK3 )) << "test for isRecvReady function: setup a connection and then check if it can transmit data.";
    }

    /** testing the methods:
        inline sal_Bool SAL_CALL isSendReady(const TimeValue *pTimeout = 0) const;
    */
    class isSendReady : public ::testing::Test
    {
    public:
        oslSocket sHandle;
        TimeValue *pTimeout;
        ::osl::AcceptorSocket asAcceptorSocket;
        ::osl::ConnectorSocket csConnectorSocket;


        // initialization
        void SetUp( )
        {
            pTimeout  = ( TimeValue* )malloc( sizeof( TimeValue ) );
            pTimeout->Seconds = 3;
            pTimeout->Nanosec = 0;
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        }

        void TearDown( )
        {
            free( pTimeout );
            sHandle = NULL;
            asAcceptorSocket.close( );
            csConnectorSocket.close( );
        }
    }; // class isSendReady

    TEST_F(isSendReady, isSendReady_001)
    {
        ::osl::SocketAddr saLocalSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT );
        ::osl::SocketAddr saTargetSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT );
        ::osl::SocketAddr saPeerSocketAddr( rtl::OUString::createFromAscii("129.158.217.202"), IP_PORT_FTP );
        ::osl::StreamSocket ssConnection;

        /// launch server socket
        asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //sal_True);
        sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
        ASSERT_TRUE(sal_True == bOK1) << "AcceptorSocket bind address failed.";
        sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
        ASSERT_TRUE(sal_True == bOK2) << "AcceptorSocket listen failed.";
        asAcceptorSocket.enableNonBlockingMode( sal_True );
        asAcceptorSocket.acceptConnection(ssConnection); /// waiting for incoming connection...

        /// launch client socket
        csConnectorSocket.connect( saTargetSocketAddr, pTimeout );   /// connecting to server...

        /// is send ready?
        sal_Bool bOK3 = csConnectorSocket.isSendReady( pTimeout );

        ASSERT_TRUE(( sal_True == bOK3 )) << "test for isSendReady function: setup a connection and then check if it can transmit data.";
    }

    /** testing the methods:
        inline oslSocketType    SAL_CALL getType() const;

    */

    class getType : public ::testing::Test
    {
    public:
        oslSocket sHandle;
        // initialization
        void SetUp( )
        {

        }

        void TearDown( )
        {
            sHandle = NULL;
        }
    }; // class getType

    TEST_F(getType, getType_001)
    {
        sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        ::osl::Socket sSocket(sHandle);

        ASSERT_TRUE(osl_Socket_TypeStream ==  sSocket.getType( )) << "test for getType function: get type of socket.";
    }

    TEST_F(getType, getType_002)
    {
        sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeDgram, osl_Socket_ProtocolIp );
        ::osl::Socket sSocket(sHandle);

        ASSERT_TRUE(osl_Socket_TypeDgram ==  sSocket.getType( )) << "test for getType function: get type of socket.";
    }

#ifdef UNX
    // mindy: since on LINUX and SOLARIS, Raw type socket can not be created, so do not test getType() here
    // mindy: and add one test case to test creating Raw type socket--> ctors_TypeRaw()
    TEST_F(getType, getType_003)
    {
        ASSERT_TRUE(sal_True) << "test for getType function: get type of socket.this is not passed in (LINUX, SOLARIS), the osl_Socket_TypeRaw, type socket can not be created.";
    }
#else
    TEST_F(getType, getType_003)
    {
        sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeRaw, osl_Socket_ProtocolIp );
        ::osl::Socket sSocket(sHandle);

        ASSERT_TRUE(osl_Socket_TypeRaw ==  sSocket.getType( )) << "test for getType function: get type of socket.";
    }
#endif


    /** testing the methods:
        inline sal_Int32 SAL_CALL getOption(
            oslSocketOption Option,
            void* pBuffer,
            sal_uInt32 BufferLen,
            oslSocketOptionLevel Level= osl_Socket_LevelSocket) const;

        inline sal_Int32 getOption( oslSocketOption option ) const;

    */

    class getOption : public ::testing::Test
    {
    public:
        oslSocket sHandle;
        // initialization
        void SetUp( )
        {

        }

        void TearDown( )
        {
            sHandle = NULL;
        }
    }; // class getOption


    /**  test writer's comment:

        in oslSocketOption, the osl_Socket_OptionType denote 1 as osl_Socket_TypeStream.
        2 as osl_Socket_TypeDgram, etc which is not mapping the oslSocketType enum. differ
        in 1.
    */

    TEST_F(getOption, getOption_001)
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

        ASSERT_TRUE(sal_True == bOK) << "test for getOption function: get type option of socket.";
    }

    // getsockopt error
    TEST_F(getOption, getOption_004)
    {
        sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeDgram, osl_Socket_ProtocolIp );
        ::osl::Socket sSocket(sHandle);

        sal_Bool * pbDontRoute = ( sal_Bool * )malloc( sizeof ( sal_Bool ) );
        sal_Int32 nRes = sSocket.getOption( osl_Socket_OptionInvalid,  pbDontRoute, sizeof ( sal_Bool ) );
        free( pbDontRoute );

        ASSERT_TRUE(nRes  ==  -1) << "test for getOption function: get invalid option of socket, should return -1.";
    }

    TEST_F(getOption, getOption_simple_001)
    {
        sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeDgram, osl_Socket_ProtocolIp );
        ::osl::Socket sSocket(sHandle);

        sal_Bool bOK = ( sal_False  ==  sSocket.getOption( osl_Socket_OptionDontRoute ) );

        ASSERT_TRUE(sal_True == bOK) << "test for getOption function: get debug option of socket.";
    }

    TEST_F(getOption, getOption_simple_002)
    {
        sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeDgram, osl_Socket_ProtocolIp );
        ::osl::Socket sSocket(sHandle);

        sal_Bool bOK = ( sal_False  ==  sSocket.getOption( osl_Socket_OptionDebug ) );

        ASSERT_TRUE(sal_True == bOK) << "test for getOption function: get debug option of socket.";
    }

    /** testing the methods:
        inline sal_Bool SAL_CALL setOption( oslSocketOption Option,
                                            void* pBuffer,
                                            sal_uInt32 BufferLen,
                                            oslSocketOptionLevel Level= osl_Socket_LevelSocket ) const;
    */

    class setOption : public ::testing::Test
    {
    public:
        TimeValue *pTimeout;
// LLA: maybe there is an error in the source,
//      as long as I remember, if a derived class do not overload all ctors there is a problem.

        ::osl::AcceptorSocket asAcceptorSocket;

        void SetUp( )
        {

        }

        void TearDown( )
        {
            asAcceptorSocket.close( );
        }
    }; // class setOption

    // LLA:
    // getSocketOption returns BufferLen, or -1 if something failed

    // setSocketOption returns sal_True, if option could stored
    // else sal_False

    TEST_F(setOption, setOption_001)
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
        ASSERT_TRUE(( sal_True == b1 )) << "setOption function failed.";
        sal_Int32 n2 = asAcceptorSocket.getOption( osl_Socket_OptionDontRoute,  pGetBuffer, nBufferLen );
        ASSERT_TRUE(( n2 == nBufferLen )) << "getOption function failed.";

        // on Linux, the value of option is 1, on Solaris, it's 16, but it's not important the exact value,
        // just judge it is zero or not!
        sal_Bool bOK = ( 0  !=  *pGetBuffer );
        printf("#setOption_001: getOption is %d \n", *pGetBuffer);

        // toggle check, set to 0
        *pbDontRouteSet = 0;

        sal_Bool  b3 = asAcceptorSocket.setOption( osl_Socket_OptionDontRoute,  pbDontRouteSet, sizeof ( sal_Int32 ) );
        ASSERT_TRUE(( sal_True == b3 )) << "setOption function failed.";
        sal_Int32 n4 = asAcceptorSocket.getOption( osl_Socket_OptionDontRoute,  pGetBuffer, nBufferLen );
        ASSERT_TRUE(( n4 == nBufferLen )) << "getOption (DONTROUTE) function failed.";

        sal_Bool bOK2 = ( 0  ==  *pGetBuffer );

        printf("#setOption_001: getOption is %d \n", *pGetBuffer);

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

        ASSERT_TRUE(( sal_True == bOK ) && (sal_True == bOK2)) << "test for setOption function: set option of a socket and then check.";

        free( pbDontRouteSet );
        free( pGetBuffer );
// LLA:             ASSERT_TRUE(sal_True == bOK) << suError;
    }

    TEST_F(setOption, setOption_002)
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
                ASSERT_TRUE(( n1 == nBufferLen )) << "getOption (SO_LINGER) function failed.";

        //printf("#setOption_002: getOption is %d \n", aLingerGet.l_linger);
        sal_Bool bOK = ( 7  ==  aLingerGet.l_linger );
        ASSERT_TRUE(sal_True == bOK) << "test for setOption function: set option of a socket and then check. ";

    }

    TEST_F(setOption, setOption_003)
    {
        linger aLingerSet;
            aLingerSet.l_onoff = 1;
                aLingerSet.l_linger = 7;

        sal_Bool b1 = asAcceptorSocket.setOption( osl_Socket_OptionLinger,  &aLingerSet, 0 );
                printUString( asAcceptorSocket.getErrorAsString( ) );
        ASSERT_TRUE(( b1 == sal_False )) << "setOption (SO_LINGER) function failed for optlen is 0.";
    }

    TEST_F(setOption, setOption_simple_001)
    {
        /// set and get option.
        asAcceptorSocket.setOption( osl_Socket_OptionDontRoute, 1 ); //sal_True );
        sal_Bool bOK = ( 0  !=  asAcceptorSocket.getOption( osl_Socket_OptionDontRoute ) );

        printf("setOption_simple_001(): getoption is %d \n", asAcceptorSocket.getOption( osl_Socket_OptionDontRoute ) );
        ASSERT_TRUE(( sal_True == bOK )) << "test for setOption function: set option of a socket and then check.";
    }

    TEST_F(setOption, setOption_simple_002)
    {
        /// set and get option.
        // LLA: this does not work, due to the fact that SO_LINGER is a structure
// LLA:         asAcceptorSocket.setOption( osl_Socket_OptionLinger,  7 );
// LLA:         sal_Bool bOK = ( 7  ==  asAcceptorSocket.getOption( osl_Socket_OptionLinger ) );

// LLA:         ASSERT_TRUE(// LLA:                                     ( sal_True == bOK )) << "test for setOption function: set option of a socket and then check.";
    }


    /** testing the method:
        inline sal_Bool SAL_CALL enableNonBlockingMode( sal_Bool bNonBlockingMode);
    */
    class enableNonBlockingMode : public ::testing::Test
    {
    public:
        ::osl::AcceptorSocket asAcceptorSocket;
    }; // class enableNonBlockingMode

    TEST_F(enableNonBlockingMode, enableNonBlockingMode_001)
    {
        ::osl::SocketAddr saLocalSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT );
        ::osl::StreamSocket ssConnection;

        /// launch server socket
        asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //sal_True);
        sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
        ASSERT_TRUE(sal_True == bOK1) << "AcceptorSocket bind address failed.";
        sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
        ASSERT_TRUE(sal_True == bOK2) << "AcceptorSocket listen failed.";
        asAcceptorSocket.enableNonBlockingMode( sal_True );
        asAcceptorSocket.acceptConnection(ssConnection); /// waiting for incoming connection...

        /// if reach this statement, it is non-blocking mode, since acceptConnection will blocked by default.
        sal_Bool bOK  = sal_True;
        asAcceptorSocket.close( );

        ASSERT_TRUE(( sal_True == bOK  )) << "test for enableNonBlockingMode function: launch a server socket and make it non blocking. if it can pass the acceptConnection statement, it is non-blocking";
    }


    /** testing the method:
        inline sal_Bool SAL_CALL isNonBlockingMode() const;
    */
    class isNonBlockingMode : public ::testing::Test
    {
    public:
        ::osl::AcceptorSocket asAcceptorSocket;
    }; // class isNonBlockingMode

    TEST_F(isNonBlockingMode, isNonBlockingMode_001)
    {
        ::osl::SocketAddr saLocalSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT );
        ::osl::StreamSocket ssConnection;

        /// launch server socket
        asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); // sal_True);
        sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
        ASSERT_TRUE(sal_True == bOK1) << "AcceptorSocket bind address failed.";
        sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
        ASSERT_TRUE(sal_True == bOK2) << "AcceptorSocket listen failed.";

        sal_Bool bOK3 = asAcceptorSocket.isNonBlockingMode( );
        asAcceptorSocket.enableNonBlockingMode( sal_True );
            asAcceptorSocket.acceptConnection(ssConnection); /// waiting for incoming connection...

        /// if reach this statement, it is non-blocking mode, since acceptConnection will blocked by default.
        sal_Bool bOK4 = asAcceptorSocket.isNonBlockingMode( );
        asAcceptorSocket.close( );

        ASSERT_TRUE(( sal_False == bOK3 ) && ( sal_True == bOK4 )) << "test for isNonBlockingMode function: launch a server socket and make it non blocking. it is expected to change from blocking mode to non-blocking mode.";
    }

    /** testing the method:
        inline void SAL_CALL clearError() const;
    */
    class clearError : public ::testing::Test
    {
    public:
        oslSocket sHandle;
        // initialization
        void SetUp( )
        {
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        }

        void TearDown( )
        {
            sHandle = NULL;
        }
    }; // class clearError

    TEST_F(clearError, clearError_001)
    {
        ::osl::Socket sSocket(sHandle);
        ::osl::SocketAddr saBindSocketAddr( rtl::OUString::createFromAscii("123.45.67.89"), IP_PORT_HTTP2 );
        ::osl::SocketAddr saLocalSocketAddr;
        sSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //sal_True);
        sSocket.bind( saBindSocketAddr );//build an error "osl_Socket_E_AddrNotAvail"
        oslSocketError seBind = sSocket.getError( );
        sSocket.clearError( );

        ASSERT_TRUE(osl_Socket_E_None == sSocket.getError( ) && seBind != osl_Socket_E_None) << "test for clearError function: trick an error called sSocket.getError( ), and then clear the error states, check the result.";
    }

    /** testing the methods:
        inline oslSocketError getError() const;
        inline ::rtl::OUString getErrorAsString( ) const;
    */
    class getError : public ::testing::Test
    {
    public:
        oslSocket sHandle;
        // initialization
        void SetUp( )
        {
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        }

        void TearDown( )
        {
            sHandle = NULL;
        }
    }; // class getError

    TEST_F(getError, getError_001)
    {
        ::osl::Socket sSocket(sHandle);
        ::osl::SocketAddr saBindSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_FTP );
        ::osl::SocketAddr saLocalSocketAddr;

        ASSERT_TRUE(osl_Socket_E_None == sSocket.getError( )) << "test for getError function: should get no error.";
    }

    TEST_F(getError, getError_002)
    {
        ::osl::Socket sSocket(sHandle);
        ::osl::SocketAddr saBindSocketAddr( rtl::OUString::createFromAscii("123.45.67.89"), IP_PORT_FTP );
        ::osl::SocketAddr saLocalSocketAddr;
        sSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //sal_True);
        sSocket.bind( saBindSocketAddr );//build an error "osl_Socket_E_AddrNotAvail"
        //on Solaris, the error no is EACCES, but it has no mapped value, so getError() returned osl_Socket_E_InvalidError.
#if defined(SOLARIS)
        ASSERT_TRUE(osl_Socket_E_InvalidError == sSocket.getError( )) << "trick an error called sSocket.getError( ), check the getError result.Failed on Solaris, returned osl_Socket_E_InvalidError because no entry to map the errno EACCES. ";
#else
        //while on Linux & Win32, the errno is EADDRNOTAVAIL, getError returned osl_Socket_E_AddrNotAvail.

        ASSERT_TRUE(osl_Socket_E_AddrNotAvail == sSocket.getError( )) << "trick an error called sSocket.getError( ), check the getError result.Failed on Solaris, returned osl_Socket_E_InvalidError because no entry to map the errno EACCES. Passed on Linux & Win32";
#endif
    }


    /** testing the methods:
        inline oslSocket getHandle() const;
    */

    class getHandle : public ::testing::Test
    {
    public:
        oslSocket sHandle;
        // initialization
        void SetUp( )
        {
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        }

        void TearDown( )
        {
            sHandle = NULL;
        }
    }; // class getHandle

    TEST_F(getHandle, getHandle_001)
    {
        ::osl::Socket sSocket(sHandle);
        ::osl::Socket assignSocket = sSocket.getHandle();

        ASSERT_TRUE(osl_Socket_TypeStream == assignSocket.getType( )) << "test for operators_assignment_handle function: test the assignment operator.";
    }

    TEST_F(getHandle, getHandle_002)
    {
        ::osl::Socket sSocket( sHandle );
        ::osl::Socket assignSocket ( sSocket.getHandle( ) );

        ASSERT_TRUE(osl_Socket_TypeStream == assignSocket.getType( )) << "test for operators_assignment function: assignment operator";
    }


} // namespace osl_Socket

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
