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

#include <algorithm>
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
        for (auto i : args)
        {
            if (!first)
                sResult.append(',');
            sResult.append('"');
            sResult.append(i.first);
            sResult.append("\",\"");
            sResult.append(i.second);
            sResult.append('"');
            first = false;
        }
        sResult.append('}');

        return sResult.makeStringAndClear();
    }

    const int m_nPid;
    const OUString m_sArgs;

    TraceEvent(std::map<OUString, OUString> args)
        : m_nPid(getPid())
        , m_sArgs(createArgsString(args))
    {
    }

public:
    static void addInstantEvent(const char* sName, const std::map<OUString, OUString>& args
                                                   = std::map<OUString, OUString>());

    static void startRecording();
    static void stopRecording();

    static std::vector<OUString> getEventVectorAndClear();

    static css::uno::Sequence<OUString> getRecordingAndClear();
};

class COMPHELPER_DLLPUBLIC NamedEvent : public TraceEvent
{
protected:
    const char* m_sName;

    NamedEvent(const char* sName,
               const std::map<OUString, OUString>& args = std::map<OUString, OUString>())
        : TraceEvent(args)
        , m_sName(sName ? sName : "(null)")
    {
    }
};

// An AsyncEvent generates a 'b' (begin) event when constructed and an 'e' (end) event when it
// itself or its nesting parent (if there is one) is destructed (or earlier, if requested)

// There are two kinds of async event pairs: Freestanding ones that are not related to other events
// at all, and nested ones that have to be nested between the 'b' and 'e' events of a parent Async
// event.

// To generate a pair of 'b' and 'e' events, create an AsyncEvent object using the AsyncEvent(const
// char* sName) constructor when you want the 'b' event to be generated, and destroy it when you
// want the corresponding 'e' event to be generated.

// To generate a pair of 'b' and 'e' events that is nested inside an outer 'b' and 'e' event pair,
// create an AsyncEvent object using the createWithParent() function. It returns a weak reference
// (weak_ptr) to the AsyncEvent. The parent keeps a strong reference (shared_ptr) to it.

// The 'e' event will be generated when the parent is about to go away, before the parent's 'e'
// event. When the parent has gone away, the weak reference will have expired. You can also generate
// it explicitly by calling the finish() function. (But in that case you could as well have used a
// freestanding AsyncEvent object, I think.)

class COMPHELPER_DLLPUBLIC AsyncEvent : public NamedEvent,
                                        public std::enable_shared_from_this<AsyncEvent>
{
    static int s_nIdCounter;
    int m_nId;
    std::vector<std::shared_ptr<AsyncEvent>> m_aChildren;
    std::weak_ptr<AsyncEvent> m_pParent;
    bool m_bBeginRecorded;

    AsyncEvent(const char* sName, int nId, const std::map<OUString, OUString>& args)
        : NamedEvent(sName, args)
        , m_nId(nId)
        , m_bBeginRecorded(false)
    {
        if (s_bRecording)
        {
            long long nNow = getNow();

            // Generate a "Begin " (type b) event
            TraceEvent::addRecording("{"
                                     "\"name\":\""
                                     + OUString(m_sName, strlen(m_sName), RTL_TEXTENCODING_UTF8)
                                     + "\","
                                       "\"ph\":\"b\""
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

            // In case somebody is holding on to a hard reference to a child we need to tell the
            // children to finish up explicitly, we can't rely on our pointers to them being the
            // only ones.
            for (auto& i : m_aChildren)
                i->generateEnd();

            m_aChildren.clear();

            long long nNow = getNow();
            // Generate a "Env " (type e) event
            TraceEvent::addRecording("{"
                                     "\"name\":\""
                                     + OUString(m_sName, strlen(m_sName), RTL_TEXTENCODING_UTF8)
                                     + "\","
                                       "\"ph\":\"e\""
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

    static std::weak_ptr<AsyncEvent>
    createWithParent(const char* sName, std::shared_ptr<AsyncEvent> pParent,
                     const std::map<OUString, OUString>& args = std::map<OUString, OUString>())
    {
        std::shared_ptr<AsyncEvent> pResult;

        if (s_bRecording && pParent->m_bBeginRecorded)
        {
            pResult.reset(new AsyncEvent(sName, pParent->m_nId, args));
            pParent->m_aChildren.push_back(pResult);
            pResult->m_pParent = pParent;
        }

        return pResult;
    }

    void finish()
    {
        generateEnd();

        auto pParent = m_pParent.lock();
        if (!pParent)
            return;

        pParent->m_aChildren.erase(std::remove(pParent->m_aChildren.begin(),
                                               pParent->m_aChildren.end(), shared_from_this()),
                                   pParent->m_aChildren.end());
        m_pParent.reset();
    }
};

} // namespace comphelper

#endif // INCLUDED_COMPHELPER_TRACEEVENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
