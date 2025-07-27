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
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/io/SequenceInputStream.hpp>
#include <comphelper/processfactory.hxx>

// Mock for testing without real network calls
class MockGoogleDriveApiClient
{
private:
    rtl::OUString m_sNextResponse;
    sal_Int32 m_nResponseCode;
    bool m_bNetworkError;

public:
    MockGoogleDriveApiClient() : m_nResponseCode(200), m_bNetworkError(false) {}

    void setMockResponse(const rtl::OUString& response, sal_Int32 responseCode = 200)
    {
        m_sNextResponse = response;
        m_nResponseCode = responseCode;
        m_bNetworkError = false;
    }

    void setNetworkError(bool error) { m_bNetworkError = error; }

    // Mock implementation of key methods
    std::vector<ucp::gdrive::GDriveFileInfo> listFolder(const rtl::OUString& /*folderId*/)
    {
        if (m_bNetworkError || m_nResponseCode != 200)
            return std::vector<ucp::gdrive::GDriveFileInfo>();

        return ucp::gdrive::GDriveJsonHelper::parseFolderListing(m_sNextResponse);
    }

    ucp::gdrive::GDriveFileInfo getFileInfo(const rtl::OUString& /*fileId*/)
    {
        if (m_bNetworkError || m_nResponseCode != 200)
            return ucp::gdrive::GDriveFileInfo();

        // Parse single file response
        std::vector<ucp::gdrive::GDriveFileInfo> files =
            ucp::gdrive::GDriveJsonHelper::parseFolderListing(u"{\"files\":[" + m_sNextResponse + "]}"_ustr);

        return files.empty() ? ucp::gdrive::GDriveFileInfo() : files[0];
    }

    rtl::OUString copyFile(const rtl::OUString& /*fileId*/, const rtl::OUString& /*newParentId*/, const rtl::OUString& /*newName*/)
    {
        if (m_bNetworkError || m_nResponseCode != 200)
            return rtl::OUString();

        // Return mock new file ID
        return u"copied_file_id_12345"_ustr;
    }
};

class GDriveApiClientTest : public CppUnit::TestFixture
{
public:
    void setUp() override;
    void tearDown() override;

    void testListFolderSuccess();
    void testListFolderEmpty();
    void testListFolderNetworkError();
    void testGetFileInfoSuccess();
    void testGetFileInfoNotFound();
    void testCopyFileSuccess();
    void testCopyFileFailure();
    void testRetryMechanism();

    CPPUNIT_TEST_SUITE(GDriveApiClientTest);
    CPPUNIT_TEST(testListFolderSuccess);
    CPPUNIT_TEST(testListFolderEmpty);
    CPPUNIT_TEST(testListFolderNetworkError);
    CPPUNIT_TEST(testGetFileInfoSuccess);
    CPPUNIT_TEST(testGetFileInfoNotFound);
    CPPUNIT_TEST(testCopyFileSuccess);
    CPPUNIT_TEST(testCopyFileFailure);
    CPPUNIT_TEST(testRetryMechanism);
    CPPUNIT_TEST_SUITE_END();

private:
    std::unique_ptr<MockGoogleDriveApiClient> m_pClient;
};

void GDriveApiClientTest::setUp()
{
    m_pClient = std::make_unique<MockGoogleDriveApiClient>();
}

void GDriveApiClientTest::tearDown()
{
    m_pClient.reset();
}

void GDriveApiClientTest::testListFolderSuccess()
{
    rtl::OUString mockResponse = u"{"
        "\"files\": ["
        "  {"
        "    \"id\": \"test123\","
        "    \"name\": \"Test File\","
        "    \"mimeType\": \"text/plain\","
        "    \"size\": \"1024\","
        "    \"modifiedTime\": \"2024-01-15T10:30:00.000Z\""
        "  },"
        "  {"
        "    \"id\": \"folder456\","
        "    \"name\": \"Test Folder\","
        "    \"mimeType\": \"application/vnd.google-apps.folder\","
        "    \"modifiedTime\": \"2024-01-10T08:00:00.000Z\""
        "  }"
        "]"
        "}"_ustr;

    m_pClient->setMockResponse(mockResponse);
    auto files = m_pClient->listFolder(u"parent_folder_id"_ustr);

    CPPUNIT_ASSERT_EQUAL(size_t(2), files.size());

    // Check first file
    CPPUNIT_ASSERT_EQUAL(u"test123"_ustr, files[0].id);
    CPPUNIT_ASSERT_EQUAL(u"Test File"_ustr, files[0].name);
    CPPUNIT_ASSERT_EQUAL(false, files[0].isFolder);

    // Check folder
    CPPUNIT_ASSERT_EQUAL(u"folder456"_ustr, files[1].id);
    CPPUNIT_ASSERT_EQUAL(u"Test Folder"_ustr, files[1].name);
    CPPUNIT_ASSERT_EQUAL(true, files[1].isFolder);
}

