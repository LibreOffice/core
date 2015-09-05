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
     SHL1STDLIBS += $(SOLARLIBDIR)$/cppunit.lib
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


//#include "osl_Socket_Const.h"
#include "sockethelper.hxx"
#include <osl/conditn.hxx>
#include <osl/signal.h>
#include "gtest/gtest.h"

using namespace osl;
using namespace rtl;

#define IP_PORT_MYPORT9  8897
#define IP_PORT_MYPORT10 18900

const char * pTestString1 = "test socket";
const char * pTestString2 = " Passed#OK";

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
            ::osl::SocketAddr saLocalSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT9 );
            ::osl::StreamSocket ssStreamConnection;

            //if has not set this option, socket addr can not be binded in some time(maybe 2 minutes) by another socket
            asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //integer not sal_Bool : sal_True);
            while ( schedule( ) == sal_True )
            {
                sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
                if  ( sal_True != bOK1 )
                {
                    printf("# ServerSocketThread: AcceptorSocket bind address failed.\n" ) ;
                    break;
                }
                sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
                if  ( sal_True != bOK2 )
                {
                    printf("# ServerSocketThread: AcceptorSocket listen address failed.\n" ) ;
                    break;
                }

                asAcceptorSocket.enableNonBlockingMode( sal_False );
                m_aCondition.set();

                oslSocketResult eResult = asAcceptorSocket.acceptConnection( ssStreamConnection );
                if (eResult != osl_Socket_Ok )
                {
                    printf("ServerSocketThread: acceptConnection failed! \n");
                    break;
                }
                sal_Int32 nReadNumber1 = ssStreamConnection.recv( pReadBuffer, 11 );
                sal_Int32 nReadNumber2 = ssStreamConnection.recv( pReadBuffer + nReadNumber1, 11 );
                pReadBuffer[nReadNumber1 + nReadNumber2] = '\0';
                //printf("# read buffer content: %s\n", pReadBuffer );
                break;
            }
            ssStreamConnection.close();
            asAcceptorSocket.close();

        }

    void SAL_CALL onTerminated( )
        {
            //printf("# normally terminate this server thread %d!\n",  m_id );
        }

public:
    // public to check if data transmition is OK
    sal_Char pReadBuffer[30];
    ServerSocketThread( osl::Condition &_aCond  ):m_aCondition(_aCond)
        {
            m_aCondition.reset();
            printf("#init ServerSocketThread\n");
            m_id = getIdentifier( );
            //printf("# successfully creat this ServerSocketThread %d!\n",  m_id );
        }

    ~ServerSocketThread( )
        {
            if ( isRunning( ) )
                printf("# error: ServerSocketThread has not terminated.\n" );
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
                printf("# ClientSocketThread: connect failed! \n");
            //  terminate();
            //}
            m_csConnectorSocket.close();
            free( pTimeout );
        }

    void SAL_CALL onTerminated( )
        {
            //printf("# normally terminate this thread %d!\n",  m_id );
        }

public:
    ClientSocketThread( osl::Condition &_aCond  ):
            m_aCondition(_aCond),
            m_saTargetSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT9 ),
            m_csConnectorSocket( )
        {
            m_id = getIdentifier( );
            //printf("# successfully creat this client thread %d!\n",  m_id );
        }

    ~ClientSocketThread( )
        {
            if ( isRunning( ) )
                printf("# error: client thread not terminated.\n" );
        }

};

// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
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
            ::osl::SocketAddr      m_aTargetSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT10 );
            ::osl::ConnectorSocket m_aConnectorSocket;

            if (! m_aTargetSocketAddr.is())
            {
                printf("# SocketAddr was NOT created successfully!\n");
            }
            else
            {
                printf("start ReadSocketThread\n");

                TimeValue *pTimeout;
                pTimeout  = ( TimeValue* )malloc( sizeof( TimeValue ) );
                pTimeout->Seconds = 5;
                pTimeout->Nanosec = 0;

                m_aCondition.wait();

                printf("connect()\n");

                oslSocketResult eResult = m_aConnectorSocket.connect( m_aTargetSocketAddr, pTimeout );
                if ( osl_Socket_Ok == eResult )
                {
                    sal_Int32 nReadCount = m_aConnectorSocket.read( m_aValues.getWriteBuffer(), m_aValues.getBufferSize() );
                    m_aValues.checkValues(nReadCount, m_nValue);
                }
                else
                {
                    printf("# ReadSocketThread: connect failed! \n");
                    printSocketResult(eResult);
                }

                //remove this line for deadlock on solaris( margritte.germany )
                m_aConnectorSocket.close();
                free( pTimeout );
            }
        }

    void SAL_CALL onTerminated( )
        {
            //printf("# normally terminate this thread %d!\n",  m_id );
        }

public:
    sal_Int32 getCount() {return m_aValues.getBufferSize();}
    bool       isOk() {return m_aValues.isFailure() == true ? false : true;}

    ReadSocketThread(sal_Int32 _nBufferSize, int _nValue, osl::Condition &_aCond )
            : m_nValue( _nValue ),
              m_aCondition(_aCond)
        {
            printf("#init ReadSocketThread\n");
            m_id = getIdentifier( );

            //printf("# successfully creat this client thread %d!\n",  m_id );
            m_aValues.createBuffer(_nBufferSize, 0);
        }

    ~ReadSocketThread( )
        {
            if ( isRunning( ) )
                printf("# error: client thread not terminated.\n" );
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
            printf("start WriteSocketThread\n");
            ::osl::AcceptorSocket asAcceptorSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
            ::osl::SocketAddr saLocalSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT10 );
            if (! saLocalSocketAddr.is())
            {
                printf("LocalSocketAddr was NOT created successfully!\n");
            }

            ::osl::StreamSocket ssStreamConnection;

            //if has not set this option, socket addr can not be binded in some time(maybe 2 minutes) by another socket
            asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, 1 );    //sal_True);

            /// if the thread should terminate, schedule return false
            // while ( schedule( ) == sal_True )
            // {
            printf("bind()\n");
            sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
            if  ( sal_True != bOK1 )
            {
                printf("# WriteSocketThread: AcceptorSocket bind address failed. \n" ) ;
            }
            else
            {
                printf("listen()\n");
                sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
                if  ( sal_True != bOK2 )
                {
                    printf("# WriteSocketThread: AcceptorSocket listen address failed. \n" ) ;
                }
                else
                {

                    // blocking mode, if read/recv failed, block until success
                    asAcceptorSocket.enableNonBlockingMode( sal_False);
                    printf("acceptConnection()\n");
                    m_aCondition.set();

                    oslSocketResult eResult = asAcceptorSocket.acceptConnection( ssStreamConnection );
                    if (eResult != osl_Socket_Ok )
                    {
                        printf("WriteSocketThread: acceptConnection failed! \n");
                    }
                    else
                    {

// LLA: removed, due to the fact, this is to error prone
// LLA:             char * pSrcRoot = getenv("SOURCE_ROOT");
// LLA:             // LLA: This is absolute wrong!
// LLA:             // strcat( pSrcRoot, "/sal/inc/osl/file.hxx");
// LLA:             rtl::OString sSrcRoot(pSrcRoot);
// LLA:             sSrcRoot += "/sal/inc/osl/file.hxx";
// LLA:
// LLA:             ::rtl::OUString sFilePath = ::rtl::OUString::createFromAscii( sSrcRoot.getStr() );
// LLA: #ifdef WNT
// LLA:             while (sFilePath.lastIndexOf('/') != -1)
// LLA:                 sFilePath = sFilePath.replace('/',(sal_Unicode)'\\');
// LLA: #endif
// LLA:             FILE *stream;
// LLA:             sal_uInt64     nCount_read;
// LLA:             sal_Char       buffer_read[FILE_READ];
// LLA:
// LLA:             if( (stream = fopen( oustring2char( sFilePath ), "r+t" )) != NULL )
// LLA:             {
// LLA:                 /* Attempt to read in 25 characters */
// LLA:                 nCount_read = fread( buffer_read, sizeof( char ), FILE_READ, stream );
// LLA:                 fclose( stream );
// LLA:             }
// LLA:             else
// LLA:                 printf("# File $SRC_ROOT/sal/inc/osl/file.hxx could not be opened\n" );

                        printf("write()\n");

                        ssStreamConnection.write( m_aValues.getBuffer(), m_aValues.getBufferSize() );
                        printf("done written.\n");
                    }
                }
            }
            ssStreamConnection.close();
            asAcceptorSocket.close();
        }

    void SAL_CALL onTerminated( )
        {
            //printf("# normally terminate this server thread %d!\n",  m_id );
        }

