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
/*
 * Dummy stub implementations for TraceEvent emission.
 */

/*
 * Dummy stub implementations for TraceEvent emission.
 * Functions: emitOneRecordingIfEnabled(), emitOneRecording()
 */

// Compile and link this into test and utility executables that need it to link

#include <config.h>

#include <common/TraceEvent.hpp>

void TraceEvent::emitOneRecordingIfEnabled([[maybe_unused]] const std::string& recording) {}

void TraceEvent::emitOneRecording([[maybe_unused]] const std::string& recording) {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
