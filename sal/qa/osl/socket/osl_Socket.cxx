/*************************************************************************
 *
 *  $RCSfile: osl_Socket.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 16:41:07 $
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

#ifndef _OSL_SOCKET_CONST_H_
#include <osl_Socket_Const.h>
#endif

#include <cppunit/simpleheader.hxx>

using namespace osl;
using namespace rtl;

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

/** compare two OUString.
*/
inline sal_Bool compareUString( const ::rtl::OUString & ustr1, const ::rtl::OUString & ustr2 )
{
    sal_Bool bOk = ustr1.equalsIgnoreAsciiCase( ustr2 );

    return bOk;
}

/** compare a OUString and an ASCII string.
*/
inline sal_Bool compareUString( const ::rtl::OUString & ustr, const sal_Char *astr )
{
    ::rtl::OUString ustr1 = rtl::OUString::createFromAscii( astr );
    sal_Bool bOk = ustr1.equalsIgnoreAsciiCase( ustr1 );

    return bOk;
}

/** compare two socket address.
*/
inline sal_Bool compareSocketAddr( const ::osl::SocketAddr & addr1 , const ::osl::SocketAddr & addr2  )
{
    return ( ( sal_True == compareUString( addr1.getHostname( 0 ), addr2.getHostname( 0 ) ) ) && ( addr2.getPort( ) == addr2.getPort( ) ) );
}


/** print a UNI_CODE String. And also print some comments of the string.
*/
inline void printUString( const ::rtl::OUString & str, const sal_Char * msg = "" )
{
    rtl::OString aString;

    printf( "#%s #printUString_u# ", msg );
    aString = ::rtl::OUStringToOString( str, RTL_TEXTENCODING_ASCII_US );
    printf( "%s\n", aString.getStr( ) );
}

/** get the local host name.
    mindy: gethostbyname( "localhost" ), on Linux, it returns the hostname in /etc/hosts + domain name,
    if no entry in /etc/hosts, it returns "localhost" + domain name
*/
inline ::rtl::OUString getHost( void )
{
    struct hostent *hptr;

    hptr = gethostbyname( "localhost" );
    CPPUNIT_ASSERT_MESSAGE( "#In getHostname function, error on gethostbyname()",  hptr != NULL );
    ::rtl::OUString aUString = ::rtl::OUString::createFromAscii( (const sal_Char *) hptr->h_name );

    return aUString;
}

/** get the full host name of the current processor, such as "aegean.prc.sun.com" --mindyliu
*/
inline ::rtl::OUString getThisHostname( void )
{
    ::rtl::OUString aUString;
#ifdef WNT
    struct hostent *hptr;
    hptr = gethostbyname( "localhost" );
    CPPUNIT_ASSERT_MESSAGE( "#In getHostname function, error on gethostbyname()",  hptr != NULL );
    aUString = ::rtl::OUString::createFromAscii( (const sal_Char *) hptr->h_name );
#else

    char hostname[255];
    //gethostname(hostname, 20);
    CPPUNIT_ASSERT_MESSAGE( "#Error: gethostname failed.",  gethostname(hostname, 255) == 0 );
    printf("#hostname is %s\n", hostname);

    aUString = ::rtl::OUString::createFromAscii( (const sal_Char *) hostname );
#endif
    return aUString;

}

/** get local ethernet IP
*/
inline ::rtl::OUString getLocalIP( )
{
    char hostname[255];
    gethostname(hostname, 255);

    struct hostent *hptr;
    hptr = gethostbyname( hostname);
    CPPUNIT_ASSERT_MESSAGE( "#In getHostname function, error on gethostbyname()",  hptr != NULL );

    struct in_addr ** addrptr;
    addrptr = (struct in_addr **) hptr->h_addr_list ;
    ::rtl::OUString aUString;
    //if there are more than one IPs on the same machine, we select one
    for (; *addrptr; addrptr++)
    {
        //printf("#Local IP Address: %s\n", inet_ntoa(**addrptr));
        aUString = ::rtl::OUString::createFromAscii( (const sal_Char *) (inet_ntoa(**addrptr)) );
    }
    return aUString;

}

/** construct error message
*/
inline ::rtl::OUString outputError( const ::rtl::OUString & returnVal, const ::rtl::OUString & rightVal, const sal_Char * msg = "")
{
    ::rtl::OUString aUString;
    if ( returnVal.equals( rightVal ) )
        return aUString;
    aUString += ::rtl::OUString::createFromAscii(msg);
    aUString += ::rtl::OUString::createFromAscii(": the returned value is '");
    aUString += returnVal;
    aUString += ::rtl::OUString::createFromAscii("', but the value should be '");
    aUString += rightVal;
    aUString += ::rtl::OUString::createFromAscii("'.");
    return aUString;
}

/** print Boolean value.
*/
inline void printBool( sal_Bool bOk )
{
    printf( "#printBool# " );
    ( sal_True == bOk ) ? printf( "YES!\n" ): printf( "NO!\n" );
}

/** print content of a ByteSequence.
*/
inline void printByteSequence_IP( const ::rtl::ByteSequence & bsByteSeq, sal_Int32 nLen )
{
     printf( "#ByteSequence is: " );
    for ( int i = 0; i < nLen; i++ ){
        if ( bsByteSeq[i] < 0 )
            printf("%d ",  256 + bsByteSeq[i] );
        else
            printf("%d ",  bsByteSeq[i] );
    }
    printf( " .\n" );
}

/** convert an IP which is stored as a UString format to a ByteSequence array for later use.
*/
inline ::rtl::ByteSequence UStringIPToByteSequence( ::rtl::OUString aUStr )
{

    rtl::OString aString = ::rtl::OUStringToOString( aUStr, RTL_TEXTENCODING_ASCII_US );
    const sal_Char *pChar = aString.getStr( ) ;
    sal_Char tmpBuffer[4];
    sal_Int32 nCharCounter = 0;
    ::rtl::ByteSequence bsByteSequence( IP_VER );
    sal_Int32 nByteSeqCounter = 0;

    for ( int i = 0; i < aString.getLength( ) + 1 ; i++ )
    {
        if ( ( *pChar != '.' ) && ( i !=aString.getLength( ) ) )
            tmpBuffer[nCharCounter++] = *pChar;
        else
        {
            tmpBuffer[nCharCounter] = '\0';
            nCharCounter = 0;
            bsByteSequence[nByteSeqCounter++] = atoi( tmpBuffer );
        }
        pChar++;
    }
    return bsByteSequence;
}

/** print a socket result name.
*/
inline void printSocketResult( oslSocketResult eResult )
{
    printf( "#printSocketResult# " );
    if (!eResult)
    switch (eResult)
    {
        case osl_Socket_Ok:
            printf( "client connected\n");
            break;
        case osl_Socket_Error:
            printf("got an error ... exiting\r\n\r\n" );
            break;
        case osl_Socket_TimedOut:
            printf( "timeout\n");
            break;
    }
}

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
        saTargetSocketAddr( aHostIp1, IP_PORT_MYPORT9 ),
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


/** Server Socket Thread, served as a temp little server to communicate with client.
*/
class ServerSocketThread : public Thread
{
protected:
    oslThreadIdentifier m_id;

    void SAL_CALL run( )
    {
        ::osl::AcceptorSocket asAcceptorSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
        ::osl::SocketAddr saLocalSocketAddr( aHostIp1, IP_PORT_MYPORT9 );
        ::osl::StreamSocket ssStreamConnection;

        //socket addr can be reused, since multiple users run test on the same machine
        asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, sal_True);
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
            sal_uInt32 nReadNumber1 = ssStreamConnection.recv( pReadBuffer, 11 );
            sal_uInt32 nReadNumber2 = ssStreamConnection.recv( pReadBuffer + nReadNumber1, 11 );
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
class ClientReadSocketThread : public Thread
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
                  readCount = csConnectorSocket.read( buffer_read, FILE_READ );
              }
              else
                  printf( "# ClientReadSocketThread: connect failed! \n");
        //      terminate();
        //}
        csConnectorSocket.close();
        free( pTimeout );
    }

    void SAL_CALL onTerminated( )
    {
        //printf( "# normally terminate this thread %d!\n",  m_id );
    }

public:
    sal_Char buffer_read[FILE_READ];
    sal_uInt32 readCount;
    ClientReadSocketThread( ):
        saTargetSocketAddr( aHostIp1, IP_PORT_MYPORT10 ),
        csConnectorSocket( )
    {
        readCount = 0;
        m_id = getIdentifier( );
        //printf( "# successfully creat this client thread %d!\n",  m_id );
    }

    ~ClientReadSocketThread( )
    {
        if ( isRunning( ) )
            printf( "# error: client thread not terminated.\n" );
    }

};

