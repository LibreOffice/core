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

#include <sal/log.hxx>

#include <comphelper/traceevent.hxx>

// implementation of XToolkitExperimental profiling API

namespace comphelper
{

class COMPHELPER_DLLPUBLIC ProfileZone : public NamedEvent
{
    long long m_nCreateTime;
    int m_nNesting;

    void addRecording();
    void setNestingLevel(int nNestingLevel);
    int getNestingLevel();

    ProfileZone(const char* sName, const OUString &sArgs)
        : NamedEvent(sName, sArgs)
        , m_nNesting(-1)
    {
        if (s_bRecording)
        {
            m_nCreateTime = getNow();

            m_nNesting = getNestingLevel();
            setNestingLevel(getNestingLevel() + 1);
        }
        else
            m_nCreateTime = 0;
    }

 public:

    // Note that the char pointer is stored as such in the ProfileZone object and used in the
    // destructor, so be sure to pass a pointer that stays valid for the duration of the object's
    // lifetime.
    ProfileZone(const char* sName, const std::map<OUString, OUString> &aArgs)
        : ProfileZone(sName, createArgsString(aArgs))
    {
    }

    ProfileZone(const char* sName)
        : ProfileZone(sName, OUString())
    {
    }

    ~ProfileZone()
    {
        if (m_nCreateTime > 0)
        {
            setNestingLevel(getNestingLevel() - 1);

            if (m_nNesting != getNestingLevel())
            {
                SAL_WARN("comphelper.traceevent", "Incorrect ProfileZone nesting for " << m_sName);
            }
            else
            {
                if (s_bRecording)
                    addRecording();
            }
        }
    }

    ProfileZone(const ProfileZone&) = delete;
    void operator=(const ProfileZone&) = delete;
};

} // namespace comphelper


#endif // INCLUDED_COMPHELPER_PROFILEZONE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
