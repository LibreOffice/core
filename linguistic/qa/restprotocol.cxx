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
#include <svtools/languagetoolcfg.hxx>
#include <unotest/bootstrapfixturebase.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/connection/XAcceptor.hpp>
#include <com/sun/star/connection/XConnector.hpp>
#include <com/sun/star/linguistic2/XProofreader.hpp>
#include <com/sun/star/linguistic2/ProofreadingResult.hpp>

using namespace ::com::sun::star::uno;

class MockServerThread : public ::osl::Thread
{
public:
    MockServerThread() :
        m_aSocketAddr("localhost", 2022)
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
        m_aStreamSocket.setOption(osl_Socket_OptionTcpNoDelay, &nTcpNoDelay,
                                  sizeof(nTcpNoDelay), osl_Socket_LevelTcp);

        nReadBytes = m_aStreamSocket.recv(aBuffer.getArray(),
                                          aBuffer.getLength());
        if (nReadBytes)
        {
            std::string aText(reinterpret_cast<const char*>(aBuffer.getConstArray()),
                              nReadBytes);

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
        OStringBuffer aResponse;

        aResponse.append("HTTP/1.1 200 OK\r\n");
        aResponse.append("Server: MockServer\r\n");
        aResponse.append("Cache-Control: no-cache\r\n");
        aResponse.append("Content-Type: application/json\r\n");

        aResponse.append("\r\n");
        aResponse.append("{\"check-positions\":[{\"offset\":15,\"length\":6,\"errorcode\":4711,\"type\":\"orth\","
                         "\"severity\":1,\"proposals\":[\"Entwurf\",\"Entw\u00fcrfe\"]},"
                         "{\"offset\":22,\"length\":3,\"errorcode\":8221,\"type\":\"orth\",\"severity\":1}]}");

        m_aStreamSocket.write(aResponse.getStr(), aResponse.getLength());
        m_aStreamSocket.close();
    }

    void NotFound()
    {
        OStringBuffer aResponse;

        aResponse.append("HTTP/1.1 404 Not Found\r\n");
        aResponse.append("Connection: Closed\r\n");
        aResponse.append("\r\n");

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

MockServerThread aMockServer;

class TestRestProtocol: public test::BootstrapFixtureBase {
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
    css::lang::Locale aLocale("en", "US", "");
    Sequence<::com::sun::star::beans::PropertyValue> aProperties;
    SvxLanguageToolOptions& rLanguageOpts = SvxLanguageToolOptions::Get();
    rLanguageOpts.setBaseURL("http://127.0.0.1:2022/api");
    rLanguageOpts.setUsername("hcastro");
    rLanguageOpts.setApiKey("hcvhcvhcv");
    rLanguageOpts.setEnabled(true);
    rLanguageOpts.setSSLVerification(false);
    rLanguageOpts.setRestProtocol("duden");
    CPPUNIT_ASSERT_EQUAL(OUString("duden"), rLanguageOpts.getRestProtocol());

    Reference<::com::sun::star::linguistic2::XProofreader> xProofreader (
        m_xSFactory->createInstance("com.sun.star.linguistic2.Proofreader"), UNO_QUERY);
    CPPUNIT_ASSERT(xProofreader.is());

    com::sun::star::linguistic2::ProofreadingResult aResult =
        xProofreader->doProofreading(OUString("id"),
                                     OUString("ths is a tst"),
                                     aLocale,
                                     0,
                                     0,
                                     aProperties);

    CPPUNIT_ASSERT_EQUAL(2, aResult.aErrors.getLength());
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
