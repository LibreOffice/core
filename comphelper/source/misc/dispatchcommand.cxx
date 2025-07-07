/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/URLTransformer.hpp>

using namespace css;

namespace comphelper {

bool dispatchCommand(const OUString& rCommand,
                     const uno::Reference<uno::XInterface>& xDispatchSource,
                     const uno::Sequence<beans::PropertyValue>& rArguments,
                     const uno::Reference<frame::XDispatchResultListener>& rListener)
{
    uno::Reference<frame::XDispatchProvider> xDispatchProvider(xDispatchSource, uno::UNO_QUERY);
    if (!xDispatchProvider.is())
        return false;

    util::URL aCommandURL;
    aCommandURL.Complete = rCommand;
    const uno::Reference<uno::XComponentContext>& xContext = ::comphelper::getProcessComponentContext();
    uno::Reference<util::XURLTransformer> xParser = util::URLTransformer::create(xContext);
    xParser->parseStrict(aCommandURL);

    uno::Reference<frame::XDispatch> xDisp = xDispatchProvider->queryDispatch(aCommandURL, OUString(), 0);
    if (!xDisp.is())
        return false;

    // And do the work...
    if (rListener.is())
    {
        uno::Reference<frame::XNotifyingDispatch> xNotifyingDisp(xDisp, uno::UNO_QUERY);
        if (xNotifyingDisp.is())
        {
            xNotifyingDisp->dispatchWithNotification(aCommandURL, rArguments, rListener);
            return true;
        }
    }

    xDisp->dispatch(aCommandURL, rArguments);

    return true;
}

bool dispatchCommand(const OUString& rCommand, const css::uno::Sequence<css::beans::PropertyValue>& rArguments, const uno::Reference<css::frame::XDispatchResultListener>& rListener)
{
    // Target where we will execute the .uno: command
    const uno::Reference<uno::XComponentContext>& xContext = ::comphelper::getProcessComponentContext();
    uno::Reference<frame::XDesktop2> xDesktop = frame::Desktop::create(xContext);

    uno::Reference<frame::XFrame> xFrame(xDesktop->getActiveFrame());
    if (!xFrame.is())
        xFrame = xDesktop;

    return dispatchCommand(rCommand, xFrame, rArguments, rListener);
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
