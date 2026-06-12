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

/*
 * Audit logging for server events.
 * Functions: auditLog()
 */

#pragma once

#include <string>
#include <map>
#include <memory>
#include <mutex>

class ChildProcess;

/** This class helps to build list of security warnings for a server instance
 */
class ServerAuditUtil
{
    mutable std::mutex _mutex;

    // <code, status>
    std::map<std::string, std::string> _entries;

    bool _disabled;

public:
    ServerAuditUtil();

    std::string getResultsJSON() const;

    /// A copy of the current code-to-status entries.
    std::map<std::string, std::string> getEntries() const;

    void set(const std::string& code, std::string status);
    void mergeSettings(const std::shared_ptr<ChildProcess> &proc);

    void disable() { _disabled = true; }
    bool isDisabled() const { return _disabled; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
