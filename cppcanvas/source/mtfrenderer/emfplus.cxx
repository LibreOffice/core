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

#include <memory>
#include <tools/stream.hxx>
#include <vcl/metaact.hxx>
#include <vcl/graphicfilter.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <basegfx/utils/gradienttools.hxx>
#include <basegfx/utils/tools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <vcl/canvastools.hxx>
#include <rtl/ustring.hxx>
#include <i18nlangtag/languagetag.hxx>

#include <com/sun/star/rendering/PanoseWeight.hpp>
#include <com/sun/star/rendering/TexturingMode.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <comphelper/propertysequence.hxx>

#include <bitmapaction.hxx>
#include <implrenderer.hxx>
#include <outdevstate.hxx>
#include <polypolyaction.hxx>
#include <textaction.hxx>

#include <emfpbrush.hxx>
#include <emfpcustomlinecap.hxx>
#include <emfpfont.hxx>
#include <emfpimage.hxx>
#include <emfppath.hxx>
#include <emfppen.hxx>
#include <emfpregion.hxx>
#include <emfpstringformat.hxx>

namespace
{

#define EmfPlusRecordTypeHeader 0x4001
#define EmfPlusRecordTypeEndOfFile 0x4002
//TODO EmfPlusRecordTypeComment 0x4003
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
//TODO EmfPlusRecordTypeFillRegion 0x4013
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
//TODO EmfPlusRecordTypeRotateWorldTransform 0x402F
#define EmfPlusRecordTypeSetPageTransform 0x4030
//TODO EmfPlusRecordTypeResetClip 0x4031
#define EmfPlusRecordTypeSetClipRect 0x4032
#define EmfPlusRecordTypeSetClipPath 0x4033
#define EmfPlusRecordTypeSetClipRegion 0x4034
//TODO EmfPlusRecordTypeOffsetClip 0x4035
#define EmfPlusRecordTypeDrawDriverString 0x4036
//TODO EmfPlusRecordTypeStrokeFillPath 0x4037
//TODO EmfPlusRecordTypeSerializableObject 0x4038
//TODO EmfPlusRecordTypeSetTSGraphics 0x4039
//TODO EmfPlusRecordTypeSetTSClip 0x403A

#define EmfPlusObjectTypeBrush 0x100
#define EmfPlusObjectTypePen 0x200
#define EmfPlusObjectTypePath 0x300
#define EmfPlusObjectTypeRegion 0x400
#define EmfPlusObjectTypeImage 0x500
#define EmfPlusObjectTypeFont 0x600
#define EmfPlusObjectTypeStringFormat 0x700
#define EmfPlusObjectTypeImageAttributes 0x800
#define EmfPlusObjectTypeCustomLineCap 0x900

#define EmfPlusRegionInitialStateInfinite 0x10000003

enum EmfPlusCombineMode
{
    EmfPlusCombineModeReplace = 0x00000000,
    EmfPlusCombineModeIntersect = 0x00000001,
    EmfPlusCombineModeUnion = 0x00000002,
    EmfPlusCombineModeXOR = 0x00000003,
    EmfPlusCombineModeExclude = 0x00000004,
    EmfPlusCombineModeComplement = 0x00000005
};

enum EmfPlusFontStyle
{
    FontStyleBold = 0x00000001,
    FontStyleItalic = 0x00000002,
    FontStyleUnderline = 0x00000004,
    FontStyleStrikeout = 0x00000008
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

const char* emfTypeToName(sal_uInt16 type)
{
    switch(type)
    {
        case EmfPlusRecordTypeHeader: return "EmfPlusRecordTypeHeader";
        case EmfPlusRecordTypeEndOfFile: return "EmfPlusRecordTypeEndOfFile";
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
        case EmfPlusRecordTypeFillPath: return "EmfPlusRecordTypeFillPath";
        case EmfPlusRecordTypeDrawPath: return "EmfPlusRecordTypeDrawPath";
        case EmfPlusRecordTypeDrawBeziers: return "EmfPlusRecordTypeDrawBeziers";
        case EmfPlusRecordTypeDrawImage: return "EmfPlusRecordTypeDrawImage";
        case EmfPlusRecordTypeDrawImagePoints: return "EmfPlusRecordTypeDrawImagePoints";
        case EmfPlusRecordTypeDrawString: return "EmfPlusRecordTypeDrawString";
        case EmfPlusRecordTypeSetRenderingOrigin: return "EmfPlusRecordTypeSetRenderingOrigin";
        case EmfPlusRecordTypeSetAntiAliasMode: return "EmfPlusRecordTypeSetAntiAliasMode";
        case EmfPlusRecordTypeSetTextRenderingHint: return "EmfPlusRecordTypeSetTextRenderingHint";
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
        case EmfPlusRecordTypeSetClipRect: return "EmfPlusRecordTypeSetClipRect";
        case EmfPlusRecordTypeSetClipPath: return "EmfPlusRecordTypeSetClipPath";
        case EmfPlusRecordTypeSetClipRegion: return "EmfPlusRecordTypeSetClipRegion";
        case EmfPlusRecordTypeDrawDriverString: return "EmfPlusRecordTypeDrawDriverString";
    }
    return "";
}

} // anonymous namespace

using namespace ::com::sun::star;
using namespace ::basegfx;

namespace cppcanvas
{
    namespace internal
    {
        void ImplRenderer::ReadRectangle (SvStream& s, float& x, float& y, float &width, float& height, bool bCompressed)
        {
            if (bCompressed) {
                sal_Int16 ix, iy, iw, ih;

                s.ReadInt16( ix ).ReadInt16( iy ).ReadInt16( iw ).ReadInt16( ih );

                x = ix;
                y = iy;
                width = iw;
                height = ih;
            } else
                s.ReadFloat( x ).ReadFloat( y ).ReadFloat( width ).ReadFloat( height );
        }

        void ImplRenderer::ReadPoint (SvStream& s, float& x, float& y, sal_uInt32 flags)
        {
            if (flags & 0x800) {
                // specifies a location in the coordinate space that is relative to
                // the location specified by the previous element in the array. In the case of the first element in
                // PointData, a previous location at coordinates (0,0) is assumed.
                SAL_WARN("cppcanvas.emf", "EMF+\t\t TODO Relative coordinates bit detected. Implement parse EMFPlusPointR");
            }
            if (flags & 0x4000) {
                sal_Int16 ix, iy;

                s.ReadInt16( ix ).ReadInt16( iy );

                x = ix;
                y = iy;
            } else
                s.ReadFloat( x ).ReadFloat( y );
        }

        void ImplRenderer::MapToDevice (double& x, double& y)
        {
            // TODO: other units
            x = 100*nMmX*x/nPixX;
            y = 100*nMmY*y/nPixY;
        }

        ::basegfx::B2DPoint ImplRenderer::Map (double ix, double iy)
        {
            double x, y;

            x = ix*aWorldTransform.eM11 + iy*aWorldTransform.eM21 + aWorldTransform.eDx;
            y = ix*aWorldTransform.eM12 + iy*aWorldTransform.eM22 + aWorldTransform.eDy;

            MapToDevice (x, y);

            x -= nFrameLeft;
            y -= nFrameTop;

            x *= aBaseTransform.eM11;
            y *= aBaseTransform.eM22;

            return ::basegfx::B2DPoint (x, y);
        }

        ::basegfx::B2DSize ImplRenderer::MapSize (double iwidth, double iheight)
        {
            double w, h;

            w = iwidth*aWorldTransform.eM11 + iheight*aWorldTransform.eM21;
            h = iwidth*aWorldTransform.eM12 + iheight*aWorldTransform.eM22;

            MapToDevice (w, h);

            w *= aBaseTransform.eM11;
            h *= aBaseTransform.eM22;

            return ::basegfx::B2DSize (w, h);
        }

#define COLOR(x) \
    vcl::unotools::colorToDoubleSequence( ::Color (0xff - (x >> 24), \
                             (x >> 16) & 0xff, \
                             (x >> 8) & 0xff, \
                             x & 0xff), \
                        rCanvas->getUNOCanvas()->getDevice()->getDeviceColorSpace());

