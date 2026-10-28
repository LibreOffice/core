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
#include <net/FakeSocket.hpp>
#include <qt/bridge.hpp>

#include <QByteArray>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMainWindow>
#include <QSize>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QUuid>
#include <QWebEngineFrame>
#include <QWebEnginePage>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineUrlRequestInfo>
#include <QWebEngineUrlRequestInterceptor>
#include <QWebEngineView>

#include <chrono>
#include <map>
#include <memory>
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

    window.codaRemotePage = { base: config.base, messageUrl: config.messageUrl };

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

/// Where the editor frame posts its messages to native code.
const QString COOLLOCAL_MESSAGE_URL = QStringLiteral("coollocal://message/");

/// Starts the fragment that names an editor frame, as in "#coda=<id>".
const QString FRAME_ID_PREFIX = QStringLiteral("coda=");

/// How often to look for the emptied editor frame after a form was kept.
constexpr std::chrono::milliseconds EDITOR_CHECK_INTERVAL(50);

/// 200 checks are 10 seconds.
constexpr int EDITOR_CHECKS = 200;

/// 2 MiB. The form Nextcloud posts for a document is about 5 KB.
constexpr qsizetype MAX_FORM_BYTES = 2 * 1024 * 1024;

/// The longest string V8 makes, in UTF-16 units: 2^29 - 24, measured in Qt WebEngine 6.11.
constexpr qsizetype MAX_JS_STRING_UNITS = 536'870'888;

/// A message from the frame is one JS string, and a UTF-16 unit is at most 3 bytes of UTF-8.
constexpr qsizetype MAX_MESSAGE_BYTES = 3 * MAX_JS_STRING_UNITS;

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
    ~CloudAccountPage() override;

    void keepForm(const QUrl& url, const QByteArray& form);
    std::optional<QByteArray> receiveMessage(const QUrl& origin, const QString& kind,
                                             const QByteArray& message);

private:
    void findEditorFrame();
    void putCodaPageIn(QWebEngineFrame& frame);

    QUrl _formUrl; ///< Where the kept form went, with the frame's id as the fragment.
    QByteArray _form;
    QTimer _editorCheck;
    int _editorChecksLeft = 0;
    QString _session; ///< Names the editor frame's session in its messages.
    QUrl _frameOrigin; ///< The origin of the editor frame given the session.
    QByteArray _parts; ///< The parts of a long message from the frame, so far.
    coda::DocumentData _document; ///< The document the editor frame shows.
    std::unique_ptr<Bridge> _bridge; ///< Takes the frame's messages. It refers to _document.
};

/// The pages with an editor frame, by the session of the frame.
std::map<QString, CloudAccountPage*> PagesBySession;

/// The scheme, host and port of url.
QUrl originOf(const QUrl& url)
{
    return url.adjusted(QUrl::RemoveUserInfo | QUrl::RemovePath | QUrl::RemoveQuery
                        | QUrl::RemoveFragment);
}

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

CloudAccountPage::~CloudAccountPage()
{
    PagesBySession.erase(_session);
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

std::optional<QByteArray> CloudAccountPage::receiveMessage(const QUrl& origin, const QString& kind,
                                                           const QByteArray& message)
{
    if (origin != _frameOrigin)
    {
        LOG_WRN("account window: refused: a message for the editor frame's session came from "
                "another origin");
        return std::nullopt;
    }

    const bool isOverLimit = _parts.size() + message.size() > MAX_MESSAGE_BYTES;
    if (isOverLimit)
    {
        LOG_WRN("account window: refused: a message from the editor frame is over "
                << MAX_MESSAGE_BYTES << " bytes, more than one JS string holds");
        _parts.clear();
        return std::nullopt;
    }

    // A long message comes in parts, each but the last of the kind "part".
    if (kind == QStringLiteral("part"))
    {
        _parts += message;
        return QByteArray();
    }

    const QString whole = QString::fromUtf8(_parts + message);
    _parts.clear();

    if (kind == QStringLiteral("cool"))
    {
        const QVariant reply = _bridge->cool(whole);
        return reply.isValid() ? reply.toString().toUtf8() : QByteArray();
    }

    if (kind == QStringLiteral("error"))
    {
        _bridge->error(whole);
        return QByteArray();
    }

    if (kind == QStringLiteral("debug"))
    {
        _bridge->debug(whole);
        return QByteArray();
    }

    LOG_WRN("account window: refused: a message from the editor frame has an unknown kind");
    return std::nullopt;
}

void CloudAccountPage::putCodaPageIn(QWebEngineFrame& frame)
{
    PagesBySession.erase(_session);
    _parts.clear();
    _session = QUuid::createUuid().toString(QUuid::Id128);
    _frameOrigin = originOf(frame.url());
    PagesBySession[_session] = this;

    // The page keeps the access token, because it opens /co/collab itself.
    auto remoteInfo = std::make_shared<coda::RemoteDocInfo>();
    remoteInfo->wopiSrc = QUrlQuery(_formUrl).queryItemValue("WOPISrc", QUrl::FullyDecoded);
    remoteInfo->coolServer = _frameOrigin.toString();
    remoteInfo->coolPath = _formUrl.path();

    _bridge.reset();
    _document = {
        ._fileURL = Poco::URI(), ///< Set on REMOTEDOC_WRITE, once the page has the document.
        ._fakeClientFd = fakeSocketSocket(),
        ._appDocId = coda::generateNewAppDocId(),
        ._remoteInfo = std::move(remoteInfo),
    };
    QWebEngineView* view = QWebEngineView::forPage(this);
    _bridge = std::make_unique<Bridge>(nullptr, nullptr, _document, view);
    _bridge->setFrame(frame);
    LOG_INF("account window: the editor frame's session has appDocId " << _document._appDocId);

    QJsonObject config;
    config["formUrl"] = _formUrl.toString(QUrl::FullyEncoded);
    config["formBody"] = QString::fromLatin1(_form);
    config["base"] = COOLLOCAL_BUNDLED_URL;
    config["messageUrl"] = COOLLOCAL_MESSAGE_URL + "?session=" + _session;
    const QByteArray configJson = QJsonDocument(config).toJson(QJsonDocument::Compact);
    const QString putPageIn = QString::fromUtf8(PUT_CODA_PAGE_IN_SCRIPT);
    const QString script = '(' + putPageIn + ")(" + QString::fromUtf8(configJson) + ')';

    frame.runJavaScript(script, QWebEngineScript::MainWorld);
    LOG_DBG("account window: putting CODA's editor page into the editor frame");
}
}

namespace coda
{

std::optional<QByteArray> receiveEditorFrameMessage(const QString& session, const QUrl& origin,
                                                    const QString& kind, const QByteArray& message)
{
    const auto page = PagesBySession.find(session);
    if (page == PagesBySession.end())
    {
        LOG_WRN("account window: refused: a message names no open editor frame session");
        return std::nullopt;
    }

    return page->second->receiveMessage(origin, kind, message);
}

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
