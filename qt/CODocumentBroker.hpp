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

#include <chrono>
#include <memory>
#include <string>

#include <wsd/DocumentBroker.hpp>

/// The document broker of the Qt app ("CO" is Collabora Office). It handles
/// the document-scoped app messages natively on its own poll thread: print
/// and export-as become a downloadas to the child, so the blocking export
/// runs on the kit poll thread while the GUI thread only shows dialogs.
class CODocumentBroker final : public DocumentBroker
{
public:
    CODocumentBroker(ChildType type, const std::string& uri, const Poco::URI& uriPublic,
                     const std::string& docKey, const std::string& configId,
                     unsigned mobileAppDocId);

    bool handleAppMessage(const std::shared_ptr<ClientSession>& session,
                          const StringVector& tokens, const std::string& firstLine) override;

    bool handleInput(const std::shared_ptr<Message>& message) override;

    void dumpState(std::ostream& os) override;

private:
    void startExport(const std::shared_ptr<ClientSession>& session, const std::string& message);

    /// True from sending a downloadas to the child until its completion or
    /// error arrives. Touched only on the broker poll thread.
    bool _exportInProgress = false;

    /// When the in-flight export was started.
    std::chrono::steady_clock::time_point _exportStart;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
