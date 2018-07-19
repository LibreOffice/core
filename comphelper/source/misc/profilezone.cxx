/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/sequence.hxx>
#include <comphelper/profilezone.hxx>
#include <osl/time.h>
#include <osl/thread.h>

namespace comphelper
{

namespace ProfileRecording
{

static bool g_bRecording(false);              // true during recording
static std::vector<OUString> g_aRecording;    // recorded data
static long long g_aSumTime(0);  // overall zone time in microsec
static int g_aNesting;           // level of overlapped zones
static long long g_aStartTime;                // start time of recording
static ::osl::Mutex g_aMutex;

void startRecording(bool bStartRecording)
{
    ::osl::MutexGuard aGuard( g_aMutex );
    if (bStartRecording)
    {
        TimeValue systemTime;
        osl_getSystemTime( &systemTime );
        g_aStartTime = static_cast<long long>(systemTime.Seconds) * 1000000 + systemTime.Nanosec/1000;
        g_aNesting = 0;
    }
    g_bRecording = bStartRecording;
}

long long addRecording(const char * aProfileId, long long aCreateTime)
{
    ::osl::MutexGuard aGuard( g_aMutex );
    if ( g_bRecording )
    {
        TimeValue systemTime;
        osl_getSystemTime( &systemTime );
        long long aTime = static_cast<long long>(systemTime.Seconds) * 1000000 + systemTime.Nanosec/1000;
        if (!aProfileId)
            aProfileId = "(null)";
        OUString aString(aProfileId, strlen(aProfileId), RTL_TEXTENCODING_UTF8);
        g_aRecording.emplace_back(OUString::number(osl_getThreadIdentifier(nullptr)) + " " +
            OUString::number(aTime/1000000.0) + " " + aString + ": " +
            (aCreateTime == 0 ? OUString("start") : OUString("stop")) +
            (aCreateTime != 0 ? (" " + OUString::number((aTime - aCreateTime)/1000.0) + " ms") : OUString(""))
        );
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
    }
    return 0;
}

css::uno::Sequence<OUString> getRecordingAndClear()
{
    bool bRecording;
    std::vector<OUString> aRecording;
    {
        ::osl::MutexGuard aGuard( g_aMutex );
        bRecording = g_bRecording;
        startRecording(false);
        aRecording.swap(g_aRecording);
        long long aSumTime = g_aSumTime;
        aRecording.insert(aRecording.begin(), OUString::number(aSumTime/1000000.0));
    }
    // reset start time and nesting level
    startRecording(bRecording);
    return ::comphelper::containerToSequence(aRecording);
}

} // namespace ProfileRecording


ProfileZone::ProfileZone(const char * sProfileId) :
    m_sProfileId(sProfileId),
    m_aCreateTime(ProfileRecording::addRecording(sProfileId, 0))
{
}

ProfileZone::~ProfileZone()
{
    ProfileRecording::addRecording(m_sProfileId, m_aCreateTime);
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
