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

#include <atomic>
#include <string>

class GenericLogger;

namespace Log
{
    /// Helper to avoid destruction ordering issues.
    struct StaticHelper
    {
    private:
        GenericLogger* _logger;
        static thread_local GenericLogger* _threadLocalLogger;
        std::string _name;
        std::string _logLevel;
        std::string _id;
        std::atomic<bool> _inited;
    public:
        StaticHelper() :
            _logger(nullptr),
            _inited(true)
        {
        }
        ~StaticHelper()
        {
            _inited = false;
        }

        bool getInited() const { return _inited; }

        void setId(const std::string& id) { _id = id; }

        const std::string& getId() const { return _id; }

        void setName(const std::string& name) { _name = name; }

        const std::string& getName() const { return _name; }

        void setLevel(const std::string& logLevel) { _logLevel = logLevel; }

        const std::string& getLevel() const { return _logLevel; }

        void setLogger(GenericLogger* logger) { _logger = logger; };

        static void setThreadLocalLogger(GenericLogger* logger)
        {
            // FIXME: What to do with the previous thread-local logger, if any? Will deleting it
            // destroy also its channel? That won't be good as we use the same channel for all
            // loggers. Best to just leak it?
            _threadLocalLogger = logger;
        }

        GenericLogger* getLogger() const { return _logger; }

        static GenericLogger* getThreadLocalLogger() { return _threadLocalLogger; }
    };

    struct StaticUIHelper : StaticHelper
    {
    private:
        bool _mergeCmd = false;
        bool _logTimeEndOfMergedCmd = false;
    public:
        void setLogMergeInfo(bool mergeCmd, bool logTimeEndOfMergedCmd)
        {
            _mergeCmd = mergeCmd;
            _logTimeEndOfMergedCmd = logTimeEndOfMergedCmd;
        }
        bool getMergeCmd() const { return _mergeCmd; }
        bool getLogTimeEndOfMergedCmd() const { return _logTimeEndOfMergedCmd; }
    };

} // namespace Log

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
