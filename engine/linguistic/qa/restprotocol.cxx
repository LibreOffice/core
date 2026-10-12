/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <algorithm>
#include <cassert>
#include <cstring>

#include <sal/log.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#include <osl/socket.hxx>
#include <osl/thread.hxx>
#include <unotest/bootstrapfixturebase.hxx>
#include <officecfg/Office/Linguistic.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/connection/XAcceptor.hpp>
#include <com/sun/star/connection/XConnector.hpp>
#include <com/sun/star/linguistic2/XProofreader.hpp>
#include <com/sun/star/linguistic2/ProofreadingResult.hpp>

using namespace ::cpo::uno;
using namespace ::cpo::uno;

namespace
{
class MockServerThread : public ::osl::Thread
{
public:
    MockServerThread()
        : m_aSocketAddr(u"localhost"_ustr, 2022)
    {
    }

    virtual void run()
    {
        if (m_aAcceptorSocket.acceptConnection(m_aStreamSocket) != osl_Socket_Ok)
        {
            return;
        }

        sal_Int32 nTcpNoDelay = sal_Int32(true);
        m_aStreamSocket.setOption(osl_Socket_OptionTcpNoDelay, &nTcpNoDelay, sizeof(nTcpNoDelay),
                                  osl_Socket_LevelTcp);

        // Read the whole request, as far as Content-Length says: closing
        // the socket with part of it unread resets the connection, and the
        // client then reports the reset instead of the answer.
        std::string aText;
        size_t nHeaderEnd = std::string::npos;
        size_t nContentLength = 0;
        while (true)
        {
            char aBuffer[1024];
            const sal_Int32 nRead = m_aStreamSocket.recv(aBuffer, sizeof(aBuffer));
            if (nRead <= 0)
                break;
            aText.append(aBuffer, nRead);
            if (nHeaderEnd == std::string::npos)
            {
                nHeaderEnd = aText.find("\r\n\r\n");
                if (nHeaderEnd == std::string::npos)
                    continue;
                const size_t nField = aText.find("Content-Length: ");
                if (nField != std::string::npos && nField < nHeaderEnd)
                    nContentLength = std::stoul(aText.substr(nField + 16));
            }
            if (aText.size() >= nHeaderEnd + 4 + nContentLength)
                break;
        }
        if (!aText.empty())
        {

            if (aText.find("POST /api/check") == std::string::npos)
            {
                NotFound();
            }
            else if (aText.find("Content-Type: application/json") == std::string::npos)
            {
                NotFound();
            }
            else if (aText.find("access_token: hcvhcvhcv") == std::string::npos)
            {
                NotFound();
            }
            else
            {
                ResponseOK();
            }
        }
    }

    void ResponseOK()
    {
        OString aResponse(
            "HTTP/1.1 200 OK\r\n"
            "Server: MockServer\r\n"
            "Cache-Control: no-cache\r\n"
            "Content-Type: application/json\r\n"
            "\r\n"
            "{\"check-positions\":[{\"offset\":15,\"length\":6,\"errorcode\":4711,\"type\":"
            "\"orth\","
            "\"severity\":1,\"proposals\":[\"Entwurf\",\"Entw\u00fcrfe\"]},"
            "{\"offset\":22,\"length\":3,\"errorcode\":8221,\"type\":\"orth\",\"severity\":1}],"
            "\"messages\":{\"4711\":\"Dieses Wort korrigieren?\"}}"_ostr);

        m_aStreamSocket.write(aResponse.getStr(), aResponse.getLength());
        m_aStreamSocket.close();
    }

    void NotFound()
    {
        OString aResponse("HTTP/1.1 404 Not Found\r\n"
                          "Connection: Closed\r\n"
                          "\r\n"_ostr);

        m_aStreamSocket.write(aResponse.getStr(), aResponse.getLength());
        m_aStreamSocket.close();
    }

    void stop()
    {
        m_aAcceptorSocket.close();
        join();
    }

    void init()
    {
        m_aAcceptorSocket.setOption(osl_Socket_OptionReuseAddr, 1);
        CPPUNIT_ASSERT(m_aAcceptorSocket.bind(m_aSocketAddr));
        CPPUNIT_ASSERT(m_aAcceptorSocket.listen());
    }

private:
    ::osl::SocketAddr m_aSocketAddr;
    ::osl::AcceptorSocket m_aAcceptorSocket;
    ::osl::StreamSocket m_aStreamSocket;
};
}

MockServerThread aMockServer;

class TestRestProtocol : public test::BootstrapFixtureBase
{
public:
    virtual void setUp() override;
    virtual void tearDown() override;

private:
    CPPUNIT_TEST_SUITE(TestRestProtocol);
    CPPUNIT_TEST(testProofreading);
    CPPUNIT_TEST_SUITE_END();

    void testProofreading();
};

void TestRestProtocol::testProofreading()
{
    css::lang::Locale aLocale(u"en"_ustr, u"US"_ustr, u""_ustr);
    using DudenCfg = officecfg::Office::Linguistic::GrammarChecking::Duden;
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());

    DudenCfg::BaseURL::set("http://127.0.0.1:2022/api", batch);
    DudenCfg::ApiKey::set("hcvhcvhcv", batch);
    DudenCfg::IsEnabled::set(true, batch);
    DudenCfg::SSLCertVerify::set(false, batch);

    batch->commit();

    // By implementation name: more than one service implements the
    // proofreader, and this is about the Duden one.
    Reference<css::linguistic2::XProofreader> xProofreader(
        m_xSFactory->createInstance(u"cpo.lingu.Duden"_ustr), UNO_QUERY);
    CPPUNIT_ASSERT(xProofreader.is());

    css::linguistic2::ProofreadingResult aResult
        = xProofreader->doProofreading(u"id"_ustr, u"ths is a tst"_ustr, aLocale, 0, 0, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aResult.aErrors.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(15), aResult.aErrors[0].nErrorStart);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), aResult.aErrors[0].nErrorLength);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aResult.aErrors[0].aSuggestions.getLength());
    CPPUNIT_ASSERT_EQUAL(u"Entwurf"_ustr, aResult.aErrors[0].aSuggestions[0]);
    // The error code is the rule to ignore.
    CPPUNIT_ASSERT_EQUAL(u"4711"_ustr, aResult.aErrors[0].aRuleIdentifier);
    // The server's message for the code, when it sent one.
    CPPUNIT_ASSERT_EQUAL(u"Dieses Wort korrigieren?"_ustr, aResult.aErrors[0].aShortComment);
    CPPUNIT_ASSERT(aResult.aErrors[1].aShortComment != "Dieses Wort korrigieren?");
}

void TestRestProtocol::setUp()
{
    test::BootstrapFixtureBase::setUp();

    aMockServer.init();
    aMockServer.create();
    osl::Thread::wait(std::chrono::seconds(1));
}

void TestRestProtocol::tearDown()
{
    aMockServer.stop();

    test::BootstrapFixtureBase::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestRestProtocol);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
