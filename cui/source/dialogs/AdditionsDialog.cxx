/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#include <AdditionsDialog.hxx>
#include <curl/curl.h>

#include <orcus/json_document_tree.hpp>
#include <orcus/config.hpp>
#include <orcus/pstring.hpp>

#include <../desktop/source/deployment/manager/dp_extensionmanager.hxx> // TODO

AdditionsDialog::AdditionsDialog(weld::Widget* pParent)
    : GenericDialogController(pParent, "cui/ui/additionsdialog.ui", "AdditionsDialog")
    , m_xScreenshot(m_xBuilder->weld_image("imgScreenshot"))
    , m_xInstallButton(m_xBuilder->weld_button("btnInstall"))
    , m_xExtensionName(m_xBuilder->weld_label("labelExtensionName"))
    , m_xAuthor(m_xBuilder->weld_label("labelAuthor"))
    , m_xIntro(m_xBuilder->weld_label("labelIntro"))
    , mpParent(pParent)
{
    m_xInstallButton->connect_clicked(LINK(this, AdditionsDialog, InstallButtonTestHdl));
}

IMPL_LINK_NOARG(AdditionsDialog, InstallButtonTestHdl, weld::Button&, void)
{
    m_xExtensionName->set_label("Test is successfully completed!");
}

namespace
{
// Callback to get the response data from server.
size_t WriteCallback(void* ptr, size_t size, size_t nmemb, void* userp)
{
    if (!userp)
        return 0;

    std::string* response = static_cast<std::string*>(userp);
    size_t real_size = size * nmemb;
    response->append(static_cast<char*>(ptr), real_size);
    return real_size;
}

// Callback to get the response data from server to a file.
size_t WriteCallbackFile(void* ptr, size_t size, size_t nmemb, void* userp)
{
    if (!userp)
        return 0;

    SvStream* response = static_cast<SvStream*>(userp);
    size_t real_size = size * nmemb;
    response->WriteBytes(ptr, real_size);
    return real_size;
}

// Gets the content of the given URL and returns as a standard string
std::string curlGet(const OString& rURL)
{
    CURL* curl = curl_easy_init();

    if (!curl)
        return std::string();

    curl_easy_setopt(curl, CURLOPT_URL, rURL.getStr());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, kUserAgent);

    std::string response_body;

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, static_cast<void*>(&response_body));

    CURLcode cc = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    if (http_code != 200)
    {
        SAL_WARN("cui.options", "Download failed. Error code: " << http_code);
    }

    if (cc != CURLE_OK)
    {
        SAL_WARN("cui.options", "curl error: " << cc);
    }

    return response_body;
}

// Downloads and saves the file at the given rURL to a local path (sFileURL)
void curlDownload(const OString& rURL, const OUString& sFileURL)
{
    CURL* curl = curl_easy_init();
    SvFileStream aFile(sFileURL, StreamMode::WRITE);

    if (!curl)
        return;

    curl_easy_setopt(curl, CURLOPT_URL, rURL.getStr());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, kUserAgent);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackFile);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, static_cast<void*>(&aFile));

    CURLcode cc = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    if (http_code != 200)
    {
        SAL_WARN("cui.options", "Download failed. Error code: " << http_code);
    }

    if (cc != CURLE_OK)
    {
        SAL_WARN("cui.options", "curl error: " << cc);
    }
}

OUString parameterGenerator()
{
    return ""; // WIP
}

OUString requestAPI()
{
    OUString apiURL = "https://extensions.libreoffice.org";
    apiURL += "/" + parameterGenerator();
    OUString apiResponse = curlGet(apiURL);
    return apiResponse;
}

void parseResponseJSON(OUString apiResponse)
{
    orcus::json::document_tree aJsonDoc;
    orcus::json_config aConfig;

    if (apiResponse.empty())
        return;

    aJsonDoc.load(apiResponse, aConfig);

    auto aDocumentRoot = aJsonDoc.get_document_root();
    if (aDocumentRoot.type() != orcus::json::node_t::object)
    {
        SAL_WARN("cui.options", "invalid root entries: " << apiResponse);
        return;
    }

    auto resultArray = aDocumentRoot.child("result");

    for (size_t i = 0; i < resultArray.child_count(); ++i)
    {
        auto arrayElement = resultArray.child(i);

        try
        {
            AdditionsExtension aNewExtension = {
                OStringToOUString(OString(arrayElement.child("id").string_value().get()),
                                  RTL_TEXTENCODING_UTF8),
                OStringToOUString(
                    OString(arrayElement.child("name").child("en-US").string_value().get()),
                    RTL_TEXTENCODING_UTF8),
                OStringToOUString(OString(arrayElement.child("intro").string_value().get()),
                                  RTL_TEXTENCODING_UTF8),
                OStringToOUString(
                    OString(arrayElement.child("tags").string_value().get()), // Array - WIP -
                    RTL_TEXTENCODING_UTF8),
                OStringToOUString(OString(arrayElement.child("url").string_value().get()),
                                  RTL_TEXTENCODING_UTF8),
                OStringToOUString(OString(arrayElement.child("downloadURL").string_value().get()),
                                  RTL_TEXTENCODING_UTF8),
                OStringToOUString(OString(arrayElement.child("screenshotURL").string_value().get()),
                                  RTL_TEXTENCODING_UTF8),
                OStringToOUString(OString(arrayElement.child("rating").string_value().get()),
                                  RTL_TEXTENCODING_UTF8),
                OStringToOUString(
                    OString(arrayElement.child("downloadNumber").string_value().get()),
                    RTL_TEXTENCODING_UTF8),
                OStringToOUString(OString(arrayElement.child("commentNumber").string_value().get()),
                                  RTL_TEXTENCODING_UTF8),
                OStringToOUString(OString(arrayElement.child("licenseType").string_value().get()),
                                  RTL_TEXTENCODING_UTF8),
                OStringToOUString(OString(arrayElement.child("requiredVersion")
                                              .string_value()
                                              .get()), // May be deleted
                                  RTL_TEXTENCODING_UTF8)
            };

            //push object to vector.
        }
        catch (orcus::json::document_error& e)
        {
            // This usually happens when one of the values is null (type() == orcus::json::node_t::null)
            // TODO: Allow null values in personas.
            SAL_WARN("cui.options", "Additions JSON parse error: " << e.what());
        }
    }
}

//dp_manager::ExtensionManager::addExtension() - WIP
