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
#include "DavURIObject.hxx"

using namespace webdav_ucp;

namespace
{

    class webdav_resource_access_test: public test::BootstrapFixture
    {

    public:
        webdav_resource_access_test() : BootstrapFixture( true, true ) {}

        // initialise your test code values here.
        void setUp() override;

        void tearDown() override;

        void DAVTestRetries();
        void DAVTestURLObjectHelper( OUString& theURL,
                                     OUString& thePercEncodedURL,
                                     OUString& thePercEncodedTitle,
                                     OUString& thePercDecodedTitle,
                                     OUString& thePercEncodedPath );
        bool DAVTestURLObjectVerify( OUString& theURL );
        void DAVTestURLObject();

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE( webdav_resource_access_test );
        CPPUNIT_TEST( DAVTestRetries );
        CPPUNIT_TEST( DAVTestURLObject );
        CPPUNIT_TEST_SUITE_END();
    };                          // class webdav_local_test

    // initialise your test code values here.
    void webdav_resource_access_test::setUp()
    {
    }

    void webdav_resource_access_test::tearDown()
    {
    }

    // test when http connection should retry
    void webdav_resource_access_test::DAVTestRetries()
    {
        // instantiate a resource access class
        DAVResourceAccess ResourceAccess(nullptr, nullptr, "http://url");
        // first check: all http errors from 100 to 399 should return true, to force a retry
        for (auto i = SC_CONTINUE; i < SC_BAD_REQUEST; i++)
        {
            const DAVException aTheException(DAVException::DAV_HTTP_ERROR, "http error code", i );
            CPPUNIT_ASSERT_EQUAL( true , ResourceAccess.handleException( aTheException, 1 ) );
        }
        // http error code from 400 to 499 should NOT force a retry
        for (auto i = SC_BAD_REQUEST; i < SC_INTERNAL_SERVER_ERROR; i++)
        {
            const DAVException aTheException(DAVException::DAV_HTTP_ERROR, "http error code", i );
            CPPUNIT_ASSERT_EQUAL( false , ResourceAccess.handleException( aTheException, 1 ) );
        }

        // http error code from 500 (SC_INTERNAL_SERVER_ERROR) up should force a retry
        // except in special value
        // 1999 as high limit is just a current (2016-09-25) choice.
        // RFC poses no limit to the max value of response status code
        for (auto i = SC_INTERNAL_SERVER_ERROR; i < 2000; i++)
        {
            const DAVException aTheException(DAVException::DAV_HTTP_ERROR, "http error code", i );
            switch ( i )
            {
                // the HTTP response status codes that can be retried
                case SC_BAD_GATEWAY:
                case SC_GATEWAY_TIMEOUT:
                case SC_SERVICE_UNAVAILABLE:
                case SC_INSUFFICIENT_STORAGE:
                    CPPUNIT_ASSERT_EQUAL( true , ResourceAccess.handleException( aTheException, 1 ) );
                    break;
                    // default is NOT retry
                default:
                    CPPUNIT_ASSERT_EQUAL( false , ResourceAccess.handleException( aTheException, 1 ) );
            }
        }

        // check the retry request
        {
            const DAVException aTheException(DAVException::DAV_HTTP_RETRY, "the-host-name", 8080 );
            CPPUNIT_ASSERT_EQUAL( true , ResourceAccess.handleException( aTheException, 1 ) );
        }
    }

    void webdav_resource_access_test::DAVTestURLObjectHelper( OUString& theURL,
                                                              OUString& thePercEncodedURL,
                                                              OUString& thePercEncodedTitle,
                                                              OUString& thePercDecodedTitle,
                                                              OUString& thePercEncodedPath )
    {
        // test percent-encode from human readable to URL
        DavURIObject aDavURL( theURL );
        // test returned URL
        CPPUNIT_ASSERT_EQUAL( thePercEncodedURL, aDavURL.GetMainURL() );
        // check percent-encoded title
        CPPUNIT_ASSERT_EQUAL( thePercEncodedTitle, aDavURL.GetPathBaseName() );
        // check percent-decoded title
        CPPUNIT_ASSERT_EQUAL( thePercDecodedTitle, aDavURL.GetPercDecodedPathBaseName() );
        // get path to be used in Web method operations
        CPPUNIT_ASSERT_EQUAL( thePercEncodedPath, aDavURL.GetPathQueryFragment() );
    }

    bool webdav_resource_access_test::DAVTestURLObjectVerify( OUString& theURL )
    {
        // try the url test exception, return false if right exception found,
        // true otherwise, meaning URL is ok
        try
        {
            DavURIObject( theURL ).verifyURL();
            return true;
        }
        catch ( DAVException& )
        {
            return false;
        }
        catch( ... )
        {
            CPPUNIT_ASSERT_MESSAGE( "Unexpected exception returned", false );
        }
        return false;
    }

    // test DavURIObject behaviour
    // percent-encode percent-decode
    void webdav_resource_access_test::DAVTestURLObject()
    {
        // test percent-encode from human readable to URL (from a bug of another era, plus variations)
        OUString theURL =              "http://server.biz:8040/aService/a segment/next segment/check.this?test=true&link=http://anotherserver.com/%3Fcheck=theapplication%26os=linuxintel%26lang=en-US%26version=5.2.0";
        OUString thePercEncodedURL =   "http://server.biz:8040/aService/a%20segment/next%20segment/check.this?test=true&link=http://anotherserver.com/%3Fcheck=theapplication%26os=linuxintel%26lang=en-US%26version=5.2.0";
        OUString thePercEncodedTitle = "check.this?test=true&link=http://anotherserver.com/%3Fcheck=theapplication%26os=linuxintel%26lang=en-US%26version=5.2.0";
        OUString thePercDecodedTitle = "check.this?test=true&link=http://anotherserver.com/%3Fcheck=theapplication%26os=linuxintel%26lang=en-US%26version=5.2.0";
        OUString thePercEncodedPath  = "/aService/a%20segment/next%20segment/check.this?test=true&link=http://anotherserver.com/%3Fcheck=theapplication%26os=linuxintel%26lang=en-US%26version=5.2.0";
        DAVTestURLObjectHelper( theURL, thePercEncodedURL, thePercEncodedTitle, thePercDecodedTitle, thePercEncodedPath );
        CPPUNIT_ASSERT_EQUAL( true, DAVTestURLObjectVerify( theURL ) );

        // again from a bug of another era, plus variations
        theURL =              "http://update23.services.myoff.org/ProductUpdateService/check.Update?test=true&link=http://myoff.bouncer.os.org/%3Fproduct=MyOffice.org%26os=linuxintel%26lang=en-US%26version=2.2.1";
        thePercEncodedURL =   "http://update23.services.myoff.org/ProductUpdateService/check.Update?test=true&link=http://myoff.bouncer.os.org/%3Fproduct=MyOffice.org%26os=linuxintel%26lang=en-US%26version=2.2.1";
        thePercEncodedTitle = "check.Update?test=true&link=http://myoff.bouncer.os.org/%3Fproduct=MyOffice.org%26os=linuxintel%26lang=en-US%26version=2.2.1";
        thePercDecodedTitle = "check.Update?test=true&link=http://myoff.bouncer.os.org/%3Fproduct=MyOffice.org%26os=linuxintel%26lang=en-US%26version=2.2.1";
        thePercEncodedPath  = "/ProductUpdateService/check.Update?test=true&link=http://myoff.bouncer.os.org/%3Fproduct=MyOffice.org%26os=linuxintel%26lang=en-US%26version=2.2.1";
        DAVTestURLObjectHelper( theURL, thePercEncodedURL, thePercEncodedTitle, thePercDecodedTitle, thePercEncodedPath );
        CPPUNIT_ASSERT_EQUAL( true, DAVTestURLObjectVerify( theURL ) );

        // from old dbz#406926, plus variations...
        theURL =              OStringToOUString( "https://upload.some_server.org/myp`ath/wiki/Máquina de Turing", RTL_TEXTENCODING_UTF8 );
        thePercEncodedURL =   "https://upload.some_server.org/myp%60ath/wiki/M%C3%A1quina%20de%20Turing";
        thePercEncodedTitle = "M%C3%A1quina%20de%20Turing";
        thePercDecodedTitle = OStringToOUString( "Máquina de Turing", RTL_TEXTENCODING_UTF8 );
        thePercEncodedPath  = "/myp%60ath/wiki/M%C3%A1quina%20de%20Turing";
        DAVTestURLObjectHelper( theURL, thePercEncodedURL, thePercEncodedTitle, thePercDecodedTitle, thePercEncodedPath );
        CPPUNIT_ASSERT_EQUAL( true, DAVTestURLObjectVerify( theURL ) );

        // unicode string path tests
        theURL =              OStringToOUString( "https://upload.some_server.org/helper/commons/thumb/7/7d/Teoría de autómatas.svg/300px-Teoría de autómatas.svg.png", RTL_TEXTENCODING_UTF8 );
        thePercEncodedURL =   "https://upload.some_server.org/helper/commons/thumb/7/7d/Teor%C3%ADa%20de%20aut%C3%B3matas.svg/300px-Teor%C3%ADa%20de%20aut%C3%B3matas.svg.png";
        thePercEncodedTitle = "300px-Teor%C3%ADa%20de%20aut%C3%B3matas.svg.png";
        thePercDecodedTitle = OStringToOUString( "300px-Teoría de autómatas.svg.png", RTL_TEXTENCODING_UTF8 );
        thePercEncodedPath  = "/helper/commons/thumb/7/7d/Teor%C3%ADa%20de%20aut%C3%B3matas.svg/300px-Teor%C3%ADa%20de%20aut%C3%B3matas.svg.png";
        DAVTestURLObjectHelper( theURL, thePercEncodedURL, thePercEncodedTitle, thePercDecodedTitle, thePercEncodedPath );
        CPPUNIT_ASSERT_EQUAL( true, DAVTestURLObjectVerify( theURL ) );

        // test specific query ,from tdf#99499 (1)
        theURL =              "http://www.digikey.com/web export/common/mkt/en/help.png?requestedName=help?requestedName=help?requestedName=help?requestedName=help?requestedName=help";
        thePercEncodedURL =   "http://www.digikey.com/web%20export/common/mkt/en/help.png?requestedName=help?requestedName=help?requestedName=help?requestedName=help?requestedName=help";
        thePercEncodedTitle = "help.png?requestedName=help?requestedName=help?requestedName=help?requestedName=help?requestedName=help";
        thePercDecodedTitle = "help.png?requestedName=help?requestedName=help?requestedName=help?requestedName=help?requestedName=help";
        thePercEncodedPath  = "/web%20export/common/mkt/en/help.png?requestedName=help?requestedName=help?requestedName=help?requestedName=help?requestedName=help";
        DAVTestURLObjectHelper( theURL, thePercEncodedURL, thePercEncodedTitle, thePercDecodedTitle, thePercEncodedPath );
        CPPUNIT_ASSERT_EQUAL( true, DAVTestURLObjectVerify( theURL ) );

        // test specific query ,from tdf#99499 (2)
        theURL =              "https://sealserver.trustkeeper.net/seal_image.php?customerId=84EDAB68F81B2B31985E5E20392A8AC1&size=105x54&style=normal?requestedName=seal_image?requestedName=seal_image?requestedName=seal_image?requestedName=seal_image";
        thePercEncodedURL =   "https://sealserver.trustkeeper.net/seal_image.php?customerId=84EDAB68F81B2B31985E5E20392A8AC1&size=105x54&style=normal?requestedName=seal_image?requestedName=seal_image?requestedName=seal_image?requestedName=seal_image";
        thePercEncodedTitle = "seal_image.php?customerId=84EDAB68F81B2B31985E5E20392A8AC1&size=105x54&style=normal?requestedName=seal_image?requestedName=seal_image?requestedName=seal_image?requestedName=seal_image";
        thePercDecodedTitle = "seal_image.php?customerId=84EDAB68F81B2B31985E5E20392A8AC1&size=105x54&style=normal?requestedName=seal_image?requestedName=seal_image?requestedName=seal_image?requestedName=seal_image";
        thePercEncodedPath  = "/seal_image.php?customerId=84EDAB68F81B2B31985E5E20392A8AC1&size=105x54&style=normal?requestedName=seal_image?requestedName=seal_image?requestedName=seal_image?requestedName=seal_image";
        DAVTestURLObjectHelper( theURL, thePercEncodedURL, thePercEncodedTitle, thePercDecodedTitle, thePercEncodedPath );
        CPPUNIT_ASSERT_EQUAL( true, DAVTestURLObjectVerify( theURL ) );

        theURL =              "/my.server";
        CPPUNIT_ASSERT_EQUAL( false, DAVTestURLObjectVerify( theURL ) );// false URL can only be absolute, for now

        theURL =              "http://";
        CPPUNIT_ASSERT_EQUAL( false, DAVTestURLObjectVerify( theURL ) );

    }

    CPPUNIT_TEST_SUITE_REGISTRATION( webdav_resource_access_test );
}                               // namespace rtl_random

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
