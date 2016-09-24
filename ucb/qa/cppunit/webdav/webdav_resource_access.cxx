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

        void DAVCheckRetries();

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE( webdav_resource_access_test );
        CPPUNIT_TEST( DAVCheckRetries );
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
    void webdav_resource_access_test::DAVCheckRetries()
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
        // except: SC_NOT_IMPLEMENTED
        for (auto i = SC_INTERNAL_SERVER_ERROR; i < 2000; i++)
        {
            const DAVException aTheException(DAVException::DAV_HTTP_ERROR, "http error code", i );
            if( i != SC_NOT_IMPLEMENTED )
                CPPUNIT_ASSERT_EQUAL( true , ResourceAccess.handleException( aTheException, 1 ) );
            else
                CPPUNIT_ASSERT_EQUAL( false , ResourceAccess.handleException( aTheException, 1 ) );
        }
    }

    CPPUNIT_TEST_SUITE_REGISTRATION( webdav_resource_access_test );
}                               // namespace rtl_random

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