/** Server Socket Thread, write a file which is large
*/
class ServerWriteSocketThread : public Thread
{
protected:
    oslThreadIdentifier m_id;

    void SAL_CALL run( )
    {
        ::osl::AcceptorSocket asAcceptorSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
        ::osl::SocketAddr saLocalSocketAddr( aHostIp1, IP_PORT_MYPORT10 );
        ::osl::StreamSocket ssStreamConnection;

        //socket addr can be reused, since multiple users run test on the same machine
        asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, sal_True);

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

            char * pSrcRoot = getenv("SRC_ROOT");
            // LLA: This is absolute wrong!
            // strcat( pSrcRoot, "/sal/inc/osl/file.hxx");
            rtl::OString sSrcRoot(pSrcRoot);
            sSrcRoot += "/sal/inc/osl/file.hxx";

            ::rtl::OUString sFilePath = ::rtl::OUString::createFromAscii( sSrcRoot.getStr() );
#ifdef WNT
            while (sFilePath.lastIndexOf('/') != -1)
                sFilePath = sFilePath.replace('/',(sal_Unicode)'\\');
#endif
            //printUString( sFilePath );
            ::rtl::OString aString = ::rtl::OUStringToOString( sFilePath, RTL_TEXTENCODING_ASCII_US );
            //::osl::File    testFile( sFilePath );
            FILE *stream;
            sal_uInt64     nCount_read;
            sal_Char       buffer_read[FILE_READ];

            if( (stream = fopen( aString.getStr( ), "r+t" )) != NULL )
            {
                /* Attempt to read in 25 characters */
                nCount_read = fread( buffer_read, sizeof( char ), FILE_READ, stream );
                fclose( stream );
            }
            else
                printf( "# File $SRC_ROOT/sal/inc/osl/file.hxx could not be opened\n" );

            sal_uInt32 nReadNumber1 = ssStreamConnection.write( buffer_read, nCount_read );
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
    ServerWriteSocketThread( )
    {
        m_id = getIdentifier( );
        //printf( "# successfully creat this server thread %d!\n",  m_id );
    }

    ~ServerWriteSocketThread( )
    {
        if ( isRunning( ) )
            printf( "# error: server thread not terminated.\n" );
    }
};

//------------------------------------------------------------------------
// tests cases begins here
//------------------------------------------------------------------------

namespace osl_SocketAddr
{

    /** testing the methods:
        inline SocketAddr();
        inline SocketAddr(const SocketAddr& Addr);
        inline SocketAddr(const oslSocketAddr , __osl_socket_NoCopy nocopy );
        inline SocketAddr(oslSocketAddr Addr);
        inline SocketAddr( const ::rtl::OUString& strAddrOrHostName, sal_Int32 nPort );
    */

    class ctors : public CppUnit::TestFixture
    {
    public:

        void ctors_none()
        {
            /// SocketAddr constructor.
            ::osl::SocketAddr saSocketAddr;

            CPPUNIT_ASSERT_MESSAGE("test for none parameter constructor function: check if the socket address was created successfully",
                                    sal_True == saSocketAddr.is( ) );
        }

        void ctors_copy()
        {
            /// SocketAddr copy constructor.
            ::osl::SocketAddr saSocketAddr( aHostName1, IP_PORT_HTTP1 );
            ::osl::SocketAddr saCopySocketAddr( saSocketAddr );

            sal_Int32 nPort = saCopySocketAddr.getPort( );

            CPPUNIT_ASSERT_MESSAGE("test for SocketAddr copy constructor function: copy constructor, do an action of copy construction then check the port with original set.",
                                    ( sal_True == saCopySocketAddr.is( ) ) && ( nPort == IP_PORT_HTTP1 ) );
        }

        void ctors_copy_no_001()
        {
#if 0
            ::osl::SocketAddr saSocketAddr( aHostName1, IP_PORT_HTTP1 );
            oslSocketAddr psaOSLSocketAddr = saSocketAddr.getHandle( );

            ::osl::SocketAddr saSocketAddrCopy( psaOSLSocketAddr, SAL_NO_COPY );
            saSocketAddrCopy.setPort( IP_PORT_HTTP2 );

            CPPUNIT_ASSERT_MESSAGE("test for SocketAddr no copy constructor function: do a no copy constructor on a given SocketAddr instance, modify the new instance's port, check the original one.",
                                    saSocketAddr.getPort( ) == IP_PORT_HTTP2 );
#endif
            ::osl::SocketAddr* pSocketAddr = new ::osl::SocketAddr( aHostName1, IP_PORT_HTTP1 );
            CPPUNIT_ASSERT_MESSAGE("check for new SocketAddr", pSocketAddr != NULL);

            oslSocketAddr psaOSLSocketAddr = pSocketAddr->getHandle( );

            ::osl::SocketAddr* pSocketAddrCopy = new ::osl::SocketAddr( psaOSLSocketAddr, SAL_NO_COPY );

            pSocketAddrCopy->setPort( IP_PORT_HTTP2 );
            CPPUNIT_ASSERT_MESSAGE("test for SocketAddr no copy constructor function: do a no copy constructor on a given SocketAddr instance, modify the new instance's port, check the original one.",
                                   pSocketAddr->getPort( ) == IP_PORT_HTTP2 );

            delete pSocketAddrCopy;
            // LLA: don't do this also:           delete pSocketAddr;
        }

        void ctors_copy_no_002()
        {
            ::osl::SocketAddr* pSocketAddr = new ::osl::SocketAddr( aHostName1, IP_PORT_HTTP1 );
                   CPPUNIT_ASSERT_MESSAGE("check for new SocketAddr", pSocketAddr != NULL);
                   oslSocketAddr psaOSLSocketAddr = pSocketAddr->getHandle( );
                   ::osl::SocketAddr* pSocketAddrCopy = new ::osl::SocketAddr( psaOSLSocketAddr, SAL_NO_COPY );

                   CPPUNIT_ASSERT_MESSAGE("test for SocketAddr no copy constructor function: do a no copy constructor on a given SocketAddr instance, modify the new instance's port, check the original one.",
                        pSocketAddr->getHandle( ) ==  pSocketAddrCopy->getHandle( ) );

                   delete pSocketAddrCopy;
        }

        void ctors_copy_handle_001()
        {
            ::osl::SocketAddr saSocketAddr( aHostName1, IP_PORT_HTTP1 );
            ::osl::SocketAddr saSocketAddrCopy( saSocketAddr.getHandle( ) );

            CPPUNIT_ASSERT_MESSAGE("test for SocketAddr copy handle constructor function: copy another Socket's handle, get its port to check copy effect.",
                                    saSocketAddrCopy.getPort( ) == IP_PORT_HTTP1 );
        }

        void ctors_copy_handle_002()
        {
            ::osl::SocketAddr saSocketAddr( aHostName1, IP_PORT_HTTP1 );
            ::osl::SocketAddr saSocketAddrCopy( saSocketAddr.getHandle( ) );
            saSocketAddrCopy.setPort( IP_PORT_HTTP2 );

            CPPUNIT_ASSERT_MESSAGE("test for SocketAddr copy handle constructor function: copy another Socket's handle, the original one should not be changed.",
                                    saSocketAddr.getPort( ) != IP_PORT_HTTP2 );
        }

        void ctors_hostname_port_001()
        {
            /// tcpip-specif constructor.
            ::osl::SocketAddr saSocketAddr( aHostIp1, IP_PORT_FTP );
            printUString(saSocketAddr.getHostname( ), "ctors_hostname_port_001:getHostname");

            CPPUNIT_ASSERT_MESSAGE("test for SocketAddr tcpip specif constructor function: do a constructor using tcpip spec, check the result.",
                                    saSocketAddr.is( ) == sal_True &&
                                    ( saSocketAddr.getPort( ) == IP_PORT_FTP )/*&&
                                    ( sal_True == compareUString( saSocketAddr.getHostname( ), aHostName1 ) ) */);
        }

        //same as is_002
        void ctors_hostname_port_002()
        {
            /// tcpip-specif constructor.
            ::osl::SocketAddr saSocketAddr( aHostIpInval1, IP_PORT_MYPORT2 );

            CPPUNIT_ASSERT_MESSAGE("test for SocketAddr tcpip specif constructor function: using an invalid IP address, the socketaddr ctors should fail", sal_False == saSocketAddr.is( ));
        }
        CPPUNIT_TEST_SUITE( ctors );
        CPPUNIT_TEST( ctors_none );
        CPPUNIT_TEST( ctors_copy );
        CPPUNIT_TEST( ctors_copy_no_001 );
        CPPUNIT_TEST( ctors_copy_no_002 );
        CPPUNIT_TEST( ctors_copy_handle_001 );
        CPPUNIT_TEST( ctors_copy_handle_002 );
        CPPUNIT_TEST( ctors_hostname_port_001 );
        CPPUNIT_TEST( ctors_hostname_port_002 );
        CPPUNIT_TEST_SUITE_END();

    }; // class ctors


    /** testing the method:
        inline sal_Bool is() const;
    */

    class is : public CppUnit::TestFixture
    {
    public:
        void is_001()
        {
            ::osl::SocketAddr saSocketAddr;

            CPPUNIT_ASSERT_MESSAGE("test for is() function: create an unknown type socket, it should be True when call is.",
                                    sal_True == saSocketAddr.is( ) );
        }
        // refer to setPort_003()
        void is_002()
        {
            ::osl::SocketAddr saSocketAddr( aHostIp1, IP_PORT_INVAL );

            CPPUNIT_ASSERT_MESSAGE("test for is() function: create a tcp-ip socket using invalid port number",
                                    sal_True == saSocketAddr.is( ) );
        }

        void is_003()
        {
            ::osl::SocketAddr saSocketAddr( aHostIpInval1, IP_PORT_MYPORT );

            CPPUNIT_ASSERT_MESSAGE("test for is() function: create a tcp-ip socket using invalid Ip number",
                                    sal_True != saSocketAddr.is( ) );
        }

        CPPUNIT_TEST_SUITE( is );
        CPPUNIT_TEST( is_001 );
        CPPUNIT_TEST( is_002 );
        CPPUNIT_TEST( is_003 );
        CPPUNIT_TEST_SUITE_END();

    }; // class is


    /** testing the method:
        inline ::rtl::OUString SAL_CALL getHostname( oslSocketResult *pResult = 0 ) const;
    */

    class getHostname : public CppUnit::TestFixture
    {
    public:
        void setUp()
        {
        }

        void tearDown()
        {
        }

        void getHostname_001()
        {
            ::osl::SocketAddr saSocketAddr( aHostIp5, IP_PORT_FTP );
            rtl::OUString suResult = saSocketAddr.getHostname( 0 );
            rtl::OUString suError = outputError(suResult, aHostName5, "test for getHostname(0)");

            CPPUNIT_ASSERT_MESSAGE( suError, sal_True == compareUString( suResult, aHostName5 ) );
        }

        void getHostname_002()
        {
            ::osl::SocketAddr saSocketAddr( aHostIp1, IP_PORT_FTP );
            sal_Bool bOK = saSocketAddr.setHostname( aHostName5 );
            CPPUNIT_ASSERT_MESSAGE("#SocketAddr.setHostname failed", sal_True == bOK );
            rtl::OUString suResult = saSocketAddr.getHostname( 0 );
            rtl::OUString suError = outputError(suResult, aHostName5, "test for getHostname(0)");

            CPPUNIT_ASSERT_MESSAGE( suError,
                                    sal_True == compareUString( suResult, aHostName5 ) );
        }

        CPPUNIT_TEST_SUITE( getHostname );
        CPPUNIT_TEST( getHostname_001 );
        CPPUNIT_TEST( getHostname_002 );
        CPPUNIT_TEST_SUITE_END();

    }; // class getHostname


    /** testing the method:
        inline sal_Int32 SAL_CALL getPort() const;
    */

    class getPort : public CppUnit::TestFixture
    {
    public:
        void getPort_001()
        {
            ::osl::SocketAddr saSocketAddr( aHostIp1, IP_PORT_FTP );

            CPPUNIT_ASSERT_MESSAGE( "test for getPort() function: get a normal port number.",
                                    IP_PORT_FTP == saSocketAddr.getPort( ) );
        }

        void getPort_002()
        {
            ::osl::SocketAddr saSocketAddr( aHostIp2, IP_PORT_INVAL );

            //printf("#getPort_002: Port number is %d \n", saSocketAddr.getPort( ));

            CPPUNIT_ASSERT_MESSAGE( "test for getPort( ) function: give an invalid port to a SocketAddr, get the port to see if it can detect. it did not pass in (W32).",
                                    saSocketAddr.getPort( )>=1 && saSocketAddr.getPort( ) <= 65535 );
        }

        CPPUNIT_TEST_SUITE( getPort );
        CPPUNIT_TEST( getPort_001 );
        CPPUNIT_TEST( getPort_002 );
        CPPUNIT_TEST_SUITE_END( );

    }; // class getPort


    /** testing the method:
        inline sal_Bool SAL_CALL setPort( sal_Int32 nPort );
    */

    class setPort : public CppUnit::TestFixture
    {
    public:
        void setPort_001()
        {
            ::osl::SocketAddr saSocketAddr( aHostIp1, IP_PORT_FTP );
            sal_Bool bOK = saSocketAddr.setPort( IP_PORT_TELNET );

            CPPUNIT_ASSERT_MESSAGE( "test for setPort() function: modify a port number setting, and check it.",
                                    ( sal_True == bOK ) &&
                                    ( IP_PORT_TELNET == saSocketAddr.getPort( ) ) );
        }

        void setPort_002()
        {
            ::osl::SocketAddr saSocketAddr( aHostIp4, IP_PORT_FTP );
            sal_Bool bOK = saSocketAddr.setPort( IP_PORT_ZERO );
            //on Linux, getPort() return 0
            //printf("#Port number is %d \n", saSocketAddr.getPort( ));

            CPPUNIT_ASSERT_MESSAGE( "test for setPort() function: port number should be in 1 ~ 65535, set port 0, it should be converted to a port number between 1024~5000. it did not pass in (W32, Linux).",
                                    ( ( 1024 <= saSocketAddr.getPort( ) ) && ( 5000 >= saSocketAddr.getPort( ) ) && ( bOK == sal_True ) ) ||
                                     bOK == sal_False );

        }

        void setPort_003()
        {
            ::osl::SocketAddr saSocketAddr( aHostIp1, IP_PORT_FTP);
            sal_Bool bOK = saSocketAddr.setPort( IP_PORT_INVAL );
            //on Linux, getPort return 34463
            //printf("#Port number is %d \n", saSocketAddr.getPort( ));

            CPPUNIT_ASSERT_MESSAGE( "test for setPort( ) function: set an address with invalid port. it should return error or convert it to a valid port.",
                                     ( ( 1 <= saSocketAddr.getPort( ) ) && ( 65535 >= saSocketAddr.getPort( ) ) &&( bOK == sal_True ) ) ||
                                     bOK == sal_False);
        }

        CPPUNIT_TEST_SUITE( setPort );
        CPPUNIT_TEST( setPort_001 );
        CPPUNIT_TEST( setPort_002 );
        CPPUNIT_TEST( setPort_003 );
        CPPUNIT_TEST_SUITE_END( );

    }; // class setPort


    /**  tester comment:

        In the following two functions, it use ::rtl::ByteSequence as an intermediate storage for address,
        the ByteSequence object can hold sal_Int8 arrays, which is raged [-127, 127], in case of IP addr
        that is greater than 127, say 129.158.217.202, it will stored as -127, -98, -39, -54,  it is unique
        in the range of sal_Int8, but lack of readability.
        so may be a sal_uInt8 array is better.
    */


    /** testing the method:
        inline sal_Bool SAL_CALL setAddr( const ::rtl::ByteSequence & address );
    */

    class setAddr : public CppUnit::TestFixture
    {
    public:
        void setAddr_001()
        {
            ::osl::SocketAddr saSocketAddr( aHostIp2, IP_PORT_FTP );
            sal_Bool bOK1 = saSocketAddr.setAddr( UStringIPToByteSequence( aHostIp1 ) );
            ::rtl::ByteSequence bsSocketAddr = saSocketAddr.getAddr( 0 );
            sal_Bool bOK = sal_False;

             if ( ( bsSocketAddr[0] == 127 ) && ( bsSocketAddr[1] == 0 ) && ( bsSocketAddr[2] == 0 ) && ( bsSocketAddr[3] == 1 ) )
                 bOK = sal_True;

            CPPUNIT_ASSERT_MESSAGE( "test for setAddr() function: construct Addr with  \"129.158.217.202\", set it to \"127.0.0.1\",  and check the correctness ",
                                      sal_True == bOK );
        }


