/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <CurlUri.hxx>

using namespace http_dav_ucp;

namespace
{

    class webdav_local_test: public CppUnit::TestFixture
    {

    public:
        void WebdavUriTest();
        void WebdavUriTest2();

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE( webdav_local_test );
        CPPUNIT_TEST( WebdavUriTest );
        CPPUNIT_TEST( WebdavUriTest2 );
        CPPUNIT_TEST_SUITE_END();
    };                          // class webdav_local_test

    void webdav_local_test::WebdavUriTest()
    {
        //try URL decomposition
        CurlUri aURI(u"http://user%40anothername@server.biz:8040/aService/asegment/nextsegment/check.this?test=true&link=http://anotherserver.com/%3Fcheck=theapplication%26os=linuxintel%26lang=en-US%26version=5.2.0" );
        CPPUNIT_ASSERT_EQUAL( OUString( "http" ), aURI.GetScheme() );
        CPPUNIT_ASSERT_EQUAL( OUString( "server.biz" ), aURI.GetHost() );
        CPPUNIT_ASSERT_EQUAL( OUString( "user%40anothername" ), aURI.GetUser() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt16( 8040 ), aURI.GetPort() );
        CPPUNIT_ASSERT_EQUAL( OUString( "/aService/asegment/nextsegment/check.this?test=true&link=http://anotherserver.com/%3Fcheck=theapplication%26os=linuxintel%26lang=en-US%26version=5.2.0" ), aURI.GetRelativeReference() );

        CurlUri uri2(aURI.CloneWithRelativeRefPathAbsolute(u"/foo/bar"));
        CPPUNIT_ASSERT_EQUAL( OUString("http"), uri2.GetScheme() );
        CPPUNIT_ASSERT_EQUAL( OUString("server.biz"), uri2.GetHost() );
        CPPUNIT_ASSERT_EQUAL( OUString("user%40anothername"), uri2.GetUser() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt16(8040), uri2.GetPort() );
        CPPUNIT_ASSERT_EQUAL( OUString("/foo/bar"), uri2.GetRelativeReference() );
        CPPUNIT_ASSERT_EQUAL( OUString("http://user%40anothername@server.biz:8040/foo/bar"), uri2.GetURI() );

        CurlUri uri3(aURI.CloneWithRelativeRefPathAbsolute(u"/?query#fragment"));
        CPPUNIT_ASSERT_EQUAL( OUString("http"), uri3.GetScheme() );
        CPPUNIT_ASSERT_EQUAL( OUString("server.biz"), uri3.GetHost() );
        CPPUNIT_ASSERT_EQUAL( OUString("user%40anothername"), uri3.GetUser() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt16(8040), uri3.GetPort() );
        CPPUNIT_ASSERT_EQUAL( OUString("/?query#fragment"), uri3.GetRelativeReference() );
        CPPUNIT_ASSERT_EQUAL( OUString("http://user%40anothername@server.biz:8040/?query#fragment"), uri3.GetURI() );
    }

    void webdav_local_test::WebdavUriTest2()
    {
        CurlUri aURI(u"https://foo:bar@server.biz:8040/aService#aaa" );
        CPPUNIT_ASSERT_EQUAL( OUString("https"), aURI.GetScheme() );
        CPPUNIT_ASSERT_EQUAL( OUString("server.biz"), aURI.GetHost() );
        CPPUNIT_ASSERT_EQUAL( OUString("foo"), aURI.GetUser() );
        CPPUNIT_ASSERT_EQUAL( OUString("bar"), aURI.GetPassword() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt16( 8040 ), aURI.GetPort() );
        CPPUNIT_ASSERT_EQUAL( OUString( "/aService#aaa" ), aURI.GetRelativeReference() );

        CurlUri uri2(aURI.CloneWithRelativeRefPathAbsolute(u"/foo/bar"));
        CPPUNIT_ASSERT_EQUAL( OUString("https"), uri2.GetScheme() );
        CPPUNIT_ASSERT_EQUAL( OUString("server.biz"), uri2.GetHost() );
        CPPUNIT_ASSERT_EQUAL( OUString("foo"), uri2.GetUser() );
        CPPUNIT_ASSERT_EQUAL( OUString("bar"), uri2.GetPassword() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt16( 8040 ), uri2.GetPort() );
        CPPUNIT_ASSERT_EQUAL( OUString("/foo/bar"), uri2.GetRelativeReference() );
        CPPUNIT_ASSERT_EQUAL( OUString("https://foo:bar@server.biz:8040/foo/bar"), uri2.GetURI() );

        CurlUri uri3(aURI.CloneWithRelativeRefPathAbsolute(u"/?query"));
        CPPUNIT_ASSERT_EQUAL( OUString("https"), uri3.GetScheme() );
        CPPUNIT_ASSERT_EQUAL( OUString("server.biz"), uri3.GetHost() );
        CPPUNIT_ASSERT_EQUAL( OUString("foo"), uri3.GetUser() );
        CPPUNIT_ASSERT_EQUAL( OUString("bar"), uri3.GetPassword() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt16(8040), uri3.GetPort() );
        CPPUNIT_ASSERT_EQUAL( OUString("/?query"), uri3.GetRelativeReference() );
        CPPUNIT_ASSERT_EQUAL( OUString("https://foo:bar@server.biz:8040/?query"), uri3.GetURI() );
    }

    CPPUNIT_TEST_SUITE_REGISTRATION( webdav_local_test );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
