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
using ::rtl::OUStringToOString;
using ::rtl::OString;

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

    class ctors : public CppUnit::TestFixture
    {
    public:

        void ctors_none()
        {
            /// SocketAddr constructor.
            ::osl::SocketAddr saSocketAddr;

            // oslSocketResult aResult;
            // rtl::OUString suHost = saSocketAddr.getLocalHostname( &aResult);

            // rtl::OUString suHost2 = getThisHostname();

            CPPUNIT_ASSERT_MESSAGE("test for none parameter constructor function: check if the socket address was created successfully",
                                    sal_True == saSocketAddr.is( ) );
        }

        void ctors_none_000()
        {
            /// SocketAddr constructor.
            ::osl::SocketAddr saSocketAddr;

            oslSocketResult aResult;
            rtl::OUString suHost = saSocketAddr.getLocalHostname( &aResult);
            rtl::OUString suHost2 = getThisHostname();

            sal_Bool bOk = compareUString(suHost, suHost2);

            rtl::OUString suError (RTL_CONSTASCII_USTRINGPARAM("Host names should be the same. From SocketAddr.getLocalHostname() it is'"));
            suError += suHost;
            suError += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("', from getThisHostname() it is '"));
            suError += suHost2;
            suError += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("'."));

            CPPUNIT_ASSERT_MESSAGE(STD_STRING(suError), sal_True == bOk);
        }

        void ctors_copy()
        {
            /// SocketAddr copy constructor.
            ::osl::SocketAddr saSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("localhost")), IP_PORT_HTTP1 );
            ::osl::SocketAddr saCopySocketAddr( saSocketAddr );

            sal_Int32 nPort = saCopySocketAddr.getPort( );

            CPPUNIT_ASSERT_MESSAGE("test for SocketAddr copy constructor function: copy constructor, do an action of copy construction then check the port with original set.",
                                    ( sal_True == saCopySocketAddr.is( ) ) && ( nPort == IP_PORT_HTTP1 ) );
        }

        void ctors_copy_no_001()
        {
            ::osl::SocketAddr* pSocketAddr = new ::osl::SocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("localhost")), IP_PORT_HTTP1 );
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
            ::osl::SocketAddr* pSocketAddr = new ::osl::SocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("localhost")), IP_PORT_HTTP1 );
                   CPPUNIT_ASSERT_MESSAGE("check for new SocketAddr", pSocketAddr != NULL);
                   oslSocketAddr psaOSLSocketAddr = pSocketAddr->getHandle( );
                   ::osl::SocketAddr* pSocketAddrCopy = new ::osl::SocketAddr( psaOSLSocketAddr, SAL_NO_COPY );

                   CPPUNIT_ASSERT_MESSAGE("test for SocketAddr no copy constructor function: do a no copy constructor on a given SocketAddr instance, modify the new instance's port, check the original one.",
                        pSocketAddr->getHandle( ) ==  pSocketAddrCopy->getHandle( ) );

                   delete pSocketAddrCopy;
        }

        void ctors_copy_handle_001()
        {
            ::osl::SocketAddr saSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("localhost")), IP_PORT_HTTP1 );
            ::osl::SocketAddr saSocketAddrCopy( saSocketAddr.getHandle( ) );

            CPPUNIT_ASSERT_MESSAGE("test for SocketAddr copy handle constructor function: copy another Socket's handle, get its port to check copy effect.",
                                    saSocketAddrCopy.getPort( ) == IP_PORT_HTTP1 );
        }

        void ctors_copy_handle_002()
        {
            ::osl::SocketAddr saSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("localhost")), IP_PORT_HTTP1 );
            ::osl::SocketAddr saSocketAddrCopy( saSocketAddr.getHandle( ) );
            saSocketAddrCopy.setPort( IP_PORT_HTTP2 );

            CPPUNIT_ASSERT_MESSAGE("test for SocketAddr copy handle constructor function: copy another Socket's handle, the original one should not be changed.",
                                    saSocketAddr.getPort( ) != IP_PORT_HTTP2 );
        }

        void ctors_hostname_port_001()
        {
            /// tcpip-specif constructor.
            ::osl::SocketAddr saSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), IP_PORT_FTP );
            printUString( saSocketAddr.getHostname( ), "ctors_hostname_port_001:getHostname");

            CPPUNIT_ASSERT_MESSAGE("test for SocketAddr tcpip specif constructor function: do a constructor using tcpip spec, check the result.",
                                    saSocketAddr.is( ) == sal_True &&
                                    ( saSocketAddr.getPort( ) == IP_PORT_FTP )
                                  );
        }

        //same as is_002
        void ctors_hostname_port_002()
        {
            /// tcpip-specif constructor.
            ::osl::SocketAddr saSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("123.345.67.89")), IP_PORT_MYPORT2 );

            CPPUNIT_ASSERT_MESSAGE("test for SocketAddr tcpip specif constructor function: using an invalid IP address, the socketaddr ctors should fail", sal_False == saSocketAddr.is( ));
        }
        CPPUNIT_TEST_SUITE( ctors );
        CPPUNIT_TEST( ctors_none );
        CPPUNIT_TEST( ctors_none_000 );
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
            ::osl::SocketAddr saSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), IP_PORT_INVAL );

            CPPUNIT_ASSERT_MESSAGE("test for is() function: create a tcp-ip socket using invalid port number",
                                    sal_True == saSocketAddr.is( ) );
        }

        void is_003()
        {
            ::osl::SocketAddr saSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("123.345.67.89")), IP_PORT_MYPORT );

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

        void getHostname_000()
            {
                ::osl::SocketAddr saSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("129.158.217.107")), IP_PORT_FTP );
                rtl::OUString suResult = saSocketAddr.getHostname( 0 );

            }

        /** it will search the Ip in current machine's /etc/hosts at first, if find, then return the
            mapped hostname, otherwise, it will search via DNS server, and often return hostname+ Domain name
            like "sceri.PRC.Sun.COM"
            The process is same as Socket::getLocalHost(), but getLocalHost can only return hostname of the current machine.
        */
        void getHostname_001()
        {
            ::osl::SocketAddr saSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("129.158.217.107")), IP_PORT_FTP );
            rtl::OUString suResult = saSocketAddr.getHostname( 0 );
            rtl::OUString suError = outputError(suResult, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sceri.PRC.Sun.COM")), "test for getHostname(0)");
            sal_Bool bOK = compareUString( suResult, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sceri.PRC.Sun.COM")) );
            // search the returned hostname in /etc/hosts, if find, and the IP in the row is same as IP
            // in the Addr, it's right also.
            if ( bOK == sal_False)
            {
                rtl::OString aString = ::rtl::OUStringToOString( suResult, RTL_TEXTENCODING_ASCII_US );
                if ( compareUString( getIPbyName( aString ), rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("129.158.217.107")) ) == sal_True )
                    bOK = sal_True;
            }
            CPPUNIT_ASSERT_MESSAGE( STD_STRING(suError), sal_True == bOK);
        }

// LLA: now we have to control, if this behaviour is right.
// LLA: this function does not work in company (Linux, Windows) but at home
        void getHostname_002()
        {
            rtl::OUString suHostname (RTL_CONSTASCII_USTRINGPARAM("cn-1.germany.sun.com"));
            rtl::OString aString = ::rtl::OUStringToOString( suHostname, RTL_TEXTENCODING_ASCII_US );
            rtl::OUString aHostIP    = getIPbyName( aString );

            ::osl::SocketAddr saSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("localhost")), IP_PORT_FTP );
            sal_Bool bOK = saSocketAddr.setHostname( suHostname );
            CPPUNIT_ASSERT_MESSAGE("#SocketAddr.setHostname failed", sal_True == bOK );
            oslSocketResult aResult;
            rtl::OUString suResult = saSocketAddr.getHostname( &aResult );
            CPPUNIT_ASSERT_MESSAGE("SocketAddr.getHostname failed.", aResult == osl_Socket_Ok);

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

            CPPUNIT_ASSERT_MESSAGE( STD_STRING(suError), sal_True == bOK );
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
            ::osl::SocketAddr saSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), IP_PORT_FTP );

            CPPUNIT_ASSERT_MESSAGE( "test for getPort() function: get a normal port number.",
                                    IP_PORT_FTP == saSocketAddr.getPort( ) );
        }

        void getPort_002()
        {
            ::osl::SocketAddr saSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("129.158.217.202")), IP_PORT_INVAL );

            //t_print("#getPort_002: Port number is %d \n", saSocketAddr.getPort( ));

            CPPUNIT_ASSERT_MESSAGE( "test for getPort( ) function: give an invalid port to a SocketAddr, get the port to see if it can detect. it did not pass in (W32).",
                                    saSocketAddr.getPort( )>=1 && saSocketAddr.getPort( ) <= 65535 );
        }
        //two cases will return OSL_INVALID_PORT: 1. not valid SocketAddr
        //2. SocketAddr family is not osl_Socket_FamilyInet, but case 2 could not be constructed
        void getPort_003()
        {
            rtl::OUString suInvalidIP (RTL_CONSTASCII_USTRINGPARAM("123.345.67.89"));
            ::osl::SocketAddr saSocketAddr( suInvalidIP, IP_PORT_MYPORT );

            CPPUNIT_ASSERT_MESSAGE( "test for getPort( ) function: give an invalid IP to a SocketAddr, get the port to see returned value. ",
                                    saSocketAddr.getPort( ) == OSL_INVALID_PORT );
        }

        CPPUNIT_TEST_SUITE( getPort );
        CPPUNIT_TEST( getPort_001 );
        CPPUNIT_TEST( getPort_002 );
        CPPUNIT_TEST( getPort_003 );
        CPPUNIT_TEST_SUITE_END( );

    }; // class getPort


    /** testing the method:
        inline sal_Bool SAL_CALL setPort( sal_Int32 nPort );
        rfc1413.txt: TCP port numbers are from 1-65535
        rfc1700.txt: 0/tcp    Reserved ;  0/udp    Reserved
    */

    class setPort : public CppUnit::TestFixture
    {
    public:
        void setPort_001()
        {
            ::osl::SocketAddr saSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), IP_PORT_FTP );
            sal_Bool bOK = saSocketAddr.setPort( IP_PORT_TELNET );

            CPPUNIT_ASSERT_MESSAGE( "test for setPort() function: modify a port number setting, and check it.",
                                    ( sal_True == bOK ) &&
                                    ( IP_PORT_TELNET == saSocketAddr.getPort( ) ) );
        }

        /** 0 to 1024 is known as the reserved port range (traditionally only root can assign programs to ports in
            this range) and the ephemeral port range from 1025 to 65535.
            As many of you programmers will know, when you specify the source port of 0 when you connect to a host,
            the OS automatically reassigns the port number to high numbered ephemeral port. The same happens if you
            try to bind a listening socket to port 0.
            http://www.securiteam.com/securityreviews/5XP0Q2AAKS.html
            another: http://www.muq.org/~cynbe/muq/mufref_564.html
        */
        void setPort_002()
        {
            ::osl::SocketAddr saSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), IP_PORT_FTP );
            sal_Bool bOK = saSocketAddr.setPort( IP_PORT_ZERO );

            oslSocket sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
            ::osl::Socket sSocket(sHandle);
            sSocket.setOption( osl_Socket_OptionReuseAddr, 1 );//sal_True);
            sal_Bool bOK1 = sSocket.bind( saSocketAddr );
            CPPUNIT_ASSERT_MESSAGE( "bind SocketAddr failed", bOK1 == sal_True );

            sal_Int32 newPort = sSocket.getLocalPort();
            //t_print("#new port is %d\n", newPort );

            CPPUNIT_ASSERT_MESSAGE( "test for setPort() function: port number should be in 1 ~ 65535, set port 0, it should be converted to a port number between 1024~65535.",
                                    ( 1024 <= newPort ) && ( 65535 >= newPort ) && ( bOK == sal_True ) );

        }

        void setPort_003()
        {
            ::osl::SocketAddr saSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), IP_PORT_FTP);
            sal_Bool bOK = saSocketAddr.setPort( IP_PORT_INVAL );
            //on Linux, getPort return 34463
            //t_print("#Port number is %d \n", saSocketAddr.getPort( ));

            CPPUNIT_ASSERT_MESSAGE( "test for setPort( ) function: set an address with invalid port. it should return error or convert it to a valid port.",
                                     ( ( 1 <= saSocketAddr.getPort( ) ) && ( 65535 >= saSocketAddr.getPort( ) ) &&( bOK == sal_True ) ) ||
                                     bOK == sal_False);
        }

        /* this is not a inet-addr => can't set port */
        void setPort_004()
        {
            ::osl::SocketAddr saSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("123.345.67.89")), IP_PORT_FTP);
            sal_Bool bOK = saSocketAddr.setPort( IP_PORT_MYPORT );

            CPPUNIT_ASSERT_MESSAGE( "test for setPort( ) function: set an invalid address with valid port. it should return error.",
                                     bOK == sal_False);
        }


        CPPUNIT_TEST_SUITE( setPort );
        CPPUNIT_TEST( setPort_001 );
        CPPUNIT_TEST( setPort_002 );
        CPPUNIT_TEST( setPort_003 );
        CPPUNIT_TEST( setPort_004 );
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
            ::osl::SocketAddr saSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("129.158.217.202")), IP_PORT_FTP );
            saSocketAddr.setAddr( UStringIPToByteSequence( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")) ) );
            ::rtl::ByteSequence bsSocketAddr = saSocketAddr.getAddr( 0 );
            sal_Bool bOK = sal_False;

            // if ( ( bsSocketAddr[0] == 127 ) && ( bsSocketAddr[1] == 0 ) && ( bsSocketAddr[2] == 0 ) && ( bsSocketAddr[3] == 1 ) )
            //  bOK = sal_True;
            bOK = ifIpv4is( bsSocketAddr, 127, 0, 0, 1 );

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
        void getAddr_001()
        {
            oslSocketResult SocketResult;
            ::osl::SocketAddr saSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), IP_PORT_FTP );
            ::rtl::ByteSequence bsSocketAddr = saSocketAddr.getAddr( &SocketResult );

            sal_Bool bOK = sal_False;

            //if ( ( osl_Socket_Ok == SocketResult ) &&( bsSocketAddr[0] == 127 ) && ( bsSocketAddr[1] == 0 ) &&( bsSocketAddr[2] == 0 ) && ( bsSocketAddr[3] == 1 ) )
            //  bOK = sal_True;
            bOK = ifIpv4is( bsSocketAddr, 127, 0, 0, 1 );

            CPPUNIT_ASSERT_MESSAGE( "test for getAddr() function: construct a socketaddr with IP assigned, get the address to check correctness.Caught unknown exception on (Win32)",
                sal_True == bOK && SocketResult == osl_Socket_Ok);
        }

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
            ::osl::SocketAddr saSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), IP_PORT_TELNET);
            ::osl::SocketAddr saSocketAddrEqual( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("129.158.217.202")), IP_PORT_FTP );

            saSocketAddrEqual = saSocketAddr;
            sal_Bool bOK = sal_False;
            ::rtl::ByteSequence bsSocketAddr = saSocketAddrEqual.getAddr( 0 );

            // if ( ( IP_PORT_TELNET == saSocketAddrEqual.getPort( ) ) &&( bsSocketAddr[0] == 127 ) && ( bsSocketAddr[1] == 0 ) &&( bsSocketAddr[2] == 0 ) && ( bsSocketAddr[3] == 1 ) )
            if ( ( IP_PORT_TELNET == saSocketAddrEqual.getPort( ) ) &&  ( ifIpv4is( bsSocketAddr, 127, 0, 0, 1 ) == sal_True ) )
                 bOK = sal_True;

            CPPUNIT_ASSERT_MESSAGE( "test for operator_equal() function: use operator= to assign Ip1 to Ip2, check its modification.",
                                      sal_True == bOK );
        }


        void operator_equal_002()
        {
            ::osl::SocketAddr saSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("129.158.217.199")), IP_PORT_TELNET);
            ::osl::SocketAddr saSocketAddrEqual( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("129.158.217.202")), IP_PORT_FTP );

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
            const ::osl::SocketAddr saSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), IP_PORT_TELNET);
            ::osl::SocketAddr saSocketAddrEqual( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("129.158.217.202")), IP_PORT_FTP );

            saSocketAddrEqual = saSocketAddr;
            sal_Bool bOK = sal_False;
            ::rtl::ByteSequence bsSocketAddr = saSocketAddrEqual.getAddr( 0 );

            // if ( ( IP_PORT_TELNET == saSocketAddrEqual.getPort( ) ) &&( bsSocketAddr[0] == 127 ) && ( bsSocketAddr[1] == 0 ) &&( bsSocketAddr[2] == 0 ) && ( bsSocketAddr[3] == 1 ) )
            if ( ( IP_PORT_TELNET == saSocketAddrEqual.getPort( ) ) && ifIpv4is( bsSocketAddr, 127, 0, 0, 1 ) == sal_True )
                 bOK = sal_True;

            CPPUNIT_ASSERT_MESSAGE( "test for operator_equal_const() function: use operator= const to assign Ip1 to Ip2, verify the change on the second one.",
                                      sal_True == bOK );
        }

        void operator_equal_const_002()
        {
            const ::osl::SocketAddr saSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), IP_PORT_TELNET);
            ::osl::SocketAddr saSocketAddrEqual( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("129.158.217.202")), IP_PORT_FTP );

            saSocketAddrEqual = saSocketAddr;
            saSocketAddrEqual.setPort( IP_PORT_HTTP1 );

            CPPUNIT_ASSERT_MESSAGE( "test for operator_equal_const() function: change the second instance, the first one should not be altered, since it does not released the handle.",
                                      IP_PORT_HTTP1 != saSocketAddr.getPort( ) );
        }

        void operator_equal_assign_001()
        {
            ::osl::SocketAddr* pSocketAddr = new ::osl::SocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), IP_PORT_TELNET );
                   CPPUNIT_ASSERT_MESSAGE("check for new SocketAddr", pSocketAddr != NULL);
                   ::osl::SocketAddr* pSocketAddrAssign = new ::osl::SocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("129.158.217.202")), IP_PORT_FTP );
                   oslSocketAddr poslSocketAddr = pSocketAddr->getHandle( );
                   //if( m_handle ) osl_destroySocketAddr( m_handle ); so pSocketAddrAssign had been destroyed and then point to pSocketAddr
                   pSocketAddrAssign->assign(poslSocketAddr, SAL_NO_COPY);

                   CPPUNIT_ASSERT_MESSAGE("test for SocketAddr no copy constructor function: do a no copy constructor on a given SocketAddr instance, modify the new instance's port, check the original one.",
                        pSocketAddrAssign->getPort( ) == IP_PORT_TELNET );

                   delete pSocketAddrAssign;
        }

        void operator_is_equal_001()
        {
            ::osl::SocketAddr saSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), IP_PORT_TELNET);
            ::osl::SocketAddr saSocketAddrequal( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), IP_PORT_TELNET );

            CPPUNIT_ASSERT_MESSAGE( "test for operator_equal_equal() function: check two identical Address.",
                                      sal_True == ( saSocketAddrequal == saSocketAddr.getHandle( ) ) );
        }

        void operator_is_equal_002()
        {
            ::osl::SocketAddr saSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("129.158.217.202")), IP_PORT_FTP);
            ::osl::SocketAddr saSocketAddrequal( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), IP_PORT_TELNET );

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
            ::osl::SocketAddr* pSocketAddr = new ::osl::SocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("localhost")), IP_PORT_HTTP1 );
                   CPPUNIT_ASSERT_MESSAGE("check for new SocketAddr", pSocketAddr != NULL);
                   oslSocketAddr psaOSLSocketAddr = pSocketAddr->getHandle( );
                   ::osl::SocketAddr* pSocketAddrCopy = new ::osl::SocketAddr( psaOSLSocketAddr, SAL_NO_COPY );

                   CPPUNIT_ASSERT_MESSAGE("test for SocketAddr no copy constructor function: do a no copy constructor on a given SocketAddr instance, modify the new instance's port, check the original one.",
                        pSocketAddr->getHandle( ) ==  pSocketAddrCopy->getHandle( ) );

                   delete pSocketAddrCopy;
        }

        void getSocketAddrHandle_002()
        {
            ::osl::SocketAddr saSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("deuce.PRC.Sun.COM")), IP_PORT_MYPORT4 );
            oslSocketAddr poslSocketAddr = saSocketAddr.getHandle( );

            sal_Bool bOK = ( saSocketAddr == poslSocketAddr );
            //t_print("getSocketAddrHandle_002\n");
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
        /* the process of getLocalHostname: 1.gethostname (same as /bin/hostname) returned name A
           2. search A in /etc/hosts, if there is an alias name is A, return the name in the same row
        */

        void getLocalHostname_000()
            {
                // _osl_getFullQualifiedDomainName( );
                oslSocketResult aResult = osl_Socket_Error;
                rtl::OUString suHostname = osl::SocketAddr::getLocalHostname(&aResult);
                CPPUNIT_ASSERT_MESSAGE("getLocalHostname failed", aResult == osl_Socket_Ok);
            }

        void getLocalHostname_001()
        {
            oslSocketResult *pResult = NULL;
            //printSocketResult(*pResult);
            ::rtl::OUString suResult = ::osl::SocketAddr::getLocalHostname( pResult );

            // LLA: IMHO localhost, or hostname by itself should be ok.
            rtl::OUString suThisHost = getThisHostname( );
            bool bOk = false;
            if (suThisHost.equals(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("localhost"))))
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

            CPPUNIT_ASSERT_MESSAGE( STD_STRING(suError), bOk == true );
        }

        CPPUNIT_TEST_SUITE( getLocalHostname );
        CPPUNIT_TEST( getLocalHostname_000 );
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
            ::osl::SocketAddr::resolveHostname( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("127.0.0.1")), saSocketAddr );
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
        static inline sal_Int32 SAL_CALL getServicePort(
            const ::rtl::OUString& strServiceName,
            const ::rtl::OUString & strProtocolName= ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("tcp")) );
    */

    class gettheServicePort : public CppUnit::TestFixture
    {
    public:
        void gettheServicePort_001()
        {
            rtl::OUString suServiceFTP  (RTL_CONSTASCII_USTRINGPARAM("ftp"));
            rtl::OUString suProtocolTCP (RTL_CONSTASCII_USTRINGPARAM("tcp"));

            CPPUNIT_ASSERT_MESSAGE( "test for getServicePort() function: try to get ftp service port on TCP protocol.",
                                      IP_PORT_FTP== ::osl::SocketAddr::getServicePort( suServiceFTP, suProtocolTCP ) );
        }

        void gettheServicePort_002()
        {
            rtl::OUString suServiceTELNET  (RTL_CONSTASCII_USTRINGPARAM("telnet"));
            rtl::OUString suProtocolTCP    (RTL_CONSTASCII_USTRINGPARAM("tcp"));
            CPPUNIT_ASSERT_MESSAGE( "test for getServicePort() function: try to get telnet service port on TCP protocol.",
                                      IP_PORT_TELNET== ::osl::SocketAddr::getServicePort( suServiceTELNET, suProtocolTCP ) );
        }

        void gettheServicePort_003()
        {
        //Solaris has no service called "https", please see /etc/services
            rtl::OUString suServiceNETBIOS (RTL_CONSTASCII_USTRINGPARAM("netbios-dgm"));
            rtl::OUString suProtocolUDP    (RTL_CONSTASCII_USTRINGPARAM("udp"));
            CPPUNIT_ASSERT_MESSAGE( "test for getServicePort() function: try to get netbios-ssn service port on UDP protocol.",
                                      IP_PORT_NETBIOS_DGM == ::osl::SocketAddr::getServicePort( suServiceNETBIOS, suProtocolUDP ) );
        }

        void gettheServicePort_004()
        {
            rtl::OUString suProtocolUDP(RTL_CONSTASCII_USTRINGPARAM( "udp" ));
            CPPUNIT_ASSERT_MESSAGE( "test for getServicePort() function: try to get a service port which is not exist.",
                                      OSL_INVALID_PORT == ::osl::SocketAddr::getServicePort( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("notexist")), suProtocolUDP ) );
        }

        CPPUNIT_TEST_SUITE( gettheServicePort );
        CPPUNIT_TEST( gettheServicePort_001 );
        CPPUNIT_TEST( gettheServicePort_002 );
        CPPUNIT_TEST( gettheServicePort_003 );
        CPPUNIT_TEST( gettheServicePort_004 );
        CPPUNIT_TEST_SUITE_END( );

    }; // class gettheServicePort

    /** testing the method:

    */

    class getFamilyOfSocketAddr : public CppUnit::TestFixture
    {
    public:
        void getFamilyOfSocketAddr_001()
        {
                   ::osl::SocketAddr saSocketAddr( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("localhost")), IP_PORT_HTTP1 );
                   oslSocketAddr psaOSLSocketAddr = saSocketAddr.getHandle( );
                   CPPUNIT_ASSERT_EQUAL(
                    osl_Socket_FamilyInet,
                    osl_getFamilyOfSocketAddr( psaOSLSocketAddr ) );

            CPPUNIT_ASSERT_MESSAGE( "test for osl_getFamilyOfSocketAddr.",
                                      osl_getFamilyOfSocketAddr( psaOSLSocketAddr ) == osl_Socket_FamilyInet );
        }

        CPPUNIT_TEST_SUITE( getFamilyOfSocketAddr );
        CPPUNIT_TEST( getFamilyOfSocketAddr_001 );
        CPPUNIT_TEST_SUITE_END( );

    }; // class getFamilyOfSocketAddr

// -----------------------------------------------------------------------------


CPPUNIT_TEST_SUITE_REGISTRATION(osl_SocketAddr::ctors);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_SocketAddr::is);
//TODO: enable Test with valid host names
//CPPUNIT_TEST_SUITE_REGISTRATION(osl_SocketAddr::getHostname);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_SocketAddr::getPort);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_SocketAddr::setPort);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_SocketAddr::setAddr);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_SocketAddr::getAddr);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_SocketAddr::operator_equal);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_SocketAddr::getSocketAddrHandle);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_SocketAddr::getLocalHostname);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_SocketAddr::resolveHostname);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_SocketAddr::gettheServicePort);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_SocketAddr::getFamilyOfSocketAddr);

} // namespace osl_SocketAddr

// -----------------------------------------------------------------------------

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
