/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Unit test for URI parsing and manipulation functionality.
 */

#include <config.h>

#include <common/Uri.hpp>
#include <common/StringVector.hpp>

#include <test/lokassert.hpp>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include <Poco/URI.h>

/// Uri unit-tests.
class UriTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(UriTests);
    CPPUNIT_TEST(testEncode);
    CPPUNIT_TEST(testProtocolMessageRoundTripWithSpaces);
    CPPUNIT_TEST(testDecodeBeforePathStrip);
    CPPUNIT_TEST_SUITE_END();

    void testEncode();
    void testProtocolMessageRoundTripWithSpaces();
    void testDecodeBeforePathStrip();
};

void UriTests::testEncode()
{
    constexpr std::string_view testname = __func__;

    LOK_ASSERT_EQUAL(false, Uri::needsEncoding("www.example.com"));
    LOK_ASSERT_EQUAL(true, Uri::needsEncoding("www.example.com/file"));

    LOK_ASSERT_EQUAL_STR("www.example.com%2Ffile", Uri("www.example.com/file").encoded());
    LOK_ASSERT_EQUAL_STR("www.example.com%2Ffile", Uri::encode("www.example.com/file"));

    LOK_ASSERT_EQUAL_STR("www.example.com/file", Uri("www.example.com%2Ffile").decoded());
    LOK_ASSERT_EQUAL_STR("www.example.com/file", Uri::decode("www.example.com%2Ffile"));
}

// Regression test for the PDF export flow: filenames with spaces (or other
// reserved characters) used to break the registerdownload: / downloadas:
// protocol messages because those are space-delimited but the filename was
// embedded with literal spaces. The producer side (kit/Kit.cpp's
// downloadAsFileSaveDialogCallback and kit/ChildSession.cpp's
// KIT_CALLBACK_EXPORT_FILE handler) must Poco::URI::encode the filename and
// jail-relative URL before placing them in the protocol message; the
// consumers (DocumentBroker / ClientRequestDispatcher) Uri::decode them.
//
// This test simulates that round-trip for a few problem filenames.
void UriTests::testProtocolMessageRoundTripWithSpaces()
{
    constexpr std::string_view testname = __func__;

    struct Case
    {
        std::string filename; // original, with literal spaces / other characters
        std::string tmpDir;
        std::string clientId;
    };

    const Case cases[] = {
        { "0. something.pdf",     "abcdef0123", "client-1" },
        { "First Second.pdf",     "deadbeef00", "client-2" },
        { "What's New 2026.pdf",  "feedface11", "client-3" }, // apostrophe + space
        { "no-spaces.pdf",        "cafebabe22", "client-4" }, // baseline
    };

    for (const Case& c : cases)
    {
        const std::string urlInJail = c.tmpDir + '/' + c.filename;

        std::string encodedUrlInJail, encodedFilename;
        Poco::URI::encode(urlInJail, "", encodedUrlInJail);
        Poco::URI::encode(c.filename, "", encodedFilename);

        // Encoded forms must contain no literal spaces — otherwise the
        // space-delimited protocol tokenizer splits them mid-value.
        LOK_ASSERT_EQUAL(std::string::npos, encodedUrlInJail.find(' '));
        LOK_ASSERT_EQUAL(std::string::npos, encodedFilename.find(' '));

        // registerdownload: downloadid=<id> url=<url> clientid=<id>
        // The wsd-side handler asserts tokens.size() == 4.
        const std::string registerMsg = "registerdownload: downloadid=" + c.tmpDir
                                        + " url=" + encodedUrlInJail
                                        + " clientid=" + c.clientId;
        StringVector regTokens = StringVector::tokenize(registerMsg);
        LOK_ASSERT_EQUAL(static_cast<size_t>(4), regTokens.size());

        // downloadas: downloadid=<id> port=<port> id=export filename=<name>
        // The browser-side parser splits on /[ \n]+/ and matches each token
        // by prefix; the filename token must survive intact.
        const std::string downloadMsg = "downloadas: downloadid=" + c.tmpDir
                                        + " port=9980 id=export filename=" + encodedFilename;
        StringVector dlTokens = StringVector::tokenize(downloadMsg);
        LOK_ASSERT_EQUAL(static_cast<size_t>(5), dlTokens.size());

        // The receiver decodes and must recover the original.
        LOK_ASSERT_EQUAL_STR(urlInJail, Uri::decode(encodedUrlInJail));
        LOK_ASSERT_EQUAL_STR(c.filename, Uri::decode(encodedFilename));
    }
}

// Poco::Path::getFileName() does not URL-decode its input, so it can only
// strip path components separated by a literal '/'. Validators that take a
// URL-encoded user-supplied name must decode before checking for path
// components.
void UriTests::testDecodeBeforePathStrip()
{
    constexpr std::string_view testname = __func__;

    // Encoded path separator is preserved by Poco::Path::getFileName.
    LOK_ASSERT_EQUAL_STR("a%2Fb.odt", Poco::Path("a%2Fb.odt").getFileName());

    // After decoding, the separator is visible.
    LOK_ASSERT(Uri::decode("a%2Fb.odt").find('/') != std::string::npos);

    // Legitimate names with encoded spaces / apostrophes have no separator
    // after decoding and must not be flagged.
    LOK_ASSERT(Uri::decode("Hello%20World.odt").find('/') == std::string::npos);
    LOK_ASSERT(Uri::decode("What%27s%20New.pdf").find('/') == std::string::npos);
}

CPPUNIT_TEST_SUITE_REGISTRATION(UriTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
