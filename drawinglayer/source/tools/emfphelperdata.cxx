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

#include "emfpcustomlinecap.hxx"
#include "emfphelperdata.hxx"
#include "emfpbrush.hxx"
#include "emfppen.hxx"
#include "emfppath.hxx"
#include "emfpregion.hxx"
#include "emfpimage.hxx"
#include "emfpimageattributes.hxx"
#include "emfpfont.hxx"
#include "emfpstringformat.hxx"
#include <basegfx/curve/b2dcubicbezier.hxx>
#include <wmfemfhelper.hxx>
#include <drawinglayer/primitive2d/PolygonStrokeArrowPrimitive2D.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolyPolygonStrokePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/svggradientprimitive2d.hxx>
#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/metafileprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/attribute/fontattribute.hxx>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/color/bcolormodifier.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <sal/log.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <algorithm>

namespace emfplushelper
{

    enum
    {
        WrapModeTile = 0x00000000,
        WrapModeTileFlipX = 0x00000001,
        WrapModeTileFlipY = 0x00000002,
        WrapModeTileFlipXY = 0x00000003,
        WrapModeClamp = 0x00000004
    };

    const char* emfTypeToName(sal_uInt16 type)
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
            case EmfPlusRecordTypeFillClosedCurve: return "EmfPlusRecordTypeFillClosedCurve";
            case EmfPlusRecordTypeDrawClosedCurve: return "EmfPlusRecordTypeDrawClosedCurve";
            case EmfPlusRecordTypeDrawCurve: return "EmfPlusRecordTypeDrawCurve";
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
            case EmfPlusRecordTypeBeginContainer: return "EmfPlusRecordTypeBeginContainer";
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

    static OUString emfObjectToName(sal_uInt16 type)
    {
        switch (type)
        {
            case EmfPlusObjectTypeBrush: return u"EmfPlusObjectTypeBrush"_ustr;
            case EmfPlusObjectTypePen: return u"EmfPlusObjectTypePen"_ustr;
            case EmfPlusObjectTypePath: return u"EmfPlusObjectTypePath"_ustr;
            case EmfPlusObjectTypeRegion: return u"EmfPlusObjectTypeRegion"_ustr;
            case EmfPlusObjectTypeImage: return u"EmfPlusObjectTypeImage"_ustr;
            case EmfPlusObjectTypeFont: return u"EmfPlusObjectTypeFont"_ustr;
            case EmfPlusObjectTypeStringFormat: return u"EmfPlusObjectTypeStringFormat"_ustr;
            case EmfPlusObjectTypeImageAttributes: return u"EmfPlusObjectTypeImageAttributes"_ustr;
            case EmfPlusObjectTypeCustomLineCap: return u"EmfPlusObjectTypeCustomLineCap"_ustr;
        }
        return u""_ustr;
    }

    static OUString PixelOffsetModeToString(sal_uInt16 nPixelOffset)
    {
        switch (nPixelOffset)
        {
            case PixelOffsetMode::PixelOffsetModeDefault: return u"PixelOffsetModeDefault"_ustr;
            case PixelOffsetMode::PixelOffsetModeHighSpeed: return u"PixelOffsetModeHighSpeed"_ustr;
            case PixelOffsetMode::PixelOffsetModeHighQuality: return u"PixelOffsetModeHighQuality"_ustr;
            case PixelOffsetMode::PixelOffsetModeNone: return u"PixelOffsetModeNone"_ustr;
            case PixelOffsetMode::PixelOffsetModeHalf: return u"PixelOffsetModeHalf"_ustr;
        }
        return u""_ustr;
    }

    static OUString SmoothingModeToString(sal_uInt16 nSmoothMode)
    {
        switch (nSmoothMode)
        {
            case SmoothingMode::SmoothingModeDefault: return u"SmoothingModeDefault"_ustr;
            case SmoothingMode::SmoothingModeHighSpeed: return u"SmoothModeHighSpeed"_ustr;
            case SmoothingMode::SmoothingModeHighQuality: return u"SmoothingModeHighQuality"_ustr;
            case SmoothingMode::SmoothingModeNone: return u"SmoothingModeNone"_ustr;
            case SmoothingMode::SmoothingModeAntiAlias8x4: return u"SmoothingModeAntiAlias8x4"_ustr;
            case SmoothingMode::SmoothingModeAntiAlias8x8: return u"SmoothingModeAntiAlias8x8"_ustr;
        }
        return u""_ustr;
    }

    static OUString TextRenderingHintToString(sal_uInt16 nHint)
    {
        switch (nHint)
        {
            case TextRenderingHint::TextRenderingHintSystemDefault: return u"TextRenderingHintSystemDefault"_ustr;
            case TextRenderingHint::TextRenderingHintSingleBitPerPixelGridFit: return u"TextRenderingHintSingleBitPerPixelGridFit"_ustr;
            case TextRenderingHint::TextRenderingHintSingleBitPerPixel: return u"TextRenderingHintSingleBitPerPixel"_ustr;
            case TextRenderingHint::TextRenderingHintAntialiasGridFit: return u"TextRenderingHintAntialiasGridFit"_ustr;
            case TextRenderingHint::TextRenderingHintAntialias: return u"TextRenderingHintAntialias"_ustr;
            case TextRenderingHint::TextRenderingHintClearTypeGridFit: return u"TextRenderingHintClearTypeGridFit"_ustr;
        }
        return u""_ustr;
    }

    static OUString InterpolationModeToString(sal_uInt16 nMode)
    {
        switch (nMode)
        {
            case InterpolationMode::InterpolationModeDefault: return u"InterpolationModeDefault"_ustr;
            case InterpolationMode::InterpolationModeLowQuality: return u"InterpolationModeLowQuality"_ustr;
            case InterpolationMode::InterpolationModeHighQuality: return u"InterpolationModeHighQuality"_ustr;
            case InterpolationMode::InterpolationModeBilinear: return u"InterpolationModeBilinear"_ustr;
            case InterpolationMode::InterpolationModeBicubic: return u"InterpolationModeBicubic"_ustr;
            case InterpolationMode::InterpolationModeNearestNeighbor: return u"InterpolationModeNearestNeighbor"_ustr;
            case InterpolationMode::InterpolationModeHighQualityBilinear: return u"InterpolationModeHighQualityBilinear"_ustr;
            case InterpolationMode::InterpolationModeHighQualityBicubic: return u"InterpolationModeHighQualityBicubic"_ustr;
        }
        return u""_ustr;
    }

    OUString UnitTypeToString(sal_uInt16 nType)
    {
        switch (nType)
        {
            case UnitTypeWorld: return u"UnitTypeWorld"_ustr;
            case UnitTypeDisplay: return u"UnitTypeDisplay"_ustr;
            case UnitTypePixel: return u"UnitTypePixel"_ustr;
            case UnitTypePoint: return u"UnitTypePoint"_ustr;
            case UnitTypeInch: return u"UnitTypeInch"_ustr;
            case UnitTypeDocument: return u"UnitTypeDocument"_ustr;
            case UnitTypeMillimeter: return u"UnitTypeMillimeter"_ustr;
        }
        return u""_ustr;
    }

    static bool IsBrush(sal_uInt16 flags)
    {
        return (!((flags >> 15) & 0x0001));
    }

    static OUString BrushIDToString(sal_uInt16 flags, sal_uInt32 brushid)
    {
        if (IsBrush(flags))
            return "EmfPlusBrush ID: " + OUString::number(brushid);
        else
            return "ARGB: 0x" + OUString::number(brushid, 16);
    }

    EMFPObject::~EMFPObject()
    {
    }

    double EmfPlusHelperData::unitToPixel(double n, sal_uInt32 aUnitType, Direction d)
    {
        switch (static_cast<UnitType>(aUnitType))
        {
            case UnitTypePixel:
                return n;

            case UnitTypePoint:
                return o3tl::convert(n, o3tl::Length::pt, o3tl::Length::in) * DPI(d);

            case UnitTypeInch:
                return n * DPI(d);

            case UnitTypeMillimeter:
                return o3tl::convert(n, o3tl::Length::mm, o3tl::Length::in) * DPI(d);

            case UnitTypeDocument:
                return n * DPI(d) / 300.0;

            case UnitTypeWorld:
            case UnitTypeDisplay:
                SAL_WARN("drawinglayer.emf", "EMF+\t Converting to World/Display.");
                return n;

            default:
                SAL_WARN("drawinglayer.emf", "EMF+\tTODO Unimplemented support of Unit Type: 0x" << std::hex << aUnitType);
                return n;
        }
    }

    void EmfPlusHelperData::processObjectRecord(SvMemoryStream& rObjectStream, sal_uInt16 flags, sal_uInt32 dataSize, bool bUseWholeStream)
    {
        sal_uInt16 objecttype = flags & 0x7f00;
        sal_uInt16 index = flags & 0xff;
        SAL_INFO("drawinglayer.emf", "EMF+ Object: " << emfObjectToName(objecttype) << " (0x" << objecttype << ")");
        SAL_INFO("drawinglayer.emf", "EMF+\tObject slot: " << index);
        SAL_INFO("drawinglayer.emf", "EMF+\tFlags: " << (flags & 0xff00));

        switch (objecttype)
        {
            case EmfPlusObjectTypeBrush:
            {
                EMFPBrush *brush = new EMFPBrush();
                maEMFPObjects[index].reset(brush);
                brush->Read(rObjectStream, *this);
                break;
            }
            case EmfPlusObjectTypePen:
            {
                EMFPPen *pen = new EMFPPen();
                maEMFPObjects[index].reset(pen);
                pen->Read(rObjectStream, *this);
                pen->penWidth = unitToPixel(pen->penWidth, pen->penUnit, Direction::horizontal);
                break;
            }
            case EmfPlusObjectTypePath:
            {
                sal_uInt32 aVersion, aPathPointCount, aPathPointFlags;

                rObjectStream.ReadUInt32(aVersion).ReadUInt32(aPathPointCount).ReadUInt32(aPathPointFlags);
                SAL_INFO("drawinglayer.emf", "EMF+\t\tVersion: 0x" << std::hex << aVersion);
                SAL_INFO("drawinglayer.emf", "EMF+\t\tNumber of points: " << std::dec << aPathPointCount);
                SAL_INFO("drawinglayer.emf", "EMF+\t\tPath point flags: 0x" << std::hex << aPathPointFlags << std::dec);
                EMFPPath *path = new EMFPPath(aPathPointCount);
                maEMFPObjects[index].reset(path);
                path->Read(rObjectStream, aPathPointFlags);
                break;
            }
            case EmfPlusObjectTypeRegion:
            {
                EMFPRegion *region = new EMFPRegion();
                maEMFPObjects[index].reset(region);
                region->ReadRegion(rObjectStream, *this);
                break;
            }
            case EmfPlusObjectTypeImage:
            {
                EMFPImage *image = new EMFPImage;
                maEMFPObjects[index].reset(image);
                image->type = 0;
                image->width = 0;
                image->height = 0;
                image->stride = 0;
                image->pixelFormat = 0;
                image->Read(rObjectStream, dataSize, bUseWholeStream);
                break;
            }
            case EmfPlusObjectTypeFont:
            {
                EMFPFont *font = new EMFPFont;
                maEMFPObjects[index].reset(font);
                font->emSize = 0;
                font->sizeUnit = 0;
                font->fontFlags = 0;
                font->Read(rObjectStream);
                // tdf#113624 Convert unit to Pixels
                font->emSize = unitToPixel(font->emSize, font->sizeUnit, Direction::horizontal);

                break;
            }
            case EmfPlusObjectTypeStringFormat:
            {
                EMFPStringFormat *stringFormat = new EMFPStringFormat();
                maEMFPObjects[index].reset(stringFormat);
                stringFormat->Read(rObjectStream);
                break;
            }
            case EmfPlusObjectTypeImageAttributes:
            {
                EMFPImageAttributes *imageAttributes = new EMFPImageAttributes();
                maEMFPObjects[index].reset(imageAttributes);
                imageAttributes->Read(rObjectStream);
                break;
            }
            case EmfPlusObjectTypeCustomLineCap:
            {
                SAL_WARN("drawinglayer.emf", "EMF+\t TODO Object type 'custom line cap' not yet implemented");
                break;
            }
            default:
            {
                SAL_WARN("drawinglayer.emf", "EMF+\t TODO Object unhandled flags: 0x" << std::hex << (flags & 0xff00) << std::dec);
            }
        }
    }

    void EmfPlusHelperData::ReadPoint(SvStream& s, float& x, float& y, sal_uInt32 flags)
    {
        if (flags & 0x800)
        {
            // specifies a location in the coordinate space that is relative to
            // the location specified by the previous element in the array. In the case of the first element in
            // PointData, a previous location at coordinates (0,0) is assumed.
            SAL_WARN("drawinglayer.emf", "EMF+\t\t TODO Relative coordinates bit detected. Implement parse EMFPlusPointR");
        }

        if (flags & 0x4000)
        {
            sal_Int16 ix, iy;

            s.ReadInt16(ix).ReadInt16(iy);

            x = ix;
            y = iy;
        }
        else
        {
            s.ReadFloat(x).ReadFloat(y);
        }
    }

    void EmfPlusHelperData::ReadRectangle(SvStream& s, float& x, float& y, float &width, float& height, bool bCompressed)
    {
        if (bCompressed)
        {
            sal_Int16 ix, iy, iw, ih;

            s.ReadInt16(ix).ReadInt16(iy).ReadInt16(iw).ReadInt16(ih);

            x = ix;
            y = iy;
            width = iw;
            height = ih;
        }
        else
        {
            s.ReadFloat(x).ReadFloat(y).ReadFloat(width).ReadFloat(height);
        }
    }

    bool EmfPlusHelperData::readXForm(SvStream& rIn, basegfx::B2DHomMatrix& rTarget)
    {
        rTarget.identity();

        if (sizeof(float) != 4)
        {
            OSL_FAIL("EnhWMFReader::sizeof( float ) != 4");
            return false;
        }
        else
        {
            float eM11(0.0);
            float eM12(0.0);
            float eM21(0.0);
            float eM22(0.0);
            float eDx(0.0);
            float eDy(0.0);
            rIn.ReadFloat(eM11).ReadFloat(eM12).ReadFloat(eM21).ReadFloat(eM22).ReadFloat(eDx).ReadFloat(eDy);
            rTarget = basegfx::B2DHomMatrix(
                eM11, eM21, eDx,
                eM12, eM22, eDy);
        }

        return true;
    }

    void EmfPlusHelperData::mappingChanged()
    {
        if (mnPixX == 0 || mnPixY == 0)
        {
            SAL_WARN("drawinglayer.emf", "dimensions in pixels is 0");
            return;
        }
        // Call when mnMmX/mnMmY/mnPixX/mnPixY/mnFrameLeft/mnFrameTop/maWorldTransform/ changes.
        // Currently not used are mnHDPI/mnVDPI/mnFrameRight/mnFrameBottom. *If* these should
        // be used in the future, this method will need to be called.
        //
        // Re-calculate maMapTransform to contain the complete former transformation so that
        // it can be applied by a single matrix multiplication or be added to an encapsulated
        // primitive later
        //
        // To evtl. correct and see where this came from, please compare with the implementations
        // of EmfPlusHelperData::MapToDevice and EmfPlusHelperData::Map* in prev versions
        maMapTransform = maWorldTransform;
        maMapTransform *= basegfx::utils::createScaleTranslateB2DHomMatrix(100.0 * mnMmX / mnPixX, 100.0 * mnMmY / mnPixY,
                                                                           double(-mnFrameLeft), double(-mnFrameTop));
        maMapTransform *= maBaseTransform;

        // Used only for performance optimization, to do not calculate it every line draw
        mdExtractedXScale = std::hypot(maMapTransform.a(), maMapTransform.b());
        mdExtractedYScale = std::hypot(maMapTransform.c(), maMapTransform.d());
    }

    ::basegfx::B2DPoint EmfPlusHelperData::Map(double ix, double iy) const
    {
        // map in one step using complete MapTransform (see mappingChanged)
        return maMapTransform * ::basegfx::B2DPoint(ix, iy);
    }

    Color EmfPlusHelperData::EMFPGetBrushColorOrARGBColor(const sal_uInt16 flags, const sal_uInt32 brushIndexOrColor) const {
        Color color;
        if (flags & 0x8000) // we use a color
        {
            color = Color(ColorAlpha, (brushIndexOrColor >> 24), (brushIndexOrColor >> 16) & 0xff,
                          (brushIndexOrColor >> 8) & 0xff, brushIndexOrColor & 0xff);
        }
        else // we use a brush
        {
            const EMFPBrush* brush = dynamic_cast<EMFPBrush*>(maEMFPObjects[brushIndexOrColor & 0xff].get());
            if (brush)
            {
                color = brush->GetColor();
                if (brush->type != BrushTypeSolidColor)
                    SAL_WARN("drawinglayer.emf", "EMF+\t\t TODO Brush other than solid color is not supported");
            }
        }
        return color;
    }

    void EmfPlusHelperData::GraphicStatePush(GraphicStateMap& map, sal_Int32 index)
    {
        GraphicStateMap::iterator iter = map.find( index );

        if ( iter != map.end() )
        {
            map.erase( iter );
            SAL_INFO("drawinglayer.emf", "EMF+\t\tStack index: " << index << " found and erased");
        }

        wmfemfhelper::PropertyHolder state = mrPropertyHolders.Current();
        // tdf#112500 We need to save world transform somehow, during graphic state push
        state.setTransformation(maWorldTransform);
        map[ index ] = state;
    }

    void EmfPlusHelperData::GraphicStatePop(GraphicStateMap& map, sal_Int32 index)
    {
        GraphicStateMap::iterator iter = map.find(index);

        if (iter != map.end())
        {
            wmfemfhelper::PropertyHolder state = iter->second;

            maWorldTransform = state.getTransformation();
            if (state.getClipPolyPolygonActive())
            {
                SAL_INFO("drawinglayer.emf",
                        "EMF+\t Restore clipping region to saved in index: " << index);
                wmfemfhelper::HandleNewClipRegion(state.getClipPolyPolygon(), mrTargetHolders,
                                                  mrPropertyHolders);
            }
            else
            {
                SAL_INFO("drawinglayer.emf", "EMF+\t Disable clipping");
                wmfemfhelper::HandleNewClipRegion(::basegfx::B2DPolyPolygon(), mrTargetHolders,
                                                  mrPropertyHolders);
            }
            mappingChanged();
            SAL_INFO("drawinglayer.emf",
                    "EMF+\t\tStack index: " << index
                                            << " found, maWorldTransform: " << maWorldTransform);
        }
    }

    drawinglayer::attribute::LineStartEndAttribute
    EmfPlusHelperData::CreateLineEnd(const sal_Int32 aCap, const float aPenWidth) const
    {
        const double pw = mdExtractedYScale * aPenWidth;
        if (aCap == LineCapTypeSquare)
        {
            basegfx::B2DPolygon aCapPolygon(
                { {-1.0, -1.0}, {1.0, -1.0}, {1.0, 1.0}, {-1.0, 1.0} });
            aCapPolygon.setClosed(true);
            return drawinglayer::attribute::LineStartEndAttribute(
                pw, basegfx::B2DPolyPolygon(aCapPolygon), true);
        }
        else if (aCap == LineCapTypeRound)
        {
            basegfx::B2DPolygon aCapPolygon(
                { {-1.0, 1.0}, {1.0, 1.0}, {1.0, 0.0}, {0.9236, -0.3827},
                  {0.7071, -0.7071}, {0.3827, -0.9236}, {0.0, -1.0}, {-0.3827, -0.9236},
                  {-0.7071, -0.7071}, {-0.9236, -0.3827}, {-1.0, 0.0} });
            aCapPolygon.setClosed(true);
            return drawinglayer::attribute::LineStartEndAttribute(
                pw, basegfx::B2DPolyPolygon(aCapPolygon), true);
        }
        else if (aCap == LineCapTypeTriangle)
        {
            basegfx::B2DPolygon aCapPolygon(
                { {-1.0, 1.0}, {1.0, 1.0}, {1.0, 0.0}, {0.0, -1.0}, {-1.0, 0.0} });
            aCapPolygon.setClosed(true);
            return drawinglayer::attribute::LineStartEndAttribute(
                pw, basegfx::B2DPolyPolygon(aCapPolygon), true);
        }
        else if (aCap == LineCapTypeSquareAnchor)
        {
            basegfx::B2DPolygon aCapPolygon(
                { {-1.0, -1.0}, {1.0, -1.0}, {1.0, 1.0}, {-1.0, 1.0} });
            aCapPolygon.setClosed(true);
            return drawinglayer::attribute::LineStartEndAttribute(
                1.5 * pw, basegfx::B2DPolyPolygon(aCapPolygon), true);
        }
        else if (aCap == LineCapTypeRoundAnchor)
        {
            const basegfx::B2DPolygon aCapPolygon
                = ::basegfx::utils::createPolygonFromEllipse(::basegfx::B2DPoint(0.0, 0.0), 1.0, 1.0);
            return drawinglayer::attribute::LineStartEndAttribute(
                2.0 * pw, basegfx::B2DPolyPolygon(aCapPolygon), true);
        }
        else if (aCap == LineCapTypeDiamondAnchor)
        {
            basegfx::B2DPolygon aCapPolygon({ {0.0, -1.0}, {1.0, 0.0}, {0.5, 0.5},
                                              {0.5, 1.0}, {-0.5, 1.0}, {-0.5, 0.5},
                                              {-1.0, 0.0} });
            aCapPolygon.setClosed(true);
            return drawinglayer::attribute::LineStartEndAttribute(
                2.0 * pw, basegfx::B2DPolyPolygon(aCapPolygon), true);
        }
        else if (aCap == LineCapTypeArrowAnchor)
        {
            basegfx::B2DPolygon aCapPolygon({ {0.0, -1.0}, {1.0, 1.0}, {-1.0, 1.0} });
            aCapPolygon.setClosed(true);
            return drawinglayer::attribute::LineStartEndAttribute(
                2.0 * pw, basegfx::B2DPolyPolygon(aCapPolygon), true);
        }
        return drawinglayer::attribute::LineStartEndAttribute();
    }

    void EmfPlusHelperData::EMFPPlusDrawPolygon(const ::basegfx::B2DPolyPolygon& polygon,
                                                sal_uInt32 penIndex)
    {
        const EMFPPen* pen = dynamic_cast<EMFPPen*>(maEMFPObjects[penIndex & 0xff].get());
        SAL_WARN_IF(!pen, "drawinglayer.emf", "emf+ missing pen");

        if (!(pen && polygon.count()))
            return;

        const double transformedPenWidth = mdExtractedYScale * pen->penWidth;
        drawinglayer::attribute::LineAttribute lineAttribute(
            pen->GetColor().getBColor(), transformedPenWidth, pen->maLineJoin,
            css::drawing::LineCap_BUTT, //TODO implement PenDataDashedLineCap support here
            pen->fMiterMinimumAngle);

        drawinglayer::attribute::LineStartEndAttribute aStart;
        if (pen->penDataFlags & EmfPlusPenDataStartCap)
        {
            if ((pen->penDataFlags & EmfPlusPenDataCustomStartCap)
                && (pen->customStartCap->polygon.begin()->count() > 1))
                aStart = drawinglayer::attribute::LineStartEndAttribute(
                    pen->customStartCap->polygon.getB2DRange().getRange().getX() * mdExtractedXScale
                        * pen->customStartCap->widthScale * pen->penWidth,
                    pen->customStartCap->polygon, false);
            else
                aStart = EmfPlusHelperData::CreateLineEnd(pen->startCap, pen->penWidth);
        }

        drawinglayer::attribute::LineStartEndAttribute aEnd;
        if (pen->penDataFlags & EmfPlusPenDataEndCap)
        {
            if ((pen->penDataFlags & EmfPlusPenDataCustomEndCap)
                && (pen->customEndCap->polygon.begin()->count() > 1))
                aEnd = drawinglayer::attribute::LineStartEndAttribute(
                    pen->customEndCap->polygon.getB2DRange().getRange().getX() * mdExtractedXScale
                        * pen->customEndCap->widthScale * pen->penWidth,
                    pen->customEndCap->polygon, false);
            else
                aEnd = EmfPlusHelperData::CreateLineEnd(pen->endCap, pen->penWidth);
        }

        if (pen->GetColor().IsTransparent())
        {
            drawinglayer::primitive2d::Primitive2DContainer aContainer;
            if (aStart.isDefault() && aEnd.isDefault())
                aContainer.append(
                    new drawinglayer::primitive2d::PolyPolygonStrokePrimitive2D(
                        polygon, lineAttribute, pen->GetStrokeAttribute(mdExtractedXScale)));
            else
            {
                aContainer.resize(polygon.count());
                for (sal_uInt32 i = 0; i < polygon.count(); i++)
                    aContainer[i] =
                        new drawinglayer::primitive2d::PolygonStrokeArrowPrimitive2D(
                            polygon.getB2DPolygon(i), lineAttribute,
                            pen->GetStrokeAttribute(mdExtractedXScale), aStart, aEnd);
            }
            mrTargetHolders.Current().append(
                new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(
                    std::move(aContainer), (255 - pen->GetColor().GetAlpha()) / 255.0));
        }
        else
        {
            if (aStart.isDefault() && aEnd.isDefault())
                mrTargetHolders.Current().append(
                    new drawinglayer::primitive2d::PolyPolygonStrokePrimitive2D(
                        polygon, lineAttribute, pen->GetStrokeAttribute(mdExtractedXScale)));
            else
                for (sal_uInt32 i = 0; i < polygon.count(); i++)
                {
                    mrTargetHolders.Current().append(
                        new drawinglayer::primitive2d::PolygonStrokeArrowPrimitive2D(
                            polygon.getB2DPolygon(i), lineAttribute,
                            pen->GetStrokeAttribute(mdExtractedXScale), aStart, aEnd));
                }
        }
        mrPropertyHolders.Current().setLineColor(pen->GetColor().getBColor());
        mrPropertyHolders.Current().setLineColorActive(true);
        mrPropertyHolders.Current().setFillColorActive(false);
    }

