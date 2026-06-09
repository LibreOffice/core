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
#include <QString>
#include <QWebEngineView>

#include <functional>
#include <memory>
#include <optional>

class Bridge;
class QWebEnginePage;
class WebView;
class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;
class QTemporaryDir;
class QUrl;

namespace coda
{
/// Create a QWebChannel on page, construct a Bridge bound to document
/// (with owner WebView and/or host window/webview), register the
/// Bridge as "bridge" on the channel, and attach the channel to the
/// page.  Shared between the normal open-in-a-tab flow and the
/// embed-mode flow.  owner is null for pages not hosted by a WebView
/// (e.g. the IntegratorFilePicker's QDialog); window is the fallback
/// host used in that case.
/// Ownership: Bridge is parented on the channel (which is parented on
/// the page), so everything lives as long as the page.
Bridge* attachRemoteBridge(QWebEnginePage* page,
                           coda::DocumentData& document,
                           WebView* owner, QWidget* window,
                           QWebEngineView* webView);

/// Add the CODA-local query parameters the Qt-flavored cool.html
/// expects (permission, appdocid, userinterfacemode, lang, dir,
/// darkTheme) to `url`.  file_path is intentionally omitted for
/// remote documents - the page-JS materialises the temp file via
/// Bridge::writeRemoteDocFile and uses the returned path itself.
void addRemoteCoolParams(QUrl& url, const coda::DocumentData& document);
} // namespace coda

// The user's portal "color-scheme" preference, cached on first call to avoid a
// D-Bus round-trip per tab: true = dark, false = light, nullopt = no preference.
std::optional<bool> portalPrefersDark();

class CODAWebEngineView : public QWebEngineView
{
public:
    CODAWebEngineView(QMainWindow* parent)
        : QWebEngineView(parent)
        , _mainWindow(parent)
        , _presentationView(nullptr)
        , _presenterConsole(nullptr)
    {
    }

    ~CODAWebEngineView();

    void setMainWindow(QMainWindow* mainWindow) { _mainWindow = mainWindow; }
    QMainWindow* mainWindow() const { return _mainWindow; }

    void arrangePresentationWindows();
    void exchangeMonitors();

    // Ends any running presentation by closing the slideshow and presenter
    // console windows (no-op otherwise).
    void endPresentation();

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

    // The hosting TabbedWindow or StandaloneWindow; updated when the view
    // moves between windows (tab drag).
    QMainWindow* _mainWindow;
    // Given the general inability of wayland based environments
    // to restore a window's position, especially after moving
    // it to another monitor full-screen, the slideshow and the
    // presenter console each play in their own window, opened
    // from the page through window.open. The original document
    // view is left untouched in its tab/window, so its
    // content stays live and there is no full-screen window to
    // restore.
    WebView* _presentationView;
    WebView* _presenterConsole;
    QMetaObject::Connection _screenAdded;
    QMetaObject::Connection _screenRemoved;

    QWebEngineView* createWindow(QWebEnginePage::WebWindowType type) override;
};

// One open document (or the starter screen). Owns its CODAWebEngineView and
// Bridge, but not a window: the containing TabbedWindow or StandaloneWindow
// hosts the view and is set via setMainWindow(). The owner deletes the WebView;
// it never deletes itself. Inherits QObject so Bridge::_owner can be a QPointer.
class WebView : public QObject
{
public:
    using CloseRequestFn = std::function<void()>;
    using TitleChangeFn = std::function<void(const QString&)>;

    explicit WebView(QWebEngineProfile* profile, bool isWelcome = false);
    ~WebView() override;

    CODAWebEngineView* webEngineView() { return _webView.get(); }

    void setMainWindow(QMainWindow* window);
    QMainWindow* mainWindow() const { return _mainWindow; }

    // When requiresSaveAs is true the document opens editable and its first save
    // is turned into a Save As, so the file at fileURL is never written to.
    void load(const Poco::URI& fileURL = Poco::URI(), bool newFile = false,
              bool isStarterMode = false, bool requiresSaveAs = false);
    void loadRemote(std::shared_ptr<coda::RemoteDocInfo> remoteInfo);

    // The owner installs these; Bridge calls requestClose()/updateTitle().
    void setOnCloseRequest(CloseRequestFn cb) { _onCloseRequest = std::move(cb); }
    void setOnTitleChange(TitleChangeFn cb) { _onTitleChange = std::move(cb); }
    void requestClose();
    void updateTitle(const QString& title);
    QString composedWindowTitle() const;

    QString documentTitle() const { return _docTitle; }
    // "writer" / "calc" / "impress" / "draw" / "starter" / "welcome" / "other"
    QString documentType() const { return _docType; }

    // Copies the template for `templateType` (or `templatePath` if set) into the
    // user's Documents dir under a free name. Returns the path, or empty on failure.
    static QString createNewDocumentFile(const std::string& templateType,
                                         const std::string& templatePath,
                                         const std::string& baseName);
    static QString docTypeFromExtension(const QString& filePath);

    // True when the file name has a document-template extension (.ott, .xltx, ...).
    static bool isTemplate(const std::string& fileName);

    // Open a template as a fresh editable document. The template is copied to a
    // temporary working file that is loaded instead of the original, and the
    // first save is redirected to Save As. The original template is never
    // modified. Returns nullptr if the working copy could not be created.
    static WebView* openTemplateAsNewDocument(QWebEngineProfile* profile,
                                              const Poco::URI& templateURL);

    static WebView* findOpenDocument(const Poco::URI& documentURI);
    static const std::vector<WebView*>& getAllInstances() { return s_instances; }
    void activateWindow();
    /// True while a user-initiated save is in flight - see
    /// Bridge::isSaveInFlight.  Used by the close handler so that
    /// closing the tab while the save round-trip is not yet
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
    /// valid - called from TabManager::closeTab before the WebView's
    /// destruction tears _document down.  No-op for local-only docs.
    void sendCollabBye();
    /// True after saveAndClose() has been initiated for this document;
    /// TabManager::closeTab uses this to skip the save-if-dirty path
    /// on the re-entry that runs after the JS-side CLOSE_WINDOW
    /// round-trip.
    bool isReadyToClose() const;
    /// Set the ready-to-close flag without going through JS.  Used by
    /// the save-in-flight defer path so the close that fires once the
    /// in-flight save finishes skips the redundant save-if-dirty
    /// round-trip.
    void markReadyToClose();
    /// Ask the page-JS to run _saveAndClose (saves if dirty, posts
    /// CLOSE_WINDOW on completion).  No-op if no bridge is attached.
    void saveAndClose();
    // Called when this document's window gains or loses the active state.
    void onWindowActiveChanged(bool active);
    bool isDocumentModified() const;
    void endPresentation();
    bool isStarterScreen() const { return _docType == QStringLiteral("starter"); }
    bool isWelcome() const { return _isWelcome; }

private:
    // query gnome font scaling factor and apply it to the web view
    void queryGnomeFontScalingUpdateZoom();

    QMainWindow* _mainWindow;
    std::unique_ptr<CODAWebEngineView> _webView;
    coda::DocumentData _document;
    QString _docTitle;
    QString _docType;
    bool _isWelcome;
    Bridge* _bridge;
    // Holds the temporary copy of a template while it is open as a new document,
    // so the copy is removed when the window closes.
    std::unique_ptr<QTemporaryDir> _templateWorkingDir;

    CloseRequestFn _onCloseRequest;
    TitleChangeFn _onTitleChange;

    static std::vector<WebView*> s_instances;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
