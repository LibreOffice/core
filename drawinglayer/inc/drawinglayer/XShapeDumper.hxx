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

#include <libxml/xmlwriter.h>
#include <drawinglayer/drawinglayerdllapi.h>
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

#ifndef ChartViewDumper_hxx
#define ChartViewDumper_hxx

class DRAWINGLAYER_DLLPUBLIC XShapeDumper
{

public:
    XShapeDumper();
    rtl::OUString dump(com::sun::star::uno::Reference<com::sun::star::drawing::XShapes> xPageShapes);

private:
    // auxiliary functions
    void dumpGradientProperty(com::sun::star::awt::Gradient aGradient, xmlTextWriterPtr xmlWriter);
    void dumpPolyPolygonBezierCoords(com::sun::star::drawing::PolyPolygonBezierCoords aPolyPolygonBezierCoords, xmlTextWriterPtr xmlWriter);
    void dumpPointSequenceSequence(com::sun::star::drawing::PointSequenceSequence aPointSequenceSequence, xmlTextWriterPtr xmlWriter);

    // FillProperties.idl
    void dumpFillStyleAsAttribute(com::sun::star::drawing::FillStyle eFillStyle, xmlTextWriterPtr xmlWriter);
    void dumpFillColorAsAttribute(sal_Int32 aColor, xmlTextWriterPtr xmlWriter);
    void dumpFillTransparenceAsAttribute(sal_Int32 aTransparence, xmlTextWriterPtr xmlWriter);
    void dumpFillTransparenceGradientNameAsAttribute(rtl::OUString sTranspGradName, xmlTextWriterPtr xmlWriter);
    void dumpFillTransparenceGradientAsElement(com::sun::star::awt::Gradient aTranspGrad, xmlTextWriterPtr xmlWriter);
    void dumpFillGradientNameAsAttribute(rtl::OUString sGradName, xmlTextWriterPtr xmlWriter);
    void dumpFillGradientAsElement(com::sun::star::awt::Gradient aGradient, xmlTextWriterPtr xmlWriter);
    void dumpFillHatchNameAsAttribute(rtl::OUString sHatchName, xmlTextWriterPtr xmlWriter);
    void dumpFillHatchAsElement(com::sun::star::drawing::Hatch aHatch, xmlTextWriterPtr xmlWriter);
    void dumpFillBackgroundAsAttribute(sal_Bool bBackground, xmlTextWriterPtr xmlWriter);
    void dumpFillBitmapNameAsAttribute(rtl::OUString sBitmapName, xmlTextWriterPtr xmlWriter);
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

    // XShape.idl
    void dumpPositionAsAttribute(const com::sun::star::awt::Point& rPoint, xmlTextWriterPtr xmlWriter);
    void dumpSizeAsAttribute(const com::sun::star::awt::Size& rSize, xmlTextWriterPtr xmlWriter);
    void dumpShapeDescriptorAsAttribute( com::sun::star::uno::Reference< com::sun::star::drawing::XShapeDescriptor > xDescr, xmlTextWriterPtr xmlWriter );
    void dumpXShape(com::sun::star::uno::Reference< com::sun::star::drawing::XShape > xShape, xmlTextWriterPtr xmlWriter);
    void dumpXShapes( com::sun::star::uno::Reference< com::sun::star::drawing::XShapes > xShapes, xmlTextWriterPtr xmlWriter );

};
#endif
