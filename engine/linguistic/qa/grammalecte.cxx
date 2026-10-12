/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// The Grammalecte checker against a server of this test's own, which
// answers one fixed reply: proves the request has the shape the real
// server takes, and that the reply's code point offsets, counted from the
// start of each line, come back as UTF-16 offsets into the paragraph.

#include <sal/config.h>

#include <string>

#include <rtl/string.hxx>
#include <osl/socket.hxx>
#include <osl/thread.hxx>
#include <unotest/bootstrapfixturebase.hxx>
#include <officecfg/Office/Linguistic.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/linguistic2/LinguServiceManager.hpp>
#include <com/sun/star/linguistic2/XProofreader.hpp>
#include <com/sun/star/linguistic2/ProofreadingResult.hpp>

using namespace ::cpo::uno;

namespace
{
class MockServerThread : public ::osl::Thread
{
public:
    MockServerThread()
        : m_aSocketAddr(u"localhost"_ustr, 2023)
    {
    }

    virtual void run()
    {
        if (m_aAcceptorSocket.acceptConnection(m_aStreamSocket) != osl_Socket_Ok)
            return;

        sal_Int32 nTcpNoDelay = sal_Int32(true);
        m_aStreamSocket.setOption(osl_Socket_OptionTcpNoDelay, &nTcpNoDelay, sizeof(nTcpNoDelay),
                                  osl_Socket_LevelTcp);

        // Read until the body announced by Content-Length is complete: the
        // options make the request longer than one read returns.
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

        const std::string aBody
            = nHeaderEnd == std::string::npos ? std::string() : aRequest.substr(nHeaderEnd + 4);
        if (aRequest.find("POST /gc_text/fr ") != 0
            || aRequest.find("Content-Type: application/x-www-form-urlencoded") == std::string::npos
            || aBody.find("text=Ligne%201") != 0 || aBody.find("&options=%7B%22typo%22") == std::string::npos)
        {
            m_bRequestOK = false;
            NotFound();
        }
        else
        {
            m_bRequestOK = true;
            ResponseOK();
        }
    }

    void ResponseOK()
    {
        const OString aBody(
            "{\"program\":\"grammalecte-fr\",\"version\":\"2.3.0\",\"lang\":\"fr\",\"error\":\"\","
            "\"data\":["
            "{\"iParagraph\":1,\"lGrammarErrors\":[{\"nStart\":9,\"nEnd\":15,\"sLineId\":\"#1\","
            "\"sRuleId\":\"typo_test\",\"sType\":\"typo\",\"aColor\":null,\"sMessage\":\"Faute.\","
            "\"aSuggestions\":[\"faute\"],\"URL\":\"https://example.org/rule\"}],"
            "\"lSpellingErrors\":[{\"nStart\":9,\"nEnd\":15,\"sValue\":\"fautte\",\"sType\":\"WORD\"}]},"
            "{\"iParagraph\":2,\"lGrammarErrors\":[{\"nStart\":9,\"nEnd\":15,\"sLineId\":\"#2\","
            "\"sRuleId\":\"gn_test\",\"sType\":\"gn\",\"aColor\":null,\"sMessage\":\"Accord.\","
            "\"aSuggestions\":[\"ligne\",\"lignes\"],\"URL\":\"\"}],\"lSpellingErrors\":[]}"
            "]}"_ostr);
        const OString aResponse = "HTTP/1.1 200 OK\r\n"
                                  "Server: MockServer\r\n"
                                  "Content-Type: application/json\r\n"
                                  "Content-Length: "
                                  + OString::number(aBody.getLength())
                                  + "\r\n"
                                    "\r\n"
                                  + aBody;
        m_aStreamSocket.write(aResponse.getStr(), aResponse.getLength());
        m_aStreamSocket.close();
    }

    void NotFound()
    {
        const OString aResponse("HTTP/1.1 404 Not Found\r\n"
                                "Connection: Closed\r\n"
                                "\r\n"_ostr);
        m_aStreamSocket.write(aResponse.getStr(), aResponse.getLength());
        m_aStreamSocket.close();
    }

    // Started by the test that talks to it, since the port can be bound
    // only once per process.
    void start()
    {
        m_aAcceptorSocket.setOption(osl_Socket_OptionReuseAddr, 1);
        CPPUNIT_ASSERT(m_aAcceptorSocket.bind(m_aSocketAddr));
        CPPUNIT_ASSERT(m_aAcceptorSocket.listen());
        create();
        m_bStarted = true;
        osl::Thread::wait(std::chrono::seconds(1));
    }

    void stop()
    {
        if (!m_bStarted)
            return;
        m_bStarted = false;
        m_aAcceptorSocket.close();
        join();
    }

    bool m_bStarted = false;

    bool m_bRequestOK = false;

private:
    ::osl::SocketAddr m_aSocketAddr;
    ::osl::AcceptorSocket m_aAcceptorSocket;
    ::osl::StreamSocket m_aStreamSocket;
};
}

MockServerThread aMockServer;

class TestGrammalecte : public test::BootstrapFixtureBase
{
public:
    virtual void tearDown() override;

private:
    CPPUNIT_TEST_SUITE(TestGrammalecte);
    CPPUNIT_TEST(testProofreading);
    CPPUNIT_TEST_SUITE_END();

    void testProofreading();
};

void TestGrammalecte::testProofreading()
{
    aMockServer.start();

    using GrammalecteCfg = officecfg::Office::Linguistic::GrammarChecking::Grammalecte;
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    GrammalecteCfg::BaseURL::set("http://127.0.0.1:2023", batch);
    GrammalecteCfg::IsEnabled::set(true, batch);
    GrammalecteCfg::SSLCertVerify::set(false, batch);
    batch->commit();

    Reference<css::linguistic2::XProofreader> xProofreader(
        m_xSFactory->createInstance(u"cpo.lingu.Grammalecte"_ustr), UNO_QUERY);
    CPPUNIT_ASSERT(xProofreader.is());

    const css::lang::Locale aLocale(u"fr"_ustr, u"FR"_ustr, u""_ustr);
    CPPUNIT_ASSERT(xProofreader->hasLocale(css::lang::Locale(u"fr"_ustr, u""_ustr, u""_ustr)));

    // An emoji before the first error, which is one code point and two
    // UTF-16 units, and a second line, whose offsets count from its start.
    const OUString aText = u"Ligne 1\U0001F600 fautte\nDeuxieme lignne"_ustr;
    css::linguistic2::ProofreadingResult aResult
        = xProofreader->doProofreading(u"id"_ustr, aText, aLocale, 0, aText.getLength(), {});

    CPPUNIT_ASSERT(aMockServer.m_bRequestOK);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aResult.aErrors.getLength());

    CPPUNIT_ASSERT_EQUAL(sal_Int32(10), aResult.aErrors[0].nErrorStart);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), aResult.aErrors[0].nErrorLength);
    CPPUNIT_ASSERT_EQUAL(u"typo_test"_ustr, aResult.aErrors[0].aRuleIdentifier);
    CPPUNIT_ASSERT_EQUAL(u"Faute."_ustr, aResult.aErrors[0].aShortComment);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aResult.aErrors[0].aSuggestions.getLength());
    bool bHasURL = false;
    for (const css::beans::PropertyValue& rProperty : aResult.aErrors[0].aProperties)
        bHasURL = bHasURL || rProperty.Name == "FullCommentURL";
    CPPUNIT_ASSERT(bHasURL);

    // Line two starts at code point 16, unit 17: the emoji shifts it by one.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(26), aResult.aErrors[1].nErrorStart);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), aResult.aErrors[1].nErrorLength);
    CPPUNIT_ASSERT_EQUAL(u"gn_test"_ustr, aResult.aErrors[1].aRuleIdentifier);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aResult.aErrors[1].aSuggestions.getLength());

    // Ignoring a rule takes its errors out of the cached answer too.
    xProofreader->ignoreRule(u"typo_test"_ustr, aLocale);
    aResult = xProofreader->doProofreading(u"id"_ustr, aText, aLocale, 0, aText.getLength(), {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aResult.aErrors.getLength());
    CPPUNIT_ASSERT_EQUAL(u"gn_test"_ustr, aResult.aErrors[0].aRuleIdentifier);
}

void TestGrammalecte::tearDown()
{
    aMockServer.stop();

    test::BootstrapFixtureBase::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestGrammalecte);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
