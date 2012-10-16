/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       [ insert your name / company etc. here eg. Jim Bob <jim@bob.org> ]
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Florian Reuter <freuter@novell.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <string>

#include <sal/types.h>
#include "cppunit/TestFixture.h"
#include <cppunit/extensions/HelperMacros.h>
#include "tools/urlobj.hxx"

#define OUSTR_TO_STDSTR( oustr ) std::string( rtl::OUStringToOString( oustr, RTL_TEXTENCODING_ASCII_US ).getStr() )

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
            INetURLObject aUrl( rtl::
                                OUString( RTL_CONSTASCII_USTRINGPARAM
                                          ( "file://10.10.1.1/sampledir/sample.file" ) ) );
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
                setFSysPath( rtl::
                             OUString( RTL_CONSTASCII_USTRINGPARAM
                                       ( "\\\\137.65.170.24\\c$\\Img0001.jpg" ) ),
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
                setFSysPath( rtl::
                             OUString( RTL_CONSTASCII_USTRINGPARAM
                                       ( "\\\\hive-winxp-x86\\pmladek\\test2.odt" ) ),
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
            INetURLObject aUrl( rtl::
                                OUString( RTL_CONSTASCII_USTRINGPARAM
                                          ( "smb://10.10.1.1/sampledir/sample.file" ) ) );
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
            aUrl.setFSysPath( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "//137.65.170.24/c$/Img0001.jpg" ) ),
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
            aUrl.setFSysPath( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "//hive-winxp-x86/pmladek/test2.odt" ) ),
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
                INetURLObject aUrl( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                "vnd.libreoffice.cmis://username@http:%2F%2Ffoo.bar.com:8080%2Fmy%2Fcmis%2Fatom%23repo-id-encoded/path/to/content" ) ) );
                CPPUNIT_ASSERT_EQUAL( std::string( "http://foo.bar.com:8080/my/cmis/atom#repo-id-encoded" ),
                        OUSTR_TO_STDSTR( aUrl.GetHost( INetURLObject::DECODE_WITH_CHARSET ) ) );
                CPPUNIT_ASSERT_EQUAL( std::string( "username" ), OUSTR_TO_STDSTR( aUrl.GetUser( ) ) );
                CPPUNIT_ASSERT_EQUAL( std::string( "/path/to/content" ),
                        OUSTR_TO_STDSTR( aUrl.GetURLPath( INetURLObject::NO_DECODE ) ) );
                CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong protocol found", INET_PROT_CMIS, aUrl.GetProtocol(  ) );
            }

            // Test without a username part
            {
                INetURLObject aUrl( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                "vnd.libreoffice.cmis://http:%2F%2Ffoo.bar.com:8080%2Fmy%2Fcmis%2Fatom%23repo-id-encoded/path/to/content" ) ) );
                CPPUNIT_ASSERT_EQUAL( std::string( "http://foo.bar.com:8080/my/cmis/atom#repo-id-encoded" ),
                        OUSTR_TO_STDSTR( aUrl.GetHost( INetURLObject::DECODE_WITH_CHARSET ) ) );
                CPPUNIT_ASSERT( !aUrl.HasUserData() );
                CPPUNIT_ASSERT_EQUAL( std::string( "/path/to/content" ),
                        OUSTR_TO_STDSTR( aUrl.GetURLPath( INetURLObject::NO_DECODE ) ) );
                CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong protocol found", INET_PROT_CMIS, aUrl.GetProtocol(  ) );
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
        CPPUNIT_TEST_SUITE_END(  );
    };                          // class createPool


    CPPUNIT_TEST_SUITE_REGISTRATION( urlobjTest );
}                               // namespace rtl_random


// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
