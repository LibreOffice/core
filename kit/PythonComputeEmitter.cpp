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

#include "PythonComputeEmitter.hpp"

#if PYTHON_COMPUTE_ENABLED

#include "ChildSession.hpp"
#include "Kit.hpp"

#include <common/ConfigUtil.hpp>
#include <common/Log.hpp>

#include <cstdint>
#include <dlfcn.h>
#include <memory>

namespace
{
using WakeupFn = void (*)();
using RequestFn = int (*)(void* userdata, const char* jsonUtf8, std::int32_t len);
using SetWakeupFn = void (*)(WakeupFn);
using SetMaxPayloadFn = void (*)(std::int64_t);
using TakeRequestsFn = int (*)(RequestFn, void*);
using CompleteFn = int (*)(const char* jsonUtf8, std::int32_t len);

/// Matches security.python_compute.max_payload_bytes in coolwsd.xml.in.
constexpr std::int64_t DEFAULT_MAX_PAYLOAD_BYTES = 16 * 1024 * 1024;

// Kit poll thread only.
SetWakeupFn gSetWakeup = nullptr;
SetMaxPayloadFn gSetMaxPayload = nullptr;
TakeRequestsFn gTakeRequests = nullptr;
CompleteFn gComplete = nullptr;
bool gInstalled = false;

void assertKitThread()
{
    if (KitSocketPoll* poll = KitSocketPoll::getMainPoll())
        poll->assertCorrectThread();
}

/// Merged builds export the AddIn's symbols globally; otherwise look in the AddIn
/// library once Calc has loaded it (NOLOAD never loads it, so runs no constructors).
void* findSymbol(const char* name)
{
    if (void* p = dlsym(RTLD_DEFAULT, name))
        return p;
    void* lib = dlopen("libpythoncomputelo.so", RTLD_NOW | RTLD_NOLOAD);
    return lib ? dlsym(lib, name) : nullptr; // keep the reference: the AddIn stays loaded
}

/// Cheap when the AddIn is absent (e.g. non-Calc documents): one failed lookup.
bool resolveSymbols()
{
    if (gComplete)
        return true;
    gSetWakeup = reinterpret_cast<SetWakeupFn>(findSymbol("pythoncompute_set_wakeup"));
    if (!gSetWakeup)
        return false;
    gSetMaxPayload
        = reinterpret_cast<SetMaxPayloadFn>(findSymbol("pythoncompute_set_max_payload_bytes"));
    gTakeRequests = reinterpret_cast<TakeRequestsFn>(findSymbol("pythoncompute_take_requests"));
    gComplete = reinterpret_cast<CompleteFn>(findSymbol("pythoncompute_complete"));
    if (gSetMaxPayload && gTakeRequests && gComplete)
        return true;
    gComplete = nullptr; // never half-installed
    return false;
}

void drainMainDocument()
{
    try
    {
        KitSocketPoll* poll = KitSocketPoll::getMainPoll();
        if (!poll)
            return;
        const std::shared_ptr<Document> doc = poll->getDocument();
        if (doc)
            pythoncompute::drain(*doc);
    }
    catch (const std::exception& ex)
    {
        LOG_ERR("pythoncompute: drain failed: " << ex.what());
    }
}

/// Called by the AddIn with SolarMutex held: only queue a callback, never block.
void wakeup()
{
    if (KitSocketPoll* poll = KitSocketPoll::getMainPoll())
        poll->addCallback([] { drainMainDocument(); });
}

struct SendTarget
{
    ChildSession* session = nullptr;
    bool failed = false;
};

/// Called by pythoncompute_take_requests without SolarMutex.
int sendRequest(void* userdata, const char* jsonUtf8, std::int32_t len)
{
    auto* target = static_cast<SendTarget*>(userdata);
    if (!target || !target->session || !jsonUtf8 || len <= 0)
        return 0;
    target->failed = true; // cleared on success
    ChildSession* session = target->session;
    try
    {
        std::string frame("pythoncompute: ");
        frame.append(jsonUtf8, static_cast<std::size_t>(len));
        if (!session->sendTextFrame(frame))
            return 0;
        target->failed = false;
        return 1;
    }
    catch (const std::exception& ex)
    {
        LOG_ERR("pythoncompute: send failed: " << ex.what());
        return 0;
    }
}
} // namespace

namespace pythoncompute
{
void ensureWakeup()
{
    assertKitThread();
    if (gInstalled || !resolveSymbols())
        return;
    std::int64_t maxPayload = ConfigUtil::getInt("security.python_compute.max_payload_bytes",
                                                 static_cast<int>(DEFAULT_MAX_PAYLOAD_BYTES));
    // 0 or less means the default, in the AddIn as in coolwsd.
    gSetMaxPayload(maxPayload);
    gSetWakeup(&wakeup); // wakes at once if requests are already queued
    gInstalled = true;
    LOG_INF("pythoncompute: AddIn wakeup installed, max_payload_bytes=" << maxPayload);
}

void drain(Document& doc)
{
    assertKitThread();
    if (doc.isLoadOngoing())
        return; // drained again right after onLoad
    ensureWakeup();
    if (!gTakeRequests)
        return;
    // Requests are document-level: any live view can carry them. If one view fails
    // (it is closing), the rest of the batch is requeued and the next view is tried.
    for (const std::shared_ptr<ChildSession>& session : doc.findLivePythonComputeSessions())
    {
        SendTarget target{ session.get(), false };
        const int sent = gTakeRequests(&sendRequest, &target);
        LOG_DBG("pythoncompute: drained " << sent << " request(s) via session "
                                          << session->getId());
        if (!target.failed)
            return;
    }
    // No view accepted them: they stay queued, drained on the next session add or load.
}

void completeFromJson(const std::string& json)
{
    assertKitThread();
    if (!gComplete)
    {
        LOG_WRN("pythoncompute: result before the AddIn is loaded; dropped, bytes=" << json.size());
        return;
    }
    const int rc = gComplete(json.data(), static_cast<std::int32_t>(json.size()));
    if (rc < 0)
        LOG_WRN("pythoncompute: invalid result body, bytes=" << json.size());
    else if (rc == 0)
        LOG_DBG("pythoncompute: late or unknown result id, ignored");
}
} // namespace pythoncompute

#else // !PYTHON_COMPUTE_ENABLED

namespace pythoncompute
{
void ensureWakeup() {}
void drain(Document&) {}
void completeFromJson(const std::string&) {}
} // namespace pythoncompute

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
