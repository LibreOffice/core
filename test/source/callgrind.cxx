/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/callgrind.hxx>

#ifdef HAVE_VALGRIND_HEADERS
#include <valgrind/callgrind.h>
#else
#define CALLGRIND_START_INSTRUMENTATION
#define CALLGRIND_ZERO_STATS
#define CALLGRIND_STOP_INSTRUMENTATION
#define CALLGRIND_DUMP_STATS_AT(name) (void)name;
#endif

void callgrindStart()
{
    CALLGRIND_ZERO_STATS;
    CALLGRIND_START_INSTRUMENTATION;
};

void callgrindDump(const char* name)
{
    CALLGRIND_STOP_INSTRUMENTATION;
    CALLGRIND_DUMP_STATS_AT(name);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
