/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#pragma once

#include <sal/types.h>
#include <tools/long.hxx>

#include <vcl/dllapi.h>
#include <vcl/mapmod.hxx>

class CoordinateMapper
{
private:
    bool mbMap = false;
    MapMode maMapMode;

    sal_Int32 mnDPIX = 0;
    sal_Int32 mnDPIY = 0;
    sal_Int32 mnDPIScalePercentage = 100;

    /// Output offset for device output in pixel (pseudo window offset within window system's frames)
    tools::Long mnOutOffX;
    /// Output offset for device output in pixel (pseudo window offset within window system's frames)
    tools::Long mnOutOffY;

    tools::Long mnOutWidth;
    tools::Long mnOutHeight;

public:
    bool IsMapModeEnabled() const { return mbMap; }
    void EnableMapMode(bool bEnable = true) { mbMap = bEnable; }

    sal_Int32 GetDPIX() const;
    sal_Int32 GetDPIY() const;

    void SetDPIX(sal_Int32 nDPIX);
    void SetDPIY(sal_Int32 nDPIY);

    sal_Int32 GetDPIScalePercentage() const;
    void SetDPIScalePercentage(sal_Int32 nPercentage);

    tools::Long GetOutOffXPixel() const;
    tools::Long GetOutOffYPixel() const;

    void SetOutOffXPixel(tools::Long nOutOffX);
    void SetOutOffYPixel(tools::Long nOutOffY);

    Point GetOutputOffPixel() const;

    tools::Long GetOutputWidthPixel() const;
    tools::Long GetOutputHeightPixel() const;
    Size GetOutputSizePixel() const;

    void SetOutputWidthPixel(tools::Long nWidth);
    void SetOutputHeightPixel(tools::Long nHeight);

    const MapMode& GetMapMode() const { return maMapMode; }
    bool IsDefaultMapMode() const { return maMapMode.IsDefault(); }
    void ResetMapMode() { maMapMode = MapMode(); }
    void ResetMapMode(const MapMode& rMapMode) { maMapMode = rMapMode; }
    MapUnit GetMapUnit() const { return maMapMode.GetMapUnit(); }

    double GetScaleX() const { return maMapMode.GetScaleX(); }
    double GetScaleY() const { return maMapMode.GetScaleY(); }
    void SetScaleX(double nX) { maMapMode.SetScaleX(nX); }
    void SetScaleY(double nY) { maMapMode.SetScaleY(nY); }

    void SetOrigin(const Point& rPt) { maMapMode.SetOrigin(rPt); }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
