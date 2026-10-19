/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <translate/providers.hxx>
#include <translate/httprequest.hxx>

#include <o3tl/string_view.hxx>
#include <officecfg/Office/Linguistic.hxx>
#include <sal/log.hxx>
#include <boost/property_tree/ptree.hpp>

#include <string_view>

namespace linguistic::translate
{
namespace
{
using DeeplCfg = officecfg::Office::Linguistic::Translation::Deepl;

/// The DeepL API: the translate endpoint from the configuration, e.g.
/// https://api.deepl.com/v2/translate, with the key in the Authorization
/// header.
class DeeplProvider final : public TranslationProvider
{
public:
    OString getId() const override { return "deepl"_ostr; }
    OUString getDisplayName() const override { return u"DeepL"_ustr; }
    bool isEnabled() const override { return DeeplCfg::IsEnabled::get(); }
    bool isConfigured() const override;
    TranslateResult translate(const OString& rHtml, const OString& rTargetLang,
                              const bool& rCancel) override;

protected:
    std::vector<OString> fetchTargetLanguages() override;
    OString getLanguageCacheKey() const override;

private:
    /// The translate endpoint and the key, trimmed; false when either is missing.
    static bool getSettings(OString& rApiUrl, OString& rAuthKey);
    /// Authenticates rRequest with the key and has it report the DeepL error message.
    static void prepareRequest(HttpRequest& rRequest, std::string_view rAuthKey);
};

bool DeeplProvider::getSettings(OString& rApiUrl, OString& rAuthKey)
{
    const std::optional<OUString> oApiUrl = DeeplCfg::ApiURL::get();
    const std::optional<OUString> oAuthKey = DeeplCfg::AuthKey::get();
    const std::u16string_view aApiUrl = oApiUrl ? o3tl::trim(*oApiUrl) : std::u16string_view();
    const std::u16string_view aAuthKey = oAuthKey ? o3tl::trim(*oAuthKey) : std::u16string_view();
    if (aApiUrl.empty() || aAuthKey.empty())
        return false;
    rApiUrl = OUStringToOString(aApiUrl, RTL_TEXTENCODING_UTF8);
    rAuthKey = OUStringToOString(aAuthKey, RTL_TEXTENCODING_UTF8);
    return true;
}

bool DeeplProvider::isConfigured() const
{
    OString aApiUrl, aAuthKey;
    return isEnabled() && getSettings(aApiUrl, aAuthKey);
}

OString DeeplProvider::getLanguageCacheKey() const
{
    OString aApiUrl, aAuthKey;
    getSettings(aApiUrl, aAuthKey);
    return aApiUrl + "\n" + aAuthKey;
}

void DeeplProvider::prepareRequest(HttpRequest& rRequest, std::string_view rAuthKey)
{
    // DeepL only accepts the key in the Authorization header; the former
    // auth_key form field is rejected with 403 since January 2026.
    rRequest.addHeader(OString::Concat("Authorization: DeepL-Auth-Key ") + rAuthKey);
    rRequest.setErrorMessageKey("message");
}

TranslateResult DeeplProvider::translate(const OString& rHtml, const OString& rTargetLang,
                                         const bool& /*rCancel*/)
{
    static constexpr std::chrono::seconds TIMEOUT(10);
    static constexpr std::chrono::seconds CONNECT_TIMEOUT(10);

    OString aApiUrl, aAuthKey;
    if (!getSettings(aApiUrl, aAuthKey))
        return { {}, "The DeepL API is not set up"_ostr };

    HttpRequest aRequest(aApiUrl, TIMEOUT, CONNECT_TIMEOUT);
    if (!aRequest.isValid())
        return { {}, "CURL initialization failed"_ostr };
    prepareRequest(aRequest, aAuthKey);

    // rHtml is HTML: tag_handling keeps the markup intact across the translation. The
    // API takes the language codes in uppercase, e.g. "EN-GB".
    aRequest.setPostFields("target_lang=" + aRequest.escape(rTargetLang.toAsciiUpperCase())
                           + "&tag_handling=html&text=" + aRequest.escape(rHtml));

    const OString aError = aRequest.perform();
    if (!aError.isEmpty())
        return { {}, aError };

    const std::optional<boost::property_tree::ptree> oRoot
        = parseJson(aRequest.getResponseBody(), "response");
    if (!oRoot)
        return { {}, "Unexpected response from the service"_ostr };
    try
    {
        const boost::property_tree::ptree& rTranslations = oRoot->get_child("translations");
        if (rTranslations.empty())
        {
            SAL_WARN("linguistic", "DeepL: API did not return any translations");
            return { {}, "The service did not return any translations"_ostr };
        }
        // take the first one
        const boost::property_tree::ptree& rTranslation = rTranslations.begin()->second;
        return { OString(rTranslation.get<std::string>("text")), {} };
    }
    catch (const boost::property_tree::ptree_error& rException)
    {
        SAL_WARN("linguistic", "DeepL: unexpected response: " << rException.what());
        return { {}, "Unexpected response from the service"_ostr };
    }
}

std::vector<OString> DeeplProvider::fetchTargetLanguages()
{
    static constexpr std::chrono::seconds TIMEOUT(5);
    // The request blocks the main loop, so an unreachable host should fail fast.
    static constexpr std::chrono::seconds CONNECT_TIMEOUT(2);
    static constexpr std::string_view aTranslatePath = "/v2/translate";

    OString aApiUrl, aAuthKey;
    if (!getSettings(aApiUrl, aAuthKey))
        return {};

    // The language list lives at /v3/languages next to the /v2/translate endpoint.
    std::string_view aBaseUrl(aApiUrl);
    if (aBaseUrl.ends_with('/'))
        aBaseUrl.remove_suffix(1);
    if (!aBaseUrl.ends_with(aTranslatePath))
    {
        SAL_INFO("linguistic", "DeepL: no language list for " << aApiUrl);
        return {};
    }
    aBaseUrl.remove_suffix(aTranslatePath.size());

    HttpRequest aRequest(OString::Concat(aBaseUrl) + "/v3/languages?resource=translate_text",
                         TIMEOUT, CONNECT_TIMEOUT);
    prepareRequest(aRequest, aAuthKey);
    if (!aRequest.perform().isEmpty())
        return {};

    // The response is an array of objects like
    // {"lang": "en-GB", "name": "English (British)", "usable_as_target": true, ...}
    const std::optional<boost::property_tree::ptree> oRoot
        = parseJson(aRequest.getResponseBody(), "language list");
    if (!oRoot)
        return {};
    std::vector<OString> aLanguages;
    for (const auto& rEntry : *oRoot)
    {
        if (!rEntry.second.get<bool>("usable_as_target", false))
            continue;
        const std::string aLang = rEntry.second.get<std::string>("lang", std::string());
        if (!aLang.empty())
            aLanguages.emplace_back(aLang);
    }
    return aLanguages;
}
}

std::shared_ptr<TranslationProvider> createDeeplProvider()
{
    return std::make_shared<DeeplProvider>();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
