/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "DrawViewShell.hxx"
#include "sdmod.hxx"

#include <comphelper/lok.hxx>

namespace sd {

void DrawViewShell::ConfigurationChanged( utl::ConfigurationBroadcaster* pCb, ConfigurationHints )
{
    ConfigureAppBackgroundColor( dynamic_cast<svtools::ColorConfig*>(pCb) );
}

void DrawViewShell::ConfigureAppBackgroundColor( svtools::ColorConfig *pColorConfig )
{
    if (!pColorConfig)
        pColorConfig = &SD_MOD()->GetColorConfig();
    Color aFillColor( pColorConfig->GetColorValue( svtools::APPBACKGROUND ).nColor );
    if (comphelper::LibreOfficeKit::isActive())
        aFillColor = COL_TRANSPARENT;
    // tdf#87905 Use darker background color for master view
    if (meEditMode == EditMode::MasterPage)
        aFillColor.DecreaseLuminance( 64 );
    mnAppBackgroundColor = aFillColor;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
