/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "unotest/macros_test.hxx"

#include <vector>

#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>

#include "cppunit/TestAssert.h"
#include <rtl/ustrbuf.hxx>
#include <comphelper/sequence.hxx>

using namespace css;

namespace unotest {

uno::Reference<css::lang::XComponent> MacrosTest::loadFromDesktop(const OUString& rURL, const OUString& rDocService, const uno::Sequence<beans::PropertyValue>& rExtraArgs)
{
    CPPUNIT_ASSERT_MESSAGE("no desktop", mxDesktop.is());
    uno::Reference<frame::XComponentLoader> xLoader = uno::Reference<frame::XComponentLoader>(mxDesktop, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("no loader", xLoader.is());
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

    uno::Reference<lang::XComponent> xComponent = xLoader->loadComponentFromURL(rURL, "_default", 0, comphelper::containerToSequence(args));
    OUString sMessage = "loading failed: " + rURL;
    CPPUNIT_ASSERT_MESSAGE(OUStringToOString( sMessage, RTL_TEXTENCODING_UTF8 ).getStr( ), xComponent.is());
    return xComponent;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