public:
    // public to check if data transmition is OK
    WriteSocketThread(sal_Int32 _nBufferSize, int _nValue, osl::Condition &_aCond )
            : m_aCondition(_aCond)
        {
            m_aCondition.reset();

            printf("#init WriteSocketThread\n");
            m_id = getIdentifier( );
            //printf("# successfully creat this server thread %d!\n",  m_id );

            m_aValues.createBuffer(_nBufferSize, _nValue);
        }

    ~WriteSocketThread( )
        {
            if ( isRunning( ) )
                printf("# error: server thread not terminated.\n" );
            m_aValues.freeBuffer();
        }
};

// -----------------------------------------------------------------------------

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
            ::osl::StreamSocket ssSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );

            ASSERT_TRUE(osl_Socket_TypeStream ==  ssSocket.getType( )) << "test for ctors_none constructor function: check if the stream socket was created successfully.";
        }

    TEST_F(ctors, ctors_acquire)
        {
            /// Socket constructor.
            ::osl::StreamSocket ssSocket( sHandle );

            ASSERT_TRUE(osl_Socket_TypeStream == ssSocket.getType( )) << "test for ctors_acquire constructor function: check if the socket was created successfully";
        }

    TEST_F(ctors, ctors_no_acquire)
        {
            /// Socket constructor.
            ::osl::StreamSocket ssSocket( sHandle, SAL_NO_ACQUIRE );

            ASSERT_TRUE(osl_Socket_TypeStream == ssSocket.getType( )) << " test for ctors_no_acquire constructor function: check if the socket was created successfully";
        }

    TEST_F(ctors, ctors_copy_ctor)
        {
            /// Socket constructor.
            ::osl::StreamSocket ssSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
            /// Socket copy constructor.
            ::osl::StreamSocket copySocket( ssSocket );

            ASSERT_TRUE(osl_Socket_TypeStream == copySocket.getType( )) << " test for ctors_copy_ctor constructor function: create new Socket instance using copy constructor";
        }

    class send_recv: public ::testing::Test
    {
    public:
        // initialization
        void SetUp( )
            {
            }

        void TearDown( )
            {

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

                printf("Length:=%d\n", nLength);
                printf(" bIsOk:=%d\n", bIsOk);

                ASSERT_TRUE(nLength == _nBufferSize && bIsOk == true) << " test for write/read values with two threads: send data from server, check readed data in client.";
            }
    }; // class send_recv

    TEST_F(send_recv, send_recv1)
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
            ASSERT_TRUE(nRes == 0) << " test for send/recv with two threads: launch Server/Client threads, send data from client, check received data in Server thread.";
        }

    // error when recv
    TEST_F(send_recv, send_recv2)
        {
            ::osl::AcceptorSocket asAcceptorSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
            ::osl::SocketAddr saLocalSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT9 );
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
            ASSERT_TRUE(nReadNumber == -1) << " test for send/recv, recv error!";
        }

    // Tests with different values and sizes
    TEST_F(send_recv, write_read_001)
        {
            write_read(50, 10);
        }
    TEST_F(send_recv, write_read_002)
        {
            write_read(1024, 20);
        }
    TEST_F(send_recv, write_read_003)
        {
            write_read(4000, 1);
        }
    TEST_F(send_recv, write_read_004)
        {
            write_read(8192, 3);
        }
    TEST_F(send_recv, write_read_005)
        {
            write_read(32768, 3);
        }

