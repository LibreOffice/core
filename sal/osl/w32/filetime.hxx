/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <osl/time.h>

BOOL TimeValueToFileTime(TimeValue const* cpTimeVal, FILETIME* pFTime);
// bDuration means cpFTime contains a time interval, not a point in time
BOOL FileTimeToTimeValue(FILETIME const* cpFTime, TimeValue* pTimeVal, bool bDuration = false);

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
