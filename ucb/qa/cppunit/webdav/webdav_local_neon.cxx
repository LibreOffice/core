/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <cppunit/TestFixture.h>
#include "NeonUri.hxx"


namespace webdav_local
{

    class webdav_local_test: public test::BootstrapFixture
    {

    public:
        webdav_local_test() : BootstrapFixture( true, true ) {}

        // initialise your test code values here.
        void setUp(  ) override;

        void tearDown(  ) override;

        void NeonUriTest();

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE( webdav_local_test );
        CPPUNIT_TEST( NeonUriTest );
        CPPUNIT_TEST_SUITE_END();
    };                          // class webdav_local_test

    // initialise your test code values here.
    void webdav_local_test::setUp()
    {
    }

    void webdav_local_test::tearDown()
    {
    }

    void webdav_local_test::NeonUriTest()
    {
        //try URL decomposition
        OUString aURL( "http://user%40anothername@server.biz:8040/aService/asegment/nextsegment/check.this?test=true&link=http://anotherserver.com/%3Fcheck=theapplication%26os=linuxintel%26lang=en-US%26version=5.2.0" );
        webdav_ucp::NeonUri aURI( aURL );
        CPPUNIT_ASSERT_MESSAGE( "webdav_ucp::NeonUri returned wrong scheme", aURI.GetScheme() == "http" );
        CPPUNIT_ASSERT_MESSAGE( "webdav_ucp::NeonUri returned wrong host", aURI.GetHost() == "server.biz" );
        CPPUNIT_ASSERT_MESSAGE( "webdav_ucp::NeonUri returned wrong host", aURI.GetUserInfo() == "user%40anothername" );
        CPPUNIT_ASSERT_MESSAGE( "webdav_ucp::NeonUri returned wrong port", aURI.GetPort() == 8040 );
        CPPUNIT_ASSERT_MESSAGE( "webdav_ucp::NeonUri returned wrong path", aURI.GetPath() == "/aService/asegment/nextsegment/check.this?test=true&link=http://anotherserver.com/%3Fcheck=theapplication%26os=linuxintel%26lang=en-US%26version=5.2.0" );
    }

    CPPUNIT_TEST_SUITE_REGISTRATION( webdav_local_test );
    CPPUNIT_PLUGIN_IMPLEMENT();
}                               // namespace webdav_local

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
