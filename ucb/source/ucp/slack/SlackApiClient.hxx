/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_UCP_SLACK_CLIENT_HXX
#define INCLUDED_UCP_SLACK_CLIENT_HXX

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <vector>
#include <memory>
#include <curl/curl.h>
#include "slack_json.hxx"

namespace ucp {
namespace slack {

class SAL_DLLPUBLIC_EXPORT SlackApiClient
{
public:
    SlackApiClient(const com::sun::star::uno::Reference<com::sun::star::ucb::XCommandEnvironment>& xCmdEnv);
    ~SlackApiClient();

    // Authentication methods
    rtl::OUString authenticate();
    rtl::OUString getCurrentAccessToken();
    bool isAuthenticated();

    // Slack workspace and channel discovery
    std::vector<SlackWorkspace> listWorkspaces();
    std::vector<SlackChannel> listChannels(const rtl::OUString& workspaceId);
    std::vector<SlackUser> listUsers(const rtl::OUString& workspaceId);

    // File sharing workflow (new Slack async upload API)
    rtl::OUString getUploadURL(const rtl::OUString& filename, sal_Int64 fileSize, const rtl::OUString& channelId);
    void uploadFileToURL(const rtl::OUString& uploadUrl, const com::sun::star::uno::Reference<com::sun::star::io::XInputStream>& xInputStream);
    rtl::OUString completeUpload(const rtl::OUString& fileId, const rtl::OUString& channelId, const rtl::OUString& message = rtl::OUString(), const rtl::OUString& threadTs = rtl::OUString());

    // Convenience method for complete share workflow
    rtl::OUString shareFile(const rtl::OUString& filename, const com::sun::star::uno::Reference<com::sun::star::io::XInputStream>& xInputStream, sal_Int64 fileSize, const rtl::OUString& channelId, const rtl::OUString& message = rtl::OUString());

private:
    com::sun::star::uno::Reference<com::sun::star::ucb::XCommandEnvironment> m_xCmdEnv;
    rtl::OUString m_sAccessToken;
    rtl::OUString m_sRefreshToken;
    rtl::OUString m_sWorkspaceId;
    CURL* m_pCurl;

    // Authentication helpers
    rtl::OUString getAccessToken();
    rtl::OUString refreshAccessToken();
    bool isTokenValid();
    rtl::OUString promptForAuthCode();
    rtl::OUString exchangeCodeForToken(const rtl::OUString& sAuthCode);

    // HTTP infrastructure
    rtl::OUString sendRequestForString(const rtl::OUString& sMethod, const rtl::OUString& sUrl, const rtl::OUString& sBody);
    rtl::OUString sendRequestForStringWithRetry(const rtl::OUString& sMethod, const rtl::OUString& sUrl, const rtl::OUString& sBody, sal_Int32 maxRetries = 3);
    void sendFileUpload(const rtl::OUString& sUrl, const com::sun::star::uno::Reference<com::sun::star::io::XInputStream>& xInputStream);

    struct HttpResponse {
        std::string data;
        long responseCode;
    };

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, HttpResponse* response);
    static size_t ReadCallback(void* ptr, size_t size, size_t nmemb, void* userdata);
};

} // namespace slack
} // namespace ucp

#endif // INCLUDED_UCP_SLACK_CLIENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
