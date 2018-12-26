/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <smmod.hxx>
#include "tmpdevice.hxx"

#include <svtools/colorcfg.hxx>
#include <vcl/window.hxx>
#include <sal/log.hxx>

// SmTmpDevice
// Allows for font and color changes. The original settings will be restored
// in the destructor.
// It's main purpose is to allow for the "const" in the 'OutputDevice'
// argument in the 'Arrange' functions and restore changes made in the 'Draw'
// functions.
// Usually a MapMode of 1/100th mm will be used.

SmTmpDevice::SmTmpDevice(OutputDevice &rTheDev, bool bUseMap100th_mm) :
    rOutDev(rTheDev)
{
    rOutDev.Push( PushFlags::FONT | PushFlags::MAPMODE |
                  PushFlags::LINECOLOR | PushFlags::FILLCOLOR | PushFlags::TEXTCOLOR );
    if (bUseMap100th_mm  &&  MapUnit::Map100thMM != rOutDev.GetMapMode().GetMapUnit())
    {
        SAL_WARN("starmath", "incorrect MapMode?");
        rOutDev.SetMapMode(MapMode(MapUnit::Map100thMM)); // format for 100% always
    }
}


Color SmTmpDevice::Impl_GetColor( const Color& rColor )
{
    Color nNewCol = rColor;
    if (nNewCol == COL_AUTO)
    {
        if (OUTDEV_PRINTER == rOutDev.GetOutDevType())
            nNewCol = COL_BLACK;
        else
        {
            Color aBgCol( rOutDev.GetBackground().GetColor() );
            if (OUTDEV_WINDOW == rOutDev.GetOutDevType())
                aBgCol = static_cast<vcl::Window &>(rOutDev).GetDisplayBackground().GetColor();

            nNewCol = SM_MOD()->GetColorConfig().GetColorValue(svtools::FONTCOLOR).nColor;

            Color aTmpColor( nNewCol );
            if (aBgCol.IsDark() && aTmpColor.IsDark())
                nNewCol = COL_WHITE;
            else if (aBgCol.IsBright() && aTmpColor.IsBright())
                nNewCol = COL_BLACK;
        }
    }
    return nNewCol;
}


void SmTmpDevice::SetFont(const vcl::Font &rNewFont)
{
    rOutDev.SetFont( rNewFont );
    rOutDev.SetTextColor( Impl_GetColor( rNewFont.GetColor() ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
