/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/ResultEvent.hpp>
#include <com/sun/star/sheet/XAddIn.hpp>
#include <com/sun/star/sheet/XResultListener.hpp>
#include <com/sun/star/sheet/XVolatileResult.hpp>
#include <org/collaboraoffice/sheet/addin/XPythonComputeFunctions.hpp>
#include <cpo/uno/Reference.hxx>
#include <cpo/uno/TypeClass.hpp>
#include <cpo/uno/XInterface.hpp>

#include <osl/process.h>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase.hxx>
#include <formula/errorcodes.hxx>
#include <o3tl/safeint.hxx>
#include <rtl/ref.hxx>
#include <vcl/svapp.hxx>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <sstream>
#include <limits>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <anyjson.hxx>
#include <bridge.hxx>
#include <bridge.h>

using namespace css;
using namespace cpo;

namespace
{
class CapturingResultListener final : public cppu::WeakImplHelper<sheet::XResultListener>
{
public:
    void modified(const sheet::ResultEvent& aEvent) override { m_aValues.push_back(aEvent.Value); }

    void disposing(const lang::EventObject&) override {}

    sal_Int32 count() const
    {
        return static_cast<sal_Int32>(m_aValues.size());
    }

    cpo::uno::Any at(sal_Int32 nIndex) const
    {
        CPPUNIT_ASSERT(nIndex >= 0);
        CPPUNIT_ASSERT(o3tl::make_unsigned(nIndex) < m_aValues.size());
        return m_aValues[o3tl::make_unsigned(nIndex)];
    }

    cpo::uno::Any last() const
    {
        CPPUNIT_ASSERT(!m_aValues.empty());
        return m_aValues.back();
    }

private:
    std::vector<cpo::uno::Any> m_aValues;
};

std::vector<std::string> g_aTaken;
int g_nRefuseAfter = -1; // -1: accept all
int g_nWakeups = 0;

int testRequestCb(void* /*userdata*/, const char* jsonUtf8, int32_t len)
{
    // The kit's callback writes a socket: never under Solar. (An assert would be swallowed
    // by take_requests; refusing makes the caller's delivery count fail instead.)
    if (Application::GetSolarMutex().IsCurrentThread())
        return 0;
    if (g_nRefuseAfter >= 0 && static_cast<int>(g_aTaken.size()) >= g_nRefuseAfter)
        return 0;
    g_aTaken.emplace_back(jsonUtf8, static_cast<size_t>(len));
    return 1;
}

void testWakeup() { ++g_nWakeups; }

/** Drains the AddIn outbox like the kit does and returns the last request (or ""). */
using co::pythoncompute::ResultKind;

std::string takeLastEmitJson()
{
    g_aTaken.clear();
    SolarMutexReleaser aReleaser; // as in the kit
    pythoncompute_take_requests(testRequestCb, nullptr);
    return g_aTaken.empty() ? std::string() : g_aTaken.back();
}

int completeStr(const std::string& s)
{
    return pythoncompute_complete(s.data(), static_cast<int32_t>(s.size()));
}

std::string reply(std::u16string_view sId, std::string_view rest)
{
    return "{\"id\":\"" + std::string(OUStringToOString(sId, RTL_TEXTENCODING_UTF8)) + "\","
           + std::string(rest) + "}";
}

/** A top-level string field of a request, decoded by a real JSON parser. */
OUString requestField(std::string_view json, const char* pKey)
{
    std::stringstream aStream{ std::string(json) };
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);
    return OUString::fromUtf8(aTree.get<std::string>(pKey));
}

/** The id of the one request the AddIn queued since the last take; fails if none. */
OUString takeEmittedId()
{
    const std::string json = takeLastEmitJson();
    CPPUNIT_ASSERT_MESSAGE("expected a queued request", !json.empty());
    return requestField(json, "id");
}

class Test : public test::BootstrapFixture
{
public:
    virtual void setUp() override;
    virtual void tearDown() override;

protected:
    cpo::uno::Reference<org::collaboraoffice::sheet::addin::XPythonComputeFunctions> mxPy;
    cpo::uno::Reference<css::sheet::XAddIn> mxAddIn;
};

/// The Online kit sets COOL_PYTHON_COMPUTE when it brokers =PY() (kit/Kit.cpp).
void setKitBrokers(bool bOn)
{
    OUString aName(u"COOL_PYTHON_COMPUTE"_ustr);
    if (bOn)
    {
        OUString aValue(u"1"_ustr);
        osl_setEnvironment(aName.pData, aValue.pData);
    }
    else
        osl_clearEnvironment(aName.pData);
}

void Test::setUp()
{
    test::BootstrapFixture::setUp();
    setKitBrokers(true);
    pythoncompute_reset_for_tests();
    g_nRefuseAfter = -1;
    g_nWakeups = 0;
    takeLastEmitJson();
    auto xFactory(comphelper::getProcessServiceFactory());
    auto xInst
        = xFactory->createInstance(u"org.collaboraoffice.sheet.addin.PythonComputeFunctions"_ustr);
    mxPy.set(xInst, cpo::uno::UNO_QUERY_THROW);
    mxAddIn.set(xInst, cpo::uno::UNO_QUERY_THROW);
}

void Test::tearDown()
{
    pythoncompute_reset_for_tests();
    takeLastEmitJson();
    mxAddIn.clear();
    mxPy.clear();
    setKitBrokers(false);
    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_FIXTURE(Test, test_emptyCodeThrows)
{
    CPPUNIT_ASSERT_THROW(mxPy->getPy(u""_ustr, {}), lang::IllegalArgumentException);
    CPPUNIT_ASSERT_THROW(mxPy->getPython(u""_ustr, {}), lang::IllegalArgumentException);
}

CPPUNIT_TEST_FIXTURE(Test, test_displayNames)
{
    CPPUNIT_ASSERT_EQUAL(u"getPy"_ustr, mxAddIn->getProgrammaticFuntionName(u"PY"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"getPython"_ustr, mxAddIn->getProgrammaticFuntionName(u"PYTHON"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"PY"_ustr, mxAddIn->getDisplayFunctionName(u"getPy"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"PYTHON"_ustr, mxAddIn->getDisplayFunctionName(u"getPython"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, test_getPyReturnsVolatile)
{
    cpo::uno::Any aRet = mxPy->getPy(u"result=1"_ustr, {});
    uno::Reference<sheet::XVolatileResult> xVol;
    CPPUNIT_ASSERT(aRet >>= xVol);
    CPPUNIT_ASSERT(xVol.is());

    cpo::uno::Any aRet2 = mxPy->getPython(u"result=1"_ustr, {});
    uno::Reference<sheet::XVolatileResult> xVol2;
    CPPUNIT_ASSERT(aRet2 >>= xVol2);
    CPPUNIT_ASSERT(xVol2.is());
}

CPPUNIT_TEST_FIXTURE(Test, test_noKitRequestStaysQueued)
{
    // T7: before the kit installs a wakeup (on-load recalc) the request is queued, not
    // failed; the first drain after load sends it.
    cpo::uno::Any aRet = mxPy->getPy(u"result=1"_ustr, {});
    uno::Reference<sheet::XVolatileResult> xVol;
    CPPUNIT_ASSERT(aRet >>= xVol);
    rtl::Reference<CapturingResultListener> xLis(new CapturingResultListener);
    xVol->addResultListener(xLis);
    OUString sBusy;
    CPPUNIT_ASSERT(xLis->last() >>= sBusy);
    CPPUNIT_ASSERT_EQUAL(u"#BUSY!"_ustr, sBusy);
    CPPUNIT_ASSERT_EQUAL(1, pythoncompute_queued_count_for_tests());

    pythoncompute_set_wakeup(testWakeup); // installing wakes for the queued request
    CPPUNIT_ASSERT_EQUAL(1, g_nWakeups);
    const std::string emitted = takeLastEmitJson();
    CPPUNIT_ASSERT_EQUAL(u"result=1"_ustr, requestField(emitted, "code"));
    CPPUNIT_ASSERT_EQUAL(0, pythoncompute_queued_count_for_tests());
}

CPPUNIT_TEST_FIXTURE(Test, test_requestFieldsSurviveEscaping)
{
    // The emitted request is real JSON: quotes, backslashes and newlines in the code
    // decode back to the original text.
    const OUString sCode = u"s = \"id\"\nresult = s + '\\\\'"_ustr;
    mxPy->getPy(sCode, {});
    CPPUNIT_ASSERT_EQUAL(sCode, requestField(takeLastEmitJson(), "code"));
}

CPPUNIT_TEST_FIXTURE(Test, test_withoutKitIsDisabled)
{
    // Without a brokering kit nothing takes requests: the cell says so at once, nothing queued.
    setKitBrokers(false);
    uno::Reference<sheet::XVolatileResult> xVol;
    CPPUNIT_ASSERT(mxPy->getPy(u"result=desktop"_ustr, {}) >>= xVol);
    rtl::Reference<CapturingResultListener> xLis(new CapturingResultListener);
    xVol->addResultListener(xLis);
    CPPUNIT_ASSERT_EQUAL(u"#DISABLED"_ustr, xLis->last().get<OUString>());
    CPPUNIT_ASSERT_EQUAL(0, pythoncompute_queued_count_for_tests());
}

CPPUNIT_TEST_FIXTURE(Test, test_everyQueuedRequestWakes)
{
    // Each new request wakes the kit, also when an earlier drain took nothing (a load was
    // ongoing): no flag may swallow later wakeups. A cache hit queues nothing and does not.
    pythoncompute_set_wakeup(testWakeup);
    mxPy->getPy(u"result=w1"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(1, g_nWakeups);
    mxPy->getPy(u"result=w2"_ustr, {}); // the kit has not drained
    CPPUNIT_ASSERT_EQUAL(2, g_nWakeups);
    mxPy->getPy(u"result=w2"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(2, g_nWakeups);
    takeLastEmitJson();
    CPPUNIT_ASSERT_EQUAL(size_t(2), g_aTaken.size());
}

CPPUNIT_TEST_FIXTURE(Test, test_undeliveredRequestsRequeuedInOrder)
{
    mxPy->getPy(u"result=q1"_ustr, {});
    mxPy->getPy(u"result=q2"_ustr, {});
    mxPy->getPy(u"result=q3"_ustr, {});
    g_nRefuseAfter = 1;
    g_aTaken.clear();
    {
        SolarMutexReleaser aReleaser;
        CPPUNIT_ASSERT_EQUAL(1, pythoncompute_take_requests(testRequestCb, nullptr));
    }
    CPPUNIT_ASSERT_EQUAL(2, pythoncompute_queued_count_for_tests());
    g_nRefuseAfter = -1;
    g_aTaken.clear();
    {
        SolarMutexReleaser aReleaser;
        CPPUNIT_ASSERT_EQUAL(2, pythoncompute_take_requests(testRequestCb, nullptr));
    }
    CPPUNIT_ASSERT_EQUAL(u"result=q2"_ustr, requestField(g_aTaken[0], "code"));
    CPPUNIT_ASSERT_EQUAL(u"result=q3"_ustr, requestField(g_aTaken[1], "code"));
}

CPPUNIT_TEST_FIXTURE(Test, test_recalcsOfOneCellLeaveOneEntry)
{
    // N recalcs with the same args reuse one volatile and one queued request.
    uno::Reference<sheet::XVolatileResult> xFirst;
    for (int i = 0; i < 100; ++i)
    {
        uno::Reference<sheet::XVolatileResult> xVol;
        CPPUNIT_ASSERT(mxPy->getPy(u"result=same"_ustr, {}) >>= xVol);
        if (i == 0)
            xFirst = xVol;
        CPPUNIT_ASSERT_EQUAL(xFirst.get(), xVol.get());
    }
    CPPUNIT_ASSERT_EQUAL(1, pythoncompute_queued_count_for_tests());
}

CPPUNIT_TEST_FIXTURE(Test, test_reentrantGetPyFromRequestCallback)
{
    // T17: the request callback runs outside the AddIn's state; re-entering getPy()
    // from it (as a kit poll callback could) must not deadlock or lose the request.
    mxPy->getPy(u"result=outer"_ustr, {});
    auto cb = [](void* pPy, const char*, int32_t) -> int {
        SolarMutexGuard aGuard; // like any Calc caller
        static_cast<org::collaboraoffice::sheet::addin::XPythonComputeFunctions*>(pPy)->getPy(
            u"result=inner"_ustr, {});
        return 1;
    };
    {
        SolarMutexReleaser aReleaser;
        CPPUNIT_ASSERT_EQUAL(1, pythoncompute_take_requests(cb, mxPy.get()));
    }
    CPPUNIT_ASSERT_EQUAL(u"result=inner"_ustr, requestField(takeLastEmitJson(), "code"));
}

CPPUNIT_TEST_FIXTURE(Test, test_emitterCompleteRoundTrip)
{

    cpo::uno::Any aRet = mxPy->getPy(u"result=1+1"_ustr, {});
    uno::Reference<sheet::XVolatileResult> xVol;
    CPPUNIT_ASSERT(aRet >>= xVol);

    rtl::Reference<CapturingResultListener> xLis(new CapturingResultListener);
    xVol->addResultListener(xLis);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xLis->count());
    OUString sBusy;
    CPPUNIT_ASSERT(xLis->at(0) >>= sBusy);
    CPPUNIT_ASSERT_EQUAL(u"#BUSY!"_ustr, sBusy);

    const std::string emitted = takeLastEmitJson();
    CPPUNIT_ASSERT(!emitted.empty());
    CPPUNIT_ASSERT(emitted.find("\"mode\": \"isolated\"") != std::string::npos
                   || emitted.find("\"mode\":\"isolated\"") != std::string::npos);
    CPPUNIT_ASSERT_EQUAL(u"result=1+1"_ustr, requestField(emitted, "code"));
    const OUString sId = requestField(emitted, "id");
    CPPUNIT_ASSERT(!sId.isEmpty());

    const std::string badComplete = "{\"id\":\"not-this-id\",\"status\":\"ok\",\"result\":2}";
    CPPUNIT_ASSERT_EQUAL(0, pythoncompute_complete(
                                badComplete.data(), static_cast<sal_Int32>(badComplete.size())));

    const OString aIdUtf8 = OUStringToOString(sId, RTL_TEXTENCODING_UTF8);
    const std::string okComplete
        = std::string("{\"id\":\"") + aIdUtf8.getStr() + "\",\"status\":\"ok\",\"result\":2}";
    CPPUNIT_ASSERT_EQUAL(1, pythoncompute_complete(okComplete.data(),
                                                        static_cast<sal_Int32>(okComplete.size())));

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xLis->count());
    double f = 0.0;
    CPPUNIT_ASSERT(xLis->last() >>= f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, f, 1e-12);

    CPPUNIT_ASSERT_EQUAL(0, pythoncompute_complete(okComplete.data(),
                                                        static_cast<sal_Int32>(okComplete.size())));
}

CPPUNIT_TEST_FIXTURE(Test, test_emitterCompleteServiceError)
{

    cpo::uno::Any aRet = mxPy->getPy(u"result=err"_ustr, {});
    uno::Reference<sheet::XVolatileResult> xVol;
    CPPUNIT_ASSERT(aRet >>= xVol);

    rtl::Reference<CapturingResultListener> xLis(new CapturingResultListener);
    xVol->addResultListener(xLis);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xLis->count());
    OUString sBusy;
    CPPUNIT_ASSERT(xLis->at(0) >>= sBusy);
    CPPUNIT_ASSERT_EQUAL(u"#BUSY!"_ustr, sBusy);

    const std::string emitted = takeLastEmitJson();
    const OUString sId = requestField(emitted, "id");
    CPPUNIT_ASSERT(!sId.isEmpty());
    const OString aIdUtf8 = OUStringToOString(sId, RTL_TEXTENCODING_UTF8);
    const std::string errComplete = std::string("{\"id\":\"") + aIdUtf8.getStr()
                                    + "\",\"status\":\"error\",\"error\":\"boom\"}";
    CPPUNIT_ASSERT_EQUAL(1, pythoncompute_complete(
                                errComplete.data(), static_cast<sal_Int32>(errComplete.size())));

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xLis->count());
    double fErr = 0.0;
    CPPUNIT_ASSERT(xLis->last() >>= fErr);
    CPPUNIT_ASSERT_EQUAL(FormulaError::NoValue, GetDoubleErrorValue(fErr));

    // An error from the user's code is deterministic: same args, same object, no resend.
    uno::Reference<sheet::XVolatileResult> xAgain;
    CPPUNIT_ASSERT(mxPy->getPy(u"result=err"_ustr, {}) >>= xAgain);
    CPPUNIT_ASSERT_EQUAL(xVol.get(), xAgain.get());
    CPPUNIT_ASSERT(takeLastEmitJson().empty());
}

CPPUNIT_TEST_FIXTURE(Test, test_lateResultOfOldRequestKeepsNewEntry)
{
    // T3: same key; the old request failed transiently and a new one is pending. A second
    // (duplicate) result for the old id must not touch the new entry; the new result lands.
    pythoncompute_set_retry_ms_for_tests(0);
    uno::Reference<sheet::XVolatileResult> xVol1;
    CPPUNIT_ASSERT(mxPy->getPy(u"result=t3"_ustr, {}) >>= xVol1);
    const OUString sId1 = takeEmittedId();
    CPPUNIT_ASSERT_EQUAL(1, completeStr(reply(sId1, "\"status\":\"error\",\"error\":\"x\",\"transient\":true")));

    uno::Reference<sheet::XVolatileResult> xVol2;
    CPPUNIT_ASSERT(mxPy->getPy(u"result=t3"_ustr, {}) >>= xVol2);
    const OUString sId2 = takeEmittedId();
    CPPUNIT_ASSERT(sId1 != sId2);

    CPPUNIT_ASSERT_EQUAL(0, completeStr(reply(sId1, "\"status\":\"ok\",\"result\":1")));
    uno::Reference<sheet::XVolatileResult> xAgain;
    CPPUNIT_ASSERT(mxPy->getPy(u"result=t3"_ustr, {}) >>= xAgain);
    CPPUNIT_ASSERT_EQUAL(xVol2.get(), xAgain.get()); // the late result left the new entry
    CPPUNIT_ASSERT(takeLastEmitJson().empty());

    rtl::Reference<CapturingResultListener> xLis(new CapturingResultListener);
    xVol2->addResultListener(xLis);
    CPPUNIT_ASSERT_EQUAL(1, completeStr(reply(sId2, "\"status\":\"ok\",\"result\":2")));
    double fVal = 0;
    CPPUNIT_ASSERT(xLis->last() >>= fVal);
    CPPUNIT_ASSERT_EQUAL(2.0, fVal);
}
CPPUNIT_TEST_FIXTURE(Test, test_transientErrorRetriesAfterWindow)
{
    // A transient error -> Error with retryAfter: no resend inside the window (no storm),
    // a resend after it.
    // Calc's ScAddInListener keeps the volatile alive; so does this test.
    uno::Reference<sheet::XVolatileResult> xKeep;
    CPPUNIT_ASSERT(mxPy->getPy(u"result=transient"_ustr, {}) >>= xKeep);
    const OUString sId = requestField(takeLastEmitJson(), "id");
    CPPUNIT_ASSERT_EQUAL(
        1, completeStr(reply(sId, "\"status\":\"error\",\"error\":\"Python compute timed out\",\"transient\":true")));

    for (int i = 0; i < 10; ++i)
        mxPy->getPy(u"result=transient"_ustr, {});
    CPPUNIT_ASSERT(takeLastEmitJson().empty());

    pythoncompute_set_retry_ms_for_tests(0);
    uno::Reference<sheet::XVolatileResult> xKeep2;
    CPPUNIT_ASSERT(mxPy->getPy(u"result=transient2"_ustr, {}) >>= xKeep2);
    const OUString sId2 = requestField(takeLastEmitJson(), "id");
    CPPUNIT_ASSERT_EQUAL(1, completeStr(reply(sId2, "\"status\":\"error\",\"error\":\"x\",\"transient\":true")));
    mxPy->getPy(u"result=transient2"_ustr, {});
    CPPUNIT_ASSERT(!takeLastEmitJson().empty());
}

CPPUNIT_TEST_FIXTURE(Test, test_completeWithValidIdButMalformedResult)
{
    // K-1: the id decodes but a later field does not (a lone surrogate). The pending cell
    // finishes #VALUE! at once; it does not stay #BUSY! (no other reply will come).
    uno::Reference<sheet::XVolatileResult> xVol;
    CPPUNIT_ASSERT(mxPy->getPy(u"result=lone"_ustr, {}) >>= xVol);
    rtl::Reference<CapturingResultListener> xLis(new CapturingResultListener);
    xVol->addResultListener(xLis);
    const OUString sId = takeEmittedId();
    CPPUNIT_ASSERT_EQUAL(1, completeStr(reply(sId, R"("status":"ok","result":"\uD83D")")));
    double f = 0;
    CPPUNIT_ASSERT(xLis->last() >>= f);
    CPPUNIT_ASSERT_EQUAL(FormulaError::NoValue, GetDoubleErrorValue(f));
}

CPPUNIT_TEST_FIXTURE(Test, test_completeTriState)
{
    CPPUNIT_ASSERT_EQUAL(-1, pythoncompute_complete(nullptr, 0));
    CPPUNIT_ASSERT_EQUAL(-1, completeStr("not json"));
    CPPUNIT_ASSERT_EQUAL(0, completeStr("{\"id\":\"py-999999\",\"status\":\"ok\",\"result\":1}"));
}

CPPUNIT_TEST_FIXTURE(Test, test_malformedAndDeepResultIsCached)
{
    uno::Reference<sheet::XVolatileResult> xVol;
    CPPUNIT_ASSERT(mxPy->getPy(u"result=deep"_ustr, {}) >>= xVol);
    rtl::Reference<CapturingResultListener> xLis(new CapturingResultListener);
    xVol->addResultListener(xLis);
    const OUString sId = requestField(takeLastEmitJson(), "id");
    std::string deep(100000, '[');
    deep += std::string(100000, ']');
    CPPUNIT_ASSERT_EQUAL(1, completeStr(reply(sId, "\"status\":\"ok\",\"result\":" + deep)));
    double f = 0;
    CPPUNIT_ASSERT(xLis->last() >>= f);
    CPPUNIT_ASSERT_EQUAL(FormulaError::NoValue, GetDoubleErrorValue(f));

    // A reply that cannot be parsed is deterministic: cached, never resent.
    pythoncompute_set_retry_ms_for_tests(0);
    uno::Reference<sheet::XVolatileResult> xAgain;
    CPPUNIT_ASSERT(mxPy->getPy(u"result=deep"_ustr, {}) >>= xAgain);
    CPPUNIT_ASSERT_EQUAL(xVol.get(), xAgain.get());
    CPPUNIT_ASSERT(takeLastEmitJson().empty());
}

OUString oversizeCode(int nBytes)
{
    return OUString::Concat("result='") + RepeatedUChar(u'x', nBytes) + "'";
}

CPPUNIT_TEST_FIXTURE(Test, test_oversizeRequestIsValueError)
{
    constexpr int kOversizeRequestBytes = 4096;
    pythoncompute_set_max_payload_bytes(1024);
    uno::Reference<sheet::XVolatileResult> xVol;
    CPPUNIT_ASSERT(mxPy->getPy(oversizeCode(kOversizeRequestBytes), {}) >>= xVol);
    rtl::Reference<CapturingResultListener> xLis(new CapturingResultListener);
    xVol->addResultListener(xLis);
    double f = 0;
    CPPUNIT_ASSERT(xLis->last() >>= f);
    CPPUNIT_ASSERT_EQUAL(FormulaError::NoValue, GetDoubleErrorValue(f));
    CPPUNIT_ASSERT_EQUAL(0, pythoncompute_queued_count_for_tests());

    // Transient: a cap raised later (the kit installs after a load-time recalc) lets it go.
    pythoncompute_set_max_payload_bytes(2 * kOversizeRequestBytes);
    pythoncompute_set_retry_ms_for_tests(0);
    mxPy->getPy(oversizeCode(kOversizeRequestBytes), {});
    CPPUNIT_ASSERT_EQUAL(1, pythoncompute_queued_count_for_tests());
}

CPPUNIT_TEST_FIXTURE(Test, test_heldBytesBackstopFailsTransient)
{
    // Past 64 x max_payload_bytes (at least 1 GiB) of held request JSON a new request is a
    // transient #N/A;
    // once a result frees room and the retry window passes, the same args are sent.
    const OUString sPad = OUString::createFromAscii(std::string(1000, 'x'));
    auto code = [&](int i) -> OUString { return "result=" + OUString::number(100 + i) + "#" + sPad; };
    mxPy->getPy(code(0), {});
    const std::string aFirst = takeLastEmitJson();
    CPPUNIT_ASSERT_EQUAL(1, completeStr(reply(requestField(aFirst, "id"),
                                              "\"status\":\"ok\",\"result\":1")));
    pythoncompute_set_min_held_bytes_for_tests(0); // test the derived part
    pythoncompute_set_max_payload_bytes(static_cast<sal_Int64>(aFirst.size()) + 16); // 64 fit
    pythoncompute_set_retry_ms_for_tests(0);

    std::vector<uno::Reference<sheet::XVolatileResult>> aKeep;
    for (int i = 1; i <= 64; ++i)
    {
        uno::Reference<sheet::XVolatileResult> xVol;
        CPPUNIT_ASSERT(mxPy->getPy(code(i), {}) >>= xVol);
        aKeep.push_back(xVol);
    }
    CPPUNIT_ASSERT_EQUAL(64, pythoncompute_queued_count_for_tests());

    uno::Reference<sheet::XVolatileResult> xOver;
    CPPUNIT_ASSERT(mxPy->getPy(code(65), {}) >>= xOver);
    rtl::Reference<CapturingResultListener> xLis(new CapturingResultListener);
    xOver->addResultListener(xLis);
    CPPUNIT_ASSERT_EQUAL(cpo::uno::TypeClass_VOID, xLis->last().getValueTypeClass()); // #N/A
    CPPUNIT_ASSERT_EQUAL(64, pythoncompute_queued_count_for_tests());

    // One result frees room: the retry is queued.
    const OUString sOne = requestField(takeLastEmitJson(), "id"); // hands off 16, last of them
    CPPUNIT_ASSERT_EQUAL(1, completeStr(reply(sOne, "\"status\":\"ok\",\"result\":1")));
    uno::Reference<sheet::XVolatileResult> xRetry;
    CPPUNIT_ASSERT(mxPy->getPy(code(65), {}) >>= xRetry);
    CPPUNIT_ASSERT(xRetry != xOver);
    CPPUNIT_ASSERT_EQUAL(64 - 16 + 1, pythoncompute_queued_count_for_tests());
}

CPPUNIT_TEST_FIXTURE(Test, test_handOffBudgetHoldsRequestsUntilResults)
{
    // The kit gets requests while those it holds total at most 16 x max_payload_bytes.
    // The rest wait (#BUSY!, never an error) and go, oldest first, as results come.
    const OUString sPad = OUString::createFromAscii(std::string(400, 'x'));
    auto code = [&](int i) -> OUString { return "result=" + OUString::number(10 + i) + "#" + sPad; };
    mxPy->getPy(code(0), {});
    const std::string aFirst = takeLastEmitJson();
    const sal_Int64 nSize = static_cast<sal_Int64>(aFirst.size());
    CPPUNIT_ASSERT_EQUAL(1, completeStr(reply(requestField(aFirst, "id"),
                                              "\"status\":\"ok\",\"result\":1")));
    pythoncompute_set_max_payload_bytes(nSize + 16); // 16 requests fit, a 17th does not
    pythoncompute_set_wakeup(testWakeup);

    std::vector<uno::Reference<sheet::XVolatileResult>> aKeep;
    for (int i = 1; i <= 20; ++i)
    {
        uno::Reference<sheet::XVolatileResult> xVol;
        CPPUNIT_ASSERT(mxPy->getPy(code(i), {}) >>= xVol);
        aKeep.push_back(xVol);
    }

    // An undelivered batch gives its reservation back.
    g_nRefuseAfter = 0;
    takeLastEmitJson();
    CPPUNIT_ASSERT_EQUAL(20, pythoncompute_queued_count_for_tests());
    g_nRefuseAfter = -1;

    takeLastEmitJson();
    CPPUNIT_ASSERT_EQUAL(size_t(16), g_aTaken.size());
    CPPUNIT_ASSERT_EQUAL(4, pythoncompute_queued_count_for_tests());
    const std::vector<std::string> aHeld = g_aTaken;
    CPPUNIT_ASSERT(takeLastEmitJson().empty()); // nothing more until a result

    // A result frees room for exactly one more: the oldest waiting one, after a wakeup.
    const int nWakeups = g_nWakeups;
    CPPUNIT_ASSERT_EQUAL(1, completeStr(reply(requestField(aHeld[0], "id"),
                                              "\"status\":\"ok\",\"result\":1")));
    CPPUNIT_ASSERT_EQUAL(nWakeups + 1, g_nWakeups);
    CPPUNIT_ASSERT_EQUAL(code(17), requestField(takeLastEmitJson(), "code"));
    CPPUNIT_ASSERT_EQUAL(size_t(1), g_aTaken.size());

    // An error result frees room too.
    CPPUNIT_ASSERT_EQUAL(1, completeStr(reply(requestField(aHeld[1], "id"),
                                              "\"status\":\"error\",\"error\":\"x\",\"transient\":true")));
    CPPUNIT_ASSERT_EQUAL(code(18), requestField(takeLastEmitJson(), "code"));
    CPPUNIT_ASSERT_EQUAL(2, pythoncompute_queued_count_for_tests());
}

class ThrowingResultListener final : public cppu::WeakImplHelper<sheet::XResultListener>
{
public:
    int m_nCalls = 0;
    void modified(const sheet::ResultEvent&) override
    {
        if (++m_nCalls > 1)
            throw std::runtime_error("listener");
    }
    void disposing(const lang::EventObject&) override {}
};

CPPUNIT_TEST_FIXTURE(Test, test_throwingListenerDoesNotStarveOthers)
{
    uno::Reference<sheet::XVolatileResult> xVol;
    CPPUNIT_ASSERT(mxPy->getPy(u"result=lis"_ustr, {}) >>= xVol);
    rtl::Reference<ThrowingResultListener> xBad(new ThrowingResultListener);
    rtl::Reference<CapturingResultListener> xGood(new CapturingResultListener);
    xVol->addResultListener(xBad);
    xVol->addResultListener(xGood);
    const OUString sId = requestField(takeLastEmitJson(), "id");
    CPPUNIT_ASSERT_EQUAL(1, completeStr(reply(sId, "\"status\":\"ok\",\"result\":5")));
    double f = 0;
    CPPUNIT_ASSERT(xGood->last() >>= f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, f, 1e-12);
}

CPPUNIT_TEST_FIXTURE(Test, test_requestIdsUnique)
{

    std::set<OUString> aIds;
    constexpr int nCount = 64;
    for (int i = 0; i < nCount; ++i)
    {
        // Distinct code → distinct cache keys so each call still emits.
        const OUString sCode = "result=" + OUString::number(static_cast<sal_Int32>(i));
        cpo::uno::Any aRet = mxPy->getPy(sCode, {});
        uno::Reference<sheet::XVolatileResult> xVol;
        CPPUNIT_ASSERT(aRet >>= xVol);
        CPPUNIT_ASSERT(xVol.is());

        const std::string emitted = takeLastEmitJson();
        CPPUNIT_ASSERT(!emitted.empty());
        const OUString sId = requestField(emitted, "id");
        CPPUNIT_ASSERT(!sId.isEmpty());
        CPPUNIT_ASSERT_MESSAGE("duplicate request id", aIds.insert(sId).second);
    }
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(nCount), aIds.size());
}

CPPUNIT_TEST_FIXTURE(Test, test_volatileIdentitySameArgs)
{

    cpo::uno::Any aRet1 = mxPy->getPy(u"result=identity"_ustr, {});
    uno::Reference<sheet::XVolatileResult> xVol1;
    CPPUNIT_ASSERT(aRet1 >>= xVol1);
    CPPUNIT_ASSERT(xVol1.is());
    const std::string emit1 = takeLastEmitJson();
    CPPUNIT_ASSERT(!emit1.empty());

    cpo::uno::Any aRet2 = mxPy->getPy(u"result=identity"_ustr, {});
    uno::Reference<sheet::XVolatileResult> xVol2;
    CPPUNIT_ASSERT(aRet2 >>= xVol2);
    CPPUNIT_ASSERT(xVol2.is());
    CPPUNIT_ASSERT_EQUAL(xVol1.get(), xVol2.get());
    CPPUNIT_ASSERT(takeLastEmitJson().empty());

    cpo::uno::Sequence<cpo::uno::Any> aData{ cpo::uno::Any(sal_Int32(7)) };
    cpo::uno::Any aRet3 = mxPy->getPy(u"result=identity"_ustr, aData);
    uno::Reference<sheet::XVolatileResult> xVol3;
    CPPUNIT_ASSERT(aRet3 >>= xVol3);
    CPPUNIT_ASSERT(xVol3.is());
    CPPUNIT_ASSERT(xVol3.get() != xVol1.get());
    CPPUNIT_ASSERT(!takeLastEmitJson().empty());
}

CPPUNIT_TEST_FIXTURE(Test, test_paramCacheIdentityUnderPressure)
{
    // Pruning (each time the cache doubles) must never evict a live entry: AddIn.idl
    // "same params -> same object". 257 keys pass several prunes.

    constexpr int nKeys = 257;
    std::vector<uno::Reference<sheet::XVolatileResult>> aHeld;
    aHeld.reserve(nKeys);
    uno::Reference<sheet::XVolatileResult> xFirst;
    for (int i = 0; i < nKeys; ++i)
    {
        const OUString sCode = "result=pressure_" + OUString::number(static_cast<sal_Int32>(i));
        cpo::uno::Any aRet = mxPy->getPy(sCode, {});
        uno::Reference<sheet::XVolatileResult> xVol;
        CPPUNIT_ASSERT(aRet >>= xVol);
        CPPUNIT_ASSERT(xVol.is());
        // Distinct args -> a fresh emit each time (no cache hit).
        CPPUNIT_ASSERT(!takeLastEmitJson().empty());
        if (i == 0)
            xFirst = xVol;
        aHeld.push_back(xVol);
    }

    // The earliest key is still referenced: same object, and no duplicate emit.
    cpo::uno::Any aRet0 = mxPy->getPy(u"result=pressure_0"_ustr, {});
    uno::Reference<sheet::XVolatileResult> xVol0;
    CPPUNIT_ASSERT(aRet0 >>= xVol0);
    CPPUNIT_ASSERT_EQUAL(xFirst.get(), xVol0.get());
    CPPUNIT_ASSERT(takeLastEmitJson().empty());
}

CPPUNIT_TEST_FIXTURE(Test, test_paramCacheDropsLapsedWeak)
{
    // Once the request has finished and no cell holds the volatile, the weak
    // lapses and the same params allocate a fresh object + re-emit.

    {
        cpo::uno::Any aRet = mxPy->getPy(u"result=lapse"_ustr, {});
        uno::Reference<sheet::XVolatileResult> xVol;
        CPPUNIT_ASSERT(aRet >>= xVol);
        CPPUNIT_ASSERT(xVol.is());
        const std::string emitted = takeLastEmitJson();
        CPPUNIT_ASSERT(!emitted.empty());

        // Complete the request so the pending map drops its strong reference;
        // an in-flight #BUSY! volatile is deliberately kept alive until then.
        const OUString sId = requestField(emitted, "id");
        const OString aIdUtf8 = OUStringToOString(sId, RTL_TEXTENCODING_UTF8);
        const std::string okComplete
            = std::string("{\"id\":\"") + aIdUtf8.getStr() + "\",\"status\":\"ok\",\"result\":1}";
        CPPUNIT_ASSERT_EQUAL(1, pythoncompute_complete(
                                    okComplete.data(), static_cast<sal_Int32>(okComplete.size())));
    }

    // Cell reference dropped + request finished -> weak lapses; same params
    // miss the cache and re-emit a fresh request.
    cpo::uno::Any aRet2 = mxPy->getPy(u"result=lapse"_ustr, {});
    uno::Reference<sheet::XVolatileResult> xVol2;
    CPPUNIT_ASSERT(aRet2 >>= xVol2);
    CPPUNIT_ASSERT(xVol2.is());
    CPPUNIT_ASSERT(!takeLastEmitJson().empty());
}

CPPUNIT_TEST_FIXTURE(Test, test_anyToJsonFragment_scalars)
{
    using co::pythoncompute::anyToJsonFragment;

    CPPUNIT_ASSERT_EQUAL(u"null"_ustr, anyToJsonFragment(cpo::uno::Any()));
    CPPUNIT_ASSERT_EQUAL(u"true"_ustr, anyToJsonFragment(cpo::uno::Any(true)));
    CPPUNIT_ASSERT_EQUAL(u"false"_ustr, anyToJsonFragment(cpo::uno::Any(false)));
    CPPUNIT_ASSERT_EQUAL(u"42"_ustr, anyToJsonFragment(cpo::uno::Any(sal_Int32(42))));
    CPPUNIT_ASSERT_EQUAL(u"\"hi\""_ustr, anyToJsonFragment(cpo::uno::Any(u"hi"_ustr)));
    CPPUNIT_ASSERT_EQUAL(u"\"\""_ustr, anyToJsonFragment(cpo::uno::Any(u""_ustr)));
}

CPPUNIT_TEST_FIXTURE(Test, test_anyToJsonFragment_64BitIntegers)
{
    // K-3: Any's >>= (double&) does not take HYPER; such values must not become 0.
    using co::pythoncompute::anyToJsonFragment;
    CPPUNIT_ASSERT_EQUAL(u"42"_ustr, anyToJsonFragment(cpo::uno::Any(sal_Int64(42))));
    CPPUNIT_ASSERT_EQUAL(u"-7"_ustr, anyToJsonFragment(cpo::uno::Any(sal_Int64(-7))));
    CPPUNIT_ASSERT_EQUAL(u"4000000000"_ustr,
                         anyToJsonFragment(cpo::uno::Any(sal_uInt64(4000000000))));
    const std::string json = co::pythoncompute::buildExecuteRequestJson(
        u"x"_ustr, u"result=data"_ustr, { cpo::uno::Any(sal_Int64(42)) });
    CPPUNIT_ASSERT_MESSAGE(json, json.find("\"data\": 42") != std::string::npos);
}

CPPUNIT_TEST_FIXTURE(Test, test_withRequestIdMatchesFullBuild)
{
    // Splicing the id into the "_" key JSON gives the same bytes as building with the id.
    using co::pythoncompute::buildExecuteRequestJson;
    const cpo::uno::Sequence<cpo::uno::Any> aData{ cpo::uno::Any(1.5),
                                                  cpo::uno::Any(u"_"_ustr) };
    const std::string keyJson = buildExecuteRequestJson(u"_"_ustr, u"result=\"_\""_ustr, aData);
    CPPUNIT_ASSERT_EQUAL(buildExecuteRequestJson(u"py-17"_ustr, u"result=\"_\""_ustr, aData),
                         co::pythoncompute::withRequestId(keyJson, "py-17"));
}

CPPUNIT_TEST_FIXTURE(Test, test_anyToJsonFragment_escapesAndNonFinite)
{
    using co::pythoncompute::anyToJsonFragment;

    CPPUNIT_ASSERT_EQUAL(u"\"a\\\"b\""_ustr, anyToJsonFragment(cpo::uno::Any(u"a\"b"_ustr)));
    CPPUNIT_ASSERT_EQUAL(u"\"a\\nb\""_ustr, anyToJsonFragment(cpo::uno::Any(u"a\nb"_ustr)));
    CPPUNIT_ASSERT_EQUAL(u"\"\\u0001\""_ustr,
                         anyToJsonFragment(cpo::uno::Any(OUString(u'\x0001'))));

    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();
    CPPUNIT_ASSERT_EQUAL(u"null"_ustr, anyToJsonFragment(cpo::uno::Any(nan)));
    CPPUNIT_ASSERT_EQUAL(u"null"_ustr, anyToJsonFragment(cpo::uno::Any(inf)));
    CPPUNIT_ASSERT_EQUAL(u"null"_ustr, anyToJsonFragment(cpo::uno::Any(-inf)));
}

CPPUNIT_TEST_FIXTURE(Test, test_anyToJsonFragment_grids)
{
    using co::pythoncompute::anyToJsonFragment;

    cpo::uno::Sequence<cpo::uno::Any> aFlat{ cpo::uno::Any(1.0), cpo::uno::Any(2.0),
                                             cpo::uno::Any(3.0) };
    // tools::JsonWriter inserts spaces after [ and commas.
    CPPUNIT_ASSERT_EQUAL(u"[ 1, 2, 3]"_ustr, anyToJsonFragment(cpo::uno::Any(aFlat)));

    // Typed matrix / vector paths — 1×N / N×1 flatten like Any grids / Classic.
    cpo::uno::Sequence<double> aDRow{ 1.0, 2.0, 3.0 };
    CPPUNIT_ASSERT_EQUAL(u"[ 1, 2, 3]"_ustr, anyToJsonFragment(cpo::uno::Any(aDRow)));

    cpo::uno::Sequence<cpo::uno::Sequence<double>> aDRowGrid{
        cpo::uno::Sequence<double>{ 1.0, 2.0, 3.0 },
    };
    CPPUNIT_ASSERT_EQUAL(u"[ 1, 2, 3]"_ustr, anyToJsonFragment(cpo::uno::Any(aDRowGrid)));

    cpo::uno::Sequence<cpo::uno::Sequence<double>> aDColGrid{
        cpo::uno::Sequence<double>{ 1.0 },
        cpo::uno::Sequence<double>{ 2.0 },
        cpo::uno::Sequence<double>{ 3.0 },
    };
    CPPUNIT_ASSERT_EQUAL(u"[ 1, 2, 3]"_ustr, anyToJsonFragment(cpo::uno::Any(aDColGrid)));

    cpo::uno::Sequence<cpo::uno::Sequence<double>> aDGrid{
        cpo::uno::Sequence<double>{ 1.0, 2.0 },
        cpo::uno::Sequence<double>{ 3.0, 4.0 },
    };
    CPPUNIT_ASSERT_EQUAL(u"[ [ 1, 2], [ 3, 4]]"_ustr, anyToJsonFragment(cpo::uno::Any(aDGrid)));

    // Calc ranges are Sequence<Sequence<Any>> — must not emit null.
    cpo::uno::Sequence<cpo::uno::Sequence<cpo::uno::Any>> aRowGrid{
        cpo::uno::Sequence<cpo::uno::Any>{ cpo::uno::Any(1.0), cpo::uno::Any(2.0),
                                           cpo::uno::Any(3.0) }
    };
    cpo::uno::Any aRowAny;
    aRowAny <<= aRowGrid;
    CPPUNIT_ASSERT_EQUAL(u"[ 1, 2, 3]"_ustr, anyToJsonFragment(aRowAny));

    cpo::uno::Sequence<cpo::uno::Sequence<cpo::uno::Any>> aColGrid{
        cpo::uno::Sequence<cpo::uno::Any>{ cpo::uno::Any(1.0) },
        cpo::uno::Sequence<cpo::uno::Any>{ cpo::uno::Any(2.0) },
        cpo::uno::Sequence<cpo::uno::Any>{ cpo::uno::Any(3.0) },
    };
    cpo::uno::Any aColAny;
    aColAny <<= aColGrid;
    CPPUNIT_ASSERT_EQUAL(u"[ 1, 2, 3]"_ustr, anyToJsonFragment(aColAny));

    // Non-flattened 2×2 mixed Any grid (production Calc shape).
    cpo::uno::Sequence<cpo::uno::Sequence<cpo::uno::Any>> aAnyGrid{
        cpo::uno::Sequence<cpo::uno::Any>{ cpo::uno::Any(1.0), cpo::uno::Any(2.0) },
        cpo::uno::Sequence<cpo::uno::Any>{ cpo::uno::Any(3.0), cpo::uno::Any(4.0) },
    };
    cpo::uno::Any aGridAny;
    aGridAny <<= aAnyGrid;
    CPPUNIT_ASSERT_EQUAL(u"[ [ 1, 2], [ 3, 4]]"_ustr, anyToJsonFragment(aGridAny));
}

CPPUNIT_TEST_FIXTURE(Test, test_buildExecuteRequestJson)
{
    using co::pythoncompute::buildExecuteRequestJson;

    const std::string noData = buildExecuteRequestJson(u"py-1"_ustr, u"result=1"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(std::string::npos, noData.find("\"data\""));
    CPPUNIT_ASSERT(noData.find("\"mode\": \"isolated\"") != std::string::npos
                   || noData.find("\"mode\":\"isolated\"") != std::string::npos);
    CPPUNIT_ASSERT_EQUAL(u"py-1"_ustr, requestField(noData, "id"));
    CPPUNIT_ASSERT_EQUAL(u"result=1"_ustr, requestField(noData, "code"));

    cpo::uno::Sequence<cpo::uno::Any> aOne{ cpo::uno::Any(sal_Int32(7)) };
    const std::string withScalar = buildExecuteRequestJson(u"py-2"_ustr, u"x"_ustr, aOne);
    CPPUNIT_ASSERT(withScalar.find("\"data\": 7") != std::string::npos
                   || withScalar.find("\"data\":7") != std::string::npos);

    cpo::uno::Sequence<cpo::uno::Any> aTwo{ cpo::uno::Any(1.0), cpo::uno::Any(2.0) };
    const std::string withArr = buildExecuteRequestJson(u"py-3"_ustr, u"x"_ustr, aTwo);
    CPPUNIT_ASSERT(withArr.find("\"data\": [ 1, 2]") != std::string::npos
                   || withArr.find("\"data\":[1,2]") != std::string::npos);

    // Single AddIn arg that is a Calc range (Sequence<Sequence<Any>>).
    cpo::uno::Sequence<cpo::uno::Sequence<cpo::uno::Any>> aRange{
        cpo::uno::Sequence<cpo::uno::Any>{ cpo::uno::Any(1.0), cpo::uno::Any(2.0) },
        cpo::uno::Sequence<cpo::uno::Any>{ cpo::uno::Any(3.0), cpo::uno::Any(4.0) },
    };
    cpo::uno::Sequence<cpo::uno::Any> aOneRange{ cpo::uno::Any(aRange) };
    const std::string withRange
        = buildExecuteRequestJson(u"py-4"_ustr, u"result=data"_ustr, aOneRange);
    CPPUNIT_ASSERT_EQUAL(std::string::npos, withRange.find("\"data\": null"));
    CPPUNIT_ASSERT_EQUAL(std::string::npos, withRange.find("\"data\":null"));
    CPPUNIT_ASSERT(withRange.find("[ 1, 2]") != std::string::npos
                   || withRange.find("[1,2]") != std::string::npos);

    // Multiple AddIn ranges → JSON array of grids (each range may flatten).
    cpo::uno::Sequence<cpo::uno::Sequence<cpo::uno::Any>> aRangeA{
        cpo::uno::Sequence<cpo::uno::Any>{ cpo::uno::Any(1.0), cpo::uno::Any(2.0) },
    };
    cpo::uno::Sequence<cpo::uno::Sequence<cpo::uno::Any>> aRangeB{
        cpo::uno::Sequence<cpo::uno::Any>{ cpo::uno::Any(3.0) },
        cpo::uno::Sequence<cpo::uno::Any>{ cpo::uno::Any(4.0) },
    };
    cpo::uno::Sequence<cpo::uno::Any> aTwoRanges{ cpo::uno::Any(aRangeA), cpo::uno::Any(aRangeB) };
    const std::string withMulti
        = buildExecuteRequestJson(u"py-5"_ustr, u"result=data"_ustr, aTwoRanges);
    CPPUNIT_ASSERT(withMulti.find("\"data\": [") != std::string::npos
                   || withMulti.find("\"data\":[") != std::string::npos);
    CPPUNIT_ASSERT(withMulti.find("[ 1, 2]") != std::string::npos
                   || withMulti.find("[1,2]") != std::string::npos);
    CPPUNIT_ASSERT(withMulti.find("[ 3, 4]") != std::string::npos
                   || withMulti.find("[3,4]") != std::string::npos);
}

CPPUNIT_TEST_FIXTURE(Test, test_jsonRejectsInvalidUtf8)
{
    using co::pythoncompute::ResultKind;
    using co::pythoncompute::jsonResultToAny;
    cpo::uno::Any aOut;
    OUString sErr;
    // Overlong NUL, a UTF-16 surrogate, and a code point above U+10FFFF.
    for (const char* pBad : { "\xC0\x80", "\xED\xA0\x80", "\xF4\x90\x80\x80" })
    {
        const std::string sJson
            = std::string(R"({"id":"1","status":"error","error":"x)") + pBad + "\"}";
        CPPUNIT_ASSERT_EQUAL(ResultKind::ServiceError, jsonResultToAny(sJson, aOut, sErr));
    }
    // Valid multi-byte text still parses.
    CPPUNIT_ASSERT_EQUAL(ResultKind::ServiceError,
                         jsonResultToAny("{\"id\":\"1\",\"status\":\"error\",\"error\":"
                                         "\"\xC3\xA9\xF0\x9F\x98\x80\"}",
                                         aOut, sErr));
    CPPUNIT_ASSERT_EQUAL(OUString::fromUtf8("\xC3\xA9\xF0\x9F\x98\x80"), sErr);
}

CPPUNIT_TEST_FIXTURE(Test, test_jsonResultToAny)
{
    // Every kind fills aOut with a cell value; each case checks both the kind and the value.
    using co::pythoncompute::jsonResultToAny;

    cpo::uno::Any aOut;
    OUString sErr;

    CPPUNIT_ASSERT_EQUAL(ResultKind::ServiceError, jsonResultToAny(
        R"({"id":"1","status":"error","error":"boom"})", aOut, sErr));
    CPPUNIT_ASSERT_EQUAL(u"boom"_ustr, sErr);
    double fErr = 0.0;
    CPPUNIT_ASSERT(aOut >>= fErr);
    CPPUNIT_ASSERT_EQUAL(FormulaError::NoValue, GetDoubleErrorValue(fErr));

    // coolwsd feature-flag reject → literal #DISABLED (not FormulaError / #VALUE!)
    CPPUNIT_ASSERT_EQUAL(ResultKind::TransientError, jsonResultToAny(
        R"({"id":"1","status":"error","error":"Python compute is disabled"})", aOut, sErr));
    CPPUNIT_ASSERT_EQUAL(u"Python compute is disabled"_ustr, sErr);
    OUString sDisabled;
    CPPUNIT_ASSERT(aOut >>= sDisabled);
    CPPUNIT_ASSERT_EQUAL(u"#DISABLED"_ustr, sDisabled);

    OUString s;

    CPPUNIT_ASSERT_EQUAL(ResultKind::Ok, jsonResultToAny(R"({"id":"1","status":"ok","result":3.5})", aOut, sErr));
    double f = 0.0;
    CPPUNIT_ASSERT(aOut >>= f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.5, f, 1e-12);

    CPPUNIT_ASSERT_EQUAL(ResultKind::Ok, jsonResultToAny(R"({"id":"1","status":"ok","result":true})", aOut, sErr));
    bool b = false;
    CPPUNIT_ASSERT(aOut >>= b);
    CPPUNIT_ASSERT(b);

    CPPUNIT_ASSERT_EQUAL(ResultKind::Ok, jsonResultToAny(R"({"id":"1","status":"ok","result":false})", aOut, sErr));
    CPPUNIT_ASSERT(aOut >>= b);
    CPPUNIT_ASSERT(!b);

    // Quoted strings must not be coerced to bool/number/null.
    CPPUNIT_ASSERT_EQUAL(ResultKind::Ok, jsonResultToAny(R"({"id":"1","status":"ok","result":"true"})", aOut, sErr));
    CPPUNIT_ASSERT(aOut >>= s);
    CPPUNIT_ASSERT_EQUAL(u"true"_ustr, s);

    CPPUNIT_ASSERT_EQUAL(ResultKind::Ok, jsonResultToAny(R"({"id":"1","status":"ok","result":"false"})", aOut, sErr));
    CPPUNIT_ASSERT(aOut >>= s);
    CPPUNIT_ASSERT_EQUAL(u"false"_ustr, s);

    CPPUNIT_ASSERT_EQUAL(ResultKind::Ok, jsonResultToAny(R"({"id":"1","status":"ok","result":"42"})", aOut, sErr));
    CPPUNIT_ASSERT(aOut >>= s);
    CPPUNIT_ASSERT_EQUAL(u"42"_ustr, s);

    CPPUNIT_ASSERT_EQUAL(ResultKind::Ok, jsonResultToAny(R"({"id":"1","status":"ok","result":"null"})", aOut, sErr));
    CPPUNIT_ASSERT(aOut >>= s);
    CPPUNIT_ASSERT_EQUAL(u"null"_ustr, s);

    CPPUNIT_ASSERT_EQUAL(ResultKind::Ok, jsonResultToAny(R"({"id":"1","status":"ok","result":"\uD83D\uDE00"})", aOut, sErr));
    CPPUNIT_ASSERT(aOut >>= s);
    CPPUNIT_ASSERT_EQUAL(OUString::fromUtf8("\xF0\x9F\x98\x80"), s);

    CPPUNIT_ASSERT_EQUAL(ResultKind::Ok, jsonResultToAny(R"({"id":"1","status":"ok","result":42})", aOut, sErr));
    CPPUNIT_ASSERT(aOut >>= f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(42.0, f, 1e-12);

    CPPUNIT_ASSERT_EQUAL(ResultKind::Ok, jsonResultToAny(R"({"id":"1","status":"ok","result":null})", aOut, sErr));
    CPPUNIT_ASSERT(aOut >>= s);
    CPPUNIT_ASSERT(s.isEmpty());

    // Empty and zero-column results show an empty cell, like null.
    for (const char* pEmpty : { R"({"id":"1","status":"ok","result":[]})",
                                R"({"id":"1","status":"ok","result":[[]]})" })
    {
        s = u"x"_ustr;
        CPPUNIT_ASSERT_EQUAL(ResultKind::Ok, jsonResultToAny(pEmpty, aOut, sErr));
        CPPUNIT_ASSERT(aOut >>= s);
        CPPUNIT_ASSERT(s.isEmpty());
    }

    CPPUNIT_ASSERT_EQUAL(ResultKind::Ok, jsonResultToAny(R"({"id":"1","status":"ok","result":[1,2,3]})", aOut, sErr));
    cpo::uno::Sequence<cpo::uno::Sequence<double>> aMatrix;
    CPPUNIT_ASSERT(aOut >>= aMatrix);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aMatrix.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aMatrix[0].getLength());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, aMatrix[0][0], 1e-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, aMatrix[0][2], 1e-12);

    // Rectangular nested numeric lists → R×C sequence<sequence<double>>.
    CPPUNIT_ASSERT_EQUAL(ResultKind::Ok, jsonResultToAny(R"({"id":"1","status":"ok","result":[[1,2],[3,4]]})", aOut, sErr));
    cpo::uno::Sequence<cpo::uno::Sequence<double>> aGrid;
    CPPUNIT_ASSERT(aOut >>= aGrid);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aGrid.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aGrid[0].getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aGrid[1].getLength());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, aGrid[0][0], 1e-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, aGrid[0][1], 1e-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, aGrid[1][0], 1e-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, aGrid[1][1], 1e-12);

    // Mixed grid keeps strings.
    CPPUNIT_ASSERT_EQUAL(ResultKind::Ok, jsonResultToAny(R"({"id":"1","status":"ok","result":[[1,"a"],[2,"b"]]})", aOut, sErr));
    cpo::uno::Sequence<cpo::uno::Sequence<cpo::uno::Any>> aMixed;
    CPPUNIT_ASSERT(aOut >>= aMixed);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aMixed.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aMixed[0].getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aMixed[1].getLength());
    double f00 = 0;
    CPPUNIT_ASSERT(aMixed[0][0] >>= f00);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, f00, 1e-12);
    OUString s01;
    CPPUNIT_ASSERT(aMixed[0][1] >>= s01);
    CPPUNIT_ASSERT_EQUAL(u"a"_ustr, s01);
    double f10 = 0;
    CPPUNIT_ASSERT(aMixed[1][0] >>= f10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, f10, 1e-12);
    OUString s11;
    CPPUNIT_ASSERT(aMixed[1][1] >>= s11);
    CPPUNIT_ASSERT_EQUAL(u"b"_ustr, s11);

    // Column vector: [[1],[2],[3]] → 3×1 (not a collapsed 1×3 row).
    CPPUNIT_ASSERT_EQUAL(ResultKind::Ok, jsonResultToAny(R"({"id":"1","status":"ok","result":[[1],[2],[3]]})", aOut, sErr));
    cpo::uno::Sequence<cpo::uno::Sequence<double>> aCol;
    CPPUNIT_ASSERT(aOut >>= aCol);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aCol.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aCol[0].getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aCol[1].getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aCol[2].getLength());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, aCol[0][0], 1e-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, aCol[1][0], 1e-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, aCol[2][0], 1e-12);

    // Single-element list stays 1×1 matrix (not bare scalar) for stack consistency.
    CPPUNIT_ASSERT_EQUAL(ResultKind::Ok, jsonResultToAny(R"({"id":"1","status":"ok","result":[5]})", aOut, sErr));
    cpo::uno::Sequence<cpo::uno::Sequence<double>> aOne;
    CPPUNIT_ASSERT(aOut >>= aOne);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aOne.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aOne[0].getLength());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, aOne[0][0], 1e-12);

    CPPUNIT_ASSERT_EQUAL(ResultKind::Ok, jsonResultToAny(R"({"id":"1","status":"ok","result":["one"]})", aOut, sErr));
    cpo::uno::Sequence<cpo::uno::Sequence<cpo::uno::Any>> aOneString;
    CPPUNIT_ASSERT(aOut >>= aOneString);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aOneString.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aOneString[0].getLength());
    CPPUNIT_ASSERT(aOneString[0][0] >>= s);
    CPPUNIT_ASSERT_EQUAL(u"one"_ustr, s);

    CPPUNIT_ASSERT_EQUAL(ResultKind::Ok, jsonResultToAny(R"({"id":"1","status":"ok","result":[true]})", aOut, sErr));
    cpo::uno::Sequence<cpo::uno::Sequence<cpo::uno::Any>> aOneBool;
    CPPUNIT_ASSERT(aOut >>= aOneBool);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aOneBool.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aOneBool[0].getLength());
    CPPUNIT_ASSERT(aOneBool[0][0] >>= b);
    CPPUNIT_ASSERT(b);

    CPPUNIT_ASSERT_EQUAL(ResultKind::Ok, jsonResultToAny(R"({"id":"1","status":"ok","result":[["a"],["b"]]})", aOut, sErr));
    cpo::uno::Sequence<cpo::uno::Sequence<cpo::uno::Any>> aStringCol;
    CPPUNIT_ASSERT(aOut >>= aStringCol);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aStringCol.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aStringCol[0].getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aStringCol[1].getLength());

    CPPUNIT_ASSERT_EQUAL(ResultKind::Ok, jsonResultToAny(R"({"id":"1","status":"ok","result":[[1,2],["a","b"]]})", aOut, sErr));
    cpo::uno::Sequence<cpo::uno::Sequence<cpo::uno::Any>> aMixedRows;
    CPPUNIT_ASSERT(aOut >>= aMixedRows);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aMixedRows.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aMixedRows[0].getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aMixedRows[1].getLength());

    CPPUNIT_ASSERT_EQUAL(ResultKind::ServiceError, jsonResultToAny(R"({"id":"1","status":"ok","result":[[1,2],[3]]})", aOut, sErr));
    CPPUNIT_ASSERT_EQUAL(u"Python compute result JSON parse failed"_ustr, sErr);
    CPPUNIT_ASSERT(aOut >>= fErr);
    CPPUNIT_ASSERT_EQUAL(FormulaError::NoValue, GetDoubleErrorValue(fErr));

    CPPUNIT_ASSERT_EQUAL(ResultKind::ServiceError, jsonResultToAny(R"({"id":"1","status":"ok","result":"\uD83D"})", aOut, sErr));
    CPPUNIT_ASSERT_EQUAL(u"Python compute result JSON parse failed"_ustr, sErr);
    CPPUNIT_ASSERT_EQUAL(ResultKind::ServiceError, jsonResultToAny(
        R"({"id":"1","status":"ok","result":1} trailing)", aOut, sErr));
    CPPUNIT_ASSERT_EQUAL(u"Python compute result JSON parse failed"_ustr, sErr);

    CPPUNIT_ASSERT_EQUAL(ResultKind::Ok, jsonResultToAny(
        R"({"id":"1","status":"ok","result":null,"images":["a"]})", aOut, sErr));
    CPPUNIT_ASSERT(aOut >>= s);
    CPPUNIT_ASSERT_EQUAL(u"Image generated (plot insert not supported yet)"_ustr, s);
    CPPUNIT_ASSERT_EQUAL(ResultKind::Ok, jsonResultToAny(
        R"({"id":"1","status":"ok","result":[[]],"images":["a"]})", aOut, sErr));
    CPPUNIT_ASSERT(aOut >>= s);
    CPPUNIT_ASSERT_EQUAL(u"Image generated (plot insert not supported yet)"_ustr, s);

    CPPUNIT_ASSERT_EQUAL(ResultKind::ServiceError, jsonResultToAny(R"({"id":"1","status":"ok"})", aOut, sErr));
    CPPUNIT_ASSERT_EQUAL(u"Python compute response missing result"_ustr, sErr);
    CPPUNIT_ASSERT(aOut >>= fErr);
    CPPUNIT_ASSERT_EQUAL(FormulaError::NoValue, GetDoubleErrorValue(fErr));
}

CPPUNIT_TEST_FIXTURE(Test, test_anyJsonFragment_roundTrip)
{
    using co::pythoncompute::anyToJsonFragment;
    // Every kind fills aOut with a cell value; each case checks both the kind and the value.
    using co::pythoncompute::jsonResultToAny;

    auto wrapOk = [](const OUString& sFragment) {
        return std::string("{\"id\":\"t\",\"status\":\"ok\",\"result\":")
               + std::string(sFragment.toUtf8()) + "}";
    };

    cpo::uno::Any aOut;
    OUString sErr;

    // Row vector Any grid → flat JSON → 1×N double matrix.
    cpo::uno::Sequence<cpo::uno::Sequence<cpo::uno::Any>> aRow{
        cpo::uno::Sequence<cpo::uno::Any>{ cpo::uno::Any(1.0), cpo::uno::Any(2.0),
                                           cpo::uno::Any(3.0) },
    };
    const OUString sRow = anyToJsonFragment(cpo::uno::Any(aRow));
    CPPUNIT_ASSERT_EQUAL(u"[ 1, 2, 3]"_ustr, sRow);
    CPPUNIT_ASSERT_EQUAL(ResultKind::Ok, jsonResultToAny(wrapOk(sRow), aOut, sErr));
    cpo::uno::Sequence<cpo::uno::Sequence<double>> aRowMat;
    CPPUNIT_ASSERT(aOut >>= aRowMat);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aRowMat.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aRowMat[0].getLength());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, aRowMat[0][1], 1e-12);

    // Typed double 2×2 stays nested through emit → parse.
    cpo::uno::Sequence<cpo::uno::Sequence<double>> aDGrid{
        cpo::uno::Sequence<double>{ 1.0, 2.0 },
        cpo::uno::Sequence<double>{ 3.0, 4.0 },
    };
    const OUString sGrid = anyToJsonFragment(cpo::uno::Any(aDGrid));
    CPPUNIT_ASSERT_EQUAL(u"[ [ 1, 2], [ 3, 4]]"_ustr, sGrid);
    CPPUNIT_ASSERT_EQUAL(ResultKind::Ok, jsonResultToAny(wrapOk(sGrid), aOut, sErr));
    cpo::uno::Sequence<cpo::uno::Sequence<double>> aParsedGrid;
    CPPUNIT_ASSERT(aOut >>= aParsedGrid);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aParsedGrid.getLength());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, aParsedGrid[1][1], 1e-12);

    // Mixed grid keeps string cells.
    cpo::uno::Sequence<cpo::uno::Sequence<cpo::uno::Any>> aMixed{
        cpo::uno::Sequence<cpo::uno::Any>{ cpo::uno::Any(1.0), cpo::uno::Any(u"a"_ustr) },
        cpo::uno::Sequence<cpo::uno::Any>{ cpo::uno::Any(2.0), cpo::uno::Any(u"b"_ustr) },
    };
    const OUString sMixed = anyToJsonFragment(cpo::uno::Any(aMixed));
    CPPUNIT_ASSERT_EQUAL(ResultKind::Ok, jsonResultToAny(wrapOk(sMixed), aOut, sErr));
    cpo::uno::Sequence<cpo::uno::Sequence<cpo::uno::Any>> aMixedOut;
    CPPUNIT_ASSERT(aOut >>= aMixedOut);
    OUString s01;
    CPPUNIT_ASSERT(aMixedOut[0][1] >>= s01);
    CPPUNIT_ASSERT_EQUAL(u"a"_ustr, s01);

    // Quoted numeric string stays a string through the envelope.
    CPPUNIT_ASSERT_EQUAL(ResultKind::Ok, jsonResultToAny(R"({"id":"t","status":"ok","result":"42"})", aOut, sErr));
    OUString sQuoted;
    CPPUNIT_ASSERT(aOut >>= sQuoted);
    CPPUNIT_ASSERT_EQUAL(u"42"_ustr, sQuoted);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
