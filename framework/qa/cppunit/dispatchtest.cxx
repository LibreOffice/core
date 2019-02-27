/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppuhelper/implbase.hxx>
#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#include <com/sun/star/frame/XInterceptorInfo.hpp>

#include <comphelper/processfactory.hxx>
#include <rtl/ref.hxx>

using namespace ::com::sun::star;

namespace
{

/// Sample interception implementation that asserts getInterceptedURLs() and queryDispatch() is in sync.
class MyInterceptor : public cppu::WeakImplHelper
    <
    frame::XDispatchProviderInterceptor,
    frame::XInterceptorInfo
    >
{
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
    virtual void SAL_CALL setMasterDispatchProvider(const uno::Reference<frame::XDispatchProvider>& xNewSupplier) override;
    virtual uno::Reference<frame::XDispatchProvider> SAL_CALL getMasterDispatchProvider() override;
    virtual void SAL_CALL setSlaveDispatchProvider(const uno::Reference<frame::XDispatchProvider>& xNewSupplier) override;
    virtual uno::Reference<frame::XDispatchProvider> SAL_CALL getSlaveDispatchProvider() override;

    // frame::XDispatchProvider
    virtual uno::Sequence<uno::Reference<frame::XDispatch>> SAL_CALL queryDispatches(const uno::Sequence<frame::DispatchDescriptor>& rRequests) override;
    virtual uno::Reference<frame::XDispatch> SAL_CALL queryDispatch(const util::URL& rURL, const OUString& rTargetFrameName, sal_Int32 SearchFlags) override;
};

MyInterceptor::MyInterceptor()
    : m_aDisabledCommands {".uno:Bold"},
      m_nExpected(0),
      m_nUnexpected(0)
{
}

int MyInterceptor::getExpected()
{
    int nRet = m_nExpected;
    m_nExpected = 0;
    return nRet;
}

int MyInterceptor::getUnexpected()
{
    int nRet = m_nUnexpected;
    m_nUnexpected = 0;
    return nRet;
}

uno::Sequence<OUString> MyInterceptor::getInterceptedURLs()
{
    return m_aDisabledCommands;
}

void MyInterceptor::setMasterDispatchProvider(const uno::Reference<frame::XDispatchProvider>& xNewSupplier)
{
    m_xMaster = xNewSupplier;
}

uno::Reference<frame::XDispatchProvider> MyInterceptor::getMasterDispatchProvider()
{
    return m_xMaster;
}

void MyInterceptor::setSlaveDispatchProvider(const uno::Reference<frame::XDispatchProvider>& xNewSupplier)
{
    m_xSlave = xNewSupplier;
}

uno::Reference<frame::XDispatchProvider> MyInterceptor::getSlaveDispatchProvider()
{
    return m_xSlave;
}

uno::Sequence<uno::Reference<frame::XDispatch>> MyInterceptor::queryDispatches(const uno::Sequence<frame::DispatchDescriptor>& rRequests)
{
    uno::Sequence<uno::Reference<frame::XDispatch>> aResult(rRequests.getLength());

    for (sal_Int32 i = 0; i < rRequests.getLength(); ++i)
    {
        aResult[i] = queryDispatch(rRequests[i].FeatureURL, rRequests[i].FrameName, rRequests[i].SearchFlags);
    }

    return aResult;
}

uno::Reference<frame::XDispatch> MyInterceptor::queryDispatch(const util::URL& rURL, const OUString& /*rTargetFrameName*/, sal_Int32 /*SearchFlags*/)
{
    if (std::find(m_aDisabledCommands.begin(), m_aDisabledCommands.end(), rURL.Complete) != m_aDisabledCommands.end())
        ++m_nExpected;
    else
        ++m_nUnexpected;

    return uno::Reference<frame::XDispatch>();
}

/// Tests how InterceptionHelper invokes a registered interceptor.
class DispatchTest : public test::BootstrapFixture, public unotest::MacrosTest
{
protected:
    uno::Reference<uno::XComponentContext> mxComponentContext;
    uno::Reference<lang::XComponent> mxComponent;
    void dispatchCommand(const uno::Reference<lang::XComponent>& xComponent, const OUString& rCommand, const uno::Sequence<beans::PropertyValue>& rPropertyValues);

public:
    virtual void setUp() override;
    virtual void tearDown() override;
};

void DispatchTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxComponentContext.set(comphelper::getComponentContext(getMultiServiceFactory()));
    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void DispatchTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

void DispatchTest::dispatchCommand(const uno::Reference<lang::XComponent>& xComponent, const OUString& rCommand, const uno::Sequence<beans::PropertyValue>& rPropertyValues)
{
    uno::Reference<frame::XController> xController = uno::Reference<frame::XModel>(xComponent, uno::UNO_QUERY)->getCurrentController();
    CPPUNIT_ASSERT(xController.is());
    uno::Reference<frame::XDispatchProvider> xFrame(xController->getFrame(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFrame.is());

    uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
    uno::Reference<frame::XDispatchHelper> xDispatchHelper(frame::DispatchHelper::create(xContext));
    CPPUNIT_ASSERT(xDispatchHelper.is());

    xDispatchHelper->executeDispatch(xFrame, rCommand, OUString(), 0, rPropertyValues);
}

CPPUNIT_TEST_FIXTURE(DispatchTest, testInterception)
{
    mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");
    CPPUNIT_ASSERT(mxComponent.is());
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xModel.is());

    uno::Reference<frame::XDispatchProviderInterception> xRegistration(xModel->getCurrentController()->getFrame(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xRegistration.is());

    rtl::Reference<MyInterceptor> pInterceptor(new MyInterceptor());
    xRegistration->registerDispatchProviderInterceptor(uno::Reference<frame::XDispatchProviderInterceptor>(pInterceptor.get()));

    dispatchCommand(mxComponent, ".uno:Bold", {});
    CPPUNIT_ASSERT_EQUAL(1, pInterceptor->getExpected());
    CPPUNIT_ASSERT_EQUAL(0, pInterceptor->getUnexpected());
    dispatchCommand(mxComponent, ".uno:Italic", {});
    CPPUNIT_ASSERT_EQUAL(1, pInterceptor->getExpected());
    // This was 1: MyInterceptor::queryDispatch() was called for .uno:Italic.
    CPPUNIT_ASSERT_EQUAL(0, pInterceptor->getUnexpected());
}

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
