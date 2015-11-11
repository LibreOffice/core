/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/buttonstatuslistener.hxx>


ButtonStatusListener::ButtonStatusListener(Button* button, css::util::URL aCommandURL, css::uno::Reference<css::frame::XDispatch> xDispatch) {
    mButton = button;
    maCommandURL = aCommandURL;
    mxDispatch = xDispatch;
}
ButtonStatusListener::~ButtonStatusListener() {}

void ButtonStatusListener::statusChanged(const css::frame::FeatureStateEvent& rEvent)
            throw(css::uno::RuntimeException, std::exception)
{
    mButton->SetStateUno(rEvent);
}

void ButtonStatusListener::disposing(const css::lang::EventObject& /*Source*/)
            throw( css::uno::RuntimeException, std::exception )
{
    mxDispatch->removeStatusListener(this, maCommandURL);
    mxDispatch.clear();
    mButton.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */