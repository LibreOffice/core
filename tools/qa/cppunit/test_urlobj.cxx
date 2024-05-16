/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <string>

#include <sal/types.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>

#define OUSTR_TO_STDSTR( oustr ) std::string( OUStringToOString( oustr, RTL_TEXTENCODING_ASCII_US ) )

template<> inline std::string CPPUNIT_NS::assertion_traits<INetProtocol>::toString(
    const INetProtocol& x )
{
    OStringStream ost;
    ost << static_cast<unsigned int>(x);
    return ost.str();
}

namespace tools_urlobj
{

    class urlobjTest:public CppUnit::TestFixture
    {

      public:
        // insert your test code here.
        // this is only demonstration code
        void urlobjTest_001(  )
        {
            INetURLObject aUrl( u"file://10.10.1.1/sampledir/sample.file" );
            CPPUNIT_ASSERT_EQUAL(INetProtocol::File, aUrl.GetProtocol());
            CPPUNIT_ASSERT_EQUAL(u"10.10.1.1"_ustr,
                                 aUrl.GetHost(INetURLObject::DecodeMechanism::NONE));
            CPPUNIT_ASSERT_EQUAL(u"/sampledir/sample.file"_ustr,
                                 aUrl.GetURLPath(INetURLObject::DecodeMechanism::NONE));
            CPPUNIT_ASSERT_EQUAL(u"sample.file"_ustr,
                                 aUrl.getName());
            CPPUNIT_ASSERT_EQUAL(u"sample"_ustr, aUrl.getBase());
            CPPUNIT_ASSERT_EQUAL(u"file"_ustr, aUrl.getExtension());
        }

        void urlobjTest_004(  )
        {
            INetURLObject aUrl( u"smb://10.10.1.1/sampledir/sample.file" );
            CPPUNIT_ASSERT_EQUAL( INetProtocol::Smb, aUrl.GetProtocol(  ) );
            CPPUNIT_ASSERT_EQUAL(u"10.10.1.1"_ustr,
                                 aUrl.GetHost(INetURLObject::DecodeMechanism::NONE));
            CPPUNIT_ASSERT_EQUAL(u"/sampledir/sample.file"_ustr,
                                 aUrl.GetURLPath(INetURLObject::DecodeMechanism::NONE));
            CPPUNIT_ASSERT_EQUAL(u"sample.file"_ustr, aUrl.getName());
            CPPUNIT_ASSERT_EQUAL(u"sample"_ustr, aUrl.getBase());
            CPPUNIT_ASSERT_EQUAL(u"file"_ustr, aUrl.getExtension());
        }

        void urlobjCmisTest(  )
        {
            // Test with a username part
            {
                INetURLObject aUrl( u"vnd.libreoffice.cmis://username@http:%2F%2Ffoo.bar.com:8080%2Fmy%2Fcmis%2Fatom%23repo-id-encoded/path/to/content" );
                CPPUNIT_ASSERT_EQUAL( std::string( "http://foo.bar.com:8080/my/cmis/atom#repo-id-encoded" ),
                        OUSTR_TO_STDSTR( aUrl.GetHost( INetURLObject::DecodeMechanism::WithCharset ) ) );
                CPPUNIT_ASSERT_EQUAL( std::string( "username" ), OUSTR_TO_STDSTR( aUrl.GetUser( ) ) );
                CPPUNIT_ASSERT_EQUAL( std::string( "/path/to/content" ),
                        OUSTR_TO_STDSTR( aUrl.GetURLPath( INetURLObject::DecodeMechanism::NONE ) ) );
                CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong protocol found", INetProtocol::Cmis, aUrl.GetProtocol(  ) );
            }

            // Test without a username part
            {
                INetURLObject aUrl(
                                u"vnd.libreoffice.cmis://http:%2F%2Ffoo.bar.com:8080%2Fmy%2Fcmis%2Fatom%23repo-id-encoded/path/to/content" );
                CPPUNIT_ASSERT_EQUAL( std::string( "http://foo.bar.com:8080/my/cmis/atom#repo-id-encoded" ),
                        OUSTR_TO_STDSTR( aUrl.GetHost( INetURLObject::DecodeMechanism::WithCharset ) ) );
                CPPUNIT_ASSERT( !aUrl.HasUserData() );
                CPPUNIT_ASSERT_EQUAL( std::string( "/path/to/content" ),
                        OUSTR_TO_STDSTR( aUrl.GetURLPath( INetURLObject::DecodeMechanism::NONE ) ) );
                CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong protocol found", INetProtocol::Cmis, aUrl.GetProtocol(  ) );
            }
        }

        void urlobjTest_emptyPath() {
            {
                INetURLObject url(u"http://example.com");
                CPPUNIT_ASSERT_EQUAL(INetProtocol::Http, url.GetProtocol());
                CPPUNIT_ASSERT_EQUAL(u"example.com"_ustr, url.GetHost());
                CPPUNIT_ASSERT_EQUAL(u"/"_ustr, url.GetURLPath());
            }
            {
                // This is a valid http URL per RFC 7230:
                INetURLObject url(u"http://example.com?query");
                CPPUNIT_ASSERT(!url.HasError());
            }
            {
                INetURLObject url(u"http://example.com#fragment");
                CPPUNIT_ASSERT_EQUAL(INetProtocol::Http, url.GetProtocol());
                CPPUNIT_ASSERT_EQUAL(u"example.com"_ustr, url.GetHost());
                CPPUNIT_ASSERT_EQUAL(u"/"_ustr, url.GetURLPath());
                CPPUNIT_ASSERT_EQUAL(u"fragment"_ustr, url.GetMark());
            }
        }

        void urlobjTest_data() {
            INetURLObject url;
            std::unique_ptr<SvMemoryStream> strm;
            unsigned char const * buf;

            url = INetURLObject(u"data:");
            //TODO: CPPUNIT_ASSERT(url.HasError());
            strm = url.getData();
            CPPUNIT_ASSERT(!strm);

            url = INetURLObject(u"data:,");
            CPPUNIT_ASSERT(!url.HasError());
            strm = url.getData();
            CPPUNIT_ASSERT(strm != nullptr);
            CPPUNIT_ASSERT_EQUAL(sal_uInt64(0), strm->GetSize());
            strm.reset();

            url = INetURLObject(u"data:,,%C3%A4%90");
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

            url = INetURLObject(u"data:base64,");
            //TODO: CPPUNIT_ASSERT(url.HasError());
            strm = url.getData();
            CPPUNIT_ASSERT(!strm);

            url = INetURLObject(u"data:;base64,");
            CPPUNIT_ASSERT(!url.HasError());
            strm = url.getData();
            CPPUNIT_ASSERT(strm != nullptr);
            CPPUNIT_ASSERT_EQUAL(sal_uInt64(0), strm->GetSize());
            strm.reset();

            url = INetURLObject(u"data:;bAsE64,");
            CPPUNIT_ASSERT(!url.HasError());
            strm = url.getData();
            CPPUNIT_ASSERT(strm != nullptr);
            CPPUNIT_ASSERT_EQUAL(sal_uInt64(0), strm->GetSize());
            strm.reset();

            url = INetURLObject(u"data:;base64,YWJjCg==");
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

            url = INetURLObject(u"data:;base64,YWJjCg=");
            CPPUNIT_ASSERT(!url.HasError());
            strm = url.getData();
            CPPUNIT_ASSERT(!strm);

            url = INetURLObject(u"data:;base64,YWJ$Cg==");
            CPPUNIT_ASSERT(!url.HasError());
            strm = url.getData();
            CPPUNIT_ASSERT(!strm);

            url = INetURLObject(u"data:text/plain;param=%22;base64,%22,YQ==");
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

            url = INetURLObject(u"http://example.com");
            CPPUNIT_ASSERT(!url.HasError());
            strm = url.getData();
            CPPUNIT_ASSERT(!strm);
        }

        void urlobjTest_isSchemeEqualTo() {
            CPPUNIT_ASSERT(INetURLObject().isSchemeEqualTo(INetProtocol::NotValid));
            CPPUNIT_ASSERT(!INetURLObject().isSchemeEqualTo(u""));
            CPPUNIT_ASSERT(
                INetURLObject(u"http://example.org").isSchemeEqualTo(
                    INetProtocol::Http));
            CPPUNIT_ASSERT(
                !INetURLObject(u"http://example.org").isSchemeEqualTo(
                    INetProtocol::Https));
            CPPUNIT_ASSERT(
                INetURLObject(u"http://example.org").isSchemeEqualTo(u"Http"));
            CPPUNIT_ASSERT(
                !INetURLObject(u"http://example.org").isSchemeEqualTo(u"dav"));
            CPPUNIT_ASSERT(
                INetURLObject(u"dav://example.org").isSchemeEqualTo(u"dav"));
        }