// -----------------------------------------------------------------------------

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
                    sal_Int32 nWrite1 = m_csConnectorSocket.write( pTestString1, 11 ); // "test socket"

                    sal_Int32 nWrite2 = m_csConnectorSocket.write( pTestString2, strlen( pTestString2 ) + 1 );
                    thread_sleep( 2 );
                    m_csConnectorSocket.write( pTestString2, strlen( pTestString2 ) + 1 );
                    printf("nWrite1 is %d, nWrite2 is %d\n", nWrite1, nWrite2 );
                    //thread_sleep( 1 );
                }
                else
                    printf("# SendClientThread: connect failed! \n");

                m_csConnectorSocket.close();
                free( pTimeout );
            }
    public:
        SendClientThread(  ):
                m_saTargetSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT9 ),
                m_csConnectorSocket( )
            {
                //printf("# successfully creat this SendClientThread %d!\n",  m_id );
            }

        ~SendClientThread( )
            {
                if ( isRunning( ) )
                    printf("# error: SendClientThread has not terminated.\n" );
            }

    };

    class shutdown: public ::testing::Test
    {
    public:
        // initialization
        void SetUp( )
            {
            }

        void TearDown( )
            {

            }
    }; // class shutdown

    // similar to close_002
    TEST_F(shutdown, shutdown_001)
        {
#if defined(LINUX)
            ::osl::AcceptorSocket asSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
            AcceptorThread myAcceptorThread( asSocket, rtl::OUString::createFromAscii("127.0.0.1") );
            myAcceptorThread.create();

            thread_sleep( 1 );

            //when accepting, shutdown the socket, the thread will not block for accepting
            asSocket.shutdown();
            myAcceptorThread.join();

            ASSERT_TRUE(myAcceptorThread.isOK( ) == sal_True) << "test for close when is accepting: the socket will quit accepting status.";
#endif
        }

    TEST_F(shutdown, shutdown_002)
        {
            ::osl::AcceptorSocket asSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
            ::osl::SocketAddr saLocalSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT9);
            asSocket.setOption( osl_Socket_OptionReuseAddr, 1 );
            ASSERT_TRUE(asSocket.bind( saLocalSocketAddr ) == sal_True) << "shutdown_002: bind fail";
            ASSERT_TRUE(asSocket.listen( 1 ) == sal_True) << "shutdown_002: listen fail";
            sal_Char pReadBuffer[40];
//          osl::Condition aCondition;
            SendClientThread mySendThread;
            mySendThread.create();

            asSocket.enableNonBlockingMode( sal_False );
            ::osl::StreamSocket ssConnectionSocket;
            oslSocketResult eResult = asSocket.acceptConnection( ssConnectionSocket );
            ASSERT_TRUE(eResult == osl_Socket_Ok) << "shutdown_002: acceptConnection fail";

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
            printf("after read 2, nRead1 is %d, nRead2 is %d, nRead3 is %d \n", nRead1, nRead2, nRead3 );
            mySendThread.join();

            ssConnectionSocket.close();
            asSocket.close();

            /* on Linux, if send is before shutdown(DirRead), can read, nRecv2 still > 0,
               http://dbforums.com/arch/186/2002/12/586417
               While on Solaris, after shutdown(DirRead), all read will return 0
            */
#ifdef LINUX
            ASSERT_TRUE(nRead1 > 0  && nRead3 == 0) << "test for shutdown read direction: the socket can not read(recv).";
#else
            ASSERT_TRUE(nRead1 > 0  && nRead2 == 0 && nRead3 == 0) << "test for shutdown read direction: the socket can not read(recv).";
#endif

        }

    TEST_F(shutdown, shutdown_003)
        {
            ::osl::AcceptorSocket asSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
            ::osl::SocketAddr saLocalSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT9);
            asSocket.setOption( osl_Socket_OptionReuseAddr, 1 );
            ASSERT_TRUE(asSocket.bind( saLocalSocketAddr ) == sal_True) << "shutdown_002: bind fail";
            ASSERT_TRUE(asSocket.listen( 1 ) == sal_True) << "shutdown_002: listen fail";
            sal_Char pReadBuffer[40];
            osl::Condition aCondition;
            SendClientThread mySendThread;
            mySendThread.create();

            asSocket.enableNonBlockingMode( sal_False );
            ::osl::StreamSocket ssConnectionSocket;
            oslSocketResult eResult = asSocket.acceptConnection( ssConnectionSocket );
            ASSERT_TRUE(eResult == osl_Socket_Ok) << "shutdown_002: acceptConnection fail";

            thread_sleep( 1 );
            //shutdown write after client the first send complete
            ssConnectionSocket.shutdown( osl_Socket_DirWrite );

            // recv should not shutdown
            sal_Int32 nRead1 = ssConnectionSocket.read( pReadBuffer, 11 );

            sal_Int32 nWrite = ssConnectionSocket.write( pReadBuffer, 11 );
            // still can read
            sal_Int32 nRead3 = ssConnectionSocket.read( pReadBuffer + nRead1 , 12 );
            printf("after read 2, nRead1 is %d, nWrite is %d, nRead3 is %d\n", nRead1, nWrite, nRead3 );
            mySendThread.join();
            ssConnectionSocket.close();
            asSocket.close();

            ASSERT_TRUE(nRead1  > 0  && nWrite == 0 && nRead3 > 0) << "test for shutdown read direction: the socket can not send(write).";

        }

    class isExceptionPending: public ::testing::Test
    {
    public:
    }; // class isExceptionPending

    /**tester's comments: lack of a case that return sal_True, do not know when it will return sal_True*/
    TEST_F(isExceptionPending, isExPending_001)
        {
            ::osl::AcceptorSocket asSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
            TimeValue *pTimeout;
            pTimeout  = ( TimeValue* )malloc( sizeof( TimeValue ) );
            pTimeout->Seconds = 3;
            pTimeout->Nanosec = 0;
            sal_Bool bOk = asSocket.isExceptionPending( pTimeout );
            free( pTimeout );

            ASSERT_TRUE(bOk == sal_False) << "test for isExceptionPending.";
        }

