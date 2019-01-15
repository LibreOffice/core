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

#ifndef INCLUDED_DRAWINGLAYER_SOURCE_TOOLS_EMFPBRUSH_HXX
#define INCLUDED_DRAWINGLAYER_SOURCE_TOOLS_EMFPBRUSH_HXX

#include "emfphelperdata.hxx"
#include <tools/color.hxx>

namespace emfplushelper
{
    enum EmfPlusHatchStyle
    {
        HatchStyleHorizontal = 0x00000000,
        HatchStyleVertical = 0x00000001,
        HatchStyleForwardDiagonal = 0x00000002,
        HatchStyleBackwardDiagonal = 0x00000003,
        HatchStyleLargeGrid = 0x00000004,
        HatchStyleDiagonalCross = 0x00000005,
        HatchStyle05Percent = 0x00000006,
        HatchStyle10Percent = 0x00000007,
        HatchStyle20Percent = 0x00000008,
        HatchStyle25Percent = 0x00000009,
        HatchStyle30Percent = 0x0000000A,
        HatchStyle40Percent = 0x0000000B,
        HatchStyle50Percent = 0x0000000C,
        HatchStyle60Percent = 0x0000000D,
        HatchStyle70Percent = 0x0000000E,
        HatchStyle75Percent = 0x0000000F,
        HatchStyle80Percent = 0x00000010,
        HatchStyle90Percent = 0x00000011,
        HatchStyleLightDownwardDiagonal = 0x00000012,
        HatchStyleLightUpwardDiagonal = 0x00000013,
        HatchStyleDarkDownwardDiagonal = 0x00000014,
        HatchStyleDarkUpwardDiagonal = 0x00000015,
        HatchStyleWideDownwardDiagonal = 0x00000016,
        HatchStyleWideUpwardDiagonal = 0x00000017,
        HatchStyleLightVertical = 0x00000018,
        HatchStyleLightHorizontal = 0x00000019,
        HatchStyleNarrowVertical = 0x0000001A,
        HatchStyleNarrowHorizontal = 0x0000001B,
        HatchStyleDarkVertical = 0x0000001C,
        HatchStyleDarkHorizontal = 0x0000001D,
        HatchStyleDashedDownwardDiagonal = 0x0000001E,
        HatchStyleDashedUpwardDiagonal = 0x0000001F,
        HatchStyleDashedHorizontal = 0x00000020,
        HatchStyleDashedVertical = 0x00000021,
        HatchStyleSmallConfetti = 0x00000022,
        HatchStyleLargeConfetti = 0x00000023,
        HatchStyleZigZag = 0x00000024,
        HatchStyleWave = 0x00000025,
        HatchStyleDiagonalBrick = 0x00000026,
        HatchStyleHorizontalBrick = 0x00000027,
        HatchStyleWeave = 0x00000028,
        HatchStylePlaid = 0x00000029,
        HatchStyleDivot = 0x0000002A,
        HatchStyleDottedGrid = 0x0000002B,
        HatchStyleDottedDiamond = 0x0000002C,
        HatchStyleShingle = 0x0000002D,
        HatchStyleTrellis = 0x0000002E,
        HatchStyleSphere = 0x0000002F,
        HatchStyleSmallGrid = 0x00000030,
        HatchStyleSmallCheckerBoard = 0x00000031,
        HatchStyleLargeCheckerBoard = 0x00000032,
        HatchStyleOutlinedDiamond = 0x00000033,
        HatchStyleSolidDiamond = 0x00000034
    };

    enum EmfPlusBrushType
    {
        BrushTypeSolidColor = 0x00000000,
        BrushTypeHatchFill = 0x00000001,
        BrushTypeTextureFill = 0x00000002,
        BrushTypePathGradient = 0x00000003,
        BrushTypeLinearGradient = 0x00000004
    };

    struct EMFPPath;

    struct EMFPBrush : public EMFPObject
    {
        ::Color solidColor;
        sal_uInt32 type;
        sal_uInt32 additionalFlags;

        /* linear gradient */
        sal_Int32 wrapMode;
        float areaX, areaY, areaWidth, areaHeight;
        ::Color secondColor; // first color is stored in solidColor;
        basegfx::B2DHomMatrix brush_transformation;
        bool hasTransformation;
        sal_Int32 blendPoints;
        std::unique_ptr<float[]> blendPositions;
        float* blendFactors;
        sal_Int32 colorblendPoints;
        std::unique_ptr<float[]> colorblendPositions;
        std::unique_ptr<::Color[]> colorblendColors;
        sal_Int32 surroundColorsNumber;
        std::unique_ptr<::Color[]> surroundColors;
        std::unique_ptr<EMFPPath> path;
        EmfPlusHatchStyle hatchStyle;

        EMFPBrush();
        virtual ~EMFPBrush() override;

        sal_uInt32 GetType() const { return type; }
        const ::Color& GetColor() const { return solidColor; }

        void Read(SvStream& s, EmfPlusHelperData const & rR);
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
