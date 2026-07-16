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

#include <QObject>
#include <QString>

class TabManager;

// QObject exposed to the tab strip page over QWebChannel: JS calls the slots,
// C++ emits the signals. Each slot forwards to TabManager.
class TabShellBridge : public QObject
{
    Q_OBJECT

public:
    explicit TabShellBridge(TabManager* manager);

public slots:
    void requestSync(); // strip is ready; push the initial state
    void tabActivated(int tabId);
    void tabCloseRequested(int tabId);
    void newTabRequested();
    void tabReordered(int fromIndex, int toIndex);
    void tabDragStarted(int tabId);
    void tabDragEnded(int tabId, bool inStripDropHandled);
    void tabAdoptFromOtherWindow(int srcTabId, int insertAt);
    void targetDragOver(int insertAt);
    void debug(const QString& message);

signals:
    // tabsJson: JSON array of {id, title, docType, modified, active}.
    void tabsChanged(const QString& tabsJson);
    void themeChanged(const QString& theme);
    // The native drag left the strip's view; the page clears its markers.
    void dragExited();

private:
    TabManager* _manager;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
