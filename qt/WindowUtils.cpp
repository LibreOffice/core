/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include "WindowUtils.hpp"

#include "TabManager.hpp"
#include "TabbedWindow.hpp"

#include <QApplication>
#include <QGuiApplication>
#include <QKeySequence>
#include <QPointer>
#include <QRect>
#include <QScreen>
#include <QShortcut>
#include <QWidget>

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

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

namespace
{
struct Quit
{
    std::vector<QPointer<TabbedWindow>> tabbedWindows;
    std::vector<QPointer<QWidget>> otherWindows;
};

// The quit under way, if any. A quit asked for again starts over in place of the old one.
std::shared_ptr<Quit> currentQuit;

void continueQuit(const std::shared_ptr<Quit>& quit)
{
    if (quit != currentQuit)
        return;

    // One document window at a time, so that a close stopped in one window (a Cancel in its
    // prompt) stops the quit before the windows after it are touched. A window closes its tabs
    // one at a time and goes away after the last one.
    while (!quit->tabbedWindows.empty())
    {
        QPointer<TabbedWindow> window = quit->tabbedWindows.front();
        quit->tabbedWindows.erase(quit->tabbedWindows.begin());
        if (!window || !window->isVisible())
            continue;

        QObject::connect(window, &QObject::destroyed, window, [quit]() { continueQuit(quit); });
        QObject::connect(window->manager(), &TabManager::closeAllStopped, window->manager(),
                         [quit]()
                         {
                             if (quit == currentQuit)
                                 currentQuit.reset();
                         });
        window->close();
        return;
    }

    // The pointers are guarded because closing one window destroys the windows parented to it.
    for (const QPointer<QWidget>& window : quit->otherWindows)
    {
        if (window)
            window->close();
    }
    currentQuit.reset();
}
}

void closeEveryWindow()
{
    // Every visible window is asked to close, the same way the window manager's close button
    // closes it, so a document that has changed is saved before its window closes. The document
    // windows go first, the most recently active one first. The other windows, the dialogs
    // among them, are closed once every document window is gone, so a dialog that a document
    // window still waits on stays up.
    //
    // A window can be visible without being drawn on screen, and one of those keeps the
    // application running just as a drawn one does, so visibility is the whole test.
    auto quit = std::make_shared<Quit>();
    const QList<TabbedWindow*> tabbedWindows = TabbedWindow::allWindows();
    for (auto it = tabbedWindows.rbegin(); it != tabbedWindows.rend(); ++it)
    {
        if ((*it)->isVisible())
            quit->tabbedWindows.emplace_back(*it);
    }
    for (QWidget* widget : QApplication::topLevelWidgets())
    {
        if (widget->isVisible() && !qobject_cast<TabbedWindow*>(widget))
            quit->otherWindows.emplace_back(widget);
    }

    currentQuit = quit;
    continueQuit(quit);
}

void installQuitShortcut(QWidget* window)
{
    auto* quitShortcut = new QShortcut(QKeySequence::Quit, window);
    quitShortcut->setContext(Qt::WindowShortcut);
    QObject::connect(quitShortcut, &QShortcut::activated, [] { closeEveryWindow(); });
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
