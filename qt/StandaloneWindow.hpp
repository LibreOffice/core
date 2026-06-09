/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <QMainWindow>

#include <functional>
#include <memory>

class QCloseEvent;
class WebView;

// A window hosting a single WebView (non-tabbed): the starter screen, the
// welcome slideshow, or the presenter console. Owns the WebView and deletes
// it on close; the app autosaves on exit, so there is no save prompt.
class StandaloneWindow : public QMainWindow
{
public:
    static StandaloneWindow* wrap(WebView* webView, QWidget* parent = nullptr);

    void setCloseCallback(std::function<void()> cb) { _closeCallback = std::move(cb); }

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    StandaloneWindow(WebView* webView, QWidget* parent);

    std::unique_ptr<WebView> _webView;
    std::function<void()> _closeCallback;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
