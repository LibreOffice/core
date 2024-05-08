/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppuhelper/implbase.hxx>
#include <test/unoapi_test.hxx>

#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#include <com/sun/star/frame/XInterceptorInfo.hpp>
#include <com/sun/star/util/URLTransformer.hpp>

#include <rtl/ref.hxx>
#include <mutex>

using namespace ::com::sun::star;

namespace
{
/// Sample interception implementation that asserts getInterceptedURLs() and queryDispatch() is in sync.
class MyInterceptor
    : public cppu::WeakImplHelper<frame::XDispatchProviderInterceptor, frame::XInterceptorInfo>
{
    std::mutex maMutex;
    uno::Reference<frame::XDispatchProvider> m_xMaster;
    uno::Reference<frame::XDispatchProvider> m_xSlave;
    uno::Sequence<OUString> m_aDisabledCommands;
    int m_nExpected;
    int m_nUnexpected;

public:
    MyInterceptor();

    /// Number of queryDispatch() calls that operate on a command advertised by getInterceptedURLs().
    int getExpected();
    /// Number of queryDispatch() calls that operate on a command not advertised by getInterceptedURLs().
    int getUnexpected();

    // frame::XInterceptorInfo
    virtual uno::Sequence<OUString> SAL_CALL getInterceptedURLs() override;

    // frame::XDispatchProviderInterceptor
    virtual void SAL_CALL setMasterDispatchProvider(
        const uno::Reference<frame::XDispatchProvider>& xNewSupplier) override;
    virtual uno::Reference<frame::XDispatchProvider> SAL_CALL getMasterDispatchProvider() override;
    virtual void SAL_CALL
    setSlaveDispatchProvider(const uno::Reference<frame::XDispatchProvider>& xNewSupplier) override;
    virtual uno::Reference<frame::XDispatchProvider> SAL_CALL getSlaveDispatchProvider() override;

    // frame::XDispatchProvider
    virtual uno::Sequence<uno::Reference<frame::XDispatch>> SAL_CALL
    queryDispatches(const uno::Sequence<frame::DispatchDescriptor>& rRequests) override;
    virtual uno::Reference<frame::XDispatch>
        SAL_CALL queryDispatch(const util::URL& rURL, const OUString& rTargetFrameName,
                               sal_Int32 SearchFlags) override;
};

MyInterceptor::MyInterceptor()
    : m_aDisabledCommands{ u".uno:Bold"_ustr }
    , m_nExpected(0)
    , m_nUnexpected(0)
{
}

int MyInterceptor::getExpected()
{
    std::unique_lock aGuard(maMutex);
    int nRet = m_nExpected;
    m_nExpected = 0;
    return nRet;
}

int MyInterceptor::getUnexpected()
{
    std::unique_lock aGuard(maMutex);
    int nRet = m_nUnexpected;
    m_nUnexpected = 0;
    return nRet;
}

uno::Sequence<OUString> MyInterceptor::getInterceptedURLs() { return m_aDisabledCommands; }

void MyInterceptor::setMasterDispatchProvider(
    const uno::Reference<frame::XDispatchProvider>& xNewSupplier)
{
    std::unique_lock aGuard(maMutex);
    m_xMaster = xNewSupplier;
}

uno::Reference<frame::XDispatchProvider> MyInterceptor::getMasterDispatchProvider()
{
    std::unique_lock aGuard(maMutex);
    return m_xMaster;
}

void MyInterceptor::setSlaveDispatchProvider(
    const uno::Reference<frame::XDispatchProvider>& xNewSupplier)
{
    std::unique_lock aGuard(maMutex);
    m_xSlave = xNewSupplier;
}

uno::Reference<frame::XDispatchProvider> MyInterceptor::getSlaveDispatchProvider()
{
    std::unique_lock aGuard(maMutex);
    return m_xSlave;
}

uno::Sequence<uno::Reference<frame::XDispatch>>
MyInterceptor::queryDispatches(const uno::Sequence<frame::DispatchDescriptor>& rRequests)
{
    uno::Sequence<uno::Reference<frame::XDispatch>> aResult(rRequests.getLength());
    auto aResultRange = asNonConstRange(aResult);

    for (sal_Int32 i = 0; i < rRequests.getLength(); ++i)
    {
        aResultRange[i] = queryDispatch(rRequests[i].FeatureURL, rRequests[i].FrameName,
                                        rRequests[i].SearchFlags);
    }

    return aResult;
}

uno::Reference<frame::XDispatch> MyInterceptor::queryDispatch(const util::URL& rURL,
                                                              const OUString& /*rTargetFrameName*/,
                                                              sal_Int32 /*SearchFlags*/)
{
    std::unique_lock aGuard(maMutex);
    if (std::find(std::cbegin(m_aDisabledCommands), std::cend(m_aDisabledCommands), rURL.Complete)
        != std::cend(m_aDisabledCommands))
        ++m_nExpected;
    else
        ++m_nUnexpected;

    return uno::Reference<frame::XDispatch>();
}

/// Tests how InterceptionHelper invokes a registered interceptor.
class DispatchTest : public UnoApiTest
{
public:
    DispatchTest()
        : UnoApiTest(u"/framework/qa/cppunit/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(DispatchTest, testInterception)
{
    mxComponent
        = loadFromDesktop(u"private:factory/swriter"_ustr, u"com.sun.star.text.TextDocument"_ustr);
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xModel.is());

    uno::Reference<frame::XDispatchProviderInterception> xRegistration(
        xModel->getCurrentController()->getFrame(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xRegistration.is());

    rtl::Reference<MyInterceptor> pInterceptor(new MyInterceptor());
    xRegistration->registerDispatchProviderInterceptor(pInterceptor);

    dispatchCommand(mxComponent, u".uno:Bold"_ustr, {});
    CPPUNIT_ASSERT_GREATER(0, pInterceptor->getExpected());
    CPPUNIT_ASSERT_EQUAL(0, pInterceptor->getUnexpected());
    dispatchCommand(mxComponent, u".uno:Italic"_ustr, {});
    // This was 1: MyInterceptor::queryDispatch() was called for .uno:Italic.
    CPPUNIT_ASSERT_EQUAL(0, pInterceptor->getUnexpected());
}

CPPUNIT_TEST_FIXTURE(DispatchTest, testSfxOfficeDispatchDispose)
{
    // this test doesn't work with a new document because of aURL.Main check in SfxBaseController::dispatch()
    loadFromFile(u"empty.fodp");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xModel.is());
    uno::Reference<frame::XController> xController(xModel->getCurrentController());
    CPPUNIT_ASSERT(xController.is());
    uno::Reference<frame::XDispatchProvider> xFrame(xController->getFrame(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFrame.is());

    uno::Reference<util::XURLTransformer> xParser(util::URLTransformer::create(mxComponentContext));
    util::URL url;
    url.Complete = xModel->getURL() + "#dummy";
    xParser->parseStrict(url);

    uno::Reference<frame::XDispatch> xDisp(xFrame->queryDispatch(url, u""_ustr, 0));
    CPPUNIT_ASSERT(xDisp.is());

    mxComponent->dispose();

    util::URL urlSlot;
    urlSlot.Complete = "slot:5598";
    xParser->parseStrict(urlSlot);
    // crashed with UAF
    xDisp->dispatch(urlSlot, {});
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
