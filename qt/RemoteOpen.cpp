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
#include <common/EditorFrameMessages.hpp>
#include <common/Log.hpp>
#include <net/FakeSocket.hpp>
#include <qt/CoolLocalSchemeHandler.hpp>
#include <qt/bridge.hpp>

#include <QBuffer>
#include <QByteArray>
#include <QDesktopServices>
#include <QFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPointer>
#include <QSize>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QUuid>
#include <QWebEngineFrame>
#include <QWebEngineNewWindowRequest>
#include <QWebEnginePage>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineUrlRequestInfo>
#include <QWebEngineUrlRequestInterceptor>
#include <QWebEngineUrlRequestJob>
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

/// 1 MiB, the most a CheckFileInfo response can be.
constexpr qint64 MAX_FILE_INFO_BYTES = 1024 * 1024;

/// The CheckFileInfo fields coolwsd passes on to the editor in its wopi: message.
const char* const EDITOR_FILE_INFO_FIELDS[] = {
    "PostMessageOrigin", "BaseFileName", "BreadcrumbDocName", "TemplateSaveAs", "TemplateSource",
    "HidePrintOption", "HideSaveOption", "HideExportOption", "HideRepairOption", "DisablePrint",
    "DisableExport", "DisableCopy", "DisableInactiveMessages", "DownloadAsPostMessage",
    "UserCanNotWriteRelative", "EnableInsertRemoteImage", "EnableInsertRemoteFile",
    "DisableInsertLocalImage", "EnableRemoteLinkPicker", "EnableRemoteAIContent",
    "DisableAISettings", "IsAnonymousUser", "EnableShare", "HideUserList", "SupportsRename",
    "UserCanRename", "UserCanChangeSecurityLabel", "FileUrl", "UserCanWrite",
    "HideChangeTrackingControls", "IsOwner", "PresentationLeader", "DisablePresentation",
    "CommentAvatarUrl", "HostName",
};

/// The longest string V8 makes, in UTF-16 units: 2^29 - 24, measured in Qt WebEngine 6.11.
constexpr qsizetype MAX_JS_STRING_UNITS = 536'870'888;

/// A message from the frame is one JS string, and a UTF-16 unit is at most 3 bytes of UTF-8.
constexpr qsizetype MAX_MESSAGE_BYTES = 3 * MAX_JS_STRING_UNITS;

void answer(QWebEngineUrlRequestJob* job, const QByteArray& text)
{
    auto* buffer = new QBuffer(job);
    buffer->setData(text);
    job->reply("text/plain", buffer);
}

/// The page takes an empty answer as a file that is not there.
QByteArray readBundledFile(const QString& path)
{
    const QString filePath = coda::bundledFilePath(path);
    if (filePath.isEmpty())
        return QByteArray();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        LOG_WRN("account window: \"" << path.toStdString() << "\" not found in the coda bundle");
        return QByteArray();
    }

    return file.readAll();
}

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
    CloudAccountPage(const QUrl& accountUrl, QWebEngineProfile* profile, QObject* parent);
    ~CloudAccountPage() override;

    const QUrl& integratorOrigin() const { return _integratorOrigin; }
    void keepForm(const QUrl& url, const QByteArray& form);
    std::optional<QByteArray> receiveMessage(const QUrl& origin, const QString& kind,
                                             const QByteArray& message);
    void checkFileInfo(QWebEngineUrlRequestJob* job, const QByteArray& argumentsJson);

protected:
    bool acceptNavigationRequest(const QUrl& url, NavigationType type, bool isMainFrame) override;

private:
    void openNewWindow(QWebEngineNewWindowRequest& request);
    void insertEmptyCoolPageScript();
    void findEditorFrame();
    void putCodaPageIn(QWebEngineFrame& frame);
    std::optional<QByteArray> callBridge(const QString& method, const QString& argumentsJson);
    void sendFileInfo(QNetworkReply* reply, const QPointer<QWebEngineUrlRequestJob>& job);

    QUrl _integratorOrigin; ///< The origin of the account address from the backstage view.
    QUrl _formUrl; ///< Where the kept form went, with the frame's id as the fragment.
    QByteArray _form;
    QTimer _editorCheck;
    int _editorChecksLeft = 0;
    QString _session; ///< Names the editor frame's session in its messages.
    QUrl _frameOrigin; ///< The origin of the editor frame given the session.
    QByteArray _parts; ///< The parts of a long message from the frame, so far.
    coda::DocumentData _document; ///< The document the editor frame shows.
    std::unique_ptr<Bridge> _bridge; ///< Takes the frame's messages. It refers to _document.
    QNetworkAccessManager _network;
};

/// The pages with an editor frame, by the session of the frame.
std::map<QString, CloudAccountPage*> PagesBySession;

/// The scheme, host and port of url.
QUrl originOf(const QUrl& url)
{
    return url.adjusted(QUrl::RemoveUserInfo | QUrl::RemovePath | QUrl::RemoveQuery
                        | QUrl::RemoveFragment);
}

/// Other schemes would open other apps, so only web addresses go to the system browser.
void openInSystemBrowser(const QUrl& url)
{
    const bool isWebAddress = url.scheme() == "https" || url.scheme() == "http";
    if (!isWebAddress)
    {
        LOG_WRN("account window: refused: the link is not a web address: "
                << url.toString(QUrl::RemoveUserInfo).toStdString());
        return;
    }

    QDesktopServices::openUrl(url);
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

    const bool isInIntegratorWindow = originOf(info.firstPartyUrl()) == _page.integratorOrigin();
    const bool isPostedByIntegrator = originOf(info.initiator()) == _page.integratorOrigin();
    if (!isInIntegratorWindow || !isPostedByIntegrator)
    {
        LOG_WRN("account window: refused: a form for an editor frame came from another page "
                "than the integrator's");
        return;
    }

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

CloudAccountPage::CloudAccountPage(const QUrl& accountUrl, QWebEngineProfile* profile,
                                   QObject* parent)
    : QWebEnginePage(profile, parent)
    , _integratorOrigin(originOf(accountUrl))
{
    setUrlRequestInterceptor(new EditorFormInterceptor(*this));
    insertEmptyCoolPageScript();

    _editorCheck.setInterval(EDITOR_CHECK_INTERVAL);
    connect(&_editorCheck, &QTimer::timeout, this, &CloudAccountPage::findEditorFrame);
    connect(this, &QWebEnginePage::newWindowRequested, this, &CloudAccountPage::openNewWindow);
}

CloudAccountPage::~CloudAccountPage()
{
    PagesBySession.erase(_session);
}

/// TODO: A sign-in through another site, such as an identity provider, is refused too.
bool CloudAccountPage::acceptNavigationRequest(const QUrl& url, NavigationType type,
                                               bool isMainFrame)
{
    const bool isOtherSite = originOf(url) != _integratorOrigin;
    if (!isMainFrame || !isOtherSite)
        return true;

    if (type != NavigationTypeLinkClicked)
    {
        LOG_WRN("account window: refused: the window would leave the integrator's site for "
                << url.toString(QUrl::RemoveUserInfo).toStdString());
        return false;
    }

    LOG_INF("account window: opening a link in the system browser");
    openInSystemBrowser(url);
    // Qt WebEngine stops injecting the page's scripts after it refuses a link click.
    QTimer::singleShot(0, this, &CloudAccountPage::insertEmptyCoolPageScript);
    return false;
}

void CloudAccountPage::insertEmptyCoolPageScript()
{
    QWebEngineScript emptyCoolPage;
    emptyCoolPage.setSourceCode(QString::fromUtf8(EMPTY_COOL_PAGE_SCRIPT));
    emptyCoolPage.setInjectionPoint(QWebEngineScript::DocumentCreation);
    emptyCoolPage.setWorldId(QWebEngineScript::ApplicationWorld);
    emptyCoolPage.setRunsOnSubFrames(true);

    // It is the only script of the page, so clear() takes away an earlier copy.
    scripts().clear();
    scripts().insert(emptyCoolPage);
}

/// The request is never opened in a page, so the app shows no new window.
void CloudAccountPage::openNewWindow(QWebEngineNewWindowRequest& request)
{
    LOG_INF("account window: opening in the system browser");
    openInSystemBrowser(request.requestedUrl());
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
        if (!EditorFrameMessages::isAllowed(whole.toStdString()))
            return QByteArray();

        const QVariant reply = _bridge->cool(whole);
        return reply.isValid() ? reply.toString().toUtf8() : QByteArray();
    }
    else if (kind == QStringLiteral("error"))
    {
        _bridge->error(whole);
        return QByteArray();
    }
    else if (kind == QStringLiteral("debug"))
    {
        _bridge->debug(whole);
        return QByteArray();
    }
    else
        return callBridge(kind, whole);
}

/// The page calls these Bridge methods by name, as the web channel gives them in CODA's own
/// window. The kind of the message names the method, and the message is its arguments in JSON.
std::optional<QByteArray> CloudAccountPage::callBridge(const QString& method,
                                                       const QString& argumentsJson)
{
    const QJsonArray arguments = QJsonDocument::fromJson(argumentsJson.toUtf8()).array();
    const QString first = arguments.at(0).toString();
    const QString second = arguments.at(1).toString();

    if (method == QStringLiteral("writeRemoteDocFile"))
        return _bridge->writeRemoteDocFile(first, second).toUtf8();
    else if (method == QStringLiteral("readLocalDocBytes"))
        return _bridge->readLocalDocBytes().toUtf8();
    else if (method == QStringLiteral("getAllPrefs"))
        return _bridge->getAllPrefs().toUtf8();
    else if (method == QStringLiteral("getBundledFile"))
        return readBundledFile(first);
    else if (method == QStringLiteral("setPref"))
    {
        _bridge->setPref(first, second);
        return QByteArray();
    }
    else if (method == QStringLiteral("saveCompleted"))
    {
        _bridge->saveCompleted();
        return QByteArray();
    }

    LOG_WRN("account window: refused: a message from the editor frame has an unknown kind");
    return std::nullopt;
}

void CloudAccountPage::checkFileInfo(QWebEngineUrlRequestJob* job, const QByteArray& argumentsJson)
{
    if (job->initiator() != _frameOrigin)
    {
        LOG_WRN("account window: refused: a CheckFileInfo request came from another origin");
        answer(job, QByteArray());
        return;
    }

    // The page sends only the token. The address is the session's own WOPISrc.
    const QString token = QJsonDocument::fromJson(argumentsJson).array().at(0).toString();
    QUrl url(_document._remoteInfo->wopiSrc);
    const QString encodedToken = QString::fromLatin1(QUrl::toPercentEncoding(token));
    url.setQuery("access_token=" + encodedToken, QUrl::StrictMode);

    // TODO: Once this approach is accepted, move this request to common/ and send it through the
    // HttpTransport of https://gerrit.collaboraoffice.com/c/online/+/15051, the proper way.
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::ManualRedirectPolicy);
    request.setAttribute(QNetworkRequest::CookieLoadControlAttribute, QNetworkRequest::Manual);
    request.setAttribute(QNetworkRequest::CookieSaveControlAttribute, QNetworkRequest::Manual);
    request.setTransferTimeout();

    QNetworkReply* reply = _network.get(request);
    connect(reply, &QNetworkReply::downloadProgress, reply,
            [reply](qint64 received, qint64)
            {
                if (received > MAX_FILE_INFO_BYTES)
                    reply->abort();
            });

    const QPointer<QWebEngineUrlRequestJob> pending(job);
    connect(reply, &QNetworkReply::finished, this,
            [this, reply, pending] { sendFileInfo(reply, pending); });
}

void CloudAccountPage::sendFileInfo(QNetworkReply* reply,
                                    const QPointer<QWebEngineUrlRequestJob>& job)
{
    reply->deleteLater();
    if (!job)
    {
        LOG_DBG("account window: the editor frame stopped waiting for its CheckFileInfo");
        return;
    }

    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const bool isOk = reply->error() == QNetworkReply::NoError && status == 200;
    if (!isOk)
    {
        LOG_WRN("account window: CheckFileInfo failed with HTTP status " << status);
        answer(job, QByteArray());
        return;
    }

    const QJsonObject fileInfo = QJsonDocument::fromJson(reply->readAll()).object();
    QJsonObject editorFileInfo;
    for (const char* field : EDITOR_FILE_INFO_FIELDS)
    {
        if (fileInfo.contains(field))
            editorFileInfo[field] = fileInfo[field];
    }

    answer(job, QJsonDocument(editorFileInfo).toJson(QJsonDocument::Compact));
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

void receiveEditorFrameMessage(QWebEngineUrlRequestJob* job, const QString& session,
                               const QString& kind, const QByteArray& message)
{
    const auto page = PagesBySession.find(session);
    if (page == PagesBySession.end())
    {
        LOG_WRN("account window: refused: a message names no open editor frame session");
        answer(job, QByteArray());
        return;
    }

    // CheckFileInfo replies once the integrator has answered, so it takes the job.
    if (kind == QStringLiteral("checkFileInfo"))
    {
        page->second->checkFileInfo(job, message);
        return;
    }

    const std::optional<QByteArray> reply =
        page->second->receiveMessage(job->initiator(), kind, message);
    answer(job, reply.value_or(QByteArray()));
}

void openRemoteFile(const QString& serverUrl, QWidget* parent,
                    QWebEngineProfile* /*profile*/, QWebEngineProfile* pickerProfile)
{
    auto* window = new QMainWindow(parent);
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->resize(ACCOUNT_WINDOW_SIZE);

    auto* view = new QWebEngineView(window);
    view->setPage(new CloudAccountPage(QUrl(serverUrl), pickerProfile, view));
    window->setCentralWidget(view);
    QObject::connect(view, &QWebEngineView::titleChanged, window, &QWidget::setWindowTitle);

    view->load(QUrl(serverUrl));
    window->show();
}

} // namespace coda

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
