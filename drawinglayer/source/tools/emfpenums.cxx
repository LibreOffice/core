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

#include <sal/log.hxx>
#include "emfpenums.hxx"

namespace emfplushelper
{
    OUString EMFPlusTypeToString(sal_uInt16 type)
    {
        switch (type)
        {
            case EmfPlusRecordTypeHeader: return "EmfPlusRecordTypeHeader";
            case EmfPlusRecordTypeEndOfFile: return "EmfPlusRecordTypeEndOfFile";
            case EmfPlusRecordTypeComment: return "EmfPlusRecordTypeComment";
            case EmfPlusRecordTypeGetDC: return "EmfPlusRecordTypeGetDC";
            case EmfPlusRecordTypeObject: return "EmfPlusRecordTypeObject";
            case EmfPlusRecordTypeFillRects: return "EmfPlusRecordTypeFillRects";
            case EmfPlusRecordTypeDrawRects: return "EmfPlusRecordTypeDrawRects";
            case EmfPlusRecordTypeFillPolygon: return "EmfPlusRecordTypeFillPolygon";
            case EmfPlusRecordTypeDrawLines: return "EmfPlusRecordTypeDrawLines";
            case EmfPlusRecordTypeFillEllipse: return "EmfPlusRecordTypeFillEllipse";
            case EmfPlusRecordTypeDrawEllipse: return "EmfPlusRecordTypeDrawEllipse";
            case EmfPlusRecordTypeFillPie: return "EmfPlusRecordTypeFillPie";
            case EmfPlusRecordTypeDrawPie: return "EmfPlusRecordTypeDrawPie";
            case EmfPlusRecordTypeDrawArc: return "EmfPlusRecordTypeDrawArc";
            case EmfPlusRecordTypeFillRegion: return "EmfPlusRecordTypeFillRegion";
            case EmfPlusRecordTypeFillPath: return "EmfPlusRecordTypeFillPath";
            case EmfPlusRecordTypeDrawPath: return "EmfPlusRecordTypeDrawPath";
            case EmfPlusRecordTypeDrawBeziers: return "EmfPlusRecordTypeDrawBeziers";
            case EmfPlusRecordTypeDrawImage: return "EmfPlusRecordTypeDrawImage";
            case EmfPlusRecordTypeDrawImagePoints: return "EmfPlusRecordTypeDrawImagePoints";
            case EmfPlusRecordTypeDrawString: return "EmfPlusRecordTypeDrawString";
            case EmfPlusRecordTypeSetRenderingOrigin: return "EmfPlusRecordTypeSetRenderingOrigin";
            case EmfPlusRecordTypeSetAntiAliasMode: return "EmfPlusRecordTypeSetAntiAliasMode";
            case EmfPlusRecordTypeSetTextRenderingHint: return "EmfPlusRecordTypeSetTextRenderingHint";
            case EmfPlusRecordTypeSetTextContrast: return "EmfPlusRecordTypeSetTextContrast";
            case EmfPlusRecordTypeSetInterpolationMode: return "EmfPlusRecordTypeSetInterpolationMode";
            case EmfPlusRecordTypeSetPixelOffsetMode: return "EmfPlusRecordTypeSetPixelOffsetMode";
            case EmfPlusRecordTypeSetCompositingQuality: return "EmfPlusRecordTypeSetCompositingQuality";
            case EmfPlusRecordTypeSave: return "EmfPlusRecordTypeSave";
            case EmfPlusRecordTypeRestore: return "EmfPlusRecordTypeRestore";
            case EmfPlusRecordTypeBeginContainerNoParams: return "EmfPlusRecordTypeBeginContainerNoParams";
            case EmfPlusRecordTypeEndContainer: return "EmfPlusRecordTypeEndContainer";
            case EmfPlusRecordTypeSetWorldTransform: return "EmfPlusRecordTypeSetWorldTransform";
            case EmfPlusRecordTypeResetWorldTransform: return "EmfPlusRecordTypeResetWorldTransform";
            case EmfPlusRecordTypeMultiplyWorldTransform: return "EmfPlusRecordTypeMultiplyWorldTransform";
            case EmfPlusRecordTypeTranslateWorldTransform: return "EmfPlusRecordTypeTranslateWorldTransform";
            case EmfPlusRecordTypeScaleWorldTransform: return "EmfPlusRecordTypeScaleWorldTransform";
            case EmfPlusRecordTypeSetPageTransform: return "EmfPlusRecordTypeSetPageTransform";
            case EmfPlusRecordTypeResetClip: return "EmfPlusRecordTypeResetClip";
            case EmfPlusRecordTypeSetClipRect: return "EmfPlusRecordTypeSetClipRect";
            case EmfPlusRecordTypeSetClipPath: return "EmfPlusRecordTypeSetClipPath";
            case EmfPlusRecordTypeSetClipRegion: return "EmfPlusRecordTypeSetClipRegion";
            case EmfPlusRecordTypeOffsetClip: return "EmfPlusRecordTypeOffsetClip";
            case EmfPlusRecordTypeDrawDriverString: return "EmfPlusRecordTypeDrawDriverString";
        }
        return "";
    }

    OUString ObjectTypeToString(sal_uInt16 type)
    {
        switch (type)
        {
            case ObjectType::ObjectTypeBrush: return "ObjectTypeBrush";
            case ObjectType::ObjectTypePen: return "ObjectTypePen";
            case ObjectType::ObjectTypePath: return "ObjectTypePath";
            case ObjectType::ObjectTypeRegion: return "ObjectTypeRegion";
            case ObjectType::ObjectTypeImage: return "ObjectTypeImage";
            case ObjectType::ObjectTypeFont: return "ObjectTypeFont";
            case ObjectType::ObjectTypeStringFormat: return "ObjectTypeStringFormat";
            case ObjectType::ObjectTypeImageAttributes: return "ObjectTypeImageAttributes";
            case ObjectType::ObjectTypeCustomLineCap: return "ObjectTypeCustomLineCap";
        }
        return "";
    }

    OUString PixelOffsetModeToString(sal_uInt16 nPixelOffset)
    {
        switch (nPixelOffset)
        {
            case PixelOffsetMode::PixelOffsetModeDefault: return "PixelOffsetModeDefault";
            case PixelOffsetMode::PixelOffsetModeHighSpeed: return "PixelOffsetModeHighSpeed";
            case PixelOffsetMode::PixelOffsetModeHighQuality: return "PixelOffsetModeHighQuality";
            case PixelOffsetMode::PixelOffsetModeNone: return "PixelOffsetModeNone";
            case PixelOffsetMode::PixelOffsetModeHalf: return "PixelOffsetModeHalf";
        }
        return "";
    }

    OUString SmoothingModeToString(sal_uInt16 nSmoothMode)
    {
        switch (nSmoothMode)
        {
            case SmoothingMode::SmoothingModeDefault: return "SmoothingModeDefault";
            case SmoothingMode::SmoothingModeHighSpeed: return "SmoothModeHighSpeed";
            case SmoothingMode::SmoothingModeHighQuality: return "SmoothingModeHighQuality";
            case SmoothingMode::SmoothingModeNone: return "SmoothingModeNone";
            case SmoothingMode::SmoothingModeAntiAlias8x4: return "SmoothingModeAntiAlias8x4";
            case SmoothingMode::SmoothingModeAntiAlias8x8: return "SmoothingModeAntiAlias8x8";
        }
        return "";
    }

    OUString TextRenderingHintToString(sal_uInt16 nHint)
    {
        switch (nHint)
        {
            case TextRenderingHint::TextRenderingHintSystemDefault: return "TextRenderingHintSystemDefault";
            case TextRenderingHint::TextRenderingHintSingleBitPerPixelGridFit: return "TextRenderingHintSingleBitPerPixelGridFit";
            case TextRenderingHint::TextRenderingHintSingleBitPerPixel: return "TextRenderingHintSingleBitPerPixel";
            case TextRenderingHint::TextRenderingHintAntialiasGridFit: return "TextRenderingHintAntialiasGridFit";
            case TextRenderingHint::TextRenderingHintAntialias: return "TextRenderingHintAntialias";
            case TextRenderingHint::TextRenderingHintClearTypeGridFit: return "TextRenderingHintClearTypeGridFit";
        }
        return "";
    }

    OUString InterpolationModeToString(sal_uInt16 nMode)
    {
        switch (nMode)
        {
            case InterpolationMode::InterpolationModeDefault: return "InterpolationModeDefault";
            case InterpolationMode::InterpolationModeLowQuality: return "InterpolationModeLowQuality";
            case InterpolationMode::InterpolationModeHighQuality: return "InterpolationModeHighQuality";
            case InterpolationMode::InterpolationModeBilinear: return "InterpolationModeBilinear";
            case InterpolationMode::InterpolationModeBicubic: return "InterpolationModeBicubic";
            case InterpolationMode::InterpolationModeNearestNeighbor: return "InterpolationModeNearestNeighbor";
            case InterpolationMode::InterpolationModeHighQualityBilinear: return "InterpolationModeHighQualityBilinear";
            case InterpolationMode::InterpolationModeHighQualityBicubic: return "InterpolationModeHighQualityBicubic";
        }
        return "";
    }