        void ImplRenderer::EMFPPlusFillPolygon (::basegfx::B2DPolyPolygon const & polygon, const ActionFactoryParameters& rParms,
                                                OutDevState& rState, const CanvasSharedPtr& rCanvas, bool isColor, sal_uInt32 brushIndexOrColor)
        {
            ::basegfx::B2DPolyPolygon localPolygon (polygon);

            SAL_INFO("cppcanvas.emf", "EMF+\tfill polygon");

            localPolygon.transform( rState.mapModeTransform );

            std::shared_ptr<Action> pPolyAction;

            if (isColor) {
                SAL_INFO("cppcanvas.emf", "EMF+\t\tcolor fill:0x" << std::hex << brushIndexOrColor << std::dec);
                rState.isFillColorSet = true;
                rState.isLineColorSet = false;

                rState.fillColor = COLOR(brushIndexOrColor);

                pPolyAction = internal::PolyPolyActionFactory::createPolyPolyAction( localPolygon, rParms.mrCanvas, rState );

            } else {
                rState.isFillColorSet = true;
                // extract UseBrush
                EMFPBrush* brush = static_cast<EMFPBrush*>( aObjects [brushIndexOrColor & 0xff].get() );
                SAL_INFO("cppcanvas.emf", "EMF+\tbrush fill slot: " << brushIndexOrColor << " (type: " << (brush ? brush->GetType() : -1) << ")");

                // give up in case something wrong happened
                if( !brush )
                    return;

                rState.isFillColorSet = false;
                rState.isLineColorSet = false;

                if (brush->type == BrushTypeHatchFill)
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
                    rState.isFillColorSet = true;
                    rState.isLineColorSet = false;
                    ::Color fillColor;
                    if (isHatchBlend)
                    {
                        fillColor = brush->solidColor;
                        fillColor.Merge(brush->secondColor, static_cast<sal_uInt8>(255 * blendFactor));
                    }
                    else
                    {
                        fillColor = brush->secondColor;
                    }
                    rState.fillColor = vcl::unotools::colorToDoubleSequence(fillColor, rCanvas->getUNOCanvas()->getDevice()->getDeviceColorSpace());
                    pPolyAction = internal::PolyPolyActionFactory::createPolyPolyAction( localPolygon, rParms.mrCanvas, rState );
                }
                else if (brush->type == BrushTypeTextureFill)
                {
                    SAL_WARN("cppcanvas.emf", "EMF+\tTODO: implement BrushTypeTextureFill brush");
                }
                else if (brush->type == BrushTypePathGradient || brush->type == BrushTypeLinearGradient)
                {
                    if (brush->type == BrushTypePathGradient && !(brush->additionalFlags & 0x1))
                    {
                        SAL_WARN("cppcanvas.emf", "EMF+\t TODO Verify proper displaying of BrushTypePathGradient with flags: " <<  std::hex << brush->additionalFlags << std::dec);
                    }
                    ::basegfx::B2DHomMatrix aTextureTransformation;
                    ::basegfx::B2DHomMatrix aWorldTransformation;
                    ::basegfx::B2DHomMatrix aBaseTransformation;
                    rendering::Texture aTexture;

                    aWorldTransformation.set (0, 0, aWorldTransform.eM11);
                    aWorldTransformation.set (0, 1, aWorldTransform.eM21);
                    aWorldTransformation.set (0, 2, aWorldTransform.eDx);
                    aWorldTransformation.set (1, 0, aWorldTransform.eM12);
                    aWorldTransformation.set (1, 1, aWorldTransform.eM22);
                    aWorldTransformation.set (1, 2, aWorldTransform.eDy);

                    aBaseTransformation.set (0, 0, aBaseTransform.eM11);
                    aBaseTransformation.set (0, 1, aBaseTransform.eM21);
                    aBaseTransformation.set (0, 2, aBaseTransform.eDx);
                    aBaseTransformation.set (1, 0, aBaseTransform.eM12);
                    aBaseTransformation.set (1, 1, aBaseTransform.eM22);
                    aBaseTransformation.set (1, 2, aBaseTransform.eDy);

                    // TODO Verify on example image, why there is shift (-0.5, -0.5)
                    if (brush->type == BrushTypePathGradient && (brush->additionalFlags & 0x1))
                    {
                        aTextureTransformation.translate (-0.5, -0.5);
                    }
                    aTextureTransformation.scale (brush->areaWidth, brush->areaHeight);
                    aTextureTransformation.translate (brush->areaX, brush->areaY);

                    if (brush->hasTransformation) {
                        ::basegfx::B2DHomMatrix aTransformation;

                        aTransformation.set (0, 0, brush->brush_transformation.eM11);
                        aTransformation.set (0, 1, brush->brush_transformation.eM21);
                        aTransformation.set (0, 2, brush->brush_transformation.eDx);
                        aTransformation.set (1, 0, brush->brush_transformation.eM12);
                        aTransformation.set (1, 1, brush->brush_transformation.eM22);
                        aTransformation.set (1, 2, brush->brush_transformation.eDy);

                        aTextureTransformation *= aTransformation;
                    }

                    aTextureTransformation *= aWorldTransformation;
                    aTextureTransformation.scale (100.0*nMmX/nPixX, 100.0*nMmY/nPixY);
                    aTextureTransformation.translate (-nFrameLeft, -nFrameTop);
                    aTextureTransformation *= rState.mapModeTransform;
                    aTextureTransformation *= aBaseTransformation;

                    aTexture.RepeatModeX = rendering::TexturingMode::CLAMP;
                    aTexture.RepeatModeY = rendering::TexturingMode::CLAMP;
                    aTexture.Alpha = 1.0;

                    basegfx::ODFGradientInfo aGradInfo;
                    OUString aGradientService;

                    const uno::Sequence< double > aStartColor(
                            vcl::unotools::colorToDoubleSequence( brush->solidColor,
                                rParms.mrCanvas->getUNOCanvas()->getDevice()->getDeviceColorSpace() ) );
                    const uno::Sequence< double > aEndColor(
                            vcl::unotools::colorToDoubleSequence( brush->secondColor,
                                rParms.mrCanvas->getUNOCanvas()->getDevice()->getDeviceColorSpace() ) );
                    uno::Sequence< uno::Sequence < double > > aColors (2);
                    uno::Sequence< double > aStops (2);

                    if (brush->blendPositions) {
                        SAL_INFO("cppcanvas.emf", "EMF+\t\tuse blend");
                        aColors.realloc (brush->blendPoints);
                        aStops.realloc (brush->blendPoints);
                        int length = aStartColor.getLength ();
                        uno::Sequence< double > aColor (length);

                        assert(length == aEndColor.getLength());
                        for (int i = 0; i < brush->blendPoints; i++) {
                            aStops[i] = brush->blendPositions [i];

                            for (int j = 0; j < length; j++) {
                                if (brush->type == BrushTypeLinearGradient) {
                                    aColor [j] = aStartColor [j]*(1 - brush->blendFactors[i]) + aEndColor [j]*brush->blendFactors[i];
                                } else
                                    aColor [j] = aStartColor [j]*brush->blendFactors[i] + aEndColor [j]*(1 - brush->blendFactors[i]);
                            }

                            aColors[i] = aColor;
                        }
                    } else if (brush->colorblendPositions) {
                        SAL_INFO("cppcanvas.emf", "EMF+\t\tuse color blend");
                        aColors.realloc (brush->colorblendPoints);
                        aStops.realloc (brush->colorblendPoints);

                        for (int i = 0; i < brush->colorblendPoints; i++) {
                            aStops[i] = brush->colorblendPositions [i];
                            aColors[(brush->type == BrushTypeLinearGradient) ? i : brush->colorblendPoints - 1 - i] = vcl::unotools::colorToDoubleSequence( brush->colorblendColors [i],
                                    rParms.mrCanvas->getUNOCanvas()->getDevice()->getDeviceColorSpace() );
                        }
                    } else {
                        aStops[0] = 0.0;
                        aStops[1] = 1.0;

                        if (brush->type == BrushTypeLinearGradient) {
                            aColors[0] = aStartColor;
                            aColors[1] = aEndColor;
                        } else {
                            aColors[1] = aStartColor;
                            aColors[0] = aEndColor;
                        }
                    }

                    SAL_INFO("cppcanvas.emf", "EMF+\t\tset gradient");
                    basegfx::B2DRange aBoundsRectangle (0, 0, 1, 1);
                    if (brush->type == BrushTypeLinearGradient) {
                        aGradientService = "LinearGradient";
                        aGradInfo = basegfx::utils::createLinearODFGradientInfo(
                                aBoundsRectangle,
                                aStops.getLength(),
                                0,
                                0);

                    } else {
                        aGradientService = "EllipticalGradient";
                        aGradInfo = basegfx::utils::createEllipticalODFGradientInfo(
                                aBoundsRectangle,
                                ::basegfx::B2DVector( 0, 0 ),
                                aStops.getLength(),
                                0,
                                0);
                    }

                    uno::Reference< lang::XMultiServiceFactory > xFactory(
                            rParms.mrCanvas->getUNOCanvas()->getDevice()->getParametricPolyPolygonFactory() );

                    if( xFactory.is() ) {
                        uno::Sequence<uno::Any> args(comphelper::InitAnyPropertySequence(
                        {
                            {"Colors", uno::Any(aColors)},
                            {"Stops", uno::Any(aStops)},
                            {"AspectRatio", uno::Any(static_cast<sal_Int32>(1))},
                        }));
                        aTexture.Gradient.set(
                                xFactory->createInstanceWithArguments( aGradientService,
                                    args ),
                                uno::UNO_QUERY);
                    }

                    ::basegfx::unotools::affineMatrixFromHomMatrix( aTexture.AffineTransform,
                            aTextureTransformation );

                    if( aTexture.Gradient.is() )
                        pPolyAction = internal::PolyPolyActionFactory::createPolyPolyAction( localPolygon,
                                        rParms.mrCanvas,
                                        rState,
                                        aTexture );
                }
            }

