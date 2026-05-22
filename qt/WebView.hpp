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

#include "Document.hpp"

#include <Poco/URI.h>

#include <QMainWindow>
#include <QObject>
#include <QWebEngineView>

#include <functional>
#include <memory>

class Bridge;
class QWebEnginePage;
class WebView;
class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;
class QUrl;

namespace coda
{
/// Create a QWebChannel on page, construct a Bridge bound to document
/// (with host window/webview), register the Bridge as "bridge" on
/// the channel, and attach the channel to the page.  Shared between
/// the normal open-in-new-window flow and the embed-mode flow.
/// Ownership: Bridge is parented on the channel (which is parented on
/// the page), so everything lives as long as the page.
Bridge* attachRemoteBridge(QWebEnginePage* page,
                           coda::DocumentData& document,
                           QWidget* window, QWebEngineView* webView);

/// Add the CODA-local query parameters the Qt-flavored cool.html
/// expects (permission, appdocid, userinterfacemode, lang, dir,
/// darkTheme) to `url`.  file_path is intentionally omitted for
/// remote documents - the page-JS materialises the temp file via
/// Bridge::writeRemoteDocFile and uses the returned path itself.
void addRemoteCoolParams(QUrl& url, const coda::DocumentData& document);

/// Preferred window size for document viewing (full viewport) or for
/// the welcome/starter screen (~40% viewport, clamped to 800x450
/// .. 1280x720).
std::pair<int, int> documentWindowSize(bool isWelcomeOrStarter);
} // namespace coda

class CODAWebEngineView : public QWebEngineView
{
public:
    CODAWebEngineView(QMainWindow* parent)
        : QWebEngineView(parent)
        , _presentationView(nullptr)
        , _presenterConsole(nullptr)
    {
    }

    ~CODAWebEngineView();

    void arrangePresentationWindows();
    void exchangeMonitors();

protected:
    // Intercept files dropped onto the window from the OS
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    void connectScreenChanges();

    // Show or hide the web-side hint shown while a file is dragged over the window.
    void setDropFeedbackVisible(bool bVisible);

    void claimChildWindow(WebView* child, const QUrl& url);

    // Given the general inability of wayland based environments
    // to restore a window's position, especially after moving
    // it to another monitor full-screen, the slideshow and the
    // presenter console each play in their own window, opened
    // from the page through window.open. The original document
    // window is left untouched on its original screen, so its
    // content stays live and there is no full-screen window to
    // restore.
    WebView* _presentationView;
    WebView* _presenterConsole;
    QMetaObject::Connection _screenAdded;
    QMetaObject::Connection _screenRemoved;

    QWebEngineView* createWindow(QWebEnginePage::WebWindowType type) override;
};

class WebView
{
public:
    explicit WebView(QWebEngineProfile* profile, bool isWelcome = false, QMainWindow* parentWindow = nullptr);
    ~WebView();
    CODAWebEngineView* webEngineView() { return _webView.get(); }
    QMainWindow* mainWindow() { return _mainWindow; }

    void load(const Poco::URI& fileURL = Poco::URI(), bool newFile = false, bool isStarterMode = false);
    void loadRemote(std::shared_ptr<coda::RemoteDocInfo> remoteInfo);

    // templatePath and basename can be empty strings and are optional.
    static WebView* createNewDocument(QWebEngineProfile* profile, const std::string& templateType,
                                      const std::string& templatePath, const std::string& baseName);

    static WebView* findOpenDocument(const Poco::URI& documentURI);
    static WebView* findStarterScreen();
    static const std::vector<WebView*>& getAllInstances() { return s_instances; }
    void activateWindow();
    /// True while a user-initiated save is in flight - see
    /// Bridge::isSaveInFlight.  Used by the close handler so that
    /// clicking the title-bar X while the save round-trip is not yet
    /// done waits for the save rather than asking about "unsaved
    /// changes" that are actually about to be saved.
    bool isSaveInFlight() const;
    /// Register a one-shot callback to run when the in-flight save
    /// completes (success or failure).  Forwards to
    /// Bridge::onSaveComplete; runs immediately if no save is in
    /// flight.
    void onSaveComplete(std::function<void()> callback);
    /// Run @script in the page on the GUI thread.  Forwards to
    /// Bridge::evalJS; no-op if no bridge is attached.
    void evalJS(const std::string& script);
    /// Announce an orderly close to the per-document collab broker.
    /// Forwarded to Bridge::sendCollabBye() while _document is still
    /// valid - called from Window::closeEvent before owner_'s
    /// destruction tears _document down.  No-op for local-only docs.
    void sendCollabBye();
    /// True after saveAndClose() has been initiated for this window;
    /// the host window's closeEvent uses this to skip the save-if-
    /// dirty path on the re-entry that runs after the JS-side
    /// CLOSE_WINDOW round-trip.
    bool isReadyToClose() const;
    /// Set the ready-to-close flag without going through JS.  Used by
    /// the save-in-flight defer path so the close that fires once the
    /// in-flight save finishes skips the redundant save-if-dirty
    /// round-trip.
    void markReadyToClose();
    /// Ask the page-JS to run _saveAndClose (saves if dirty, posts
    /// CLOSE_WINDOW on completion).  No-op if no bridge is attached.
    void saveAndClose();
    bool isStarterScreen() const { return _document._fakeClientFd == -1 && _document._appDocId == 0; }
    QMainWindow* getMainWindow() const { return _mainWindow; }

private:
    // query gnome font scaling factor and apply it to the web view
    void queryGnomeFontScalingUpdateZoom();
    QMainWindow* _mainWindow;
    std::unique_ptr<CODAWebEngineView> _webView;
    coda::DocumentData _document;
    bool _isWelcome;
    Bridge* _bridge;

    static std::vector<WebView*> s_instances;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
