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

#include <QMainWindow>
#include <QSize>
#include <QUrl>
#include <QWebEnginePage>
#include <QWebEngineView>

namespace
{
/// In pixels.
const QSize ACCOUNT_WINDOW_SIZE(1280, 860);
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
    view->setPage(new QWebEnginePage(pickerProfile, view));
    window->setCentralWidget(view);
    QObject::connect(view, &QWebEngineView::titleChanged, window, &QWidget::setWindowTitle);

    view->load(QUrl(serverUrl));
    window->show();
}

} // namespace coda

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
