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

#include <QObject>
#include <QPointer>
#include <QString>

#include <memory>
#include <string>
#include <vector>

class QStackedWidget;
class QWebEngineProfile;
class QWebEngineView;
class TabbedWindow;
class TabShellBridge;
class WebView;

// Owns the ordered list of tabs in a TabbedWindow, and bridges the HTML tab
// strip to the QStackedWidget that holds the document views.
class TabManager : public QObject
{
    Q_OBJECT

public:
    TabManager(TabbedWindow* window, QWebEngineProfile* profile);
    ~TabManager() override;

    QWidget* shellWidget() const;
    QStackedWidget* stackWidget() const { return _stack; }

    int activeTabId() const { return _activeTabId; }
    WebView* webViewForTab(int tabId) const;
    int tabIdForWebView(WebView* wv) const;

    // Return the new tab's id. requiresSaveAs turns the document's first save
    // into a Save As, so the file at fileURL is never written to (see
    // WebView::openTemplateAsNewDocument).
    int addStarterTab();
    int addDocumentTab(const Poco::URI& fileURL, bool newFile = false,
                       bool requiresSaveAs = false);

    int adoptTab(WebView* webView, int insertAt);
    std::unique_ptr<WebView> releaseTab(int tabId);

    int tabCount() const { return static_cast<int>(_tabs.size()); }

    // Runs the document's save-if-dirty round-trip first (see Bridge::
    // saveAndClose); the tab goes away once the page-JS confirms with
    // CLOSE_WINDOW. Emits requestWindowClose if this was the last tab.
    void closeTab(int tabId);

    // Close every tab in order (each with its save round-trip), then emit
    // requestWindowClose. Used by TabbedWindow::closeEvent.
    void requestCloseAll();

    void activateTab(int tabId);
    void reorderTab(int fromIndex, int toIndex);

    // Cross-window drag. Wayland freezes the source's cursor and drag coords
    // during a grab, so the move is driven by the target side: its live
    // dragover registers it as the hover target, and the source reads that at
    // dragend. Tab ids are unique process-wide, so a srcTabId not in this
    // strip's list belongs to another window.
    void onSourceDragEnded(int tabId, bool inStripDropHandled);
    int adoptFromOtherWindow(int srcTabId, int insertAt);
    void onTargetDragOver(int insertAt);
    void onTargetDragLeave();

    void onWebViewTitleChanged(WebView* wv, const QString& title);

    // Forwarded from TabbedWindow when the window gains or loses the active
    // state, so each document can trim its off-screen tiles while backgrounded.
    void onWindowActiveChanged(bool active);

    void emitTabsChangedNow();
    // Called by the strip JS once its QWebChannel slots are connected, so the
    // first state push is not lost to a signal emitted before JS was ready.
    void emitInitialState();
    void applyTheme(const QString& theme);

signals:
    void requestWindowClose();

private:
    struct Entry
    {
        int id = 0;
        std::unique_ptr<WebView> webView;
        // A close is parked on this tab's save-in-flight completion (see
        // closeTab), holding one override cursor that the parked callback
        // (or detachAt, if the tab leaves another way) must release.
        bool closeWaitsForSave = false;
    };

    int registerTab(std::unique_ptr<WebView> wv, int insertAt);
    std::unique_ptr<WebView> detachAt(std::vector<Entry>::iterator it);
    std::vector<Entry>::iterator findTab(int tabId);
    std::vector<Entry>::const_iterator findTab(int tabId) const;
    QString currentTabsJson() const;

    TabbedWindow* _window;
    QWebEngineProfile* _profile;
    QStackedWidget* _stack;
    QWebEngineView* _shellView;
    TabShellBridge* _shellBridge;

    std::vector<Entry> _tabs;
    static int s_nextTabId;
    int _activeTabId = -1;
    // Set by requestCloseAll(): after each tab finishes closing, closeTab()
    // continues with the next one until none remain.
    bool _closingAll = false;

    // The strip currently under a cross-window drag, and the insert index its
    // last dragover settled on. One drag at a time, so these are process-wide;
    // the QPointer self-nulls if the target window dies mid-drag.
    static QPointer<TabManager> s_dragHoverTarget;
    static int s_dragHoverInsertAt;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
