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

/**  test coder preface:
     1. the BSD socket function will meet "unresolved external symbol error" on Windows platform
     if you are not including ws2_32.lib in makefile.mk,  the including format will be like this:

     .IF "$(OS)" == "WNT"
     SHL1STDLIBS += $(SOLARLIBDIR)$/cppunit.lib
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

#include "sockethelper.hxx"
#include <osl/conditn.hxx>

using namespace osl;

using ::rtl::OUString;
using ::rtl::OString;

#define IP_PORT_MYPORT9  8897
#define IP_PORT_MYPORT10 18900

const char * pTestString1 = "test socket";
const char * pTestString2 = " Passed#OK";


// helper functions


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

/** Server Socket Thread, served as a temp little server to communicate with client.
 */
class ServerSocketThread : public Thread
{
    osl::Condition    &m_aCondition;
protected:
    oslThreadIdentifier m_id;

    void SAL_CALL run( )
        {
            ::osl::AcceptorSocket asAcceptorSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
            ::osl::SocketAddr saLocalSocketAddr( rtl::OUString("127.0.0.1"), IP_PORT_MYPORT9 );
            ::osl::StreamSocket ssStreamConnection;

            //if has not set this option, socket addr can not be binded in some time(maybe 2 minutes) by another socket
            asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //integer not sal_Bool : sal_True);
            while ( schedule( ) == sal_True )
            {
                sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
                if  ( sal_True != bOK1 )
                {
                    t_print("# ServerSocketThread: AcceptorSocket bind address failed.\n" ) ;
                    break;
                }
                sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
                if  ( sal_True != bOK2 )
                {
                    t_print("# ServerSocketThread: AcceptorSocket listen address failed.\n" ) ;
                    break;
                }

                asAcceptorSocket.enableNonBlockingMode( sal_False );
                m_aCondition.set();

                oslSocketResult eResult = asAcceptorSocket.acceptConnection( ssStreamConnection );
                if (eResult != osl_Socket_Ok )
                {
                    t_print("ServerSocketThread: acceptConnection failed! \n");
                    break;
                }
                sal_Int32 nReadNumber1 = ssStreamConnection.recv( pReadBuffer, 11 );
                sal_Int32 nReadNumber2 = ssStreamConnection.recv( pReadBuffer + nReadNumber1, 11 );
                pReadBuffer[nReadNumber1 + nReadNumber2] = '\0';
                //t_print("# read buffer content: %s\n", pReadBuffer );
                break;
            }
            ssStreamConnection.close();
            asAcceptorSocket.close();

        }

    void SAL_CALL onTerminated( )
        {
            //t_print("# normally terminate this server thread %d!\n",  m_id );
        }

public:
    // public to check if data transmition is OK
    sal_Char pReadBuffer[30];
    ServerSocketThread( osl::Condition &_aCond  ):m_aCondition(_aCond)
        {
            m_aCondition.reset();
            t_print("#init ServerSocketThread\n");
            m_id = getIdentifier( );
            //t_print("# successfully creat this ServerSocketThread %d!\n",  m_id );
        }

    ~ServerSocketThread( )
        {
            if ( isRunning( ) )
                t_print("# error: ServerSocketThread has not terminated.\n" );
        }
};

/** Client Socket Thread, served as a temp little client to communicate with server.
 */
class ClientSocketThread : public Thread
{
protected:
    osl::Condition    &m_aCondition;
    oslThreadIdentifier m_id;
    ::osl::SocketAddr m_saTargetSocketAddr;
    ::osl::ConnectorSocket m_csConnectorSocket;

    void SAL_CALL run( )
        {
            TimeValue *pTimeout;
            pTimeout  = ( TimeValue* )malloc( sizeof( TimeValue ) );
            pTimeout->Seconds = 5;
            pTimeout->Nanosec = 0;

            /// if the thread should terminate, schedule return false
            //while ( schedule( ) == sal_True )
            //{
            if ( osl::Condition::result_ok != m_aCondition.wait( pTimeout ) )
            {
                free( pTimeout );
                return;
            }

            if ( osl_Socket_Ok == m_csConnectorSocket.connect( m_saTargetSocketAddr, pTimeout ))
            {
                m_csConnectorSocket.send( pTestString1, 11 ); // "test socket"
                m_csConnectorSocket.send( pTestString2, 10);
            }
            else
                t_print("# ClientSocketThread: connect failed! \n");
            //  terminate();
            //}
            m_csConnectorSocket.close();
            free( pTimeout );
        }

    void SAL_CALL onTerminated( )
        {
            //t_print("# normally terminate this thread %d!\n",  m_id );
        }

public:
    ClientSocketThread( osl::Condition &_aCond  ):
            m_aCondition(_aCond),
            m_saTargetSocketAddr( rtl::OUString("127.0.0.1"), IP_PORT_MYPORT9 ),
            m_csConnectorSocket( )
        {
            m_id = getIdentifier( );
            //t_print("# successfully creat this client thread %d!\n",  m_id );
        }

    ~ClientSocketThread( )
        {
            if ( isRunning( ) )
                t_print("# error: client thread not terminated.\n" );
        }

};


// Helper functions, to create buffers, check buffers
class ValueCheckProvider
{
    bool m_bFoundFailure;
    char *m_pBuffer;
    sal_Int32 m_nBufferSize;

public:
    ValueCheckProvider()
            :m_bFoundFailure(false),
             m_pBuffer(NULL),
             m_nBufferSize(0)
        {
        }

    bool       isFailure() {return m_bFoundFailure;}

    const char* getBuffer() {return m_pBuffer;}
    char*       getWriteBuffer() {return m_pBuffer;}

    sal_Int32   getBufferSize() {return m_nBufferSize;}

    bool checkValues(sal_Int32 _nLength, int _nValue)
        {
            m_bFoundFailure = false;
            for(sal_Int32 i=0;i<_nLength;i++)
            {
                if (m_pBuffer[i] != _nValue)
                {
                    m_bFoundFailure = true;
                }
            }
            return m_bFoundFailure;
        }

    void createBuffer(sal_Int32 _nLength, int _nValue)
        {
            m_nBufferSize = _nLength;
            m_pBuffer = (char*) malloc(m_nBufferSize);
            if (m_pBuffer)
            {
                memset(m_pBuffer, _nValue, m_nBufferSize);
            }
        }

    void freeBuffer()
        {
            if (m_pBuffer) free(m_pBuffer);
        }

};


/** Client Socket Thread, served as a temp little client to communicate with server.
 */

class ReadSocketThread : public Thread
{
    ValueCheckProvider m_aValues;
    int m_nValue;
    osl::Condition    &m_aCondition;

protected:
    oslThreadIdentifier m_id;

    void SAL_CALL run( )
        {
            ::osl::SocketAddr      m_aTargetSocketAddr( rtl::OUString("127.0.0.1"), IP_PORT_MYPORT10 );
            ::osl::ConnectorSocket m_aConnectorSocket;

            if (! m_aTargetSocketAddr.is())
            {
                t_print("# SocketAddr was NOT created successfully!\n");
            }
            else
            {
                t_print("start ReadSocketThread\n");

                TimeValue *pTimeout;
                pTimeout  = ( TimeValue* )malloc( sizeof( TimeValue ) );
                pTimeout->Seconds = 5;
                pTimeout->Nanosec = 0;

                m_aCondition.wait();

                t_print("connect()\n");

                oslSocketResult eResult = m_aConnectorSocket.connect( m_aTargetSocketAddr, pTimeout );
                if ( osl_Socket_Ok == eResult )
                {
                    sal_Int32 nReadCount = m_aConnectorSocket.read( m_aValues.getWriteBuffer(), m_aValues.getBufferSize() );
                    m_aValues.checkValues(nReadCount, m_nValue);
                }
                else
                {
                    t_print("# ReadSocketThread: connect failed! \n");
                    printSocketResult(eResult);
                }

                //remove this line for deadlock on solaris( margritte.germany )
                m_aConnectorSocket.close();
                free( pTimeout );
            }
        }

    void SAL_CALL onTerminated( )
        {
            //t_print("# normally terminate this thread %d!\n",  m_id );
        }

public:
    sal_Int32 getCount() {return m_aValues.getBufferSize();}
    bool       isOk() {return m_aValues.isFailure() == true ? false : true;}

    ReadSocketThread(sal_Int32 _nBufferSize, int _nValue, osl::Condition &_aCond )
            : m_nValue( _nValue ),
              m_aCondition(_aCond)
        {
            t_print("#init ReadSocketThread\n");
            m_id = getIdentifier( );

            //t_print("# successfully creat this client thread %d!\n",  m_id );
            m_aValues.createBuffer(_nBufferSize, 0);
        }

    ~ReadSocketThread( )
        {
            if ( isRunning( ) )
                t_print("# error: client thread not terminated.\n" );
            m_aValues.freeBuffer();
        }

};

/** Server Socket Thread, write a file which is large
 */
class WriteSocketThread : public Thread
{
    ValueCheckProvider m_aValues;
    osl::Condition    &m_aCondition;

protected:
    oslThreadIdentifier m_id;

    void SAL_CALL run( )
        {
            t_print("start WriteSocketThread\n");
            ::osl::AcceptorSocket asAcceptorSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
            ::osl::SocketAddr saLocalSocketAddr( rtl::OUString("127.0.0.1"), IP_PORT_MYPORT10 );
            if (! saLocalSocketAddr.is())
            {
                t_print("LocalSocketAddr was NOT created successfully!\n");
            }

            ::osl::StreamSocket ssStreamConnection;

            //if has not set this option, socket addr can not be binded in some time(maybe 2 minutes) by another socket
            asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, 1 );    //sal_True);

            /// if the thread should terminate, schedule return false
            // while ( schedule( ) == sal_True )
            // {
            t_print("bind()\n");
            sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
            if  ( sal_True != bOK1 )
            {
                t_print("# WriteSocketThread: AcceptorSocket bind address failed. \n" ) ;
            }
            else
            {
                t_print("listen()\n");
                sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
                if  ( sal_True != bOK2 )
                {
                    t_print("# WriteSocketThread: AcceptorSocket listen address failed. \n" ) ;
                }
                else
                {

                    // blocking mode, if read/recv failed, block until success
                    asAcceptorSocket.enableNonBlockingMode( sal_False);
                    t_print("acceptConnection()\n");
                    m_aCondition.set();

                    oslSocketResult eResult = asAcceptorSocket.acceptConnection( ssStreamConnection );
                    if (eResult != osl_Socket_Ok )
                    {
                        t_print("WriteSocketThread: acceptConnection failed! \n");
                    }
                    else
                    {

                        t_print("write()\n");

                        ssStreamConnection.write( m_aValues.getBuffer(), m_aValues.getBufferSize() );
                        t_print("done written.\n");
                    }
                }
            }
            ssStreamConnection.close();
            asAcceptorSocket.close();
        }

    void SAL_CALL onTerminated( )
        {
            //t_print("# normally terminate this server thread %d!\n",  m_id );
        }

public:
    // public to check if data transmition is OK
    WriteSocketThread(sal_Int32 _nBufferSize, int _nValue, osl::Condition &_aCond )
            : m_aCondition(_aCond)
        {
            m_aCondition.reset();

            t_print("#init WriteSocketThread\n");
            m_id = getIdentifier( );
            //t_print("# successfully creat this server thread %d!\n",  m_id );

            m_aValues.createBuffer(_nBufferSize, _nValue);
        }

    ~WriteSocketThread( )
        {
            if ( isRunning( ) )
                t_print("# error: server thread not terminated.\n" );
            m_aValues.freeBuffer();
        }
};



namespace osl_StreamSocket
{

