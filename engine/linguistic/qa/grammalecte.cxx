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

#include <chrono>
#include <string>

#include <rtl/string.hxx>
#include <osl/socket.hxx>
#include <unotest/bootstrapfixturebase.hxx>
#include <unotest/mockhttpserver.hxx>
#include <officecfg/Office/Linguistic.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/linguistic2/LinguServiceManager.hpp>
#include <com/sun/star/linguistic2/XProofreader.hpp>
#include <com/sun/star/linguistic2/ProofreadingResult.hpp>

using namespace ::cpo::uno;

namespace
{
// The request the checker is expected to send: the text and the options as
// form fields.
bool isCheckRequest(const test::MockHttpServer::Request& rRequest)
{
    return rRequest.aMethod == "POST" && rRequest.aPath == "/gc_text/fr"
           && rRequest.aHeaders.find("Content-Type: application/x-www-form-urlencoded")
                  != std::string::npos
           && rRequest.aBody.find("text=Ligne%201") == 0
           && rRequest.aBody.find("&options=%7B%22typo%22") != std::string::npos;
}

// Stands in for a Grammalecte server: answers the check with a finding on
// each of two paragraphs, and anything else with 404.
test::MockHttpServer::Response grammalecteHandler(const test::MockHttpServer::Request& rRequest)
{
    test::MockHttpServer::Response aResponse;
    if (!isCheckRequest(rRequest))
    {
        aResponse.nStatus = 404;
        aResponse.aStatusText = "Not Found";
        aResponse.aContentType.clear();
        return aResponse;
    }
    aResponse.aBody
        = "{\"program\":\"grammalecte-fr\",\"version\":\"2.3.0\",\"lang\":\"fr\",\"error\":\"\","
          "\"data\":["
          "{\"iParagraph\":1,\"lGrammarErrors\":[{\"nStart\":9,\"nEnd\":15,\"sLineId\":\"#1\","
          "\"sRuleId\":\"typo_test\",\"sType\":\"typo\",\"aColor\":null,\"sMessage\":\"Faute.\","
          "\"aSuggestions\":[\"faute\"],\"URL\":\"https://example.org/rule\"}],"
          "\"lSpellingErrors\":[{\"nStart\":9,\"nEnd\":15,\"sValue\":\"fautte\",\"sType\":\"WORD\"}]},"
          "{\"iParagraph\":2,\"lGrammarErrors\":[{\"nStart\":9,\"nEnd\":15,\"sLineId\":\"#2\","
          "\"sRuleId\":\"gn_test\",\"sType\":\"gn\",\"aColor\":null,\"sMessage\":\"Accord.\","
          "\"aSuggestions\":[\"ligne\",\"lignes\"],\"URL\":\"\"}],\"lSpellingErrors\":[]}"
          "]}";
    return aResponse;
}
}


class TestGrammalecte : public test::BootstrapFixtureBase
{
private:
    CPPUNIT_TEST_SUITE(TestGrammalecte);
    CPPUNIT_TEST(testProofreading);
    CPPUNIT_TEST(testStaysConfiguredWhileOff);
    CPPUNIT_TEST(testProbeGivesUp);
    CPPUNIT_TEST_SUITE_END();

    void testProofreading();
    void testStaysConfiguredWhileOff();
    void testProbeGivesUp();
};

// Asking a server whether it is there is waited for, so it gives up sooner
// than a check does: a server that never answers is reported as not there
// within the probe's limits, 3 seconds for the connection and 5 in all,
// not the check's 10.
void TestGrammalecte::testProbeGivesUp()
{
    // Listens and never accepts: the connection waits in the backlog or
    // never completes, and no answer comes either way.
    osl::AcceptorSocket aSilent;
    aSilent.setOption(osl_Socket_OptionReuseAddr, 1);
    CPPUNIT_ASSERT(aSilent.bind(osl::SocketAddr(u"localhost"_ustr, 2024)));
    CPPUNIT_ASSERT(aSilent.listen());

    using GrammalecteCfg = officecfg::Office::Linguistic::GrammarChecking::Grammalecte;
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    GrammalecteCfg::BaseURL::set(u"http://127.0.0.1:2024"_ustr, batch);
    GrammalecteCfg::IsEnabled::set(true, batch);
    batch->commit();

    Reference<css::beans::XPropertyAccess> xChecker(
        m_xSFactory->createInstance(u"cpo.lingu.Grammalecte"_ustr), UNO_QUERY);
    CPPUNIT_ASSERT(xChecker.is());

    const std::chrono::steady_clock::time_point aStart = std::chrono::steady_clock::now();
    const Sequence<css::beans::PropertyValue> aAnswer = xChecker->getPropertyValues();
    const std::chrono::steady_clock::duration aTaken = std::chrono::steady_clock::now() - aStart;

    bool bReachable = true;
    for (const css::beans::PropertyValue& rValue : aAnswer)
    {
        if (rValue.Name == "IsReachable")
            rValue.Value >>= bReachable;
    }
    CPPUNIT_ASSERT(!bReachable);
    CPPUNIT_ASSERT(aTaken < std::chrono::seconds(8));
    aSilent.close();
}

// A checker that is off when the service manager starts reports no locale,
// which must not cost it its place in the list of checkers for French: the
// list is what the iterator asks at check time, and the checker may be
// switched on by then.
void TestGrammalecte::testStaysConfiguredWhileOff()
{
    using GrammalecteCfg = officecfg::Office::Linguistic::GrammarChecking::Grammalecte;
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    GrammalecteCfg::BaseURL::set(u""_ustr, batch);
    GrammalecteCfg::IsEnabled::set(false, batch);
    batch->commit();

    Reference<css::linguistic2::XProofreader> xProofreader(
        m_xSFactory->createInstance(u"cpo.lingu.Grammalecte"_ustr), UNO_QUERY);
    CPPUNIT_ASSERT(xProofreader.is());
    const css::lang::Locale aFrench(u"fr"_ustr, u""_ustr, u""_ustr);
    CPPUNIT_ASSERT(!xProofreader->hasLocale(aFrench));

    Reference<css::linguistic2::XLinguServiceManager2> xManager
        = css::linguistic2::LinguServiceManager::create(m_xContext);
    const Sequence<OUString> aConfigured
        = xManager->getConfiguredServices(u"com.sun.star.linguistic2.Proofreader"_ustr, aFrench);
    bool bListed = false;
    for (const OUString& rName : aConfigured)
        bListed = bListed || rName == "cpo.lingu.Grammalecte";
    CPPUNIT_ASSERT(bListed);

    // Switched on, it serves French at once.
    batch = comphelper::ConfigurationChanges::create();
    GrammalecteCfg::BaseURL::set(u"http://127.0.0.1:2023"_ustr, batch);
    GrammalecteCfg::IsEnabled::set(true, batch);
    batch->commit();
    CPPUNIT_ASSERT(xProofreader->hasLocale(aFrench));
}

void TestGrammalecte::testProofreading()
{
    test::MockHttpServer aMockServer(2023, grammalecteHandler);
    CPPUNIT_ASSERT(aMockServer.start());

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

    const std::vector<test::MockHttpServer::Request> aRequests = aMockServer.getRequests();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aRequests.size());
    CPPUNIT_ASSERT(isCheckRequest(aRequests[0]));
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

CPPUNIT_TEST_SUITE_REGISTRATION(TestGrammalecte);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
