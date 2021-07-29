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

#include <comphelper/profilezone.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/traceevent.hxx>

namespace comphelper
{
#ifdef DBG_UTIL
std::atomic<bool> TraceEvent::s_bRecording = (getenv("TRACE_EVENT_RECORDING") != nullptr);
#else
std::atomic<bool> TraceEvent::s_bRecording = false;
#endif

std::size_t TraceEvent::s_nBufferSize = 0;
void (*TraceEvent::s_pBufferFullCallback)() = nullptr;

int AsyncEvent::s_nIdCounter = 0;

static thread_local int nProfileZoneNesting = 0; // Level of Nested Profile Zones

namespace
{
std::vector<OUString> g_aRecording; // recorded data
osl::Mutex g_aMutex;
}

void TraceEvent::addRecording(const OUString& sObject)
{
    osl::MutexGuard aGuard(g_aMutex);

    g_aRecording.emplace_back(sObject);

    if (s_nBufferSize > 0 && g_aRecording.size() >= s_nBufferSize)
    {
        if (s_pBufferFullCallback != nullptr)
            (*s_pBufferFullCallback)();
    }
}

void TraceEvent::addInstantEvent(const char* sName, const std::map<OUString, OUString>& args)
{
    long long nNow = getNow();

    int nPid = 0;
    oslProcessInfo aProcessInfo;
    aProcessInfo.Size = sizeof(oslProcessInfo);
    if (osl_getProcessInfo(nullptr, osl_Process_IDENTIFIER, &aProcessInfo) == osl_Process_E_None)
        nPid = aProcessInfo.Ident;

    addRecording("{"
                 "\"name:\""
                 + OUString(sName, strlen(sName), RTL_TEXTENCODING_UTF8)
                 + "\","
                   "\"ph\":\"i\""
                 + createArgsString(args) + ",\"ts\":" + OUString::number(nNow)
                 + ","
                   "\"pid\":"
                 + OUString::number(nPid)
                 + ","
                   "\"tid\":"
                 + OUString::number(osl_getThreadIdentifier(nullptr)) + "},");
}

void TraceEvent::startRecording()
{
    osl::MutexGuard aGuard(g_aMutex);
    s_bRecording = true;
}

void TraceEvent::stopRecording() { s_bRecording = false; }

void TraceEvent::setBufferSizeAndCallback(std::size_t bufferSize, void (*bufferFullCallback)())
{
    s_nBufferSize = bufferSize;
    s_pBufferFullCallback = bufferFullCallback;
}

std::vector<OUString> TraceEvent::getEventVectorAndClear()
{
    bool bRecording;
    std::vector<OUString> aRecording;
    {
        osl::MutexGuard aGuard(g_aMutex);
        bRecording = s_bRecording;
        stopRecording();
        aRecording.swap(g_aRecording);
    }
    // reset start time and nesting level
    if (bRecording)
        startRecording();
    return aRecording;
}

css::uno::Sequence<OUString> TraceEvent::getRecordingAndClear()
{
    return comphelper::containerToSequence(getEventVectorAndClear());
}

void ProfileZone::addRecording()
{
    assert(s_bRecording);

    long long nNow = getNow();

    // Generate a single "Complete Event" (type X)
    TraceEvent::addRecording("{"
                             "\"name\":\""
                             + OUString(m_sName, strlen(m_sName), RTL_TEXTENCODING_UTF8)
                             + "\","
                               "\"ph\":\"X\","
                               "\"ts\":"
                             + OUString::number(m_nCreateTime)
                             + ","
                               "\"dur\":"
                             + OUString::number(nNow - m_nCreateTime) + m_sArgs
                             + ","
                               "\"pid\":"
                             + OUString::number(m_nPid)
                             + ","
                               "\"tid\":"
                             + OUString::number(osl_getThreadIdentifier(nullptr)) + "},");
}

int ProfileZone::getNestingLevel() { return nProfileZoneNesting; }

void ProfileZone::setNestingLevel(int nNestingLevel) { nProfileZoneNesting = nNestingLevel; }

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
