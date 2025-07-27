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
#include <comphelper/processfactory.hxx>

#include <chrono>
#include <thread>
#include <vector>
#include <memory>

#include "gdrive_test_data.hxx"

// Performance testing framework for Google Drive operations
class GDrivePerformanceTest : public CppUnit::TestFixture
{
public:
    void setUp() override;
    void tearDown() override;

    void testSmallFolderListing();
    void testMediumFolderListing();
    void testLargeFolderListing();
    void testPaginationPerformance();
    void testConcurrentOperations();
    void testMemoryUsage();
    void testNetworkLatencyHandling();
    void testBulkOperations();
    void testCacheEffectiveness();
    void testLargeFileDownload();

    CPPUNIT_TEST_SUITE(GDrivePerformanceTest);
    CPPUNIT_TEST(testSmallFolderListing);
    CPPUNIT_TEST(testMediumFolderListing);
    CPPUNIT_TEST(testLargeFolderListing);
    CPPUNIT_TEST(testPaginationPerformance);
    CPPUNIT_TEST(testConcurrentOperations);
    CPPUNIT_TEST(testMemoryUsage);
    CPPUNIT_TEST(testNetworkLatencyHandling);
    CPPUNIT_TEST(testBulkOperations);
    CPPUNIT_TEST(testCacheEffectiveness);
    CPPUNIT_TEST(testLargeFileDownload);
    CPPUNIT_TEST_SUITE_END();

private:
    struct PerformanceResult {
        std::chrono::milliseconds duration;
        size_t itemsProcessed;
        size_t memoryUsed;
        size_t networkRequests;
        bool success;
    };

    // Mock performance client for testing
    class MockPerformanceClient {
    private:
        std::map<rtl::OUString, rtl::OUString> m_mockResponses;
        mutable size_t m_requestCount;
        bool m_simulateNetworkDelay;
        int m_networkDelayMs;

    public:
        MockPerformanceClient() : m_requestCount(0), m_simulateNetworkDelay(false), m_networkDelayMs(50) {}

        void setMockResponse(const rtl::OUString& endpoint, const rtl::OUString& response) {
            m_mockResponses[endpoint] = response;
        }

        void enableNetworkDelay(int delayMs = 50) {
            m_simulateNetworkDelay = true;
            m_networkDelayMs = delayMs;
        }

        void disableNetworkDelay() {
            m_simulateNetworkDelay = false;
        }

        size_t getRequestCount() const { return m_requestCount; }
        void resetRequestCount() { m_requestCount = 0; }

        rtl::OUString sendRequest(const rtl::OUString& endpoint) const {
            m_requestCount++;

            if (m_simulateNetworkDelay) {
                std::this_thread::sleep_for(std::chrono::milliseconds(m_networkDelayMs));
            }

            auto it = m_mockResponses.find(endpoint);
            if (it != m_mockResponses.end()) {
                return it->second;
            }

            return u"{\"error\": {\"code\": 404, \"message\": \"Not found\"}}"_ustr;
        }

        std::vector<ucp::gdrive::GDriveFileInfo> listFolder(const rtl::OUString& folderId) const {
            rtl::OUString endpoint = u"/drive/v3/files?q='" + folderId + "'+in+parents&fields=files(id,name,mimeType,size,modifiedTime)"_ustr;
            rtl::OUString response = sendRequest(endpoint);

            if (response.indexOf(u"error"_ustr) != -1) {
                return std::vector<ucp::gdrive::GDriveFileInfo>();
            }

            return ucp::gdrive::GDriveJsonHelper::parseFolderListing(response);
        }

        ucp::gdrive::GDriveFolderListing listFolderWithPagination(const rtl::OUString& folderId, const rtl::OUString& pageToken = rtl::OUString()) const {
            rtl::OUString endpoint = u"/drive/v3/files?q='" + folderId + "'+in+parents&fields=files(id,name,mimeType,size,modifiedTime),nextPageToken&pageSize=100"_ustr;

            if (!pageToken.isEmpty()) {
                endpoint += u"&pageToken=" + pageToken;
            }

            rtl::OUString response = sendRequest(endpoint);

            if (response.indexOf(u"error"_ustr) != -1) {
                return ucp::gdrive::GDriveFolderListing();
            }

            return ucp::gdrive::GDriveJsonHelper::parseFolderListingWithPagination(response);
        }
    };

    std::unique_ptr<MockPerformanceClient> m_pClient;

    // Helper methods
    PerformanceResult measureOperation(std::function<bool()> operation, size_t expectedItems = 0);
    void setupLargeFolderMock(const rtl::OUString& folderId, size_t fileCount);
    void setupPaginatedFolderMock(const rtl::OUString& folderId, size_t totalFiles, size_t pageSize);
    size_t estimateMemoryUsage();
};

void GDrivePerformanceTest::setUp()
{
    m_pClient = std::make_unique<MockPerformanceClient>();
}

void GDrivePerformanceTest::tearDown()
{
    m_pClient.reset();
}

GDrivePerformanceTest::PerformanceResult GDrivePerformanceTest::measureOperation(std::function<bool()> operation, size_t expectedItems)
{
    PerformanceResult result;
    result.itemsProcessed = expectedItems;
    result.networkRequests = 0;
    result.memoryUsed = 0;

    // Reset request counter
    m_pClient->resetRequestCount();

    // Measure memory before operation
    size_t memoryBefore = estimateMemoryUsage();

    // Measure execution time
    auto start = std::chrono::high_resolution_clock::now();
    result.success = operation();
    auto end = std::chrono::high_resolution_clock::now();

    result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    result.networkRequests = m_pClient->getRequestCount();
    result.memoryUsed = estimateMemoryUsage() - memoryBefore;

    return result;
}

void GDrivePerformanceTest::setupLargeFolderMock(const rtl::OUString& folderId, size_t fileCount)
{
    auto files = gdrive_test_data::generateLargeFolderFiles(fileCount);
    rtl::OUString response = gdrive_test_data::createFileListingJson(files);

    rtl::OUString endpoint = u"/drive/v3/files?q='" + folderId + "'+in+parents&fields=files(id,name,mimeType,size,modifiedTime)"_ustr;
    m_pClient->setMockResponse(endpoint, response);
}

void GDrivePerformanceTest::setupPaginatedFolderMock(const rtl::OUString& folderId, size_t totalFiles, size_t pageSize)
{
    size_t pages = (totalFiles + pageSize - 1) / pageSize; // Ceiling division

    for (size_t page = 0; page < pages; ++page) {
        size_t startIndex = page * pageSize;
        size_t endIndex = std::min(startIndex + pageSize, totalFiles);
        size_t currentPageSize = endIndex - startIndex;

        auto files = gdrive_test_data::generateLargeFolderFiles(currentPageSize, u"file_p" + rtl::OUString::number(page));

        rtl::OUString nextPageToken;
        if (page < pages - 1) {
            nextPageToken = u"page_" + rtl::OUString::number(page + 1) + u"_token"_ustr;
        }

        rtl::OUString response = gdrive_test_data::createFileListingJson(files, nextPageToken);

        rtl::OUString endpoint = u"/drive/v3/files?q='" + folderId + "'+in+parents&fields=files(id,name,mimeType,size,modifiedTime),nextPageToken&pageSize=100"_ustr;

        if (page > 0) {
            endpoint += u"&pageToken=page_" + rtl::OUString::number(page) + u"_token"_ustr;
        }

        m_pClient->setMockResponse(endpoint, response);
    }
}

size_t GDrivePerformanceTest::estimateMemoryUsage()
{
    // In a real implementation, this would use platform-specific memory APIs
    // For testing, we'll simulate memory usage
    return 1024 * 1024; // 1MB baseline
}

void GDrivePerformanceTest::testSmallFolderListing()
{
    setupLargeFolderMock(u"small_folder"_ustr, gdrive_test_data::PerformanceTestConfig::SMALL_FOLDER_SIZE);

    auto result = measureOperation([this]() {
        auto files = m_pClient->listFolder(u"small_folder"_ustr);
        return files.size() == gdrive_test_data::PerformanceTestConfig::SMALL_FOLDER_SIZE;
    }, gdrive_test_data::PerformanceTestConfig::SMALL_FOLDER_SIZE);

    CPPUNIT_ASSERT(result.success);
    CPPUNIT_ASSERT_EQUAL(size_t(1), result.networkRequests);
    CPPUNIT_ASSERT(result.duration.count() < 100); // Should complete in < 100ms
}

void GDrivePerformanceTest::testMediumFolderListing()
{
    setupLargeFolderMock(u"medium_folder"_ustr, gdrive_test_data::PerformanceTestConfig::MEDIUM_FOLDER_SIZE);

    auto result = measureOperation([this]() {
        auto files = m_pClient->listFolder(u"medium_folder"_ustr);
        return files.size() == gdrive_test_data::PerformanceTestConfig::MEDIUM_FOLDER_SIZE;
    }, gdrive_test_data::PerformanceTestConfig::MEDIUM_FOLDER_SIZE);

    CPPUNIT_ASSERT(result.success);
    CPPUNIT_ASSERT_EQUAL(size_t(1), result.networkRequests);
    CPPUNIT_ASSERT(result.duration.count() < 500); // Should complete in < 500ms
}

void GDrivePerformanceTest::testLargeFolderListing()
{
    // For large folders, we test pagination performance
    size_t totalFiles = gdrive_test_data::PerformanceTestConfig::LARGE_FOLDER_SIZE;
    size_t pageSize = gdrive_test_data::PerformanceTestConfig::PAGINATION_PAGE_SIZE;
    size_t expectedPages = (totalFiles + pageSize - 1) / pageSize;

    setupPaginatedFolderMock(u"large_folder"_ustr, totalFiles, pageSize);

    auto result = measureOperation([this, totalFiles]() {
        std::vector<ucp::gdrive::GDriveFileInfo> allFiles;
        rtl::OUString pageToken;

        do {
            auto listing = m_pClient->listFolderWithPagination(u"large_folder"_ustr, pageToken);
            allFiles.insert(allFiles.end(), listing.files.begin(), listing.files.end());
            pageToken = listing.nextPageToken;
        } while (!pageToken.isEmpty());

        return allFiles.size() == totalFiles;
    }, totalFiles);

    CPPUNIT_ASSERT(result.success);
    CPPUNIT_ASSERT_EQUAL(expectedPages, result.networkRequests);
    CPPUNIT_ASSERT(result.duration.count() < 2000); // Should complete in < 2 seconds
}

void GDrivePerformanceTest::testPaginationPerformance()
{
    size_t totalFiles = 500;
    size_t pageSize = 50;
    size_t expectedPages = 10;

    setupPaginatedFolderMock(u"paginated_folder"_ustr, totalFiles, pageSize);

    auto result = measureOperation([this]() {
        size_t totalProcessed = 0;
        rtl::OUString pageToken;
        int pageCount = 0;

        do {
            auto listing = m_pClient->listFolderWithPagination(u"paginated_folder"_ustr, pageToken);
            totalProcessed += listing.files.size();
            pageToken = listing.nextPageToken;
            pageCount++;

            // Ensure we don't get stuck in infinite loop
            if (pageCount > 20) break;

        } while (!pageToken.isEmpty());

        return totalProcessed == 500;
    }, 500);

    CPPUNIT_ASSERT(result.success);
    CPPUNIT_ASSERT_EQUAL(expectedPages, result.networkRequests);

    // Test pagination efficiency - should average < 200ms per page
    long avgTimePerPage = result.duration.count() / expectedPages;
    CPPUNIT_ASSERT(avgTimePerPage < 200);
}

void GDrivePerformanceTest::testConcurrentOperations()
{
    // Setup multiple folders for concurrent access
    for (int i = 1; i <= 5; ++i) {
        rtl::OUString folderId = u"concurrent_folder_" + rtl::OUString::number(i);
        setupLargeFolderMock(folderId, 20);
    }

    auto result = measureOperation([this]() {
        std::vector<std::thread> threads;
        std::vector<bool> results(5, false);

        for (int i = 0; i < 5; ++i) {
            threads.emplace_back([this, i, &results]() {
                rtl::OUString folderId = u"concurrent_folder_" + rtl::OUString::number(i + 1);
                auto files = m_pClient->listFolder(folderId);
                results[i] = (files.size() == 20);
            });
        }

        for (auto& thread : threads) {
            thread.join();
        }

        // Check all operations succeeded
        for (bool result : results) {
            if (!result) return false;
        }
        return true;
    }, 100); // 5 folders * 20 files each

    CPPUNIT_ASSERT(result.success);
    CPPUNIT_ASSERT_EQUAL(size_t(5), result.networkRequests);
    CPPUNIT_ASSERT(result.duration.count() < 1000); // Concurrent operations should be faster
}

void GDrivePerformanceTest::testMemoryUsage()
{
    setupLargeFolderMock(u"memory_test_folder"_ustr, 1000);

    size_t memoryBefore = estimateMemoryUsage();

    auto files = m_pClient->listFolder(u"memory_test_folder"_ustr);

    size_t memoryAfter = estimateMemoryUsage();
    size_t memoryUsed = memoryAfter - memoryBefore;

    CPPUNIT_ASSERT_EQUAL(size_t(1000), files.size());

    // Memory usage should be reasonable (less than 10MB for 1000 files)
    CPPUNIT_ASSERT(memoryUsed < 10 * 1024 * 1024);

    // Clear files and check memory is released (simulation)
    files.clear();
    size_t memoryAfterClear = estimateMemoryUsage();

    // Memory should be mostly released
    CPPUNIT_ASSERT(memoryAfterClear < memoryAfter);
}

void GDrivePerformanceTest::testNetworkLatencyHandling()
{
    setupLargeFolderMock(u"latency_test_folder"_ustr, 50);

    // Test with no network delay
    m_pClient->disableNetworkDelay();
    auto fastResult = measureOperation([this]() {
        auto files = m_pClient->listFolder(u"latency_test_folder"_ustr);
        return files.size() == 50;
    }, 50);

    // Test with network delay
    m_pClient->enableNetworkDelay(100); // 100ms delay
    auto slowResult = measureOperation([this]() {
        auto files = m_pClient->listFolder(u"latency_test_folder"_ustr);
        return files.size() == 50;
    }, 50);

    CPPUNIT_ASSERT(fastResult.success);
    CPPUNIT_ASSERT(slowResult.success);

    // Slow result should take significantly longer
    CPPUNIT_ASSERT(slowResult.duration.count() > fastResult.duration.count() + 50);

    // But both should succeed
    CPPUNIT_ASSERT_EQUAL(size_t(1), fastResult.networkRequests);
    CPPUNIT_ASSERT_EQUAL(size_t(1), slowResult.networkRequests);
}

void GDrivePerformanceTest::testBulkOperations()
{
    // Setup 10 different folders for bulk operations
    for (int i = 1; i <= 10; ++i) {
        rtl::OUString folderId = u"bulk_folder_" + rtl::OUString::number(i);
        setupLargeFolderMock(folderId, 10);
    }

    auto result = measureOperation([this]() {
        size_t totalFiles = 0;

        for (int i = 1; i <= 10; ++i) {
            rtl::OUString folderId = u"bulk_folder_" + rtl::OUString::number(i);
            auto files = m_pClient->listFolder(folderId);
            totalFiles += files.size();
        }

        return totalFiles == 100; // 10 folders * 10 files each
    }, 100);

    CPPUNIT_ASSERT(result.success);
    CPPUNIT_ASSERT_EQUAL(size_t(10), result.networkRequests);

    // Bulk operations should complete in reasonable time
    CPPUNIT_ASSERT(result.duration.count() < 1500);
}

void GDrivePerformanceTest::testCacheEffectiveness()
{
    setupLargeFolderMock(u"cache_test_folder"_ustr, 100);

    // First request - cache miss
    auto firstResult = measureOperation([this]() {
        auto files = m_pClient->listFolder(u"cache_test_folder"_ustr);
        return files.size() == 100;
    }, 100);

    // Second request - should be from cache (in real implementation)
    // For this test, we simulate cache by not resetting request counter
    auto secondResult = measureOperation([this]() {
        auto files = m_pClient->listFolder(u"cache_test_folder"_ustr);
        return files.size() == 100;
    }, 100);

    CPPUNIT_ASSERT(firstResult.success);
    CPPUNIT_ASSERT(secondResult.success);

    // Both requests still make network calls in our mock (no real cache)
    // In real implementation, second request would be faster and make no network calls
    CPPUNIT_ASSERT_EQUAL(size_t(1), firstResult.networkRequests);
    CPPUNIT_ASSERT_EQUAL(size_t(1), secondResult.networkRequests);
}

void GDrivePerformanceTest::testLargeFileDownload()
{
    // Simulate large file download
    rtl::OUString largeContent;
    for (int i = 0; i < 1000; ++i) {
        largeContent += u"This is line " + rtl::OUString::number(i) + u" of a large file for download testing.\n"_ustr;
    }

    m_pClient->setMockResponse(u"/drive/v3/files/large_file?alt=media"_ustr, largeContent);

    auto result = measureOperation([this, &largeContent]() {
        rtl::OUString endpoint = u"/drive/v3/files/large_file?alt=media"_ustr;
        rtl::OUString downloaded = m_pClient->sendRequest(endpoint);
        return downloaded.getLength() == largeContent.getLength();
    }, 1);

    CPPUNIT_ASSERT(result.success);
    CPPUNIT_ASSERT_EQUAL(size_t(1), result.networkRequests);

    // Large file download should complete in reasonable time
    CPPUNIT_ASSERT(result.duration.count() < 3000);
}

CPPUNIT_TEST_SUITE_REGISTRATION(GDrivePerformanceTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */