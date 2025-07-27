/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "DropboxApiClient.hxx"
#include "dropbox_json.hxx"
#include "oauth2_http_server.hxx"
#include <config_oauth2.h>
#include <com/sun/star/io/SequenceInputStream.hpp>
#include <com/sun/star/io/SequenceOutputStream.hpp>
#include <com/sun/star/io/Pipe.hpp>
#include <com/sun/star/ucb/AuthenticationRequest.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <rtl/ustrbuf.hxx>
#include <rtl/string.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/authenticationfallback.hxx>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <sal/log.hxx>
#include <curl/curl.h>
#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <sstream>
#include <thread>
#include <chrono>

using namespace com::sun::star;

// Base64 encoding function for Basic Auth
std::string base64Encode(const std::string& input) {
    static const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string encoded;
    int val = 0, valb = -6;
    for (unsigned char c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            encoded.push_back(chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        encoded.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    while (encoded.size() % 4) {
        encoded.push_back('=');
    }
    return encoded;
}

namespace ucp {
namespace dropbox {

DropboxApiClient::DropboxApiClient(const uno::Reference<ucb::XCommandEnvironment>& xCmdEnv)
    : m_xCmdEnv(xCmdEnv)
    , m_sAccessToken()
    , m_pCurl(nullptr)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    m_pCurl = curl_easy_init();
}

DropboxApiClient::~DropboxApiClient()
{
    if (m_pCurl) {
        curl_easy_cleanup(m_pCurl);
    }
    curl_global_cleanup();
}

std::vector<DropboxFileInfo> DropboxApiClient::listFolder(const rtl::OUString& folderId)
{
    SAL_WARN("ucb.ucp.dropbox", "listFolder() called for folder ID: " + folderId);

    // Note: listFolder() method called

    std::vector<DropboxFileInfo> aFiles;

    rtl::OUString sAccessToken = getAccessToken();
    SAL_WARN("ucb.ucp.dropbox", "Got access token length: " + OUString::number(sAccessToken.getLength()));
    if (sAccessToken.isEmpty()) {
        SAL_WARN("ucb.ucp.dropbox", "No access token available - returning empty list");
        return aFiles;
    }

    SAL_WARN("ucb.ucp.dropbox", "Access token preview: " + sAccessToken.copy(0, std::min(30, (int)sAccessToken.getLength())));

    // Build Dropbox API v2 URL and JSON request body for listing files
    rtl::OUString sApiUrl = u"https://api.dropboxapi.com/2/files/list_folder"_ustr;

    // Prepare JSON request body for Dropbox API v2
    rtl::OUStringBuffer sJsonBody;
    sJsonBody.append(u"{"_ustr);
    sJsonBody.append(u"\"path\": "_ustr);
    if (folderId.isEmpty() || folderId == u"root"_ustr) {
        sJsonBody.append(u"\"\""_ustr);  // Empty string for root folder
    } else {
        sJsonBody.append(u"\""_ustr);
        if (!folderId.startsWith(u"/"_ustr)) {
            sJsonBody.append(u"/"_ustr);
        }
        sJsonBody.append(folderId);
        sJsonBody.append(u"\""_ustr);
    }
    sJsonBody.append(u",\"limit\": 2000"_ustr);
    sJsonBody.append(u",\"recursive\": false"_ustr);
    sJsonBody.append(u"}"_ustr);

    rtl::OUString sRequestBody = sJsonBody.makeStringAndClear();
    SAL_WARN("ucb.ucp.dropbox", "Making API request to: " + sApiUrl);
    SAL_WARN("ucb.ucp.dropbox", "Request body: " + sRequestBody);

    rtl::OUString sResponse = sendRequestForString(
        u"POST"_ustr,
        sApiUrl,
        sRequestBody
    );

    SAL_WARN("ucb.ucp.dropbox", "API response length: " + OUString::number(sResponse.getLength()));

    // Show debug dialog with API result (fallback method)
    try {
        uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
        if (xContext.is()) {
            uno::Reference<task::XInteractionHandler> xIH(
                xContext->getServiceManager()->createInstanceWithContext(
                    u"com.sun.star.task.InteractionHandler"_ustr, xContext),
                uno::UNO_QUERY);
            if (xIH.is()) {
                OUString sDebugMsg = u"DEBUG: API URL: "_ustr + sApiUrl +
                    u"\nResponse length: "_ustr + OUString::number(sResponse.getLength()) +
                    (sResponse.isEmpty() ? u" (EMPTY!)"_ustr : u" chars received"_ustr);
                if (sResponse.getLength() > 0) {
                    sDebugMsg += u"\nFirst 100 chars: "_ustr + sResponse.copy(0, std::min(100, (int)sResponse.getLength()));
                }

                rtl::Reference<ucbhelper::AuthenticationFallbackRequest> xApiRequest
                    = new ucbhelper::AuthenticationFallbackRequest(
                        sDebugMsg,
                        u"test://debug-api-fallback"_ustr);
                xIH->handle(xApiRequest);
            }
        }
    } catch (const uno::Exception& e) {
        SAL_WARN("ucb.ucp.dropbox", "Exception during debug API request: " + e.Message);
    } catch (...) {
        SAL_WARN("ucb.ucp.dropbox", "Unknown exception during debug API request");
    }

    // Parse JSON response using proper JSON helper
    if (!sResponse.isEmpty()) {
        SAL_WARN("ucb.ucp.dropbox", "Parsing JSON response with " + OUString::number(sResponse.getLength()) + " chars");

        aFiles = DropboxJsonHelper::parseFolderListing(sResponse);

        SAL_WARN("ucb.ucp.dropbox", "Successfully parsed " + OUString::number(aFiles.size()) + " files from JSON");

        // Log parsed files for debugging
        for (const auto& fileInfo : aFiles) {
            SAL_WARN("ucb.ucp.dropbox", "Parsed file: " + fileInfo.name + " (id: " + fileInfo.id +
                     ", folder: " + (fileInfo.isFolder ? u"yes" : u"no") + ")");
        }
    }

    return aFiles;
}

std::vector<DropboxFileInfo> DropboxApiClient::listFolderComplete(const rtl::OUString& folderId, sal_Int32 maxFiles)
{
    SAL_WARN("ucb.ucp.dropbox", "listFolderComplete() called for folder ID: " + folderId + " (max files: " + OUString::number(maxFiles) + ")");

    std::vector<DropboxFileInfo> allFiles;
    rtl::OUString nextPageToken;
    sal_Int32 totalFiles = 0;
    sal_Int32 pageCount = 0;

    rtl::OUString sAccessToken = getAccessToken();
    if (sAccessToken.isEmpty()) {
        SAL_WARN("ucb.ucp.dropbox", "No access token available - returning empty list");
        return allFiles;
    }

    do {
        pageCount++;
        rtl::OUString tokenDisplay = nextPageToken.isEmpty() ?
            u"(none)"_ustr :
            (nextPageToken.copy(0, 20) + u"..."_ustr);
        SAL_WARN("ucb.ucp.dropbox", "Fetching page " << pageCount << " with token: " << tokenDisplay);

        // Build Dropbox API v2 URL for listing files with pagination
        rtl::OUString sApiUrl = u"https://api.dropboxapi.com/2/files/list_folder"_ustr;

        // Prepare JSON request body for Dropbox API v2
        rtl::OUStringBuffer sJsonBody;
        sJsonBody.append(u"{"_ustr);
        sJsonBody.append(u"\"path\": "_ustr);
        if (folderId.isEmpty() || folderId == u"root"_ustr) {
            sJsonBody.append(u"\"\""_ustr);  // Empty string for root folder
        } else {
            sJsonBody.append(u"\""_ustr);
            if (!folderId.startsWith(u"/"_ustr)) {
                sJsonBody.append(u"/"_ustr);
            }
            sJsonBody.append(folderId);
            sJsonBody.append(u"\""_ustr);
        }
        sJsonBody.append(u",\"limit\": 100"_ustr);
        sJsonBody.append(u",\"recursive\": false"_ustr);

        if (!nextPageToken.isEmpty()) {
            sJsonBody.append(u",\"cursor\": \""_ustr);
            sJsonBody.append(nextPageToken);
            sJsonBody.append(u"\""_ustr);
        }

        sJsonBody.append(u"}"_ustr);

        rtl::OUString sRequestBody = sJsonBody.makeStringAndClear();
        SAL_WARN("ucb.ucp.dropbox", "Making paginated API request to: " + sApiUrl);
        SAL_WARN("ucb.ucp.dropbox", "Request body: " + sRequestBody);

        rtl::OUString sResponse = sendRequestForString(
            u"POST"_ustr,
            sApiUrl,
            sRequestBody
        );

        if (!sResponse.isEmpty()) {
            DropboxFolderListing listing = DropboxJsonHelper::parseFolderListingWithPagination(sResponse);

            SAL_WARN("ucb.ucp.dropbox", "Page " + OUString::number(pageCount) + " returned " +
                     OUString::number(listing.files.size()) + " files, hasMore: " +
                     (listing.hasMore ? u"yes" : u"no"));

            // Add files from this page
            for (const auto& fileInfo : listing.files) {
                if (totalFiles >= maxFiles) {
                    SAL_WARN("ucb.ucp.dropbox", "Reached maximum file limit of " + OUString::number(maxFiles));
                    return allFiles;
                }
                allFiles.push_back(fileInfo);
                totalFiles++;
            }

            // Update pagination cursor
            nextPageToken = listing.cursor;

            // Break if no more pages or we hit a reasonable limit
            if (!listing.hasMore || pageCount >= 20) { // Safety limit of 20 pages (2000 files max)
                break;
            }
        } else {
            SAL_WARN("ucb.ucp.dropbox", "Empty response from API, stopping pagination");
            break;
        }

    } while (!nextPageToken.isEmpty() && totalFiles < maxFiles);

    SAL_WARN("ucb.ucp.dropbox", "listFolderComplete finished: " + OUString::number(totalFiles) +
             " total files in " + OUString::number(pageCount) + " pages");

    return allFiles;
}

DropboxFileInfo DropboxApiClient::getFileInfo(const rtl::OUString& fileId)
{
    SAL_WARN("ucb.ucp.dropbox", "getFileInfo() called for file ID: " + fileId);

    DropboxFileInfo aFileInfo;

    if (fileId.isEmpty()) {
        return aFileInfo;
    }

    rtl::OUString sAccessToken = getAccessToken();
    if (sAccessToken.isEmpty()) {
        SAL_WARN("ucb.ucp.dropbox", "No access token available for file info");
        return aFileInfo;
    }

    // Build Dropbox API v2 URL for getting single file info
    rtl::OUString sApiUrl = u"https://api.dropboxapi.com/2/files/get_metadata"_ustr;

    // Prepare JSON request body for Dropbox API v2
    rtl::OUStringBuffer sJsonBody;
    sJsonBody.append(u"{"_ustr);
    sJsonBody.append(u"\"path\": \""_ustr);
    if (!fileId.startsWith(u"/"_ustr)) {
        sJsonBody.append(u"/"_ustr);
    }
    sJsonBody.append(fileId);
    sJsonBody.append(u"\""_ustr);
    sJsonBody.append(u"}"_ustr);

    rtl::OUString sRequestBody = sJsonBody.makeStringAndClear();
    SAL_WARN("ucb.ucp.dropbox", "Making file info request to: " + sApiUrl);
    SAL_WARN("ucb.ucp.dropbox", "Request body: " + sRequestBody);

    rtl::OUString sResponse = sendRequestForString(
        u"POST"_ustr,
        sApiUrl,
        sRequestBody
    );

    SAL_WARN("ucb.ucp.dropbox", "File info response length: " + OUString::number(sResponse.getLength()));

    if (!sResponse.isEmpty()) {
        SAL_WARN("ucb.ucp.dropbox", "Parsing file info JSON response");

        try {
            boost::property_tree::ptree root;
            std::string jsonStr = rtl::OUStringToOString(sResponse, RTL_TEXTENCODING_UTF8).getStr();
            std::istringstream jsonStream(jsonStr);
            boost::property_tree::read_json(jsonStream, root);

            aFileInfo.id = rtl::OUString::createFromAscii(root.get<std::string>("path_lower", "").c_str());
            aFileInfo.name = rtl::OUString::createFromAscii(root.get<std::string>("name", "").c_str());
            aFileInfo.tag = rtl::OUString::createFromAscii(root.get<std::string>(".tag", "").c_str());
            aFileInfo.size = rtl::OUString::createFromAscii(root.get<std::string>("size", "").c_str());
            aFileInfo.modifiedTime = rtl::OUString::createFromAscii(root.get<std::string>("client_modified", "").c_str());
            aFileInfo.isFolder = (aFileInfo.tag == "folder");

            SAL_WARN("ucb.ucp.dropbox", "Successfully parsed file info: " + aFileInfo.name +
                     " (folder: " + (aFileInfo.isFolder ? u"yes" : u"no") + ")");
        } catch (const boost::property_tree::json_parser_error&) {
            SAL_WARN("ucb.ucp.dropbox", "JSON parsing failed for file info");
        } catch (const boost::property_tree::ptree_bad_path&) {
            SAL_WARN("ucb.ucp.dropbox", "Missing expected fields in file info response");
        }
    }

    return aFileInfo;
}

uno::Reference<io::XInputStream> DropboxApiClient::downloadFile(const rtl::OUString& fileId)
{
    SAL_WARN("ucb.ucp.dropbox", "downloadFile() called for file ID: " + fileId);

    if (fileId.isEmpty()) {
        return nullptr;
    }

    rtl::OUString sAccessToken = getAccessToken();
    if (sAccessToken.isEmpty()) {
        SAL_WARN("ucb.ucp.dropbox", "No access token available for download");
        return nullptr;
    }

    // Build Dropbox API v2 URL for downloading file content
    rtl::OUString sApiUrl = u"https://content.dropboxapi.com/2/files/download"_ustr;

    // Prepare JSON argument for Dropbox-API-Arg header
    rtl::OUStringBuffer sApiArg;
    sApiArg.append(u"{\"path\": \""_ustr);
    if (!fileId.startsWith(u"/"_ustr)) {
        sApiArg.append(u"/"_ustr);
    }
    sApiArg.append(fileId);
    sApiArg.append(u"\"}"_ustr);

    rtl::OUString sDropboxApiArg = sApiArg.makeStringAndClear();
    SAL_WARN("ucb.ucp.dropbox", "Making download request to: " + sApiUrl);
    SAL_WARN("ucb.ucp.dropbox", "Dropbox-API-Arg: " + sDropboxApiArg);

    // Use CURL to download file content
    if (!m_pCurl) {
        return nullptr;
    }

    HttpResponse response;
    response.responseCode = 0;

    std::string url = rtl::OUStringToOString(sApiUrl, RTL_TEXTENCODING_UTF8).getStr();

    curl_easy_setopt(m_pCurl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(m_pCurl, CURLOPT_POST, 1L);
    curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDS, "");  // Empty POST body
    curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDSIZE, 0L);

    // Add authorization and Dropbox-API-Arg headers
    struct curl_slist* headers = nullptr;
    std::string authHeader = std::string("Authorization: Bearer ") +
        rtl::OUStringToOString(sAccessToken, RTL_TEXTENCODING_UTF8).getStr();
    std::string apiArgHeader = std::string("Dropbox-API-Arg: ") +
        rtl::OUStringToOString(sDropboxApiArg, RTL_TEXTENCODING_UTF8).getStr();
    headers = curl_slist_append(headers, authHeader.c_str());
    headers = curl_slist_append(headers, apiArgHeader.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/octet-stream");

    curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(m_pCurl);

    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        SAL_WARN("ucb.ucp.dropbox", "CURL error during download: " + OUString::createFromAscii(curl_easy_strerror(res)));
        return nullptr;
    }

    curl_easy_getinfo(m_pCurl, CURLINFO_RESPONSE_CODE, &response.responseCode);

    if (response.responseCode >= 200 && response.responseCode < 300) {
        SAL_WARN("ucb.ucp.dropbox", "Download successful, received " << response.data.length() << " bytes");

        // Create input stream from downloaded data
        uno::Sequence<sal_Int8> aData(response.data.length());
        std::memcpy(aData.getArray(), response.data.c_str(), response.data.length());

        uno::Reference<io::XInputStream> xInputStream(
            io::SequenceInputStream::createStreamFromSequence(
                comphelper::getProcessComponentContext(), aData),
            uno::UNO_QUERY);

        return xInputStream;
    } else {
        SAL_WARN("ucb.ucp.dropbox", "Download failed with HTTP response code: " << response.responseCode);
        if (!response.data.empty()) {
            SAL_WARN("ucb.ucp.dropbox", "Error response: " << response.data.substr(0, 500));
        }
        return nullptr;
    }
}

void DropboxApiClient::uploadFile(const rtl::OUString& parentId, const rtl::OUString& fileName, const uno::Reference<io::XInputStream>& xInputStream)
{
    SAL_WARN("ucb.ucp.dropbox", "uploadFile() called: '" + fileName + "' to parent: " + parentId);

    if (fileName.isEmpty() || !xInputStream.is()) {
        SAL_WARN("ucb.ucp.dropbox", "Cannot upload file: invalid parameters");
        return;
    }

    rtl::OUString sAccessToken = getAccessToken();
    if (sAccessToken.isEmpty()) {
        SAL_WARN("ucb.ucp.dropbox", "No access token available for file upload");
        return;
    }

    // Read file content from input stream
    uno::Sequence<sal_Int8> aBuffer;
    sal_Int32 nBytesRead = 0;
    std::string fileContent;

    try {
        do {
            nBytesRead = xInputStream->readBytes(aBuffer, 8192);
            if (nBytesRead > 0) {
                fileContent.append(reinterpret_cast<const char*>(aBuffer.getConstArray()), nBytesRead);
            }
        } while (nBytesRead > 0);
    } catch (const uno::Exception& e) {
        SAL_WARN("ucb.ucp.dropbox", "Error reading file content: " + e.Message);
        return;
    }

    SAL_WARN("ucb.ucp.dropbox", "Read " + OUString::number(fileContent.length()) + " bytes from input stream");

    // Create JSON metadata for the new file (for Dropbox-API-Arg header)
    rtl::OUString sJsonMetadata = DropboxJsonHelper::createFileMetadata(fileName, parentId);
    std::string metadata = rtl::OUStringToOString(sJsonMetadata, RTL_TEXTENCODING_UTF8).getStr();

    // Build Dropbox API URL for uploading files
    rtl::OUString sApiUrl = u"https://content.dropboxapi.com/2/files/upload"_ustr;

    SAL_WARN("ucb.ucp.dropbox", "Making file upload request to: " + sApiUrl);

    // Use CURL to upload the file
    if (!m_pCurl) {
        return;
    }

    HttpResponse response;
    response.responseCode = 0;

    std::string url = rtl::OUStringToOString(sApiUrl, RTL_TEXTENCODING_UTF8).getStr();

    curl_easy_setopt(m_pCurl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(m_pCurl, CURLOPT_POST, 1L);
    curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDS, fileContent.c_str());
    curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDSIZE, fileContent.length());

    // Set headers for Dropbox API (Content-Type: application/octet-stream and Dropbox-API-Arg)
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/octet-stream");

    std::string authHeader = std::string("Authorization: Bearer ") +
        rtl::OUStringToOString(sAccessToken, RTL_TEXTENCODING_UTF8).getStr();
    headers = curl_slist_append(headers, authHeader.c_str());

    std::string dropboxApiArg = "Dropbox-API-Arg: " + metadata;
    headers = curl_slist_append(headers, dropboxApiArg.c_str());

    curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(m_pCurl);

    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        SAL_WARN("ucb.ucp.dropbox", "CURL error during file upload: " + OUString::createFromAscii(curl_easy_strerror(res)));
        return;
    }

    curl_easy_getinfo(m_pCurl, CURLINFO_RESPONSE_CODE, &response.responseCode);

    if (response.responseCode >= 200 && response.responseCode < 300) {
        SAL_WARN("ucb.ucp.dropbox", "File upload successful");
        SAL_WARN("ucb.ucp.dropbox", "Response: " + OUString::createFromAscii(response.data.substr(0, 200).c_str()));
    } else {
        SAL_WARN("ucb.ucp.dropbox", "File upload failed with HTTP response code: " + OUString::number(response.responseCode));
        SAL_WARN("ucb.ucp.dropbox", "Error response: " + OUString::createFromAscii(response.data.c_str()));
    }
}

