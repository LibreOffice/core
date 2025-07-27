/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "GoogleDriveApiClient.hxx"
#include "gdrive_json.hxx"
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

namespace ucp {
namespace gdrive {

GoogleDriveApiClient::GoogleDriveApiClient(const uno::Reference<ucb::XCommandEnvironment>& xCmdEnv)
    : m_xCmdEnv(xCmdEnv)
    , m_sAccessToken()
    , m_pCurl(nullptr)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    m_pCurl = curl_easy_init();
}

GoogleDriveApiClient::~GoogleDriveApiClient()
{
    if (m_pCurl) {
        curl_easy_cleanup(m_pCurl);
    }
    curl_global_cleanup();
}

std::vector<GDriveFileInfo> GoogleDriveApiClient::listFolder(const rtl::OUString& folderId)
{
    SAL_WARN("ucb.ucp.gdrive", "listFolder() called for folder ID: " + folderId);

    // TEST: Show a simple message to confirm this code is being reached
    if (m_xCmdEnv.is()) {
        uno::Reference<task::XInteractionHandler> xIH = m_xCmdEnv->getInteractionHandler();
        if (xIH.is()) {
            rtl::Reference<ucbhelper::AuthenticationFallbackRequest> xRequest
                = new ucbhelper::AuthenticationFallbackRequest(
                    u"DEBUG: Google Drive listFolder() method was called! Click OK to continue."_ustr,
                    u"test://debug"_ustr);
            xIH->handle(xRequest);
        }
    }

    std::vector<GDriveFileInfo> aFiles;

    rtl::OUString sAccessToken = getAccessToken();
    SAL_WARN("ucb.ucp.gdrive", "Got access token length: " + OUString::number(sAccessToken.getLength()));
    if (sAccessToken.isEmpty()) {
        SAL_WARN("ucb.ucp.gdrive", "No access token available - returning empty list");
        return aFiles;
    }

    SAL_WARN("ucb.ucp.gdrive", "Access token preview: " + sAccessToken.copy(0, std::min(30, (int)sAccessToken.getLength())));

    // Build Google Drive API URL for listing files with required fields
    rtl::OUStringBuffer sUrl;
    sUrl.append(u"https://www.googleapis.com/drive/v3/files"_ustr);
    sUrl.append(u"?fields=files(id,name,mimeType,size,modifiedTime)"_ustr);
    if (!folderId.isEmpty() && folderId != u"root"_ustr) {
        sUrl.append(u"&q='"_ustr);
        sUrl.append(folderId);
        sUrl.append(u"'+in+parents"_ustr);
    }

    rtl::OUString sApiUrl = sUrl.makeStringAndClear();
    SAL_WARN("ucb.ucp.gdrive", "Making API request to: " + sApiUrl);

    rtl::OUString sResponse = sendRequestForString(
        u"GET"_ustr,
        sApiUrl,
        rtl::OUString()
    );

    SAL_WARN("ucb.ucp.gdrive", "API response length: " + OUString::number(sResponse.getLength()));

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
    } catch (...) {}

    // Parse JSON response using proper JSON helper
    if (!sResponse.isEmpty()) {
        SAL_WARN("ucb.ucp.gdrive", "Parsing JSON response with " + OUString::number(sResponse.getLength()) + " chars");

        aFiles = GDriveJsonHelper::parseFolderListing(sResponse);

        SAL_WARN("ucb.ucp.gdrive", "Successfully parsed " + OUString::number(aFiles.size()) + " files from JSON");

        // Log parsed files for debugging
        for (const auto& fileInfo : aFiles) {
            SAL_WARN("ucb.ucp.gdrive", "Parsed file: " + fileInfo.name + " (id: " + fileInfo.id +
                     ", folder: " + (fileInfo.isFolder ? u"yes" : u"no") + ")");
        }
    }

    return aFiles;
}

