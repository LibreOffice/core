/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <config_features.h>

#include <algorithm>
#include <string>
#include <vector>

#include <comphelper/configuration.hxx>
#include <comphelper/scopeguard.hxx>
#include <linguistic/translate.hxx>
#include <officecfg/Office/Linguistic.hxx>
#include <unotest/bootstrapfixturebase.hxx>
#include <unotest/mockhttpserver.hxx>

namespace
{
/// Stands in for the translation services: answers each request by its path with a fixed
/// body, so the tests can check what was sent and how the answer is read. With nFailStatus
/// set, every request fails with that status and rFailBody.
std::unique_ptr<test::MockHttpServer> createMockServer(int nFailStatus = 0,
                                                       const std::string& rFailStatusText = "",
                                                       const std::string& rFailBody = "")
{
    return std::make_unique<test::MockHttpServer>(
        18101, [nFailStatus, rFailStatusText, rFailBody](const test::MockHttpServer::Request& rRequest) {
            test::MockHttpServer::Response aResponse;
            if (nFailStatus)
            {
                aResponse.nStatus = nFailStatus;
                aResponse.aStatusText = rFailStatusText;
                aResponse.aBody = rFailBody;
            }
            else if (rRequest.aMethod == "POST" && rRequest.aPath == "/v2/translate")
                aResponse.aBody = "{\"translations\":[{\"detected_source_language\":\"EN\","
                                  "\"text\":\"TRANSLATED-TEXT\"}]}";
            else if (rRequest.aMethod == "GET" && rRequest.aPath.starts_with("/v3/languages"))
                aResponse.aBody = "[{\"lang\":\"de\",\"name\":\"German\",\"usable_as_source\":true,"
                                  "\"usable_as_target\":true},"
                                  "{\"lang\":\"en\",\"name\":\"English\",\"usable_as_source\":true,"
                                  "\"usable_as_target\":false},"
                                  "{\"lang\":\"en-GB\",\"name\":\"English (British)\","
                                  "\"usable_as_source\":false,\"usable_as_target\":true},"
                                  "{\"lang\":\"zh-Hant\",\"name\":\"Chinese (traditional)\","
                                  "\"usable_as_source\":false,\"usable_as_target\":true}]";
            else if (rRequest.aMethod == "POST" && rRequest.aPath == "/translate")
                aResponse.aBody = "{\"translatedText\":\"TRANSLATED-TEXT\"}";
            else if (rRequest.aMethod == "GET" && rRequest.aPath == "/languages")
                aResponse.aBody = "[{\"code\":\"en\",\"name\":\"English\","
                                  "\"targets\":[\"de\",\"en\",\"pt-BR\",\"zt\"]},"
                                  "{\"code\":\"de\",\"name\":\"German\","
                                  "\"targets\":[\"en\",\"fr\"]}]";
            else
            {
                aResponse.nStatus = 404;
                aResponse.aStatusText = "Not Found";
            }
            return aResponse;
        });
}

using DeeplCfg = officecfg::Office::Linguistic::Translation::Deepl;
using LibreTranslateCfg = officecfg::Office::Linguistic::Translation::LibreTranslate;
using ProviderCfg = officecfg::Office::Linguistic::Translation::Provider;

/// Sets up DeepL with the given endpoint and key, and puts the configuration back when the
/// returned guard is gone.
comphelper::ScopeGuard<std::function<void()>>
setDeepl(const OUString& rApiUrl, const OUString& rAuthKey, bool bEnabled = true)
{
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch
        = comphelper::ConfigurationChanges::create();
    DeeplCfg::ApiURL::set(rApiUrl, pBatch);
    DeeplCfg::AuthKey::set(rAuthKey, pBatch);
    DeeplCfg::IsEnabled::set(bEnabled, pBatch);
    pBatch->commit();
    return comphelper::ScopeGuard<std::function<void()>>([] {
        std::shared_ptr<comphelper::ConfigurationChanges> pResetBatch
            = comphelper::ConfigurationChanges::create();
        DeeplCfg::ApiURL::set(std::optional<OUString>(), pResetBatch);
        DeeplCfg::AuthKey::set(std::optional<OUString>(), pResetBatch);
        DeeplCfg::IsEnabled::set(true, pResetBatch);
        ProviderCfg::set(OUString(), pResetBatch);
        pResetBatch->commit();
    });
}

/// Sets up LibreTranslate with the given server and key, and puts the configuration back
/// when the returned guard is gone.
comphelper::ScopeGuard<std::function<void()>> setLibreTranslate(const OUString& rBaseUrl,
                                                                const OUString& rApiKey)
{
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch
        = comphelper::ConfigurationChanges::create();
    LibreTranslateCfg::BaseURL::set(rBaseUrl, pBatch);
    LibreTranslateCfg::ApiKey::set(rApiKey, pBatch);
    LibreTranslateCfg::IsEnabled::set(true, pBatch);
    pBatch->commit();
    return comphelper::ScopeGuard<std::function<void()>>([] {
        std::shared_ptr<comphelper::ConfigurationChanges> pResetBatch
            = comphelper::ConfigurationChanges::create();
        LibreTranslateCfg::BaseURL::set(std::optional<OUString>(), pResetBatch);
        LibreTranslateCfg::ApiKey::set(std::optional<OUString>(), pResetBatch);
        LibreTranslateCfg::IsEnabled::set(false, pResetBatch);
        ProviderCfg::set(OUString(), pResetBatch);
        pResetBatch->commit();
    });
}

void setProvider(const OUString& rProvider)
{
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch
        = comphelper::ConfigurationChanges::create();
    ProviderCfg::set(rProvider, pBatch);
    pBatch->commit();
}

class TranslateTest : public test::BootstrapFixtureBase
{
};

#if HAVE_FEATURE_CURL
CPPUNIT_TEST_FIXTURE(TranslateTest, testDeeplTranslate)
{
    std::unique_ptr<test::MockHttpServer> pMockServer = createMockServer();
    CPPUNIT_ASSERT(pMockServer->start());
    comphelper::ScopeGuard<std::function<void()>> aResetConfig
        = setDeepl(u"http://127.0.0.1:18101/v2/translate"_ustr, u"translate-key"_ustr);

    // Given DeepL set up:
    std::shared_ptr<linguistic::TranslationProvider> pProvider
        = linguistic::getTranslationProvider("deepl");
    CPPUNIT_ASSERT(pProvider);
    CPPUNIT_ASSERT_EQUAL(u"DeepL"_ustr, pProvider->getDisplayName());
    CPPUNIT_ASSERT(pProvider->isEnabled());
    CPPUNIT_ASSERT(pProvider->isConfigured());

    // When translating an HTML fragment to a language given as a BCP 47 tag:
    const bool bCancel = false;
    const linguistic::TranslateResult aResult
        = pProvider->translate("<span>Hello</span>"_ostr, "en-GB"_ostr, bCancel);

    // Then the answer is the text the service returned:
    CPPUNIT_ASSERT_EQUAL("TRANSLATED-TEXT"_ostr, aResult.aText);
    CPPUNIT_ASSERT(aResult.aError.isEmpty());
    // and the request went to the endpoint with the key in the Authorization header, the
    // target language in uppercase, as the API wants it, and the HTML as a form field:
    const std::vector<test::MockHttpServer::Request> aRequests = pMockServer->getRequests();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aRequests.size());
    CPPUNIT_ASSERT_EQUAL(std::string("POST"), aRequests[0].aMethod);
    CPPUNIT_ASSERT_EQUAL(std::string("/v2/translate"), aRequests[0].aPath);
    CPPUNIT_ASSERT(aRequests[0].aHeaders.find("Authorization: DeepL-Auth-Key translate-key\r\n")
                   != std::string::npos);
    CPPUNIT_ASSERT_EQUAL(
        std::string("target_lang=EN-GB&tag_handling=html&text=%3Cspan%3EHello%3C%2Fspan%3E"),
        aRequests[0].aBody);
}

