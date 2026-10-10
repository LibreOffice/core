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

#include <QMainWindow>
#include <QString>

#include <functional>

class QWebEngineProfile;
class QWebEngineView;

/// Shows an integrator's web UI in a QWebEngineView.  WOPI params
/// are extracted from the intercepted iframe URL and the
/// wopiSelected() signal is emitted; the caller is expected to
/// read wopiSrc()/accessToken()/coolServer()/coolPath(), then
/// close() the picker so a separate WebView opens for the editor.
class IntegratorFilePicker : public QMainWindow
{
    Q_OBJECT
public:
    IntegratorFilePicker(const QString& serverUrl, QWebEngineProfile* profile,
                         QWidget* parent = nullptr);

    QString wopiSrc() const { return _wopiSrc; }
    QString accessToken() const { return _accessToken; }
    QString coolServer() const { return _coolServer; }
    QString coolPath() const { return _coolPath; }

    // InterceptPage needs access to set these
    friend class InterceptPage;

signals:
    /// Emitted after the picker has extracted the WOPI params from
    /// the integrator's intercepted iframe nav.  The caller reads
    /// them off the picker and then close()s it.
    void wopiSelected();

private:
    void extractAccessToken();
    /// Run integrator-specific JS to locate the WOPI access_token in
    /// the current page DOM.  `then` is called with the extracted
    /// value (empty string on failure).
    void extractAccessTokenAsync(
        std::function<void(const QString&)> then);

    QWebEngineView* _webView;
    QString _wopiSrc;
    QString _accessToken;
    QString _coolServer;
    QString _coolPath;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
