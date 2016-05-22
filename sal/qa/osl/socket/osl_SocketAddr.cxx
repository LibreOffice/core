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

#define IP_PORT_ZERO   0
#define IP_PORT_FTP    21
#define IP_PORT_TELNET 23
#define IP_PORT_HTTP1  80
#define IP_PORT_HTTP2  8080

#define IP_PORT_MYPORT  8881    //8888
#define IP_PORT_MYPORT2  8883   //8890
#define IP_PORT_MYPORT3  8884   //8891
#define IP_PORT_INVAL  99999
#define IP_PORT_MYPORT4  8885   //8892
#define IP_PORT_NETBIOS_DGM  138


namespace osl_SocketAddr
{

    /** testing the methods:
        inline SocketAddr();
        inline SocketAddr(const SocketAddr& Addr);
        inline SocketAddr(const oslSocketAddr , __osl_socket_NoCopy nocopy );
        inline SocketAddr(oslSocketAddr Addr);
        inline SocketAddr( const ::rtl::OUString& strAddrOrHostName, sal_Int32 nPort );
    */

    class ctors : public ::testing::Test
    {
    public:
    }; // class ctors

    TEST_F(ctors, ctors_none)
    {
        /// SocketAddr constructor.
        ::osl::SocketAddr saSocketAddr;

        // oslSocketResult aResult;
        // rtl::OUString suHost = saSocketAddr.getLocalHostname( &aResult);

        // rtl::OUString suHost2 = getThisHostname();

        ASSERT_TRUE(sal_True == saSocketAddr.is( )) << "test for none parameter constructor function: check if the socket address was created successfully";
    }

    TEST_F(ctors, ctors_none_000)
    {
        /// SocketAddr constructor.
        ::osl::SocketAddr saSocketAddr;

        oslSocketResult aResult;
        rtl::OUString suHost = saSocketAddr.getLocalHostname( &aResult);
        rtl::OUString suHost2 = getThisHostname();

        sal_Bool bOk = compareUString(suHost, suHost2);

        rtl::OUString suError = rtl::OUString::createFromAscii("Host names should be the same. From SocketAddr.getLocalHostname() it is'");
        suError += suHost;
        suError += rtl::OUString::createFromAscii("', from getThisHostname() it is '");
        suError += suHost2;
        suError += rtl::OUString::createFromAscii("'.");

        ASSERT_TRUE(sal_True == bOk) << suError.pData;
    }

    TEST_F(ctors, ctors_copy)
    {
        /// SocketAddr copy constructor.
        ::osl::SocketAddr saSocketAddr( rtl::OUString::createFromAscii("localhost"), IP_PORT_HTTP1 );
        ::osl::SocketAddr saCopySocketAddr( saSocketAddr );

        sal_Int32 nPort = saCopySocketAddr.getPort( );

        ASSERT_TRUE(( sal_True == saCopySocketAddr.is( ) ) && ( nPort == IP_PORT_HTTP1 )) << "test for SocketAddr copy constructor function: copy constructor, do an action of copy construction then check the port with original set.";
    }

    TEST_F(ctors, ctors_copy_no_001)
    {
#if 0
        ::osl::SocketAddr saSocketAddr( rtl::OUString::createFromAscii("localhost"), IP_PORT_HTTP1 );
        oslSocketAddr psaOSLSocketAddr = saSocketAddr.getHandle( );

        ::osl::SocketAddr saSocketAddrCopy( psaOSLSocketAddr, SAL_NO_COPY );
        saSocketAddrCopy.setPort( IP_PORT_HTTP2 );

        ASSERT_TRUE(saSocketAddr.getPort( ) == IP_PORT_HTTP2) << "test for SocketAddr no copy constructor function: do a no copy constructor on a given SocketAddr instance, modify the new instance's port, check the original one.";
#endif
        ::osl::SocketAddr* pSocketAddr = new ::osl::SocketAddr( rtl::OUString::createFromAscii("localhost"), IP_PORT_HTTP1 );
        ASSERT_TRUE(pSocketAddr != NULL) << "check for new SocketAddr";

        oslSocketAddr psaOSLSocketAddr = pSocketAddr->getHandle( );

        ::osl::SocketAddr* pSocketAddrCopy = new ::osl::SocketAddr( psaOSLSocketAddr, SAL_NO_COPY );

        pSocketAddrCopy->setPort( IP_PORT_HTTP2 );
        ASSERT_TRUE(pSocketAddr->getPort( ) == IP_PORT_HTTP2) << "test for SocketAddr no copy constructor function: do a no copy constructor on a given SocketAddr instance, modify the new instance's port, check the original one.";

        delete pSocketAddrCopy;
        // LLA: don't do this also:           delete pSocketAddr;
    }

    TEST_F(ctors, ctors_copy_no_002)
    {
        ::osl::SocketAddr* pSocketAddr = new ::osl::SocketAddr( rtl::OUString::createFromAscii("localhost"), IP_PORT_HTTP1 );
            ASSERT_TRUE(pSocketAddr != NULL) << "check for new SocketAddr";
            oslSocketAddr psaOSLSocketAddr = pSocketAddr->getHandle( );
            ::osl::SocketAddr* pSocketAddrCopy = new ::osl::SocketAddr( psaOSLSocketAddr, SAL_NO_COPY );

            ASSERT_TRUE(pSocketAddr->getHandle( ) ==  pSocketAddrCopy->getHandle( )) << "test for SocketAddr no copy constructor function: do a no copy constructor on a given SocketAddr instance, modify the new instance's port, check the original one.";

            delete pSocketAddrCopy;
    }

    TEST_F(ctors, ctors_copy_handle_001)
    {
        ::osl::SocketAddr saSocketAddr( rtl::OUString::createFromAscii("localhost"), IP_PORT_HTTP1 );
        ::osl::SocketAddr saSocketAddrCopy( saSocketAddr.getHandle( ) );

        ASSERT_TRUE(saSocketAddrCopy.getPort( ) == IP_PORT_HTTP1) << "test for SocketAddr copy handle constructor function: copy another Socket's handle, get its port to check copy effect.";
    }

    TEST_F(ctors, ctors_copy_handle_002)
    {
        ::osl::SocketAddr saSocketAddr( rtl::OUString::createFromAscii("localhost"), IP_PORT_HTTP1 );
        ::osl::SocketAddr saSocketAddrCopy( saSocketAddr.getHandle( ) );
        saSocketAddrCopy.setPort( IP_PORT_HTTP2 );

        ASSERT_TRUE(saSocketAddr.getPort( ) != IP_PORT_HTTP2) << "test for SocketAddr copy handle constructor function: copy another Socket's handle, the original one should not be changed.";
    }

    TEST_F(ctors, ctors_hostname_port_001)
    {
        /// tcpip-specif constructor.
        ::osl::SocketAddr saSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_FTP );
        printUString( saSocketAddr.getHostname( ), "ctors_hostname_port_001:getHostname");

        ASSERT_TRUE(saSocketAddr.is( ) == sal_True &&
                                ( saSocketAddr.getPort( ) == IP_PORT_FTP )/*&&
                                ( sal_True == compareUString( saSocketAddr.getHostname( ), aHostName1 ) ) */) << "test for SocketAddr tcpip specific constructor function: do a constructor using tcpip spec, check the result.";
    }

    //same as is_002
    TEST_F(ctors, ctors_hostname_port_002)
    {
        /// tcpip-specif constructor.
        ::osl::SocketAddr saSocketAddr( rtl::OUString::createFromAscii("123.345.67.89"), IP_PORT_MYPORT2 );

        ASSERT_TRUE(sal_False == saSocketAddr.is( )) << "test for SocketAddr tcpip specific constructor function: using an invalid IP address, the socketaddr ctors should fail";
    }


    /** testing the method:
        inline sal_Bool is() const;
    */

    class is : public ::testing::Test
    {
    public:
    }; // class is

    TEST_F(is, is_001)
    {
        ::osl::SocketAddr saSocketAddr;

        ASSERT_TRUE(sal_True == saSocketAddr.is( )) << "test for is() function: create an unknown type socket, it should be True when call is.";
    }
    // refer to setPort_003()
    TEST_F(is, is_002)
    {
        ::osl::SocketAddr saSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_INVAL );

        ASSERT_TRUE(sal_True == saSocketAddr.is( )) << "test for is() function: create a tcp-ip socket using invalid port number";
    }

    TEST_F(is, is_003)
    {
        ::osl::SocketAddr saSocketAddr( rtl::OUString::createFromAscii("123.345.67.89"), IP_PORT_MYPORT );

        ASSERT_TRUE(sal_True != saSocketAddr.is( )) << "test for is() function: create a tcp-ip socket using invalid Ip number";
    }

    /** testing the method:
        inline ::rtl::OUString SAL_CALL getHostname( oslSocketResult *pResult = 0 ) const;
    */

    class getHostname : public ::testing::Test
    {
    public:
        void SetUp()
        {
        }

        void TearDown()
        {
        }
    }; // class getHostname

    TEST_F(getHostname, getHostname_000)
        {
            ::osl::SocketAddr saSocketAddr( rtl::OUString::createFromAscii("129.158.217.107"), IP_PORT_FTP );
            rtl::OUString suResult = saSocketAddr.getHostname( 0 );

        }

    /** it will search the Ip in current machine's /etc/hosts at first, if find, then return the
        mapped hostname, otherwise, it will search via DNS server, and often return hostname+ Domain name
        like "sceri.PRC.Sun.COM"
        The process is same as Socket::getLocalHost(), but getLocalHost can only return hostname of the current machine.
    */
    TEST_F(getHostname, getHostname_001)
    {
        ::osl::SocketAddr saSocketAddr( rtl::OUString::createFromAscii("129.158.217.107"), IP_PORT_FTP );
        rtl::OUString suResult = saSocketAddr.getHostname( 0 );
        rtl::OUString suError = outputError(suResult, rtl::OUString::createFromAscii("sceri.PRC.Sun.COM"), "test for getHostname(0)");
        sal_Bool bOK = compareUString( suResult, rtl::OUString::createFromAscii("sceri.PRC.Sun.COM") );
        // search the returned hostname in /etc/hosts, if find, and the IP in the row is same as IP
        // in the Addr, it's right also.
        if ( bOK == sal_False)
        {
            rtl::OString aString = ::rtl::OUStringToOString( suResult, RTL_TEXTENCODING_ASCII_US );
            if ( compareUString( getIPbyName( aString ), rtl::OUString::createFromAscii("129.158.217.107") ) == sal_True )
                bOK = sal_True;
        }
        ASSERT_TRUE(sal_True == bOK) << suError.pData;
    }

// LLA: now we have to control, if this behaviour is right.
// LLA: this function does not work in company (Linux, Windows) but at home
    TEST_F(getHostname, getHostname_002)
    {
        rtl::OUString suHostname = rtl::OUString::createFromAscii("cn-1.germany.sun.com");
        rtl::OString aString = ::rtl::OUStringToOString( suHostname, RTL_TEXTENCODING_ASCII_US );
        rtl::OUString aHostIP    = getIPbyName( aString );

        ::osl::SocketAddr saSocketAddr( rtl::OUString::createFromAscii("localhost"), IP_PORT_FTP );
        sal_Bool bOK = saSocketAddr.setHostname( suHostname );
        ASSERT_TRUE(sal_True == bOK) << "#SocketAddr.setHostname failed";
        oslSocketResult aResult;
        rtl::OUString suResult = saSocketAddr.getHostname( &aResult );
        ASSERT_TRUE(aResult == osl_Socket_Ok) << "SocketAddr.getHostname failed.";

        rtl::OUString suError = outputError(suResult, suHostname, "test for getHostname(0)");
        bOK = compareUString( suResult, suHostname );
        if ( bOK == sal_False)
        {
            rtl::OString aStringResult = ::rtl::OUStringToOString( suResult, RTL_TEXTENCODING_ASCII_US );
            rtl::OString aStringHostname = ::rtl::OUStringToOString( suHostname, RTL_TEXTENCODING_ASCII_US );
            if ( compareUString( getIPbyName( aStringResult ) ,  getIPbyName( aStringHostname ) ) == sal_True )
            {
                bOK = sal_True;
            }
        }

        ASSERT_TRUE(sal_True == bOK) << suError.pData;
    }

    /** testing the method:
        inline sal_Int32 SAL_CALL getPort() const;
    */

    class getPort : public ::testing::Test
    {
    public:
    }; // class getPort

    TEST_F(getPort, getPort_001)
    {
        ::osl::SocketAddr saSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_FTP );

        ASSERT_TRUE(IP_PORT_FTP == saSocketAddr.getPort( )) << "test for getPort() function: get a normal port number.";
    }

    TEST_F(getPort, getPort_002)
    {
        ::osl::SocketAddr saSocketAddr( rtl::OUString::createFromAscii("129.158.217.202"), IP_PORT_INVAL );

        //t_print("#getPort_002: Port number is %d \n", saSocketAddr.getPort( ));

        ASSERT_TRUE(saSocketAddr.getPort( )>=1 && saSocketAddr.getPort( ) <= 65535) << "test for getPort( ) function: give an invalid port to a SocketAddr, get the port to see if it can detect. it did not pass in (W32).";
    }
    //two cases will return OSL_INVALID_PORT: 1. not valid SocketAddr
    //2. SocketAddr family is not osl_Socket_FamilyInet, but case 2 could not be constructed
    TEST_F(getPort, getPort_003)
    {
        rtl::OUString suInvalidIP = rtl::OUString::createFromAscii("123.345.67.89");
        ::osl::SocketAddr saSocketAddr( suInvalidIP, IP_PORT_MYPORT );

        ASSERT_TRUE(saSocketAddr.getPort( ) == OSL_INVALID_PORT) << "test for getPort( ) function: give an invalid IP to a SocketAddr, get the port to see returned value. ";
    }

    /** testing the method:
        inline sal_Bool SAL_CALL setPort( sal_Int32 nPort );
        rfc1413.txt: TCP port numbers are from 1-65535
        rfc1700.txt: 0/tcp    Reserved ;  0/udp    Reserved
    */

    class setPort : public ::testing::Test
    {
    public:
    }; // class setPort

    TEST_F(setPort, setPort_001)
    {
        ::osl::SocketAddr saSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_FTP );
        sal_Bool bOK = saSocketAddr.setPort( IP_PORT_TELNET );

        ASSERT_TRUE(( sal_True == bOK ) &&
                                ( IP_PORT_TELNET == saSocketAddr.getPort( ) )) << "test for setPort() function: modify a port number setting, and check it.";
    }

    /** 0 to 1024 is known as the reserved port range (traditionally only root can assign programs to ports in
        this range) and the ephemeral port range from 1025 to 65535.
        As many of you programmers will know, when you specify the source port of 0 when you connect to a host,
        the OS automatically reassigns the port number to high numbered ephemeral port. The same happens if you
        try to bind a listening socket to port 0.
        http://www.securiteam.com/securityreviews/5XP0Q2AAKS.html
        another: http://www.muq.org/~cynbe/muq/mufref_564.html
    */
    TEST_F(setPort, setPort_002)
    {
        ::osl::SocketAddr saSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_FTP );
        sal_Bool bOK = saSocketAddr.setPort( IP_PORT_ZERO );

        oslSocket sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
        ::osl::Socket sSocket(sHandle);
        sSocket.setOption( osl_Socket_OptionReuseAddr, 1 );//sal_True);
        sal_Bool bOK1 = sSocket.bind( saSocketAddr );
        ASSERT_TRUE(bOK1 == sal_True) << "bind SocketAddr failed";

        sal_Int32 newPort = sSocket.getLocalPort();
        //t_print("#new port is %d\n", newPort );

        ASSERT_TRUE(( 1024 <= newPort ) && ( 65535 >= newPort ) && ( bOK == sal_True )) << "test for setPort() function: port number should be in 1 ~ 65535, set port 0, it should be converted to a port number between 1024~65535.";

    }

    TEST_F(setPort, setPort_003)
    {
        ::osl::SocketAddr saSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_FTP);
        sal_Bool bOK = saSocketAddr.setPort( IP_PORT_INVAL );
        //on Linux, getPort return 34463
        //t_print("#Port number is %d \n", saSocketAddr.getPort( ));

        ASSERT_TRUE(( ( 1 <= saSocketAddr.getPort( ) ) && ( 65535 >= saSocketAddr.getPort( ) ) &&( bOK == sal_True ) ) ||
                                 bOK == sal_False) << "test for setPort( ) function: set an address with invalid port. it should return error or convert it to a valid port.";
    }

    /* this is not a inet-addr => can't set port */
    TEST_F(setPort, setPort_004)
    {
        ::osl::SocketAddr saSocketAddr( rtl::OUString::createFromAscii("123.345.67.89"), IP_PORT_FTP);
        sal_Bool bOK = saSocketAddr.setPort( IP_PORT_MYPORT );

        ASSERT_TRUE(bOK == sal_False) << "test for setPort( ) function: set an invalid address with valid port. it should return error.";
    }

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

    class setAddr : public ::testing::Test
    {
    public:
    }; // class setAddr

    TEST_F(setAddr, setAddr_001)
    {
        ::osl::SocketAddr saSocketAddr( rtl::OUString::createFromAscii("129.158.217.202"), IP_PORT_FTP );
        saSocketAddr.setAddr( UStringIPToByteSequence( rtl::OUString::createFromAscii("127.0.0.1") ) );
        ::rtl::ByteSequence bsSocketAddr = saSocketAddr.getAddr( 0 );
        sal_Bool bOK = sal_False;

        // if ( ( bsSocketAddr[0] == 127 ) && ( bsSocketAddr[1] == 0 ) && ( bsSocketAddr[2] == 0 ) && ( bsSocketAddr[3] == 1 ) )
        //  bOK = sal_True;
        bOK = ifIpv4is( bsSocketAddr, 127, 0, 0, 1 );

        ASSERT_TRUE(sal_True == bOK) << "test for setAddr() function: construct Addr with  \"129.158.217.202\", set it to \"127.0.0.1\",  and check the correctness ";
    }

    /** testing the method:
        inline ::rtl::ByteSequence  SAL_CALL getAddr( oslSocketResult *pResult = 0 ) const;
    */

    class getAddr : public ::testing::Test
    {
    public:
    }; // class getAddr

    TEST_F(getAddr, getAddr_001)
    {
        oslSocketResult SocketResult;
        ::osl::SocketAddr saSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_FTP );
        ::rtl::ByteSequence bsSocketAddr = saSocketAddr.getAddr( &SocketResult );

        sal_Bool bOK = sal_False;

        //if ( ( osl_Socket_Ok == SocketResult ) &&( bsSocketAddr[0] == 127 ) && ( bsSocketAddr[1] == 0 ) &&( bsSocketAddr[2] == 0 ) && ( bsSocketAddr[3] == 1 ) )
        //  bOK = sal_True;
        bOK = ifIpv4is( bsSocketAddr, 127, 0, 0, 1 );

        ASSERT_TRUE(sal_True == bOK && SocketResult == osl_Socket_Ok) << "test for getAddr() function: construct a socketaddr with IP assigned, get the address to check correctness.Caught unknown exception on (Win32)";
    }

    /** testing the methods:
        inline SocketAddr & SAL_CALL operator= (oslSocketAddr Addr);
        inline SocketAddr & SAL_CALL operator= (const SocketAddr& Addr);
        inline SocketAddr & SAL_CALL assign( oslSocketAddr Addr, __osl_socket_NoCopy nocopy );
        inline sal_Bool SAL_CALL operator== (oslSocketAddr Addr) const;
        inline sal_Bool SAL_CALL operator== (const SocketAddr & Addr) const;    /// not implemented.
    */

    class operator_equal : public ::testing::Test
    {
    public:
    }; // class operator_equal

    TEST_F(operator_equal, operator_equal_001)
    {
        ::osl::SocketAddr saSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_TELNET);
        ::osl::SocketAddr saSocketAddrEqual( rtl::OUString::createFromAscii("129.158.217.202"), IP_PORT_FTP );

        saSocketAddrEqual = saSocketAddr;
        sal_Bool bOK = sal_False;
        ::rtl::ByteSequence bsSocketAddr = saSocketAddrEqual.getAddr( 0 );

        // if ( ( IP_PORT_TELNET == saSocketAddrEqual.getPort( ) ) &&( bsSocketAddr[0] == 127 ) && ( bsSocketAddr[1] == 0 ) &&( bsSocketAddr[2] == 0 ) && ( bsSocketAddr[3] == 1 ) )
        if ( ( IP_PORT_TELNET == saSocketAddrEqual.getPort( ) ) &&  ( ifIpv4is( bsSocketAddr, 127, 0, 0, 1 ) == sal_True ) )
            bOK = sal_True;

        ASSERT_TRUE(sal_True == bOK) << "test for operator_equal() function: use operator= to assign Ip1 to Ip2, check its modification.";
    }


    TEST_F(operator_equal, operator_equal_002)
    {
        ::osl::SocketAddr saSocketAddr( rtl::OUString::createFromAscii("129.158.217.199"), IP_PORT_TELNET);
        ::osl::SocketAddr saSocketAddrEqual( rtl::OUString::createFromAscii("129.158.217.202"), IP_PORT_FTP );

        saSocketAddrEqual = saSocketAddr;
        ASSERT_TRUE(IP_PORT_TELNET == saSocketAddrEqual.getPort( )) << "after assign, the assigned SocketAddr is not same as the original Addr";
        saSocketAddrEqual.setPort( IP_PORT_MYPORT3 );
        saSocketAddr.setPort( IP_PORT_HTTP2 );

        ASSERT_TRUE(IP_PORT_MYPORT3 == saSocketAddrEqual.getPort( )) << "test for operator_equal() function: perform an equal action, then try to change the original address's port. it should not be changed ( handle released), it did not pass in (W32), this is under discussion.";
    }

    TEST_F(operator_equal, operator_equal_const_001)
    {
        const ::osl::SocketAddr saSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_TELNET);
        ::osl::SocketAddr saSocketAddrEqual( rtl::OUString::createFromAscii("129.158.217.202"), IP_PORT_FTP );

        saSocketAddrEqual = saSocketAddr;
        sal_Bool bOK = sal_False;
        ::rtl::ByteSequence bsSocketAddr = saSocketAddrEqual.getAddr( 0 );

        // if ( ( IP_PORT_TELNET == saSocketAddrEqual.getPort( ) ) &&( bsSocketAddr[0] == 127 ) && ( bsSocketAddr[1] == 0 ) &&( bsSocketAddr[2] == 0 ) && ( bsSocketAddr[3] == 1 ) )
        if ( ( IP_PORT_TELNET == saSocketAddrEqual.getPort( ) ) && ifIpv4is( bsSocketAddr, 127, 0, 0, 1 ) == sal_True )
            bOK = sal_True;

        ASSERT_TRUE(sal_True == bOK) << "test for operator_equal_const() function: use operator= const to assign Ip1 to Ip2, verify the change on the second one.";
    }

    TEST_F(operator_equal, operator_equal_const_002)
    {
        const ::osl::SocketAddr saSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_TELNET);
        ::osl::SocketAddr saSocketAddrEqual( rtl::OUString::createFromAscii("129.158.217.202"), IP_PORT_FTP );

        saSocketAddrEqual = saSocketAddr;
        saSocketAddrEqual.setPort( IP_PORT_HTTP1 );

        ASSERT_TRUE(IP_PORT_HTTP1 != saSocketAddr.getPort( )) << "test for operator_equal_const() function: change the second instance, the first one should not be altered, since it does not released the handle.";
    }

    TEST_F(operator_equal, operator_equal_assign_001)
    {
        ::osl::SocketAddr* pSocketAddr = new ::osl::SocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_TELNET );
            ASSERT_TRUE(pSocketAddr != NULL) << "check for new SocketAddr";
            ::osl::SocketAddr* pSocketAddrAssign = new ::osl::SocketAddr( rtl::OUString::createFromAscii("129.158.217.202"), IP_PORT_FTP );
            oslSocketAddr poslSocketAddr = pSocketAddr->getHandle( );
            //if( m_handle ) osl_destroySocketAddr( m_handle ); so pSocketAddrAssign had been destroyed and then point to pSocketAddr
            pSocketAddrAssign->assign(poslSocketAddr, SAL_NO_COPY);

            ASSERT_TRUE(pSocketAddrAssign->getPort( ) == IP_PORT_TELNET) << "test for SocketAddr no copy constructor function: do a no copy constructor on a given SocketAddr instance, modify the new instance's port, check the original one.";

            delete pSocketAddrAssign;
    }

    TEST_F(operator_equal, operator_is_equal_001)
    {
        ::osl::SocketAddr saSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_TELNET);
        ::osl::SocketAddr saSocketAddrequal( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_TELNET );

        ASSERT_TRUE(sal_True == ( saSocketAddrequal == saSocketAddr.getHandle( ) )) << "test for operator_equal_equal() function: check two identical Address.";
    }

    TEST_F(operator_equal, operator_is_equal_002)
    {
        ::osl::SocketAddr saSocketAddr( rtl::OUString::createFromAscii("129.158.217.202"), IP_PORT_FTP);
        ::osl::SocketAddr saSocketAddrequal( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_TELNET );

        ASSERT_TRUE(sal_False == ( saSocketAddrequal == saSocketAddr.getHandle( ) )) << "test for operator_equal_equal() function: check two different Address.";
    }


    /** testing the method:
        inline oslSocketAddr SAL_CALL getHandle() const;
    */

    class getSocketAddrHandle : public ::testing::Test
    {
    public:
    }; // class getSocketAddrHandle

    TEST_F(getSocketAddrHandle, getSocketAddrHandle_001)
    {
        ::osl::SocketAddr* pSocketAddr = new ::osl::SocketAddr( rtl::OUString::createFromAscii("localhost"), IP_PORT_HTTP1 );
            ASSERT_TRUE(pSocketAddr != NULL) << "check for new SocketAddr";
            oslSocketAddr psaOSLSocketAddr = pSocketAddr->getHandle( );
            ::osl::SocketAddr* pSocketAddrCopy = new ::osl::SocketAddr( psaOSLSocketAddr, SAL_NO_COPY );

            ASSERT_TRUE(pSocketAddr->getHandle( ) ==  pSocketAddrCopy->getHandle( )) << "test for SocketAddr no copy constructor function: do a no copy constructor on a given SocketAddr instance, modify the new instance's port, check the original one.";

            delete pSocketAddrCopy;
    }

    TEST_F(getSocketAddrHandle, getSocketAddrHandle_002)
    {
        ::osl::SocketAddr saSocketAddr( rtl::OUString::createFromAscii("deuce.PRC.Sun.COM"), IP_PORT_MYPORT4 );
        oslSocketAddr poslSocketAddr = saSocketAddr.getHandle( );

        sal_Bool bOK = ( saSocketAddr == poslSocketAddr );
        //t_print("getSocketAddrHandle_002\n");
        ASSERT_TRUE(sal_True == bOK) << "test for getHandle() function: use getHandle() function as an intermediate way to create identical address.";
    }

    /** testing the method:
        static inline ::rtl::OUString SAL_CALL getLocalHostname( oslSocketResult *pResult = 0);
    */

    class getLocalHostname : public ::testing::Test
    {
    public:
    }; // class getLocalHostname

    /* the process of getLocalHostname: 1.gethostname (same as /bin/hostname) returned name A
       2. search A in /etc/hosts, if there is an alias name is A, return the name in the same row
    */

    TEST_F(getLocalHostname, getLocalHostname_000)
        {
            // _osl_getFullQualifiedDomainName( );
            oslSocketResult aResult = osl_Socket_Error;
            rtl::OUString suHostname = osl::SocketAddr::getLocalHostname(&aResult);
            ASSERT_TRUE(aResult == osl_Socket_Ok) << "getLocalHostname failed";
        }

    TEST_F(getLocalHostname, getLocalHostname_001)
    {
        oslSocketResult *pResult = NULL;
        //printSocketResult(*pResult);
        ::rtl::OUString suResult = ::osl::SocketAddr::getLocalHostname( pResult );

        // LLA: IMHO localhost, or hostname by itself should be ok.
        rtl::OUString suThisHost = getThisHostname( );
        bool bOk = false;
        if (suThisHost.equals(rtl::OUString::createFromAscii("localhost")))
        {
            bOk = true;
        }
        else
        {
            if (suThisHost.equals(suResult))
            {
                bOk = true;
            }
        }

        ::rtl::OUString suError;
        suError = outputError(suResult, getThisHostname( ), "test for getLocalHostname() function");

        ASSERT_TRUE(bOk == true) << suError.pData;
    }

    /** testing the method:
        static inline void SAL_CALL resolveHostname( const ::rtl::OUString & strHostName , SocketAddr & Addr );
    */

    class resolveHostname : public ::testing::Test
    {
    public:
    }; // class resolveHostname

    TEST_F(resolveHostname, resolveHostname_001)
    {
        ::osl::SocketAddr saSocketAddr;
        ::osl::SocketAddr::resolveHostname( rtl::OUString::createFromAscii("127.0.0.1"), saSocketAddr );
        ::rtl::ByteSequence bsSocketAddr = saSocketAddr.getAddr( 0 );
        sal_Bool bOK = sal_False;

         if ( ( bsSocketAddr[0] == 127 ) && ( bsSocketAddr[1] == 0 ) &&( bsSocketAddr[2] == 0 ) && ( bsSocketAddr[3] == 1 ) )
            bOK = sal_True;

        ASSERT_TRUE(sal_True == bOK) << "test for resolveHostname() function: try to resolve localhost to 127.0.0.1.";
    }

    /** testing the method:
        static inline sal_Int32 SAL_CALL getServicePort(
            const ::rtl::OUString& strServiceName,
            const ::rtl::OUString & strProtocolName= ::rtl::OUString::createFromAscii( "tcp" ) );
    */

    class gettheServicePort : public ::testing::Test
    {
    public:
    }; // class gettheServicePort

    TEST_F(gettheServicePort, gettheServicePort_001)
    {
        rtl::OUString suServiceFTP  = rtl::OUString::createFromAscii( "ftp" );
        rtl::OUString suProtocolTCP = rtl::OUString::createFromAscii( "tcp" );

        ASSERT_TRUE(IP_PORT_FTP== ::osl::SocketAddr::getServicePort( suServiceFTP, suProtocolTCP )) << "test for getServicePort() function: try to get ftp service port on TCP protocol.";
    }

    TEST_F(gettheServicePort, gettheServicePort_002)
    {
        rtl::OUString suServiceTELNET  = rtl::OUString::createFromAscii( "telnet" );
        rtl::OUString suProtocolTCP    = rtl::OUString::createFromAscii( "tcp" );
        ASSERT_TRUE(IP_PORT_TELNET== ::osl::SocketAddr::getServicePort( suServiceTELNET, suProtocolTCP )) << "test for getServicePort() function: try to get telnet service port on TCP protocol.";
    }

    TEST_F(gettheServicePort, gettheServicePort_003)
    {
    //Solaris has no service called "https", please see /etc/services
        rtl::OUString suServiceNETBIOS = rtl::OUString::createFromAscii( "netbios-dgm" );
        rtl::OUString suProtocolUDP    = rtl::OUString::createFromAscii( "udp" );
        ASSERT_TRUE(IP_PORT_NETBIOS_DGM == ::osl::SocketAddr::getServicePort( suServiceNETBIOS, suProtocolUDP )) << "test for getServicePort() function: try to get netbios-ssn service port on UDP protocol.";
    }

    TEST_F(gettheServicePort, gettheServicePort_004)
    {
        rtl::OUString suProtocolUDP    = rtl::OUString::createFromAscii( "udp" );
        ASSERT_TRUE(OSL_INVALID_PORT == ::osl::SocketAddr::getServicePort( ::rtl::OUString::createFromAscii( "notexist" ), suProtocolUDP )) << "test for getServicePort() function: try to get a service port which is not exist.";
    }

    /** testing the method:

    */

    class getFamilyOfSocketAddr : public ::testing::Test
    {
    public:
    }; // class getFamilyOfSocketAddr

    TEST_F(getFamilyOfSocketAddr, getFamilyOfSocketAddr_001)
    {
            ::osl::SocketAddr saSocketAddr( rtl::OUString::createFromAscii("localhost"), IP_PORT_HTTP1 );
            oslSocketAddr psaOSLSocketAddr = saSocketAddr.getHandle( );
            ASSERT_EQ(
                osl_Socket_FamilyInet,
                osl_getFamilyOfSocketAddr( psaOSLSocketAddr ) );

        ASSERT_TRUE(osl_getFamilyOfSocketAddr( psaOSLSocketAddr ) == osl_Socket_FamilyInet) << "test for osl_getFamilyOfSocketAddr.";
    }

} // namespace osl_SocketAddr

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
