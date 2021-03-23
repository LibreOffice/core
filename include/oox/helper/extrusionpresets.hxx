/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <array>

#include <drawingml/customshapeproperties.hxx>
#include <drawingml/shape3dproperties.hxx>

namespace oox
{
struct CameraSettings
{
    // OOXML Camera, CT_Camera
    OUString msType; // OOXML attribute "prst"

    // OOXML attribute "fov" (field of view angle), "Perspective" in UI in MS Office
    // unit 1/600000 degree, range [0..180] deg in spec, [0..120] deg in UI in MS Office
    // ToDo remove multiply by 60000 in export
    sal_uInt32 mnFOVAngle;

    // OOXML childelement <dir> (Direction), rotation
    // range [0..360[ deg, unit 1/60000 degree
    sal_uInt32 mnRotLon; // Longitude, MS Office UI X, OOXML attribute "lon", axis vertical
    sal_uInt32 mnRotLat; // Latitude, MS Office UI Y, OOXML attribute "lat", axis horizontal
    sal_uInt32 mnRotRev; // Revolution, MS Office UI Z, OOXML attribute "rev"

    // ODF camera, attributes of element <draw:enhanced-geometry>
    // Settings belong to property set "Extrusion" in "CustomShapeGeometry". Complex components are
    // not defined with their API types. That makes it easier to write a map using an initalizer list

    // dr3d:projection, value "parallel" or "perspective"
    // API css::drawing::ProjectionMode, use string here because of initializer list
    OUString msProjectionMode;

    // draw:extrusion-skew, type string. API Skew, type EnhancedCustomShapeParameterPair
    // only evaluated for projection mode "parallel"
    double mfSkewAmount; // range 0 to 100, percent of depth used as slant length
    double mfSkewAngle; // range [0..360[, internal unit degree

    // draw:extrusion-origin, type string. API Origin, type EnhancedCustomShapeParameterPair
    // range [-0.5 (left).. 0.5 (right)] and [-0.5 (top) 0.5 (right)], relative to shape
    double mfOriginX;
    double mfOriginY;

    // draw:extrusion-viewpoint, type point3D, length with unit. API ViewPoint, type Position3D
    // Internal unit 1/100mm. Only evaluated for projection 'perspective'
    // LO uses it relative to the rotated scene, evaluated together with 'Origin'.
    // LO has no UI for it, but it is needed as counterpart to mnFOVAngle.
    double mfViewPointX;
    double mfViewPointY;
    double mfViewPointZ;

    // draw:extrusion-rotation-angle, type string.
    // API RotateAngle, type EnhancedCustomShapeParameterPair,
    // internal unit degree, values beyond -360 deg and 360 deg are allowed
    // Rotation center is the center of the shape, see comment below
    double mfRotateAngleFirst; // at x-axis, positive means lower part of front face comes nearer
    double mfRotateAngleSecond; // at y-axis, positiv means right part of front face comes nearer
    // ODF does rotation at z-axis on generating shape, not in the scene camera.
    double mfRotateAngleZ; // counter-clockwise on screen, unit degree

    //ctor with default values
    CameraSettings()
        : msType("orthographicFront")
        , mnFOVAngle(270000) // 45deg
        , mnRotLon(0)
        , mnRotLat(0)
        , mnRotRev(0)
        , msProjectionMode("parallel")
        , mfSkewAmount(0.0)
        , mfSkewAngle(0.0)
        , mfOriginX(0.0)
        , mfOriginY(0.0)
        , mfViewPointX(0.0)
        , mfViewPointY(0.0)
        , mfViewPointZ(25000.0)
        , mfRotateAngleFirst(0.0)
        , mfRotateAngleSecond(0.0)
        , mfRotateAngleZ(0.0)
    {
    }

    // ctor with value list
    CameraSettings(OUString _msType, sal_uInt32 _mnFOVAngle, sal_uInt32 _mnRotLon,
                   sal_uInt32 _mnRotLat, sal_uInt32 _mnRotRev, OUString _msProjectionMode,
                   double _mfSkewAmount, double _mfSkewAngle, double _mfOriginX, double _mfOriginY,
                   double _mfViewPointX, double _mfViewPointY, double _mfViewPointZ,
                   double _mfRotateAngleFirst, double _mfRotateAngleSecond, double _mfRotateAngleZ)
        : msType(_msType)
        , mnFOVAngle(_mnFOVAngle)
        , mnRotLon(_mnRotLon)
        , mnRotLat(_mnRotLat)
        , mnRotRev(_mnRotRev)
        , msProjectionMode(_msProjectionMode)
        , mfSkewAmount(_mfSkewAmount)
        , mfSkewAngle(_mfSkewAngle)
        , mfOriginX(_mfOriginX)
        , mfOriginY(_mfOriginY)
        , mfViewPointX(_mfViewPointX)
        , mfViewPointY(_mfViewPointY)
        , mfViewPointZ(_mfViewPointZ)
        , mfRotateAngleFirst(_mfRotateAngleFirst)
        , mfRotateAngleSecond(_mfRotateAngleSecond)
        , mfRotateAngleZ(_mfRotateAngleZ)
    {
    }

    // copy ctor
    CameraSettings(const CameraSettings& rOther)
        : msType(rOther.msType)
        , mnFOVAngle(rOther.mnFOVAngle)
        , mnRotLon(rOther.mnRotLon)
        , mnRotLat(rOther.mnRotLat)
        , mnRotRev(rOther.mnRotRev)
        , msProjectionMode(rOther.msProjectionMode)
        , mfSkewAmount(rOther.mfSkewAmount)
        , mfSkewAngle(rOther.mfSkewAngle)
        , mfOriginX(rOther.mfOriginX)
        , mfOriginY(rOther.mfOriginY)
        , mfViewPointX(rOther.mfViewPointX)
        , mfViewPointY(rOther.mfViewPointY)
        , mfViewPointZ(rOther.mfViewPointZ)
        , mfRotateAngleFirst(rOther.mfRotateAngleFirst)
        , mfRotateAngleSecond(rOther.mfRotateAngleSecond)
        , mfRotateAngleZ(rOther.mfRotateAngleZ)
    {
    }

    // assignment
    CameraSettings& operator=(const CameraSettings& rOther)
    {
        if (this != &rOther)
        {
            msType = rOther.msType;
            mnFOVAngle = rOther.mnFOVAngle;
            mnRotLon = rOther.mnRotLon;
            mnRotLat = rOther.mnRotLat;
            mnRotRev = rOther.mnRotRev;
            msProjectionMode = rOther.msProjectionMode;
            mfSkewAmount = rOther.mfSkewAmount;
            mfSkewAngle = rOther.mfSkewAngle;
            mfOriginX = rOther.mfOriginX;
            mfOriginY = rOther.mfOriginY;
            mfViewPointX = rOther.mfViewPointX;
            mfViewPointY = rOther.mfViewPointY;
            mfViewPointZ = rOther.mfViewPointZ;
            mfRotateAngleFirst = rOther.mfRotateAngleFirst;
            mfRotateAngleSecond = rOther.mfRotateAngleSecond;
            mfRotateAngleZ = rOther.mfRotateAngleZ;
        }
    }

    // dummy to test compiling
    double getRotateAngleZ();
};

// The OOXML camera attribute "zoom" is not included in above struct, because ODF has no corresponding
// attribute, Powerpoint has no UI for it and LO cannot render it in custom shape extrusion scene.
// ToDo: Examine, whether preserve in grabBag is possible and meaningful.

// The ODF attribute draw:extrusion-rotation-center is not included in above struct. LO can read and
// render it, but has no UI for it. OOXML has no corresponding property but assumes center of shape
// (without extrusion part) as rotation center, which is default in LO and ODF too.
// API "RotationCenter", internal unit 1/100 mm, right hand system with x-axis to right, y-axis down,
// z-axis into the screen. Problem: Read/Write to ODF is not in shape-size fraction as specified in
// ODF and described in API, but in 1/100 mm without unit.

struct CameraMapper
{
    CameraSettings maCameraPresetArray[62];

    // order: prst, fov, rotLat, rotLon, rotRev, ProjectionMode, Skew amount, Skew angle,
    // OriginX, OriginY, RotateAngle First, RotateAngle Second, Rotate Z

    CameraMapper()
        : maCameraPresetArray{
            { "orthographicFront", 2700000, 0, 0, 0, "parallel", 0, 0, 0, 0, 0, 0, 25000, 0, 0, 0 },
            // legacyOblique. Used by MS Office in import from ODF, suitable for export to OOXML
            // UI 'North'
            { "legacyObliqueBottom", 2700000, 0, 0, 0, "parallel", 50, 90, 0, 0.5, 0, 3472, 25000,
              0, 0, 0 },
            // UI 'North East'
            { "legacyObliqueBottomLeft", 2700000, 0, 0, 0, "parallel", 50, 45, -0.5, 0.5, -3472,
              3472, 25000, 0, 0, 0 },
            // UI 'North West'
            { "legacyObliqueBottomRight", 2700000, 0, 0, 0, "parallel", 50, 135, 0.5, 0.5, 3472,
              3472, 25000, 0, 0, 0 },
            // UI 'Backwards'
            { "legacyObliqueFront", 2700000, 0, 0, 0, "parallel", 0, 0, 0, 0, 0, 0, 25000, 0, 0,
              0 },
            // UI 'East'
            { "legacyObliqueLeft", 2700000, 0, 0, 0, "parallel", 50, -360, -0.5, 0, -3472, 0, 25000,
              0, 0, 0 },
            // UI 'West'
            { "legacyObliqueRight", 2700000, 0, 0, 0, "parallel", 50, 180, 0.5, 0, 3472, 0, 25000,
              0, 0, 0 },
            // UI 'South'
            { "legacyObliqueTop", 2700000, 0, 0, 0, "parallel", 50, -90, -0.5, 0, 0, -3472, 25000,
              0, 0, 0 },
            // UI 'South East'
            { "legacyObliqueTopLeft", 2700000, 0, 0, 0, "parallel", 50, -45, -0.5, -0.5, -3472,
              -3472, 25000, 0, 0, 0 },
            // UI 'South West'
            { "legacyObliqueTopRight", 2700000, 0, 0, 0, "parallel", 50, -135, 0.5, -0.5, 3472,
              -3472, 25000, 0, 0, 0 },

            // legacyPerspective. Used by MS Office in import from ODF, suitable for export to OOXML
            // UI 'North'
            { "legacyPerspectiveBottom", 3900000, 0, 0, 0, "perspective", 50, 90, 0, 0.5, 0, 3472,
              25000, 0, 0, 0 },
            // UI 'North East'
            { "legacyPerspectiveBottomLeft", 3900000, 0, 0, 0, "perspective", 50, 45, -0.5, 0.5,
              -3472, 3472, 25000, 0, 0, 0 },
            // UI 'North West'
            { "legacyPerspectiveBottomRight", 3900000, 0, 0, 0, "perspective", 50, 135, 0.5, 0.5,
              3472, 3472, 25000, 0, 0, 0 },
            // UI 'Backwards'
            { "legacyPerspectiveFront", 3900000, 0, 0, 0, "perspective", 0, 0, 0, 0, 0, 0, 25000, 0,
              0, 0 },
            // UI 'East'
            { "legacyPerspectiveLeft", 3900000, 0, 0, 0, "perspective", 50, -360, -0.5, 0, -3472, 0,
              25000, 0, 0, 0 },
            // UI 'West'
            { "legacyPerspectiveRight", 3900000, 0, 0, 0, "perspective", 50, 180, 0.5, 0, 3472, 0,
              25000, 0, 0, 0 },
            // UI 'South'
            { "legacyPerspectiveTop", 3900000, 0, 0, 0, "perspective", 50, -90, 0, -0.5, 0, -3472,
              25000, 0, 0, 0 },
            // UI 'South East'
            { "legacyPerspectiveTopLeft", 3900000, 0, 0, 0, "perspective", 50, -45, -0.5, -0.5,
              -3472, -3472, 25000, 0, 0, 0 },
            // UI 'South West'
            { "legacyPerspectiveTopRight", 3900000, 0, 0, 0, "perspective", 50, -135, 0.5, -0.5,
              3472, -3472, 25000, 0, 0, 0 },

            // isometric. Values in comment are from UI in PowerPoint, at vert, hori, depth axis
            { "isometricBottomDown", 2700000, 2123775, 18883146, 17990253, "parallel", 0, 0, 0, 0,
              3472, -3472, 25000, -55, 0, -45 }, // MS Office UI 314.7, 35.4, 299.8
            { "isometricBottomUp", 2700000, 2123775, 2716853, 3609746, "parallel", 0, 0, 0, 0, 3472,
              -3472, 25000, -55, 0, 45 }, // MS Office UI 45.3, 35.4, 60.2
            { "isometricLeftDown", 2700000, 2100000, 2700000, 0, "parallel", 0, 0, 0, 0, 3472,
              -3472, 25000, -35, 45, 0 }, // MS Office UI 45, 35, 0
            { "isometricLeftUp", 2700000, 19500000, 2700000, 0, "parallel", 0, 0, 0, 0, 3472, -3472,
              25000, 35, 45, 0 }, // MS Office UI 45, 325, 0
            { "isometricOffAxis1Left", 2700000, 1080000, 3840000, 0, "parallel", 0, 0, 0, 0, 3472,
              -3472, 25000, -18, 64, 0 }, // MS Office UI 64, 18, 0
            { "isometricOffAxis1Right", 2700000, 1080000, 20040000, 0, "parallel", 0, 0, 0, 0, 3472,
              -3472, 25000, -18, 334, 0 }, // MS Office UI 334, 18, 0
            { "isometricOffAxis1Top", 2700000, 18075715, 18392745, 3458551, "parallel", 0, 0, 0, 0,
              3472, -3472, 25000, 72, 0, 26 }, // MS Office UI 306.5, 301.3, 57.6
            { "isometricOffAxis2Left", 2700000, 1080000, 1560000, 0, "parallel", 0, 0, 0, 0, 3472,
              -3472, 25000, -18, 26, 0 }, // MS Office UI 26, 18, 0
            { "isometricOffAxis2Right", 2700000, 1080000, 17760000, 0, "parallel", 0, 0, 0, 0, 3472,
              -3472, 25000, -18, 296, 0 }, // MS Office UI 296, 18, 0
            { "isometricOffAxis2Top", 2700000, 18075715, 3207254, 18141448, "parallel", 0, 0, 0, 0,
              3472, -3472, 25000, 72, 0, -26 }, // MS Office UI 53.5, 301.3, 302.4
            { "isometricOffAxis3Bottom", 2700000, 3524284, 18392745, 18141448, "parallel", 0, 0, 0,
              0, 3472, -3472, 25000, -72, 0, -26 }, // MS Office UI 306.5, 58.7, 302.4
            { "isometricOffAxis3Left", 2700000, 20520000, 3840000, 0, "parallel", 0, 0, 0, 0, 3472,
              -3472, 25000, 18, 64, 0 }, // MS Office UI 64, 342, 0
            { "isometricOffAxis3Right", 2700000, 20520000, 20040000, 0, "parallel", 0, 0, 0, 0,
              3472, -3472, 25000, 18, -26, 0 }, // MS Office UI 334, 342, 0
            { "isometricOffAxis4Bottom", 2700000, 3524284, 3207254, 3458551, "parallel", 0, 0, 0, 0,
              3472, -3472, 25000, -72, 0, 26 }, // MS Office UI 53.5, 58.7, 57.6
            { "isometricOffAxis4Left", 2700000, 20520000, 1560000, 0, "parallel", 0, 0, 0, 0, 3472,
              -3472, 25000, 18, 26, 0 }, // MS Office UI 26, 342, 0
            { "isometricOffAxis4Right", 2700000, 20520000, 17760000, 0, "parallel", 0, 0, 0, 0,
              3472, -3472, 25000, 18, -64, 0 }, // MS Office UI 296, 342, 0
            { "isometricRightDown", 2700000, 19500000, 18900000, 0, "parallel", 0, 0, 0, 0, 3472,
              -3472, 25000, 35, -45, 0 }, // MS Office UI 315, 325, 0
            { "isometricRightUp", 2700000, 2100000, 18900000, 0, "parallel", 0, 0, 0, 0, 3472,
              -3472, 25000, -35, -45, 0 }, // MS Office UI 315, 35, 0
            { "isometricTopDown", 2700000, 19476224, 2716853, 17990253, "parallel", 0, 0, 0, 0,
              3472, -3472, 25000, 55, 0, -45 }, // MS Office UI 45.3, 324.6, 299.8
            { "isometricTopUp", 2700000, 19476224, 18883146, 3609746, "parallel", 0, 0, 0, 0, 3472,
              -3472, 25000, 55, 0, 45 }, //MS Office UI 314.7, 324.6, 60.2

            // oblique. Not in UI in PowerPoint. Similar as legacy oblique but with 30% skewAmount
            { "obliqueBottom", 2700000, 0, 0, 0, "parallel", 30, 90, 0, 0.5, 0, 3472, 25000, 0, 0,
              0 }, // ToDo
            { "obliqueBottomLeft", 2700000, 0, 0, 0, "parallel", 30, 45, -0.5, 0.5, -3472, 3472,
              25000, 0, 0, 0 }, // ToDo
            { "obliqueBottomRight", 2700000, 0, 0, 0, "parallel", 30, 135, 0.5, 0.5, 3472, 3472,
              25000, 0, 0, 0 }, // ToDo
            { "obliqueLeft", 2700000, 0, 0, 0, "parallel", 30, -360, -0.5, 0, -3472, 0, 25000, 0, 0,
              0 }, // ToDo
            { "obliqueRight", 2700000, 0, 0, 0, "parallel", 30, 180, 0.5, 0, 3471, 0, 25000, 0, 0,
              0 }, // ToDo
            { "obliqueTop", 2700000, 0, 0, 0, "parallel", 30, -90, 0, -0.5, 0, -3472, 25000, 0, 0,
              0 }, // ToDo
            { "obliqueTopLeft", 2700000, 0, 0, 0, "parallel", 30, -45, -0.5, -0.5, -3472, -3472,
              25000, 0, 0, 0 }, // ToDo
            { "obliqueTopRight", 2700000, 0, 0, 0, "parallel", 30, -135, 0.5, -0.5, 3472, -3472,
              25000, 0, 0, 0 }, // ToDo

            // perspective.
            // Values in comment are from UI in PowerPoint, at vert, hori, depth axis, perspective.
            // Perspective is 45 if not listed. ToDo: add suitable default Origin and viewPoint
            { "perspectiveAbove", 2700000, 20400000, 0, 0, "perspective", 0, 0, 0, 0, 0, 0, 25000,
              -340, 0, 0 }, // MS Office UI 0, 340, 0
            { "perspectiveAboveLeftFacing", 2700000, 0, 0, 0, "perspective", 0, 0, 0, 0, 0, 0,
              25000, -33.729692, 25.625585, -16.141175 }, // MS Office UI 14.3, 39.3, 341.1
            { "perspectiveAboveRightFacing", 2700000, 0, 0, 0, "perspective", 0, 0, 0, 0, 0, 0,
              25000, -33.729692, -25.625585, 16.141175 }, // MS Office UI 345.7, 39.3, 18.9
            { "perspectiveBelow", 2700000, 1200000, 0, 0, "perspective", 0, 0, 0, 0, 0, 0, 25000,
              -20, 0, 0 }, // MS Office UI 0, 20, 0
            { "perspectiveContrastingLeftFacing", 2700000, 624000, 2634000, 21384000, "perspective",
              0, 0, 0, 0, 0, 0, 25000, -10.4, 43.9, -356.4 }, // MS Office UI 43.9, 10.4, 356.4
            { "perspectiveContrastingRightFacing", 2700000, 624000, 18966000, 216000, "perspective",
              0, 0, 0, 0, 0, 0, 25000, -10.4, 316.1, 3.6 }, // MS Office UI 316.1, 10.4, 3.6
            { "perspectiveFront", 2700000, 0, 0, 0, "perspective", 0, 0, 0, 0, 0, 0, 25000, 0, 0,
              0 }, // MS Office UI 0, 0, 0
            { "perspectiveHeroicExtremeLeftFacing", 2700000, 486000, 2070000, 21426000,
              "perspective", 0, 0, 0, 0, 0, 0, 25000, -8.1, 34.5,
              357.1 }, // MS Office UI 34.5, 8.1, 357.1, 80
            { "perspectiveHeroicExtremeRightFacing", 2700000, 486000, 1953000, 174000,
              "perspective", 0, 0, 0, 0, 0, 0, 25000, -8.1, 325.5,
              2.9 }, // MS Office UI 325.5, 8.1, 2.9, 80
            { "perspectiveHeroicLeftFacing", 2700000, 0, 0, 0, "perspective", 0, 0, 0, 0, 0, 0,
              25000, 10.337163, 14.781420, 2.639612 }, // MS Office UI 14.3, 349, 2.6
            { "perspectiveHeroicRightFacing", 2700000, 0, 0, 0, "perspective", 0, 0, 0, 0, 0, 0,
              25000, 10.337163, -14.781420, -2.639612 }, // MS Office UI 345.7, 349, 357.4
            { "perspectiveLeft", 2700000, 0, 1200000, 0, "perspective", 0, 0, 0, 0, 0, 0, 25000, 0,
              20, 0 }, // MS Office UI 20, 0, 0
            { "perspectiveRelaxed", 2700000, 18576000, 0, 0, "perspective", 0, 0, 0, 0, 0, 0, 25000,
              -309.6, 0, 0 }, // MS Office UI 0, 309.6, 0
            { "perspectiveRelaxedModerately", 2700000, 19488000, 0, 0, "perspective", 0, 0, 0, 0, 0,
              0, 25000, -324.8, 0, 0 }, // MS Office UI 0, 324.8, 0
            { "perspectiveRight", 2700000, 0, 20400000, 0, "perspective", 0, 0, 0, 0, 0, 0, 25000,
              0, 340, 0 } // MS Office UI 340, 0, 0

        }
    {
    }

    // Dummy method to test Compilation
    CameraSettings getDefaultCameraSettings();
    void fillExtrusionWithPresetValues(const OUString sPresetName,
                                       oox::drawingml::Extrusion& rExtrusion,
                                       double& rShapeRotation);
    sal_uInt16 getPrstIndex(const OUString& rPresetType);
    double getRotationZDegFromPrst(const sal_uInt16 nPresetIndex);
    void getRotationAngleOOXFromPrst(oox::drawingml::RotationProperties& rRot,
                                     const sal_uInt16 nPresetIndex);
    void addCameraPrstSettingsToExtrusion(oox::drawingml::Extrusion& rExtrusion,
                                          const sal_uInt16 nPresetIndex);
    void calculateRotationAngleLODegFromAngleOOX(
        css::drawing::EnhancedCustomShapeParameterPair& rSceneRotDeg, double& rShapeRotDeg,
        const oox::drawingml::RotationProperties& aOOXRot);
}; // end struct */

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
