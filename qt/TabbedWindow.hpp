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

#include <Poco/URI.h>

#include <QList>
#include <QMainWindow>

class QWebEngineProfile;
class TabManager;
class WebView;

// Top-level window hosting one or more open documents as tabs.
class TabbedWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit TabbedWindow(QWebEngineProfile* profile, QWidget* parent = nullptr);
    ~TabbedWindow() override;

    TabManager* manager() const { return _manager; }

    // Opens the file as a new tab and returns its WebView. A missing or
    // unreadable file still gets a tab; the page surfaces the load error.
    WebView* openFile(const Poco::URI& fileURL);

    void activateTabFor(WebView* wv);

    // The most-recently-active visible window, or a fresh one if none exist.
    static TabbedWindow* getOrCreate(QWebEngineProfile* profile);

    static QList<TabbedWindow*> allWindows() { return s_windows; }

protected:
    void closeEvent(QCloseEvent* event) override;
    bool event(QEvent* e) override;

private:
    QWebEngineProfile* _profile;
    TabManager* _manager;

    static QList<TabbedWindow*> s_windows;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
