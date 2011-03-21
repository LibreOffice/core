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
            t_print("# error: CloseSocketThread not terminated.\n" );
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

    class ctors : public CppUnit::TestFixture
    {
    public:

        void ctors_001()
        {
            /// Socket constructor.
            ::osl::DatagramSocket dsSocket;

            CPPUNIT_ASSERT_MESSAGE( "test for ctors_001 constructor function: check if the datagram socket was created successfully.",
                                    osl_Socket_TypeDgram ==  dsSocket.getType( ) );
        }


        CPPUNIT_TEST_SUITE( ctors );
        CPPUNIT_TEST( ctors_001 );
        CPPUNIT_TEST_SUITE_END();

    }; // class ctors

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
        saTargetSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), IP_PORT_MYPORT9 )
    {
    }

    ~TalkerThread( )
    {
        if ( isRunning( ) )
            t_print("# error: TalkerThread not terminated normally.\n" );
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
        ::osl::SocketAddr saLocalSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), IP_PORT_MYPORT10 );
        dsSocket.setOption( osl_Socket_OptionReuseAddr, 1 );
        if ( dsSocket.bind( saLocalSocketAddr ) == sal_False )
        {
            t_print("DatagramSocket bind failed \n");
            return;
        }
        //blocking mode: default
#if !SILENT_TEST
        sal_Int32 nRecv =
#endif
            dsSocket.recvFrom( pRecvBuffer, 30, &saTargetSocketAddr); //strlen( pTestString2 ) + 1
        t_print("After recvFrom, nRecv is %d\n", (int) nRecv);
    }

    void SAL_CALL onTerminated( )
    {
    }

public:
    sal_Char pRecvBuffer[30];
    ListenerThread( ):
        saTargetSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), IP_PORT_MYPORT10 )
    {
        pRecvBuffer[0] = '\0';
    }

    ~ListenerThread( )
    {
        if ( isRunning( ) )
            t_print("# error: ListenerThread not terminated normally.\n" );
    }

};

    /** testing the methods:
        inline sal_Int32 DatagramSocket::recvFrom(void*  pBuffer, sal_uInt32 BufferSize,
              SocketAddr* pSenderAddr, oslSocketMsgFlag Flag )
        inline sal_Int32  DatagramSocket::sendTo( const SocketAddr& ReceiverAddr,
              const void* pBuffer, sal_uInt32 BufferSize, oslSocketMsgFlag Flag )
    */

    class sendTo_recvFrom : public CppUnit::TestFixture
    {
    public:

        void sr_001()
        {
            ::osl::SocketAddr saLocalSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), IP_PORT_MYPORT9 );
            ::osl::DatagramSocket dsSocket;
            dsSocket.setOption( osl_Socket_OptionReuseAddr, 1 );
            dsSocket.bind( saLocalSocketAddr );

            sal_Char pReadBuffer[30];
            TalkerThread myTalkThread;
            myTalkThread.create();
            sal_Int32 nRecv = dsSocket.recvFrom( pReadBuffer, 30, &saLocalSocketAddr);
            myTalkThread.join();
            //t_print("#received buffer is %s# \n", pReadBuffer);

            sal_Bool bOk = ( strcmp(pReadBuffer, pTestString1) == 0 );

            CPPUNIT_ASSERT_MESSAGE( "test for sendTo/recvFrom function: create a talker thread and recvFrom in the main thread, check if the datagram socket can communicate successfully.",
                                    nRecv > 0 && bOk == sal_True );
        }

        void sr_002()
        {
            ::osl::SocketAddr saListenSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), IP_PORT_MYPORT10 );
            ::osl::DatagramSocket dsSocket;

            //listener thread construct a DatagramSocket, recvFrom waiting for data, then main thread sendto data
            ListenerThread myListenThread;
            myListenThread.create();
            //to grantee the recvFrom is before sendTo
            thread_sleep( 1 );

            sal_Int32 nSend = dsSocket.sendTo( saListenSocketAddr, pTestString2, strlen( pTestString2 ) + 1 );

            CPPUNIT_ASSERT_MESSAGE( "DatagramSocket sendTo failed: nSend <= 0.", nSend > 0);

            myListenThread.join();
            //t_print("#received buffer is %s# \n", myListenThread.pRecvBuffer);

            sal_Bool bOk = ( strcmp( myListenThread.pRecvBuffer, pTestString2) == 0 );

            CPPUNIT_ASSERT_MESSAGE( "test for sendTo/recvFrom function: create a listener thread and sendTo in the main thread, check if the datagram socket can communicate successfully.",
                                    bOk == sal_True );
        }

        //sendTo error, return -1; recvFrom error, return -1
        void sr_003()
        {
            ::osl::SocketAddr saListenSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("123.345.67.89")), IP_PORT_MYPORT10 );
            ::osl::DatagramSocket dsSocket;
            // Transport endpoint is not connected
            sal_Int32 nSend = dsSocket.sendTo( saListenSocketAddr, pTestString2, strlen( pTestString2 ) + 1 );
            CPPUNIT_ASSERT_MESSAGE( "DatagramSocket sendTo should fail: nSend <= 0.",
                nSend == -1 );
        }

        void sr_004()
        {
            ::osl::SocketAddr saListenSocketAddr1( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("123.345.67.89")), IP_PORT_MYPORT10 );
            ::osl::SocketAddr saListenSocketAddr2( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("129.158.217.202")), IP_PORT_MYPORT10 );
            ::osl::DatagramSocket dsSocket;

            dsSocket.enableNonBlockingMode( sal_True );

            sal_Char pReadBuffer[30];
            //sal_Int32 nRecv1 = dsSocket.recvFrom( pReadBuffer, 30, &saListenSocketAddr1 );

            // will block ?
            CloseSocketThread myThread( dsSocket );
            myThread.create();
            sal_Int32 nRecv2 = dsSocket.recvFrom( pReadBuffer, 30, &saListenSocketAddr1 );
            myThread.join();
            //t_print("#nRecv1 is %d nRecv2 is %d\n", nRecv1, nRecv2 );
            CPPUNIT_ASSERT_MESSAGE( "DatagramSocket sendTo should fail: nSend <= 0.",
                 nRecv2 == -1 );
        }

        CPPUNIT_TEST_SUITE( sendTo_recvFrom );
        CPPUNIT_TEST( sr_001 );
        CPPUNIT_TEST( sr_002 );
        CPPUNIT_TEST( sr_003 );
        CPPUNIT_TEST( sr_004 );
        CPPUNIT_TEST_SUITE_END();

    }; // class sendTo_recvFrom

// -----------------------------------------------------------------------------

CPPUNIT_TEST_SUITE_REGISTRATION(osl_DatagramSocket::ctors);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_DatagramSocket::sendTo_recvFrom);

} // namespace osl_DatagramSocket

// -----------------------------------------------------------------------------

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