            if( pPolyAction )
            {
                SAL_INFO("cppcanvas.emf", "EMF+\t\tadd poly action");

                maActions.emplace_back(
                        pPolyAction,
                        rParms.mrCurrActionIndex );

                rParms.mrCurrActionIndex += pPolyAction->getActionCount()-1;
            }
        }

        double ImplRenderer::EMFPPlusDrawLineCap(const ::basegfx::B2DPolygon& rPolygon, double fPolyLength,
                const ::basegfx::B2DPolyPolygon& rLineCap, bool bIsFilled, bool bStart, const rendering::StrokeAttributes& rAttributes,
                const ActionFactoryParameters& rParms, OutDevState& rState)
        {
            if (!rLineCap.count())
                return 0.0;

            // createAreaGeometryForLineStartEnd normalises the arrows height
            // before scaling (i.e. scales down by rPolygon.height), hence
            // we pre-scale it (which means we can avoid changing the logic
            // that would affect arrows rendered outside of EMF+).
            const double fWidth = rAttributes.StrokeWidth*rLineCap.getB2DRange().getWidth();

            // When drawing an outline (as opposed to a filled endCap), we also
            // need to take account that the brush width also adds to the area
            // of the polygon.
            const double fShift = bIsFilled ? 0 : rAttributes.StrokeWidth;
            double fConsumed = 0;
            basegfx::B2DPolyPolygon aArrow(basegfx::utils::createAreaGeometryForLineStartEnd(
                        rPolygon, rLineCap, bStart,
                        fWidth, fPolyLength, 0, &fConsumed, fShift));

            // createAreaGeometryForLineStartEnd from some reason always sets
            // the path as closed, correct it
            aArrow.setClosed(rLineCap.isClosed());

            // If the endcap is filled, we draw ONLY the filling, if it isn't
            // filled we draw ONLY the outline, but never both.
            if (bIsFilled)
            {
                bool bWasFillColorSet = rState.isFillColorSet;
                rState.isFillColorSet = true;
                rState.fillColor = rState.lineColor;
                std::shared_ptr<Action> pAction2(internal::PolyPolyActionFactory::createPolyPolyAction(aArrow, rParms.mrCanvas, rState));
                if (pAction2)
                {
                    maActions.emplace_back(pAction2, rParms.mrCurrActionIndex);
                    rParms.mrCurrActionIndex += pAction2->getActionCount()-1;
                }
                rState.isFillColorSet = bWasFillColorSet;
            }
            else
            {
                std::shared_ptr<Action> pAction(internal::PolyPolyActionFactory::createPolyPolyAction(aArrow, rParms.mrCanvas, rState, rAttributes));
                if (pAction)
                {
                    maActions.emplace_back(pAction, rParms.mrCurrActionIndex);
                    rParms.mrCurrActionIndex += pAction->getActionCount()-1;
                }
            }

            // There isn't any clear definition of how far the line should extend
            // for arrows, however the following values seem to give best results
            // (fConsumed/2 draws the line to the center-point of the endcap
            // for filled caps -- however it is likely this will need to be
            // changed once we start taking baseInset into account).
            if (bIsFilled)
                return fConsumed/2;
            else
                return rAttributes.StrokeWidth;
        }

        void ImplRenderer::EMFPPlusDrawPolygon (const ::basegfx::B2DPolyPolygon& polygon, const ActionFactoryParameters& rParms,
                                                OutDevState& rState, const CanvasSharedPtr& rCanvas, sal_uInt32 penIndex)
        {
            EMFPPen* pen = static_cast<EMFPPen*>( aObjects [penIndex & 0xff].get() );

            SAL_WARN_IF( !pen, "cppcanvas.emf", "emf+ missing pen" );

            if (pen)
            {
                rState.isFillColorSet = false;
                rState.isLineColorSet = true;
                rState.lineColor = vcl::unotools::colorToDoubleSequence (pen->GetColor (),
                                                                           rCanvas->getUNOCanvas ()->getDevice()->getDeviceColorSpace());

                basegfx::B2DPolyPolygon aPolyPolygon(polygon);
                aPolyPolygon.transform(rState.mapModeTransform);
                rendering::StrokeAttributes aCommonAttributes;

                // some attributes are common for the polygon, and the line
                // starts & ends - like the stroke width
                pen->SetStrokeWidth(aCommonAttributes, *this, rState);

                // but eg. dashing has to be additionally set only on the
                // polygon
                rendering::StrokeAttributes aPolygonAttributes(aCommonAttributes);
                pen->SetStrokeAttributes(aPolygonAttributes);

                basegfx::B2DPolyPolygon aFinalPolyPolygon;

                // render line starts & ends if present
                if (!pen->customStartCap && !pen->customEndCap)
                    aFinalPolyPolygon = aPolyPolygon;
                else
                {
                    for (sal_uInt32 i = 0; i < aPolyPolygon.count(); ++i)
                    {
                        basegfx::B2DPolygon aPolygon(aPolyPolygon.getB2DPolygon(i));

                        if (!aPolygon.isClosed())
                        {
                            double fStart = 0.0;
                            double fEnd = 0.0;
                            double fPolyLength = basegfx::utils::getLength(aPolygon);

                            // line start
                            if (pen->customStartCap)
                            {
                                rendering::StrokeAttributes aAttributes(aCommonAttributes);
                                pen->customStartCap->SetAttributes(aAttributes);

                                fStart = EMFPPlusDrawLineCap(aPolygon, fPolyLength, pen->customStartCap->polygon,
                                        pen->customStartCap->mbIsFilled,
                                        true, aAttributes, rParms, rState);
                            }

                            // line end
                            if (pen->customEndCap)
                            {
                                rendering::StrokeAttributes aAttributes(aCommonAttributes);
                                pen->customEndCap->SetAttributes(aAttributes);

                                fEnd = EMFPPlusDrawLineCap(aPolygon, fPolyLength, pen->customEndCap->polygon,
                                        pen->customEndCap->mbIsFilled,
                                        false, aAttributes, rParms, rState);
                            }

                            // build new poly, consume something from the old poly
                            if (fStart != 0.0 || fEnd != 0.0)
                                aPolygon = basegfx::utils::getSnippetAbsolute(aPolygon, fStart, fPolyLength - fEnd, fPolyLength);
                        }

                        aFinalPolyPolygon.append(aPolygon);
                    }
                }

                // finally render the polygon
                std::shared_ptr<Action> pPolyAction(internal::PolyPolyActionFactory::createPolyPolyAction(aFinalPolyPolygon, rParms.mrCanvas, rState, aPolygonAttributes));
                if( pPolyAction )
                {
                    maActions.emplace_back(pPolyAction, rParms.mrCurrActionIndex);
                    rParms.mrCurrActionIndex += pPolyAction->getActionCount()-1;
                }
            }
        }