    void EmfPlusHelperData::EMFPPlusFillPolygonSolidColor(const ::basegfx::B2DPolyPolygon& polygon, Color const& color)
    {
        if (color.GetAlpha() == 0)
            return;

        if (!color.IsTransparent())
        {
            // not transparent
            mrTargetHolders.Current().append(
                        new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
                            polygon,
                            color.getBColor()));
        }
        else
        {
            const drawinglayer::primitive2d::Primitive2DReference aPrimitive(
                        new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
                            polygon,
                            color.getBColor()));

            mrTargetHolders.Current().append(
                        new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(
                            drawinglayer::primitive2d::Primitive2DContainer { aPrimitive },
                            (255 - color.GetAlpha()) / 255.0));
        }
    }

    void EmfPlusHelperData::EMFPPlusFillPolygon(const ::basegfx::B2DPolyPolygon& polygon, const bool isColor, const sal_uInt32 brushIndexOrColor)
    {
        if (!polygon.count())
          return;

        if (isColor) // use Color
        {
            SAL_INFO("drawinglayer.emf", "EMF+\t\t Fill polygon, ARGB color: 0x" << std::hex << brushIndexOrColor << std::dec);

            // EMF Alpha (1 byte): An 8-bit unsigned integer that specifies the transparency of the background,
            // ranging from 0 for completely transparent to 0xFF for completely opaque.
            const Color color(ColorAlpha, (brushIndexOrColor >> 24), (brushIndexOrColor >> 16) & 0xff, (brushIndexOrColor >> 8) & 0xff, brushIndexOrColor & 0xff);
            EMFPPlusFillPolygonSolidColor(polygon, color);

            mrPropertyHolders.Current().setFillColor(color.getBColor());
            mrPropertyHolders.Current().setFillColorActive(true);
            mrPropertyHolders.Current().setLineColorActive(false);
        }
        else // use Brush
        {
            EMFPBrush* brush = dynamic_cast<EMFPBrush*>(maEMFPObjects[brushIndexOrColor & 0xff].get());
            SAL_INFO("drawinglayer.emf", "EMF+\t\t Fill polygon, brush slot: " << brushIndexOrColor << " (brush type: " << (brush ? brush->GetType() : -1) << ")");

            // give up in case something wrong happened
            if( !brush )
                return;

            mrPropertyHolders.Current().setFillColorActive(false);
            mrPropertyHolders.Current().setLineColorActive(false);

            if (brush->type == BrushTypeSolidColor)
            {
                Color fillColor = brush->solidColor;
                EMFPPlusFillPolygonSolidColor(polygon, fillColor);
            }
            else if (brush->type == BrushTypeHatchFill)
            {
                // EMF+ like hatching is currently not supported. These are just color blends which serve as an approximation for some of them
                // for the others the hatch "background" color (secondColor in brush) is used.

                bool isHatchBlend = true;
                double blendFactor = 0.0;

                switch (brush->hatchStyle)
                {
                    case HatchStyle05Percent: blendFactor = 0.05; break;
                    case HatchStyle10Percent: blendFactor = 0.10; break;
                    case HatchStyle20Percent: blendFactor = 0.20; break;
                    case HatchStyle25Percent: blendFactor = 0.25; break;
                    case HatchStyle30Percent: blendFactor = 0.30; break;
                    case HatchStyle40Percent: blendFactor = 0.40; break;
                    case HatchStyle50Percent: blendFactor = 0.50; break;
                    case HatchStyle60Percent: blendFactor = 0.60; break;
                    case HatchStyle70Percent: blendFactor = 0.70; break;
                    case HatchStyle75Percent: blendFactor = 0.75; break;
                    case HatchStyle80Percent: blendFactor = 0.80; break;
                    case HatchStyle90Percent: blendFactor = 0.90; break;
                    default:
                        isHatchBlend = false;
                        break;
                }
                Color fillColor;
                if (isHatchBlend)
                {
                    fillColor = brush->solidColor;
                    fillColor.Merge(brush->secondColor, static_cast<sal_uInt8>(255 * blendFactor));
                }
                else
                {
                    fillColor = brush->secondColor;
                }
                // temporal solution: create a solid colored polygon
                // TODO create a 'real' hatching primitive
                mrTargetHolders.Current().append(
                    new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
                        polygon,
                        fillColor.getBColor()));
            }
            else if (brush->type == BrushTypeTextureFill)
            {
                SAL_WARN("drawinglayer.emf", "EMF+\tTODO: implement BrushTypeTextureFill brush");
            }
            else if (brush->type == BrushTypePathGradient || brush->type == BrushTypeLinearGradient)

            {
                if (brush->type == BrushTypePathGradient && !(brush->additionalFlags & 0x1))
                {
                    SAL_WARN("drawinglayer.emf", "EMF+\t TODO Implement displaying BrushTypePathGradient with Boundary: ");
                }
                ::basegfx::B2DHomMatrix aTextureTransformation;

                if (brush->hasTransformation) {
                   aTextureTransformation = brush->brush_transformation;

                   // adjust aTextureTransformation for our world space:
                   // -> revert the mapping -> apply the transformation -> map back
                   basegfx::B2DHomMatrix aInvertedMapTrasform(maMapTransform);
                   aInvertedMapTrasform.invert();
                   aTextureTransformation =  maMapTransform * aTextureTransformation * aInvertedMapTrasform;
                }

                // select the stored colors
                const basegfx::BColor aStartColor = brush->solidColor.getBColor();
                const basegfx::BColor aEndColor = brush->secondColor.getBColor();
                drawinglayer::primitive2d::SvgGradientEntryVector aVector;

                if (brush->blendPositions)
                {
                    SAL_INFO("drawinglayer.emf", "EMF+\t\tUse blend");

                    // store the blendpoints in the vector
                    for (sal_uInt32 i = 0; i < brush->blendPoints; i++)
                    {
                        const double aBlendPoint = brush->blendPositions[i];
                        basegfx::BColor aColor;
                        aColor.setGreen(aStartColor.getGreen() + brush->blendFactors[i] * (aEndColor.getGreen() - aStartColor.getGreen()));
                        aColor.setBlue (aStartColor.getBlue()  + brush->blendFactors[i] * (aEndColor.getBlue() - aStartColor.getBlue()));
                        aColor.setRed  (aStartColor.getRed()   + brush->blendFactors[i] * (aEndColor.getRed() - aStartColor.getRed()));
                        const double aAlpha = brush->solidColor.GetAlpha() + brush->blendFactors[i] * (brush->secondColor.GetAlpha() - brush->solidColor.GetAlpha());
                        aVector.emplace_back(aBlendPoint, aColor, aAlpha / 255.0);
                    }
                }
                else if (brush->colorblendPositions)
                {
                    SAL_INFO("drawinglayer.emf", "EMF+\t\tUse color blend");

                    // store the colorBlends in the vector
                    for (sal_uInt32 i = 0; i < brush->colorblendPoints; i++)
                    {
                        const double aBlendPoint = brush->colorblendPositions[i];
                        const basegfx::BColor aColor = brush->colorblendColors[i].getBColor();
                        aVector.emplace_back(aBlendPoint, aColor, brush->colorblendColors[i].GetAlpha() / 255.0);
                    }
                }
                else // ok, no extra points: just start and end
                {
                    aVector.emplace_back(0.0, aStartColor, brush->solidColor.GetAlpha() / 255.0);
                    aVector.emplace_back(1.0, aEndColor, brush->secondColor.GetAlpha() / 255.0);
                }

                // get the polygon range to be able to map the start/end/center point correctly
                // therefore, create a mapping and invert it
                basegfx::B2DRange aPolygonRange= polygon.getB2DRange();
                basegfx::B2DHomMatrix aPolygonTransformation = basegfx::utils::createScaleTranslateB2DHomMatrix(
                    aPolygonRange.getWidth(),aPolygonRange.getHeight(),
                    aPolygonRange.getMinX(), aPolygonRange.getMinY());
                aPolygonTransformation.invert();

                if (brush->type == BrushTypeLinearGradient)
                {
                    // support for public enum EmfPlusWrapMode
                    basegfx::B2DPoint aStartPoint = Map(brush->firstPointX, 0.0);
                    aStartPoint = aPolygonTransformation * aStartPoint;
                    basegfx::B2DPoint aEndPoint = Map(brush->firstPointX + brush->aWidth, 0.0);
                    aEndPoint = aPolygonTransformation * aEndPoint;

                    // support for public enum EmfPlusWrapMode
                    drawinglayer::primitive2d::SpreadMethod aSpreadMethod(drawinglayer::primitive2d::SpreadMethod::Pad);
                    switch(brush->wrapMode)
                    {
                        case WrapModeTile:
                        case WrapModeTileFlipY:
                        {
                            aSpreadMethod = drawinglayer::primitive2d::SpreadMethod::Repeat;
                            break;
                        }
                        case WrapModeTileFlipX:
                        case WrapModeTileFlipXY:
                        {
                            aSpreadMethod = drawinglayer::primitive2d::SpreadMethod::Reflect;
                            break;
                        }
                        default:
                            break;
                    }

                    // create the same one used for SVG
                    mrTargetHolders.Current().append(
                        new drawinglayer::primitive2d::SvgLinearGradientPrimitive2D(
                            aTextureTransformation,
                            polygon,
                            std::move(aVector),
                            aStartPoint,
                            aEndPoint,
                            false,                  // do not use UnitCoordinates
                            aSpreadMethod));
                }
                else // BrushTypePathGradient
                { // TODO The PathGradient is not implemented, and Radial Gradient is used instead
                    basegfx::B2DPoint aCenterPoint = Map(brush->firstPointX, brush->firstPointY);
                    aCenterPoint = aPolygonTransformation * aCenterPoint;

                    // create the same one used for SVG
                    mrTargetHolders.Current().append(
                        new drawinglayer::primitive2d::SvgRadialGradientPrimitive2D(
                            aTextureTransformation,
                            polygon,
                            std::move(aVector),
                            aCenterPoint,
                            0.7, // relative radius little bigger to cover all elements
                            true, // use UnitCoordinates to stretch the gradient
                            drawinglayer::primitive2d::SpreadMethod::Pad,
                            nullptr));
                }
            }
        }
    }

    EmfPlusHelperData::EmfPlusHelperData(
        SvMemoryStream& rMS,
        wmfemfhelper::TargetHolders& rTargetHolders,
        wmfemfhelper::PropertyHolders& rPropertyHolders)
    :   mfPageScale(0.0),
        mnOriginX(0),
        mnOriginY(0),
        mnHDPI(0),
        mnVDPI(0),
        mbSetTextContrast(false),
        mnTextContrast(0),
        mnFrameLeft(0),
        mnFrameTop(0),
        mnFrameRight(0),
        mnFrameBottom(0),
        mnPixX(0),
        mnPixY(0),
        mnMmX(0),
        mnMmY(0),
        mbMultipart(false),
        mMFlags(0),
        mdExtractedXScale(1.0),
        mdExtractedYScale(1.0),
        mrTargetHolders(rTargetHolders),
        mrPropertyHolders(rPropertyHolders),
        bIsGetDCProcessing(false)
    {
        rMS.ReadInt32(mnFrameLeft).ReadInt32(mnFrameTop).ReadInt32(mnFrameRight).ReadInt32(mnFrameBottom);
        SAL_INFO("drawinglayer.emf", "EMF+ picture frame: " << mnFrameLeft << "," << mnFrameTop << " - " << mnFrameRight << "," << mnFrameBottom);
        rMS.ReadInt32(mnPixX).ReadInt32(mnPixY).ReadInt32(mnMmX).ReadInt32(mnMmY);
        SAL_INFO("drawinglayer.emf", "EMF+ ref device pixel size: " << mnPixX << "x" << mnPixY << " mm size: " << mnMmX << "x" << mnMmY);
        readXForm(rMS, maBaseTransform);
        SAL_INFO("drawinglayer.emf", "EMF+ base transform: " << maBaseTransform);
        mappingChanged();
    }

    EmfPlusHelperData::~EmfPlusHelperData()
    {
    }

    ::basegfx::B2DPolyPolygon EmfPlusHelperData::combineClip(::basegfx::B2DPolyPolygon const & leftPolygon, int combineMode, ::basegfx::B2DPolyPolygon const & rightPolygon)
    {
        basegfx::B2DPolyPolygon aClippedPolyPolygon;
        switch (combineMode)
        {
        case EmfPlusCombineModeReplace:
        {
            aClippedPolyPolygon = rightPolygon;
            break;
        }
        case EmfPlusCombineModeIntersect:
        {
            aClippedPolyPolygon = basegfx::utils::clipPolyPolygonOnPolyPolygon(
                leftPolygon, rightPolygon, true, false);
            break;
        }
        case EmfPlusCombineModeUnion:
        {
            aClippedPolyPolygon = ::basegfx::utils::solvePolygonOperationOr(leftPolygon, rightPolygon);
            break;
        }
        case EmfPlusCombineModeXOR:
        {
            aClippedPolyPolygon = ::basegfx::utils::solvePolygonOperationXor(leftPolygon, rightPolygon);
            break;
        }
        case EmfPlusCombineModeExclude:
        {
            // Replaces the existing region with the part of itself that is not in the new region.
            aClippedPolyPolygon = ::basegfx::utils::solvePolygonOperationDiff(leftPolygon, rightPolygon);
            break;
        }
        case EmfPlusCombineModeComplement:
        {
            // Replaces the existing region with the part of the new region that is not in the existing region.
            aClippedPolyPolygon = ::basegfx::utils::solvePolygonOperationDiff(rightPolygon, leftPolygon);
            break;
        }
        }
        return aClippedPolyPolygon;
    }

    void EmfPlusHelperData::processEmfPlusData(
        SvMemoryStream& rMS,
        const drawinglayer::geometry::ViewInformation2D& /*rViewInformation*/)
    {
        sal_uInt64 length = rMS.GetSize();

        if (length < 12)
            SAL_WARN("drawinglayer.emf", "length is less than required header size");

        // 12 is minimal valid EMF+ record size; remaining bytes are padding
        while (length >= 12)
        {
            sal_uInt16 type, flags;
            sal_uInt32 size, dataSize;
            sal_uInt64 next;

            rMS.ReadUInt16(type).ReadUInt16(flags).ReadUInt32(size).ReadUInt32(dataSize);

            next = rMS.Tell() + (size - 12);

            if (size < 12)
            {
                SAL_WARN("drawinglayer.emf", "Size field is less than 12 bytes");
                break;
            }
            else if (size > length)
            {
                SAL_WARN("drawinglayer.emf", "Size field is greater than bytes left");
                break;
            }

            if (dataSize > (size - 12))
            {
                SAL_WARN("drawinglayer.emf", "DataSize field is greater than Size-12");
                break;
            }

            SAL_INFO("drawinglayer.emf", "EMF+ " << emfTypeToName(type) << " (0x" << std::hex << type << ")" << std::dec);
            SAL_INFO("drawinglayer.emf", "EMF+\t record size: " << size);
            SAL_INFO("drawinglayer.emf", "EMF+\t flags: 0x" << std::hex << flags << std::dec);
            SAL_INFO("drawinglayer.emf", "EMF+\t data size: " << dataSize);

            if (bIsGetDCProcessing)
            {
                if (aGetDCState.getClipPolyPolygonActive())
                {
                    SAL_INFO("drawinglayer.emf", "EMF+\t Restore region to GetDC saved");
                    wmfemfhelper::HandleNewClipRegion(aGetDCState.getClipPolyPolygon(), mrTargetHolders,
                                                      mrPropertyHolders);
                }
                else
                {
                    SAL_INFO("drawinglayer.emf", "EMF+\t Disable clipping");
                    wmfemfhelper::HandleNewClipRegion(::basegfx::B2DPolyPolygon(), mrTargetHolders,
                                                      mrPropertyHolders);
                }
                bIsGetDCProcessing = false;
            }
            if (type == EmfPlusRecordTypeObject && ((mbMultipart && (flags & 0x7fff) == (mMFlags & 0x7fff)) || (flags & 0x8000)))
            {
                if (!mbMultipart)
                {
                    mbMultipart = true;
                    mMFlags = flags;
                    mMStream.Seek(0);
                }

                OSL_ENSURE(dataSize >= 4, "No room for TotalObjectSize in EmfPlusContinuedObjectRecord");

                // 1st 4 bytes are TotalObjectSize
                mMStream.WriteBytes(static_cast<const char *>(rMS.GetData()) + rMS.Tell() + 4, dataSize - 4);
                SAL_INFO("drawinglayer.emf", "EMF+ read next object part size: " << size << " type: " << type << " flags: " << flags << " data size: " << dataSize);
            }
            else
            {
                if (mbMultipart)
                {
                    SAL_INFO("drawinglayer.emf", "EMF+ multipart record flags: " << mMFlags);
                    mMStream.Seek(0);
                    processObjectRecord(mMStream, mMFlags, 0, true);
                }

                mbMultipart = false;
            }

            if (type != EmfPlusRecordTypeObject || !(flags & 0x8000))
            {
                switch (type)
                {
                    case EmfPlusRecordTypeHeader:
                    {
                        sal_uInt32 version, emfPlusFlags;
                        SAL_INFO("drawinglayer.emf", "EMF+\tDual: " << ((flags & 1) ? "true" : "false"));

                        rMS.ReadUInt32(version).ReadUInt32(emfPlusFlags).ReadUInt32(mnHDPI).ReadUInt32(mnVDPI);
                        SAL_INFO("drawinglayer.emf", "EMF+\tVersion: 0x" << std::hex << version);
                        SAL_INFO("drawinglayer.emf", "EMF+\tEmf+ Flags: 0x"  << emfPlusFlags << std::dec);
                        SAL_INFO("drawinglayer.emf", "EMF+\tMetafile was recorded with a reference device context for " << ((emfPlusFlags & 1) ? "video display" : "printer"));
                        SAL_INFO("drawinglayer.emf", "EMF+\tHorizontal DPI: " << mnHDPI);
                        SAL_INFO("drawinglayer.emf", "EMF+\tVertical DPI: " << mnVDPI);
                        break;
                    }
                    case EmfPlusRecordTypeEndOfFile:
                    {
                        break;
                    }
                    case EmfPlusRecordTypeComment:
                    {
#if OSL_DEBUG_LEVEL > 1
                        unsigned char data;
                        OUString hexdata;

                        SAL_INFO("drawinglayer.emf", "EMF+\tDatasize: 0x" << std::hex << dataSize << std::dec);

                        for (sal_uInt32 i=0; i<dataSize; i++)
                        {
                            rMS.ReadUChar(data);

                            if (i % 16 == 0)
                                hexdata += "\n";

                            OUString padding;
                            if ((data & 0xF0) == 0)
                                padding = "0";

                            hexdata += "0x" + padding + OUString::number(data, 16) + " ";
                        }

                        SAL_INFO("drawinglayer.emf", "EMF+\t" << hexdata);
#endif
                        break;
                    }
                    case EmfPlusRecordTypeGetDC:
                    {
                        bIsGetDCProcessing = true;
                        aGetDCState = mrPropertyHolders.Current();
                        SAL_INFO("drawinglayer.emf", "EMF+\tAlready used in svtools wmf/emf filter parser");
                        break;
                    }
                    case EmfPlusRecordTypeObject:
                    {
                        processObjectRecord(rMS, flags, dataSize);
                        break;
                    }
                    case EmfPlusRecordTypeFillPie:
                    case EmfPlusRecordTypeDrawPie:
                    case EmfPlusRecordTypeDrawArc:
                    {
                        float startAngle, sweepAngle;

                        // Silent MSVC warning C4701: potentially uninitialized local variable 'brushIndexOrColor' used
                        sal_uInt32 brushIndexOrColor = 999;

                        if (type == EmfPlusRecordTypeFillPie)
                        {
                            rMS.ReadUInt32(brushIndexOrColor);
                            SAL_INFO("drawinglayer.emf", "EMF+\t FillPie colorOrIndex: " << brushIndexOrColor);
                        }
                        else if (type == EmfPlusRecordTypeDrawPie)
                        {
                            SAL_INFO("drawinglayer.emf", "EMF+\t DrawPie");
                        }
                        else
                        {
                            SAL_INFO("drawinglayer.emf", "EMF+\t DrawArc");
                        }

                        rMS.ReadFloat(startAngle).ReadFloat(sweepAngle);
                        float dx, dy, dw, dh;
                        ReadRectangle(rMS, dx, dy, dw, dh, bool(flags & 0x4000));
                        SAL_INFO("drawinglayer.emf", "EMF+\t RectData: " << dx << "," << dy << " " << dw << "x" << dh);
                        startAngle = basegfx::deg2rad(startAngle);
                        sweepAngle = basegfx::deg2rad(sweepAngle);
                        float endAngle = startAngle + sweepAngle;
                        startAngle = fmodf(startAngle, static_cast<float>(M_PI * 2));

                        if (startAngle < 0.0)
                        {
                            startAngle += static_cast<float>(M_PI * 2.0);
                        }
                        endAngle = fmodf(endAngle, static_cast<float>(M_PI * 2.0));

                        if (endAngle < 0.0)
                        {
                            endAngle += static_cast<float>(M_PI * 2.0);
                        }
                        if (sweepAngle < 0)
                        {
                            std::swap(endAngle, startAngle);
                        }

                        SAL_INFO("drawinglayer.emf", "EMF+\t Adjusted angles: start " <<
                            basegfx::rad2deg(startAngle) << ", end: " << basegfx::rad2deg(endAngle) <<
                            " startAngle: " << startAngle << " sweepAngle: " << sweepAngle);
                        const ::basegfx::B2DPoint centerPoint(dx + 0.5 * dw, dy + 0.5 * dh);
                        ::basegfx::B2DPolygon polygon(
                            ::basegfx::utils::createPolygonFromEllipseSegment(centerPoint,
                                                                              0.5 * dw, 0.5 * dh,
                                                                              startAngle, endAngle));
                        if (type != EmfPlusRecordTypeDrawArc)
                        {
                            polygon.append(centerPoint);
                            polygon.setClosed(true);
                        }
                        ::basegfx::B2DPolyPolygon polyPolygon(polygon);
                        polyPolygon.transform(maMapTransform);
                        if (type == EmfPlusRecordTypeFillPie)
                            EMFPPlusFillPolygon(polyPolygon, flags & 0x8000, brushIndexOrColor);
                        else
                            EMFPPlusDrawPolygon(polyPolygon, flags & 0xff);
                    }
                    break;
                    case EmfPlusRecordTypeFillPath:
                    {
                        sal_uInt32 index = flags & 0xff;
                        sal_uInt32 brushIndexOrColor;
                        rMS.ReadUInt32(brushIndexOrColor);
                        SAL_INFO("drawinglayer.emf", "EMF+ FillPath slot: " << index);

                        EMFPPath* path = dynamic_cast<EMFPPath*>(maEMFPObjects[index].get());
                        if (path)
                            EMFPPlusFillPolygon(path->GetPolygon(*this), flags & 0x8000, brushIndexOrColor);
                        else
                            SAL_WARN("drawinglayer.emf", "EMF+\tEmfPlusRecordTypeFillPath missing path");
                    }
                    break;
                    case EmfPlusRecordTypeFillRegion:
                    {
                        sal_uInt32 index = flags & 0xff;
                        sal_uInt32 brushIndexOrColor;
                        rMS.ReadUInt32(brushIndexOrColor);
                        SAL_INFO("drawinglayer.emf", "EMF+\t FillRegion slot: " << index);

                        EMFPRegion* region = dynamic_cast<EMFPRegion*>(maEMFPObjects[flags & 0xff].get());
                        if (region)
                            EMFPPlusFillPolygon(region->regionPolyPolygon, flags & 0x8000, brushIndexOrColor);
                        else
                            SAL_WARN("drawinglayer.emf", "EMF+\tEmfPlusRecordTypeFillRegion missing region");
                    }
                    break;
                    case EmfPlusRecordTypeDrawEllipse:
                    case EmfPlusRecordTypeFillEllipse:
                    {
                        // Intentionally very bogus initial value to avoid MSVC complaining about potentially uninitialized local
                        // variable. As long as the code stays as intended, this variable will be assigned a (real) value in the case
                        // when it is later used.
                        sal_uInt32 brushIndexOrColor = 1234567;

                        if (type == EmfPlusRecordTypeFillEllipse)
                        {
                            rMS.ReadUInt32(brushIndexOrColor);
                        }

                        SAL_INFO("drawinglayer.emf", "EMF+\t " << (type == EmfPlusRecordTypeFillEllipse ? "Fill" : "Draw") << "Ellipse slot: " << (flags & 0xff));
                        float dx, dy, dw, dh;
                        ReadRectangle(rMS, dx, dy, dw, dh, bool(flags & 0x4000));
                        SAL_INFO("drawinglayer.emf", "EMF+\t RectData: " << dx << "," << dy << " " << dw << "x" << dh);
                        ::basegfx::B2DPolyPolygon polyPolygon(
                            ::basegfx::utils::createPolygonFromEllipse(::basegfx::B2DPoint(dx + 0.5 * dw, dy + 0.5 * dh),
                                                                       0.5 * dw, 0.5 * dh));
                        polyPolygon.transform(maMapTransform);
                        if (type == EmfPlusRecordTypeFillEllipse)
                            EMFPPlusFillPolygon(polyPolygon, flags & 0x8000, brushIndexOrColor);
                        else
                            EMFPPlusDrawPolygon(polyPolygon, flags & 0xff);
                    }
                    break;
                    case EmfPlusRecordTypeFillRects:
                    case EmfPlusRecordTypeDrawRects:
                    {
                        // Silent MSVC warning C4701: potentially uninitialized local variable 'brushIndexOrColor' used
                        sal_uInt32 brushIndexOrColor = 999;
                        ::basegfx::B2DPolyPolygon polyPolygon;
                        sal_uInt32 rectangles;
                        float x, y, width, height;
                        const bool isColor = (flags & 0x8000);
                        ::basegfx::B2DPolygon polygon;

                        if (EmfPlusRecordTypeFillRects == type)
                        {
                            SAL_INFO("drawinglayer.emf", "EMF+\t FillRects");
                            rMS.ReadUInt32(brushIndexOrColor);
                            SAL_INFO("drawinglayer.emf", "EMF+\t" << (isColor ? "color" : "brush index") << ": 0x" << std::hex << brushIndexOrColor << std::dec);
                        }
                        else
                        {
                            SAL_INFO("drawinglayer.emf", "EMF+\t DrawRects");
                        }

                        rMS.ReadUInt32(rectangles);
                        for (sal_uInt32 i = 0; i < rectangles; i++)
                        {
                            ReadRectangle(rMS, x, y, width, height, bool(flags & 0x4000));
                            polygon.clear();
                            polygon.append(Map(x, y));
                            polygon.append(Map(x + width, y));
                            polygon.append(Map(x + width, y + height));
                            polygon.append(Map(x, y + height));
                            polygon.setClosed(true);

                            SAL_INFO("drawinglayer.emf", "EMF+\t\t rectangle: " << x << ", "<< y << " " << width << "x" << height);
                            polyPolygon.append(polygon);
                        }
                        if (type == EmfPlusRecordTypeFillRects)
                            EMFPPlusFillPolygon(polyPolygon, isColor, brushIndexOrColor);
                        else
                            EMFPPlusDrawPolygon(polyPolygon, flags & 0xff);
                        break;
                    }
                    case EmfPlusRecordTypeFillPolygon:
                    {
                        sal_uInt32 brushIndexOrColor, points;

                        rMS.ReadUInt32(brushIndexOrColor);
                        rMS.ReadUInt32(points);
                        SAL_INFO("drawinglayer.emf", "EMF+\t Points: " << points);
                        SAL_INFO("drawinglayer.emf", "EMF+\t " << ((flags & 0x8000) ? "Color" : "Brush index") << " : 0x" << std::hex << brushIndexOrColor << std::dec);

                        EMFPPath path(points, true);
                        path.Read(rMS, flags);

                        EMFPPlusFillPolygon(path.GetPolygon(*this), flags & 0x8000, brushIndexOrColor);
                        break;
                    }
                    case EmfPlusRecordTypeDrawLines:
                    {
                        sal_uInt32 points;
                        rMS.ReadUInt32(points);
                        SAL_INFO("drawinglayer.emf", "EMF+\t Points: " << points);
                        EMFPPath path(points, true);
                        path.Read(rMS, flags);

                        // 0x2000 bit indicates whether to draw an extra line between the last point
                        // and the first point, to close the shape.
                        EMFPPlusDrawPolygon(path.GetPolygon(*this, true, (flags & 0x2000)), flags);

                        break;
                    }
                    case EmfPlusRecordTypeDrawPath:
                    {
                        sal_uInt32 penIndex;
                        rMS.ReadUInt32(penIndex);
                        SAL_INFO("drawinglayer.emf", "EMF+\t Pen: " << penIndex);

                        EMFPPath* path = dynamic_cast<EMFPPath*>(maEMFPObjects[flags & 0xff].get());
                        if (path)
                            EMFPPlusDrawPolygon(path->GetPolygon(*this), penIndex);
                        else
                            SAL_WARN("drawinglayer.emf", "\t\tEmfPlusRecordTypeDrawPath missing path");

                        break;
                    }
                    case EmfPlusRecordTypeDrawBeziers:
                    {
                        sal_uInt32 aCount;
                        float x1, y1, x2, y2, x3, y3, x4, y4;
                        ::basegfx::B2DPolygon aPolygon;
                        rMS.ReadUInt32(aCount);
                        SAL_INFO("drawinglayer.emf", "EMF+\t DrawBeziers slot: " << (flags & 0xff));
                        SAL_INFO("drawinglayer.emf", "EMF+\t Number of points: " << aCount);
                        SAL_WARN_IF((aCount - 1) % 3 != 0, "drawinglayer.emf",
                                    "EMF+\t Bezier Draw not support number of points other than 4, 7, "
                                    "10, 13, 16...");

                        if (aCount < 4)
                        {
                            SAL_WARN("drawinglayer.emf", "EMF+\t Bezier Draw does not support less "
                                                         "than 4 points. Number of points: "
                                                             << aCount);
                            break;
                        }

                        ReadPoint(rMS, x1, y1, flags);
                        // We need to add first starting point
                        aPolygon.append(Map(x1, y1));
                        SAL_INFO("drawinglayer.emf",
                                 "EMF+\t Bezier starting point: " << x1 << "," << y1);
                        for (sal_uInt32 i = 4; i <= aCount; i += 3)
                        {
                            ReadPoint(rMS, x2, y2, flags);
                            ReadPoint(rMS, x3, y3, flags);
                            ReadPoint(rMS, x4, y4, flags);

                            SAL_INFO("drawinglayer.emf",
                                     "EMF+\t Bezier points: " << x2 << "," << y2 << " " << x3 << ","
                                                              << y3 << " " << x4 << "," << y4);
                            aPolygon.appendBezierSegment(Map(x2, y2), Map(x3, y3), Map(x4, y4));
                        }
                        EMFPPlusDrawPolygon(::basegfx::B2DPolyPolygon(aPolygon), flags & 0xff);
                        break;
                    }
                    case EmfPlusRecordTypeDrawCurve:
                    {
                        sal_uInt32 aOffset, aNumSegments, points;
                        float aTension;
                        rMS.ReadFloat(aTension);
                        rMS.ReadUInt32(aOffset);
                        rMS.ReadUInt32(aNumSegments);
                        rMS.ReadUInt32(points);
                        SAL_WARN("drawinglayer.emf",
                                "EMF+\t Tension: " << aTension << " Offset: " << aOffset
                                                   << " NumSegments: " << aNumSegments
                                                   << " Points: " << points);

                        EMFPPath path(points, true);
                        path.Read(rMS, flags);

                        if (points >= 2)
                            EMFPPlusDrawPolygon(
                                path.GetCardinalSpline(*this, aTension, aOffset, aNumSegments),
                                flags & 0xff);
                        else
                            SAL_WARN("drawinglayer.emf", "Not enough number of points");
                        break;
                    }
                    case EmfPlusRecordTypeDrawClosedCurve:
                    case EmfPlusRecordTypeFillClosedCurve:
                    {
                        // Silent MSVC warning C4701: potentially uninitialized local variable 'brushIndexOrColor' used
                        sal_uInt32 brushIndexOrColor = 999, points;
                        float aTension;
                        if (type == EmfPlusRecordTypeFillClosedCurve)
                        {
                            rMS.ReadUInt32(brushIndexOrColor);
                            SAL_INFO(
                                "drawinglayer.emf",
                                "EMF+\t Fill Mode: " << (flags & 0x2000 ? "Winding" : "Alternate"));
                        }
                        rMS.ReadFloat(aTension);
                        rMS.ReadUInt32(points);
                        SAL_WARN("drawinglayer.emf",
                                "EMF+\t Tension: " << aTension << " Points: " << points);
                        SAL_INFO("drawinglayer.emf",
                                "EMF+\t " << (flags & 0x8000 ? "Color" : "Brush index") << " : 0x"
                                        << std::hex << brushIndexOrColor << std::dec);
                        if (points < 3)
                        {
                            SAL_WARN("drawinglayer.emf", "Not enough number of points");
                            break;
                        }
                        EMFPPath path(points, true);
                        path.Read(rMS, flags);
                        if (type == EmfPlusRecordTypeFillClosedCurve)
                            EMFPPlusFillPolygon(path.GetClosedCardinalSpline(*this, aTension),
                                                flags & 0x8000, brushIndexOrColor);
                        else
                            EMFPPlusDrawPolygon(path.GetClosedCardinalSpline(*this, aTension),
                                                flags & 0xff);
                        break;
                    }
                    case EmfPlusRecordTypeDrawImage:
                    case EmfPlusRecordTypeDrawImagePoints:
                    {
                        sal_uInt32 imageAttributesId;
                        sal_Int32 sourceUnit;
                        rMS.ReadUInt32(imageAttributesId).ReadInt32(sourceUnit);
                        SAL_INFO("drawinglayer.emf",
                                "EMF+\t " << (type == EmfPlusRecordTypeDrawImage ? "DrawImage"
                                                                                : "DrawImagePoints")
                                        << " image attributes Id: " << imageAttributesId
                                        << " source unit: " << sourceUnit);
                        SAL_INFO("drawinglayer.emf", "EMF+\t TODO: use image attributes");

                        // Source unit of measurement type must be 1 pixel
                        if (EMFPImage* image = sourceUnit == UnitTypePixel ?
                                dynamic_cast<EMFPImage*>(maEMFPObjects[flags & 0xff].get()) :
                                nullptr)
                        {
                            float sx, sy, sw, sh;
                            ReadRectangle(rMS, sx, sy, sw, sh);

                            ::tools::Rectangle aSource(Point(sx, sy), Size(sw + 1, sh + 1));
                            SAL_INFO("drawinglayer.emf",
                                     "EMF+\t "
                                        << (type == EmfPlusRecordTypeDrawImage ? "DrawImage"
                                                                                : "DrawImagePoints")
                                        << " source rectangle: " << sx << "," << sy << " " << sw << "x"
                                        << sh);

                            float dx(0.), dy(0.), dw(0.), dh(0.);
                            double fShearX = 0.0;
                            double fShearY = 0.0;
                            if (type == EmfPlusRecordTypeDrawImagePoints)
                            {
                                sal_uInt32 aCount;
                                rMS.ReadUInt32(aCount);

                                // Number of points used by DrawImagePoints. Exactly 3 points must be specified.
                                if (aCount != 3)
                                {
                                    SAL_WARN("drawinglayer.emf", "EMF+\t Wrong EMF+ file. Expected "
                                                                "3 points, received: "
                                                                    << aCount);
                                    break;
                                }
                                float x1, y1, x2, y2, x3, y3;

                                ReadPoint(rMS, x1, y1, flags); // upper-left point
                                ReadPoint(rMS, x2, y2, flags); // upper-right
                                ReadPoint(rMS, x3, y3, flags); // lower-left

                                SAL_INFO("drawinglayer.emf", "EMF+\t destination points: "
                                                                << x1 << "," << y1 << " " << x2 << ","
                                                                << y2 << " " << x3 << "," << y3);
                                dx = x1;
                                dy = y2;
                                dw = x2 - x1;
                                dh = y3 - y1;
                                fShearX = x3 - x1;
                                fShearY = y2 - y1;
                            }
                            else if (type == EmfPlusRecordTypeDrawImage)
                                ReadRectangle(rMS, dx, dy, dw, dh, bool(flags & 0x4000));

                            SAL_INFO("drawinglayer.emf",
                                    "EMF+\t Rectangle: " << dx << "," << dy << " " << dw << "x" << dh);
                            Size aSize;
                            if (image->type == ImageDataTypeBitmap)
                            {
                                aSize = image->graphic.GetBitmapEx().GetSizePixel();
                                SAL_INFO("drawinglayer.emf", "EMF+\t Bitmap size: " << aSize.Width()
                                                                                    << "x"
                                                                                    << aSize.Height());
                                if (sx < 0)
                                {
                                    // If src position is negative then we need shift image to right
                                    dx = dx + ((-sx) / sw) * dw;
                                    if (sx + sw <= aSize.Width())
                                        dw = ((sw + sx) / sw) * dw;
                                    else
                                        dw = (aSize.Width() / sw) * dw;
                                }
                                else if (sx + sw > aSize.Width())
                                    // If the src image is smaller that what we want to cut, then we need to scale down
                                    dw = ((aSize.Width() - sx) / sw) * dw;

                                if (sy < 0)
                                {
                                    dy = dy + ((-sy) / sh) * dh;
                                    if (sy + sh <= aSize.Height())
                                        dh = ((sh + sy) / sh) * dh;
                                    else
                                        dh = (aSize.Height() / sh) * dh;
                                }
                                else if (sy + sh > aSize.Height())
                                    dh = ((aSize.Height() - sy) / sh) * dh;
                            }
                            else
                                SAL_INFO(
                                    "drawinglayer.emf",
                                    "EMF+\t TODO: Add support for SrcRect to ImageDataTypeMetafile");
                            const ::basegfx::B2DPoint aDstPoint(dx, dy);
                            const ::basegfx::B2DSize aDstSize(dw, dh);

                            const basegfx::B2DHomMatrix aTransformMatrix
                                = maMapTransform
                                * basegfx::B2DHomMatrix(
                                    /* Row 0, Column 0 */ aDstSize.getWidth(),
                                    /* Row 0, Column 1 */ fShearX,
                                    /* Row 0, Column 2 */ aDstPoint.getX(),
                                    /* Row 1, Column 0 */ fShearY,
                                    /* Row 1, Column 1 */ aDstSize.getHeight(),
                                    /* Row 1, Column 2 */ aDstPoint.getY());

                            if (image->type == ImageDataTypeBitmap)
                            {
                                BitmapEx aBmp(image->graphic.GetBitmapEx());
                                aBmp.Crop(aSource);
                                aSize = aBmp.GetSizePixel();
                                if (aSize.Width() > 0 && aSize.Height() > 0)
                                {
                                    mrTargetHolders.Current().append(
                                        new drawinglayer::primitive2d::BitmapPrimitive2D(
                                            aBmp, aTransformMatrix));
                                }
                                else
                                    SAL_WARN("drawinglayer.emf", "EMF+\t warning: empty bitmap");
                            }
                            else if (image->type == ImageDataTypeMetafile)
                            {
                                GDIMetaFile aGDI(image->graphic.GetGDIMetaFile());
                                aGDI.Clip(aSource);
                                mrTargetHolders.Current().append(
                                    new drawinglayer::primitive2d::MetafilePrimitive2D(aTransformMatrix,
                                                                                    aGDI));
                            }
                        }
                        else
                        {
                            SAL_WARN("drawinglayer.emf",
                                    "EMF+\tDrawImage(Points) Wrong EMF+ file. Only Unit Type Pixel is "
                                    "support by EMF+ specification for DrawImage(Points)");
                        }
                        break;
                    }
                    case EmfPlusRecordTypeDrawString:
                    {
                        sal_uInt32 brushId, formatId, stringLength;
                        rMS.ReadUInt32(brushId).ReadUInt32(formatId).ReadUInt32(stringLength);
                        SAL_INFO("drawinglayer.emf", "EMF+\t FontId: " << OUString::number(flags & 0xFF));
                        SAL_INFO("drawinglayer.emf", "EMF+\t BrushId: " << BrushIDToString(flags, brushId));
                        SAL_INFO("drawinglayer.emf", "EMF+\t FormatId: " << formatId);
                        SAL_INFO("drawinglayer.emf", "EMF+\t Length: " << stringLength);

                        // read the layout rectangle
                        float lx, ly, lw, lh;
                        rMS.ReadFloat(lx).ReadFloat(ly).ReadFloat(lw).ReadFloat(lh);

                        SAL_INFO("drawinglayer.emf", "EMF+\t DrawString layoutRect: " << lx << "," << ly << " - " << lw << "x" << lh);
                        // parse the string
                        const OUString text = read_uInt16s_ToOUString(rMS, stringLength);
                        SAL_INFO("drawinglayer.emf", "EMF+\t DrawString string: " << text);
                        // get the stringFormat from the Object table ( this is OPTIONAL and may be nullptr )
                        const EMFPStringFormat *stringFormat = dynamic_cast<EMFPStringFormat*>(maEMFPObjects[formatId & 0xff].get());
                        // get the font from the flags
                        const EMFPFont *font = dynamic_cast<EMFPFont*>(maEMFPObjects[flags & 0xff].get());
                        if (!font)
                        {
                            break;
                        }
                        mrPropertyHolders.Current().setFont(vcl::Font(font->family, Size(font->emSize, font->emSize)));

                        drawinglayer::attribute::FontAttribute fontAttribute(
                            font->family,                                          // font family
                            u""_ustr,                                                    // (no) font style
                            font->Bold() ? 8u : 1u,                                // weight: 8 = bold
                            font->family == "SYMBOL",                              // symbol
                            stringFormat && stringFormat->DirectionVertical(),     // vertical
                            font->Italic(),                                        // italic
                            false,                                                 // monospaced
                            false,                                                 // outline = false, no such thing in MS-EMFPLUS
                            stringFormat && stringFormat->DirectionRightToLeft(),  // right-to-left
                            false);                                                // BiDiStrong

                        css::lang::Locale locale;
                        double stringAlignmentHorizontalOffset = 0.0;
                        double stringAlignmentVerticalOffset = font->emSize;
                        if (stringFormat)
                        {
                            LanguageTag aLanguageTag(static_cast<LanguageType>(stringFormat->language));
                            locale = aLanguageTag.getLocale();
                            drawinglayer::primitive2d::TextLayouterDevice aTextLayouter;

                            aTextLayouter.setFontAttribute(fontAttribute, font->emSize,
                                font->emSize, locale);

                            double fTextWidth = aTextLayouter.getTextWidth(text, 0, stringLength);
                            SAL_WARN_IF(stringFormat->DirectionRightToLeft(), "drawinglayer.emf",
                                        "EMF+\t DrawString Alignment TODO For a right-to-left layout rectangle, the origin should be at the upper right.");
                            if (stringFormat->stringAlignment == StringAlignmentNear)
                                // Alignment is to the left side of the layout rectangle (lx, ly, lw, lh)
                                stringAlignmentHorizontalOffset = stringFormat->leadingMargin * font->emSize;
                            else if (stringFormat->stringAlignment == StringAlignmentCenter)
                                // Alignment is centered between the origin and extent of the layout rectangle
                                stringAlignmentHorizontalOffset = 0.5 * lw + (stringFormat->leadingMargin - stringFormat->trailingMargin) * font->emSize - 0.5 * fTextWidth;
                            else if (stringFormat->stringAlignment == StringAlignmentFar)
                                // Alignment is to the right side of the layout rectangle
                                stringAlignmentHorizontalOffset = lw - stringFormat->trailingMargin * font->emSize - fTextWidth;

                            if (stringFormat->lineAlign == StringAlignmentNear)
                                stringAlignmentVerticalOffset = font->emSize;
                            else if (stringFormat->lineAlign == StringAlignmentCenter)
                                stringAlignmentVerticalOffset = 0.5 * lh + 0.5 * font->emSize;
                            else if (stringFormat->lineAlign == StringAlignmentFar)
                                stringAlignmentVerticalOffset = lh;
                        }
                        else
                        {
                            // By default LeadingMargin is 1/6 inch
                            // TODO for typographic fonts set value to 0.
                            stringAlignmentHorizontalOffset = 16.0;

                            // use system default
                            locale = Application::GetSettings().GetLanguageTag().getLocale();
                        }

                        const basegfx::B2DHomMatrix transformMatrix = basegfx::utils::createScaleTranslateB2DHomMatrix(
                                    ::basegfx::B2DVector(font->emSize, font->emSize),
                                    ::basegfx::B2DPoint(lx + stringAlignmentHorizontalOffset,
                                                        ly + stringAlignmentVerticalOffset));

                        Color uncorrectedColor = EMFPGetBrushColorOrARGBColor(flags, brushId);
                        Color color;

                        if (mbSetTextContrast)
                        {
                            const auto gammaVal = mnTextContrast / 1000;
                            const basegfx::BColorModifier_gamma gamma(gammaVal);

                            // gamma correct transparency color
                            sal_uInt16 alpha = uncorrectedColor.GetAlpha();
                            alpha = std::clamp(std::pow(alpha, 1.0 / gammaVal), 0.0, 1.0) * 255;

                            basegfx::BColor modifiedColor(gamma.getModifiedColor(uncorrectedColor.getBColor()));
                            color.SetRed(modifiedColor.getRed() * 255);
                            color.SetGreen(modifiedColor.getGreen() * 255);
                            color.SetBlue(modifiedColor.getBlue() * 255);
                            color.SetAlpha(alpha);
                        }
                        else
                        {
                            color = uncorrectedColor;
                        }

                        mrPropertyHolders.Current().setTextColor(color.getBColor());
                        mrPropertyHolders.Current().setTextColorActive(true);

                        if (color.GetAlpha() > 0)
                        {
                            std::vector<double> emptyVector;
                            rtl::Reference<drawinglayer::primitive2d::BasePrimitive2D> pBaseText;
                            if (font->Underline() || font->Strikeout())
                            {
                                pBaseText = new drawinglayer::primitive2d::TextDecoratedPortionPrimitive2D(
                                            transformMatrix,
                                            text,
                                            0,             // text always starts at 0
                                            stringLength,
                                            std::move(emptyVector),   // EMF-PLUS has no DX-array
                                            {},
                                            fontAttribute,
                                            locale,
                                            color.getBColor(), // Font Color
                                            COL_TRANSPARENT,   // Fill Color
                                            color.getBColor(), // OverlineColor
                                            color.getBColor(), // TextlineColor
                                            drawinglayer::primitive2d::TEXT_LINE_NONE,
                                            font->Underline() ? drawinglayer::primitive2d::TEXT_LINE_SINGLE : drawinglayer::primitive2d::TEXT_LINE_NONE,
                                            false,
                                            font->Strikeout() ? drawinglayer::primitive2d::TEXT_STRIKEOUT_SINGLE : drawinglayer::primitive2d::TEXT_STRIKEOUT_NONE);
                            }
                            else
                            {
                                pBaseText = new drawinglayer::primitive2d::TextSimplePortionPrimitive2D(
                                            transformMatrix,
                                            text,
                                            0,             // text always starts at 0
                                            stringLength,
                                            std::move(emptyVector),   // EMF-PLUS has no DX-array
                                            {},
                                            fontAttribute,
                                            locale,
                                            color.getBColor());
                            }
                            drawinglayer::primitive2d::Primitive2DReference aPrimitiveText(pBaseText);
                            if (color.IsTransparent())
                            {
                                aPrimitiveText = new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(
                                            drawinglayer::primitive2d::Primitive2DContainer { aPrimitiveText },
                                            (255 - color.GetAlpha()) / 255.0);
                            }

                            mrTargetHolders.Current().append(
                                        new drawinglayer::primitive2d::TransformPrimitive2D(
                                            maMapTransform,
                                            drawinglayer::primitive2d::Primitive2DContainer { aPrimitiveText } ));
                        }
                        break;
                    }
                    case EmfPlusRecordTypeSetPageTransform:
                    {
                        rMS.ReadFloat(mfPageScale);
                        SAL_INFO("drawinglayer.emf", "EMF+\t Scale: " << mfPageScale << " unit: " << UnitTypeToString(flags));

                        if ((flags == UnitTypeDisplay) || (flags == UnitTypeWorld))
                        {
                            SAL_WARN("drawinglayer.emf", "EMF+\t file error. UnitTypeDisplay and UnitTypeWorld are not supported by SetPageTransform in EMF+ specification.");
                        }
                        else
                        {
                            mnMmX = std::round(unitToPixel(static_cast<double>(mnMmX) * mfPageScale, flags, Direction::horizontal));
                            mnMmY = std::round(unitToPixel(static_cast<double>(mnMmY) * mfPageScale, flags, Direction::vertical));
                            mappingChanged();
                        }
                        break;
                    }
                    case EmfPlusRecordTypeSetRenderingOrigin:
                    {
                        rMS.ReadInt32(mnOriginX).ReadInt32(mnOriginY);
                        SAL_INFO("drawinglayer.emf", "EMF+\t SetRenderingOrigin, [x,y]: " << mnOriginX << "," << mnOriginY);
                        break;
                    }
                    case EmfPlusRecordTypeSetTextContrast:
                    {
                        const sal_uInt16 LOWERGAMMA = 1000;
                        const sal_uInt16 UPPERGAMMA = 2200;

                        mbSetTextContrast = true;
                        mnTextContrast = flags & 0xFFF;
                        SAL_WARN_IF(mnTextContrast > UPPERGAMMA || mnTextContrast < LOWERGAMMA,
                            "drawinglayer.emf", "EMF+\t Gamma value is not with bounds 1000 to 2200, value is " << mnTextContrast);
                        mnTextContrast = std::min(mnTextContrast, UPPERGAMMA);
                        mnTextContrast = std::max(mnTextContrast, LOWERGAMMA);
                        SAL_INFO("drawinglayer.emf", "EMF+\t Text contrast: " << (mnTextContrast / 1000) << " gamma");
                        break;
                    }
                    case EmfPlusRecordTypeSetTextRenderingHint:
                    {
                        sal_uInt8 nTextRenderingHint = (flags & 0xFF) >> 1;
                        SAL_INFO("drawinglayer.emf", "EMF+\t Text rendering hint: " << TextRenderingHintToString(nTextRenderingHint));
                        SAL_WARN("drawinglayer.emf", "EMF+\t TODO SetTextRenderingHint");
                        break;
                    }
                    case EmfPlusRecordTypeSetAntiAliasMode:
                    {
                        bool bUseAntiAlias = (flags & 0x0001);
                        sal_uInt8 nSmoothingMode = (flags & 0xFE00) >> 1;
                        SAL_INFO("drawinglayer.emf", "EMF+\t Antialiasing: " << (bUseAntiAlias ? "enabled" : "disabled"));
                        SAL_INFO("drawinglayer.emf", "EMF+\t Smoothing mode: " << SmoothingModeToString(nSmoothingMode));
                        SAL_WARN("drawinglayer.emf", "EMF+\t TODO SetAntiAliasMode");
                        break;
                    }
                    case EmfPlusRecordTypeSetInterpolationMode:
                    {
                        sal_uInt16 nInterpolationMode = flags & 0xFF;
                        SAL_INFO("drawinglayer.emf", "EMF+\t Interpolation mode: " << InterpolationModeToString(nInterpolationMode));
                        SAL_WARN("drawinglayer.emf", "EMF+\t TODO InterpolationMode");
                        break;
                    }
                    case EmfPlusRecordTypeSetPixelOffsetMode:
                    {
                        SAL_INFO("drawinglayer.emf", "EMF+\t Pixel offset mode: " << PixelOffsetModeToString(flags));
                        SAL_WARN("drawinglayer.emf", "EMF+\t TODO SetPixelOffsetMode");
                        break;
                    }
                    case EmfPlusRecordTypeSetCompositingQuality:
                    {
                        SAL_INFO("drawinglayer.emf", "EMF+\t TODO SetCompositingQuality");
                        break;
                    }
                    case EmfPlusRecordTypeSave:
                    {
                        sal_uInt32 stackIndex;
                        rMS.ReadUInt32(stackIndex);
                        SAL_INFO("drawinglayer.emf", "EMF+\t Save stack index: " << stackIndex);

                        GraphicStatePush(mGSStack, stackIndex);

                        break;
                    }
                    case EmfPlusRecordTypeRestore:
                    {
                        sal_uInt32 stackIndex;
                        rMS.ReadUInt32(stackIndex);
                        SAL_INFO("drawinglayer.emf", "EMF+\t Restore stack index: " << stackIndex);

                        GraphicStatePop(mGSStack, stackIndex);
                        break;
                    }
                    case EmfPlusRecordTypeBeginContainer:
                    {
                        float dx, dy, dw, dh;
                        ReadRectangle(rMS, dx, dy, dw, dh);
                        SAL_INFO("drawinglayer.emf", "EMF+\t Dest RectData: " << dx << "," << dy << " " << dw << "x" << dh);

                        float sx, sy, sw, sh;
                        ReadRectangle(rMS, sx, sy, sw, sh);
                        SAL_INFO("drawinglayer.emf", "EMF+\t Source RectData: " << sx << "," << sy << " " << sw << "x" << sh);

                        sal_uInt32 stackIndex;
                        rMS.ReadUInt32(stackIndex);
                        SAL_INFO("drawinglayer.emf", "EMF+\t Begin Container stack index: " << stackIndex << ", PageUnit: " << flags);

                        if ((flags == UnitTypeDisplay) || (flags == UnitTypeWorld))
                        {
                            SAL_WARN("drawinglayer.emf", "EMF+\t file error. UnitTypeDisplay and UnitTypeWorld are not supported by BeginContainer in EMF+ specification.");
                            break;
                        }
                        GraphicStatePush(mGSContainerStack, stackIndex);
                        const basegfx::B2DHomMatrix transform = basegfx::utils::createScaleTranslateB2DHomMatrix(
                            unitToPixel(static_cast<double>(dw) / sw, flags, Direction::horizontal),
                            unitToPixel(static_cast<double>(dh) / sh, flags, Direction::vertical),
                            unitToPixel(static_cast<double>(dx) - sx, flags, Direction::horizontal),
                            unitToPixel(static_cast<double>(dy) - sy, flags, Direction::vertical));
                        maWorldTransform *= transform;
                        mappingChanged();
                        break;
                    }
                    case EmfPlusRecordTypeBeginContainerNoParams:
                    {
                        sal_uInt32 stackIndex;
                        rMS.ReadUInt32(stackIndex);
                        SAL_INFO("drawinglayer.emf", "EMF+\t Begin Container No Params stack index: " << stackIndex);

                        GraphicStatePush(mGSContainerStack, stackIndex);
                        break;
                    }
                    case EmfPlusRecordTypeEndContainer:
                    {
                        sal_uInt32 stackIndex;
                        rMS.ReadUInt32(stackIndex);
                        SAL_INFO("drawinglayer.emf", "EMF+\t End Container stack index: " << stackIndex);

                        GraphicStatePop(mGSContainerStack, stackIndex);
                        break;
                    }
                    case EmfPlusRecordTypeSetWorldTransform:
                    {
                        SAL_INFO("drawinglayer.emf", "EMF+\t SetWorldTransform, Post multiply: " << bool(flags & 0x2000));
                        readXForm(rMS, maWorldTransform);
                        mappingChanged();
                        SAL_INFO("drawinglayer.emf", "EMF+\t\t: " << maWorldTransform);
                        break;
                    }
                    case EmfPlusRecordTypeResetWorldTransform:
                    {
                        maWorldTransform.identity();
                        SAL_INFO("drawinglayer.emf", "EMF+\t World transform: " << maWorldTransform);
                        mappingChanged();
                        break;
                    }
                    case EmfPlusRecordTypeMultiplyWorldTransform:
                    {
                        SAL_INFO("drawinglayer.emf", "EMF+\t MultiplyWorldTransform, post multiply: " << bool(flags & 0x2000));
                        basegfx::B2DHomMatrix transform;
                        readXForm(rMS, transform);

                        SAL_INFO("drawinglayer.emf",
                                 "EMF+\t Transform matrix: " << transform);

                        if (flags & 0x2000)
                        {
                            // post multiply
                            maWorldTransform *= transform;
                        }
                        else
                        {
                            // pre multiply
                            transform *= maWorldTransform;
                            maWorldTransform = transform;
                        }

                        mappingChanged();

                        SAL_INFO("drawinglayer.emf",
                                 "EMF+\t World transform matrix: " << maWorldTransform);
                        break;
                    }
                    case EmfPlusRecordTypeTranslateWorldTransform:
                    {
                        SAL_INFO("drawinglayer.emf", "EMF+\t TranslateWorldTransform, Post multiply: " << bool(flags & 0x2000));

                        basegfx::B2DHomMatrix transform;
                        float eDx, eDy;
                        rMS.ReadFloat(eDx).ReadFloat(eDy);
                        transform.set(0, 2, eDx);
                        transform.set(1, 2, eDy);

                        SAL_INFO("drawinglayer.emf",
                            "EMF+\t Translate matrix: " << transform);

                        if (flags & 0x2000)
                        {
                            // post multiply
                            maWorldTransform *= transform;
                        }
                        else
                        {
                            // pre multiply
                            transform *= maWorldTransform;
                            maWorldTransform = transform;
                        }

                        mappingChanged();

                        SAL_INFO("drawinglayer.emf",
                                 "EMF+\t World transform matrix: " << maWorldTransform);
                        break;
                    }
                    case EmfPlusRecordTypeScaleWorldTransform:
                    {
                        basegfx::B2DHomMatrix transform;
                        float eSx, eSy;
                        rMS.ReadFloat(eSx).ReadFloat(eSy);
                        transform.set(0, 0, eSx);
                        transform.set(1, 1, eSy);

                        SAL_INFO("drawinglayer.emf", "EMF+\t ScaleWorldTransform Sx: " << eSx <<
                                 " Sy: " << eSy << ", Post multiply:" << bool(flags & 0x2000));
                        SAL_INFO("drawinglayer.emf",
                                 "EMF+\t World transform matrix: " << maWorldTransform);

                        if (flags & 0x2000)
                        {
                            // post multiply
                            maWorldTransform *= transform;
                        }
                        else
                        {
                            // pre multiply
                            transform *= maWorldTransform;
                            maWorldTransform = transform;
                        }

                        mappingChanged();

                        SAL_INFO("drawinglayer.emf",
                                 "EMF+\t World transform matrix: " << maWorldTransform);
                        break;
                    }
                    case EmfPlusRecordTypeRotateWorldTransform:
                    {
                        // Angle of rotation in degrees
                        float eAngle;
                        rMS.ReadFloat(eAngle);

                        SAL_INFO("drawinglayer.emf", "EMF+\t RotateWorldTransform Angle: " << eAngle <<
                                 ", post multiply: " << bool(flags & 0x2000));
                        // Skipping flags & 0x2000
                        // For rotation transformation there is no difference between post and pre multiply
                        maWorldTransform.rotate(basegfx::deg2rad(eAngle));
                        mappingChanged();

                        SAL_INFO("drawinglayer.emf",
                                "EMF+\t " << maWorldTransform);
                        break;
                    }
                    case EmfPlusRecordTypeResetClip:
                    {
                        SAL_INFO("drawinglayer.emf", "EMF+ ResetClip");
                        // We don't need to read anything more, as Size needs to be set 0x0000000C
                        // and DataSize must be set to 0.

                        // Resets the current clipping region for the world space to infinity.
                        HandleNewClipRegion(::basegfx::B2DPolyPolygon(), mrTargetHolders, mrPropertyHolders);
                        break;
                    }
                    case EmfPlusRecordTypeSetClipRect:
                    case EmfPlusRecordTypeSetClipPath:
                    case EmfPlusRecordTypeSetClipRegion:
                    {
                        int combineMode = (flags >> 8) & 0xf;
                        ::basegfx::B2DPolyPolygon polyPolygon;
                        if (type == EmfPlusRecordTypeSetClipRect)
                        {
                            SAL_INFO("drawinglayer.emf", "EMF+\t SetClipRect");

                            float dx, dy, dw, dh;
                            ReadRectangle(rMS, dx, dy, dw, dh);
                            SAL_INFO("drawinglayer.emf",
                                    "EMF+\t RectData: " << dx << "," << dy << " " << dw << "x" << dh);
                            ::basegfx::B2DPoint mappedPoint1(Map(dx, dy));
                            ::basegfx::B2DPoint mappedPoint2(Map(dx + dw, dy + dh));

                            polyPolygon
                                = ::basegfx::B2DPolyPolygon(::basegfx::utils::createPolygonFromRect(
                                    ::basegfx::B2DRectangle(mappedPoint1.getX(), mappedPoint1.getY(),
                                                            mappedPoint2.getX(), mappedPoint2.getY())));
                        }
                        else if (type == EmfPlusRecordTypeSetClipPath)
                        {
                            SAL_INFO("drawinglayer.emf", "EMF+\tSetClipPath " << (flags & 0xff));

                            EMFPPath* path = dynamic_cast<EMFPPath*>(maEMFPObjects[flags & 0xff].get());
                            if (!path)
                            {
                                SAL_WARN("drawinglayer.emf",
                                        "EMF+\t TODO Unable to find path in slot: " << (flags & 0xff));
                                break;
                            }
                            polyPolygon = path->GetPolygon(*this);
                        }
                        else if (type == EmfPlusRecordTypeSetClipRegion)
                        {
                            SAL_INFO("drawinglayer.emf", "EMF+\t Region in slot: " << (flags & 0xff));
                            EMFPRegion* region
                                = dynamic_cast<EMFPRegion*>(maEMFPObjects[flags & 0xff].get());
                            if (!region)
                            {
                                SAL_WARN(
                                    "drawinglayer.emf",
                                    "EMF+\t TODO Unable to find region in slot: " << (flags & 0xff));
                                break;
                            }
                            polyPolygon = region->regionPolyPolygon;
                        }
                        SAL_INFO("drawinglayer.emf", "EMF+\t Combine mode: " << combineMode);
                        ::basegfx::B2DPolyPolygon aClippedPolyPolygon;
                        if (mrPropertyHolders.Current().getClipPolyPolygonActive())
                        {
                            aClippedPolyPolygon
                                = combineClip(mrPropertyHolders.Current().getClipPolyPolygon(),
                                            combineMode, polyPolygon);
                        }
                        else
                        {
                            //Combine with infinity
                            switch (combineMode)
                            {
                                case EmfPlusCombineModeReplace:
                                case EmfPlusCombineModeIntersect:
                                {
                                    aClippedPolyPolygon = polyPolygon;
                                    break;
                                }
                                case EmfPlusCombineModeUnion:
                                {
                                    // Disable clipping as the clipping is infinity
                                    aClippedPolyPolygon = ::basegfx::B2DPolyPolygon();
                                    break;
                                }
                                case EmfPlusCombineModeXOR:
                                case EmfPlusCombineModeComplement:
                                {
                                    //TODO It is not correct and it should be fixed
                                    aClippedPolyPolygon = polyPolygon;
                                    break;
                                }
                                case EmfPlusCombineModeExclude:
                                {
                                    //TODO It is not correct and it should be fixed
                                    aClippedPolyPolygon = ::basegfx::B2DPolyPolygon();
                                    break;
                                }
                            }
                        }
                        HandleNewClipRegion(aClippedPolyPolygon, mrTargetHolders, mrPropertyHolders);
                        break;
                    }
                    case EmfPlusRecordTypeOffsetClip:
                    {
                        float dx, dy;
                        rMS.ReadFloat(dx).ReadFloat(dy);
                        SAL_INFO("drawinglayer.emf", "EMF+\tOffset x:" << dx << ", y:" << dy);

                        basegfx::B2DPolyPolygon aPolyPolygon(
                                    mrPropertyHolders.Current().getClipPolyPolygon());

                        SAL_INFO("drawinglayer.emf",
                                 "EMF+\t PolyPolygon before translate: " << aPolyPolygon);

                        basegfx::B2DPoint aOffset = Map(dx, dy);
                        basegfx::B2DHomMatrix transformMatrix;
                        transformMatrix.set(0, 2, aOffset.getX());
                        transformMatrix.set(1, 2, aOffset.getY());
                        aPolyPolygon.transform(transformMatrix);

                        SAL_INFO("drawinglayer.emf",
                                 "EMF+\t PolyPolygon after translate: " << aPolyPolygon <<
                                 ", mapped offset x" << aOffset.getX() << ", mapped offset y" << aOffset.getY());
                        HandleNewClipRegion(aPolyPolygon, mrTargetHolders, mrPropertyHolders);
                        break;
                    }
                    case EmfPlusRecordTypeDrawDriverString:
                    {
                        sal_uInt32 brushIndexOrColor;
                        sal_uInt32 optionFlags;
                        sal_uInt32 hasMatrix;
                        sal_uInt32 glyphsCount;
                        rMS.ReadUInt32(brushIndexOrColor).ReadUInt32(optionFlags).ReadUInt32(hasMatrix).ReadUInt32(glyphsCount);
                        SAL_INFO("drawinglayer.emf", "EMF+\t " << ((flags & 0x8000) ? "Color" : "Brush index") << ": 0x" << std::hex << brushIndexOrColor << std::dec);
                        SAL_INFO("drawinglayer.emf", "EMF+\t Option flags: 0x" << std::hex << optionFlags << std::dec);
                        SAL_INFO("drawinglayer.emf", "EMF+\t Has matrix: " << hasMatrix);
                        SAL_INFO("drawinglayer.emf", "EMF+\t Glyphs: " << glyphsCount);

                        if ((optionFlags & 1) && glyphsCount > 0)
                        {
                            std::unique_ptr<float[]> charsPosX(new float[glyphsCount]);
                            std::unique_ptr<float[]> charsPosY(new float[glyphsCount]);
                            OUString text = read_uInt16s_ToOUString(rMS, glyphsCount);
                            SAL_INFO("drawinglayer.emf", "EMF+\t DrawDriverString string: " << text);

                            for (sal_uInt32 i = 0; i<glyphsCount; i++)
                            {
                                rMS.ReadFloat(charsPosX[i]).ReadFloat(charsPosY[i]);
                                SAL_INFO("drawinglayer.emf", "EMF+\t\t glyphPosition[" << i << "]: " << charsPosX[i] << "," << charsPosY[i]);
                            }

                            basegfx::B2DHomMatrix transform;

                            if (hasMatrix)
                            {
                                readXForm(rMS, transform);
                                SAL_INFO("drawinglayer.emf", "EMF+\tmatrix: " << transform);
                            }

                            // get the font from the flags
                            EMFPFont *font = dynamic_cast<EMFPFont*>(maEMFPObjects[flags & 0xff].get());
                            if (!font)
                            {
                                break;
                            }
                            // done reading

                            drawinglayer::attribute::FontAttribute fontAttribute(
                                font->family,                                    // font family
                                u""_ustr,                                              // (no) font style
                                font->Bold() ? 8u : 1u,                          // weight: 8 = bold
                                font->family == "SYMBOL",                        // symbol
                                optionFlags & 0x2,                               // vertical
                                font->Italic(),                                  // italic
                                false,                                           // monospaced
                                false,                                           // outline = false, no such thing in MS-EMFPLUS
                                false,                                           // right-to-left
                                false);                                          // BiDiStrong

                            const Color color = EMFPGetBrushColorOrARGBColor(flags, brushIndexOrColor);

                            // generate TextSimplePortionPrimitive2Ds or TextDecoratedPortionPrimitive2D
                            // for all portions of text with the same charsPosY values
                            sal_uInt32 pos = 0;
                            while (pos < glyphsCount)
                            {
                                //determine the current length
                                sal_uInt32 aLength = 1;
                                while (pos + aLength < glyphsCount && std::abs( charsPosY[pos + aLength] - charsPosY[pos] ) < std::numeric_limits< float >::epsilon())
                                    aLength++;

                                // generate the DX-Array
                                std::vector<double> aDXArray;
                                for (size_t i = 0; i < aLength - 1; i++)
                                {
                                    aDXArray.push_back(charsPosX[pos + i + 1] - charsPosX[pos]);
                                }
                                // last entry
                                aDXArray.push_back(0);

                                basegfx::B2DHomMatrix transformMatrix = basegfx::utils::createScaleTranslateB2DHomMatrix(
                                            ::basegfx::B2DVector(font->emSize, font->emSize),
                                            ::basegfx::B2DPoint(charsPosX[pos], charsPosY[pos]));
                                if (hasMatrix)
                                    transformMatrix *= transform;
                                if (color.GetAlpha() > 0)
                                {
                                    rtl::Reference<drawinglayer::primitive2d::BasePrimitive2D> pBaseText;
                                    if (font->Underline() || font->Strikeout())
                                    {
                                        pBaseText = new drawinglayer::primitive2d::TextDecoratedPortionPrimitive2D(
                                                    transformMatrix,
                                                    text,
                                                    pos,            // take character at current pos
                                                    aLength,        // use determined length
                                                    std::move(aDXArray),       // generated DXArray
                                                    {},
                                                    fontAttribute,
                                                    Application::GetSettings().GetLanguageTag().getLocale(),
                                                    color.getBColor(),
                                                    COL_TRANSPARENT,
                                                    color.getBColor(),
                                                    color.getBColor(),
                                                    drawinglayer::primitive2d::TEXT_LINE_NONE,
                                                    font->Underline() ? drawinglayer::primitive2d::TEXT_LINE_SINGLE : drawinglayer::primitive2d::TEXT_LINE_NONE,
                                                    false,
                                                    font->Strikeout() ? drawinglayer::primitive2d::TEXT_STRIKEOUT_SINGLE : drawinglayer::primitive2d::TEXT_STRIKEOUT_NONE);
                                    }
                                    else
                                    {
                                        pBaseText = new drawinglayer::primitive2d::TextSimplePortionPrimitive2D(
                                                    transformMatrix,
                                                    text,
                                                    pos,            // take character at current pos
                                                    aLength,        // use determined length
                                                    std::move(aDXArray),       // generated DXArray
                                                    {},
                                                    fontAttribute,
                                                    Application::GetSettings().GetLanguageTag().getLocale(),
                                                    color.getBColor());
                                    }
                                    drawinglayer::primitive2d::Primitive2DReference aPrimitiveText(pBaseText);
                                    if (color.IsTransparent())
                                    {
                                        aPrimitiveText = new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(
                                                    drawinglayer::primitive2d::Primitive2DContainer { aPrimitiveText },
                                                    (255 - color.GetAlpha()) / 255.0);
                                    }
                                    mrTargetHolders.Current().append(
                                                new drawinglayer::primitive2d::TransformPrimitive2D(
                                                    maMapTransform,
                                                    drawinglayer::primitive2d::Primitive2DContainer { aPrimitiveText } ));
                                }

                                // update pos
                                pos += aLength;
                            }
                        }
                        else
                        {
                            SAL_WARN("drawinglayer.emf", "EMF+\tTODO: fonts (non-unicode glyphs chars)");
                        }
                        break;
                    }
                    default:
                    {
                        SAL_WARN("drawinglayer.emf", "EMF+ TODO unhandled record type: 0x" << std::hex << type << std::dec);
                    }
                }
            }

            rMS.Seek(next);

            if (size <= length)
            {
                length -= size;
            }
            else
            {
                SAL_WARN("drawinglayer.emf", "ImplRenderer::processEMFPlus: "
                    "size " << size << " > length " << length);
                length = 0;
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