void DropboxApiClient::updateFile(const rtl::OUString& fileId, const uno::Reference<io::XInputStream>& xInputStream)
{
    SAL_WARN("ucb.ucp.dropbox", "updateFile() called for file ID: " + fileId);

    if (fileId.isEmpty() || !xInputStream.is()) {
        SAL_WARN("ucb.ucp.dropbox", "Cannot update file: invalid parameters");
        return;
    }

    rtl::OUString sAccessToken = getAccessToken();
    if (sAccessToken.isEmpty()) {
        SAL_WARN("ucb.ucp.dropbox", "No access token available for file update");
        return;
    }

    // Read file content from input stream
    uno::Sequence<sal_Int8> aBuffer;
    sal_Int32 nBytesRead = 0;
    std::string fileContent;

    try {
        do {
            nBytesRead = xInputStream->readBytes(aBuffer, 8192);
            if (nBytesRead > 0) {
                fileContent.append(reinterpret_cast<const char*>(aBuffer.getConstArray()), nBytesRead);
            }
        } while (nBytesRead > 0);
    } catch (const uno::Exception& e) {
        SAL_WARN("ucb.ucp.dropbox", "Error reading file content: " + e.Message);
        return;
    }

    SAL_WARN("ucb.ucp.dropbox", "Read " + OUString::number(fileContent.length()) + " bytes from input stream");

    // Build Dropbox API URL for updating files (upload with overwrite mode)
    rtl::OUString sApiUrl = u"https://content.dropboxapi.com/2/files/upload"_ustr;

    // Create JSON metadata for file update (fileId is the path in Dropbox)
    rtl::OUString sJsonMetadata = DropboxJsonHelper::createFileMetadata(fileId, "");
    std::string metadata = rtl::OUStringToOString(sJsonMetadata, RTL_TEXTENCODING_UTF8).getStr();

    SAL_WARN("ucb.ucp.dropbox", "Making file update request to: " + sApiUrl);

    // Use CURL to update the file
    if (!m_pCurl) {
        return;
    }

    HttpResponse response;
    response.responseCode = 0;

    std::string url = rtl::OUStringToOString(sApiUrl, RTL_TEXTENCODING_UTF8).getStr();

    curl_easy_setopt(m_pCurl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(m_pCurl, CURLOPT_POST, 1L);
    curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDS, fileContent.c_str());
    curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDSIZE, fileContent.length());

    // Set headers for Dropbox API (Content-Type: application/octet-stream and Dropbox-API-Arg)
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/octet-stream");

    std::string authHeader = std::string("Authorization: Bearer ") +
        rtl::OUStringToOString(sAccessToken, RTL_TEXTENCODING_UTF8).getStr();
    headers = curl_slist_append(headers, authHeader.c_str());

    std::string dropboxApiArg = "Dropbox-API-Arg: " + metadata;
    headers = curl_slist_append(headers, dropboxApiArg.c_str());

    curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(m_pCurl);

    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        SAL_WARN("ucb.ucp.dropbox", "CURL error during file update: " + OUString::createFromAscii(curl_easy_strerror(res)));
        return;
    }

    curl_easy_getinfo(m_pCurl, CURLINFO_RESPONSE_CODE, &response.responseCode);

    if (response.responseCode >= 200 && response.responseCode < 300) {
        SAL_WARN("ucb.ucp.dropbox", "File update successful");
        SAL_WARN("ucb.ucp.dropbox", "Response: " + OUString::createFromAscii(response.data.substr(0, 200).c_str()));
    } else {
        SAL_WARN("ucb.ucp.dropbox", "File update failed with HTTP response code: " + OUString::number(response.responseCode));
        SAL_WARN("ucb.ucp.dropbox", "Error response: " + OUString::createFromAscii(response.data.c_str()));
    }
}

