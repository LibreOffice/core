/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <comphelper/dispatchcommand.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/DispatchResultEvent.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/URLTransformer.hpp>

#include <algorithm>
#include <iterator>
#include <vector>

using namespace css;
using namespace ::cpo;

namespace comphelper {

bool dispatchCommand(const OUString& rCommand,
                     const uno::Reference<frame::XDispatchProvider>& xDispatchProvider,
                     const cpo::uno::Sequence<beans::PropertyValue>& rArguments,
                     const uno::Reference<frame::XDispatchResultListener>& xListener)
{
    if (!xDispatchProvider.is())
        return false;

    util::URL aCommandURL;
    aCommandURL.Complete = rCommand;
    const uno::Reference<cpo::uno::XComponentContext>& xContext = ::comphelper::getProcessComponentContext();
    uno::Reference<util::XURLTransformer> xParser = util::URLTransformer::create(xContext);
    xParser->parseStrict(aCommandURL);

    uno::Reference<frame::XDispatch> xDisp = xDispatchProvider->queryDispatch(aCommandURL, OUString(), 0);
    if (!xDisp.is())
        return false;

    // Without a listener, nothing waits for a result, so the command runs as the caller asked,
    // synchronously or not.
    if (!xListener.is())
    {
        xDisp->dispatch(aCommandURL, rArguments);
        return true;
    }

    uno::Reference<frame::XNotifyingDispatch> xNotifyingDisp(xDisp, uno::UNO_QUERY);
    if (xNotifyingDisp.is())
    {
        xNotifyingDisp->dispatchWithNotification(aCommandURL, rArguments, xListener);
        return true;
    }

    // This dispatch does not implement XNotifyingDispatch, so it has no way to tell the listener
    // how the command went. It is asked to run the command before it returns, so that the command
    // has run when the listener is told, with an unknown outcome. This holds for every command
    // with a listener that reaches such a dispatch, whatever mode the caller asked for. A dispatch
    // that ignores SynchronMode still runs the command later, and the listener is told before then.
    std::vector<beans::PropertyValue> aArguments;
    aArguments.reserve(rArguments.getLength() + 1);
    std::copy_if(rArguments.begin(), rArguments.end(), std::back_inserter(aArguments),
                 [](const beans::PropertyValue& rArgument)
                 { return rArgument.Name != "SynchronMode"; });
    aArguments.push_back(comphelper::makePropertyValue(u"SynchronMode"_ustr, true));
    xDisp->dispatch(aCommandURL, comphelper::containerToSequence(aArguments));

    frame::DispatchResultEvent aEvent;
    aEvent.Source = xDisp;
    aEvent.State = frame::DispatchResultState::DONTKNOW;
    xListener->dispatchFinished(aEvent);

    return true;
}

bool dispatchCommand(const OUString& rCommand,
                     const uno::Reference<frame::XFrame>& xFrame,
                     const cpo::uno::Sequence<beans::PropertyValue>& rArguments,
                     const uno::Reference<frame::XDispatchResultListener>& xListener)
{
    return dispatchCommand(rCommand, xFrame.query<frame::XDispatchProvider>(), rArguments, xListener);
}

bool dispatchCommand(const OUString& rCommand,
                     const uno::Reference<frame::XController>& xController,
                     const cpo::uno::Sequence<beans::PropertyValue>& rArguments,
                     const uno::Reference<frame::XDispatchResultListener>& xListener)
{
    return dispatchCommand(rCommand, xController.query<frame::XDispatchProvider>(), rArguments, xListener);
}

bool dispatchCommand(const OUString& rCommand,
                     const cpo::uno::Sequence<beans::PropertyValue>& rArguments,
                     const uno::Reference<frame::XDispatchResultListener>& xListener)
{
    // Target where we will execute the .uno: command
    const uno::Reference<cpo::uno::XComponentContext>& xContext = ::comphelper::getProcessComponentContext();
    uno::Reference<frame::XDesktop> xDesktop = frame::Desktop::create(xContext);

    uno::Reference<frame::XFrame> xFrame(xDesktop->getActiveFrame());
    if (!xFrame.is())
        xFrame = xDesktop;

    return dispatchCommand(rCommand, xFrame, rArguments, xListener);
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
