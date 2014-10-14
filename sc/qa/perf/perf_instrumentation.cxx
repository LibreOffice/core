/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/perf_instrumentation.hxx>
#include <valgrind/callgrind.h>
#include <string>

#include "cppunit/extensions/HelperMacros.h"

void PerfInstrumentation::startPerfInstrumentation()
{
  #ifdef IS_PERFCHECK
    CALLGRIND_START_INSTRUMENTATION;
    CALLGRIND_ZERO_STATS;
  #endif
}

void PerfInstrumentation::endPerfInstrumentation(const char* message)
{
  // TODO : avoid [-Wunused-parameter] at build time
  CPPUNIT_ASSERT_MESSAGE(message , true);

  #ifdef IS_PERFCHECK
    CALLGRIND_DUMP_STATS_AT(message);
    CALLGRIND_STOP_INSTRUMENTATION;
  #endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
