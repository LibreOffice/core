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
        CPPUNIT_ASSERT_EQUAL( u"http"_ustr, aURI.GetScheme() );
        CPPUNIT_ASSERT_EQUAL( u"server.biz"_ustr, aURI.GetHost() );
        CPPUNIT_ASSERT_EQUAL( u"user%40anothername"_ustr, aURI.GetUser() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt16( 8040 ), aURI.GetPort() );
        CPPUNIT_ASSERT_EQUAL( u"/aService/asegment/nextsegment/check.this?test=true&link=http://anotherserver.com/%3Fcheck=theapplication%26os=linuxintel%26lang=en-US%26version=5.2.0"_ustr, aURI.GetRelativeReference() );

        CurlUri uri2(aURI.CloneWithRelativeRefPathAbsolute(u"/foo/bar"));
        CPPUNIT_ASSERT_EQUAL( u"http"_ustr, uri2.GetScheme() );
        CPPUNIT_ASSERT_EQUAL( u"server.biz"_ustr, uri2.GetHost() );
        CPPUNIT_ASSERT_EQUAL( u"user%40anothername"_ustr, uri2.GetUser() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt16(8040), uri2.GetPort() );
        CPPUNIT_ASSERT_EQUAL( u"/foo/bar"_ustr, uri2.GetRelativeReference() );
        CPPUNIT_ASSERT_EQUAL( u"http://user%40anothername@server.biz:8040/foo/bar"_ustr, uri2.GetURI() );

        CurlUri uri3(aURI.CloneWithRelativeRefPathAbsolute(u"/?query#fragment"));
        CPPUNIT_ASSERT_EQUAL( u"http"_ustr, uri3.GetScheme() );
        CPPUNIT_ASSERT_EQUAL( u"server.biz"_ustr, uri3.GetHost() );
        CPPUNIT_ASSERT_EQUAL( u"user%40anothername"_ustr, uri3.GetUser() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt16(8040), uri3.GetPort() );
        CPPUNIT_ASSERT_EQUAL( u"/?query#fragment"_ustr, uri3.GetRelativeReference() );
        CPPUNIT_ASSERT_EQUAL( u"http://user%40anothername@server.biz:8040/?query#fragment"_ustr, uri3.GetURI() );
    }

    void webdav_local_test::WebdavUriTest2()
    {
        CurlUri aURI(u"https://foo:bar@server.biz:8040/aService#aaa" );
        CPPUNIT_ASSERT_EQUAL( u"https"_ustr, aURI.GetScheme() );
        CPPUNIT_ASSERT_EQUAL( u"server.biz"_ustr, aURI.GetHost() );
        CPPUNIT_ASSERT_EQUAL( u"foo"_ustr, aURI.GetUser() );
        CPPUNIT_ASSERT_EQUAL( u"bar"_ustr, aURI.GetPassword() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt16( 8040 ), aURI.GetPort() );
        CPPUNIT_ASSERT_EQUAL( u"/aService#aaa"_ustr, aURI.GetRelativeReference() );

        CurlUri uri2(aURI.CloneWithRelativeRefPathAbsolute(u"/foo/bar"));
        CPPUNIT_ASSERT_EQUAL( u"https"_ustr, uri2.GetScheme() );
        CPPUNIT_ASSERT_EQUAL( u"server.biz"_ustr, uri2.GetHost() );
        CPPUNIT_ASSERT_EQUAL( u"foo"_ustr, uri2.GetUser() );
        CPPUNIT_ASSERT_EQUAL( u"bar"_ustr, uri2.GetPassword() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt16( 8040 ), uri2.GetPort() );
        CPPUNIT_ASSERT_EQUAL( u"/foo/bar"_ustr, uri2.GetRelativeReference() );
        CPPUNIT_ASSERT_EQUAL( u"https://foo:bar@server.biz:8040/foo/bar"_ustr, uri2.GetURI() );

        CurlUri uri3(aURI.CloneWithRelativeRefPathAbsolute(u"/?query"));
        CPPUNIT_ASSERT_EQUAL( u"https"_ustr, uri3.GetScheme() );
        CPPUNIT_ASSERT_EQUAL( u"server.biz"_ustr, uri3.GetHost() );
        CPPUNIT_ASSERT_EQUAL( u"foo"_ustr, uri3.GetUser() );
        CPPUNIT_ASSERT_EQUAL( u"bar"_ustr, uri3.GetPassword() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt16(8040), uri3.GetPort() );
        CPPUNIT_ASSERT_EQUAL( u"/?query"_ustr, uri3.GetRelativeReference() );
        CPPUNIT_ASSERT_EQUAL( u"https://foo:bar@server.biz:8040/?query"_ustr, uri3.GetURI() );
    }

    CPPUNIT_TEST_SUITE_REGISTRATION( webdav_local_test );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
