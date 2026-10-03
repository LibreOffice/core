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
 * Asynchronous DNS resolution using a dedicated background thread.
 * Classes: AsyncDNS, DNSResolver
 */

#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include <net/NetUtil.hpp>

namespace net
{

class DNSResolver;
class HostEntry;

class AsyncDNS
{
public:
    AsyncDNS();
    ~AsyncDNS();

    static void startAsyncDNS();
    static void stopAsyncDNS();

    static void dumpState(std::ostream& os);

    using DNSThreadFn = std::function<void(const HostEntry& hostEntry)>;
    using DNSThreadDumpStateFn = std::function<std::string()>;

    static void lookup(std::string searchEntry, DNSThreadFn cb, DNSThreadDumpStateFn dumpState);

private:
    std::atomic<bool> _exit;
    std::unique_ptr<DNSResolver> _resolver;
    std::unique_ptr<std::thread> _thread;
    std::mutex _lock;
    std::condition_variable _condition;
    struct Lookup
    {
        std::string query;
        AsyncDNS::DNSThreadFn cb;
        AsyncDNS::DNSThreadDumpStateFn dumpState;

        Lookup() = default;
        Lookup(std::string q, AsyncDNS::DNSThreadFn c, AsyncDNS::DNSThreadDumpStateFn d)
            : query(std::move(q))
            , cb(std::move(c))
            , dumpState(std::move(d))
        {
        }
    };
    std::queue<Lookup> _lookups;
    Lookup _activeLookup;

    void resolveDNS();
    void addLookup(std::string lookup, DNSThreadFn cb, DNSThreadDumpStateFn dumpState);

    void startThread();
    void joinThread();

    void dumpQueueState(std::ostream& os) const;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
