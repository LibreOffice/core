/*************************************************************************
 *
 *  $RCSfile: osl_StreamSocket.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-01-05 21:23:49 $
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
 *  Copyright 2000 by Sun Microsystems,
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

#include <cppunit/simpleheader.hxx>

#include "osl_Socket_Const.h"
#include "sockethelper.hxx"

using namespace osl;
using namespace rtl;

#define IP_PORT_MYPORT9  8897
#define IP_PORT_MYPORT10 8898

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
            printf( "# AcceptorSocket bind address failed.\n" ) ;
            return;
        }
        sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
        if  ( sal_True != bOK2 )
        {
            printf( "# AcceptorSocket listen address failed.\n" ) ;
            return;
        }

        asAcceptorSocket.enableNonBlockingMode( sal_False );

        oslSocketResult eResult = asAcceptorSocket.acceptConnection( ssStreamConnection );
        if (eResult != osl_Socket_Ok )
        {
            bOK = sal_True;
            printf("# AcceptorThread: acceptConnection failed! \n");
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
            printf( "# error: Acceptor thread not terminated.\n" );
        }
    }
};

/** Server Socket Thread, served as a temp little server to communicate with client.
*/
class ServerSocketThread : public Thread
{
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
            printf( "# ServerSocketThread: AcceptorSocket bind address failed.\n" ) ;
            break;
        }
        sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
        if  ( sal_True != bOK2 )
        {
            printf( "# ServerSocketThread: AcceptorSocket listen address failed.\n" ) ;
            break;
        }

        asAcceptorSocket.enableNonBlockingMode( sal_False );

        oslSocketResult eResult = asAcceptorSocket.acceptConnection( ssStreamConnection );
        if (eResult != osl_Socket_Ok )
        {
            printf("# ServerSocketThread: acceptConnection failed! \n");
            break;
        }
            sal_Int32 nReadNumber1 = ssStreamConnection.recv( pReadBuffer, 11 );
            sal_Int32 nReadNumber2 = ssStreamConnection.recv( pReadBuffer + nReadNumber1, 11 );
            pReadBuffer[nReadNumber1 + nReadNumber2] = '\0';
            //printf( "# read buffer content: %s\n", pReadBuffer );
            break;
        }
        ssStreamConnection.close();
        asAcceptorSocket.close();

    }

    void SAL_CALL onTerminated( )
    {
        //printf( "# normally terminate this server thread %d!\n",  m_id );
    }

public:
    // public to check if data transmition is OK
    sal_Char pReadBuffer[30];
    ServerSocketThread( )
    {
        m_id = getIdentifier( );
        //printf( "# successfully creat this server thread %d!\n",  m_id );
    }

    ~ServerSocketThread( )
    {
        if ( isRunning( ) )
            printf( "# error: server thread not terminated.\n" );
    }
};

/** Client Socket Thread, served as a temp little client to communicate with server.
*/
class ClientSocketThread : public Thread
{
protected:
    oslThreadIdentifier m_id;
    ::osl::SocketAddr saTargetSocketAddr;
    ::osl::ConnectorSocket csConnectorSocket;

    void SAL_CALL run( )
    {
        TimeValue *pTimeout;
        pTimeout  = ( TimeValue* )malloc( sizeof( TimeValue ) );
        pTimeout->Seconds = 5;
        pTimeout->Nanosec = 0;

        /// if the thread should terminate, schedule return false
        //while ( schedule( ) == sal_True )
        //{
              if ( osl_Socket_Ok == csConnectorSocket.connect( saTargetSocketAddr, pTimeout ))
              {
                  csConnectorSocket.send( pTestString1, 11 ); // "test socket"
                  csConnectorSocket.send( pTestString2, 10);
              }
              else
                  printf( "# ClientSocketThread: connect failed! \n");
         //     terminate();
        //}
        csConnectorSocket.close();
        free( pTimeout );
    }

    void SAL_CALL onTerminated( )
    {
        //printf( "# normally terminate this thread %d!\n",  m_id );
    }

public:
    ClientSocketThread( ):
        saTargetSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT9 ),
        csConnectorSocket( )
    {
        m_id = getIdentifier( );
        //printf( "# successfully creat this client thread %d!\n",  m_id );
    }

    ~ClientSocketThread( )
    {
        if ( isRunning( ) )
            printf( "# error: client thread not terminated.\n" );
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
            :m_pBuffer(NULL),
             m_bFoundFailure(false),
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

class ClientReadSocketThread : public Thread
{
    ValueCheckProvider m_aValues;
    int m_nValue;

protected:
    oslThreadIdentifier m_id;
    ::osl::SocketAddr saTargetSocketAddr;
    ::osl::ConnectorSocket csConnectorSocket;

    void SAL_CALL run( )
    {
        TimeValue *pTimeout;
        pTimeout  = ( TimeValue* )malloc( sizeof( TimeValue ) );
        pTimeout->Seconds = 5;
        pTimeout->Nanosec = 0;

        /// if the thread should terminate, schedule return false
        //while ( schedule( ) == sal_True )
        //{
              if ( osl_Socket_Ok == csConnectorSocket.connect( saTargetSocketAddr, pTimeout ))
              {
                  sal_Int32 nReadCount = csConnectorSocket.read( m_aValues.getWriteBuffer(), m_aValues.getBufferSize() );
                m_aValues.checkValues(nReadCount, m_nValue);
              }
              else
            {
                  printf( "# ClientReadSocketThread: connect failed! \n");
            }
        //      terminate();
        //}
        //remove this line for deadlock on solaris( margritte.germany )
        csConnectorSocket.close();
        free( pTimeout );
    }

    void SAL_CALL onTerminated( )
    {
        //printf( "# normally terminate this thread %d!\n",  m_id );
    }

public:
    sal_uInt32 getCount() {return m_aValues.getBufferSize();}
    bool       isOk() {return m_aValues.isFailure() == true ? false : true;}

    ClientReadSocketThread(sal_Int32 _nBufferSize, int _nValue ):
        saTargetSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT10 ),
        csConnectorSocket( ),
        m_nValue( _nValue )
    {
        m_id = getIdentifier( );
        //printf( "# successfully creat this client thread %d!\n",  m_id );
        m_aValues.createBuffer(_nBufferSize, 0);
    }

    ~ClientReadSocketThread( )
        {
            if ( isRunning( ) )
                printf( "# error: client thread not terminated.\n" );
            m_aValues.freeBuffer();
        }

};

/** Server Socket Thread, write a file which is large
*/
class ServerWriteSocketThread : public Thread
{
    ValueCheckProvider m_aValues;

protected:
    oslThreadIdentifier m_id;

    void SAL_CALL run( )
    {
        ::osl::AcceptorSocket asAcceptorSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
        ::osl::SocketAddr saLocalSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT10 );
        ::osl::StreamSocket ssStreamConnection;

        //if has not set this option, socket addr can not be binded in some time(maybe 2 minutes) by another socket
        asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, 1 );    //sal_True);

        /// if the thread should terminate, schedule return false
        while ( schedule( ) == sal_True )
        {
            sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
            if  ( sal_True != bOK1 )
            {
                printf( "# ServerWriteSocketThread: AcceptorSocket bind address failed. \n" ) ;
                break;
            }
            sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
            if  ( sal_True != bOK2 )
            {
                printf( "# ServerWriteSocketThread: AcceptorSocket listen address failed. \n" ) ;
                break;
            }
            // blocking mode, if read/recv failed, block until success
            asAcceptorSocket.enableNonBlockingMode( sal_False);

            oslSocketResult eResult = asAcceptorSocket.acceptConnection( ssStreamConnection );
            if (eResult != osl_Socket_Ok )
            {
                printf("# ServerWriteSocketThread: acceptConnection failed! \n");
                break;
            }


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
// LLA:                 printf( "# File $SRC_ROOT/sal/inc/osl/file.hxx could not be opened\n" );

            sal_Int32 nReadNumber1 = ssStreamConnection.write( m_aValues.getBuffer(), m_aValues.getBufferSize() );
            break;
        }
        ssStreamConnection.close();
        asAcceptorSocket.close();
    }

    void SAL_CALL onTerminated( )
    {
        //printf( "# normally terminate this server thread %d!\n",  m_id );
    }

public:
    // public to check if data transmition is OK
    ServerWriteSocketThread(sal_Int32 _nBufferSize, int _nValue )
    {
        m_id = getIdentifier( );
        //printf( "# successfully creat this server thread %d!\n",  m_id );

        m_aValues.createBuffer(_nBufferSize, _nValue);
    }

    ~ServerWriteSocketThread( )
        {
            if ( isRunning( ) )
                printf( "# error: server thread not terminated.\n" );
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
            //client sent two strings, and server received, check the order and value
            ServerSocketThread myServerThread;
            ClientSocketThread myClientThread;
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
            ::osl::SocketAddr saLocalSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT9 );
            ::osl::StreamSocket ssStreamConnection;
            sal_Char pReadBuffer[30] = "";

            ClientSocketThread myClientThread;
            myClientThread.create( );

            asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, 1 );

            sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
            sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
            asAcceptorSocket.enableNonBlockingMode( sal_True );
            oslSocketResult eResult = asAcceptorSocket.acceptConnection( ssStreamConnection );
            sal_Int32 nReadNumber = ssStreamConnection.recv( pReadBuffer, 11 );

            myClientThread.join( ) ;
            ssStreamConnection.close();
            asAcceptorSocket.close();
            CPPUNIT_ASSERT_MESSAGE(" test for send/recv, recv error!", nReadNumber == -1 );
        }

        void write_read(sal_Int32 _nBufferSize, int _nValue)
        {
            //client sent two strings, and server received, check the order and value
            ServerWriteSocketThread myServerThread(_nBufferSize, _nValue);
            ClientReadSocketThread myClientThread(_nBufferSize, _nValue);
            myServerThread.create( );
//          thread_sleep( 1 );
            myClientThread.create( );

            //wait until the thread terminate
            myClientThread.join( );
            myServerThread.join( );

            //Maximum Packet Size is ( ARPANET, MILNET = 1007 Ethernet (10Mb) = 1500
            // Proteon PRONET  = 2046), so here test read 4000 bytes
            sal_uInt32 nLength = myClientThread.getCount();
            bool       bIsOk   = myClientThread.isOk(); // check if the values are right.

            printf("# Length:=%d\n", nLength);
            printf("#  bIsOk:=%d\n", bIsOk);

            CPPUNIT_ASSERT_MESSAGE(" test for write/read values with two threads: send data from server, check readed data in client.",
                                    nLength == _nBufferSize && bIsOk == true);
        }

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

        CPPUNIT_TEST_SUITE( send_recv );
        CPPUNIT_TEST( write_read_001 );
        CPPUNIT_TEST( write_read_002 );
        CPPUNIT_TEST( write_read_003 );
        CPPUNIT_TEST( write_read_004 );
        CPPUNIT_TEST( send_recv1 );
        CPPUNIT_TEST( send_recv2 );
//      CPPUNIT_TEST( write_read );
        CPPUNIT_TEST_SUITE_END();
    }; // class send_recv

class SendClientThread : public ClientSocketThread
{
protected:

    void SAL_CALL run( )
    {
        TimeValue *pTimeout;
        pTimeout  = ( TimeValue* )malloc( sizeof( TimeValue ) );
        pTimeout->Seconds = 5;
        pTimeout->Nanosec = 0;

          if ( osl_Socket_Ok == csConnectorSocket.connect( saTargetSocketAddr, pTimeout ))
          {
              sal_Int32 nWrite1 = csConnectorSocket.write( pTestString1, 11 ); // "test socket"

              sal_Int32 nWrite2 = csConnectorSocket.write( pTestString2, strlen( pTestString2 ) + 1 );
            thread_sleep( 2 );
              sal_Int32 nWrite3 = csConnectorSocket.write( pTestString2, strlen( pTestString2 ) + 1 );
              printf("# nWrite1 is %d, nWrite2 is %d\n", nWrite1, nWrite2 );
              //thread_sleep( 1 );
          }
          else
              printf( "# SendClientThread: connect failed! \n");

        csConnectorSocket.close();
        free( pTimeout );
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
            AcceptorThread myAcceptorThread( asSocket, rtl::OUString::createFromAscii("127.0.0.1") );
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
            ::osl::SocketAddr saLocalSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT9);
            asSocket.setOption( osl_Socket_OptionReuseAddr, 1 );
            CPPUNIT_ASSERT_MESSAGE("shutdown_002: bind fail", asSocket.bind( saLocalSocketAddr ) == sal_True);
            CPPUNIT_ASSERT_MESSAGE("shutdown_002: listen fail", asSocket.listen( 1 ) == sal_True );
            sal_Char pReadBuffer[40];
            SendClientThread mySendThread;
            mySendThread.create();

            asSocket.enableNonBlockingMode( sal_False );
            ::osl::StreamSocket ssConnectionSocket;
            oslSocketResult eResult = asSocket.acceptConnection( ssConnectionSocket );
            CPPUNIT_ASSERT_MESSAGE("shutdown_002: acceptConnection fail", eResult == osl_Socket_Ok );

            /* set socket option SO_LINGER 0, so close immediatly */
            linger aLingerSet;
                sal_Int32 nBufferLen = sizeof( struct linger );
                    aLingerSet.l_onoff = 0;
                    aLingerSet.l_linger = 0;

            sal_Bool b1 = ssConnectionSocket.setOption( osl_Socket_OptionLinger,  &aLingerSet, nBufferLen );
            thread_sleep( 1 );
            //sal_uInt32 nRecv1 = 0;
            sal_Int32 nRead1 = ssConnectionSocket.read( pReadBuffer, 11 );

            //shutdown read after client the first send complete
            ssConnectionSocket.shutdown( osl_Socket_DirRead );

            sal_Int32 nRead2 = ssConnectionSocket.read( pReadBuffer + nRead1, 12 );
            sal_Int32 nRead3 = ssConnectionSocket.read( pReadBuffer + nRead1 + nRead2, 12 );
            printf("# after read 2, nRead1 is %d, nRead2 is %d, nRead3 is %d \n", nRead1, nRead2, nRead3 );
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
            ::osl::SocketAddr saLocalSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT9);
            asSocket.setOption( osl_Socket_OptionReuseAddr, 1 );
            CPPUNIT_ASSERT_MESSAGE("shutdown_002: bind fail", asSocket.bind( saLocalSocketAddr ) == sal_True);
            CPPUNIT_ASSERT_MESSAGE("shutdown_002: listen fail", asSocket.listen( 1 ) == sal_True );
            sal_Char pReadBuffer[40];
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
            printf("# after read 2, nRead1 is %d, nWrite is %d, nRead3 is %d\n", nRead1, nWrite, nRead3 );
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

// -----------------------------------------------------------------------------

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_StreamSocket::ctors, "osl_StreamSocket");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_StreamSocket::send_recv, "osl_StreamSocket");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_StreamSocket::shutdown, "osl_StreamSocket");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_StreamSocket::isExceptionPending, "osl_StreamSocket");

} // namespace osl_StreamSocket

// -----------------------------------------------------------------------------

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
NOADDITIONAL;
