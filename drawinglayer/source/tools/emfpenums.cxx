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
            case ObjectTypeBrush: return "ObjectTypeBrush";
            case ObjectTypePen: return "ObjectTypePen";
            case ObjectTypePath: return "ObjectTypePath";
            case ObjectTypeRegion: return "ObjectTypeRegion";
            case ObjectTypeImage: return "ObjectTypeImage";
            case ObjectTypeFont: return "ObjectTypeFont";
            case ObjectTypeStringFormat: return "ObjectTypeStringFormat";
            case ObjectTypeImageAttributes: return "ObjectTypeImageAttributes";
            case ObjectTypeCustomLineCap: return "ObjectTypeCustomLineCap";
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

    OUString PixelFormatToString(sal_uInt32 format)
    {
        switch (format)
        {
            case PixelFormatUndefined: return "PixelFormatUndefined";
            case PixelFormat1bppIndexed: return "PixelFormat1bppIndexed";
            case PixelFormat4bppIndexed: return "PixelFormat4bppIndexed";
            case PixelFormat8bppIndexed: return "PixelFormat8bppIndexed";
            case PixelFormat16bppGrayScale: return "PixelFormat16bppGrayScale";
            case PixelFormat16bppRGB555: return "PixelFormat16bppRGB555";
            case PixelFormat16bppRGB565: return "PixelFormat16bppRGB565";
            case PixelFormat16bppARGB1555: return "PixelFormat16bppARGB1555";
            case PixelFormat24bppRGB: return "PixelFormat24bppRGB";
            case PixelFormat32bppRGB: return "PixelFormat32bppRGB";
            case PixelFormat32bppARGB: return "PixelFormat32bppARGB";
            case PixelFormat32bppPARGB: return "PixelFormat32bppPARGB";
            case PixelFormat48bppRGB: return "PixelFormat48bppRGB";
            case PixelFormat64bppARGB: return "PixelFormat64bppARGB";
            case PixelFormat64bppPARGB: return "PixelFormat64bppPARGB";
        }
        return "";
    }

    bool PixelFormatUsesPalette(sal_uInt32 format)
    {
        return (format & 0x00008000) >> 15;
    }

    bool PixelFormatGDISupported(sal_uInt32 format)
    {
        return (format & 0x00004000) >> 14;
    }

    bool PixelFormatIncludesAlpha(sal_uInt32 format)
    {
        return (format & 0x00002000) >> 13;
    }

    bool PixelFormatIsPremultiplied(sal_uInt32 format)
    {
        return (format & 0x00001000) >> 12;
    }

    bool PixelFormatSupportsExtendedColors(sal_uInt32 format)
    {
        return (format & 0x00000800) >> 11;
    }

    bool PixelFormatIsCanonical(sal_uInt32 format)
    {
        return (format & 0x00000400) >> 10;
    }

    sal_uInt32 PixelFormatBitsPerPixel(sal_uInt32 format)
    {
        switch(format)
        {
            case PixelFormat1bppIndexed:
                return 1;

            case PixelFormat4bppIndexed:
                return 4;

            case PixelFormat8bppIndexed:
                return 8;

            case PixelFormat16bppGrayScale:
            case PixelFormat16bppRGB555:
            case PixelFormat16bppRGB565:
            case PixelFormat16bppARGB1555:
                return 16;

            case PixelFormat24bppRGB:
                return 24;

            case PixelFormatUndefined:
            case PixelFormat32bppRGB:
            case PixelFormat32bppARGB:
            case PixelFormat32bppPARGB:
                return 32;

            case PixelFormat48bppRGB:
                return 48;

            case PixelFormat64bppARGB:
            case PixelFormat64bppPARGB:
                return 64;
        }

        return 32;
    }

    sal_uInt32 PixelFormatEnumerationIndex(sal_uInt32 format)
    {
        return (format & 0xFF000000) >> 24;
    }

    OUString BitmapDataTypeToString(sal_uInt32 type)
    {
        switch (type)
        {
            case BitmapDataTypePixel: return "BitmapDataTypePixel";
            case BitmapDataTypeCompressed: return "BitmapDataTypeCompressed";
        }
        return "";
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
