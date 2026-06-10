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

#include <config.h>

#include "StandaloneWindow.hpp"
#include "WebView.hpp"
#include "WindowUtils.hpp"

#include <QCloseEvent>
#include <QGuiApplication>
#include <QScreen>

#include <algorithm>
#include <utility>

namespace
{
// Welcome dialog: 40% of screen width clamped to [800x450, 1280x720] at 16:9.
// Starter / presenter console: 1.5x that.
std::pair<int, int> standaloneWindowSize(bool isWelcome)
{
    const int viewportWidth = QGuiApplication::primaryScreen()->availableGeometry().width();
    int width = std::clamp(static_cast<int>(viewportWidth * 0.4), 800, 1280);
    int height = std::clamp(static_cast<int>((width * 9.0) / 16.0), 450, 720);
    if (!isWelcome)
    {
        width = static_cast<int>(1.5 * width);
        height = static_cast<int>(1.5 * height);
    }
    return { width, height };
}
} // namespace

StandaloneWindow::StandaloneWindow(WebView* webView, QWidget* parent)
    : QMainWindow(parent)
    , _webView(webView)
{
    setCentralWidget(webView->webEngineView());
    webView->setMainWindow(this);

    const bool isWelcome = webView->isWelcome();
    if (isWelcome)
    {
        setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        setWindowModality(Qt::WindowModal);
    }
    else
        setMinimumSize(defaultWindowMinimumSize());

    auto[w, h] = standaloneWindowSize(isWelcome);
    resize(w, h);
    if (!isWelcome)
        centerOnWorkArea(this);

    installQuitShortcut(this);
}

StandaloneWindow* StandaloneWindow::wrap(WebView* webView, QWidget* parent)
{
    StandaloneWindow* w = new StandaloneWindow(webView, parent);
    w->show();
    return w;
}

void StandaloneWindow::closeEvent(QCloseEvent* event)
{
    if (_closeCallback)
    {
        // Move out first so a re-entrant close can't run it twice.
        auto cb = std::move(_closeCallback);
        cb();
    }
    _webView.reset();

    QMainWindow::closeEvent(event);
    deleteLater();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
