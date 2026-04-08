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

#include <QDialog>
#include <QString>
#include <QUrl>

class QWebEngineView;

/// Shows an integrator's web UI in a QWebEngineView.  When the user
/// opens a document, the integrator creates an iframe pointing to the
/// COOL server; we intercept that navigation to extract the WOPI
/// parameters (WOPISrc, access_token, COOL server URL).
///
/// The WOPISrc and COOL server URL are extracted generically from the
/// iframe URL (all integrators put WOPISrc there).  The access_token
/// extraction is integrator-specific; when the token is not in the
/// URL, known extraction strategies are tried in order.
class IntegratorFilePicker : public QDialog
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

private:
    void extractAccessToken();

    QWebEngineView* _webView;
    QString _wopiSrc;
    QString _accessToken;
    QString _coolServer;
    QString _coolPath;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
