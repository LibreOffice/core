/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <tools/hostfilter.hxx>

namespace
{
class TestHostFilter : public CppUnit::TestFixture
{
public:
    void testEmptyAllowlist();
    void testNonFileUrl();
    void testAllowedPath();
    void testSiblingDirectoryNotAllowed();
    void testParentDirectoryNotAllowed();
    void testMultiplePaths();
    void testEncodedFileUrl();
    void testParentDirectorySegments();

    CPPUNIT_TEST_SUITE(TestHostFilter);
    CPPUNIT_TEST(testEmptyAllowlist);
    CPPUNIT_TEST(testNonFileUrl);
    CPPUNIT_TEST(testAllowedPath);
    CPPUNIT_TEST(testSiblingDirectoryNotAllowed);
    CPPUNIT_TEST(testParentDirectoryNotAllowed);
    CPPUNIT_TEST(testMultiplePaths);
    CPPUNIT_TEST(testEncodedFileUrl);
    CPPUNIT_TEST(testParentDirectorySegments);
    CPPUNIT_TEST_SUITE_END();
};

void TestHostFilter::testEmptyAllowlist()
{
    // empty string means "block all file URLs"
    HostFilter::setAllowedExtRefPaths("");
    CPPUNIT_ASSERT(HostFilter::isFileUrlForbidden(u"file:///home/user/doc.ods"_ustr));
    CPPUNIT_ASSERT(HostFilter::isFileUrlForbidden(u"file:///tmp/doc.ods"_ustr));
    HostFilter::resetAllowedExtRefPaths();
}

void TestHostFilter::testNonFileUrl()
{
    HostFilter::setAllowedExtRefPaths("");
    // non-file URLs are not subject to the extref path allowlist
    CPPUNIT_ASSERT(!HostFilter::isFileUrlForbidden(u"http://example.com/doc.ods"_ustr));
    CPPUNIT_ASSERT(!HostFilter::isFileUrlForbidden(u"https://example.com/doc.ods"_ustr));
    CPPUNIT_ASSERT(!HostFilter::isFileUrlForbidden(u"ftp://example.com/doc.ods"_ustr));
    HostFilter::resetAllowedExtRefPaths();
}

void TestHostFilter::testAllowedPath()
{
    HostFilter::setAllowedExtRefPaths("/tmp/docs");
    // file inside allowed directory is permitted
    CPPUNIT_ASSERT(!HostFilter::isFileUrlForbidden(u"file:///tmp/docs/sheet.ods"_ustr));
    // file in a subdirectory is permitted
    CPPUNIT_ASSERT(!HostFilter::isFileUrlForbidden(u"file:///tmp/docs/sub/sheet.ods"_ustr));
    // file outside allowed directory is blocked
    CPPUNIT_ASSERT(HostFilter::isFileUrlForbidden(u"file:///home/user/doc.ods"_ustr));
    HostFilter::resetAllowedExtRefPaths();
}

void TestHostFilter::testSiblingDirectoryNotAllowed()
{
    // /tmp/user must not match /tmp/username
    HostFilter::setAllowedExtRefPaths("/tmp/user");
    CPPUNIT_ASSERT(!HostFilter::isFileUrlForbidden(u"file:///tmp/user/doc.ods"_ustr));
    CPPUNIT_ASSERT(HostFilter::isFileUrlForbidden(u"file:///tmp/username/doc.ods"_ustr));
    CPPUNIT_ASSERT(HostFilter::isFileUrlForbidden(u"file:///tmp/usera/doc.ods"_ustr));
    HostFilter::resetAllowedExtRefPaths();
}

void TestHostFilter::testParentDirectoryNotAllowed()
{
    HostFilter::setAllowedExtRefPaths("/tmp/docs");
    // parent directory is not allowed
    CPPUNIT_ASSERT(HostFilter::isFileUrlForbidden(u"file:///tmp/secret.ods"_ustr));
    // unrelated path is not allowed
    CPPUNIT_ASSERT(HostFilter::isFileUrlForbidden(u"file:///var/data/doc.ods"_ustr));
    HostFilter::resetAllowedExtRefPaths();
}

void TestHostFilter::testMultiplePaths()
{
    // colon-separated list of allowed paths
    HostFilter::setAllowedExtRefPaths("/tmp/a:/tmp/b");
    CPPUNIT_ASSERT(!HostFilter::isFileUrlForbidden(u"file:///tmp/a/doc.ods"_ustr));
    CPPUNIT_ASSERT(!HostFilter::isFileUrlForbidden(u"file:///tmp/b/doc.ods"_ustr));
    CPPUNIT_ASSERT(HostFilter::isFileUrlForbidden(u"file:///tmp/c/doc.ods"_ustr));
    HostFilter::resetAllowedExtRefPaths();
}

void TestHostFilter::testEncodedFileUrl()
{
    HostFilter::setAllowedExtRefPaths("/tmp/my docs");
    // percent-encoded space in URL should match allowed path with space
    CPPUNIT_ASSERT(!HostFilter::isFileUrlForbidden(u"file:///tmp/my%20docs/sheet.ods"_ustr));
    CPPUNIT_ASSERT(!HostFilter::isFileUrlForbidden(u"file:///tmp/my%20docs/sub/sheet.ods"_ustr));
    // encoded URL outside allowed path is still blocked
    CPPUNIT_ASSERT(HostFilter::isFileUrlForbidden(u"file:///tmp/other%20docs/sheet.ods"_ustr));
    HostFilter::resetAllowedExtRefPaths();
}

void TestHostFilter::testParentDirectorySegments()
{
    HostFilter::setAllowedExtRefPaths("/tmp/docs");
    // .. segments that escape the allowed directory should be blocked
    CPPUNIT_ASSERT(HostFilter::isFileUrlForbidden(u"file:///tmp/docs/../other/sheet.ods"_ustr));
    // .. segments that stay within the allowed directory are permitted
    CPPUNIT_ASSERT(!HostFilter::isFileUrlForbidden(u"file:///tmp/docs/sub/../sheet.ods"_ustr));
    HostFilter::resetAllowedExtRefPaths();
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestHostFilter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
