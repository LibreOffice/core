/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/buttonstatuslistener.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/URLTransformer.hpp>

ButtonStatusListener::ButtonStatusListener(Button* button, const rtl::OUString& aCommand) {
    mButton = button;

    css::uno::Reference<css::uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
    css::uno::Reference<css::frame::XDesktop2> xDesktop = css::frame::Desktop::create(xContext);

    css::uno::Reference<css::frame::XFrame> xFrame(xDesktop->getActiveFrame());
    if (!xFrame.is())
        xFrame = css::uno::Reference<css::frame::XFrame>(xDesktop, css::uno::UNO_QUERY);

    css::uno::Reference<css::frame::XDispatchProvider> xDispatchProvider(xFrame, css::uno::UNO_QUERY);
    if (!xDispatchProvider.is())
        return;

    maCommandURL.Complete = aCommand;
    css::uno::Reference<css::util::XURLTransformer> xParser = css::util::URLTransformer::create(xContext);
    xParser->parseStrict(maCommandURL);

    mxDispatch = xDispatchProvider->queryDispatch(maCommandURL, "", 0);
    if (!mxDispatch.is())
        return;

    mxDispatch->addStatusListener(this, maCommandURL);
}

void ButtonStatusListener::statusChanged(const css::frame::FeatureStateEvent& rEvent)
            throw(css::uno::RuntimeException, std::exception)
{
    mButton->Enable(rEvent.IsEnabled);
}

void ButtonStatusListener::disposing(const css::lang::EventObject& /*Source*/)
            throw( css::uno::RuntimeException, std::exception )
{
    mxDispatch.clear();
}

void ButtonStatusListener::dispose()
{
    if (mxDispatch.is()) {
        mxDispatch->removeStatusListener(this, maCommandURL);
        mxDispatch.clear();
    }
    mButton.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */