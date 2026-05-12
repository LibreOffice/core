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

#include <qt/Document.hpp>
#include <qt/IntegratorFilePicker.hpp>
#include <qt/WebView.hpp>
#include <common/Log.hpp>

#include <memory>

namespace coda
{

void openRemoteFile(const QString& serverUrl, QWidget* parent,
                    QWebEngineProfile* profile)
{
    // Show the integrator's web UI.  In embed mode the picker morphs
    // into the document editor in place; in the non-embed flow it
    // emits wopiSelected() once the user picks a document, at which
    // point we extract WOPI params, close the picker, and open a
    // separate WebView for the editor.  The /co/collab fetch and
    // download bytes-on-the-wire are handled by the page-JS once
    // cool.html loads.
    auto* picker = new IntegratorFilePicker(serverUrl, parent);
    picker->setAttribute(Qt::WA_DeleteOnClose);
    QObject::connect(picker, &IntegratorFilePicker::wopiSelected, picker,
        [picker, profile]() {
            const QString wopiSrc = picker->wopiSrc();
            const QString accessToken = picker->accessToken();
            const QString coolServer = picker->coolServer();
            const QString coolPath = picker->coolPath();
            picker->close();

            if (wopiSrc.isEmpty())
            {
                LOG_ERR("openRemoteFile: no WOPISrc from picker");
                return;
            }

            LOG_TRC("openRemoteFile: WOPISrc=" << wopiSrc.toStdString()
                    << " coolServer=" << coolServer.toStdString());

            auto remoteInfo = std::make_shared<RemoteDocInfo>();
            remoteInfo->wopiSrc = wopiSrc;
            remoteInfo->accessToken = accessToken;
            remoteInfo->coolServer = coolServer;
            remoteInfo->coolPath = coolPath;

            WebView* webViewInstance = new WebView(profile);
            webViewInstance->loadRemote(std::move(remoteInfo));
        });
    picker->show();
}

} // namespace coda

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
