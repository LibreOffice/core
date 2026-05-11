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

#include <qt/Document.hpp>

#include <QMainWindow>
#include <QString>
#include <QUrl>
#include <QUrlQuery>

#include <functional>

class Bridge;
class QCloseEvent;
class QWebEngineView;

/// Shows an integrator's web UI in a QWebEngineView, and (when the
/// integrator implements the X-Collab-Frame-Origin protocol) morphs
/// into the embedded document editor in-place once the user picks a
/// document.  Because the same window is both the picker and the
/// editor, it inherits from QMainWindow rather than QDialog: that
/// way the close-with-unsaved-changes prompt and other editor-window
/// semantics match the open-in-new-window flow's WebView.
///
/// In the non-embed flow (integrator does not echo our header),
/// WOPI params are extracted from the intercepted iframe URL and
/// the wopiSelected() signal is emitted; the caller is expected to
/// read wopiSrc()/accessToken()/coolServer()/coolPath(), then
/// close() the picker so a separate WebView opens for the editor.
class IntegratorFilePicker : public QMainWindow
{
    Q_OBJECT
public:
    explicit IntegratorFilePicker(const QString& serverUrl,
                                  QWidget* parent = nullptr);

    QString wopiSrc() const { return _wopiSrc; }
    QString accessToken() const { return _accessToken; }
    QString coolServer() const { return _coolServer; }
    QString coolPath() const { return _coolPath; }

    // InterceptPage needs access to set these
    friend class InterceptPage;

signals:
    /// Emitted in the non-embed flow after the picker has extracted
    /// the WOPI params from the integrator's intercepted iframe nav.
    /// The caller reads them off the picker and then close()s it.
    void wopiSelected();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void extractAccessToken();
    /// Run integrator-specific JS to locate the WOPI access_token in
    /// the current page DOM.  `then` is called with the extracted
    /// value (empty string on failure).
    void extractAccessTokenAsync(
        std::function<void(const QString&)> then);
    /// Embed-mode download-and-attach: fetches the document via
    /// /co/collab, populates _document, attaches a Bridge to the
    /// picker page, and navigates the picker to the local-server
    /// cool.html.  origQuery carries through the UI hints NC set on
    /// the intercepted iframe URL (lang, closebutton, ...).
    void attachEmbeddedDocument(const QString& wopiSrc,
                                const QString& accessToken,
                                const QString& coolServer,
                                const QString& coolPath,
                                QUrlQuery origQuery);

    QWebEngineView* _webView;
    QString _wopiSrc;
    QString _accessToken;
    QString _coolServer;
    QString _coolPath;
    /// Port of the embed-mode HTTP server (0 when embed mode is off).
    quint16 _embedPort = 0;
    /// Embed-mode: populated document + Bridge, owned by the picker
    /// for the lifetime of the dialog.
    coda::DocumentData _document;
    Bridge* _bridge = nullptr;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
