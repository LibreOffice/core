/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_DRAWINGLAYER_SOURCE_DUMPER_ENHANCEDSHAPEDUMPER_HXX
#define INCLUDED_DRAWINGLAYER_SOURCE_DUMPER_ENHANCEDSHAPEDUMPER_HXX

#include <libxml/xmlwriter.h>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/drawing/Position3D.hpp>

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameter.hpp>

#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeTextFrame.hpp>
#include <com/sun/star/awt/Size.hpp>

#include <com/sun/star/drawing/EnhancedCustomShapeTextPathMode.hpp>

class EnhancedShapeDumper
{
public:
    explicit EnhancedShapeDumper(xmlTextWriterPtr writer)
        : xmlWriter(writer)
    {
    }

    // auxiliary functions
    void dumpEnhancedCustomShapeParameterPair(css::drawing::EnhancedCustomShapeParameterPair aParameterPair);
    void dumpDirection3D(css::drawing::Direction3D aDirection3D);
    void dumpPropertyValueAsElement(css::beans::PropertyValue aPropertyValue);
    void dumpEnhancedCustomShapeParameter(css::drawing::EnhancedCustomShapeParameter aParameter);

    // EnhancedCustomShapeExtrusion.idl
    void dumpEnhancedCustomShapeExtrusionService(css::uno::Reference< css::beans::XPropertySet > xPropSet);
    void dumpExtrusionAsAttribute(bool bExtrusion);
    void dumpBrightnessAsAttribute(double aBrightness);
    void dumpDepthAsElement(css::drawing::EnhancedCustomShapeParameterPair aDepth);
    void dumpDiffusionAsAttribute(double aDiffusion);
    void dumpNumberOfLineSegmentsAsAttribute(sal_Int32 aNumberOfLineSegments);
    void dumpLightFaceAsAttribute(bool bLightFace);
    void dumpFirstLightHarshAsAttribute(bool bFirstLightHarsh);
    void dumpSecondLightHarshAsAttribute(bool bSecondLightHarsh);
    void dumpFirstLightLevelAsAttribute(double aFirstLightLevel);
    void dumpSecondLightLevelAsAttribute(double aSecondLightLevel);
    void dumpFirstLightDirectionAsElement(css::drawing::Direction3D aFirstLightDirection);
    void dumpSecondLightDirectionAsElement(css::drawing::Direction3D aSecondLightDirection);
    void dumpMetalAsAttribute(bool bMetal);
    void dumpShadeModeAsAttribute(css::drawing::ShadeMode eShadeMode);
    void dumpRotateAngleAsElement(css::drawing::EnhancedCustomShapeParameterPair aRotateAngle);
    void dumpRotationCenterAsElement(css::drawing::Direction3D aRotationCenter);
    void dumpShininessAsAttribute(double aShininess);
    void dumpSkewAsElement(css::drawing::EnhancedCustomShapeParameterPair aSkew);
    void dumpSpecularityAsAttribute(double aSpecularity);
    void dumpProjectionModeAsAttribute(css::drawing::ProjectionMode eProjectionMode);
    void dumpViewPointAsElement(css::drawing::Position3D aViewPoint);
    void dumpOriginAsElement(css::drawing::EnhancedCustomShapeParameterPair aOrigin);
    void dumpExtrusionColorAsAttribute(bool bExtrusionColor);

    // EnhancedCustomShapeGeometry.idl
    void dumpEnhancedCustomShapeGeometryService(css::uno::Reference< css::beans::XPropertySet > xPropSet);
    void dumpTypeAsAttribute(const OUString& sType);
    void dumpViewBoxAsElement(css::awt::Rectangle aViewBox);
    void dumpMirroredXAsAttribute(bool bMirroredX); // also used in EnhancedCustomShapeHandle
    void dumpMirroredYAsAttribute(bool bMirroredY); // also used in EnhancedCustomShapeHandle
    void dumpTextRotateAngleAsAttribute(double aTextRotateAngle);
    void dumpAdjustmentValuesAsElement(const css::uno::Sequence< css::drawing::EnhancedCustomShapeAdjustmentValue>& aAdjustmentValues);
    void dumpExtrusionAsElement(const css::uno::Sequence< css::beans::PropertyValue >& aExtrusion);
    void dumpPathAsElement(const css::uno::Sequence< css::beans::PropertyValue >& aPath);
    void dumpTextPathAsElement(const css::uno::Sequence< css::beans::PropertyValue >& aTextPath);
    void dumpEquationsAsElement(const css::uno::Sequence< OUString >& aEquations);
    void dumpHandlesAsElement(const css::uno::Sequence< css::beans::PropertyValues >& aHandles);

