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

#include <sal/types.h>
#include <sal/log.hxx>

#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/drawables/PixelDrawable.hxx>

#include <outdata.hxx>
#include <salgdi.hxx>

#include <memory>

Color OutputDevice::GetPixel(Point const& rPoint) const
{
    Color aColor;

    if (mpGraphics || AcquireGraphics())
    {
        if (mbInitClipRegion)
            const_cast<OutputDevice*>(this)->InitClipRegion();

        if (!mbOutputClipped)
        {
            const long nX = ImplLogicXToDevicePixel(rPoint.X());
            const long nY = ImplLogicYToDevicePixel(rPoint.Y());
            aColor = mpGraphics->GetPixel(nX, nY, this);

            if (mpAlphaVDev)
            {
                Color aAlphaColor = mpAlphaVDev->GetPixel(rPoint);
                aColor.SetTransparency(aAlphaColor.GetBlue());
            }
        }
    }
    return aColor;
}

void OutputDevice::DrawPixel(Point const& rPt) { Draw(vcl::PixelDrawable(rPt)); }

void OutputDevice::DrawPixel(Point const& rPt, Color const& rColor) { Draw(vcl::PixelDrawable(rPt, rColor)); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
