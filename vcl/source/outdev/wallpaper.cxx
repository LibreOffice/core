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

#include <cassert>

#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/drawables/WallpaperDrawable.hxx>

Color OutputDevice::GetReadableFontColor(const Color& rFontColor, const Color& rBgColor) const
{
    if (rBgColor.IsDark() && rFontColor.IsDark())
        return COL_WHITE;
    else if (rBgColor.IsBright() && rFontColor.IsBright())
        return COL_BLACK;
    else
        return rFontColor;
}

Color OutputDevice::GetBackgroundColor() const { return GetBackground().GetColor(); }

void OutputDevice::DrawWallpaper(const tools::Rectangle& rRect, const Wallpaper& rWallpaper)
{
    Draw(vcl::WallpaperDrawable(rRect, rWallpaper));
}

void OutputDevice::Erase()
{
    if (!IsDeviceOutputNecessary() || ImplIsRecordLayout())
        return;

    if (mbBackground)
    {
        RasterOp eRasterOp = GetRasterOp();
        if (eRasterOp != RasterOp::OverPaint)
            SetRasterOp(RasterOp::OverPaint);
        Draw(vcl::WallpaperDrawable(tools::Rectangle(0, 0, mnOutWidth - 1, mnOutHeight - 1), maBackground,
                               false));
        if (eRasterOp != RasterOp::OverPaint)
            SetRasterOp(eRasterOp);
    }

    if (mpAlphaVDev)
        mpAlphaVDev->Erase();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
