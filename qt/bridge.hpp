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

#include <QWebEngineView>
#include <QObject>
#include <QStringList>
#include <QVariant>
#include <functional>
#include <string>
#include <thread>
#include "Document.hpp"

class QWidget;

// Qt ⇄ JavaScript bridge.  Bridge is attached to a QWebEnginePage
// (via QWebChannel) and brokers messages between the page's JS and
// the rest of CODA.  The host window/webview are taken as the base
// Qt types so that Bridge can attach to any page (e.g., the
// IntegratorFilePicker's QDialog + QWebEngineView); CODA-specific
// methods (presentation full-screen, monitor exchange) are reached
// via qobject_cast at their call sites.
class Bridge : public QObject
{
    Q_OBJECT

    coda::DocumentData& _document;
    QWidget* _window;
    QWebEngineView* _webView;
    int _closeNotificationPipeForForwardingThread[2];
    std::thread _app2js;
    // true between sending a copy/cut command and receiving its COMMANDRESULT
    bool _copyInProgress = false;
    // How many times we have reloaded the page because the server was still
    // cleaning up the previous use of this document when we tried to load it.
    int _docUnloadingRetries = 0;
    // true between showing the deferred cross-window paste progress snackbar and
    // receiving the paste's COMMANDRESULT; touched only on the GUI thread.
    bool _pasteInProgress = false;
    // true from JS-side SAVESTARTED until the JS-side SAVECOMPLETED:
    // that brackets both the .uno:Save round-trip and (for remote
    // docs) the JS-driven collabUploadFile POST that pushes the
    // just-saved bytes to the integrator.  Watched by close-handlers
    // so they don't prompt about "unsaved changes" during the window
    // where core's status indicator has already gone away but the
    // save is not actually finished.
    bool _saveInFlight = false;
    // One-shot callback to run when _saveInFlight transitions back to
    // false.  Used by closeEvent paths to defer the window-close
    // until the in-flight save has fully completed.
    std::function<void()> _onSaveComplete;

    void promptSaveLocation(std::function<void(const std::string&, const std::string&)> callback);
    void saveDocumentAs();
    void createAndStartMessagePumpThread();
    // Reconnect and load the document again after the server rejected the load
    // because the previous use of the same document was still being unloaded.
    void retryLoadAfterUnloading();

    void showProgressSnackbar();
    void closeSnackbar();

    /// Mark the current save as complete: clear _saveInFlight and
    /// invoke (then drop) any pending _onSaveComplete callback.
    void finishSave();


public:
    explicit Bridge(QObject* parent, coda::DocumentData& document, QWidget* window, QWebEngineView* webView)
        : QObject(parent)
        , _document(document)
        , _window(window)
        , _webView(webView)
        , _closeNotificationPipeForForwardingThread{ -1, -1 }
    {
    }

    ~Bridge() override;

    // TODO: move these to webview...
    // Helper: post JavaScript code safely on GUI thread
    void evalJS(const std::string& script);

    // send Online → JS
    void send2JS(const std::vector<char>& buffer);

    /// True while a user-initiated .uno:Save is in flight (between
    /// the JS-side SAVESTARTED hand-off and the COMMANDRESULT, plus -
    /// for remote docs - the subsequent integrator upload).  Close
    /// handlers consult this so they don't show the "unsaved changes"
    /// prompt during the window after core's "Saving..." status
    /// indicator has gone away but the dispatch has not yet round-
    /// tripped back through the COMMANDRESULT path.
    bool isSaveInFlight() const { return _saveInFlight; }

    /// Register a one-shot callback to run when the in-flight save
    /// completes (success or failure).  Replaces any previously
    /// registered callback.  If no save is in flight, the callback
    /// is invoked immediately.
    void onSaveComplete(std::function<void()> callback);

    /// Announce an orderly close to the per-document collab broker
    /// by asking the page's JS to send {"type":"bye"} on its
    /// WebSocket.  Must be called from the host window's closeEvent
    /// while the bridge is still alive.  No-op for local-only docs.
    void sendCollabBye();

public slots: // called from JavaScript
    // Called from JS via window.postMobileMessage
    void debug(const QString& msg);
    // Called from JS via window.postMobileError
    void error(const QString& msg);
    /**
    * Called from JS via window.postMobileMessage()
    *
    * If the function has a meaningful reply for JavaScript, return a valid QVariant holding a
    * QString — this arrives in JS as that string. Otherwise return an *invalid* QVariant (e.g.
    * `return {}` or `return QVariant{}`); the Qt-to-JS marshaller converts an invalid QVariant to
    * the JavaScript value **undefined**.
    */
    QVariant cool(const QString& msg);

    /// Return all persistent prefs as a single flat JSON object
    /// (string keys, string values).  The JS side calls this once
    /// at WebChannel connect to populate its pref cache in one
    /// synchronous-from-JS'-view shot; subsequent get() reads hit
    /// the cache instead of round-tripping here.
    QString getAllPrefs();

    /// Persist @key = @value across CODA invocations.  Writes
    /// through to the prefs file so the next getAllPrefs() (on any
    /// webview) sees it.
    void setPref(const QString& key, const QString& value);

    /// Return WOPI connection params (wopiSrc, accessToken,
    /// coolServer, coolPath) for a remote document as a JSON object
    /// string, or the empty string for a local-only doc.  Called by
    /// the page-JS bootstrap to decide whether to open the collab
    /// WebSocket and fetch the document body before the standard
    /// load flow runs.
    QString getRemoteInfo();

    /// Decode @base64Bytes and write them to a fresh temp file
    /// preserving the extension of @filename, set
    /// _document._fileURL to the resulting path, and return that
    /// path so the page-JS can advance the standard load flow.
    /// Used by remote-doc bootstrap after the JS-side collab fetch
    /// gives us the bytes.
    QString writeRemoteDocFile(const QString& filename,
                               const QString& base64Bytes);

    /// Read _document._fileURL bytes (the file LOKit just wrote on
    /// a successful .uno:Save) and return them base64-encoded so
    /// the page-JS upload path can POST them through the collab
    /// /co/collab/put endpoint.  Returns the empty string when
    /// there is no remote document or the file cannot be read.
    QString readLocalDocBytes();

    /// JS-side notification that a save (and, for remote docs,
    /// the subsequent integrator upload) has finished.  Clears
    /// _saveInFlight and runs any pending _onSaveComplete callback
    /// (e.g. a deferred window-close).
    void saveCompleted();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
