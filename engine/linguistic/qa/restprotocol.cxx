/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>


#include <sal/log.hxx>
#include <unotest/bootstrapfixturebase.hxx>
#include <unotest/mockhttpserver.hxx>
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
// Stands in for the Duden Korrekturserver: answers a well-formed check,
// with the token, with two findings, and anything else with 404.
test::MockHttpServer::Response dudenHandler(const test::MockHttpServer::Request& rRequest)
{
    test::MockHttpServer::Response aResponse;
    if (rRequest.aMethod != "POST" || rRequest.aPath != "/api/check"
        || rRequest.aHeaders.find("Content-Type: application/json") == std::string::npos
        || rRequest.aHeaders.find("access_token: hcvhcvhcv") == std::string::npos)
    {
        aResponse.nStatus = 404;
        aResponse.aStatusText = "Not Found";
        aResponse.aContentType.clear();
        return aResponse;
    }
    aResponse.aBody
        = "{\"check-positions\":[{\"offset\":15,\"length\":6,\"errorcode\":4711,\"type\":"
          "\"orth\","
          "\"severity\":1,\"proposals\":[\"Entwurf\",\"Entw\u00fcrfe\"]},"
          "{\"offset\":22,\"length\":3,\"errorcode\":8221,\"type\":\"orth\",\"severity\":1}],"
          "\"messages\":{\"4711\":\"Dieses Wort korrigieren?\"}}";
    return aResponse;
}
}

static test::MockHttpServer aMockServer(2022, dudenHandler);

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

    CPPUNIT_ASSERT(aMockServer.start());
}

void TestRestProtocol::tearDown()
{
    aMockServer.stop();

    test::BootstrapFixtureBase::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestRestProtocol);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
