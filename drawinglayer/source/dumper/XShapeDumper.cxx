/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <drawinglayer/XShapeDumper.hxx>
#include "EnhancedShapeDumper.hxx"
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/drawing/RectanglePoint.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>

#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>

#include <com/sun/star/drawing/PolygonKind.hpp>

#include <com/sun/star/drawing/TextFitToSizeType.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/TextAnimationDirection.hpp>
#include <com/sun/star/drawing/TextAnimationKind.hpp>
#include <com/sun/star/text/WritingMode.hpp>

#include <com/sun/star/drawing/HomogenMatrixLine3.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XText.hpp>
#include <rtl/strbuf.hxx>
#include <libxml/xmlwriter.h>
#include <iostream>
#include <string_view>
#include <rtl/ustring.hxx>

#define DEBUG_DUMPER 0

using namespace com::sun::star;

namespace {

// FUNCTION DECLARATIONS

// auxiliary functions
void dumpGradientProperty(const css::awt::Gradient& rGradient, xmlTextWriterPtr xmlWriter);
void dumpPolyPolygonBezierCoords(const css::drawing::PolyPolygonBezierCoords& rPolyPolygonBezierCoords, xmlTextWriterPtr xmlWriter);
void dumpPointSequenceSequence(const css::drawing::PointSequenceSequence& rPointSequenceSequence, const uno::Sequence<uno::Sequence<drawing::PolygonFlags> >*, xmlTextWriterPtr xmlWriter);
void dumpPropertyValueAsElement(const beans::PropertyValue& rPropertyValue, xmlTextWriterPtr xmlWriter);

// FillProperties.idl
void dumpFillStyleAsAttribute(css::drawing::FillStyle eFillStyle, xmlTextWriterPtr xmlWriter);
void dumpFillColorAsAttribute(sal_Int32 aColor, xmlTextWriterPtr xmlWriter);
void dumpFillTransparenceAsAttribute(sal_Int32 aTransparence, xmlTextWriterPtr xmlWriter);
void dumpFillTransparenceGradientNameAsAttribute(std::u16string_view sTranspGradName, xmlTextWriterPtr xmlWriter);
void dumpFillTransparenceGradientAsElement(const css::awt::Gradient& rTranspGrad, xmlTextWriterPtr xmlWriter);
void dumpFillGradientNameAsAttribute(std::u16string_view sGradName, xmlTextWriterPtr xmlWriter);
void dumpFillGradientAsElement(const css::awt::Gradient& rGradient, xmlTextWriterPtr xmlWriter);
void dumpFillHatchAsElement(const css::drawing::Hatch& rHatch, xmlTextWriterPtr xmlWriter);
void dumpFillBackgroundAsAttribute(bool bBackground, xmlTextWriterPtr xmlWriter);
void dumpFillBitmapAsElement(const css::uno::Reference<css::awt::XBitmap>& xBitmap, xmlTextWriterPtr xmlWriter);
void dumpFillBitmapPositionOffsetXAsAttribute(sal_Int32 aBitmapPositionOffsetX, xmlTextWriterPtr xmlWriter);
void dumpFillBitmapPositionOffsetYAsAttribute(sal_Int32 aBitmapPositionOffsetY, xmlTextWriterPtr xmlWriter);
void dumpFillBitmapOffsetXAsAttribute(sal_Int32 aBitmapOffsetX, xmlTextWriterPtr xmlWriter);
void dumpFillBitmapOffsetYAsAttribute(sal_Int32 aBitmapOffsetY, xmlTextWriterPtr xmlWriter);
void dumpFillBitmapRectanglePointAsAttribute(css::drawing::RectanglePoint eBitmapRectanglePoint, xmlTextWriterPtr xmlWriter);
void dumpFillBitmapLogicalSizeAsAttribute(bool bBitmapLogicalSize, xmlTextWriterPtr xmlWriter);
void dumpFillBitmapSizeXAsAttribute(sal_Int32 aBitmapSizeX, xmlTextWriterPtr xmlWriter);
void dumpFillBitmapSizeYAsAttribute(sal_Int32 aBitmapSizeY, xmlTextWriterPtr xmlWriter);
void dumpFillBitmapModeAsAttribute(css::drawing::BitmapMode eBitmapMode, xmlTextWriterPtr xmlWriter);
void dumpFillBitmapStretchAsAttribute(bool bBitmapStretch, xmlTextWriterPtr xmlWriter);
void dumpFillBitmapTileAsAttribute(bool bBitmapTile, xmlTextWriterPtr xmlWriter);

// LineProperties.idl
void dumpLineStyleAsAttribute(css::drawing::LineStyle eLineStyle, xmlTextWriterPtr xmlWriter);
void dumpLineDashAsElement(const css::drawing::LineDash& rLineDash, xmlTextWriterPtr xmlWriter);
void dumpLineDashNameAsAttribute(std::u16string_view sLineDashName, xmlTextWriterPtr xmlWriter);
void dumpLineColorAsAttribute(sal_Int32 aLineColor, xmlTextWriterPtr xmlWriter);
void dumpLineTransparenceAsAttribute(sal_Int32 aLineTransparence, xmlTextWriterPtr xmlWriter);
void dumpLineWidthAsAttribute(sal_Int32 aLineWidth, xmlTextWriterPtr xmlWriter);
void dumpLineJointAsAttribute(css::drawing::LineJoint eLineJoint, xmlTextWriterPtr xmlWriter);
void dumpLineStartNameAsAttribute(std::u16string_view sLineStartName, xmlTextWriterPtr xmlWriter);
void dumpLineEndNameAsAttribute(std::u16string_view sLineEndName, xmlTextWriterPtr xmlWriter);
void dumpLineStartAsElement(const css::drawing::PolyPolygonBezierCoords& rLineStart, xmlTextWriterPtr xmlWriter);
void dumpLineEndAsElement(const css::drawing::PolyPolygonBezierCoords& rLineEnd, xmlTextWriterPtr xmlWriter);
void dumpLineStartCenterAsAttribute(bool bLineStartCenter, xmlTextWriterPtr xmlWriter);
void dumpLineStartWidthAsAttribute(sal_Int32 aLineStartWidth, xmlTextWriterPtr xmlWriter);
void dumpLineEndCenterAsAttribute(bool bLineEndCenter, xmlTextWriterPtr xmlWriter);
void dumpLineEndWidthAsAttribute(sal_Int32 aLineEndWidth, xmlTextWriterPtr xmlWriter);

// PolyPolygonDescriptor.idl
void dumpPolygonKindAsAttribute(css::drawing::PolygonKind ePolygonKind, xmlTextWriterPtr xmlWriter);
void dumpPolyPolygonAsElement(const css::drawing::PointSequenceSequence& rPolyPolygon, xmlTextWriterPtr xmlWriter);
void dumpGeometryAsElement(const css::drawing::PointSequenceSequence& rGeometry, xmlTextWriterPtr xmlWriter);

// CharacterProperties.idl
void dumpCharHeightAsAttribute(float fHeight, xmlTextWriterPtr xmlWriter);
void dumpCharColorAsAttribute(sal_Int32 aColor, xmlTextWriterPtr xmlWriter);

// TextProperties.idl
void dumpIsNumberingAsAttribute(bool bIsNumbering, xmlTextWriterPtr xmlWriter);
void dumpTextAutoGrowHeightAsAttribute(bool bTextAutoGrowHeight, xmlTextWriterPtr xmlWriter);
void dumpTextAutoGrowWidthAsAttribute(bool bTextAutoGrowWidth, xmlTextWriterPtr xmlWriter);
void dumpTextContourFrameAsAttribute(bool bTextContourFrame, xmlTextWriterPtr xmlWriter);
void dumpTextFitToSizeAsAttribute(css::drawing::TextFitToSizeType eTextFitToSize, xmlTextWriterPtr xmlWriter);
void dumpTextHorizontalAdjustAsAttribute(css::drawing::TextHorizontalAdjust eTextHorizontalAdjust, xmlTextWriterPtr xmlWriter);
void dumpTextVerticalAdjustAsAttribute(css::drawing::TextVerticalAdjust eTextVerticalAdjust, xmlTextWriterPtr xmlWriter);
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
void dumpTextAnimationDirectionAsAttribute(css::drawing::TextAnimationDirection eTextAnimationDirection, xmlTextWriterPtr xmlWriter);
void dumpTextAnimationKindAsAttribute(css::drawing::TextAnimationKind eTextAnimationKind, xmlTextWriterPtr xmlWriter);
void dumpTextAnimationStartInsideAsAttribute(bool bTextAnimationStartInside, xmlTextWriterPtr xmlWriter);
void dumpTextAnimationStopInsideAsAttribute(bool bTextAnimationStopInside, xmlTextWriterPtr xmlWriter);
void dumpTextWritingModeAsAttribute(css::text::WritingMode eWritingMode, xmlTextWriterPtr xmlWriter);

// ShadowProperties.idl
void dumpShadowAsAttribute(bool bShadow, xmlTextWriterPtr xmlWriter);
void dumpShadowColorAsAttribute(sal_Int32 aShadowColor, xmlTextWriterPtr xmlWriter);
void dumpShadowTransparenceAsAttribute(sal_Int32 aShadowTransparence, xmlTextWriterPtr xmlWriter);
void dumpShadowXDistanceAsAttribute(sal_Int32 aShadowXDistance, xmlTextWriterPtr xmlWriter);
void dumpShadowYDistanceAsAttribute(sal_Int32 aShadowYDistance, xmlTextWriterPtr xmlWriter);

//Shape.idl
void dumpZOrderAsAttribute(sal_Int32 aZOrder, xmlTextWriterPtr xmlWriter);
void dumpLayerIDAsAttribute(sal_Int32 aLayerID, xmlTextWriterPtr xmlWriter);
void dumpLayerNameAsAttribute(std::u16string_view sLayerName, xmlTextWriterPtr xmlWriter);
void dumpVisibleAsAttribute(bool bVisible, xmlTextWriterPtr xmlWriter);
void dumpPrintableAsAttribute(bool bPrintable, xmlTextWriterPtr xmlWriter);
void dumpMoveProtectAsAttribute(bool bMoveProtect, xmlTextWriterPtr xmlWriter);
void dumpNameAsAttribute(std::u16string_view sName, xmlTextWriterPtr xmlWriter);
void dumpSizeProtectAsAttribute(bool bSizeProtect, xmlTextWriterPtr xmlWriter);
void dumpHomogenMatrixLine3(const css::drawing::HomogenMatrixLine3& rLine, xmlTextWriterPtr xmlWriter);
void dumpTransformationAsElement(const css::drawing::HomogenMatrix3& rTransformation, xmlTextWriterPtr xmlWriter);
void dumpNavigationOrderAsAttribute(sal_Int32 aNavigationOrder, xmlTextWriterPtr xmlWriter);
void dumpHyperlinkAsAttribute(std::u16string_view sHyperlink, xmlTextWriterPtr xmlWriter);
void dumpInteropGrabBagAsElement(const uno::Sequence< beans::PropertyValue>& aInteropGrabBag, xmlTextWriterPtr xmlWriter);

// CustomShape.idl
void dumpCustomShapeEngineAsAttribute(std::u16string_view sCustomShapeEngine, xmlTextWriterPtr xmlWriter);
void dumpCustomShapeDataAsAttribute(
    std::u16string_view sCustomShapeData, xmlTextWriterPtr xmlWriter);
void dumpCustomShapeGeometryAsElement(const css::uno::Sequence< css::beans::PropertyValue>& aCustomShapeGeometry, xmlTextWriterPtr xmlWriter);
void dumpCustomShapeReplacementURLAsAttribute(std::u16string_view sCustomShapeReplacementURL, xmlTextWriterPtr xmlWriter);

// XShape.idl
void dumpPositionAsAttribute(const css::awt::Point& rPoint, xmlTextWriterPtr xmlWriter);
void dumpSizeAsAttribute(const css::awt::Size& rSize, xmlTextWriterPtr xmlWriter);

// the rest
void dumpShapeDescriptorAsAttribute( const css::uno::Reference< css::drawing::XShapeDescriptor >& xDescr, xmlTextWriterPtr xmlWriter );
void dumpXShape(const css::uno::Reference< css::drawing::XShape >& xShape, xmlTextWriterPtr xmlWriter, bool bDumpInteropProperties);
void dumpXShapes( const css::uno::Reference< css::drawing::XShapes >& xShapes, xmlTextWriterPtr xmlWriter, bool bDumpInteropProperties );
void dumpTextPropertiesService(const css::uno::Reference< css::beans::XPropertySet >& xPropSet, xmlTextWriterPtr xmlWriter);
void dumpFillPropertiesService(const css::uno::Reference< css::beans::XPropertySet >& xPropSet, xmlTextWriterPtr xmlWriter);
void dumpLinePropertiesService(const css::uno::Reference< css::beans::XPropertySet >& xPropSet, xmlTextWriterPtr xmlWriter);
void dumpShadowPropertiesService(const css::uno::Reference< css::beans::XPropertySet >& xPropSet, xmlTextWriterPtr xmlWriter);
void dumpPolyPolygonDescriptorService(const css::uno::Reference< css::beans::XPropertySet >& xPropSet, xmlTextWriterPtr xmlWriter);
void dumpShapeService(const css::uno::Reference< css::beans::XPropertySet >& xPropSet, xmlTextWriterPtr xmlWriter, bool bDumpInteropProperties);
void dumpPolyPolygonBezierDescriptorService(const css::uno::Reference< css::beans::XPropertySet >& xPropSet, xmlTextWriterPtr xmlWriter);
void dumpCustomShapeService(const css::uno::Reference< css::beans::XPropertySet >& xPropSet, xmlTextWriterPtr xmlWriter);


int writeCallback(void* pContext, const char* sBuffer, int nLen)
{
    OStringBuffer* pBuffer = static_cast<OStringBuffer*>(pContext);
    pBuffer->append(sBuffer);
    return nLen;
}

int closeCallback(void* )
{
    return 0;
}

bool m_bNameDumped;


// ---------- FillProperties.idl ----------

void dumpFillStyleAsAttribute(drawing::FillStyle eFillStyle, xmlTextWriterPtr xmlWriter)
{
    switch(eFillStyle)
    {
        case drawing::FillStyle_NONE:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillStyle"), "%s", "NONE");
            break;
        case drawing::FillStyle_SOLID:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillStyle"), "%s", "SOLID");
            break;
        case drawing::FillStyle_GRADIENT:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillStyle"), "%s", "GRADIENT");
            break;
        case drawing::FillStyle_HATCH:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillStyle"), "%s", "HATCH");
            break;
        case drawing::FillStyle_BITMAP:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillStyle"), "%s", "BITMAP");
            break;
        default:
            break;
    }
}

void dumpFillColorAsAttribute(sal_Int32 aColor, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillColor"), "%06x", static_cast<unsigned int>(aColor));
}

void dumpFillTransparenceAsAttribute(sal_Int32 aTransparence, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillTransparence"), "%" SAL_PRIdINT32, aTransparence);
}

void dumpFillTransparenceGradientNameAsAttribute(std::u16string_view sTranspGradName, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillTransparenceGradientName"), "%s",
        OUStringToOString(sTranspGradName, RTL_TEXTENCODING_UTF8).getStr());
}

//because there's more awt::Gradient properties to dump
void dumpGradientProperty(const awt::Gradient& rGradient, xmlTextWriterPtr xmlWriter)
{
    switch (rGradient.Style)   //enum GradientStyle
    {
        case awt::GradientStyle_LINEAR:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "LINEAR");
            break;
        case awt::GradientStyle_AXIAL:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "AXIAL");
            break;
        case awt::GradientStyle_RADIAL:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "RADIAL");
            break;
        case awt::GradientStyle_ELLIPTICAL:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "ELLIPTICAL");
            break;
        case awt::GradientStyle_SQUARE:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "SQUARE");
            break;
        case awt::GradientStyle_RECT:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "RECT");
            break;
        default:
            break;
    }
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("startColor"), "%06x", static_cast<unsigned int>(rGradient.StartColor));
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("endColor"), "%06x", static_cast<unsigned int>(rGradient.EndColor));
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("angle"), "%" SAL_PRIdINT32, static_cast<sal_Int32>(rGradient.Angle));
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("border"), "%" SAL_PRIdINT32, static_cast<sal_Int32>(rGradient.Border));
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("xOffset"), "%" SAL_PRIdINT32, static_cast<sal_Int32>(rGradient.XOffset));
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("yOffset"), "%" SAL_PRIdINT32, static_cast<sal_Int32>(rGradient.YOffset));
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("startIntensity"), "%" SAL_PRIdINT32, static_cast<sal_Int32>(rGradient.StartIntensity));
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("endIntensity"), "%" SAL_PRIdINT32, static_cast<sal_Int32>(rGradient.EndIntensity));
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("stepCount"), "%" SAL_PRIdINT32, static_cast<sal_Int32>(rGradient.StepCount));
}

void dumpFillTransparenceGradientAsElement(const awt::Gradient& rTranspGrad, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "FillTransparenceGradient" ));
    dumpGradientProperty(rTranspGrad, xmlWriter);
    (void)xmlTextWriterEndElement( xmlWriter );
}

void dumpFillGradientNameAsAttribute(std::u16string_view sGradName, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillGradientName"), "%s",
        OUStringToOString(sGradName, RTL_TEXTENCODING_UTF8).getStr());
}

void dumpFillGradientAsElement(const awt::Gradient& rGradient, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "FillGradient" ));
    dumpGradientProperty(rGradient, xmlWriter);
    (void)xmlTextWriterEndElement( xmlWriter );
}

void dumpFillHatchAsElement(const drawing::Hatch& rHatch, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "FillHatch" ));
    switch (rHatch.Style)
    {
        case drawing::HatchStyle_SINGLE:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "SINGLE");
            break;
        case drawing::HatchStyle_DOUBLE:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "DOUBLE");
            break;
        case drawing::HatchStyle_TRIPLE:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "TRIPLE");
            break;
        default:
            break;
    }
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("color"), "%06x", static_cast<unsigned int>(rHatch.Color));
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("distance"), "%" SAL_PRIdINT32, rHatch.Distance);
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("angle"), "%" SAL_PRIdINT32, rHatch.Angle);
    (void)xmlTextWriterEndElement( xmlWriter );
}

void dumpFillBackgroundAsAttribute(bool bBackground, xmlTextWriterPtr xmlWriter)
{
    if(bBackground)
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBackground"), "%s", "true");
    else
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBackground"), "%s", "false");
}

void dumpFillBitmapAsElement(const uno::Reference<awt::XBitmap>& xBitmap, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "FillBitmap" ));
    if (xBitmap.is())
    {
        awt::Size const aSize = xBitmap->getSize();
        (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("width"), "%" SAL_PRIdINT32, aSize.Width);
        (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("height"), "%" SAL_PRIdINT32, aSize.Height);
    }
    (void)xmlTextWriterEndElement( xmlWriter );
}

void dumpFillBitmapPositionOffsetXAsAttribute(sal_Int32 aBitmapPositionOffsetX, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillBitmapPositionOffsetX"), "%" SAL_PRIdINT32, aBitmapPositionOffsetX);
}

void dumpFillBitmapPositionOffsetYAsAttribute(sal_Int32 aBitmapPositionOffsetY, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillBitmapPositionOffsetY"), "%" SAL_PRIdINT32, aBitmapPositionOffsetY);
}

void dumpFillBitmapOffsetXAsAttribute(sal_Int32 aBitmapOffsetX, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillBitmapOffsetX"), "%" SAL_PRIdINT32, aBitmapOffsetX);
}

void dumpFillBitmapOffsetYAsAttribute(sal_Int32 aBitmapOffsetY, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillBitmapOffsetY"), "%" SAL_PRIdINT32, aBitmapOffsetY);
}

void dumpFillBitmapRectanglePointAsAttribute(drawing::RectanglePoint eBitmapRectanglePoint, xmlTextWriterPtr xmlWriter)
{
    switch(eBitmapRectanglePoint)
    {
        case drawing::RectanglePoint_LEFT_TOP:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapRectanglePoint"), "%s", "LEFT_TOP");
            break;
        case drawing::RectanglePoint_MIDDLE_TOP:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapRectanglePoint"), "%s", "MIDDLE_TOP");
            break;
        case drawing::RectanglePoint_RIGHT_TOP:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapRectanglePoint"), "%s", "RIGHT_TOP");
            break;
        case drawing::RectanglePoint_LEFT_MIDDLE:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapRectanglePoint"), "%s", "LEFT_MIDDLE");
            break;
        case drawing::RectanglePoint_MIDDLE_MIDDLE:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapRectanglePoint"), "%s", "MIDDLE_MIDDLE");
            break;
        case drawing::RectanglePoint_RIGHT_MIDDLE:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapRectanglePoint"), "%s", "RIGHT_MIDDLE");
            break;
        case drawing::RectanglePoint_LEFT_BOTTOM:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapRectanglePoint"), "%s", "LEFT_BOTTOM");
            break;
        case drawing::RectanglePoint_MIDDLE_BOTTOM:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapRectanglePoint"), "%s", "MIDDLE_BOTTOM");
            break;
        case drawing::RectanglePoint_RIGHT_BOTTOM:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapRectanglePoint"), "%s", "RIGHT_BOTTOM");
            break;
        default:
            break;
    }
}

void dumpFillBitmapLogicalSizeAsAttribute(bool bBitmapLogicalSize, xmlTextWriterPtr xmlWriter)
{
    if(bBitmapLogicalSize)
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapLogicalSize"), "%s", "true");
    else
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapLogicalSize"), "%s", "false");
}

void dumpFillBitmapSizeXAsAttribute(sal_Int32 aBitmapSizeX, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillBitmapSizeX"), "%" SAL_PRIdINT32, aBitmapSizeX);
}

void dumpFillBitmapSizeYAsAttribute(sal_Int32 aBitmapSizeY, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillBitmapSizeY"), "%" SAL_PRIdINT32, aBitmapSizeY);
}

void dumpFillBitmapModeAsAttribute(drawing::BitmapMode eBitmapMode, xmlTextWriterPtr xmlWriter)
{
    switch(eBitmapMode)
    {
        case drawing::BitmapMode_REPEAT:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapMode"), "%s", "REPEAT");
            break;
        case drawing::BitmapMode_STRETCH:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapMode"), "%s", "STRETCH");
            break;
        case drawing::BitmapMode_NO_REPEAT:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapMode"), "%s", "NO_REPEAT");
            break;
        default:
            break;
    }
}

void dumpFillBitmapStretchAsAttribute(bool bBitmapStretch, xmlTextWriterPtr xmlWriter)
{
    if(bBitmapStretch)
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapStretch"), "%s", "true");
    else
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapStretch"), "%s", "false");
}

void dumpFillBitmapTileAsAttribute(bool bBitmapTile, xmlTextWriterPtr xmlWriter)
{
    if(bBitmapTile)
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapTile"), "%s", "true");
    else
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapTile"), "%s", "false");
}


// ---------- LineProperties.idl ----------


void dumpLineStyleAsAttribute(drawing::LineStyle eLineStyle, xmlTextWriterPtr xmlWriter)
{
    switch(eLineStyle)
    {
        case drawing::LineStyle_NONE:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("lineStyle"), "%s", "NONE");
            break;
        case drawing::LineStyle_SOLID:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("lineStyle"), "%s", "SOLID");
            break;
        case drawing::LineStyle_DASH:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("lineStyle"), "%s", "DASH");
            break;
        default:
            break;
    }
}

void dumpLineDashAsElement(const drawing::LineDash& rLineDash, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "LineDash" ));
    switch (rLineDash.Style)
    {
        case drawing::DashStyle_RECT:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "RECT");
            break;
        case drawing::DashStyle_ROUND:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "ROUND");
            break;
        case drawing::DashStyle_RECTRELATIVE:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "RECTRELATIVE");
            break;
        case drawing::DashStyle_ROUNDRELATIVE:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "ROUNDRELATIVE");
            break;
        default:
            break;
    }
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("dots"), "%" SAL_PRIdINT32, static_cast<sal_Int32>(rLineDash.Dots));
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("dotLen"), "%" SAL_PRIdINT32, rLineDash.DotLen);
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("dashes"), "%" SAL_PRIdINT32, static_cast<sal_Int32>(rLineDash.Dashes));
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("dashLen"), "%" SAL_PRIdINT32, rLineDash.DashLen);
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("distance"), "%" SAL_PRIdINT32, rLineDash.Distance);
    (void)xmlTextWriterEndElement( xmlWriter );
}

void dumpLineDashNameAsAttribute(std::u16string_view sLineDashName, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("lineDashName"), "%s",
        OUStringToOString(sLineDashName, RTL_TEXTENCODING_UTF8).getStr());
}

void dumpLineColorAsAttribute(sal_Int32 aLineColor, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("lineColor"), "%06x", static_cast<unsigned int>(aLineColor));
}

void dumpLineTransparenceAsAttribute(sal_Int32 aLineTransparence, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("lineTransparence"), "%" SAL_PRIdINT32, aLineTransparence);
}

void dumpLineWidthAsAttribute(sal_Int32 aLineWidth, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("lineWidth"), "%" SAL_PRIdINT32, aLineWidth);
}

void dumpLineJointAsAttribute(drawing::LineJoint eLineJoint, xmlTextWriterPtr xmlWriter)
{
    switch(eLineJoint)
    {
        case drawing::LineJoint_NONE:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("lineJoint"), "%s", "NONE");
            break;
        case drawing::LineJoint_MIDDLE:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("lineJoint"), "%s", "MIDDLE");
            break;
        case drawing::LineJoint_BEVEL:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("lineJoint"), "%s", "BEVEL");
            break;
        case drawing::LineJoint_MITER:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("lineJoint"), "%s", "MITER");
            break;
        case drawing::LineJoint_ROUND:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("lineJoint"), "%s", "ROUND");
            break;
        default:
            break;
    }
}

void dumpLineStartNameAsAttribute(std::u16string_view sLineStartName, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("lineStartName"), "%s",
    OUStringToOString(sLineStartName, RTL_TEXTENCODING_UTF8).getStr());
}

void dumpLineEndNameAsAttribute(std::u16string_view sLineEndName, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("lineEndName"), "%s",
    OUStringToOString(sLineEndName, RTL_TEXTENCODING_UTF8).getStr());
}

void dumpPolyPolygonBezierCoords(const drawing::PolyPolygonBezierCoords& rPolyPolygonBezierCoords, xmlTextWriterPtr xmlWriter)
{
    dumpPointSequenceSequence(rPolyPolygonBezierCoords.Coordinates, &rPolyPolygonBezierCoords.Flags, xmlWriter);
}

void dumpLineStartAsElement(const drawing::PolyPolygonBezierCoords& rLineStart, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "LineStart" ));
    dumpPolyPolygonBezierCoords(rLineStart, xmlWriter);
    (void)xmlTextWriterEndElement( xmlWriter );
}

void dumpLineEndAsElement(const drawing::PolyPolygonBezierCoords& rLineEnd, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "LineEnd" ));
    dumpPolyPolygonBezierCoords(rLineEnd, xmlWriter);
    (void)xmlTextWriterEndElement( xmlWriter );
}

void dumpLineStartCenterAsAttribute(bool bLineStartCenter, xmlTextWriterPtr xmlWriter)
{
    if(bLineStartCenter)
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("lineStartCenter"), "%s", "true");
    else
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("lineStartCenter"), "%s", "false");
}

void dumpLineStartWidthAsAttribute(sal_Int32 aLineStartWidth, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("lineStartWidth"), "%" SAL_PRIdINT32, aLineStartWidth);
}

void dumpLineEndCenterAsAttribute(bool bLineEndCenter, xmlTextWriterPtr xmlWriter)
{
    if(bLineEndCenter)
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("lineEndCenter"), "%s", "true");
    else
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("lineEndCenter"), "%s", "false");
}

void dumpLineEndWidthAsAttribute(sal_Int32 aLineEndWidth, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("lineEndWidth"), "%" SAL_PRIdINT32, aLineEndWidth);
}


// ---------- PolyPolygonDescriptor.idl ----------


void dumpPolygonKindAsAttribute(drawing::PolygonKind ePolygonKind, xmlTextWriterPtr xmlWriter)
{
    switch(ePolygonKind)
    {
        case drawing::PolygonKind_LINE:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("polygonKind"), "%s", "LINE");
            break;
        case drawing::PolygonKind_POLY:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("polygonKind"), "%s", "POLY");
            break;
        case drawing::PolygonKind_PLIN:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("polygonKind"), "%s", "PLIN");
            break;
        case drawing::PolygonKind_PATHLINE:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("polygonKind"), "%s", "PATHLINE");
            break;
        case drawing::PolygonKind_PATHFILL:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("polygonKind"), "%s", "PATHFILL");
            break;
        case drawing::PolygonKind_FREELINE:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("polygonKind"), "%s", "FREELINE");
            break;
        case drawing::PolygonKind_FREEFILL:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("polygonKind"), "%s", "FREEFILL");
            break;
        case drawing::PolygonKind_PATHPOLY:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("polygonKind"), "%s", "PATHPOLY");
            break;
        case drawing::PolygonKind_PATHPLIN:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("polygonKind"), "%s", "PATHPLIN");
            break;
        default:
            break;
    }
}

void dumpPointSequenceSequence(const drawing::PointSequenceSequence& aPointSequenceSequence, const uno::Sequence<uno::Sequence< drawing::PolygonFlags > >* pFlags, xmlTextWriterPtr xmlWriter)
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

        (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "pointSequence" ));

        for(sal_Int32 j = 0; j < nPoints; ++j)
        {
            (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "point" ));
            (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("positionX"), "%" SAL_PRIdINT32, pointSequence[j].X);
            (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("positionY"), "%" SAL_PRIdINT32, pointSequence[j].Y);

            if(pFlags)
            {
                switch(flagsSequence[j])
                {
                    case drawing::PolygonFlags_NORMAL:
                        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("polygonFlags"), "%s", "NORMAL");
                        break;
                    case drawing::PolygonFlags_SMOOTH:
                        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("polygonFlags"), "%s", "SMOOTH");
                        break;
                    case drawing::PolygonFlags_CONTROL:
                        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("polygonFlags"), "%s", "CONTROL");
                        break;
                    case drawing::PolygonFlags_SYMMETRIC:
                        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("polygonFlags"), "%s", "SYMMETRIC");
                        break;
                    default:
                        break;
                }
            }

            (void)xmlTextWriterEndElement( xmlWriter );
        }
        (void)xmlTextWriterEndElement( xmlWriter );
    }
}

void dumpPolyPolygonAsElement(const drawing::PointSequenceSequence& rPolyPolygon, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "PolyPolygon" ));
    dumpPointSequenceSequence(rPolyPolygon, nullptr, xmlWriter);
    (void)xmlTextWriterEndElement( xmlWriter );
}

void dumpGeometryAsElement(const drawing::PointSequenceSequence& aGeometry, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Geometry" ));
    dumpPointSequenceSequence(aGeometry, nullptr, xmlWriter);
    (void)xmlTextWriterEndElement( xmlWriter );
}

// CharacterProperties.idl
void dumpCharHeightAsAttribute(float fHeight, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fontHeight"), "%f", fHeight );
}

void dumpCharColorAsAttribute(sal_Int32 aColor, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fontColor"), "%06x", static_cast<unsigned int>(aColor));
}


// ---------- TextProperties.idl ----------


void dumpIsNumberingAsAttribute(bool bIsNumbering, xmlTextWriterPtr xmlWriter)
{
    if(bIsNumbering)
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("isNumbering"), "%s", "true");
    else
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("isNumbering"), "%s", "false");
}

void dumpTextAutoGrowHeightAsAttribute(bool bTextAutoGrowHeight, xmlTextWriterPtr xmlWriter)
{
    if(bTextAutoGrowHeight)
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAutoGrowHeight"), "%s", "true");
    else
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAutoGrowHeight"), "%s", "false");
}

void dumpTextAutoGrowWidthAsAttribute(bool bTextAutoGrowWidth, xmlTextWriterPtr xmlWriter)
{
    if(bTextAutoGrowWidth)
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAutoGrowWidth"), "%s", "true");
    else
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAutoGrowWidth"), "%s", "false");
}

void dumpTextContourFrameAsAttribute(bool bTextContourFrame, xmlTextWriterPtr xmlWriter)
{
    if(bTextContourFrame)
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textContourFrame"), "%s", "true");
    else
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textContourFrame"), "%s", "false");
}

void dumpTextFitToSizeAsAttribute(drawing::TextFitToSizeType eTextFitToSize, xmlTextWriterPtr xmlWriter)
{
    switch(eTextFitToSize)
    {
        case drawing::TextFitToSizeType_NONE:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textFitToSize"), "%s", "NONE");
            break;
        case drawing::TextFitToSizeType_PROPORTIONAL:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textFitToSize"), "%s", "PROPORTIONAL");
            break;
        case drawing::TextFitToSizeType_ALLLINES:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textFitToSize"), "%s", "ALLLINES");
            break;
        case drawing::TextFitToSizeType_AUTOFIT:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textFitToSize"), "%s", "AUTOFIT");
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
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textHorizontalAdjust"), "%s", "LEFT");
            break;
        case drawing::TextHorizontalAdjust_CENTER:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textHorizontalAdjust"), "%s", "CENTER");
            break;
        case drawing::TextHorizontalAdjust_RIGHT:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textHorizontalAdjust"), "%s", "RIGHT");
            break;
        case drawing::TextHorizontalAdjust_BLOCK:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textHorizontalAdjust"), "%s", "BLOCK");
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
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textVerticalAdjust"), "%s", "TOP");
            break;
        case drawing::TextVerticalAdjust_CENTER:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textVerticalAdjust"), "%s", "CENTER");
            break;
        case drawing::TextVerticalAdjust_BOTTOM:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textVerticalAdjust"), "%s", "BOTTOM");
            break;
        case drawing::TextVerticalAdjust_BLOCK:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textVerticalAdjust"), "%s", "BLOCK");
            break;
        default:
            break;
    }
}

void dumpTextLeftDistanceAsAttribute(sal_Int32 aTextLeftDistance, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("textLeftDistance"), "%" SAL_PRIdINT32, aTextLeftDistance);
}

void dumpTextRightDistanceAsAttribute(sal_Int32 aTextRightDistance, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("textRightDistance"), "%" SAL_PRIdINT32, aTextRightDistance);
}

void dumpTextUpperDistanceAsAttribute(sal_Int32 aTextUpperDistance, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("textUpperDistance"), "%" SAL_PRIdINT32, aTextUpperDistance);
}

void dumpTextLowerDistanceAsAttribute(sal_Int32 aTextLowerDistance, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("textLowerDistance"), "%" SAL_PRIdINT32, aTextLowerDistance);
}

void dumpTextMaximumFrameHeightAsAttribute(sal_Int32 aTextMaximumFrameHeight, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("textMaximumFrameHeight"), "%" SAL_PRIdINT32, aTextMaximumFrameHeight);
}

void dumpTextMaximumFrameWidthAsAttribute(sal_Int32 aTextMaximumFrameWidth, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("textMaximumFrameWidth"), "%" SAL_PRIdINT32, aTextMaximumFrameWidth);
}

void dumpTextMinimumFrameHeightAsAttribute(sal_Int32 aTextMinimumFrameHeight, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("textMinimumFrameHeight"), "%" SAL_PRIdINT32, aTextMinimumFrameHeight);
}

void dumpTextMinimumFrameWidthAsAttribute(sal_Int32 aTextMinimumFrameWidth, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("textMinimumFrameWidth"), "%" SAL_PRIdINT32, aTextMinimumFrameWidth);
}

void dumpTextAnimationAmountAsAttribute(sal_Int32 aTextAnimationAmount, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("textAnimationAmount"), "%" SAL_PRIdINT32, aTextAnimationAmount);
}

void dumpTextAnimationCountAsAttribute(sal_Int32 aTextAnimationCount, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("textAnimationCount"), "%" SAL_PRIdINT32, aTextAnimationCount);
}

void dumpTextAnimationDelayAsAttribute(sal_Int32 aTextAnimationDelay, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("textAnimationDelay"), "%" SAL_PRIdINT32, aTextAnimationDelay);
}

void dumpTextAnimationDirectionAsAttribute(drawing::TextAnimationDirection eTextAnimationDirection, xmlTextWriterPtr xmlWriter)
{
    switch(eTextAnimationDirection)
    {
        case drawing::TextAnimationDirection_LEFT:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAnimationDirection"), "%s", "LEFT");
            break;
        case drawing::TextAnimationDirection_RIGHT:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAnimationDirection"), "%s", "RIGHT");
            break;
        case drawing::TextAnimationDirection_UP:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAnimationDirection"), "%s", "UP");
            break;
        case drawing::TextAnimationDirection_DOWN:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAnimationDirection"), "%s", "DOWN");
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
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAnimationKind"), "%s", "NONE");
            break;
        case drawing::TextAnimationKind_BLINK:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAnimationKind"), "%s", "BLINK");
            break;
        case drawing::TextAnimationKind_SCROLL:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAnimationKind"), "%s", "SCROLL");
            break;
        case drawing::TextAnimationKind_ALTERNATE:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAnimationKind"), "%s", "ALTERNATE");
            break;
        case drawing::TextAnimationKind_SLIDE:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAnimationKind"), "%s", "SLIDE");
            break;
        default:
            break;
    }
}

void dumpTextAnimationStartInsideAsAttribute(bool bTextAnimationStartInside, xmlTextWriterPtr xmlWriter)
{
    if(bTextAnimationStartInside)
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAnimationStartInside"), "%s", "true");
    else
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAnimationStartInside"), "%s", "false");
}

void dumpTextAnimationStopInsideAsAttribute(bool bTextAnimationStopInside, xmlTextWriterPtr xmlWriter)
{
    if(bTextAnimationStopInside)
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAnimationStopInside"), "%s", "true");
    else
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textAnimationStopInside"), "%s", "false");
}

void dumpTextWritingModeAsAttribute(text::WritingMode eTextWritingMode, xmlTextWriterPtr xmlWriter)
{
    switch(eTextWritingMode)
    {
        case text::WritingMode_LR_TB:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textWritingMode"), "%s", "LR_TB");
            break;
        case text::WritingMode_RL_TB:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textWritingMode"), "%s", "RL_TB");
            break;
        case text::WritingMode_TB_RL:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textWritingMode"), "%s", "TB_RL");
            break;
        default:
            break;
    }
}


// ---------- ShadowProperties.idl ----------


void dumpShadowAsAttribute(bool bShadow, xmlTextWriterPtr xmlWriter)
{
    if(bShadow)
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("shadow"), "%s", "true");
    else
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("shadow"), "%s", "false");
}

void dumpShadowColorAsAttribute(sal_Int32 aShadowColor, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("shadowColor"), "%06x", static_cast<unsigned int>(aShadowColor));
}

void dumpShadowTransparenceAsAttribute(sal_Int32 aShadowTransparence, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("shadowTransparence"), "%" SAL_PRIdINT32, aShadowTransparence);
}

void dumpShadowXDistanceAsAttribute(sal_Int32 aShadowXDistance, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("shadowXDistance"), "%" SAL_PRIdINT32, aShadowXDistance);
}

void dumpShadowYDistanceAsAttribute(sal_Int32 aShadowYDistance, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("shadowYDistance"), "%" SAL_PRIdINT32, aShadowYDistance);
}


// ---------- Shape.idl ----------


void dumpZOrderAsAttribute(sal_Int32 aZOrder, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("zOrder"), "%" SAL_PRIdINT32, aZOrder);
}

void dumpLayerIDAsAttribute(sal_Int32 aLayerID, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("layerID"), "%" SAL_PRIdINT32, aLayerID);
}

void dumpLayerNameAsAttribute(std::u16string_view sLayerName, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("layerName"), "%s",
        OUStringToOString(sLayerName, RTL_TEXTENCODING_UTF8).getStr());
}

void dumpVisibleAsAttribute(bool bVisible, xmlTextWriterPtr xmlWriter)
{
    if(bVisible)
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("visible"), "%s", "true");
    else
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("visible"), "%s", "false");
}

void dumpPrintableAsAttribute(bool bPrintable, xmlTextWriterPtr xmlWriter)
{
    if(bPrintable)
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("printable"), "%s", "true");
    else
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("printable"), "%s", "false");
}

void dumpMoveProtectAsAttribute(bool bMoveProtect, xmlTextWriterPtr xmlWriter)
{
    if(bMoveProtect)
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("moveProtect"), "%s", "true");
    else
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("moveProtect"), "%s", "false");
}

void dumpNameAsAttribute(std::u16string_view sName, xmlTextWriterPtr xmlWriter)
{
    if(!sName.empty() && !m_bNameDumped)
    {
        (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("name"), "%s", OUStringToOString(sName, RTL_TEXTENCODING_UTF8).getStr());
        m_bNameDumped = true;
    }
}

void dumpSizeProtectAsAttribute(bool bSizeProtect, xmlTextWriterPtr xmlWriter)
{
    if(bSizeProtect)
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("sizeProtect"), "%s", "true");
    else
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("sizeProtect"), "%s", "false");
}

void dumpHomogenMatrixLine3(const drawing::HomogenMatrixLine3& rHomogenMatrixLine3, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("column1"), "%f", rHomogenMatrixLine3.Column1);
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("column2"), "%f", rHomogenMatrixLine3.Column2);
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("column3"), "%f", rHomogenMatrixLine3.Column3);
}

void dumpTransformationAsElement(const drawing::HomogenMatrix3& rTransformation, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Transformation" ));
    {
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Line1" ));
    dumpHomogenMatrixLine3(rTransformation.Line1, xmlWriter);
    (void)xmlTextWriterEndElement( xmlWriter );
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Line2" ));
    dumpHomogenMatrixLine3(rTransformation.Line2, xmlWriter);
    (void)xmlTextWriterEndElement( xmlWriter );
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Line3" ));
    dumpHomogenMatrixLine3(rTransformation.Line3, xmlWriter);
    (void)xmlTextWriterEndElement( xmlWriter );
    }
    (void)xmlTextWriterEndElement( xmlWriter );
}

void dumpNavigationOrderAsAttribute(sal_Int32 aNavigationOrder, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("navigationOrder"), "%" SAL_PRIdINT32, aNavigationOrder);
}

void dumpHyperlinkAsAttribute(std::u16string_view sHyperlink, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("hyperlink"), "%s",
        OUStringToOString(sHyperlink, RTL_TEXTENCODING_UTF8).getStr());
}

void dumpInteropGrabBagAsElement(const uno::Sequence< beans::PropertyValue>& aInteropGrabBag, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "InteropGrabBag" ));

    sal_Int32 nLength = aInteropGrabBag.getLength();
    for (sal_Int32 i = 0; i < nLength; ++i)
        dumpPropertyValueAsElement(aInteropGrabBag[i], xmlWriter);

    (void)xmlTextWriterEndElement( xmlWriter );
}


// ---------- XShape.idl ----------


void dumpPositionAsAttribute(const awt::Point& rPoint, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("positionX"), "%" SAL_PRIdINT32, rPoint.X);
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("positionY"), "%" SAL_PRIdINT32, rPoint.Y);
}

void dumpSizeAsAttribute(const awt::Size& rSize, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("sizeX"), "%" SAL_PRIdINT32, rSize.Width);
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("sizeY"), "%" SAL_PRIdINT32, rSize.Height);
}

void dumpShapeDescriptorAsAttribute( const uno::Reference< drawing::XShapeDescriptor >& xDescr, xmlTextWriterPtr xmlWriter )
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("type"), "%s",
        OUStringToOString(xDescr->getShapeType(), RTL_TEXTENCODING_UTF8).getStr());
}


// ---------- CustomShape.idl ----------


void dumpCustomShapeEngineAsAttribute(std::u16string_view sCustomShapeEngine, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("customShapeEngine"), "%s",
        OUStringToOString(sCustomShapeEngine, RTL_TEXTENCODING_UTF8).getStr());
}

void dumpCustomShapeDataAsAttribute(
    std::u16string_view sCustomShapeData, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("customShapeData"), "%s",
        OUStringToOString(sCustomShapeData, RTL_TEXTENCODING_UTF8).getStr());
}

void dumpPropertyValueAsElement(const beans::PropertyValue& rPropertyValue, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "PropertyValue" ));

    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("name"), "%s",
            OUStringToOString(rPropertyValue.Name, RTL_TEXTENCODING_UTF8).getStr());

    uno::Any aAny = rPropertyValue.Value;
    OUString sValue;
    float fValue;
    sal_Int32 nValue;
    bool bValue;
    awt::Rectangle aRectangleValue;
    uno::Sequence< drawing::EnhancedCustomShapeAdjustmentValue> aAdjustmentValues;
    uno::Sequence< drawing::EnhancedCustomShapeParameterPair > aCoordinates;
    uno::Sequence< drawing::EnhancedCustomShapeSegment > aSegments;
    uno::Sequence< beans::PropertyValue > aPropSeq;
    if(aAny >>= sValue)
    {
        (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("value"), "%s",
                OUStringToOString(sValue, RTL_TEXTENCODING_UTF8).getStr());
    }
    else if(aAny >>= nValue)
    {
        (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("value"), "%" SAL_PRIdINT32, nValue);
    }
    else if(aAny >>= fValue)
    {
        (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("value"), "%f", fValue);
    }
    else if(aAny >>= bValue)
    {
        (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("value"), "%s", (bValue? "true": "false"));
    }
    else if(rPropertyValue.Name == "ViewBox" && (aAny >>= aRectangleValue))
    {
        EnhancedShapeDumper enhancedDumper(xmlWriter);
        enhancedDumper.dumpViewBoxAsElement(aRectangleValue);
    }
    else if(rPropertyValue.Name == "AdjustmentValues" && (aAny >>= aAdjustmentValues))
    {
        EnhancedShapeDumper enhancedDumper(xmlWriter);
        enhancedDumper.dumpAdjustmentValuesAsElement(aAdjustmentValues);
    }
    else if(rPropertyValue.Name == "Coordinates" && (aAny >>= aCoordinates))
    {
        EnhancedShapeDumper enhancedDumper(xmlWriter);
        enhancedDumper.dumpCoordinatesAsElement(aCoordinates);
    }
    else if(rPropertyValue.Name == "Segments" && (aAny >>= aSegments))
    {
        EnhancedShapeDumper enhancedDumper(xmlWriter);
        enhancedDumper.dumpSegmentsAsElement(aSegments);
    }
    else if(aAny >>= aPropSeq)
    {
        (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( OUStringToOString(rPropertyValue.Name, RTL_TEXTENCODING_UTF8).getStr() ));

        sal_Int32 i = 0, nCount = aPropSeq.getLength();
        for ( ; i < nCount; i++ )
            dumpPropertyValueAsElement(aPropSeq[ i ], xmlWriter);

        (void)xmlTextWriterEndElement(xmlWriter);
    }

    // TODO: Add here dumping of XDocument for future OOX Smart-Art
    // properties.

    // TODO more, if necessary

    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("handle"), "%" SAL_PRIdINT32, rPropertyValue.Handle);

    switch(rPropertyValue.State)
    {
        case beans::PropertyState_DIRECT_VALUE:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("propertyState"), "%s", "DIRECT_VALUE");
            break;
        case beans::PropertyState_DEFAULT_VALUE:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("propertyState"), "%s", "DEFAULT_VALUE");
            break;
        case beans::PropertyState_AMBIGUOUS_VALUE:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("propertyState"), "%s", "AMBIGUOUS_VALUE");
            break;
        default:
            break;
    }
    (void)xmlTextWriterEndElement( xmlWriter );
}

void dumpCustomShapeGeometryAsElement(const uno::Sequence< beans::PropertyValue>& aCustomShapeGeometry, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "CustomShapeGeometry" ));

    sal_Int32 nLength = aCustomShapeGeometry.getLength();
    for (sal_Int32 i = 0; i < nLength; ++i)
        dumpPropertyValueAsElement(aCustomShapeGeometry[i], xmlWriter);

    (void)xmlTextWriterEndElement( xmlWriter );
}

void dumpCustomShapeReplacementURLAsAttribute(std::u16string_view sCustomShapeReplacementURL, xmlTextWriterPtr xmlWriter)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("customShapeReplacementURL"), "%s",
        OUStringToOString(sCustomShapeReplacementURL, RTL_TEXTENCODING_UTF8).getStr());
}

// methods dumping whole services

void dumpTextPropertiesService(const uno::Reference< beans::XPropertySet >& xPropSet, xmlTextWriterPtr xmlWriter)
{
    uno::Reference< beans::XPropertySetInfo> xInfo = xPropSet->getPropertySetInfo();
    if(xInfo->hasPropertyByName("CharHeight"))
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("CharHeight");
        float fHeight;
        if(anotherAny >>= fHeight)
            dumpCharHeightAsAttribute(fHeight, xmlWriter);
    }
    if(xInfo->hasPropertyByName("CharColor"))
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("CharColor");
        sal_Int32 aColor = sal_Int32();
        if(anotherAny >>= aColor)
            dumpCharColorAsAttribute(aColor, xmlWriter);
    }
    // TODO - more properties from CharacterProperties.idl (similar to above)

    if(xInfo->hasPropertyByName("IsNumbering"))
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("IsNumbering");
        bool bIsNumbering;
        if(anotherAny >>= bIsNumbering)
            dumpIsNumberingAsAttribute(bIsNumbering, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextAutoGrowHeight");
        bool bTextAutoGrowHeight;
        if(anotherAny >>= bTextAutoGrowHeight)
            dumpTextAutoGrowHeightAsAttribute(bTextAutoGrowHeight, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextAutoGrowWidth");
        bool bTextAutoGrowWidth;
        if(anotherAny >>= bTextAutoGrowWidth)
            dumpTextAutoGrowWidthAsAttribute(bTextAutoGrowWidth, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextContourFrame");
        bool bTextContourFrame;
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
        bool bTextAnimationStartInside;
        if(anotherAny >>= bTextAnimationStartInside)
            dumpTextAnimationStartInsideAsAttribute(bTextAnimationStartInside, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextAnimationStopInside");
        bool bTextAnimationStopInside;
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

void dumpFillPropertiesService(const uno::Reference< beans::XPropertySet >& xPropSet, xmlTextWriterPtr xmlWriter)
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
        OUString sTranspGradName;
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
        OUString sGradName;
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
        OUString sHatchName;
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
        bool bFillBackground;
        if(anotherAny >>= bFillBackground)
            dumpFillBackgroundAsAttribute(bFillBackground, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("FillBitmapName");
        OUString sBitmapName;
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
        bool bBitmapLogicalSize;
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
        bool bBitmapStretch;
        if(anotherAny >>= bBitmapStretch)
            dumpFillBitmapStretchAsAttribute(bBitmapStretch, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("FillBitmapTile");
        bool bBitmapTile;
        if(anotherAny >>= bBitmapTile)
            dumpFillBitmapTileAsAttribute(bBitmapTile, xmlWriter);
    }
}

void dumpLinePropertiesService(const uno::Reference< beans::XPropertySet >& xPropSet, xmlTextWriterPtr xmlWriter)
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
        OUString sLineDashName;
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
        OUString sLineStartName;
        if(anotherAny >>= sLineStartName)
            dumpLineStartNameAsAttribute(sLineStartName, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("LineEndName");
        OUString sLineEndName;
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
        bool bLineStartCenter;
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
        bool bLineEndCenter;
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

void dumpShadowPropertiesService(const uno::Reference< beans::XPropertySet >& xPropSet, xmlTextWriterPtr xmlWriter)
{
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Shadow");
        bool bShadow;
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

void dumpPolyPolygonDescriptorService(const uno::Reference< beans::XPropertySet >& xPropSet, xmlTextWriterPtr xmlWriter)
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

void dumpShapeService(const uno::Reference< beans::XPropertySet >& xPropSet, xmlTextWriterPtr xmlWriter, bool bDumpInteropProperties)
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
        OUString sLayerName;
        if(anotherAny >>= sLayerName)
            dumpLayerNameAsAttribute(sLayerName, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Visible");
        bool bVisible;
        if(anotherAny >>= bVisible)
            dumpVisibleAsAttribute(bVisible, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Printable");
        bool bPrintable;
        if(anotherAny >>= bPrintable)
            dumpPrintableAsAttribute(bPrintable, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("MoveProtect");
        bool bMoveProtect;
        if(anotherAny >>= bMoveProtect)
            dumpMoveProtectAsAttribute(bMoveProtect, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Name");
        OUString sName;
        if(anotherAny >>= sName)
            dumpNameAsAttribute(sName, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("SizeProtect");
        bool bSizeProtect;
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
        OUString sHyperlink;
        if(anotherAny >>= sHyperlink)
            dumpHyperlinkAsAttribute(sHyperlink, xmlWriter);
    }
    if(xInfo->hasPropertyByName("InteropGrabBag") && bDumpInteropProperties)
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("InteropGrabBag");
        uno::Sequence< beans::PropertyValue> aInteropGrabBag;
        if(anotherAny >>= aInteropGrabBag)
            dumpInteropGrabBagAsElement(aInteropGrabBag, xmlWriter);
    }
}

void dumpPolyPolygonBezierDescriptorService(const uno::Reference< beans::XPropertySet >& xPropSet, xmlTextWriterPtr xmlWriter)
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

void dumpCustomShapeService(const uno::Reference< beans::XPropertySet >& xPropSet, xmlTextWriterPtr xmlWriter)
{
    uno::Reference< beans::XPropertySetInfo> xInfo = xPropSet->getPropertySetInfo();
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("CustomShapeEngine");
        OUString sCustomShapeEngine;
        if(anotherAny >>= sCustomShapeEngine)
            dumpCustomShapeEngineAsAttribute(sCustomShapeEngine, xmlWriter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("CustomShapeData");
        OUString sCustomShapeData;
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
        OUString sCustomShapeReplacementURL;
        if(anotherAny >>= sCustomShapeReplacementURL)
            dumpCustomShapeReplacementURLAsAttribute(sCustomShapeReplacementURL, xmlWriter);
    }
}

void dumpXShape(const uno::Reference< drawing::XShape >& xShape, xmlTextWriterPtr xmlWriter, bool bDumpInteropProperties)
{
    (void)xmlTextWriterStartElement( xmlWriter, BAD_CAST( "XShape" ) );
    uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY_THROW);
    OUString aName;
    m_bNameDumped = false;

    dumpPositionAsAttribute(xShape->getPosition(), xmlWriter);
    dumpSizeAsAttribute(xShape->getSize(), xmlWriter);
    uno::Reference< drawing::XShapeDescriptor > xDescr(xShape, uno::UNO_QUERY_THROW);
    dumpShapeDescriptorAsAttribute(xDescr, xmlWriter);

    // uno::Sequence<beans::Property> aProperties = xPropSetInfo->getProperties();

    uno::Reference< lang::XServiceInfo > xServiceInfo( xShape, uno::UNO_QUERY_THROW );

    uno::Reference< beans::XPropertySetInfo> xInfo = xPropSet->getPropertySetInfo();
    if(xInfo->hasPropertyByName("Name"))
    {
        uno::Any aAny = xPropSet->getPropertyValue("Name");
        if ((aAny >>= aName) && !aName.isEmpty())
        {
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("name"), "%s", OUStringToOString(aName, RTL_TEXTENCODING_UTF8).getStr());
            m_bNameDumped = true;
        }
    }

    try
    {
    if (xServiceInfo->supportsService("com.sun.star.drawing.Text"))
    {
        uno::Reference< text::XText > xText(xShape, uno::UNO_QUERY_THROW);
        OUString aText = xText->getString();
        if(!aText.isEmpty())
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("text"), "%s", OUStringToOString(aText, RTL_TEXTENCODING_UTF8).getStr());
    }
    if(xServiceInfo->supportsService("com.sun.star.drawing.TextProperties"))
        dumpTextPropertiesService(xPropSet, xmlWriter);

    if(xServiceInfo->supportsService("com.sun.star.drawing.GroupShape"))
    {
        uno::Reference< drawing::XShapes > xShapes(xShape, uno::UNO_QUERY_THROW);
        dumpXShapes(xShapes, xmlWriter, bDumpInteropProperties);
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
        dumpShapeService(xPropSet, xmlWriter, bDumpInteropProperties);

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
        uno::Sequence< OUString > aServiceNames = xServiceInfo->getSupportedServiceNames();
        sal_Int32 nServices = aServiceNames.getLength();
        for (sal_Int32 i = 0; i < nServices; ++i)
        {
            (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "ServiceName" ));
            (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST( "name" ), "%s", OUStringToOString(aServiceNames[i], RTL_TEXTENCODING_UTF8).getStr());
            (void)xmlTextWriterEndElement( xmlWriter );
        }
    #endif

    (void)xmlTextWriterEndElement( xmlWriter );
}

void dumpXShapes( const uno::Reference< drawing::XShapes >& xShapes, xmlTextWriterPtr xmlWriter, bool bDumpInteropProperties )
{
    (void)xmlTextWriterStartElement( xmlWriter, BAD_CAST( "XShapes" ) );
    uno::Reference< container::XIndexAccess > xIA( xShapes, uno::UNO_QUERY_THROW);
    sal_Int32 nLength = xIA->getCount();
    for (sal_Int32 i = 0; i < nLength; ++i)
    {
        uno::Reference< drawing::XShape > xShape( xIA->getByIndex( i ), uno::UNO_QUERY_THROW );
        dumpXShape( xShape, xmlWriter, bDumpInteropProperties );
    }

    (void)xmlTextWriterEndElement( xmlWriter );
}
} //end of namespace

OUString XShapeDumper::dump(const uno::Reference<drawing::XShapes>& xPageShapes, bool bDumpInteropProperties)
{
    OStringBuffer aString;
    xmlOutputBufferPtr xmlOutBuffer = xmlOutputBufferCreateIO( writeCallback, closeCallback, &aString, nullptr );
    xmlTextWriterPtr xmlWriter = xmlNewTextWriter( xmlOutBuffer );
    xmlTextWriterSetIndent( xmlWriter, 1 );

    (void)xmlTextWriterStartDocument( xmlWriter, nullptr, nullptr, nullptr );

    try
    {
        dumpXShapes( xPageShapes, xmlWriter, bDumpInteropProperties );
    }
    catch (const beans::UnknownPropertyException& e)
    {
        std::cout << "Exception caught in XShapeDumper: " << e.Message << std::endl;
    }

    (void)xmlTextWriterEndDocument( xmlWriter );
    xmlFreeTextWriter( xmlWriter );

    return OUString::fromUtf8(aString.makeStringAndClear());
}

OUString XShapeDumper::dump(const uno::Reference<drawing::XShape>& xPageShapes, bool bDumpInteropProperties)
{
    OStringBuffer aString;
    xmlOutputBufferPtr xmlOutBuffer = xmlOutputBufferCreateIO( writeCallback, closeCallback, &aString, nullptr );
    xmlTextWriterPtr xmlWriter = xmlNewTextWriter( xmlOutBuffer );
    xmlTextWriterSetIndent( xmlWriter, 1 );

    (void)xmlTextWriterStartDocument( xmlWriter, nullptr, nullptr, nullptr );

    try
    {
        dumpXShape( xPageShapes, xmlWriter, bDumpInteropProperties );
    }
    catch (const beans::UnknownPropertyException& e)
    {
        std::cout << "Exception caught in XShapeDumper: " << e.Message << std::endl;
    }

    (void)xmlTextWriterEndDocument( xmlWriter );
    xmlFreeTextWriter( xmlWriter );

    return OUString::fromUtf8(aString.makeStringAndClear());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