        CPPUNIT_TEST_SUITE( setAddr );
        CPPUNIT_TEST( setAddr_001 );
        CPPUNIT_TEST_SUITE_END( );

    }; // class setAddr


    /** testing the method:
        inline ::rtl::ByteSequence  SAL_CALL getAddr( oslSocketResult *pResult = 0 ) const;
    */

    class getAddr : public CppUnit::TestFixture
    {
    public:
#ifdef UNX
        void getAddr_001()
        {
            CPPUNIT_ASSERT_MESSAGE( "test for getAddr() function: construct a socketaddr with IP assigned, get the address to check correctness, this is not pass in (LINUX,SOLARIS), it is core dump in the pSocketResult assignment.",
                                      sal_False );
        }
#else
        void getAddr_001()
        {
            //oslSocketResult *pSocketResult = NULL;
            oslSocketResult SocketResult;
            ::osl::SocketAddr saSocketAddr( aHostIp1, IP_PORT_FTP );
            ::rtl::ByteSequence bsSocketAddr = saSocketAddr.getAddr( &SocketResult );

            sal_Bool bOK = sal_False;

            if ( ( osl_Socket_Ok == SocketResult ) &&( bsSocketAddr[0] == 127 ) && ( bsSocketAddr[1] == 0 ) &&( bsSocketAddr[2] == 0 ) && ( bsSocketAddr[3] == 1 ) )
                 bOK = sal_True;

//          delete pSocketResult;

            CPPUNIT_ASSERT_MESSAGE( "test for getAddr() function: construct a socketaddr with IP assigned, get the address to check correctness.Caught unknown exception on (Win32)",
                                      sal_True == bOK );
        }
#endif

        CPPUNIT_TEST_SUITE( getAddr );
        CPPUNIT_TEST( getAddr_001 );
        CPPUNIT_TEST_SUITE_END( );

    }; // class getAddr


    /** testing the methods:
        inline SocketAddr & SAL_CALL operator= (oslSocketAddr Addr);
        inline SocketAddr & SAL_CALL operator= (const SocketAddr& Addr);
        inline SocketAddr & SAL_CALL assign( oslSocketAddr Addr, __osl_socket_NoCopy nocopy );
        inline sal_Bool SAL_CALL operator== (oslSocketAddr Addr) const;
        inline sal_Bool SAL_CALL operator== (const SocketAddr & Addr) const;    /// not implemented.
    */

    class operator_equal : public CppUnit::TestFixture
    {
    public:
        void operator_equal_001()
        {
            ::osl::SocketAddr saSocketAddr( aHostIp1, IP_PORT_TELNET);
            ::osl::SocketAddr saSocketAddrEqual( aHostIp2, IP_PORT_FTP );

            saSocketAddrEqual = saSocketAddr;
            sal_Bool bOK = sal_False;
            ::rtl::ByteSequence bsSocketAddr = saSocketAddrEqual.getAddr( 0 );

             if ( ( IP_PORT_TELNET == saSocketAddrEqual.getPort( ) ) &&( bsSocketAddr[0] == 127 ) && ( bsSocketAddr[1] == 0 ) &&( bsSocketAddr[2] == 0 ) && ( bsSocketAddr[3] == 1 ) )
                 bOK = sal_True;

            CPPUNIT_ASSERT_MESSAGE( "test for operator_equal() function: use operator= to assign Ip1 to Ip2, check its modification.",
                                      sal_True == bOK );
        }


        void operator_equal_002()
        {
            ::osl::SocketAddr saSocketAddr( aHostIp3, IP_PORT_TELNET);
            ::osl::SocketAddr saSocketAddrEqual( aHostIp2, IP_PORT_FTP );

            saSocketAddrEqual = saSocketAddr;
            CPPUNIT_ASSERT_MESSAGE( "after assign, the assigned SocketAddr is not same as the original Addr",
                                     IP_PORT_TELNET == saSocketAddrEqual.getPort( )  );
            saSocketAddrEqual.setPort( IP_PORT_MYPORT3 );
            saSocketAddr.setPort( IP_PORT_HTTP2 );

            CPPUNIT_ASSERT_MESSAGE( "test for operator_equal() function: perform an equal action, then try to change the original address's port. it should not be changed ( handle released), it did not pass in (W32), this is under discussion.",
                                     IP_PORT_MYPORT3 == saSocketAddrEqual.getPort( )  );
        }

        void operator_equal_const_001()
        {
            const ::osl::SocketAddr saSocketAddr( aHostIp1, IP_PORT_TELNET);
            ::osl::SocketAddr saSocketAddrEqual( aHostIp2, IP_PORT_FTP );

            saSocketAddrEqual = saSocketAddr;
            sal_Bool bOK = sal_False;
            ::rtl::ByteSequence bsSocketAddr = saSocketAddrEqual.getAddr( 0 );

             if ( ( IP_PORT_TELNET == saSocketAddrEqual.getPort( ) ) &&( bsSocketAddr[0] == 127 ) && ( bsSocketAddr[1] == 0 ) &&( bsSocketAddr[2] == 0 ) && ( bsSocketAddr[3] == 1 ) )
                 bOK = sal_True;

            CPPUNIT_ASSERT_MESSAGE( "test for operator_equal_const() function: use operator= const to assign Ip1 to Ip2, verify the change on the second one.",
                                      sal_True == bOK );
        }

        void operator_equal_const_002()
        {
            const ::osl::SocketAddr saSocketAddr( aHostIp1, IP_PORT_TELNET);
            ::osl::SocketAddr saSocketAddrEqual( aHostIp2, IP_PORT_FTP );

            saSocketAddrEqual = saSocketAddr;
            saSocketAddrEqual.setPort( IP_PORT_HTTP1 );

            CPPUNIT_ASSERT_MESSAGE( "test for operator_equal_const() function: change the second instance, the first one should not be altered, since it does not released the handle.",
                                      IP_PORT_HTTP1 != saSocketAddr.getPort( ) );
        }

        void operator_equal_assign_001()
        {
            ::osl::SocketAddr* pSocketAddr = new ::osl::SocketAddr( aHostIp1, IP_PORT_TELNET );
                   CPPUNIT_ASSERT_MESSAGE("check for new SocketAddr", pSocketAddr != NULL);
                   ::osl::SocketAddr* pSocketAddrAssign = new ::osl::SocketAddr( aHostIp2, IP_PORT_FTP );
                   oslSocketAddr poslSocketAddr = pSocketAddr->getHandle( );
                   //if( m_handle ) osl_destroySocketAddr( m_handle ); so pSocketAddrAssign had been destroyed and then point to pSocketAddr
                   pSocketAddrAssign->assign(poslSocketAddr, SAL_NO_COPY);

                   CPPUNIT_ASSERT_MESSAGE("test for SocketAddr no copy constructor function: do a no copy constructor on a given SocketAddr instance, modify the new instance's port, check the original one.",
                        pSocketAddrAssign->getPort( ) == IP_PORT_TELNET );

                   delete pSocketAddrAssign;
        }

        void operator_is_equal_001()
        {
            ::osl::SocketAddr saSocketAddr( aHostIp1, IP_PORT_TELNET);
            ::osl::SocketAddr saSocketAddrequal( aHostIp1, IP_PORT_TELNET );

            CPPUNIT_ASSERT_MESSAGE( "test for operator_equal_equal() function: check two identical Address.",
                                      sal_True == ( saSocketAddrequal == saSocketAddr.getHandle( ) ) );
        }

        void operator_is_equal_002()
        {
            ::osl::SocketAddr saSocketAddr( aHostIp2, IP_PORT_FTP);
            ::osl::SocketAddr saSocketAddrequal( aHostIp1, IP_PORT_TELNET );

            CPPUNIT_ASSERT_MESSAGE( "test for operator_equal_equal() function: check two different Address.",
                                      sal_False == ( saSocketAddrequal == saSocketAddr.getHandle( ) ) );
        }

        CPPUNIT_TEST_SUITE( operator_equal );
        CPPUNIT_TEST( operator_equal_001 );
        CPPUNIT_TEST( operator_equal_002 );
        CPPUNIT_TEST( operator_equal_const_001 );
        CPPUNIT_TEST( operator_equal_const_002 );
        CPPUNIT_TEST( operator_equal_assign_001 );
        CPPUNIT_TEST( operator_is_equal_001 );
        CPPUNIT_TEST( operator_is_equal_002 );
        CPPUNIT_TEST_SUITE_END( );

    }; // class operator_equal



