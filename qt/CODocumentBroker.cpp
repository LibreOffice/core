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

#include "CODocumentBroker.hpp"

#include "DocumentOperations.hpp"
#include "bridge.hpp"

#include <common/Log.hpp>
#include <common/Message.hpp>
#include <common/Protocol.hpp>
#include <common/StringVector.hpp>
#include <wsd/ClientSession.hpp>

#include <QString>
#include <QUrl>

CODocumentBroker::CODocumentBroker(ChildType type, const std::string& uri,
                                   const Poco::URI& uriPublic, const std::string& docKey,
                                   const std::string& configId, unsigned mobileAppDocId)
    : DocumentBroker(type, uri, uriPublic, docKey, configId, mobileAppDocId)
{
}

bool CODocumentBroker::handleAppMessage(const std::shared_ptr<ClientSession>& session,
                                        const StringVector& tokens, const std::string& firstLine)
{
    if (tokens.equals(0, "PRINT"))
    {
        // The id in the reply picks the dialog that opens on completion.
        // Form fields and notes are left out of the printout, matching the
        // PDF options the browser uses when printing.
        startExport(session,
                    "downloadas name=print.pdf id=print format=pdf options="
                    "{\"ExportFormFields\":{\"type\":\"boolean\",\"value\":\"false\"},"
                    "\"ExportNotes\":{\"type\":\"boolean\",\"value\":\"false\"}}");
        return true;
    }

    if (tokens.equals(0, "downloadas"))
    {
        std::string id;
        COOLProtocol::getTokenString(tokens, "id", id);
        if (id == "print" || id == "export")
        {
            startExport(session, firstLine);
            return true;
        }
    }

    return false;
}

void CODocumentBroker::startExport(const std::shared_ptr<ClientSession>& session,
                                   const std::string& message)
{
    if (_exportInProgress)
    {
        LOG_DBG("export request ignored: an export is already in progress");
        return;
    }

    _exportInProgress = true;
    _exportStart = std::chrono::steady_clock::now();
    coda::invokeOnBridge(getMobileAppDocId(),
                         [](Bridge& bridge) { bridge.showProgressSnackbar(); });

    if (!forwardToChild(session, message))
    {
        _exportInProgress = false;
        coda::invokeOnBridge(getMobileAppDocId(), [](Bridge& bridge) { bridge.onExportFailed(); });
    }
}

bool CODocumentBroker::handleInput(const std::shared_ptr<Message>& message)
{
    if (COOLProtocol::getFirstToken(message->forwardToken(), '-') == "client")
    {
        if (_exportInProgress && message->firstTokenMatches("downloadas:"))
        {
            std::string id;
            COOLProtocol::getTokenString(message->tokens(), "id", id);
            if (id == "print" || id == "export")
            {
                _exportInProgress = false;

                std::string url;
                COOLProtocol::getTokenString(message->tokens(), "url", url);
                const QString path = QUrl(QString::fromStdString(url)).toLocalFile();
                if (path.isEmpty())
                {
                    LOG_ERR("downloadas reply for id=" << id << " carries no usable url");
                    coda::invokeOnBridge(getMobileAppDocId(),
                                         [](Bridge& bridge) { bridge.onExportFailed(); });
                    return true;
                }
                bool delivered;
                if (id == "print")
                    delivered = coda::invokeOnBridge(getMobileAppDocId(), [path](Bridge& bridge)
                                                     { bridge.onPrintExportReady(path); });
                else
                    delivered = coda::invokeOnBridge(getMobileAppDocId(), [path](Bridge& bridge)
                                                     { bridge.onSaveExportReady(path); });
                if (!delivered)
                {
                    // With no bridge registered under the id (the window has
                    // moved on to another document), nothing will show a
                    // dialog or clean up after this file: remove it here.
                    LOG_WRN("no bridge for the downloadas completion of id="
                            << id << "; removing the exported file");
                    removeExportTempDirectory(path.toStdString());
                }
                return true;
            }
        }
        else if (_exportInProgress && message->firstTokenMatches("error:") &&
                 message->tokens().equals(1, "cmd=downloadas"))
        {
            // Only the failure of this broker's own export clears the flag
            // and shows the export-failed dialog. An error for any other
            // download id continues to the client untouched.
            std::string id;
            COOLProtocol::getTokenString(message->tokens(), "id", id);
            if (id == "print" || id == "export")
            {
                _exportInProgress = false;
                coda::invokeOnBridge(getMobileAppDocId(),
                                     [](Bridge& bridge) { bridge.onExportFailed(); });
                return true;
            }
        }
    }

    return DocumentBroker::handleInput(message);
}

void CODocumentBroker::dumpState(std::ostream& os)
{
    DocumentBroker::dumpState(os);

    os << "  export in progress: " << _exportInProgress;
    if (_exportInProgress)
        os << " (started "
           << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() -
                                                               _exportStart)
           << " ago)";
    os << '\n';
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
