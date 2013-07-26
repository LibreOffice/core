/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>

#include <sal/types.h>
#include "cppunit/TestFixture.h"
#include <cppunit/extensions/HelperMacros.h>
#include "tools/urlobj.hxx"

#define OUSTR_TO_STDSTR( oustr ) std::string( OUStringToOString( oustr, RTL_TEXTENCODING_ASCII_US ).getStr() )

CPPUNIT_NS_BEGIN

template<> struct assertion_traits<INetProtocol>
{
    static bool equal( const INetProtocol& x, const INetProtocol& y )
    {
        return x == y;
    }

    static std::string toString( const INetProtocol& x )
    {
        OStringStream ost;
        ost << static_cast<unsigned int>(x);
        return ost.str();
    }
};

CPPUNIT_NS_END

namespace tools_urlobj
{

    class urlobjTest:public CppUnit::TestFixture
    {

      public:
        // initialise your test code values here.
        void setUp(  )
        {
        }

        void tearDown(  )
        {
        }

        // insert your test code here.
        // this is only demonstration code
        void urlobjTest_001(  )
        {
            INetURLObject aUrl( OUString( "file://10.10.1.1/sampledir/sample.file" ) );
#ifdef LINUX
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE ).
                            compareToAscii
                            ( "smb://10.10.1.1/sampledir/sample.file" ) ==
                            0 );
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INET_PROT_SMB );
#endif
#ifdef WIN
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE ).
                            compareToAscii
                            ( "file://10.10.1.1/sampledir/sample.file" ) ==
                            0 );
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INET_PROT_FILE );
#endif
            CPPUNIT_ASSERT( aUrl.GetHost( INetURLObject::NO_DECODE ).
                            compareToAscii( "10.10.1.1" ) == 0 );
            CPPUNIT_ASSERT( aUrl.GetURLPath( INetURLObject::NO_DECODE ).
                            compareToAscii( "/sampledir/sample.file" ) == 0 );
            CPPUNIT_ASSERT( aUrl.getName(  ).
                            compareToAscii( "sample.file" ) == 0 );
            CPPUNIT_ASSERT( aUrl.getBase(  ).compareToAscii( "sample" ) ==
                            0 );
            CPPUNIT_ASSERT( aUrl.getExtension(  ).compareToAscii( "file" ) ==
                            0 );
        }

        void urlobjTest_002(  )
        {
            INetURLObject aUrl;
            aUrl.
                setFSysPath( OUString( "\\\\137.65.170.24\\c$\\Img0001.jpg" ),
                             INetURLObject::FSYS_DETECT );
#ifdef LINUX
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE ).
                            compareToAscii
                            ( "smb://137.65.170.24/c$/Img0001.jpg" ) == 0 );
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INET_PROT_SMB );
#endif
#ifdef WIN
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE ).
                            compareToAscii
                            ( "file://137.65.170.24/c$/Img0001.jpg" ) == 0 );
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INET_PROT_FILE );
#endif
            CPPUNIT_ASSERT( aUrl.GetHost( INetURLObject::NO_DECODE ).
                            compareToAscii( "137.65.170.24" ) == 0 );
            CPPUNIT_ASSERT( aUrl.GetURLPath( INetURLObject::NO_DECODE ).
                            compareToAscii( "/c$/Img0001.jpg" ) == 0 );
            CPPUNIT_ASSERT( aUrl.getName(  ).
                            compareToAscii( "Img0001.jpg" ) == 0 );
            CPPUNIT_ASSERT( aUrl.getBase(  ).compareToAscii( "Img0001" ) ==
                            0 );
            CPPUNIT_ASSERT( aUrl.getExtension(  ).compareToAscii( "jpg" ) ==
                            0 );
        }


        void urlobjTest_003(  )
        {
            INetURLObject aUrl;
            aUrl.
                setFSysPath( OUString( "\\\\hive-winxp-x86\\pmladek\\test2.odt" ),
                             INetURLObject::FSYS_DETECT );
#ifdef LINUX
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE ).
                            compareToAscii
                            ( "smb://hive-winxp-x86/pmladek/test2.odt" ) ==
                            0 );
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INET_PROT_SMB );
#endif
#ifdef WIN
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE ).
                            compareToAscii
                            ( "file://hive-winxp-x86/pmladek/test2.odt" ) ==
                            0 );
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INET_PROT_FILE );
#endif
            CPPUNIT_ASSERT( aUrl.GetHost( INetURLObject::NO_DECODE ).
                            compareToAscii( "hive-winxp-x86" ) == 0 );
            CPPUNIT_ASSERT( aUrl.GetURLPath( INetURLObject::NO_DECODE ).
                            compareToAscii( "/pmladek/test2.odt" ) == 0 );
        }

        void urlobjTest_004(  )
        {
            INetURLObject aUrl( OUString( "smb://10.10.1.1/sampledir/sample.file" ) );
#ifdef LINUX
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE ).
                            compareToAscii
                            ( "smb://10.10.1.1/sampledir/sample.file" ) ==
                            0 );
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INET_PROT_SMB );
#endif
#ifdef WIN
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE ).
                            compareToAscii
                            ( "file://10.10.1.1/sampledir/sample.file" ) ==
                            0 );
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INET_PROT_FILE );
#endif
            CPPUNIT_ASSERT( aUrl.GetHost( INetURLObject::NO_DECODE ).
                            compareToAscii( "10.10.1.1" ) == 0 );
            CPPUNIT_ASSERT( aUrl.GetURLPath( INetURLObject::NO_DECODE ).
                            compareToAscii( "/sampledir/sample.file" ) == 0 );
            CPPUNIT_ASSERT( aUrl.getName(  ).
                            compareToAscii( "sample.file" ) == 0 );
            CPPUNIT_ASSERT( aUrl.getBase(  ).compareToAscii( "sample" ) ==
                            0 );
            CPPUNIT_ASSERT( aUrl.getExtension(  ).compareToAscii( "file" ) ==
                            0 );
        }

        void urlobjTest_005(  )
        {
            INetURLObject aUrl;
            aUrl.setFSysPath( OUString( "//137.65.170.24/c$/Img0001.jpg" ),
                              INetURLObject::FSYS_DETECT );
#ifdef LINUX
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE ).
                            compareToAscii( "smb://137.65.170.24/c$/Img0001.jpg" ) == 0 );
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INET_PROT_SMB );
#endif
#ifdef WIN
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE ).
                            compareToAscii( "file://137.65.170.24/c$/Img0001.jpg" ) == 0 );
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INET_PROT_FILE );
#endif
            CPPUNIT_ASSERT( aUrl.GetHost( INetURLObject::NO_DECODE ).
                            compareToAscii( "137.65.170.24" ) == 0 );
            CPPUNIT_ASSERT( aUrl.GetURLPath( INetURLObject::NO_DECODE ).
                            compareToAscii( "/c$/Img0001.jpg" ) == 0 );
            CPPUNIT_ASSERT( aUrl.getName(  ).compareToAscii( "Img0001.jpg" ) == 0 );
            CPPUNIT_ASSERT( aUrl.getBase(  ).compareToAscii( "Img0001" ) == 0 );
            CPPUNIT_ASSERT( aUrl.getExtension(  ).compareToAscii( "jpg" ) == 0 );
        }


        void urlobjTest_006(  )
        {
            INetURLObject aUrl;
            aUrl.setFSysPath( OUString( "//hive-winxp-x86/pmladek/test2.odt" ),
                              INetURLObject::FSYS_DETECT );
#ifdef LINUX
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE ).
                            compareToAscii( "smb://hive-winxp-x86/pmladek/test2.odt" ) == 0 );
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INET_PROT_SMB );
#endif
#ifdef WIN
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE ).
                            compareToAscii( "file://hive-winxp-x86/pmladek/test2.odt" ) == 0 );
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INET_PROT_FILE );
#endif
            CPPUNIT_ASSERT( aUrl.GetHost( INetURLObject::NO_DECODE ).
                            compareToAscii( "hive-winxp-x86" ) == 0 );
            CPPUNIT_ASSERT( aUrl.GetURLPath( INetURLObject::NO_DECODE ).
                            compareToAscii( "/pmladek/test2.odt" ) == 0 );
        }

        void urlobjCmisTest(  )
        {
            // Test with a username part
            {
                INetURLObject aUrl( OUString( "vnd.libreoffice.cmis://username@http:%2F%2Ffoo.bar.com:8080%2Fmy%2Fcmis%2Fatom%23repo-id-encoded/path/to/content" ) );
                CPPUNIT_ASSERT_EQUAL( std::string( "http://foo.bar.com:8080/my/cmis/atom#repo-id-encoded" ),
                        OUSTR_TO_STDSTR( aUrl.GetHost( INetURLObject::DECODE_WITH_CHARSET ) ) );
                CPPUNIT_ASSERT_EQUAL( std::string( "username" ), OUSTR_TO_STDSTR( aUrl.GetUser( ) ) );
                CPPUNIT_ASSERT_EQUAL( std::string( "/path/to/content" ),
                        OUSTR_TO_STDSTR( aUrl.GetURLPath( INetURLObject::NO_DECODE ) ) );
                CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong protocol found", INET_PROT_CMIS, aUrl.GetProtocol(  ) );
            }

            // Test without a username part
            {
                INetURLObject aUrl( OUString(
                                "vnd.libreoffice.cmis://http:%2F%2Ffoo.bar.com:8080%2Fmy%2Fcmis%2Fatom%23repo-id-encoded/path/to/content" ) );
                CPPUNIT_ASSERT_EQUAL( std::string( "http://foo.bar.com:8080/my/cmis/atom#repo-id-encoded" ),
                        OUSTR_TO_STDSTR( aUrl.GetHost( INetURLObject::DECODE_WITH_CHARSET ) ) );
                CPPUNIT_ASSERT( !aUrl.HasUserData() );
                CPPUNIT_ASSERT_EQUAL( std::string( "/path/to/content" ),
                        OUSTR_TO_STDSTR( aUrl.GetURLPath( INetURLObject::NO_DECODE ) ) );
                CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong protocol found", INET_PROT_CMIS, aUrl.GetProtocol(  ) );
            }
        }

        void urlobjTest_emptyPath() {
            {
                INetURLObject url(OUString("http://example.com"));
                CPPUNIT_ASSERT_EQUAL(INET_PROT_HTTP, url.GetProtocol());
                CPPUNIT_ASSERT_EQUAL(OUString("example.com"), url.GetHost());
                CPPUNIT_ASSERT_EQUAL(OUString("/"), url.GetURLPath());
            }
            {
                // This is an invalid http URL per RFC 2616:
                INetURLObject url(OUString("http://example.com?query"));
                CPPUNIT_ASSERT(url.HasError());
            }
            {
                INetURLObject url(OUString("http://example.com#fragment"));
                CPPUNIT_ASSERT_EQUAL(INET_PROT_HTTP, url.GetProtocol());
                CPPUNIT_ASSERT_EQUAL(OUString("example.com"), url.GetHost());
                CPPUNIT_ASSERT_EQUAL(OUString("/"), url.GetURLPath());
                CPPUNIT_ASSERT_EQUAL(OUString("fragment"), url.GetMark());
            }
        }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE( urlobjTest );
        CPPUNIT_TEST( urlobjTest_001 );
        CPPUNIT_TEST( urlobjTest_002 );
        CPPUNIT_TEST( urlobjTest_003 );
        CPPUNIT_TEST( urlobjTest_004 );
        CPPUNIT_TEST( urlobjTest_005 );
        CPPUNIT_TEST( urlobjTest_006 );
        CPPUNIT_TEST( urlobjCmisTest );
        CPPUNIT_TEST( urlobjTest_emptyPath );
        CPPUNIT_TEST_SUITE_END(  );
    };                          // class createPool


    CPPUNIT_TEST_SUITE_REGISTRATION( urlobjTest );
}                               // namespace rtl_random


// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
