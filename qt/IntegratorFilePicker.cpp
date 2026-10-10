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

#include <config.h>

#include <qt/IntegratorFilePicker.hpp>

#include <common/Log.hpp>

#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrlQuery>
#include <QWebEnginePage>
#include <QWebEngineView>

/// Custom page that intercepts iframe navigations to detect the COOL
/// URL that the integrator constructs when opening a document.
class InterceptPage : public QWebEnginePage
{
public:
    IntegratorFilePicker* picker;

    using QWebEnginePage::QWebEnginePage;

protected:
    bool acceptNavigationRequest(
        const QUrl& url, NavigationType type, bool isMainFrame) override
    {
        QUrlQuery q(url);
        if (q.hasQueryItem("WOPISrc"))
        {
            LOG_TRC("IntegratorFilePicker: intercepted COOL URL: "
                    << url.toString().toStdString());

            picker->_wopiSrc = q.queryItemValue(
                "WOPISrc", QUrl::FullyDecoded);
            picker->_accessToken = q.queryItemValue(
                "access_token", QUrl::FullyDecoded);
            picker->_coolServer = url.scheme() + "://"
                + url.host()
                + (url.port(-1) != -1
                       ? ":" + QString::number(url.port())
                       : QString());
            picker->_coolPath = url.path();

            if (picker->_accessToken.isEmpty())
            {
                // Token not in the URL - let the integrator-specific
                // subclass extract it.
                picker->extractAccessToken();
            }
            else
            {
                emit picker->wopiSelected();
            }
            return false;
        }

        return QWebEnginePage::acceptNavigationRequest(
            url, type, isMainFrame);
    }

    QWebEnginePage* createWindow(WebWindowType) override
    {
        return this;
    }
};

namespace
{
/// Probe serverUrl and, when the integrator is recognized, return a
/// URL that drops the user straight onto the files view instead of
/// the default landing page.  Returns serverUrl unchanged otherwise.
QString resolveLandingUrl(const QString& serverUrl)
{
    QUrl base(serverUrl);
    QUrl probe = base;
    QString probePath = base.path();
    if (!probePath.endsWith('/'))
        probePath += '/';
    probe.setPath(probePath + "status.php");

    QNetworkAccessManager nam;
    QEventLoop loop;
    QNetworkRequest req(probe);
    QNetworkReply* reply = nam.get(req);
    QObject::connect(reply, &QNetworkReply::finished,
                     &loop, &QEventLoop::quit);
    QTimer::singleShot(3000, &loop, &QEventLoop::quit);
    loop.exec();

    QString result = serverUrl;
    if (reply->isFinished() && reply->error() == QNetworkReply::NoError)
    {
        QJsonDocument jdoc = QJsonDocument::fromJson(reply->readAll());
        if (jdoc.object()["productname"].toString() == "Nextcloud")
        {
            QUrl u = base;
            u.setPath(probePath + "apps/files/files");
            result = u.toString();
            LOG_TRC("IntegratorFilePicker: detected Nextcloud, "
                    "landing on " << result.toStdString());
        }
    }
    reply->deleteLater();
    return result;
}
}

IntegratorFilePicker::IntegratorFilePicker(const QString& serverUrl,
                                           QWebEngineProfile* profile, QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Open Remote Document");
    resize(800, 900);

    _webView = new QWebEngineView;

    auto* page = new InterceptPage(profile, _webView);
    page->picker = this;
    _webView->setPage(page);

    setCentralWidget(_webView);

    _webView->load(QUrl(resolveLandingUrl(serverUrl)));
}

void IntegratorFilePicker::extractAccessTokenAsync(
    std::function<void(const QString&)> then)
{
    // Try known integrator-specific extraction strategies.  Each
    // returns a non-empty string on success or empty string on
    // failure.  The JS tries them all and returns the first hit.
    _webView->page()->runJavaScript(
        "(() => {"
        "  var token = '';"
        // Nextcloud richdocuments: POST form hidden input
        "  var form = document.querySelector("
        "    'form[target] input[name=access_token]');"
        "  if (form && form.value) return form.value;"
        // Nextcloud richdocuments: JS object
        "  if (window.OCA && window.OCA.RichDocuments"
        "      && window.OCA.RichDocuments.token)"
        "    return window.OCA.RichDocuments.token;"
        // Generic: any hidden input named access_token
        "  var input = document.querySelector("
        "    'input[name=access_token]');"
        "  if (input && input.value) return input.value;"
        // Add more integrator strategies here as needed.
        "  return '';"
        "})()",
        [then = std::move(then)](const QVariant& result) {
            then(result.toString());
        });
}

void IntegratorFilePicker::extractAccessToken()
{
    extractAccessTokenAsync([this](const QString& val) {
        if (!val.isEmpty())
            _accessToken = val;
        emit wopiSelected();
    });
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
