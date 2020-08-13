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

#pragma once

#include <tools/degree.hxx>
#include <vcl/dllapi.h>
#include <vcl/bitmap.hxx>
#include <basegfx/bitmap/BitmapAttributes.hxx>

enum class GraphicDrawMode
{
    Standard = 0,
    Greys = 1,
    Mono = 2,
    Watermark = 3
};

class VCL_DLLPUBLIC GraphicAttr
{
private:
    double mfGamma;
    basegfx::MirrorDirectionFlags mnMirrFlags;
    tools::Long mnLeftCrop;
    tools::Long mnTopCrop;
    tools::Long mnRightCrop;
    tools::Long mnBottomCrop;
    Degree10 mnRotate10;
    short mnContPercent;
    short mnLumPercent;
    short mnRPercent;
    short mnGPercent;
    short mnBPercent;
    bool mbInvert;
    sal_uInt8 mcAlpha;
    GraphicDrawMode meDrawMode;

public:
    GraphicAttr()
        : mfGamma(1.0)
        , mnMirrFlags(basegfx::MirrorDirectionFlags::NONE)
        , mnLeftCrop(0)
        , mnTopCrop(0)
        , mnRightCrop(0)
        , mnBottomCrop(0)
        , mnRotate10(0)
        , mnContPercent(0)
        , mnLumPercent(0)
        , mnRPercent(0)
        , mnGPercent(0)
        , mnBPercent(0)
        , mbInvert(false)
        , mcAlpha(255)
        , meDrawMode(GraphicDrawMode::Standard)
    {
    }

    bool operator==(const GraphicAttr& rAttr) const
    {
        return mfGamma == rAttr.mfGamma && mnMirrFlags == rAttr.mnMirrFlags
               && mnLeftCrop == rAttr.mnLeftCrop && mnTopCrop == rAttr.mnTopCrop
               && mnRightCrop == rAttr.mnRightCrop && mnBottomCrop == rAttr.mnBottomCrop
               && mnRotate10 == rAttr.mnRotate10 && mnContPercent == rAttr.mnContPercent
               && mnLumPercent == rAttr.mnLumPercent && mnRPercent == rAttr.mnRPercent
               && mnGPercent == rAttr.mnGPercent && mnBPercent == rAttr.mnBPercent
               && mbInvert == rAttr.mbInvert && mcAlpha == rAttr.mcAlpha
               && meDrawMode == rAttr.meDrawMode;
    }

    bool operator!=(const GraphicAttr& rAttr) const { return !(*this == rAttr); }

    void SetDrawMode(GraphicDrawMode eDrawMode) { meDrawMode = eDrawMode; }
    GraphicDrawMode GetDrawMode() const { return meDrawMode; }

    void SetMirrorFlags(basegfx::MirrorDirectionFlags nMirrFlags) { mnMirrFlags = nMirrFlags; }
    basegfx::MirrorDirectionFlags GetMirrorFlags() const { return mnMirrFlags; }

    void SetCrop(tools::Long nLeft_100TH_MM, tools::Long nTop_100TH_MM, tools::Long nRight_100TH_MM,
                 tools::Long nBottom_100TH_MM)
    {
        mnLeftCrop = nLeft_100TH_MM;
        mnTopCrop = nTop_100TH_MM;
        mnRightCrop = nRight_100TH_MM;
        mnBottomCrop = nBottom_100TH_MM;
    }
    tools::Long GetLeftCrop() const { return mnLeftCrop; }
    tools::Long GetTopCrop() const { return mnTopCrop; }
    tools::Long GetRightCrop() const { return mnRightCrop; }
    tools::Long GetBottomCrop() const { return mnBottomCrop; }

    void SetRotation(Degree10 nRotate10) { mnRotate10 = nRotate10; }
    Degree10 GetRotation() const { return mnRotate10; }

    void SetLuminance(short nLuminancePercent) { mnLumPercent = nLuminancePercent; }
    short GetLuminance() const { return mnLumPercent; }

    void SetContrast(short nContrastPercent) { mnContPercent = nContrastPercent; }
    short GetContrast() const { return mnContPercent; }

    void SetChannelR(short nChannelRPercent) { mnRPercent = nChannelRPercent; }
    short GetChannelR() const { return mnRPercent; }

    void SetChannelG(short nChannelGPercent) { mnGPercent = nChannelGPercent; }
    short GetChannelG() const { return mnGPercent; }

    void SetChannelB(short nChannelBPercent) { mnBPercent = nChannelBPercent; }
    short GetChannelB() const { return mnBPercent; }

    void SetGamma(double fGamma) { mfGamma = fGamma; }
    double GetGamma() const { return mfGamma; }

    void SetInvert(bool bInvert) { mbInvert = bInvert; }
    bool IsInvert() const { return mbInvert; }

    void SetAlpha(sal_uInt8 cAlpha) { mcAlpha = cAlpha; }
    sal_uInt8 GetAlpha() const { return mcAlpha; }

    bool IsSpecialDrawMode() const { return (meDrawMode != GraphicDrawMode::Standard); }
    bool IsMirrored() const { return mnMirrFlags != basegfx::MirrorDirectionFlags::NONE; }
    bool IsCropped() const
    {
        return (mnLeftCrop != 0 || mnTopCrop != 0 || mnRightCrop != 0 || mnBottomCrop != 0);
    }
    bool IsRotated() const { return ((mnRotate10 % 3600_deg10) != 0_deg10); }
    bool IsTransparent() const { return (mcAlpha < 255); }
    bool IsAdjusted() const
    {
        return (mnLumPercent != 0 || mnContPercent != 0 || mnRPercent != 0 || mnGPercent != 0
                || mnBPercent != 0 || mfGamma != 1.0 || mbInvert);
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