    OUString UnitTypeToString(sal_uInt16 nType)
    {
        switch (nType)
        {
            case UnitTypeWorld: return "UnitTypeWorld";
            case UnitTypeDisplay: return "UnitTypeDisplay";
            case UnitTypePixel: return "UnitTypePixel";
            case UnitTypePoint: return "UnitTypePoint";
            case UnitTypeInch: return "UnitTypeInch";
            case UnitTypeDocument: return "UnitTypeDocument";
            case UnitTypeMillimeter: return "UnitTypeMillimeter";
        }
        return "";
    }

    OUString HatchStyleToString(sal_uInt32 style)
    {
        switch (style)
        {
            case HatchStyleHorizontal: return "HatchStyleHorizontal";
            case HatchStyleVertical: return "HatchStyleVertical";
            case HatchStyleForwardDiagonal: return "HatchStyleForwardDiagonal";
            case HatchStyleBackwardDiagonal: return "HatchStyleBackwardDiagonal";
            case HatchStyleLargeGrid: return "HatchStyleLargeGrid";
            case HatchStyleDiagonalCross: return "HatchStyleDiagonalCross";
            case HatchStyle05Percent: return "HatchStyle05Percent";
            case HatchStyle10Percent: return "HatchStyle10Percent";
            case HatchStyle20Percent: return "HatchStyle20Percent";
            case HatchStyle25Percent: return "HatchStyle25Percent";
            case HatchStyle30Percent: return "HatchStyle30Percent";
            case HatchStyle40Percent: return "HatchStyle40Percent";
            case HatchStyle50Percent: return "HatchStyle50Percent";
            case HatchStyle60Percent: return "HatchStyle60Percent";
            case HatchStyle70Percent: return "HatchStyle70Percent";
            case HatchStyle75Percent: return "HatchStyle75Percent";
            case HatchStyle80Percent: return "HatchStyle80Percent";
            case HatchStyle90Percent: return "HatchStyle90Percent";
            case HatchStyleLightDownwardDiagonal: return "HatchStyleLightDownwardDiagonal";
            case HatchStyleLightUpwardDiagonal: return "HatchStyleLightUpwardDiagonal";
            case HatchStyleDarkDownwardDiagonal: return "HatchStyleDarkDownwardDiagonal";
            case HatchStyleDarkUpwardDiagonal: return "HatchStyleDarkUpwardDiagonal";
            case HatchStyleWideDownwardDiagonal: return "HatchStyleWideDownwardDiagonal";
            case HatchStyleWideUpwardDiagonal: return "HatchStyleWideUpwardDiagonal";
            case HatchStyleLightVertical: return "HatchStyleLightVertical";
            case HatchStyleLightHorizontal: return "HatchStyleLightHorizontal";
            case HatchStyleNarrowVertical: return "HatchStyleNarrowVertical";
            case HatchStyleNarrowHorizontal: return "HatchStyleNarrowHorizontal";
            case HatchStyleDarkVertical: return "HatchStyleDarkVertical";
            case HatchStyleDarkHorizontal: return "HatchStyleDarkHorizontal";
            case HatchStyleDashedDownwardDiagonal: return "HatchStyleDashedDownwardDiagonal";
            case HatchStyleDashedUpwardDiagonal: return "HatchStyleDashedUpwardDiagonal";
            case HatchStyleDashedHorizontal: return "HatchStyleDashedHorizontal";
            case HatchStyleDashedVertical: return "HatchStyleDashedVertical";
            case HatchStyleSmallConfetti: return "HatchStyleSmallConfetti";
            case HatchStyleLargeConfetti: return "HatchStyleLargeConfetti";
            case HatchStyleZigZag: return "HatchStyleZigZag";
            case HatchStyleWave: return "HatchStyleWave";
            case HatchStyleDiagonalBrick: return "HatchStyleDiagonalBrick";
            case HatchStyleHorizontalBrick: return "HatchStyleHorizontalBrick";
            case HatchStyleWeave: return "HatchStyleWeave";
            case HatchStylePlaid: return "HatchStylePlaid";
            case HatchStyleDivot: return "HatchStyleDivot";
            case HatchStyleDottedGrid: return "HatchStyleDottedGrid";
            case HatchStyleDottedDiamond: return "HatchStyleDottedDiamond";
            case HatchStyleShingle: return "HatchStyleShingle";
            case HatchStyleTrellis: return "HatchStyleTrellis";
            case HatchStyleSphere: return "HatchStyleSphere";
            case HatchStyleSmallGrid: return "HatchStyleSmallGrid";
            case HatchStyleSmallCheckerBoard: return "HatchStyleSmallCheckerBoard";
            case HatchStyleLargeCheckerBoard: return "HatchStyleLargeCheckerBoard";
            case HatchStyleOutlinedDiamond: return "HatchStyleOutlinedDiamond";
            case HatchStyleSolidDiamond: return "HatchStyleSolidDiamond";
        }

        return "";
    }

