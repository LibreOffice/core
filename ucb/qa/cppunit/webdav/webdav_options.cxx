/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <cppunit/plugin/TestPlugIn.h>
#include "DAVResourceAccess.hxx"
#include "DAVException.hxx"
#include "DAVTypes.hxx"

namespace
{

    class webdav_opts_test: public test::BootstrapFixture
    {

    public:
        webdav_opts_test() : BootstrapFixture( true, true ) {}

        // initialise your test code values here.
        void setUp(  ) override;

        void tearDown(  ) override;

        void DAVTypesCheckInit( webdav_ucp::DAVOptions aDavType );
        void DAVTypesTest();

        void DAVOptsCacheTests();

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE( webdav_opts_test );
        CPPUNIT_TEST( DAVTypesTest );
        CPPUNIT_TEST( DAVOptsCacheTests );
        CPPUNIT_TEST_SUITE_END();
    };                          // class webdav_local_test

    // initialise your test code values here.
    void webdav_opts_test::setUp()
    {
    }

    void webdav_opts_test::tearDown()
    {
    }

    void webdav_opts_test::DAVTypesCheckInit( webdav_ucp::DAVOptions aDavType )
    {
        // check if the class is at reset state
        // using accessors
        CPPUNIT_ASSERT_EQUAL( false, aDavType.isClass1() );
        CPPUNIT_ASSERT_EQUAL( false, aDavType.isClass2() );
        CPPUNIT_ASSERT_EQUAL( false, aDavType.isClass3() );
        CPPUNIT_ASSERT_EQUAL( false, aDavType.isLocked() );
        CPPUNIT_ASSERT_EQUAL( true, aDavType.isHeadAllowed() );
        CPPUNIT_ASSERT_EQUAL( true, aDavType.getAllowedMethods().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( false, aDavType.isLockAllowed() );
        CPPUNIT_ASSERT_EQUAL( true, aDavType.getURL().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( true, aDavType.getRedirectedURL().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt32( 0 ), aDavType.getStaleTime() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt16( 0 ), aDavType.getHttpResponseStatusCode() );
        CPPUNIT_ASSERT_EQUAL( true, aDavType.getHttpResponseStatusText().isEmpty() );
    }

    void webdav_opts_test::DAVTypesTest()
    {
        //our DAVOptions
        webdav_ucp::DAVOptions aDavOpt;
        DAVTypesCheckInit( aDavOpt );

        aDavOpt.setClass1();
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.isClass1() );
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isClass2() );
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isClass3() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.isHeadAllowed() );
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isLockAllowed() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getAllowedMethods().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt32( 0 ), aDavOpt.getStaleTime() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getURL().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getRedirectedURL().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt16( 0 ), aDavOpt.getHttpResponseStatusCode() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getHttpResponseStatusText().isEmpty() );

        aDavOpt.setClass1( false );
        aDavOpt.setClass2();
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isClass1() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.isClass2() );
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isClass3() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.isHeadAllowed() );
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isLockAllowed() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getAllowedMethods().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt32( 0 ), aDavOpt.getStaleTime() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getURL().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getRedirectedURL().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt16( 0 ), aDavOpt.getHttpResponseStatusCode() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getHttpResponseStatusText().isEmpty() );

        aDavOpt.setClass2( false );
        aDavOpt.setClass3();
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isClass1() );
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isClass2() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.isClass3() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.isHeadAllowed() );
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isLockAllowed() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getAllowedMethods().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt32( 0 ), aDavOpt.getStaleTime() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getURL().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getRedirectedURL().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt16( 0 ), aDavOpt.getHttpResponseStatusCode() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getHttpResponseStatusText().isEmpty() );

        aDavOpt.setClass3( false );

        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isClass1() );
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isClass2() );
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isClass3() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.isHeadAllowed() );
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isLockAllowed() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getAllowedMethods().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt32( 0 ), aDavOpt.getStaleTime() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getURL().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getRedirectedURL().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt16( 0 ), aDavOpt.getHttpResponseStatusCode() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getHttpResponseStatusText().isEmpty() );

        DAVTypesCheckInit( aDavOpt );
        //example of allowed method for a Web resource
        OUString aAllowedMethods = "POST,OPTIONS,GET,HEAD,TRACE";
        aDavOpt.setAllowedMethods( aAllowedMethods );
        // now check...
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isClass1() );
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isClass2() );
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isClass3() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.isHeadAllowed() );
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isLockAllowed() );
        CPPUNIT_ASSERT_EQUAL( aAllowedMethods, aDavOpt.getAllowedMethods() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt32( 0 ), aDavOpt.getStaleTime() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getURL().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getRedirectedURL().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt16( 0 ), aDavOpt.getHttpResponseStatusCode() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getHttpResponseStatusText().isEmpty() );

        //example of allowed method for a WebDAV resource supporting LOCK
        aAllowedMethods = "OPTIONS,GET,HEAD,POST,DELETE,TRACE,PROPFIND,PROPPATCH,COPY,MOVE,PUT,LOCK,UNLOCK";
        aDavOpt.setAllowedMethods( aAllowedMethods );
        // now check...
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isClass1() );
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isClass2() );
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isClass3() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.isHeadAllowed() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.isLockAllowed() );
        CPPUNIT_ASSERT_EQUAL( aAllowedMethods, aDavOpt.getAllowedMethods() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt32( 0 ), aDavOpt.getStaleTime() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getURL().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getRedirectedURL().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt16( 0 ), aDavOpt.getHttpResponseStatusCode() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getHttpResponseStatusText().isEmpty() );

        aAllowedMethods.clear();
        aDavOpt.setAllowedMethods( aAllowedMethods );
        aDavOpt.setStaleTime( 12345678 );
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isClass1() );
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isClass2() );
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isClass3() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.isHeadAllowed() );
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isLockAllowed() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getAllowedMethods().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt32( 12345678 ), aDavOpt.getStaleTime() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getURL().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getRedirectedURL().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt16( 0 ), aDavOpt.getHttpResponseStatusCode() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getHttpResponseStatusText().isEmpty() );

        aDavOpt.setStaleTime( 0 );

        OUString aHTTPResponseStatusText = "522 Origin Connection Time-out";
        aDavOpt.setHttpResponseStatusCode( 522 );
        aDavOpt.setHttpResponseStatusText( aHTTPResponseStatusText );
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isClass1() );
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isClass2() );
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isClass3() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getAllowedMethods().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isLockAllowed() );
        CPPUNIT_ASSERT_EQUAL( true , aDavOpt.getURL().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getRedirectedURL().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt32( 0 ), aDavOpt.getStaleTime() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt16( 522 ), aDavOpt.getHttpResponseStatusCode() );
        CPPUNIT_ASSERT_EQUAL( aHTTPResponseStatusText, aDavOpt.getHttpResponseStatusText() );

        aDavOpt.setHttpResponseStatusCode( 0 );
        aHTTPResponseStatusText.clear();
        aDavOpt.setHttpResponseStatusText( aHTTPResponseStatusText );

        OUString aURL = "http://a%20fake%20url/to%20test";
        aDavOpt.setURL( aURL );
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isClass1() );
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isClass2() );
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isClass3() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.isHeadAllowed() );
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isLockAllowed() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getAllowedMethods().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt32( 0 ), aDavOpt.getStaleTime() );
        CPPUNIT_ASSERT_EQUAL( aURL, aDavOpt.getURL() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getRedirectedURL().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt16( 0 ), aDavOpt.getHttpResponseStatusCode() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getHttpResponseStatusText().isEmpty() );

        aURL.clear();
        aDavOpt.setURL( aURL );
        aURL = "http://a%20fake%20url/to%20test/another-url";
        aDavOpt.setRedirectedURL( aURL );
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isClass1() );
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isClass2() );
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isClass3() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.isHeadAllowed() );
        CPPUNIT_ASSERT_EQUAL( false, aDavOpt.isLockAllowed() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getAllowedMethods().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt32( 0 ), aDavOpt.getStaleTime() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getURL().isEmpty() );
        CPPUNIT_ASSERT_EQUAL( aURL, aDavOpt.getRedirectedURL() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt16( 0 ), aDavOpt.getHttpResponseStatusCode() );
        CPPUNIT_ASSERT_EQUAL( true, aDavOpt.getHttpResponseStatusText().isEmpty() );

        //check the init() function
        aAllowedMethods = "OPTIONS,GET,HEAD,POST,DELETE,TRACE,PROPFIND,PROPPATCH,COPY,MOVE,PUT,LOCK,UNLOCK";
        aURL = "http://a%20fake%20url/to%20test/another-url";
        aHTTPResponseStatusText = "404 Not Found";
        aDavOpt.setClass1();
        aDavOpt.setClass2();
        aDavOpt.setClass3();
        aDavOpt.setHeadAllowed( false );
        aDavOpt.setLocked();
        aDavOpt.setAllowedMethods( aAllowedMethods );
        aDavOpt.setStaleTime( 1234567 );
        aDavOpt.setURL( aURL );
        aDavOpt.setRedirectedURL( aURL );
        aDavOpt.setHttpResponseStatusCode( 404 );
        aDavOpt.setHttpResponseStatusText( aHTTPResponseStatusText );

        aDavOpt.init();
        DAVTypesCheckInit( aDavOpt );
        // equality check
        webdav_ucp::DAVOptions aDavOptTarget;
        CPPUNIT_ASSERT_EQUAL( true , aDavOpt == aDavOptTarget );

        aDavOpt.setClass1();
        CPPUNIT_ASSERT_EQUAL( false , aDavOpt == aDavOptTarget );
        aDavOpt.setClass1( false );
        CPPUNIT_ASSERT_EQUAL( true , aDavOpt == aDavOptTarget );

        aDavOpt.setClass2();
        CPPUNIT_ASSERT_EQUAL( false , aDavOpt == aDavOptTarget );
        aDavOpt.setClass2( false );
        CPPUNIT_ASSERT_EQUAL( true , aDavOpt == aDavOptTarget );

        aDavOpt.setClass3();
        CPPUNIT_ASSERT_EQUAL( false , aDavOpt == aDavOptTarget );
        aDavOpt.setClass3( false );
        CPPUNIT_ASSERT_EQUAL( true , aDavOpt == aDavOptTarget );

        aDavOpt.setHeadAllowed( false );
        CPPUNIT_ASSERT_EQUAL( false , aDavOpt == aDavOptTarget );
        aDavOpt.setHeadAllowed();
        CPPUNIT_ASSERT_EQUAL( true , aDavOpt == aDavOptTarget );

        aDavOpt.setLocked();
        CPPUNIT_ASSERT_EQUAL( false , aDavOpt == aDavOptTarget );
        aDavOpt.setLocked( false );
        CPPUNIT_ASSERT_EQUAL( true , aDavOpt == aDavOptTarget );

        aDavOpt.setStaleTime( 1234567 );
        CPPUNIT_ASSERT_EQUAL( false , aDavOpt == aDavOptTarget );
        aDavOpt.setStaleTime( 0 );
        CPPUNIT_ASSERT_EQUAL( true , aDavOpt == aDavOptTarget );

        aDavOpt.setHttpResponseStatusCode( 404 );
        CPPUNIT_ASSERT_EQUAL( false , aDavOpt == aDavOptTarget );
        aDavOpt.setHttpResponseStatusCode( 0 );
        CPPUNIT_ASSERT_EQUAL( true , aDavOpt == aDavOptTarget );

        aDavOpt.setAllowedMethods( aAllowedMethods );
        CPPUNIT_ASSERT_EQUAL( false , aDavOpt == aDavOptTarget );
        aAllowedMethods.clear();
        aDavOpt.setAllowedMethods( aAllowedMethods );
        CPPUNIT_ASSERT_EQUAL( true , aDavOpt == aDavOptTarget );

        aDavOpt.setURL( aURL );
        CPPUNIT_ASSERT_EQUAL( false , aDavOpt == aDavOptTarget );
        aURL.clear();
        aDavOpt.setURL( aURL );
        CPPUNIT_ASSERT_EQUAL( true , aDavOpt == aDavOptTarget );

        aDavOpt.setHttpResponseStatusText( aHTTPResponseStatusText );
        CPPUNIT_ASSERT_EQUAL( false , aDavOpt == aDavOptTarget );
        aHTTPResponseStatusText.clear();
        aDavOpt.setHttpResponseStatusText( aHTTPResponseStatusText );
        CPPUNIT_ASSERT_EQUAL( true , aDavOpt == aDavOptTarget );

    }

    void webdav_opts_test::DAVOptsCacheTests()
    {
        // define a local cache to test
        webdav_ucp::DAVOptionsCache aDAVOptsCache;
        // the value to cache
        webdav_ucp::DAVOptions aDavOpt;
        // the returned value to test
        webdav_ucp::DAVOptions aDavOptCached;
        // init the values
        OUString aAllowedMethods = "OPTIONS,GET,HEAD,POST,DELETE,TRACE,PROPFIND,PROPPATCH,COPY,MOVE,PUT,LOCK,UNLOCK";
        OUString aURL = "http://a%20fake%20url/to%20test/another-url";
        OUString aRedirectedURL = "http://a%20fake%20url/to%20test/another-url/redirected";
        aDavOpt.setURL( aURL );
        aDavOpt.setRedirectedURL( aRedirectedURL );
        aDavOpt.setClass1();
        aDavOpt.setClass2();
        aDavOpt.setClass3();
        aDavOpt.setHeadAllowed( false );
        aDavOpt.setAllowedMethods( aAllowedMethods );
        // add to cache
        aDAVOptsCache.addDAVOptions( aDavOpt, 30000 );
        CPPUNIT_ASSERT_EQUAL( true ,aDAVOptsCache.getDAVOptions( aURL, aDavOptCached ) );
        CPPUNIT_ASSERT_EQUAL( true , aDavOpt == aDavOptCached );
    }

    CPPUNIT_TEST_SUITE_REGISTRATION( webdav_opts_test );
}                               // namespace rtl_random

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