std::vector<GDriveFileInfo> GoogleDriveApiClient::listFolderComplete(const rtl::OUString& folderId, sal_Int32 maxFiles)
{
    SAL_WARN("ucb.ucp.gdrive", "listFolderComplete() called for folder ID: " + folderId + " (max files: " + OUString::number(maxFiles) + ")");

    std::vector<GDriveFileInfo> allFiles;
    rtl::OUString nextPageToken;
    sal_Int32 totalFiles = 0;
    sal_Int32 pageCount = 0;

    rtl::OUString sAccessToken = getAccessToken();
    if (sAccessToken.isEmpty()) {
        SAL_WARN("ucb.ucp.gdrive", "No access token available - returning empty list");
        return allFiles;
    }

    do {
        pageCount++;
        rtl::OUString tokenDisplay = nextPageToken.isEmpty() ?
            u"(none)"_ustr :
            (nextPageToken.copy(0, 20) + u"..."_ustr);
        SAL_WARN("ucb.ucp.gdrive", "Fetching page " << pageCount << " with token: " << tokenDisplay);

        // Build Google Drive API URL for listing files with pagination
        rtl::OUStringBuffer sUrl;
        sUrl.append(u"https://www.googleapis.com/drive/v3/files"_ustr);
        sUrl.append(u"?fields=files(id,name,mimeType,size,modifiedTime),nextPageToken"_ustr);
        sUrl.append(u"&pageSize=100"_ustr); // Request 100 items per page

        if (!folderId.isEmpty() && folderId != u"root"_ustr) {
            sUrl.append(u"&q='"_ustr);
            sUrl.append(folderId);
            sUrl.append(u"'+in+parents"_ustr);
        }

        if (!nextPageToken.isEmpty()) {
            sUrl.append(u"&pageToken="_ustr);
            sUrl.append(nextPageToken);
        }

        rtl::OUString sApiUrl = sUrl.makeStringAndClear();
        SAL_WARN("ucb.ucp.gdrive", "Making paginated API request to: " + sApiUrl);

        rtl::OUString sResponse = sendRequestForString(
            u"GET"_ustr,
            sApiUrl,
            rtl::OUString()
        );

        if (!sResponse.isEmpty()) {
            GDriveFolderListing listing = GDriveJsonHelper::parseFolderListingWithPagination(sResponse);

            SAL_WARN("ucb.ucp.gdrive", "Page " + OUString::number(pageCount) + " returned " +
                     OUString::number(listing.files.size()) + " files, hasMore: " +
                     (listing.hasMore ? u"yes" : u"no"));

            // Add files from this page
            for (const auto& fileInfo : listing.files) {
                if (totalFiles >= maxFiles) {
                    SAL_WARN("ucb.ucp.gdrive", "Reached maximum file limit of " + OUString::number(maxFiles));
                    return allFiles;
                }
                allFiles.push_back(fileInfo);
                totalFiles++;
            }

            // Update pagination token
            nextPageToken = listing.nextPageToken;

            // Break if no more pages or we hit a reasonable limit
            if (!listing.hasMore || pageCount >= 20) { // Safety limit of 20 pages (2000 files max)
                break;
            }
        } else {
            SAL_WARN("ucb.ucp.gdrive", "Empty response from API, stopping pagination");
            break;
        }

    } while (!nextPageToken.isEmpty() && totalFiles < maxFiles);

    SAL_WARN("ucb.ucp.gdrive", "listFolderComplete finished: " + OUString::number(totalFiles) +
             " total files in " + OUString::number(pageCount) + " pages");

    return allFiles;
}

GDriveFileInfo GoogleDriveApiClient::getFileInfo(const rtl::OUString& fileId)
{
    SAL_WARN("ucb.ucp.gdrive", "getFileInfo() called for file ID: " + fileId);

    GDriveFileInfo aFileInfo;

    if (fileId.isEmpty()) {
        return aFileInfo;
    }

    rtl::OUString sAccessToken = getAccessToken();
    if (sAccessToken.isEmpty()) {
        SAL_WARN("ucb.ucp.gdrive", "No access token available for file info");
        return aFileInfo;
    }

    // Build Google Drive API URL for getting single file info
    rtl::OUStringBuffer sUrl;
    sUrl.append(u"https://www.googleapis.com/drive/v3/files/"_ustr);
    sUrl.append(fileId);
    sUrl.append(u"?fields=id,name,mimeType,size,modifiedTime"_ustr);

    rtl::OUString sApiUrl = sUrl.makeStringAndClear();
    SAL_WARN("ucb.ucp.gdrive", "Making file info request to: " + sApiUrl);

    rtl::OUString sResponse = sendRequestForString(
        u"GET"_ustr,
        sApiUrl,
        rtl::OUString()
    );

    SAL_WARN("ucb.ucp.gdrive", "File info response length: " + OUString::number(sResponse.getLength()));

    if (!sResponse.isEmpty()) {
        SAL_WARN("ucb.ucp.gdrive", "Parsing file info JSON response");

        try {
            boost::property_tree::ptree root;
            std::string jsonStr = rtl::OUStringToOString(sResponse, RTL_TEXTENCODING_UTF8).getStr();
            std::istringstream jsonStream(jsonStr);
            boost::property_tree::read_json(jsonStream, root);

            aFileInfo.id = rtl::OUString::createFromAscii(root.get<std::string>("id", "").c_str());
            aFileInfo.name = rtl::OUString::createFromAscii(root.get<std::string>("name", "").c_str());
            aFileInfo.mimeType = rtl::OUString::createFromAscii(root.get<std::string>("mimeType", "").c_str());
            aFileInfo.size = rtl::OUString::createFromAscii(root.get<std::string>("size", "").c_str());
            aFileInfo.modifiedTime = rtl::OUString::createFromAscii(root.get<std::string>("modifiedTime", "").c_str());
            aFileInfo.isFolder = (aFileInfo.mimeType == "application/vnd.google-apps.folder");

            SAL_WARN("ucb.ucp.gdrive", "Successfully parsed file info: " + aFileInfo.name +
                     " (folder: " + (aFileInfo.isFolder ? u"yes" : u"no") + ")");
        } catch (const boost::property_tree::json_parser_error&) {
            SAL_WARN("ucb.ucp.gdrive", "JSON parsing failed for file info");
        } catch (const boost::property_tree::ptree_bad_path&) {
            SAL_WARN("ucb.ucp.gdrive", "Missing expected fields in file info response");
        }
    }

    return aFileInfo;
}

