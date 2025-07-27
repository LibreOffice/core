/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <rtl/ustring.hxx>
#include <com/sun/star/util/DateTime.hpp>

// Include our GDrive JSON helper
#include "../../source/ucp/gdrive/gdrive_json.hxx"

using namespace ucp::gdrive;

class GDriveJsonTest : public CppUnit::TestFixture
{
public:
    void testParseFolderListingEmpty();
    void testParseFolderListingSingle();
    void testParseFolderListingMultiple();
    void testParseFolderListingWithPagination();
    void testParseDateTime();
    void testCreateFolderMetadata();
    void testCreateFileMetadata();
    void testCreateCopyMetadata();
    void testParseTokenResponse();

    CPPUNIT_TEST_SUITE(GDriveJsonTest);
    CPPUNIT_TEST(testParseFolderListingEmpty);
    CPPUNIT_TEST(testParseFolderListingSingle);
    CPPUNIT_TEST(testParseFolderListingMultiple);
    CPPUNIT_TEST(testParseFolderListingWithPagination);
    CPPUNIT_TEST(testParseDateTime);
    CPPUNIT_TEST(testCreateFolderMetadata);
    CPPUNIT_TEST(testCreateFileMetadata);
    CPPUNIT_TEST(testCreateCopyMetadata);
    CPPUNIT_TEST(testParseTokenResponse);
    CPPUNIT_TEST_SUITE_END();
};

void GDriveJsonTest::testParseFolderListingEmpty()
{
    // Test empty JSON
    rtl::OUString emptyJson = u"{\"files\":[]}"_ustr;
    auto files = GDriveJsonHelper::parseFolderListing(emptyJson);
    CPPUNIT_ASSERT_EQUAL(size_t(0), files.size());

    // Test completely empty string
    auto filesEmpty = GDriveJsonHelper::parseFolderListing(rtl::OUString());
    CPPUNIT_ASSERT_EQUAL(size_t(0), filesEmpty.size());
}

void GDriveJsonTest::testParseFolderListingSingle()
{
    rtl::OUString singleFileJson = u"{"
        "\"files\": ["
        "  {"
        "    \"id\": \"1BxiMVs0XRA5nFMdKvBdBZjgmUUqptlbs74OgvE2upms\","
        "    \"name\": \"Test Document\","
        "    \"mimeType\": \"application/vnd.google-apps.document\","
        "    \"size\": \"1024\","
        "    \"modifiedTime\": \"2024-01-15T10:30:00.000Z\""
        "  }"
        "]"
        "}"_ustr;

    auto files = GDriveJsonHelper::parseFolderListing(singleFileJson);
    CPPUNIT_ASSERT_EQUAL(size_t(1), files.size());

    const auto& file = files[0];
    CPPUNIT_ASSERT_EQUAL(u"1BxiMVs0XRA5nFMdKvBdBZjgmUUqptlbs74OgvE2upms"_ustr, file.id);
    CPPUNIT_ASSERT_EQUAL(u"Test Document"_ustr, file.name);
    CPPUNIT_ASSERT_EQUAL(u"application/vnd.google-apps.document"_ustr, file.mimeType);
    CPPUNIT_ASSERT_EQUAL(u"1024"_ustr, file.size);
    CPPUNIT_ASSERT_EQUAL(u"2024-01-15T10:30:00.000Z"_ustr, file.modifiedTime);
    CPPUNIT_ASSERT_EQUAL(false, file.isFolder);
}

void GDriveJsonTest::testParseFolderListingMultiple()
{
    rtl::OUString multipleFilesJson = u"{"
        "\"files\": ["
        "  {"
        "    \"id\": \"folder123\","
        "    \"name\": \"My Folder\","
        "    \"mimeType\": \"application/vnd.google-apps.folder\","
        "    \"modifiedTime\": \"2024-01-10T08:00:00.000Z\""
        "  },"
        "  {"
        "    \"id\": \"doc456\","
        "    \"name\": \"Document.docx\","
        "    \"mimeType\": \"application/vnd.openxmlformats-officedocument.wordprocessingml.document\","
        "    \"size\": \"2048\","
        "    \"modifiedTime\": \"2024-01-12T14:20:00.000Z\""
        "  }"
        "]"
        "}"_ustr;

    auto files = GDriveJsonHelper::parseFolderListing(multipleFilesJson);
    CPPUNIT_ASSERT_EQUAL(size_t(2), files.size());

    // Check folder
    const auto& folder = files[0];
    CPPUNIT_ASSERT_EQUAL(u"folder123"_ustr, folder.id);
    CPPUNIT_ASSERT_EQUAL(u"My Folder"_ustr, folder.name);
    CPPUNIT_ASSERT_EQUAL(true, folder.isFolder);

    // Check document
    const auto& doc = files[1];
    CPPUNIT_ASSERT_EQUAL(u"doc456"_ustr, doc.id);
    CPPUNIT_ASSERT_EQUAL(u"Document.docx"_ustr, doc.name);
    CPPUNIT_ASSERT_EQUAL(false, doc.isFolder);
    CPPUNIT_ASSERT_EQUAL(u"2048"_ustr, doc.size);
}

