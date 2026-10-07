/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "bridge.h"
#include "bridge.hxx"
#include "anyjson.hxx"
#include "volatile.hxx"

#include <comphelper/hash.hxx>
#include <rtl/ref.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <unotools/weakref.hxx>
#include <vcl/svapp.hxx>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <list>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace css;
using namespace cpo;
using namespace co::pythoncompute;

namespace
{
using Clock = std::chrono::steady_clock;
using VolRef = rtl::Reference<PythonComputeVolatileResult>;

/** A transient error (transport, timeout, too large, disabled) is shown for at
 *  least this long before the next recalc may send the same request again. Stops a
 *  finish -> modified -> recalc -> request storm. */
constexpr sal_Int32 kRetryAfterMs = 30000;
/** Limits in units of max_payload_bytes, so they scale with the one admin setting.
 *  Hand-off: the kit is handed requests while those it holds (sent to coolwsd, no result yet)
 *  total at most this much; the rest wait here, #BUSY!, and go as results free room. This
 *  bounds coolwsd's per-document queue, and nothing fails because of it. The first request
 *  always goes, so one at the payload cap is never stuck. */
constexpr sal_Int64 kHandedOffPayloads = 16;
/** Held: all request JSON of pending cells, waiting or handed off. Only a backstop for kit
 *  memory, e.g. inputs that change faster than the service answers: past it a new request
 *  is a transient error (retried after kRetryAfterMs). The only pending request is never
 *  refused. At least kMinHeldBytes, so a small max_payload_bytes cannot fail a big sheet. */
constexpr sal_Int64 kHeldPayloads = 4 * kHandedOffPayloads;
constexpr sal_Int64 kMinHeldBytes = sal_Int64(1) << 30;
/** Default for security.python_compute.max_payload_bytes; the kit passes the real value. */
constexpr sal_Int64 kDefaultMaxPayloadBytes = 16 * 1024 * 1024;

struct CacheEntry
{
    enum class State
    {
        Pending,
        Ok,
        Error
    };
    unotools::WeakReference<PythonComputeVolatileResult> xVolatile;
    State eState = State::Pending;
    Clock::time_point aErrorAt; ///< when the transient error came; retry after nRetryAfterMs
};

struct PendingEntry
{
    VolRef xVolatile;
    std::string cacheKey;
    sal_Int64 nBytes = 0; ///< request JSON size
    bool bHandedOff = false; ///< the kit holds it (counted in nHandedOffBytes)
};

struct OutItem
{
    std::string idKey;
    std::string json;
};

/** All AddIn state. Guarded by SolarMutex only. Heap allocated and never deleted, so no UNO
 *  reference is destroyed after DeInitVCL at process exit. */
struct BridgeState
{
    std::unordered_map<std::string, PendingEntry> aPending; // key: request id
    std::unordered_map<std::string, CacheEntry> aCache; // key: SHA-256 of code+data
    std::list<OutItem> aOutbox; // not yet handed to the kit; a list so requeueing cannot throw
    pythoncompute_wakeup_fn pWakeup = nullptr;
    size_t nPruneAt = 1; ///< cache size of the next prune (doubles: amortized O(1))
    sal_uInt64 nRequestSeq = 0;
    sal_Int32 nRetryAfterMs = kRetryAfterMs;
    sal_Int64 nMinHeldBytes = kMinHeldBytes;
    sal_Int64 nMaxPayloadBytes = kDefaultMaxPayloadBytes;
    sal_Int64 nHeldBytes = 0; ///< sum of aPending nBytes
    sal_Int64 nHandedOffBytes = 0; ///< sum of aPending nBytes with bHandedOff
};

BridgeState& state()
{
    static BridgeState* const pState = new BridgeState;
    return *pState;
}

/** nPayloads x max_payload_bytes, saturating. */
sal_Int64 payloadBudget(const BridgeState& s, sal_Int64 nPayloads)
{
    if (s.nMaxPayloadBytes > std::numeric_limits<sal_Int64>::max() / nPayloads)
        return std::numeric_limits<sal_Int64>::max();
    return s.nMaxPayloadBytes * nPayloads;
}

/** Cache key: hash of the request JSON with a fixed id, so only 64 bytes stay per entry. */
std::string makeCacheKey(const std::string& rKeyJson)
{
    return comphelper::hashToString(comphelper::Hash::calculateHash(
        rKeyJson.data(), rKeyJson.size(), comphelper::HashType::SHA256));
}

void finishQuietly(const VolRef& xVol, const cpo::uno::Any& rValue)
{
    if (!xVol.is())
        return;
    try
    {
        xVol->finish(rValue);
    }
    catch (...)
    {
        SAL_WARN("scaddins.pythoncompute", "finish: exception");
    }
}

/** Marks the cache entry of a pending request. A pending entry's key is never reused: its
 *  volatile cannot lapse (aPending holds it) and its state is not Error. */
void markCacheEntry(BridgeState& s, const std::string& cacheKey, bool bTransient)
{
    auto it = s.aCache.find(cacheKey);
    if (it == s.aCache.end())
        return;
    if (bTransient)
    {
        it->second.eState = CacheEntry::State::Error;
        it->second.aErrorAt = Clock::now();
    }
    else
        it->second.eState = CacheEntry::State::Ok;
}

void removeFromOutbox(BridgeState& s, const std::string& idKey)
{
    for (auto it = s.aOutbox.begin(); it != s.aOutbox.end(); ++it)
    {
        if (it->idKey == idKey)
        {
            s.aOutbox.erase(it);
            return;
        }
    }
}

/** Erases a pending entry, its outbox item and its share of the hand-off budget. */
void erasePending(BridgeState& s, std::unordered_map<std::string, PendingEntry>::iterator it)
{
    s.nHeldBytes -= it->second.nBytes;
    if (it->second.bHandedOff)
        s.nHandedOffBytes -= it->second.nBytes;
    else
        removeFromOutbox(s, it->first); // only a waiting request is still in the outbox
    s.aPending.erase(it);
}

// Dead keys of changed inputs are never looked up: sweep each doubling (amortized O(1), no knob).
void pruneCache(BridgeState& s)
{
    if (s.aCache.size() < s.nPruneAt)
        return;
    for (auto it = s.aCache.begin(); it != s.aCache.end();)
    {
        if (it->second.xVolatile.get().is())
            ++it;
        else
            it = s.aCache.erase(it);
    }
    s.nPruneAt = std::max<size_t>(1, 2 * s.aCache.size());
}

/** A volatile that is already finished with rValue and cached as eState. */
VolRef finishedResult(BridgeState& s, const std::string& cacheKey, const cpo::uno::Any& rValue,
                      CacheEntry::State eState)
{
    VolRef xVol(new PythonComputeVolatileResult);
    xVol->finish(rValue); // no listener yet: cannot re-enter
    pruneCache(s);
    CacheEntry& e = s.aCache[cacheKey];
    e = CacheEntry();
    e.xVolatile = xVol.get();
    e.eState = eState;
    e.aErrorAt = Clock::now();
    return xVol;
}

/** Ask the kit to drain the outbox. Not coalesced: a drain may end without taking (a load
 *  is ongoing, no live view), and a flag would then drop every later wakeup. If the kit
 *  poll is not running, the kit drains after load instead. */
void wake(BridgeState& s)
{
    if (!s.pWakeup || s.aOutbox.empty())
        return;
    try
    {
        s.pWakeup();
    }
    catch (...)
    {
        SAL_WARN("scaddins.pythoncompute", "wake: wakeup function threw");
    }
}
} // namespace

extern "C" SAL_DLLPUBLIC_EXPORT void pythoncompute_set_wakeup(pythoncompute_wakeup_fn fn)
{
    SolarMutexGuard aSolar;
    BridgeState& s = state();
    s.pWakeup = fn;
    wake(s);
}

extern "C" SAL_DLLPUBLIC_EXPORT void pythoncompute_set_max_payload_bytes(int64_t nBytes)
{
    SolarMutexGuard aSolar;
    // 0 or less: the default, as in coolwsd.xml.
    state().nMaxPayloadBytes = nBytes > 0 ? nBytes : kDefaultMaxPayloadBytes;
}

extern "C" SAL_DLLPUBLIC_EXPORT int pythoncompute_take_requests(pythoncompute_request_fn fn,
                                                                void* userdata)
{
    if (!fn)
        return 0;
    std::list<OutItem> aBatch;
    {
        SolarMutexGuard aSolar;
        BridgeState& s = state();
        // Hand off what fits the budget, oldest first; reserve it now.
        const sal_Int64 nBudget = payloadBudget(s, kHandedOffPayloads);
        while (!s.aOutbox.empty())
        {
            const sal_Int64 nSize = static_cast<sal_Int64>(s.aOutbox.front().json.size());
            if (s.nHandedOffBytes > 0 && nSize > nBudget - s.nHandedOffBytes)
                break;
            auto it = s.aPending.find(s.aOutbox.front().idKey);
            if (it != s.aPending.end())
            {
                it->second.bHandedOff = true;
                s.nHandedOffBytes += it->second.nBytes;
            }
            aBatch.splice(aBatch.end(), s.aOutbox, s.aOutbox.begin());
        }
    }
    // The callback does socket I/O: it must not run under Solar.
    assert(!Application::GetSolarMutex().IsCurrentThread());

    int nDelivered = 0;
    auto itRest = aBatch.begin();
    for (; itRest != aBatch.end(); ++itRest)
    {
        int nOk = 0;
        try
        {
            nOk = fn(userdata, itRest->json.data(), static_cast<int32_t>(itRest->json.size()));
        }
        catch (...)
        {
            SAL_WARN("scaddins.pythoncompute", "take_requests: callback exception");
        }
        if (!nOk)
            break;
        ++nDelivered;
    }
    if (itRest == aBatch.end())
        return nDelivered;

    // Not delivered: release their reservation and put them back in order, ahead of
    // anything queued meanwhile. Splicing cannot throw.
    SolarMutexGuard aSolar;
    BridgeState& s = state();
    std::list<OutItem> aRest;
    aRest.splice(aRest.end(), aBatch, itRest, aBatch.end());
    for (auto it = aRest.begin(); it != aRest.end();)
    {
        auto itPending = s.aPending.find(it->idKey);
        if (itPending == s.aPending.end())
        {
            it = aRest.erase(it); // finished meanwhile
            continue;
        }
        s.nHandedOffBytes -= itPending->second.nBytes;
        itPending->second.bHandedOff = false;
        ++it;
    }
    s.aOutbox.splice(s.aOutbox.begin(), aRest);
    return nDelivered;
}

