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

#include <osl/process.h>
#include <osl/time.h>
#include <com/sun/star/uno/Sequence.h>
#include <comphelper/comphelperdllapi.h>
#include <rtl/ustring.hxx>

// implementation of XToolkitExperimental profiling API

namespace comphelper
{
class COMPHELPER_DLLPUBLIC TraceEvent
{
protected:
    static std::atomic<bool> s_bRecording; // true during recording
    static int s_nNesting;

    static void addRecording(const OUString& sObject);

public:
    static void addInstantEvent(const char* sProfileId);
};

} // namespace comphelper

#endif // INCLUDED_COMPHELPER_TRACEEVENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
