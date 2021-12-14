/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef INCLUDED_COMPHELPER_TRACEEVENT_HXX
#define INCLUDED_COMPHELPER_TRACEEVENT_HXX

#include <sal/config.h>

#include <atomic>
#include <map>
#include <memory>
#include <vector>

#include <osl/process.h>
#include <osl/thread.h>
#include <osl/time.h>
#include <com/sun/star/uno/Sequence.h>
#include <comphelper/comphelperdllapi.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>

// implementation of XToolkitExperimental profiling API

namespace comphelper
{
class COMPHELPER_DLLPUBLIC TraceEvent
{
private:
    static int getPid()
    {
        oslProcessInfo aProcessInfo;
        aProcessInfo.Size = sizeof(oslProcessInfo);
        if (osl_getProcessInfo(nullptr, osl_Process_IDENTIFIER, &aProcessInfo)
            == osl_Process_E_None)
            return aProcessInfo.Ident;
        return -1;
    }

    static std::size_t s_nBufferSize;
    static void (*s_pBufferFullCallback)();

protected:
    static std::atomic<bool> s_bRecording; // true during recording

    static void addRecording(const OUString& sObject);

    static long long getNow()
    {
        TimeValue systemTime;
        osl_getSystemTime(&systemTime);
        return static_cast<long long>(systemTime.Seconds) * 1000000 + systemTime.Nanosec / 1000;
    }

    static OUString createArgsString(const std::map<OUString, OUString>& args)
    {
        if (args.size() == 0)
            return "";

        OUStringBuffer sResult;
        sResult.append(",\"args\":{");
        bool first = true;
        for (auto& i : args)
        {
            if (!first)
                sResult.append(',');
            sResult.append('"');
            sResult.append(i.first);
            sResult.append("\":\"");
            sResult.append(i.second);
            sResult.append('"');
            first = false;
        }
        sResult.append('}');

        return sResult.makeStringAndClear();
    }

    const int m_nPid;
    const OUString m_sArgs;

    TraceEvent(const OUString& sArgs)
        : m_nPid(s_bRecording ? getPid() : 1)
        , m_sArgs(sArgs)
    {
    }

    TraceEvent(const std::map<OUString, OUString>& aArgs)
        : TraceEvent(createArgsString(aArgs))
    {
    }

public:
    static void addInstantEvent(const char* sName, const std::map<OUString, OUString>& args
                                                   = std::map<OUString, OUString>());

    static void startRecording();
    static void stopRecording();
    static void setBufferSizeAndCallback(std::size_t bufferSize, void (*bufferFullCallback)());

    static std::vector<OUString> getEventVectorAndClear();

    static css::uno::Sequence<OUString> getRecordingAndClear();
};

class COMPHELPER_DLLPUBLIC NamedEvent : public TraceEvent
{
protected:
    const char* m_sName;

    NamedEvent(const char* sName, const OUString& sArgs)
        : TraceEvent(sArgs)
        , m_sName(sName ? sName : "(null)")
    {
    }

    NamedEvent(const char* sName, const std::map<OUString, OUString>& aArgs)
        : TraceEvent(aArgs)
        , m_sName(sName ? sName : "(null)")
    {
    }
};

// An AsyncEvent generates an 'S' (start) event when constructed and a 'F' (finish) event when it
// is destructed.

// The Trace Event specification claims that these event types are deprecated and replaces by
// nestable 'b' (begin) and 'e' (end) events, but Chrome does not seem to support those.

// To generate a pair of 'S' and 'F' events, create an AsyncEvent object using the AsyncEvent(const
// char* sName) constructor when you want the 'S' event to be generated, and destroy it when you
// want the corresponding 'F' event to be generated.

class COMPHELPER_DLLPUBLIC AsyncEvent : public NamedEvent,
                                        public std::enable_shared_from_this<AsyncEvent>
{
    static int s_nIdCounter;
    int m_nId;
    bool m_bBeginRecorded;

    AsyncEvent(const char* sName, int nId, const std::map<OUString, OUString>& args)
        : NamedEvent(sName, args)
        , m_nId(nId)
        , m_bBeginRecorded(false)
    {
        if (s_bRecording)
        {
            long long nNow = getNow();

            // Generate a "Start" (type S) event
            TraceEvent::addRecording("{"
                                     "\"name\":\""
                                     + OUString(m_sName, strlen(m_sName), RTL_TEXTENCODING_UTF8)
                                     + "\","
                                       "\"ph\":\"S\""
                                       ","
                                       "\"id\":"
                                     + OUString::number(m_nId) + m_sArgs
                                     + ","
                                       "\"ts\":"
                                     + OUString::number(nNow)
                                     + ","
                                       "\"pid\":"
                                     + OUString::number(m_nPid)
                                     + ","
                                       "\"tid\":"
                                     + OUString::number(osl_getThreadIdentifier(nullptr)) + "},");
            m_bBeginRecorded = true;
        }
    }

    void generateEnd()
    {
        if (m_bBeginRecorded)
        {
            m_bBeginRecorded = false;

            long long nNow = getNow();
            // Generate a "Finish" (type F) event
            TraceEvent::addRecording("{"
                                     "\"name\":\""
                                     + OUString(m_sName, strlen(m_sName), RTL_TEXTENCODING_UTF8)
                                     + "\","
                                       "\"ph\":\"F\""
                                       ","
                                       "\"id\":"
                                     + OUString::number(m_nId) + m_sArgs
                                     + ","
                                       "\"ts\":"
                                     + OUString::number(nNow)
                                     + ","
                                       "\"pid\":"
                                     + OUString::number(m_nPid)
                                     + ","
                                       "\"tid\":"
                                     + OUString::number(osl_getThreadIdentifier(nullptr)) + "},");
        }
    }

public:
    AsyncEvent(const char* sName,
               const std::map<OUString, OUString>& args = std::map<OUString, OUString>())
        : AsyncEvent(sName, s_nIdCounter++, args)
    {
    }

    ~AsyncEvent() { generateEnd(); }

    void finish() { generateEnd(); }
};

} // namespace comphelper

#endif // INCLUDED_COMPHELPER_TRACEEVENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