void DropboxApiClient::createFolder(const rtl::OUString& parentId, const rtl::OUString& folderName)
{
    SAL_WARN("ucb.ucp.dropbox", "createFolder() called: '" + folderName + "' in parent: " + parentId);

    if (folderName.isEmpty()) {
        SAL_WARN("ucb.ucp.dropbox", "Cannot create folder with empty name");
        return;
    }

    rtl::OUString sAccessToken = getAccessToken();
    if (sAccessToken.isEmpty()) {
        SAL_WARN("ucb.ucp.dropbox", "No access token available for folder creation");
        return;
    }

    // Create JSON metadata for the new folder
    rtl::OUString sJsonMetadata = DropboxJsonHelper::createFolderMetadata(folderName, parentId);
    SAL_WARN("ucb.ucp.dropbox", "Folder metadata: " + sJsonMetadata);

    // Build Dropbox API URL for creating folders
    rtl::OUString sApiUrl = u"https://api.dropboxapi.com/2/files/create_folder_v2"_ustr;

    SAL_WARN("ucb.ucp.dropbox", "Making folder creation request to: " + sApiUrl);

    // Use CURL to create the folder
    if (!m_pCurl) {
        return;
    }

    HttpResponse response;
    response.responseCode = 0;

    std::string url = rtl::OUStringToOString(sApiUrl, RTL_TEXTENCODING_UTF8).getStr();
    std::string jsonData = rtl::OUStringToOString(sJsonMetadata, RTL_TEXTENCODING_UTF8).getStr();

    curl_easy_setopt(m_pCurl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(m_pCurl, CURLOPT_POST, 1L);
    curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDS, jsonData.c_str());
    curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDSIZE, jsonData.length());

    // Set headers for JSON content and authorization
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    std::string authHeader = std::string("Authorization: Bearer ") +
        rtl::OUStringToOString(sAccessToken, RTL_TEXTENCODING_UTF8).getStr();
    headers = curl_slist_append(headers, authHeader.c_str());

    curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(m_pCurl);

    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        SAL_WARN("ucb.ucp.dropbox", "CURL error during folder creation: " + OUString::createFromAscii(curl_easy_strerror(res)));
        return;
    }

    curl_easy_getinfo(m_pCurl, CURLINFO_RESPONSE_CODE, &response.responseCode);

    if (response.responseCode >= 200 && response.responseCode < 300) {
        SAL_WARN("ucb.ucp.dropbox", "Folder creation successful");
        SAL_WARN("ucb.ucp.dropbox", "Response: " + OUString::createFromAscii(response.data.substr(0, 200).c_str()));
    } else {
        SAL_WARN("ucb.ucp.dropbox", "Folder creation failed with HTTP response code: " + OUString::number(response.responseCode));
        SAL_WARN("ucb.ucp.dropbox", "Error response: " + OUString::createFromAscii(response.data.c_str()));
    }
}

void DropboxApiClient::deleteFile(const rtl::OUString& fileId)
{
    SAL_WARN("ucb.ucp.dropbox", "deleteFile() called for file ID: " + fileId);

    if (fileId.isEmpty()) {
        SAL_WARN("ucb.ucp.dropbox", "Cannot delete file with empty ID");
        return;
    }

    rtl::OUString sAccessToken = getAccessToken();
    if (sAccessToken.isEmpty()) {
        SAL_WARN("ucb.ucp.dropbox", "No access token available for file deletion");
        return;
    }

    // Build Dropbox API URL for deleting files and create JSON metadata
    rtl::OUString sApiUrl = u"https://api.dropboxapi.com/2/files/delete_v2"_ustr;

    // Create JSON for delete request (fileId is the path in Dropbox)
    tools::JsonWriter writer;
    writer.put("path", rtl::OUStringToOString(fileId, RTL_TEXTENCODING_UTF8).getStr());
    std::string deleteJson = writer.finishAndGetAsOString().getStr();

    SAL_WARN("ucb.ucp.dropbox", "Making file deletion request to: " + sApiUrl);

    // Use CURL to delete the file
    if (!m_pCurl) {
        return;
    }

    HttpResponse response;
    response.responseCode = 0;

    std::string url = rtl::OUStringToOString(sApiUrl, RTL_TEXTENCODING_UTF8).getStr();

    curl_easy_setopt(m_pCurl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(m_pCurl, CURLOPT_POST, 1L);
    curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDS, deleteJson.c_str());
    curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDSIZE, deleteJson.length());

    // Set headers for JSON content and authorization
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    std::string authHeader = std::string("Authorization: Bearer ") +
        rtl::OUStringToOString(sAccessToken, RTL_TEXTENCODING_UTF8).getStr();
    headers = curl_slist_append(headers, authHeader.c_str());

    curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(m_pCurl);

    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        SAL_WARN("ucb.ucp.dropbox", "CURL error during file deletion: " + OUString::createFromAscii(curl_easy_strerror(res)));
        return;
    }

    curl_easy_getinfo(m_pCurl, CURLINFO_RESPONSE_CODE, &response.responseCode);

    if (response.responseCode >= 200 && response.responseCode < 300) {
        SAL_WARN("ucb.ucp.dropbox", "File deletion successful");
    } else {
        SAL_WARN("ucb.ucp.dropbox", "File deletion failed with HTTP response code: " + OUString::number(response.responseCode));
        SAL_WARN("ucb.ucp.dropbox", "Error response: " + OUString::createFromAscii(response.data.c_str()));
    }
}

rtl::OUString DropboxApiClient::copyFile(const rtl::OUString& fileId, const rtl::OUString& newParentId, const rtl::OUString& newName)
{
    SAL_WARN("ucb.ucp.dropbox", "copyFile() called for file ID: " + fileId + " to parent: " + newParentId + " with name: " + newName);

    if (fileId.isEmpty()) {
        SAL_WARN("ucb.ucp.dropbox", "Cannot copy file with empty ID");
        return rtl::OUString();
    }

    rtl::OUString sAccessToken = getAccessToken();
    if (sAccessToken.isEmpty()) {
        SAL_WARN("ucb.ucp.dropbox", "No access token available for file copy");
        return rtl::OUString();
    }

    // Build Dropbox API URL for copying files
    rtl::OUString sApiUrl = u"https://api.dropboxapi.com/2/files/copy_v2"_ustr;

    // Create JSON metadata for the copy operation (Dropbox format)
    tools::JsonWriter writer;
    writer.put("from_path", rtl::OUStringToOString(fileId, RTL_TEXTENCODING_UTF8).getStr());

    // Build to_path
    rtl::OUString toPath;
    if (newParentId.isEmpty() || newParentId == "root") {
        toPath = "/" + newName;
    } else {
        toPath = newParentId + "/" + newName;
    }
    writer.put("to_path", rtl::OUStringToOString(toPath, RTL_TEXTENCODING_UTF8).getStr());
    writer.put("allow_shared_folder", false);
    writer.put("autorename", false);

    rtl::OUString sJsonMetadata = rtl::OUString::createFromAscii(writer.finishAndGetAsOString().getStr());
    SAL_WARN("ucb.ucp.dropbox", "Copy metadata: " + sJsonMetadata);

    SAL_WARN("ucb.ucp.dropbox", "Making file copy request to: " + sApiUrl);

    rtl::OUString sResponse = sendRequestForString(
        u"POST"_ustr,
        sApiUrl,
        sJsonMetadata
    );

    if (!sResponse.isEmpty()) {
        SAL_WARN("ucb.ucp.dropbox", "File copy successful");
        SAL_WARN("ucb.ucp.dropbox", "Response: " + sResponse.copy(0, std::min(200, (int)sResponse.getLength())));

        // Parse the response to get the new file path (ID in Dropbox)
        try {
            boost::property_tree::ptree root;
            std::string jsonStr = rtl::OUStringToOString(sResponse, RTL_TEXTENCODING_UTF8).getStr();
            std::istringstream jsonStream(jsonStr);
            boost::property_tree::read_json(jsonStream, root);

            std::string newFilePath = root.get<std::string>("metadata.path_lower", "");
            if (!newFilePath.empty()) {
                rtl::OUString sNewFileId = rtl::OUString::createFromAscii(newFilePath.c_str());
                SAL_WARN("ucb.ucp.dropbox", "New file ID (path): " + sNewFileId);
                return sNewFileId;
            }
        } catch (const boost::property_tree::json_parser_error&) {
            SAL_WARN("ucb.ucp.dropbox", "Failed to parse copy response");
        }
    } else {
        SAL_WARN("ucb.ucp.dropbox", "File copy failed - empty response");
    }

    return rtl::OUString();
}

void DropboxApiClient::moveFile(const rtl::OUString& fileId, const rtl::OUString& newParentId, const rtl::OUString& newName)
{
    SAL_WARN("ucb.ucp.dropbox", "moveFile() called for file ID: " + fileId + " to parent: " + newParentId + " with name: " + newName);

    if (fileId.isEmpty()) {
        SAL_WARN("ucb.ucp.dropbox", "Cannot move file with empty ID");
        return;
    }

    rtl::OUString sAccessToken = getAccessToken();
    if (sAccessToken.isEmpty()) {
        SAL_WARN("ucb.ucp.dropbox", "No access token available for file move");
        return;
    }

    // Build Dropbox API URL for moving files
    rtl::OUString sApiUrl = u"https://api.dropboxapi.com/2/files/move_v2"_ustr;

    // Create JSON metadata for the move operation (Dropbox format)
    tools::JsonWriter writer;
    writer.put("from_path", rtl::OUStringToOString(fileId, RTL_TEXTENCODING_UTF8).getStr());

    // Build to_path
    rtl::OUString toPath;
    if (newParentId.isEmpty() || newParentId == "root") {
        toPath = "/" + newName;
    } else {
        toPath = newParentId + "/" + newName;
    }
    writer.put("to_path", rtl::OUStringToOString(toPath, RTL_TEXTENCODING_UTF8).getStr());
    writer.put("allow_shared_folder", false);
    writer.put("autorename", false);

    rtl::OUString sJsonMetadata = rtl::OUString::createFromAscii(writer.finishAndGetAsOString().getStr());
    SAL_WARN("ucb.ucp.dropbox", "Move metadata: " + sJsonMetadata);

    SAL_WARN("ucb.ucp.dropbox", "Making file move request to: " + sApiUrl);

    // Use CURL to move the file
    if (!m_pCurl) {
        return;
    }

    HttpResponse response;
    response.responseCode = 0;

    std::string url = rtl::OUStringToOString(sApiUrl, RTL_TEXTENCODING_UTF8).getStr();
    std::string jsonData = rtl::OUStringToOString(sJsonMetadata, RTL_TEXTENCODING_UTF8).getStr();

    curl_easy_setopt(m_pCurl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(m_pCurl, CURLOPT_POST, 1L);
    curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDS, jsonData.c_str());
    curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDSIZE, jsonData.length());

    // Set headers for JSON content and authorization
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    std::string authHeader = std::string("Authorization: Bearer ") +
        rtl::OUStringToOString(sAccessToken, RTL_TEXTENCODING_UTF8).getStr();
    headers = curl_slist_append(headers, authHeader.c_str());

    curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(m_pCurl);

    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        SAL_WARN("ucb.ucp.dropbox", "CURL error during file move: " + OUString::createFromAscii(curl_easy_strerror(res)));
        return;
    }

    curl_easy_getinfo(m_pCurl, CURLINFO_RESPONSE_CODE, &response.responseCode);

    if (response.responseCode >= 200 && response.responseCode < 300) {
        SAL_WARN("ucb.ucp.dropbox", "File move successful");
        SAL_WARN("ucb.ucp.dropbox", "Response: " + OUString::createFromAscii(response.data.substr(0, 200).c_str()));
    } else {
        SAL_WARN("ucb.ucp.dropbox", "File move failed with HTTP response code: " + OUString::number(response.responseCode));
        SAL_WARN("ucb.ucp.dropbox", "Error response: " + OUString::createFromAscii(response.data.c_str()));
    }
}

rtl::OUString DropboxApiClient::getAccessToken()
{
    SAL_WARN("ucb.ucp.dropbox", "getAccessToken() called, current token length: " + OUString::number(m_sAccessToken.getLength()));

    // If we have a token, check if it's still valid
    if (!m_sAccessToken.isEmpty()) {
        if (isTokenValid()) {
            SAL_WARN("ucb.ucp.dropbox", "Current token is valid");
            return m_sAccessToken;
        } else {
            SAL_WARN("ucb.ucp.dropbox", "Current token is invalid, attempting refresh");
            rtl::OUString sRefreshedToken = refreshAccessToken();
            if (!sRefreshedToken.isEmpty()) {
                SAL_WARN("ucb.ucp.dropbox", "Successfully refreshed token");
                return sRefreshedToken;
            } else {
                SAL_WARN("ucb.ucp.dropbox", "Token refresh failed, will re-authenticate");
                m_sAccessToken.clear(); // Clear invalid token
            }
        }
    }

    // Try to get a new token if we don't have one or refresh failed
    if (m_sAccessToken.isEmpty())
    {
        SAL_WARN("ucb.ucp.dropbox", "Token is empty, starting OAuth2 HTTP callback flow");

        // Create HTTP server to listen for OAuth2 callback
        ucp::dropbox::OAuth2HttpServer httpServer;
        if (!httpServer.start()) {
            SAL_WARN("ucb.ucp.dropbox", "Failed to start HTTP callback server");
            return rtl::OUString();
        }

        try {
            // Construct OAuth2 authorization URL with HTTP callback for Dropbox
            rtl::OUStringBuffer aAuthUrl;
            aAuthUrl.append(rtl::OUString::createFromAscii(DROPBOX_AUTH_URL));
            aAuthUrl.append(u"?response_type=code"_ustr);
            aAuthUrl.append(u"&client_id="_ustr);
            aAuthUrl.append(rtl::OUString::createFromAscii(DROPBOX_CLIENT_ID));
            aAuthUrl.append(u"&redirect_uri="_ustr);
            aAuthUrl.append(rtl::OUString::createFromAscii(DROPBOX_REDIRECT_URI));
            aAuthUrl.append(u"&token_access_type=offline"_ustr);

            rtl::OUString sAuthUrl = aAuthUrl.makeStringAndClear();
            SAL_WARN("ucb.ucp.dropbox", "Starting OAuth2 flow with URL: " + sAuthUrl);

            // Show user a message and open browser
            uno::Reference<task::XInteractionHandler> xIH;
            if (m_xCmdEnv.is()) {
                xIH = m_xCmdEnv->getInteractionHandler();
            }

            if (!xIH.is()) {
                // Fallback: Create interaction handler directly
                uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
                if (xContext.is()) {
                    xIH = uno::Reference<task::XInteractionHandler>(
                        xContext->getServiceManager()->createInstanceWithContext(
                            u"com.sun.star.task.InteractionHandler"_ustr, xContext),
                        uno::UNO_QUERY);
                }
            }

            // Open browser for authentication
            SAL_WARN("ucb.ucp.dropbox", "Opening browser for authentication");
            try {
                uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
                uno::Reference<css::system::XSystemShellExecute> xSystemShellExecute(
                    css::system::SystemShellExecute::create(xContext));

                xSystemShellExecute->execute(sAuthUrl, OUString(),
                    css::system::SystemShellExecuteFlags::URIS_ONLY);

                SAL_WARN("ucb.ucp.dropbox", "Browser opened successfully");
            } catch (const css::uno::Exception& e) {
                SAL_WARN("ucb.ucp.dropbox", "Failed to open browser: " + e.Message);
            }

            // Show notification to user if we have an interaction handler
            if (xIH.is()) {
                rtl::Reference<ucbhelper::AuthenticationFallbackRequest> xRequest
                    = new ucbhelper::AuthenticationFallbackRequest(
                        u"Please sign in to Dropbox in your browser and authorize LibreOffice."_ustr,
                        sAuthUrl);
                xIH->handle(xRequest);
            }

            // Wait for the callback with auth code
            rtl::OUString sAuthCode = httpServer.waitForAuthCode(120); // 2 minute timeout

            if (!sAuthCode.isEmpty()) {
                SAL_WARN("ucb.ucp.dropbox", "Received authorization code via HTTP callback");

                // Exchange auth code for access token
                m_sAccessToken = exchangeCodeForToken(sAuthCode);

                if (!m_sAccessToken.isEmpty()) {
                    SAL_WARN("ucb.ucp.dropbox", "Successfully obtained access token via HTTP callback");
                } else {
                    SAL_WARN("ucb.ucp.dropbox", "Failed to exchange authorization code for access token");
                }
            } else {
                SAL_WARN("ucb.ucp.dropbox", "Failed to receive authorization code via HTTP callback");
            }

        } catch (...) {
            SAL_WARN("ucb.ucp.dropbox", "Exception during OAuth2 HTTP callback flow");
        }

        // Stop the HTTP server
        httpServer.stop();
    }

    SAL_WARN("ucb.ucp.dropbox", "Returning token with length: " + OUString::number(m_sAccessToken.getLength()));
    return m_sAccessToken;
}

rtl::OUString DropboxApiClient::exchangeCodeForToken(const rtl::OUString& sAuthCode)
{
    SAL_WARN("ucb.ucp.dropbox", "exchangeCodeForToken called with code: " + sAuthCode.copy(0, 10) + "...");

    if (sAuthCode.isEmpty())
    {
        SAL_WARN("ucb.ucp.dropbox", "Auth code is empty");
        return rtl::OUString();
    }

    // Prepare token exchange request
    rtl::OUStringBuffer aBody;
    aBody.append(u"code="_ustr);
    aBody.append(sAuthCode);
    aBody.append(u"&client_id="_ustr);
    aBody.append(rtl::OUString::createFromAscii(DROPBOX_CLIENT_ID));
    aBody.append(u"&client_secret="_ustr);
    aBody.append(rtl::OUString::createFromAscii(DROPBOX_CLIENT_SECRET));
    aBody.append(u"&redirect_uri="_ustr);
    aBody.append(rtl::OUString::createFromAscii(DROPBOX_REDIRECT_URI));
    aBody.append(u"&grant_type=authorization_code"_ustr);

    SAL_WARN("ucb.ucp.dropbox", "Sending token request to: " + rtl::OUString::createFromAscii(DROPBOX_TOKEN_URL));

    rtl::OUString sResponse = sendRequestForString(
        u"POST"_ustr,
        rtl::OUString::createFromAscii(DROPBOX_TOKEN_URL),
        aBody.makeStringAndClear()
    );

    SAL_WARN("ucb.ucp.dropbox", "Token response length: " + OUString::number(sResponse.getLength()));
    if (!sResponse.isEmpty()) {
        SAL_WARN("ucb.ucp.dropbox", "Token response: " + sResponse.copy(0, std::min(200, (int)sResponse.getLength())));

        // Use proper JSON helper to parse token response
        auto tokenPair = DropboxJsonHelper::parseTokenResponse(sResponse);
        rtl::OUString sAccessToken = tokenPair.first;
        rtl::OUString sRefreshToken = tokenPair.second;

        if (!sAccessToken.isEmpty()) {
            SAL_WARN("ucb.ucp.dropbox", "Extracted access token: " + sAccessToken.copy(0, 20) + "...");

            // Store refresh token if available
            if (!sRefreshToken.isEmpty()) {
                m_sRefreshToken = sRefreshToken;
                SAL_WARN("ucb.ucp.dropbox", "Stored refresh token: " + sRefreshToken.copy(0, 20) + "...");
            }

            return sAccessToken;
        } else {
            SAL_WARN("ucb.ucp.dropbox", "Failed to parse access_token from response");
            return rtl::OUString();
        }
    }

    SAL_WARN("ucb.ucp.dropbox", "Empty response from token exchange");
    return rtl::OUString();
}

rtl::OUString DropboxApiClient::refreshAccessToken()
{
    SAL_WARN("ucb.ucp.dropbox", "refreshAccessToken called");

    if (m_sRefreshToken.isEmpty()) {
        SAL_WARN("ucb.ucp.dropbox", "No refresh token available");
        return rtl::OUString();
    }

    // Prepare refresh token request
    rtl::OUStringBuffer aBody;
    aBody.append(u"refresh_token="_ustr);
    aBody.append(m_sRefreshToken);
    aBody.append(u"&client_id="_ustr);
    aBody.append(rtl::OUString::createFromAscii(DROPBOX_CLIENT_ID));
    aBody.append(u"&client_secret="_ustr);
    aBody.append(rtl::OUString::createFromAscii(DROPBOX_CLIENT_SECRET));
    aBody.append(u"&grant_type=refresh_token"_ustr);

    SAL_WARN("ucb.ucp.dropbox", "Sending refresh token request");

    rtl::OUString sResponse = sendRequestForString(
        u"POST"_ustr,
        rtl::OUString::createFromAscii(DROPBOX_TOKEN_URL),
        aBody.makeStringAndClear()
    );

    if (!sResponse.isEmpty()) {
        auto tokenPair = DropboxJsonHelper::parseTokenResponse(sResponse);
        rtl::OUString sNewAccessToken = tokenPair.first;

        if (!sNewAccessToken.isEmpty()) {
            m_sAccessToken = sNewAccessToken;
            SAL_WARN("ucb.ucp.dropbox", "Successfully refreshed access token");
            return sNewAccessToken;
        }
    }

    SAL_WARN("ucb.ucp.dropbox", "Failed to refresh access token");
    return rtl::OUString();
}

bool DropboxApiClient::isTokenValid()
{
    if (m_sAccessToken.isEmpty()) {
        return false;
    }

    try {
        // Test token validity with a simple API call
        rtl::OUString sResponse = sendRequestForString(
            u"POST"_ustr,
            u"https://api.dropboxapi.com/2/check/user"_ustr,
            u"null"_ustr
        );
        return !sResponse.isEmpty();
    } catch (...) {
        return false;
    }
}

rtl::OUString DropboxApiClient::authenticate()
{
    SAL_WARN("ucb.ucp.dropbox", "authenticate() called");
    return getAccessToken();
}

rtl::OUString DropboxApiClient::getCurrentAccessToken()
{
    SAL_WARN("ucb.ucp.dropbox", "getCurrentAccessToken() called, returning current token");
    return m_sAccessToken;
}

size_t DropboxApiClient::WriteCallback(void* contents, size_t size, size_t nmemb, HttpResponse* response)
{
    size_t totalSize = size * nmemb;
    response->data.append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

rtl::OUString DropboxApiClient::sendRequestForString(const rtl::OUString& sMethod, const rtl::OUString& sUrl, const rtl::OUString& sBody)
{
    return sendRequestForStringWithRetry(sMethod, sUrl, sBody, 3);
}

rtl::OUString DropboxApiClient::sendRequestForStringWithRetry(const rtl::OUString& sMethod, const rtl::OUString& sUrl, const rtl::OUString& sBody, sal_Int32 maxRetries)
{
    if (!m_pCurl) {
        SAL_WARN("ucb.ucp.dropbox", "sendRequestForString: CURL not initialized");
        return rtl::OUString();
    }

    // Convert OUString to std::string for CURL
    std::string url = rtl::OUStringToOString(sUrl, RTL_TEXTENCODING_UTF8).getStr();
    std::string method = rtl::OUStringToOString(sMethod, RTL_TEXTENCODING_UTF8).getStr();
    std::string body = rtl::OUStringToOString(sBody, RTL_TEXTENCODING_UTF8).getStr();

    sal_Int32 attemptCount = 0;

    while (attemptCount <= maxRetries) {
        attemptCount++;

        if (attemptCount > 1) {
            SAL_WARN("ucb.ucp.dropbox", "sendRequestForString: Retry attempt " + OUString::number(attemptCount) + " of " + OUString::number(maxRetries + 1));

            // Exponential backoff: wait 1s, 2s, 4s...
            sal_Int32 waitSeconds = 1 << (attemptCount - 2); // 2^(attempt-2)
            SAL_WARN("ucb.ucp.dropbox", "Waiting " + OUString::number(waitSeconds) + " seconds before retry");

            // Simple sleep implementation
            for (sal_Int32 i = 0; i < waitSeconds; i++) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }

        SAL_WARN("ucb.ucp.dropbox", "sendRequestForString: " + sMethod + " " + sUrl + " (attempt " + OUString::number(attemptCount) + ")");

        HttpResponse response;
        response.responseCode = 0;

        // Reset CURL state for clean request
        curl_easy_reset(m_pCurl);

        curl_easy_setopt(m_pCurl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, &response);

        // Enable following redirects
        curl_easy_setopt(m_pCurl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(m_pCurl, CURLOPT_MAXREDIRS, 5L);

        // Set timeout
        curl_easy_setopt(m_pCurl, CURLOPT_TIMEOUT, 30L);

        // Verify SSL certificates
        curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYHOST, 2L);

        // Set method and headers
        struct curl_slist* headers = nullptr;

        if (method == "POST") {
            curl_easy_setopt(m_pCurl, CURLOPT_POST, 1L);
            if (!body.empty()) {
                curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDS, body.c_str());
                curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDSIZE, body.length());

                // Determine content type based on body content
                if (body.find('{') != std::string::npos || body.find('[') != std::string::npos ||
                    body == "null" || body.find("\"") != std::string::npos ||
                    url.find("api.dropboxapi.com") != std::string::npos) {
                    // Looks like JSON or is a Dropbox API call
                    headers = curl_slist_append(headers, "Content-Type: application/json");
                } else {
                    // Assume form data
                    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
                }
            }
        } else if (method == "GET") {
            curl_easy_setopt(m_pCurl, CURLOPT_HTTPGET, 1L);
        }

        // Add authorization header
        if (url.find("oauth2/token") != std::string::npos) {
            // For Dropbox token requests, do NOT add Authorization header
            // Dropbox expects client_secret in the form body, not in Basic Auth
            SAL_WARN("ucb.ucp.dropbox", "Token request - using form data authentication (no Authorization header)");
        } else if (!m_sAccessToken.isEmpty()) {
            // For API requests, use Bearer token
            std::string authHeader = std::string("Authorization: Bearer ") +
                rtl::OUStringToOString(m_sAccessToken, RTL_TEXTENCODING_UTF8).getStr();
            headers = curl_slist_append(headers, authHeader.c_str());
        }

        if (headers) {
            curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, headers);
        }

        CURLcode res = curl_easy_perform(m_pCurl);

        if (headers) {
            curl_slist_free_all(headers);
        }

        bool shouldRetry = false;

        if (res != CURLE_OK) {
            SAL_WARN("ucb.ucp.dropbox", "sendRequestForString: CURL error: " +
                     OUString::createFromAscii(curl_easy_strerror(res)));

            // Retry on network errors
            shouldRetry = (res == CURLE_COULDNT_CONNECT ||
                          res == CURLE_OPERATION_TIMEDOUT ||
                          res == CURLE_COULDNT_RESOLVE_HOST ||
                          res == CURLE_RECV_ERROR ||
                          res == CURLE_SEND_ERROR);
        } else {
            curl_easy_getinfo(m_pCurl, CURLINFO_RESPONSE_CODE, &response.responseCode);

            SAL_WARN("ucb.ucp.dropbox", "sendRequestForString: HTTP response code: " +
                     OUString::number(response.responseCode));

            if (response.responseCode >= 200 && response.responseCode < 300) {
                SAL_WARN("ucb.ucp.dropbox", "sendRequestForString: Success, received " +
                         OUString::number(response.data.length()) + " bytes");
                return rtl::OUString::createFromAscii(response.data.c_str());
            } else if (response.responseCode == 401) {
                SAL_WARN("ucb.ucp.dropbox", "sendRequestForString: Authentication failed (401)");
                // Clear stored access token so it will be refreshed on next request
                m_sAccessToken = rtl::OUString();
                shouldRetry = true; // Retry with fresh token
            } else if (response.responseCode == 429 || response.responseCode >= 500) {
                // Retry on rate limiting or server errors
                shouldRetry = true;
                SAL_WARN("ucb.ucp.dropbox", "sendRequestForString: Retryable error (" +
                         OUString::number(response.responseCode) + ")");
            } else if (response.responseCode == 403) {
                SAL_WARN("ucb.ucp.dropbox", "sendRequestForString: Access forbidden (403) - insufficient permissions");
            } else if (response.responseCode == 404) {
                SAL_WARN("ucb.ucp.dropbox", "sendRequestForString: Resource not found (404)");
            } else if (response.responseCode >= 400 && response.responseCode < 500) {
                SAL_WARN("ucb.ucp.dropbox", "sendRequestForString: Client error (" +
                         OUString::number(response.responseCode) + ")");
            }

            // Log error response body if available
            if (!response.data.empty()) {
                SAL_WARN("ucb.ucp.dropbox", "sendRequestForString: Error response: " +
                         OUString::createFromAscii(response.data.substr(0, 500).c_str()));
            }
        }

        // If this is the last attempt or we shouldn't retry, return empty
        if (!shouldRetry || attemptCount > maxRetries) {
            return rtl::OUString();
        }
    }

    // Should never reach here
    return rtl::OUString();
}

} // namespace dropbox
} // namespace ucp