    /** testing the method:
        inline oslSocketAddr SAL_CALL getHandle() const;
    */

    class getSocketAddrHandle : public CppUnit::TestFixture
    {
    public:

        void getSocketAddrHandle_001()
        {
            ::osl::SocketAddr* pSocketAddr = new ::osl::SocketAddr( aHostName1, IP_PORT_HTTP1 );
                   CPPUNIT_ASSERT_MESSAGE("check for new SocketAddr", pSocketAddr != NULL);
                   oslSocketAddr psaOSLSocketAddr = pSocketAddr->getHandle( );
                   ::osl::SocketAddr* pSocketAddrCopy = new ::osl::SocketAddr( psaOSLSocketAddr, SAL_NO_COPY );

                   CPPUNIT_ASSERT_MESSAGE("test for SocketAddr no copy constructor function: do a no copy constructor on a given SocketAddr instance, modify the new instance's port, check the original one.",
                        pSocketAddr->getHandle( ) ==  pSocketAddrCopy->getHandle( ) );

                   delete pSocketAddrCopy;
        }

        void getSocketAddrHandle_002()
        {
            ::osl::SocketAddr saSocketAddr( aHostName3, IP_PORT_MYPORT4 );
            oslSocketAddr poslSocketAddr = saSocketAddr.getHandle( );

            sal_Bool bOK = ( saSocketAddr == poslSocketAddr );
            //printf("getSocketAddrHandle_002\n");
            CPPUNIT_ASSERT_MESSAGE( "test for getHandle() function: use getHandle() function as an intermediate way to create identical address.",
                                      sal_True == bOK );
        }

        CPPUNIT_TEST_SUITE( getSocketAddrHandle );
        CPPUNIT_TEST( getSocketAddrHandle_001 );
        CPPUNIT_TEST( getSocketAddrHandle_002 );
        CPPUNIT_TEST_SUITE_END( );

    }; // class getSocketAddrHandle


    /** testing the method:
        static inline ::rtl::OUString SAL_CALL getLocalHostname( oslSocketResult *pResult = 0);
    */

    class getLocalHostname : public CppUnit::TestFixture
    {
    public:
        void getLocalHostname_001()
        {
            oslSocketResult *pResult = NULL;
            //printSocketResult(*pResult);
            ::rtl::OUString suResult = ::osl::SocketAddr::getLocalHostname( pResult );
            ::rtl::OUString suError = outputError(suResult, getThisHostname( ),
                "test for getLocalHostname() function");

            CPPUNIT_ASSERT_MESSAGE( suError,
                sal_True == compareUString( suResult, getThisHostname( ) ) );
        }

        CPPUNIT_TEST_SUITE( getLocalHostname );
        CPPUNIT_TEST( getLocalHostname_001 );
        CPPUNIT_TEST_SUITE_END( );

    }; // class getLocalHostname


    /** testing the method:
        static inline void SAL_CALL resolveHostname( const ::rtl::OUString & strHostName , SocketAddr & Addr );
    */

    class resolveHostname : public CppUnit::TestFixture
    {
    public:
        void resolveHostname_001()
        {
            ::osl::SocketAddr saSocketAddr;
            ::osl::SocketAddr::resolveHostname( aHostIp1, saSocketAddr );
            ::rtl::ByteSequence bsSocketAddr = saSocketAddr.getAddr( 0 );
            sal_Bool bOK = sal_False;

             if ( ( bsSocketAddr[0] == 127 ) && ( bsSocketAddr[1] == 0 ) &&( bsSocketAddr[2] == 0 ) && ( bsSocketAddr[3] == 1 ) )
                 bOK = sal_True;

            CPPUNIT_ASSERT_MESSAGE( "test for resolveHostname() function: try to resolve localhost to 127.0.0.1.",
                                      sal_True == bOK );
        }

        CPPUNIT_TEST_SUITE( resolveHostname );
        CPPUNIT_TEST( resolveHostname_001 );
        CPPUNIT_TEST_SUITE_END( );

    }; // class resolveHostname


    /** testing the method:
        static inline sal_Int32 SAL_CALL gettheServicePort(
            const ::rtl::OUString& strServiceName,
            const ::rtl::OUString & strProtocolName= ::rtl::OUString::createFromAscii( "tcp" ) );
    */

    class gettheServicePort : public CppUnit::TestFixture
    {
    public:
        void gettheServicePort_001()
        {
            CPPUNIT_ASSERT_MESSAGE( "test for gettheServicePort() function: try to get ftp service port on TCP protocol.",
                                      IP_PORT_FTP== ::osl::SocketAddr::getServicePort( aServiceFTP, aProtocolTCP ) );
        }

        void gettheServicePort_002()
        {
            CPPUNIT_ASSERT_MESSAGE( "test for gettheServicePort() function: try to get telnet service port on TCP protocol.",
                                      IP_PORT_TELNET== ::osl::SocketAddr::getServicePort( aServiceTELNET, aProtocolTCP ) );
        }

        void gettheServicePort_003()
        {
        //Solaris has no service called "https", please see /etc/services
            CPPUNIT_ASSERT_MESSAGE( "test for gettheServicePort() function: try to get netbios-ssn service port on UDP protocol.",
                                      IP_PORT_NETBIOS_DGM == ::osl::SocketAddr::getServicePort( aServiceNETBIOS, aProtocolUDP ) );
        }

        CPPUNIT_TEST_SUITE( gettheServicePort );
        CPPUNIT_TEST( gettheServicePort_001 );
        CPPUNIT_TEST( gettheServicePort_002 );
        CPPUNIT_TEST( gettheServicePort_003 );
        CPPUNIT_TEST_SUITE_END( );

    }; // class gettheServicePort

// -----------------------------------------------------------------------------


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_SocketAddr::ctors, "osl_SocketAddr");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_SocketAddr::is, "osl_SocketAddr");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_SocketAddr::getHostname, "osl_SocketAddr");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_SocketAddr::getPort, "osl_SocketAddr");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_SocketAddr::setPort, "osl_SocketAddr");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_SocketAddr::setAddr, "osl_SocketAddr");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_SocketAddr::getAddr, "osl_SocketAddr");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_SocketAddr::operator_equal, "osl_SocketAddr");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_SocketAddr::getSocketAddrHandle, "osl_SocketAddr");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_SocketAddr::getLocalHostname, "osl_SocketAddr");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_SocketAddr::resolveHostname, "osl_SocketAddr");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_SocketAddr::gettheServicePort, "osl_SocketAddr");


} // namespace osl_SocketAddr



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
            ::osl::Socket sSocket();

            CPPUNIT_ASSERT_MESSAGE( "test for ctors_none constructor function: check if the socket was created successfully, if no exception occured",
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

        CPPUNIT_TEST_SUITE( ctors );
        CPPUNIT_TEST( ctors_none );
        CPPUNIT_TEST( ctors_acquire );
        CPPUNIT_TEST( ctors_no_acquire );
        CPPUNIT_TEST( ctors_copy_ctor );
        CPPUNIT_TEST( ctors_TypeRaw );
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
                                    1 == 1 );
        }


        CPPUNIT_TEST_SUITE( close );
        CPPUNIT_TEST( close_001 );
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
            ::osl::SocketAddr saBindSocketAddr( aHostIp1, IP_PORT_MYPORT8 );
            ::osl::SocketAddr saLocalSocketAddr;

            sSocket.setOption( osl_Socket_OptionReuseAddr, sal_True);

            sal_Bool bOK1 = sSocket.bind( saBindSocketAddr );
            CPPUNIT_ASSERT_MESSAGE( "Socket bind fail.", sal_True == bOK1 );

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
            ::osl::SocketAddr saBindSocketAddr( aHostIp1, IP_PORT_MYPORT7 );  // aHostIp1 localhost
            ::osl::SocketAddr saLocalSocketAddr;

            sSocket.setOption( osl_Socket_OptionReuseAddr, sal_True);

            sal_Bool bOK1 = sSocket.bind( saBindSocketAddr );
            CPPUNIT_ASSERT_MESSAGE( "Socket bind fail.", sal_True == bOK1 );
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
            sal_Bool bOK;
            ::osl::SocketAddr saBindSocketAddr( aHostIpInval, IP_PORT_TELNET);
#ifdef WNT
            ::osl::Socket sSocket(sHandle);
            sSocket.setOption( osl_Socket_OptionReuseAddr, sal_True);
            sSocket.bind( saBindSocketAddr );
            //Invalid IP, so bind should fail
            ::rtl::OUString suError = outputError(::rtl::OUString::valueOf(sSocket.getLocalPort( )),
                ::rtl::OUString::valueOf((sal_Int32)OSL_INVALID_PORT),
                "test for getLocalPort function: first create a new socket, then an invalid socket address, bind them, and check the port assigned.");
            bOK = ( OSL_INVALID_PORT == sSocket.getLocalPort( ) );
#else
            //on Unix, if Addr is not an address of type osl_Socket_FamilyInet, it returns OSL_INVALID_PORT
            ::rtl::OUString suError = ::rtl::OUString::createFromAscii( "on Unix, if Addr is not an address of type osl_Socket_FamilyInet, it returns OSL_INVALID_PORT, but can not create Addr of that case");
#endif
            CPPUNIT_ASSERT_MESSAGE( suError, sal_False );

        }

        void getLocalPort_003()
        {
            ::osl::Socket sSocket(sHandle);
            ::osl::SocketAddr saBindSocketAddr( getLocalIP(), IP_PORT_INVAL);

            sSocket.setOption( osl_Socket_OptionReuseAddr, sal_True);

            sal_Bool bOK1 = sSocket.bind( saBindSocketAddr );

            ::rtl::OUString suError = outputError(::rtl::OUString::valueOf(sSocket.getLocalPort( )),
                ::rtl::OUString::createFromAscii("34463"),
                "test for getLocalPort function: first create a new socket, then an invalid socket address, bind them, and check the port assigned");
            sal_Bool bOK = ( sSocket.getLocalPort( ) >= 1 &&  sSocket.getLocalPort( ) <= 65535);

            CPPUNIT_ASSERT_MESSAGE( suError, sal_True == bOK );
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
            ::osl::SocketAddr saBindSocketAddr( aHostIp1, IP_PORT_MYPORT6 );

            sSocket.setOption( osl_Socket_OptionReuseAddr, sal_True);

            sal_Bool bOK1 = sSocket.bind( saBindSocketAddr );
            CPPUNIT_ASSERT_MESSAGE( "Socket bind fail.", sal_True == bOK1 );
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
            CPPUNIT_ASSERT_MESSAGE( suError, sal_True == bOK );
        }

        void getLocalHost_002()
        {
            ::osl::Socket sSocket(sHandle);
            ::osl::SocketAddr saBindSocketAddr( aHostIpInval, IP_PORT_POP3);
            ::osl::SocketAddr saLocalSocketAddr;

            sSocket.setOption( osl_Socket_OptionReuseAddr, sal_True);
            sal_Bool bOK1 = sSocket.bind( saBindSocketAddr );
            //Invalid IP, so bind should fail
            sal_Bool bOK = compareUString( sSocket.getLocalHost( ), aNullURL ) ;
            ::rtl::OUString suError = outputError(sSocket.getLocalHost( ), aNullURL, "test for getLocalHost function: getLocalHost with invalid SocketAddr");

            CPPUNIT_ASSERT_MESSAGE( suError, sal_True == bOK );
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
            ::osl::SocketAddr saLocalSocketAddr( aHostIp1, IP_PORT_MYPORT );
            ::osl::SocketAddr saTargetSocketAddr( aHostIp1, IP_PORT_MYPORT );
            ::osl::SocketAddr saPeerSocketAddr( aHostIp2, IP_PORT_FTP );
            ::osl::StreamSocket ssConnection;
            asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, sal_True);
            /// launch server socket
            sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
            CPPUNIT_ASSERT_MESSAGE( "AcceptorSocket bind '127.0.0.1' address failed.", sal_True == bOK1 );
            sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
            CPPUNIT_ASSERT_MESSAGE( "AcceptorSocket listen failed.",  sal_True == bOK2 );

            asAcceptorSocket.enableNonBlockingMode( sal_True );
            oslSocketResult eResult = asAcceptorSocket.acceptConnection(ssConnection); /// waiting for incoming connection...

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

            sSocket.setOption( osl_Socket_OptionReuseAddr, sal_True);
            sal_Bool bOK1 = sSocket.bind( saBindSocketAddr );
            CPPUNIT_ASSERT_MESSAGE( "Socket bind fail.", sal_True == bOK1 );

            sal_Bool bOK2 = compareUString( sSocket.getLocalHost( ), saBindSocketAddr.getHostname( ) ) ;

            sSocket.close();
            CPPUNIT_ASSERT_MESSAGE( "test for bind function: bind a valid address.", sal_True == bOK2 );
        }

        void bind_002()
        {
            ::osl::Socket sSocket(sHandle);
            ::osl::SocketAddr saBindSocketAddr( aHostIpInval, IP_PORT_NETBIOS );
            ::osl::SocketAddr saLocalSocketAddr;

            sSocket.setOption( osl_Socket_OptionReuseAddr, sal_True);
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
            ::osl::SocketAddr saLocalSocketAddr( aHostIp1, IP_PORT_MYPORT1 );
            ::osl::SocketAddr saTargetSocketAddr( aHostIp1, IP_PORT_MYPORT1 );
            ::osl::SocketAddr saPeerSocketAddr( aHostIp2, IP_PORT_FTP );
            ::osl::StreamSocket ssConnection;
            /// launch server socket
            asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, sal_True);
            sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
            CPPUNIT_ASSERT_MESSAGE( "AcceptorSocket bind address failed.", sal_True == bOK1 );
            sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
            CPPUNIT_ASSERT_MESSAGE( "AcceptorSocket listen failed.",  sal_True == bOK2 );
            asAcceptorSocket.enableNonBlockingMode( sal_True );
            oslSocketResult eResult = asAcceptorSocket.acceptConnection(ssConnection); /// waiting for incoming connection...

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
            ::osl::SocketAddr saLocalSocketAddr( aHostIp1, IP_PORT_MYPORT );
            ::osl::SocketAddr saTargetSocketAddr( aHostIp1, IP_PORT_MYPORT );
            ::osl::SocketAddr saPeerSocketAddr( aHostIp2, IP_PORT_FTP );
            ::osl::StreamSocket ssConnection;

            /// launch server socket
            asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, sal_True);
            sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
            CPPUNIT_ASSERT_MESSAGE( "AcceptorSocket bind address failed.", sal_True == bOK1 );
            sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
            CPPUNIT_ASSERT_MESSAGE( "AcceptorSocket listen failed.",  sal_True == bOK2 );
            asAcceptorSocket.enableNonBlockingMode( sal_True );
            oslSocketResult eResult = asAcceptorSocket.acceptConnection(ssConnection); /// waiting for incoming connection...

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

        void getOption_002()
        {
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeDgram, osl_Socket_ProtocolIp );
            ::osl::Socket sSocket(sHandle);

            sal_Bool * pbDebug = ( sal_Bool * )malloc( sizeof ( sal_Bool ) );
            sSocket.getOption( osl_Socket_OptionDebug,  pbDebug, sizeof ( sal_Bool ) );
            sal_Bool bOK = ( sal_False  ==  *pbDebug );
            free( pbDebug );


            CPPUNIT_ASSERT_MESSAGE( "test for getOption function: get debug option of socket.",
                                    sal_True == bOK );
        }

        void getOption_003()
        {
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeDgram, osl_Socket_ProtocolIp );
            ::osl::Socket sSocket(sHandle);

            sal_Bool * pbDontRoute = ( sal_Bool * )malloc( sizeof ( sal_Bool ) );
            sSocket.getOption( osl_Socket_OptionDontRoute,  pbDontRoute, sizeof ( sal_Bool ) );
            sal_Bool bOK = ( sal_False  ==  *pbDontRoute );
            free( pbDontRoute );


            CPPUNIT_ASSERT_MESSAGE( "test for getOption function: get debug option of socket.",
                                    sal_True == bOK );
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
        CPPUNIT_TEST( getOption_002 );
        CPPUNIT_TEST( getOption_003 );
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
            sal_Bool  b1 = asAcceptorSocket.setOption( osl_Socket_OptionDontRoute,  pbDontRouteSet, sizeof ( sal_Int32 ) );
            CPPUNIT_ASSERT_MESSAGE( "setOption function failed.", ( sal_True == b1 ) );
            sal_Int32 n2 = asAcceptorSocket.getOption( osl_Socket_OptionDontRoute,  pGetBuffer, nBufferLen );
            CPPUNIT_ASSERT_MESSAGE( "getOption function failed.", ( n2 == nBufferLen ) );

            sal_Bool bOK = ( 1  ==  *pGetBuffer );
            //printf("#setOption_001: getOption is %d \n", *pGetBuffer);

            // toggle check, set to 0
            *pbDontRouteSet = 0;

            sal_Bool  b3 = asAcceptorSocket.setOption( osl_Socket_OptionDontRoute,  pbDontRouteSet, sizeof ( sal_Int32 ) );
            CPPUNIT_ASSERT_MESSAGE( "setOption function failed.", ( sal_True == b3 ) );
            sal_Int32 n4 = asAcceptorSocket.getOption( osl_Socket_OptionDontRoute,  pGetBuffer, nBufferLen );
            CPPUNIT_ASSERT_MESSAGE( "getOption (DONTROUTE) function failed.", ( n2 == nBufferLen ) );

            sal_Bool bOK2 = ( 0  ==  *pGetBuffer );
            //printf("#setOption_001: getOption is %d \n", *pGetBuffer);

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

            sal_Bool b1 = asAcceptorSocket.setOption( osl_Socket_OptionLinger,  &aLingerSet, nBufferLen );

            sal_Int32 n1 = asAcceptorSocket.getOption( osl_Socket_OptionLinger,  &aLingerGet, nBufferLen );
            CPPUNIT_ASSERT_MESSAGE( "getOption (SO_LINGER) function failed.", ( n1 == nBufferLen ) );

            //printf("#setOption_002: getOption is %d \n", aLingerGet.l_linger);
            sal_Bool bOK = ( 7  ==  aLingerGet.l_linger );
            // free( pbLingerSet );
            // free( pbLingerGet );

        }

        void setOption_simple_001()
        {
            /// set and get option.
            asAcceptorSocket.setOption( osl_Socket_OptionDontRoute,  sal_True );
            sal_Bool bOK = ( sal_True  ==  asAcceptorSocket.getOption( osl_Socket_OptionDontRoute ) );

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
        CPPUNIT_TEST( setOption_001 );
        CPPUNIT_TEST( setOption_002 );
        CPPUNIT_TEST( setOption_simple_001 );
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
            ::osl::SocketAddr saLocalSocketAddr( aHostIp1, IP_PORT_MYPORT );
            ::osl::StreamSocket ssConnection;

            /// launch server socket
            asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, sal_True);
            sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
            CPPUNIT_ASSERT_MESSAGE( "AcceptorSocket bind address failed.", sal_True == bOK1 );
            sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
            CPPUNIT_ASSERT_MESSAGE( "AcceptorSocket listen failed.",  sal_True == bOK2 );
            asAcceptorSocket.enableNonBlockingMode( sal_True );
            oslSocketResult eResult = asAcceptorSocket.acceptConnection(ssConnection); /// waiting for incoming connection...

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
            ::osl::SocketAddr saLocalSocketAddr( aHostIp1, IP_PORT_MYPORT );
            ::osl::StreamSocket ssConnection;

            /// launch server socket
            asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, sal_True);
            sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
            CPPUNIT_ASSERT_MESSAGE( "AcceptorSocket bind address failed.", sal_True == bOK1 );
            sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
            CPPUNIT_ASSERT_MESSAGE( "AcceptorSocket listen failed.",  sal_True == bOK2 );

            sal_Bool bOK3 = asAcceptorSocket.isNonBlockingMode( );
            asAcceptorSocket.enableNonBlockingMode( sal_True );
             oslSocketResult eResult = asAcceptorSocket.acceptConnection(ssConnection); /// waiting for incoming connection...

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
            ::osl::SocketAddr saBindSocketAddr( aHostIpInval, IP_PORT_HTTP2 );
            ::osl::SocketAddr saLocalSocketAddr;
            sSocket.setOption( osl_Socket_OptionReuseAddr, sal_True);
            sal_Bool bOK1 = sSocket.bind( saBindSocketAddr );//build an error "osl_Socket_E_AddrNotAvail"
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
            ::osl::SocketAddr saBindSocketAddr( aHostIp1, IP_PORT_FTP );
            ::osl::SocketAddr saLocalSocketAddr;

            CPPUNIT_ASSERT_MESSAGE( "test for getError function: should get no error.",
                                    osl_Socket_E_None == sSocket.getError( )  );
        }

        void getError_002()
        {
            ::osl::Socket sSocket(sHandle);
            ::osl::SocketAddr saBindSocketAddr( aHostIpInval, IP_PORT_FTP );
            ::osl::SocketAddr saLocalSocketAddr;
            sSocket.setOption( osl_Socket_OptionReuseAddr, sal_True);
            sSocket.bind( saBindSocketAddr );//build an error "osl_Socket_E_AddrNotAvail"
            //on Solaris, the error no is EACCES, but it has no mapped value, so getError() returned osl_Socket_E_InvalidError.
            //while on Linux, the error no is EADDRNOTAVAIL getError returned osl_Socket_E_AddrNotAvail.

            CPPUNIT_ASSERT_MESSAGE( "trick an error called sSocket.getError( ), check the getError result.Failed on Solaris, returned osl_Socket_E_InvalidError because no entry to map the errno EACCES. Passed on Linux & Win32",
                                    osl_Socket_E_AddrNotAvail == sSocket.getError( )  );
        }

        void getErrorAsString_001()
        {
            ::osl::Socket sSocket(sHandle);
            ::osl::SocketAddr saBindSocketAddr( aHostIpInval, IP_PORT_FTP );
            ::osl::SocketAddr saLocalSocketAddr;
            sSocket.setOption( osl_Socket_OptionReuseAddr, sal_True);
            sSocket.bind( saBindSocketAddr );//build an error "osl_Socket_E_AddrNotAvail"

            CPPUNIT_ASSERT_MESSAGE( "test for getErrorAsString function: trick an error called sSocket.getError( ), check the getError result.",
                                    sal_True == compareUString( sSocket.getErrorAsString( ) , "osl_Socket_E_AddrNotAvail"  ) );
        }

        void getErrorAsString_002()
        {
            ::osl::Socket sSocket(sHandle);
            ::osl::SocketAddr saBindSocketAddr( aHostIp1, IP_PORT_FTP );
            ::osl::SocketAddr saLocalSocketAddr;

            CPPUNIT_ASSERT_MESSAGE( "test for getErrorAsString function: trick an error called sSocket.getError( ), check the getError result.",
                                    sal_True == compareUString( sSocket.getErrorAsString( ) , "osl_Socket_E_None"  ) );
        }


        CPPUNIT_TEST_SUITE( getError );
        CPPUNIT_TEST( getError_001 );
        CPPUNIT_TEST( getError_002 );
        CPPUNIT_TEST( getErrorAsString_001 );
        CPPUNIT_TEST( getErrorAsString_002 );
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


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Socket::ctors, "osl_Socket");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Socket::operators, "osl_Socket");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Socket::close, "osl_Socket");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Socket::getLocalAddr, "osl_Socket");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Socket::getLocalPort, "osl_Socket");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Socket::getLocalHost, "osl_Socket");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Socket::getPeer, "osl_Socket");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Socket::bind, "osl_Socket");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Socket::isRecvReady, "osl_Socket");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Socket::isSendReady, "osl_Socket");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Socket::getType, "osl_Socket");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Socket::getOption, "osl_Socket");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Socket::setOption, "osl_Socket");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Socket::enableNonBlockingMode, "osl_Socket");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Socket::isNonBlockingMode, "osl_Socket");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Socket::clearError, "osl_Socket");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Socket::getError, "osl_Socket");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Socket::getHandle, "osl_Socket");

} // namespace osl_Socket



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
        void write_read()
        {
            //client sent two strings, and server received, check the order and value
            ServerWriteSocketThread myServerThread;
            ClientReadSocketThread myClientThread;
            myServerThread.create( );
            myClientThread.create( );

            //wait until the thread terminate
            myClientThread.join( );
            myServerThread.join( );
            //Maximum Packet Size is ( ARPANET, MILNET = 1007 Ethernet (10Mb) = 1500
            // Proteon PRONET  = 2046), so here test read 4000 bytes
            sal_uInt32 nLength = myClientThread.readCount ;
            CPPUNIT_ASSERT_MESSAGE(" test for write/read a file with two threads: the file is about 50kb, send data from server, check readed data in Client.",
                                    nLength >= 3000);
        }


        CPPUNIT_TEST_SUITE( send_recv );
        CPPUNIT_TEST( write_read );
        CPPUNIT_TEST( send_recv1 );
//      CPPUNIT_TEST( write_read );
        CPPUNIT_TEST_SUITE_END();
    }; // class send_recv

// -----------------------------------------------------------------------------

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_StreamSocket::ctors, "osl_StreamSocket");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_StreamSocket::send_recv, "osl_StreamSocket");

} // namespace osl_StreamSocket


namespace osl_ConnectorSocket
{

    /** testing the method:
        ConnectorSocket(oslAddrFamily Family = osl_Socket_FamilyInet,
                        oslProtocol Protocol = osl_Socket_ProtocolIp,
                        oslSocketType   Type = osl_Socket_TypeStream);
    */

    class ctors : public CppUnit::TestFixture
    {
    public:
        void ctors_001()
        {
            /// Socket constructor.
            ::osl::ConnectorSocket csSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );

            CPPUNIT_ASSERT_MESSAGE( "test for ctors_001 constructor function: check if the connector socket was created successfully.",
                                    osl_Socket_TypeStream ==  csSocket.getType( ) );
        }

        CPPUNIT_TEST_SUITE( ctors );
        CPPUNIT_TEST( ctors_001 );
        CPPUNIT_TEST_SUITE_END();

    }; // class ctors

    /** testing the method:
        oslSocketResult SAL_CALL connect(const SocketAddr& TargetHost, const TimeValue* pTimeout = 0);
    */

    class connect : public CppUnit::TestFixture
    {
    public:
        TimeValue *pTimeout;
        ::osl::AcceptorSocket asAcceptorSocket;
        ::osl::ConnectorSocket csConnectorSocket;


        // initialization
        void setUp( )
        {
            pTimeout  = ( TimeValue* )malloc( sizeof( TimeValue ) );
            pTimeout->Seconds = 3;
            pTimeout->Nanosec = 0;
        //  sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        }

        void tearDown( )
        {
            free( pTimeout );
        //  sHandle = NULL;
            asAcceptorSocket.close( );
            csConnectorSocket.close( );
        }


        void connect_001()
        {
            ::osl::SocketAddr saLocalSocketAddr( aHostIp1, IP_PORT_MYPORT2 );
            ::osl::SocketAddr saTargetSocketAddr( aHostIp1, IP_PORT_MYPORT2 );
            ::osl::SocketAddr saPeerSocketAddr( aHostIp2, IP_PORT_FTP );
            ::osl::StreamSocket ssConnection;

            /// launch server socket
            asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, sal_True);
            sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
            CPPUNIT_ASSERT_MESSAGE( "AcceptorSocket bind address failed.", sal_True == bOK1 );
            sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
            CPPUNIT_ASSERT_MESSAGE( "AcceptorSocket listen failed.",  sal_True == bOK2 );

            asAcceptorSocket.enableNonBlockingMode( sal_True );
            asAcceptorSocket.acceptConnection(ssConnection); /// waiting for incoming connection...

            /// launch client socket
            oslSocketResult eResult = csConnectorSocket.connect( saTargetSocketAddr, pTimeout );   /// connecting to server...

            /// get peer information
            csConnectorSocket.getPeerAddr( saPeerSocketAddr );/// connected.

            CPPUNIT_ASSERT_MESSAGE( "test for connect function: try to create a connection with remote host. and check the setup address.",
                                    ( sal_True == compareSocketAddr( saPeerSocketAddr, saLocalSocketAddr ) ) &&
                                    ( osl_Socket_Ok == eResult ));
        }

        CPPUNIT_TEST_SUITE( connect );
        CPPUNIT_TEST( connect_001 );
        CPPUNIT_TEST_SUITE_END();

    }; // class connect


// -----------------------------------------------------------------------------

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_ConnectorSocket::ctors, "osl_ConnectorSocket");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_ConnectorSocket::connect, "osl_ConnectorSocket");

} // namespace osl_ConnectorSocket



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
            /// Socket constructor.
            ::osl::AcceptorSocket asSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );

            CPPUNIT_ASSERT_MESSAGE( "test for ctors_001 constructor function: check if the acceptor socket was created successfully.",
                                    osl_Socket_TypeStream ==  asSocket.getType( ) );
        }

        CPPUNIT_TEST_SUITE( ctors );
        CPPUNIT_TEST( ctors_001 );
        CPPUNIT_TEST_SUITE_END();

    }; // class ctors

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


        // initialization
        void setUp( )
        {
            pTimeout  = ( TimeValue* )malloc( sizeof( TimeValue ) );
            pTimeout->Seconds = 3;
            pTimeout->Nanosec = 0;
            asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, sal_True);
        //  sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        }

        void tearDown( )
        {
            free( pTimeout );
        //  sHandle = NULL;
            asAcceptorSocket.close( );
            csConnectorSocket.close( );
        }


        void listen_accept_001()
        {
            ::osl::SocketAddr saLocalSocketAddr( aHostIp1, IP_PORT_MYPORT3 );
            ::osl::SocketAddr saTargetSocketAddr( aHostIp1, IP_PORT_MYPORT3 );
            ::osl::StreamSocket ssConnection;

            /// launch server socket
            sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
            CPPUNIT_ASSERT_MESSAGE( "AcceptorSocket bind address failed.", sal_True == bOK1 );
            sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
            CPPUNIT_ASSERT_MESSAGE( "AcceptorSocket listen failed.",  sal_True == bOK2 );
            asAcceptorSocket.enableNonBlockingMode( sal_True );

            /// launch client socket
            csConnectorSocket.connect( saTargetSocketAddr, pTimeout );   /// connecting to server...

            oslSocketResult eResult = asAcceptorSocket.acceptConnection(ssConnection); /// waiting for incoming connection...

            CPPUNIT_ASSERT_MESSAGE( "test for listen_accept function: try to create a connection with remote host, using listen and accept.",
                                    ( sal_True == bOK2 ) &&
                                    ( osl_Socket_Ok == eResult ) );
        }

        void listen_accept_002()
        {
            ::osl::SocketAddr saLocalSocketAddr( aHostIp1, IP_PORT_MYPORT4 );
            ::osl::SocketAddr saTargetSocketAddr( aHostIp1, IP_PORT_MYPORT4 );
            ::osl::SocketAddr saPeerSocketAddr( aHostIp2, IP_PORT_FTP );
            ::osl::StreamSocket ssConnection;

            /// launch server socket
            sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
            CPPUNIT_ASSERT_MESSAGE( "AcceptorSocket bind address failed.", sal_True == bOK1 );
            sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
            CPPUNIT_ASSERT_MESSAGE( "AcceptorSocket listen failed.",  sal_True == bOK2 );
            asAcceptorSocket.enableNonBlockingMode( sal_True );

            /// launch client socket
            csConnectorSocket.connect( saTargetSocketAddr, pTimeout );   /// connecting to server...

            oslSocketResult eResult = asAcceptorSocket.acceptConnection(ssConnection, saPeerSocketAddr); /// waiting for incoming connection...

            CPPUNIT_ASSERT_MESSAGE( "test for listen_accept function: try to create a connection with remote host, using listen and accept, accept with peer address.",
                                    ( sal_True == bOK2 ) &&
                                    ( osl_Socket_Ok == eResult ) &&
                                    ( sal_True == compareSocketAddr( saPeerSocketAddr, saLocalSocketAddr ) ) );
        }

        CPPUNIT_TEST_SUITE( listen_accept );
        CPPUNIT_TEST( listen_accept_001 );
        CPPUNIT_TEST( listen_accept_002 );
        CPPUNIT_TEST_SUITE_END();

    }; // class listen_accept


// -----------------------------------------------------------------------------

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_AcceptorSocket::ctors, "osl_AcceptorSocket");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_AcceptorSocket::listen_accept, "osl_AcceptorSocket");

} // namespace osl_AcceptorSocket


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
            ::osl::StreamSocket ssSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeDgram );

            CPPUNIT_ASSERT_MESSAGE( "test for ctors_001 constructor function: check if the datagram socket was created successfully.",
                                    osl_Socket_TypeDgram ==  ssSocket.getType( ) );
        }


        CPPUNIT_TEST_SUITE( ctors );
        CPPUNIT_TEST( ctors_001 );
        CPPUNIT_TEST_SUITE_END();

    }; // class ctors

// -----------------------------------------------------------------------------

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_DatagramSocket::ctors, "osl_DatagramSocket");

} // namespace osl_DatagramSocket


// -----------------------------------------------------------------------------

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
NOADDITIONAL;
