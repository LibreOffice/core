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
/// load, and finds the editor's iframe the form went to.
class CloudAccountPage : public QWebEnginePage
{
public:
    CloudAccountPage(QWebEngineProfile* profile, QObject* parent);

    void keepForm(const QUrl& url, const QByteArray& form);

private:
    void findEditorFrame();

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

    LOG_DBG("account window: found the editor frame for the kept form");
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
