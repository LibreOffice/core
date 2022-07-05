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
#include <wrtsh.hxx>
#include <pam.hxx>
#include <node.hxx>
#include <ndtxt.hxx>
#include <translatehelper.hxx>
#include <sal/log.hxx>
#include <rtl/string.h>
#include <shellio.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/svapp.hxx>
#include <curl/curl.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <vcl/htmltransferable.hxx>
#include <vcl/transfer.hxx>
#include <swdtflvr.hxx>

namespace SwTranslateHelper
{
OString Translate(const OString& rTargetLang, const OString& rAPIUrl, const OString& rAuthKey,
                  const OString& rData)
{
    constexpr tools::Long CURL_TIMEOUT = 10L;

    std::unique_ptr<CURL, std::function<void(CURL*)>> curl(curl_easy_init(),
                                                           [](CURL* p) { curl_easy_cleanup(p); });
    curl_easy_setopt(curl.get(), CURLOPT_URL, rAPIUrl.getStr());
    curl_easy_setopt(curl.get(), CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, CURL_TIMEOUT);

    std::string response_body;
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION,
                     +[](void* buffer, size_t size, size_t nmemb, void* userp) -> size_t {
                         if (!userp)
                             return 0;
                         std::string* response = static_cast<std::string*>(userp);
                         size_t real_size = size * nmemb;
                         response->append(static_cast<char*>(buffer), real_size);
                         return real_size;
                     });
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, static_cast<void*>(&response_body));

    OString aPostData("auth_key=" + rAuthKey + "&target_lang=" + rTargetLang + "&text="
                      + OString(curl_easy_escape(curl.get(), rData.getStr(), rData.getLength())));

    curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, aPostData.getStr());
    CURLcode cc = curl_easy_perform(curl.get());
    if (cc != CURLE_OK)
    {
        SAL_WARN("translatehelper",
                 "CURL perform returned with error: " << static_cast<sal_Int32>(cc));
        return {};
    }
    tools::Long nStatusCode;
    curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &nStatusCode);
    if (nStatusCode != 200)
    {
        SAL_WARN("translatehelper", "CURL request returned with status code: " << nStatusCode);
        return {};
    }
    // parse the response
    boost::property_tree::ptree root;
    std::stringstream aStream(response_body.data());
    boost::property_tree::read_json(aStream, root);
    boost::property_tree::ptree& translations = root.get_child("translations");
    size_t size = translations.size();
    if (size <= 0)
    {
        SAL_WARN("translatehelper", "API did not return any translations");
    }
    // take the first one
    const boost::property_tree::ptree& translation = translations.begin()->second;
    const std::string text = translation.get<std::string>("text");
    return OString(text);
}

OString ExportPaMToHTML(SwPaM* pCursor, bool bReplacePTag)
{
    SolarMutexGuard gMutex;
    OString aResult;
    WriterRef xWrt;
    GetHTMLWriter(OUString("NoLineLimit,SkipHeaderFooter"), OUString(), xWrt);
    if (pCursor != nullptr)
    {
        SvMemoryStream aMemoryStream;
        SwWriter aWriter(aMemoryStream, *pCursor);
        ErrCode nError = aWriter.Write(xWrt);
        if (nError.IsError())
        {
            SAL_WARN("translatehelper", "failed to export selection to HTML");
            return {};
        }
        aResult
            = OString(static_cast<const char*>(aMemoryStream.GetData()), aMemoryStream.GetSize());
        if (bReplacePTag)
        {
            aResult = aResult.replaceAll("<p", "<span");
            aResult = aResult.replaceAll("</p>", "</span>");
        }
        return aResult;
    }
    return {};
}

void PasteHTMLToPaM(SwWrtShell& rWrtSh, SwPaM* pCursor, const OString& rData, bool bSetSelection)
{
    SolarMutexGuard gMutex;
    rtl::Reference<vcl::unohelper::HtmlTransferable> pHtmlTransferable
        = new vcl::unohelper::HtmlTransferable(rData);
    if (pHtmlTransferable.is())
    {
        TransferableDataHelper aDataHelper(pHtmlTransferable);
        if (aDataHelper.GetXTransferable().is()
            && SwTransferable::IsPasteSpecial(rWrtSh, aDataHelper))
        {
            if (bSetSelection)
            {
                rWrtSh.SetSelection(*pCursor);
            }
            SwTransferable::Paste(rWrtSh, aDataHelper);
            rWrtSh.KillSelection(nullptr, false);
        }
    }
}
}