extern "C" SAL_DLLPUBLIC_EXPORT int pythoncompute_complete(const char* jsonUtf8, int32_t len)
{
    SolarMutexGuard aSolar;
    BridgeState& s = state();
    if (!jsonUtf8 || len <= 0)
        return -1;
    const std::string_view json(jsonUtf8, static_cast<size_t>(len));

    VolRef xVol;
    std::string idKey;
    try
    {
        OUString sId;
        if (!extractRequestIdFromJson(json, sId) || sId.isEmpty())
        {
            SAL_INFO("scaddins.pythoncompute", "complete: invalid body bytes=" << len);
            return -1;
        }
        idKey = OUStringToOString(sId, RTL_TEXTENCODING_UTF8).getStr();
        auto it = s.aPending.find(idKey);
        if (it == s.aPending.end())
        {
            SAL_INFO("scaddins.pythoncompute", "complete: unknown or late id=[" << idKey << ']');
            return 0;
        }
        xVol = it->second.xVolatile;
        const std::string cacheKey = it->second.cacheKey;

        cpo::uno::Any aResult;
        OUString sErr;
        const ResultKind eKind = jsonResultToAny(json, aResult, sErr);
        if (!sErr.isEmpty())
            SAL_INFO("scaddins.pythoncompute", "complete: detail=[" << sErr << "] id=[" << idKey << ']');

        // State first, finish last: listeners may re-enter getPy().
        erasePending(s, it);
        markCacheEntry(s, cacheKey, eKind == ResultKind::TransientError);
        finishQuietly(xVol, aResult);
        wake(s); // the result freed hand-off room for waiting requests
        return 1;
    }
    catch (...)
    {
        SAL_WARN("scaddins.pythoncompute", "complete: exception id=[" << idKey << ']');
        // Never leave a #BUSY! volatile with no pending entry behind.
        auto it = s.aPending.find(idKey);
        if (it != s.aPending.end())
        {
            xVol = it->second.xVolatile;
            markCacheEntry(s, it->second.cacheKey, true);
            erasePending(s, it);
        }
        if (xVol.is())
        {
            finishQuietly(xVol, makeFormulaErrorAny(FormulaError::NotAvailable));
            wake(s);
            return 1;
        }
        return -1;
    }
}

extern "C" SAL_DLLPUBLIC_EXPORT void pythoncompute_reset_for_tests()
{
    SolarMutexGuard aSolar;
    BridgeState& s = state();
    std::vector<VolRef> aToFinish;
    for (auto& e : s.aPending)
        aToFinish.push_back(std::move(e.second.xVolatile));
    s.aPending.clear();
    s.aCache.clear();
    s.aOutbox.clear();
    s.pWakeup = nullptr;
    s.nPruneAt = 1;
    s.nRetryAfterMs = kRetryAfterMs;
    s.nMinHeldBytes = kMinHeldBytes;
    s.nMaxPayloadBytes = kDefaultMaxPayloadBytes;
    s.nHeldBytes = 0;
    s.nHandedOffBytes = 0;
    for (const VolRef& xVol : aToFinish)
        finishQuietly(xVol, makeFormulaErrorAny(FormulaError::NotAvailable));
}

extern "C" SAL_DLLPUBLIC_EXPORT void pythoncompute_set_min_held_bytes_for_tests(sal_Int64 nBytes)
{
    SolarMutexGuard aSolar;
    state().nMinHeldBytes = nBytes;
}

extern "C" SAL_DLLPUBLIC_EXPORT void pythoncompute_set_retry_ms_for_tests(sal_Int32 retryMs)
{
    SolarMutexGuard aSolar;
    state().nRetryAfterMs = retryMs < 0 ? 0 : retryMs;
}

extern "C" SAL_DLLPUBLIC_EXPORT int pythoncompute_queued_count_for_tests()
{
    SolarMutexGuard aSolar;
    return static_cast<int>(state().aOutbox.size());
}

namespace co::pythoncompute
{
std::string withRequestId(std::string_view keyJson, std::string_view idKey)
{
    // buildExecuteRequestJson writes "id" first, so the first "_" string is its value.
    static constexpr std::string_view aPlaceholder = "\"_\"";
    const size_t nPos = keyJson.find(aPlaceholder);
    if (nPos == std::string_view::npos || keyJson.substr(0, nPos).find("\"id\"") == std::string_view::npos)
        throw std::logic_error("pythoncompute: request JSON without the id placeholder");
    std::string aOut;
    aOut.reserve(keyJson.size() + idKey.size());
    aOut.append(keyJson.substr(0, nPos + 1));
    aOut.append(idKey);
    aOut.append(keyJson.substr(nPos + aPlaceholder.size() - 1));
    return aOut;
}

VolRef startCompute(const OUString& sCode, const cpo::uno::Sequence<cpo::uno::Any>& aData)
{
    SolarMutexGuard aSolar;
    BridgeState& s = state();

    const std::string keyJson = buildExecuteRequestJson(u"_"_ustr, sCode, aData);
    const std::string cacheKey = makeCacheKey(keyJson);
    const auto now = Clock::now();

    auto itCache = s.aCache.find(cacheKey);
    if (itCache != s.aCache.end())
    {
        VolRef xVol = itCache->second.xVolatile.get();
        const bool bRetry
            = itCache->second.eState == CacheEntry::State::Error
              && now >= itCache->second.aErrorAt + std::chrono::milliseconds(s.nRetryAfterMs);
        if (xVol.is() && !bRetry)
            return xVol; // same params -> same object (AddIn.idl)
        s.aCache.erase(itCache); // lapsed, or a transient error whose window passed
    }

    // Only a kit built to broker =PY() sets this (before any load); without it (desktop,
    // mobile, Online built without python compute) a request would stay #BUSY! forever.
    if (!std::getenv("COOL_PYTHON_COMPUTE"))
        return finishedResult(s, cacheKey, cpo::uno::Any(u"#DISABLED"_ustr), CacheEntry::State::Ok);

    // Size-check the exact bytes that are sent (the real id is longer than "_"). Transient:
    // the kit may raise the cap after a load-time recalc (non-merged builds).
    const std::string idKey = "py-" + std::to_string(++s.nRequestSeq);
    std::string json = withRequestId(keyJson, idKey);
    if (static_cast<sal_Int64>(json.size()) > s.nMaxPayloadBytes)
    {
        SAL_INFO("scaddins.pythoncompute", "startCompute: request " << json.size()
                                               << " bytes over max_payload_bytes");
        return finishedResult(s, cacheKey, makeFormulaErrorAny(FormulaError::NoValue),
                              CacheEntry::State::Error);
    }

    const sal_Int64 nBytes = static_cast<sal_Int64>(json.size());
    const sal_Int64 nHeldCap = std::max(s.nMinHeldBytes, payloadBudget(s, kHeldPayloads));
    if (!s.aPending.empty() && nBytes > nHeldCap - s.nHeldBytes)
    {
        SAL_WARN("scaddins.pythoncompute", "startCompute: " << s.nHeldBytes
                                               << " request bytes held; refused");
        return finishedResult(s, cacheKey, makeFormulaErrorAny(FormulaError::NotAvailable),
                              CacheEntry::State::Error);
    }

    VolRef xVol(new PythonComputeVolatileResult);

    // Publish cache and pending, then queue last; roll back on any throw.
    pruneCache(s);
    try
    {
        CacheEntry& e = s.aCache[cacheKey];
        e = CacheEntry();
        e.xVolatile = xVol.get();
        s.aPending.emplace(idKey, PendingEntry{ xVol, cacheKey, nBytes });
        s.nHeldBytes += nBytes;
        s.aOutbox.push_back(OutItem{ idKey, std::move(json) });
    }
    catch (...)
    {
        if (auto it = s.aPending.find(idKey); it != s.aPending.end())
            erasePending(s, it);
        s.aCache.erase(cacheKey);
        throw;
    }
    SAL_INFO("scaddins.pythoncompute", "startCompute: queued id=[" << idKey << ']');
    wake(s);
    return xVol;
}
} // namespace co::pythoncompute

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