    // EnhancedCustomShapeHandle.idl
    void dumpEnhancedCustomShapeHandleService(css::uno::Reference< css::beans::XPropertySet > xPropSet);
    void dumpSwitchedAsAttribute(bool bSwitched);
    void dumpPositionAsElement(css::drawing::EnhancedCustomShapeParameterPair aPosition);
    void dumpPolarAsElement(css::drawing::EnhancedCustomShapeParameterPair aPolar);
    void dumpRefXAsAttribute(sal_Int32 aRefX);
    void dumpRefYAsAttribute(sal_Int32 aRefY);
    void dumpRefAngleAsAttribute(sal_Int32 aRefAngle);
    void dumpRefRAsAttribute(sal_Int32 aRefR);
    void dumpRangeXMinimumAsElement(css::drawing::EnhancedCustomShapeParameter aRangeXMinimum);
    void dumpRangeXMaximumAsElement(css::drawing::EnhancedCustomShapeParameter aRangeXMaximum);
    void dumpRangeYMinimumAsElement(css::drawing::EnhancedCustomShapeParameter aRangeYMinimum);
    void dumpRangeYMaximumAsElement(css::drawing::EnhancedCustomShapeParameter aRangeXMaximum);
    void dumpRadiusRangeMinimumAsElement(css::drawing::EnhancedCustomShapeParameter aRadiusRangeMinimum);
    void dumpRadiusRangeMaximumAsElement(css::drawing::EnhancedCustomShapeParameter aRadiusRangeMaximum);

    // EnhancedCustomShapePath.idl
    void dumpEnhancedCustomShapePathService(css::uno::Reference< css::beans::XPropertySet > xPropSet);
    void dumpCoordinatesAsElement(const css::uno::Sequence< css::drawing::EnhancedCustomShapeParameterPair >& aCoordinates);
    void dumpSegmentsAsElement(const css::uno::Sequence< css::drawing::EnhancedCustomShapeSegment >& aSegments);
    void dumpStretchXAsAttribute(sal_Int32 aStretchX);
    void dumpStretchYAsAttribute(sal_Int32 aStretchY);
    void dumpTextFramesAsElement(const css::uno::Sequence< css::drawing::EnhancedCustomShapeTextFrame >& aTextFrames);
    void dumpGluePointsAsElement(const css::uno::Sequence< css::drawing::EnhancedCustomShapeParameterPair >& aGluePoints);
    void dumpGluePointLeavingDirectionsAsElement(const css::uno::Sequence< double >& aGluePointLeavingDirections);
    void dumpGluePointTypeAsAttribute(sal_Int32 aGluePointType);
    void dumpExtrusionAllowedAsAttribute(bool bExtrusionAllowed);
    void dumpConcentricGradientFillAllowedAsAttribute(bool bConcentricGradientFillAllowed);
    void dumpTextPathAllowedAsAttribute(bool bTextPathAllowed);
    void dumpSubViewSizeAsElement(const css::uno::Sequence< css::awt::Size >& aSubViewSize);

    // EnhancedCustomShapePath.idl
    void dumpEnhancedCustomShapeTextPathService(css::uno::Reference< css::beans::XPropertySet > xPropSet);
    void dumpTextPathAsAttribute(bool bTextPath);
    void dumpTextPathModeAsAttribute(css::drawing::EnhancedCustomShapeTextPathMode eTextPathMode);
    void dumpScaleXAsAttribute(bool bScaleX);

private:
    xmlTextWriterPtr xmlWriter;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