    /** testing the methods:
        inline StreamSocket(oslAddrFamily Family = osl_Socket_FamilyInet,
        oslProtocol Protocol = osl_Socket_ProtocolIp,
        oslSocketType   Type = osl_Socket_TypeStream);

        inline StreamSocket( const StreamSocket & );

        inline StreamSocket( oslSocket Socket , __sal_NoAcquire noacquire );

        inline StreamSocket( oslSocket Socket );
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
                ::osl::StreamSocket ssSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );

                CPPUNIT_ASSERT_MESSAGE( "test for ctors_none constructor function: check if the stream socket was created successfully.",
                                        osl_Socket_TypeStream ==  ssSocket.getType( ) );
            }

        void ctors_acquire()
            {
                /// Socket constructor.
                ::osl::StreamSocket ssSocket( sHandle );

                CPPUNIT_ASSERT_MESSAGE( "test for ctors_acquire constructor function: check if the socket was created successfully",
                                        osl_Socket_TypeStream == ssSocket.getType( ) );
            }

        void ctors_no_acquire()
            {
                /// Socket constructor.
                ::osl::StreamSocket ssSocket( sHandle, SAL_NO_ACQUIRE );

                CPPUNIT_ASSERT_MESSAGE(" test for ctors_no_acquire constructor function: check if the socket was created successfully",
                                       osl_Socket_TypeStream == ssSocket.getType( ) );
            }

        void ctors_copy_ctor()
            {
                /// Socket constructor.
                ::osl::StreamSocket ssSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
                /// Socket copy constructor.
                ::osl::StreamSocket copySocket( ssSocket );

                CPPUNIT_ASSERT_MESSAGE(" test for ctors_copy_ctor constructor function: create new Socket instance using copy constructor",
                                       osl_Socket_TypeStream == copySocket.getType( ) );
            }

        CPPUNIT_TEST_SUITE( ctors );
        CPPUNIT_TEST( ctors_none );
        CPPUNIT_TEST( ctors_acquire );
        CPPUNIT_TEST( ctors_no_acquire );
        CPPUNIT_TEST( ctors_copy_ctor );
        CPPUNIT_TEST_SUITE_END();

    }; // class ctors

    class send_recv: public CppUnit::TestFixture
    {
    public:
        // initialization
        void setUp( )
            {
            }

        void tearDown( )
            {

            }

        void send_recv1()
            {
                osl::Condition aCondition;
                //client sent two strings, and server received, check the order and value
                ServerSocketThread myServerThread( aCondition );
                ClientSocketThread myClientThread( aCondition );
                myServerThread.create( );
                myClientThread.create( );

                //wait until the thread terminate
                myClientThread.join( );
                myServerThread.join( );
                sal_Char myStr[30] = "";
                strcat( myStr, pTestString1 );
                strcat( myStr, pTestString2 );
                sal_Int32 nRes = strcmp( myServerThread.pReadBuffer, myStr );
                CPPUNIT_ASSERT_MESSAGE(" test for send/recv with two threads: launch Server/Client threads, send data from client, check received data in Server thread.",
                                       nRes == 0 );
            }

        // error when recv
        void send_recv2()
            {
                ::osl::AcceptorSocket asAcceptorSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
                ::osl::SocketAddr saLocalSocketAddr( rtl::OUString("127.0.0.1"), IP_PORT_MYPORT9 );
                ::osl::StreamSocket ssStreamConnection;
                sal_Char pReadBuffer[30] = "";

                osl::Condition aCondition;
                aCondition.reset();
                ClientSocketThread myClientThread( aCondition );
                myClientThread.create( );

                asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, 1 );

                asAcceptorSocket.bind( saLocalSocketAddr );
                asAcceptorSocket.listen( 1 );
                asAcceptorSocket.enableNonBlockingMode( sal_True );
                aCondition.set();

                asAcceptorSocket.acceptConnection( ssStreamConnection );
                sal_Int32 nReadNumber = ssStreamConnection.recv( pReadBuffer, 11 );

                myClientThread.join( ) ;
                ssStreamConnection.close();
                asAcceptorSocket.close();
                CPPUNIT_ASSERT_MESSAGE(" test for send/recv, recv error!", nReadNumber == -1 );
            }

        // LLA: This is a helper function, which create 2 threads, a server and a client.
        // the server writes the buffersize to the client.

        void write_read(sal_Int32 _nBufferSize, int _nValue)
            {
                //client sent two strings, and server received, check the order and value
                osl::Condition aCondition;
                WriteSocketThread myServerThread(_nBufferSize, _nValue, aCondition);
                ReadSocketThread myClientThread(_nBufferSize, _nValue, aCondition);
                myServerThread.create( );
//          thread_sleep( 1 );
                myClientThread.create( );

                //wait until the thread terminate
                myClientThread.join( );
                myServerThread.join( );

                //Maximum Packet Size is ( ARPANET, MILNET = 1007 Ethernet (10Mb) = 1500
                // Proteon PRONET  = 2046), so here test read 4000 bytes
                sal_Int32 nLength = myClientThread.getCount();
                bool       bIsOk   = myClientThread.isOk(); // check if the values are right.

                t_print("Length:=%d\n", (int) nLength);
                t_print(" bIsOk:=%d\n", bIsOk);

                CPPUNIT_ASSERT_MESSAGE(" test for write/read values with two threads: send data from server, check readed data in client.",
                                       nLength == _nBufferSize && bIsOk == true);
            }

        // Tests with different values and sizes
        void write_read_001()
            {
                write_read(50, 10);
            }
        void write_read_002()
            {
                write_read(1024, 20);
            }
        void write_read_003()
            {
                write_read(4000, 1);
            }
        void write_read_004()
            {
                write_read(8192, 3);
            }
        void write_read_005()
            {
                write_read(32768, 3);
            }

        CPPUNIT_TEST_SUITE( send_recv );
        CPPUNIT_TEST( write_read_001 );
        CPPUNIT_TEST( write_read_002 );
        CPPUNIT_TEST( write_read_003 );
        CPPUNIT_TEST( write_read_004 );
        CPPUNIT_TEST( write_read_005 );
        CPPUNIT_TEST( send_recv1 );
        CPPUNIT_TEST( send_recv2 );
//      CPPUNIT_TEST( write_read );
        CPPUNIT_TEST_SUITE_END();
    }; // class send_recv



    class SendClientThread : public Thread
    {
    protected:
        ::osl::SocketAddr m_saTargetSocketAddr;
        ::osl::ConnectorSocket m_csConnectorSocket;
        void SAL_CALL run( )
            {
                TimeValue *pTimeout;
                pTimeout  = ( TimeValue* )malloc( sizeof( TimeValue ) );
                pTimeout->Seconds = 5;
                pTimeout->Nanosec = 0;

                if ( osl_Socket_Ok == m_csConnectorSocket.connect( m_saTargetSocketAddr, pTimeout ))
                {
#if !SILENT_TEST
                    sal_Int32 nWrite1 =
#endif
                        m_csConnectorSocket.write( pTestString1, 11 ); // "test socket"
#if !SILENT_TEST
                    sal_Int32 nWrite2 =
#endif
                        m_csConnectorSocket.write( pTestString2, strlen( pTestString2 ) + 1 );
                    thread_sleep( 2 );
                    m_csConnectorSocket.write( pTestString2, strlen( pTestString2 ) + 1 );
                    t_print("nWrite1 is %d, nWrite2 is %d\n", (int) nWrite1, (int) nWrite2 );
                    //thread_sleep( 1 );
                }
                else
                    t_print("# SendClientThread: connect failed! \n");

                m_csConnectorSocket.close();
                free( pTimeout );
            }
    public:
        SendClientThread(  ):
                m_saTargetSocketAddr( rtl::OUString("127.0.0.1"), IP_PORT_MYPORT9 ),
                m_csConnectorSocket( )
            {
                //t_print("# successfully creat this SendClientThread %d!\n",  m_id );
            }

        ~SendClientThread( )
            {
                if ( isRunning( ) )
                    t_print("# error: SendClientThread has not terminated.\n" );
            }

    };

    class shutdown: public CppUnit::TestFixture
    {
    public:
        // initialization
        void setUp( )
            {
            }

        void tearDown( )
            {

            }

        // similar to close_002
        void shutdown_001()
            {
#if defined(LINUX)
                ::osl::AcceptorSocket asSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
                AcceptorThread myAcceptorThread( asSocket, rtl::OUString("127.0.0.1") );
                myAcceptorThread.create();

                thread_sleep( 1 );

                //when accepting, shutdown the socket, the thread will not block for accepting
                asSocket.shutdown();
                myAcceptorThread.join();

                CPPUNIT_ASSERT_MESSAGE( "test for close when is accepting: the socket will quit accepting status.",
                                        myAcceptorThread.isOK( ) == sal_True );
#endif
            }

        void shutdown_002()
            {
                ::osl::AcceptorSocket asSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
                ::osl::SocketAddr saLocalSocketAddr( rtl::OUString("127.0.0.1"), IP_PORT_MYPORT9);
                asSocket.setOption( osl_Socket_OptionReuseAddr, 1 );
                CPPUNIT_ASSERT_MESSAGE("shutdown_002: bind fail", asSocket.bind( saLocalSocketAddr ) == sal_True);
                CPPUNIT_ASSERT_MESSAGE("shutdown_002: listen fail", asSocket.listen( 1 ) == sal_True );
                sal_Char pReadBuffer[40];
//          osl::Condition aCondition;
                SendClientThread mySendThread;
                mySendThread.create();

                asSocket.enableNonBlockingMode( sal_False );
                ::osl::StreamSocket ssConnectionSocket;
                oslSocketResult eResult = asSocket.acceptConnection( ssConnectionSocket );
                CPPUNIT_ASSERT_MESSAGE("shutdown_002: acceptConnection fail", eResult == osl_Socket_Ok );

                /* set socket option SO_LINGER 0, so close immediately */
                linger aLingerSet;
                sal_Int32 nBufferLen = sizeof( struct linger );
                aLingerSet.l_onoff = 0;
                aLingerSet.l_linger = 0;

