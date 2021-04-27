/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef INCLUDED_COMPHELPER_PROFILEZONE_HXX
#define INCLUDED_COMPHELPER_PROFILEZONE_HXX

#include <sal/config.h>

#include <comphelper/traceevent.hxx>

// implementation of XToolkitExperimental profiling API

namespace comphelper
{
class COMPHELPER_DLLPUBLIC ProfileZone : public NamedEvent
{
    static int s_nNesting; // level of nested zones.

    long long m_nCreateTime;
    bool m_bConsole;
    void stopConsole();
    int m_nPid;
    int m_nNesting;

    void addRecording();

public:
    /**
     * Starts measuring the cost of a C++ scope.
     *
     * Note that the char pointer is stored as such in the ProfileZone object and used in the
     * destructor, so be sure to pass a pointer that stays valid for the duration of the object's
     * lifetime.
     *
     * The second parameter can be used for ad-hoc local measuring by adding a single line of code
     * at a C++ scope start. Example:
     *
     * comphelper::ProfileZone aZone("RtfFilter::filter", true);
     *
     * Similar to the DEBUG macro in sal/log.hxx, don't forget to remove these lines before
     * committing.
     */
    ProfileZone(const char* sName, bool bConsole = false)
        : NamedEvent(sName)
        , m_bConsole(bConsole)
    {
        if (s_bRecording || m_bConsole)
        {
            TimeValue systemTime;
            osl_getSystemTime(&systemTime);
            m_nCreateTime
                = static_cast<long long>(systemTime.Seconds) * 1000000 + systemTime.Nanosec / 1000;

            oslProcessInfo aProcessInfo;
            aProcessInfo.Size = sizeof(oslProcessInfo);
            if (osl_getProcessInfo(nullptr, osl_Process_IDENTIFIER, &aProcessInfo)
                == osl_Process_E_None)
                m_nPid = aProcessInfo.Ident;

            m_nNesting = s_nNesting++;
        }
        else
            m_nCreateTime = 0;
    }

    ~ProfileZone()
    {
        if (m_nCreateTime > 0)
        {
            s_nNesting--;
            assert(m_nNesting == s_nNesting);

            if (s_bRecording)
                addRecording();

            if (m_bConsole)
                stopConsole();
        }
    }

    ProfileZone(const ProfileZone&) = delete;
    void operator=(const ProfileZone&) = delete;
};

} // namespace comphelper

#endif // INCLUDED_COMPHELPER_PROFILEZONE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
