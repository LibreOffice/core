/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_UCP_GDRIVE_CLIENT_HXX
#define INCLUDED_UCP_GDRIVE_CLIENT_HXX

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <vector>
#include <memory>
#include <curl/curl.h>
#include "gdrive_json.hxx"

namespace ucp {
namespace gdrive {

class SAL_DLLPUBLIC_EXPORT GoogleDriveApiClient
{
public:
    GoogleDriveApiClient(const com::sun::star::uno::Reference<com::sun::star::ucb::XCommandEnvironment>& xCmdEnv);
    ~GoogleDriveApiClient();

    std::vector<GDriveFileInfo> listFolder(const rtl::OUString& folderId);
    std::vector<GDriveFileInfo> listFolderComplete(const rtl::OUString& folderId, sal_Int32 maxFiles = 1000);
    GDriveFileInfo getFileInfo(const rtl::OUString& fileId);
    com::sun::star::uno::Reference<com::sun::star::io::XInputStream> downloadFile(const rtl::OUString& fileId);
    void uploadFile(const rtl::OUString& parentId, const rtl::OUString& fileName, const com::sun::star::uno::Reference<com::sun::star::io::XInputStream>& xInputStream);
    void updateFile(const rtl::OUString& fileId, const com::sun::star::uno::Reference<com::sun::star::io::XInputStream>& xInputStream);
    void createFolder(const rtl::OUString& parentId, const rtl::OUString& folderName);
    void deleteFile(const rtl::OUString& fileId);
    rtl::OUString copyFile(const rtl::OUString& fileId, const rtl::OUString& newParentId, const rtl::OUString& newName = rtl::OUString());
    void moveFile(const rtl::OUString& fileId, const rtl::OUString& newParentId, const rtl::OUString& newName = rtl::OUString());

private:
    com::sun::star::uno::Reference<com::sun::star::ucb::XCommandEnvironment> m_xCmdEnv;
    rtl::OUString m_sAccessToken;
    CURL* m_pCurl;

    rtl::OUString getAccessToken();
    rtl::OUString exchangeCodeForToken(const rtl::OUString& sAuthCode);
    rtl::OUString sendRequestForString(const rtl::OUString& sMethod, const rtl::OUString& sUrl, const rtl::OUString& sBody);
    rtl::OUString sendRequestForStringWithRetry(const rtl::OUString& sMethod, const rtl::OUString& sUrl, const rtl::OUString& sBody, sal_Int32 maxRetries);

    struct HttpResponse {
        std::string data;
        long responseCode;
    };

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, HttpResponse* response);
};

} // namespace gdrive
} // namespace ucp

#endif // INCLUDED_UCP_GDRIVE_CLIENT_HXX
