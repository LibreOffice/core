/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
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
        
        void setUp(  )
        {
        }

        void tearDown(  )
        {
        }

        
        
        void urlobjTest_001(  )
        {
            INetURLObject aUrl( OUString( "file:
#ifdef LINUX
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE ).
                            equalsAscii
                            ( "smb:
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INET_PROT_SMB );
#endif
#ifdef WIN
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE ).
                            equalsAscii
                            ( "file:
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INET_PROT_FILE );
#endif
            CPPUNIT_ASSERT( aUrl.GetHost( INetURLObject::NO_DECODE ).
                            equalsAscii( "10.10.1.1" ) );
            CPPUNIT_ASSERT( aUrl.GetURLPath( INetURLObject::NO_DECODE ).
                            equalsAscii( "/sampledir/sample.file" ) );
            CPPUNIT_ASSERT( aUrl.getName(  ).
                            equalsAscii( "sample.file" ) );
            CPPUNIT_ASSERT( aUrl.getBase(  ).equalsAscii( "sample" ) );
            CPPUNIT_ASSERT( aUrl.getExtension(  ).equalsAscii( "file" ) );
        }

        void urlobjTest_002(  )
        {
            INetURLObject aUrl;
            aUrl.
                setFSysPath( OUString( "\\\\137.65.170.24\\c$\\Img0001.jpg" ),
                             INetURLObject::FSYS_DETECT );
#ifdef LINUX
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE ).
                            equalsAscii( "smb:
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INET_PROT_SMB );
#endif
#ifdef WIN
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE ).
                            equalsAscii( "file:
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INET_PROT_FILE );
#endif
            CPPUNIT_ASSERT( aUrl.GetHost( INetURLObject::NO_DECODE ).
                            equalsAscii( "137.65.170.24" ) );
            CPPUNIT_ASSERT( aUrl.GetURLPath( INetURLObject::NO_DECODE ).
                            equalsAscii( "/c$/Img0001.jpg" ) );
            CPPUNIT_ASSERT( aUrl.getName(  ).
                            equalsAscii( "Img0001.jpg" ) );
            CPPUNIT_ASSERT( aUrl.getBase(  ).equalsAscii( "Img0001" ) );
            CPPUNIT_ASSERT( aUrl.getExtension(  ).equalsAscii( "jpg" ) );
        }


        void urlobjTest_003(  )
        {
            INetURLObject aUrl;
            aUrl.
                setFSysPath( OUString( "\\\\hive-winxp-x86\\pmladek\\test2.odt" ),
                             INetURLObject::FSYS_DETECT );
#ifdef LINUX
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE ).
                            equalsAscii( "smb:
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INET_PROT_SMB );
#endif
#ifdef WIN
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE ).
                            equalsAscii( "file:
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INET_PROT_FILE );
#endif
            CPPUNIT_ASSERT( aUrl.GetHost( INetURLObject::NO_DECODE ).
                            equalsAscii( "hive-winxp-x86" ) );
            CPPUNIT_ASSERT( aUrl.GetURLPath( INetURLObject::NO_DECODE ).
                            equalsAscii( "/pmladek/test2.odt" ) );
        }

        void urlobjTest_004(  )
        {
            INetURLObject aUrl( OUString( "smb:
#ifdef LINUX
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE ).
                            equalsAscii( "smb:
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INET_PROT_SMB );
#endif
#ifdef WIN
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE ).
                            equalsAscii( "file:
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INET_PROT_FILE );
#endif
            CPPUNIT_ASSERT( aUrl.GetHost( INetURLObject::NO_DECODE ).
                            equalsAscii( "10.10.1.1" ) );
            CPPUNIT_ASSERT( aUrl.GetURLPath( INetURLObject::NO_DECODE ).
                            equalsAscii( "/sampledir/sample.file" ) );
            CPPUNIT_ASSERT( aUrl.getName(  ).
                            equalsAscii( "sample.file" ) );
            CPPUNIT_ASSERT( aUrl.getBase(  ).equalsAscii( "sample" ) );
            CPPUNIT_ASSERT( aUrl.getExtension(  ).equalsAscii( "file" ) );
        }

        void urlobjTest_005(  )
        {
            INetURLObject aUrl;
            aUrl.setFSysPath( OUString( "
                              INetURLObject::FSYS_DETECT );
#ifdef LINUX
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE ).
                            equalsAscii( "smb:
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INET_PROT_SMB );
#endif
#ifdef WIN
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE ).
                            equalsAscii( "file:
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INET_PROT_FILE );
#endif
            CPPUNIT_ASSERT( aUrl.GetHost( INetURLObject::NO_DECODE ).
                            equalsAscii( "137.65.170.24" ) );
            CPPUNIT_ASSERT( aUrl.GetURLPath( INetURLObject::NO_DECODE ).
                            equalsAscii( "/c$/Img0001.jpg" ) );
            CPPUNIT_ASSERT( aUrl.getName(  ).equalsAscii( "Img0001.jpg" ) );
            CPPUNIT_ASSERT( aUrl.getBase(  ).equalsAscii( "Img0001" ) );
            CPPUNIT_ASSERT( aUrl.getExtension(  ).equalsAscii( "jpg" ) );
        }


        void urlobjTest_006(  )
        {
            INetURLObject aUrl;
            aUrl.setFSysPath( OUString( "
                              INetURLObject::FSYS_DETECT );
#ifdef LINUX
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE ).
                            equalsAscii( "smb:
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INET_PROT_SMB );
#endif
#ifdef WIN
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE ).
                            equalsAscii( "file:
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INET_PROT_FILE );
#endif
            CPPUNIT_ASSERT( aUrl.GetHost( INetURLObject::NO_DECODE ).
                            equalsAscii( "hive-winxp-x86" ) );
            CPPUNIT_ASSERT( aUrl.GetURLPath( INetURLObject::NO_DECODE ).
                            equalsAscii( "/pmladek/test2.odt" ) );
        }

        void urlobjCmisTest(  )
        {
            
            {
                INetURLObject aUrl( OUString( "vnd.libreoffice.cmis:
                CPPUNIT_ASSERT_EQUAL( std::string( "http:
                        OUSTR_TO_STDSTR( aUrl.GetHost( INetURLObject::DECODE_WITH_CHARSET ) ) );
                CPPUNIT_ASSERT_EQUAL( std::string( "username" ), OUSTR_TO_STDSTR( aUrl.GetUser( ) ) );
                CPPUNIT_ASSERT_EQUAL( std::string( "/path/to/content" ),
                        OUSTR_TO_STDSTR( aUrl.GetURLPath( INetURLObject::NO_DECODE ) ) );
                CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong protocol found", INET_PROT_CMIS, aUrl.GetProtocol(  ) );
            }

            
            {
                INetURLObject aUrl( OUString(
                                "vnd.libreoffice.cmis:
                CPPUNIT_ASSERT_EQUAL( std::string( "http:
                        OUSTR_TO_STDSTR( aUrl.GetHost( INetURLObject::DECODE_WITH_CHARSET ) ) );
                CPPUNIT_ASSERT( !aUrl.HasUserData() );
                CPPUNIT_ASSERT_EQUAL( std::string( "/path/to/content" ),
                        OUSTR_TO_STDSTR( aUrl.GetURLPath( INetURLObject::NO_DECODE ) ) );
                CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong protocol found", INET_PROT_CMIS, aUrl.GetProtocol(  ) );
            }
        }

        void urlobjTest_emptyPath() {
            {
                INetURLObject url(OUString("http:
                CPPUNIT_ASSERT_EQUAL(INET_PROT_HTTP, url.GetProtocol());
                CPPUNIT_ASSERT_EQUAL(OUString("example.com"), url.GetHost());
                CPPUNIT_ASSERT_EQUAL(OUString("/"), url.GetURLPath());
            }
            {
                
                INetURLObject url(OUString("http:
                CPPUNIT_ASSERT(url.HasError());
            }
            {
                INetURLObject url(OUString("http:
                CPPUNIT_ASSERT_EQUAL(INET_PROT_HTTP, url.GetProtocol());
                CPPUNIT_ASSERT_EQUAL(OUString("example.com"), url.GetHost());
                CPPUNIT_ASSERT_EQUAL(OUString("/"), url.GetURLPath());
                CPPUNIT_ASSERT_EQUAL(OUString("fragment"), url.GetMark());
            }
        }

        
        
        

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
    };                          


    CPPUNIT_TEST_SUITE_REGISTRATION( urlobjTest );
}                               




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
