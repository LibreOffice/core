/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/buttonstatuslistener.hxx>


ButtonStatusListener::ButtonStatusListener(Button* button) {
    mButton = button;
}
ButtonStatusListener::~ButtonStatusListener() {
    mButton.disposeAndClear();
}

void ButtonStatusListener::statusChanged(const css::frame::FeatureStateEvent& rEvent)
            throw(css::uno::RuntimeException, std::exception)
{
    mButton->Enable(rEvent.IsEnabled);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */