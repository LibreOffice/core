/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/ustring.hxx>
#include <vector>
#include <map>

/**
 * Test data utilities for Google Drive integration tests
 * Provides predefined test data, mock responses, and test environment setup
 */
namespace gdrive_test_data {

// Standard test file IDs
constexpr char MOCK_ROOT_FOLDER_ID[] = "root";
constexpr char MOCK_DOCUMENTS_FOLDER_ID[] = "documents_folder_123";
constexpr char MOCK_TEST_FILE_ID[] = "test_file_456";
constexpr char MOCK_LARGE_FOLDER_ID[] = "large_folder_789";

// Test file information
struct MockFileInfo {
    rtl::OUString id;
    rtl::OUString name;
    rtl::OUString mimeType;
    rtl::OUString size;
    rtl::OUString modifiedTime;
    bool isFolder;
};

// Predefined test files
inline std::vector<MockFileInfo> getStandardTestFiles() {
    return {
        {
            u"folder_001"_ustr,
            u"Documents"_ustr,
            u"application/vnd.google-apps.folder"_ustr,
            u""_ustr,
            u"2024-01-15T10:30:00.000Z"_ustr,
            true
        },
        {
            u"file_001"_ustr,
            u"Important.docx"_ustr,
            u"application/vnd.openxmlformats-officedocument.wordprocessingml.document"_ustr,
            u"2048"_ustr,
            u"2024-01-16T14:20:00.000Z"_ustr,
            false
        },
        {
            u"file_002"_ustr,
            u"Spreadsheet.xlsx"_ustr,
            u"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"_ustr,
            u"4096"_ustr,
            u"2024-01-17T09:15:00.000Z"_ustr,
            false
        },
        {
            u"file_003"_ustr,
            u"Presentation.pptx"_ustr,
            u"application/vnd.openxmlformats-officedocument.presentationml.presentation"_ustr,
            u"8192"_ustr,
            u"2024-01-18T16:45:00.000Z"_ustr,
            false
        }
    };
}

// Generate JSON for file listing
inline rtl::OUString createFileListingJson(const std::vector<MockFileInfo>& files, const rtl::OUString& nextPageToken = rtl::OUString()) {
    rtl::OUString json = u"{\"files\": ["_ustr;

    for (size_t i = 0; i < files.size(); ++i) {
        if (i > 0) json += u","_ustr;

        json += u"{"_ustr;
        json += u"\"id\": \"" + files[i].id + u"\","_ustr;
        json += u"\"name\": \"" + files[i].name + u"\","_ustr;
        json += u"\"mimeType\": \"" + files[i].mimeType + u"\""_ustr;

        if (!files[i].size.isEmpty()) {
            json += u",\"size\": \"" + files[i].size + u"\""_ustr;
        }

        json += u",\"modifiedTime\": \"" + files[i].modifiedTime + u"\""_ustr;
        json += u"}"_ustr;
    }

    json += u"]"_ustr;

    if (!nextPageToken.isEmpty()) {
        json += u",\"nextPageToken\": \"" + nextPageToken + u"\""_ustr;
    }

    json += u"}"_ustr;
    return json;
}

// Generate large folder with many files for pagination testing
inline std::vector<MockFileInfo> generateLargeFolderFiles(int count, const rtl::OUString& prefix = u"file"_ustr) {
    std::vector<MockFileInfo> files;
    files.reserve(count);

    for (int i = 1; i <= count; ++i) {
        MockFileInfo file;
        file.id = prefix + u"_" + rtl::OUString::number(i);
        file.name = u"File " + rtl::OUString::number(i) + u".txt"_ustr;
        file.mimeType = u"text/plain"_ustr;
        file.size = rtl::OUString::number(i * 100);
        file.modifiedTime = u"2024-01-01T00:00:00.000Z"_ustr;
        file.isFolder = false;
        files.push_back(file);
    }

    return files;
}

// Standard mock responses for common operations
inline std::map<rtl::OUString, rtl::OUString> getStandardMockResponses() {
    std::map<rtl::OUString, rtl::OUString> responses;

    // OAuth token response
    responses[u"/oauth2/v4/token"_ustr] = u"{"
        "\"access_token\": \"mock_access_token_12345\","
        "\"refresh_token\": \"mock_refresh_token_67890\","
        "\"expires_in\": 3600,"
        "\"token_type\": \"Bearer\""
        "}"_ustr;

    // Root folder listing
    auto rootFiles = getStandardTestFiles();
    responses[u"/drive/v3/files?q='root'+in+parents&fields=files(id,name,mimeType,size,modifiedTime)"_ustr] =
        createFileListingJson(rootFiles);

    // Single file info
    responses[u"/drive/v3/files/file_001?fields=id,name,mimeType,size,modifiedTime"_ustr] = u"{"
        "\"id\": \"file_001\","
        "\"name\": \"Important.docx\","
        "\"mimeType\": \"application/vnd.openxmlformats-officedocument.wordprocessingml.document\","
        "\"size\": \"2048\","
        "\"modifiedTime\": \"2024-01-16T14:20:00.000Z\""
        "}"_ustr;

    // File download
    responses[u"/drive/v3/files/file_001?alt=media"_ustr] =
        u"Mock file content for testing download functionality"_ustr;

    // Folder creation
    responses[u"/drive/v3/files"_ustr] = u"{"
        "\"id\": \"new_folder_123\","
        "\"name\": \"New Test Folder\","
        "\"mimeType\": \"application/vnd.google-apps.folder\""
        "}"_ustr;

    // File copy
    responses[u"/drive/v3/files/file_001/copy"_ustr] = u"{"
        "\"id\": \"copied_file_456\","
        "\"name\": \"Copy of Important.docx\""
        "}"_ustr;

    return responses;
}

// Error responses for testing error scenarios
inline std::map<rtl::OUString, rtl::OUString> getErrorResponses() {
    std::map<rtl::OUString, rtl::OUString> errors;

    errors[u"404"_ustr] = u"{\"error\": {\"code\": 404, \"message\": \"File not found\"}}"_ustr;
    errors[u"401"_ustr] = u"{\"error\": {\"code\": 401, \"message\": \"Invalid credentials\"}}"_ustr;
    errors[u"403"_ustr] = u"{\"error\": {\"code\": 403, \"message\": \"Forbidden\"}}"_ustr;
    errors[u"429"_ustr] = u"{\"error\": {\"code\": 429, \"message\": \"Too many requests\"}}"_ustr;
    errors[u"500"_ustr] = u"{\"error\": {\"code\": 500, \"message\": \"Internal server error\"}}"_ustr;
    errors[u"503"_ustr] = u"{\"error\": {\"code\": 503, \"message\": \"Service unavailable\"}}"_ustr;

    return errors;
}

// Test file content for download testing
inline std::map<rtl::OUString, rtl::OUString> getTestFileContents() {
    std::map<rtl::OUString, rtl::OUString> contents;

    contents[u"text/plain"_ustr] = u"This is a plain text file for testing purposes."_ustr;
    contents[u"application/json"_ustr] = u"{\"test\": \"data\", \"number\": 42}"_ustr;
    contents[u"text/html"_ustr] = u"<html><body><h1>Test HTML Content</h1></body></html>"_ustr;
    contents[u"application/xml"_ustr] = u"<?xml version=\"1.0\"?><root><test>data</test></root>"_ustr;

    // Binary content simulation (base64 encoded)
    contents[u"image/png"_ustr] = u"iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg=="_ustr;

    return contents;
}

// URLs for various test scenarios
struct TestUrls {
    static constexpr char ROOT_LISTING[] = "/drive/v3/files?q='root'+in+parents&fields=files(id,name,mimeType,size,modifiedTime)";
    static constexpr char FILE_INFO[] = "/drive/v3/files/{FILE_ID}?fields=id,name,mimeType,size,modifiedTime";
    static constexpr char FILE_DOWNLOAD[] = "/drive/v3/files/{FILE_ID}?alt=media";
    static constexpr char FILE_UPLOAD[] = "/upload/drive/v3/files?uploadType=multipart";
    static constexpr char FILE_UPDATE[] = "/upload/drive/v3/files/{FILE_ID}?uploadType=multipart";
    static constexpr char FILE_COPY[] = "/drive/v3/files/{FILE_ID}/copy";
    static constexpr char FILE_DELETE[] = "/drive/v3/files/{FILE_ID}";
    static constexpr char FOLDER_CREATE[] = "/drive/v3/files";
    static constexpr char OAUTH_TOKEN[] = "/oauth2/v4/token";
};

// Helper to replace placeholders in URLs
inline rtl::OUString replaceUrlPlaceholders(const rtl::OUString& url, const rtl::OUString& fileId) {
    return url.replaceAll(u"{FILE_ID}"_ustr, fileId);
}

// Performance test configuration
struct PerformanceTestConfig {
    static constexpr int SMALL_FOLDER_SIZE = 10;
    static constexpr int MEDIUM_FOLDER_SIZE = 100;
    static constexpr int LARGE_FOLDER_SIZE = 1000;
    static constexpr int PAGINATION_PAGE_SIZE = 100;
    static constexpr int MAX_RETRY_ATTEMPTS = 3;
    static constexpr int NETWORK_DELAY_MS = 50;
    static constexpr int TIMEOUT_MS = 5000;
};

} // namespace gdrive_test_data

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */