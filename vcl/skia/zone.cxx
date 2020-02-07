/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <skia/zone.hxx>

#include <officecfg/Office/Common.hxx>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>

#include <sal/log.hxx>

/**
 * Called from a signal handler or watchdog thread if we get
 * a crash or hang in some driver.
 */
void SkiaZone::hardDisable()
{
    // protect ourselves from double calling etc.
    static bool bDisabled = false;
    if (!bDisabled)
    {
        bDisabled = true;

        // Instead of disabling Skia as a whole, only force the CPU-based
        // raster mode, which should be safe as it does not use drivers.
        std::shared_ptr<comphelper::ConfigurationChanges> xChanges(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::VCL::ForceSkiaRaster::set(true, xChanges);
        xChanges->commit();

        // Force synchronous config write
        css::uno::Reference<css::util::XFlushable>(
            css::configuration::theDefaultProvider::get(comphelper::getProcessComponentContext()),
            css::uno::UNO_QUERY_THROW)
            ->flush();
    }
}

void SkiaZone::checkDebug(int nUnchanged, const CrashWatchdogTimingsValues& aTimingValues)
{
    SAL_INFO("vcl.watchdog", "Skia watchdog - unchanged "
                                 << nUnchanged << " enter count " << enterCount()
                                 << " breakpoints mid: " << aTimingValues.mnDisableEntries
                                 << " max " << aTimingValues.mnAbortAfter);
}

const CrashWatchdogTimingsValues& SkiaZone::getCrashWatchdogTimingsValues()
{
    static const CrashWatchdogTimingsValues values = { 6, 20 }; /* 1.5s,  5s */
    return values;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
