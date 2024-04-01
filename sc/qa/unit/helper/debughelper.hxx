/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#ifdef _WIN32
#if !defined NOMINMAX
#define NOMINMAX
#endif
#include <prewin.h>
#include <postwin.h>
#undef NOMINMAX
#undef RGB
#endif

#define CALC_DEBUG_OUTPUT 0
#define CALC_TEST_PERF 0

#include <sal/types.h>

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
