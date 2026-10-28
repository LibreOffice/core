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

#include <qt/RemoteOpen.hpp>
#include <common/Log.hpp>

#include <QByteArray>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMainWindow>
#include <QSize>
#include <QTimer>
#include <QUrl>
#include <QUuid>
#include <QWebEngineFrame>
#include <QWebEnginePage>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineUrlRequestInfo>
#include <QWebEngineUrlRequestInterceptor>
#include <QWebEngineView>

#include <chrono>
#include <optional>

namespace
{
/// In pixels.
const QSize ACCOUNT_WINDOW_SIZE(1280, 860);

/// In the editor's iframe, replaces coolwsd's page with an empty document before it is parsed.
const char* EMPTY_COOL_PAGE_SCRIPT = R"JS(
if (window !== window.top && location.pathname.endsWith('/cool.html')) {
    document.open();
    document.close();
}
)JS";

/// Builds CODA's editor page in the emptied editor iframe. It asks coolwsd for its settings by
/// posting the integrator's form again, then loads CODA's page, scripts and styles from coollocal:.
const char* PUT_CODA_PAGE_IN_SCRIPT = R"JS(
(config) => {
    if (window.codaRemotePage)
        return;

    window.codaRemotePage = { base: config.base };

    const text = (response) => {
        if (!response.ok)
            throw new Error('a request returned ' + response.status);
        return response.text();
    };

    const serverPage = fetch(config.formUrl, {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: config.formBody,
    }).then(text);

    const appPage = fetch(config.base + 'cool-qt.html').then(text);

    Promise.all([serverPage, appPage]).then(([serverHtml, appHtml]) => {
        // coolwsd writes the values for its editor into these two elements.
        const server = new DOMParser().parseFromString(serverHtml, 'text/html');
        const variables = server.getElementById('initial-variables');
        const cssVariables = server.getElementById('init-css-vars');

        window.codaRemotePage.serverVariables = variables ? { ...variables.dataset } : {};
        window.codaRemotePage.cssVariables = cssVariables ? cssVariables.value : '';

        const app = new DOMParser().parseFromString(appHtml, 'text/html');
        const isRelative = (address) => address && !address.includes(':');

        for (const link of app.querySelectorAll('link[href]'))
            if (isRelative(link.getAttribute('href')))
                link.href = config.base + link.getAttribute('href');

        // Remove the scripts from the local cool-qt.html: moved scripts never run. We add them
        // again later as 'coollocal://bundled/<path>' so that they are loaded from CODA.
        const scripts = Array.from(app.scripts);
        scripts.forEach((script) => script.remove());
        const sources = scripts.map((script) => script.getAttribute('src')).filter(isRelative);

        // Replace the frame's empty <html>, left by EMPTY_COOL_PAGE_SCRIPT, with CODA's <html>.
        document.documentElement.replaceWith(document.adoptNode(app.documentElement));

        // The load event fired when the page was emptied, and CODA's start waits for it.
        const loaded = () => window.dispatchEvent(new Event('load'));

        // Add CODA's scripts again, from config.base (coollocal://bundled/). With async off,
        // they load in parallel and run in order.
        sources.forEach((source, index) => {
            const script = document.createElement('script');
            script.src = config.base + source;
            script.async = false;
            if (index === sources.length - 1)
                script.addEventListener('load', loaded);

            document.head.append(script);
        });
    }).catch((error) => console.error('CODA could not put its editor page in: ' + error.message));
}
)JS";

/// Where the editor frame loads CODA's files from.
const QString COOLLOCAL_BUNDLED_URL = QStringLiteral("coollocal://bundled/");

/// Starts the fragment that names an editor frame, as in "#coda=<id>".
const QString FRAME_ID_PREFIX = QStringLiteral("coda=");

/// How often to look for the emptied editor frame after a form was kept.
constexpr std::chrono::milliseconds EDITOR_CHECK_INTERVAL(50);

/// 200 checks are 10 seconds.
constexpr int EDITOR_CHECKS = 200;

/// 2 MiB. The form Nextcloud posts for a document is about 5 KB.
constexpr qsizetype MAX_FORM_BYTES = 2 * 1024 * 1024;

class CloudAccountPage;

/// The integrator opens a document by posting a form with the token and settings into the iframe
/// that holds the editor. This keeps a copy of that form. It redirects the request to the same
/// address with a new id as its fragment. coolwsd never sees the fragment, and the frame keeps it.
class EditorFormInterceptor : public QWebEngineUrlRequestInterceptor
{
public:
    explicit EditorFormInterceptor(CloudAccountPage& page);

    void interceptRequest(QWebEngineUrlRequestInfo& info) override;

private:
    CloudAccountPage& _page;
};

/// A page in a cloud account window. It keeps the integrator's form posted for the document at
/// load, and finds the editor's iframe the form went to. It injects a script there that builds
/// CODA's editor page.
class CloudAccountPage : public QWebEnginePage
{
public:
    CloudAccountPage(QWebEngineProfile* profile, QObject* parent);

    void keepForm(const QUrl& url, const QByteArray& form);

private:
    void findEditorFrame();
    void putCodaPageIn(QWebEngineFrame& frame);

    QUrl _formUrl; ///< Where the kept form went, with the frame's id as the fragment.
    QByteArray _form;
    QTimer _editorCheck;
    int _editorChecksLeft = 0;
};

