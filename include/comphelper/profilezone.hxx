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

#include <atomic>

#include <com/sun/star/uno/Sequence.h>
#include <comphelper/comphelperdllapi.h>
#include <rtl/ustring.hxx>

// implementation of XToolkitExperimental profiling API

namespace comphelper
{
namespace ProfileRecording
{
COMPHELPER_DLLPUBLIC void startRecording();
COMPHELPER_DLLPUBLIC void stopRecording();

COMPHELPER_DLLPUBLIC long long addRecording(const char* aProfileId, long long aCreateTime);

COMPHELPER_DLLPUBLIC css::uno::Sequence<OUString> getRecordingAndClear();

} // namespace ProfileRecording

class COMPHELPER_DLLPUBLIC ProfileZone
{
private:
    const char* m_sProfileId;
    long long m_aCreateTime;
    bool m_bConsole;
    void startConsole();
    void stopConsole();

public:
    static std::atomic<bool> g_bRecording; // true during recording

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
    ProfileZone(const char* sProfileId, bool bConsole = false)
        : m_sProfileId(sProfileId)
        , m_aCreateTime(g_bRecording ? ProfileRecording::addRecording(sProfileId, 0) : 0)
        , m_bConsole(bConsole)
    {
        if (m_bConsole)
        {
            startConsole();
        }
    }
    ~ProfileZone()
    {
        if (g_bRecording)
            ProfileRecording::addRecording(m_sProfileId, m_aCreateTime);
        if (m_bConsole)
        {
            stopConsole();
        }
    }
};

} // namespace comphelper

#endif // INCLUDED_COMPHELPER_PROFILEZONE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