        void ImplRenderer::processObjectRecord(SvMemoryStream& rObjectStream, sal_uInt16 flags, sal_uInt32 dataSize, bool bUseWholeStream)
        {
            sal_uInt32 index;

            SAL_INFO("cppcanvas.emf", "EMF+ Object slot: " << (flags & 0xff) << " flags: " << (flags & 0xff00));

            index = flags & 0xff;
            if (aObjects [index] != nullptr) {
                aObjects [index].reset();
            }

            switch (flags & 0x7f00) {
            case EmfPlusObjectTypeBrush:
                {
                    std::unique_ptr<EMFPBrush> brush(new EMFPBrush());
                    brush->Read (rObjectStream, *this);
                    aObjects [index] = std::move(brush);

                    break;
                }
            case EmfPlusObjectTypePen:
                {
                    std::unique_ptr<EMFPPen> pen(new EMFPPen ());
                    pen->Read (rObjectStream, *this);
                    aObjects [index] = std::move(pen);

                    break;
                }
            case EmfPlusObjectTypePath: {
                sal_uInt32 header, pathFlags;
                sal_Int32 points;

                rObjectStream.ReadUInt32( header ).ReadInt32( points ).ReadUInt32( pathFlags );

                SAL_INFO("cppcanvas.emf", "EMF+\tpath");
                SAL_INFO("cppcanvas.emf", "EMF+\theader: 0x" << std::hex << header << " points: " << std::dec << points << " additional flags: 0x" << std::hex << pathFlags << std::dec);

                std::unique_ptr<EMFPPath> path(new EMFPPath (points));
                path->Read (rObjectStream, pathFlags, *this);
                aObjects [index] = std::move(path);

                break;
            }
            case EmfPlusObjectTypeRegion: {
                std::unique_ptr<EMFPRegion> region(new EMFPRegion ());
                region->Read (rObjectStream);
                aObjects [index] = std::move(region);
                break;
            }
            case EmfPlusObjectTypeImage:
                {
                    std::unique_ptr<EMFPImage> image(new EMFPImage);
                    image->type = 0;
                    image->width = 0;
                    image->height = 0;
                    image->stride = 0;
                    image->pixelFormat = 0;
                    image->Read (rObjectStream, dataSize, bUseWholeStream);
                    aObjects [index] = std::move(image);

                    break;
                }
            case EmfPlusObjectTypeFont:
                {
                    std::unique_ptr<EMFPFont> font(new EMFPFont);
                    font->emSize = 0;
                    font->sizeUnit = 0;
                    font->fontFlags = 0;
                    font->Read (rObjectStream);
                    aObjects [index] = std::move(font);

                    break;
                }
            case EmfPlusObjectTypeStringFormat:
                {
                    std::unique_ptr<EMFPStringFormat> stringFormat(new EMFPStringFormat());
                    stringFormat->Read (rObjectStream);
                    aObjects [index] = std::move(stringFormat);
                    break;
                }
            case EmfPlusObjectTypeImageAttributes:
                {
                    SAL_INFO("cppcanvas.emf", "EMF+\t Object type 'image attributes' not yet implemented");
                    break;
                }
            case EmfPlusObjectTypeCustomLineCap:
                {
                    SAL_INFO("cppcanvas.emf", "EMF+\t Object type 'custom line cap' not yet implemented");
                    break;
                }
            default:
                SAL_INFO("cppcanvas.emf", "EMF+\tObject unhandled flags: 0x" << std::hex << (flags & 0xff00) << std::dec);
                break;
            }
        }

        double ImplRenderer::setFont (css::rendering::FontRequest& aFontRequest, sal_uInt8 fontObjectId, const ActionFactoryParameters& rParms, OutDevState& rState)
        {
            EMFPFont *font = static_cast< EMFPFont* >( aObjects[ fontObjectId ].get() );

            aFontRequest.FontDescription.FamilyName = font->family;
            double cellSize = font->emSize;
            if( font->fontFlags & FontStyleBold )
            {
                aFontRequest.FontDescription.FontDescription.Weight = rendering::PanoseWeight::BOLD;
            }
            aFontRequest.CellSize = (rState.mapModeTransform*MapSize( cellSize, 0 )).getX();
            rState.xFont = rParms.mrCanvas->getUNOCanvas()->createFont( aFontRequest,
                                               uno::Sequence< beans::PropertyValue >(),
                                               geometry::Matrix2D() );

            return cellSize;
        }

        void ImplRenderer::GraphicStatePush(GraphicStateMap& map, sal_Int32 index, OutDevState const & rState)
        {
            GraphicStateMap::iterator iter = map.find( index );

            if ( iter != map.end() )
            {
                EmfPlusGraphicState state = iter->second;
                map.erase( iter );

                SAL_INFO("cppcanvas.emf", "stack index: " << index << " found and erased");
            }

            EmfPlusGraphicState state;

            state.aWorldTransform = aWorldTransform;
            state.aDevState = rState;

            map[ index ] = state;
        }

        void ImplRenderer::GraphicStatePop(GraphicStateMap& map, sal_Int32 index, OutDevState& rState)
        {
            GraphicStateMap::iterator iter = map.find( index );

            if ( iter != map.end() )
            {
                SAL_INFO("cppcanvas.emf", "stack index: " << index << " found");

                EmfPlusGraphicState state = iter->second;

                aWorldTransform = state.aWorldTransform;
                rState.clip = state.aDevState.clip;
                rState.clipRect = state.aDevState.clipRect;
                rState.xClipPoly = state.aDevState.xClipPoly;
            }
        }

