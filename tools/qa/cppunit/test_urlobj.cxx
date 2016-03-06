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
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>

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
        // insert your test code here.
        // this is only demonstration code
        void urlobjTest_001(  )
        {
            INetURLObject aUrl( OUString( "file://10.10.1.1/sampledir/sample.file" ) );
#ifdef LINUX
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE )
                            == "smb://10.10.1.1/sampledir/sample.file" );
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INetProtocol::Smb );
#endif
#ifdef _WIN32
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE ).
                            equalsAscii
                            ( "file://10.10.1.1/sampledir/sample.file" ) );
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INetProtocol::File );
#endif
            CPPUNIT_ASSERT( aUrl.GetHost( INetURLObject::NO_DECODE )
                            == "10.10.1.1" );
            CPPUNIT_ASSERT( aUrl.GetURLPath( INetURLObject::NO_DECODE )
                            == "/sampledir/sample.file" );
            CPPUNIT_ASSERT( aUrl.getName(  )
                            == "sample.file" );
            CPPUNIT_ASSERT( aUrl.getBase(  ) == "sample" );
            CPPUNIT_ASSERT( aUrl.getExtension(  ) == "file" );
        }

        void urlobjTest_002(  )
        {
            INetURLObject aUrl;
            aUrl.
                setFSysPath( "\\\\137.65.170.24\\c$\\Img0001.jpg",
                             INetURLObject::FSYS_DETECT );
#ifdef LINUX
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE )
                            == "smb://137.65.170.24/c$/Img0001.jpg" );
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INetProtocol::Smb );
#endif
#ifdef _WIN32
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE ).
                            equalsAscii( "file://137.65.170.24/c$/Img0001.jpg" ) );
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INetProtocol::File );
#endif
            CPPUNIT_ASSERT( aUrl.GetHost( INetURLObject::NO_DECODE )
                            == "137.65.170.24" );
            CPPUNIT_ASSERT( aUrl.GetURLPath( INetURLObject::NO_DECODE )
                            == "/c$/Img0001.jpg" );
            CPPUNIT_ASSERT( aUrl.getName(  ) == "Img0001.jpg" );
            CPPUNIT_ASSERT( aUrl.getBase(  ) == "Img0001" );
            CPPUNIT_ASSERT( aUrl.getExtension(  ) == "jpg" );
        }


        void urlobjTest_003(  )
        {
            INetURLObject aUrl;
            aUrl.
                setFSysPath( "\\\\hive-winxp-x86\\pmladek\\test2.odt",
                             INetURLObject::FSYS_DETECT );
#ifdef LINUX
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE )
                            == "smb://hive-winxp-x86/pmladek/test2.odt" );
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INetProtocol::Smb );
#endif
#ifdef _WIN32
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE ).
                            equalsAscii( "file://hive-winxp-x86/pmladek/test2.odt" ) );
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INetProtocol::File );
#endif
            CPPUNIT_ASSERT( aUrl.GetHost( INetURLObject::NO_DECODE )
                            == "hive-winxp-x86" );
            CPPUNIT_ASSERT( aUrl.GetURLPath( INetURLObject::NO_DECODE )
                            == "/pmladek/test2.odt" );
        }

        void urlobjTest_004(  )
        {
            INetURLObject aUrl( OUString( "smb://10.10.1.1/sampledir/sample.file" ) );
#ifdef LINUX
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE )
                            == "smb://10.10.1.1/sampledir/sample.file" );
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INetProtocol::Smb );
#endif
#ifdef _WIN32
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE ).
                            equalsAscii( "file://10.10.1.1/sampledir/sample.file" ) );
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INetProtocol::File );
#endif
            CPPUNIT_ASSERT( aUrl.GetHost( INetURLObject::NO_DECODE )
                            == "10.10.1.1" );
            CPPUNIT_ASSERT( aUrl.GetURLPath( INetURLObject::NO_DECODE )
                            == "/sampledir/sample.file" );
            CPPUNIT_ASSERT( aUrl.getName(  ) == "sample.file" );
            CPPUNIT_ASSERT( aUrl.getBase(  ) == "sample" );
            CPPUNIT_ASSERT( aUrl.getExtension(  ) == "file" );
        }

        void urlobjTest_005(  )
        {
            INetURLObject aUrl;
            aUrl.setFSysPath( "//137.65.170.24/c$/Img0001.jpg",
                              INetURLObject::FSYS_DETECT );
#ifdef LINUX
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE )
                            == "smb://137.65.170.24/c$/Img0001.jpg" );
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INetProtocol::Smb );
#endif
#ifdef _WIN32
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE ).
                            equalsAscii( "file://137.65.170.24/c$/Img0001.jpg" ) );
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INetProtocol::File );
#endif
            CPPUNIT_ASSERT( aUrl.GetHost( INetURLObject::NO_DECODE )
                            == "137.65.170.24" );
            CPPUNIT_ASSERT( aUrl.GetURLPath( INetURLObject::NO_DECODE )
                            == "/c$/Img0001.jpg" );
            CPPUNIT_ASSERT( aUrl.getName(  ) == "Img0001.jpg" );
            CPPUNIT_ASSERT( aUrl.getBase(  ) == "Img0001" );
            CPPUNIT_ASSERT( aUrl.getExtension(  ) == "jpg" );
        }


        void urlobjTest_006(  )
        {
            INetURLObject aUrl;
            aUrl.setFSysPath( "//hive-winxp-x86/pmladek/test2.odt",
                              INetURLObject::FSYS_DETECT );
#ifdef LINUX
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE )
                            == "smb://hive-winxp-x86/pmladek/test2.odt" );
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INetProtocol::Smb );
#endif
#ifdef _WIN32
            CPPUNIT_ASSERT( aUrl.GetMainURL( INetURLObject::NO_DECODE ).
                            equalsAscii( "file://hive-winxp-x86/pmladek/test2.odt" ) );
            CPPUNIT_ASSERT( aUrl.GetProtocol(  ) == INetProtocol::File );
#endif
            CPPUNIT_ASSERT( aUrl.GetHost( INetURLObject::NO_DECODE )
                            == "hive-winxp-x86" );
            CPPUNIT_ASSERT( aUrl.GetURLPath( INetURLObject::NO_DECODE )
                            == "/pmladek/test2.odt" );
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
                CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong protocol found", INetProtocol::Cmis, aUrl.GetProtocol(  ) );
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
                CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong protocol found", INetProtocol::Cmis, aUrl.GetProtocol(  ) );
            }
        }

        void urlobjTest_emptyPath() {
            {
                INetURLObject url(OUString("http://example.com"));
                CPPUNIT_ASSERT_EQUAL(INetProtocol::Http, url.GetProtocol());
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
                CPPUNIT_ASSERT_EQUAL(INetProtocol::Http, url.GetProtocol());
                CPPUNIT_ASSERT_EQUAL(OUString("example.com"), url.GetHost());
                CPPUNIT_ASSERT_EQUAL(OUString("/"), url.GetURLPath());
                CPPUNIT_ASSERT_EQUAL(OUString("fragment"), url.GetMark());
            }
        }

        void urlobjTest_data() {
            INetURLObject url;
            std::unique_ptr<SvMemoryStream> strm;
            unsigned char const * buf;

            url = INetURLObject("data:");
            //TODO: CPPUNIT_ASSERT(url.HasError());
            strm = url.getData();
            CPPUNIT_ASSERT(strm == nullptr);

            url = INetURLObject("data:,");
            CPPUNIT_ASSERT(!url.HasError());
            strm = url.getData();
            CPPUNIT_ASSERT(strm != nullptr);
            CPPUNIT_ASSERT_EQUAL(sal_uInt64(0), strm->GetSize());
            strm.reset();

            url = INetURLObject("data:,,%C3%A4%90");
            CPPUNIT_ASSERT(!url.HasError());
            strm = url.getData();
            CPPUNIT_ASSERT(strm != nullptr);
            CPPUNIT_ASSERT_EQUAL(sal_uInt64(4), strm->GetSize());
            buf = static_cast<unsigned char const *>(strm->GetData());
            CPPUNIT_ASSERT_EQUAL(0x2C, int(buf[0]));
            CPPUNIT_ASSERT_EQUAL(0xC3, int(buf[1]));
            CPPUNIT_ASSERT_EQUAL(0xA4, int(buf[2]));
            CPPUNIT_ASSERT_EQUAL(0x90, int(buf[3]));
            strm.reset();

            url = INetURLObject("data:base64,");
            //TODO: CPPUNIT_ASSERT(url.HasError());
            strm = url.getData();
            CPPUNIT_ASSERT(strm == nullptr);

            url = INetURLObject("data:;base64,");
            CPPUNIT_ASSERT(!url.HasError());
            strm = url.getData();
            CPPUNIT_ASSERT(strm != nullptr);
            CPPUNIT_ASSERT_EQUAL(sal_uInt64(0), strm->GetSize());
            strm.reset();

            url = INetURLObject("data:;bAsE64,");
            CPPUNIT_ASSERT(!url.HasError());
            strm = url.getData();
            CPPUNIT_ASSERT(strm != nullptr);
            CPPUNIT_ASSERT_EQUAL(sal_uInt64(0), strm->GetSize());
            strm.reset();

            url = INetURLObject("data:;base64,YWJjCg==");
            CPPUNIT_ASSERT(!url.HasError());
            strm = url.getData();
            CPPUNIT_ASSERT(strm != nullptr);
            CPPUNIT_ASSERT_EQUAL(sal_uInt64(4), strm->GetSize());
            buf = static_cast<unsigned char const *>(strm->GetData());
            CPPUNIT_ASSERT_EQUAL(0x61, int(buf[0]));
            CPPUNIT_ASSERT_EQUAL(0x62, int(buf[1]));
            CPPUNIT_ASSERT_EQUAL(0x63, int(buf[2]));
            CPPUNIT_ASSERT_EQUAL(0x0A, int(buf[3]));
            strm.reset();

            url = INetURLObject("data:;base64,YWJjCg=");
            CPPUNIT_ASSERT(!url.HasError());
            strm = url.getData();
            CPPUNIT_ASSERT(strm == nullptr);

            url = INetURLObject("data:;base64,YWJ$Cg==");
            CPPUNIT_ASSERT(!url.HasError());
            strm = url.getData();
            CPPUNIT_ASSERT(strm == nullptr);

            url = INetURLObject("data:text/plain;param=%22;base64,%22,YQ==");
            CPPUNIT_ASSERT(!url.HasError());
            strm = url.getData();
            CPPUNIT_ASSERT(strm != nullptr);
            CPPUNIT_ASSERT_EQUAL(sal_uInt64(4), strm->GetSize());
            buf = static_cast<unsigned char const *>(strm->GetData());
            CPPUNIT_ASSERT_EQUAL(0x59, int(buf[0]));
            CPPUNIT_ASSERT_EQUAL(0x51, int(buf[1]));
            CPPUNIT_ASSERT_EQUAL(0x3D, int(buf[2]));
            CPPUNIT_ASSERT_EQUAL(0x3D, int(buf[3]));
            strm.reset();

            url = INetURLObject("http://example.com");
            CPPUNIT_ASSERT(!url.HasError());
            strm = url.getData();
            CPPUNIT_ASSERT(strm == nullptr);
        }

        void urlobjTest_isSchemeEqualTo() {
            CPPUNIT_ASSERT(INetURLObject().isSchemeEqualTo(INetProtocol::NotValid));
            CPPUNIT_ASSERT(!INetURLObject().isSchemeEqualTo(""));
            CPPUNIT_ASSERT(
                INetURLObject("http://example.org").isSchemeEqualTo(
                    INetProtocol::Http));
            CPPUNIT_ASSERT(
                !INetURLObject("http://example.org").isSchemeEqualTo(
                    INetProtocol::Https));
            CPPUNIT_ASSERT(
                INetURLObject("http://example.org").isSchemeEqualTo("Http"));
            CPPUNIT_ASSERT(
                !INetURLObject("http://example.org").isSchemeEqualTo("dav"));
            CPPUNIT_ASSERT(
                INetURLObject("dav://example.org").isSchemeEqualTo("dav"));
        }

        void urlobjTest_isAnyKnownWebDAVScheme() {
            CPPUNIT_ASSERT(
                INetURLObject("http://example.org").isAnyKnownWebDAVScheme());
            CPPUNIT_ASSERT(
                INetURLObject("https://example.org").isAnyKnownWebDAVScheme());
            CPPUNIT_ASSERT(
                INetURLObject("vnd.sun.star.webdav://example.org").isAnyKnownWebDAVScheme());
            CPPUNIT_ASSERT(
                INetURLObject("vnd.sun.star.webdavs://example.org").isAnyKnownWebDAVScheme());
            CPPUNIT_ASSERT(
                !INetURLObject("ftp://example.org").isAnyKnownWebDAVScheme());
            CPPUNIT_ASSERT(
                !INetURLObject("file://example.org").isAnyKnownWebDAVScheme());
            CPPUNIT_ASSERT(
                !INetURLObject("dav://example.org").isAnyKnownWebDAVScheme());
            CPPUNIT_ASSERT(
                !INetURLObject("davs://example.org").isAnyKnownWebDAVScheme());
            CPPUNIT_ASSERT(
                !INetURLObject("vnd.sun.star.pkg://example.org").isAnyKnownWebDAVScheme());
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
        CPPUNIT_TEST( urlobjTest_data );
        CPPUNIT_TEST( urlobjTest_isSchemeEqualTo );
        CPPUNIT_TEST( urlobjTest_isAnyKnownWebDAVScheme );
        CPPUNIT_TEST_SUITE_END(  );
    };                          // class createPool


    CPPUNIT_TEST_SUITE_REGISTRATION( urlobjTest );
}                               // namespace rtl_random


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
