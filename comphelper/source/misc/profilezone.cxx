/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <atomic>
#include <iostream>
#include <string_view>

#include <comphelper/sequence.hxx>
#include <comphelper/profilezone.hxx>
#include <osl/time.h>
#include <osl/thread.h>

namespace comphelper
{

std::atomic<bool> ProfileZone::g_bRecording(false);

namespace ProfileRecording
{

static std::vector<OUString> g_aRecording;    // recorded data
static long long g_aSumTime(0);  // overall zone time in microsec
static int g_aNesting;           // level of overlapped zones
static long long g_aStartTime;                // start time of recording
static ::osl::Mutex g_aMutex;

void startRecording()
{
    TimeValue systemTime;
    osl_getSystemTime( &systemTime );
    ::osl::MutexGuard aGuard( g_aMutex );
    g_aStartTime = static_cast<long long>(systemTime.Seconds) * 1000000 + systemTime.Nanosec/1000;
    g_aNesting = 0;
    ProfileZone::g_bRecording = true;
}

void stopRecording()
{
    ProfileZone::g_bRecording = false;
}

long long addRecording(const char * aProfileId, long long aCreateTime)
{
    assert( ProfileZone::g_bRecording );

    TimeValue systemTime;
    osl_getSystemTime( &systemTime );
    long long aTime = static_cast<long long>(systemTime.Seconds) * 1000000 + systemTime.Nanosec/1000;

    if (!aProfileId)
        aProfileId = "(null)";
    OUString aString(aProfileId, strlen(aProfileId), RTL_TEXTENCODING_UTF8);

    OUString sRecordingData(OUString::number(osl_getThreadIdentifier(nullptr)) + " " +
        OUString::number(aTime/1000000.0) + " " + aString + ": " +
        (aCreateTime == 0 ? std::u16string_view(u"start") : std::u16string_view(u"stop")) +
        (aCreateTime != 0 ? (" " + OUString::number((aTime - aCreateTime)/1000.0) + " ms") : OUString("")));

    ::osl::MutexGuard aGuard( g_aMutex );

    g_aRecording.emplace_back(sRecordingData);
    if (aCreateTime == 0)
    {
        g_aNesting++;
        return aTime;
    }
    // neglect ProfileZones created before startRecording
    else if (aCreateTime >= g_aStartTime)
    {
        if (g_aNesting > 0)
            g_aNesting--;
        if (g_aNesting == 0)
            g_aSumTime += aTime - aCreateTime;
    }
    return 0;
}

css::uno::Sequence<OUString> getRecordingAndClear()
{
    bool bRecording;
    std::vector<OUString> aRecording;
    {
        ::osl::MutexGuard aGuard( g_aMutex );
        bRecording = ProfileZone::g_bRecording;
        stopRecording();
        aRecording.swap(g_aRecording);
        long long aSumTime = g_aSumTime;
        aRecording.insert(aRecording.begin(), OUString::number(aSumTime/1000000.0));
    }
    // reset start time and nesting level
    if (bRecording)
        startRecording();
    return ::comphelper::containerToSequence(aRecording);
}

} // namespace ProfileRecording

void ProfileZone::startConsole() { m_aCreateTime = osl_getGlobalTimer(); }

void ProfileZone::stopConsole()
{
    sal_uInt32 nEndTime = osl_getGlobalTimer();
    std::cerr << "comphelper::ProfileZone: " << m_sProfileId << " finished in "
              << nEndTime - m_aCreateTime << " ms" << std::endl;
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
