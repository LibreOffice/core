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
    }

    CPPUNIT_TEST_SUITE_REGISTRATION( webdav_local_test );
}                               // namespace rtl_random

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
