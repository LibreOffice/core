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

std::atomic<bool> ProfileZone::s_bRecording = false;

int ProfileZone::s_nNesting = 0; // level of overlapped zones

namespace
{
    std::vector<OUString> g_aRecording; // recorded data
    long long g_nSumTime(0);  // overall zone time in microsec
    long long g_nStartTime;   // start time of recording
    ::osl::Mutex g_aMutex;
}

void ProfileZone::startRecording()
{
    TimeValue aSystemTime;
    osl_getSystemTime(&aSystemTime );
    ::osl::MutexGuard aGuard(g_aMutex);
    g_nStartTime = static_cast<long long>(aSystemTime.Seconds) * 1000000 + aSystemTime.Nanosec/1000;
    s_nNesting = 0;
    s_bRecording = true;
}

void ProfileZone::stopRecording()
{
    s_bRecording = false;
}

void ProfileZone::addRecording()
{
    assert(s_bRecording);

    TimeValue aSystemTime;
    osl_getSystemTime(&aSystemTime);
    long long nNow = static_cast<long long>(aSystemTime.Seconds) * 1000000 + aSystemTime.Nanosec/1000;

     // Generate a single "Complete Event" (type X)
    OUString sRecordingData("{"
                            "\"name\":\"" + OUString(m_sProfileId, strlen(m_sProfileId), RTL_TEXTENCODING_UTF8) + "\","
                            "\"ph\":\"X\","
                            "\"ts\":" + OUString::number(m_nCreateTime) + ","
                            "\"dur\":" + OUString::number(nNow - m_nCreateTime) + ","
                            "\"pid\":" + OUString::number(m_nPid) + ","
                            "\"tid\":" + OUString::number(osl_getThreadIdentifier(nullptr)) +
                            "}");
    ::osl::MutexGuard aGuard(g_aMutex);

    g_aRecording.emplace_back(sRecordingData);
    if (s_nNesting == 0)
        g_nSumTime += nNow - m_nCreateTime;
}

css::uno::Sequence<OUString> ProfileZone::getRecordingAndClear()
{
    bool bRecording;
    std::vector<OUString> aRecording;
    {
        ::osl::MutexGuard aGuard( g_aMutex );
        bRecording = s_bRecording;
        stopRecording();
        aRecording.swap(g_aRecording);
        long long nSumTime = g_nSumTime;
        aRecording.insert(aRecording.begin(), OUString::number(nSumTime/1000000.0));
    }
    // reset start time and nesting level
    if (bRecording)
        startRecording();
    return ::comphelper::containerToSequence(aRecording);
}

void ProfileZone::startConsole() { m_nCreateTime = osl_getGlobalTimer(); }

void ProfileZone::stopConsole()
{
    sal_uInt32 nEndTime = osl_getGlobalTimer();
    std::cerr << "comphelper::ProfileZone: " << m_sProfileId << " finished in "
              << nEndTime - m_nCreateTime << " ms" << std::endl;
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
