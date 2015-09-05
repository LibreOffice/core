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

#define IP_PORT_MYPORT9  8897
#define IP_PORT_MYPORT10 8898

const char * pTestString1 = "test socket";
const char * pTestString2 = " Passed#OK";

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

class CloseSocketThread : public Thread
{
    ::osl::Socket m_sSocket;
protected:
    void SAL_CALL run( )
    {
        thread_sleep( 1 );
        m_sSocket.close( );
    }
public:
    CloseSocketThread(::osl::Socket & sSocket )
        : m_sSocket( sSocket )
    {
    }

    ~CloseSocketThread( )
    {
        if ( isRunning( ) )
        {
            printf("# error: CloseSocketThread not terminated.\n" );
        }
    }
};

//------------------------------------------------------------------------
// tests cases begins here
//------------------------------------------------------------------------

namespace osl_DatagramSocket
{

    /** testing the methods:
        inline DatagramSocket(oslAddrFamily Family= osl_Socket_FamilyInet,
                              oslProtocol   Protocol= osl_Socket_ProtocolIp,
                              oslSocketType Type= osl_Socket_TypeDgram);
    */

    class ctors : public ::testing::Test
    {
    public:
    }; // class ctors

    TEST_F(ctors, ctors_001)
    {
        /// Socket constructor.
        ::osl::DatagramSocket dsSocket;

        ASSERT_TRUE(osl_Socket_TypeDgram ==  dsSocket.getType( ))
            << "test for ctors_001 constructor function: check if the datagram socket was created successfully.";
    }

/**thread do sendTo, refer to http://www.coding-zone.co.uk/cpp/articles/140101networkprogrammingv.shtml
*/
class TalkerThread : public Thread
{
protected:
    ::osl::SocketAddr saTargetSocketAddr;
    ::osl::DatagramSocket dsSocket;

    void SAL_CALL run( )
    {
        dsSocket.sendTo( saTargetSocketAddr, pTestString1, strlen( pTestString1 ) + 1 ); // "test socket"
        dsSocket.shutdown();
    }

    void SAL_CALL onTerminated( )
    {
    }

public:
    TalkerThread( ):
        saTargetSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT9 )
    {
    }

    ~TalkerThread( )
    {
        if ( isRunning( ) )
            printf("# error: TalkerThread not terminated normally.\n" );
    }
};

/**thread do listen, refer to http://www.coding-zone.co.uk/cpp/articles/140101networkprogrammingv.shtml
*/
class ListenerThread : public Thread
{
protected:
    ::osl::SocketAddr saTargetSocketAddr;
    ::osl::DatagramSocket dsSocket;

    void SAL_CALL run( )
    {
        ::osl::SocketAddr saLocalSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT10 );
        dsSocket.setOption( osl_Socket_OptionReuseAddr, 1 );
        if ( dsSocket.bind( saLocalSocketAddr ) == sal_False )
        {
            printf("DatagramSocket bind failed \n");
            return;
        }
        //blocking mode: default
        sal_Int32 nRecv = dsSocket.recvFrom( pRecvBuffer, 30, &saTargetSocketAddr); //strlen( pTestString2 ) + 1
        printf("After recvFrom, nRecv is %d\n", nRecv);
    }

    void SAL_CALL onTerminated( )
    {
    }

public:
    sal_Char pRecvBuffer[30];
    ListenerThread( ):
        saTargetSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT10 )
    {
        pRecvBuffer[0] = '\0';
    }

    ~ListenerThread( )
    {
        if ( isRunning( ) )
            printf("# error: ListenerThread not terminated normally.\n" );
    }

};

    /** testing the methods:
        inline sal_Int32 DatagramSocket::recvFrom(void*  pBuffer, sal_uInt32 BufferSize,
              SocketAddr* pSenderAddr, oslSocketMsgFlag Flag )
        inline sal_Int32  DatagramSocket::sendTo( const SocketAddr& ReceiverAddr,
              const void* pBuffer, sal_uInt32 BufferSize, oslSocketMsgFlag Flag )
    */

    class sendTo_recvFrom : public ::testing::Test
    {
    public:
    }; // class sendTo_recvFrom

    TEST_F(sendTo_recvFrom, sr_001)
    {
        ::osl::SocketAddr saLocalSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT9 );
        ::osl::DatagramSocket dsSocket;
        dsSocket.setOption( osl_Socket_OptionReuseAddr, 1 );
        dsSocket.bind( saLocalSocketAddr );

        sal_Char pReadBuffer[30];
        TalkerThread myTalkThread;
        myTalkThread.create();
        sal_Int32 nRecv = dsSocket.recvFrom( pReadBuffer, 30, &saLocalSocketAddr);
        myTalkThread.join();
        //printf("#received buffer is %s# \n", pReadBuffer);

        sal_Bool bOk = ( strcmp(pReadBuffer, pTestString1) == 0 );

        ASSERT_TRUE(nRecv > 0 && bOk == sal_True )
            << "test for sendTo/recvFrom function: create a talker thread and recvFrom in the main thread, check if the datagram socket can communicate successfully.";
    }

    TEST_F(sendTo_recvFrom, sr_002)
    {
        ::osl::SocketAddr saListenSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT10 );
        ::osl::DatagramSocket dsSocket;

        //listener thread construct a DatagramSocket, recvFrom waiting for data, then main thread sendto data
        ListenerThread myListenThread;
        myListenThread.create();
        //to grantee the recvFrom is before sendTo
        thread_sleep( 1 );

        sal_Int32 nSend = dsSocket.sendTo( saListenSocketAddr, pTestString2, strlen( pTestString2 ) + 1 );

        ASSERT_TRUE(nSend > 0) << "DatagramSocket sendTo failed: nSend <= 0.";

        myListenThread.join();
        //printf("#received buffer is %s# \n", myListenThread.pRecvBuffer);

        sal_Bool bOk = ( strcmp( myListenThread.pRecvBuffer, pTestString2) == 0 );

        ASSERT_TRUE( bOk == sal_True )
            << "test for sendTo/recvFrom function: create a listener thread and sendTo in the main thread, check if the datagram socket can communicate successfully.";
    }

    //sendTo error, return -1; recvFrom error, return -1
    TEST_F(sendTo_recvFrom, sr_003)
    {
        ::osl::SocketAddr saListenSocketAddr( rtl::OUString::createFromAscii("123.345.67.89"), IP_PORT_MYPORT10 );
        ::osl::DatagramSocket dsSocket;
        // Transport endpoint is not connected
        sal_Int32 nSend = dsSocket.sendTo( saListenSocketAddr, pTestString2, strlen( pTestString2 ) + 1 );
        ASSERT_TRUE(nSend == -1) << "DatagramSocket sendTo should fail: nSend <= 0.";
    }

    TEST_F(sendTo_recvFrom, sr_004)
    {
        ::osl::SocketAddr saListenSocketAddr1( rtl::OUString::createFromAscii("123.345.67.89"), IP_PORT_MYPORT10 );
        ::osl::SocketAddr saListenSocketAddr2( rtl::OUString::createFromAscii("129.158.217.202"), IP_PORT_MYPORT10 );
        ::osl::DatagramSocket dsSocket;

        dsSocket.enableNonBlockingMode( sal_True );

        sal_Char pReadBuffer[30];
        //sal_Int32 nRecv1 = dsSocket.recvFrom( pReadBuffer, 30, &saListenSocketAddr1 );

        // will block ?
        CloseSocketThread myThread( dsSocket );
        myThread.create();
        sal_Int32 nRecv2 = dsSocket.recvFrom( pReadBuffer, 30, &saListenSocketAddr1 );
        myThread.join();
        //printf("#nRecv1 is %d nRecv2 is %d\n", nRecv1, nRecv2 );
        ASSERT_TRUE(nRecv2 == -1) << "DatagramSocket sendTo should fail: nSend <= 0.";
    }

} // namespace osl_DatagramSocket

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