/// The frame whose address is url, searched from frame down.
std::optional<QWebEngineFrame> findFrame(const QWebEngineFrame& frame, const QUrl& url)
{
    if (frame.url() == url)
        return frame;

    for (const QWebEngineFrame& child : frame.children())
    {
        std::optional<QWebEngineFrame> found = findFrame(child, url);
        if (found)
            return found;
    }

    return std::nullopt;
}

EditorFormInterceptor::EditorFormInterceptor(CloudAccountPage& page)
    : QWebEngineUrlRequestInterceptor(&page)
    , _page(page)
{
}

void EditorFormInterceptor::interceptRequest(QWebEngineUrlRequestInfo& info)
{
    const bool isIFrame = info.resourceType() == QWebEngineUrlRequestInfo::ResourceTypeSubFrame;
    const bool isPost = info.requestMethod() == "POST";
    const bool isEditorPage = info.requestUrl().path().endsWith("/cool.html");
    if (!isIFrame || !isPost || !isEditorPage)
        return;

    // The redirected request comes here again, with the id and without the body.
    const bool hasFrameId = info.requestUrl().fragment().startsWith(FRAME_ID_PREFIX);
    if (hasFrameId)
        return;

    QIODevice* body = info.requestBody();
    if (!body)
    {
        LOG_WRN("account window: refused: the form posted into an editor frame has no body");
        return;
    }

    body->open(QIODevice::ReadOnly);
    // Qt WebEngine returns the same first bytes on every read in pieces,
    // so readAll() never ends (it's a bug). One read takes the whole body.
    const QByteArray form = body->read(MAX_FORM_BYTES + 1);
    if (form.size() > MAX_FORM_BYTES)
    {
        LOG_WRN("account window: refused: the form posted into an editor frame is at least "
                << form.size() << " bytes, over the limit of " << MAX_FORM_BYTES << " bytes");
        return;
    }

    QUrl frameUrl = info.requestUrl();
    frameUrl.setFragment(FRAME_ID_PREFIX + QUuid::createUuid().toString(QUuid::Id128));

    LOG_DBG("account window: kept the " << form.size()
                                        << "-byte form posted into an editor frame");
    _page.keepForm(frameUrl, form);
    info.redirect(frameUrl);
}

CloudAccountPage::CloudAccountPage(QWebEngineProfile* profile, QObject* parent)
    : QWebEnginePage(profile, parent)
{
    setUrlRequestInterceptor(new EditorFormInterceptor(*this));

    QWebEngineScript emptyCoolPage;
    emptyCoolPage.setSourceCode(QString::fromUtf8(EMPTY_COOL_PAGE_SCRIPT));
    emptyCoolPage.setInjectionPoint(QWebEngineScript::DocumentCreation);
    emptyCoolPage.setWorldId(QWebEngineScript::ApplicationWorld);
    emptyCoolPage.setRunsOnSubFrames(true);
    scripts().insert(emptyCoolPage);

    _editorCheck.setInterval(EDITOR_CHECK_INTERVAL);
    connect(&_editorCheck, &QTimer::timeout, this, &CloudAccountPage::findEditorFrame);
}

void CloudAccountPage::keepForm(const QUrl& url, const QByteArray& form)
{
    _formUrl = url;
    _form = form;
    _editorChecksLeft = EDITOR_CHECKS;
    _editorCheck.start();
}

void CloudAccountPage::findEditorFrame()
{
    std::optional<QWebEngineFrame> frame = findFrame(mainFrame(), _formUrl);
    _editorChecksLeft--;
    const bool keepLooking = !frame && _editorChecksLeft > 0;
    if (keepLooking)
        return;

    _editorCheck.stop();
    if (!frame)
    {
        LOG_WRN("account window: refused: no editor frame opened for the kept form");
        return;
    }

    putCodaPageIn(*frame);
}

void CloudAccountPage::putCodaPageIn(QWebEngineFrame& frame)
{
    QJsonObject config;
    config["formUrl"] = _formUrl.toString(QUrl::FullyEncoded);
    config["formBody"] = QString::fromLatin1(_form);
    config["base"] = COOLLOCAL_BUNDLED_URL;
    const QByteArray configJson = QJsonDocument(config).toJson(QJsonDocument::Compact);
    const QString putPageIn = QString::fromUtf8(PUT_CODA_PAGE_IN_SCRIPT);
    const QString script = '(' + putPageIn + ")(" + QString::fromUtf8(configJson) + ')';

    frame.runJavaScript(script, QWebEngineScript::MainWorld);
    LOG_DBG("account window: putting CODA's editor page into the editor frame");
}
}

namespace coda
{

void openRemoteFile(const QString& serverUrl, QWidget* parent,
                    QWebEngineProfile* /*profile*/, QWebEngineProfile* pickerProfile)
{
    auto* window = new QMainWindow(parent);
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->resize(ACCOUNT_WINDOW_SIZE);

    auto* view = new QWebEngineView(window);
    view->setPage(new CloudAccountPage(pickerProfile, view));
    window->setCentralWidget(view);
    QObject::connect(view, &QWebEngineView::titleChanged, window, &QWidget::setWindowTitle);

    view->load(QUrl(serverUrl));
    window->show();
}

} // namespace coda

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
