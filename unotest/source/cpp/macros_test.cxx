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

#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>

#include <basic/basrdll.hxx>
#include <cppunit/TestAssert.h>
#include <comphelper/sequence.hxx>
#include <comphelper/processfactory.hxx>

using namespace css;

namespace unotest {

MacrosTest::MacrosTest()
    : mpDll(std::make_unique<BasicDLL>())
{
}

MacrosTest::~MacrosTest() = default;

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

void MacrosTest::dispatchCommand(const uno::Reference<lang::XComponent>& xComponent,
                                 const OUString& rCommand,
                                 const uno::Sequence<beans::PropertyValue>& rPropertyValues)
{
    uno::Reference<frame::XController> xController
        = uno::Reference<frame::XModel>(xComponent, uno::UNO_QUERY_THROW)->getCurrentController();
    CPPUNIT_ASSERT(xController.is());
    uno::Reference<frame::XDispatchProvider> xFrame(xController->getFrame(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFrame.is());

    uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
    uno::Reference<frame::XDispatchHelper> xDispatchHelper(frame::DispatchHelper::create(xContext));
    CPPUNIT_ASSERT(xDispatchHelper.is());

    xDispatchHelper->executeDispatch(xFrame, rCommand, OUString(), 0, rPropertyValues);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
