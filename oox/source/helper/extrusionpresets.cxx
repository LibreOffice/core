/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <oox/helper/extrusionpresets.hxx>

#include <com/sun/star/drawing/EnhancedCustomShapeParameterType.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>

#include <basegfx/matrix/b3dhommatrix.hxx>
#include <drawingml/customshapeproperties.hxx>
#include <drawingml/shape3dproperties.hxx>

namespace oox
{
// order: prst, fov, rotLat, rotLon, rotRev, ProjectionMode, Skew amount, Skew angle,
// OriginX, OriginY, RotateAngle First, RotateAngle Second, Rotate Z
constexpr sal_uInt16 nCameraPresetCount(62); // Fixed, defined in OOXML standard.
static const CameraSettings aCameraPresetArray[nCameraPresetCount] = {
    { u"orthographicFront", 2700000, 0, 0, 0, u"parallel", 0, 0, 0, 0, 0, 0, 25000, 0, 0, 0 },
    // legacyOblique. Used by MS Office in import from ODF, suitable for export to OOXML
    // UI 'North'
    { u"legacyObliqueBottom", 2700000, 0, 0, 0, u"parallel", 50, 90, 0, 0.5, 0, 3472, 25000, 0, 0,
      0 },
    // UI 'North East'
    { u"legacyObliqueBottomLeft", 2700000, 0, 0, 0, u"parallel", 50, 45, -0.5, 0.5, -3472, 3472,
      25000, 0, 0, 0 },
    // UI 'North West'
    { u"legacyObliqueBottomRight", 2700000, 0, 0, 0, u"parallel", 50, 135, 0.5, 0.5, 3472, 3472,
      25000, 0, 0, 0 },
    // UI 'Backwards'
    { u"legacyObliqueFront", 2700000, 0, 0, 0, u"parallel", 0, 0, 0, 0, 0, 0, 25000, 0, 0, 0 },
    // UI 'East'
    { u"legacyObliqueLeft", 2700000, 0, 0, 0, u"parallel", 50, -360, -0.5, 0, -3472, 0, 25000, 0, 0,
      0 },
    // UI 'West'
    { u"legacyObliqueRight", 2700000, 0, 0, 0, u"parallel", 50, 180, 0.5, 0, 3472, 0, 25000, 0, 0,
      0 },
    // UI 'South'
    { u"legacyObliqueTop", 2700000, 0, 0, 0, u"parallel", 50, -90, -0.5, 0, 0, -3472, 25000, 0, 0,
      0 },
    // UI 'South East'
    { u"legacyObliqueTopLeft", 2700000, 0, 0, 0, u"parallel", 50, -45, -0.5, -0.5, -3472, -3472,
      25000, 0, 0, 0 },
    // UI 'South West'
    { u"legacyObliqueTopRight", 2700000, 0, 0, 0, u"parallel", 50, -135, 0.5, -0.5, 3472, -3472,
      25000, 0, 0, 0 },

    // legacyPerspective. Used by MS Office in import from ODF, suitable for export to OOXML
    // UI 'North'
    { u"legacyPerspectiveBottom", 3900000, 0, 0, 0, u"perspective", 50, 90, 0, 0.5, 0, 3472, 25000,
      0, 0, 0 },
    // UI 'North East'
    { u"legacyPerspectiveBottomLeft", 3900000, 0, 0, 0, u"perspective", 50, 45, -0.5, 0.5, -3472,
      3472, 25000, 0, 0, 0 },
    // UI 'North West'
    { u"legacyPerspectiveBottomRight", 3900000, 0, 0, 0, u"perspective", 50, 135, 0.5, 0.5, 3472,
      3472, 25000, 0, 0, 0 },
    // UI 'Backwards'
    { u"legacyPerspectiveFront", 3900000, 0, 0, 0, u"perspective", 0, 0, 0, 0, 0, 0, 25000, 0, 0,
      0 },
    // UI 'East'
    { u"legacyPerspectiveLeft", 3900000, 0, 0, 0, u"perspective", 50, -360, -0.5, 0, -3472, 0,
      25000, 0, 0, 0 },
    // UI 'West'
    { u"legacyPerspectiveRight", 3900000, 0, 0, 0, u"perspective", 50, 180, 0.5, 0, 3472, 0, 25000,
      0, 0, 0 },
    // UI 'South'
    { u"legacyPerspectiveTop", 3900000, 0, 0, 0, u"perspective", 50, -90, 0, -0.5, 0, -3472, 25000,
      0, 0, 0 },
    // UI 'South East'
    { u"legacyPerspectiveTopLeft", 3900000, 0, 0, 0, u"perspective", 50, -45, -0.5, -0.5, -3472,
      -3472, 25000, 0, 0, 0 },
    // UI 'South West'
    { u"legacyPerspectiveTopRight", 3900000, 0, 0, 0, u"perspective", 50, -135, 0.5, -0.5, 3472,
      -3472, 25000, 0, 0, 0 },

    // isometric. Values in comment are from UI in PowerPoint, at vert, hori, depth axis
    { u"isometricBottomDown", 2700000, 2123775, 18883146, 17990253, u"parallel", 0, 0, 0, 0, 3472,
      -3472, 25000, -55, 0, -45 }, // MS Office UI 314.7, 35.4, 299.8
    { u"isometricBottomUp", 2700000, 2123775, 2716853, 3609746, u"parallel", 0, 0, 0, 0, 3472,
      -3472, 25000, -55, 0, 45 }, // MS Office UI 45.3, 35.4, 60.2
    { u"isometricLeftDown", 2700000, 2100000, 2700000, 0, u"parallel", 0, 0, 0, 0, 3472, -3472,
      25000, -35, 45, 0 }, // MS Office UI 45, 35, 0
    { u"isometricLeftUp", 2700000, 19500000, 2700000, 0, u"parallel", 0, 0, 0, 0, 3472, -3472,
      25000, 35, 45, 0 }, // MS Office UI 45, 325, 0
    { u"isometricOffAxis1Left", 2700000, 1080000, 3840000, 0, u"parallel", 0, 0, 0, 0, 3472, -3472,
      25000, -18, 64, 0 }, // MS Office UI 64, 18, 0
    { u"isometricOffAxis1Right", 2700000, 1080000, 20040000, 0, u"parallel", 0, 0, 0, 0, 3472,
      -3472, 25000, -18, 334, 0 }, // MS Office UI 334, 18, 0
    { u"isometricOffAxis1Top", 2700000, 18075715, 18392745, 3458551, u"parallel", 0, 0, 0, 0, 3472,
      -3472, 25000, 72, 0, 26 }, // MS Office UI 306.5, 301.3, 57.6
    { u"isometricOffAxis2Left", 2700000, 1080000, 1560000, 0, u"parallel", 0, 0, 0, 0, 3472, -3472,
      25000, -18, 26, 0 }, // MS Office UI 26, 18, 0
    { u"isometricOffAxis2Right", 2700000, 1080000, 17760000, 0, u"parallel", 0, 0, 0, 0, 3472,
      -3472, 25000, -18, 296, 0 }, // MS Office UI 296, 18, 0
    { u"isometricOffAxis2Top", 2700000, 18075715, 3207254, 18141448, u"parallel", 0, 0, 0, 0, 3472,
      -3472, 25000, 72, 0, -26 }, // MS Office UI 53.5, 301.3, 302.4
    { u"isometricOffAxis3Bottom", 2700000, 3524284, 18392745, 18141448, u"parallel", 0, 0, 0, 0,
      3472, -3472, 25000, -72, 0, -26 }, // MS Office UI 306.5, 58.7, 302.4
    { u"isometricOffAxis3Left", 2700000, 20520000, 3840000, 0, u"parallel", 0, 0, 0, 0, 3472, -3472,
      25000, 18, 64, 0 }, // MS Office UI 64, 342, 0
    { u"isometricOffAxis3Right", 2700000, 20520000, 20040000, 0, u"parallel", 0, 0, 0, 0, 3472,
      -3472, 25000, 18, -26, 0 }, // MS Office UI 334, 342, 0
    { u"isometricOffAxis4Bottom", 2700000, 3524284, 3207254, 3458551, u"parallel", 0, 0, 0, 0, 3472,
      -3472, 25000, -72, 0, 26 }, // MS Office UI 53.5, 58.7, 57.6
    { u"isometricOffAxis4Left", 2700000, 20520000, 1560000, 0, u"parallel", 0, 0, 0, 0, 3472, -3472,
      25000, 18, 26, 0 }, // MS Office UI 26, 342, 0
    { u"isometricOffAxis4Right", 2700000, 20520000, 17760000, 0, u"parallel", 0, 0, 0, 0, 3472,
      -3472, 25000, 18, -64, 0 }, // MS Office UI 296, 342, 0
    { u"isometricRightDown", 2700000, 19500000, 18900000, 0, u"parallel", 0, 0, 0, 0, 3472, -3472,
      25000, 35, -45, 0 }, // MS Office UI 315, 325, 0
    { u"isometricRightUp", 2700000, 2100000, 18900000, 0, u"parallel", 0, 0, 0, 0, 3472, -3472,
      25000, -35, -45, 0 }, // MS Office UI 315, 35, 0
    { u"isometricTopDown", 2700000, 19476224, 2716853, 17990253, u"parallel", 0, 0, 0, 0, 3472,
      -3472, 25000, 55, 0, -45 }, // MS Office UI 45.3, 324.6, 299.8
    { u"isometricTopUp", 2700000, 19476224, 18883146, 3609746, u"parallel", 0, 0, 0, 0, 3472, -3472,
      25000, 55, 0, 45 }, //MS Office UI 314.7, 324.6, 60.2

    // oblique. Not in UI in PowerPoint. Similar as legacy oblique but with 30% skewAmount
    { u"obliqueBottom", 2700000, 0, 0, 0, u"parallel", 30, 90, 0, 0.5, 0, 3472, 25000, 0, 0,
      0 }, // ToDo
    { u"obliqueBottomLeft", 2700000, 0, 0, 0, u"parallel", 30, 45, -0.5, 0.5, -3472, 3472, 25000, 0,
      0, 0 }, // ToDo
    { u"obliqueBottomRight", 2700000, 0, 0, 0, u"parallel", 30, 135, 0.5, 0.5, 3472, 3472, 25000, 0,
      0, 0 }, // ToDo
    { u"obliqueLeft", 2700000, 0, 0, 0, u"parallel", 30, -360, -0.5, 0, -3472, 0, 25000, 0, 0,
      0 }, // ToDo
    { u"obliqueRight", 2700000, 0, 0, 0, u"parallel", 30, 180, 0.5, 0, 3471, 0, 25000, 0, 0,
      0 }, // ToDo
    { u"obliqueTop", 2700000, 0, 0, 0, u"parallel", 30, -90, 0, -0.5, 0, -3472, 25000, 0, 0,
      0 }, // ToDo
    { u"obliqueTopLeft", 2700000, 0, 0, 0, u"parallel", 30, -45, -0.5, -0.5, -3472, -3472, 25000, 0,
      0, 0 }, // ToDo
    { u"obliqueTopRight", 2700000, 0, 0, 0, u"parallel", 30, -135, 0.5, -0.5, 3472, -3472, 25000, 0,
      0, 0 }, // ToDo

    // perspective.
    // Values in comment are from UI in PowerPoint, at vert, hori, depth axis, perspective.
    // Perspective is 45 if not listed. ToDo: add suitable default Origin and viewPoint
    { u"perspectiveAbove", 2700000, 20400000, 0, 0, u"perspective", 0, 0, 0, 0, 0, 0, 25000, -340,
      0, 0 }, // MS Office UI 0, 340, 0
    { u"perspectiveAboveLeftFacing", 2700000, 0, 0, 0, u"perspective", 0, 0, 0, 0, 0, 0, 25000,
      -33.729692, 25.625585, -16.141175 }, // MS Office UI 14.3, 39.3, 341.1
    { u"perspectiveAboveRightFacing", 2700000, 0, 0, 0, u"perspective", 0, 0, 0, 0, 0, 0, 25000,
      -33.729692, -25.625585, 16.141175 }, // MS Office UI 345.7, 39.3, 18.9
    { u"perspectiveBelow", 2700000, 1200000, 0, 0, u"perspective", 0, 0, 0, 0, 0, 0, 25000, -20, 0,
      0 }, // MS Office UI 0, 20, 0
    { u"perspectiveContrastingLeftFacing", 2700000, 624000, 2634000, 21384000, u"perspective", 0, 0,
      0, 0, 0, 0, 25000, -10.4, 43.9, -356.4 }, // MS Office UI 43.9, 10.4, 356.4
    { u"perspectiveContrastingRightFacing", 2700000, 624000, 18966000, 216000, u"perspective", 0, 0,
      0, 0, 0, 0, 25000, -10.4, 316.1, 3.6 }, // MS Office UI 316.1, 10.4, 3.6
    { u"perspectiveFront", 2700000, 0, 0, 0, u"perspective", 0, 0, 0, 0, 0, 0, 25000, 0, 0,
      0 }, // MS Office UI 0, 0, 0
    { u"perspectiveHeroicExtremeLeftFacing", 2700000, 486000, 2070000, 21426000, u"perspective", 0,
      0, 0, 0, 0, 0, 25000, -8.1, 34.5, 357.1 }, // MS Office UI 34.5, 8.1, 357.1, 80
    { u"perspectiveHeroicExtremeRightFacing", 2700000, 486000, 1953000, 174000, u"perspective", 0,
      0, 0, 0, 0, 0, 25000, -8.1, 325.5, 2.9 }, // MS Office UI 325.5, 8.1, 2.9, 80
    { u"perspectiveHeroicLeftFacing", 2700000, 0, 0, 0, u"perspective", 0, 0, 0, 0, 0, 0, 25000,
      10.337163, 14.781420, 2.639612 }, // MS Office UI 14.3, 349, 2.6
    { u"perspectiveHeroicRightFacing", 2700000, 0, 0, 0, u"perspective", 0, 0, 0, 0, 0, 0, 25000,
      10.337163, -14.781420, -2.639612 }, // MS Office UI 345.7, 349, 357.4
    { u"perspectiveLeft", 2700000, 0, 1200000, 0, u"perspective", 0, 0, 0, 0, 0, 0, 25000, 0, 20,
      0 }, // MS Office UI 20, 0, 0
    { u"perspectiveRelaxed", 2700000, 18576000, 0, 0, u"perspective", 0, 0, 0, 0, 0, 0, 25000,
      -309.6, 0, 0 }, // MS Office UI 0, 309.6, 0
    { u"perspectiveRelaxedModerately", 2700000, 19488000, 0, 0, u"perspective", 0, 0, 0, 0, 0, 0,
      25000, -324.8, 0, 0 }, // MS Office UI 0, 324.8, 0
    { u"perspectiveRight", 2700000, 0, 20400000, 0, u"perspective", 0, 0, 0, 0, 0, 0, 25000, 0, 340,
      0 } // MS Office UI 340, 0, 0
};

double CameraSettings::getRotateAngleZ() { return mfRotateAngleZ; }

CameraSettings CameraMapper::getDefaultCameraSettings() { return aCameraPresetArray[0]; }

sal_uInt16 CameraMapper::getPrstIndex(const OUString& rPresetType)
{
    sal_uInt16 nIt(0);
    while (nIt < nCameraPresetCount && aCameraPresetArray[nIt].msType != rPresetType)
        ++nIt;
    if (nIt >= nCameraPresetCount)
    {
        SAL_WARN("oox", "camera preset type does not exist in OOXML");
        nIt = 0; // orthographicFront
    }
    return nIt;
}

double CameraMapper::getRotationZDegFromPrst(const sal_uInt16 nPresetIndex)
{
    if (nPresetIndex >= nCameraPresetCount)
    {
        SAL_WARN("oox", "invalid camera preset type index");
        return 0.0;
    }
    else
        return aCameraPresetArray[nPresetIndex].mfRotateAngleZ;
}

void CameraMapper::getRotationAngleOOXFromPrst(oox::drawingml::RotationProperties& rRot,
                                               const sal_uInt16 nPresetIndex)
{
    // Alternatively, this could be implemented by calculating it from mfRotateAngleFirst,
    // mfRotateAngleSecond and mfRotateAngleZ.
    if (nPresetIndex >= nCameraPresetCount)
    {
        SAL_WARN("oox", "invalid camera preset type index");
        return;
    }
    rRot.mnLatitude
        = aCameraPresetArray[nPresetIndex].mnRotLat; // ToDo ? cast sal_uInt32 to sal_Int32
    rRot.mnLongitude = aCameraPresetArray[nPresetIndex].mnRotLon;
    rRot.mnRevolution = aCameraPresetArray[nPresetIndex].mnRotRev;
    return;
}

void CameraMapper::addCameraPrstSettingsToExtrusion(oox::drawingml::Extrusion& rExtrusion,
                                                    const sal_uInt16 nPresetIndex)
{
    if (nPresetIndex >= nCameraPresetCount)
    {
        SAL_WARN("oox", "invalid camera preset type index");
        return;
    }
    rExtrusion.meProjectionMode = (aCameraPresetArray[nPresetIndex].msProjectionMode == u"parallel")
                                      ? css::drawing::ProjectionMode_PARALLEL
                                      : css::drawing::ProjectionMode_PERSPECTIVE;
    rExtrusion.maRotateAngle.First.Value <<= aCameraPresetArray[nPresetIndex].mfRotateAngleFirst;
    rExtrusion.maRotateAngle.First.Type <<= 0;
    rExtrusion.maRotateAngle.Second.Value <<= aCameraPresetArray[nPresetIndex].mfRotateAngleSecond;
    rExtrusion.maRotateAngle.Second.Type <<= 0;
    rExtrusion.maOrigin.First.Value <<= aCameraPresetArray[nPresetIndex].mfOriginX;
    rExtrusion.maOrigin.First.Type <<= 0;
    rExtrusion.maOrigin.Second.Value <<= aCameraPresetArray[nPresetIndex].mfOriginY;
    rExtrusion.maOrigin.Second.Type <<= 0;
    rExtrusion.maSkew.First.Value <<= aCameraPresetArray[nPresetIndex].mfSkewAmount;
    rExtrusion.maSkew.First.Type <<= 0;
    rExtrusion.maSkew.Second.Value <<= aCameraPresetArray[nPresetIndex].mfSkewAngle;
    rExtrusion.maSkew.Second.Type <<= 0;
    rExtrusion.maViewPoint.PositionX = aCameraPresetArray[nPresetIndex].mfViewPointX;
    rExtrusion.maViewPoint.PositionY = aCameraPresetArray[nPresetIndex].mfViewPointY;
    rExtrusion.maViewPoint.PositionZ = aCameraPresetArray[nPresetIndex].mfViewPointZ;
    return;
}

void CameraMapper::fillExtrusionWithPresetValues(const OUString sPresetName,
                                                 oox::drawingml::Extrusion& rExtrusion,
                                                 double& fShapeRotation)
{
    sal_uInt16 nIt(0);
    while (nIt < nCameraPresetCount && aCameraPresetArray[nIt].msType != sPresetName)
        ++nIt;
    if (nIt >= nCameraPresetCount)
    {
        SAL_WARN("oox", "camera preset type does not exist in OOXML");
        return;
    }
    rExtrusion.meProjectionMode = (aCameraPresetArray[nIt].msProjectionMode == u"parallel")
                                      ? css::drawing::ProjectionMode_PARALLEL
                                      : css::drawing::ProjectionMode_PERSPECTIVE;
    rExtrusion.maRotateAngle.First.Value <<= aCameraPresetArray[nIt].mfRotateAngleFirst;
    rExtrusion.maRotateAngle.First.Type <<= css::drawing::EnhancedCustomShapeParameterType::NORMAL;
    rExtrusion.maRotateAngle.Second.Value <<= aCameraPresetArray[nIt].mfRotateAngleSecond;
    rExtrusion.maRotateAngle.Second.Type <<= css::drawing::EnhancedCustomShapeParameterType::NORMAL;
    fShapeRotation = aCameraPresetArray[nIt].mfRotateAngleZ;
    rExtrusion.maOrigin.First.Value <<= aCameraPresetArray[nIt].mfOriginX;
    rExtrusion.maOrigin.First.Type <<= css::drawing::EnhancedCustomShapeParameterType::NORMAL;
    rExtrusion.maOrigin.Second.Value <<= aCameraPresetArray[nIt].mfOriginY;
    rExtrusion.maOrigin.Second.Type <<= css::drawing::EnhancedCustomShapeParameterType::NORMAL;
    rExtrusion.maSkew.First.Value <<= aCameraPresetArray[nIt].mfSkewAmount;
    rExtrusion.maSkew.First.Type <<= css::drawing::EnhancedCustomShapeParameterType::NORMAL;
    rExtrusion.maSkew.Second.Value <<= aCameraPresetArray[nIt].mfSkewAngle;
    rExtrusion.maSkew.Second.Type <<= css::drawing::EnhancedCustomShapeParameterType::NORMAL;
    rExtrusion.maViewPoint.PositionX = aCameraPresetArray[nIt].mfViewPointX;
    rExtrusion.maViewPoint.PositionY = aCameraPresetArray[nIt].mfViewPointY;
    rExtrusion.maViewPoint.PositionZ = aCameraPresetArray[nIt].mfViewPointZ;
    return;
}

void CameraMapper::calculateRotationAngleLODegFromAngleOOX(
    css::drawing::EnhancedCustomShapeParameterPair& rSceneRotDeg, double& rShapeRotDeg,
    const oox::drawingml::RotationProperties& aOOXRot)
{
    // Convert OOX angles to rad and correct sign to usual orientation in 3D right-hand
    // coordinate system
    double fLat = basegfx::deg2rad(aOOXRot.mnLatitude.get(0) / 60000.0); // horizontal
    double fLon = -basegfx::deg2rad(aOOXRot.mnLongitude.get(0) / 60000.0); // vertical
    double fRev = basegfx::deg2rad(aOOXRot.mnRevolution.get(0) / 60000.0); // depth
    // MS Office rotates first at vertical axis, then at horizontal axis and last at depth-axis
    // rotate uses order first X, then Y, last Z. Parameter order (X, Y, Z)
    basegfx::B3DHomMatrix aRot;
    aRot.rotate(0.0, fLon, 0.0);
    aRot.rotate(fLat, 0.0, fRev);
    // ODF specifies order first Z, then Y, last X. Decompose aRot as if this order was used.
    // caution: matrix element indices start with 0 so we have first row 00, 01, 02
    // second row 10, 11, 12 and third row 20, 21, 22. Forth row is default and not used.
    double fRotX;
    double fRotY;
    double fRotZ;
    if (basegfx::fTools::equal(fabs(aRot.get(0, 2)), 1.0)) // I expect rounding errors
    {
        // The rotation at vertical axis is about 90deg or 270deg. The x-axis is mapped to the
        // z-axis. It is only possible to determine x+z or x-z respectively, but not single
        // values for x and z. Set z=0 here.
        fRotY = aRot.get(0, 2) > 0.0 ? F_PI2 : -F_PI2;
        fRotZ = 0.0;
        fRotX = atan2(aRot.get(2, 1), aRot.get(1, 1));
    }
    else
    {
        fRotZ = atan2(-aRot.get(0, 1), aRot.get(0, 0));
        fRotX = atan2(-aRot.get(1, 2), aRot.get(2, 2));
        if (basegfx::fTools::equalZero(cos(fRotZ)))
        {
            // cos(angle)==0  =>  sin(angle)!= 0
            fRotY = atan2(-aRot.get(0, 1), -aRot.get(0, 1) / sin(fRotZ));
        }
        else
            fRotY = atan2(aRot.get(0, 2), aRot.get(0, 0) / cos(fRotZ));
    }
    // LO uses rotation angles in Api not with the usual sign, so we need to correct it here.
    rSceneRotDeg.First.Value <<= basegfx::rad2deg(-fRotX);
    rSceneRotDeg.First.Type <<= css::drawing::EnhancedCustomShapeParameterType::NORMAL;
    rSceneRotDeg.Second.Value <<= basegfx::rad2deg(-fRotY);
    rSceneRotDeg.Second.Type <<= css::drawing::EnhancedCustomShapeParameterType::NORMAL;
    rShapeRotDeg = basegfx::rad2deg(fRotZ);
    return;
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
