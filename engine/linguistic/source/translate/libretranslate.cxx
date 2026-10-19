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
#include <tools/json_writer.hxx>
#include <boost/property_tree/ptree.hpp>

#include <algorithm>
#include <map>
#include <string_view>

namespace linguistic::translate
{
namespace
{
using LibreTranslateCfg = officecfg::Office::Linguistic::Translation::LibreTranslate;

/// A LibreTranslate server: its root from the configuration, with /translate
/// and /languages appended, and the key, if any, as the api_key member of
/// the request.
class LibreTranslateProvider final : public TranslationProvider
{
public:
    OString getId() const override { return "libretranslate"_ostr; }
    OUString getDisplayName() const override { return u"LibreTranslate"_ustr; }
    bool isEnabled() const override { return LibreTranslateCfg::IsEnabled::get(); }
    bool isConfigured() const override;
    TranslateResult translate(const OString& rHtml, const OString& rTargetLang,
                              const bool& rCancel) override;

protected:
    std::vector<OString> fetchTargetLanguages() override;
    OString getLanguageCacheKey() const override;

private:
    /// The server's root without a trailing slash, and the key, which may be empty;
    /// false without a server.
    static bool getSettings(OString& rBaseUrl, OString& rApiKey);
    /// Has rRequest check the certificate as configured and report the server's error message.
    static void prepareRequest(HttpRequest& rRequest);
    /// The code the server knows the language rBcp47 by.
    OString toServiceCode(const OString& rBcp47);

