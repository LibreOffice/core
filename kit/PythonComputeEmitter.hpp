/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Bridges coolkit ChildSession ↔ LibreOffice scaddins pythoncompute library
 * (libpythoncomputelo.so): finds its C API once Calc has loaded it.
 */

#pragma once

#include <string>

/// =PY() is brokered only with --enable-python-compute and never on mobile.
#if defined(ENABLE_PYTHON_COMPUTE) && ENABLE_PYTHON_COMPUTE && !MOBILEAPP
#define PYTHON_COMPUTE_ENABLED 1
#else
#define PYTHON_COMPUTE_ENABLED 0
#endif

class Document;

/// Kit side of the Calc =PY() AddIn (engine scaddins/source/pythoncompute/bridge.h).
///
/// Threading and locks: everything here runs on the kit poll thread and takes no lock.
/// The AddIn calls our wakeup with SolarMutex held; it only queues a poll callback (the
/// poll's callback mutex is a leaf). The drain runs from that callback with SolarMutex
/// released (kitPoll runs under SolarMutexReleaser) and pythoncompute_take_requests
/// releases its own guard before it calls us, so socket I/O never happens under Solar.
/// One document per kit process: requests and results are document-level.
namespace pythoncompute
{
/// Resolve the AddIn's C API (global symbols, else its already-loaded library) and install
/// the wakeup and payload cap once found. Cheap to call again.
void ensureWakeup();

/// Send queued requests through a live session of doc. No-op while a load is ongoing.
void drain(Document& doc);

/// Finish the AddIn request for a document-level pythoncomputeresult: body. Unknown or late
/// ids and invalid bodies are logged and dropped.
void completeFromJson(const std::string& json);
} // namespace pythoncompute

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
