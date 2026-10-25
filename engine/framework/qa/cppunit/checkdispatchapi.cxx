/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/awt/XReschedule.hpp>
#include <com/sun/star/awt/XToolkitExperimental.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XDispatchInformationProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <test/unoapi_test.hxx>
#include "Interceptor.hxx"
#include <thread>

using namespace ::cpo;
using namespace ::cpo::uno;
using namespace css::frame;
using namespace css::beans;

class CheckDispatchAPI : public UnoApiTest
{
    /** can be used to create new test frames. */
    Reference<XFrame> m_xDesktop;
    /** provides XDispatchInformationProvider interface. */
    Reference<XFrame> m_xFrame;

public:
    CheckDispatchAPI()
        : UnoApiTest(u"/framework/qa/cppunit/data"_ustr)
    {
    }

    void setUp() override;
    void tearDown() override;

    void checkDispatchInfoOfWriter();
    void checkDispatchInfoOfCalc();
    void checkDispatchInfoOfDraw();
    void checkDispatchInfoOfImpress();
    void checkDispatchInfoOfChart();
    void checkDispatchInfoOfMath();
    void checkDispatchInfoOfBasic();
    void checkDispatchInfoOfStartModule();
    void checkInterceptorLifeTime();
    void checkInterception();

    void impl_checkDispatchInfoOfXXX(const OUString& sXXX);
    void impl_loadIntoFrame(const Reference<XFrame>& xFrame, const OUString& sURL, const Sequence<PropertyValue>& args);
    Reference<XInterface> impl_createUNOComponent(const OUString& sName);
    void impl_checkDispatchInfo(const Reference<XInterface>& aComponent);
    Reference<XFrame> impl_createNewFrame();
    void impl_closeFrame(const Reference<XFrame>& xFrame);

    CPPUNIT_TEST_SUITE(CheckDispatchAPI);
    CPPUNIT_TEST(checkDispatchInfoOfWriter);
    CPPUNIT_TEST(checkDispatchInfoOfCalc);
    CPPUNIT_TEST(checkDispatchInfoOfDraw);
    CPPUNIT_TEST(checkDispatchInfoOfImpress);
    CPPUNIT_TEST(checkDispatchInfoOfChart);
    CPPUNIT_TEST(checkDispatchInfoOfMath);
    CPPUNIT_TEST(checkDispatchInfoOfBasic);
    CPPUNIT_TEST(checkDispatchInfoOfStartModule);
    CPPUNIT_TEST(checkInterceptorLifeTime);
    CPPUNIT_TEST(checkInterception);
    CPPUNIT_TEST_SUITE_END();
};


void CheckDispatchAPI::setUp()
{
    UnoApiTest::setUp();

    // create desktop
    m_xDesktop.set(getMultiServiceFactory()->createInstance(u"com.sun.star.frame.Desktop"_ustr), UNO_QUERY);

    m_xFrame = impl_createNewFrame();
}

void CheckDispatchAPI::tearDown()
{
    impl_closeFrame(m_xFrame);
    m_xFrame = nullptr;
    UnoApiTest::tearDown();
}

void CheckDispatchAPI::checkDispatchInfoOfWriter()
{
    impl_checkDispatchInfoOfXXX(u"private:factory/swriter"_ustr);
}

void CheckDispatchAPI::checkDispatchInfoOfCalc()
{
    impl_checkDispatchInfoOfXXX(u"private:factory/scalc"_ustr);
}

void CheckDispatchAPI::checkDispatchInfoOfDraw()
{
    impl_checkDispatchInfoOfXXX(u"private:factory/sdraw"_ustr);
}

void CheckDispatchAPI::checkDispatchInfoOfImpress()
{
    impl_checkDispatchInfoOfXXX(u"private:factory/simpress"_ustr);
}

void CheckDispatchAPI::checkDispatchInfoOfChart()
{
    impl_checkDispatchInfoOfXXX(u"private:factory/schart"_ustr);
}

void CheckDispatchAPI::checkDispatchInfoOfMath()
{
    impl_checkDispatchInfoOfXXX(u"private:factory/smath"_ustr);
}

void CheckDispatchAPI::checkDispatchInfoOfBasic()
{
    Reference<XInterface> aComponent = impl_createUNOComponent(u"com.sun.star.script.BasicIDE"_ustr);
    impl_checkDispatchInfo(aComponent);
}

void CheckDispatchAPI::checkDispatchInfoOfStartModule()
{
    Reference<XInterface> aComponent = impl_createUNOComponent(u"com.sun.star.frame.StartModule"_ustr);
    impl_checkDispatchInfo(aComponent);
}

void CheckDispatchAPI::checkInterceptorLifeTime()
{
    // Note: It's important for the following test, that xInterceptor will be hold alive by the uno reference
    // xInterceptor. Otherwise we can't check some internal states of xInterceptor at the end of this method, because
    // it was already killed .-)

    rtl::Reference<Interceptor> xInterceptor = new Interceptor(getMultiServiceFactory());

    Reference<XFrame> xFrame = impl_createNewFrame();
    Reference<XDispatchProviderInterception> xInterception(xFrame, UNO_QUERY);

    xInterception->registerDispatchProviderInterceptor(xInterceptor);
    impl_closeFrame(xFrame);

    sal_Int32 nRegCount = xInterceptor->getRegistrationCount();
    bool bIsRegistered = xInterceptor->isRegistered();


    if (nRegCount < 1)
    {
        CPPUNIT_FAIL("Interceptor was never registered.");
    }

    if (bIsRegistered)
    {
        CPPUNIT_FAIL("Interceptor was not deregistered automatically on closing the corresponding frame.");
    }

}

void CheckDispatchAPI::checkInterception()
{
    std::vector<OUString> lDisabledURLs  { u".uno:Open"_ustr };

    rtl::Reference<Interceptor> xInterceptor = new Interceptor(getMultiServiceFactory());
    xInterceptor->setURLs4URLs4Blocking(lDisabledURLs);

    Reference<XFrame> xFrame = impl_createNewFrame();

    Reference<XDispatchProviderInterception> xInterception(xFrame, UNO_QUERY);
    xInterception->registerDispatchProviderInterceptor(xInterceptor);

    impl_loadIntoFrame(xFrame, u"private:factory/swriter"_ustr, {});

    // Framework dispatcher update is on a ~50ms wait.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    Reference<XReschedule> m_xReschedule(
        getMultiServiceFactory()->createInstance(u"com.sun.star.awt.Toolkit"_ustr), UNO_QUERY);
    // queryDispatch for toolbars etc. happens asynchronously.
    m_xReschedule->reschedule();
    Reference<XToolkitExperimental> m_xIdles(m_xReschedule, UNO_QUERY);
    m_xIdles->processEventsToIdle();

    // deregister interceptor
    xInterception->releaseDispatchProviderInterceptor(xInterceptor);
}

void CheckDispatchAPI::impl_checkDispatchInfoOfXXX(const OUString& sXXX)
{
    Reference<XFrame> xFrame = impl_createNewFrame();
    impl_loadIntoFrame(xFrame, sXXX, {});
    impl_checkDispatchInfo(xFrame);
    impl_closeFrame(xFrame);
}

/** @short  load a URL into the current test frame.
 */
void CheckDispatchAPI::impl_loadIntoFrame(const Reference<XFrame>& xFrame, const OUString& sURL, const Sequence<PropertyValue>& args)
{
    Reference<XComponentLoader> xLoader(xFrame, UNO_QUERY);
    if (xLoader == nullptr)
    {
        CPPUNIT_FAIL("Frame does not provide required interface XComponentLoader.");
    }

    Reference<XComponent> xDoc = xLoader->loadComponentFromURL(sURL, u"_self"_ustr, 0, args);

    if (xDoc == nullptr)
    {
        CPPUNIT_FAIL("Could not load \"" + std::string(sURL.toUtf8()) + "\".");
    }
}

/** @short  create a uno implementation directly.
 */
Reference<XInterface> CheckDispatchAPI::impl_createUNOComponent(const OUString& sName)
{
    Reference<XInterface> aComponent;
    try
    {
        aComponent = getMultiServiceFactory()->createInstance(sName);
    }
    catch (Exception )
    {
    }

    if (aComponent == nullptr)
    {
        CPPUNIT_FAIL("Could not create UNO component \"" + std::string(sName.toUtf8()) + "\".");
    }
    return aComponent;
}

/** @short  check the interface XDispatchInformationProvider
at the specified component.
 */
void CheckDispatchAPI::impl_checkDispatchInfo(const Reference<XInterface>& aComponent)
{
    Reference<XDispatchInformationProvider> xInfoProvider(aComponent, UNO_QUERY);
    if (xInfoProvider == nullptr)
    {
        // Warning:\tComponent does not provide the [optional!] interface XDispatchInformationProvider.
        return;
    }

    Sequence<sal_Int16> lGroups = xInfoProvider->getSupportedCommandGroups();
    sal_Int32 c1 = lGroups.getLength();
    sal_Int32 i1 = 0;
    for (i1 = 0; i1 < c1; ++i1)
    {
        sal_Int16 nGroup = lGroups[i1];
        Sequence<DispatchInformation> lInfos = xInfoProvider->getConfigurableDispatchInformation(nGroup);
        sal_Int32 c2 = lInfos.getLength();
        sal_Int32 i2 = 0;

        // check for empty lists
        // Warning
        if (lInfos.getLength() < 1)
        {
            // Warning:\tCould not get any DispatchInformation for group [" + nGroup + "]."
        }

        // check for duplicates (and by the way, if the info item match the requested group)
        std::map<OUString, OUString> aCheckMap;
        for (i2 = 0; i2 < c2; ++i2)
        {
            DispatchInformation aInfo = lInfos[i2];
            if (aInfo.GroupId != nGroup)
            {
                // Error
                CPPUNIT_FAIL("At least one DispatchInformation item does not match the requested group.\n\trequested group=["
                        + std::to_string(nGroup)
                        + "] returned group=[" + std::to_string(aInfo.GroupId) + "] command=\""
                        + aInfo.Command.toUtf8().getStr() + "\""); // true => don't break this test
            }

            if (aCheckMap.find(aInfo.Command) != aCheckMap.end())
            {
                // Error
                CPPUNIT_FAIL("Found a duplicate item: group=["
                    + std::to_string(aInfo.GroupId) + "] command=\""
                    + aInfo.Command.toUtf8().getStr() + "\""); // true => don't break this test
            }

            aCheckMap[aInfo.Command] = aInfo.Command;
        }
    }
}

Reference<XFrame> CheckDispatchAPI::impl_createNewFrame()
{
    Reference<XFrame> xFrame = m_xDesktop->findFrame(u"_blank"_ustr, 0);
    xFrame->getContainerWindow()->setVisible(true);
    return xFrame;
}

void CheckDispatchAPI::impl_closeFrame(const Reference<XFrame>& xFrame)
{
    Reference<XCloseable> xClose(xFrame, UNO_QUERY);
    xClose->close(false);
}

CPPUNIT_TEST_SUITE_REGISTRATION(CheckDispatchAPI);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