    /// The server's own code of each language it listed, by the BCP 47 tag the list
    /// reports it as: "zt" is listed as "zh-Hant", the rest as they are.
    std::map<OString, OString> m_aServiceCodes;
};

/// The BCP 47 tag of a language code of the server. The codes are BCP 47 tags, except
/// for "zt", the server's name for traditional Chinese.
OString toBcp47(const OString& rCode) { return rCode == "zt" ? "zh-Hant"_ostr : rCode; }

bool LibreTranslateProvider::getSettings(OString& rBaseUrl, OString& rApiKey)
{
    const std::optional<OUString> oBaseUrl = LibreTranslateCfg::BaseURL::get();
    const std::optional<OUString> oApiKey = LibreTranslateCfg::ApiKey::get();
    std::u16string_view aBaseUrl = oBaseUrl ? o3tl::trim(*oBaseUrl) : std::u16string_view();
    const std::u16string_view aApiKey = oApiKey ? o3tl::trim(*oApiKey) : std::u16string_view();
    // The root is wanted, but the translate endpoint is what one tends to paste.
    while (aBaseUrl.ends_with(u'/'))
        aBaseUrl.remove_suffix(1);
    if (aBaseUrl.ends_with(u"/translate"))
        aBaseUrl.remove_suffix(std::u16string_view(u"/translate").size());
    if (aBaseUrl.empty())
        return false;
    rBaseUrl = OUStringToOString(aBaseUrl, RTL_TEXTENCODING_UTF8);
    rApiKey = OUStringToOString(aApiKey, RTL_TEXTENCODING_UTF8);
    return true;
}

bool LibreTranslateProvider::isConfigured() const
{
    OString aBaseUrl, aApiKey;
    return isEnabled() && getSettings(aBaseUrl, aApiKey);
}

OString LibreTranslateProvider::getLanguageCacheKey() const
{
    OString aBaseUrl, aApiKey;
    getSettings(aBaseUrl, aApiKey);
    return aBaseUrl + "\n" + aApiKey;
}

void LibreTranslateProvider::prepareRequest(HttpRequest& rRequest)
{
    rRequest.setVerifySSL(LibreTranslateCfg::SSLCertVerify::get());
    rRequest.setErrorMessageKey("error");
}

OString LibreTranslateProvider::toServiceCode(const OString& rBcp47)
{
    const OString aTag = rBcp47.toAsciiLowerCase();
    const sal_Int32 nSeparator = aTag.indexOf('-');
    const OString aLanguage = nSeparator == -1 ? aTag : aTag.copy(0, nSeparator);

    // The language as the server lists it, when its list is known: the whole tag where
    // the server has it, e.g. "pt-BR", else its language, e.g. "de" for "de-AT", each by
    // the server's own code for it.
    const std::vector<OString> aKnown = getTargetLanguages(/*bAskService=*/false);
    for (const OString& rKnown : aKnown)
    {
        if (rKnown.equalsIgnoreAsciiCase(aTag))
            return m_aServiceCodes[rKnown];
    }
    for (const OString& rKnown : aKnown)
    {
        if (rKnown.equalsIgnoreAsciiCase(aLanguage))
            return m_aServiceCodes[rKnown];
    }

    // Without the list, the codes the server has used so far: "zt" for traditional
    // Chinese, "zh" for the rest of Chinese, the language alone otherwise.
    if (aTag == "zh-hant" || aTag == "zh-tw" || aTag == "zh-hk" || aTag == "zh-mo")
        return "zt"_ostr;
    if (aLanguage == "zh")
        return "zh"_ostr;
    return aLanguage;
}

TranslateResult LibreTranslateProvider::translate(const OString& rHtml, const OString& rTargetLang,
                                                  const bool& /*rCancel*/)
{
    // A self-hosted server translates on its CPU: a long paragraph takes seconds.
    static constexpr std::chrono::seconds TIMEOUT(30);
    static constexpr std::chrono::seconds CONNECT_TIMEOUT(3);

    OString aBaseUrl, aApiKey;
    if (!getSettings(aBaseUrl, aApiKey))
        return { {}, "The LibreTranslate server is not set up"_ostr };

    HttpRequest aRequest(aBaseUrl + "/translate", TIMEOUT, CONNECT_TIMEOUT);
    if (!aRequest.isValid())
        return { {}, "CURL initialization failed"_ostr };
    prepareRequest(aRequest);

    // rHtml is HTML: the html format keeps the markup intact across the translation. The
    // source language is for the server to detect.
    tools::JsonWriter aJson;
    aJson.put("q", rHtml);
    aJson.put("source", "auto");
    aJson.put("target", toServiceCode(rTargetLang));
    aJson.put("format", "html");
    if (!aApiKey.isEmpty())
        aJson.put("api_key", aApiKey);
    aRequest.setJsonBody(aJson.finishAndGetAsOString());

    const OString aError = aRequest.perform();
    if (!aError.isEmpty())
        return { {}, aError };

    const std::optional<boost::property_tree::ptree> oRoot
        = parseJson(aRequest.getResponseBody(), "response");
    if (!oRoot)
        return { {}, "Unexpected response from the service"_ostr };
    const std::string aText = oRoot->get<std::string>("translatedText", std::string());
    if (aText.empty())
    {
        SAL_WARN("linguistic", "LibreTranslate: no translatedText in the response");
        return { {}, "The service did not return any translations"_ostr };
    }
    return { OString(aText), {} };
}

std::vector<OString> LibreTranslateProvider::fetchTargetLanguages()
{
    static constexpr std::chrono::seconds TIMEOUT(5);
    // The request blocks the main loop, so an unreachable host should fail fast.
    static constexpr std::chrono::seconds CONNECT_TIMEOUT(2);

    OString aBaseUrl, aApiKey;
    if (!getSettings(aBaseUrl, aApiKey))
        return {};

    HttpRequest aRequest(aBaseUrl + "/languages", TIMEOUT, CONNECT_TIMEOUT);
    prepareRequest(aRequest);
    if (!aRequest.perform().isEmpty())
        return {};

    // The response is an array of objects like
    // {"code": "en", "name": "English", "targets": ["de", "fr", ...]}: the languages
    // each source language can be translated into. With the source detected by the
    // server, every target of any source is a target.
    const std::optional<boost::property_tree::ptree> oRoot
        = parseJson(aRequest.getResponseBody(), "language list");
    if (!oRoot)
        return {};
    std::vector<OString> aLanguages;
    std::map<OString, OString> aServiceCodes;
    for (const auto& rEntry : *oRoot)
    {
        const boost::optional<const boost::property_tree::ptree&> oTargets
            = rEntry.second.get_child_optional("targets");
        if (!oTargets)
            continue;
        for (const auto& rTarget : *oTargets)
        {
            const OString aServiceCode(rTarget.second.get_value<std::string>());
            const OString aCode = toBcp47(aServiceCode);
            if (aCode.isEmpty()
                || std::find(aLanguages.begin(), aLanguages.end(), aCode) != aLanguages.end())
                continue;
            aLanguages.push_back(aCode);
            aServiceCodes[aCode] = aServiceCode;
        }
    }
    m_aServiceCodes = aServiceCodes;
    return aLanguages;
}
}

std::shared_ptr<TranslationProvider> createLibreTranslateProvider()
{
    return std::make_shared<LibreTranslateProvider>();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
