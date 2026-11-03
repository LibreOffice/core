/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <oox/export/drawingml.hxx>

#include <algorithm>
#include <array>
#include <cmath>
#include <string_view>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <comphelper/sequenceashashmap.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <sax/fshelper.hxx>
#include <tools/color.hxx>

using namespace ::com::sun::star;
using namespace ::cpo::uno;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::beans::XPropertySet;

namespace oox::drawingml
{
namespace
{
// A legacy camera of OOXML, the one the extrusion of a custom shape maps to: the skew and the
// origin of a parallel projection, or the view point of a perspective one, as the engine's
// extrusion toolbar sets them for its nine directions.
struct LegacyCamera
{
    std::string_view sParallel;
    std::string_view sPerspective;
    double fOriginX;
    double fOriginY;
    double fSkewAngle;
};

constexpr std::array<LegacyCamera, 9> aLegacyCameras = { {
    { "legacyObliqueTopLeft", "legacyPerspectiveTopLeft", -0.5, -0.5, -45 },
    { "legacyObliqueTop", "legacyPerspectiveTop", 0, -0.5, -90 },
    { "legacyObliqueTopRight", "legacyPerspectiveTopRight", 0.5, -0.5, -135 },
    { "legacyObliqueLeft", "legacyPerspectiveLeft", -0.5, 0, -360 },
    { "legacyObliqueFront", "legacyPerspectiveFront", 0, 0, 0 },
    { "legacyObliqueRight", "legacyPerspectiveRight", 0.5, 0, 180 },
    { "legacyObliqueBottomLeft", "legacyPerspectiveBottomLeft", -0.5, 0.5, 45 },
    { "legacyObliqueBottom", "legacyPerspectiveBottom", 0, 0.5, 90 },
    { "legacyObliqueBottomRight", "legacyPerspectiveBottomRight", 0.5, 0.5, 135 },
} };

// The extrusion of a custom shape as the engine draws it, with the defaults of the renderer for
// what the shape does not say.
struct Extrusion
{
    bool bOn = false;
    double fDepth = 1270; // 1/100 mm
    double fDepthFraction = 0;
    double fRotateX = 0; // degrees
    double fRotateY = 0;
    double fSkewAmount = 50;
    double fSkewAngle = -135;
    double fOriginX = 0.5;
    double fOriginY = -0.5;
    bool bParallel = true;
    double fLightX = 50000;
    double fLightY = 0;
    double fSecondLightLevel = 66;
    bool bMetal = false;
    bool bMetalMSCompatible = false;
    bool bWireFrame = false;
    double fSpecularity = 0;
    double fDiffusion = 0;
    bool bColor = false;
};

bool lcl_ReadPair(const Any& rAny, double& rFirst, double& rSecond)
{
    drawing::EnhancedCustomShapeParameterPair aPair;
    return (rAny >>= aPair) && (aPair.First.Value >>= rFirst) && (aPair.Second.Value >>= rSecond);
}

// Reads the "Extrusion" sequence of the custom shape geometry. Returns false when the shape has
// no extrusion switched on.
bool lcl_ReadExtrusion(const Reference<XPropertySet>& rXPropSet, Extrusion& rOut)
{
    Reference<beans::XPropertySetInfo> xInfo = rXPropSet->getPropertySetInfo();
    if (!xInfo.is() || !xInfo->hasPropertyByName(u"CustomShapeGeometry"_ustr))
        return false;
    comphelper::SequenceAsHashMap aGeometry(rXPropSet->getPropertyValue(u"CustomShapeGeometry"_ustr));
    Sequence<PropertyValue> aExtrusionSeq;
    if (!(aGeometry.getValue(u"Extrusion"_ustr) >>= aExtrusionSeq))
        return false;
    comphelper::SequenceAsHashMap aExtrusion(aExtrusionSeq);
    if (!(aExtrusion.getValue(u"Extrusion"_ustr) >>= rOut.bOn) || !rOut.bOn)
        return false;
    double fA = 0;
    double fB = 0;
    if (lcl_ReadPair(aExtrusion.getValue(u"Depth"_ustr), fA, fB))
    {
        rOut.fDepth = fA;
        rOut.fDepthFraction = fB;
    }
    if (lcl_ReadPair(aExtrusion.getValue(u"RotateAngle"_ustr), fA, fB))
    {
        rOut.fRotateX = fA;
        rOut.fRotateY = fB;
    }
    if (lcl_ReadPair(aExtrusion.getValue(u"Skew"_ustr), fA, fB))
    {
        rOut.fSkewAmount = fA;
        rOut.fSkewAngle = fB;
    }
    if (lcl_ReadPair(aExtrusion.getValue(u"Origin"_ustr), fA, fB))
    {
        rOut.fOriginX = fA;
        rOut.fOriginY = fB;
    }
    drawing::ProjectionMode eProjection = drawing::ProjectionMode_PARALLEL;
    if (aExtrusion.getValue(u"ProjectionMode"_ustr) >>= eProjection)
        rOut.bParallel = eProjection == drawing::ProjectionMode_PARALLEL;
    drawing::Direction3D aLight;
    if (aExtrusion.getValue(u"FirstLightDirection"_ustr) >>= aLight)
    {
        rOut.fLightX = aLight.DirectionX;
        rOut.fLightY = aLight.DirectionY;
    }
    aExtrusion.getValue(u"SecondLightLevel"_ustr) >>= rOut.fSecondLightLevel;
    aExtrusion.getValue(u"Metal"_ustr) >>= rOut.bMetal;
    sal_Int16 nMetalType = 0;
    if ((aExtrusion.getValue(u"MetalType"_ustr) >>= nMetalType) && nMetalType == 1)
        rOut.bMetalMSCompatible = true;
    drawing::ShadeMode eShade = drawing::ShadeMode_FLAT;
    if (aExtrusion.getValue(u"ShadeMode"_ustr) >>= eShade)
        rOut.bWireFrame = eShade == drawing::ShadeMode_DRAFT;
    aExtrusion.getValue(u"Specularity"_ustr) >>= rOut.fSpecularity;
    aExtrusion.getValue(u"Diffusion"_ustr) >>= rOut.fDiffusion;
    aExtrusion.getValue(u"Color"_ustr) >>= rOut.bColor;
    return true;
}

// Whether the 3D grab bag of the shape (or of its text) holds the 3D a file brought along.
bool lcl_GrabBagHas3D(const Reference<XPropertySet>& rXPropSet, bool bIsText)
{
    Reference<beans::XPropertySetInfo> xInfo = rXPropSet->getPropertySetInfo();
    if (!xInfo.is() || !xInfo->hasPropertyByName(u"InteropGrabBag"_ustr))
        return false;
    comphelper::SequenceAsHashMap aGrabBag(rXPropSet->getPropertyValue(u"InteropGrabBag"_ustr));
    Sequence<PropertyValue> a3DEffects;
    return (aGrabBag.getValue(bIsText ? u"Text3DEffectProperties"_ustr : u"3DEffectProperties"_ustr)
            >>= a3DEffects)
           && a3DEffects.hasElements();
}

// The two angles normalised to the same turn for a comparison.
double lcl_NormalisedAngle(double fDegrees)
{
    double fResult = std::fmod(fDegrees, 360.0);
    if (fResult < 0)
        fResult += 360.0;
    return fResult;
}

// The legacy camera preset of the extrusion: the direction whose skew angle and origin match, or
// the front when the skew has no amount; the nearest direction by its origin otherwise.
std::string_view lcl_CameraPreset(const Extrusion& rExtrusion)
{
    const bool bFront = rExtrusion.bParallel ? rExtrusion.fSkewAmount == 0
                                             : (rExtrusion.fOriginX == 0 && rExtrusion.fOriginY == 0);
    if (bFront)
        return rExtrusion.bParallel ? aLegacyCameras[4].sParallel : aLegacyCameras[4].sPerspective;
    const LegacyCamera* pBest = nullptr;
    double fBestDistance = 0;
    for (const LegacyCamera& rCamera : aLegacyCameras)
    {
        const double fDistance = std::abs(rCamera.fOriginX - rExtrusion.fOriginX)
                                 + std::abs(rCamera.fOriginY - rExtrusion.fOriginY);
        const bool bSameSkew = lcl_NormalisedAngle(rCamera.fSkewAngle)
                               == lcl_NormalisedAngle(rExtrusion.fSkewAngle);
        if (rExtrusion.bParallel && bSameSkew && fDistance < 0.01)
            return rCamera.sParallel;
        if (!pBest || fDistance < fBestDistance)
        {
            pBest = &rCamera;
            fBestDistance = fDistance;
        }
    }
    return rExtrusion.bParallel ? pBest->sParallel : pBest->sPerspective;
}

// The direction of the light rig for the quadrant of the first light, as PowerPoint writes its
// own preset lighting directions: the corners by their names, the sides turned by a quarter (top
// as r, right as b, bottom as l, left as t).
const char* lcl_LightDirection(const Extrusion& rExtrusion)
{
    const double fX = rExtrusion.fLightX;
    const double fY = rExtrusion.fLightY;
    if (fX == 0 && fY == 0)
        return "t";
    if (fY < 0)
        return fX < 0 ? "tl" : (fX > 0 ? "tr" : "r");
    if (fY > 0)
        return fX < 0 ? "bl" : (fX > 0 ? "br" : "l");
    return fX < 0 ? "t" : "b";
}

// The light rig of the extrusion: the legacy rigs of Office are the two-light model the engine
// draws, flat, normal and harsh being the bright, normal and dim levels of its lighting toolbar
// (second light at 66, 37 and 21 percent); the digit puts the first light in front (1) or above
// (3), from where the direction turns it to its quadrant.
const char* lcl_LightRig(const Extrusion& rExtrusion)
{
    const bool bFront = rExtrusion.fLightX == 0 && rExtrusion.fLightY == 0;
    if (rExtrusion.fSecondLightLevel < 29)
        return bFront ? "legacyHarsh1" : "legacyHarsh3";
    if (rExtrusion.fSecondLightLevel < 52)
        return bFront ? "legacyNormal1" : "legacyNormal3";
    return bFront ? "legacyFlat1" : "legacyFlat3";
}

// The material of the extrusion: wire frame, the two metals, plastic for a shiny surface and
// matte for a dull one.
const char* lcl_Material(const Extrusion& rExtrusion)
{
    if (rExtrusion.bWireFrame)
        return "legacyWireframe";
    if (rExtrusion.bMetal)
        return rExtrusion.bMetalMSCompatible ? "legacyMetal" : "metal";
    if (rExtrusion.fSpecularity >= 100)
        return "legacyPlastic";
    if (rExtrusion.fSpecularity > 0)
        return "plastic";
    return rExtrusion.fDiffusion >= 100 ? "legacyMatte" : "matte";
}

// The rotation of the camera for the rotation of the extrusion: Office turns around y by the
// longitude first, then around x by the latitude; the engine turns around y, then x, by the two
// angles of RotateAngle, which are the same turns in the same order with the x angle negated.
void lcl_CameraRotation(const Extrusion& rExtrusion, sal_Int32& rLat, sal_Int32& rLon)
{
    rLat = static_cast<sal_Int32>(std::lround(lcl_NormalisedAngle(-rExtrusion.fRotateX) * 60000));
    rLon = static_cast<sal_Int32>(std::lround(lcl_NormalisedAngle(rExtrusion.fRotateY) * 60000));
}
}

bool DrawingML::WriteExtrusion3D(const Reference<XPropertySet>& rXPropSet, bool bIsText)
{
    Extrusion aExtrusion;
    if (!lcl_ReadExtrusion(rXPropSet, aExtrusion))
        return false;
    // the 3D a file brought along is replayed as it was by the caller, so that Office gets its own
    // presets back; the extrusion set is written for a shape extruded in the engine
    if (lcl_GrabBagHas3D(rXPropSet, bIsText))
        return false;

    const std::string_view sCamera = lcl_CameraPreset(aExtrusion);
    sal_Int32 nLat = 0;
    sal_Int32 nLon = 0;
    lcl_CameraRotation(aExtrusion, nLat, nLon);

    mpFS->startElementNS(XML_a, XML_scene3d);
    mpFS->startElementNS(XML_a, XML_camera, XML_prst, OString(sCamera));
    if (nLat != 0 || nLon != 0)
        mpFS->singleElementNS(XML_a, XML_rot, XML_lat, OString::number(nLat), XML_lon,
                              OString::number(nLon), XML_rev, "0");
    mpFS->endElementNS(XML_a, XML_camera);
    mpFS->singleElementNS(XML_a, XML_lightRig, XML_rig, lcl_LightRig(aExtrusion), XML_dir,
                          lcl_LightDirection(aExtrusion));
    mpFS->endElementNS(XML_a, XML_scene3d);

    // the depth is the extrusion height; the part of it in front of the shape plane is z, as the
    // import reads them
    const double fDepthEmu = aExtrusion.fDepth * 360.0; // 1/100 mm to EMU
    const sal_Int64 nHeight = static_cast<sal_Int64>(std::lround(fDepthEmu));
    const sal_Int64 nFront = static_cast<sal_Int64>(std::lround(fDepthEmu * aExtrusion.fDepthFraction));
    mpFS->startElementNS(XML_a, XML_sp3d, XML_extrusionH, OString::number(std::max<sal_Int64>(nHeight, 0)),
                         XML_z, sax_fastparser::UseIf(OString::number(nFront), nFront != 0),
                         XML_prstMaterial, lcl_Material(aExtrusion));
    if (aExtrusion.bColor)
    {
        sal_Int32 nColor = 0;
        if (rXPropSet->getPropertyValue(u"FillColor2"_ustr) >>= nColor)
        {
            mpFS->startElementNS(XML_a, XML_extrusionClr);
            WriteColor(::Color(ColorTransparency, nColor & 0xffffff));
            mpFS->endElementNS(XML_a, XML_extrusionClr);
        }
    }
    mpFS->endElementNS(XML_a, XML_sp3d);
    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
