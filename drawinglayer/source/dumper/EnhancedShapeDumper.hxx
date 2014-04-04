/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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

#ifndef EnhancedShapeDumper_hxx
#define EnhancedShapeDumper_hxx

class EnhancedShapeDumper
{
public:
    EnhancedShapeDumper(xmlTextWriterPtr writer)
    :
    xmlWriter(writer)
    {

    }

    // auxiliary functions
    void dumpEnhancedCustomShapeParameterPair(com::sun::star::drawing::EnhancedCustomShapeParameterPair aParameterPair);
    void dumpDirection3D(com::sun::star::drawing::Direction3D aDirection3D);
    void dumpPropertyValueAsElement(com::sun::star::beans::PropertyValue aPropertyValue);
    void dumpEnhancedCustomShapeParameter(com::sun::star::drawing::EnhancedCustomShapeParameter aParameter);

    // EnhancedCustomShapeExtrusion.idl
    void dumpEnhancedCustomShapeExtrusionService(com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xPropSet);
    void dumpExtrusionAsAttribute(bool bExtrusion);
    void dumpBrightnessAsAttribute(double aBrightness);
    void dumpDepthAsElement(com::sun::star::drawing::EnhancedCustomShapeParameterPair aDepth);
    void dumpDiffusionAsAttribute(double aDiffusion);
    void dumpNumberOfLineSegmentsAsAttribute(sal_Int32 aNumberOfLineSegments);
    void dumpLightFaceAsAttribute(bool bLightFace);
    void dumpFirstLightHarshAsAttribute(bool bFirstLightHarsh);
    void dumpSecondLightHarshAsAttribute(bool bSecondLightHarsh);
    void dumpFirstLightLevelAsAttribute(double aFirstLightLevel);
    void dumpSecondLightLevelAsAttribute(double aSecondLightLevel);
    void dumpFirstLightDirectionAsElement(com::sun::star::drawing::Direction3D aFirstLightDirection);
    void dumpSecondLightDirectionAsElement(com::sun::star::drawing::Direction3D aSecondLightDirection);
    void dumpMetalAsAttribute(bool bMetal);
    void dumpShadeModeAsAttribute(com::sun::star::drawing::ShadeMode eShadeMode);
    void dumpRotateAngleAsElement(com::sun::star::drawing::EnhancedCustomShapeParameterPair aRotateAngle);
    void dumpRotationCenterAsElement(com::sun::star::drawing::Direction3D aRotationCenter);
    void dumpShininessAsAttribute(double aShininess);
    void dumpSkewAsElement(com::sun::star::drawing::EnhancedCustomShapeParameterPair aSkew);
    void dumpSpecularityAsAttribute(double aSpecularity);
    void dumpProjectionModeAsAttribute(com::sun::star::drawing::ProjectionMode eProjectionMode);
    void dumpViewPointAsElement(com::sun::star::drawing::Position3D aViewPoint);
    void dumpOriginAsElement(com::sun::star::drawing::EnhancedCustomShapeParameterPair aOrigin);
    void dumpExtrusionColorAsAttribute(bool bExtrusionColor);

    // EnhancedCustomShapeGeometry.idl
    void dumpEnhancedCustomShapeGeometryService(com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xPropSet);
    void dumpTypeAsAttribute(const OUString& sType);
    void dumpViewBoxAsElement(com::sun::star::awt::Rectangle aViewBox);
    void dumpMirroredXAsAttribute(bool bMirroredX); // also used in EnhancedCustomShapeHandle
    void dumpMirroredYAsAttribute(bool bMirroredY); // also used in EnhancedCustomShapeHandle
    void dumpTextRotateAngleAsAttribute(double aTextRotateAngle);
    void dumpAdjustmentValuesAsElement(com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeAdjustmentValue> aAdjustmentValues);
    void dumpExtrusionAsElement(com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > aExtrusion);
    void dumpPathAsElement(com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > aPath);
    void dumpTextPathAsElement(com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > aTextPath);
    void dumpEquationsAsElement(com::sun::star::uno::Sequence< OUString > aEquations);
    void dumpHandlesAsElement(com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValues > aHandles);

    // EnhancedCustomShapeHandle.idl
    void dumpEnhancedCustomShapeHandleService(com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xPropSet);
    void dumpSwitchedAsAttribute(bool bSwitched);
    void dumpPositionAsElement(com::sun::star::drawing::EnhancedCustomShapeParameterPair aPosition);
    void dumpPolarAsElement(com::sun::star::drawing::EnhancedCustomShapeParameterPair aPolar);
    void dumpRefXAsAttribute(sal_Int32 aRefX);
    void dumpRefYAsAttribute(sal_Int32 aRefY);
    void dumpRefAngleAsAttribute(sal_Int32 aRefAngle);
    void dumpRefRAsAttribute(sal_Int32 aRefR);
    void dumpRangeXMinimumAsElement(com::sun::star::drawing::EnhancedCustomShapeParameter aRangeXMinimum);
    void dumpRangeXMaximumAsElement(com::sun::star::drawing::EnhancedCustomShapeParameter aRangeXMaximum);
    void dumpRangeYMinimumAsElement(com::sun::star::drawing::EnhancedCustomShapeParameter aRangeYMinimum);
    void dumpRangeYMaximumAsElement(com::sun::star::drawing::EnhancedCustomShapeParameter aRangeXMaximum);
    void dumpRadiusRangeMinimumAsElement(com::sun::star::drawing::EnhancedCustomShapeParameter aRadiusRangeMinimum);
    void dumpRadiusRangeMaximumAsElement(com::sun::star::drawing::EnhancedCustomShapeParameter aRadiusRangeMaximum);

    // EnhancedCustomShapePath.idl
    void dumpEnhancedCustomShapePathService(com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xPropSet);
    void dumpCoordinatesAsElement(com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeParameterPair > aCoordinates);
    void dumpSegmentsAsElement(com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeSegment > aSegments);
    void dumpStretchXAsAttribute(sal_Int32 aStretchX);
    void dumpStretchYAsAttribute(sal_Int32 aStretchY);
    void dumpTextFramesAsElement(com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeTextFrame > aTextFrames);
    void dumpGluePointsAsElement(com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeParameterPair > aGluePoints);
    void dumpGluePointLeavingDirectionsAsElement(com::sun::star::uno::Sequence< double > aGluePointLeavingDirections);
    void dumpGluePointTypeAsAttribute(sal_Int32 aGluePointType);
    void dumpExtrusionAllowedAsAttribute(bool bExtrusionAllowed);
    void dumpConcentricGradientFillAllowedAsAttribute(bool bConcentricGradientFillAllowed);
    void dumpTextPathAllowedAsAttribute(bool bTextPathAllowed);
    void dumpSubViewSizeAsElement(com::sun::star::uno::Sequence< com::sun::star::awt::Size > aSubViewSize);

    // EnhancedCustomShapePath.idl
    void dumpEnhancedCustomShapeTextPathService(com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xPropSet);
    void dumpTextPathAsAttribute(bool bTextPath);
    void dumpTextPathModeAsAttribute(com::sun::star::drawing::EnhancedCustomShapeTextPathMode eTextPathMode);
    void dumpScaleXAsAttribute(bool bScaleX);

private:
    xmlTextWriterPtr xmlWriter;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