uno::Reference<io::XInputStream> GoogleDriveApiClient::downloadFile(const rtl::OUString& fileId)
{
    SAL_WARN("ucb.ucp.gdrive", "downloadFile() called for file ID: " + fileId);

    if (fileId.isEmpty()) {
        return nullptr;
    }

    rtl::OUString sAccessToken = getAccessToken();
    if (sAccessToken.isEmpty()) {
        SAL_WARN("ucb.ucp.gdrive", "No access token available for download");
        return nullptr;
    }

    // Build Google Drive API URL for downloading file content
    rtl::OUStringBuffer sUrl;
    sUrl.append(u"https://www.googleapis.com/drive/v3/files/"_ustr);
    sUrl.append(fileId);
    sUrl.append(u"?alt=media"_ustr);

    rtl::OUString sApiUrl = sUrl.makeStringAndClear();
    SAL_WARN("ucb.ucp.gdrive", "Making download request to: " + sApiUrl);

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
    curl_easy_setopt(m_pCurl, CURLOPT_HTTPGET, 1L);

    // Add authorization header
    struct curl_slist* headers = nullptr;
    std::string authHeader = std::string("Authorization: Bearer ") +
        rtl::OUStringToOString(sAccessToken, RTL_TEXTENCODING_UTF8).getStr();
    headers = curl_slist_append(headers, authHeader.c_str());

    curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(m_pCurl);

    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        SAL_WARN("ucb.ucp.gdrive", "CURL error during download: " + OUString::createFromAscii(curl_easy_strerror(res)));
        return nullptr;
    }

    curl_easy_getinfo(m_pCurl, CURLINFO_RESPONSE_CODE, &response.responseCode);

    if (response.responseCode >= 200 && response.responseCode < 300) {
        SAL_WARN("ucb.ucp.gdrive", "Download successful, received " << response.data.length() << " bytes");

        // Create input stream from downloaded data
        uno::Sequence<sal_Int8> aData(response.data.length());
        std::memcpy(aData.getArray(), response.data.c_str(), response.data.length());

        uno::Reference<io::XInputStream> xInputStream(
            io::SequenceInputStream::createStreamFromSequence(
                comphelper::getProcessComponentContext(), aData),
            uno::UNO_QUERY);

        return xInputStream;
    } else {
        SAL_WARN("ucb.ucp.gdrive", "Download failed with HTTP response code: " << response.responseCode);
        return nullptr;
    }
}

void GoogleDriveApiClient::uploadFile(const rtl::OUString& parentId, const rtl::OUString& fileName, const uno::Reference<io::XInputStream>& xInputStream)
{
    SAL_WARN("ucb.ucp.gdrive", "uploadFile() called: '" + fileName + "' to parent: " + parentId);

    if (fileName.isEmpty() || !xInputStream.is()) {
        SAL_WARN("ucb.ucp.gdrive", "Cannot upload file: invalid parameters");
        return;
    }

    rtl::OUString sAccessToken = getAccessToken();
    if (sAccessToken.isEmpty()) {
        SAL_WARN("ucb.ucp.gdrive", "No access token available for file upload");
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
        SAL_WARN("ucb.ucp.gdrive", "Error reading file content: " + e.Message);
        return;
    }

    SAL_WARN("ucb.ucp.gdrive", "Read " + OUString::number(fileContent.length()) + " bytes from input stream");

    // Create JSON metadata for the new file
    rtl::OUString sJsonMetadata = GDriveJsonHelper::createFileMetadata(fileName, parentId);
    std::string metadata = rtl::OUStringToOString(sJsonMetadata, RTL_TEXTENCODING_UTF8).getStr();

    // Create multipart form data
    std::string boundary = "----formdata-boundary-" + std::to_string(rand());
    std::string multipartData;

    // Add metadata part
    multipartData += "--" + boundary + "\r\n";
    multipartData += "Content-Type: application/json; charset=UTF-8\r\n\r\n";
    multipartData += metadata + "\r\n";

    // Add file content part
    multipartData += "--" + boundary + "\r\n";
    multipartData += "Content-Type: application/octet-stream\r\n\r\n";
    multipartData += fileContent + "\r\n";
    multipartData += "--" + boundary + "--\r\n";

    // Build Google Drive API URL for uploading files
    rtl::OUString sApiUrl = u"https://www.googleapis.com/upload/drive/v3/files?uploadType=multipart"_ustr;

    SAL_WARN("ucb.ucp.gdrive", "Making file upload request to: " + sApiUrl);

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
    curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDS, multipartData.c_str());
    curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDSIZE, multipartData.length());

    // Set headers for multipart content and authorization
    struct curl_slist* headers = nullptr;
    std::string contentType = "Content-Type: multipart/related; boundary=" + boundary;
    headers = curl_slist_append(headers, contentType.c_str());

    std::string authHeader = std::string("Authorization: Bearer ") +
        rtl::OUStringToOString(sAccessToken, RTL_TEXTENCODING_UTF8).getStr();
    headers = curl_slist_append(headers, authHeader.c_str());

    curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(m_pCurl);

    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        SAL_WARN("ucb.ucp.gdrive", "CURL error during file upload: " + OUString::createFromAscii(curl_easy_strerror(res)));
        return;
    }

    curl_easy_getinfo(m_pCurl, CURLINFO_RESPONSE_CODE, &response.responseCode);

    if (response.responseCode >= 200 && response.responseCode < 300) {
        SAL_WARN("ucb.ucp.gdrive", "File upload successful");
        SAL_WARN("ucb.ucp.gdrive", "Response: " + OUString::createFromAscii(response.data.substr(0, 200).c_str()));
    } else {
        SAL_WARN("ucb.ucp.gdrive", "File upload failed with HTTP response code: " + OUString::number(response.responseCode));
        SAL_WARN("ucb.ucp.gdrive", "Error response: " + OUString::createFromAscii(response.data.c_str()));
    }
}

void GoogleDriveApiClient::updateFile(const rtl::OUString& fileId, const uno::Reference<io::XInputStream>& xInputStream)
{
    SAL_WARN("ucb.ucp.gdrive", "updateFile() called for file ID: " + fileId);

    if (fileId.isEmpty() || !xInputStream.is()) {
        SAL_WARN("ucb.ucp.gdrive", "Cannot update file: invalid parameters");
        return;
    }

    rtl::OUString sAccessToken = getAccessToken();
    if (sAccessToken.isEmpty()) {
        SAL_WARN("ucb.ucp.gdrive", "No access token available for file update");
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
        SAL_WARN("ucb.ucp.gdrive", "Error reading file content: " + e.Message);
        return;
    }

    SAL_WARN("ucb.ucp.gdrive", "Read " + OUString::number(fileContent.length()) + " bytes from input stream");

    // Build Google Drive API URL for updating files using media upload
    rtl::OUStringBuffer sUrl;
    sUrl.append(u"https://www.googleapis.com/upload/drive/v3/files/"_ustr);
    sUrl.append(fileId);
    sUrl.append(u"?uploadType=media"_ustr);

    rtl::OUString sApiUrl = sUrl.makeStringAndClear();

    SAL_WARN("ucb.ucp.gdrive", "Making file update request to: " + sApiUrl);

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
    curl_easy_setopt(m_pCurl, CURLOPT_CUSTOMREQUEST, "PATCH");
    curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDS, fileContent.c_str());
    curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDSIZE, fileContent.length());

    // Set headers for binary content and authorization
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/octet-stream");

    std::string authHeader = std::string("Authorization: Bearer ") +
        rtl::OUStringToOString(sAccessToken, RTL_TEXTENCODING_UTF8).getStr();
    headers = curl_slist_append(headers, authHeader.c_str());

    curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(m_pCurl);

    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        SAL_WARN("ucb.ucp.gdrive", "CURL error during file update: " + OUString::createFromAscii(curl_easy_strerror(res)));
        return;
    }

    curl_easy_getinfo(m_pCurl, CURLINFO_RESPONSE_CODE, &response.responseCode);

    if (response.responseCode >= 200 && response.responseCode < 300) {
        SAL_WARN("ucb.ucp.gdrive", "File update successful");
        SAL_WARN("ucb.ucp.gdrive", "Response: " + OUString::createFromAscii(response.data.substr(0, 200).c_str()));
    } else {
        SAL_WARN("ucb.ucp.gdrive", "File update failed with HTTP response code: " + OUString::number(response.responseCode));
        SAL_WARN("ucb.ucp.gdrive", "Error response: " + OUString::createFromAscii(response.data.c_str()));
    }
}

void GoogleDriveApiClient::createFolder(const rtl::OUString& parentId, const rtl::OUString& folderName)
{
    SAL_WARN("ucb.ucp.gdrive", "createFolder() called: '" + folderName + "' in parent: " + parentId);

    if (folderName.isEmpty()) {
        SAL_WARN("ucb.ucp.gdrive", "Cannot create folder with empty name");
        return;
    }

    rtl::OUString sAccessToken = getAccessToken();
    if (sAccessToken.isEmpty()) {
        SAL_WARN("ucb.ucp.gdrive", "No access token available for folder creation");
        return;
    }

    // Create JSON metadata for the new folder
    rtl::OUString sJsonMetadata = GDriveJsonHelper::createFolderMetadata(folderName, parentId);
    SAL_WARN("ucb.ucp.gdrive", "Folder metadata: " + sJsonMetadata);

    // Build Google Drive API URL for creating files/folders
    rtl::OUString sApiUrl = u"https://www.googleapis.com/drive/v3/files"_ustr;

    SAL_WARN("ucb.ucp.gdrive", "Making folder creation request to: " + sApiUrl);

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
        SAL_WARN("ucb.ucp.gdrive", "CURL error during folder creation: " + OUString::createFromAscii(curl_easy_strerror(res)));
        return;
    }

    curl_easy_getinfo(m_pCurl, CURLINFO_RESPONSE_CODE, &response.responseCode);

    if (response.responseCode >= 200 && response.responseCode < 300) {
        SAL_WARN("ucb.ucp.gdrive", "Folder creation successful");
        SAL_WARN("ucb.ucp.gdrive", "Response: " + OUString::createFromAscii(response.data.substr(0, 200).c_str()));
    } else {
        SAL_WARN("ucb.ucp.gdrive", "Folder creation failed with HTTP response code: " + OUString::number(response.responseCode));
        SAL_WARN("ucb.ucp.gdrive", "Error response: " + OUString::createFromAscii(response.data.c_str()));
    }
}

void GoogleDriveApiClient::deleteFile(const rtl::OUString& fileId)
{
    SAL_WARN("ucb.ucp.gdrive", "deleteFile() called for file ID: " + fileId);

    if (fileId.isEmpty()) {
        SAL_WARN("ucb.ucp.gdrive", "Cannot delete file with empty ID");
        return;
    }

    rtl::OUString sAccessToken = getAccessToken();
    if (sAccessToken.isEmpty()) {
        SAL_WARN("ucb.ucp.gdrive", "No access token available for file deletion");
        return;
    }

    // Build Google Drive API URL for deleting files
    rtl::OUStringBuffer sUrl;
    sUrl.append(u"https://www.googleapis.com/drive/v3/files/"_ustr);
    sUrl.append(fileId);

    rtl::OUString sApiUrl = sUrl.makeStringAndClear();

    SAL_WARN("ucb.ucp.gdrive", "Making file deletion request to: " + sApiUrl);

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
    curl_easy_setopt(m_pCurl, CURLOPT_CUSTOMREQUEST, "DELETE");

    // Set headers for authorization
    struct curl_slist* headers = nullptr;
    std::string authHeader = std::string("Authorization: Bearer ") +
        rtl::OUStringToOString(sAccessToken, RTL_TEXTENCODING_UTF8).getStr();
    headers = curl_slist_append(headers, authHeader.c_str());

    curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(m_pCurl);

    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        SAL_WARN("ucb.ucp.gdrive", "CURL error during file deletion: " + OUString::createFromAscii(curl_easy_strerror(res)));
        return;
    }

    curl_easy_getinfo(m_pCurl, CURLINFO_RESPONSE_CODE, &response.responseCode);

    if (response.responseCode >= 200 && response.responseCode < 300) {
        SAL_WARN("ucb.ucp.gdrive", "File deletion successful");
    } else {
        SAL_WARN("ucb.ucp.gdrive", "File deletion failed with HTTP response code: " + OUString::number(response.responseCode));
        SAL_WARN("ucb.ucp.gdrive", "Error response: " + OUString::createFromAscii(response.data.c_str()));
    }
}

rtl::OUString GoogleDriveApiClient::copyFile(const rtl::OUString& fileId, const rtl::OUString& newParentId, const rtl::OUString& newName)
{
    SAL_WARN("ucb.ucp.gdrive", "copyFile() called for file ID: " + fileId + " to parent: " + newParentId + " with name: " + newName);

    if (fileId.isEmpty()) {
        SAL_WARN("ucb.ucp.gdrive", "Cannot copy file with empty ID");
        return rtl::OUString();
    }

    rtl::OUString sAccessToken = getAccessToken();
    if (sAccessToken.isEmpty()) {
        SAL_WARN("ucb.ucp.gdrive", "No access token available for file copy");
        return rtl::OUString();
    }

    // Create JSON metadata for the copy operation
    rtl::OUString sJsonMetadata = GDriveJsonHelper::createCopyMetadata(newName, newParentId);
    SAL_WARN("ucb.ucp.gdrive", "Copy metadata: " + sJsonMetadata);

    // Build Google Drive API URL for copying files
    rtl::OUStringBuffer sUrl;
    sUrl.append(u"https://www.googleapis.com/drive/v3/files/"_ustr);
    sUrl.append(fileId);
    sUrl.append(u"/copy"_ustr);

    rtl::OUString sApiUrl = sUrl.makeStringAndClear();

    SAL_WARN("ucb.ucp.gdrive", "Making file copy request to: " + sApiUrl);

    rtl::OUString sResponse = sendRequestForString(
        u"POST"_ustr,
        sApiUrl,
        sJsonMetadata
    );

    if (!sResponse.isEmpty()) {
        SAL_WARN("ucb.ucp.gdrive", "File copy successful");
        SAL_WARN("ucb.ucp.gdrive", "Response: " + sResponse.copy(0, std::min(200, (int)sResponse.getLength())));

        // Parse the response to get the new file ID
        try {
            boost::property_tree::ptree root;
            std::string jsonStr = rtl::OUStringToOString(sResponse, RTL_TEXTENCODING_UTF8).getStr();
            std::istringstream jsonStream(jsonStr);
            boost::property_tree::read_json(jsonStream, root);

            std::string newFileId = root.get<std::string>("id", "");
            if (!newFileId.empty()) {
                rtl::OUString sNewFileId = rtl::OUString::createFromAscii(newFileId.c_str());
                SAL_WARN("ucb.ucp.gdrive", "New file ID: " + sNewFileId);
                return sNewFileId;
            }
        } catch (const boost::property_tree::json_parser_error&) {
            SAL_WARN("ucb.ucp.gdrive", "Failed to parse copy response");
        }
    } else {
        SAL_WARN("ucb.ucp.gdrive", "File copy failed - empty response");
    }

    return rtl::OUString();
}

void GoogleDriveApiClient::moveFile(const rtl::OUString& fileId, const rtl::OUString& newParentId, const rtl::OUString& newName)
{
    SAL_WARN("ucb.ucp.gdrive", "moveFile() called for file ID: " + fileId + " to parent: " + newParentId + " with name: " + newName);

    if (fileId.isEmpty()) {
        SAL_WARN("ucb.ucp.gdrive", "Cannot move file with empty ID");
        return;
    }

    rtl::OUString sAccessToken = getAccessToken();
    if (sAccessToken.isEmpty()) {
        SAL_WARN("ucb.ucp.gdrive", "No access token available for file move");
        return;
    }

    // For moving, we need to first get the current parent to remove it,
    // then update with new parent and optionally new name
    GDriveFileInfo currentInfo = getFileInfo(fileId);
    if (currentInfo.id.isEmpty()) {
        SAL_WARN("ucb.ucp.gdrive", "Cannot get current file info for move operation");
        return;
    }

    // Create JSON metadata for the move operation
    rtl::OUString sJsonMetadata = GDriveJsonHelper::createMoveMetadata(newName, newParentId);
    SAL_WARN("ucb.ucp.gdrive", "Move metadata: " + sJsonMetadata);

    // Build Google Drive API URL for updating file (which includes moving)
    rtl::OUStringBuffer sUrl;
    sUrl.append(u"https://www.googleapis.com/drive/v3/files/"_ustr);
    sUrl.append(fileId);

    rtl::OUString sApiUrl = sUrl.makeStringAndClear();

    SAL_WARN("ucb.ucp.gdrive", "Making file move request to: " + sApiUrl);

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
    curl_easy_setopt(m_pCurl, CURLOPT_CUSTOMREQUEST, "PATCH");
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
        SAL_WARN("ucb.ucp.gdrive", "CURL error during file move: " + OUString::createFromAscii(curl_easy_strerror(res)));
        return;
    }

    curl_easy_getinfo(m_pCurl, CURLINFO_RESPONSE_CODE, &response.responseCode);

    if (response.responseCode >= 200 && response.responseCode < 300) {
        SAL_WARN("ucb.ucp.gdrive", "File move successful");
        SAL_WARN("ucb.ucp.gdrive", "Response: " + OUString::createFromAscii(response.data.substr(0, 200).c_str()));
    } else {
        SAL_WARN("ucb.ucp.gdrive", "File move failed with HTTP response code: " + OUString::number(response.responseCode));
        SAL_WARN("ucb.ucp.gdrive", "Error response: " + OUString::createFromAscii(response.data.c_str()));
    }
}

rtl::OUString GoogleDriveApiClient::getAccessToken()
{
    SAL_WARN("ucb.ucp.gdrive", "getAccessToken() called, current token: " + m_sAccessToken);

    // Try to get a new token if we don't have one
    if (m_sAccessToken.isEmpty())
    {
        SAL_WARN("ucb.ucp.gdrive", "Token is empty, starting OAuth2 HTTP callback flow");

        // Create HTTP server to listen for OAuth2 callback
        OAuth2HttpServer httpServer;
        if (!httpServer.start()) {
            SAL_WARN("ucb.ucp.gdrive", "Failed to start HTTP callback server");
            return rtl::OUString();
        }

        try {
            // Construct OAuth2 authorization URL with HTTP callback
            rtl::OUStringBuffer aAuthUrl;
            aAuthUrl.append(rtl::OUString::createFromAscii(GDRIVE_AUTH_URL));
            aAuthUrl.append(rtl::OUString::createFromAscii("?response_type=code"));
            aAuthUrl.append(rtl::OUString::createFromAscii("&client_id="));
            aAuthUrl.append(rtl::OUString::createFromAscii(GDRIVE_CLIENT_ID));
            aAuthUrl.append(rtl::OUString::createFromAscii("&redirect_uri="));
            aAuthUrl.append(rtl::OUString::createFromAscii(GDRIVE_REDIRECT_URI));
            aAuthUrl.append(rtl::OUString::createFromAscii("&scope="));
            aAuthUrl.append(rtl::OUString::createFromAscii(GDRIVE_SCOPE));
            aAuthUrl.append(rtl::OUString::createFromAscii("&access_type=offline"));

            rtl::OUString sAuthUrl = aAuthUrl.makeStringAndClear();
            SAL_WARN("ucb.ucp.gdrive", "Starting OAuth2 flow with URL: " + sAuthUrl);

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
            SAL_WARN("ucb.ucp.gdrive", "Opening browser for authentication");
            try {
                uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
                uno::Reference<css::system::XSystemShellExecute> xSystemShellExecute(
                    css::system::SystemShellExecute::create(xContext));

                xSystemShellExecute->execute(sAuthUrl, OUString(),
                    css::system::SystemShellExecuteFlags::URIS_ONLY);

                SAL_WARN("ucb.ucp.gdrive", "Browser opened successfully");
            } catch (const css::uno::Exception& e) {
                SAL_WARN("ucb.ucp.gdrive", "Failed to open browser: " + e.Message);
            }

            // Show notification to user if we have an interaction handler
            if (xIH.is()) {
                rtl::Reference<ucbhelper::AuthenticationFallbackRequest> xRequest
                    = new ucbhelper::AuthenticationFallbackRequest(
                        u"Please sign in to Google Drive in your browser and authorize LibreOffice."_ustr,
                        sAuthUrl);
                xIH->handle(xRequest);
            }

            // Wait for the callback with auth code
            rtl::OUString sAuthCode = httpServer.waitForAuthCode(120); // 2 minute timeout

            if (!sAuthCode.isEmpty()) {
                SAL_WARN("ucb.ucp.gdrive", "Received authorization code via HTTP callback");

                // Exchange auth code for access token
                m_sAccessToken = exchangeCodeForToken(sAuthCode);

                if (!m_sAccessToken.isEmpty()) {
                    SAL_WARN("ucb.ucp.gdrive", "Successfully obtained access token via HTTP callback");
                } else {
                    SAL_WARN("ucb.ucp.gdrive", "Failed to exchange authorization code for access token");
                }
            } else {
                SAL_WARN("ucb.ucp.gdrive", "Failed to receive authorization code via HTTP callback");
            }

        } catch (...) {
            SAL_WARN("ucb.ucp.gdrive", "Exception during OAuth2 HTTP callback flow");
        }

        // Stop the HTTP server
        httpServer.stop();
    }

    SAL_WARN("ucb.ucp.gdrive", "Returning token: " + m_sAccessToken);
    return m_sAccessToken;
}

rtl::OUString GoogleDriveApiClient::exchangeCodeForToken(const rtl::OUString& sAuthCode)
{
    SAL_WARN("ucb.ucp.gdrive", "exchangeCodeForToken called with code: " + sAuthCode.copy(0, 10) + "...");

    if (sAuthCode.isEmpty())
    {
        SAL_WARN("ucb.ucp.gdrive", "Auth code is empty");
        return rtl::OUString();
    }

    // Prepare token exchange request
    rtl::OUStringBuffer aBody;
    aBody.append(u"code="_ustr);
    aBody.append(sAuthCode);
    aBody.append(u"&client_id="_ustr);
    aBody.append(rtl::OUString::createFromAscii(GDRIVE_CLIENT_ID));
    aBody.append(u"&client_secret="_ustr);
    aBody.append(rtl::OUString::createFromAscii(GDRIVE_CLIENT_SECRET));
    aBody.append(u"&redirect_uri="_ustr);
    aBody.append(rtl::OUString::createFromAscii(GDRIVE_REDIRECT_URI));
    aBody.append(u"&grant_type=authorization_code"_ustr);

    SAL_WARN("ucb.ucp.gdrive", "Sending token request to: " + rtl::OUString::createFromAscii(GDRIVE_TOKEN_URL));

    rtl::OUString sResponse = sendRequestForString(
        u"POST"_ustr,
        rtl::OUString::createFromAscii(GDRIVE_TOKEN_URL),
        aBody.makeStringAndClear()
    );

    SAL_WARN("ucb.ucp.gdrive", "Token response length: " + OUString::number(sResponse.getLength()));
    if (!sResponse.isEmpty()) {
        SAL_WARN("ucb.ucp.gdrive", "Token response: " + sResponse.copy(0, std::min(200, (int)sResponse.getLength())));

        // Use proper JSON helper to parse token response
        auto tokenPair = GDriveJsonHelper::parseTokenResponse(sResponse);
        rtl::OUString sAccessToken = tokenPair.first;

        if (!sAccessToken.isEmpty()) {
            SAL_WARN("ucb.ucp.gdrive", "Extracted access token: " + sAccessToken.copy(0, 20) + "...");
            return sAccessToken;
        } else {
            SAL_WARN("ucb.ucp.gdrive", "Failed to parse access_token from response");
            return rtl::OUString();
        }
    }

    SAL_WARN("ucb.ucp.gdrive", "Empty response from token exchange");
    return rtl::OUString();
}

size_t GoogleDriveApiClient::WriteCallback(void* contents, size_t size, size_t nmemb, HttpResponse* response)
{
    size_t totalSize = size * nmemb;
    response->data.append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

rtl::OUString GoogleDriveApiClient::sendRequestForString(const rtl::OUString& sMethod, const rtl::OUString& sUrl, const rtl::OUString& sBody)
{
    return sendRequestForStringWithRetry(sMethod, sUrl, sBody, 3);
}

rtl::OUString GoogleDriveApiClient::sendRequestForStringWithRetry(const rtl::OUString& sMethod, const rtl::OUString& sUrl, const rtl::OUString& sBody, sal_Int32 maxRetries)
{
    if (!m_pCurl) {
        SAL_WARN("ucb.ucp.gdrive", "sendRequestForString: CURL not initialized");
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
            SAL_WARN("ucb.ucp.gdrive", "sendRequestForString: Retry attempt " + OUString::number(attemptCount) + " of " + OUString::number(maxRetries + 1));

            // Exponential backoff: wait 1s, 2s, 4s...
            sal_Int32 waitSeconds = 1 << (attemptCount - 2); // 2^(attempt-2)
            SAL_WARN("ucb.ucp.gdrive", "Waiting " + OUString::number(waitSeconds) + " seconds before retry");

            // Simple sleep implementation
            for (sal_Int32 i = 0; i < waitSeconds; i++) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }

        SAL_WARN("ucb.ucp.gdrive", "sendRequestForString: " + sMethod + " " + sUrl + " (attempt " + OUString::number(attemptCount) + ")");

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
                if (body.find('{') != std::string::npos || body.find('[') != std::string::npos) {
                    // Looks like JSON
                    headers = curl_slist_append(headers, "Content-Type: application/json");
                } else {
                    // Assume form data
                    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
                }
            }
        } else if (method == "GET") {
            curl_easy_setopt(m_pCurl, CURLOPT_HTTPGET, 1L);
        }

        // Add authorization header if we have an access token
        if (!m_sAccessToken.isEmpty()) {
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
            SAL_WARN("ucb.ucp.gdrive", "sendRequestForString: CURL error: " +
                     OUString::createFromAscii(curl_easy_strerror(res)));

            // Retry on network errors
            shouldRetry = (res == CURLE_COULDNT_CONNECT ||
                          res == CURLE_OPERATION_TIMEDOUT ||
                          res == CURLE_COULDNT_RESOLVE_HOST ||
                          res == CURLE_RECV_ERROR ||
                          res == CURLE_SEND_ERROR);
        } else {
            curl_easy_getinfo(m_pCurl, CURLINFO_RESPONSE_CODE, &response.responseCode);

            SAL_WARN("ucb.ucp.gdrive", "sendRequestForString: HTTP response code: " +
                     OUString::number(response.responseCode));

            if (response.responseCode >= 200 && response.responseCode < 300) {
                SAL_WARN("ucb.ucp.gdrive", "sendRequestForString: Success, received " +
                         OUString::number(response.data.length()) + " bytes");
                return rtl::OUString::createFromAscii(response.data.c_str());
            } else if (response.responseCode == 401) {
                SAL_WARN("ucb.ucp.gdrive", "sendRequestForString: Authentication failed (401)");
                // Clear stored access token so it will be refreshed on next request
                m_sAccessToken = rtl::OUString();
                shouldRetry = true; // Retry with fresh token
            } else if (response.responseCode == 429 || response.responseCode >= 500) {
                // Retry on rate limiting or server errors
                shouldRetry = true;
                SAL_WARN("ucb.ucp.gdrive", "sendRequestForString: Retryable error (" +
                         OUString::number(response.responseCode) + ")");
            } else if (response.responseCode == 403) {
                SAL_WARN("ucb.ucp.gdrive", "sendRequestForString: Access forbidden (403) - insufficient permissions");
            } else if (response.responseCode == 404) {
                SAL_WARN("ucb.ucp.gdrive", "sendRequestForString: Resource not found (404)");
            } else if (response.responseCode >= 400 && response.responseCode < 500) {
                SAL_WARN("ucb.ucp.gdrive", "sendRequestForString: Client error (" +
                         OUString::number(response.responseCode) + ")");
            }

            // Log error response body if available
            if (!response.data.empty()) {
                SAL_WARN("ucb.ucp.gdrive", "sendRequestForString: Error response: " +
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

} // namespace gdrive
} // namespace ucp