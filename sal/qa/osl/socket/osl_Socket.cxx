/*************************************************************************
 *
 *  $RCSfile: osl_Socket.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-08-07 15:11:40 $
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


/** print a UNI_CODE String.
*/
inline void printUString( const ::rtl::OUString & str )
{
    rtl::OString aString;

    printf( "#printUString_u# " );
    aString = ::rtl::OUStringToOString( str, RTL_TEXTENCODING_ASCII_US );
    printf( "%s\n", aString.getStr( ) );
}

/** get the local host name.
*/
inline ::rtl::OUString getHost( void )
{
    struct hostent *hptr;

    hptr = gethostbyname( "localhost" );
    CPPUNIT_ASSERT_MESSAGE( "#In getHostname function, error on gethostbyname()",  hptr != NULL );
    ::rtl::OUString aUString = ::rtl::OUString::createFromAscii( (const sal_Char *) hptr->h_name );

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
    oslThreadIdentifier m_id, m_CurId;
    ::osl::SocketAddr saTargetSocketAddr;
    ::osl::ConnectorSocket csConnectorSocket;

    void SAL_CALL run( )
    {
        TimeValue *pTimeout;

        pTimeout->Seconds = 5;
        pTimeout->Nanosec = 0;

        /// if the thread should terminate, schedule return false
        while ( schedule( ) == sal_True )
            ;//csConnectorSocket.connect( saTargetSocketAddr, pTimeout );
    }

    void SAL_CALL onTerminated( )
    {
        printf( "# normally terminate this thread %d!\n",  m_id );
    }

public:

    ClientSocketThread( ):
        saTargetSocketAddr( aHostIp1, IP_PORT_MYPORT ),
        csConnectorSocket( )
    {
        m_id = getIdentifier( );
        m_CurId = getCurrentIdentifier( );

        printf( "# successfully creat this thread %d!\n",  m_id );
    }

    ~ClientSocketThread( )
    {
        if ( isRunning( ) )
            printf( "# error: not terminated.\n" );
    }

};


/** Server Socket Thread, served as a temp little server to communicate with client.
*/
class ServerSocketThread : public Thread
{
protected:
    oslThreadIdentifier m_id, m_CurId;

    void SAL_CALL run( )
    {
        ::osl::AcceptorSocket asAcceptorSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
        ::osl::SocketAddr saLocalSocketAddr( aHostIp1, IP_PORT_MYPORT );
        ::osl::SocketAddr saPeerSocketAddr;
        ::osl::StreamSocket ssConnection;

        sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
        sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
        CPPUNIT_ASSERT(  ( sal_True == bOK1 ) && ( sal_True == bOK2 ) );
        asAcceptorSocket.enableNonBlockingMode( sal_True );
        sal_Char pReadBuffer[5];
            oslSocketResult eResult = asAcceptorSocket.acceptConnection(ssConnection);

        /// if the thread should terminate, schedule return false
        while ( schedule( ) == sal_True )
        {
            while( ssConnection.read( pReadBuffer, 3 ) > 0 )
                continue;

            //printf("#reading from stream socket....\n");
            //ssConnection.recv( pReadBuffer, 3 );
            printf( "#read buffer content.%s\n", pReadBuffer );
            break;
        }

    }

    void SAL_CALL onTerminated( )
    {
        printf( "# normally terminate this server thread %d!\n",  m_id );
    }

public:

    ServerSocketThread( )
    {
        m_id = getIdentifier( );
        m_CurId = getCurrentIdentifier( );

        printf( "# successfully creat this thread %d!\n",  m_id );
    }

    ~ServerSocketThread( )
    {
        if ( isRunning( ) )
            printf( "# error: not terminated.\n" );
    }

};


 /** launch a new client.
*/
inline void launchClientSocket( void )
{
    ClientSocketThread *pCSockThread = new ClientSocketThread;
    sal_Bool bOK = pCSockThread->create();
    CPPUNIT_ASSERT_MESSAGE(  "#Error in launchClientSocket(), cann not creates a new thread!\n",  sal_True == bOK );


    pCSockThread->terminate( );
    pCSockThread->join( );
    delete pCSockThread;

}

 /** launch a new server.
*/
inline void launchServerSocket( void )
{
    ServerSocketThread *pSSockThread = new ServerSocketThread;
    sal_Bool bOK = pSSockThread->create();
    CPPUNIT_ASSERT_MESSAGE(  "#Error in launchServerSocket(), can not creates a new thread!\n",  sal_True == bOK );

    pSSockThread->terminate( );
    pSSockThread->join( );
    delete pSSockThread;
}

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

            CPPUNIT_ASSERT_MESSAGE("test for ctors_none constructor function: check if the socket address was created successfully",
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
            ::osl::SocketAddr saSocketAddr( aHostName1, IP_PORT_HTTP1 );
            oslSocketAddr psaOSLSocketAddr = saSocketAddr.getHandle( );

            ::osl::SocketAddr saSocketAddrCopy( psaOSLSocketAddr, SAL_NO_COPY );
            saSocketAddrCopy.setPort( IP_PORT_HTTP2 );

            CPPUNIT_ASSERT_MESSAGE("test for SocketAddr no copy constructor function: do a no copy constructor on a given SocketAddr instance, modify the new instance's port, check the original one.",
                                    saSocketAddr.getPort( ) == IP_PORT_HTTP2 );
        }

#ifdef WNT
        void ctors_copy_no_002()
        {
            ::osl::SocketAddr saSocketAddr( aHostName1, IP_PORT_HTTP1 );
            {
                ::osl::SocketAddr saSocketAddrCopy( saSocketAddr.getHandle( ), SAL_NO_COPY );
            }

            CPPUNIT_ASSERT_MESSAGE("test for SocketAddr no copy constructor function: do a no copy constructor, destroy the new instance, check the existence of original one, it did not pass in (W32), this test is under discussion.",
                                    NULL == saSocketAddr.getHandle( ) );
        }
#else
        void ctors_copy_no_002()
        {
            CPPUNIT_ASSERT_MESSAGE("test for SocketAddr no copy constructor function: do a no copy constructor, destroy the new instance, check the existence of original one, it did not pass in (W32), this test is under discussion.",
                                    sal_True );
        }
#endif


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

            CPPUNIT_ASSERT_MESSAGE("test for SocketAddr tcpip specif constructor function: do a constructor using tcpip spec, check the result.",
                                    saSocketAddr.is( ) &&
                                    ( saSocketAddr.getPort( ) == IP_PORT_FTP )/*&&
                                    ( sal_True == compareUString( saSocketAddr.getHostname( ), aHostName1 ) ) */);
        }

        void ctors_hostname_port_002()
        {
            /// tcpip-specif constructor.
            ::osl::SocketAddr saSocketAddr( aHostIp1, IP_PORT_INVAL );

            CPPUNIT_ASSERT_MESSAGE("test for SocketAddr tcpip specif constructor function: using an invalid port number, the socket should be invalid, it did not pas in (W32).",
                                    sal_False == saSocketAddr.is( ) );
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

        void is_002()
        {
            ::osl::SocketAddr saSocketAddr( aHostIp1, IP_PORT_INVAL );

            CPPUNIT_ASSERT_MESSAGE("test for is() function: create a tcp-ip socket using invalid port number",
                                    sal_True != saSocketAddr.is( ) );
        }

        CPPUNIT_TEST_SUITE( is );
        CPPUNIT_TEST( is_001 );
        CPPUNIT_TEST( is_002 );
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
            ::osl::SocketAddr saSocketAddr( aHostIp1, IP_PORT_FTP );

            CPPUNIT_ASSERT_MESSAGE( "test for getHostname() function: ",
                                    sal_True == compareUString( saSocketAddr.getHostname( 0 ), getHost( ) ) );
        }

        void getHostname_002()
        {
            ::osl::SocketAddr saSocketAddr( aHostIp1, IP_PORT_FTP );
            sal_Bool bOK = saSocketAddr.setHostname( aHostName1 );
            CPPUNIT_ASSERT( sal_True == bOK );

            CPPUNIT_ASSERT_MESSAGE( "test for getHostname( ) function: ",
                                    sal_True == compareUString( saSocketAddr.getHostname( 0 ), aHostName1 ) );
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
            ::osl::SocketAddr saSocketAddr( aHostIp1, IP_PORT_INVAL );

            CPPUNIT_ASSERT_MESSAGE( "test for getPort( ) function: give an invalid port to a SocketAddr, get the port to see if it can detect. it did not pass in (W32).",
                                    OSL_INVALID_PORT == saSocketAddr.getPort( ) );
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
            ::osl::SocketAddr saSocketAddr( aHostIp1, IP_PORT_FTP );
            sal_Bool bOK = saSocketAddr.setPort( IP_PORT_ZERO );

            CPPUNIT_ASSERT_MESSAGE( "test for setPort() function: port number should be in 1 ~ 65535, set port t 0, it should be converted to a port number between 1024~5000. it did not pass in (W32).",
                                    ( ( 1024 <= saSocketAddr.getPort( ) ) && ( 5000 >= saSocketAddr.getPort( ) ) && ( bOK == sal_True ) ) ||
                                     bOK == sal_False );
        }

        void setPort_003()
        {
            ::osl::SocketAddr saSocketAddr( aHostIp1, IP_PORT_FTP);
            sal_Bool bOK = saSocketAddr.setPort( IP_PORT_INVAL );

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
            oslSocketResult *pSocketResult = NULL;
            ::osl::SocketAddr saSocketAddr( aHostIp1, IP_PORT_FTP );
            ::rtl::ByteSequence bsSocketAddr = saSocketAddr.getAddr( 0 );

            sal_Bool bOK = sal_False;

             if ( ( osl_Socket_Ok == *pSocketResult ) &&( bsSocketAddr[0] == 127 ) && ( bsSocketAddr[1] == 0 ) &&( bsSocketAddr[2] == 0 ) && ( bsSocketAddr[3] == 1 ) )
                 bOK = sal_True;

            CPPUNIT_ASSERT_MESSAGE( "test for getAddr() function: construct a socketaddr with IP assigned, get the address to check correctness.",
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
            ::osl::SocketAddr saSocketAddr( aHostIp1, IP_PORT_TELNET);
            ::osl::SocketAddr saSocketAddrEqual( aHostIp2, IP_PORT_FTP );

            saSocketAddrEqual = saSocketAddr;
            saSocketAddrEqual.setPort( IP_PORT_HTTP1 );
            saSocketAddr.setPort( IP_PORT_HTTP2 );

            CPPUNIT_ASSERT_MESSAGE( "test for operator_equal() function: perform an equal action, then try tochange the origial address's port. it should not be changed ( handle released), it did not pass in (W32), this is under discussion.",
                                     IP_PORT_HTTP2 != saSocketAddr.getPort( )  );
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
            const ::osl::SocketAddr saSocketAddr( aHostIp1, IP_PORT_TELNET);
            ::osl::SocketAddr saSocketAddrAssign( aHostIp2, IP_PORT_FTP );

            //saSocketAddrAssign.assign( saSocketAddr.getHandle(), SAL_NO_COPY );
            sal_Bool bOK = sal_False;
            ::rtl::ByteSequence bsSocketAddr = saSocketAddrAssign.getAddr( 0 );

             if ( ( IP_PORT_TELNET == saSocketAddrAssign.getPort( ) ) &&( bsSocketAddr[0] == 127 ) && ( bsSocketAddr[1] == 0 ) &&( bsSocketAddr[2] == 0 ) && ( bsSocketAddr[3] == 1 ) )
                 bOK = sal_True;

            CPPUNIT_ASSERT_MESSAGE( "test for operator_equal_assign() function: use operator= const to assign Ip1 to Ip2, verify the change on the second one, this is not passed on (W32), it will cause memory access violence.",
                                      sal_True == bOK );
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

    class getSocketHandle : public CppUnit::TestFixture
    {
    public:

#ifdef LINUX
    void getSocketHandle_001()
        {
            CPPUNIT_ASSERT_MESSAGE( "test for getHandle() function: use getHandle() function as an intermediate way to create identical address, it is not passed in (LINUX).",
                                      sal_False );
        }
#else
    void getSocketHandle_001()
        {
            ::osl::SocketAddr saSocketAddr( aHostName1, IP_PORT_HTTP1 );
            oslSocketAddr poslSocketAddr = saSocketAddr.getHandle( );
            ::osl::SocketAddr saSocketAddrCopy( poslSocketAddr, SAL_NO_COPY );

            ::rtl::ByteSequence bsSocketAddr = saSocketAddrCopy.getAddr( 0 );
            sal_Bool bOK = sal_False;

             if ( ( IP_PORT_HTTP1 == saSocketAddrCopy.getPort( ) ) &&( bsSocketAddr[0] == 127 ) && ( bsSocketAddr[1] == 0 ) &&( bsSocketAddr[2] == 0 ) && ( bsSocketAddr[3] == 1 ) )
                 bOK = sal_True;

            CPPUNIT_ASSERT_MESSAGE( "test for getHandle() function: use getHandle() function as an intermediate way to create identical address.",
                                      sal_True == bOK );
        }
#endif

#ifdef LINUX
        void getSocketHandle_002()
        {
            CPPUNIT_ASSERT_MESSAGE( "test for getHandle() function: use getHandle() function as an intermediate way to create identical address, it is not passed in (LINUX).",
                                      sal_False );
        }
#else
        void getSocketHandle_002()
        {
            ::osl::SocketAddr saSocketAddr( aHostName1, IP_PORT_HTTP1 );
            oslSocketAddr poslSocketAddr = saSocketAddr.getHandle( );

            sal_Bool bOK = ( saSocketAddr == poslSocketAddr );
            CPPUNIT_ASSERT_MESSAGE( "test for getHandle() function: use getHandle() function as an intermediate way to create identical address.",
                                      sal_True == bOK );
        }
#endif

        CPPUNIT_TEST_SUITE( getSocketHandle );
        CPPUNIT_TEST( getSocketHandle_001 );
        CPPUNIT_TEST( getSocketHandle_002 );
        CPPUNIT_TEST_SUITE_END( );

    }; // class getSocketHandle


    /** testing the method:
        static inline ::rtl::OUString SAL_CALL getLocalHostname( oslSocketResult *pResult = 0);
    */

    class getLocalHostname : public CppUnit::TestFixture
    {
    public:
        void getLocalHostname_001()
        {
            oslSocketResult *pResult = NULL;
            sal_Bool bOK = compareUString( ::osl::SocketAddr::getLocalHostname( pResult ), getHost( ) );

            CPPUNIT_ASSERT_MESSAGE( "test for getLocalHostname() function: I write a function to do the same thing compare the result with it.",
                                      sal_True == bOK );
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
            CPPUNIT_ASSERT_MESSAGE( "test for gettheServicePort() function: try to get https service port on UDP protocol.",
                                      IP_PORT_HTTPS== ::osl::SocketAddr::getServicePort( aServiceHTTPS, aProtocolUDP ) );
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
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_SocketAddr::getSocketHandle, "osl_SocketAddr");
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

        CPPUNIT_TEST_SUITE( ctors );
        CPPUNIT_TEST( ctors_none );
        CPPUNIT_TEST( ctors_acquire );
        CPPUNIT_TEST( ctors_no_acquire );
        CPPUNIT_TEST( ctors_copy_ctor );
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


        void getLocalAddr_001()
        {
            ::osl::Socket sSocket(sHandle);
            ::osl::SocketAddr saBindSocketAddr( aHostIp1, IP_PORT_FTP );
            ::osl::SocketAddr saLocalSocketAddr;

            sSocket.bind( saBindSocketAddr );
            sSocket.getLocalAddr( saLocalSocketAddr );
            sal_Bool bOK = compareUString( saLocalSocketAddr.getHostname( 0 ), getHost( ) ) ;

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
            ::osl::SocketAddr saBindSocketAddr( aHostIp1, IP_PORT_FTP );
            ::osl::SocketAddr saLocalSocketAddr;

            sSocket.bind( saBindSocketAddr );
            sal_Bool bOK = ( IP_PORT_FTP == sSocket.getLocalPort( )  );

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
            ::osl::Socket sSocket(sHandle);
            ::osl::SocketAddr saBindSocketAddr( aHostIpInval, IP_PORT_FTP);
            ::osl::SocketAddr saLocalSocketAddr;

            sSocket.bind( saBindSocketAddr );
            sal_Bool bOK = ( OSL_INVALID_PORT == sSocket.getLocalPort( ) );

            CPPUNIT_ASSERT_MESSAGE( "test for getLocalPort function: first create a new socket, then an invalid socket address, bind them, and check the port assigned.",
                                    sal_True == bOK );
        }

        CPPUNIT_TEST_SUITE( getLocalPort );
        CPPUNIT_TEST( getLocalPort_001 );
        CPPUNIT_TEST( getLocalPort_002 );
        CPPUNIT_TEST_SUITE_END();

    }; // class getLocalPort


    /** testing the method:
        inline ::rtl::OUString SAL_CALL getLocalHost() const;
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
            ::osl::SocketAddr saBindSocketAddr( aHostIp1, IP_PORT_FTP );
            ::osl::SocketAddr saLocalSocketAddr;

            sSocket.bind( saBindSocketAddr );
            sal_Bool bOK = compareUString( sSocket.getLocalHost( ), getHost( ) ) ;

            CPPUNIT_ASSERT_MESSAGE( "test for getLocalHost function: create localhost socket and check name.",
                                    sal_True == bOK );
        }

        void getLocalHost_002()
        {
            ::osl::Socket sSocket(sHandle);
            ::osl::SocketAddr saBindSocketAddr( aHostIpInval, IP_PORT_FTP);
            ::osl::SocketAddr saLocalSocketAddr;

            sSocket.bind( saBindSocketAddr );
            sal_Bool bOK = compareUString( sSocket.getLocalHost( ), aNullURL ) ;

            CPPUNIT_ASSERT_MESSAGE( "test for getLocalHost function: getLocalHost with invalid SocketAddr.",
                                    sal_True == bOK );
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

            /// launch server socket
            sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
            sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
            CPPUNIT_ASSERT(  ( sal_True == bOK1 ) && ( sal_True == bOK2 ) );
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
            ::osl::SocketAddr saBindSocketAddr( aHostIp1, IP_PORT_FTP );
            ::osl::SocketAddr saLocalSocketAddr;

            sal_Bool bOK1 = sSocket.bind( saBindSocketAddr );
            sal_Bool bOK2 = compareUString( sSocket.getLocalHost( ), getHost( ) ) ;

            CPPUNIT_ASSERT_MESSAGE( "test for bind function: bind a valid address.",
                                    ( sal_True == bOK1 ) && ( sal_True == bOK2 ) );
        }

        void bind_002()
        {
            ::osl::Socket sSocket(sHandle);
            ::osl::SocketAddr saBindSocketAddr( aHostIpInval, IP_PORT_FTP );
            ::osl::SocketAddr saLocalSocketAddr;

            sal_Bool bOK1 = sSocket.bind( saBindSocketAddr );
            sal_Bool bOK2 = compareUString( sSocket.getLocalHost( ), getHost( ) ) ;

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
            ::osl::SocketAddr saLocalSocketAddr( aHostIp1, IP_PORT_MYPORT );
            ::osl::SocketAddr saTargetSocketAddr( aHostIp1, IP_PORT_MYPORT );
            ::osl::SocketAddr saPeerSocketAddr( aHostIp2, IP_PORT_FTP );
            ::osl::StreamSocket ssConnection;

            /// launch server socket
            sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
            sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
            CPPUNIT_ASSERT(  ( sal_True == bOK1 ) && ( sal_True == bOK2 ) );
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
            sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
            sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
            CPPUNIT_ASSERT(  ( sal_True == bOK1 ) && ( sal_True == bOK2 ) );
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
        void getType_003()
        {
            CPPUNIT_ASSERT_MESSAGE( "test for getType function: get type of socket.this is not passed in (LINUX, SOLARIS), the osl_Socket_TypeRaw, type socket can not be created.",
                                    sal_False );
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
            sSocket.getOption( osl_Socket_OptionType,  pType, sizeof ( sal_Int32 ) );
            sal_Bool bOK = ( osl_Socket_TypeStream + 1  ==  *pType );
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

            sal_Bool * pbDontToute = ( sal_Bool * )malloc( sizeof ( sal_Bool ) );
            sSocket.getOption( osl_Socket_OptionDontRoute,  pbDontToute, sizeof ( sal_Bool ) );
            sal_Bool bOK = ( sal_False  ==  *pbDontToute );
            free( pbDontToute );


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
        oslSocket sHandle;
        TimeValue *pTimeout;
        ::osl::AcceptorSocket asAcceptorSocket;


        // initialization
        void setUp( )
        {
            sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        }

        void tearDown( )
        {
            sHandle = NULL;
            asAcceptorSocket.close( );
        }


        void setOption_001()
        {
            /// set and get option.
            sal_Bool * pbDontTouteSet = ( sal_Bool * )malloc( sizeof ( sal_Bool ) );
            *pbDontTouteSet = sal_True;
            sal_Bool * pbDontTouteGet = ( sal_Bool * )malloc( sizeof ( sal_Bool ) );
            asAcceptorSocket.setOption( osl_Socket_OptionDontRoute,  pbDontTouteSet, sizeof ( sal_Bool ) );
            asAcceptorSocket.getOption( osl_Socket_OptionDontRoute,  pbDontTouteGet, sizeof ( sal_Bool ) );

            sal_Bool bOK = ( sal_True  ==  *pbDontTouteGet );
            free( pbDontTouteSet );
            free( pbDontTouteGet );

            CPPUNIT_ASSERT_MESSAGE( "test for setOption function: set option of a socket and then check.",
                                      ( sal_True == bOK ) );
        }

        void setOption_002()
        {
            /// set and get option.
            sal_Int32 * pbLingerSet = ( sal_Int32 * )malloc( sizeof ( sal_Int32 ) );
            *pbLingerSet = 7;
            sal_Int32 * pbLingerGet = ( sal_Int32 * )malloc( sizeof ( sal_Int32 ) );
            asAcceptorSocket.setOption( osl_Socket_OptionLinger,  pbLingerSet, sizeof ( sal_Int32 ) );
            asAcceptorSocket.getOption( osl_Socket_OptionLinger,  pbLingerGet, sizeof ( sal_Int32 ) );

            sal_Bool bOK = ( 7  ==  *pbLingerGet );
            free( pbLingerSet );
            free( pbLingerGet );

            CPPUNIT_ASSERT_MESSAGE( "test for setOption function: set option of a socket and then check.",
                                      ( sal_True == bOK ) );
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
            asAcceptorSocket.setOption( osl_Socket_OptionLinger,  7 );
            sal_Bool bOK = ( 7  ==  asAcceptorSocket.getOption( osl_Socket_OptionLinger ) );

            CPPUNIT_ASSERT_MESSAGE( "test for setOption function: set option of a socket and then check.",
                                      ( sal_True == bOK ) );
        }

        CPPUNIT_TEST_SUITE( setOption );
        CPPUNIT_TEST( setOption_001 );
        CPPUNIT_TEST( setOption_002 );
        CPPUNIT_TEST( setOption_simple_001 );
        CPPUNIT_TEST( setOption_simple_002 );
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
            sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
            sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
            CPPUNIT_ASSERT(  ( sal_True == bOK1 ) && ( sal_True == bOK2 ) );
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
            sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
            sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
            CPPUNIT_ASSERT(  ( sal_True == bOK1 ) && ( sal_True == bOK2 ) );

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
            ::osl::SocketAddr saBindSocketAddr( aHostIpInval, IP_PORT_FTP );
            ::osl::SocketAddr saLocalSocketAddr;

            sal_Bool bOK1 = sSocket.bind( saBindSocketAddr );//build an error "osl_Socket_E_AddrNotAvail"
            sSocket.clearError( );

            CPPUNIT_ASSERT_MESSAGE( "test for clearError function: trick an error called sSocket.getError( ), and then clear the error states, check the result.",
                                    0 == sSocket.getError( )  );
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

            sSocket.bind( saBindSocketAddr );//build an error "osl_Socket_E_AddrNotAvail"

            CPPUNIT_ASSERT_MESSAGE( "test for getError function: trick an error called sSocket.getError( ), check the getError result.",
                                    osl_Socket_E_AddrNotAvail == sSocket.getError( )  );
        }

        void getErrorAsString_001()
        {
            ::osl::Socket sSocket(sHandle);
            ::osl::SocketAddr saBindSocketAddr( aHostIpInval, IP_PORT_FTP );
            ::osl::SocketAddr saLocalSocketAddr;

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

// -----------------------------------------------------------------------------

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_StreamSocket::ctors, "osl_StreamSocket");

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


        void connect_001()
        {
            ::osl::SocketAddr saLocalSocketAddr( aHostIp1, IP_PORT_MYPORT );
            ::osl::SocketAddr saTargetSocketAddr( aHostIp1, IP_PORT_MYPORT );
            ::osl::SocketAddr saPeerSocketAddr( aHostIp2, IP_PORT_FTP );
            ::osl::StreamSocket ssConnection;

            /// launch server socket
            sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
            sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
            CPPUNIT_ASSERT(  ( sal_True == bOK1 ) && ( sal_True == bOK2 ) );
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


        void ctors_001()
        {
            /// Socket constructor.
            ::osl::AcceptorSocket asSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );

            CPPUNIT_ASSERT_MESSAGE( "test for ctors_001 constructor function: check if the connector socket was created successfully.",
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


        void listen_accept_001()
        {
            ::osl::SocketAddr saLocalSocketAddr( aHostIp1, IP_PORT_MYPORT );
            ::osl::SocketAddr saTargetSocketAddr( aHostIp1, IP_PORT_MYPORT );
            ::osl::StreamSocket ssConnection;

            /// launch server socket
            sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
            sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
            CPPUNIT_ASSERT(  ( sal_True == bOK1 ) && ( sal_True == bOK2 ) );
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
            ::osl::SocketAddr saLocalSocketAddr( aHostIp1, IP_PORT_MYPORT );
            ::osl::SocketAddr saTargetSocketAddr( aHostIp1, IP_PORT_MYPORT );
            ::osl::SocketAddr saPeerSocketAddr( aHostIp2, IP_PORT_FTP );
            ::osl::StreamSocket ssConnection;

            /// launch server socket
            sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
            sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
            CPPUNIT_ASSERT(  ( sal_True == bOK1 ) && ( sal_True == bOK2 ) );
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
