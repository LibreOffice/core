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
 * Forking mode entry point for COOLWSD daemon.
 * Functions: main()
 */

#include <config.h>

#include <common/StringVector.hpp>
#include <common/TraceEvent.hpp>
#include <common/Util.hpp>
#include <wsd/COOLWSD.hpp>

int createForkit(const std::string& forKitPath, const StringVector& args)
{
    // create forkit in a process
    return ProcUtil::spawnProcess(forKitPath, args);
};

// FIXME: Somewhat idiotically, the parameter to emitOneRecordingIfEnabled() should end with a
// newline, while the paramter to emitOneRecording() should not.

void TraceEvent::emitOneRecordingIfEnabled(const std::string& recording)
{
    if (COOLWSD::TraceEventFile == NULL)
        return;

    COOLWSD::writeTraceEventRecording(recording);
}

void TraceEvent::emitOneRecording(const std::string& recording)
{
    if (COOLWSD::TraceEventFile == NULL)
        return;

    if (!TraceEvent::isRecordingOn())
        return;

    COOLWSD::writeTraceEventRecording(recording + "\n");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
