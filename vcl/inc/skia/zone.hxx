/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <comphelper/crashzone.hxx>

#include <vcl/dllapi.h>
#include <vcl/svapp.hxx>

// Used around calls to Skia code to detect crashes in drivers.
class VCL_DLLPUBLIC SkiaZone : public CrashZone<SkiaZone>
{
public:
    static void hardDisable();
    static void relaxWatchdogTimings();
    static const CrashWatchdogTimingsValues& getCrashWatchdogTimingsValues();
    static void checkDebug(int nUnchanged, const CrashWatchdogTimingsValues& aTimingValues);
    static const char* name() { return "Skia"; }

private:
    SolarMutexGuard g;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
