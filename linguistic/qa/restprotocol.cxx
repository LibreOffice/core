/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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

using namespace ::com::sun::star::uno;

namespace
{
class MockServerThread : public ::osl::Thread
{
public:
    MockServerThread()
        : m_aSocketAddr(u"localhost"_ustr, 2022)
    {
    }

    virtual void SAL_CALL run()
    {
        if (m_aAcceptorSocket.acceptConnection(m_aStreamSocket) != osl_Socket_Ok)
        {
            return;
        }

        sal_Int32 nReadBytes;
        Sequence<sal_Int8> aBuffer(512);
        sal_Int32 nTcpNoDelay = sal_Int32(true);
        m_aStreamSocket.setOption(osl_Socket_OptionTcpNoDelay, &nTcpNoDelay, sizeof(nTcpNoDelay),
                                  osl_Socket_LevelTcp);

        nReadBytes = m_aStreamSocket.recv(aBuffer.getArray(), aBuffer.getLength());
        if (nReadBytes)
        {
            std::string aText(reinterpret_cast<const char*>(aBuffer.getConstArray()), nReadBytes);

            if (aText.find("POST /api/check") == std::string::npos)
            {
                NotFound();
            }
            else if (aText.find("Content-Type: application/json") == std::string::npos)
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
            "{\"offset\":22,\"length\":3,\"errorcode\":8221,\"type\":\"orth\",\"severity\":1}]}"_ostr);

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
    using LanguageToolCfg = officecfg::Office::Linguistic::GrammarChecking::LanguageTool;
    auto batch(comphelper::ConfigurationChanges::create());

    LanguageToolCfg::BaseURL::set("http://127.0.0.1:2022/api", batch);
    LanguageToolCfg::Username::set("hcastro", batch);
    LanguageToolCfg::ApiKey::set("hcvhcvhcv", batch);
    LanguageToolCfg::IsEnabled::set(true, batch);
    LanguageToolCfg::SSLCertVerify::set(false, batch);
    LanguageToolCfg::RestProtocol::set("duden", batch);

    batch->commit();

    CPPUNIT_ASSERT_EQUAL(u"duden"_ustr, *LanguageToolCfg::RestProtocol::get());

    Reference<::com::sun::star::linguistic2::XProofreader> xProofreader(
        m_xSFactory->createInstance(u"com.sun.star.linguistic2.Proofreader"_ustr), UNO_QUERY);
    CPPUNIT_ASSERT(xProofreader.is());

    com::sun::star::linguistic2::ProofreadingResult aResult
        = xProofreader->doProofreading(u"id"_ustr, u"ths is a tst"_ustr, aLocale, 0, 0, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aResult.aErrors.getLength());
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
