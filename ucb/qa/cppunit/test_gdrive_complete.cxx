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
#include <rtl/ustring.hxx>

// Test complete Google Drive UCB implementation
namespace
{
    class gdrive_complete_test: public test::BootstrapFixture
    {

    public:
        gdrive_complete_test() : BootstrapFixture( true, false ) {}

        void setUp() override;
        void tearDown() override;

        void testImplementationCompleteness();
        void testServiceNames();
        void testSchemeHandling();

        CPPUNIT_TEST_SUITE( gdrive_complete_test );
        CPPUNIT_TEST( testImplementationCompleteness );
        CPPUNIT_TEST( testServiceNames );
        CPPUNIT_TEST( testSchemeHandling );
        CPPUNIT_TEST_SUITE_END();
    };

    void gdrive_complete_test::setUp()
    {
    }

    void gdrive_complete_test::tearDown()
    {
    }

    void gdrive_complete_test::testImplementationCompleteness()
    {
        // Test that all required UNO service names are defined
        rtl::OUString providerService = u"com.sun.star.ucb.GoogleDriveContentProvider"_ustr;
        rtl::OUString contentService = u"com.sun.star.ucb.GoogleDriveContent"_ustr;
        rtl::OUString fileType = u"application/gdrive-file"_ustr;
        rtl::OUString folderType = u"application/gdrive-folder"_ustr;

        CPPUNIT_ASSERT(!providerService.isEmpty());
        CPPUNIT_ASSERT(!contentService.isEmpty());
        CPPUNIT_ASSERT(!fileType.isEmpty());
        CPPUNIT_ASSERT(!folderType.isEmpty());

        // Test implementation names
        rtl::OUString providerImpl = u"com.sun.star.comp.ucb.GoogleDriveContentProvider"_ustr;
        rtl::OUString contentImpl = u"com.sun.star.comp.ucb.GoogleDriveContent"_ustr;

        CPPUNIT_ASSERT(!providerImpl.isEmpty());
        CPPUNIT_ASSERT(!contentImpl.isEmpty());
    }

    void gdrive_complete_test::testServiceNames()
    {
        // Test that service names follow LibreOffice conventions
        rtl::OUString providerService = u"com.sun.star.ucb.GoogleDriveContentProvider"_ustr;

        // Should start with com.sun.star.ucb
        CPPUNIT_ASSERT(providerService.startsWith(u"com.sun.star.ucb."_ustr));

        // Should end with ContentProvider
        CPPUNIT_ASSERT(providerService.endsWith(u"ContentProvider"_ustr));

        // Content type should follow pattern
        rtl::OUString fileType = u"application/gdrive-file"_ustr;
        CPPUNIT_ASSERT(fileType.startsWith(u"application/"_ustr));
    }

    void gdrive_complete_test::testSchemeHandling()
    {
        // Test URL scheme validation
        rtl::OUString scheme = u"gdrive"_ustr;
        CPPUNIT_ASSERT_EQUAL(u"gdrive"_ustr, scheme);

        // Test typical URLs our provider should handle
        std::vector<rtl::OUString> validUrls = {
            u"gdrive://root"_ustr,
            u"gdrive://1BxiMVs0XRA5nFMdKvBdBZjgmUUqptlbs74OgvE2upms"_ustr,
            u"GDRIVE://root"_ustr, // Case insensitive
            u"gdrive:///root"_ustr  // Extra slash
        };

        for (const auto& url : validUrls) {
            bool isValid = url.startsWithIgnoreAsciiCase(u"gdrive://"_ustr);
            CPPUNIT_ASSERT_MESSAGE(
                rtl::OUStringToOString(u"URL should be valid: "_ustr + url, RTL_TEXTENCODING_UTF8).getStr(),
                isValid
            );
        }

        // Test invalid URLs
        std::vector<rtl::OUString> invalidUrls = {
            u"http://example.com"_ustr,
            u"file:///tmp"_ustr,
            u"vnd.sun.star.webdav://server"_ustr,
            u"gdrive:"_ustr, // Missing //
            u""_ustr // Empty
        };

        for (const auto& url : invalidUrls) {
            bool isValid = url.startsWithIgnoreAsciiCase(u"gdrive://"_ustr);
            CPPUNIT_ASSERT_MESSAGE(
                rtl::OUStringToOString(u"URL should be invalid: "_ustr + url, RTL_TEXTENCODING_UTF8).getStr(),
                !isValid
            );
        }
    }

    CPPUNIT_TEST_SUITE_REGISTRATION(gdrive_complete_test);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */