/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <unotest/macros_test.hxx>

#include <vector>

#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/util/URLTransformer.hpp>

#include <cppunit/TestAssert.h>
#include <rtl/ustrbuf.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/sequence.hxx>

using namespace css;

namespace unotest {

uno::Reference<css::lang::XComponent> MacrosTest::loadFromDesktop(const OUString& rURL, const OUString& rDocService, const uno::Sequence<beans::PropertyValue>& rExtraArgs)
{
    CPPUNIT_ASSERT_MESSAGE("no desktop", mxDesktop.is());
    std::vector<beans::PropertyValue> args;
    beans::PropertyValue aMacroValue;
    aMacroValue.Name = "MacroExecutionMode";
    aMacroValue.Handle = -1;
    aMacroValue.Value <<= document::MacroExecMode::ALWAYS_EXECUTE_NO_WARN;
    aMacroValue.State = beans::PropertyState_DIRECT_VALUE;
    args.push_back(aMacroValue);

    if (!rDocService.isEmpty())
    {
        beans::PropertyValue aValue;
        aValue.Name = "DocumentService";
        aValue.Handle = -1;
        aValue.Value <<= rDocService;
        aValue.State = beans::PropertyState_DIRECT_VALUE;
        args.push_back(aValue);
    }

    args.insert(args.end(), rExtraArgs.begin(), rExtraArgs.end());

    uno::Reference<lang::XComponent> xComponent = mxDesktop->loadComponentFromURL(rURL, "_default", 0, comphelper::containerToSequence(args));
    OUString sMessage = "loading failed: " + rURL;
    CPPUNIT_ASSERT_MESSAGE(OUStringToOString( sMessage, RTL_TEXTENCODING_UTF8 ).getStr( ), xComponent.is());
    return xComponent;
}

namespace
{
class StateGetter : public ::cppu::WeakImplHelper<frame::XStatusListener>
{
public:
    uno::Any& m_rOldValue;
    bool m_Received{ false };
    StateGetter(uno::Any& rOldValue)
        : m_rOldValue(rOldValue)
    {
    }

    virtual void SAL_CALL disposing(lang::EventObject const&) override
    {
        CPPUNIT_ASSERT(m_Received);
    }
    virtual void SAL_CALL statusChanged(frame::FeatureStateEvent const& rEvent) override
    {
        if (!m_Received)
        {
            m_rOldValue = rEvent.State;
            m_Received = true;
        }
    }
};

} // namespace

uno::Any MacrosTest::queryDispatchStatus(uno::Reference<lang::XComponent> const& xComponent,
                                         uno::Reference<uno::XComponentContext> const& xContext,
                                         OUString const& rURL)
{
    uno::Any ret;

    util::URL url;
    url.Complete = rURL;
    {
        uno::Reference<css::util::XURLTransformer> const xParser(
            css::util::URLTransformer::create(xContext));
        CPPUNIT_ASSERT(xParser.is());
        xParser->parseStrict(url);
    }

    uno::Reference<frame::XController> const xController
        = uno::Reference<frame::XModel>(xComponent, uno::UNO_QUERY_THROW)->getCurrentController();
    uno::Reference<frame::XDispatchProvider> const xFrame(xController->getFrame(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFrame.is());
    uno::Reference<frame::XDispatch> const xDisp(xFrame->queryDispatch(url, "", 0));
    CPPUNIT_ASSERT(xDisp.is());

    uno::Reference<frame::XStatusListener> const xListener{ new StateGetter(ret) };
    xDisp->addStatusListener(xListener, url);

    return ret;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