        void urlobjTest_isAnyKnownWebDAVScheme() {
            CPPUNIT_ASSERT(
                INetURLObject(u"http://example.org").isAnyKnownWebDAVScheme());
            CPPUNIT_ASSERT(
                INetURLObject(u"https://example.org").isAnyKnownWebDAVScheme());
            CPPUNIT_ASSERT(
                INetURLObject(u"vnd.sun.star.webdav://example.org").isAnyKnownWebDAVScheme());
            CPPUNIT_ASSERT(
                INetURLObject(u"vnd.sun.star.webdavs://example.org").isAnyKnownWebDAVScheme());
            CPPUNIT_ASSERT(
                !INetURLObject(u"ftp://example.org").isAnyKnownWebDAVScheme());
            CPPUNIT_ASSERT(
                !INetURLObject(u"file://example.org").isAnyKnownWebDAVScheme());
            CPPUNIT_ASSERT(
                !INetURLObject(u"dav://example.org").isAnyKnownWebDAVScheme());
            CPPUNIT_ASSERT(
                !INetURLObject(u"davs://example.org").isAnyKnownWebDAVScheme());
            CPPUNIT_ASSERT(
                !INetURLObject(u"vnd.sun.star.pkg://example.org").isAnyKnownWebDAVScheme());
        }

        void testSetName() {
            {
                INetURLObject obj(u"file:///");
                bool ok = obj.setName(u"foo");
                CPPUNIT_ASSERT(!ok);
            }
            {
                INetURLObject obj(u"file:///foo");
                bool ok = obj.setName(u"bar");
                CPPUNIT_ASSERT(ok);
                CPPUNIT_ASSERT_EQUAL(
                    u"file:///bar"_ustr, obj.GetMainURL(INetURLObject::DecodeMechanism::NONE));
            }
            {
                INetURLObject obj(u"file:///foo/");
                bool ok = obj.setName(u"bar");
                CPPUNIT_ASSERT(ok);
                CPPUNIT_ASSERT_EQUAL(
                    u"file:///bar/"_ustr, obj.GetMainURL(INetURLObject::DecodeMechanism::NONE));
            }
            {
                INetURLObject obj(u"file:///foo/bar");
                bool ok = obj.setName(u"baz");
                CPPUNIT_ASSERT(ok);
                CPPUNIT_ASSERT_EQUAL(
                    u"file:///foo/baz"_ustr,
                    obj.GetMainURL(INetURLObject::DecodeMechanism::NONE));
            }
            {
                INetURLObject obj(u"file:///foo/bar/");
                bool ok = obj.setName(u"baz");
                CPPUNIT_ASSERT(ok);
                CPPUNIT_ASSERT_EQUAL(
                    u"file:///foo/baz/"_ustr,
                    obj.GetMainURL(INetURLObject::DecodeMechanism::NONE));
            }
        }

        void testSetExtension() {
            INetURLObject obj(u"file:///foo/bar.baz/");
            bool ok = obj.setExtension(
                u"other", INetURLObject::LAST_SEGMENT, false);
            CPPUNIT_ASSERT(ok);
            CPPUNIT_ASSERT_EQUAL(
                u"file:///foo/bar.baz/.other"_ustr,
                obj.GetMainURL(INetURLObject::DecodeMechanism::NONE));
        }

        void testChangeScheme() {
            INetURLObject obj(u"unknown://example.com/foo/bar");
            CPPUNIT_ASSERT(!obj.HasError());
            obj.changeScheme(INetProtocol::Http);
            CPPUNIT_ASSERT_EQUAL(
                u"http://example.com/foo/bar"_ustr,
                obj.GetMainURL(INetURLObject::DecodeMechanism::NONE));
            obj.changeScheme(INetProtocol::Https);
            CPPUNIT_ASSERT_EQUAL(
                u"https://example.com/foo/bar"_ustr,
                obj.GetMainURL(INetURLObject::DecodeMechanism::NONE));
            obj.changeScheme(INetProtocol::Ftp);
            CPPUNIT_ASSERT_EQUAL(
                u"ftp://example.com/foo/bar"_ustr,
                obj.GetMainURL(INetURLObject::DecodeMechanism::NONE));
        }

        void testTd146382() {
            INetURLObject obj(u"file://share.allotropia.de@SSL/DavWWWRoot/remote.php");
            CPPUNIT_ASSERT(!obj.HasError());
            CPPUNIT_ASSERT_EQUAL(
                u"file://share.allotropia.de@SSL/DavWWWRoot/remote.php"_ustr,
                obj.GetMainURL(INetURLObject::DecodeMechanism::NONE));
        }

        void testParseSmart()
        {
            {
                // host:port must not be misinterpreted as scheme:path
                INetURLObject obj(u"example.com:8080/foo", INetProtocol::Http);
                CPPUNIT_ASSERT(!obj.HasError());
                CPPUNIT_ASSERT_EQUAL(u"http://example.com:8080/foo"_ustr,
                    obj.GetMainURL(INetURLObject::DecodeMechanism::NONE));
                CPPUNIT_ASSERT_EQUAL(INetProtocol::Http, obj.GetProtocol());
                CPPUNIT_ASSERT_EQUAL(u"example.com"_ustr, obj.GetHost());
                CPPUNIT_ASSERT_EQUAL(sal_uInt32(8080), obj.GetPort());
                CPPUNIT_ASSERT_EQUAL(u"/foo"_ustr, obj.GetURLPath());
            }
            {
                // port may only contain decimal digits, so this must be treated as unknown scheme
                INetURLObject obj(u"example.com:80a0/foo", INetProtocol::Http);
                CPPUNIT_ASSERT(!obj.HasError());
                CPPUNIT_ASSERT_EQUAL(u"example.com:80a0/foo"_ustr,
                    obj.GetMainURL(INetURLObject::DecodeMechanism::NONE));
                CPPUNIT_ASSERT_EQUAL(INetProtocol::Generic, obj.GetProtocol());
                CPPUNIT_ASSERT(obj.isSchemeEqualTo(u"example.com"));
                CPPUNIT_ASSERT_EQUAL(u""_ustr, obj.GetHost());
                CPPUNIT_ASSERT_EQUAL(u"80a0/foo"_ustr, obj.GetURLPath());
            }
            {
                // Test Windows \\?\C:... long path scheme
                INetURLObject base(u"file:///C:/foo"); // set up base path
                bool bWasAbsolute = false;
                INetURLObject obj
                    = base.smartRel2Abs(u"\\\\?\\D:\\bar\\baz.ext"_ustr, bWasAbsolute);
                CPPUNIT_ASSERT(bWasAbsolute);
                CPPUNIT_ASSERT_EQUAL(u"file:///D:/bar/baz.ext"_ustr,
                                     obj.GetMainURL(INetURLObject::DecodeMechanism::NONE));
            }
            {
                // Test Windows \\?\UNC\Server... long path scheme
                INetURLObject base(u"file://ServerFoo/fooShare"); // set up base path
                bool bWasAbsolute = false;
                INetURLObject obj = base.smartRel2Abs(
                    u"\\\\?\\UNC\\ServerBar\\barShare\\baz.ext"_ustr, bWasAbsolute);
                CPPUNIT_ASSERT(bWasAbsolute);
                CPPUNIT_ASSERT_EQUAL(u"file://ServerBar/barShare/baz.ext"_ustr,
                                     obj.GetMainURL(INetURLObject::DecodeMechanism::NONE));
            }
        }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE( urlobjTest );
        CPPUNIT_TEST( urlobjTest_001 );
        CPPUNIT_TEST( urlobjTest_004 );
        CPPUNIT_TEST( urlobjCmisTest );
        CPPUNIT_TEST( urlobjTest_emptyPath );
        CPPUNIT_TEST( urlobjTest_data );
        CPPUNIT_TEST( urlobjTest_isSchemeEqualTo );
        CPPUNIT_TEST( urlobjTest_isAnyKnownWebDAVScheme );
        CPPUNIT_TEST( testSetName );
        CPPUNIT_TEST( testSetExtension );
        CPPUNIT_TEST( testChangeScheme );
        CPPUNIT_TEST( testTd146382 );
        CPPUNIT_TEST( testParseSmart );
        CPPUNIT_TEST_SUITE_END(  );
    };                          // class createPool


    CPPUNIT_TEST_SUITE_REGISTRATION( urlobjTest );
}                               // namespace rtl_random


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
