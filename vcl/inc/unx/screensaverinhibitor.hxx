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

#include <rtl/ustring.hxx>
#include <vcl/dllapi.h>

#include <boost/optional.hpp>

#include <glib.h>

class VCL_PLUGIN_PUBLIC ScreenSaverInhibitor
{
public:
    void inhibit( bool bInhibit, bool bIsX11, const rtl::OUString& sReason );

private:
    boost::optional<guint> mnFDOCookie;

    void inhibitFDO( bool bInhibit, const gchar* appname, const gchar* reason );
};

#endif // INCLUDED_VCL_INC_UNX_SCREENSAVERINHIBITOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