CPPUNIT_TEST_FIXTURE(TranslateTest, testDeeplTranslateFailure)
{
    std::unique_ptr<test::MockHttpServer> pMockServer = createMockServer(
        403, "Forbidden", "{\"message\":\"Authorization failure, check auth_key\"}");
    CPPUNIT_ASSERT(pMockServer->start());
    comphelper::ScopeGuard<std::function<void()>> aResetConfig
        = setDeepl(u"http://127.0.0.1:18101/v2/translate"_ustr, u"wrong-key"_ustr);

    // When the service refuses the request:
    const bool bCancel = false;
    const linguistic::TranslateResult aResult
        = linguistic::getTranslationProvider("deepl")->translate("<span>Hello</span>"_ostr,
                                                                 "de"_ostr, bCancel);

    // Then there is no text, and the error names the status and the service's message:
    CPPUNIT_ASSERT(aResult.aText.isEmpty());
    CPPUNIT_ASSERT_EQUAL("HTTP 403: Authorization failure, check auth_key"_ostr, aResult.aError);
}

CPPUNIT_TEST_FIXTURE(TranslateTest, testDeeplTargetLanguages)
{
    std::unique_ptr<test::MockHttpServer> pMockServer = createMockServer();
    CPPUNIT_ASSERT(pMockServer->start());
    // A key of its own: the language list is kept per endpoint and key.
    comphelper::ScopeGuard<std::function<void()>> aResetConfig
        = setDeepl(u"http://127.0.0.1:18101/v2/translate"_ustr, u"languages-key"_ustr);
    std::shared_ptr<linguistic::TranslationProvider> pProvider
        = linguistic::getTranslationProvider("deepl");

    // When asking which languages the service can translate into:
    std::vector<OString> aLanguages = pProvider->getTargetLanguages();

    // Then the list holds the languages usable as a target, from /v3/languages next to the
    // translate endpoint, asked with the key:
    const std::vector<OString> aExpected{ "de"_ostr, "en-GB"_ostr, "zh-Hant"_ostr };
    CPPUNIT_ASSERT_EQUAL(aExpected.size(), aLanguages.size());
    CPPUNIT_ASSERT(std::equal(aExpected.begin(), aExpected.end(), aLanguages.begin()));
    std::vector<test::MockHttpServer::Request> aRequests = pMockServer->getRequests();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aRequests.size());
    CPPUNIT_ASSERT_EQUAL(std::string("GET"), aRequests[0].aMethod);
    CPPUNIT_ASSERT_EQUAL(std::string("/v3/languages?resource=translate_text"), aRequests[0].aPath);
    CPPUNIT_ASSERT(aRequests[0].aHeaders.find("Authorization: DeepL-Auth-Key languages-key\r\n")
                   != std::string::npos);

    // And the service is not asked again for the same configuration:
    CPPUNIT_ASSERT_EQUAL(aExpected.size(), pProvider->getTargetLanguages().size());
    CPPUNIT_ASSERT_EQUAL(aExpected.size(), pProvider->getTargetLanguages(false).size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), pMockServer->getRequests().size());

    // And with an endpoint of another form there is no list, without asking:
    comphelper::ScopeGuard<std::function<void()>> aOtherConfig
        = setDeepl(u"http://127.0.0.1:18101/"_ustr, u"languages-key"_ustr);
    CPPUNIT_ASSERT(pProvider->getTargetLanguages().empty());
    CPPUNIT_ASSERT_EQUAL(size_t(1), pMockServer->getRequests().size());
}

CPPUNIT_TEST_FIXTURE(TranslateTest, testActiveProvider)
{
    // Given nothing set up, there is no provider to translate with:
    CPPUNIT_ASSERT(!linguistic::getActiveTranslationProvider());

    // Given DeepL set up, it is the one, as the first configured provider:
    comphelper::ScopeGuard<std::function<void()>> aResetConfig
        = setDeepl(u"http://127.0.0.1:18101/v2/translate"_ustr, u"active-key"_ustr);
    std::shared_ptr<linguistic::TranslationProvider> pActive
        = linguistic::getActiveTranslationProvider();
    CPPUNIT_ASSERT(pActive);
    CPPUNIT_ASSERT_EQUAL("deepl"_ostr, pActive->getId());

    // And when it is asked for by name, in whatever spelling:
    setProvider(u" DeepL "_ustr);
    pActive = linguistic::getActiveTranslationProvider();
    CPPUNIT_ASSERT(pActive);
    CPPUNIT_ASSERT_EQUAL("deepl"_ostr, pActive->getId());

    // But a provider nobody has heard of translates nothing, DeepL or not:
    setProvider(u"babelfish"_ustr);
    CPPUNIT_ASSERT(!linguistic::getActiveTranslationProvider());

    // And a provider that is switched off is not configured, so there is none:
    setProvider(OUString());
    comphelper::ScopeGuard<std::function<void()>> aDisabledConfig
        = setDeepl(u"http://127.0.0.1:18101/v2/translate"_ustr, u"active-key"_ustr, false);
    CPPUNIT_ASSERT(!linguistic::getTranslationProvider("deepl")->isConfigured());
    CPPUNIT_ASSERT(!linguistic::getActiveTranslationProvider());
    setProvider(u"deepl"_ustr);
    CPPUNIT_ASSERT(!linguistic::getActiveTranslationProvider());
}