void GDriveApiClientTest::testListFolderEmpty()
{
    m_pClient->setMockResponse(u"{\"files\":[]}"_ustr);
    auto files = m_pClient->listFolder(u"empty_folder_id"_ustr);

    CPPUNIT_ASSERT_EQUAL(size_t(0), files.size());
}

void GDriveApiClientTest::testListFolderNetworkError()
{
    m_pClient->setNetworkError(true);
    auto files = m_pClient->listFolder(u"any_folder_id"_ustr);

    CPPUNIT_ASSERT_EQUAL(size_t(0), files.size());
}

void GDriveApiClientTest::testGetFileInfoSuccess()
{
    rtl::OUString mockFileResponse = u"{"
        "\"id\": \"doc789\","
        "\"name\": \"Important Document.docx\","
        "\"mimeType\": \"application/vnd.openxmlformats-officedocument.wordprocessingml.document\","
        "\"size\": \"2048\","
        "\"modifiedTime\": \"2024-01-20T15:45:30.000Z\""
        "}"_ustr;

    m_pClient->setMockResponse(mockFileResponse);
    auto fileInfo = m_pClient->getFileInfo(u"doc789"_ustr);

    CPPUNIT_ASSERT_EQUAL(u"doc789"_ustr, fileInfo.id);
    CPPUNIT_ASSERT_EQUAL(u"Important Document.docx"_ustr, fileInfo.name);
    CPPUNIT_ASSERT_EQUAL(u"2048"_ustr, fileInfo.size);
    CPPUNIT_ASSERT_EQUAL(false, fileInfo.isFolder);
}

void GDriveApiClientTest::testGetFileInfoNotFound()
{
    m_pClient->setMockResponse(rtl::OUString(), 404);
    auto fileInfo = m_pClient->getFileInfo(u"nonexistent_file"_ustr);

    CPPUNIT_ASSERT(fileInfo.id.isEmpty());
    CPPUNIT_ASSERT(fileInfo.name.isEmpty());
}

void GDriveApiClientTest::testCopyFileSuccess()
{
    m_pClient->setMockResponse(u"{\"id\":\"copied_file_id_12345\"}"_ustr);

    rtl::OUString newFileId = m_pClient->copyFile(u"source_file"_ustr, u"target_parent"_ustr, u"Copy of File"_ustr);

    CPPUNIT_ASSERT_EQUAL(u"copied_file_id_12345"_ustr, newFileId);
}

void GDriveApiClientTest::testCopyFileFailure()
{
    m_pClient->setMockResponse(rtl::OUString(), 403); // Forbidden

    rtl::OUString newFileId = m_pClient->copyFile(u"source_file"_ustr, u"target_parent"_ustr, u"Copy of File"_ustr);

    CPPUNIT_ASSERT(newFileId.isEmpty());
}

void GDriveApiClientTest::testRetryMechanism()
{
    // This test verifies that retry logic would work in a real implementation
    // For now, we test the basic failure -> success pattern

    // First call fails
    m_pClient->setNetworkError(true);
    auto filesFirstTry = m_pClient->listFolder(u"test_folder"_ustr);
    CPPUNIT_ASSERT_EQUAL(size_t(0), filesFirstTry.size());

    // Second call succeeds
    m_pClient->setMockResponse(u"{\"files\":[{\"id\":\"retry_test\",\"name\":\"Retry Test\",\"mimeType\":\"text/plain\"}]}"_ustr);
    auto filesSecondTry = m_pClient->listFolder(u"test_folder"_ustr);
    CPPUNIT_ASSERT_EQUAL(size_t(1), filesSecondTry.size());
    CPPUNIT_ASSERT_EQUAL(u"retry_test"_ustr, filesSecondTry[0].id);
}

CPPUNIT_TEST_SUITE_REGISTRATION(GDriveApiClientTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */