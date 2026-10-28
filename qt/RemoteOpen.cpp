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
#include <QUrl>
#include <QWebEnginePage>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineUrlRequestInfo>
#include <QWebEngineUrlRequestInterceptor>
#include <QWebEngineView>

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

/// 2 MiB. The form Nextcloud posts for a document is about 5 KB.
constexpr qsizetype MAX_FORM_BYTES = 2 * 1024 * 1024;

/// The integrator opens a document by posting a form with the token and settings into the iframe
/// that holds the editor. This keeps a copy of that form.
class EditorFormInterceptor : public QWebEngineUrlRequestInterceptor
{
public:
    using QWebEngineUrlRequestInterceptor::QWebEngineUrlRequestInterceptor;

    void interceptRequest(QWebEngineUrlRequestInfo& info) override;

private:
    QByteArray _form; ///< The body of the last form posted into an editor frame.
};

void EditorFormInterceptor::interceptRequest(QWebEngineUrlRequestInfo& info)
{
    const bool isIFrame = info.resourceType() == QWebEngineUrlRequestInfo::ResourceTypeSubFrame;
    const bool isPost = info.requestMethod() == "POST";
    const bool isEditorPage = info.requestUrl().path().endsWith("/cool.html");
    if (!isIFrame || !isPost || !isEditorPage)
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

    _form = form;
    LOG_DBG("account window: kept the " << _form.size()
                                        << "-byte form posted into an editor frame");
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
    auto* page = new QWebEnginePage(pickerProfile, view);
    page->setUrlRequestInterceptor(new EditorFormInterceptor(page));

    QWebEngineScript emptyCoolPage;
    emptyCoolPage.setSourceCode(QString::fromUtf8(EMPTY_COOL_PAGE_SCRIPT));
    emptyCoolPage.setInjectionPoint(QWebEngineScript::DocumentCreation);
    emptyCoolPage.setWorldId(QWebEngineScript::ApplicationWorld);
    emptyCoolPage.setRunsOnSubFrames(true);
    page->scripts().insert(emptyCoolPage);

    view->setPage(page);
    window->setCentralWidget(view);
    QObject::connect(view, &QWebEngineView::titleChanged, window, &QWidget::setWindowTitle);

    view->load(QUrl(serverUrl));
    window->show();
}

} // namespace coda

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