    bool IsBrush(sal_uInt16 flags)
    {
        return (!((flags >> 15) & 0x0001));
    }

    OUString BrushTypeToString(sal_uInt32 type)
    {
        switch (type)
        {
            case BrushTypeSolidColor: return "BrushTypeSolidColor";
            case BrushTypeHatchFill: return "BrushTypeHatchFill";
            case BrushTypeTextureFill: return "BrushTypeTextureFill";
            case BrushTypePathGradient: return "BrushTypePathGradient";
            case BrushTypeLinearGradient: return "BrushTypeLinearGradient";
        }
        return "";
    }

    OUString BrushIDToString(sal_uInt16 flags, sal_uInt32 brushid)
    {
        OUString sBrushId;

        if (IsBrush(flags))
            sBrushId = sBrushId.concat("EmfPlusBrush ID: ").concat(OUString::number(brushid));
        else
            sBrushId = sBrushId.concat("ARGB: 0x").concat(OUString::number(brushid, 16));

        return sBrushId;
    }

    OUString WrapModeToString(sal_uInt32 mode)
    {
        switch (mode)
        {
            case WrapModeTile: return "WrapModeTile";
            case WrapModeTileFlipX: return "WrapModeTileFlipX";
            case WrapModeTileFlipY: return "WrapModeTileFlipY";
            case WrapModeTileFlipXY: return "WrapModeTileFlipXY";
            case WrapModeClamp: return "WrapModeClamp";
        }
        return "";
    }

    OUString BrushDataFlagsToString(sal_uInt32 brush)
    {
        OUString sBrush;

        if (brush == BrushDataPath)
            sBrush = sBrush.concat("BrushDataPath");

        if (brush == BrushDataTransform)
        {
            if (sBrush.isEmpty())
                sBrush = sBrush.concat(", ");

            sBrush = sBrush.concat("BrushDataTransform");
        }

        if (brush == BrushDataPresetColors)
        {
            if (sBrush.isEmpty())
                sBrush = sBrush.concat(", ");

            sBrush = sBrush.concat("BrushDataPresetColors");
        }

        if (brush == BrushDataBlendFactorsH)
        {
            if (sBrush.isEmpty())
                sBrush = sBrush.concat(", ");

            sBrush = sBrush.concat("BrushDataBlendFactorsH");
        }

        if (brush == BrushDataBlendFactorsV)
        {
            if (sBrush.isEmpty())
                sBrush = sBrush.concat(", ");

            sBrush = sBrush.concat("BrushDataBlendFactorsV");
        }

        if (brush == BrushDataFocusScales)
        {
            if (sBrush.isEmpty())
                sBrush = sBrush.concat(", ");

            sBrush = sBrush.concat("BrushDataFocusScales");
        }

        if (brush == BrushDataGammaCorrected)
        {
            if (sBrush.isEmpty())
                sBrush = sBrush.concat(", ");

            sBrush = sBrush.concat("BrushDataGammaCorrected");
        }

        if (brush == BrushDataDoNotTransform)
        {
            if (sBrush.isEmpty())
                sBrush = sBrush.concat(", ");

            sBrush = sBrush.concat("BrushDataDoNotTransform");
        }

        return sBrush;
    }

    OUString ImageDataTypeToString(sal_uInt32 type)
    {
        switch (type)
        {
            case ImageDataTypeUnknown: return "ImageDataTypeUnknown";
            case ImageDataTypeBitmap: return "ImageDataTypeBitmap";
            case ImageDataTypeMetafile: return "ImageDataTypeMetafile";
        }

        return "";
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