// -----------------------------------------------------------------------------
/** Server Socket Thread, write a file which is large
 */
// LLA: class WriteSocketThread : public Thread
// LLA: {
// LLA:     ValueCheckProvider m_aValues;
// LLA:
// LLA: protected:
// LLA:     oslThreadIdentifier m_id;
// LLA:
// LLA:     void SAL_CALL run( )
// LLA:     {
// LLA:         ::osl::AcceptorSocket asAcceptorSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
// LLA:         ::osl::SocketAddr saLocalSocketAddr( rtl::OUString::createFromAscii("10.16.66.252"), 8888 );
// LLA:         ::osl::StreamSocket ssStreamConnection;
// LLA:
// LLA:         //if has not set this option, socket addr can not be binded in some time(maybe 2 minutes) by another socket
// LLA:         asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, 1 );    //sal_True);
// LLA:
// LLA:         /// if the thread should terminate, schedule return false
// LLA:         while ( schedule( ) == sal_True )
// LLA:         {
// LLA:             sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
// LLA:             if  ( sal_True != bOK1 )
// LLA:             {
// LLA:                 printf("# WriteSocketThread: AcceptorSocket bind address failed. \n" ) ;
// LLA:                 break;
// LLA:             }
// LLA:             sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
// LLA:             if  ( sal_True != bOK2 )
// LLA:             {
// LLA:                 printf("# WriteSocketThread: AcceptorSocket listen address failed. \n" ) ;
// LLA:                 break;
// LLA:             }
// LLA:             // blocking mode, if read/recv failed, block until success
// LLA:             asAcceptorSocket.enableNonBlockingMode( sal_False);
// LLA:
// LLA:             oslSocketResult eResult = asAcceptorSocket.acceptConnection( ssStreamConnection );
// LLA:             if (eResult != osl_Socket_Ok )
// LLA:             {
// LLA:                 printf("WriteSocketThread: acceptConnection failed! \n");
// LLA:                 break;
// LLA:             }
// LLA:
// LLA:
// LLA:             sal_Int32 nReadNumber1 = ssStreamConnection.write( m_aValues.getBuffer(), m_aValues.getBufferSize() );
// LLA:             break;
// LLA:         }
// LLA:         ssStreamConnection.close();
// LLA:         asAcceptorSocket.close();
// LLA:     }
// LLA: }
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
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
                printf("wait done\n");

                TimeValue *pTimeout;
                pTimeout  = ( TimeValue* )malloc( sizeof( TimeValue ) );
                pTimeout->Seconds = 20;
                pTimeout->Nanosec = 0;


                // blocking mode, if read/recv failed, block until success
                printf("enableNonBlockingMode(false)\n");
                aSocket.enableNonBlockingMode( sal_False );


                printf("connect()\n");
                oslSocketResult eResult = aSocket.connect( aSocketAddr, pTimeout );
                if ( osl_Socket_Ok == eResult)
                {
                    if (m_pBuffer)
                    {
                        printf("read()\n");
                        m_nReadCount = aSocket.read( m_pBuffer, m_nBufferSize );
                        printf("%d bytes received.\n", m_nReadCount);
                    }
                }
                else
                {
                    printf("# ReadSocket2Thread: connect failed! \n");
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
                //printf("# normally terminate this thread %d!\n",  m_id );
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
                //printf("# successfully creat this client thread %d!\n",  m_id );
            }

        void setAddr(rtl::OString const& _sAddr)
            {
                m_sAddr = _sAddr;
            }

        ~ReadSocket2Thread( )
            {
                if ( isRunning( ) )
                    printf("# error: client thread not terminated.\n" );
                free(m_pBuffer);
            }

    };

    // -----------------------------------------------------------------------------

    class justtest : public ::testing::Test
    {
    protected:
        void send_Acceptor(rtl::OString const& _sAddr, osl::Condition &)
            {
                ::osl::AcceptorSocket aSocket; // ( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
                ::osl::SocketAddr aSocketAddr;

                if (! aSocketAddr.setPort(IP_PORT_TEST))
                {
                    printf("# can't set port\n" );
                }

                if (! aSocketAddr.setHostname(rtl::OUString::createFromAscii(_sAddr.getStr())))
                {
                    printf("# can't set hostname/ip\n" );
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
                    printf("# can't bind.\n" );
                }
                if (! aSocket.listen( ))
                {
                    printf("# can't listen. \n" );
                }

                // blocking mode, if read/recv failed, block until success
                aSocket.enableNonBlockingMode( sal_False);
                ::osl::StreamSocket ssStreamConnection;

                oslSocketResult eResult = aSocket.acceptConnection( ssStreamConnection );
                if (eResult != osl_Socket_Ok )
                {
                    printf("WriteSocketThread: acceptConnection failed! \n");
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

        // -----------------------------------------------------------------------------

        void send_Connector(rtl::OString const& _sAddr, osl::Condition &/*_aCondition*/ )
            {
                ::osl::ConnectorSocket aSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
                ::osl::SocketAddr aSocketAddr( rtl::OUString::createFromAscii(_sAddr.getStr()), IP_PORT_TEST );

                if (! aSocketAddr.is())
                {
                    printf("is failed.\n");
                    return;
                }

                //if has not set this option, socket addr can not be binded in some time(maybe 2 minutes) by another socket
                aSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //sal_True;

                oslSocketResult aResult = aSocket.connect( aSocketAddr );
                if  ( aResult != osl_Socket_Ok )
                {
                    printf("# send_Connector: connect failed. \n" );
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

                    printf("received: %s\n", pBuffer2);

                    // char * pBuffer3 = "quit\n";
                    // nBufferSize = strlen(pBuffer3);
                    // nWriteNumber = ssStreamConnection.write( pBuffer3, nBufferSize );

                    rtl::OUString suError = ssStreamConnection.getErrorAsString();
                    free(pBuffer2);
                    // ssStreamConnection.close();

                    // ssStreamConnection.close();
                }
                aSocket.shutdown(osl_Socket_DirReadWrite);
                aSocket.close();
            }


    public:
// LLA: orig        void send_recv()
// LLA: orig             {
// LLA: orig                if ( ifAvailable(rtl::OUString::createFromAscii("margritte.germany")) == sal_True )
// LLA: orig                    printf("margritte is alive ! \n");
// LLA: orig                if ( ifAvailable(rtl::OUString::createFromAscii("10.16.66.252")) == sal_False )
// LLA: orig                {
// LLA: orig            printf("ip 10.16.66.252 is not alive! \n");
// LLA: orig            return;
// LLA: orig        }
// LLA: orig                 ReadSocket2Thread myReadThread;
// LLA: orig                 myReadThread.create();
// LLA: orig
// LLA: orig                 thread_sleep( 2 );
// LLA: orig                 // send_Acceptor();
// LLA: orig                 send_Connector();
// LLA: orig
// LLA: orig                 myReadThread.join();
// LLA: orig
// LLA: orig                 // statistics
// LLA: orig                 sal_uInt32 nLength = myReadThread.getCount();
// LLA: orig                 bool       bIsOk   = myReadThread.isOk(); // check if the values are right.
// LLA: orig
// LLA: orig                 printf("Length:=%d\n", nLength);
// LLA: orig                 printf(" bIsOk:=%d\n", bIsOk);
// LLA: orig             }

        // -----------------------------------------------------------------------------

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
                    printf("# connect failed. \n" );
                }
                else
                {
                    // blocking mode, if read/recv failed, block until success
                    aSocket.enableNonBlockingMode( sal_False );

                    ::osl::StreamSocket ssStreamConnection(aSocket);

                    char const * pBuffer = "Test String\n";
                    sal_Int32 nBufferSize = strlen(pBuffer);
                    sal_Int32 nWriteNumber = ssStreamConnection.write( pBuffer, nBufferSize );

                    // char * pBuffer2 = "                                                                                                 ";
                    // sal_Int32 nReadNumber = ssStreamConnection.read( pBuffer2, strlen(pBuffer2) );

                    char const * pBuffer3 = "quit\n";
                    nBufferSize = strlen(pBuffer3);
                    nWriteNumber = ssStreamConnection.write( pBuffer3, nBufferSize );

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
                    printf("found %s!\n", sAddr.getStr());
                }
                send_Connector_2_margritte(sAddr);
            }

        // -----------------------------------------------------------------------------



        void getPage(rtl::OString const& _sAddr);
    }; // class isExceptionPending

    TEST_F(justtest, send_recv)
        {
            rtl::OString sAddr;
            // if ( ifAvailable(rtl::OUString::createFromAscii("margritte.germany")) == sal_True )
            // {
            //     printf("margritte is alive ! \n");
            //     sAddr = "margritte.germany";
            // }

            sAddr = "margritte.germany.sun.com";
            if ( ifAvailable(rtl::OUString::createFromAscii(sAddr.getStr())) == sal_True )
            {
                printf("found %s!\n", sAddr.getStr());
            }
//                 else
//                 {
//                     if ( ifAvailable(rtl::OUString::createFromAscii("192.168.7.2")) == sal_True )
//                     {
//                         sAddr = "192.168.7.2";
//                         printf("moon found ! \n");
//                     }
//                     else
//                     {
//                         if ( ifAvailable(rtl::OUString::createFromAscii("moon.linux.bogus")) == sal_True )
//                         {
//                             sAddr = "moon.linux.bogus";
//                             printf("moon found ! \n");
//                         }
//                         else
//                         {
//                             if ( ifAvailable(rtl::OUString::createFromAscii("moon")) == sal_True )
//                             {
//                                 sAddr = "moon";
//                                 printf("moon found ! \n");
//                             }
//                         }
//                     }
//                 }

            // if ( ifAvailable(rtl::OUString::createFromAscii("10.16.64.196")) == sal_False )
            // {
            //     printf("ip 10.16.64.196 is not alive! \n");
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
                sal_uInt32 nLength = myReadThread.getCount();
                bool       bIsOk   = myReadThread.isOk(); // check if the values are right.

                printf("Length:=%d\n", nLength);
                printf(" bIsOk:=%d\n", bIsOk);
            }
            else
            {
                printf("ERROR: No echo Server on %s found.\n", sAddr.getStr());
            }
        }

    TEST_F(justtest, test_getPage)
        {
            // rtl::OString sPage("lla-1.germany.sun.com");
            // getPage(sPage);

            rtl::OString sPage("lla-1");
            getPage(sPage);
        }


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
                    printf("problem with getAddr: ");
                    printSocketResult(aResult);
                }

                rtl::OUString sStr = aSocketAddr.getHostname(&aResult);
                if (aResult != osl_Socket_Ok)
                {
                    printf("problem with hostname: ");
                    printSocketResult(aResult);
                }
            }

                oslSocketResult aResult;

                // SocketAddr::resolveHostname(suAddr, aSocketAddr);
                // if (! aSocketAddr.is())
                // {
                //     printf("Can't resolve Hostname.\n");
                //     return;
                // }
                // rtl::OUString sStr = aSocketAddr.getHostname(&aResult);
                // if (aResult != osl_Socket_Ok)
                // {
                //     printf("problem with hostname: ");
                //     printSocketResult(aResult);
                //
                // }

                if (! aSocketAddr.is())
                {
                    printf("SocketAddr::is() failed.\n");
                    return;
                }

                //if has not set this option, socket addr can not be binded in some time(maybe 2 minutes) by another socket
                aSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //sal_True;

                aResult = aSocket.connect( aSocketAddr );
                if  ( aResult != osl_Socket_Ok )
                {
                    printf("# send_Connector: connect failed. \n" );
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
                            printf("%s", pBuffer2);
                            free(pBuffer2);
                        }
                    }
                    free(pBufferPeek);

                    // char * pBuffer3 = "quit\n";
                    // nBufferSize = strlen(pBuffer3);
                    // nWriteNumber = ssStreamConnection.write( pBuffer3, nBufferSize );

                    rtl::OUString suError = ssStreamConnection.getErrorAsString();
                }
                aSocket.shutdown(osl_Socket_DirReadWrite);
                aSocket.close();
            }


} // namespace osl_StreamSocket

static oslSignalAction SAL_CALL signalHandler(void* pData, oslSignalInfo* pInfo)
{
    return osl_Signal_ActCallNextHdl;
}

int main(int argc, char **argv)
{
    osl_addSignalHandler(signalHandler, NULL);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
