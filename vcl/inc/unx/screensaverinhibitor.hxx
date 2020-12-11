/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_UNX_SCREENSAVERINHIBITOR_HXX
#define INCLUDED_VCL_INC_UNX_SCREENSAVERINHIBITOR_HXX

#include <X11/Xlib.h>
#include <X11/Xmd.h>

#include <rtl/ustring.hxx>
#include <vcl/dllapi.h>

#include <optional>
#include <string_view>

class VCL_PLUGIN_PUBLIC ScreenSaverInhibitor
{
public:
    void inhibit(bool bInhibit, std::u16string_view sReason, bool bIsX11,
                 const std::optional<unsigned int>& xid, std::optional<Display*> pDisplay);

private:
    // These are all used as guint, however this header may be included
    // in kde/tde/etc backends, where we would ideally avoid having
    // any glib dependencies, hence the direct use of unsigned int.
    std::optional<unsigned int> mnFDOCookie; // FDO ScreenSaver Inhibit
    std::optional<unsigned int> mnFDOPMCookie; // FDO PowerManagement Inhibit
    std::optional<unsigned int> mnGSMCookie;
    std::optional<unsigned int> mnMSMCookie;

    std::optional<int> mnXScreenSaverTimeout;

#if !defined(__sun) && !defined(AIX)
    BOOL mbDPMSWasEnabled;
    CARD16 mnDPMSStandbyTimeout;
    CARD16 mnDPMSSuspendTimeout;
    CARD16 mnDPMSOffTimeout;
#endif

    // There are a bunch of different dbus based inhibition APIs. Some call
    // themselves ScreenSaver inhibition, some are PowerManagement inhibition,
    // but they appear to have the same effect. There doesn't appear to be one
    // all encompassing standard, hence we should just try all of them.
    //
    // The current APIs we have: (note: the list of supported environments is incomplete)
    // FDO: org.freedesktop.ScreenSaver::Inhibit - appears to be supported only by KDE?
    // FDOPM: org.freedesktop.PowerManagement.Inhibit::Inhibit - XFCE, (KDE) ?
    //        (KDE: doesn't inhibit screensaver, but does inhibit PowerManagement)
    // GSM: org.gnome.SessionManager::Inhibit - gnome 3
    // MSM: org.mate.Sessionmanager::Inhibit - Mate <= 1.10, is identical to GSM
    //       (This is replaced by the GSM interface from Mate 1.12 onwards)
    //
    // Note: the Uninhibit call has different spelling in FDO (UnInhibit) vs GSM (Uninhibit)
    void inhibitFDO(bool bInhibit, const char* appname, const char* reason);
    void inhibitFDOPM(bool bInhibit, const char* appname, const char* reason);
    void inhibitGSM(bool bInhibit, const char* appname, const char* reason, const unsigned int xid);
    void inhibitMSM(bool bInhibit, const char* appname, const char* reason, const unsigned int xid);

    void inhibitXScreenSaver(bool bInhibit, Display* pDisplay);
    static void inhibitXAutoLock(bool bInhibit, Display* pDisplay);
    void inhibitDPMS(bool bInhibit, Display* pDisplay);
};

#endif // INCLUDED_VCL_INC_UNX_SCREENSAVERINHIBITOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
