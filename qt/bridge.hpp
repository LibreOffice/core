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

    void promptSaveLocation(std::function<void(const std::string&, const std::string&)> callback);
    void saveDocumentAs();
    void createAndStartMessagePumpThread();
    // Reconnect and load the document again after the server rejected the load
    // because the previous use of the same document was still being unloaded.
    void retryLoadAfterUnloading();

    void showProgressSnackbar();
    void closeSnackbar();

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
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
