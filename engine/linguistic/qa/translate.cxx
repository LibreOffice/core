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
#include <mutex>
#include <string>
#include <vector>

#include <comphelper/configuration.hxx>
#include <comphelper/scopeguard.hxx>
#include <linguistic/translate.hxx>
#include <officecfg/Office/Linguistic.hxx>
#include <osl/socket.hxx>
#include <osl/thread.hxx>
#include <unotest/bootstrapfixturebase.hxx>

namespace
{
/// Stands in for the translation services: answers each request by its path with a fixed
/// body, so the tests can check what was sent and how the answer is read. Same in-process
/// approach as restprotocol.cxx.
class MockTranslateServer : public ::osl::Thread
{
public:
    MockTranslateServer()
        : m_aSocketAddr(u"localhost"_ustr, 18101)
    {
    }

    virtual void run() override
    {
        while (m_aAcceptorSocket.acceptConnection(m_aStreamSocket) == osl_Socket_Ok)
        {
            // Read the whole request, as far as Content-Length says: closing the socket
            // with part of it unread resets the connection, and the client then reports
            // the reset instead of the answer.
            std::string aRequest;
            size_t nHeaderEnd = std::string::npos;
            size_t nContentLength = 0;
            while (true)
            {
                char aBuffer[1024];
                const sal_Int32 nRead = m_aStreamSocket.recv(aBuffer, sizeof(aBuffer));
                if (nRead <= 0)
                    break;
                aRequest.append(aBuffer, nRead);
                if (nHeaderEnd == std::string::npos)
                {
                    nHeaderEnd = aRequest.find("\r\n\r\n");
                    if (nHeaderEnd == std::string::npos)
                        continue;
                    const size_t nField = aRequest.find("Content-Length: ");
                    if (nField != std::string::npos && nField < nHeaderEnd)
                        nContentLength = std::stoul(aRequest.substr(nField + 16));
                }
                if (aRequest.size() >= nHeaderEnd + 4 + nContentLength)
                    break;
            }
            {
                std::scoped_lock aGuard(m_aMutex);
                m_aRequests.emplace_back(aRequest);
            }

            OString aStatus = "200 OK"_ostr;
            OString aBody;
            if (m_bFail)
            {
                aStatus = m_aFailStatus;
                aBody = m_aFailBody;
            }
            else if (aRequest.starts_with("POST /v2/translate"))
                aBody = "{\"translations\":[{\"detected_source_language\":\"EN\","
                        "\"text\":\"TRANSLATED-TEXT\"}]}"_ostr;
            else if (aRequest.starts_with("GET /v3/languages"))
                aBody = "[{\"lang\":\"de\",\"name\":\"German\",\"usable_as_source\":true,"
                        "\"usable_as_target\":true},"
                        "{\"lang\":\"en\",\"name\":\"English\",\"usable_as_source\":true,"
                        "\"usable_as_target\":false},"
                        "{\"lang\":\"en-GB\",\"name\":\"English (British)\","
                        "\"usable_as_source\":false,\"usable_as_target\":true},"
                        "{\"lang\":\"zh-Hant\",\"name\":\"Chinese (traditional)\","
                        "\"usable_as_source\":false,\"usable_as_target\":true}]"_ostr;
            else
                aStatus = "404 Not Found"_ostr;

            const OString aResponse = "HTTP/1.1 " + aStatus
                                      + "\r\nContent-Type: application/json\r\nContent-Length: "
                                      + OString::number(aBody.getLength()) + "\r\n\r\n" + aBody;
            m_aStreamSocket.write(aResponse.getStr(), aResponse.getLength());
            m_aStreamSocket.close();
        }
    }

    void init()
    {
        m_aAcceptorSocket.setOption(osl_Socket_OptionReuseAddr, 1);
        CPPUNIT_ASSERT(m_aAcceptorSocket.bind(m_aSocketAddr));
        CPPUNIT_ASSERT(m_aAcceptorSocket.listen());
    }

    void stop()
    {
        m_aAcceptorSocket.close();
        join();
    }

    /// Makes every request fail with the given status and body; call before create().
    void setFail(const OString& rStatus, const OString& rBody)
    {
        m_bFail = true;
        m_aFailStatus = rStatus;
        m_aFailBody = rBody;
    }

    /// The requests received so far, each complete with its headers and body.
    std::vector<OString> getRequests()
    {
        std::scoped_lock aGuard(m_aMutex);
        return m_aRequests;
    }

private:
    ::osl::SocketAddr m_aSocketAddr;
    ::osl::AcceptorSocket m_aAcceptorSocket;
    ::osl::StreamSocket m_aStreamSocket;
    std::mutex m_aMutex;
    std::vector<OString> m_aRequests;
    bool m_bFail = false;
    OString m_aFailStatus;
    OString m_aFailBody;
};

using DeeplCfg = officecfg::Office::Linguistic::Translation::Deepl;
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
    MockTranslateServer aMockServer;
    aMockServer.init();
    aMockServer.create();
    comphelper::ScopeGuard aStopServer([&aMockServer] { aMockServer.stop(); });
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
    const std::vector<OString> aRequests = aMockServer.getRequests();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aRequests.size());
    CPPUNIT_ASSERT(aRequests[0].startsWith("POST /v2/translate "));
    CPPUNIT_ASSERT(aRequests[0].indexOf("Authorization: DeepL-Auth-Key translate-key\r\n") != -1);
    CPPUNIT_ASSERT(aRequests[0].endsWith(
        "\r\n\r\ntarget_lang=EN-GB&tag_handling=html&text=%3Cspan%3EHello%3C%2Fspan%3E"));
}

CPPUNIT_TEST_FIXTURE(TranslateTest, testDeeplTranslateFailure)
{
    MockTranslateServer aMockServer;
    aMockServer.init();
    aMockServer.setFail("403 Forbidden"_ostr,
                        "{\"message\":\"Authorization failure, check auth_key\"}"_ostr);
    aMockServer.create();
    comphelper::ScopeGuard aStopServer([&aMockServer] { aMockServer.stop(); });
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
    MockTranslateServer aMockServer;
    aMockServer.init();
    aMockServer.create();
    comphelper::ScopeGuard aStopServer([&aMockServer] { aMockServer.stop(); });
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
    std::vector<OString> aRequests = aMockServer.getRequests();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aRequests.size());
    CPPUNIT_ASSERT(aRequests[0].startsWith("GET /v3/languages?resource=translate_text "));
    CPPUNIT_ASSERT(aRequests[0].indexOf("Authorization: DeepL-Auth-Key languages-key\r\n") != -1);

    // And the service is not asked again for the same configuration:
    CPPUNIT_ASSERT_EQUAL(aExpected.size(), pProvider->getTargetLanguages().size());
    CPPUNIT_ASSERT_EQUAL(aExpected.size(), pProvider->getTargetLanguages(false).size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), aMockServer.getRequests().size());

    // And with an endpoint of another form there is no list, without asking:
    comphelper::ScopeGuard<std::function<void()>> aOtherConfig
        = setDeepl(u"http://127.0.0.1:18101/"_ustr, u"languages-key"_ustr);
    CPPUNIT_ASSERT(pProvider->getTargetLanguages().empty());
    CPPUNIT_ASSERT_EQUAL(size_t(1), aMockServer.getRequests().size());
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
#endif
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