        void ImplRenderer::processEMFPlus( MetaCommentAction const * pAct, const ActionFactoryParameters& rFactoryParms,
                                           OutDevState& rState, const CanvasSharedPtr& rCanvas )
        {
            sal_uInt32 length = pAct->GetDataSize ();
            SvMemoryStream rMF (const_cast<sal_uInt8 *>(pAct->GetData ()), length, StreamMode::READ);

            if (length < 12) {
                SAL_INFO("cppcanvas.emf", "length is less than required header size");
            }

            // 12 is minimal valid EMF+ record size; remaining bytes are padding
            while (length >= 12) {
                sal_uInt16 type, flags;
                sal_uInt32 size, dataSize;
                sal_uInt64 next;

                rMF.ReadUInt16( type ).ReadUInt16( flags ).ReadUInt32( size ).ReadUInt32( dataSize );

                next = rMF.Tell() + ( size - 12 );

                if (size < 12) {
                    SAL_INFO("cppcanvas.emf", "Size field is less than 12 bytes");
                } else if (size > length) {
                    SAL_INFO("cppcanvas.emf", "Size field is greater than bytes left");
                }
                if (dataSize > (size-12)) {
                    SAL_INFO("cppcanvas.emf", "DataSize field is greater than Size-12");
                }

                SAL_INFO("cppcanvas.emf", "EMF+ record size: " << size << " type: " << emfTypeToName(type) << " flags: " << flags << " data size: " << dataSize);

                if (type == EmfPlusRecordTypeObject && ((mbMultipart && (flags & 0x7fff) == (mMFlags & 0x7fff)) || (flags & 0x8000))) {
                    if (!mbMultipart) {
                        mbMultipart = true;
                        mMFlags = flags;
                        mMStream.Seek(0);
                    }

                    OSL_ENSURE(dataSize >= 4, "No room for TotalObjectSize in EmfPlusContinuedObjectRecord");
                    // 1st 4 bytes are TotalObjectSize
                    mMStream.WriteBytes(static_cast<const char *>(rMF.GetData()) + rMF.Tell() + 4, dataSize - 4);
                    SAL_INFO("cppcanvas.emf", "EMF+ read next object part size: " << size << " type: " << type << " flags: " << flags << " data size: " << dataSize);
                } else {
                    if (mbMultipart) {
                        SAL_INFO("cppcanvas.emf", "EMF+ multipart record flags: " << mMFlags);
                        mMStream.Seek (0);
                        processObjectRecord (mMStream, mMFlags, 0, true);
                    }
                    mbMultipart = false;
                }

                if (type != EmfPlusRecordTypeObject || !(flags & 0x8000))
                {
                    switch (type) {
                    case EmfPlusRecordTypeHeader:
                        sal_uInt32 header, version;

                        rMF.ReadUInt32( header ).ReadUInt32( version ).ReadInt32( nHDPI ).ReadInt32( nVDPI );

                        SAL_INFO("cppcanvas.emf", "EMF+ Header");
                        SAL_INFO("cppcanvas.emf", "EMF+\theader: 0x" << std::hex << header << " version: " << std::dec << version << " horizontal DPI: " << nHDPI << " vertical DPI: " << nVDPI << " dual: " << (flags & 1));

                        break;
                    case EmfPlusRecordTypeEndOfFile:
                        SAL_INFO("cppcanvas.emf", "EMF+ EndOfFile");
                        break;
                    case EmfPlusRecordTypeGetDC:
                        SAL_INFO("cppcanvas.emf", "EMF+ GetDC");
                        SAL_INFO("cppcanvas.emf", "EMF+\talready used in svtools wmf/emf filter parser");
                        break;
                    case EmfPlusRecordTypeObject:
                        processObjectRecord (rMF, flags, dataSize);
                        break;
                    case EmfPlusRecordTypeFillPie:
                    case EmfPlusRecordTypeDrawPie:
                    case EmfPlusRecordTypeDrawArc:
                        {
                            float startAngle, sweepAngle;

                            // Silent MSVC warning C4701: potentially uninitialized local variable 'brushIndexOrColor' used
                            sal_uInt32 brushIndexOrColor = 999;

                            if ( type == EmfPlusRecordTypeFillPie )
                            {
                                rMF.ReadUInt32( brushIndexOrColor );
                                SAL_INFO("cppcanvas.emf", "EMF+ FillPie colorOrIndex: " << brushIndexOrColor);
                            }
                            else if ( type == EmfPlusRecordTypeDrawPie )
                            {
                                SAL_INFO("cppcanvas.emf", "EMF+ DrawPie");
                            }
                            else
                            {
                                SAL_INFO("cppcanvas.emf", "EMF+ DrawArc");
                            }
                            rMF.ReadFloat( startAngle ).ReadFloat( sweepAngle );

                            float dx, dy, dw, dh;

                            ReadRectangle (rMF, dx, dy, dw, dh, bool(flags & 0x4000));

                            SAL_INFO("cppcanvas.emf", "EMF+\t RectData: " << dx << "," << dy << " " << dw << "x" << dh);

                            startAngle = 2*M_PI*startAngle/360;
                            sweepAngle = 2*M_PI*sweepAngle/360;

                            B2DPoint mappedCenter (Map (dx + dw/2, dy + dh/2));
                            B2DSize mappedSize( MapSize (dw/2, dh/2));

                            float endAngle = startAngle + sweepAngle;
                            startAngle = fmodf(startAngle, static_cast<float>(M_PI*2));
                            if (startAngle < 0)
                                startAngle += static_cast<float>(M_PI*2);
                            endAngle = fmodf(endAngle, static_cast<float>(M_PI*2));
                            if (endAngle < 0)
                                endAngle += static_cast<float>(M_PI*2);

                            if (sweepAngle < 0)
                                std::swap (endAngle, startAngle);

                            SAL_INFO("cppcanvas.emf", "EMF+\t adjusted angles: start " <<
                                     (360.0*startAngle/M_PI) << ", end: " << (360.0*endAngle/M_PI) <<
                                     " startAngle: " << startAngle << " sweepAngle: " << sweepAngle);

                            B2DPolygon polygon = basegfx::utils::createPolygonFromEllipseSegment (mappedCenter, mappedSize.getX (), mappedSize.getY (), startAngle, endAngle);
                            if ( type != EmfPlusRecordTypeDrawArc )
                            {
                                polygon.append (mappedCenter);
                                polygon.setClosed (true);
                            }

                            B2DPolyPolygon polyPolygon (polygon);
                            if ( type == EmfPlusRecordTypeFillPie )
                                EMFPPlusFillPolygon( polyPolygon,
                                                     rFactoryParms, rState, rCanvas, flags & 0x8000, brushIndexOrColor );
                            else
                                EMFPPlusDrawPolygon( polyPolygon,
                                                     rFactoryParms, rState, rCanvas, flags & 0xff );
                        }
                        break;
                    case EmfPlusRecordTypeFillPath:
                        {
                            sal_uInt32 index = flags & 0xff;
                            sal_uInt32 brushIndexOrColor;

                            rMF.ReadUInt32( brushIndexOrColor );

                            SAL_INFO("cppcanvas.emf", "EMF+ FillPath slot: " << index);

                            EMFPPlusFillPolygon( static_cast<EMFPPath*>( aObjects [index].get())->GetPolygon (*this), rFactoryParms, rState, rCanvas, flags & 0x8000, brushIndexOrColor);
                        }
                        break;
                    case EmfPlusRecordTypeDrawEllipse:
                    case EmfPlusRecordTypeFillEllipse:
                        {
                            // Intentionally very bogus initial value to avoid MSVC complaining about potentially uninitialized local
                            // variable. As long as the code stays as intended, this variable will be assigned a (real) value in the case
                            // when it is later used.
                            sal_uInt32 brushIndexOrColor = 1234567;

                            if ( type == EmfPlusRecordTypeFillEllipse )
                                rMF.ReadUInt32( brushIndexOrColor );

                            SAL_INFO("cppcanvas.emf", "EMF+ " << (type == EmfPlusRecordTypeFillEllipse ? "Fill" : "Draw") << "Ellipse slot: " << (flags & 0xff));

                            float dx, dy, dw, dh;

                            ReadRectangle (rMF, dx, dy, dw, dh, bool(flags & 0x4000));

                            SAL_INFO("cppcanvas.emf", "EMF+ RectData: " << dx << "," << dy << " " << dw << "x" << dh);

                            B2DPoint mappedCenter (Map (dx + dw/2, dy + dh/2));
                            B2DSize mappedSize( MapSize (dw/2, dh/2));

                            ::basegfx::B2DPolyPolygon polyPolygon( ::basegfx::B2DPolygon( ::basegfx::utils::createPolygonFromEllipse( mappedCenter, mappedSize.getX (), mappedSize.getY () ) ) );

                            if ( type == EmfPlusRecordTypeFillEllipse )
                                EMFPPlusFillPolygon( polyPolygon,
                                                     rFactoryParms, rState, rCanvas, flags & 0x8000, brushIndexOrColor );
                            else
                                EMFPPlusDrawPolygon( polyPolygon,
                                                     rFactoryParms, rState, rCanvas, flags & 0xff );
                        }
                        break;
                    case EmfPlusRecordTypeFillRects:
                    case EmfPlusRecordTypeDrawRects:
                        {
                            // Silent MSVC warning C4701: potentially uninitialized local variable 'brushIndexOrColor' used
                            sal_uInt32 brushIndexOrColor = 999;
                            sal_Int32 rectangles;
                            bool isColor = (flags & 0x8000);
                            ::basegfx::B2DPolygon polygon;

                            if ( type == EmfPlusRecordTypeFillRects )
                            {
                                SAL_INFO("cppcanvas.emf", "EMF+ FillRects");
                                rMF.ReadUInt32( brushIndexOrColor );
                                SAL_INFO("cppcanvas.emf", "EMF+\t" << (isColor ? "color" : "brush index") << ": 0x" << std::hex << brushIndexOrColor << std::dec);
                            }
                            else
                            {
                                SAL_INFO("cppcanvas.emf", "EMF+ DrawRects");
                            }

                            rMF.ReadInt32( rectangles );

                            for (int i=0; i < rectangles; i++) {
                                float x, y, width, height;
                                ReadRectangle (rMF, x, y, width, height, bool(flags & 0x4000));

                                polygon.append (Map (x, y));
                                polygon.append (Map (x + width, y));
                                polygon.append (Map (x + width, y + height));
                                polygon.append (Map (x, y + height));
                                polygon.append (Map (x, y));

                                SAL_INFO("cppcanvas.emf", "EMF+\trectangle: " << x << ", " << width << "x" << height);

                                ::basegfx::B2DPolyPolygon polyPolygon (polygon);
                                if( type == EmfPlusRecordTypeFillRects )
                                    EMFPPlusFillPolygon( polyPolygon,
                                                         rFactoryParms, rState, rCanvas, isColor, brushIndexOrColor );
                                else
                                    EMFPPlusDrawPolygon( polyPolygon,
                                                         rFactoryParms, rState, rCanvas, flags & 0xff );
                            }
                            break;
                        }
                    case EmfPlusRecordTypeFillPolygon:
                        {
                            sal_uInt8 index = flags & 0xff;
                            sal_uInt32 brushIndexOrColor;
                            sal_Int32 points;

                            rMF.ReadUInt32( brushIndexOrColor );
                            rMF.ReadInt32( points );

                            SAL_INFO("cppcanvas.emf", "EMF+ FillPolygon in slot: " << +index << " points: " << points);
                            SAL_INFO("cppcanvas.emf", "EMF+\t: " << ((flags & 0x8000) ? "color" : "brush index") << " 0x" << std::hex << brushIndexOrColor << std::dec);

                            EMFPPath path (points, true);
                            path.Read (rMF, flags, *this);

                            EMFPPlusFillPolygon (path.GetPolygon (*this), rFactoryParms, rState, rCanvas, flags & 0x8000, brushIndexOrColor);

                            break;
                        }
                    case EmfPlusRecordTypeDrawLines:
                        {
                            sal_uInt32 points;

                            rMF.ReadUInt32( points );

                            SAL_INFO("cppcanvas.emf", "EMF+ DrawLines in slot: " << (flags & 0xff) << " points: " << points);

                            EMFPPath path (points, true);
                            path.Read (rMF, flags, *this);

                            // 0x2000 bit indicates whether to draw an extra line between the last point
                            // and the first point, to close the shape.
                            EMFPPlusDrawPolygon (path.GetPolygon (*this, true, (flags & 0x2000)), rFactoryParms, rState, rCanvas, flags);

                            break;
                        }
                    case EmfPlusRecordTypeDrawPath:
                        {
                            sal_uInt32 penIndex;

                            rMF.ReadUInt32( penIndex );

                            SAL_INFO("cppcanvas.emf", "EMF+ DrawPath");
                            SAL_INFO("cppcanvas.emf", "EMF+\tpen: " << penIndex);

                            EMFPPath* path = static_cast<EMFPPath*>( aObjects [flags & 0xff].get() );
                            SAL_WARN_IF( !path, "cppcanvas.emf", "EmfPlusRecordTypeDrawPath missing path" );

                            EMFPPlusDrawPolygon (path->GetPolygon (*this), rFactoryParms, rState, rCanvas, penIndex);

                            break;
                        }
                    case EmfPlusRecordTypeDrawBeziers:
                        {
                            sal_uInt32 aCount;
                            float x1, y1, x2, y2, x3, y3, x4, y4;
                            ::basegfx::B2DPoint aStartPoint, aControlPointA, aControlPointB, aEndPoint;
                            ::basegfx::B2DPolygon aPolygon;
                            rMF.ReadUInt32( aCount );

                            SAL_INFO("cppcanvas.emf", "EMF+ DrawBeziers slot: " << (flags & 0xff) << "Number of points: " << aCount);

                            SAL_WARN_IF( ( aCount - 1 ) % 3 != 0, "cppcanvas.emf", "EMF+\t Bezier Draw not support number of points other than 4, 7, 10, 13, 16...");

                            if( aCount < 4 )
                            {
                                SAL_WARN("cppcanvas.emf", "EMF+\t Bezier Draw does not support less than 4 points. Number of points: " << aCount);
                                break;
                            }

                            ReadPoint (rMF, x1, y1, flags);
                            // We need to add first starting point
                            aStartPoint = Map (x1, y1);
                            aPolygon.append( aStartPoint );

                            for( sal_uInt32 i = 4; i <= aCount; i += 3 )
                            {
                                ReadPoint (rMF, x2, y2, flags);
                                ReadPoint (rMF, x3, y3, flags);
                                ReadPoint (rMF, x4, y4, flags);

                                SAL_INFO("cppcanvas.emf", "EMF+\t Bezier points: " << x1 << "," << y1 << " " << x2 << "," << y2 << " " << x3 << "," << y3 << " " << x4 << "," << y4);

                                aStartPoint = Map (x1, y1);
                                aControlPointA = Map (x2, y2);
                                aControlPointB = Map (x3, y3);
                                aEndPoint = Map (x4, y4);

                                ::basegfx::B2DCubicBezier cubicBezier( aStartPoint, aControlPointA, aControlPointB, aEndPoint );
                                cubicBezier.adaptiveSubdivideByDistance( aPolygon, 10.0 );
                                EMFPPlusDrawPolygon( ::basegfx::B2DPolyPolygon( aPolygon ), rFactoryParms,
                                                     rState, rCanvas, flags & 0xff );
                                // The ending coordinate of one Bezier curve is the starting coordinate of the next.
                                x1 = x4;
                                y1 = y4;
                            }
                            break;
                        }
                    case EmfPlusRecordTypeDrawImage:
                    case EmfPlusRecordTypeDrawImagePoints:
                        {
                            sal_uInt32 attrIndex;
                            sal_Int32 sourceUnit;

                            rMF.ReadUInt32( attrIndex ).ReadInt32( sourceUnit );

                            SAL_INFO("cppcanvas.emf", "EMF+ " << (type == EmfPlusRecordTypeDrawImagePoints ? "DrawImagePoints" : "DrawImage") << "attributes index: " << attrIndex << "source unit: " << sourceUnit);
                            SAL_INFO("cppcanvas.emf", "EMF+\tTODO: use image attributes");

                            if (sourceUnit == 2 && aObjects [flags & 0xff]) { // we handle only GraphicsUnit.Pixel now
                                EMFPImage& image = *static_cast<EMFPImage *>( aObjects [flags & 0xff].get());
                                float sx, sy, sw, sh;
                                sal_Int32 aCount;

                                ReadRectangle (rMF, sx, sy, sw, sh);
                                ::tools::Rectangle aSource(Point(sx, sy), Size(sw, sh));

                                SAL_INFO("cppcanvas.emf", "EMF+ " << (type == EmfPlusRecordTypeDrawImagePoints ? "DrawImagePoints" : "DrawImage") << " source rectangle: " << sx << "," << sy << " " << sw << "x" << sh);

                                ::basegfx::B2DPoint aDstPoint;
                                ::basegfx::B2DSize aDstSize;
                                bool bValid = false;

                                if (type == EmfPlusRecordTypeDrawImagePoints) {
                                    rMF.ReadInt32( aCount );

                                    if( aCount == 3) { // TODO: now that we now that this value is count we should support it better
                                        float x1, y1, x2, y2, x3, y3;

                                        ReadPoint (rMF, x1, y1, flags);
                                        ReadPoint (rMF, x2, y2, flags);
                                        ReadPoint (rMF, x3, y3, flags);

                                        SAL_INFO("cppcanvas.emf", "EMF+ destination points: " << x1 << "," << y1 << " " << x2 << "," << y2 << " " << x3 << "," << y3);
                                        SAL_INFO("cppcanvas.emf", "EMF+ destination rectangle: " << x1 << "," << y1 << " " << x2 - x1 << "x" << y3 - y1);

                                        aDstPoint = Map (x1, y1);
                                        aDstSize = MapSize(x2 - x1, y3 - y1);

                                        bValid = true;
                                    }
                                } else if (type == EmfPlusRecordTypeDrawImage) {
                                    float dx, dy, dw, dh;

                                    ReadRectangle (rMF, dx, dy, dw, dh, bool(flags & 0x4000));

                                    SAL_INFO("cppcanvas.emf", "EMF+ destination rectangle: " << dx << "," << dy << " " << dw << "x" << dh);

                                    aDstPoint = Map (dx, dy);
                                    aDstSize = MapSize(dw, dh);

                                    bValid = true;
                                }

                                if (bValid) {
                                    BitmapEx aBmp( image.graphic.GetBitmapEx () );
                                    aBmp.Crop( aSource );

                                    Size aSize( aBmp.GetSizePixel() );
                                    SAL_INFO("cppcanvas.emf", "EMF+ bitmap size: " << aSize.Width() << "x" << aSize.Height());
                                    if( aSize.Width() > 0 && aSize.Height() > 0 ) {
                                        std::shared_ptr<Action> pBmpAction (
                                            internal::BitmapActionFactory::createBitmapAction (
                                                aBmp,
                                                rState.mapModeTransform * aDstPoint,
                                                rState.mapModeTransform * aDstSize,
                                                rCanvas,
                                                rState));

                                        if( pBmpAction ) {
                                            maActions.emplace_back( pBmpAction,
                                                                            rFactoryParms.mrCurrActionIndex );

                                            rFactoryParms.mrCurrActionIndex += pBmpAction->getActionCount()-1;
                                        }
                                    } else {
                                        SAL_INFO("cppcanvas.emf", "EMF+ warning: empty bitmap");
                                    }
                                } else {
                                    SAL_WARN("cppcanvas.emf", "EMF+ DrawImage(Points) TODO (fixme)");
                                }
                            } else {
                                SAL_WARN("cppcanvas.emf", "EMF+ DrawImage(Points) TODO (fixme) - possibly unsupported source units for crop rectangle");
                            }
                            break;
                        }
                    case EmfPlusRecordTypeDrawString:
                        {
                            SAL_INFO("cppcanvas.emf", "EMF+ DrawString");

                            sal_uInt32 brushId;
                            sal_uInt32 formatId;
                            sal_uInt32 stringLength;

                            rMF.ReadUInt32( brushId ).ReadUInt32( formatId ).ReadUInt32( stringLength );
                            SAL_INFO("cppcanvas.emf", "EMF+ DrawString brushId: " << brushId << " formatId: " << formatId << " length: " << stringLength);

                            if (flags & 0x8000) {
                                float lx, ly, lw, lh;

                                rMF.ReadFloat( lx ).ReadFloat( ly ).ReadFloat( lw ).ReadFloat( lh );

                                SAL_INFO("cppcanvas.emf", "EMF+ DrawString layoutRect: " << lx << "," << ly << " - " << lw << "x" << lh);

                                OUString text = read_uInt16s_ToOUString(rMF, stringLength);

                                EMFPStringFormat *stringFormat = static_cast< EMFPStringFormat* >( aObjects[ formatId & 0xff ].get() );
                                css::rendering::FontRequest aFontRequest;
                                if (stringFormat)
                                {
                                    LanguageTag aLanguageTag( static_cast< LanguageType >( stringFormat->language ) );
                                    aFontRequest.Locale = aLanguageTag.getLocale( false );
                                    SAL_INFO("cppcanvas.emf", "EMF+\t\t Font locale, Country:" << aLanguageTag.getCountry() <<" Language:" << aLanguageTag.getLanguage() );
                                }
                                SAL_INFO("cppcanvas.emf", "EMF+\t\t TODO Use all string formatting attributes during drawing");

                                double cellSize = setFont (aFontRequest, flags & 0xff, rFactoryParms, rState);
                                rState.textColor = COLOR( brushId );

                                ::basegfx::B2DPoint point( Map( lx + 0.15*cellSize, ly + cellSize ) );

                                std::shared_ptr<Action> pTextAction(
                                    TextActionFactory::createTextAction(
                                                                        // position is just rough guess for now
                                                                        // we should calculate it exactly from layoutRect or font
                                        vcl::unotools::pointFromB2DPoint ( point ),
                                        ::Size(),
                                        ::Color(),
                                        ::Size(),
                                        ::Color(),
                                        text,
                                        0,
                                        stringLength,
                                        nullptr,
                                        rFactoryParms.mrVDev,
                                        rFactoryParms.mrCanvas,
                                        rState,
                                        rFactoryParms.mrParms,
                                        false ) );
                                if( pTextAction )
                                {
                                    SAL_INFO("cppcanvas.emf", "EMF+\t\tadd text action");

                                    maActions.emplace_back(
                                                                  pTextAction,
                                                                  rFactoryParms.mrCurrActionIndex );

                                    rFactoryParms.mrCurrActionIndex += pTextAction->getActionCount()-1;
                                }
                            } else {
                                SAL_WARN("cppcanvas.emf", "EMF+ DrawString TODO - drawing with brush not yet supported");
                            }
                        }
                        break;

                    case EmfPlusRecordTypeSetPageTransform:
                        {
                            rMF.ReadFloat( fPageScale );

                            SAL_INFO("cppcanvas.emf", "EMF+ SetPageTransform");
                            SAL_INFO("cppcanvas.emf", "EMF+\tscale: " << fPageScale << " unit: " << flags);

                            if (flags != UnitTypePixel)
                                SAL_WARN("cppcanvas.emf", "EMF+\t TODO Only UnitTypePixel is supported. ");
                            else
                            {
                                nMmX *= fPageScale;
                                nMmY *= fPageScale;
                            }
                        }
                        break;
                    case EmfPlusRecordTypeSetRenderingOrigin:
                        rMF.ReadInt32( nOriginX ).ReadInt32( nOriginY );
                        SAL_INFO("cppcanvas.emf", "EMF+ SetRenderingOrigin");
                        SAL_INFO("cppcanvas.emf", "EMF+\torigin [x,y]: " << nOriginX << "," << nOriginY);
                        break;
                    case EmfPlusRecordTypeSetTextRenderingHint:
                        SAL_INFO("cppcanvas.emf", "EMF+ SetTextRenderingHint");
                        SAL_INFO("cppcanvas.emf", "EMF+\tTODO");
                        break;
                    case EmfPlusRecordTypeSetAntiAliasMode:
                        SAL_INFO("cppcanvas.emf", "EMF+ SetAntiAliasMode");
                        SAL_INFO("cppcanvas.emf", "EMF+\tTODO");
                        break;
                    case EmfPlusRecordTypeSetInterpolationMode:
                        SAL_INFO("cppcanvas.emf", "EMF+ InterpolationMode");
                        SAL_INFO("cppcanvas.emf", "EMF+\tTODO");
                        break;
                    case EmfPlusRecordTypeSetPixelOffsetMode:
                        SAL_INFO("cppcanvas.emf", "EMF+ SetPixelOffsetMode");
                        SAL_INFO("cppcanvas.emf", "EMF+\tTODO");
                        break;
                    case EmfPlusRecordTypeSetCompositingQuality:
                        SAL_INFO("cppcanvas.emf", "EMF+ SetCompositingQuality");
                        SAL_INFO("cppcanvas.emf", "EMF+\tTODO");
                        break;
                    case EmfPlusRecordTypeSave:
                    {
                        sal_uInt32 stackIndex;

                        rMF.ReadUInt32( stackIndex );

                        SAL_INFO("cppcanvas.emf", "EMF+ Save stack index: " << stackIndex);

                        GraphicStatePush( mGSStack, stackIndex, rState );

                        break;
                    }
                    case EmfPlusRecordTypeRestore:
                    {
                        sal_uInt32 stackIndex;

                        rMF.ReadUInt32( stackIndex );

                        SAL_INFO("cppcanvas.emf", "EMF+ Restore stack index: " << stackIndex);

                        GraphicStatePop( mGSStack, stackIndex, rState );

                        break;
                    }
                    case EmfPlusRecordTypeBeginContainerNoParams:
                    {
                        sal_uInt32 stackIndex;

                        rMF.ReadUInt32( stackIndex );

                        SAL_INFO("cppcanvas.emf", "EMF+ Begin Container No Params stack index: " << stackIndex);

                        GraphicStatePush( mGSContainerStack, stackIndex, rState );
                    }
                    break;
                    case EmfPlusRecordTypeEndContainer:
                    {
                        sal_uInt32 stackIndex;

                        rMF.ReadUInt32( stackIndex );

                        SAL_INFO("cppcanvas.emf", "EMF+ End Container stack index: " << stackIndex);

                        GraphicStatePop( mGSContainerStack, stackIndex, rState );
                    }
                    break;
                    case EmfPlusRecordTypeSetWorldTransform: {
                        SAL_INFO("cppcanvas.emf", "EMF+ SetWorldTransform");
                        XForm transform;
                        ReadXForm( rMF, transform );
                        aWorldTransform.Set (transform);
                        SAL_INFO("cppcanvas.emf",
                                "EMF+\tm11: " << aWorldTransform.eM11 << "\tm12: " << aWorldTransform.eM12 <<
                                "\tm21: " << aWorldTransform.eM21 << "\tm22: " << aWorldTransform.eM22 <<
                                "\tdx: "  << aWorldTransform.eDx  << "\tdy: "  << aWorldTransform.eDy);
                        break;
                    }
                    case EmfPlusRecordTypeResetWorldTransform:
                        SAL_INFO("cppcanvas.emf", "EMF+ ResetWorldTransform");
                        aWorldTransform.SetIdentity ();
                        break;
                    case EmfPlusRecordTypeMultiplyWorldTransform: {
                        SAL_INFO("cppcanvas.emf", "EMF+ MultiplyWorldTransform");
                        XForm transform;
                        ReadXForm( rMF, transform );

                        SAL_INFO("cppcanvas.emf",
                                "EMF+\tmatrix m11: " << transform.eM11 << "m12: " << transform.eM12 <<
                                "EMF+\tm21: "        << transform.eM21 << "m22: " << transform.eM22 <<
                                "EMF+\tdx: "         << transform.eDx  << "dy: "  << transform.eDy);

                        if (flags & 0x2000)  // post multiply
                            aWorldTransform.Multiply (transform);
                        else {               // pre multiply
                            transform.Multiply (aWorldTransform);
                            aWorldTransform.Set (transform);
                        }
                        SAL_INFO("cppcanvas.emf",
                                "EMF+\tm11: " << aWorldTransform.eM11 << "m12: " << aWorldTransform.eM12 <<
                                "EMF+\tm21: " << aWorldTransform.eM21 << "m22: " << aWorldTransform.eM22 <<
                                "EMF+\tdx: "  << aWorldTransform.eDx  << "dy: "  << aWorldTransform.eDy);
                        break;
                    }
                    case EmfPlusRecordTypeTranslateWorldTransform:
                        {
                            SAL_INFO("cppcanvas.emf", "EMF+ TranslateWorldTransform");

                            XForm transform = XForm();
                            rMF.ReadFloat( transform.eDx ).ReadFloat( transform.eDy );

                            SAL_INFO("cppcanvas.emf",
                                     "EMF+\t m11: " << transform.eM11 << ", m12: " << transform.eM12 <<
                                     "EMF+\t m21: " << transform.eM21 << ", m22: " << transform.eM22 <<
                                     "EMF+\t dx: "  << transform.eDx  << ", dy: "  << transform.eDy);

                            if (flags & 0x2000)  // post multiply
                                aWorldTransform.Multiply (transform);
                            else {               // pre multiply
                                transform.Multiply (aWorldTransform);
                                aWorldTransform.Set (transform);
                            }
                            SAL_INFO("cppcanvas.emf",
                                     "EMF+\t m11: " << aWorldTransform.eM11 << ", m12: " << aWorldTransform.eM12 <<
                                     "EMF+\t m21: " << aWorldTransform.eM21 << ", m22: " << aWorldTransform.eM22 <<
                                     "EMF+\t dx: "  << aWorldTransform.eDx  << ", dy: "  << aWorldTransform.eDy);
                            break;
                        }
                    case EmfPlusRecordTypeScaleWorldTransform:
                        {
                            XForm transform = XForm();
                            rMF.ReadFloat( transform.eM11 ).ReadFloat( transform.eM22 );
                            SAL_INFO("cppcanvas.emf", "EMF+ ScaleWorldTransform Sx: " << transform.eM11 << " Sy: " << transform.eM22);

                            SAL_INFO("cppcanvas.emf",
                                     "EMF+\t m11: " << aWorldTransform.eM11 << ", m12: " << aWorldTransform.eM12 <<
                                     "EMF+\t m21: " << aWorldTransform.eM21 << ", m22: " << aWorldTransform.eM22 <<
                                     "EMF+\t dx: "  << aWorldTransform.eDx  << ", dy: "  << aWorldTransform.eDy);

                            if (flags & 0x2000)  // post multiply
                                aWorldTransform.Multiply (transform);
                            else {               // pre multiply
                                transform.Multiply (aWorldTransform);
                                aWorldTransform.Set (transform);
                            }
                            SAL_INFO("cppcanvas.emf",
                                     "EMF+\t m11: " << aWorldTransform.eM11 << ", m12: " << aWorldTransform.eM12 <<
                                     "EMF+\t m21: " << aWorldTransform.eM21 << ", m22: " << aWorldTransform.eM22 <<
                                     "EMF+\t dx: "  << aWorldTransform.eDx  << ", dy: "  << aWorldTransform.eDy);
                            break;
                        }
                    case EmfPlusRecordTypeSetClipRect:
                        {
                            int combineMode = (flags >> 8) & 0xf;

                            SAL_INFO("cppcanvas.emf", "EMF+ SetClipRect combine mode: " << combineMode);
#if OSL_DEBUG_LEVEL > 1
                            if (combineMode > 1) {
                                SAL_INFO ("cppcanvas.emf", "EMF+ TODO combine mode > 1");
                            }
#endif

                            float dx, dy, dw, dh;

                            ReadRectangle (rMF, dx, dy, dw, dh);

                            SAL_INFO("cppcanvas.emf", "EMF+ RectData: " << dx << "," << dy << " " << dw << "x" << dh);

                            B2DPoint mappedPoint (Map (dx, dy));
                            B2DSize mappedSize( MapSize (dw, dh));

                            ::basegfx::B2DPolyPolygon polyPolygon( ::basegfx::B2DPolygon( ::basegfx::utils::createPolygonFromRect( ::basegfx::B2DRectangle( mappedPoint.getX(), mappedPoint.getY(),
                                                                                                                                                            mappedPoint.getX() + mappedSize.getX(),
                                                                                                                                                            mappedPoint.getY() + mappedSize.getY() ) ) ) );
                            polyPolygon.transform(rState.mapModeTransform);

                            updateClipping (polyPolygon, rFactoryParms, combineMode == 1);

                            break;
                        }
                    case EmfPlusRecordTypeSetClipPath:
                        {
                            int combineMode = (flags >> 8) & 0xf;

                            SAL_INFO("cppcanvas.emf", "EMF+ SetClipPath combine mode: " << combineMode);
                            SAL_INFO("cppcanvas.emf", "EMF+\tpath in slot: " << (flags & 0xff));

                            EMFPPath& path = *static_cast<EMFPPath*>( aObjects [flags & 0xff].get() );
                            ::basegfx::B2DPolyPolygon& clipPoly (path.GetPolygon (*this));

                            clipPoly.transform (rState.mapModeTransform);
                            switch (combineMode)
                            {
                            case EmfPlusCombineModeReplace:
                            case EmfPlusCombineModeIntersect:
                            case EmfPlusCombineModeUnion: // Is this, EmfPlusCombineModeXOR and EmfPlusCombineModeComplement correct?
                            case EmfPlusCombineModeXOR:
                            case EmfPlusCombineModeComplement:
                                updateClipping (clipPoly, rFactoryParms, combineMode == 1);
                                break;
                            case EmfPlusCombineModeExclude:
                                // Not doing anything is better then including exactly what we wanted to exclude.
                                break;
                            }

                            break;
                        }
                    case EmfPlusRecordTypeSetClipRegion: {
                        int combineMode = (flags >> 8) & 0xf;

                        SAL_INFO("cppcanvas.emf", "EMF+ SetClipRegion");
                        SAL_INFO("cppcanvas.emf", "EMF+\tregion in slot: " << (flags & 0xff) << " combine mode: " << combineMode);
                        EMFPRegion *region = static_cast<EMFPRegion*>(aObjects [flags & 0xff].get());

                        // reset clip
                        if (region && region->parts == 0 && region->initialState == EmfPlusRegionInitialStateInfinite) {
                            updateClipping (::basegfx::B2DPolyPolygon (), rFactoryParms, combineMode == 1);
                        } else {
                            SAL_INFO("cppcanvas.emf", "EMF+\tTODO");
                        }
                        break;
                    }
                    case EmfPlusRecordTypeDrawDriverString: {
                        SAL_INFO("cppcanvas.emf", "EMF+ DrawDriverString, flags: 0x" << std::hex << flags << std::dec);
                        sal_uInt32 brushIndexOrColor;
                        sal_uInt32 optionFlags;
                        sal_uInt32 hasMatrix;
                        sal_uInt32 glyphsCount;

                        rMF.ReadUInt32( brushIndexOrColor ).ReadUInt32( optionFlags ).ReadUInt32( hasMatrix ).ReadUInt32( glyphsCount );

                        SAL_INFO("cppcanvas.emf", "EMF+\t: " << ((flags & 0x8000) ? "color" : "brush index") << " 0x" << std::hex << brushIndexOrColor << std::dec);
                        SAL_INFO("cppcanvas.emf", "EMF+\toption flags: 0x" << std::hex << optionFlags << std::dec);
                        SAL_INFO("cppcanvas.emf", "EMF+\thas matrix: " << hasMatrix);
                        SAL_INFO("cppcanvas.emf", "EMF+\tglyphs: " << glyphsCount);

                        if( ( optionFlags & 1 ) && glyphsCount > 0 ) {
                            std::unique_ptr<float[]> charsPosX( new float[glyphsCount] );
                            std::unique_ptr<float[]> charsPosY( new float[glyphsCount] );

                            OUString text = read_uInt16s_ToOUString(rMF, glyphsCount);

                            for( sal_uInt32 i=0; i<glyphsCount; i++) {
                                rMF.ReadFloat( charsPosX[i] ).ReadFloat( charsPosY[i] );
                                SAL_INFO("cppcanvas.emf", "EMF+\tglyphPosition[" << i << "]: " << charsPosX[i] << "," << charsPosY[i]);
                            }

                            XForm transform;
                            if( hasMatrix ) {
                                ReadXForm( rMF, transform );
                                SAL_INFO("cppcanvas.emf", "EMF+\tmatrix: " << transform.eM11 << ", " << transform.eM12 << ", " << transform.eM21 << ", " << transform.eM22 << ", " << transform.eDx << ", " << transform.eDy);
                            }

                            rendering::FontRequest aFontRequest;
                            // add the text action
                            setFont (aFontRequest, flags & 0xff, rFactoryParms, rState);

                            if( flags & 0x8000 )
                                rState.textColor = COLOR( brushIndexOrColor );

                            ::basegfx::B2DPoint point( Map( charsPosX[0], charsPosY[0] ) );

                            std::shared_ptr<Action> pTextAction(
                                    TextActionFactory::createTextAction(
                                        vcl::unotools::pointFromB2DPoint ( point ),
                                        ::Size(),
                                        ::Color(),
                                        ::Size(),
                                        ::Color(),
                                        text,
                                        0,
                                        glyphsCount,
                                        nullptr,
                                        rFactoryParms.mrVDev,
                                        rFactoryParms.mrCanvas,
                                        rState,
                                        rFactoryParms.mrParms,
                                        false ) );

                            if( pTextAction )
                            {
                                SAL_INFO("cppcanvas.emf", "EMF+\t\tadd text action");

                                maActions.emplace_back(
                                            pTextAction,
                                            rFactoryParms.mrCurrActionIndex );

                                rFactoryParms.mrCurrActionIndex += pTextAction->getActionCount()-1;
                            }
                        } else {
                            SAL_WARN("cppcanvas.emf", "EMF+\tTODO: fonts (non-unicode glyphs chars)");
                        }

                        break;
                    }
                    default:
                        SAL_WARN("cppcanvas.emf", "EMF+ TODO unhandled record type: 0x" << std::hex << type << std::dec);
                    }
                }

                rMF.Seek (next);

                if (size <= length)
                {
                    length -= size;
                }
                else
                {
                    SAL_WARN("cppcanvas.emf", "ImplRenderer::processEMFPlus: "
                            "size " << size << " > length " << length);
                    length = 0;
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