void GDriveJsonTest::testParseFolderListingWithPagination()
{
    rtl::OUString paginatedJson = u"{"
        "\"files\": ["
        "  {"
        "    \"id\": \"file1\","
        "    \"name\": \"File 1\","
        "    \"mimeType\": \"text/plain\","
        "    \"size\": \"100\","
        "    \"modifiedTime\": \"2024-01-01T00:00:00.000Z\""
        "  }"
        "],"
        "\"nextPageToken\": \"ABCD1234xyz\""
        "}"_ustr;

    auto listing = GDriveJsonHelper::parseFolderListingWithPagination(paginatedJson);

    CPPUNIT_ASSERT_EQUAL(size_t(1), listing.files.size());
    CPPUNIT_ASSERT_EQUAL(u"ABCD1234xyz"_ustr, listing.nextPageToken);
    CPPUNIT_ASSERT_EQUAL(true, listing.hasMore);

    const auto& file = listing.files[0];
    CPPUNIT_ASSERT_EQUAL(u"file1"_ustr, file.id);
    CPPUNIT_ASSERT_EQUAL(u"File 1"_ustr, file.name);
}

void GDriveJsonTest::testParseDateTime()
{
    // Test ISO 8601 parsing
    rtl::OUString dateStr = u"2024-01-15T10:30:45.123Z"_ustr;
    css::util::DateTime dateTime = GDriveJsonHelper::parseDateTime(dateStr);

    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2024), dateTime.Year);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), dateTime.Month);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(15), dateTime.Day);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(10), dateTime.Hours);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(30), dateTime.Minutes);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(45), dateTime.Seconds);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(123000000), dateTime.NanoSeconds);

    // Test empty string
    css::util::DateTime emptyDateTime = GDriveJsonHelper::parseDateTime(rtl::OUString());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), emptyDateTime.Year);
}

void GDriveJsonTest::testCreateFolderMetadata()
{
    rtl::OUString folderJson = GDriveJsonHelper::createFolderMetadata(u"Test Folder"_ustr, u"parent123"_ustr);

    // Should contain folder MIME type and name
    CPPUNIT_ASSERT(folderJson.indexOf(u"Test Folder"_ustr) != -1);
    CPPUNIT_ASSERT(folderJson.indexOf(u"application/vnd.google-apps.folder"_ustr) != -1);
    CPPUNIT_ASSERT(folderJson.indexOf(u"parent123"_ustr) != -1);

    // Test with root parent
    rtl::OUString rootFolderJson = GDriveJsonHelper::createFolderMetadata(u"Root Folder"_ustr, u"root"_ustr);
    CPPUNIT_ASSERT(rootFolderJson.indexOf(u"Root Folder"_ustr) != -1);
    CPPUNIT_ASSERT(rootFolderJson.indexOf(u"application/vnd.google-apps.folder"_ustr) != -1);
}

void GDriveJsonTest::testCreateFileMetadata()
{
    rtl::OUString fileJson = GDriveJsonHelper::createFileMetadata(u"Test File.txt"_ustr, u"parent456"_ustr);

    CPPUNIT_ASSERT(fileJson.indexOf(u"Test File.txt"_ustr) != -1);
    CPPUNIT_ASSERT(fileJson.indexOf(u"parent456"_ustr) != -1);
    // Should NOT contain folder MIME type
    CPPUNIT_ASSERT(fileJson.indexOf(u"application/vnd.google-apps.folder"_ustr) == -1);
}

void GDriveJsonTest::testCreateCopyMetadata()
{
    rtl::OUString copyJson = GDriveJsonHelper::createCopyMetadata(u"Copy of File"_ustr, u"newparent"_ustr);

    CPPUNIT_ASSERT(copyJson.indexOf(u"Copy of File"_ustr) != -1);
    CPPUNIT_ASSERT(copyJson.indexOf(u"newparent"_ustr) != -1);

    // Test with empty name (should work)
    rtl::OUString copyJsonNoName = GDriveJsonHelper::createCopyMetadata(rtl::OUString(), u"parent"_ustr);
    CPPUNIT_ASSERT(copyJsonNoName.indexOf(u"parent"_ustr) != -1);
}

void GDriveJsonTest::testParseTokenResponse()
{
    rtl::OUString tokenJson = u"{"
        "\"access_token\": \"ya29.abc123xyz\","
        "\"refresh_token\": \"1//refresh_token_here\","
        "\"expires_in\": 3600,"
        "\"token_type\": \"Bearer\""
        "}"_ustr;

    auto tokenPair = GDriveJsonHelper::parseTokenResponse(tokenJson);

    CPPUNIT_ASSERT_EQUAL(u"ya29.abc123xyz"_ustr, tokenPair.first);
    CPPUNIT_ASSERT_EQUAL(u"1//refresh_token_here"_ustr, tokenPair.second);

    // Test malformed JSON
    auto emptyPair = GDriveJsonHelper::parseTokenResponse(u"invalid json"_ustr);
    CPPUNIT_ASSERT(emptyPair.first.isEmpty());
    CPPUNIT_ASSERT(emptyPair.second.isEmpty());
}

CPPUNIT_TEST_SUITE_REGISTRATION(GDriveJsonTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */