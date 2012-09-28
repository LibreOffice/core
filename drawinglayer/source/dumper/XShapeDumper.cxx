/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Artur Dorda <artur.dorda+libo@gmail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <drawinglayer/XShapeDumper.hxx>
#include "EnhancedShapeDumper.hxx"
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XText.hpp>
#include <rtl/strbuf.hxx>
#include <libxml/xmlwriter.h>
#include <iostream>
#include <rtl/ustring.hxx>

#define DEBUG_DUMPER 0

using namespace com::sun::star;
//class XShapeDumper

XShapeDumper::XShapeDumper()
{

}

namespace {

// FUNCTION DECLARATIONS

// auxiliary functions
void dumpGradientProperty(com::sun::star::awt::Gradient aGradient, xmlTextWriterPtr xmlWriter);
void dumpPolyPolygonBezierCoords(com::sun::star::drawing::PolyPolygonBezierCoords aPolyPolygonBezierCoords, xmlTextWriterPtr xmlWriter);
void dumpPointSequenceSequence(com::sun::star::drawing::PointSequenceSequence aPointSequenceSequence, uno::Sequence<uno::Sequence<drawing::PolygonFlags> >*, xmlTextWriterPtr xmlWriter);

// FillProperties.idl
void dumpFillStyleAsAttribute(com::sun::star::drawing::FillStyle eFillStyle, xmlTextWriterPtr xmlWriter);
void dumpFillColorAsAttribute(sal_Int32 aColor, xmlTextWriterPtr xmlWriter);
void dumpFillTransparenceAsAttribute(sal_Int32 aTransparence, xmlTextWriterPtr xmlWriter);
void dumpFillTransparenceGradientNameAsAttribute(rtl::OUString sTranspGradName, xmlTextWriterPtr xmlWriter);
void dumpFillTransparenceGradientAsElement(com::sun::star::awt::Gradient aTranspGrad, xmlTextWriterPtr xmlWriter);
void dumpFillGradientNameAsAttribute(rtl::OUString sGradName, xmlTextWriterPtr xmlWriter);
void dumpFillGradientAsElement(com::sun::star::awt::Gradient aGradient, xmlTextWriterPtr xmlWriter);
void dumpFillHatchAsElement(com::sun::star::drawing::Hatch aHatch, xmlTextWriterPtr xmlWriter);
void dumpFillBackgroundAsAttribute(sal_Bool bBackground, xmlTextWriterPtr xmlWriter);
void dumpFillBitmapAsElement(com::sun::star::uno::Reference<com::sun::star::awt::XBitmap> xBitmap, xmlTextWriterPtr xmlWriter);
void dumpFillBitmapURLAsAttribute(rtl::OUString sBitmapURL, xmlTextWriterPtr xmlWriter);
void dumpFillBitmapPositionOffsetXAsAttribute(sal_Int32 aBitmapPositionOffsetX, xmlTextWriterPtr xmlWriter);
void dumpFillBitmapPositionOffsetYAsAttribute(sal_Int32 aBitmapPositionOffsetY, xmlTextWriterPtr xmlWriter);
void dumpFillBitmapOffsetXAsAttribute(sal_Int32 aBitmapOffsetX, xmlTextWriterPtr xmlWriter);
void dumpFillBitmapOffsetYAsAttribute(sal_Int32 aBitmapOffsetY, xmlTextWriterPtr xmlWriter);
void dumpFillBitmapRectanglePointAsAttribute(com::sun::star::drawing::RectanglePoint eBitmapRectanglePoint, xmlTextWriterPtr xmlWriter);
void dumpFillBitmapLogicalSizeAsAttribute(sal_Bool bBitmapLogicalSize, xmlTextWriterPtr xmlWriter);
void dumpFillBitmapSizeXAsAttribute(sal_Int32 aBitmapSizeX, xmlTextWriterPtr xmlWriter);
void dumpFillBitmapSizeYAsAttribute(sal_Int32 aBitmapSizeY, xmlTextWriterPtr xmlWriter);
void dumpFillBitmapModeAsAttribute(com::sun::star::drawing::BitmapMode eBitmapMode, xmlTextWriterPtr xmlWriter);
void dumpFillBitmapStretchAsAttribute(sal_Bool bBitmapStretch, xmlTextWriterPtr xmlWriter);
void dumpFillBitmapTileAsAttribute(sal_Bool bBitmapTile, xmlTextWriterPtr xmlWriter);

// LineProperties.idl
void dumpLineStyleAsAttribute(com::sun::star::drawing::LineStyle eLineStyle, xmlTextWriterPtr xmlWriter);
void dumpLineDashAsElement(com::sun::star::drawing::LineDash aLineDash, xmlTextWriterPtr xmlWriter);
void dumpLineDashNameAsAttribute(rtl::OUString sLineDashName, xmlTextWriterPtr xmlWriter);
void dumpLineColorAsAttribute(sal_Int32 aLineColor, xmlTextWriterPtr xmlWriter);
void dumpLineTransparenceAsAttribute(sal_Int32 aLineTransparence, xmlTextWriterPtr xmlWriter);
void dumpLineWidthAsAttribute(sal_Int32 aLineWidth, xmlTextWriterPtr xmlWriter);
void dumpLineJointAsAttribute(com::sun::star::drawing::LineJoint eLineJoint, xmlTextWriterPtr xmlWriter);
void dumpLineStartNameAsAttribute(rtl::OUString sLineStartName, xmlTextWriterPtr xmlWriter);
void dumpLineEndNameAsAttribute(rtl::OUString sLineEndName, xmlTextWriterPtr xmlWriter);
void dumpLineStartAsElement(com::sun::star::drawing::PolyPolygonBezierCoords aLineStart, xmlTextWriterPtr xmlWriter);
void dumpLineEndAsElement(com::sun::star::drawing::PolyPolygonBezierCoords aLineEnd, xmlTextWriterPtr xmlWriter);
void dumpLineStartCenterAsAttribute(sal_Bool bLineStartCenter, xmlTextWriterPtr xmlWriter);
void dumpLineStartWidthAsAttribute(sal_Int32 aLineStartWidth, xmlTextWriterPtr xmlWriter);
void dumpLineEndCenterAsAttribute(sal_Bool bLineEndCenter, xmlTextWriterPtr xmlWriter);
void dumpLineEndWidthAsAttribute(sal_Int32 aLineEndWidth, xmlTextWriterPtr xmlWriter);

// PolyPolygonDescriptor.idl
void dumpPolygonKindAsAttribute(com::sun::star::drawing::PolygonKind ePolygonKind, xmlTextWriterPtr xmlWriter);
void dumpPolyPolygonAsElement(com::sun::star::drawing::PointSequenceSequence aPolyPolygon, xmlTextWriterPtr xmlWriter);
void dumpGeometryAsElement(com::sun::star::drawing::PointSequenceSequence aGeometry, xmlTextWriterPtr xmlWriter);

// TextProperties.idl
void dumpIsNumberingAsAttribute(sal_Bool bIsNumbering, xmlTextWriterPtr xmlWriter);
void dumpTextAutoGrowHeightAsAttribute(sal_Bool bTextAutoGrowHeight, xmlTextWriterPtr xmlWriter);
void dumpTextAutoGrowWidthAsAttribute(sal_Bool bTextAutoGrowWidth, xmlTextWriterPtr xmlWriter);
void dumpTextContourFrameAsAttribute(sal_Bool bTextContourFrame, xmlTextWriterPtr xmlWriter);
void dumpTextFitToSizeAsAttribute(com::sun::star::drawing::TextFitToSizeType eTextFitToSize, xmlTextWriterPtr xmlWriter);
void dumpTextHorizontalAdjustAsAttribute(com::sun::star::drawing::TextHorizontalAdjust eTextHorizontalAdjust, xmlTextWriterPtr xmlWriter);
void dumpTextVerticalAdjustAsAttribute(com::sun::star::drawing::TextVerticalAdjust eTextVerticalAdjust, xmlTextWriterPtr xmlWriter);
void dumpTextLeftDistanceAsAttribute(sal_Int32 aTextLeftDistance, xmlTextWriterPtr xmlWriter);
void dumpTextRightDistanceAsAttribute(sal_Int32 aTextRightDistance, xmlTextWriterPtr xmlWriter);
void dumpTextUpperDistanceAsAttribute(sal_Int32 aTextUpperDistance, xmlTextWriterPtr xmlWriter);
void dumpTextLowerDistanceAsAttribute(sal_Int32 aTextLowerDistance, xmlTextWriterPtr xmlWriter);
void dumpTextMaximumFrameHeightAsAttribute(sal_Int32 aTextMaximumFrameHeight, xmlTextWriterPtr xmlWriter);
void dumpTextMaximumFrameWidthAsAttribute(sal_Int32 aTextMaximumFrameWidth, xmlTextWriterPtr xmlWriter);
void dumpTextMinimumFrameHeightAsAttribute(sal_Int32 aTextMinimumFrameHeight, xmlTextWriterPtr xmlWriter);
void dumpTextMinimumFrameWidthAsAttribute(sal_Int32 aTextMinimumFrameWidth, xmlTextWriterPtr xmlWriter);
void dumpTextAnimationAmountAsAttribute(sal_Int32 aTextAnimationAmount, xmlTextWriterPtr xmlWriter);
void dumpTextAnimationCountAsAttribute(sal_Int32 aTextAnimationCount, xmlTextWriterPtr xmlWriter);
void dumpTextAnimationDelayAsAttribute(sal_Int32 aTextAnimationDelay, xmlTextWriterPtr xmlWriter);
void dumpTextAnimationDirectionAsAttribute(com::sun::star::drawing::TextAnimationDirection eTextAnimationDirection, xmlTextWriterPtr xmlWriter);
void dumpTextAnimationKindAsAttribute(com::sun::star::drawing::TextAnimationKind eTextAnimationKind, xmlTextWriterPtr xmlWriter);
void dumpTextAnimationStartInsideAsAttribute(sal_Bool bTextAnimationStartInside, xmlTextWriterPtr xmlWriter);
void dumpTextAnimationStopInsideAsAttribute(sal_Bool bTextAnimationStopInside, xmlTextWriterPtr xmlWriter);
void dumpTextWritingModeAsAttribute(com::sun::star::text::WritingMode eWritingMode, xmlTextWriterPtr xmlWriter);

// ShadowProperties.idl
void dumpShadowAsAttribute(sal_Bool bShadow, xmlTextWriterPtr xmlWriter);
void dumpShadowColorAsAttribute(sal_Int32 aShadowColor, xmlTextWriterPtr xmlWriter);
void dumpShadowTransparenceAsAttribute(sal_Int32 aShadowTransparence, xmlTextWriterPtr xmlWriter);
void dumpShadowXDistanceAsAttribute(sal_Int32 aShadowXDistance, xmlTextWriterPtr xmlWriter);
void dumpShadowYDistanceAsAttribute(sal_Int32 aShadowYDistance, xmlTextWriterPtr xmlWriter);

//Shape.idl
void dumpZOrderAsAttribute(sal_Int32 aZOrder, xmlTextWriterPtr xmlWriter);
void dumpLayerIDAsAttribute(sal_Int32 aLayerID, xmlTextWriterPtr xmlWriter);
void dumpLayerNameAsAttribute(rtl::OUString sLayerName, xmlTextWriterPtr xmlWriter);
void dumpVisibleAsAttribute(sal_Bool bVisible, xmlTextWriterPtr xmlWriter);
void dumpPrintableAsAttribute(sal_Bool bPrintable, xmlTextWriterPtr xmlWriter);
void dumpMoveProtectAsAttribute(sal_Bool bMoveProtect, xmlTextWriterPtr xmlWriter);
void dumpNameAsAttribute(rtl::OUString sName, xmlTextWriterPtr xmlWriter);
void dumpSizeProtectAsAttribute(sal_Bool bSizeProtect, xmlTextWriterPtr xmlWriter);
void dumpHomogenMatrixLine3(com::sun::star::drawing::HomogenMatrixLine3 aLine, xmlTextWriterPtr xmlWriter);
void dumpTransformationAsElement(com::sun::star::drawing::HomogenMatrix3 aTransformation, xmlTextWriterPtr xmlWriter);
void dumpNavigationOrderAsAttribute(sal_Int32 aNavigationOrder, xmlTextWriterPtr xmlWriter);
void dumpHyperlinkAsAttribute(rtl::OUString sHyperlink, xmlTextWriterPtr xmlWriter);

// CustomShape.idl
void dumpCustomShapeEngineAsAttribute(rtl::OUString sCustomShapeEngine, xmlTextWriterPtr xmlWriter);
void dumpCustomShapeDataAsAttribute(rtl::OUString sCustomShapeData, xmlTextWriterPtr xmlWriter);
void dumpCustomShapeGeometryAsElement(com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue> aCustomShapeGeometry, xmlTextWriterPtr xmlWriter);
void dumpCustomShapeReplacementURLAsAttribute(rtl::OUString sCustomShapeReplacementURL, xmlTextWriterPtr xmlWriter);

// XShape.idl
void dumpPositionAsAttribute(const com::sun::star::awt::Point& rPoint, xmlTextWriterPtr xmlWriter);
void dumpSizeAsAttribute(const com::sun::star::awt::Size& rSize, xmlTextWriterPtr xmlWriter);

// the rest
void dumpShapeDescriptorAsAttribute( com::sun::star::uno::Reference< com::sun::star::drawing::XShapeDescriptor > xDescr, xmlTextWriterPtr xmlWriter );
void dumpXShape(com::sun::star::uno::Reference< com::sun::star::drawing::XShape > xShape, xmlTextWriterPtr xmlWriter);
void dumpXShapes( com::sun::star::uno::Reference< com::sun::star::drawing::XShapes > xShapes, xmlTextWriterPtr xmlWriter );
void dumpTextPropertiesService(com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xPropSet, xmlTextWriterPtr xmlWriter);
void dumpFillPropertiesService(com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xPropSet, xmlTextWriterPtr xmlWriter);
void dumpLinePropertiesService(com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xPropSet, xmlTextWriterPtr xmlWriter);
void dumpShadowPropertiesService(com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xPropSet, xmlTextWriterPtr xmlWriter);
void dumpPolyPolygonDescriptorService(com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xPropSet, xmlTextWriterPtr xmlWriter);
void dumpShapeService(com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xPropSet, xmlTextWriterPtr xmlWriter);
void dumpPolyPolygonBezierDescriptorService(com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xPropSet, xmlTextWriterPtr xmlWriter);
void dumpCustomShapeService(com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xPropSet, xmlTextWriterPtr xmlWriter);


int writeCallback(void* pContext, const char* sBuffer, int nLen)
{
    rtl::OStringBuffer* pBuffer = static_cast<rtl::OStringBuffer*>(pContext);
    pBuffer->append(sBuffer);
    return nLen;
}

int closeCallback(void* )
{
    return 0;
}

// ----------------------------------------
// ---------- FillProperties.idl ----------
// ----------------------------------------
void dumpFillStyleAsAttribute(drawing::FillStyle eFillStyle, xmlTextWriterPtr xmlWriter)
{
    switch(eFillStyle)
    {
        case drawing::FillStyle_NONE:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillStyle"), "%s", "NONE");
            break;
        case drawing::FillStyle_SOLID:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillStyle"), "%s", "SOLID");
            break;
        case drawing::FillStyle_GRADIENT:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillStyle"), "%s", "GRADIENT");
            break;
        case drawing::FillStyle_HATCH:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillStyle"), "%s", "HATCH");
            break;
        case drawing::FillStyle_BITMAP:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillStyle"), "%s", "BITMAP");
            break;
        default:
            break;
    }
}

void dumpFillColorAsAttribute(sal_Int32 aColor, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillColor"), "%" SAL_PRIdINT32, aColor);
}

void dumpFillTransparenceAsAttribute(sal_Int32 aTransparence, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillTransparence"), "%" SAL_PRIdINT32, aTransparence);
}

void dumpFillTransparenceGradientNameAsAttribute(rtl::OUString sTranspGradName, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillTransparenceGradientName"), "%s",
        rtl::OUStringToOString(sTranspGradName, RTL_TEXTENCODING_UTF8).getStr());
}

//because there's more awt::Gradient properties to dump
void dumpGradientProperty(awt::Gradient aGradient, xmlTextWriterPtr xmlWriter)
{
    switch(aGradient.Style)   //enum GradientStyle
    {
        case awt::GradientStyle_LINEAR:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "LINEAR");
            break;
        case awt::GradientStyle_AXIAL:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "AXIAL");
            break;
        case awt::GradientStyle_RADIAL:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "RADIAL");
            break;
        case awt::GradientStyle_ELLIPTICAL:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "ELLIPTICAL");
            break;
        case awt::GradientStyle_SQUARE:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "SQUARE");
            break;
        case awt::GradientStyle_RECT:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "RECT");
            break;
        default:
            break;
    }
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("startColor"), "%" SAL_PRIdINT32, (sal_Int32) aGradient.StartColor);
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("endColor"), "%" SAL_PRIdINT32, (sal_Int32) aGradient.EndColor);
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("angle"), "%" SAL_PRIdINT32, (sal_Int32) aGradient.Angle);
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("border"), "%" SAL_PRIdINT32, (sal_Int32) aGradient.Border);
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("xOffset"), "%" SAL_PRIdINT32, (sal_Int32) aGradient.XOffset);
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("yOffset"), "%" SAL_PRIdINT32, (sal_Int32) aGradient.YOffset);
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("startIntensity"), "%" SAL_PRIdINT32, (sal_Int32) aGradient.StartIntensity);
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("endIntensity"), "%" SAL_PRIdINT32, (sal_Int32) aGradient.EndIntensity);
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("stepCount"), "%" SAL_PRIdINT32, (sal_Int32) aGradient.StepCount);
}

void dumpFillTransparenceGradientAsElement(awt::Gradient aTranspGrad, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "FillTransparenceGradient" ));
    dumpGradientProperty(aTranspGrad, xmlWriter);
    xmlTextWriterEndElement( xmlWriter );
}

void dumpFillGradientNameAsAttribute(rtl::OUString sGradName, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillGradientName"), "%s",
        rtl::OUStringToOString(sGradName, RTL_TEXTENCODING_UTF8).getStr());
}

void dumpFillGradientAsElement(awt::Gradient aGradient, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "FillGradient" ));
    dumpGradientProperty(aGradient, xmlWriter);
    xmlTextWriterEndElement( xmlWriter );
}

void dumpFillHatchAsElement(drawing::Hatch aHatch, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "FillHatch" ));
    switch(aHatch.Style)
    {
        case drawing::HatchStyle_SINGLE:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "SINGLE");
            break;
        case drawing::HatchStyle_DOUBLE:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "DOUBLE");
            break;
        case drawing::HatchStyle_TRIPLE:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "TRIPLE");
            break;
        default:
            break;
    }
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("color"), "%" SAL_PRIdINT32, (sal_Int32) aHatch.Color);
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("distance"), "%" SAL_PRIdINT32, (sal_Int32) aHatch.Distance);
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("angle"), "%" SAL_PRIdINT32, (sal_Int32) aHatch.Angle);
    xmlTextWriterEndElement( xmlWriter );
}

void dumpFillBackgroundAsAttribute(sal_Bool bBackground, xmlTextWriterPtr xmlWriter)
{
    if(bBackground)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBackground"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBackground"), "%s", "false");
}

void dumpFillBitmapAsElement(uno::Reference<awt::XBitmap> xBitmap, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "FillBitmap" ));
    if (xBitmap.is())
    {
        awt::Size const aSize = xBitmap->getSize();
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("width"), "%" SAL_PRIdINT32, aSize.Width);
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("height"), "%" SAL_PRIdINT32, aSize.Height);
    }
    xmlTextWriterEndElement( xmlWriter );
}

void dumpFillBitmapURLAsAttribute(rtl::OUString sBitmapURL, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillBitmapURL"), "%s",
        rtl::OUStringToOString(sBitmapURL, RTL_TEXTENCODING_UTF8).getStr());
}

void dumpFillBitmapPositionOffsetXAsAttribute(sal_Int32 aBitmapPositionOffsetX, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillBitmapPositionOffsetX"), "%" SAL_PRIdINT32, aBitmapPositionOffsetX);
}

void dumpFillBitmapPositionOffsetYAsAttribute(sal_Int32 aBitmapPositionOffsetY, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillBitmapPositionOffsetY"), "%" SAL_PRIdINT32, aBitmapPositionOffsetY);
}

void dumpFillBitmapOffsetXAsAttribute(sal_Int32 aBitmapOffsetX, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillBitmapOffsetX"), "%" SAL_PRIdINT32, aBitmapOffsetX);
}

void dumpFillBitmapOffsetYAsAttribute(sal_Int32 aBitmapOffsetY, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillBitmapOffsetY"), "%" SAL_PRIdINT32, aBitmapOffsetY);
}

void dumpFillBitmapRectanglePointAsAttribute(drawing::RectanglePoint eBitmapRectanglePoint, xmlTextWriterPtr xmlWriter)
{
    switch(eBitmapRectanglePoint)
    {
        case drawing::RectanglePoint_LEFT_TOP:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapRectanglePoint"), "%s", "LEFT_TOP");
            break;
        case drawing::RectanglePoint_MIDDLE_TOP:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapRectanglePoint"), "%s", "MIDDLE_TOP");
            break;
        case drawing::RectanglePoint_RIGHT_TOP:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapRectanglePoint"), "%s", "RIGHT_TOP");
            break;
        case drawing::RectanglePoint_LEFT_MIDDLE:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapRectanglePoint"), "%s", "LEFT_MIDDLE");
            break;
        case drawing::RectanglePoint_MIDDLE_MIDDLE:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapRectanglePoint"), "%s", "MIDDLE_MIDDLE");
            break;
        case drawing::RectanglePoint_RIGHT_MIDDLE:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapRectanglePoint"), "%s", "RIGHT_MIDDLE");
            break;
        case drawing::RectanglePoint_LEFT_BOTTOM:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapRectanglePoint"), "%s", "LEFT_BOTTOM");
            break;
        case drawing::RectanglePoint_MIDDLE_BOTTOM:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapRectanglePoint"), "%s", "MIDDLE_BOTTOM");
            break;
        case drawing::RectanglePoint_RIGHT_BOTTOM:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapRectanglePoint"), "%s", "RIGHT_BOTTOM");
            break;
        default:
            break;
    }
}

void dumpFillBitmapLogicalSizeAsAttribute(sal_Bool bBitmapLogicalSize, xmlTextWriterPtr xmlWriter)
{
    if(bBitmapLogicalSize)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapLogicalSize"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapLogicalSize"), "%s", "false");
}

void dumpFillBitmapSizeXAsAttribute(sal_Int32 aBitmapSizeX, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillBitmapSizeX"), "%" SAL_PRIdINT32, aBitmapSizeX);
}

void dumpFillBitmapSizeYAsAttribute(sal_Int32 aBitmapSizeY, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillBitmapSizeY"), "%" SAL_PRIdINT32, aBitmapSizeY);
}

void dumpFillBitmapModeAsAttribute(drawing::BitmapMode eBitmapMode, xmlTextWriterPtr xmlWriter)
{
    switch(eBitmapMode)
    {
        case drawing::BitmapMode_REPEAT:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapMode"), "%s", "REPEAT");
            break;
        case drawing::BitmapMode_STRETCH:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapMode"), "%s", "STRETCH");
            break;
        case drawing::BitmapMode_NO_REPEAT:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapMode"), "%s", "NO_REPEAT");
            break;
        default:
            break;
    }
}

void dumpFillBitmapStretchAsAttribute(sal_Bool bBitmapStretch, xmlTextWriterPtr xmlWriter)
{
    if(bBitmapStretch)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapStretch"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapStretch"), "%s", "false");
}

void dumpFillBitmapTileAsAttribute(sal_Bool bBitmapTile, xmlTextWriterPtr xmlWriter)
{
    if(bBitmapTile)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapTile"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapTile"), "%s", "false");
}

// ----------------------------------------
// ---------- LineProperties.idl ----------
// ----------------------------------------

void dumpLineStyleAsAttribute(drawing::LineStyle eLineStyle, xmlTextWriterPtr xmlWriter)
{
    switch(eLineStyle)
    {
        case drawing::LineStyle_NONE:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("lineStyle"), "%s", "NONE");
            break;
        case drawing::LineStyle_SOLID:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("lineStyle"), "%s", "SOLID");
            break;
        case drawing::LineStyle_DASH:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("lineStyle"), "%s", "DASH");
            break;
        default:
            break;
    }
}

void dumpLineDashAsElement(drawing::LineDash aLineDash, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "LineDash" ));
    switch(aLineDash.Style)
    {
        case drawing::DashStyle_RECT:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "RECT");
            break;
        case drawing::DashStyle_ROUND:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "ROUND");
            break;
        case drawing::DashStyle_RECTRELATIVE:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "RECTRELATIVE");
            break;
        case drawing::DashStyle_ROUNDRELATIVE:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "ROUNDRELATIVE");
            break;
        default:
            break;
    }
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("dots"), "%" SAL_PRIdINT32, (sal_Int32) aLineDash.Dots);
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("dotLen"), "%" SAL_PRIdINT32, (sal_Int32) aLineDash.DotLen);
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("dashes"), "%" SAL_PRIdINT32, (sal_Int32) aLineDash.Dashes);
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("dashLen"), "%" SAL_PRIdINT32, (sal_Int32) aLineDash.DashLen);
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("distance"), "%" SAL_PRIdINT32, (sal_Int32) aLineDash.Distance);
    xmlTextWriterEndElement( xmlWriter );
}

void dumpLineDashNameAsAttribute(rtl::OUString sLineDashName, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("lineDashName"), "%s",
        rtl::OUStringToOString(sLineDashName, RTL_TEXTENCODING_UTF8).getStr());
}

void dumpLineColorAsAttribute(sal_Int32 aLineColor, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("lineColor"), "%" SAL_PRIdINT32, aLineColor);
}

void dumpLineTransparenceAsAttribute(sal_Int32 aLineTransparence, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("lineTransparence"), "%" SAL_PRIdINT32, aLineTransparence);
}

void dumpLineWidthAsAttribute(sal_Int32 aLineWidth, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("lineWidth"), "%" SAL_PRIdINT32, aLineWidth);
}

void dumpLineJointAsAttribute(drawing::LineJoint eLineJoint, xmlTextWriterPtr xmlWriter)
{
    switch(eLineJoint)
    {
        case drawing::LineJoint_NONE:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("lineJoint"), "%s", "NONE");
            break;
        case drawing::LineJoint_MIDDLE:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("lineJoint"), "%s", "MIDDLE");
            break;
        case drawing::LineJoint_BEVEL:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("lineJoint"), "%s", "BEVEL");
            break;
        case drawing::LineJoint_MITER:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("lineJoint"), "%s", "MITER");
            break;
        case drawing::LineJoint_ROUND:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("lineJoint"), "%s", "ROUND");
            break;
        default:
            break;
    }
}

void dumpLineStartNameAsAttribute(rtl::OUString sLineStartName, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("lineStartName"), "%s",
    rtl::OUStringToOString(sLineStartName, RTL_TEXTENCODING_UTF8).getStr());
}

void dumpLineEndNameAsAttribute(rtl::OUString sLineEndName, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("lineEndName"), "%s",
    rtl::OUStringToOString(sLineEndName, RTL_TEXTENCODING_UTF8).getStr());
}

void dumpPolyPolygonBezierCoords(drawing::PolyPolygonBezierCoords aPolyPolygonBezierCoords, xmlTextWriterPtr xmlWriter)
{
    dumpPointSequenceSequence(aPolyPolygonBezierCoords.Coordinates, &aPolyPolygonBezierCoords.Flags, xmlWriter);
}

void dumpLineStartAsElement(drawing::PolyPolygonBezierCoords aLineStart, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "LineStart" ));
    dumpPolyPolygonBezierCoords(aLineStart, xmlWriter);
    xmlTextWriterEndElement( xmlWriter );
}

void dumpLineEndAsElement(drawing::PolyPolygonBezierCoords aLineEnd, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "LineEnd" ));
    dumpPolyPolygonBezierCoords(aLineEnd, xmlWriter);
    xmlTextWriterEndElement( xmlWriter );
}

void dumpLineStartCenterAsAttribute(sal_Bool bLineStartCenter, xmlTextWriterPtr xmlWriter)
{
    if(bLineStartCenter)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("lineStartCenter"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("lineStartCenter"), "%s", "false");
}

void dumpLineStartWidthAsAttribute(sal_Int32 aLineStartWidth, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("lineStartWidth"), "%" SAL_PRIdINT32, aLineStartWidth);
}

void dumpLineEndCenterAsAttribute(sal_Bool bLineEndCenter, xmlTextWriterPtr xmlWriter)
{
    if(bLineEndCenter)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("lineEndCenter"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("lineEndCenter"), "%s", "false");
}

void dumpLineEndWidthAsAttribute(sal_Int32 aLineEndWidth, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("lineEndWidth"), "%" SAL_PRIdINT32, aLineEndWidth);
}

// -----------------------------------------------
// ---------- PolyPolygonDescriptor.idl ----------
// -----------------------------------------------

void dumpPolygonKindAsAttribute(drawing::PolygonKind ePolygonKind, xmlTextWriterPtr xmlWriter)
{
    switch(ePolygonKind)
    {
        case drawing::PolygonKind_LINE:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("polygonKind"), "%s", "LINE");
            break;
        case drawing::PolygonKind_POLY:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("polygonKind"), "%s", "POLY");
            break;
        case drawing::PolygonKind_PLIN:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("polygonKind"), "%s", "PLIN");
            break;
        case drawing::PolygonKind_PATHLINE:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("polygonKind"), "%s", "PATHLINE");
            break;
        case drawing::PolygonKind_PATHFILL:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("polygonKind"), "%s", "PATHFILL");
            break;
        case drawing::PolygonKind_FREELINE:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("polygonKind"), "%s", "FREELINE");
            break;
        case drawing::PolygonKind_FREEFILL:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("polygonKind"), "%s", "FREEFILL");
            break;
        case drawing::PolygonKind_PATHPOLY:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("polygonKind"), "%s", "PATHPOLY");
            break;
        case drawing::PolygonKind_PATHPLIN:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("polygonKind"), "%s", "PATHPLIN");
            break;
        default:
            break;
    }
}

void dumpPointSequenceSequence(drawing::PointSequenceSequence aPointSequenceSequence, uno::Sequence<uno::Sequence< drawing::PolygonFlags > >* pFlags, xmlTextWriterPtr xmlWriter)
{
    // LibreOffice proudly presents - The Sequenception
    uno::Sequence<uno::Sequence< awt::Point > > pointSequenceSequence = aPointSequenceSequence;
    sal_Int32 nPointsSequence = pointSequenceSequence.getLength();

    for (sal_Int32 i = 0; i < nPointsSequence; ++i)
    {
        uno::Sequence< awt::Point > pointSequence = pointSequenceSequence[i];
        sal_Int32 nPoints = pointSequence.getLength();

        uno::Sequence< drawing::PolygonFlags> flagsSequence;
        if(pFlags)
            flagsSequence = (*pFlags)[i];

        xmlTextWriterStartElement(xmlWriter, BAD_CAST( "pointSequence" ));

        for(sal_Int32 j = 0; j < nPoints; ++j)
        {
            xmlTextWriterStartElement(xmlWriter, BAD_CAST( "point" ));
            xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("positionX"), "%" SAL_PRIdINT32, pointSequence[j].X);
            xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("positionY"), "%" SAL_PRIdINT32, pointSequence[j].Y);

            if(pFlags)
            {
                switch(flagsSequence[j])
                {
                    case drawing::PolygonFlags_NORMAL:
                        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("polygonFlags"), "%s", "NORMAL");
                        break;
                    case drawing::PolygonFlags_SMOOTH:
                        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("polygonFlags"), "%s", "SMOOTH");
                        break;
                    case drawing::PolygonFlags_CONTROL:
                        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("polygonFlags"), "%s", "CONTROL");
                        break;
                    case drawing::PolygonFlags_SYMMETRIC:
                        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("polygonFlags"), "%s", "SYMMETRIC");
                        break;
                    default:
                        break;
                }
            }

            xmlTextWriterEndElement( xmlWriter );
        }
        xmlTextWriterEndElement( xmlWriter );
    }
}

void dumpPolyPolygonAsElement(drawing::PointSequenceSequence aPolyPolygon, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "PolyPolygon" ));
    dumpPointSequenceSequence(aPolyPolygon, NULL, xmlWriter);
    xmlTextWriterEndElement( xmlWriter );
}

void dumpGeometryAsElement(drawing::PointSequenceSequence aGeometry, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Geometry" ));
    dumpPointSequenceSequence(aGeometry, NULL, xmlWriter);
    xmlTextWriterEndElement( xmlWriter );
}

// ----------------------------------------
// ---------- TextProperties.idl ----------
// ----------------------------------------

void dumpIsNumberingAsAttribute(sal_Bool bIsNumbering, xmlTextWriterPtr xmlWriter)
{
    if(bIsNumbering)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("isNumbering"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("isNumbering"), "%s", "false");
}

void dumpTextAutoGrowHeightAsAttribute(sal_Bool bTextAutoGrowHeight, xmlTextWriterPtr xmlWriter)
{
    if(bTextAutoGrowHeight)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAutoGrowHeight"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAutoGrowHeight"), "%s", "false");
}

void dumpTextAutoGrowWidthAsAttribute(sal_Bool bTextAutoGrowWidth, xmlTextWriterPtr xmlWriter)
{
    if(bTextAutoGrowWidth)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAutoGrowWidth"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAutoGrowWidth"), "%s", "false");
}

void dumpTextContourFrameAsAttribute(sal_Bool bTextContourFrame, xmlTextWriterPtr xmlWriter)
{
    if(bTextContourFrame)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textContourFrame"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textContourFrame"), "%s", "false");
}

void dumpTextFitToSizeAsAttribute(drawing::TextFitToSizeType eTextFitToSize, xmlTextWriterPtr xmlWriter)
{
    switch(eTextFitToSize)
    {
        case drawing::TextFitToSizeType_NONE:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textFitToSize"), "%s", "NONE");
            break;
        case drawing::TextFitToSizeType_PROPORTIONAL:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textFitToSize"), "%s", "PROPORTIONAL");
            break;
        case drawing::TextFitToSizeType_ALLLINES:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textFitToSize"), "%s", "ALLLINES");
            break;
        case drawing::TextFitToSizeType_AUTOFIT:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textFitToSize"), "%s", "AUTOFIT");
            break;
        default:
            break;
    }
}

void dumpTextHorizontalAdjustAsAttribute(drawing::TextHorizontalAdjust eTextHorizontalAdjust, xmlTextWriterPtr xmlWriter)
{
    switch(eTextHorizontalAdjust)
    {
        case drawing::TextHorizontalAdjust_LEFT:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textHorizontalAdjust"), "%s", "LEFT");
            break;
        case drawing::TextHorizontalAdjust_CENTER:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textHorizontalAdjust"), "%s", "CENTER");
            break;
        case drawing::TextHorizontalAdjust_RIGHT:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textHorizontalAdjust"), "%s", "RIGHT");
            break;
        case drawing::TextHorizontalAdjust_BLOCK:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textHorizontalAdjust"), "%s", "BLOCK");
            break;
        default:
            break;
    }
}

void dumpTextVerticalAdjustAsAttribute(drawing::TextVerticalAdjust eTextVerticalAdjust, xmlTextWriterPtr xmlWriter)
{
    switch(eTextVerticalAdjust)
    {
        case drawing::TextVerticalAdjust_TOP:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textVerticalAdjust"), "%s", "TOP");
            break;
        case drawing::TextVerticalAdjust_CENTER:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textVerticalAdjust"), "%s", "CENTER");
            break;
        case drawing::TextVerticalAdjust_BOTTOM:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textVerticalAdjust"), "%s", "BOTTOM");
            break;
        case drawing::TextVerticalAdjust_BLOCK:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textVerticalAdjust"), "%s", "BLOCK");
            break;
        default:
            break;
    }
}

void dumpTextLeftDistanceAsAttribute(sal_Int32 aTextLeftDistance, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("textLeftDistance"), "%" SAL_PRIdINT32, aTextLeftDistance);
}

void dumpTextRightDistanceAsAttribute(sal_Int32 aTextRightDistance, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("textRightDistance"), "%" SAL_PRIdINT32, aTextRightDistance);
}

void dumpTextUpperDistanceAsAttribute(sal_Int32 aTextUpperDistance, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("textUpperDistance"), "%" SAL_PRIdINT32, aTextUpperDistance);
}

void dumpTextLowerDistanceAsAttribute(sal_Int32 aTextLowerDistance, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("textLowerDistance"), "%" SAL_PRIdINT32, aTextLowerDistance);
}

void dumpTextMaximumFrameHeightAsAttribute(sal_Int32 aTextMaximumFrameHeight, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("textMaximumFrameHeight"), "%" SAL_PRIdINT32, aTextMaximumFrameHeight);
}

void dumpTextMaximumFrameWidthAsAttribute(sal_Int32 aTextMaximumFrameWidth, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("textMaximumFrameWidth"), "%" SAL_PRIdINT32, aTextMaximumFrameWidth);
}

void dumpTextMinimumFrameHeightAsAttribute(sal_Int32 aTextMinimumFrameHeight, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("textMinimumFrameHeight"), "%" SAL_PRIdINT32, aTextMinimumFrameHeight);
}

void dumpTextMinimumFrameWidthAsAttribute(sal_Int32 aTextMinimumFrameWidth, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("textMinimumFrameWidth"), "%" SAL_PRIdINT32, aTextMinimumFrameWidth);
}

void dumpTextAnimationAmountAsAttribute(sal_Int32 aTextAnimationAmount, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("textAnimationAmount"), "%" SAL_PRIdINT32, aTextAnimationAmount);
}

void dumpTextAnimationCountAsAttribute(sal_Int32 aTextAnimationCount, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("textAnimationCount"), "%" SAL_PRIdINT32, aTextAnimationCount);
}

void dumpTextAnimationDelayAsAttribute(sal_Int32 aTextAnimationDelay, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("textAnimationDelay"), "%" SAL_PRIdINT32, aTextAnimationDelay);
}

void dumpTextAnimationDirectionAsAttribute(drawing::TextAnimationDirection eTextAnimationDirection, xmlTextWriterPtr xmlWriter)
{
    switch(eTextAnimationDirection)
    {
        case drawing::TextAnimationDirection_LEFT:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAnimationDirection"), "%s", "LEFT");
            break;
        case drawing::TextAnimationDirection_RIGHT:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAnimationDirection"), "%s", "RIGHT");
            break;
        case drawing::TextAnimationDirection_UP:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAnimationDirection"), "%s", "UP");
            break;
        case drawing::TextAnimationDirection_DOWN:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAnimationDirection"), "%s", "DOWN");
            break;
        default:
            break;
    }
}

void dumpTextAnimationKindAsAttribute(drawing::TextAnimationKind eTextAnimationKind, xmlTextWriterPtr xmlWriter)
{
    switch(eTextAnimationKind)
    {
        case drawing::TextAnimationKind_NONE:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAnimationKind"), "%s", "NONE");
            break;
        case drawing::TextAnimationKind_BLINK:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAnimationKind"), "%s", "BLINK");
            break;
        case drawing::TextAnimationKind_SCROLL:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAnimationKind"), "%s", "SCROLL");
            break;
        case drawing::TextAnimationKind_ALTERNATE:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAnimationKind"), "%s", "ALTERNATE");
            break;
        case drawing::TextAnimationKind_SLIDE:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAnimationKind"), "%s", "SLIDE");
            break;
        default:
            break;
    }
}

void dumpTextAnimationStartInsideAsAttribute(sal_Bool bTextAnimationStartInside, xmlTextWriterPtr xmlWriter)
{
    if(bTextAnimationStartInside)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAnimationStartInside"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAnimationStartInside"), "%s", "false");
}

void dumpTextAnimationStopInsideAsAttribute(sal_Bool bTextAnimationStopInside, xmlTextWriterPtr xmlWriter)
{
    if(bTextAnimationStopInside)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAnimationStopInside"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAnimationStopInside"), "%s", "false");
}

void dumpTextWritingModeAsAttribute(text::WritingMode eTextWritingMode, xmlTextWriterPtr xmlWriter)
{
    switch(eTextWritingMode)
    {
        case text::WritingMode_LR_TB:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textWritingMode"), "%s", "LR_TB");
            break;
        case text::WritingMode_RL_TB:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textWritingMode"), "%s", "RL_TB");
            break;
        case text::WritingMode_TB_RL:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textWritingMode"), "%s", "TB_RL");
            break;
        default:
            break;
    }
}

// ------------------------------------------
// ---------- ShadowProperties.idl ----------
// ------------------------------------------

void dumpShadowAsAttribute(sal_Bool bShadow, xmlTextWriterPtr xmlWriter)
{
    if(bShadow)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("shadow"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("shadow"), "%s", "false");
}

void dumpShadowColorAsAttribute(sal_Int32 aShadowColor, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("shadowColor"), "%" SAL_PRIdINT32, aShadowColor);
}

void dumpShadowTransparenceAsAttribute(sal_Int32 aShadowTransparence, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("shadowTransparence"), "%" SAL_PRIdINT32, aShadowTransparence);
}

void dumpShadowXDistanceAsAttribute(sal_Int32 aShadowXDistance, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("shadowXDistance"), "%" SAL_PRIdINT32, aShadowXDistance);
}

void dumpShadowYDistanceAsAttribute(sal_Int32 aShadowYDistance, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("shadowYDistance"), "%" SAL_PRIdINT32, aShadowYDistance);
}

// -------------------------------
// ---------- Shape.idl ----------
// -------------------------------

void dumpZOrderAsAttribute(sal_Int32 aZOrder, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("zOrder"), "%" SAL_PRIdINT32, aZOrder);
}

void dumpLayerIDAsAttribute(sal_Int32 aLayerID, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("layerID"), "%" SAL_PRIdINT32, aLayerID);
}

void dumpLayerNameAsAttribute(rtl::OUString sLayerName, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("layerName"), "%s",
        rtl::OUStringToOString(sLayerName, RTL_TEXTENCODING_UTF8).getStr());
}

void dumpVisibleAsAttribute(sal_Bool bVisible, xmlTextWriterPtr xmlWriter)
{
    if(bVisible)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("visible"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("visible"), "%s", "false");
}

void dumpPrintableAsAttribute(sal_Bool bPrintable, xmlTextWriterPtr xmlWriter)
{
    if(bPrintable)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("printable"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("printable"), "%s", "false");
}

void dumpMoveProtectAsAttribute(sal_Bool bMoveProtect, xmlTextWriterPtr xmlWriter)
{
    if(bMoveProtect)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("moveProtect"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("moveProtect"), "%s", "false");
}

void dumpNameAsAttribute(rtl::OUString sName, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("name"), "%s",
        rtl::OUStringToOString(sName, RTL_TEXTENCODING_UTF8).getStr());
}

void dumpSizeProtectAsAttribute(sal_Bool bSizeProtect, xmlTextWriterPtr xmlWriter)
{
    if(bSizeProtect)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("sizeProtect"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("sizeProtect"), "%s", "false");
}

void dumpHomogenMatrixLine3(drawing::HomogenMatrixLine3 aHomogenMatrixLine3, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("column1"), "%f", aHomogenMatrixLine3.Column1);
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("column2"), "%f", aHomogenMatrixLine3.Column2);
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("column3"), "%f", aHomogenMatrixLine3.Column3);
}

void dumpTransformationAsElement(drawing::HomogenMatrix3 aTransformation, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Transformation" ));
    {
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Line1" ));
    dumpHomogenMatrixLine3(aTransformation.Line1, xmlWriter);
    xmlTextWriterEndElement( xmlWriter );
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Line2" ));
    dumpHomogenMatrixLine3(aTransformation.Line2, xmlWriter);
    xmlTextWriterEndElement( xmlWriter );
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Line3" ));
    dumpHomogenMatrixLine3(aTransformation.Line3, xmlWriter);
    xmlTextWriterEndElement( xmlWriter );
    }
    xmlTextWriterEndElement( xmlWriter );
}

void dumpNavigationOrderAsAttribute(sal_Int32 aNavigationOrder, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("navigationOrder"), "%" SAL_PRIdINT32, aNavigationOrder);
}

void dumpHyperlinkAsAttribute(rtl::OUString sHyperlink, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("hyperlink"), "%s",
        rtl::OUStringToOString(sHyperlink, RTL_TEXTENCODING_UTF8).getStr());
}

// --------------------------------
// ---------- XShape.idl ----------
// --------------------------------

void dumpPositionAsAttribute(const awt::Point& rPoint, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("positionX"), "%" SAL_PRIdINT32, rPoint.X);
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("positionY"), "%" SAL_PRIdINT32, rPoint.Y);
}

void dumpSizeAsAttribute(const awt::Size& rSize, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("sizeX"), "%" SAL_PRIdINT32, rSize.Width);
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("sizeY"), "%" SAL_PRIdINT32, rSize.Height);
}

void dumpShapeDescriptorAsAttribute( uno::Reference< drawing::XShapeDescriptor > xDescr, xmlTextWriterPtr xmlWriter )
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("type"), "%s",
        rtl::OUStringToOString(xDescr->getShapeType(), RTL_TEXTENCODING_UTF8).getStr());
}

// -------------------------------------
// ---------- CustomShape.idl ----------
// -------------------------------------

void dumpCustomShapeEngineAsAttribute(rtl::OUString sCustomShapeEngine, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("customShapeEngine"), "%s",
        rtl::OUStringToOString(sCustomShapeEngine, RTL_TEXTENCODING_UTF8).getStr());
}

void dumpCustomShapeDataAsAttribute(rtl::OUString sCustomShapeData, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("customShapeData"), "%s",
        rtl::OUStringToOString(sCustomShapeData, RTL_TEXTENCODING_UTF8).getStr());
}

void dumpCustomShapeGeometryAsElement(uno::Sequence< beans::PropertyValue> aCustomShapeGeometry, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "CustomShapeGeometry" ));
    sal_Int32 nLength = aCustomShapeGeometry.getLength();
    for (sal_Int32 i = 0; i < nLength; ++i)
    {
        xmlTextWriterStartElement(xmlWriter, BAD_CAST( "PropertyValue" ));

        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("name"), "%s",
            rtl::OUStringToOString(aCustomShapeGeometry[i].Name, RTL_TEXTENCODING_UTF8).getStr());
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("handle"), "%" SAL_PRIdINT32, aCustomShapeGeometry[i].Handle);

        uno::Any aAny = aCustomShapeGeometry[i].Value;
        rtl::OUString sValue;
        if(aAny >>= sValue)
        {
            xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("value"), "%s",
                rtl::OUStringToOString(sValue, RTL_TEXTENCODING_UTF8).getStr());
        }
        switch(aCustomShapeGeometry[i].State)
        {
            case beans::PropertyState_DIRECT_VALUE:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("propertyState"), "%s", "DIRECT_VALUE");
                break;
            case beans::PropertyState_DEFAULT_VALUE:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("propertyState"), "%s", "DEFAULT_VALUE");
                break;
            case beans::PropertyState_AMBIGUOUS_VALUE:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("propertyState"), "%s", "AMBIGUOUS_VALUE");
                break;
            default:
                break;
        }
        xmlTextWriterEndElement( xmlWriter );
    }
    xmlTextWriterEndElement( xmlWriter );
}

void dumpCustomShapeReplacementURLAsAttribute(rtl::OUString sCustomShapeReplacementURL, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("customShapeReplacementURL"), "%s",
        rtl::OUStringToOString(sCustomShapeReplacementURL, RTL_TEXTENCODING_UTF8).getStr());
}

// methods dumping whole services

void dumpTextPropertiesService(uno::Reference< beans::XPropertySet > xPropSet, xmlTextWriterPtr xmlWriter)
{
    uno::Reference< beans::XPropertySetInfo> xInfo = xPropSet->getPropertySetInfo();
    if(xInfo->hasPropertyByName("IsNumbering"))
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("IsNumbering");
        sal_Bool bIsNumbering = sal_Bool();
        if(anotherAny >>= bIsNumbering)
            dumpIsNumberingAsAttribute(bIsNumbering, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextAutoGrowHeight");
        sal_Bool bTextAutoGrowHeight = sal_Bool();
        if(anotherAny >>= bTextAutoGrowHeight)
            dumpTextAutoGrowHeightAsAttribute(bTextAutoGrowHeight, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextAutoGrowWidth");
        sal_Bool bTextAutoGrowWidth = sal_Bool();
        if(anotherAny >>= bTextAutoGrowWidth)
            dumpTextAutoGrowWidthAsAttribute(bTextAutoGrowWidth, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextContourFrame");
        sal_Bool bTextContourFrame = sal_Bool();
        if(anotherAny >>= bTextContourFrame)
            dumpTextContourFrameAsAttribute(bTextContourFrame, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextFitToSize");
        drawing::TextFitToSizeType eTextFitToSize;
        if(anotherAny >>= eTextFitToSize)
            dumpTextFitToSizeAsAttribute(eTextFitToSize, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextHorizontalAdjust");
        drawing::TextHorizontalAdjust eTextHorizontalAdjust;
        if(anotherAny >>= eTextHorizontalAdjust)
            dumpTextHorizontalAdjustAsAttribute(eTextHorizontalAdjust, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextVerticalAdjust");
        drawing::TextVerticalAdjust eTextVerticalAdjust;
        if(anotherAny >>= eTextVerticalAdjust)
            dumpTextVerticalAdjustAsAttribute(eTextVerticalAdjust, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextLeftDistance");
        sal_Int32 aTextLeftDistance = sal_Int32();
        if(anotherAny >>= aTextLeftDistance)
            dumpTextLeftDistanceAsAttribute(aTextLeftDistance, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextRightDistance");
        sal_Int32 aTextRightDistance = sal_Int32();
        if(anotherAny >>= aTextRightDistance)
            dumpTextRightDistanceAsAttribute(aTextRightDistance, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextUpperDistance");
        sal_Int32 aTextUpperDistance = sal_Int32();
        if(anotherAny >>= aTextUpperDistance)
            dumpTextUpperDistanceAsAttribute(aTextUpperDistance, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextLowerDistance");
        sal_Int32 aTextLowerDistance = sal_Int32();
        if(anotherAny >>= aTextLowerDistance)
            dumpTextLowerDistanceAsAttribute(aTextLowerDistance, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextMaximumFrameHeight");
        sal_Int32 aTextMaximumFrameHeight = sal_Int32();
        if(anotherAny >>= aTextMaximumFrameHeight)
            dumpTextMaximumFrameHeightAsAttribute(aTextMaximumFrameHeight, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextMaximumFrameWidth");
        sal_Int32 aTextMaximumFrameWidth = sal_Int32();
        if(anotherAny >>= aTextMaximumFrameWidth)
            dumpTextMaximumFrameWidthAsAttribute(aTextMaximumFrameWidth, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextMinimumFrameHeight");
        sal_Int32 aTextMinimumFrameHeight = sal_Int32();
        if(anotherAny >>= aTextMinimumFrameHeight)
            dumpTextMinimumFrameHeightAsAttribute(aTextMinimumFrameHeight, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextMinimumFrameWidth");
        sal_Int32 aTextMinimumFrameWidth = sal_Int32();
        if(anotherAny >>= aTextMinimumFrameWidth)
            dumpTextMinimumFrameWidthAsAttribute(aTextMinimumFrameWidth, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextAnimationAmount");
        sal_Int32 aTextAnimationAmount = sal_Int32();
        if(anotherAny >>= aTextAnimationAmount)
            dumpTextAnimationAmountAsAttribute(aTextAnimationAmount, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextAnimationCount");
        sal_Int32 aTextAnimationCount = sal_Int32();
        if(anotherAny >>= aTextAnimationCount)
            dumpTextAnimationCountAsAttribute(aTextAnimationCount, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextAnimationDelay");
        sal_Int32 aTextAnimationDelay = sal_Int32();
        if(anotherAny >>= aTextAnimationDelay)
            dumpTextAnimationDelayAsAttribute(aTextAnimationDelay, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextAnimationDirection");
        drawing::TextAnimationDirection eTextAnimationDirection;
        if(anotherAny >>= eTextAnimationDirection)
            dumpTextAnimationDirectionAsAttribute(eTextAnimationDirection, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextAnimationKind");
        drawing::TextAnimationKind eTextAnimationKind;
        if(anotherAny >>= eTextAnimationKind)
            dumpTextAnimationKindAsAttribute(eTextAnimationKind, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextAnimationStartInside");
        sal_Bool bTextAnimationStartInside = sal_Bool();
        if(anotherAny >>= bTextAnimationStartInside)
            dumpTextAnimationStartInsideAsAttribute(bTextAnimationStartInside, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextAnimationStopInside");
        sal_Bool bTextAnimationStopInside = sal_Bool();
        if(anotherAny >>= bTextAnimationStopInside)
            dumpTextAnimationStopInsideAsAttribute(bTextAnimationStopInside, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextWritingMode");
        text::WritingMode eTextWritingMode;
        if(anotherAny >>= eTextWritingMode)
            dumpTextWritingModeAsAttribute(eTextWritingMode, xmlWriter);
    }
}

void dumpFillPropertiesService(uno::Reference< beans::XPropertySet > xPropSet, xmlTextWriterPtr xmlWriter)
{
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("FillStyle");
        drawing::FillStyle eFillStyle;
        if(anotherAny >>= eFillStyle)
            dumpFillStyleAsAttribute(eFillStyle, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("FillColor");
        sal_Int32 aColor = sal_Int32();
        if(anotherAny >>= aColor)
            dumpFillColorAsAttribute(aColor, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("FillTransparence");
        sal_Int32 aTransparence = sal_Int32();
        if(anotherAny >>= aTransparence)
            dumpFillTransparenceAsAttribute(aTransparence, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("FillTransparenceGradientName");
        rtl::OUString sTranspGradName;
        if(anotherAny >>= sTranspGradName)
            dumpFillTransparenceGradientNameAsAttribute(sTranspGradName, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("FillTransparenceGradient");
        awt::Gradient aTranspGrad;
        if(anotherAny >>= aTranspGrad)
            dumpFillTransparenceGradientAsElement(aTranspGrad, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("FillGradientName");
        rtl::OUString sGradName;
        if(anotherAny >>= sGradName)
            dumpFillGradientNameAsAttribute(sGradName, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("FillGradient");
        awt::Gradient aGradient;
        if(anotherAny >>= aGradient)
            dumpFillGradientAsElement(aGradient, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("FillHatchName");
        rtl::OUString sHatchName;
        if(anotherAny >>= sHatchName)
            dumpFillGradientNameAsAttribute(sHatchName, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("FillHatch");
        drawing::Hatch aHatch;
        if(anotherAny >>= aHatch)
            dumpFillHatchAsElement(aHatch, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("FillBackground");
        sal_Bool bFillBackground = sal_Bool();
        if(anotherAny >>= bFillBackground)
            dumpFillBackgroundAsAttribute(bFillBackground, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("FillBitmapName");
        rtl::OUString sBitmapName;
        if(anotherAny >>= sBitmapName)
            dumpFillGradientNameAsAttribute(sBitmapName, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("FillBitmap");
        uno::Reference<awt::XBitmap> xBitmap;
        if(anotherAny >>= xBitmap)
            dumpFillBitmapAsElement(xBitmap, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("FillBitmapURL");
        rtl::OUString sBitmapURL;
        if(anotherAny >>= sBitmapURL)
            dumpFillBitmapURLAsAttribute(sBitmapURL, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("FillBitmapPositionOffsetX");
        sal_Int32 aBitmapPositionOffsetX = sal_Int32();
        if(anotherAny >>= aBitmapPositionOffsetX)
            dumpFillBitmapPositionOffsetXAsAttribute(aBitmapPositionOffsetX, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("FillBitmapPositionOffsetY");
        sal_Int32 aBitmapPositionOffsetY = sal_Int32();
        if(anotherAny >>= aBitmapPositionOffsetY)
            dumpFillBitmapPositionOffsetYAsAttribute(aBitmapPositionOffsetY, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("FillBitmapOffsetX");
        sal_Int32 aBitmapOffsetX = sal_Int32();
        if(anotherAny >>= aBitmapOffsetX)
            dumpFillBitmapOffsetXAsAttribute(aBitmapOffsetX, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("FillBitmapOffsetY");
        sal_Int32 aBitmapOffsetY = sal_Int32();
        if(anotherAny >>= aBitmapOffsetY)
            dumpFillBitmapOffsetYAsAttribute(aBitmapOffsetY, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("FillBitmapRectanglePoint");
        drawing::RectanglePoint eBitmapRectanglePoint;
        if(anotherAny >>= eBitmapRectanglePoint)
            dumpFillBitmapRectanglePointAsAttribute(eBitmapRectanglePoint, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("FillBitmapLogicalSize");
        sal_Bool bBitmapLogicalSize = sal_Bool();
        if(anotherAny >>= bBitmapLogicalSize)
            dumpFillBitmapLogicalSizeAsAttribute(bBitmapLogicalSize, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("FillBitmapSizeX");
        sal_Int32 aBitmapSizeX = sal_Int32();
        if(anotherAny >>= aBitmapSizeX)
            dumpFillBitmapSizeXAsAttribute(aBitmapSizeX, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("FillBitmapSizeY");
        sal_Int32 aBitmapSizeY = sal_Int32();
        if(anotherAny >>= aBitmapSizeY)
            dumpFillBitmapSizeYAsAttribute(aBitmapSizeY, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("FillBitmapMode");
        drawing::BitmapMode eBitmapMode;
        if(anotherAny >>= eBitmapMode)
            dumpFillBitmapModeAsAttribute(eBitmapMode, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("FillBitmapStretch");
        sal_Bool bBitmapStretch = sal_Bool();
        if(anotherAny >>= bBitmapStretch)
            dumpFillBitmapStretchAsAttribute(bBitmapStretch, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("FillBitmapTile");
        sal_Bool bBitmapTile = sal_Bool();
        if(anotherAny >>= bBitmapTile)
            dumpFillBitmapTileAsAttribute(bBitmapTile, xmlWriter);
    }
}

void dumpLinePropertiesService(uno::Reference< beans::XPropertySet > xPropSet, xmlTextWriterPtr xmlWriter)
{
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("LineStyle");
        drawing::LineStyle eLineStyle;
        if(anotherAny >>= eLineStyle)
            dumpLineStyleAsAttribute(eLineStyle, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("LineDash");
        drawing::LineDash aLineDash;
        if(anotherAny >>= aLineDash)
            dumpLineDashAsElement(aLineDash, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("LineDashName");
        rtl::OUString sLineDashName;
        if(anotherAny >>= sLineDashName)
            dumpLineDashNameAsAttribute(sLineDashName, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("LineColor");
        sal_Int32 aLineColor = sal_Int32();
        if(anotherAny >>= aLineColor)
            dumpLineColorAsAttribute(aLineColor, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("LineTransparence");
        sal_Int32 aLineTransparence = sal_Int32();
        if(anotherAny >>= aLineTransparence)
            dumpLineTransparenceAsAttribute(aLineTransparence, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("LineWidth");
        sal_Int32 aLineWidth = sal_Int32();
        if(anotherAny >>= aLineWidth)
            dumpLineWidthAsAttribute(aLineWidth, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("LineJoint");
        drawing::LineJoint eLineJoint;
        if(anotherAny >>= eLineJoint)
            dumpLineJointAsAttribute(eLineJoint, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("LineStartName");
        rtl::OUString sLineStartName;
        if(anotherAny >>= sLineStartName)
            dumpLineStartNameAsAttribute(sLineStartName, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("LineEndName");
        rtl::OUString sLineEndName;
        if(anotherAny >>= sLineEndName)
            dumpLineEndNameAsAttribute(sLineEndName, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("LineStart");
        drawing::PolyPolygonBezierCoords aLineStart;
        if(anotherAny >>= aLineStart)
            dumpLineStartAsElement(aLineStart, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("LineEnd");
        drawing::PolyPolygonBezierCoords aLineEnd;
        if(anotherAny >>= aLineEnd)
            dumpLineEndAsElement(aLineEnd, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("LineStartCenter");
        sal_Bool bLineStartCenter = sal_Bool();
        if(anotherAny >>= bLineStartCenter)
            dumpLineStartCenterAsAttribute(bLineStartCenter, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("LineStartWidth");
        sal_Int32 aLineStartWidth = sal_Int32();
        if(anotherAny >>= aLineStartWidth)
            dumpLineStartWidthAsAttribute(aLineStartWidth, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("LineEndCenter");
        sal_Bool bLineEndCenter = sal_Bool();
        if(anotherAny >>= bLineEndCenter)
            dumpLineEndCenterAsAttribute(bLineEndCenter, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("LineEndWidth");
        sal_Int32 aLineEndWidth = sal_Int32();
        if(anotherAny >>= aLineEndWidth)
            dumpLineEndWidthAsAttribute(aLineEndWidth, xmlWriter);
    }
}

void dumpShadowPropertiesService(uno::Reference< beans::XPropertySet > xPropSet, xmlTextWriterPtr xmlWriter)
{
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Shadow");
        sal_Bool bShadow = sal_Bool();
        if(anotherAny >>= bShadow)
            dumpShadowAsAttribute(bShadow, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("ShadowColor");
        sal_Int32 aShadowColor = sal_Int32();
        if(anotherAny >>= aShadowColor)
            dumpShadowColorAsAttribute(aShadowColor, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("ShadowTransparence");
        sal_Int32 aShadowTransparence = sal_Int32();
        if(anotherAny >>= aShadowTransparence)
            dumpShadowTransparenceAsAttribute(aShadowTransparence, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("ShadowXDistance");
        sal_Int32 aShadowXDistance = sal_Int32();
        if(anotherAny >>= aShadowXDistance)
            dumpShadowXDistanceAsAttribute(aShadowXDistance, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("ShadowYDistance");
        sal_Int32 aShadowYDistance = sal_Int32();
        if(anotherAny >>= aShadowYDistance)
            dumpShadowYDistanceAsAttribute(aShadowYDistance, xmlWriter);
    }
}

void dumpPolyPolygonDescriptorService(uno::Reference< beans::XPropertySet > xPropSet, xmlTextWriterPtr xmlWriter)
{
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("PolygonKind");
        drawing::PolygonKind ePolygonKind;
        if(anotherAny >>= ePolygonKind)
            dumpPolygonKindAsAttribute(ePolygonKind, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("PolyPolygon");
        drawing::PointSequenceSequence aPolyPolygon;
        if(anotherAny >>= aPolyPolygon)
            dumpPolyPolygonAsElement(aPolyPolygon, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Geometry");
        drawing::PointSequenceSequence aGeometry;
        if(anotherAny >>= aGeometry)
            dumpGeometryAsElement(aGeometry, xmlWriter);
    }
}

void dumpShapeService(uno::Reference< beans::XPropertySet > xPropSet, xmlTextWriterPtr xmlWriter)
{
    uno::Reference< beans::XPropertySetInfo> xInfo = xPropSet->getPropertySetInfo();
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("ZOrder");
        sal_Int32 aZOrder = sal_Int32();
        if(anotherAny >>= aZOrder)
            dumpZOrderAsAttribute(aZOrder, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("LayerID");
        sal_Int32 aLayerID = sal_Int32();
        if(anotherAny >>= aLayerID)
            dumpLayerIDAsAttribute(aLayerID, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("LayerName");
        rtl::OUString sLayerName;
        if(anotherAny >>= sLayerName)
            dumpLayerNameAsAttribute(sLayerName, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Visible");
        sal_Bool bVisible = sal_Bool();
        if(anotherAny >>= bVisible)
            dumpVisibleAsAttribute(bVisible, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Printable");
        sal_Bool bPrintable = sal_Bool();
        if(anotherAny >>= bPrintable)
            dumpPrintableAsAttribute(bPrintable, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("MoveProtect");
        sal_Bool bMoveProtect = sal_Bool();
        if(anotherAny >>= bMoveProtect)
            dumpMoveProtectAsAttribute(bMoveProtect, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Name");
        rtl::OUString sName;
        if(anotherAny >>= sName)
            dumpNameAsAttribute(sName, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("SizeProtect");
        sal_Bool bSizeProtect = sal_Bool();
        if(anotherAny >>= bSizeProtect)
            dumpSizeProtectAsAttribute(bSizeProtect, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Transformation");
        drawing::HomogenMatrix3 aTransformation;
        if(anotherAny >>= aTransformation)
            dumpTransformationAsElement(aTransformation, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("NavigationOrder");
        sal_Int32 aNavigationOrder = sal_Int32();
        if(anotherAny >>= aNavigationOrder)
            dumpNavigationOrderAsAttribute(aNavigationOrder, xmlWriter);
    }
    if(xInfo->hasPropertyByName("Hyperlink"))
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Hyperlink");
        rtl::OUString sHyperlink;
        if(anotherAny >>= sHyperlink)
            dumpHyperlinkAsAttribute(sHyperlink, xmlWriter);
    }
}

void dumpPolyPolygonBezierDescriptorService(uno::Reference< beans::XPropertySet > xPropSet, xmlTextWriterPtr xmlWriter)
{
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("PolygonKind");
        drawing::PolygonKind ePolygonKind;
        if(anotherAny >>= ePolygonKind)
            dumpPolygonKindAsAttribute(ePolygonKind, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("PolyPolygonBezier");
        drawing::PolyPolygonBezierCoords aPolyPolygonBezier;
        if(anotherAny >>= aPolyPolygonBezier)
            dumpPolyPolygonBezierCoords(aPolyPolygonBezier, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Geometry");
        drawing::PolyPolygonBezierCoords aGeometry;
        if(anotherAny >>= aGeometry)
            dumpPolyPolygonBezierCoords(aGeometry, xmlWriter);
    }
}

void dumpCustomShapeService(uno::Reference< beans::XPropertySet > xPropSet, xmlTextWriterPtr xmlWriter)
{
    uno::Reference< beans::XPropertySetInfo> xInfo = xPropSet->getPropertySetInfo();
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("CustomShapeEngine");
        rtl::OUString sCustomShapeEngine;
        if(anotherAny >>= sCustomShapeEngine)
            dumpCustomShapeEngineAsAttribute(sCustomShapeEngine, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("CustomShapeData");
        rtl::OUString sCustomShapeData;
        if(anotherAny >>= sCustomShapeData)
            dumpCustomShapeDataAsAttribute(sCustomShapeData, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("CustomShapeGeometry");
        uno::Sequence< beans::PropertyValue> aCustomShapeGeometry;
        if(anotherAny >>= aCustomShapeGeometry)
            dumpCustomShapeGeometryAsElement(aCustomShapeGeometry, xmlWriter);
    }
    if(xInfo->hasPropertyByName("CustomShapeReplacementURL"))
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("CustomShapeReplacementURL");
        rtl::OUString sCustomShapeReplacementURL;
        if(anotherAny >>= sCustomShapeReplacementURL)
            dumpCustomShapeReplacementURLAsAttribute(sCustomShapeReplacementURL, xmlWriter);
    }
}

void dumpXShape(uno::Reference< drawing::XShape > xShape, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterStartElement( xmlWriter, BAD_CAST( "XShape" ) );
    uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySetInfo> xPropSetInfo = xPropSet->getPropertySetInfo();
    rtl::OUString aName;

    dumpPositionAsAttribute(xShape->getPosition(), xmlWriter);
    dumpSizeAsAttribute(xShape->getSize(), xmlWriter);
    uno::Reference< drawing::XShapeDescriptor > xDescr(xShape, uno::UNO_QUERY_THROW);
    dumpShapeDescriptorAsAttribute(xDescr, xmlWriter);

    // uno::Sequence<beans::Property> aProperties = xPropSetInfo->getProperties();

    uno::Reference< lang::XServiceInfo > xServiceInfo( xShape, uno::UNO_QUERY_THROW );
    uno::Sequence< rtl::OUString > aServiceNames = xServiceInfo->getSupportedServiceNames();

    uno::Reference< beans::XPropertySetInfo> xInfo = xPropSet->getPropertySetInfo();
    if(xInfo->hasPropertyByName("Name"))
    {
        uno::Any aAny = xPropSet->getPropertyValue("Name");
        if (aAny >>= aName)
        {
            if (!aName.isEmpty())
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("name"), "%s", rtl::OUStringToOString(aName, RTL_TEXTENCODING_UTF8).getStr());
        }
    }

    try
    {
    if (xServiceInfo->supportsService("com.sun.star.drawing.Text"))
    {
        uno::Reference< text::XText > xText(xShape, uno::UNO_QUERY_THROW);
        rtl::OUString aText = xText->getString();
        if(!aText.isEmpty())
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("text"), "%s", rtl::OUStringToOString(aText, RTL_TEXTENCODING_UTF8).getStr());
    }
    if(xServiceInfo->supportsService("com.sun.star.drawing.TextProperties"))
        dumpTextPropertiesService(xPropSet, xmlWriter);

    if(xServiceInfo->supportsService("com.sun.star.drawing.GroupShape"))
    {
        uno::Reference< drawing::XShapes > xShapes(xShape, uno::UNO_QUERY_THROW);
        dumpXShapes(xShapes, xmlWriter);
    }
    if(xServiceInfo->supportsService("com.sun.star.drawing.FillProperties"))
        dumpFillPropertiesService(xPropSet, xmlWriter);

    if(xServiceInfo->supportsService("com.sun.star.drawing.LineProperties"))
        dumpLinePropertiesService(xPropSet, xmlWriter);

    if(xServiceInfo->supportsService("com.sun.star.drawing.PolyPolygonDescriptor"))
        dumpPolyPolygonDescriptorService(xPropSet, xmlWriter);

    if(xServiceInfo->supportsService("com.sun.star.drawing.ShadowProperties"))
        dumpShadowPropertiesService(xPropSet, xmlWriter);

    if(xServiceInfo->supportsService("com.sun.star.drawing.Shape"))
        dumpShapeService(xPropSet, xmlWriter);

    if(xServiceInfo->supportsService("com.sun.star.drawing.PolyPolygonBezierDescriptor"))
        dumpPolyPolygonBezierDescriptorService(xPropSet, xmlWriter);

    if(xServiceInfo->supportsService("com.sun.star.drawing.CustomShape"))
        dumpCustomShapeService(xPropSet, xmlWriter);

    // EnhancedShapeDumper used

    if(xServiceInfo->supportsService("com.sun.star.drawing.EnhancedCustomShapeExtrusion"))
    {
        EnhancedShapeDumper enhancedDumper(xmlWriter);
        enhancedDumper.dumpEnhancedCustomShapeExtrusionService(xPropSet);
    }
    if(xServiceInfo->supportsService("com.sun.star.drawing.EnhancedCustomShapeGeometry"))
    {
        EnhancedShapeDumper enhancedDumper(xmlWriter);
        enhancedDumper.dumpEnhancedCustomShapeGeometryService(xPropSet);
    }
    if(xServiceInfo->supportsService("com.sun.star.drawing.EnhancedCustomShapeHandle"))
    {
        EnhancedShapeDumper enhancedDumper(xmlWriter);
        enhancedDumper.dumpEnhancedCustomShapeHandleService(xPropSet);
    }
    if(xServiceInfo->supportsService("com.sun.star.drawing.EnhancedCustomShapePath"))
    {
        EnhancedShapeDumper enhancedDumper(xmlWriter);
        enhancedDumper.dumpEnhancedCustomShapePathService(xPropSet);
    }
    if(xServiceInfo->supportsService("com.sun.star.drawing.EnhancedCustomShapeTextPath"))
    {
        EnhancedShapeDumper enhancedDumper(xmlWriter);
        enhancedDumper.dumpEnhancedCustomShapeTextPathService(xPropSet);
    }
    }   // end of the 'try' block
    catch (const beans::UnknownPropertyException& e)
    {
        std::cout << "Exception caught in XShapeDumper.cxx: " << e.Message << std::endl;
    }

    #if DEBUG_DUMPER
        sal_Int32 nServices = aServiceNames.getLength();
        for (sal_Int32 i = 0; i < nServices; ++i)
        {
            xmlTextWriterStartElement(xmlWriter, BAD_CAST( "ServiceName" ));
            xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST( "name" ), "%s", rtl::OUStringToOString(aServiceNames[i], RTL_TEXTENCODING_UTF8).getStr());
            xmlTextWriterEndElement( xmlWriter );
        }
    #endif

    xmlTextWriterEndElement( xmlWriter );
}

void dumpXShapes( uno::Reference< drawing::XShapes > xShapes, xmlTextWriterPtr xmlWriter )
{
    xmlTextWriterStartElement( xmlWriter, BAD_CAST( "XShapes" ) );
    uno::Reference< container::XIndexAccess > xIA( xShapes, uno::UNO_QUERY_THROW);
    sal_Int32 nLength = xIA->getCount();
    for (sal_Int32 i = 0; i < nLength; ++i)
    {
        uno::Reference< drawing::XShape > xShape( xIA->getByIndex( i ), uno::UNO_QUERY_THROW );
        dumpXShape( xShape, xmlWriter );
    }

    xmlTextWriterEndElement( xmlWriter );
}
} //end of namespace

rtl::OUString XShapeDumper::dump(uno::Reference<drawing::XShapes> xPageShapes)
{

    rtl::OStringBuffer aString;
    xmlOutputBufferPtr xmlOutBuffer = xmlOutputBufferCreateIO( writeCallback, closeCallback, &aString, NULL );
    xmlTextWriterPtr xmlWriter = xmlNewTextWriter( xmlOutBuffer );
    xmlTextWriterSetIndent( xmlWriter, 1 );

    xmlTextWriterStartDocument( xmlWriter, NULL, NULL, NULL );

    try
    {
        dumpXShapes( xPageShapes, xmlWriter );
    }
    catch (const beans::UnknownPropertyException& e)
    {
        std::cout << "Exception caught in XShapeDumper: " << e.Message << std::endl;
    }

    xmlTextWriterEndDocument( xmlWriter );
    xmlFreeTextWriter( xmlWriter );

    return OStringToOUString(aString.makeStringAndClear(), RTL_TEXTENCODING_UTF8);
}

