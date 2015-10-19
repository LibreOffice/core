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

#include <prex.h>
#include <postx.h>

#include <rtl/ustring.hxx>
#include <vcl/dllapi.h>

#include <boost/optional.hpp>

#include <glib.h>

class VCL_PLUGIN_PUBLIC ScreenSaverInhibitor
{
public:
    void inhibit( bool bInhibit, const rtl::OUString& sReason,
                  bool bIsX11, const boost::optional<guint> xid, boost::optional<Display*> pDisplay );

private:
    boost::optional<guint> mnFDOCookie;
    boost::optional<guint> mnGSMCookie;

    boost::optional<int> mnXScreenSaverTimeout;

    // Note: the Uninhibit call has different spelling in FDO (UnInhibit) vs GSM (Uninhibit)
    void inhibitFDO( bool bInhibit, const gchar* appname, const gchar* reason );
    void inhibitGSM( bool bInhibit, const gchar* appname, const gchar* reason, const guint xid );

    void inhibitXScreenSaver( bool bInhibit, Display* pDisplay );
    static void inhibitXAutoLock( bool bInhibit, Display* pDisplay );
};

#endif // INCLUDED_VCL_INC_UNX_SCREENSAVERINHIBITOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
