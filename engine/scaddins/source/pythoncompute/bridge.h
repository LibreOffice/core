/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * C API between the =PY() AddIn and the Online kit (same process, found with
 * dlsym(RTLD_DEFAULT)).
 *
 * Threading and lock contract (the whole graph; see README.md "Locking"):
 *  - AddIn state is guarded by SolarMutex only. No other lock exists in this code.
 *  - The AddIn never calls kit code synchronously except the wakeup function,
 *    which must only queue a callback on the kit poll (a leaf lock) and return.
 *  - pythoncompute_take_requests() releases SolarMutex before it calls the
 *    request callback, so kit socket I/O never runs under Solar.
 *  - One document per kit process: requests carry no document or view affinity.
 */

#pragma once

#include <sal/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Asks the kit to call pythoncompute_take_requests() soon, from its poll loop.
 * Called with SolarMutex held; must not block or call back into the AddIn.
 */
typedef void (*pythoncompute_wakeup_fn)(void);

/**
 * Delivers one queued request to the service.
 * Called WITHOUT SolarMutex held (from pythoncompute_take_requests).
 * @return 1 if the request was handed over, 0 if it could not be delivered now;
 *         on 0 this and every later request of the batch are queued again.
 */
typedef int (*pythoncompute_request_fn)(void* userdata, const char* jsonUtf8, int32_t len);

/** Installed by the kit once; may be called again with the same or a new function. */
SAL_DLLPUBLIC_EXPORT void pythoncompute_set_wakeup(pythoncompute_wakeup_fn fn);

/** Largest request accepted, in bytes (security.python_compute.max_payload_bytes). */
SAL_DLLPUBLIC_EXPORT void pythoncompute_set_max_payload_bytes(int64_t nBytes);

/**
 * Moves queued requests out under SolarMutex, releases it, and passes each one to fn.
 * Only as many as fit the hand-off budget (16 x max_payload_bytes of requests the kit
 * holds without a result; at least one) are passed; the rest wait for results.
 * Requests that fn does not accept are queued again, in order.
 * @return the number of requests delivered.
 */
SAL_DLLPUBLIC_EXPORT int pythoncompute_take_requests(pythoncompute_request_fn fn, void* userdata);

/**
 * Finishes a pending XVolatileResult from a pythoncomputeresult JSON body.
 * @return 1 completed, 0 unknown or late id (ignore quietly), -1 invalid body.
 */
SAL_DLLPUBLIC_EXPORT int pythoncompute_complete(const char* jsonUtf8, int32_t len);

/** Finishes every pending request with #N/A, clears all state, restores defaults. */
SAL_DLLPUBLIC_EXPORT void pythoncompute_reset_for_tests();

/** Override the 1 GiB floor of the held-bytes backstop (CppUnit only). */
SAL_DLLPUBLIC_EXPORT void pythoncompute_set_min_held_bytes_for_tests(sal_Int64 nBytes);

/** Override the retry-after window for error results (CppUnit only). Clamped to >= 0 ms. */
SAL_DLLPUBLIC_EXPORT void pythoncompute_set_retry_ms_for_tests(sal_Int32 retryMs);

/** Number of requests waiting in the outbox (CppUnit only). */
SAL_DLLPUBLIC_EXPORT int pythoncompute_queued_count_for_tests();

#ifdef __cplusplus
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