                ssConnectionSocket.setOption( osl_Socket_OptionLinger,  &aLingerSet, nBufferLen );
                thread_sleep( 1 );
                //sal_uInt32 nRecv1 = 0;
                sal_Int32 nRead1 = ssConnectionSocket.read( pReadBuffer, 11 );

                //shutdown read after client the first send complete
                ssConnectionSocket.shutdown( osl_Socket_DirRead );

                sal_Int32 nRead2 = ssConnectionSocket.read( pReadBuffer + nRead1, 12 );
                sal_Int32 nRead3 = ssConnectionSocket.read( pReadBuffer + nRead1 + nRead2, 12 );
                t_print("after read 2, nRead1 is %d, nRead2 is %d, nRead3 is %d \n", (int) nRead1, (int) nRead2, (int) nRead3 );
                mySendThread.join();

                ssConnectionSocket.close();
                asSocket.close();

                /* on Linux, if send is before shutdown(DirRead), can read, nRecv2 still > 0,
                   http://dbforums.com/arch/186/2002/12/586417
                   While on Solaris, after shutdown(DirRead), all read will return 0
                */
#ifdef LINUX
                CPPUNIT_ASSERT_MESSAGE( "test for shutdown read direction: the socket can not read(recv).",
                                        nRead1 > 0  && nRead3 == 0 );
#else
                CPPUNIT_ASSERT_MESSAGE( "test for shutdown read direction: the socket can not read(recv).",
                                        nRead1 > 0  && nRead2 == 0 && nRead3 == 0 );
#endif

            }

        void shutdown_003()
            {
                ::osl::AcceptorSocket asSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
                ::osl::SocketAddr saLocalSocketAddr( rtl::OUString("127.0.0.1"), IP_PORT_MYPORT9);
                asSocket.setOption( osl_Socket_OptionReuseAddr, 1 );
                CPPUNIT_ASSERT_MESSAGE("shutdown_002: bind fail", asSocket.bind( saLocalSocketAddr ) == sal_True);
                CPPUNIT_ASSERT_MESSAGE("shutdown_002: listen fail", asSocket.listen( 1 ) == sal_True );
                sal_Char pReadBuffer[40];
                osl::Condition aCondition;
                SendClientThread mySendThread;
                mySendThread.create();

                asSocket.enableNonBlockingMode( sal_False );
                ::osl::StreamSocket ssConnectionSocket;
                oslSocketResult eResult = asSocket.acceptConnection( ssConnectionSocket );
                CPPUNIT_ASSERT_MESSAGE("shutdown_002: acceptConnection fail", eResult == osl_Socket_Ok );

                thread_sleep( 1 );
                //shutdown write after client the first send complete
                ssConnectionSocket.shutdown( osl_Socket_DirWrite );

                // recv should not shutdown
                sal_Int32 nRead1 = ssConnectionSocket.read( pReadBuffer, 11 );

                sal_Int32 nWrite = ssConnectionSocket.write( pReadBuffer, 11 );
                // still can read
                sal_Int32 nRead3 = ssConnectionSocket.read( pReadBuffer + nRead1 , 12 );
                t_print("after read 2, nRead1 is %d, nWrite is %d, nRead3 is %d\n", (int) nRead1, (int) nWrite, (int) nRead3 );
                mySendThread.join();
                ssConnectionSocket.close();
                asSocket.close();

                CPPUNIT_ASSERT_MESSAGE( "test for shutdown read direction: the socket can not send(write).",
                                        nRead1  > 0  && nWrite == 0 && nRead3 > 0);

            }

        CPPUNIT_TEST_SUITE( shutdown );
        CPPUNIT_TEST( shutdown_001 );
        CPPUNIT_TEST( shutdown_002 );
        CPPUNIT_TEST( shutdown_003 );
        CPPUNIT_TEST_SUITE_END();
    }; // class shutdown

    class isExceptionPending: public CppUnit::TestFixture
    {
    public:
        void isExPending_001()
            {
                ::osl::AcceptorSocket asSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
                TimeValue *pTimeout;
                pTimeout  = ( TimeValue* )malloc( sizeof( TimeValue ) );
                pTimeout->Seconds = 3;
                pTimeout->Nanosec = 0;
                sal_Bool bOk = asSocket.isExceptionPending( pTimeout );
                free( pTimeout );

                CPPUNIT_ASSERT_MESSAGE( "test for isExceptionPending.",
                                        bOk == sal_False );
            }

        /**tester's comments: lack of a case that return sal_True, do not know when it will return sal_True*/


        CPPUNIT_TEST_SUITE( isExceptionPending );
        CPPUNIT_TEST( isExPending_001 );
        CPPUNIT_TEST_SUITE_END();
    }; // class isExceptionPending


/** Client Socket Thread, served as a temp little client to communicate with server.
 */

#define IP_PORT_TEST 8900

    class ReadSocket2Thread : public Thread
    {
        osl::Condition &m_aCondition;
        char*        m_pBuffer;
        sal_Int32    m_nBufferSize;
        sal_Int32    m_nReadCount;
        rtl::OString m_sAddr;

        bool         m_bOk;

        void setFailed()
            {
                m_bOk = false;
            }

    protected:
        oslThreadIdentifier m_id;

        void read()
            {
                if (m_sAddr.getLength() == 0)
                {
                    setFailed();
                    return;
                }

                // 10.16.66.252
                ::osl::SocketAddr aSocketAddr( rtl::OUString::createFromAscii(m_sAddr.getStr()), IP_PORT_TEST );
                ::osl::ConnectorSocket aSocket; // ( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );

                aSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //sal_True;

                m_aCondition.wait();
                t_print("wait done\n");

                TimeValue *pTimeout;
                pTimeout  = ( TimeValue* )malloc( sizeof( TimeValue ) );
                pTimeout->Seconds = 20;
                pTimeout->Nanosec = 0;


                // blocking mode, if read/recv failed, block until success
                t_print("enableNonBlockingMode(false)\n");
                aSocket.enableNonBlockingMode( sal_False );


                t_print("connect()\n");
                oslSocketResult eResult = aSocket.connect( aSocketAddr, pTimeout );
                if ( osl_Socket_Ok == eResult)
                {
                    if (m_pBuffer)
                    {
                        t_print("read()\n");
                        m_nReadCount = aSocket.read( m_pBuffer, m_nBufferSize );
                        t_print("%d bytes received.\n", (int) m_nReadCount);
                    }
                }
                else
                {
                    t_print("# ReadSocket2Thread: connect failed! \n");
                    printSocketResult(eResult);
                    setFailed();
                }

                //remove this line for deadlock on solaris( margritte.germany )
                aSocket.close();
                free( pTimeout );
            }

        void SAL_CALL run( )
            {
                read();
            }

        void SAL_CALL onTerminated( )
            {
                //t_print("# normally terminate this thread %d!\n",  m_id );
            }

    public:
        sal_Int32 getCount() {return m_nReadCount;}
        bool       isOk() {return m_nReadCount == 0 ? false : true;}
        bool       getFailed() {return m_bOk == false ? true : false;}

        ReadSocket2Thread(osl::Condition &_aCondition)
                :m_aCondition(_aCondition),
                 m_nReadCount(0),
                 m_bOk( true )
            {
                m_aCondition.reset();
                m_pBuffer = (char*) malloc(1024);
                if (m_pBuffer)
                {
                    m_nBufferSize = 1024;
                }

                m_id = getIdentifier( );
                //t_print("# successfully creat this client thread %d!\n",  m_id );
            }

        void setAddr(rtl::OString const& _sAddr)
            {
                m_sAddr = _sAddr;
            }

        ~ReadSocket2Thread( )
            {
                if ( isRunning( ) )
                    t_print("# error: client thread not terminated.\n" );
                free(m_pBuffer);
            }

    };



    class justtest : public CppUnit::TestFixture
    {
        void send_Acceptor(rtl::OString const& _sAddr, osl::Condition &)
            {
                ::osl::AcceptorSocket aSocket; // ( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
                ::osl::SocketAddr aSocketAddr;

                if (! aSocketAddr.setPort(IP_PORT_TEST))
                {
                    t_print("# can not set port\n" );
                }

                if (! aSocketAddr.setHostname(rtl::OUString::createFromAscii(_sAddr.getStr())))
                {
                    t_print("# can not set hostname/ip\n" );
                }

                rtl::OUString aHostname = aSocketAddr.getHostname();
                aSocketAddr.getPort();


                //if has not set this option, socket addr can not be binded in some time(maybe 2 minutes) by another socket
                aSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //sal_True);

                /// if the thread should terminate, schedule return false
                // while ( schedule( ) == sal_True )
                // {
                if (! aSocket.bind( aSocketAddr ))
                {
                    t_print("# can't bind.\n" );
                }
                if (! aSocket.listen( ))
                {
                    t_print("# can't listen. \n" );
                }

                // blocking mode, if read/recv failed, block until success
                aSocket.enableNonBlockingMode( sal_False);
                ::osl::StreamSocket ssStreamConnection;

                oslSocketResult eResult = aSocket.acceptConnection( ssStreamConnection );
                if (eResult != osl_Socket_Ok )
                {
                    t_print("WriteSocketThread: acceptConnection failed! \n");
                    // break;
                }
                char const * pBuffer = "Test String\n";
                sal_Int32 nBufferSize = strlen(pBuffer);
                ssStreamConnection.write( pBuffer, nBufferSize );
                // break;
                // }

                // ssStreamConnection.close();
                aSocket.close();
            }



        void send_Connector(rtl::OString const& _sAddr, osl::Condition &/*_aCondition*/ )
            {
                ::osl::ConnectorSocket aSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
                ::osl::SocketAddr aSocketAddr( rtl::OUString::createFromAscii(_sAddr.getStr()), IP_PORT_TEST );

                if (! aSocketAddr.is())
                {
                    t_print("is failed.\n");
                    return;
                }

                //if has not set this option, socket addr can not be binded in some time(maybe 2 minutes) by another socket
                aSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //sal_True;

                oslSocketResult aResult = aSocket.connect( aSocketAddr );
                if  ( aResult != osl_Socket_Ok )
                {
                    t_print("# send_Connector: connect failed. \n" );
                }
                else
                {
                    // blocking mode, if read/recv failed, block until success
//                    aSocket.enableNonBlockingMode( sal_False );

//                     _aCondition.set();

                    ::osl::StreamSocket ssStreamConnection(aSocket);

                    char const * pBuffer = "GET / HTTP/1.0\015\012\015\012";
                    sal_Int32 nBufferSize = strlen(pBuffer);
                    ssStreamConnection.write( pBuffer, nBufferSize );

                    char *pBufferPeek = (char*) malloc(1024);
                    sal_Int32 nReadNumber = ssStreamConnection.recv( pBufferPeek, 1024, osl_Socket_MsgPeek);
                    free(pBufferPeek);

                    char *pBuffer2 = (char*) malloc(nReadNumber + 1);
                    sal_Int32 nReadNumberReal = ssStreamConnection.read( pBuffer2, nReadNumber );
                    pBuffer2[nReadNumberReal] = '\0';

                    t_print("received: %s\n", pBuffer2);

                    rtl::OUString suError = ssStreamConnection.getErrorAsString();
                    free(pBuffer2);
                    // ssStreamConnection.close();

                    // ssStreamConnection.close();
                }
                aSocket.shutdown(osl_Socket_DirReadWrite);
                aSocket.close();
            }


    public:
        // LLA: send_Connector_2_margritte works, it send strings to echo server on margritte
        //      but can not receive anything

        void send_Connector_2_margritte(rtl::OString const& _sAddr)
            {
                ::osl::ConnectorSocket aSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
                ::osl::SocketAddr aSocketAddr( rtl::OUString::createFromAscii(_sAddr.getStr()), IP_PORT_TEST );

                //if has not set this option, socket addr can not be binded in some time(maybe 2 minutes) by another socket
                aSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //sal_True;

                oslSocketResult aResult = aSocket.connect( aSocketAddr );
                if  ( aResult != osl_Socket_Ok )
                {
                    t_print("# connect failed. \n" );
                }
                else
                {
                    // blocking mode, if read/recv failed, block until success
                    aSocket.enableNonBlockingMode( sal_False );

                    ::osl::StreamSocket ssStreamConnection(aSocket);

                    char const * pBuffer = "Test String\n";
                    sal_Int32 nBufferSize = strlen(pBuffer);
                    ssStreamConnection.write( pBuffer, nBufferSize );

                    char const * pBuffer3 = "quit\n";
                    nBufferSize = strlen(pBuffer3);
                    ssStreamConnection.write( pBuffer3, nBufferSize );

                    ssStreamConnection.close();
                }
                aSocket.close();
            }

        void send_recv_2_margritte()
            {
                rtl::OString sAddr;
                sAddr = "margritte.germany.sun.com";
                if ( ifAvailable(rtl::OUString::createFromAscii(sAddr.getStr())) == sal_True )
                {
                    t_print("found %s!\n", sAddr.getStr());
                }
                send_Connector_2_margritte(sAddr);
            }



        void send_recv()
            {
                rtl::OString sAddr;
                // if ( ifAvailable(rtl::OUString("margritte.germany")) == sal_True )
                // {
                //     t_print("margritte is alive ! \n");
                //     sAddr = "margritte.germany";
                // }

                sAddr = "margritte.germany.sun.com";
                if ( ifAvailable(rtl::OUString::createFromAscii(sAddr.getStr())) == sal_True )
                {
                    t_print("found %s!\n", sAddr.getStr());
                }
//                 else
//                 {
//                     if ( ifAvailable(rtl::OUString("192.168.7.2")) == sal_True )
//                     {
//                         sAddr = "192.168.7.2";
//                         t_print("moon found ! \n");
//                     }
//                     else
//                     {
//                         if ( ifAvailable(rtl::OUString("moon.linux.bogus")) == sal_True )
//                         {
//                             sAddr = "moon.linux.bogus";
//                             t_print("moon found ! \n");
//                         }
//                         else
//                         {
//                             if ( ifAvailable(rtl::OUString("moon")) == sal_True )
//                             {
//                                 sAddr = "moon";
//                                 t_print("moon found ! \n");
//                             }
//                         }
//                     }
//                 }

                // if ( ifAvailable(rtl::OUString("10.16.64.196")) == sal_False )
                // {
                //     t_print("ip 10.16.64.196 is not alive! \n");
                //     return;
                // }

                osl::Condition aCondition;
                ReadSocket2Thread myReadThread(aCondition);
                myReadThread.setAddr(sAddr);
//                myReadThread.create();

                thread_sleep( 2 );
                if (! myReadThread.getFailed())
                {
                    // send_Acceptor(sAddr, aCondition);
                    send_Connector(sAddr, aCondition);

                    thread_sleep( 2 );
                    if (myReadThread.isRunning())
                    {
                        myReadThread.join();
                    }
                    // termAndJoinThread(&myReadThread);

                    // statistics
#if !SILENT_TEST
                    sal_uInt32 nLength =
#endif
                        myReadThread.getCount();

#if !SILENT_TEST
                    bool       bIsOk   =
#endif
                        myReadThread.isOk(); // check if the values are right.

                    t_print("Length:=%d\n", (int) nLength);
                    t_print(" bIsOk:=%d\n", bIsOk);
                }
                else
                {
                    t_print("ERROR: No echo Server on %s found.\n", sAddr.getStr());
                }
            }


        void getPage(rtl::OString const& _sAddr);
        void test_getPage()
            {
                // rtl::OString sPage("lla-1.germany.sun.com");
                // getPage(sPage);

                rtl::OString sPage("lla-1");
                getPage(sPage);
            }

        CPPUNIT_TEST_SUITE( justtest );
        CPPUNIT_TEST( send_recv );
        CPPUNIT_TEST( test_getPage );
        CPPUNIT_TEST_SUITE_END();
    }; // class isExceptionPending


    void justtest::getPage(rtl::OString const& _sAddr)
            {
                rtl::OUString suAddr = rtl::OUString::createFromAscii(_sAddr.getStr());
                ::osl::ConnectorSocket aSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
                ::osl::SocketAddr aSocketAddr( suAddr, 80 );

            {
                // some checks
                aSocketAddr.getPort();
                oslSocketResult aResult;
                rtl::ByteSequence aSeq = aSocketAddr.getAddr(&aResult);
                if (aResult != osl_Socket_Ok)
                {
                    t_print("problem with getAddr: ");
                    printSocketResult(aResult);
                }

                rtl::OUString sStr = aSocketAddr.getHostname(&aResult);
                if (aResult != osl_Socket_Ok)
                {
                    t_print("problem with hostname: ");
                    printSocketResult(aResult);
                }
            }

                oslSocketResult aResult;

                // SocketAddr::resolveHostname(suAddr, aSocketAddr);
                // if (! aSocketAddr.is())
                // {
                //     t_print("Can't resolve Hostname.\n");
                //     return;
                // }
                // rtl::OUString sStr = aSocketAddr.getHostname(&aResult);
                // if (aResult != osl_Socket_Ok)
                // {
                //     t_print("problem with hostname: ");
                //     printSocketResult(aResult);
                //
                // }

                if (! aSocketAddr.is())
                {
                    t_print("SocketAddr::is() failed.\n");
                    return;
                }

                //if has not set this option, socket addr can not be binded in some time(maybe 2 minutes) by another socket
                aSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //sal_True;

                aResult = aSocket.connect( aSocketAddr );
                if  ( aResult != osl_Socket_Ok )
                {
                    t_print("# send_Connector: connect failed. \n" );
                }
                else
                {
                    // blocking mode, if read/recv failed, block until success
//                    aSocket.enableNonBlockingMode( sal_False );

//                     _aCondition.set();

                    ::osl::StreamSocket ssStreamConnection(aSocket);

                    char const * pBuffer = "GET / HTTP/1.0\015\012\015\012";
                    sal_Int32 nBufferSize = strlen(pBuffer);
                    ssStreamConnection.write( pBuffer, nBufferSize );


                    char *pBufferPeek = (char*) malloc(1024);
                    sal_Int32 nReadNumber = 1;
                    while ( nReadNumber != 0)
                    {
                        nReadNumber = ssStreamConnection.recv( pBufferPeek, 1024, osl_Socket_MsgPeek);
                        if (nReadNumber > 0)
                        {
                            char *pBuffer2 = (char*) malloc(nReadNumber + 1);
                            sal_Int32 nReadNumberReal = ssStreamConnection.read( pBuffer2, nReadNumber );
                            pBuffer2[nReadNumberReal] = '\0';
                            t_print("%s", pBuffer2);
                            free(pBuffer2);
                        }
                    }
                    free(pBufferPeek);

                    rtl::OUString suError = ssStreamConnection.getErrorAsString();
                }
                aSocket.shutdown(osl_Socket_DirReadWrite);
                aSocket.close();
            }



    CPPUNIT_TEST_SUITE_REGISTRATION(osl_StreamSocket::ctors);
    CPPUNIT_TEST_SUITE_REGISTRATION(osl_StreamSocket::send_recv);
//    CPPUNIT_TEST_SUITE_REGISTRATION(osl_StreamSocket::shutdown);
    CPPUNIT_TEST_SUITE_REGISTRATION(osl_StreamSocket::isExceptionPending);

//    CPPUNIT_TEST_SUITE_REGISTRATION(osl_StreamSocket::justtest);

} // namespace osl_StreamSocket



// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