CPPUNIT_TEST_FIXTURE(TranslateTest, testLibreTranslateTranslate)
{
    std::unique_ptr<test::MockHttpServer> pMockServer = createMockServer();
    CPPUNIT_ASSERT(pMockServer->start());
    // The translate endpoint pasted instead of the root, with a slash on top: both are
    // taken for the root.
    comphelper::ScopeGuard<std::function<void()>> aResetConfig
        = setLibreTranslate(u"http://127.0.0.1:18101/translate/"_ustr, u"translate-key"_ustr);

    // Given LibreTranslate set up:
    std::shared_ptr<linguistic::TranslationProvider> pProvider
        = linguistic::getTranslationProvider("libretranslate");
    CPPUNIT_ASSERT(pProvider);
    CPPUNIT_ASSERT_EQUAL(u"LibreTranslate"_ustr, pProvider->getDisplayName());
    CPPUNIT_ASSERT(pProvider->isConfigured());

    // When translating an HTML fragment:
    const bool bCancel = false;
    const linguistic::TranslateResult aResult
        = pProvider->translate("<span>Hello</span>"_ostr, "de-AT"_ostr, bCancel);

    // Then the answer is the text the service returned:
    CPPUNIT_ASSERT_EQUAL("TRANSLATED-TEXT"_ostr, aResult.aText);
    CPPUNIT_ASSERT(aResult.aError.isEmpty());
    // and the request was a JSON document posted to /translate of the root, with the
    // source left to the server, the html format and the key, and the language the
    // server knows, with no language list to go by:
    const std::vector<test::MockHttpServer::Request> aRequests = pMockServer->getRequests();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aRequests.size());
    CPPUNIT_ASSERT_EQUAL(std::string("POST"), aRequests[0].aMethod);
    CPPUNIT_ASSERT_EQUAL(std::string("/translate"), aRequests[0].aPath);
    CPPUNIT_ASSERT(aRequests[0].aHeaders.find("Content-Type: application/json\r\n")
                   != std::string::npos);
    CPPUNIT_ASSERT_EQUAL(
        std::string("{ \"q\": \"<span>Hello<\\/span>\", \"source\": \"auto\", \"target\": \"de\", "
                    "\"format\": \"html\", \"api_key\": \"translate-key\"}"),
        aRequests[0].aBody);
}

CPPUNIT_TEST_FIXTURE(TranslateTest, testLibreTranslateNoKey)
{
    std::unique_ptr<test::MockHttpServer> pMockServer = createMockServer();
    CPPUNIT_ASSERT(pMockServer->start());
    comphelper::ScopeGuard<std::function<void()>> aResetConfig
        = setLibreTranslate(u"http://127.0.0.1:18101"_ustr, OUString());

    // Given a server that wants no key, the provider is set up without one:
    std::shared_ptr<linguistic::TranslationProvider> pProvider
        = linguistic::getTranslationProvider("libretranslate");
    CPPUNIT_ASSERT(pProvider->isConfigured());

    // When translating to traditional Chinese:
    const bool bCancel = false;
    const linguistic::TranslateResult aResult
        = pProvider->translate("<span>Hello</span>"_ostr, "zh-Hant"_ostr, bCancel);

    // Then the request has no api_key member, and the server's own code for the language:
    CPPUNIT_ASSERT_EQUAL("TRANSLATED-TEXT"_ostr, aResult.aText);
    const std::vector<test::MockHttpServer::Request> aRequests = pMockServer->getRequests();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aRequests.size());
    CPPUNIT_ASSERT_EQUAL(std::string::npos, aRequests[0].aBody.find("api_key"));
    CPPUNIT_ASSERT(aRequests[0].aBody.find("\"target\": \"zt\"") != std::string::npos);
}

CPPUNIT_TEST_FIXTURE(TranslateTest, testLibreTranslateFailure)
{
    std::unique_ptr<test::MockHttpServer> pMockServer
        = createMockServer(400, "Bad Request", "{\"error\":\"de is not supported\"}");
    CPPUNIT_ASSERT(pMockServer->start());
    comphelper::ScopeGuard<std::function<void()>> aResetConfig
        = setLibreTranslate(u"http://127.0.0.1:18101"_ustr, OUString());

    // When the server refuses the request:
    const bool bCancel = false;
    const linguistic::TranslateResult aResult
        = linguistic::getTranslationProvider("libretranslate")
              ->translate("<span>Hello</span>"_ostr, "de"_ostr, bCancel);

    // Then there is no text, and the error names the status and the server's message:
    CPPUNIT_ASSERT(aResult.aText.isEmpty());
    CPPUNIT_ASSERT_EQUAL("HTTP 400: de is not supported"_ostr, aResult.aError);
}

CPPUNIT_TEST_FIXTURE(TranslateTest, testLibreTranslateTargetLanguages)
{
    std::unique_ptr<test::MockHttpServer> pMockServer = createMockServer();
    CPPUNIT_ASSERT(pMockServer->start());
    comphelper::ScopeGuard<std::function<void()>> aResetConfig
        = setLibreTranslate(u"http://127.0.0.1:18101"_ustr, u"languages-key"_ustr);
    std::shared_ptr<linguistic::TranslationProvider> pProvider
        = linguistic::getTranslationProvider("libretranslate");

    // When asking which languages the server can translate into:
    std::vector<OString> aLanguages = pProvider->getTargetLanguages();

    // Then the list is the targets of every source, each once, as BCP 47 tags:
    const std::vector<OString> aExpected{ "de"_ostr, "en"_ostr, "pt-BR"_ostr, "zh-Hant"_ostr,
                                          "fr"_ostr };
    CPPUNIT_ASSERT_EQUAL(aExpected.size(), aLanguages.size());
    CPPUNIT_ASSERT(std::equal(aExpected.begin(), aExpected.end(), aLanguages.begin()));
    std::vector<test::MockHttpServer::Request> aRequests = pMockServer->getRequests();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aRequests.size());
    CPPUNIT_ASSERT_EQUAL(std::string("GET"), aRequests[0].aMethod);
    CPPUNIT_ASSERT_EQUAL(std::string("/languages"), aRequests[0].aPath);

    // And when translating with the list known, the language goes as the server lists it:
    // the whole tag where the server has it, else the language alone.
    const bool bCancel = false;
    pProvider->translate("<span>Hello</span>"_ostr, "pt-br"_ostr, bCancel);
    pProvider->translate("<span>Hello</span>"_ostr, "fr-CA"_ostr, bCancel);
    // Traditional Chinese goes by the code the server listed it under.
    pProvider->translate("<span>Hello</span>"_ostr, "zh-Hant"_ostr, bCancel);
    aRequests = pMockServer->getRequests();
    CPPUNIT_ASSERT_EQUAL(size_t(4), aRequests.size());
    CPPUNIT_ASSERT(aRequests[1].aBody.find("\"target\": \"pt-BR\"") != std::string::npos);
    CPPUNIT_ASSERT(aRequests[2].aBody.find("\"target\": \"fr\"") != std::string::npos);
    CPPUNIT_ASSERT(aRequests[3].aBody.find("\"target\": \"zt\"") != std::string::npos);
}

CPPUNIT_TEST_FIXTURE(TranslateTest, testProviderOrder)
{
    // Given both providers set up, DeepL comes first:
    comphelper::ScopeGuard<std::function<void()>> aResetDeepl
        = setDeepl(u"http://127.0.0.1:18101/v2/translate"_ustr, u"order-key"_ustr);
    comphelper::ScopeGuard<std::function<void()>> aResetLibreTranslate
        = setLibreTranslate(u"http://127.0.0.1:18101"_ustr, OUString());
    std::shared_ptr<linguistic::TranslationProvider> pActive
        = linguistic::getActiveTranslationProvider();
    CPPUNIT_ASSERT(pActive);
    CPPUNIT_ASSERT_EQUAL("deepl"_ostr, pActive->getId());

    // Unless LibreTranslate is asked for:
    setProvider(u"libretranslate"_ustr);
    pActive = linguistic::getActiveTranslationProvider();
    CPPUNIT_ASSERT(pActive);
    CPPUNIT_ASSERT_EQUAL("libretranslate"_ostr, pActive->getId());

    // And a provider asked for but not set up is not swapped for the other:
    comphelper::ScopeGuard<std::function<void()>> aNoServer
        = setLibreTranslate(OUString(), OUString());
    CPPUNIT_ASSERT(!linguistic::getActiveTranslationProvider());
}
#endif
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
