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
 * Installation and management of configuration presets.
 * Functions: applyPresetConfig()
 */

#pragma once

#include <net/Socket.hpp>

#include <Poco/Timestamp.h>

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>

struct CacheQuery;
class ClientSession;

class PresetsInstallTask : public std::enable_shared_from_this<PresetsInstallTask>
{
private:
    std::set<std::string> _installingPresets;
    std::string _configId;
    std::string _presetsPath;
    // Per-group destination override; a group listed here goes under <override>/<group>/ instead of
    // the default <_presetsPath>/<group>/:
    std::map<std::string, std::string> _groupOverridePath;
    // Extension ids the integrator advertised for this round; anything present on disk under
    // <extensions>/ but not in this set after the install completes is a leftover from a prior
    // round (the integrator dropped that entry) and gets swept on completion:
    std::set<std::string> _expectedExtensions;
    // Time when this task started; the stale-extension sweep skips dirs whose last-modified time is
    // newer than this so a concurrent install task on the same configId (e.g. another
    // DocumentBroker for a sibling doc) cannot have its just-unpacked content wiped by ours:
    Poco::Timestamp _installStartTime;
    std::vector<std::function<void(bool)>> _installFinishedCBs;
    std::weak_ptr<SocketPoll> _poll;
    int _idCount;
    bool _reportedStatus;
    bool _overallSuccess;

    void asyncInstall(const std::string& uri, const std::string& stamp,
                      const std::string& fileName,
                      const std::shared_ptr<ClientSession>& session);

    void installPresetStarted(const std::string& id);

    void installPresetFinished(const std::string& id, bool presetResult);

    void completed();

    // Remove on-disk extension directories that the integrator no longer advertises in the current
    // settings round:
    void sweepStaleExtensions();

    void addGroup(const Poco::JSON::Object::Ptr& settings, const std::string& groupName,
                  std::vector<CacheQuery>& queries);

public:
    PresetsInstallTask(const std::shared_ptr<SocketPoll>& poll, const std::string& configId,
                       const std::string& presetsPath,
                       std::map<std::string, std::string> groupOverridePath,
                       const std::function<void(bool)>& installFinishedCB);

    bool empty() const
    {
        return _installingPresets.empty();
    }

    void appendCallback(const std::function<void(bool)>& installFinishedCB)
    {
        _installFinishedCBs.emplace_back(installFinishedCB);
    }

    void install(const Poco::JSON::Object::Ptr& settings,
                 const std::shared_ptr<ClientSession>& session);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
