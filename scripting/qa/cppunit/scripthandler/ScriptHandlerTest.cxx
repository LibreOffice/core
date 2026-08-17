/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <comphelper/propertyvalue.hxx>
#include <comphelper/scopeguard.hxx>
#include <cppuhelper/implbase.hxx>
#include <test/unoapi_test.hxx>
#include <vcl/scheduler.hxx>

#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/script/provider/XScriptProvider.hpp>
#include <com/sun/star/util/URLTransformer.hpp>

namespace
{
constexpr OUString DUMMY_PROVIDER_SERVICE
    = u"com.sun.star.script.provider.ScriptProviderForDummy"_ustr;
constexpr OUString LANGUAGE_PROVIDER_SERVICE
    = u"com.sun.star.script.provider.LanguageScriptProvider"_ustr;
constexpr OUString SCRIPT_URL
    = u"vnd.sun.star.script:Whatever?language=Dummy&location=document"_ustr;

// A Dummy script provider so that we can execute our own function instead of running a script
class DummyProvider
    : public cppu::WeakImplHelper<css::script::provider::XScriptProvider,
                                  css::script::provider::XScript, css::lang::XServiceInfo,
                                  css::lang::XSingleComponentFactory>
{
public:
    // XScriptProvider
    css::uno::Reference<css::script::provider::XScript>
        SAL_CALL getScript(const OUString& sScriptURI) override;

    // XScript
    css::uno::Any SAL_CALL invoke(const css::uno::Sequence<css::uno::Any>& aParams,
                                  css::uno::Sequence<sal_Int16>& aOutParamIndex,
                                  css::uno::Sequence<css::uno::Any>& aOutParam) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override;
    sal_Bool SAL_CALL supportsService(const OUString& sServiceName) override;
    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XSingleComponentFactory
    css::uno::Reference<css::uno::XInterface> SAL_CALL createInstanceWithContext(
        const css::uno::Reference<css::uno::XComponentContext>& xContext) override;
    css::uno::Reference<css::uno::XInterface> SAL_CALL createInstanceWithArgumentsAndContext(
        const css::uno::Sequence<css::uno::Any>& rArguments,
        const css::uno::Reference<css::uno::XComponentContext>& xContext) override;

    bool m_bInvoked = false;
};

css::uno::Reference<css::script::provider::XScript>
    SAL_CALL DummyProvider::getScript(const OUString&)
{
    return this;
}

css::uno::Any SAL_CALL DummyProvider::invoke(const css::uno::Sequence<css::uno::Any>& aParams,
                                             css::uno::Sequence<sal_Int16>&,
                                             css::uno::Sequence<css::uno::Any>&)
{
    CPPUNIT_ASSERT_GREATEREQUAL(sal_Int32(6), aParams.getLength());

    // First argument should be the URL
    OUString sUrl;
    CPPUNIT_ASSERT(aParams[0] >>= sUrl);
    CPPUNIT_ASSERT_EQUAL(SCRIPT_URL, sUrl);

    // Second argument should be an XInteractionHandler
    css::uno::Reference<css::task::XInteractionHandler> xInteractionHandler;
    CPPUNIT_ASSERT(aParams[1] >>= xInteractionHandler);

    // Third argument is the OpenNewView value
    bool bOpenNewView = true;
    CPPUNIT_ASSERT(aParams[2] >>= bOpenNewView);
    CPPUNIT_ASSERT(!bOpenNewView);

    // Fourth argument is the frame name
    OUString sFrameName;
    CPPUNIT_ASSERT(aParams[3] >>= sFrameName);
    CPPUNIT_ASSERT_EQUAL(u"_self"_ustr, sFrameName);

    // Fifth argument is the macro execution mode
    sal_Int16 nMacroExecutionMode = 0;
    CPPUNIT_ASSERT(aParams[4] >>= nMacroExecutionMode);
    CPPUNIT_ASSERT_EQUAL(css::document::MacroExecMode::USE_CONFIG, nMacroExecutionMode);

    // Sixth argument is the update doc mode
    sal_Int16 nUpdateDocMode = 0;
    CPPUNIT_ASSERT(aParams[5] >>= nUpdateDocMode);
    CPPUNIT_ASSERT_EQUAL(css::document::UpdateDocMode::ACCORDING_TO_CONFIG, nUpdateDocMode);

    m_bInvoked = true;

    return css::uno::Any();
}

OUString SAL_CALL DummyProvider::getImplementationName()
{
    return u"com.sun.star.test.ScriptProviderForDummy"_ustr;
}

sal_Bool SAL_CALL DummyProvider::supportsService(const OUString& sServiceName)
{
    return sServiceName == DUMMY_PROVIDER_SERVICE || sServiceName == LANGUAGE_PROVIDER_SERVICE;
}

css::uno::Sequence<OUString> SAL_CALL DummyProvider::getSupportedServiceNames()
{
    return css::uno::Sequence<OUString>{ DUMMY_PROVIDER_SERVICE, LANGUAGE_PROVIDER_SERVICE };
}

css::uno::Reference<css::uno::XInterface> SAL_CALL
DummyProvider::createInstanceWithContext(const css::uno::Reference<css::uno::XComponentContext>&)
{
    return static_cast<css::script::provider::XScriptProvider*>(this);
}

css::uno::Reference<css::uno::XInterface>
    SAL_CALL DummyProvider::createInstanceWithArgumentsAndContext(
        const css::uno::Sequence<css::uno::Any>&,
        const css::uno::Reference<css::uno::XComponentContext>&)
{
    return static_cast<css::script::provider::XScriptProvider*>(this);
}

class ScriptHandlerTest : public UnoApiTest
{
public:
    ScriptHandlerTest()
        : UnoApiTest(u"/scripting/qa/extras"_ustr)
    {
    }

private:
    void testDispatchArguments();

    CPPUNIT_TEST_SUITE(ScriptHandlerTest);
    CPPUNIT_TEST(testDispatchArguments);
    CPPUNIT_TEST_SUITE_END();
};

void ScriptHandlerTest::testDispatchArguments()
{
    // Tests that the first argument to a macro when invoked via a dispatch URL is the URL itself.
    // See tdf#173160.
    rtl::Reference<DummyProvider> xDummyProvider = new DummyProvider;

    // Temporarily register our dummy provider with the service manager so that the master script
    // provider can find it
    css::uno::Reference<css::container::XSet> xServiceSet(m_xFactory, css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::lang::XServiceInfo> xService(xDummyProvider);
    xServiceSet->insert(css::uno::Any(xService));
    comphelper::ScopeGuard g(
        [&xServiceSet, &xService] { xServiceSet->remove(css::uno::Any(xService)); });

    css::uno::Sequence<css::beans::PropertyValue> aPropertyValues = {
        comphelper::makePropertyValue(u"FileName"_ustr, SCRIPT_URL),
        comphelper::makePropertyValue(u"InteractionHandler"_ustr,
                                      css::uno::Reference<css::task::XInteractionHandler>()),
        comphelper::makePropertyValue(u"OpenNewView"_ustr, false),
        comphelper::makePropertyValue(u"FrameName"_ustr, u"_self"_ustr),
        comphelper::makePropertyValue(u"MacroExecutionMode"_ustr,
                                      css::document::MacroExecMode::USE_CONFIG),
        comphelper::makePropertyValue(u"UpdateDocMode"_ustr,
                                      css::document::UpdateDocMode::ACCORDING_TO_CONFIG),
        comphelper::makePropertyValue(u"SynchronMode"_ustr, false),
    };

    loadFromURL(u"private:factory/swriter"_ustr);

    // Dispatch the .uno:Open command. We can’t use XDispatchHelper for this because it will set
    // SynchronMode to true and there isn’t a synchronous handler for it.

    css::uno::Reference<css::frame::XModel> xModel(mxComponent, css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::frame::XDispatchProvider> xDispatchProvider(
        xModel->getCurrentController(), css::uno::UNO_QUERY_THROW);

    css::uno::Reference<css::util::XURLTransformer> xParser
        = css::util::URLTransformer::create(m_xContext);
    css::util::URL aURL;
    aURL.Complete = ".uno:Open";
    xParser->parseStrict(aURL);

    css::uno::Reference<css::frame::XDispatch> xDispatch
        = xDispatchProvider->queryDispatch(aURL, OUString(), 0);
    CPPUNIT_ASSERT(xDispatch.is());

    xDispatch->dispatch(aURL, aPropertyValues);

    Scheduler::ProcessEventsToIdle();

    // Make sure the script was actually invoked
    CPPUNIT_ASSERT(xDummyProvider->m_bInvoked);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScriptHandlerTest);

} // namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
