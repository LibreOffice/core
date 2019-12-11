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

#ifndef INCLUDED_DRAWINGLAYER_SOURCE_TOOLS_EMFPENUMS_HXX
#define INCLUDED_DRAWINGLAYER_SOURCE_TOOLS_EMFPENUMS_HXX

#include <rtl/ustring.hxx>

namespace emfplushelper
{
    const char* emfTypeToName(sal_uInt16 type);
    OUString emfObjectToName(sal_uInt16 type);
    OUString PixelOffsetModeToString(sal_uInt16 nPixelOffset);
    OUString SmoothingModeToString(sal_uInt16 nSmoothMode);
    OUString TextRenderingHintToString(sal_uInt16 nHint);
    OUString InterpolationModeToString(sal_uInt16 nMode);
    OUString UnitTypeToString(sal_uInt16 nType);
    bool IsBrush(sal_uInt16 flags);
    OUString BrushIDToString(sal_uInt16 flags, sal_uInt32 brushid);
    OUString BrushDataFlagsToString(sal_uInt32 brush);
    OUString WrapModeToString(sal_uInt32 mode);

    // EMF+ commands
    #define EmfPlusRecordTypeHeader 0x4001
    #define EmfPlusRecordTypeEndOfFile 0x4002
    #define EmfPlusRecordTypeComment 0x4003
    #define EmfPlusRecordTypeGetDC 0x4004
    //TODO EmfPlusRecordTypeMultiFormatStart 0x4005
    //TODO EmfPlusRecordTypeMultiFormatSection 0x4006
    //TODO EmfPlusRecordTypeMultiFormatEnd 0x4007
    #define EmfPlusRecordTypeObject 0x4008
    //TODO EmfPlusRecordTypeClear 0x4009
    #define EmfPlusRecordTypeFillRects 0x400A
    #define EmfPlusRecordTypeDrawRects 0x400B
    #define EmfPlusRecordTypeFillPolygon 0x400C
    #define EmfPlusRecordTypeDrawLines 0x400D
    #define EmfPlusRecordTypeFillEllipse 0x400E
    #define EmfPlusRecordTypeDrawEllipse 0x400F
    #define EmfPlusRecordTypeFillPie 0x4010
    #define EmfPlusRecordTypeDrawPie 0x4011
    #define EmfPlusRecordTypeDrawArc 0x4012
    #define EmfPlusRecordTypeFillRegion 0x4013
    #define EmfPlusRecordTypeFillPath 0x4014
    #define EmfPlusRecordTypeDrawPath 0x4015
    //TODO EmfPlusRecordTypeFillClosedCurve 0x4016
    //TODO EmfPlusRecordTypeDrawClosedCurve 0x4017
    //TODO EmfPlusRecordTypeDrawCurve 0x4018
    #define EmfPlusRecordTypeDrawBeziers 0x4019
    #define EmfPlusRecordTypeDrawImage 0x401A
    #define EmfPlusRecordTypeDrawImagePoints 0x401B
    #define EmfPlusRecordTypeDrawString 0x401C
    #define EmfPlusRecordTypeSetRenderingOrigin 0x401D
    #define EmfPlusRecordTypeSetAntiAliasMode 0x401E
    #define EmfPlusRecordTypeSetTextRenderingHint 0x401F
    #define EmfPlusRecordTypeSetTextContrast 0x4020
    #define EmfPlusRecordTypeSetInterpolationMode 0x4021
    #define EmfPlusRecordTypeSetPixelOffsetMode 0x4022
    //TODO EmfPlusRecordTypeSetCompositingMode 0x4023
    #define EmfPlusRecordTypeSetCompositingQuality 0x4024
    #define EmfPlusRecordTypeSave 0x4025
    #define EmfPlusRecordTypeRestore 0x4026
    //TODO EmfPlusRecordTypeBeginContainer 0x4027
    #define EmfPlusRecordTypeBeginContainerNoParams 0x4028
    #define EmfPlusRecordTypeEndContainer 0x4029
    #define EmfPlusRecordTypeSetWorldTransform 0x402A
    #define EmfPlusRecordTypeResetWorldTransform 0x402B
    #define EmfPlusRecordTypeMultiplyWorldTransform 0x402C
    #define EmfPlusRecordTypeTranslateWorldTransform 0x402D
    #define EmfPlusRecordTypeScaleWorldTransform 0x402E
    #define EmfPlusRecordTypeRotateWorldTransform 0x402F
    #define EmfPlusRecordTypeSetPageTransform 0x4030
    #define EmfPlusRecordTypeResetClip 0x4031
    #define EmfPlusRecordTypeSetClipRect 0x4032
    #define EmfPlusRecordTypeSetClipPath 0x4033
    #define EmfPlusRecordTypeSetClipRegion 0x4034
    #define EmfPlusRecordTypeOffsetClip 0x4035
    #define EmfPlusRecordTypeDrawDriverString 0x4036
    //TODO EmfPlusRecordTypeStrokeFillPath 0x4037
    //TODO EmfPlusRecordTypeSerializableObject 0x4038
    //TODO EmfPlusRecordTypeSetTSGraphics 0x4039
    //TODO EmfPlusRecordTypeSetTSClip 0x403A

    // EMF+object types
    #define EmfPlusObjectTypeBrush 0x100
    #define EmfPlusObjectTypePen 0x200
    #define EmfPlusObjectTypePath 0x300
    #define EmfPlusObjectTypeRegion 0x400
    #define EmfPlusObjectTypeImage 0x500
    #define EmfPlusObjectTypeFont 0x600
    #define EmfPlusObjectTypeStringFormat 0x700
    #define EmfPlusObjectTypeImageAttributes 0x800
    #define EmfPlusObjectTypeCustomLineCap 0x900

    enum PixelOffsetMode
    {
        PixelOffsetModeDefault = 0x00,
        PixelOffsetModeHighSpeed = 0x01,
        PixelOffsetModeHighQuality = 0x02,
        PixelOffsetModeNone = 0x03,
        PixelOffsetModeHalf = 0x04
    };

    enum SmoothingMode
    {
        SmoothingModeDefault = 0x00,
        SmoothingModeHighSpeed = 0x01,
        SmoothingModeHighQuality = 0x02,
        SmoothingModeNone = 0x03,
        SmoothingModeAntiAlias8x4 = 0x04,
        SmoothingModeAntiAlias8x8 = 0x05
    };

    enum TextRenderingHint
    {
        TextRenderingHintSystemDefault = 0x00,
        TextRenderingHintSingleBitPerPixelGridFit = 0x01,
        TextRenderingHintSingleBitPerPixel = 0x02,
        TextRenderingHintAntialiasGridFit = 0x03,
        TextRenderingHintAntialias = 0x04,
        TextRenderingHintClearTypeGridFit = 0x05
    };

    enum InterpolationMode
    {
        InterpolationModeDefault = 0x00,
        InterpolationModeLowQuality = 0x01,
        InterpolationModeHighQuality = 0x02,
        InterpolationModeBilinear = 0x03,
        InterpolationModeBicubic = 0x04,
        InterpolationModeNearestNeighbor = 0x05,
        InterpolationModeHighQualityBilinear = 0x06,
        InterpolationModeHighQualityBicubic = 0x07
    };

    enum UnitType
    {
        UnitTypeWorld = 0x00,
        UnitTypeDisplay = 0x01,
        UnitTypePixel = 0x02,
        UnitTypePoint = 0x03,
        UnitTypeInch = 0x04,
        UnitTypeDocument = 0x05,
        UnitTypeMillimeter = 0x06
    };

    enum EmfPlusCombineMode
    {
        EmfPlusCombineModeReplace = 0x00000000,
        EmfPlusCombineModeIntersect = 0x00000001,
        EmfPlusCombineModeUnion = 0x00000002,
        EmfPlusCombineModeXOR = 0x00000003,
        EmfPlusCombineModeExclude = 0x00000004,
        EmfPlusCombineModeComplement = 0x00000005
    };

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

    const sal_uInt32 BrushDataPath = 0x00000001;
    const sal_uInt32 BrushDataTransform = 0x00000002;
    const sal_uInt32 BrushDataPresetColors = 0x00000004;
    const sal_uInt32 BrushDataBlendFactorsH = 0x00000008;
    const sal_uInt32 BrushDataBlendFactorsV = 0x00000010;
    const sal_uInt32 BrushDataFocusScales = 0x00000040;
    const sal_uInt32 BrushDataGammaCorrected = 0x00000080;
    const sal_uInt32 BrushDataDoNotTransform = 0x00000100;

    enum WrapMode
    {
        WrapModeTile = 0x00000000,
        WrapModeTileFlipX = 0x00000001,
        WrapModeTileFlipY = 0x00000002,
        WrapModeTileFlipXY = 0x00000003,
        WrapModeClamp = 0x00000004
    };

    enum ImageDataType
    {
        ImageDataTypeUnknown = 0x00000000,
        ImageDataTypeBitmap = 0x00000001,
        ImageDataTypeMetafile = 0x00000002
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
