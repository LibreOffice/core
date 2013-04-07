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
    void dumpExtrusionAsAttribute(sal_Bool bExtrusion);
    void dumpBrightnessAsAttribute(double aBrightness);
    void dumpDepthAsElement(com::sun::star::drawing::EnhancedCustomShapeParameterPair aDepth);
    void dumpDiffusionAsAttribute(double aDiffusion);
    void dumpNumberOfLineSegmentsAsAttribute(sal_Int32 aNumberOfLineSegments);
    void dumpLightFaceAsAttribute(sal_Bool bLightFace);
    void dumpFirstLightHarshAsAttribute(sal_Bool bFirstLightHarsh);
    void dumpSecondLightHarshAsAttribute(sal_Bool bSecondLightHarsh);
    void dumpFirstLightLevelAsAttribute(double aFirstLightLevel);
    void dumpSecondLightLevelAsAttribute(double aSecondLightLevel);
    void dumpFirstLightDirectionAsElement(com::sun::star::drawing::Direction3D aFirstLightDirection);
    void dumpSecondLightDirectionAsElement(com::sun::star::drawing::Direction3D aSecondLightDirection);
    void dumpMetalAsAttribute(sal_Bool bMetal);
    void dumpShadeModeAsAttribute(com::sun::star::drawing::ShadeMode eShadeMode);
    void dumpRotateAngleAsElement(com::sun::star::drawing::EnhancedCustomShapeParameterPair aRotateAngle);
    void dumpRotationCenterAsElement(com::sun::star::drawing::Direction3D aRotationCenter);
    void dumpShininessAsAttribute(double aShininess);
    void dumpSkewAsElement(com::sun::star::drawing::EnhancedCustomShapeParameterPair aSkew);
    void dumpSpecularityAsAttribute(double aSpecularity);
    void dumpProjectionModeAsAttribute(com::sun::star::drawing::ProjectionMode eProjectionMode);
    void dumpViewPointAsElement(com::sun::star::drawing::Position3D aViewPoint);
    void dumpOriginAsElement(com::sun::star::drawing::EnhancedCustomShapeParameterPair aOrigin);
    void dumpExtrusionColorAsAttribute(sal_Bool bExtrusionColor);

    // EnhancedCustomShapeGeometry.idl
    void dumpEnhancedCustomShapeGeometryService(com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xPropSet);
    void dumpTypeAsAttribute(OUString sType);
    void dumpViewBoxAsElement(com::sun::star::awt::Rectangle aViewBox);
    void dumpMirroredXAsAttribute(sal_Bool bMirroredX); // also used in EnhancedCustomShapeHandle
    void dumpMirroredYAsAttribute(sal_Bool bMirroredY); // also used in EnhancedCustomShapeHandle
    void dumpTextRotateAngleAsAttribute(double aTextRotateAngle);
    void dumpAdjustmentValuesAsElement(com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeAdjustmentValue> aAdjustmentValues);
    void dumpExtrusionAsElement(com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > aExtrusion);
    void dumpPathAsElement(com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > aPath);
    void dumpTextPathAsElement(com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > aTextPath);
    void dumpEquationsAsElement(com::sun::star::uno::Sequence< OUString > aEquations);
    void dumpHandlesAsElement(com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValues > aHandles);

    // EnhancedCustomShapeHandle.idl
    void dumpEnhancedCustomShapeHandleService(com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xPropSet);
    void dumpSwitchedAsAttribute(sal_Bool bSwitched);
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
    void dumpExtrusionAllowedAsAttribute(sal_Bool bExtrusionAllowed);
    void dumpConcentricGradientFillAllowedAsAttribute(sal_Bool bConcentricGradientFillAllowed);
    void dumpTextPathAllowedAsAttribute(sal_Bool bTextPathAllowed);
    void dumpSubViewSizeAsElement(com::sun::star::uno::Sequence< com::sun::star::awt::Size > aSubViewSize);

    // EnhancedCustomShapePath.idl
    void dumpEnhancedCustomShapeTextPathService(com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xPropSet);
    void dumpTextPathAsAttribute(sal_Bool bTextPath);
    void dumpTextPathModeAsAttribute(com::sun::star::drawing::EnhancedCustomShapeTextPathMode eTextPathMode);
    void dumpScaleXAsAttribute(sal_Bool bScaleX);

private:
    xmlTextWriterPtr xmlWriter;
};
#endif
