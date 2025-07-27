/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "SlackApiClient.hxx"
#include "slack_json.hxx"
#include "slack_oauth2_server.hxx"
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
namespace slack {

SlackApiClient::SlackApiClient(const uno::Reference<ucb::XCommandEnvironment>& xCmdEnv)
    : m_xCmdEnv(xCmdEnv)
    , m_sAccessToken()
    , m_sRefreshToken()
    , m_sWorkspaceId()
    , m_pCurl(nullptr)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    m_pCurl = curl_easy_init();

    SAL_WARN("ucb.ucp.slack", "SlackApiClient initialized");
}

SlackApiClient::~SlackApiClient()
{
    if (m_pCurl) {
        curl_easy_cleanup(m_pCurl);
    }
    curl_global_cleanup();
}

rtl::OUString SlackApiClient::authenticate()
{
    SAL_WARN("ucb.ucp.slack", "Starting Slack OAuth2 manual authentication");

    try {
        // Build authorization URL with manual redirect
        rtl::OUStringBuffer authUrl;
        authUrl.append(SLACK_AUTH_URL);
        authUrl.append("?client_id=");
        authUrl.append(SLACK_CLIENT_ID);
        authUrl.append("&scope=");
        authUrl.append(SLACK_SCOPES);
        authUrl.append("&redirect_uri=");
        authUrl.append(SLACK_REDIRECT_URI);
        authUrl.append("&response_type=code");
        authUrl.append("&state=libreoffice_slack_auth");

        SAL_WARN("ucb.ucp.slack", "Opening browser for manual Slack authentication");

        // Open browser for user authentication
        uno::Reference<uno::XComponentContext> xContext = comphelper::getProcessComponentContext();
        uno::Reference<css::system::XSystemShellExecute> xSystemShell(
            css::system::SystemShellExecute::create(xContext));

        xSystemShell->execute(authUrl.makeStringAndClear(),
                             rtl::OUString(),
                             css::system::SystemShellExecuteFlags::URIS_ONLY);

        // Create OAuth2 callback server with HTTPS support
        SlackOAuth2Server oauthServer;
        if (!oauthServer.start()) {
            SAL_WARN("ucb.ucp.slack", "Failed to start OAuth2 HTTPS callback server");
            return rtl::OUString();
        }

        // Wait for authorization code from HTTPS callback
        rtl::OUString authCode = oauthServer.waitForAuthCode(120); // 2 minute timeout
        oauthServer.stop();

        if (authCode.isEmpty()) {
            SAL_WARN("ucb.ucp.slack", "No authorization code provided by user");
            return rtl::OUString();
        }

        SAL_WARN("ucb.ucp.slack", "Received authorization code from user, exchanging for token");

        // Exchange authorization code for access token
        rtl::OUString accessToken = exchangeCodeForToken(authCode);

        if (!accessToken.isEmpty()) {
            m_sAccessToken = accessToken;
            SAL_WARN("ucb.ucp.slack", "Successfully authenticated with Slack");
        } else {
            SAL_WARN("ucb.ucp.slack", "Token exchange returned empty token");
        }

        return accessToken;

    } catch (const std::exception& e) {
        SAL_WARN("ucb.ucp.slack", "Exception during authentication: " << e.what());
        return rtl::OUString();
    }
}

rtl::OUString SlackApiClient::getCurrentAccessToken()
{
    return getAccessToken();
}

bool SlackApiClient::isAuthenticated()
{
    return !getAccessToken().isEmpty() && isTokenValid();
}

rtl::OUString SlackApiClient::getAccessToken()
{
    if (m_sAccessToken.isEmpty()) {
        // Try to authenticate if no token available
        return authenticate();
    }

    // Check if token is still valid, refresh if needed
    if (!isTokenValid()) {
        rtl::OUString refreshed = refreshAccessToken();
        if (!refreshed.isEmpty()) {
            m_sAccessToken = refreshed;
        }
    }

    return m_sAccessToken;
}

bool SlackApiClient::isTokenValid()
{
    if (m_sAccessToken.isEmpty()) {
        return false;
    }

    // Test token validity by making a simple API call
    rtl::OUString response = sendRequestForString("GET", SLACK_BASE_URL "/auth.test", "");
    return !SlackJsonHelper::isErrorResponse(response);
}

rtl::OUString SlackApiClient::refreshAccessToken()
{
    // Slack tokens don't have refresh tokens in the same way
    // If token is invalid, user needs to re-authenticate
    SAL_WARN("ucb.ucp.slack", "Token refresh needed - user must re-authenticate");
    m_sAccessToken.clear();
    return rtl::OUString();
}

rtl::OUString SlackApiClient::exchangeCodeForToken(const rtl::OUString& sAuthCode)
{
    SAL_WARN("ucb.ucp.slack", "=== STARTING TOKEN EXCHANGE ===");

    try {
        SAL_WARN("ucb.ucp.slack", "Creating token request body");

        // Create token request
        rtl::OUString requestBody = SlackJsonHelper::createTokenRequest(sAuthCode);
        SAL_WARN("ucb.ucp.slack", "Token request body created successfully");

        SAL_WARN("ucb.ucp.slack", "Sending token exchange request to Slack");

        // Send token exchange request
        rtl::OUString response = sendRequestForString("POST", SLACK_TOKEN_URL, requestBody);
        SAL_WARN("ucb.ucp.slack", "Token exchange request completed");

        if (response.isEmpty()) {
            SAL_WARN("ucb.ucp.slack", "Empty response from token exchange");
            return rtl::OUString();
        }

        SAL_WARN("ucb.ucp.slack", "Parsing token response");

        // Parse token response
        auto tokens = SlackJsonHelper::parseTokenResponse(response);
        SAL_WARN("ucb.ucp.slack", "Token response parsed");

        if (tokens.first.isEmpty()) {
            SAL_WARN("ucb.ucp.slack", "Failed to parse access token from response");
            return rtl::OUString();
        }

        m_sRefreshToken = tokens.second; // May be empty for Slack
        SAL_WARN("ucb.ucp.slack", "Successfully obtained access token");
        SAL_WARN("ucb.ucp.slack", "=== TOKEN EXCHANGE COMPLETED ===");

        return tokens.first;

    } catch (const std::exception& e) {
        SAL_WARN("ucb.ucp.slack", "Exception in token exchange: " << e.what());
        return rtl::OUString();
    } catch (...) {
        SAL_WARN("ucb.ucp.slack", "Unknown exception in token exchange");
        return rtl::OUString();
    }
}

std::vector<SlackWorkspace> SlackApiClient::listWorkspaces()
{
    // For now, return single workspace - Slack OAuth2 typically grants access to one workspace
    std::vector<SlackWorkspace> workspaces;
    SlackWorkspace workspace;
    workspace.id = "current";
    workspace.name = "Current Workspace";
    workspace.isActive = true;
    workspaces.push_back(workspace);
    return workspaces;
}

std::vector<SlackChannel> SlackApiClient::listChannels(const rtl::OUString& workspaceId)
{
    SAL_WARN("ucb.ucp.slack", "=== LISTING CHANNELS for workspace: " + workspaceId + " ===");

    rtl::OUString accessToken = getAccessToken();
    if (accessToken.isEmpty()) {
        SAL_WARN("ucb.ucp.slack", "No access token available for channel listing");
        return std::vector<SlackChannel>();
    }

    SAL_WARN("ucb.ucp.slack", "Access token available, making API request");

    // Get all channel types including DMs, with higher limit for pagination
    rtl::OUString url = rtl::OUString(SLACK_BASE_URL) + "/conversations.list?types=public_channel,private_channel,im,mpim&limit=200";
    SAL_WARN("ucb.ucp.slack", "API URL: " + url);

    rtl::OUString response = sendRequestForString("GET", url, "");

    SAL_WARN("ucb.ucp.slack", "API response received, length: " + rtl::OUString::number(response.getLength()));

    if (response.isEmpty()) {
        SAL_WARN("ucb.ucp.slack", "Empty response from channels API");
        return std::vector<SlackChannel>();
    }

    if (SlackJsonHelper::isErrorResponse(response)) {
        SAL_WARN("ucb.ucp.slack", "Error response from channels API: " + response);
        return std::vector<SlackChannel>();
    }

    SAL_WARN("ucb.ucp.slack", "Parsing channel list from response");
    std::vector<SlackChannel> channels = SlackJsonHelper::parseChannelList(response);

    SAL_WARN("ucb.ucp.slack", "=== PARSED " << channels.size() << " CHANNELS ===");
    return channels;
}

std::vector<SlackUser> SlackApiClient::listUsers(const rtl::OUString& /*workspaceId*/)
{
    // TODO: Implement user listing for DM support
    return std::vector<SlackUser>();
}

rtl::OUString SlackApiClient::shareFile(const rtl::OUString& filename,
                                       const uno::Reference<io::XInputStream>& xInputStream,
                                       sal_Int64 fileSize,
                                       const rtl::OUString& channelId,
                                       const rtl::OUString& message)
{
    SAL_WARN("ucb.ucp.slack", "Starting file share workflow for: " + filename + " (" + OUString::number(fileSize) + " bytes)");

    try {
        // Step 1: Get upload URL and file ID from Slack
        rtl::OUString accessToken = getAccessToken();
        if (accessToken.isEmpty()) {
            SAL_WARN("ucb.ucp.slack", "No access token available for file sharing");
            return rtl::OUString();
        }

        // Create JSON request body for files.getUploadURLExternal
        rtl::OUString requestBody = SlackJsonHelper::createUploadURLRequest(filename, fileSize);

        // Make API call to get upload URL
        rtl::OUString url = rtl::OUString(SLACK_BASE_URL) + "/files.getUploadURLExternal";
        rtl::OUString response = sendRequestForString("POST", url, requestBody);

        if (response.isEmpty() || SlackJsonHelper::isErrorResponse(response)) {
            SAL_WARN("ucb.ucp.slack", "Failed to get upload URL");
            if (!response.isEmpty()) {
                SAL_WARN("ucb.ucp.slack", "Full response: " + response);
                SAL_WARN("ucb.ucp.slack", "Error: " + SlackJsonHelper::extractErrorMessage(response));
            }
            return rtl::OUString();
        }

        // Parse response to get upload URL and file ID
        SlackUploadInfo uploadInfo = SlackJsonHelper::parseUploadURLResponse(response);
        if (uploadInfo.uploadUrl.isEmpty() || uploadInfo.fileId.isEmpty()) {
            SAL_WARN("ucb.ucp.slack", "Missing upload URL or file ID in response");
            return rtl::OUString();
        }

        SAL_WARN("ucb.ucp.slack", "Got upload URL and file ID: " + uploadInfo.fileId);

        // Step 2: Upload file to Slack's provided URL
        uploadFileToURL(uploadInfo.uploadUrl, xInputStream);

        // Step 3: Complete upload (tells Slack to finalize the file and share it)
        rtl::OUString result = completeUpload(uploadInfo.fileId, channelId, message);

        if (!result.isEmpty()) {
            SAL_WARN("ucb.ucp.slack", "File share completed successfully - permalink: " + result);
        } else {
            SAL_WARN("ucb.ucp.slack", "File upload completed but no permalink returned");
        }

        return result;

    } catch (const std::exception& e) {
        SAL_WARN("ucb.ucp.slack", "Exception during file share: " << e.what());
        return rtl::OUString();
    }
}

rtl::OUString SlackApiClient::getUploadURL(const rtl::OUString& filename, sal_Int64 fileSize, const rtl::OUString& /*channelId*/)
{
    SAL_WARN("ucb.ucp.slack", "Getting upload URL for file: " + filename + " (" + OUString::number(fileSize) + " bytes)");

    rtl::OUString accessToken = getAccessToken();
    if (accessToken.isEmpty()) {
        SAL_WARN("ucb.ucp.slack", "No access token available for upload URL request");
        return rtl::OUString();
    }

    // Create JSON request body for files.getUploadURLExternal
    rtl::OUString requestBody = SlackJsonHelper::createUploadURLRequest(filename, fileSize);

    // Make API call to get upload URL
    rtl::OUString url = rtl::OUString(SLACK_BASE_URL) + "/files.getUploadURLExternal";
    rtl::OUString response = sendRequestForString("POST", url, requestBody);

    if (response.isEmpty() || SlackJsonHelper::isErrorResponse(response)) {
        SAL_WARN("ucb.ucp.slack", "Failed to get upload URL");
        if (!response.isEmpty()) {
            SAL_WARN("ucb.ucp.slack", "Error: " + SlackJsonHelper::extractErrorMessage(response));
        }
        return rtl::OUString();
    }

    // Parse response to get upload URL
    SlackUploadInfo uploadInfo = SlackJsonHelper::parseUploadURLResponse(response);
    if (uploadInfo.uploadUrl.isEmpty()) {
        SAL_WARN("ucb.ucp.slack", "No upload URL in response");
        return rtl::OUString();
    }

    SAL_WARN("ucb.ucp.slack", "Successfully obtained upload URL (expires in " + OUString::number(uploadInfo.expiresIn) + "s)");
    return uploadInfo.uploadUrl;
}

void SlackApiClient::uploadFileToURL(const rtl::OUString& uploadUrl, const uno::Reference<io::XInputStream>& xInputStream)
{
    SAL_WARN("ucb.ucp.slack", "Uploading file to URL: " + uploadUrl);

    if (!m_pCurl || !xInputStream.is()) {
        SAL_WARN("ucb.ucp.slack", "Invalid parameters for file upload");
        return;
    }

    // Read file content from input stream
    uno::Sequence<sal_Int8> aBuffer;
    sal_Int32 nBytesRead = 0;
    std::string fileContent;

    try {
        // Reset stream position to beginning if seekable
        uno::Reference<io::XSeekable> xSeekable(xInputStream, uno::UNO_QUERY);
        if (xSeekable.is()) {
            xSeekable->seek(0);
            SAL_WARN("ucb.ucp.slack", "Reset stream position to beginning");
        }

        do {
            nBytesRead = xInputStream->readBytes(aBuffer, 8192);
            if (nBytesRead > 0) {
                fileContent.append(reinterpret_cast<const char*>(aBuffer.getConstArray()), nBytesRead);
            }
        } while (nBytesRead > 0);
    } catch (const uno::Exception& e) {
        SAL_WARN("ucb.ucp.slack", "Error reading file content: " + e.Message);
        return;
    }

    SAL_WARN("ucb.ucp.slack", "Read " + OUString::number(fileContent.length()) + " bytes from input stream");

    // Upload file content to Slack's provided URL
    HttpResponse response;
    response.responseCode = 0;

    std::string url = rtl::OUStringToOString(uploadUrl, RTL_TEXTENCODING_UTF8).getStr();

    curl_easy_reset(m_pCurl);
    curl_easy_setopt(m_pCurl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(m_pCurl, CURLOPT_POST, 1L);
    curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDS, fileContent.c_str());
    curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDSIZE, fileContent.length());

    // Set timeout for large file uploads
    curl_easy_setopt(m_pCurl, CURLOPT_TIMEOUT, 300L); // 5 minutes

    // Verify SSL certificates
    curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYHOST, 2L);

    // Set content type for file upload
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/octet-stream");
    curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(m_pCurl);

    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        SAL_WARN("ucb.ucp.slack", "CURL error during file upload: " + OUString::createFromAscii(curl_easy_strerror(res)));
        return;
    }

    curl_easy_getinfo(m_pCurl, CURLINFO_RESPONSE_CODE, &response.responseCode);

    if (response.responseCode >= 200 && response.responseCode < 300) {
        SAL_WARN("ucb.ucp.slack", "File upload to Slack URL successful");
    } else {
        SAL_WARN("ucb.ucp.slack", "File upload failed with HTTP response code: " + OUString::number(response.responseCode));
        if (!response.data.empty()) {
            SAL_WARN("ucb.ucp.slack", "Error response: " + OUString::createFromAscii(response.data.c_str()));
        }
    }
}

rtl::OUString SlackApiClient::completeUpload(const rtl::OUString& fileId, const rtl::OUString& channelId, const rtl::OUString& message, const rtl::OUString& threadTs)
{
    SAL_WARN("ucb.ucp.slack", "Completing upload for file ID: " + fileId + " to channel: " + channelId);

    rtl::OUString accessToken = getAccessToken();
    if (accessToken.isEmpty()) {
        SAL_WARN("ucb.ucp.slack", "No access token available for complete upload request");
        return rtl::OUString();
    }

    // Create JSON request body for files.completeUploadExternal
    rtl::OUString requestBody = SlackJsonHelper::createCompleteUploadRequest(fileId, channelId, message, threadTs);

    // Make API call to complete upload
    rtl::OUString url = rtl::OUString(SLACK_BASE_URL) + "/files.completeUploadExternal";
    rtl::OUString response = sendRequestForString("POST", url, requestBody);

    if (response.isEmpty() || SlackJsonHelper::isErrorResponse(response)) {
        SAL_WARN("ucb.ucp.slack", "Failed to complete upload");
        if (!response.isEmpty()) {
            SAL_WARN("ucb.ucp.slack", "Full response: " + response);
            SAL_WARN("ucb.ucp.slack", "Error: " + SlackJsonHelper::extractErrorMessage(response));
        }
        return rtl::OUString();
    }

    // Parse response to get file info
    SlackFileInfo fileInfo = SlackJsonHelper::parseCompleteUploadResponse(response);
    if (fileInfo.id.isEmpty()) {
        SAL_WARN("ucb.ucp.slack", "No file ID in complete upload response");
        return rtl::OUString();
    }

    SAL_WARN("ucb.ucp.slack", "Successfully completed upload - file ID: " + fileInfo.id);
    return fileInfo.permalink; // Return permalink to shared file
}

rtl::OUString SlackApiClient::sendRequestForString(const rtl::OUString& sMethod, const rtl::OUString& sUrl, const rtl::OUString& sBody)
{
    return sendRequestForStringWithRetry(sMethod, sUrl, sBody, 3);
}

rtl::OUString SlackApiClient::sendRequestForStringWithRetry(const rtl::OUString& sMethod, const rtl::OUString& sUrl, const rtl::OUString& sBody, sal_Int32 maxRetries)
{
    if (!m_pCurl) {
        SAL_WARN("ucb.ucp.slack", "sendRequestForString: CURL not initialized");
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
            SAL_WARN("ucb.ucp.slack", "sendRequestForString: Retry attempt " + OUString::number(attemptCount) + " of " + OUString::number(maxRetries + 1));

            // Exponential backoff: wait 1s, 2s, 4s...
            sal_Int32 waitSeconds = 1 << (attemptCount - 2); // 2^(attempt-2)
            SAL_WARN("ucb.ucp.slack", "Waiting " + OUString::number(waitSeconds) + " seconds before retry");

            // Simple sleep implementation
            for (sal_Int32 i = 0; i < waitSeconds; i++) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }

        SAL_WARN("ucb.ucp.slack", "sendRequestForString: " + sMethod + " " + sUrl + " (attempt " + OUString::number(attemptCount) + ")");

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

                // Determine content type based on URL and body content
                if (url.find("oauth.v2.access") != std::string::npos || url.find("files.completeUploadExternal") != std::string::npos) {
                    // Slack token requests and file completion expect form data
                    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
                } else if (body[0] == '{' || body[0] == '[') {
                    // JSON content (starts with { or [)
                    headers = curl_slist_append(headers, "Content-Type: application/json");
                } else {
                    // Default to form data
                    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
                }
            }
        } else if (method == "GET") {
            curl_easy_setopt(m_pCurl, CURLOPT_HTTPGET, 1L);
        }

        // Add authorization header
        if (url.find("oauth.v2.access") != std::string::npos) {
            // For Slack token requests, do NOT add Authorization header
            // Slack expects client_secret in the form body
            SAL_WARN("ucb.ucp.slack", "Token request - using form data authentication (no Authorization header)");
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
            SAL_WARN("ucb.ucp.slack", "sendRequestForString: CURL error: " +
                     OUString::createFromAscii(curl_easy_strerror(res)));

            // Retry on network errors
            shouldRetry = (res == CURLE_COULDNT_CONNECT ||
                          res == CURLE_OPERATION_TIMEDOUT ||
                          res == CURLE_COULDNT_RESOLVE_HOST ||
                          res == CURLE_RECV_ERROR ||
                          res == CURLE_SEND_ERROR);
        } else {
            curl_easy_getinfo(m_pCurl, CURLINFO_RESPONSE_CODE, &response.responseCode);

            SAL_WARN("ucb.ucp.slack", "sendRequestForString: HTTP response code: " +
                     OUString::number(response.responseCode));

            if (response.responseCode >= 200 && response.responseCode < 300) {
                SAL_WARN("ucb.ucp.slack", "sendRequestForString: Success, received " +
                         OUString::number(response.data.length()) + " bytes");
                return rtl::OUString::createFromAscii(response.data.c_str());
            } else if (response.responseCode == 401) {
                SAL_WARN("ucb.ucp.slack", "sendRequestForString: Authentication failed (401)");
                // Clear stored access token so it will be refreshed on next request
                m_sAccessToken = rtl::OUString();
                shouldRetry = true; // Retry with fresh token
            } else if (response.responseCode == 429 || response.responseCode >= 500) {
                // Retry on rate limiting or server errors
                shouldRetry = true;
                SAL_WARN("ucb.ucp.slack", "sendRequestForString: Retryable error (" +
                         OUString::number(response.responseCode) + ")");
            } else if (response.responseCode == 403) {
                SAL_WARN("ucb.ucp.slack", "sendRequestForString: Access forbidden (403) - insufficient permissions");
            } else if (response.responseCode == 404) {
                SAL_WARN("ucb.ucp.slack", "sendRequestForString: Resource not found (404)");
            } else if (response.responseCode >= 400 && response.responseCode < 500) {
                SAL_WARN("ucb.ucp.slack", "sendRequestForString: Client error (" +
                         OUString::number(response.responseCode) + ")");
            }

            // Log error response body if available
            if (!response.data.empty()) {
                SAL_WARN("ucb.ucp.slack", "sendRequestForString: Error response: " +
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

void SlackApiClient::sendFileUpload(const rtl::OUString& sUrl, const uno::Reference<io::XInputStream>& xInputStream)
{
    // This method is now handled by uploadFileToURL - keeping for interface compatibility
    uploadFileToURL(sUrl, xInputStream);
}

size_t SlackApiClient::WriteCallback(void* contents, size_t size, size_t nmemb, HttpResponse* response)
{
    size_t totalSize = size * nmemb;
    response->data.append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

size_t SlackApiClient::ReadCallback(void* /*ptr*/, size_t /*size*/, size_t /*nmemb*/, void* /*userdata*/)
{
    // This callback is used for streaming file uploads
    // Currently we read the entire file into memory in uploadFileToURL
    // This could be optimized for large files in the future
    return 0;
}

rtl::OUString SlackApiClient::promptForAuthCode()
{
    // Show a simple input dialog asking user to paste the auth code
    // This replaces the complex OAuth callback server approach

    SAL_WARN("ucb.ucp.slack", "Prompting user for authorization code");

    // For now, return empty string - this will be handled by the calling dialog
    // The actual UI prompt will be implemented in the SlackShareDialog
    return rtl::OUString();
}

} // namespace slack
} // namespace ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
