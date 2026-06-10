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

#include "TabbedWindow.hpp"
#include "TabManager.hpp"
#include "WebView.hpp"
#include "WindowUtils.hpp"

#include <QCloseEvent>
#include <QEvent>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

QList<TabbedWindow*> TabbedWindow::s_windows;

TabbedWindow::TabbedWindow(QWebEngineProfile* profile, QWidget* parent)
    : QMainWindow(parent)
    , _profile(profile)
    , _manager(new TabManager(this, profile))
{
    QWidget* central = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(_manager->shellWidget());
    layout->addWidget(_manager->stackWidget(), /*stretch*/ 1);
    setCentralWidget(central);

    connect(_manager, &TabManager::requestWindowClose, this, &QMainWindow::close);

    setMinimumSize(defaultWindowMinimumSize());
    resize(defaultWindowSize());
    centerOnWorkArea(this);

    installQuitShortcut(this);

    s_windows.append(this);
}

TabbedWindow::~TabbedWindow()
{
    s_windows.removeAll(this);
    // Delete the manager (and its views) before Qt destroys the central widget,
    // since the views are owned by unique_ptr but parented into the stack.
    delete _manager;
    _manager = nullptr;
}

WebView* TabbedWindow::openFile(const Poco::URI& fileURL)
{
    return _manager->webViewForTab(_manager->addDocumentTab(fileURL));
}

void TabbedWindow::activateTabFor(WebView* wv)
{
    int id = _manager->tabIdForWebView(wv);
    if (id >= 0)
        _manager->activateTab(id);
    surfaceWindow(this);
}

TabbedWindow* TabbedWindow::getOrCreate(QWebEngineProfile* profile)
{
    for (int i = s_windows.size() - 1; i >= 0; --i)
        if (s_windows[i]->isVisible())
            return s_windows[i];
    TabbedWindow* tw = new TabbedWindow(profile);
    tw->show();
    return tw;
}

void TabbedWindow::closeEvent(QCloseEvent* event)
{
    // Close the tabs one at a time first, so each document gets its
    // save-if-dirty round-trip (see TabManager::closeTab). Once the last tab
    // is gone the manager emits requestWindowClose and we re-enter with an
    // empty tab list.
    if (_manager && _manager->tabCount() > 0)
    {
        event->ignore();
        _manager->requestCloseAll();
        return;
    }

    QMainWindow::closeEvent(event);
    // deleteLater, not WA_DeleteOnClose: a close can come from inside a
    // TabManager method (a drag adopting our last tab), so defer the delete
    // until we are back in the event loop.
    deleteLater();
}

bool TabbedWindow::event(QEvent* e)
{
    if (e->type() == QEvent::WindowActivate)
    {
        // Most-recently-activated last, so getOrCreate() picks it first.
        s_windows.removeAll(this);
        s_windows.append(this);
    }
    if (e->type() == QEvent::ActivationChange && _manager)
        _manager->onWindowActiveChanged(isActiveWindow());
    return QMainWindow::event(e);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
