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

#include "WindowUtils.hpp"

#include <QApplication>
#include <QGuiApplication>
#include <QKeySequence>
#include <QRect>
#include <QScreen>
#include <QShortcut>
#include <QWidget>

#include <algorithm>
#include <cmath>

QSize defaultWindowMinimumSize()
{
    // Window managers refuse to tile a window when its minimum size does not
    // fit the target tile, so keep the minimum small enough for tiling to work.
    return QSize(600, 450);
}

QSize defaultWindowSize()
{
    // Wide enough for the full notebookbar where the screen allows, but always
    // clearly smaller than the work area.
    const int fullToolbarWidth = 1760;
    const QRect available = QGuiApplication::primaryScreen()->availableGeometry();
    // Not std::clamp: it is undefined when lo > hi, and the work area can be
    // narrower or shorter than the floor below.
    const int width =
        std::min(std::max(static_cast<int>(std::lround(available.width() * 0.85)), 800),
                 std::min(fullToolbarWidth, available.width()));
    const int height =
        std::min(std::max(static_cast<int>(std::lround(available.height() * 0.80)), 600),
                 std::min(1200, available.height()));
    return QSize(width, height);
}

void centerOnWorkArea(QWidget* window)
{
    // Best effort: the compositor may place the window itself.
    const QRect work = QGuiApplication::primaryScreen()->availableGeometry();
    window->move(work.x() + (work.width() - window->width()) / 2,
                 work.y() + (work.height() - window->height()) / 2);
}

void surfaceWindow(QWidget* window)
{
    window->setWindowState(window->windowState() & ~Qt::WindowMinimized);
    window->raise();
    window->activateWindow();
}

void installQuitShortcut(QWidget* window)
{
    auto* quitShortcut = new QShortcut(QKeySequence::Quit, window);
    quitShortcut->setContext(Qt::WindowShortcut);
    QObject::connect(quitShortcut, &QShortcut::activated, [] { QApplication::closeAllWindows(); });
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
